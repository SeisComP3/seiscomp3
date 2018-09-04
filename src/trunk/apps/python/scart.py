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

import glob, re, time, sys, os
import seiscomp3.IO, seiscomp3.Logging, seiscomp3.Client, seiscomp3.System
from   getopt import getopt, GetoptError
import bisect


class Archive:
  def __init__(self, archiveDirectory):
    self.archiveDirectory = archiveDirectory
    self.filePool = dict()
    self.filePoolSize = 100


  def iterators(self, begin, end, net, sta, loc, cha):
    t = time.gmtime(begin.seconds())
    t_end = time.gmtime(end.seconds())

    start_year = t[0]

    for year in range(start_year,t_end[0]+1):
      if year > start_year:
        begin = seiscomp3.Core.Time.FromYearDay(year,1)
        t = time.gmtime(begin.seconds())

      if net == "*":
        netdir = self.archiveDirectory + str(year) + "/"
        try: files = os.listdir(netdir)
        except: continue

        its = []
        for file in files:
          if os.path.isdir(netdir + file) == False: continue
          tmp_its = self.iterators(begin, end, file, sta, loc, cha)
          for it in tmp_its:
            its.append(it)

        return its

      if sta == "*":
        stadir = self.archiveDirectory + str(year) + "/" + net + "/"
        files = os.listdir(stadir)
        its = []
        for file in files:
          if os.path.isdir(stadir + file) == False: continue
          tmp_its = self.iterators(begin, end, net, file, loc, cha)
          for it in tmp_its:
            its.append(it)

        return its

      # Check if cha contains a regular expression or not
      mr = re.match("[A-Z|a-z|0-9]*", cha)
      if (mr and mr.group() != cha) or cha == "*":
        cha = cha.replace('?', '.')
        stadir = self.archiveDirectory + str(year) + "/" + net + "/" + sta + "/"
        try: files = os.listdir(stadir)
        except: return []
        its = []
        for file in files:
          if os.path.isdir(stadir + file) == False: continue
          part = file[:3]
          if cha != "*":
            mr = re.match(cha, part)
            if not mr or mr.group() != part:
              continue

          tmp_its = self.iterators(begin, end, net, sta, loc, part)
          for it in tmp_its:
            its.append(it)

        return its

      if loc == "*":
        dir = self.archiveDirectory + str(year) + "/" + net + "/" + sta + "/" + cha + ".D/"
        its = []

        start_day = t[7]
        if t_end[0] > year:
          end_day = 366
        else:
          end_day = t_end[7]

        files = files = glob.glob(dir + "*.%03d" % start_day)

        # Find first day with data
        while len(files) == 0 and start_day <= end_day:
          start_day += 1
          begin = seiscomp3.Core.Time.FromYearDay(year, start_day)
          files = glob.glob(dir + "*.%03d" % start_day)

        for file in files:
          file = file.split('/')[-1]
          if os.path.isfile(dir + file) == False: continue

          tmp_its = self.iterators(begin, end, net, sta, file.split('.')[2], cha)
          for it in tmp_its:
            its.append(it)

        return its

      it = StreamIterator(self, begin, end, net, sta, loc, cha)
      if not it.record is None:
        return [it]

    return []


  def location(self, rt, net, sta, loc, cha):
    t = time.gmtime(rt.seconds())
    dir = str(t[0]) + "/" + net + "/" + sta + "/" + cha + ".D/"
    file = net + "." + sta + "." + loc + "." + cha + ".D." + str(t[0]) + ".%03d" % t[7]
    return dir, file


  def findIndex(self, begin, end, file):
    rs = seiscomp3.IO.FileRecordStream()
    rs.setRecordType("mseed")
    if rs.setSource(self.archiveDirectory + file) == False:
      return None, None

    ri = seiscomp3.IO.RecordInput(rs)

    index = None
    retRec = None

    for rec in ri:
      if rec is None: break

      if rec.samplingFrequency() <= 0:
        continue

      if rec.startTime() >= end: break
      if rec.endTime() < begin: continue

      index = rs.tell()
      retRec = rec
      break

    rs.close()

    return retRec, index


  def readRecord(self, file, index):
    try:
      rs = self.filePool[file]
    except:
      rs = seiscomp3.IO.FileRecordStream()
      rs.setRecordType("mseed")
      if rs.setSource(self.archiveDirectory + file) == False:
        return (None, None)

      rs.seek(index)

      # Remove old handles
      if len(self.filePool) < self.filePoolSize:
        #self.filePool.pop(self.fileList[-1])
        #print "Remove %s from filepool" % self.fileList[-1]
        #del self.fileList[-1]
        self.filePool[file] = rs

    ri = seiscomp3.IO.RecordInput(rs, seiscomp3.Core.Array.INT, seiscomp3.Core.Record.SAVE_RAW)
    # Read only valid records
    while True:
      rec = ri.next()
      if rec is None: break
      if rec.samplingFrequency() <= 0: continue
      break

    index = rs.tell()

    if rec is None:
      # Remove file handle from pool
      rs.close()
      try: self.filePool.pop(file)
      except: pass

    return rec, index


  def stepTime(self, rt):
    rt = rt + seiscomp3.Core.TimeSpan(86400)
    t = rt.get()
    rt.set(t[1], t[2], t[3], 0, 0, 0, 0)
    return rt



class StreamIterator:
  def __init__(self, ar, begin, end, net, sta, loc, cha):
    self.archive = ar

    self.begin = begin
    self.end = end

    self.net = net
    self.sta = sta
    self.loc = loc
    self.cha = cha

    self.compareEndTime = False

    workdir, file = ar.location(begin, net, sta, loc, cha)
    self.file = workdir + file
    #print "Starting at file %s" % self.file

    self.record, self.index = ar.findIndex(begin, end, self.file)
    if self.record:
      self.current = self.record.startTime()
      self.currentEnd = self.record.endTime()


  def next(self):
    while True:
      self.record, self.index = self.archive.readRecord(self.file, self.index)
      if self.record:
        self.current = self.record.startTime()
        self.currentEnd = self.record.endTime()
        if self.current >= self.end:
          self.record = None
        return self.record
      else:
        # Skip the current day file
        self.current = self.archive.stepTime(self.current)
        # Are we out of scope?
        if self.current >= self.end:
          self.record = None
          return self.record

        # Use the new file and start from the beginning
        workdir, file = self.archive.location(self.current, self.net, self.sta, self.loc, self.cha)
        self.file = workdir + file
        self.index = 0
        #print "Continue at " + self.file

  def __cmp__(self, other):
    if self.compareEndTime:
      if self.currentEnd > other.currentEnd:
        return 1
      elif self.currentEnd < other.currentEnd:
        return -1
      return 0
    else:
      if self.current > other.current:
        return 1
      elif self.current < other.current:
        return -1
      return 0


class ArchiveIterator:
  def __init__(self, ar, sortByEndTime):
    self.archive = ar
    self.streams = []
    self.sortByEndTime = sortByEndTime

  def append(self, beginTime, endTime, net, sta, loc, cha):
    its = self.archive.iterators(beginTime, endTime, net, sta, loc, cha)
    for it in its:
      it.compareEndTime = self.sortByEndTime
      bisect.insort(self.streams, it)

  def appendStation(self, beginTime, endTime, net, sta):
    self.append(beginTime, endTime, net, sta, "*", "*")

  def nextSort(self):
    if len(self.streams) == 0:
      return None

    stream = self.streams.pop(0)

    rec = stream.record

    stream.next()

    if stream.record is not None:
      # Put the stream back on the right (sorted) position
      bisect.insort(self.streams, stream)

    return rec


class Copy:
  def __init__(self, it):
    self.iterator = it

  def __iter__(self):
    for stream in self.iterator.streams:
      rec = stream.record
      while rec:
        yield rec
        rec = stream.next()

    raise StopIteration


class Sorter:
  def __init__(self, it):
    self.iterator = it

  def __iter__(self):
    while 1:
      rec = self.iterator.nextSort()
      if not rec:
        raise StopIteration

      yield rec



####################################################################
##
## Application block
##
####################################################################


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
    return time.toString("%Y-%m-%d %H:%M:%S.%2f")


def create_dir(dir):
  if os.access(dir, os.W_OK):
    return True

  try:
    os.makedirs(dir)
    return True
  except:
    return False


def isFile(url):
  toks = url.split('://')
  return len(toks) < 2 or toks[0] == "file"


def readStreamList(file):
  streams = []

  try:
    if file == "-":
      f = sys.stdin
      file = "stdin"
    else:
      f = open(listFile, 'r')
  except:
    sys.stderr.write("%s: error: unable to open\n" % listFile)
    return []

  lineNumber = -1
  for line in f:
    lineNumber = lineNumber + 1
    line = line.strip()
    # ignore comments
    if len(line) > 0 and line[0] == '#':
      continue

    if len(line) == 0: continue

    toks = line.split(';')
    if len(toks) != 3:
      f.close()
      sys.stderr.write("%s:%d: error: invalid line format, expected 3 items separated by ';'\n" % (listFile, lineNumber))
      return []

    try: tmin = str2time(toks[0].strip())
    except:
      f.close()
      sys.stderr.write("%s:%d: error: invalid time format (tmin)\n" % (listFile, lineNumber))
      return []

    try: tmax = str2time(toks[1].strip())
    except:
      f.close()
      sys.stderr.write("%s:%d: error: invalid time format (tmax)\n" % (listFile, lineNumber))
      return []

    streamID = toks[2].strip()
    toks = streamID.split('.')
    if len(toks) != 4:
      f.close()
      sys.stderr.write("%s:%d: error: invalid stream format\n" % (listFile, lineNumber))
      return []

    streams.append((tmin, tmax, toks[0], toks[1], toks[2], toks[3]))

  f.close()

  return streams


usage_info = """
scart - dump records of an SDS structure, sort them, modify the
        time and replay them

Usage: scart [options] [archive]

Options:
    --stdout         writes on stdout if import mode is used instead
                     of creating a SDS archive
    -I               specify recordstream URL when in import mode
                     when using another recordstream than file a
                     stream list file is needed
                     default: file://- (stdin)
    -t t1~t2         specify time window (as one -properly quoted- string)
                     times are of course UTC and separated by a tilde ~
    -d, --dump       export (dump) mode
    -l, --list       uses a stream list file instead of defined networks and
                     channels (-n and -c are ignored)
                     line format: starttime;endtime;streamID
                                  2007-03-28 15:48;2007-03-28 16:18;GE.LAST.*.*
                                  2007-03-28 15:48;2007-03-28 16:18;GE.PMBI..BH?
    -s, --sort       sort records
    -m, --modify     modify the record time for realtime playback when dumping
        --speed      specify the speed to dump the records
                     a value of 0 means no delay otherwise speed is a multiplier
                     of the real time difference between the records
    -n               network list (comma separated), default: *
    -c               channel filter (regular expression),
                     default: (B|S|M|H|E)(L|H|N|G)(Z|N|E)
    -E               sort according to record end time; default is start time
        --files      specify the file handles to cache; default: 100
    -v, --verbose    verbose mode
        --test       test mode, no record output
    -h, --help       display this help message

Example:

  scart -dsv -t '2007-03-28 15:48~2007-03-28 16:18' /archive > sorted.mseed

"""

def usage(exitcode=0):
  sys.stderr.write(usage_info)
  sys.exit(exitcode)

try:
  opts, files = getopt(sys.argv[1:], "I:dsmEn:c:t:l:hv",
                       ["stdout", "dump", "list=", "sort", "modify", "speed=", "files=", "verbose", "test", "help"])
except GetoptError:
  usage(exitcode=1)


endtime      = False
verbose      = False
sort         = False
modifyTime   = False
dump         = False
listFile     = None
test         = False
filePoolSize = 100
# default = stdin
recordURL    = "file://-"

speed        = 0
stdout       = False

channels     = "(B|S|M|H|E)(L|H|N|G)(Z|N|E)"
networks     = "*"

archiveDirectory = "./"

for flag, arg in opts:
    if   flag == "-t":  tmin, tmax = map(str2time, arg.split("~"))
    elif flag == "-E":  endtime = True
    elif flag in ["-h", "--help"]:    usage(exitcode=0)
    elif flag in ["--stdout"]:        stdout = True
    elif flag in ["-v", "--verbose"]: verbose = True
    elif flag in ["-d", "--dump"]:    dump = True
    elif flag in ["-l", "--list"]:    listFile = arg
    elif flag in ["-s", "--sort"]:    sort = True
    elif flag in ["-m", "--modify"]:  modifyTime = True
    elif flag in ["--speed"]:         speed = float(arg)
    elif flag in ["--files"]:         filePoolSize = int(arg)
    elif flag in ["--test"]:          test = True
    elif flag == "-I":                recordURL = arg
    elif flag == "-n":                networks = arg
    elif flag == "-c":                channels = arg
    else: usage(exitcode=1)


if files:
  archiveDirectory = files[0]
else:
  try:
    archiveDirectory = os.environ["SEISCOMP_ROOT"] + "/var/lib/archive"
  except: pass

try:
  if archiveDirectory[-1] != '/':
    archiveDirectory = archiveDirectory + '/'
except: pass


archive = Archive(archiveDirectory)
archive.filePoolSize = filePoolSize

if verbose:
  seiscomp3.Logging.enableConsoleLogging(seiscomp3.Logging.getAll())
  if dump and not listFile:
    sys.stderr.write("Time window: %s~%s\n" % (time2str(tmin), time2str(tmax)))
  sys.stderr.write("Archive: %s\n" % archiveDirectory)
  if dump:
    if not sort and not modifyTime:
      sys.stderr.write("Mode: DUMP\n")
    elif sort and not modifyTime:
      sys.stderr.write("Mode: DUMP & SORT\n")
    elif not sort and modifyTime:
      sys.stderr.write("Mode: DUMP & MODIFY_TIME\n")
    elif sort and modifyTime:
      sys.stderr.write("Mode: DUMP & SORT & MODIFY_TIME\n")
  else:
    sys.stderr.write("Mode: IMPORT\n")

it = ArchiveIterator(archive, endtime)

if dump:
  if listFile:
    streams = readStreamList(listFile)
    for stream in streams:
      if verbose:
        sys.stderr.write("adding stream: %s.%s.%s.%s\n" % (stream[2], stream[3], stream[4], stream[5]))
      it.append(stream[0], stream[1], stream[2], stream[3], stream[4], stream[5])
  else:
    if networks == "*":
      it.append(tmin, tmax, "*", "*", "*", channels)
    else:
      items = networks.split(",")
      for n in items:
        n = n.strip()
        it.append(tmin, tmax, n, "*", "*", channels)

  stime = None
  realTime = seiscomp3.Core.Time.GMT()

  if sort:
    records = Sorter(it)
  else:
    records = Copy(it)

  for rec in records:
    # skip corrupt records
    etime = seiscomp3.Core.Time(rec.endTime())

    if stime is None:
      stime = etime
      if verbose: sys.stderr.write("First record: %s\n" % stime.iso())

    dt = etime - stime

    now = seiscomp3.Core.Time.GMT()

    if speed > 0:
      playTime = (realTime + dt).toDouble() / speed;
    else:
      playTime = now.toDouble()

    sleepTime = playTime - now.toDouble()
    if sleepTime > 0:
      time.sleep(sleepTime)

    if modifyTime:
      recLength = etime - rec.startTime()
      rec.setStartTime(seiscomp3.Core.Time(playTime) - recLength)

    if verbose:
      etime = rec.endTime()
      sys.stderr.write("%s %s %s %s\n" % (rec.streamID(), seiscomp3.Core.Time.LocalTime().iso(), rec.startTime().iso(), etime.iso()))

    if test == False:
      sys.stdout.write(rec.raw().str())

else:
  env = seiscomp3.System.Environment.Instance()
  cfg = seiscomp3.Config.Config()
  env.initConfig(cfg, "scart")
  try:
    plugins = cfg.getStrings("plugins")
    registry = seiscomp3.Client.PluginRegistry.Instance()
    for p in plugins:
      registry.addPluginName(p)
    registry.loadPlugins()
  except Exception,e: pass

  rs = seiscomp3.IO.RecordStream.Open(recordURL)
  if rs is None:
    sys.stderr.write("Unable to open recordstream '%s'\n" % recordURL)
    sys.exit(-1)

  if rs.setRecordType("mseed") == False:
    sys.stderr.write("Format 'mseed' is not supported by recordstream '%s'\n" % recordURL)
    sys.exit(-1)

  if not isFile(recordURL):
    if not listFile:
      sys.stderr.write("A stream list is needed to fetch data from another source than a file\n")
      sys.exit(-1)

    streams = readStreamList(listFile)
    for stream in streams:
      # Add stream to recordstream
      if rs.addStream(stream[2], stream[3], stream[4], stream[5], stream[0], stream[1]) == False:
        if verbose:
          sys.stderr.write("error: adding stream: %s %s %s.%s.%s.%s\n" % (stream[0], stream[1], stream[2], stream[3], stream[4], stream[5]))
      else:
        if verbose:
          sys.stderr.write("adding stream: %s %s %s.%s.%s.%s\n" % (stream[0], stream[1], stream[2], stream[3], stream[4], stream[5]))

  input = seiscomp3.IO.RecordInput(rs, seiscomp3.Core.Array.INT, seiscomp3.Core.Record.SAVE_RAW)
  filePool = dict()
  f = None
  try:
    for rec in input:
      if stdout:
        sys.stdout.write(rec.raw().str())
        continue

      dir, file = archive.location(rec.startTime(), rec.networkCode(), rec.stationCode(), rec.locationCode(), rec.channelCode())
      file = dir + file

      if test == False:
        try:
          f = filePool[file]
        except:
          outdir = '/'.join((archiveDirectory + file).split('/')[:-1])
          if create_dir(outdir) == False:
            sys.stderr.write("Could not create directory '%s'\n" % outdir)
            sys.exit(-1)

          try:
            f = open(archiveDirectory + file, 'ab')
          except:
            sys.stderr.write("File '%s' could not be opened for writing\n" % (outputDirectory + file))
            sys.exit(-1)

          # Remove old handles
          if len(filePool) < filePoolSize:
            filePool[file] = f

        f.write(rec.raw().str())

      if verbose:
        sys.stderr.write("%s %s %s\n" % (rec.streamID(), rec.startTime().iso(), file))
  except Exception, e:
    sys.stderr.write("Exception: %s\n" % str(e))
