#!/usr/bin/env python

############################################################################
#    Copyright (C) by GFZ Potsdam                                          #
#                                                                          #
#    You can redistribute and/or modify this program under the             #
#    terms of the SeisComP Public License.                                 #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    SeisComP Public License for more details.                             #
############################################################################

import sys
import optparse
import seiscomp3.Seismology
import seiscomp3.Core
import seiscomp3.IO


class MyOptionParser(optparse.OptionParser):
    def format_epilog(self, formatter):
        return self.epilog


def str2time(timestring):
    """
    Liberally accept many time string formats and convert them to a
    seiscomp3.Core.Time
    """

    timestring = timestring.strip()
    for c in ["-", "/", ":", "T", "Z"]:
        timestring = timestring.replace(c, " ")
    timestring = timestring.split()
    assert 3 <= len(timestring) <= 6
    timestring.extend((6-len(timestring))*["0"])
    timestring = " ".join(timestring)
    format = "%Y %m %d %H %M %S"
    if timestring.find(".") != -1:
        format += ".%f"

    t = seiscomp3.Core.Time()
    t.fromString(timestring, format)
    return t


def time2str(time):
    """
    Convert a seiscomp3.Core.Time to a string
    """
    return time.toString("%Y-%m-%d %H:%M:%S.%f000000")[:23]


def RecordInput(filename=None):
    """
    Simple Record iterator that reads from a file (to be specified by
    filename) or -- if no filename was specified -- reads from standard input
    """

    stream = seiscomp3.IO.RecordStream.Create("file")
    if not stream:
        raise IOError("failed to create a RecordStream")

    if not filename:
        filename = "-"

    if not stream.setSource(filename):
        raise IOError("failed to assign source file to RecordStream")

    inp = seiscomp3.IO.RecordInput(
        stream, seiscomp3.Core.Array.INT, seiscomp3.Core.Record.SAVE_RAW)

    while True:
        rec = next(inp)
        if not rec:
            return
        yield rec


tmin = str2time("1970-01-01 00:00:00")
tmax = str2time("2500-01-01 00:00:00")
ifile = "-"

description = "%prog - read unsorted (and possibly multiplexed) MiniSEED files and sort the individual records by time. This is useful e.g. for simulating data acquisition."

epilog = """
Example:

cat f1.mseed f2.mseed f3.mseed |
scmssort -v -t '2007-03-28 15:48~2007-03-28 16:18' > sorted.mseed
"""

p = MyOptionParser(
    usage="%prog [options] [files | < ] > ", description=description, epilog=epilog)
p.add_option("-t", "--time-window", action="store",
             help="specify time window (as one -properly quoted- string). Times are of course UTC and separated by a tilde ~")
p.add_option("-E", "--sort-by-end-time", action="store_true",
             help="sort according to record end time; default is start time")
p.add_option("-u", "--uniqueness", action="store_true",
             help="ensure uniqueness of output, i.e. skip duplicate records")
p.add_option("-v", "--verbose", action="store_true",
             help="run in verbose mode")

(opt, filenames) = p.parse_args()

if opt.time_window:
    tmin, tmax = list(map(str2time, opt.time_window.split("~")))

if opt.verbose:
    sys.stderr.write("Time window: %s~%s\n" % (time2str(tmin), time2str(tmax)))


def _time(rec):
    if opt.sort_by_end_time:
        return seiscomp3.Core.Time(rec.endTime())
    return seiscomp3.Core.Time(rec.startTime())


def _in_time_window(rec, tmin, tmax):
    return rec.endTime() >= tmin and rec.startTime() <= tmax

if not filenames:
    filenames = ["-"]

if filenames:
    first = None
    time_raw = []
    for filename in filenames:
        if opt.verbose:
            sys.stderr.write("reading file '%s'\n" % filename)
        recordInput = RecordInput(filename)
        for rec in recordInput:
            if not rec:
                continue
            if not _in_time_window(rec, tmin, tmax):
                continue

            raw = rec.raw().str()
            t = _time(rec)
            if first is None:
                first = t
            t = float(t-first)  # float needs less memory
            time_raw.append((t, raw))

    if opt.verbose:
        sys.stderr.write("sorting records\n")
    time_raw.sort()

    if opt.verbose:
        sys.stderr.write("writing output\n")
    previous = None

    out = sys.stdout
    try:
        # needed in Python 3, fails in Python 2
        out = out.buffer
    except AttributeError:
        # assuming this is Python 2, nothing to be done
        pass

    for item in time_raw:
        if opt.uniqueness and item == previous:
            continue
        t, raw = item
        out.write(raw)
        previous = item

    if opt.verbose:
        sys.stderr.write("finished\n")
