#!/usr/bin/env python
#***************************************************************************** 
# fill_db.py
#
# Fill database using inventory data in XML format
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
import time
import datetime
from seiscomp import logs
from seiscomp.db.seiscomp3 import sc3wrap
from seiscomp.db.seiscomp3.routing import Routing as SC3Routing
from seiscomp.db.seiscomp3.inventory import Inventory as SC3Inventory
from seiscomp3 import Core, Client, DataModel, Logging

VERSION = "1.2 (2012.313)"

class FillDB(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
    
        self.routingMode = False
        self.input_files = None

        self.setLoggingToStdErr(True)
        
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)

        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)
    
        self.setPrimaryMessagingGroup("LISTENER_GROUP")

    def createCommandLineDescription(self):
        Client.Application.createCommandLineDescription(self)

        self.commandline().addGroup("ArcLink")
        self.commandline().addOption("ArcLink", "routing", "fill routing instead of inventory")
        
    def validateParameters(self):
        try:
            if self.commandline().hasOption("routing"):
                self.routingMode = True

            args = self.commandline().unrecognizedOptions()
            if len(args) < 1:
                print("Usage: fill_db [options] files...",file=sys.stderr)
                return False

            self.input_files = args

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
        
    def send(self, *args):
        while not self.connection().send(*args):
            logs.warning("send failed, retrying")
            time.sleep(1)
    
    def send_notifiers(self, group):
        Nsize = DataModel.Notifier.Size()

        if Nsize > 0:
            logs.warning("trying to apply %d changes..." % Nsize)
        else:
            logs.notice("no changes to apply")
            return 0

        Nmsg = DataModel.Notifier.GetMessage(True)
            
        it = Nmsg.iter()
        msg = DataModel.NotifierMessage()

        maxmsg = 100
        sent = 0
        mcount = 0

        try:
            try:
                while it.get():
                    msg.attach(DataModel.Notifier_Cast(it.get()))
                    mcount += 1
                    if msg and mcount == maxmsg:
                        sent += mcount
                        logs.debug("sending message (%5.1f %%)" % (sent / float(Nsize) * 100.0))
                        self.send(group, msg)
                        msg.clear()
                        mcount = 0
                        self.sync("fill-db")
                    it.next()
            except:
                pass
        finally:
            if msg.size():
                logs.debug("sending message (%5.1f %%)" % 100.0)
                self.send(group, msg)
                msg.clear()
                self.sync("fill-db")

        return mcount

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

                for f in self.input_files:
                    self.inv.load_xml(f)

                self.inv.flush()
                self.send_notifiers("INVENTORY")
            else:
                self.rtn = SC3Routing(self.query().loadRouting())
                self.rtn.load_routes("*", "*")

                for f in self.input_files:
                    self.rtn.load_xml(f)

                self.rtn.flush()
                self.send_notifiers("ROUTING")

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
    app = FillDB(len(sys.argv), sys.argv)
    sys.exit(app())

