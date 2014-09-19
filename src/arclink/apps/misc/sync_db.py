#!/usr/bin/env python
#***************************************************************************** 
# sync_db.py
#
# Synchronize ArcLink/SC3 database
#
# (c) 2008 Andres Heinloo, GFZ Potsdam
# (c) 2008 Mathias Hoffmann, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import os
import sys
import time
import datetime
from seiscomp import logs
from seiscomp.arclink.manager import *
from seiscomp.db.generic.routing import Routing as GRouting
from seiscomp.db.generic.inventory import Inventory as GInventory
from seiscomp3 import Core, Config, Client, DataModel, Logging

try:
    from seiscomp.db.seiscomp3 import sc3wrap
    from seiscomp.db.seiscomp3.routing import Routing as SC3Routing
    from seiscomp.db.seiscomp3.inventory import Inventory as SC3Inventory
    have_sc3wrap = True

except ImportError:
    have_sc3wrap = False

try:
    import sqlobject
    from seiscomp.db.sqlobject.routing import Routing as SORouting
    from seiscomp.db.sqlobject.inventory import Inventory as SOInventory
    have_sqlobject = True

except ImportError:
    have_sqlobject = False

VERSION = "1.0 (2010.256)"

_rx_datetime = re.compile("([0-9]{4})-([0-9]{2})-([0-9]{2})T" \
    "([0-9]{2}):([0-9]{2}):([0-9]{2})(.([0-9]*))?" \
    "(Z|([+-])([0-9]{2}):([0-9]{2}))?$")

_rx_date = re.compile("([0-9]*)-([0-9]*)-([0-9]*)" \
    "(Z|([+-])([0-9]{2}):([0-9]{2}))?$")

def _parse_datetime(val):
    m = _rx_datetime.match(val)
    if m == None:
        m = _rx_date.match(val)
        if m == None:
            raise ValueError, "invalid datetime: " + val

        (year, month, mday, tz, plusminus, tzhours, tzminutes) = m.groups()

        try:
            # ignore time zone
            obj = datetime.datetime(int(year), int(month), int(mday), 0, 0, 0)
        except ValueError:
            raise ValueError, "invalid datetime: " + val
    else:
        (year, month, mday, hour, min, sec, sfract1, sfract,
            tz, plusminus, tzhours, tzminutes) = m.groups()

        if sfract is None:
            sfract = "0"

        try:
            obj = datetime.datetime(int(year), int(month), int(mday),
                int(hour), int(min), int(sec), int((sfract + "000000")[:6]))
            if tz is not None and tz != "Z":
                delta = datetime.timedelta(hours = int(tzhours),
                    minutes=int(tzminutes))
                if plusminus == "-":
                    obj -= delta
                else:
                    obj += delta
                
        except ValueError:
            raise ValueError, "invalid datetime: " + val

    return obj

class ArclinkSynchronizer(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
    
        self.source_addr = None
        self.dcid = None
        self.sync_dcid = None
        self.modified_after = None
        self.use_sc3db = have_sc3wrap
        self.db_url = None

        self.setLoggingToStdErr(True)
        
        self.setMessagingEnabled(have_sc3wrap)
        self.setDatabaseEnabled(have_sc3wrap, have_sc3wrap)
    
        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)
    
        self.setPrimaryMessagingGroup("LISTENER_GROUP")

    def createCommandLineDescription(self):
        Client.Application.createCommandLineDescription(self)

        self.commandline().addGroup("ArcLink")
        self.commandline().addStringOption("ArcLink", "source", "source address")
        self.commandline().addStringOption("ArcLink", "dcid", "own datacenter/archive ID")
        self.commandline().addStringOption("ArcLink", "sync-dcid", "foreign datacenter/archive ID")
        self.commandline().addStringOption("ArcLink", "modified-after", "timestamp")
        self.commandline().addIntOption("ArcLink", "use-sc3db", "use SC3 messaging/database", have_sc3wrap)
        self.commandline().addStringOption("ArcLink", "db-url", "database URL (sqlobject only)")
        
    def validateParameters(self):
        try:
            if self.commandline().hasOption("source"):
                self.source_addr = self.commandline().optionString("source")

            if self.commandline().hasOption("dcid"):
                self.dcid = self.commandline().optionString("dcid")

            if self.commandline().hasOption("sync-dcid"):
                self.sync_dcid = self.commandline().optionString("sync-dcid")

            if self.commandline().hasOption("modified-after"):
                self.modified_after = self.commandline().optionString("modified-after")

            if self.commandline().hasOption("use-sc3db"):
                self.use_sc3db = self.commandline().optionInt("use-sc3db")

            if self.commandline().hasOption("db-url"):
                self.db_url = self.commandline().optionString("db-url")

            if self.source_addr is None:
                print >>sys.stderr, "Please specify source address using --source"
                return False

            if self.modified_after:
                try:
                    _parse_datetime(self.modified_after)

                except ValueError, e:
                    logs.error(str(e))
                    return False

            if not have_sc3wrap and not have_sqlobject:
                print >>sys.stderr, "Neither SC3 nor sqlobject support is available"
                return False

            if self.use_sc3db:
                if not have_sc3wrap:
                    print >>sys.stderr, "SC3 database support is not available"
                    return False

            else:
                if not have_sqlobject:
                    print >>sys.stderr, "sqlobject support not is available"
                    return False

                if self.db_url is None:
                    print >>sys.stderr, "Please specify database URL using --db-url"
                    return False

        except Exception:
            logs.print_exc()
            return False

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

    def send(self, *args):
        while not self.connection().send(*args):
            logs.warning("send failed, retrying")
            time.sleep(1)
    
    def send_notifiers(self, group):
        Nsize = DataModel.Notifier.Size()

        if Nsize > 0:
            logs.info("trying to apply %d changes..." % Nsize)
        else:
            logs.info("no changes to apply")
            return

        Nmsg = DataModel.Notifier.GetMessage(True)
            
        it = Nmsg.iter()
        msg = DataModel.NotifierMessage()

        maxmsg = 100
        sent = 0
        mcount = 0

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
                    self.sync("sync-db")
                it.next()
        except:
            pass

        finally:
            if msg.size():
                logs.debug("sending message (%5.1f %%)" % 100.0)
                self.send(group, msg)
                msg.clear()
                self.sync("sync-db")

    def request_netlist(self):
        req_args = {"compression": "bzip2" }

        req_args["self_dcid"] = self.dcid

        if self.sync_dcid:
            req_args["sync_dcid"] = self.sync_dcid

        req = self.arcl.new_request("INVENTORY", {"compression": "bzip2"})

        req.add("*", ".", ".", ".", datetime.datetime(1980,1,1,0,0,0),
            datetime.datetime(2030,12,31,0,0,0))

        req.submit(self.source_addr, "sync@" + self.dcid)

        if req.error is not None:
            raise ArclinkError, req.error

        inv = GInventory()
        req.download_xml(inv, True)

        self.nets = set()
        for net_tp in inv.network.itervalues():
            for net in net_tp.itervalues():
                self.nets.add(net.code)

    def request_stations(self, type):
        req_args = {"compression": "bzip2" }

        if type == "INVENTORY":
            req_args["instruments"] = "true"
            req_args["self_dcid"] = self.dcid

            if self.sync_dcid:
                req_args["sync_dcid"] = self.sync_dcid

        if self.modified_after:
            req_args["modified_after"] = self.modified_after

        req = self.arcl.new_request(type, req_args)

        if type == "INVENTORY":
            req.add("*", "*", "*", "*", datetime.datetime(1980,1,1,0,0,0),
                datetime.datetime(2030,12,31,0,0,0))

        else:
            for net_code in self.nets:
                req.add(net_code, "*", "*", "*", datetime.datetime(1980,1,1,0,0,0),
                    datetime.datetime(2030,12,31,0,0,0))

        req.submit(self.source_addr, "sync@" + self.dcid)

        if req.error is not None:
            raise ArclinkError, req.error

        return req

    def cleanup_inventory(self):
        self.cleaned_stations = set()

        for (net_code, net_tp) in self.inv.network.items():
            net_tp0 = self.inv0.network.get(net_code)
            for net in net_tp.values():
                if net_tp0 is None:
                   net0 = None
                else:
                   net0 = net_tp0.get(net.start)

                for (sta_code, sta_tp) in net.station.items():
                    if net0 is None:
                        sta_tp0 = None
                    else:
                        sta_tp0 = net0.station.get(sta_code)

                    for sta in sta_tp.values():
                        if sta.archive == self.dcid or \
                            (self.sync_dcid and sta.archive != self.sync_dcid):
                            continue

                        self.cleaned_stations.add((net_code, sta_code))

                        if sta_tp0 is None:
                            net.remove_station(sta.code, sta.start)
                            continue

                        sta0 = sta_tp0.get(sta.start)
                        if sta0 is None:
                            net.remove_station(sta.code, sta.start)
                            continue

                        for (loc_code, loc_tp) in sta.sensorLocation.items():
                            loc_tp0 = sta0.sensorLocation.get(loc_code)
                            for loc in loc_tp.values():
                                if loc_tp0 is None:
                                    sta.remove_sensorLocation(loc.code, loc.start)
                                    continue

                                loc0 = loc_tp0.get(loc.start)
                                if loc0 is None:
                                    sta.remove_sensorLocation(loc.code, loc.start)
                                    continue
                        
                                for (strm_code, strm_tp) in loc.stream.items():
                                    strm_tp0 = loc0.stream.get(strm_code)
                                    for strm in strm_tp.values():
                                        if strm_tp0 is None:
                                            loc.remove_stream(strm.code, strm.start)
                                            continue

                                        strm0 = strm_tp0.get(strm.start)
                                        if strm0 is None:
                                            loc.remove_stream(strm.code, strm.start)

                                for (strm_code, strm_tp) in loc.auxStream.items():
                                    strm_tp0 = loc0.auxStream.get(strm_code)
                                    for strm in strm_tp.values():
                                        if strm_tp0 is None:
                                            loc.remove_auxStream(strm.code, strm.start)
                                            continue

                                        strm0 = strm_tp0.get(strm.start)
                                        if strm0 is None:
                                            loc.remove_auxStream(strm.code, strm.start)

                if not net.station:
                    self.inv.remove_network(net.code, net.start)

    def cleanup_routing(self):
        for route_net in self.rtn.route.values():
            for route_sta in route_net.values():
                for route_loc in route_sta.values():
                    for route in route_loc.values():
                        if (route.networkCode, route.stationCode) not in self.cleaned_stations:
                            continue

                        try:
                            route0 = self.rtn0.route[route.networkCode][route.stationCode][route.locationCode][route.streamCode]

                        except KeyError:
                            self.rtn.remove_route(route.networkCode, route.stationCode, route.locationCode, route.streamCode)
                            continue

                        for arclink_addr in route.arclink.values():
                            for arclink in arclink_addr.values():
                                try:
                                    arclink0 = route0.arclink[arclink.address][arclink.start]

                                except KeyError:
                                    route.remove_arclink(arclink.address, arclink.start)

                        for seedlink in route.seedlink.values():
                            try:
                                seedlink0 = route0.seedlink[seedlink.address]

                            except KeyError:
                                route.remove_seedlink(seedlink.address)

    def run(self):
        try:
            if self.dcid is None:
                print >>sys.stderr, "Please specify datacenter/archive ID"
                return False
            
            if self.use_sc3db:
                sc3wrap.dbQuery = self.query()
                DataModel.Notifier.Enable()
                DataModel.Notifier.SetCheckEnabled(False)
            
            else:
                connection = sqlobject.connectionForURI(self.db_url)
                sqlobject.sqlhub.processConnection = connection

            self.arcl = ArclinkManager(self.source_addr)

            logs.info("request network list from " + self.source_addr)
            self.request_netlist()

            self.inv0 = GInventory()
            logs.info("request inventory from " + self.source_addr)
            req = self.request_stations("INVENTORY")
            req.download_xml(self.inv0, True)
            self.inv0.save_xml("/tmp/inventory.xml", instr=2)

            logs.info("load local inventory")

            if self.use_sc3db:
                self.inv = SC3Inventory(self.query().loadInventory())

            else:
                self.inv = SOInventory()

            self.inv.load_stations("*")
            self.inv.load_stations("*", "*")
            self.inv.load_stations("*", "*", "*")
            self.inv.load_stations("*", "*", "*", "*")
            self.inv.load_instruments()
            
            logs.info("merge remote inventory")
            self.inv.load_xml("/tmp/inventory.xml")

            if not self.modified_after:
                logs.info("cleanup local inventory")
                self.cleanup_inventory()

            logs.info("flush changes")
            self.inv.flush()

            if self.use_sc3db:
                self.send_notifiers("INVENTORY")

            self.rtn0 = GRouting()
            logs.info("request routing from " + self.source_addr)
            req = self.request_stations("ROUTING")
            req.download_xml(self.rtn0, True)
            self.rtn0.save_xml("/tmp/routing.xml")

            logs.info("load local routing")

            if self.use_sc3db:
                self.rtn = SC3Routing(self.query().loadRouting())

            else:
                self.rtn = SORouting()

            self.rtn.load_routes("*", "*")

            logs.info("merge remote routing")
            self.rtn.load_xml("/tmp/routing.xml")
            
            if not self.modified_after:
                logs.info("cleanup local routing")
                self.cleanup_routing()

            logs.info("flush changes")
            self.rtn.flush()

            if self.use_sc3db:
                self.send_notifiers("ROUTING")
            
            os.unlink("/tmp/inventory.xml")
            os.unlink("/tmp/routing.xml")

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
    app = ArclinkSynchronizer(len(sys.argv), sys.argv)
    sys.exit(app())

