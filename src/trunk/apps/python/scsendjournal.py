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
import seiscomp3.Client


class SendJournal(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)
        self.setDatabaseEnabled(False, False)
        self.setMessagingEnabled(True)
        self.setMessagingUsername("")
        self.setPrimaryMessagingGroup("EVENT")

    def init(self):
        if not seiscomp3.Client.Application.init(self):
            return False
        self.params = self.commandline().unrecognizedOptions()
        if len(self.params) < 2:
            sys.stderr.write(
                self.name() + " [opts] {objectID} {action} [parameters]\n")
            return False
        return True

    def run(self):
        msg = seiscomp3.DataModel.NotifierMessage()

        entry = seiscomp3.DataModel.JournalEntry()
        entry.setCreated(seiscomp3.Core.Time.GMT())
        entry.setObjectID(self.params[0])
        entry.setSender(self.author())
        entry.setAction(self.params[1])

        sys.stderr.write(
            "Sending entry (" + entry.objectID() + "," + entry.action() + ")\n")

        if len(self.params) > 2:
            entry.setParameters(self.params[2])

        n = seiscomp3.DataModel.Notifier(
            seiscomp3.DataModel.Journaling.ClassName(), seiscomp3.DataModel.OP_ADD, entry)
        msg.attach(n)
        self.connection().send(msg)

        return True


def main(argc, argv):
    app = SendJournal(argc, argv)
    return app()


if __name__ == "__main__":
    sys.exit(main(len(sys.argv), sys.argv))
