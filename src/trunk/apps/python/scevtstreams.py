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

import sys, traceback
import seiscomp3.Client

class EventStreams(seiscomp3.Client.Application):
  def __init__(self, argc, argv):
    seiscomp3.Client.Application.__init__(self, argc, argv)

    self.setMessagingEnabled(False)
    self.setDatabaseEnabled(True, False)
    self.setDaemonEnabled(False)

    self.eventID = None
    self.margin = 300

    self.allComponents = True
    self.allLocations = True

    self.streams = []


  def createCommandLineDescription(self):
    self.commandline().addGroup("Dump")
    self.commandline().addStringOption("Dump", "event,E", "event id")
    self.commandline().addIntOption("Dump", "margin,m", "time margin around the picked timewindow, default is 300")
    self.commandline().addStringOption("Dump", "streams,S", "comma separated list of streams per station to add, e.g. BH,SH,HH")
    self.commandline().addIntOption("Dump", "all-components,C", "all components or just the picked one, default is True")
    self.commandline().addIntOption("Dump", "all-locations,L", "all components or just the picked one, default is True")
    self.commandline().addOption("Dump", "resolve-wildcards,R", "if all components are used, use inventory to resolve stream components instead of using '?' (important when Arclink should be used)")
    return True

  def validateParameters(self):
    if self.commandline().hasOption("resolve-wildcards"):
      self.setLoadStationsEnabled(True)
    return True

  def init(self):
    try:
      if not seiscomp3.Client.Application.init(self): return False

      try:
        self.eventID = self.commandline().optionString("event")
      except:
        sys.stderr.write("An eventID is mandatory")
        return False

      try:
        self.margin = self.commandline().optionInt("margin")
      except: pass

      try:
        self.streams = self.commandline().optionString("streams").split(",")
      except: pass

      try:
        self.allComponents = self.commandline().optionInt("all-components") != 0
      except: pass

      try:
        self.allLocations = self.commandline().optionInt("all-locations") != 0
      except: pass

      return True
    except:
      cla, exc, trbk = sys.exc_info()
      sys.stderr.write("%s\n" % cla.__name__)
      sys.stderr.write("%s\n" % exc.__dict__["args"])
      sys.stderr.write("%s\n" % traceback.format_tb(trbk, 5))


  def run(self):
    try:
      picks = []
      minTime = None
      maxTime = None

      resolveWildcards = self.commandline().hasOption("resolve-wildcards")

      for obj in self.query().getEventPicks(self.eventID):
        pick = seiscomp3.DataModel.Pick.Cast(obj)
        if pick is None: continue
        picks.append(pick)

        if minTime is None: minTime = pick.time().value()
        elif minTime > pick.time().value(): minTime = pick.time().value()

        if maxTime is None: maxTime = pick.time().value()
        elif maxTime < pick.time().value(): maxTime = pick.time().value()

      if minTime: minTime = minTime - seiscomp3.Core.TimeSpan(self.margin)
      if maxTime: maxTime = maxTime + seiscomp3.Core.TimeSpan(self.margin)

      inv = seiscomp3.Client.Inventory.Instance().inventory()

      lines = set()
      for pick in picks:
        loc = pick.waveformID().locationCode()
        streams = [pick.waveformID().channelCode()]
        rawStream = streams[0][:2]

        if self.allComponents == True:
          if resolveWildcards:
            iloc = seiscomp3.DataModel.getSensorLocation(inv, pick)
            if iloc:
              tc = seiscomp3.DataModel.ThreeComponents()
              seiscomp3.DataModel.getThreeComponents(tc, iloc, rawStream, pick.time().value());
              streams = []
              if tc.vertical(): streams.append(tc.vertical().code())
              if tc.firstHorizontal(): streams.append(tc.firstHorizontal().code())
              if tc.secondHorizontal(): streams.append(tc.secondHorizontal().code())
          else:
            streams = [rawStream + "?"]

        if self.allLocations == True:
          loc = ""

        for s in streams:
          line = minTime.toString("%F %T") + ";" + maxTime.toString("%F %T") + ";" \
          + pick.waveformID().networkCode() + "." + pick.waveformID().stationCode() \
          + "." + loc + "." +  s
          lines.add(line)

        for s in self.streams:
          if s != rawStream:
            line = minTime.toString("%F %T") + ";" + maxTime.toString("%F %T") + ";" \
            + pick.waveformID().networkCode() + "." + pick.waveformID().stationCode() \
            + "." + loc + "." +  s  + streams[0][2]
            lines.add(line)

      for line in sorted(lines):
          sys.stdout.write(line+"\n")

      return True
    except:
      info = traceback.format_exception(*sys.exc_info())
      for i in info: sys.stderr.write(i)


app = EventStreams(len(sys.argv), sys.argv)
sys.exit(app())
