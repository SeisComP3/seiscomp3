#!/usr/bin/env python
# -*- coding: utf-8 -*-

###############################################################################
# (C) 2016-2017 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ#
#                                                                             #
# License: LGPLv3 (https://www.gnu.org/copyleft/lesser.html)                  #
###############################################################################

from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

import sys
import os
import optparse
import datetime
import random
import subprocess
import dateutil.parser

from seiscomp import mseedlite, logs

VERSION = "2017.272"


class Error(Exception):
    pass


class Timespan(object):
    def __init__(self, start, end):
        self.start = start
        self.current = start
        self.end = end


def exec_fetch(param, data, verbose, no_check):
    cmd = [sys.path[0] + "/fdsnws_fetch"]

    if verbose:
        cmd += ["-v"]

    if no_check:
        cmd += ["-Z"]

    if data is not None:
        cmd += ["-p", "/dev/stdin"]

    cmd += ["-o", "/dev/stdout"]
    cmd += map(str, param)

    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    if data is not None:
        proc.stdin.write(data)

    proc.stdin.close()
    return proc


def scan_sds(d, timespan, nets):
    def scan_cha(d):
        last_file = {}

        for f in os.listdir(d):
            try:
                (net, sta, loc, cha, ext, year, doy) = f.split('.')
                nets.add((net, int(year)))

            except ValueError:
                logs.error("invalid SDS file:" + p, True)
                continue

            if (net, sta, loc, cha) not in timespan:
                continue

            try:
                if doy > last_file[loc][0]:
                    last_file[loc] = (doy, f)

            except KeyError:
                last_file[loc] = (doy, f)

        for (loc, (doy, f)) in last_file.items():
            with open(d + '/' + f, 'rb') as fd:
                nslc = tuple(f.split('.')[:4])
                rec = mseedlite.Record(fd)
                fd.seek(-rec.size, 2)
                rec = mseedlite.Record(fd)
                ts = timespan[nslc]

                if ts.start < rec.end_time < ts.end:
                    ts.start = rec.end_time
                    ts.current = rec.end_time

                elif rec.end_time >= ts.end:
                    del timespan[nslc]

    def scan_sta(d):
        for cha in os.listdir(d):
            if not cha.endswith('.D'):
                continue

            scan_cha(d + '/' + cha)

    def scan_net(d):
        for sta in os.listdir(d):
            scan_sta(d + '/' + sta)

    def scan_year(d):
        for net in os.listdir(d):
            scan_net(d + '/' + net)

    for year in os.listdir(d):
        scan_year(d + "/" + year)


def get_citation(nets, param, verbose):
    postdata = ""
    for (net, year) in nets:
        postdata += "%s * * * %d-01-01T00:00:00Z %d-12-31T23:59:59Z\n" \
                    % (net, year, year)

    if not isinstance(postdata, bytes):
        postdata = postdata.encode('utf-8')

    try:
        proc = exec_fetch(param, postdata, verbose, True)

    except OSError as e:
        logs.error(str(e))
        logs.error("error running fdsnws_fetch")
        return 1

    net_desc = {}

    for line in proc.stdout:
        try:
            if isinstance(line, bytes):
                line = line.decode('utf-8')

            if not line or line.startswith('#'):
                continue

            (code, desc, start) = line.split('|')[:3]

            year = dateutil.parser.parse(start).year

        except (ValueError, UnicodeDecodeError) as e:
            logs.error("error parsing text format: %s" % str(e))
            continue

        if code[0] in "0123456789XYZ":
            net_desc["%s_%d" % (code, year)] = desc

        else:
            net_desc[code] = desc

    logs.notice("You received seismic waveform data from the following "
                "network(s):")

    for code in sorted(net_desc):
        logs.notice("%s %s" % (code, net_desc[code]))

    logs.notice("\nAcknowledgment is extremely important for network operators\n"
                "providing open data. When preparing publications, please\n"
                "cite the data appropriately. The FDSN service at\n\n"
                "    http://www.fdsn.org/networks/citation/?networks=%s\n\n"
                "provides a helpful guide based on available network\n"
                "Digital Object Identifiers.\n"
                % "+".join(sorted(net_desc)))


def main():
    param0 = ["-y", "station", "-q", "format=text", "-q", "level=network"]
    param1 = ["-y", "station", "-q", "format=text", "-q", "level=channel"]
    param2 = ["-y", "dataselect", "-z"]
    times = {"starttime": datetime.datetime(1900, 1, 1), "endtime": datetime.datetime(2100, 1, 1)}
    nets = set()

    def add_param0(option, opt_str, value, parser):
        param0.append(opt_str)
        param0.append(value)

    def add_param1(option, opt_str, value, parser):
        param1.append(opt_str)
        param1.append(value)

    def add_param2(option, opt_str, value, parser):
        param2.append(opt_str)
        param2.append(value)

    def add_param(option, opt_str, value, parser):
        add_param0(option, opt_str, value, parser)
        add_param1(option, opt_str, value, parser)
        add_param2(option, opt_str, value, parser)

    def add_time(option, opt_str, value, parser):
        add_param1(option, opt_str, value, parser)

        try:
            t = dateutil.parser.parse(value)

        except ValueError as e:
            raise optparse.OptionValueError("option '%s': invalid time value: '%s'" % (opt_str, value))

        if t.tzinfo is not None:
            t = t.astimezone(dateutil.tz.tzutc()).replace(tzinfo=None)

        times[option.dest] = t

    parser = optparse.OptionParser(
            usage="Usage: %prog [-h|--help] [OPTIONS] -o directory",
            version="%prog " + VERSION)

    parser.set_defaults(
            url="http://geofon.gfz-potsdam.de/eidaws/routing/1/",
            timeout=600,
            retries=10,
            retry_wait=60,
            threads=5,
            max_lines=1000,
            max_timespan=1440)

    parser.add_option("-v", "--verbose", action="store_true", default=False,
                      help="verbose mode")

    parser.add_option("-u", "--url", type="string", action="callback",
                      callback=add_param,
                      help="URL of routing service (default %default)")

    parser.add_option("-N", "--network", type="string", action="callback",
                      callback=add_param1,
                      help="network code or pattern")

    parser.add_option("-S", "--station", type="string", action="callback",
                      callback=add_param1,
                      help="station code or pattern")

    parser.add_option("-L", "--location", type="string", action="callback",
                      callback=add_param1,
                      help="location code or pattern")

    parser.add_option("-C", "--channel", type="string", action="callback",
                      callback=add_param1,
                      help="channel code or pattern")

    parser.add_option("-s", "--starttime", type="string", action="callback",
                      callback=add_time,
                      help="start time")

    parser.add_option("-e", "--endtime", type="string", action="callback",
                      callback=add_time,
                      help="end time")

    parser.add_option("-t", "--timeout", type="int", action="callback",
                      callback=add_param,
                      help="request timeout in seconds (default %default)")

    parser.add_option("-r", "--retries", type="int", action="callback",
                      callback=add_param,
                      help="number of retries (default %default)")

    parser.add_option("-w", "--retry-wait", type="int", action="callback",
                      callback=add_param,
                      help="seconds to wait before each retry (default %default)")

    parser.add_option("-n", "--threads", type="int", action="callback",
                      callback=add_param,
                      help="maximum number of download threads (default %default)")

    parser.add_option("-c", "--credentials-file", type="string", action="callback",
                      callback=add_param2,
                      help="URL,user,password file (CSV format) for queryauth")

    parser.add_option("-a", "--auth-file", type="string", action="callback",
                      callback=add_param2,
                      help="file that contains the auth token")

    parser.add_option("-o", "--output-dir", type="string",
                      help="SDS directory where downloaded data is written")

    parser.add_option("-l", "--max-lines", type="int",
                      help="max lines per request (default %default)")

    parser.add_option("-m", "--max-timespan", type="int",
                      help="max timespan per request in minutes (default %default)")

    parser.add_option("-z", "--no-citation", action="store_true", default=False,
                      help="suppress network citation info")

    parser.add_option("-Z", "--no-check", action="store_true", default=False,
                      help="suppress checking received routes and data")

    (options, args) = parser.parse_args()

    if args or not options.output_dir:
        parser.print_usage(sys.stderr)
        return 1

    def log_alert(s):
        if sys.stderr.isatty():
            s = "\033[31m" + s + "\033[m"

        sys.stderr.write(s + '\n')
        sys.stderr.flush()

    def log_notice(s):
        if sys.stderr.isatty():
            s = "\033[32m" + s + "\033[m"

        sys.stderr.write(s + '\n')
        sys.stderr.flush()

    def log_verbose(s):
        sys.stderr.write(s + '\n')
        sys.stderr.flush()

    def log_silent(s):
        pass

    logs.error = log_alert
    logs.warning = log_alert
    logs.notice = log_notice
    logs.info = (log_silent, log_verbose)[options.verbose]
    logs.debug = log_silent

    try:
        try:
            proc = exec_fetch(param1, None, options.verbose, options.no_check)

        except OSError as e:
            logs.error(str(e))
            logs.error("error running fdsnws_fetch")
            return 1

        timespan = {}

        for line in proc.stdout:
            if isinstance(line, bytes):
                line = line.decode('utf-8')

            if not line or line.startswith('#'):
                continue

            starttime = max(dateutil.parser.parse(line.split('|')[15]), times['starttime'])
            endtime = min(dateutil.parser.parse(line.split('|')[16]), times['endtime'])

            if starttime.tzinfo is not None:
                starttime = starttime.astimezone(dateutil.tz.tzutc()).replace(tzinfo=None)

            if endtime.tzinfo is not None:
                endtime = endtime.astimezone(dateutil.tz.tzutc()).replace(tzinfo=None)

            try:
                ts = timespan[tuple(line.split('|')[:4])]

                if ts.start > starttime:
                    ts.start = starttime
                    ts.current = starttime

                if ts.end < endtime:
                    ts.end = endtime

            except KeyError:
                timespan[tuple(line.split('|')[:4])] = Timespan(starttime, endtime)

        proc.stdout.close()
        proc.wait()

        if proc.returncode != 0:
            logs.error("error running fdsnws_fetch")
            return 1

        if os.path.exists(options.output_dir):
            scan_sds(options.output_dir, timespan, nets)

        while len(timespan) > 0:
            postdata = ""

            ts_used = random.sample(timespan.items(), min(len(timespan), options.max_lines))

            for ((net, sta, loc, cha), ts) in ts_used:
                te = min(ts.end, ts.start + datetime.timedelta(minutes=options.max_timespan))

                if loc == '':
                    loc = '--'

                postdata += "%s %s %s %s %sZ %sZ\n" \
                            % (net, sta, loc, cha, ts.start.isoformat(), te.isoformat())

            if not isinstance(postdata, bytes):
                postdata = postdata.encode('utf-8')

            try:
                proc = exec_fetch(param2, postdata, options.verbose, options.no_check)

            except OSError as e:
                logs.error(str(e))
                logs.error("error running fdsnws_fetch")
                return 1

            got_data = False

            try:
                for rec in mseedlite.Input(proc.stdout):
                    try:
                        ts = timespan[(rec.net, rec.sta, rec.loc, rec.cha)]

                    except KeyError:
                        logs.warning("unexpected data: %s.%s.%s.%s" % (rec.net, rec.sta, rec.loc, rec.cha))
                        continue

                    if rec.end_time <= ts.current:
                        continue

                    sds_dir = "%s/%d/%s/%s/%s.D" \
                              % (options.output_dir, rec.begin_time.year, rec.net, rec.sta, rec.cha)

                    sds_file = "%s.%s.%s.%s.D.%s" \
                              % (rec.net, rec.sta, rec.loc, rec.cha, rec.begin_time.strftime('%Y.%j'))

                    if not os.path.exists(sds_dir):
                        os.makedirs(sds_dir)

                    with open(sds_dir + '/' + sds_file, 'ab') as fd:
                        fd.write(rec.header + rec.data)

                    ts.current = rec.end_time
                    nets.add((rec.net, rec.begin_time.year))
                    got_data = True

            except mseedlite.MSeedError as e:
                logs.error(str(e))

            proc.stdout.close()
            proc.wait()

            if proc.returncode != 0:
                logs.error("error running fdsnws_fetch")
                return 1

            for ((net, sta, loc, cha), ts) in ts_used:
                if not got_data:
                    # no progress, skip to next segment
                    ts.start += datetime.timedelta(minutes=options.max_timespan)

                else:
                    # continue from current position
                    ts.start = ts.current

                if ts.start >= ts.end:
                    # timespan completed
                    del timespan[(net, sta, loc, cha)]

        if nets and not options.no_citation:
            logs.info("retrieving network citation info")
            get_citation(nets, param0, options.verbose)

    except (IOError, Error) as e:
        logs.error(str(e))
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())

