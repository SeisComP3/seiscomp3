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

import traceback
import sys
import seiscomp3.Client

class OriginList(seiscomp3.Client.Application):
  def __init__(self, argc, argv):
    seiscomp3.Client.Application.__init__(self, argc, argv)

    self.setMessagingEnabled(False)
    self.setDatabaseEnabled(True, False)
    self.setDaemonEnabled(False)

    self._startTime = None
    self._endTime = None


  def createCommandLineDescription(self):
    self.commandline().addGroup("Origins")
    self.commandline().addStringOption("Origins", "begin", "specify the lower bound of the time interval. Time format: '1970-01-01 00:00:00'")
    self.commandline().addStringOption("Origins", "end", "specify the upper bound of the time interval. Time format: '1970-01-01 00:00:00'")
    return True


  def init(self):
    if not seiscomp3.Client.Application.init(self): return False

    try:
      start = self.commandline().optionString("begin")
      self._startTime = seiscomp3.Core.Time()
      if self._startTime.fromString(start, "%F %T") == False:
        sys.stderr.write("Wrong 'begin' format '%s' -> setting to None\n" % start)
    except:
      sys.stderr.write("Wrong 'begin' format -> setting to None\n")
      self._startTime = seiscomp3.Core.Time()

#   sys.stderr.write("Setting start to %s\n" % self._startTime.toString("%F %T"))

    try:
      end = self.commandline().optionString("end")
      self._endTime = seiscomp3.Core.Time.FromString(end, "%F %T")
    except:
      self._endTime = seiscomp3.Core.Time.GMT()

#   sys.stderr.write("Setting end to %s\n" % self._endTime.toString("%F %T"))

    return True


  def run(self):
    q = "select PublicObject.%s, Origin.* from Origin, PublicObject where Origin._oid=PublicObject._oid and Origin.%s >= '%s' and Origin.%s < '%s'" %\
        (self.database().convertColumnName("publicID"),
         self.database().convertColumnName("time_value"),
         self.database().timeToString(self._startTime),
         self.database().convertColumnName("time_value"),
         self.database().timeToString(self._endTime))

    for obj in self.query().getObjectIterator(q, seiscomp3.DataModel.Origin.TypeInfo()):
      org = seiscomp3.DataModel.Origin.Cast(obj)
      if org:
        sys.stdout.write("%s\n" % org.publicID())

    return True


try:
  app = OriginList(len(sys.argv), sys.argv)
  rc = app()
except:
  sys.stderr.write("%s\n" % traceback.format_exc())
  sys.exit(1)

sys.exit(rc)
