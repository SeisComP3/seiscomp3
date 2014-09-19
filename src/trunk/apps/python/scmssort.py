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

import sys, seiscomp3.Seismology, seiscomp3.Core, seiscomp3.IO
from   getopt import getopt, GetoptError

def str2time(timestring):
    """
    Liberally accept many time string formats and convert them to a
    seiscomp3.Core.Time
    """
    
    timestring = timestring.strip()
    for c in ["-","/",":", "T", "Z"]:
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


def RecordInput(filename=None,
                datatype=seiscomp3.Core.Array.INT):
    """
    Simple Record iterator that reads from a file (to be specified by
    filename) or -- if no filename was specified -- reads from standard input
    """

    stream = seiscomp3.IO.RecordStream.Create("file")
    if not stream:
        raise IOError, "failed 1"

    if not filename:
        filename = "-"

    if not stream.setSource(filename):
        raise IOError, "failed 2"

    input = seiscomp3.IO.RecordInput(
                    stream, datatype, seiscomp3.Core.Record.SAVE_RAW)

    while 1:
        rec = input.next()
        if not rec:
            raise StopIteration

        yield rec


tmin = str2time("1970-01-01 00:00:00")
tmax = str2time("2030-01-01 00:00:00")
ifile = "-"
verbose = False
endtime = False
unique = False

usage_info = """
mssort - read unsorted (and possibly multiplexed) MiniSEED files and
         sort the individual records by time. This is useful e.g. for
         simulating data acquisition.

Usage: mssort.py [options] [file[s]]

Options:
    -t t1~t2    specify time window (as one -properly quoted- string)
                times are of course UTC and separated by a tilde ~
    -E          sort according to record end time; default is start time
    -u          ensure uniqueness of output, i.e. skip duplicate records
    -v          verbose mode
    -h          display this help message

Example:

cat f1..mseed f2.mseed f3.mseed |
mssort -v -t '2007-03-28 15:48~2007-03-28 16:18' > sorted.mseed

"""
#   -F          specify one input file

def usage(exitcode=0):
    sys.stderr.write(usage_info)
    sys.exit(exitcode)

try:
    opts, files = getopt(sys.argv[1:], "EF:t:uhv")
except GetoptError:
    usage(exitcode=1)

ffile = None

for flag, arg in opts:
    if   flag == "-t":  tmin, tmax = map(str2time, arg.split("~"))
    elif flag == "-E":  endtime = True
    elif flag == "-u":  unique = True
    elif flag == "-F":  ffile = arg
    elif flag == "-h":  usage(exitcode=0)
    elif flag == "-v":  verbose = True
    else: usage(exitcode=1)

if verbose:
    sys.stderr.write("Time window: %s~%s\n" % (time2str(tmin), time2str(tmax)))

def _time(rec):
    if endtime:
        return seiscomp3.Core.Time(rec.endTime())
    return seiscomp3.Core.Time(rec.startTime())

if not ffile:

    if not files:
        files = [ "-" ]

    time_rec = []
    for fname in files:
        input = RecordInput(fname)
	
	for rec in input:
            if rec is None:
                continue
            if not (rec.endTime() >= tmin and rec.startTime() <= tmax):
                continue

            raw = rec.raw().str()
            t = _time(rec)
            time_rec.append( (t,raw) )
#            time_rec.extend( [ (_time(rec), rec) for rec in input \
#                            if  rec is not None and \
#                                rec.endTime()   >= tmin and \
#                                rec.startTime() <= tmax ] )
    time_rec.sort()

    previous = None
    for item in time_rec:
        if item == previous:
            continue
        t,raw = item        
        sys.stdout.write(raw)
        previous = item

#input = None
#sys.stderr.write("Remaining Objects: %d\n" % seiscomp3.Core.BaseObject.ObjectCount())
#sys.exit(0)





### ffile is not None
##
##time_off, off = [], 0
##for rec in seiscomp.mseed.Input(ffile):
##    if rec==None:
##        sys.stderr.write("A record was None\n")
##        continue
##    time_off.append( (rec.time, off) )
##    off += len(rec.raw)
##
##time_off.sort()
##
##import seiscomp._mseed
##
##f = file(ffile)
##for t, offset in time_off:
##    f.seek(offset)
##    rec = seiscomp._mseed.Record(f)
##    if not rec:
##        continue
##    if rec.endtime >= tmin and rec.time <= tmax:
##        if verbose:
##            sys.stderr.write( "%s offset=%-10d %s\n" \
##                % (rec.time, offset, rec.stream_id) )
##        sys.stdout.write(rec.raw)
##
##
##for rec in RecordInput(ifile):
##
##    print rec.data()
##    print rec.startTime()
##    
