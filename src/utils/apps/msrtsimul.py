#!/usr/bin/env python

from __future__ import absolute_import, division, print_function

import sys
import os
import time
import datetime
import calendar
import stat

from getopt import getopt, GetoptError
from seiscomp import mseedlite as mseed


#------------------------------------------------------------------------------
def read_mseed_with_delays(delaydict, reciterable):
    """
        Create an iterator which takes into account configurable realistic delays.

        This function creates an iterator which returns one miniseed record at a time.
        Artificial delays can be introduced by using delaydict.

        This function can be used to make simulations in real time more realistic
        when e.g. some stations have a much higher delay than others due to
        narrow bandwidth communication channels etc.

        A delaydict has the following data structure:
        keys: XX.ABC (XX: network code, ABC: station code). The key "default" is
        a special value for the default delay.
        values: Delay to be introduced in seconds

        This function will rearrange the iterable object which has been used as
        input for rt_simul() so that it can again be used by rt_simul but taking
        artificial delays into account.
    """
    import heapq #pylint: disable=C0415

    heap = []
    min_delay = 0
    default_delay = 0
    if 'default' in delaydict:
        default_delay = delaydict['default']
    for rec in reciterable:
        rec_time = calendar.timegm(rec.end_time.timetuple())
        delay_time = rec_time
        stationname = "%s.%s" % (rec.net, rec.sta)
        if stationname in delaydict:
            delay_time = rec_time + delaydict[stationname]
        else:
            delay_time = rec_time + default_delay
        heapq.heappush(heap, (delay_time, rec))
        toprectime = heap[0][0]
        if toprectime - min_delay < rec_time:
            topelement = heapq.heappop(heap)
            yield topelement
    while heap:
        topelement = heapq.heappop(heap)
        yield topelement


#------------------------------------------------------------------------------
def rt_simul(f, speed=1., jump=0., delaydict=None):
    """
    Iterator to simulate "real-time" MSeed input

    At startup, the first MSeed record is read. The following records are
    read in pseudo-real-time relative to the time of the first record,
    resulting in data flowing at realistic speed. This is useful e.g. for
    demonstrating real-time processing using real data of past events.

    The data in the input file may be multiplexed, but *must* be sorted by
    time, e.g. using 'mssort'.
    """
    rtime = time.time()
    etime = None
    skipping = True
    record_iterable = mseed.Input(f)
    if delaydict:
        record_iterable = read_mseed_with_delays(delaydict, record_iterable)
    for rec in record_iterable:
        if delaydict:
            rec_time = rec[0]
            rec = rec[1]
        else:
            rec_time = calendar.timegm(rec.end_time.timetuple())
        if etime is None:
            etime = rec_time

        if skipping:
            if (rec_time - etime) / 60.0 < jump:
                continue

            etime = rec_time
            skipping = False

        tmax = etime + speed * (time.time() - rtime)
        ms = 1000000.0 * (rec.nsamp / rec.fsamp)
        last_sample_time = rec.begin_time + datetime.timedelta(microseconds=ms)
        last_sample_time = calendar.timegm(last_sample_time.timetuple())
        if last_sample_time > tmax:
            time.sleep((last_sample_time - tmax + 0.001) / speed)
        yield rec


#------------------------------------------------------------------------------
def usage():
    sys.stderr.write("""
msrtsimul - read sorted (and possibly multiplexed) MiniSEED files and
        write the individual records in pseudo-real-time. This is useful
        e.g. for testing and simulating data acquisition. Output
        is $SEISCOMP_ROOT/var/run/seedlink/mseedfifo unless -c is used.

Usage: msrtsimul [options] [file]

Options:
    -c, --stdout        write on standard output
    -d, --delays        add artificial delays
    -s, --speed         speed factor (float)
    -j, --jump          minutes to skip (float)
        --test          test mode
    -m  --mode          choose between 'realtime' and 'historic'
    -v, --verbose       verbose mode
    -h, --help          display this help message
""")


#------------------------------------------------------------------------------
def main():
    py2 = sys.version_info < (3,)

    ifile = sys.stdin if py2 else sys.stdin.buffer
    verbosity = 0
    speed = 1.
    jump = 0.
    test = False
    mode = 'realtime'

    try:
        opts, args = getopt(sys.argv[1:], "cd:s:j:hvm:",
                            ["stdout", "delays=", "speed=", "jump=", "test",
                             "verbose", "help", "mode="])
    except GetoptError:
        usage()
        return 1

    out_channel = None
    delays = None

    for flag, arg in opts:
        if flag in ("-c", "--stdout"):
            out_channel = sys.stdout if py2 else sys.stdout.buffer
        elif flag in ("-d", "--delays"):
            delays = arg
        elif flag in ("-s", "--speed"):
            speed = float(arg)
        elif flag in ("-j", "--jump"):
            jump = float(arg)
        elif flag in ("-m", "--mode"):
            mode = arg
        elif flag in ("-v", "--verbose"):
            verbosity += 1
        elif flag == "--test":
            test = True
        else:
            usage()
            if flag in ("-h", "--help"):
                return 0
            return 1

    if len(args) == 1:
        if args[0] != "-":
            try:
                ifile = open(args[0], "rb")
            except IOError as e:
                print("could not open input file {} for reading: {}" \
                      .format(args[0], e), file=sys.stderr)
    elif len(args) != 0:
        usage()
        return 1

    if out_channel is None:
        try:
            sc_root = os.environ["SEISCOMP_ROOT"]
        except KeyError:
            print("SEISCOMP_ROOT environment variable is not set", file=sys.stderr)
            sys.exit(1)

        mseed_fifo = os.path.join(sc_root, "var", "run", "seedlink", "mseedfifo")
        if not os.path.exists(mseed_fifo):
            print("""\
ERROR: {} does not exist.
In order to push the records to SeedLink, it needs to run and must be configured for real-time playback.
""".format(mseed_fifo), file=sys.stderr)
            sys.exit(1)

        if not stat.S_ISFIFO(os.stat(mseed_fifo).st_mode):
            print("""\
ERROR: {} is not a named pipe
Check if SeedLink is running and configured for real-time playback.
""".format(mseed_fifo), file=sys.stderr)
            sys.exit(1)

        try:
            out_channel = open(mseed_fifo, "wb")
        except Exception as e:
            print(str(e), file=sys.stderr)
            sys.exit(1)

    try:
        stime = time.time()
        delaydict = None
        if delays:
            delaydict = dict()
            try:
                f = open(delays, 'r')
                for line in f:
                    content = line.split(':')
                    if len(content) != 2:
                        raise Exception("Could not parse a line in file %s: %s\n" % (delays, line))
                    delaydict[content[0].strip()] = float(content[1].strip())
            except Exception as e:
                print("Error reading delay file {}: {}".format(delays, e),
                      file=sys.stderr)

        inp = rt_simul(ifile, speed=speed, jump=jump, delaydict=delaydict)

    #input = rt_simul(ifile, speed=speed, jump=jump)
        time_diff = None
        print("Starting msrtsimul at {}".format(datetime.datetime.utcnow()), file=sys.stderr)
        for rec in inp:
            if time_diff is None:
                ms = 1000000.0 * (rec.nsamp / rec.fsamp)
                time_diff = datetime.datetime.utcnow() - rec.begin_time - \
                    datetime.timedelta(microseconds=ms)
            if mode == 'realtime':
                rec.begin_time += time_diff

            if verbosity:
                print("%s_%s %7.2f %s %7.2f" % \
                      (rec.net, rec.sta, (time.time() - stime), str(rec.begin_time),
                       time.time() - calendar.timegm(rec.begin_time.timetuple())),
                      file=sys.stderr)

            if not test:
                rec.write(out_channel, 9)
                out_channel.flush()

    except KeyboardInterrupt:
        pass
    except Exception as e:
        print("Exception: {}".format(str(e)), file=sys.stderr)
        return 1

    return 0


#------------------------------------------------------------------------------
if __name__ == "__main__":
    sys.exit(main())
