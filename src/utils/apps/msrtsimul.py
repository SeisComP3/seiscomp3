#!/usr/bin/env python

import sys, os, time, datetime, calendar, stat
from   getopt import getopt, GetoptError
from seiscomp import mseedlite as mseed

ifile       = sys.stdin
verbosity   = 0
speed       = 1.
jump        = 0
test        = False
mode = 'realtime'

def read_mseed_with_delays(delaydict,reciterable):
    """
        Create an iterator which takes into account configurable realistic delays.
    
        This function creates an iterator which returns one miniseed record at a time. Artificial delays can be introduced by using delaydict.
        
        This function can be used to make simulations in real time more realistic when e.g. some stations have a much higher delay than others
        due to narrow bandwidth communication channels etc. 
        
        A delaydict has the following data structure:
        keys: XX.ABC (XX: network code, ABC: station code). The key "default" is a special value for the default delay.
        values: Delay to be introduced in seconds
        
        This function will rearrange the iterable object which has been used as input for rt_simul() so that it can again be used by rt_simul
        but taking artificial delays into account.
        
    """
    import time
    import heapq
    import itertools
        
    heap = []
    min_delay = 0
    default_delay = 0
    if 'default' in delaydict:
        default_delay = delaydict['default']
    reciterator = itertools.chain(reciterable)
    rec = reciterator.next()
    while rec:
        rec_time = calendar.timegm(rec.begin_time.timetuple())
        delay_time = rec_time
        stationname = "%s.%s" % (rec.net,rec.sta)
        if stationname in delaydict:
            delay_time = rec_time + delaydict[stationname]
        else:
            delay_time = rec_time + default_delay
        heapq.heappush(heap,(delay_time,rec))
        toprectime = heap[0][0]
        if toprectime - min_delay < rec_time:
            topelement = heapq.heappop(heap)
            yield topelement
        rec = reciterator.next()
    while h:
        topelement = heapq.heappop(heap)
        yield topelement
   

def rt_simul(f, speed=1., jump=0, delaydict = None):
    """
    Iterator to simulate "real-time" MSeed input

    At startup, the first MSeed record is read. The following records are
    read in pseudo-real-time relative to the time of the first record,
    resulting in data flowing at realistic speed. This is useful e.g. for
    demonstrating real-time processing using real data of past events.

    The data in the input file may be multiplexed, but *must* be sorted by
    time, e.g. using 'mssort'.
    """
    import time

    rtime = time.time()
    stime = None
    skipping = True
    record_iterable = mseed.Input(f)
    if delaydict:
        record_iterable = read_mseed_with_delays(delaydict,record_iterable)
    for rec in record_iterable:
        rec_time = None
        if delaydict:
            rec_time = rec[0]
            rec = rec[1]
        else:
            rec_time = calendar.timegm(rec.begin_time.timetuple())
        if stime is None:
            stime = rec_time

        if skipping:
            if (rec_time - stime) / 60 < jump:
                continue

            stime = rec_time
            skipping = False

        tmax = stime + speed * (time.time() - rtime)
        if rec_time > tmax:
            time.sleep((rec_time - tmax + 0.001) / speed)
        yield rec


usage_info = """
msrtsimul - read sorted (and possibly multiplexed) MiniSEED files and
        write the individual records in pseudo-real-time. This is useful
        e.g. for testing and simulating data acquisition. Output
        is $SEISCOMP_ROOT/var/run/seedlink/mseedfifo unless -c is used.

Usage: msrtsimul.py [options] [file]

Options:
    -c, --stdout        write on standard output
    -d, --delays        add artificial delays
    -s, --speed         speed factor
    -j, --jump          number of minutes to skip
        --test          test mode
    -m  --mode          choose between 'realtime' and 'historic'   
    -v, --verbose       verbose mode
    -h, --help          display this help message
"""

def usage(exitcode=0):
    sys.stderr.write(usage_info)
    sys.exit(exitcode)

try:
    opts, args = getopt(sys.argv[1:], "cd:s:j:hvm:",
                        [ "stdout","delays=", "speed=", "jump=", "test", "verbose", "help","mode=" ])
except GetoptError:
    usage(exitcode=1)

out_channel = None
delays = None

for flag, arg in opts:
    if   flag in ("-c", "--stdout"):    out_channel = sys.stdout
    elif flag in ("-d","--delays"):     delays = arg
    elif flag in ("-s", "--speed"):     speed = float(arg)
    elif flag in ("-j", "--jump"):      jump = int(arg)
    elif flag in ("-h", "--help"):      usage(exitcode=0)
    elif flag in ("-m", "--mode"):      mode = arg
    elif flag in ("-v", "--verbose"):   verbosity += 1
    elif flag in ("--test"):            test = True
    else: usage(exitcode=1)

if len(args) == 0:
    pass
elif len(args) == 1:
    fname = args[0]
    if fname != "-":
        ifile = file(fname)
else: usage(exitcode=1)

if out_channel is None:
    try: sc_root = os.environ["SEISCOMP_ROOT"]
    except:
        sys.stderr.write("SEISCOMP_ROOT environment variable is not set\n")
        sys.exit(1)

    mseed_fifo = os.path.join(sc_root, "var", "run", "seedlink", "mseedfifo")
    if not os.path.exists(mseed_fifo):
        sys.stderr.write("""\
ERROR: %s does not exist.
In order to push the records to SeedLink, it needs to run and must be configured for real-time playback.
""" % mseed_fifo)
        sys.exit(1)

    if not stat.S_ISFIFO(os.stat(mseed_fifo).st_mode):
        sys.stderr.write("""\
ERROR: %s is not a named pipe
Check if SeedLink is running and configured for real-time playback.
""" % mseed_fifo)
        sys.exit(1)

    try: out_channel = open(mseed_fifo, "w")
    except Exception, e:
        sys.stderr.write("%s\n" % str(e))
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
        except: pass

    input = rt_simul(ifile, speed=speed, jump=jump, delaydict=delaydict)
    
    #input = rt_simul(ifile, speed=speed, jump=jump)
    time_diff = None
    
    for rec in input:
        if time_diff is None:
            time_diff = datetime.datetime.utcnow() - rec.begin_time - \
                datetime.timedelta(microseconds = 1000000.0 * (rec.nsamp / rec.fsamp))
        if mode == 'realtime':
            rec.begin_time += time_diff

        if verbosity:
            sys.stderr.write("%s_%s %7.2f %s %7.2f\n" % (rec.net, rec.sta, (time.time() - stime), str(rec.begin_time), 
                                                         time.time() - calendar.timegm(rec.begin_time.timetuple())))
            #sys.stderr.write("%s_%s %7.2f %s\n" % (rec.net, rec.sta, (time.time()-stime), str(rec.begin_time)))

        if not test:
            rec.write(out_channel, 9)
            out_channel.flush()

except KeyboardInterrupt:
    pass
except Exception, e:
    sys.stderr.write("Exception:  %s\n" % str(e))
    sys.exit(1)
