#***************************************************************************** 
# manager.py
#
# ArcLink higher-level client support
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import socket
import cStringIO
import threading
import fnmatch
from tempfile import NamedTemporaryFile
from seiscomp import logs
from seiscomp.db.generic.routing import Routing as _Routing
from seiscomp.db.generic.inventory import Inventory as _Inventory
from seiscomp.arclink.client import *

_DEFAULT_USER = "guest@anywhere"
_SOCKET_TIMEOUT = 300
_REQUEST_TIMEOUT = 300  # currently not used
_DOWNLOAD_RETRY = 5
_MAX_REQ_LINES = 990
_MAX_REQ_MB = 500

class RequestLine(object):
    __slots__ = ('start_time', 'end_time', 'net', 'sta', 'cha', 'loc',
        'constraints', 'routes_tried', 'estimated_size')

    def __init__(self, start_time, end_time, net, sta, cha, loc,
            constraints=None, routes_tried=None, estimated_size=0):
        self.start_time = start_time
        self.end_time = end_time
        if net and net != '.': self.net = net
        else: self.net = ''
        if sta and sta != '.': self.sta = sta
        else: self.sta = ''
        if cha and cha != '.': self.cha = cha
        else: self.cha = ''
        if loc and loc != '.': self.loc = loc
        else: self.loc = ''
        if constraints: self.constraints = constraints
        else: self.constraints = {}
        if routes_tried: self.routes_tried = routes_tried
        else: self.routes_tried = set()
        self.estimated_size = estimated_size

    def items(self):
        return (self.start_time, self.end_time, self.net, self.sta, self.cha, self.loc,
            self.constraints)

    # for backwards compatibility
    def __getitem__(self, i):
        if self.net: net = self.net
        else: net = '.'
        if self.sta: sta = self.sta
        else: sta = '.'
        if self.cha: cha = self.cha
        else: cha = '.'
        if self.loc: loc = self.loc
        else: loc = '.'
        return (net, sta, cha, loc,
            self.start_time, self.end_time, self.constraints, self.routes_tried)[i]

    def __repr__(self):
        if self.net: net = self.net
        else: net = '.'
        if self.sta: sta = self.sta
        else: sta = '.'
        if self.cha: cha = self.cha
        else: cha = '.'
        if self.loc: loc = self.loc
        else: loc = '.'
        return ("%d,%d,%d,%d,%d,%d %d,%d,%d,%d,%d,%d %s %s %s %s %s" % (
            self.start_time.year, self.start_time.month, self.start_time.day,
            self.start_time.hour, self.start_time.minute, self.start_time.second,
            self.end_time.year, self.end_time.month, self.end_time.day,
            self.end_time.hour, self.end_time.minute, self.end_time.second,
            net, sta, cha, loc,
            " ".join([ "%s=%s" % (a, v) for a, v in self.constraints.items() ]))).rstrip()

class _Request(object):
    def __init__(self, rtype, args, label, socket_timeout, request_timeout,
        download_retry):
        self.rtype = rtype
        self.args = args
        self.label = label
        self.socket_timeout = socket_timeout
        self.request_timeout = request_timeout
        self.download_retry = download_retry
        self.content = []
        self.address = None
        self.dcname = None
        self.error = None
        self.id = None
        self.__arcl = Arclink()
        self.__arcl_wait = None
        self.__host = None
        self.__port = None
        self.__user = None
        self.__passwd = None
        self.__status = None
#BIANCHI|ENCRYPTION: Password for decrypting data.
        self.encStatus = None
        self.decStatus = None

    def new(self):
        return _Request(self.rtype, self.args, self.label, self.socket_timeout,
            self.request_timeout, self.download_retry)

    def __add_nscl_first(self, net, sta, cha, loc, start_time, end_time,
            constraints=None, routes_tried=None, estimated_size=0):
        self.content.append(RequestLine(start_time, end_time, net, sta, cha, loc,
            constraints, routes_tried, estimated_size))

    def __add_time_first(self, start_time, end_time, net, sta, cha, loc,
            constraints=None, routes_tried=None, estimated_size=0):
        self.content.append(RequestLine(start_time, end_time, net, sta, cha, loc,
            constraints, routes_tried, estimated_size))

    def add(self, *items):
        if isinstance(items[0], datetime.datetime):
            self.__add_time_first(*items)
        else:
            # for backwards compatibility
            self.__add_nscl_first(*items)

    def dump(self, fd):
        if self.label:
             print >>fd, "LABEL %s" % (self.label,)

        print >>fd, "REQUEST %s %s" % (self.rtype,
            " ".join([ "%s=%s" % (a, v) for a, v in self.args.items() ]))

        for rl in self.content:
            print >>fd, repr(rl)

        print >>fd, "END"

    def submit(self, addr, user, passwd=None, user_ip=None):
        try:
            (host, port) = addr.split(':')
            port = int(port)

        except ValueError:
            self.error = "invalid ArcLink address"
            return

        try:
            self.__arcl.open_connection(host, port, user, passwd, None,
                self.socket_timeout, user_ip)

            try:
                reqf = cStringIO.StringIO()
                try:
                    self.dump(reqf)
                    reqf.seek(0)
                    self.id = self.__arcl.submit(reqf)
                    self.__host = host
                    self.__port = port
                    self.__user = user
                    self.__passwd = passwd
                    self.address = addr
                    self.dcname = self.__arcl.organization

                finally:
                    reqf.close()

            finally:
                self.__arcl.close_connection()

        except (ArclinkError, socket.error), e:
            self.error = str(e)

    def status(self):
        if self.id is None:
            raise ArclinkError, "request not submitted"

        if self.__status:
            return self.__status

        self.__arcl.open_connection(self.__host, self.__port, self.__user,
            self.__passwd, None, self.socket_timeout)

        try:
            # This method should return the FULL status including the ARCLINK node.
            # This would avoid the creation of the 'dcencryption' variable
            status = self.__arcl.get_status(self.id).request[0]
            if status.ready:
                self.__status = status

            return status

        finally:
            self.__arcl.close_connection()

    def wait(self):
        if self.id is None:
            raise ArclinkError, "request not submitted"

        self.__arcl_wait = Arclink()

        retry = 0

        while True:
            self.__arcl_wait.open_connection(self.__host, self.__port,
                self.__user, self.__passwd, None, self.socket_timeout)

            try:
                self.__arcl_wait.wait(self.id, timeout=self.socket_timeout)
                break

            except ArclinkTimeout, e:
                reason = str(e)

                retry += 1
                if retry > self.download_retry:
                    self.__arcl_wait.close_connection()
                    self.__arcl_wait = None
                    raise ArclinkError, "download failed: " + reason

    def close(self):
        if self.__arcl_wait:
            self.__arcl_wait.close_connection()
            self.__arcl_wait = None

    def download_data(self, fd, vol_id=None, block=False, purge=True, password=None, raw=False):
        if self.id is None:
            raise ArclinkError, "request not submitted"

        retry = 0
        if block:
            tmo = self.socket_timeout
        else:
            tmo = 0

        if vol_id is not None:
            self.close()

        while True:
            if self.__arcl_wait:
                arcl = self.__arcl_wait
                self.__arcl_wait = None
            else:
                arcl = self.__arcl
                arcl.open_connection(self.__host, self.__port,
                    self.__user, self.__passwd, None, self.socket_timeout)

            try:
                try:
#BIANCHI|ENCRYPTION: Check what we will get.
                    (self.encStatus, self.decStatus) = arcl.download_data(fd, self.id, vol_id, timeout=tmo, password=password, raw=raw)

                    if purge:
                        arcl.purge(self.id)
                        self.id = None

                    break

                except ArclinkTimeout, e:
                    reason = str(e)

                    try:
                        if arcl.errstate():
                            fd.seek(0)
                            fd.truncate(0)

                    except IOError:
                        raise ArclinkError, "download failed: " + reason

                    retry += 1
                    if retry > self.download_retry:
                        raise ArclinkError, "download failed: " + reason

            finally:
                arcl.close_connection()

    def download_xml(self, db, block=False, purge=True):
        if self.id is None:
            raise ArclinkError, "request not submitted"

        retry = 0
        if block:
            tmo = self.socket_timeout
        else:
            tmo = 0

        while True:
            arcl = self.__arcl
            self.__arcl_wait = None
            arcl.open_connection(self.__host, self.__port,
                self.__user, self.__passwd, None, self.socket_timeout)

            try:
                try:
                    arcl.download_xml(db, self.id, timeout=tmo)

                    if purge:
                        arcl.purge(self.id)
                        self.id = None

                    break

                except ArclinkTimeout, e:
                    retry += 1
                    if retry > self.download_retry:
                        raise ArclinkError, "download failed: " + str(e)

            finally:
                arcl.close_connection()

    def purge(self):
        if self.id is None:
            return

        self.__arcl.open_connection(self.__host, self.__port, self.__user,
            self.__passwd, None, self.socket_timeout)

        try:
            self.__arcl.purge(self.id)

        finally:
            self.__arcl.close_connection()

class RequestThread(threading.Thread):
    def __init__(self, req, addr, user, passwd, user_ip, req_sent, req_retry,
            max_req_lines, max_req_mb):
        threading.Thread.__init__(self)
        self.__req = req
        self.__addr = addr
        self.__user = user
        self.__passwd = passwd
        self.__user_ip = user_ip
        self.__req_sent = req_sent
        self.__req_retry = req_retry
        self.__max_req_lines = max_req_lines
        self.__max_req_mb = max_req_mb

    def __process(self, req):
        try:
            req.submit(self.__addr, self.__user, self.__passwd, self.__user_ip)
            if req.error is not None:
                logs.warning("error submitting request to %s: %s" %
                    (self.__addr, req.error))

                for rl in req.content:
                    rl.routes_tried.add(self.__addr)
                    self.__req_retry.content.append(rl)

                return

            req.wait()
            logs.debug("%s: request %s ready" % (req.address, req.id))

            fail_content = {}
            for rl in req.content:
                rl.routes_tried.add(self.__addr)
                fail_content[repr(rl)] = rl

            sr = req.status()
            for sv in sr.volume:
                for sl in sv.line:
                    if (sv.status != STATUS_OK and sv.status != STATUS_WARN and sv.status != STATUS_DENIED) or \
                        (sl.status != STATUS_OK and sl.status != STATUS_WARN and sl.status != STATUS_DENIED):
                        self.__req_retry.content.append(fail_content[sl.content.strip()])

            #req.purge()
            self.__req_sent.append(req)
            return

        except ArclinkError, e:
            try:
                sr = req.status()
                if sr.error:
                    logs.warning("%s: request %s failed (%s)" % (req.address, req.id, str(e)))

                else:
                    logs.warning("%s: request %s returned no data (%s)" % (req.address, req.id, str(e)))

                #req.purge()
                self.__req_sent.append(req)

            except (ArclinkError, socket.error), e:
                logs.warning("%s: error: %s" % (req.address, str(e)))

        except socket.error, e:
            logs.warning("%s: error: %s" % (req.address, str(e)))

        for rl in req.content:
            rl.routes_tried.add(self.__addr)
            self.__req_retry.content.append(rl)

    def run(self):
        req = self.__req.new()
        size = 0

        for rl in self.__req.content:
            req.content.append(rl)
            size += rl.estimated_size

            if len(req.content) >= self.__max_req_lines or size/(1024*1024) >= self.__max_req_mb:
                self.__process(req)
                req = self.__req.new()
                size = 0

        if req.content:
            self.__process(req)

class ArclinkManager(object):
    def __init__(self, address, default_user=_DEFAULT_USER, user_ip=None,
            pwtable={}, addr_alias={}, socket_timeout=_SOCKET_TIMEOUT,
            request_timeout=_REQUEST_TIMEOUT, download_retry=_DOWNLOAD_RETRY,
            max_req_lines=_MAX_REQ_LINES, max_req_mb=_MAX_REQ_MB):
        self.__myaddr = address
        self.__default_user = default_user
        self.__user_ip = user_ip
        self.__pwtable = pwtable
        self.__addr_alias = addr_alias
        self.__socket_timeout = socket_timeout
        self.__request_timeout = request_timeout
        self.__download_retry = download_retry
        self.__max_req_lines = max_req_lines
        self.__max_req_mb = max_req_mb

        usr_pwd = self.__pwtable.get(address)
        if usr_pwd is None:
            self.__myuser = self.__default_user
            self.__mypasswd = None
        else:
            (self.__myuser, self.__mypasswd) = usr_pwd

        alias = self.__addr_alias.get(address)
        if alias is None:
            self.__myaddr = address
        else:
            self.__myaddr = alias

    def checkServer(self):
        try:
            (host, port) = self.__myaddr.split(':')
            port = int(port)

        except ValueError:
            raise Exception("Hostname should be in a form of host:port")

        try:
            socket.gethostbyname(host)
        except socket.error:
            raise Exception("Cannot resolv supplied address")

        # Test that the ArcLink server is up and running
        try:
            acl = self.__arcl = Arclink()
            acl.open_connection(host, port, self.__myuser)
            acl.close_connection()
            acl = None
        except Exception, e:
            raise Exception("Arclink Server is down.")

    def new_request(self, rtype, args={}, label=None):
        return _Request(rtype, args, label, self.__socket_timeout,
            self.__request_timeout, self.__download_retry)

    def get_inventory(self, network='*', station=None, stream=None,
            loc_id=None, start_time=None, end_time=None, sensortype=None,
            permanent=None, restricted=None, latmin=None, latmax=None,
            lonmin=None, lonmax=None, instr=False, allnet=False,
            modified_after=None, qc_constraints=None):

        constraints = {}

        if qc_constraints is not None:
            constraints = qc_constraints

        if sensortype is not None:
            constraints['sensortype'] = sensortype

        if permanent is not None:
            if permanent:
                constraints['permanent'] = 'true'
            else:
                constraints['permanent'] = 'false'

        if restricted is not None:
            if restricted:
                constraints['restricted'] = 'true'
            else:
                constraints['restricted'] = 'false'

        if latmin is not None:
            constraints['latmin'] = str(latmin)

        if latmax is not None:
            constraints['latmax'] = str(latmax)

        if lonmin is not None:
            constraints['lonmin'] = str(lonmin)

        if lonmax is not None:
            constraints['lonmax'] = str(lonmax)

        if start_time is None:
            start_time = datetime.datetime(1980,1,1,0,0,0)

        if end_time is None:
            end_time = datetime.datetime(2030,12,31,0,0,0)

        args = { 'compression': 'bzip2' }
        if instr:
            args['instruments'] = 'true'
        else:
            args['instruments'] = 'false'

        if modified_after is not None:
            args['modified_after'] = modified_after.isoformat()

        req = self.new_request("INVENTORY", args)
        req.add(start_time, end_time, network, station, stream, loc_id, constraints)

        if allnet:
            req.add(start_time, end_time, '*', '', '', '', constraints)

        req.submit(self.__myaddr, self.__myuser, self.__mypasswd, self.__user_ip)

        if req.error is not None:
            raise ArclinkError, req.error

        inv = _Inventory()
        req.download_xml(inv, True)
        return inv

    def __estimate_size(self, strm, start_time, end_time):
        if strm.sampleRateDenominator == 0:
            return 0

        tdiff = end_time - start_time
        tdiff = tdiff.days * 86400 + tdiff.seconds
        spfr = float(strm.sampleRateNumerator) / float(strm.sampleRateDenominator)
        return max(int(tdiff * spfr * 1.5), 512)

    def __spfr_diff(self, strm, spfr):
        if strm.sampleRateDenominator == 0:
            return 10000

        return abs(spfr - float(strm.sampleRateNumerator) / float(strm.sampleRateDenominator))

    def __expand_request(self, req, inv, spfr):
        def _dot(s):
            if not s:
                return "."

            return s

        for i in req.content:
            if i[0] in inv.stationGroup:
                break

            for j in i[:4]:
                if j.find("*") >= 0 or j.find("?") >= 0:
                    break
            else:
                continue

            break

        else:
            return

        content = []

        for rl in req.content:
            expanded = {}
            spfr_diff = 10000

            for sgrp in inv.stationGroup.values():
                if not fnmatch.fnmatchcase(sgrp.code, rl.net):
                    continue

                for sref in sgrp.stationReference.values():
                    for net in sum([i.values() for i in inv.network.values()], []):
                        try:
                            sta = net.object[sref.stationID]
                            break

                        except KeyError:
                            pass

                    else:
                        continue

                    for loc in sum([i.values() for i in sta.sensorLocation.values()], []):
                        if not fnmatch.fnmatchcase(_dot(loc.code), _dot(rl.loc)):
                            continue

                        for strm in sum([i.values() for i in loc.stream.values()], []):
                            if fnmatch.fnmatchcase(strm.code, rl.cha):
                                if spfr is not None:
                                    if self.__spfr_diff(strm, spfr) < spfr_diff:
                                        spfr_diff = self.__spfr_diff(strm, spfr)
                                        expanded.clear()

                                    elif self.__spfr_diff(strm, spfr) > spfr_diff:
                                        continue

                                expanded[(rl.start_time, rl.end_time, net.code, sta.code, strm.code, _dot(loc.code))] = \
                                    self.__estimate_size(strm, rl.start_time, rl.end_time)

                        for strm in sum([i.values() for i in loc.auxStream.values()], []):
                            if fnmatch.fnmatchcase(strm.code, rl.cha):
                                if spfr is not None:
                                    if self.__spfr_diff(strm, spfr) < spfr_diff:
                                        spfr_diff = self.__spfr_diff(strm, spfr)
                                        expanded.clear()

                                    elif self.__spfr_diff(strm, spfr) > spfr_diff:
                                        continue

                                expanded[(rl.start_time, rl.end_time, net.code, sta.code, strm.code, _dot(loc.code))] = 0
                                    # auxStream doesn't have sample rate info, so this wouldn't work
                                    # self.__estimate_size(strm, rl.start_time, rl.end_time)

            for net in sum([i.values() for i in inv.network.values()], []):
                if not fnmatch.fnmatchcase(net.code, rl.net):
                    continue

                for sta in sum([i.values() for i in net.station.values()], []):
                    if not fnmatch.fnmatchcase(sta.code, rl.sta):
                        continue

                    for loc in sum([i.values() for i in sta.sensorLocation.values()], []):
                        if not fnmatch.fnmatchcase(_dot(loc.code), _dot(rl.loc)):
                            continue

                        for strm in sum([i.values() for i in loc.stream.values()], []):
                            if fnmatch.fnmatchcase(strm.code, rl.cha):
                                if spfr is not None:
                                    if self.__spfr_diff(strm, spfr) < spfr_diff:
                                        spfr_diff = self.__spfr_diff(strm, spfr)
                                        expanded.clear()

                                    elif self.__spfr_diff(strm, spfr) > spfr_diff:
                                        continue

                                expanded[(rl.start_time, rl.end_time, net.code, sta.code, strm.code, _dot(loc.code))] = \
                                    self.__estimate_size(strm, rl.start_time, rl.end_time)

                        for strm in sum([i.values() for i in loc.auxStream.values()], []):
                            if fnmatch.fnmatchcase(strm.code, rl.cha):
                                if spfr is not None:
                                    if self.__spfr_diff(strm, spfr) < spfr_diff:
                                        spfr_diff = self.__spfr_diff(strm, spfr)
                                        expanded.clear()

                                    elif self.__spfr_diff(strm, spfr) > spfr_diff:
                                        continue

                                expanded[(rl.start_time, rl.end_time, net.code, sta.code, strm.code, _dot(loc.code))] = 0
                                    # auxStream doesn't have sample rate info, so this wouldn't work
                                    # self.__estimate_size(strm, rl.start_time, rl.end_time)

            if expanded:
                for (x, estimated_size) in expanded.items():
                    rlx = RequestLine(*x)
                    rlx.routes_tried = rl.routes_tried.copy()
                    rlx.estimated_size = estimated_size
                    content.append(rlx)

            else:
                logs.warning("no match for " + repr(rl))

        req.content[:] = content

    def __execute(self, inv, request, req_sent, req_noroute, req_nodata, dry_run):
        def _cmptime(t1, t2):
            if t1 is None and t2 is None:
                return 0
            elif t2 is None or (t1 is not None and t1 < t2):
                return -1
            elif t1 is None or (t2 is not None and t1 > t2):
                return 1

            return 0

        req_retry = request.new()
        req_route = {}

        for rl in request.content:
            for x in (15, 14, 13, 11, 7, 12, 10, 9, 6, 5, 3, 8, 4, 2, 1, 0):
                net = (rl.net, "")[not (x & 8)]
                sta = (rl.sta, "")[not (x & 4)]
                cha = (rl.cha, "")[not (x & 2)]
                loc = (rl.loc, "")[not (x & 1)]

                try:
                    route = inv.route[net][sta][loc][cha]
                    break

                except KeyError:
                    continue

            else:
                logs.warning("route to station %s %s not found" % (rl.net, rl.sta))
                req_noroute.content.append(rl)
                continue

            server_list = sum([i.values() for i in route.arclink.values()], [])
            server_list.sort(key=lambda x: x.priority)
            arclink_addrs = []
            for server in server_list:
                if _cmptime(server.start, rl.end_time) > 0 or \
                    _cmptime(server.end, rl.start_time) < 0:
                    continue

                alias = self.__addr_alias.get(server.address)
                if alias is None:
                    arclink_addrs.append(server.address)
                else:
                    arclink_addrs.append(alias)

            for addr in arclink_addrs:
                if addr not in rl.routes_tried:
                    if addr not in req_route:
                        req_route[addr] = request.new()

                    req_route[addr].content.append(rl)
                    break
            else:
                if arclink_addrs:
                    req_nodata.content.append(rl)
                else:
                    req_noroute.content.append(rl)

        req_thr = []
        for (addr, req) in req_route.items():
            usr_pwd = self.__pwtable.get(addr)
            if usr_pwd is None:
                user = self.__default_user
                passwd = None
            else:
                (user, passwd) = usr_pwd

            if not dry_run:
                logs.info("launching request thread (%s)" % (addr,))

                thr = RequestThread(req, addr, user, passwd, self.__user_ip, req_sent, req_retry,
                    self.__max_req_lines, self.__max_req_mb)

                thr.start()
                req_thr.append(thr)

            else:
                req_sent.append(req)

        for thr in req_thr:
            thr.join()

        if req_retry.content:
            return self.__execute(inv, req_retry, req_sent, req_noroute, req_nodata, dry_run)

    def execute(self, request, use_inventory=True, use_routing=True, preferred_sample_rate=None, dry_run=False):
        if len(request.content) == 0:
            raise ArclinkError, "empty request"

        inv = _Inventory()
        rtn = _Routing()

        req_sent = []
        req_noroute = request.new()
        req_nodata = request.new()

        if use_inventory:
            logs.debug("requesting inventory from %s" % (self.__myaddr))

            startidx = 0
            while startidx < len(request.content):
                args = {'instruments': 'true', 'compression': 'bzip2'}
                req = self.new_request("INVENTORY", args, request.label)
                req.content = request.content[startidx:startidx+self.__max_req_lines]
                req.submit(self.__myaddr, self.__myuser, self.__mypasswd, self.__user_ip)

                if req.error is not None:
                    raise ArclinkError, "error getting inventory data from %s: %s" % \
                        (self.__myaddr, req.error)

                try:
                    req.download_xml(inv, True)

                except ArclinkError, e:
                    raise ArclinkError, "error getting inventory data from %s: %s" % \
                        (self.__myaddr, str(e))

                startidx += len(req.content)

            self.__expand_request(request, inv, preferred_sample_rate)
            if len(request.content) == 0:
                raise ArclinkError, "empty request after wildcard expansion"

        if use_routing:
            logs.debug("requesting routing from %s" % (self.__myaddr))

            startidx = 0
            while startidx < len(request.content):
                args = { 'compression': 'bzip2' }
                req = self.new_request("ROUTING", args, request.label)
                req.content = request.content[startidx:startidx+self.__max_req_lines]
                req.submit(self.__myaddr, self.__myuser, self.__mypasswd, self.__user_ip)

                if req.error is not None:
                    raise ArclinkError, "error getting routing data from %s: %s" % \
                        (self.__myaddr, req.error)

                try:
                    req.download_xml(rtn, True)

                except ArclinkError, e:
                    raise ArclinkError, "error getting routing data from %s: %s" % \
                        (self.__myaddr, str(e))

                startidx += len(req.content)

            self.__execute(rtn, request, req_sent, req_noroute, req_nodata, dry_run)

        else:
            logs.debug("requesting %s data from %s" % (request.rtype.lower(), self.__myaddr))

            startidx = 0
            while startidx < len(request.content):
                req = request.new()
                req.content = request.content[startidx:startidx+self.__max_req_lines]
                req.submit(self.__myaddr, self.__myuser, self.__mypasswd, self.__user_ip)

                if req.error is not None:
                    raise ArclinkError, "error getting %s data from %s: %s" % \
                        (req.rtype.lower(), self.__myaddr, req.error)

                try:
                    req.wait()

                except ArclinkError, e:
                    sr = req.status()
                    if sr.error:
                        logs.warning("%s: request %s failed (%s)" % (req.address, req.id, str(e)))

                    else:
                        logs.warning("%s: request %s returned no data (%s)" % (req.address, req.id, str(e)))

                req_sent.append(req)
                startidx += len(req.content)

        if not req_noroute.content:
            req_noroute = None

        if not req_nodata.content:
            req_nodata = None

        return (inv, req_sent, req_noroute, req_nodata)

    def __route_request(self, inv, req_sent, request, dry_run):
        def _cmptime(t1, t2):
            if t1 is None and t2 is None:
                return 0
            elif t2 is None or (t1 is not None and t1 < t2):
                return -1
            elif t1 is None or (t2 is not None and t1 > t2):
                return 1

            return 0

        req_fail = None
        req_route = {}

        for rl in request.content:
            for x in (15, 14, 13, 11, 7, 12, 10, 9, 6, 5, 3, 8, 4, 2, 1, 0):
                net = (rl.net, "")[not (x & 8)]
                sta = (rl.sta, "")[not (x & 4)]
                cha = (rl.cha, "")[not (x & 2)]
                loc = (rl.loc, "")[not (x & 1)]

                try:
                    route = inv.route[net][sta][loc][cha]
                    break

                except KeyError:
                    continue

            else:
                logs.warning("route to station %s %s not found" % (rl.net, rl.sta))
                if req_fail is None:
                    req_fail = request.new()

                req_fail.content.append(rl)
                continue

            server_list = sum([i.values() for i in route.arclink.values()], [])
            server_list.sort(key=lambda x: x.priority)
            arclink_addrs = []
            for server in server_list:
                if _cmptime(server.start, rl.end_time) > 0 or \
                    _cmptime(server.end, rl.start_time) < 0:
                    continue

                alias = self.__addr_alias.get(server.address)
                if alias is None:
                    arclink_addrs.append(server.address)
                else:
                    arclink_addrs.append(alias)

            for addr in arclink_addrs:
                if addr not in rl.routes_tried:
                    if addr not in req_route:
                        req = request.new()
                        req.size = 0
                        req_route[addr] = [ req ]

                    else:
                        req = req_route[addr][-1]

                        if len(req.content) >= self.__max_req_lines or req.size/(1024*1024) >= self.__max_req_mb:
                            req = request.new()
                            req.size = 0
                            req_route[addr].append(req)

                    req.content.append(rl)
                    req.size += rl.estimated_size
                    break

            else:
                if req_fail is None:
                    req_fail = request.new()

                req_fail.content.append(rl)

        for (addr, reqlist) in req_route.items():
            usr_pwd = self.__pwtable.get(addr)
            if usr_pwd is None:
                user = self.__default_user
                passwd = None
            else:
                (user, passwd) = usr_pwd

            error = False
            for req in reqlist:
                for rl in req.content:
                    rl.routes_tried.add(addr)

                if not error:
                    if not dry_run:
                        req.submit(addr, user, passwd, self.__user_ip)

                    if req.error is None:
                        req_sent.append(req)

                    else:
                        logs.warning("error submitting request to %s: %s" %
                            (addr, req.error))

                        error = True

                if error:
                    if req_fail is None:
                        req_fail = request.new()

                    for rl in req.content:
                        req_fail.content.append(rl)

        if req_fail is not None and len(req_route) > 0:
            return self.__route_request(inv, req_sent, req_fail, dry_run)

        return req_fail

    def route_request(self, request, use_inventory=True, preferred_sample_rate=None, dry_run=False):
        if len(request.content) == 0:
            raise ArclinkError, "empty request"

        inv = _Inventory()
        rtn = _Routing()

        if use_inventory:
            startidx = 0
            while startidx < len(request.content):
                args = {'instruments': 'false', 'compression': 'bzip2'}
                req = self.new_request("INVENTORY", args, request.label)
                req.content = request.content[startidx:startidx+self.__max_req_lines]
                req.submit(self.__myaddr, self.__myuser, self.__mypasswd, self.__user_ip)

                if req.error is not None:
                    raise ArclinkError, req.error

                req.download_xml(inv, True)
                startidx += len(req.content)

            self.__expand_request(request, inv, preferred_sample_rate)
            if len(request.content) == 0:
                raise ArclinkError, "empty request after wildcard expansion"

        startidx = 0
        while startidx < len(request.content):
            args = { 'compression': 'bzip2' }
            req = self.new_request("ROUTING", args, request.label)
            req.content = request.content[startidx:startidx+self.__max_req_lines]
            req.submit(self.__myaddr, self.__myuser, self.__mypasswd, self.__user_ip)

            if req.error is not None:
                raise ArclinkError, req.error

            req.download_xml(rtn, True)
            startidx += len(req.content)

        req_sent = []
        req_fail = self.__route_request(rtn, req_sent, request, dry_run)

        return (inv, req_sent, req_fail)

