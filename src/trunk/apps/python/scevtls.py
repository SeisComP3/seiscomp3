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
from seiscomp3 import Core, Client, DataModel, Logging


def _parseTime(timestring):
    t = Core.Time()
    if t.fromString(timestring, "%F %T"):
        return t
    if t.fromString(timestring, "%FT%T"):
        return t
    if t.fromString(timestring, "%FT%TZ"):
        return t
    return None


class EventList(Client.Application):

    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)

        self.setMessagingEnabled(False)
        self.setDatabaseEnabled(True, False)
        self.setDaemonEnabled(False)

        self._startTime = None
        self._endTime = None
        self._delimiter = None
        self._modifiedAfterTime = None

    def createCommandLineDescription(self):
        self.commandline().addGroup("Events")
        self.commandline().addStringOption("Events", "begin",
                                           "specify the lower bound of the time interval")
        self.commandline().addStringOption("Events", "modified-after",
                                           "select events modified after the specified time")
        self.commandline().addStringOption(
            "Events", "end", "specify the upper bound of the time interval")
        self.commandline().addStringOption("Events", "delimiter,D",
                                           "specify the delimiter of the resulting event ids (default: '\\n')")
        return True

    def init(self):
        if not Client.Application.init(self):
            return False

        try:
            start = self.commandline().optionString("begin")
        except:
            start = "1900-01-01T00:00:00Z"
        self._startTime = _parseTime(start)
        if self._startTime is None:
            Logging.error("Wrong 'begin' format '%s'" % start)
            return False
        Logging.debug("Setting start to %s" %
                      self._startTime.toString("%FT%TZ"))

        try:
            end = self.commandline().optionString("end")
        except:
            end = "2500-01-01T00:00:00Z"
        self._endTime = _parseTime(end)
        if self._endTime is None:
            Logging.error("Wrong 'end' format '%s'" % end)
            return False
        Logging.debug("Setting end to %s" % self._endTime.toString("%FT%TZ"))

        try:
            self._delimiter = self.commandline().optionString("delimiter")
        except:
            self._delimiter = "\n"

        try:
            modifiedAfter = self.commandline().optionString("modified-after")
            self._modifiedAfterTime = _parseTime(modifiedAfter)
            if self._modifiedAfterTime is None:
                Logging.error("Wrong 'modified-after' format '%s'" %
                              modifiedAfter)
                return False
            Logging.debug("Setting 'modified-after' time to %s" %
                          self._modifiedAfterTime.toString("%FT%TZ"))
        except:
            pass

        return True

    def run(self):
        first = True
        out = []
        for obj in self.query().getEvents(self._startTime, self._endTime):
            evt = DataModel.Event.Cast(obj)
            if not evt:
                continue

            if self._modifiedAfterTime is not None:
                try:
                    if evt.creationInfo().modificationTime() < self._modifiedAfterTime:
                        continue
                except ValueError:
                    continue

            out.append(evt.publicID())

        sys.stdout.write("%s\n" % self._delimiter.join(out))

        return True


def main():
    app = EventList(len(sys.argv), sys.argv)
    app()


if __name__ == "__main__":
    main()
