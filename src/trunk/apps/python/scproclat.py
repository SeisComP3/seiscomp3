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

import time, sys, os, traceback
import seiscomp3.Client, seiscomp3.Utils, seiscomp3.System


def createDirectory(dir):
  if os.access(dir, os.W_OK):
    return True

  try:
    os.makedirs(dir)
    return True
  except:
    return False


def timeToString(t):
  return t.toString("%T.%6f")


def timeSpanToString(ts):
  neg = ts.seconds() < 0 or ts.microseconds() < 0
  secs = abs(ts.seconds())
  days = secs / 86400
  daySecs = secs % 86400
  hours = daySecs / 3600
  hourSecs = daySecs % 3600
  mins = hourSecs / 60
  secs = hourSecs % 60
  usecs = abs(ts.microseconds())

  if neg:
    return "-%.2d:%.2d:%.2d:%.2d.%06d" % (days, hours, mins, secs, usecs)
  else:
    return "%.2d:%.2d:%.2d:%.2d.%06d" % (days, hours, mins, secs, usecs)


class ProcLatency(seiscomp3.Client.Application):
  def __init__(self, argc, argv):
    seiscomp3.Client.Application.__init__(self, argc, argv)
  
    self.setMessagingEnabled(True)
    self.setDatabaseEnabled(False, False)

    self.setAutoApplyNotifierEnabled(False)
    self.setInterpretNotifierEnabled(True)
  
    self.addMessagingSubscription("PICK")
    self.addMessagingSubscription("AMPLITUDE")
    self.addMessagingSubscription("LOCATION")
    self.addMessagingSubscription("MAGNITUDE")
    self.addMessagingSubscription("EVENT")

    self.setPrimaryMessagingGroup("LISTENER_GROUP")

    self._directory = ""
    self._nowDirectory = ""
    self._triggeredDirectory = ""
    self._logCreated = False


  def createCommandLineDescription(self):
    try:
      self.commandline().addGroup("Storage")
      self.commandline().addStringOption("Storage", "directory,o", "Specify the storage directory")
    except:
      seiscomp3.Logging.warning("caught unexpected error %s" % sys.exc_info())


  def initConfiguration(self):
    if not seiscomp3.Client.Application.initConfiguration(self): return False

    try: self._directory = self.configGetString("directory")
    except: pass

    try: self._logCreated = self.configGetBool("logMsgLatency")
    except: pass

    return True


  def init(self):
    if not seiscomp3.Client.Application.init(self): return False

    try: self._directory = self.commandline().optionString("directory")
    except: pass

    try:
      if self._directory[-1] != "/":
        self._directory = self._directory + "/"
    except: pass

    if self._directory:
      self._directory = seiscomp3.System.Environment.Instance().absolutePath(self._directory)
      sys.stderr.write("Logging latencies to %s\n" % self._directory)

    return True


  def addObject(self, parentID, obj):
    try:
      self.logObject(parentID, obj, False)
    except:
      print >> sys.stderr, traceback.format_exc()


  def updateObject(self, parentID, obj):
    try:
      self.logObject("", obj, True)
    except:
      print >> sys.stderr, traceback.format_exc()


  def logObject(self, parentID, obj, update):
    now = seiscomp3.Core.Time.GMT()
    time = None

    pick = seiscomp3.DataModel.Pick.Cast(obj)
    if pick:
      phase = ""
      try:
        phase = pick.phaseHint().code()
      except: pass

      created = None
      if self._logCreated:
        try: created = pick.creationInfo().creationTime()
        except: pass

      self.logStation(now, created, pick.time().value(), pick.publicID() + ";P;" + phase, pick.waveformID(), update)
      return

    amp = seiscomp3.DataModel.Amplitude.Cast(obj)
    if amp:
      created = None
      if self._logCreated:
        try: created = amp.creationInfo().creationTime()
        except: pass

      try:
        self.logStation(now, created, amp.timeWindow().reference(), amp.publicID() + ";A;" + amp.type() + ";" + "%.2f" % amp.amplitude().value(), amp.waveformID(), update)
      except: pass
      return

    org = seiscomp3.DataModel.Origin.Cast(obj)
    if org:
      status = ""
      lat = "%.2f" % org.latitude().value()
      lon = "%.2f" % org.longitude().value()
      try: depth = "%d" % org.depth().value()
      except: pass

      try: status = seiscomp3.DataModel.EOriginStatusNames.name(org.status())
      except: pass

      self.logFile(now, org.time().value(), org.publicID() + ";O;" + status + ";" + lat + ";" + lon + ";" + depth, update)
      return

    mag = seiscomp3.DataModel.Magnitude.Cast(obj)
    if mag:
      count = ""
      try: count = "%d" % mag.stationCount()
      except: pass
      self.logFile(now, None, mag.publicID() + ";M;" + mag.type() + ";" + "%.4f" % mag.magnitude().value() + ";" + count, update)
      return

    orgref = seiscomp3.DataModel.OriginReference.Cast(obj)
    if orgref:
      self.logFile(now, None, parentID + ";OR;" + orgref.originID(), update)
      return

    evt = seiscomp3.DataModel.Event.Cast(obj)
    if evt:
      self.logFile(now, None, evt.publicID() + ";E;" + evt.preferredOriginID() + ";" + evt.preferredMagnitudeID(), update)
      return


  def logStation(self, received, created, triggered, text, waveformID, update):
    streamID = waveformID.networkCode() + "." + waveformID.stationCode() + "." + waveformID.locationCode() + "." + waveformID.channelCode()

    aNow = received.get()
    aTriggered = triggered.get()

    nowDirectory = self._directory + "/".join(["%.2d" % i for i in aNow[1:4]]) + "/"
    triggeredDirectory = self._directory + "/".join(["%.2d" % i for i in aTriggered[1:4]]) + "/"

    logEntry = timeSpanToString(received - triggered) + ";"
    if created is not None:
      logEntry = logEntry + timeSpanToString(received - created) + ";"
    else:
      logEntry = logEntry + ";"

    if update:
      logEntry = logEntry + "U"
    else:
      logEntry = logEntry + "A"

    logEntry = logEntry + ";" + text

    print timeToString(received) + ";" + logEntry

    if nowDirectory != self._nowDirectory:
      if createDirectory(nowDirectory) == False:
        seiscomp3.Logging.error("Unable to create directory %s" % nowDirectory)
        return False

      self._nowDirectory = nowDirectory

    self.writeLog(self._nowDirectory + streamID + ".rcv", timeToString(received) + ";" + logEntry)

    if triggeredDirectory != self._triggeredDirectory:
      if createDirectory(triggeredDirectory) == False:
        seiscomp3.Logging.error("Unable to create directory %s" % triggeredDirectory)
        return False

      self._triggeredDirectory = triggeredDirectory

    self.writeLog(self._triggeredDirectory + streamID + ".trg", timeToString(triggered) + ";" + logEntry)

    return True


  def logFile(self, received, triggered, text, update):
    aNow = received.get()
    nowDirectory = self._directory + "/".join(["%.2d" % i for i in aNow[1:4]]) + "/"
    triggeredDirectory = None

    #logEntry = timeToString(received)
    logEntry = ""

    if not triggered is None:
      aTriggered = triggered.get()
      triggeredDirectory = self._directory + "/".join(["%.2d" % i for i in aTriggered[1:4]]) + "/"

      logEntry = logEntry + timeSpanToString(received - triggered)

    logEntry = logEntry + ";"

    if update:
      logEntry = logEntry + "U"
    else:
      logEntry = logEntry + "A"

    logEntry = logEntry + ";" + text

    print timeToString(received) + ";" + logEntry

    if nowDirectory != self._nowDirectory:
      if createDirectory(nowDirectory) == False:
        seiscomp3.Logging.error("Unable to create directory %s" % nowDirectory)
        return False

      self._nowDirectory = nowDirectory

    self.writeLog(self._nowDirectory + "objects.rcv", timeToString(received) + ";" + logEntry)

    if triggeredDirectory:
      if triggeredDirectory != self._triggeredDirectory:
        if createDirectory(triggeredDirectory) == False:
          seiscomp3.Logging.error("Unable to create directory %s" % triggeredDirectory)
          return False

        self._triggeredDirectory = triggeredDirectory

      self.writeLog(self._triggeredDirectory + "objects.trg", timeToString(triggered) + ";" + logEntry)

    return True


  def writeLog(self, file, text):
    of = open(file, "a")
    if of:
      of.write(text)
      of.write("\n")
      of.close()


app = ProcLatency(len(sys.argv), sys.argv)
sys.exit(app())
