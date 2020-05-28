#!/usr/bin/env seiscomp-python

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
import re
from seiscomp3 import Client, IO, DataModel, Core


class WfqQuery(Client.Application):

    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)

        self.setPrimaryMessagingGroup("LISTENER_GROUP")
        self.addMessagingSubscription("QC")
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(True)

    def createCommandLineDescription(self):
        self.commandline().addGroup("Output")
        self.commandline().addStringOption("Output", "filename,o", "xml output filename")
        self.commandline().addOption("Output", "formatted,f", "use formatted xml output")

        self.commandline().addGroup("Query")
        self.commandline().addStringOption("Query", "streamID,w",
                                           "waveform stream ID: <networkCode>.<stationCode>.<locationCode>.<channelCode>")
        self.commandline().addStringOption("Query", "parameter,p",
                                           "QC parameter: (e.g. LATENCY, DELAY, ...)")
        self.commandline().addStringOption(
            "Query", "startTime,b", "start time: YYYY-MM-DD hh:mm:ss")
        self.commandline().addStringOption(
            "Query", "endTime,e", "end time: YYYY-MM-DD hh:mm:ss")

        return True

    def validateParameters(self):
        if self.commandline().hasOption("filename"):
            self._outfile = self.commandline().optionString("filename")
        else:
            sys.stderr.write("Please specify the xml output filename!\n")
            return False

        if (not self.commandline().hasOption("streamID") or not self.commandline().hasOption("parameter") or
                not self.commandline().hasOption("startTime") or not self.commandline().hasOption("endTime")):
            sys.stderr.write(
                "Please give me all query parameters (--streamID, --parameter, --startTime, --endTime)!\n")
            return False

        self._streamID = self.commandline().optionString("streamID")
        if re.search("[*?]", self._streamID):
            sys.stderr.write("Please do not use wildcards in the streamID!\n")
            return False

        self._parameter = self.commandline().optionString("parameter")
        # if not self._parameter in ([DataModel.EQCNameNames.name(i).replace(" ","_").upper() for i in xrange(DataModel.EQCNameQuantity)]):
        #sys.stderr.write("The given parameter name (%s) is unknown\n" % self._parameter)
        # return False

        self._start = self.commandline().optionString("startTime")
        self._end = self.commandline().optionString("endTime")

        return True

    def run(self):
        if not self.query():
            sys.stderr.write("No database connection!\n")
            return False

        xarc = IO.XMLArchive()
        if not xarc.create(self._outfile, True, True):
            sys.stderr.write(
                "Could not create xml output file %s!\n" % self._outfile)
            return False

        xarc.setFormattedOutput(self.commandline().hasOption("formatted"))
        (net, sta, loc, cha) = self._streamID.split(".")
        it = self.query().getWaveformQuality(DataModel.WaveformStreamID(net, sta, loc, cha, ""),
                                             self._parameter,
                                             Core.Time.FromString(
                                                 self._start, "%Y-%m-%d %H:%M:%S"),
                                             Core.Time.FromString(self._end, "%Y-%m-%d %H:%M:%S"))
        while it.get():
            wfq = DataModel.WaveformQuality.Cast(it.get())
            xarc.writeObject(wfq)
            it.step()

        xarc.close()
        return True


app = WfqQuery(len(sys.argv), sys.argv)
sys.exit(app())
