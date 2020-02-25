#!/usr/bin/env python
#***************************************************************************** 
# dump_db.py
#
# Dump inventory database in XML format
#
# (c) 2006 Andres Heinloo, GFZ Potsdam
# (c) 2007 Mathias Hoffmann, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import sys
from seiscomp import logs
from seiscomp.db.seiscomp3 import sc3wrap
from seiscomp.db.seiscomp3.inventory import Inventory as SC3Inventory
from seiscomp.db.seiscomp3.routing import Routing as SC3Routing
from seiscomp3 import Core, Client, DataModel, Logging

VERSION = "1.2 (2012.313)"

class DumpDB(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
        self.routingMode = False
        self.addAccess = False
    
        self.output_file = None

        self.setLoggingToStdErr(True)

        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
    
        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)
    
        self.setPrimaryMessagingGroup("LISTENER_GROUP")

    def createCommandLineDescription(self):
        Client.Application.createCommandLineDescription(self)

        self.commandline().addGroup("ArcLink")
        self.commandline().addOption("ArcLink", "routing", "dump routing instead of inventory")
        self.commandline().addOption("ArcLink", "with-access", "dump access together with routing information")
        
    def validateParameters(self):
        try:
            if self.commandline().hasOption("routing"):
                self.routingMode = True

            if self.commandline().hasOption("with-access"):
                self.addAccess = True

            args = self.commandline().unrecognizedOptions()
            if len(args) != 1:
                print("Usage: dump_db [options] file",file=sys.stderr)
                return False

            self.output_file = args[0]

        except Exception:
            logs.print_exc()
            return False

        return True
    
    def initConfiguration(self):
        if not Client.Application.initConfiguration(self):
            return False
 
        # force logging to stderr even if logging.file = 1
        self.setLoggingToStdErr(True)

        return True
        
    def run(self):
        try:
            sc3wrap.dbQuery = self.query()
            DataModel.Notifier.Enable()
            DataModel.Notifier.SetCheckEnabled(False)
            
            if not self.routingMode:
                self.inv = SC3Inventory(self.query().loadInventory())
                self.inv.load_stations("*")
                self.inv.load_stations("*", "*")
                self.inv.load_stations("*", "*", "*")
                self.inv.load_stations("*", "*", "*", "*")
                self.inv.load_instruments()
                self.inv.save_xml(self.output_file, instr=2)
            else:
                self.rtn = SC3Routing(self.query().loadRouting())
                self.rtn.load_routes("*", "*")
                if self.addAccess:
                    self.rtn.load_access()
                self.rtn.save_xml(self.output_file, self.addAccess)

        except Exception:
            logs.print_exc()
            return False

        return True

if __name__ == "__main__":
    logs.debug = Logging.debug
    logs.info = Logging.info
    logs.notice = Logging.notice
    logs.warning = Logging.warning
    logs.error = Logging.error
    app = DumpDB(len(sys.argv), sys.argv)
    sys.exit(app())

