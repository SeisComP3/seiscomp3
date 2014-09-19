#!/usr/bin/env python
#***************************************************************************** 
# nettab2xml.py
#
# Convert network tables (version 1) to XML
#
# (c) 2009 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import sys
import time
import datetime
import os.path
import decimal
import fnmatch
import copy
import glob
from decimal import Decimal
from seiscomp import logs
from seiscomp.db.seiscomp3 import sc3wrap
from seiscomp.db.seiscomp3.routing import Routing as SC3Routing
from seiscomp.db.seiscomp3.inventory import Inventory as SC3Inventory
from nettab.nettab import Nettab, Instruments, NettabError, _EPOCH_DATE
from seiscomp3 import Core, Client, DataModel, Logging, Config, System, IO
from seiscomp3.helpers import writeobj

VERSION = "0.1 (2012.143)"

class SyncNettab(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
    
        self.dcid = None
        self.inst_db_file = None
        self.stat_map_file = None
        self.access_net_file = None
        self.access_stat_file = None
        self.sensor_attr_file = None
        self.datalogger_attr_file = None
        self.network_attr_file = None
        self.station_attr_file = None

        self.setLoggingToStdErr(True)
        self.setMessagingEnabled(False)
        self.setDatabaseEnabled(False, False)
    
    def createCommandLineDescription(self):
        Client.Application.createCommandLineDescription(self)

        self.commandline().addGroup("Convert")
        self.commandline().addOption("Convert", "formatted,f", "Enables formatted output")

        self.commandline().addGroup("ArcLink")
        self.commandline().addStringOption("ArcLink", "dcid", "datacenter/archive ID")
        self.commandline().addStringOption("ArcLink", "inst-db", "path to inst.db")
        self.commandline().addStringOption("ArcLink", "stat-map", "path to stat_map.lst")
        self.commandline().addStringOption("ArcLink", "access-net", "path to access.net")
        self.commandline().addStringOption("ArcLink", "access-stat", "path to access.stat")
        self.commandline().addStringOption("ArcLink", "sensor-attr", "path to sensor_attr.csv")
        self.commandline().addStringOption("ArcLink", "datalogger-attr", "path to datalogger_attr.csv")
        self.commandline().addStringOption("ArcLink", "network-attr", "path to network_attr.csv")
        self.commandline().addStringOption("ArcLink", "station-attr", "path to station_attr.csv")

    def validateParameters(self):
        try:
            opts = self.commandline().unrecognizedOptions()
            if len(opts) < 1:
                self.input_dir = os.path.join(System.Environment.Instance().appConfigDir(), "nettab");
            else:
                self.input_dir = opts[0]

            if len(opts) < 2:
                self.out_file = "-"
            else:
                self.out_file = opts[1]

            if self.commandline().hasOption("dcid"):
                self.dcid = self.commandline().optionString("dcid")

            if self.commandline().hasOption("stat-map"):
                self.stat_map_file = self.commandline().optionString("stat-map")
            else:
                self.stat_map_file = os.path.join(self.input_dir, "stat_map.lst")

            if self.commandline().hasOption("access-net"):
                self.access_net_file = self.commandline().optionString("access-net")
            else:
                self.access_net_file = os.path.join(self.input_dir, "access.net")

            if self.commandline().hasOption("access-stat"):
                self.access_stat_file = self.commandline().optionString("access-stat")
            else:
                self.access_stat_file = os.path.join(self.input_dir, "access.stat")

            if self.commandline().hasOption("sensor-attr"):
                self.sensor_attr_file = self.commandline().optionString("sensor-attr")
            else:
                self.sensor_attr_file = os.path.join(self.input_dir, "sensor_attr.csv")

            if self.commandline().hasOption("datalogger-attr"):
                self.datalogger_attr_file = self.commandline().optionString("datalogger-attr")
            else:
                self.datalogger_attr_file = os.path.join(self.input_dir, "datalogger_attr.csv")

            if self.commandline().hasOption("network-attr"):
                self.network_attr_file = self.commandline().optionString("network-attr")
            else:
                self.network_attr_file = os.path.join(self.input_dir, "network_attr.csv")

            if self.commandline().hasOption("station-attr"):
                self.station_attr_file = self.commandline().optionString("station-attr")
            else:
                self.station_attr_file = os.path.join(self.input_dir, "station_attr.csv")

            if self.commandline().hasOption("inst-db"):
                self.inst_db_file = self.commandline().optionString("inst-db")
            else:
                self.inst_db_file = os.path.join(self.input_dir, "inst.db")

        except Exception:
            logs.print_exc()
            return False

        self.tab_files = glob.glob(os.path.join(self.input_dir, "*.tab"))

        return True
    
    def initConfiguration(self):
        if not Client.Application.initConfiguration(self):
            return False
 
        # force logging to stderr even if logging.file = 1
        self.setLoggingToStdErr(True)

        try:
            self.dcid = self.configGetString("datacenterID")

        except Config.Exception:
            pass

        return True
        
    def __load_file(self, func, file):
        if file:
            logs.info("loading " + file)
            func(file)

    def run(self):
        try:
            if self.dcid is None:
                print >>sys.stderr, "Please specify datacenter/archive ID"
                return False
            
            nettab = Nettab(self.dcid)
            instdb = Instruments(self.dcid)

            try:
                self.__load_file(instdb.load_db, self.inst_db_file)
                self.__load_file(nettab.load_statmap, self.stat_map_file)
                self.__load_file(nettab.load_access_net, self.access_net_file)
                self.__load_file(nettab.load_access_stat, self.access_stat_file)
                self.__load_file(instdb.load_sensor_attr, self.sensor_attr_file)
                self.__load_file(instdb.load_datalogger_attr, self.datalogger_attr_file)
                self.__load_file(nettab.load_network_attr, self.network_attr_file)
                self.__load_file(nettab.load_station_attr, self.station_attr_file)

                inv = SC3Inventory(DataModel.Inventory())

                idx = 1
                for tab in sorted(self.tab_files):
                    print >>sys.stderr, "Loading %s (%d/%d)" % (tab, idx, len(self.tab_files))
                    self.__load_file(nettab.load_tab, tab)
                    print >>sys.stderr, "Generating data structures"
                    nettab.update_inventory(instdb, inv)
                    idx = idx+1
                    if self.isExitRequested():
                        print >>sys.stderr, "Exit requested, abort"
                        return False

                print >>sys.stderr, "Generating output"
                ar = IO.XMLArchive()
                ar.setFormattedOutput(self.commandline().hasOption("formatted"))
                ar.create(self.out_file)
                ar.writeObject(inv.obj)
                ar.close()
                print >>sys.stderr, "Finished"

            except (IOError, NettabError), e:
                logs.error("fatal error: " + str(e))
                return False

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
    app = SyncNettab(len(sys.argv), sys.argv)
    sys.exit(app())

