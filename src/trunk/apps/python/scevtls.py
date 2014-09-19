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

import traceback, sys
import seiscomp3.Client

class EventList(seiscomp3.Client.Application):
  def __init__(self, argc, argv):
    seiscomp3.Client.Application.__init__(self, argc, argv)

    self.setMessagingEnabled(False)
    self.setDatabaseEnabled(True, False)
    self.setDaemonEnabled(False)

    self._startTime = None
    self._endTime = None
    self._delimiter = None


  def createCommandLineDescription(self):
    self.commandline().addGroup("Events")
    self.commandline().addStringOption("Events", "begin", "specify the lower bound of the time interval")
    self.commandline().addStringOption("Events", "end", "specify the upper bound of the time interval")
    self.commandline().addStringOption("Events", "delimiter,D", "specify the delimiter of the resulting event ids (default: '\\n')")
    return True


  def init(self):
    if not seiscomp3.Client.Application.init(self): return False

    try:
      start = self.commandline().optionString("begin")
      self._startTime = seiscomp3.Core.Time()
      if self._startTime.fromString(start, "%F %T") == False:
        print >> sys.stderr, "Wrong 'begin' format '%s' -> setting to None" % start
    except:
      print >> sys.stderr, "Wrong 'begin' format -> setting to None"
      self._startTime = seiscomp3.Core.Time()

    print >> sys.stderr, "Setting start to %s" % self._startTime.toString("%F %T")

    try:
      end = self.commandline().optionString("end")
      self._endTime = seiscomp3.Core.Time.FromString(end, "%F %T")
    except:
      self._endTime = seiscomp3.Core.Time.GMT()

    print >> sys.stderr, "Setting end to %s" % self._endTime.toString("%F %T")

    try:
      self._delimiter = self.commandline().optionString("delimiter")
    except:
      pass

    return True


  def run(self):
    first = True
    for obj in self.query().getEvents(self._startTime, self._endTime):
      evt = seiscomp3.DataModel.Event.Cast(obj)
      if evt is None: continue

      if self._delimiter is None:
        print evt.publicID()
      else:
        if first:
          first = False
          sys.stdout.write(evt.publicID())
        else:
          sys.stdout.write("%s%s" % (self._delimiter, evt.publicID()))
        sys.stdout.flush()

    return True


try:
  app = EventList(len(sys.argv), sys.argv)
  rc = app()
except:
  print traceback.format_exc()
  sys.exit(1)

sys.exit(rc)
