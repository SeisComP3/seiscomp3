import  os, sys, tempfile
import  datetime, time, re
from    seiscomp import mseedlite as mseed

def _timeparse(t, format):
    """Parse a time string that might contain fractions of a second.

    Fractional seconds are supported using a fragile, miserable hack.
    Given a time string like '02:03:04.234234' and a format string of
    '%H:%M:%S', time.strptime() will raise a ValueError with this
    message: 'unconverted data remains: .234234'.  If %S is in the
    format string and the ValueError matches as above, a datetime
    object will be created from the part that matches and the
    microseconds in the time string.
    """
    try:
        return datetime.datetime(*time.strptime(t, format)[0:6]).time()
    except ValueError, msg:
        if "%S" in format:
            msg = str(msg)
            mat = re.match(r"unconverted data remains:"
                           " \.([0-9]{1,6})$", msg)
            if mat is not None:
                # fractional seconds are present - this is the style
                # used by datetime's isoformat() method
                frac = "." + mat.group(1)
                t = t[:-len(frac)]
                t = datetime.datetime(*time.strptime(t, format)[0:6])
                microsecond = int(float(frac)*1e6)
                return t.replace(microsecond=microsecond)
            else:
                mat = re.match(r"unconverted data remains:"
                               " \,([0-9]{3,3})$", msg)
                if mat is not None:
                    # fractional seconds are present - this is the style
                    # used by the logging module
                    frac = "." + mat.group(1)
                    t = t[:-len(frac)]
                    t = datetime.datetime(*time.strptime(t, format)[0:6])
                    microsecond = int(float(frac)*1e6)
                    return t.replace(microsecond=microsecond)

        raise

def timeparse(t):
    return _timeparse(t, "%Y/%m/%d %H:%M:%S")


class Input(mseed.Input):

    def __init__(self, server, streams,
                 stime=None, etime=None, timeout=None, verbose=0):

# XXX Add the possibility for supplying stime and etime as
#     individual times for each stream. 

        """
        'streams' must be a list containing tuples of (net,sta,loc,cha)
        """

        import subprocess

        streams = [ "%-3s %5s %s%3s.D" % s for s in streams ]
        streams.sort()

        self.tmp = tempfile.NamedTemporaryFile(mode="w", prefix="slinktool.")
        self.tmp.write("\n".join(streams)+"\n")
        self.tmp.flush()
        if verbose:
            sys.stderr.write("\n".join(streams)+"\n")

        slinktool = os.getenv("SLINKTOOL")
        if not slinktool:
            slinktool = "slinktool"
        args = [slinktool, "-l", self.tmp.name, "-o", "-"]
        if stime:
            args.append("-tw")
            tw = "%d,%d,%d,%d,%d,%d:" % (stime.year,stime.month,stime.day,stime.hour,stime.minute,stime.second)
            if etime:
                rw += "%d,%d,%d,%d,%d,%d" % (etime.year,etime.month,etime.day,etime.hour,etime.minute,etime.second)
            args.append(tw)
        if verbose: args.append("-v")
        
        if timeout:
            try:    assert int(timeout) > 0
            except: raise TypeError, "illegal timeout parameter"
            args += ["-nt", "%d" % int(timeout)]
        
        args.append(server)
        # start 'slinktool' as sub-process
        self.popen = subprocess.Popen(args, stdout=subprocess.PIPE, shell=False)
        infile = self.popen.stdout

        mseed.Input.__init__(self, infile)

    def __del__(self):
        """
        Shut down SeedLink connections and close input.
        """
        sys.stderr.write("shutting down slinktool\n")
        sys.stderr.flush()

        slinktool_pid = self.popen.pid
        # It would of course be much better to send SIGTERM,
        # but somehow slinktool often appears to ignore it.
        # XXX Need to figure out why, and perhaps fix it (not critical).
        self.popen.kill()
        self.popen.communicate()
#       mseed.Input.__del__(self) # closes the input file



class Input2(mseed.Input):

    def __init__(self, server, streams, stime=None, etime=None, verbose=0):

        """
        XXX information not uptodate!!! XXX
        
        'streams' must be a dict containing tuples of (stime, etime),
        with the key being the stream_id and stime and etime being
        the starting and end time of the time window, respectively.
        The times must be seis.Time objects. For instance

        stime = seis.Time(...)
        etime = seis.Time(...)
        streams["GE.KBS.00.BHZ.D"] = (stime, etime)

        It is more efficient to request the same time interval for
        all streams. Wildcards for the channels are allowed. If
        stime is None, only new data are retrieved as they come in.
        """

        streams = [ "%-3s %5s %s%3s.D" % tuple(s.split(".")[:4])
                                         for s in streams ]
        streams.sort()

        self.tmp = tempfile.NamedTemporaryFile(mode="w", prefix="slinktool.")
        self.tmp.write("\n".join(streams)+"\n")
        sys.stderr.write("\n".join(streams)+"\n")
        self.tmp.flush()

        cmd = "slinktool -l %s -o -" % self.tmp.name
        if stime:
            assert isinstance(stime, seis.Time)
            cmd += " -tw %d,%d,%d,%d,%d,%d:" % stime.asDate
            if etime:
                assert isinstance(etime, seis.Time)
                cmd += "%d,%d,%d,%d,%d,%d" % etime.asDate
        cmd = cmd + "%s '%s'" % (verbose*" -v", server)

        infile = os.popen(cmd)
        
        mseed.Input.__init__(self, infile)


def available(server="localhost:18000",
              time_window=None, stream_ids=None, verbose=0):
 
    """ 
    Connects to server and returns a dictionary of lists of available
    time windows as tuples of (start_time, end_time) for each available
    stream. The stream set can be limited by specifying a list of
    stream_ids in the format usual format, i.e. net.sta.loc.cha.type,
    e.g. "GE.KBS.00.BHZ.D".
    Note that often the returned lists contain only one time tuple,
    corresponding to one contiguous time window available.

    NEW:
    The search for available data can be limited to a time window by
    specifying the "time_window" parameter, which must be a tuple
    containing the starting and end time as seis.Time objects.
    """
 
    import re

    if time_window:
        stime, etime = time_window
        assert stime <= etime
    else:
        stime, etime = None, None

    cmd = "slinktool -Q %s %s " % (verbose*"-v ", server)
    infile  = os.popen(cmd)
    windows = {}

    # parse the output of "slinktool -Q"
    # It is assumed that the lines consist of the fields
    # net,sta,[loc,], cha, type, date1, time1, "-", date2, time2
    # Since the location code (loc) may or may not be present, we
    # determine the position of the dash "-" to determine where the
    # other fields are.
    regex = re.compile("^[A-Z][A-Z]\ [A-Z].*[12][0-9]{3}(/[0-9]{2}){2}.*$")
    for line in infile.xreadlines(): 
        if regex.match(line): # line containing a time window, a bit crude

            line = line.split()
            try:
                dash = line.index("-")
            except ValueError:
                continue

            if dash==7: # location code is present
                    loc = line[2]
            else:   loc = ""

            net, sta, cha, typ = line[0], line[1], line[dash-4], line[dash-3]
 
            stream_id = "%s.%s.%s.%s.%s" % (net, sta, loc, cha, typ)
 
            if stream_ids and stream_id not in stream_ids:
                continue
            
            t1  = seis.Time("%s %s" % (line[dash-2], line[dash-1]))
            t2  = seis.Time("%s %s" % (line[dash+1], line[dash+2])) 

            if stime and t2<stime or etime and t1>etime:
                continue # non-overlapping time windows

            if stime and t1<stime:
                t1 = stime
            if etime and t2>etime:
                t2 = etime

            if not stream_id in windows:
                windows[stream_id] = []

            windows[stream_id].append((t1,t2))

        elif verbose:
            # probably some diagnostic output
            print(line.strip())
 
    return windows
 
# windows = available("st32:18000", verbose=1)
 
# for stream_id in windows: 
#    print stream_id, windows[stream_id] 


def server_version(host, port=18000):

    import socket

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((host, port))
    except:
        return None
    s.send("HELLO\n")
    data = s.recv(1024)
    s.close() 
    if data[:8] != "SeedLink":
        return None

    return data[10:13]


def server_running(host, port=18000):

    if server_version(host, port):
        return True

    return False
