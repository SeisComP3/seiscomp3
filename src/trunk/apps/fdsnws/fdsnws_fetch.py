#!/usr/bin/env python
# -*- coding: utf-8 -*-

###############################################################################
# (C) 2014-2017 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ#
#                                                                             #
# License: LGPLv3 (https://www.gnu.org/copyleft/lesser.html)                  #
#                                                                             #
# modified by Fabian Euchner, ETHZ                                            #
###############################################################################

"""
A command-line FDSN Web Service client using EIDA routing and authentication.

Usage Examples
==============

Request 60 minutes of the ``"LHZ"`` channel of EIDA stations starting with
``"A"`` for a seismic event around 2010-02-27 07:00 (UTC). Optionally add
``"-v"`` for verbosity. Resulting Mini-SEED data will be written to file
``"data.mseed"``.

.. code-block:: bash

    $ %(prog)s -N '*' -S 'A*' -L '*' -C 'LHZ' \
-s "2010-02-27T07:00:00Z" -e "2010-02-27T08:00:00Z" -v -o data.mseed

The above request is anonymous and therefore restricted data will not be
included. To include restricted data, use a file containing a token obtained
from an EIDA authentication service and/or a CSV file with username and
password for each node not implementing the EIDA auth extension.

.. code-block:: bash

    $ %(prog)s -a token.asc -c credentials.csv -N '*' -S 'A*' -L '*' -C 'LHZ' \
-s "2010-02-27T07:00:00Z" -e "2010-02-27T08:00:00Z" -v -o data.mseed

StationXML metadata for the above request can be requested using the following
command:

.. code-block:: bash

    $ %(prog)s -N '*' -S 'A*' -L '*' -C 'LHZ' \
-s "2010-02-27T07:00:00Z" -e "2010-02-27T08:00:00Z" -y station \
-q level=response -v -o station.xml

Multiple query parameters can be used:

.. code-block:: bash

    $ %(prog)s -N '*' -S '*' -L '*' -C '*' \
-s "2010-02-27T07:00:00Z" -e "2010-02-27T08:00:00Z" -y station \
-q format=text -q level=channel -q latitude=20 -q longitude=-150 \
-q maxradius=15 -v -o station.txt

Bulk requests can be made in ArcLink (-f), breq_fast (-b) or native FDSNWS POST
(-p) format. Query parameters should not be included in the request file, but
specified on the command line.

.. code-block:: bash

    $ %(prog)s -p request.txt -y station -q level=channel -v -o station.xml
"""
from __future__ import absolute_import, division, print_function

import sys
import time
import datetime
import optparse
import threading
import socket
import csv
import re
import struct
import io
import os
import fnmatch
import subprocess
import dateutil.parser

try:
    # Python 3.2 and earlier
    from xml.etree import cElementTree as ET  # NOQA

except ImportError:
    from xml.etree import ElementTree as ET  # NOQA

try:
    # Python 2.x
    import Queue
    import urllib2
    import urlparse
    import urllib

except ImportError:
    # Python 3.x
    import queue as Queue
    import urllib.request as urllib2
    import urllib.parse as urlparse
    import urllib.parse as urllib

VERSION = "2017.271"

GET_PARAMS = set(('net', 'network',
                  'sta', 'station',
                  'loc', 'location',
                  'cha', 'channel',
                  'start', 'starttime',
                  'end', 'endtime',
                  'service',
                  'alternative'))

POST_PARAMS = set(('service',
                   'alternative'))

STATIONXML_RESOURCE_METADATA_ELEMENTS = (
    '{http://www.fdsn.org/xml/station/1}Source',
    '{http://www.fdsn.org/xml/station/1}Created',
    '{http://www.fdsn.org/xml/station/1}Sender',
    '{http://www.fdsn.org/xml/station/1}Module',
    '{http://www.fdsn.org/xml/station/1}ModuleURI')

FIXED_DATA_HEADER_SIZE = 48
DATA_ONLY_BLOCKETTE_SIZE = 8

DATA_ONLY_BLOCKETTE_NUMBER = 1000
MINIMUM_RECORD_LENGTH = 256

DEFAULT_TOKEN_LOCATION = os.environ.get("HOME", "") + "/.eidatoken"


class Error(Exception):
    pass


class AuthNotSupported(Exception):
    pass


class TargetURL(object):
    def __init__(self, url, qp):
        self.__scheme = url.scheme
        self.__netloc = url.netloc
        self.__path = url.path.rstrip('query').rstrip('/')
        self.__qp = dict(qp)

    def wadl(self):
        path = self.__path + '/application.wadl'
        return urlparse.urlunparse((self.__scheme,
                                    self.__netloc,
                                    path,
                                    '',
                                    '',
                                    ''))

    def auth(self):
        path = self.__path + '/auth'
        return urlparse.urlunparse(('https',
                                    self.__netloc,
                                    path,
                                    '',
                                    '',
                                    ''))

    def post(self):
        path = self.__path + '/query'
        return urlparse.urlunparse((self.__scheme,
                                    self.__netloc,
                                    path,
                                    '',
                                    '',
                                    ''))

    def post_qa(self):
        path = self.__path + '/queryauth'
        return urlparse.urlunparse((self.__scheme,
                                    self.__netloc,
                                    path,
                                    '',
                                    '',
                                    ''))

    def post_params(self):
        return self.__qp.items()


class RoutingURL(object):
    def __init__(self, url, qp):
        self.__scheme = url.scheme
        self.__netloc = url.netloc
        self.__path = url.path.rstrip('query').rstrip('/')
        self.__qp = dict(qp)

    def get(self):
        path = self.__path + '/query'
        qp = [(p, v) for (p, v) in self.__qp.items() if p in GET_PARAMS]
        qp.append(('format', 'post'))
        query = urllib.urlencode(qp)
        return urlparse.urlunparse((self.__scheme,
                                    self.__netloc,
                                    path,
                                    '',
                                    query,
                                    ''))

    def post(self):
        path = self.__path + '/query'
        return urlparse.urlunparse((self.__scheme,
                                    self.__netloc,
                                    path,
                                    '',
                                    '',
                                    ''))

    def post_params(self):
        qp = [(p, v) for (p, v) in self.__qp.items() if p in POST_PARAMS]
        qp.append(('format', 'post'))
        return qp

    def target_params(self):
        return [(p, v) for (p, v) in self.__qp.items() if p not in GET_PARAMS]


class TextCombiner(object):
    def __init__(self):
        self.__header = bytes()
        self.__text = bytes()

    def set_header(self, text):
        self.__header = text

    def combine(self, text):
        self.__text += text

    def dump(self, fd):
        if self.__text:
            fd.write(self.__header + self.__text)


class XMLCombiner(object):
    def __init__(self):
        self.__et = None

    def __combine_element(self, one, other):
        mapping = {}

        for el in one:
            try:
                eid = (el.tag, el.attrib['code'], el.attrib['startDate'])
                mapping[eid] = el

            except KeyError:
                pass

        for el in other:

            # skip Sender, Source, Module, ModuleURI, Created elements of
            # subsequent trees
            if el.tag in STATIONXML_RESOURCE_METADATA_ELEMENTS:
                continue

            try:
                eid = (el.tag, el.attrib['code'], el.attrib['startDate'])

                try:
                    self.__combine_element(mapping[eid], el)

                except KeyError:
                    mapping[eid] = el
                    one.append(el)

            except KeyError:
                pass

    def combine(self, et):
        if self.__et:
            self.__combine_element(self.__et.getroot(), et.getroot())

        else:
            self.__et = et
            root = self.__et.getroot()

            # Note: this assumes well-formed StationXML
            # first StationXML tree: modify Source, Created
            try:
                source = root.find(STATIONXML_RESOURCE_METADATA_ELEMENTS[0])
                source.text = 'FDSNWS'
            except Exception:
                pass

            try:
                created = root.find(STATIONXML_RESOURCE_METADATA_ELEMENTS[1])
                created.text = datetime.datetime.utcnow().strftime(
                    '%Y-%m-%dT%H:%M:%S')
            except Exception:
                pass

            # remove Sender, Module, ModuleURI
            for tag in STATIONXML_RESOURCE_METADATA_ELEMENTS[2:]:
                el = root.find(tag)
                if el is not None:
                    root.remove(el)

    def dump(self, fd):
        if self.__et:
            self.__et.write(fd)


class ArclinkParser(object):
    def __init__(self):
        self.postdata = ""
        self.failstr = ""

    def __parse_line(self, line):
        items = line.split()

        if len(items) < 2:
            self.failstr += "%s [syntax error]\n" % line
            return

        try:
            beg_time = datetime.datetime(*map(int, items[0].split(",")))
            end_time = datetime.datetime(*map(int, items[1].split(",")))

        except ValueError as e:
            self.failstr += "%s [invalid begin or end time: %s]\n" \
                            % (line, str(e))
            return

        network = 'XX'
        station = 'XXXXX'
        channel = 'XXX'
        location = '--'

        if len(items) > 2 and items[2] != '.':
            network = items[2]

            if len(items) > 3 and items[3] != '.':
                station = items[3]

                if len(items) > 4 and items[4] != '.':
                    channel = items[4]

                    if len(items) > 5 and items[5] != '.':
                        location = items[5]

        self.postdata += "%s %s %s %s %sZ %sZ\n" \
                         % (network,
                            station,
                            location,
                            channel,
                            beg_time.isoformat(),
                            end_time.isoformat())

    def parse(self, path):
        with open(path) as fd:
            for line in fd:
                line = line.rstrip()

                if line:
                    self.__parse_line(line)


class BreqParser(object):
    __tokenrule = "^\.[A-Z_]+[:]?\s"

    __reqlist = ("(?P<station>[\w?\*]+)",
                 "(?P<network>[\w?]+)",
                 "((?P<beg_2year>\d{2})|(?P<beg_4year>\d{4}))",
                 "(?P<beg_month>\d{1,2})",
                 "(?P<beg_day>\d{1,2})",
                 "(?P<beg_hour>\d{1,2})",
                 "(?P<beg_min>\d{1,2})",
                 "(?P<beg_sec>\d{1,2})(\.\d*)?",
                 "((?P<end_2year>\d{2})|(?P<end_4year>\d{4}))",
                 "(?P<end_month>\d{1,2})",
                 "(?P<end_day>\d{1,2})",
                 "(?P<end_hour>\d{1,2})",
                 "(?P<end_min>\d{1,2})",
                 "(?P<end_sec>\d{1,2})(\.\d*)?",
                 "(?P<cha_num>\d+)",
                 "(?P<cha_list>[\w?\s*]+)")

    def __init__(self):
        self.__rx_tokenrule = re.compile(BreqParser.__tokenrule)
        self.__rx_reqlist = re.compile("\s+".join(BreqParser.__reqlist))
        self.postdata = ""
        self.failstr = ""

    def __parse_line(self, line):
        m = self.__rx_reqlist.match(line)

        if m:
            d = m.groupdict()

            # catch two digit year inputs
            if d["beg_2year"]:
                if int(d["beg_2year"]) > 50:
                    d["beg_4year"] = "19%s" % d["beg_2year"]

                else:
                    d["beg_4year"] = "20%s" % d["beg_2year"]

            if d["end_2year"]:
                if int(d["end_2year"]) > 50:
                    d["end_4year"] = "19%s" % d["end_2year"]

                else:
                    d["end_4year"] = "20%s" % d["end_2year"]

            # some users have problems with time...
            if int(d["beg_hour"]) > 23:
                d["beg_hour"] = "23"

            if int(d["end_hour"]) > 23:
                d["end_hour"] = "23"

            if int(d["beg_min"]) > 59:
                d["beg_min"] = "59"

            if int(d["end_min"]) > 59:
                d["end_min"] = "59"

            if int(d["beg_sec"]) > 59:
                d["beg_sec"] = "59"

            if int(d["end_sec"]) > 59:
                d["end_sec"] = "59"

            try:
                beg_time = datetime.datetime(int(d["beg_4year"]),
                                             int(d["beg_month"]),
                                             int(d["beg_day"]),
                                             int(d["beg_hour"]),
                                             int(d["beg_min"]),
                                             int(d["beg_sec"]))

                end_time = datetime.datetime(int(d["end_4year"]),
                                             int(d["end_month"]),
                                             int(d["end_day"]),
                                             int(d["end_hour"]),
                                             int(d["end_min"]),
                                             int(d["end_sec"]))

            except ValueError as e:
                self.failstr += "%s [error: wrong begin or end time: %s]\n" \
                                % (line, str(e))
                return

            location = "*"
            cha_list = re.findall("([\w?\*]+)\s*", d["cha_list"])

            if len(cha_list) == int(d['cha_num'])+1:
                location = cha_list.pop()

            for channel in cha_list:
                self.postdata += "%s %s %s %s %sZ %sZ\n" \
                                 % (d["network"],
                                    d["station"],
                                    location,
                                    channel,
                                    beg_time.isoformat(),
                                    end_time.isoformat())

        else:
            self.failstr += "%s [syntax error]\n" % line

    def parse(self, path):
        with open(path) as fd:
            for line in fd:
                if self.__rx_tokenrule.match(line):
                    continue

                line = line.rstrip()

                if line:
                    self.__parse_line(line)


msglock = threading.Lock()


def msg(s, verbose=3):
    if verbose:
        if verbose == 3:
            if sys.stderr.isatty():
                s = "\033[31m" + s + "\033[m"

        elif verbose == 2:
            if sys.stderr.isatty():
                s = "\033[32m" + s + "\033[m"

        with msglock:
            sys.stderr.write(s + '\n')
            sys.stderr.flush()


def retry(urlopen, url, data, timeout, count, wait, verbose):
    n = 0

    while True:
        if n >= count:
            return urlopen(url, data, timeout)

        try:
            n += 1

            fd = urlopen(url, data, timeout)

            if fd.getcode() == 200 or fd.getcode() == 204:
                return fd

            msg("retrying %s (%d) after %d seconds due to HTTP status code %d"
                % (url, n, wait, fd.getcode()), verbose)

            fd.close()
            time.sleep(wait)

        except urllib2.HTTPError as e:
            if e.code >= 400 and e.code < 500:
                raise

            msg("retrying %s (%d) after %d seconds due to %s"
                % (url, n, wait, str(e)), verbose)

            time.sleep(wait)

        except (urllib2.URLError, socket.error) as e:
            msg("retrying %s (%d) after %d seconds due to %s"
                % (url, n, wait, str(e)), verbose)

            time.sleep(wait)


def fetch(url, cred, authdata, postlines, xc, tc, dest, nets, chans,
          timeout, retry_count, retry_wait, finished, lock, verbose):
    try:
        url_handlers = []

        if cred and url.post_qa() in cred:  # use static credentials
            query_url = url.post_qa()
            (user, passwd) = cred[query_url]
            mgr = urllib2.HTTPPasswordMgrWithDefaultRealm()
            mgr.add_password(None, query_url, user, passwd)
            h = urllib2.HTTPDigestAuthHandler(mgr)
            url_handlers.append(h)

        elif authdata:  # use the pgp-based auth method if supported
            wadl_url = url.wadl()
            auth_url = url.auth()
            query_url = url.post_qa()

            try:
                fd = retry(urllib2.urlopen, wadl_url, None, timeout,
                           retry_count, retry_wait, verbose)

                try:
                    root = ET.parse(fd).getroot()
                    ns = "{http://wadl.dev.java.net/2009/02}"
                    el = "resource[@path='auth']"

                    if root.find(".//" + ns + el) is None:
                        raise AuthNotSupported

                finally:
                    fd.close()

                msg("authenticating at %s" % auth_url, verbose)

                try:
                    fd = retry(urllib2.urlopen, auth_url, authdata, timeout,
                               retry_count, retry_wait, verbose)

                    try:
                        resp = fd.read()

                        if isinstance(resp, bytes):
                            resp = resp.decode('utf-8')

                        if fd.getcode() == 200:
                            try:
                                (user, passwd) = resp.split(':')
                                mgr = urllib2.HTTPPasswordMgrWithDefaultRealm()
                                mgr.add_password(None, query_url, user, passwd)
                                h = urllib2.HTTPDigestAuthHandler(mgr)
                                url_handlers.append(h)

                            except ValueError:
                                msg("invalid auth response: %s" % resp)
                                return

                            msg("authentication at %s successful"
                                % auth_url, verbose)

                        else:
                            msg("authentication at %s failed with HTTP status "
                                "code %d:\n%s" % (auth_url, fd.getcode(), resp))

                            query_url = url.post()

                    finally:
                        fd.close()

                except urllib2.HTTPError as e:
                    resp = e.read()

                    if isinstance(resp, bytes):
                        resp = resp.decode('utf-8')

                    msg("authentication at %s failed with HTTP status "
                        "code %d:\n%s" % (auth_url, e.code, resp))

                    query_url = url.post()

                except (urllib2.URLError, socket.error) as e:
                    msg("authentication at %s failed: %s" % (auth_url, str(e)))
                    query_url = url.post()

            except (urllib2.URLError, socket.error, ET.ParseError) as e:
                msg("reading %s failed: %s" % (wadl_url, str(e)))
                query_url = url.post()

            except AuthNotSupported:
                msg("authentication at %s is not supported"
                    % auth_url, verbose)

                query_url = url.post()

        else:  # fetch data anonymously
            query_url = url.post()

        opener = urllib2.build_opener(*url_handlers)

        i = 0
        n = len(postlines)

        while i < len(postlines):
            if n == len(postlines):
                msg("getting data from %s" % query_url, verbose)

            else:
                msg("getting data from %s (%d%%..%d%%)"
                    % (query_url,
                       100*i/len(postlines),
                       min(100, 100*(i+n)/len(postlines))),
                    verbose)

            postdata = (''.join((p + '=' + v + '\n')
                                for (p, v) in url.post_params()) +
                        ''.join(postlines[i:i+n]))

            if not isinstance(postdata, bytes):
                postdata = postdata.encode('utf-8')

            try:
                fd = retry(opener.open, query_url, postdata, timeout,
                           retry_count, retry_wait, verbose)

                try:
                    if fd.getcode() == 204:
                        msg("received no data from %s" % query_url)

                    elif fd.getcode() != 200:
                        resp = fd.read()

                        if isinstance(resp, bytes):
                            resp = resp.decode('utf-8')

                        msg("getting data from %s failed with HTTP status "
                            "code %d:\n%s" % (query_url, fd.getcode(), resp))

                        break

                    else:
                        size = 0

                        content_type = fd.info().get('Content-Type')
                        content_type = content_type.split(';')[0]

                        if content_type == "application/vnd.fdsn.mseed":

                            record_idx = 1

                            # NOTE: cannot use fixed chunk size, because
                            # response from single node mixes mseed record
                            # sizes. E.g., a 4096 byte chunk could contain 7
                            # 512 byte records and the first 512 bytes of a
                            # 4096 byte record, which would not be completed
                            # in the same write operation
                            while True:

                                # read fixed header
                                buf = fd.read(FIXED_DATA_HEADER_SIZE)
                                if not buf:
                                    break

                                record = buf
                                curr_size = len(buf)

                                # get offset of data (value before last,
                                # 2 bytes, unsigned short)
                                data_offset_idx = FIXED_DATA_HEADER_SIZE - 4
                                data_offset, = struct.unpack(
                                    b'!H',
                                    buf[data_offset_idx:data_offset_idx+2])

                                if data_offset >= FIXED_DATA_HEADER_SIZE:
                                    remaining_header_size = data_offset - \
                                        FIXED_DATA_HEADER_SIZE

                                elif data_offset == 0:
                                    # This means that blockettes can follow,
                                    # but no data samples. Use minimum record
                                    # size to read following blockettes. This
                                    # can still fail if blockette 1000 is after
                                    # position 256
                                    remaining_header_size = \
                                        MINIMUM_RECORD_LENGTH - \
                                        FIXED_DATA_HEADER_SIZE

                                else:
                                    # Full header size cannot be smaller than
                                    # fixed header size. This is an error.
                                    msg("record %s: data offset smaller than "
                                        "fixed header length: %s, bailing "
                                        "out" % (record_idx, data_offset))
                                    break

                                buf = fd.read(remaining_header_size)
                                if not buf:
                                    msg("remaining header corrupt in record "
                                        "%s" % record_idx)
                                    break

                                record += buf
                                curr_size += len(buf)

                                # scan variable header for blockette 1000
                                blockette_start = 0
                                b1000_found = False

                                while (blockette_start < remaining_header_size):

                                    # 2 bytes, unsigned short
                                    blockette_id, = struct.unpack(
                                        b'!H',
                                        buf[blockette_start:blockette_start+2])

                                    # get start of next blockette (second
                                    # value, 2 bytes, unsigned short)
                                    next_blockette_start, = struct.unpack(
                                        b'!H',
                                        buf[blockette_start+2:blockette_start+4])

                                    if blockette_id == \
                                            DATA_ONLY_BLOCKETTE_NUMBER:

                                        b1000_found = True
                                        break

                                    elif next_blockette_start == 0:
                                        # no blockettes follow
                                        msg("record %s: no blockettes follow "
                                            "after blockette %s at pos %s" % (
                                                record_idx, blockette_id,
                                                blockette_start))
                                        break

                                    else:
                                        blockette_start = next_blockette_start

                                # blockette 1000 not found
                                if not b1000_found:
                                    msg("record %s: blockette 1000 not found,"
                                        " stop reading" % record_idx)
                                    break

                                # get record size (1 byte, unsigned char)
                                record_size_exponent_idx = blockette_start + 6
                                record_size_exponent, = struct.unpack(
                                    b'!B',
                                    buf[record_size_exponent_idx:
                                        record_size_exponent_idx+1])

                                remaining_record_size = \
                                    2**record_size_exponent - curr_size

                                # read remainder of record (data section)
                                buf = fd.read(remaining_record_size)
                                if not buf:
                                    msg("cannot read data section of record "
                                        "%s" % record_idx)
                                    break

                                record += buf

                                # collect network IDs
                                try:
                                    net = record[18:20].decode(
                                        'ascii').rstrip()
                                    sta = record[8:13].decode('ascii').rstrip()
                                    loc = record[13:15].decode(
                                        'ascii').rstrip()
                                    cha = record[15:18].decode(
                                        'ascii').rstrip()

                                except UnicodeDecodeError:
                                    msg("invalid miniseed record")
                                    break

                                year, = struct.unpack(b'!H', record[20:22])

                                with lock:
                                    nets.add((net, year))
                                    chans.add('.'.join((net, sta, loc, cha)))
                                    dest.write(record)

                                size += len(record)
                                record_idx += 1

                        elif content_type == "text/plain":

                            # this is the station service in text format
                            text = bytes()

                            while True:
                                buf = fd.readline()

                                if not buf:
                                    break

                                if buf.startswith(b'#'):
                                    tc.set_header(buf)

                                else:
                                    text += buf

                                size += len(buf)

                            with lock:
                                tc.combine(text)

                        elif content_type == "application/xml":
                            fdread = fd.read
                            s = [0]

                            def read(self, *args, **kwargs):
                                buf = fdread(self, *args, **kwargs)
                                s[0] += len(buf)
                                return buf

                            fd.read = read
                            et = ET.parse(fd)
                            size = s[0]

                            with lock:
                                xc.combine(et)

                        else:
                            msg("getting data from %s failed: unsupported "
                                "content type '%s'" % (query_url,
                                                       content_type))

                            break

                        msg("got %d bytes (%s) from %s"
                            % (size, content_type, query_url), verbose)

                    i += n

                finally:
                    fd.close()

            except urllib2.HTTPError as e:
                if e.code == 413 and n > 1:
                    msg("request too large for %s, splitting"
                        % query_url, verbose)

                    n = -(n//-2)

                else:
                    resp = e.read()

                    if isinstance(resp, bytes):
                        resp = resp.decode('utf-8')

                    msg("getting data from %s failed with HTTP status "
                        "code %d:\n%s" % (query_url, e.code, resp))

                    break

            except (urllib2.URLError, socket.error, ET.ParseError) as e:
                msg("getting data from %s failed: %s"
                    % (query_url, str(e)))

                break

    finally:
        finished.put(threading.current_thread())


def route(url, cred, authdata, postdata, dest, chans_to_check, timeout,
          retry_count, retry_wait, maxthreads, verbose):
    threads = []
    running = 0
    finished = Queue.Queue()
    lock = threading.Lock()
    xc = XMLCombiner()
    tc = TextCombiner()
    nets = set()
    check = bool(chans_to_check)
    chans1 = chans_to_check
    chans2 = set()
    chans3 = set()

    if postdata:
        query_url = url.post()
        postdata = (''.join((p + '=' + v + '\n')
                            for (p, v) in url.post_params()) +
                    postdata)

        if not isinstance(postdata, bytes):
            postdata = postdata.encode('utf-8')

    else:
        query_url = url.get()

    msg("getting routes from %s" % query_url, verbose)

    try:
        fd = retry(urllib2.urlopen, query_url, postdata, timeout, retry_count,
                   retry_wait, verbose)

        try:
            if fd.getcode() == 204:
                msg("received no routes from %s" % query_url)

            elif fd.getcode() != 200:
                resp = fd.read()

                if isinstance(resp, bytes):
                    resp = resp.decode('utf-8')

                msg("getting routes from %s failed with HTTP status "
                    "code %d:\n%s" % (query_url, fd.getcode(), resp))

            else:
                urlline = None
                postlines = []

                while True:
                    line = fd.readline()

                    if isinstance(line, bytes):
                        line = line.decode('utf-8')

                    if not urlline:
                        urlline = line.strip()

                    elif not line.strip():
                        if postlines:
                            target_url = TargetURL(urlparse.urlparse(urlline),
                                                   url.target_params())
                            threads.append(threading.Thread(target=fetch,
                                                            args=(target_url,
                                                                  cred,
                                                                  authdata,
                                                                  postlines,
                                                                  xc,
                                                                  tc,
                                                                  dest,
                                                                  nets,
                                                                  chans3,
                                                                  timeout,
                                                                  retry_count,
                                                                  retry_wait,
                                                                  finished,
                                                                  lock,
                                                                  verbose)))

                        urlline = None
                        postlines = []

                        if not line:
                            break

                    else:
                        postlines.append(line)

                        if check:
                            nslc = line.split()[:4]
                            if nslc[2] == '--':
                                nslc[2] = ''
                            chans2.add('.'.join(nslc))

        finally:
            fd.close()

    except urllib2.HTTPError as e:
        resp = e.read()

        if isinstance(resp, bytes):
            resp = resp.decode('utf-8')

        msg("getting routes from %s failed with HTTP status "
            "code %d:\n%s" % (query_url, e.code, resp))

    except (urllib2.URLError, socket.error) as e:
        msg("getting routes from %s failed: %s" % (query_url, str(e)))

    if check:
        for c1 in list(chans1):
            for c2 in list(chans2):
                if fnmatch.fnmatch(c2, c1):
                    chans1.remove(c1)
                    break

        if chans1:
            msg("did not receive routes to %s" % ", ".join(sorted(chans1)))

    for t in threads:
        if running >= maxthreads:
            thr = finished.get(True)
            thr.join()
            running -= 1

        t.start()
        running += 1

    while running:
        thr = finished.get(True)
        thr.join()
        running -= 1

    xc.dump(dest)
    tc.dump(dest)

    if check:
        for p in url.post_params():
            if p[0] == 'service' and p[1] != 'dataselect':
                return nets

        for c2 in list(chans2):
            for c3 in list(chans3):
                if fnmatch.fnmatch(c3, c2):
                    chans2.remove(c2)
                    break

        if chans2:
            msg("did not receive data from %s" % ", ".join(sorted(chans2)))

    return nets


def get_citation(nets, options):
    postdata = ""
    for (net, year) in nets:
        postdata += "%s * * * %d-01-01T00:00:00Z %d-12-31T23:59:59Z\n" \
                    % (net, year, year)

    qp = {'service': 'station', 'level': 'network', 'format': 'text'}
    url = RoutingURL(urlparse.urlparse(options.url), qp)
    dest = io.BytesIO()

    route(url, None, None, postdata, dest, None, options.timeout,
          options.retries, options.retry_wait, options.threads,
          options.verbose)

    dest.seek(0)
    net_desc = {}

    for line in dest:
        try:
            if isinstance(line, bytes):
                line = line.decode('utf-8')

            (code, desc, start) = line.split('|')[:3]

            if code.startswith('#'):
                continue

            year = dateutil.parser.parse(start).year

        except (ValueError, UnicodeDecodeError) as e:
            msg("error parsing text format: %s" % str(e))
            continue

        if code[0] in '0123456789XYZ':
            net_desc['%s_%d' % (code, year)] = desc

        else:
            net_desc[code] = desc

    msg("\nYou received seismic waveform data from the following network(s):", 2)

    for code in sorted(net_desc):
        msg("%s %s" % (code, net_desc[code]), 2)

    msg("\nAcknowledgment is extremely important for network operators\n"
        "providing open data. When preparing publications, please\n"
        "cite the data appropriately. The FDSN service at\n\n"
        "    http://www.fdsn.org/networks/citation/?networks=%s\n\n"
        "provides a helpful guide based on available network\n"
        "Digital Object Identifiers.\n"
        % "+".join(sorted(net_desc)), 2)


def main():
    qp = {}

    def add_qp(option, opt_str, value, parser):
        if option.dest == 'query':
            try:
                (p, v) = value.split('=', 1)
                qp[p] = v

            except ValueError:
                raise optparse.OptionValueError("%s expects parameter=value"
                                                % opt_str)

        else:
            qp[option.dest] = value

    parser = optparse.OptionParser(
        usage="Usage: %prog [-h|--help] [OPTIONS] -o file",
        version="%prog " + VERSION,
        add_help_option=False)

    parser.set_defaults(
        url="http://geofon.gfz-potsdam.de/eidaws/routing/1/",
        timeout=600,
        retries=10,
        retry_wait=60,
        threads=5)

    parser.add_option("-h", "--help", action="store_true", default=False,
                      help="show help message and exit")

    parser.add_option("-l", "--longhelp", action="store_true", default=False,
                      help="show extended help message and exit")

    parser.add_option("-v", "--verbose", action="store_true", default=False,
                      help="verbose mode")

    parser.add_option("-u", "--url", type="string",
                      help="URL of routing service (default %default)")

    parser.add_option("-y", "--service", type="string", action="callback",
                      callback=add_qp,
                      help="target service (default dataselect)")

    parser.add_option("-N", "--network", type="string", action="callback",
                      callback=add_qp,
                      help="network code or pattern")

    parser.add_option("-S", "--station", type="string", action="callback",
                      callback=add_qp,
                      help="station code or pattern")

    parser.add_option("-L", "--location", type="string", action="callback",
                      callback=add_qp,
                      help="location code or pattern")

    parser.add_option("-C", "--channel", type="string", action="callback",
                      callback=add_qp,
                      help="channel code or pattern")

    parser.add_option("-s", "--starttime", type="string", action="callback",
                      callback=add_qp,
                      help="start time")

    parser.add_option("-e", "--endtime", type="string", action="callback",
                      callback=add_qp,
                      help="end time")

    parser.add_option("-q", "--query", type="string", action="callback",
                      callback=add_qp, metavar="PARAMETER=VALUE",
                      help="additional query parameter")

    parser.add_option("-t", "--timeout", type="int",
                      help="request timeout in seconds (default %default)")

    parser.add_option("-r", "--retries", type="int",
                      help="number of retries (default %default)")

    parser.add_option("-w", "--retry-wait", type="int",
                      help="seconds to wait before each retry "
                           "(default %default)")

    parser.add_option("-n", "--threads", type="int",
                      help="maximum number of download threads "
                           "(default %default)")

    parser.add_option("-c", "--credentials-file", type="string",
                      help="URL,user,password file (CSV format) for queryauth")

    parser.add_option("-a", "--auth-file", type="string",
                      help="file that contains the auth token")

    parser.add_option("-p", "--post-file", type="string",
                      help="request file in FDSNWS POST format")

    parser.add_option("-f", "--arclink-file", type="string",
                      help="request file in ArcLink format")

    parser.add_option("-b", "--breqfast-file", type="string",
                      help="request file in breq_fast format")

    parser.add_option("-o", "--output-file", type="string",
                      help="file where downloaded data is written")

    parser.add_option("-z", "--no-citation", action="store_true", default=False,
                      help="suppress network citation info")

    parser.add_option("-Z", "--no-check", action="store_true", default=False,
                      help="suppress checking received routes and data")

    (options, args) = parser.parse_args()

    if options.help:
        print(__doc__.split("Usage Examples", 1)[0], end="")
        parser.print_help()
        return 0

    if options.longhelp:
        print(__doc__)
        parser.print_help()
        return 0

    if args or not options.output_file:
        parser.print_usage(sys.stderr)
        return 1

    if bool(options.post_file) + bool(options.arclink_file) + \
            bool(options.breqfast_file) > 1:
        msg("only one of (--post-file, --arclink-file, --breqfast-file) "
            "can be used")
        return 1

    try:
        cred = {}
        authdata = None
        postdata = None
        chans_to_check = set()

        if options.credentials_file:
            with open(options.credentials_file) as fd:
                try:
                    for (url, user, passwd) in csv.reader(fd):
                        cred[url] = (user, passwd)

                except (ValueError, csv.Error):
                    raise Error("error parsing %s" % options.credentials_file)

                except UnicodeDecodeError:
                    raise Error("invalid unicode character found in %s"
                                % options.credentials_file)

        if options.auth_file:
            with open(options.auth_file, 'rb') as fd:
                authdata = fd.read()

        else:
            try:
                with open(DEFAULT_TOKEN_LOCATION, 'rb') as fd:
                    authdata = fd.read()
                    options.auth_file = DEFAULT_TOKEN_LOCATION

            except IOError:
                pass

        if authdata:
            msg("using token in %s:" % options.auth_file, options.verbose)

            try:
                proc = subprocess.Popen(['gpg', '--decrypt'],
                                        stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE)

                out, err = proc.communicate(authdata)

                if not out:
                    if isinstance(err, bytes):
                        err = err.decode('utf-8')

                    msg(err)
                    return 1

                if isinstance(out, bytes):
                    out = out.decode('utf-8')

                msg(out, options.verbose)

            except OSError as e:
                msg(str(e))

        if options.post_file:
            try:
                with open(options.post_file) as fd:
                    postdata = fd.read()

            except UnicodeDecodeError:
                raise Error("invalid unicode character found in %s"
                            % options.post_file)

        else:
            parser = None

            if options.arclink_file:
                parser = ArclinkParser()

                try:
                    parser.parse(options.arclink_file)

                except UnicodeDecodeError:
                    raise Error("invalid unicode character found in %s"
                                % options.arclink_file)

            elif options.breqfast_file:
                parser = BreqParser()

                try:
                    parser.parse(options.breqfast_file)

                except UnicodeDecodeError:
                    raise Error("invalid unicode character found in %s"
                                % options.breqfast_file)

            if parser is not None:
                if parser.failstr:
                    msg(parser.failstr)
                    return 1

                postdata = parser.postdata

        if not options.no_check:
            if postdata:
                for line in postdata.splitlines():
                    nslc = line.split()[:4]
                    if nslc[2] == '--':
                        nslc[2] = ''
                    chans_to_check.add('.'.join(nslc))

            else:
                net = qp.get('network', '*')
                sta = qp.get('station', '*')
                loc = qp.get('location', '*')
                cha = qp.get('channel', '*')

                for n in net.split(','):
                    for s in sta.split(','):
                        for l in loc.split(','):
                            for c in cha.split(','):
                                if l == '--':
                                    l = ''
                                chans_to_check.add('.'.join((n, s, l, c)))

        url = RoutingURL(urlparse.urlparse(options.url), qp)
        dest = open(options.output_file, 'wb')

        nets = route(url, cred, authdata, postdata, dest, chans_to_check,
                     options.timeout, options.retries, options.retry_wait,
                     options.threads, options.verbose)

        if nets and not options.no_citation:
            msg("retrieving network citation info", options.verbose)
            get_citation(nets, options)

        else:
            msg("", options.verbose)

        msg("In case of problems with your request, plese use the contact "
            "form at\n\n"
            "    http://www.orfeus-eu.org/organization/contact/form/"
            "?recipient=EIDA\n", options.verbose)

    except (IOError, Error) as e:
        msg(str(e))
        return 1

    return 0


if __name__ == "__main__":
    __doc__ %= {"prog": sys.argv[0]}
    sys.exit(main())
