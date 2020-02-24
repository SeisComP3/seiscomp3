#!/usr/bin/env python
#***************************************************************************** 
# sync_arc.py
#
# Merges remote ArcLink/SC3 database locally by following a master table located
# in a different server
#
# (c) 2012 Marcelo Bianchi, GFZ Potsdam
# (c) 2008 Andres Heinloo, GFZ Potsdam
# (c) 2008 Mathias Hoffmann, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import os
import glob
import sys
import time
import datetime
from urllib2 import urlopen, URLError, HTTPError
from xml.dom.minidom import parseString

from seiscomp import logs
from seiscomp.arclink.manager import *
from seiscomp.db.seiscomp3 import sc3wrap
from seiscomp.db.generic.routing import Routing as GRouting
from seiscomp.db.seiscomp3.routing import Routing as SC3Routing
from seiscomp.db.generic.inventory import Inventory as GInventory
from seiscomp.db.seiscomp3.inventory import Inventory as SC3Inventory
from seiscomp3 import Core, Config, Client, DataModel, Logging

VERSION = "0.14a (2013.016)"

def overlaps(pstart, pend, cstart, cend):
    if pend:
        if pend > cstart:
            if not cend or pstart < cend:
                return True
    else:
        if not cend or pstart < cend:
            return True
    return False

def isInside(start, end, s, e):
    ## Check if the supplied s->e span is inside start->end
    
    if e and s >= e:
#        raise Exception("Invalid supplied interval for checking (%s %s)" % (s,e))
        return False

    if end and start >= end:
#        raise Exception("Invalid supplied interval (%s %s)" % (start,end))
        return False

    if not s:
#        raise Exception("Invalid interval start")
        return False

    if not start:
#        raise Exception("Invalid start")
        return False

    if s < start:
        return False
    
    if end:
        if s >= end: return False
        if not e: return False
        if e > end: return False
    
    return True

def unWrapNSLC(objs, archive = None, onlyShared = False):
    # unwrap lists of lists into arrays
    list = []
    for (code, spam) in objs.items():
        for (start, obj) in spam.items():
            try:
                if archive is not None and getattr(obj, "archive") != archive:
                    continue
                if onlyShared is True and getattr(obj, "shared") == False:
                    continue
            except:
                pass
            list.append((code, start, obj))
    return list

def unWrapRTN(objs, network = None, type = 'arclink'):
    list = []
    for (nCode, networks) in objs.route.items():
        if network and nCode != network: continue;
        for (sCode, stations) in networks.items():
            for (lCode, locations) in stations.items():
                for (cCode, route) in locations.items():
                    if type == 'arclink':
                        for (address, spans) in route.arclink.items():
                            for (start, arclink) in spans.items():
                                list.append((nCode, sCode, lCode, cCode, address, start, arclink))
                    elif type == 'seedlink':
                        for (address, seedlink) in route.seedlink.items():
                            list.append((nCode, sCode, lCode, cCode, address, seedlink))
                    else:
                        raise Exception("Invalid type: arclink or seedlink")
    return list

def prepareMap(inv, stationReferenceList = None):
    map = {}
    for (ncode, nstart, net) in unWrapNSLC(inv.network):
        for (scode, sstart, sta) in unWrapNSLC(net.station):
            if stationReferenceList and sta.publicID not in stationReferenceList: continue
            map[sta.publicID] = (net.code, net.start, sta.code, sta.start)
    return map

def findSID(map, content):
    for (id, values) in map.items():
        if values == content:
            return id 
    return None

""" 
    This is a copy of the class Node from the file eida.py
    It is copied here to make sync_arc independent from eida.py. 
"""
class Node(object):
    __slots__ = (
            "dcid",
            "name",
            "address",
            "port",
            "contact",
            "email",
            "networks",
            "stationGroups"
    )

    def __init__(self, dcid, address, port):
        for at in self.__slots__:
            setattr(self, at, None)
        self.dcid = dcid
        self.address = address
        self.port = port
        self.networks = {}
        self.stationGroups = {}

    def networkList(self):
        pack = []
        for ones in self.networks.values():
            for obj in ones:
                pack.append(obj)
        return pack

    def stationGroupList(self):
        pack = []
        for ones in self.stationGroups.values():
            for obj in ones:
                pack.append(obj)
        return pack

    def _xml(self, doc):
        xnode = doc.createElement("node")
        for att in self.__slots__:
            if att in ["networks", "stationGroups"]:
                continue
            if getattr(self, att) is not None:
                xnode.setAttribute(att, str(getattr(self, att)))
        for (n,s,e) in self.networkList():
            nnode = doc.createElement("network")
            nnode.setAttribute("code", n)
            nnode.setAttribute("start", str(s))
            if e is not None: nnode.setAttribute("end", str(e))
            xnode.appendChild(nnode)
        for (n,s,e) in self.stationGroupList():
            nnode = doc.createElement("stationGroup")
            nnode.setAttribute("code", n)
            nnode.setAttribute("start", str(s))
            if e is not None: nnode.setAttribute("end", str(e))
            xnode.appendChild(nnode)
        return xnode

    def __overlap__(self, s1, e1, s2, e2):
        if e1:
            if e1 > s2:
                if not e2 or s1 < e2:
                    return True
        else:
            if not e2 or s1 < e2:
                return True
        return False

    def _conflict(self, obj, nn, ss, ee):
        try:
            list = obj[nn]
        except:
            list = []
        
        for (n,s,e) in list:
            if n != nn: continue
            if self.__overlap__(s, e, ss, ee):
                return True
        return False

    def _add(self, obj, code, start, end):
        if code is None:
            raise Exception("Invalid code")
        
        if start is None:
            raise Exception("Invalid start date")
        
        if self._conflict(obj, code, start, end):
            raise Exception("Conflicting items")
        
        if code not in obj:
            obj[code] = []
        
        obj[code].append((code, start, end))
        obj[code].sort()

    def _remove(self, obj, code, start):
        if code is None:
            raise Exception("Invalid network code")
        
        if start is None:
            raise Exception("Invalid start date")
        
        try:
            ones = obj[code]
            for (n,s,e) in ones:
                if code == n and start == s:
                    ones.remove((n,s,e))
                    return
        except:
            raise Exception("Code Not found")
        raise Exception("Start/End not found")

    def addNetwork(self, code, start, end):
        self._add(self.networks, code, start, end)

    def removeNetwork(self, code, start):
        self._remove(self.networks, code, start)

    def addStationGroup(self, code, start, end):
        self._add(self.stationGroups, code, start, end)

    def removeStationGroup(self, code, start):
        self._remove(self.stationGroups, code, start)

    def info(self, where=sys.stderr):
        print >>where,"%s" % (self.dcid)
        print >>where," Name: %s" % (self.name)
        print >>where," Contact: %s" % (self.contact),
        print >>where,"\tEmail: %s" % (self.email)
        print >>where," Address: %-15s" % (self.address),
        print >>where,"\tPort: %s" % (self.port)
        
        nList = self.networkList()
        sgList = self.stationGroupList()
        
        print >>where," %d network%s\t %d station group%s" % (len(nList),"" if len(nList) == 1 else "s", len(sgList), "" if len(sgList) == 1 else "s")
        i=1
        for (n,s,e) in nList:
            print >>where,"  [%d] %s (%s) (%s)" % (i,n,s,e)
            i=i+1
        for (n,s,e) in sgList:
            print >>where,"  [%d] %s (%s) (%s)" % (i,n,s,e)
            i=i+1
        print >>where,""

class SyncNode(Node):
    __slots__ = (
            "_pool",
            "_inv",
            "_rtn",
            "_sginv"
    )

    def __init__(self, dcid, address, port, pool = '.'):
        Node.__init__(self, dcid, address, port)
        self._pool = pool
        self._inv = None
        self._rtn = None
        self._sginv = None

    def _sendRequest(self, obj, type, args, dcid, lines, locationAndChannel = True):
        if len(lines) == 0:
            logs.warning("No items defined for node %s" % self.dcid)
            return False

        sport="%s:%s" % (self.address, self.port)
        arcl = ArclinkManager(sport, socket_timeout = 120, download_retry = 1)
        
        if "compression" not in args:
            args["compression"] = "bzip2"
        
        req = arcl.new_request(type, args)
        for (n, s, e) in lines:
            if s is None:
                e = datetime.datetime(1970,1,1,0,0,0)
            if e is None:
                e = datetime.datetime(2030,1,1,0,0,0)
            if locationAndChannel:
                req.add(n, "*", "*", "*", s, e)
            else:
                req.add(n, "*", ".", ".", s, e)
        
        user = "sync@%s" % dcid
        logs.info("Submiting %s request to server: %s as %s" % (type, sport, user))
        try:
            req.submit(sport, user, None, None)
        except Exception,e:
            logs.error("Error on submit: %s" % str(e))
            return False
        
        if req.error is not None:
            logs.error("Error sending request [%s]" % req.error)
            try:
                req.purge()
                req.close()
            except:
                pass
            return False
        
        try:
            req.download_xml(obj, True, True)
        except Exception,e:
            logs.error("Error on download: %s" % str(e))
            req.close()
            return False
        
        req.close()
        return True

    def _saveToDisk(self, obj):
        if isinstance(obj, GInventory):
            t = "Inventory"
        elif isinstance(obj, GRouting):
            t = "Routing"
        
        if not os.path.isdir(self._pool):
            raise Exception("Cannot save inventory to %s, folder does not exist." % self._pool)

        if not os.access(self._pool, os.W_OK):
            raise Exception("Cannot write to folder %s" % self._pool)

        filename = "%s/%s-%s-%s-RAW.xml" % (self._pool, t, self.dcid, datetime.datetime.now().strftime("%Y%m%d"))
        
        if isinstance(obj, GInventory):
            obj.save_xml(filename, True)
        elif isinstance(obj, GRouting):
            obj.save_xml(filename)
        
        return True
    
    def _loadFromDisk(self, obj, version):
        
        if isinstance(obj, GInventory):
            t = "Inventory"
        elif isinstance(obj, GRouting):
            t = "Routing"
        
        if version:
            filename = "%s/%s-%s-%s-RAW.xml" % (self._pool, t, self.dcid, version)
        else:
            filename = "%s/%s-%s-%s-RAW.xml" % (self._pool, t, self.dcid, datetime.datetime.now().strftime("%Y%m%d"))

        if not os.path.isfile(filename):
            logs.error("%s file %s not found" % (t,filename))
            return False
        
        logs.warning("Loading %s from %s" % (t,filename))
        obj.load_xml(filename)
        
        return True

    def isDefined(self, networkCode, networkStart, networkEnd):
        try:
            for (code, start, end) in self.networks[networkCode]:
                if code == networkCode and start == networkStart:
                    return True
        except:
            return False
        
        return False

    def isActive(self, networkCode):
        now = datetime.datetime.now()
        
        if networkCode not in self.networks:
            raise Exception("Network not found")

        for (code, start, end) in self.networks[networkCode]:
            if end:
                if now > start and now <= end:
                    return True
            else:
                if now > start:
                    return True

            return False

    def checkSpam(self, code, object):
        found = False
        
        # Sanity Check
        if not object:
            return found
        
        if code not in self.networks:
            return found
        
        try:
            for (c, s, e) in self.networks[code]:
                if found: break
                found = isInside(s, e, object.start, object.end)
        except:
            return found
        
        return found

    def overlaps(self, code, object):
        found = False
        
        # Sanity Check
        if not object:
            return found
        
        if code not in self.networks:
            return found
        
        try:
            for (code, start, end) in self.networks[code]:
                if found: break
                found = overlaps(start, end, object.start, object.end)
        except:
            return found
        
        return found

    def _validateRouting(self, rtn):
        if rtn is None:
            return True
    
        aCollection =  []
        sCollection =  []
        collection =  []
        for net in rtn.route.values():
            for sta in net.values():
                for loc in sta.values():
                    for route in loc.values():
                        if route.networkCode is None or route.networkCode == "" or route.networkCode not in self.networks:
                            collection.append((route.networkCode, route.stationCode, route.locationCode, route.streamCode))
                            continue
                        
                        if not self.isActive(route.networkCode):
                            for address in route.seedlink:
                                sCollection.append((route.networkCode, route.stationCode, route.locationCode, route.streamCode, address))
                        
                        for address in route.arclink:
                            for start in route.arclink[address]:
                                if not self.checkSpam(route.networkCode, route.arclink[address][start]):
                                    aCollection.append((route.networkCode, route.stationCode, route.locationCode, route.streamCode, address, start))

        for (n, s, l, c, a, st) in aCollection:
            logs.warning("Cleaning Arclink route: %s/%s to %s.%s.%s.%s" % (a, st, n, s, l, c))
            rtn.route[n][s][l][c].remove_arclink(a,st)

        for (n, s, l, c, a) in sCollection:
            logs.warning("Cleaning Seedlink route: %s to %s.%s.%s.%s" % (a, n, s, l, c))
            rtn.route[n][s][l][c].remove_seedlink(a)

        for (n, s, l, c) in collection:
            logs.warning("Cleaning invalid routing: %s.%s.%s.%s" % (n, s, l, c))
            rtn.remove_route(n,s,l,c)

        for net in rtn.route.values():
            for sta in net.values():
                for loc in sta.values():
                    for route in loc.values():
                        if len(route.arclink) == 0 and len(route.seedlink) == 0:
                            rtn.remove_route(route.networkCode, route.stationCode, route.locationCode, route.streamCode)
        
        return True

    def getRouting(self, dcid, saveraw, loadVersion):
        if self._rtn is None:
            rtn = GRouting()
            if loadVersion:
                status = self._loadFromDisk(rtn, loadVersion)
            else:
                status = self._sendRequest(rtn, "ROUTING", {}, dcid, self.networkList(), True)
                if status is True and saveraw:
                    try:
                        self._saveToDisk(rtn)
                    except Exception,e:
                        logs.error(str(e))
            if status and self._validateRouting(rtn):
                self._rtn = rtn
        
        return self._rtn

    def _validateInventory(self, inv):
        if inv is None:
            return True
        
        for(ncode, nsp, network) in unWrapNSLC(inv.network):
            if network.code not in self.networks:
                logs.warning("Cleaning network %s,%d not on master table" % (network.code, network.start.year))
                inv.remove_network(network.code, network.start)
                continue
            
            if network.archive != self.dcid:
                logs.warning("Cleaning network %s,%d has invalid archive == %s" % (network.code, network.start.year, network.archive))
                inv.remove_network(network.code, network.start)
                continue
            
            if network.shared == False:
                logs.warning("Cleaning network %s,%d has shared flag set to false" % (network.code, network.start.year))
                inv.remove_network(network.code, network.start)
                continue
            
            if not self.isDefined(network.code, network.start, network.end):
                logs.warning("Cleaning network %s,%d has start (%s) and end (%s) dates not listed" %  (network.code, network.start.year, network.start, network.end))
                inv.remove_network(network.code, network.start)
                continue

            for(scode, ss, station) in unWrapNSLC(network.station):
                bad = False
                # Archive flag
                if station.archive != self.dcid:
                    logs.warning("Cleaning station %s,%d on %s,%d has invalid archive == %s" % (station.code, station.start.year, network.code, network.start.year, station.archive))
                    bad = True

                # Shared flag
                if station.shared == False:
                    logs.warning("Cleaning Station %s,%d on %s,%d has shared flag set to false" % (station.code, station.start.year, network.code, network.start.year))
                    bad = True

                # Station Dates (Start)
                if station.start < network.start or (network.end and station.start > network.end):
                    logs.warning("Warning Station %s,%d on %s,%d with invalid start (%s)" % (station.code, station.start.year, network.code, network.start.year, station.start))
                    #bad = True

                # Station Dates (End)
                if station.end:
                    if station.end < network.start or (network.end and station.end > network.end):
                        logs.warning("Cleaning Station %s,%d on %s,%d with invalid end (%s)" % (station.code, station.start.year, network.code, network.start.year, station.end))
                        #bad = True
                else:
                    if network.end:
                        logs.warning("Cleaning Station %s,%d on %s,%d with invalid end (%s)" % (station.code, station.start.year, network.code, network.start.year, station.end))
                        #bad = True
                
                if bad:
                    network.remove_station(station.code, station.start)
        return True

    def getInventory(self, dcid, saveraw, loadVersion):
        if self._inv is None:
            inv = GInventory()
            if loadVersion:
                status = self._loadFromDisk(inv, loadVersion)
            else:
                status = self._sendRequest(inv, "INVENTORY", {"instruments" : "true" }, dcid, self.networkList(), True)
                if status is True and saveraw:
                    try:
                        self._saveToDisk(inv)
                    except Exception,e:
                        logs.error(str(e))
            if status and self._validateInventory(inv):
                self._inv = inv
        return self._inv

    def _validateStationGroup(self, inv):
        map = prepareMap(inv, None)
        for (sgcode, sg) in inv.stationGroup.items():
            for sid in sg.stationReference:
                if sid not in map:
                    logs.warning("Cleaning invalid station reference %s" % sid)
                    sg.remove_stationReference(sid)
        return True

    def getStationGroup(self, dcid):
        if self._sginv is None:
            inv = GInventory()
            status = self._sendRequest(inv, "INVENTORY", {}, dcid, self.stationGroupList(), False)
            self._validateStationGroup(inv)
            if status  and self._validateStationGroup(inv):
                self._sginv = inv
        return self._sginv

class ArclinkSynchronizer(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
        
        self.sendOnDone = False
        self.tidyOnDone = False
        
        self.force = False
        self.doSanityClean = False
        self.doSanity = False
        self.doCheck = False
        self.doMerge = False
        self.doRemove = False
        self.doErase = False
        self.doList = False
        
        self.poolFolder = None
        self.poolKeep = None
        self.dcid = None
        
        self.loadversion = None
        self.keepraw = False
        self.keep = False
        self.nodeList = None
        self.nodeExclude = None
        self.stationGroup = False
        self.dryrun = False
        
        self.inv = None
        self.rtn = None
        
        self.setLoggingToStdErr(True)
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)
        self.setPrimaryMessagingGroup("LISTENER_GROUP")

        DataModel.Notifier.Enable()
        DataModel.Notifier.SetCheckEnabled(False)

    def initConfiguration(self):
        if not Client.Application.initConfiguration(self):
            return False
        
        # force logging to stderr even if logging.file = 1
        self.setLoggingToStdErr(True)
        
        try:
            self.poolFolder = self.configGetString("poolFolder")
        except Config.Exception:
            self.poolFolder = "./"
        
        try:
            self.poolKeep = self.configGetString("poolKeep")
            try:
                self.poolKeep = int(self.poolKeep)
                if self.poolKeep < 0:
                    raise("")
            except Exception, e:
                logs.error("Invalid poolKeep value [%s]" % self.poolKeep)
                logs.error("Using default value of poolKepp=0")
                self.poolKeep = 0
        except Config.Exception:
            self.poolKeep = 0

        try:
            self.mtAddress = self.configGetString("masterTableAddress")
        except Config.Exception:
            self.mtAddress = "http://www.webdc.eu/arclink"

        try:
            self.arclinkGroup = self.configGetString("arclinkGroup")
        except Config.Exception:
            self.arclinkGroup = "eida"
        
        return True

    def validateParameters(self):
        count = 0
        
        if self.commandline().hasOption("sanity-clean"):
            count = count + 1
            self.doSanityClean = True

        if self.commandline().hasOption("sanity-check"):
            count = count + 1
            self.doSanity = True

        if self.commandline().hasOption("check"):
            count = count + 1
            self.doCheck = True

        if self.commandline().hasOption("merge"):
            count = count + 1
            self.doMerge = True
        
        if self.commandline().hasOption("remove"):
            count = count + 1
            self.doRemove = True
        
        if self.commandline().hasOption("erase"):
            count = count + 1
            self.doErase = True
        
        if self.commandline().hasOption("list"):
            count = count + 1
            self.doList = True
        
        if self.commandline().hasOption("load-raw"):
            self.loadversion = self.commandline().optionString("load-raw")
        
        if self.commandline().hasOption("exclude"):
            nodeList = self.commandline().optionString("exclude")
            self.nodeExclude = nodeList.upper().split(",")
        
        if self.commandline().hasOption("nodes"):
            nodeList = self.commandline().optionString("nodes")
            self.nodeList = nodeList.upper().split(",")
        
        if self.commandline().hasOption("keep-synced"):
            self.keep = True
        
        if self.commandline().hasOption("with-sg"):
            self.stationGroup = True
        
        if self.commandline().hasOption("keep-raw"):
            self.keepraw = True

        if self.commandline().hasOption("force"):
            self.force = True

        if self.commandline().hasOption("dry-run"):
            self.dryrun = True

        ### checks of the parameters
        error = False
        
        if self.commandline().unrecognizedOptions():
            print >>sys.stderr,"Invalid options: ",
            for i in self.commandline().unrecognizedOptions():
                print i,
            print ""
            error = True
        
        if count != 1:
            print >> sys.stderr, "You have to choose one (and only one) operation mode."
            error = True
        
        if (self.keep or self.keepraw) and not self.doMerge:
            print >> sys.stderr, "Options --keep and --keepraw can only be used in the merge mode."
            error = True
        
        if (self.nodeList or self.nodeExclude) and (self.doSanity or self.doSanityClean):
            print >>sys.stderr, "Sanity check needs the complete master table loaded."
            error = True

        if (self.nodeList or self.nodeExclude) and self.doErase:
            print >>sys.stderr, "Erase cannot be guided by a master table."
            error = True

        if error: return False
        
        if self.doErase or (self.doRemove and (self.nodeList is None and self.nodeExclude is None)):
            if self.doErase:
                print >>sys.stderr,"You are about to do a complete ERASING on your machine inventory/routing tables"
            else:
                print >>sys.stderr,"You are about to do a complete REMOVING on your machine inventory/routing tables"
            print >>sys.stderr,"Enter to go / Ctrl-C to abort !"
            ans = sys.stdin.readline()
        
        return True

    def createCommandLineDescription(self):
        Client.Application.createCommandLineDescription(self)

        self.commandline().addGroup("Mode of Operation")
        self.commandline().addOption("Mode of Operation", "check", "Check my own inventory against the Master Table (the list of nodes affected can be controlled by the --nodes/--exclude options).")
        self.commandline().addOption("Mode of Operation", "sanity-check", "Check the sanity of the inventory loaded on this node finding possible overlaps with the Master Table.")
        self.commandline().addOption("Mode of Operation", "sanity-clean", "Remove the conflicting elements loaded on my host pointed out by the sanity command.")
        self.commandline().addOption("Mode of Operation", "merge", "Performs a merging (one-direction synchronization) of inventory and routing based on the content of the Master Table (the list of nodes affected can be controlled by the --nodes/--exclude options).")
        self.commandline().addOption("Mode of Operation", "remove", "Remove (undo the merge) inventory/routing information that was merged (the list of nodes affected can be controlled by the --nodes/--exclude options).")
        self.commandline().addOption("Mode of Operation", "erase", "Erase my inventory/routing entries restoring the local seiscomp3 database to an empty state (use with caution).")
        self.commandline().addOption("Mode of Operation", "list", "Fetch and list the Master Table content.")

        self.commandline().addGroup("Options")
        self.commandline().addStringOption("Options", "nodes", "Comma separated list of nodes to remove, merge, check.")
        self.commandline().addStringOption("Options", "exclude", "Comma separated list of nodes to exclude from remove, merge, check.")
        self.commandline().addOption("Options", "keep-synced", "keep a copy of the Inventory and Routing tables merged.")
        self.commandline().addOption("Options", "keep-raw", "keep a copy of the RAW Inventory and Routing tables fetched.")
        self.commandline().addStringOption("Options", "load-raw", "load a copy of the RAW Inventory/Routing information from disk. It expects a date value in the following format YYYY-MM-DD.")
        self.commandline().addOption("Options", "force", "Force merging against MYSELF.")
        self.commandline().addOption("Options", "dry-run", "Perform everything that should be done but don't write changes to the database.")
        self.commandline().addOption("Options", "with-sg", "Merge also the Station Groups (Virtual Networks) after the inventory is merged.")

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

        return mcount

    def _loadLocalInventory(self):
        logs.notice("** load local inventory")
        
        inv = SC3Inventory(self.query().loadInventory())
        inv.load_stations("*")
        inv.load_stations("*", "*")
        inv.load_stations("*", "*", "*")
        inv.load_stations("*", "*", "*", "*")
        inv.load_instruments()
        
        return inv

    def _loadLocalRouting(self):
        logs.notice("** load local routing")
        rtn = SC3Routing(self.query().loadRouting())
        rtn.load_routes("*", "*")
        return rtn

    def _loadMasterTable(self):
        logs.notice("** loading master table")
        table = {}
        url = self.mtAddress + "/table?group=" + self.arclinkGroup

        try:
            r = urlopen(url)
        except HTTPError, e:
            raise Exception("Invalid master table location %s [HTTP Error code %s]" % (url, e.code))
        except URLError, e:
            (code, reason) = e.reason
            raise Exception("Error on fetching master table: %s" % (reason)) 
        xdoc = parseString(r.read())

        for node in xdoc.getElementsByTagName("node"):
            id = str(node.getAttribute("dcid"))
            
            if self.nodeList and id.upper() not in self.nodeList:
                continue
            if self.nodeExclude and id.upper() in self.nodeExclude:
                continue
            if str(id) in table:
                raise Exception("Invalid XML, node is duplicated")

            ad = str(node.getAttribute("address"))
            pt = int(node.getAttribute("port"))
            
            n = SyncNode(id, ad, pt, self.poolFolder)
            n.name = str(node.getAttribute("name"))
            n.email = str(node.getAttribute("email"))
            n.contact = str(node.getAttribute("contact"))

            for network in node.getElementsByTagName("network"):
                code = str(network.getAttribute("code"))
                start = datetime.datetime.strptime(network.getAttribute("start"), "%Y-%m-%d %H:%M:%S")
                end = network.getAttribute("end")
                if end == "":
                    end = None
                else:
                    end = datetime.datetime.strptime(end, "%Y-%m-%d %H:%M:%S")
                n.addNetwork(code, start, end)

            for stationGroup in node.getElementsByTagName("stationGroup"):
                code = str(stationGroup.getAttribute("code"))
                start = datetime.datetime.strptime(stationGroup.getAttribute("start"), "%Y-%m-%d %H:%M:%S")
                end = stationGroup.getAttribute("end")
                if end == "":
                    end = None
                else:
                    end = datetime.datetime.strptime(end, "%Y-%m-%d %H:%M:%S")
                n.addStationGroup(code, start, end)

            table[id] = n
        return table

    def _routingClean(self, node, referenceRoute):
        rtn = referenceRoute
        ## Cleaning
        logs.debug("Routing clean-up")
        aCollection = []
        sCollection = []
        for (network, stations) in self.rtn.route.items():
            if network not in node.networks: continue
            for (station, locations) in stations.items():
                for (location, streams) in locations.items():
                    for (stream, route) in streams.items():
                        ## Arclink
                        if route.arclink:
                            for (address, spams) in route.arclink.items():
                                for (start, aroute) in spams.items():
                                    ## Arclink has start & end times so we check against node object to see if it fits
                                    if not node.checkSpam(network, aroute): continue
                                    try:
                                        rtn.route[network][station][location][stream].arclink[address][start]
                                        logs.debug("A [=] ArcLink %s.%s.%s.%s / %s / %s" % (network, station, location, stream, address, start))
                                    except:
                                        logs.warning("A [-] ArcLink %s.%s.%s.%s / %s / %s" % (network, station, location, stream, address, start))
                                        ## Collect routes to delete
                                        aCollection.append((network, station, location, stream, address, start))
                        ## Seedlink
                        if route.seedlink:
                            for (address, sroute) in route.seedlink.items():
                                try:
                                    rtn.route[network][station][location][stream].seedlink[address]
                                    logs.debug("S [=] SeedLink %s.%s.%s.%s / %s" % (network, station, location, stream, address))
                                except:
                                    logs.warning("S [-] SeedLink %s.%s.%s.%s / %s" % (network, station, location, stream, address))
                                    ## Collect routes to delete
                                    sCollection.append((network, station, location, stream, address))
        
        ## Real Delete Arclink
        for (n, s, l, c, a, st) in aCollection:
            self.rtn.route[n][s][l][c].remove_arclink(a,st)
        
        ## Real Delete Seedlink
        for (n, s, l, c, a) in sCollection:
            self.rtn.route[n][s][l][c].remove_seedlink(a)
        return

    def _mRouting(self, node):
        if node is None or not isinstance(node, SyncNode):
            raise Exception("Invalid node object")
        
        if node.dcid == self.dcid:
            if self.force:
                logs.warning("Performing a routing merge against myself")
            else:
                logs.warning("Will not merge route against myself (--force to overwrite this)")
                return
        
        logs.notice("** merging routing from %s (%s:%d)" % (node.dcid, node.address, node.port))
        
        rtn = node.getRouting(self.dcid, self.keepraw, self.loadversion)
        if rtn is None:
            logs.error("Could not fetch routing from node: %s" % node.dcid)
            return
        
        filename = "Routing-%s-SYNC.xml" % (node.dcid)
        rtn.save_xml(filename)

        # Pre Clean up
        self._routingClean(node, rtn)
        
        ## Merging
        logs.debug("Merging routing")
        self.rtn.load_xml(filename)
        if not self.keep:
            os.unlink(filename)
        else:
            logs.warning("Keeping synced routing in %s" % filename)
        
        return

    def _inventoryClean(self, node, referenceInventory):
        inv = referenceInventory
        logs.debug("Merging clean-up")      
        for (ncode, nspam) in self.inv.network.items():
            for (nstart, network) in nspam.items():
                if network.archive != node.dcid:
                    continue
                if not network.shared:
                    continue
                try:
                    nref = inv.network[ncode][nstart]
                    logs.debug("I [=] %s %d" % (ncode, nstart.year))
                except Exception:
                    logs.warning("I [-] %s %d" % (ncode, nstart.year))
                    self.inv.remove_network(ncode, nstart)
                    continue

                for (scode, sspam) in network.station.items():
                    for (sstart, station) in sspam.items():
                        try:
                            sref = nref.station[scode][sstart]
                            logs.debug("I [=] %s %d %s %d" % (ncode, nstart.year, scode, sstart.year))
                        except Exception:
                            logs.warning("I [-] %s %d %s %d" % (ncode, nstart.year, scode, sstart.year))
                            network.remove_station(scode, sstart)
                            continue

                        for (lcode, lspam) in station.sensorLocation.items():
                            for (lstart, location) in lspam.items():
                                try:
                                    lref = sref.sensorLocation[lcode][lstart]
                                    logs.debug("I [=] %s %d %s %d %s %d" % (ncode, nstart.year, scode, sstart.year, lcode, lstart.year))
                                except Exception:
                                    logs.warning("I [-] %s %d %s %d %s %d" % (ncode, nstart.year, scode, sstart.year, lcode, lstart.year))
                                    station.remove_sensorLocation(lcode, lstart)
                                    continue

                                for (ccode, cspam) in location.stream.items():
                                    for (cstart, channel) in cspam.items():
                                        try:
                                            cref = lref.stream[ccode][cstart]
                                            logs.debug("I [=] %s %d %s %d %s %d %s %d" % (ncode, nstart.year, scode, sstart.year, lcode, lstart.year, ccode, cstart.year))
                                        except Exception:
                                            logs.warning("I [-] %s %d %s %d %s %d %s %d" % (ncode, nstart.year, scode, sstart.year, lcode, lstart.year, ccode, cstart.year))
                                            location.remove_stream(ccode, cstart)
                                            continue

        for (ncode, nspam) in inv.network.items():
            for (nstart, network) in nspam.items():
                try:
                    nref = self.inv.network[ncode][nstart]
                    if nref.archive != network.archive: raise Exception("")
                except Exception:
                    logs.warning("I [+] %s %d" % (ncode, nstart.year))
                    continue

                for (scode, sspam) in network.station.items():
                    for (sstart, station) in sspam.items():
                        try:
                            sref = nref.station[scode][sstart]
                        except Exception:
                            logs.warning("I [+] %s %d %s %d" % (ncode, nstart.year, scode, sstart.year))
                            continue

                        for (lcode, lspam) in station.sensorLocation.items():
                            for (lstart, location) in lspam.items():
                                try:
                                    lref = sref.sensorLocation[lcode][lstart]
                                except Exception:
                                    logs.warning("I [+] %s %d %s %d %s %d" % (ncode, nstart.year, scode, sstart.year, lcode, lstart.year))
                                    continue

                                for (ccode, cspam) in location.stream.items():
                                    for (cstart, channel) in cspam.items():
                                        try:
                                            cref = lref.stream[ccode][cstart]
                                        except Exception:
                                            logs.warning("I [+] %s %d %s %d %s %d %s %d" % (ncode, nstart.year, scode, sstart.year, lcode, lstart.year, ccode, cstart.year))
                                            continue

        return

    def _mInventory(self, node):
        if node is None or not isinstance(node, SyncNode):
            raise Exception("Invalid node object")
        
        if node.dcid == self.dcid:
            if self.force:
                logs.warning("Performing an inventory merge against myself")
            else:
                logs.warning("Will not merge inventory against myself (--force to overwrite this)")
                return
        
        logs.notice("** merging inventory from %s (%s:%d)" % (node.dcid, node.address, node.port))
        
        inv = node.getInventory(self.dcid, self.keepraw, self.loadversion)
        if inv is None:
            logs.error("Could not fetch inventory from node: %s" % node.dcid)
            return
        
        filename = "Inventory-%s-SYNC.xml" % (node.dcid)
        inv.save_xml(filename, True)
        
        # Pre-Cleanup
        self._inventoryClean(node, inv)

        ## Merging
        logs.debug("Merging inventory")
        self.inv.load_xml(filename)
        if not self.keep:
            os.unlink(filename)
        else:
            logs.warning("Keeping synced inventory in %s" % filename)

        return

    def _fullClean(self):
        inv = self.inv
        rtn = self.rtn

        if inv:
            for (ncode, nspam) in inv.network.items():
                for (nstart, network) in nspam.items():
                    inv.remove_network(ncode, nstart)
                    logs.notice("I [-] %s %s" % (ncode,nstart))
            for (sgcode, sg) in inv.stationGroup.items():
                inv.remove_stationGroup(sgcode)
                logs.notice("Sg [-] %s" % (sgcode))
        else:
            logs.warning("No inventory to clean")

        if rtn:
            aCollection = []
            sCollection = []
            for (network, stations) in rtn.route.items():
                for (station, locations) in stations.items():
                    for (location, streams) in locations.items():
                        for (stream, route) in streams.items():
                            ## Arclink
                            if route.arclink:
                                for (address, spams) in route.arclink.items():
                                    for (start, aroute) in spams.items():
                                        ## Arclink has start & end times so we check against node object to see if it fits
                                        aCollection.append((network, station, location, stream, address, start))
                            ## Seedlink
                            if route.seedlink:
                                for (address, sroute) in route.seedlink.items():
                                    sCollection.append((network, station, location, stream, address))
    
            ## Real Delete Arclink
            for (n, s, l, c, a, st) in aCollection:
                self.rtn.route[n][s][l][c].remove_arclink(a,st)
                logs.notice("I [-] ArcLink %s.%s.%s.%s / %s / %s" % (n, s, l, s, a, st))
    
            ## Real Delete Seedlink
            for (n, s, l, c, a) in sCollection:
                self.rtn.route[n][s][l][c].remove_seedlink(a)
                logs.notice("I [-] SeedLink %s.%s.%s.%s / %s" % (n, s, l, s, a))
        else:
            logs.warning("No routing to clean")

        return

    def _clean(self, node):
        inv = self.inv
        rtn = self.rtn

        if node.dcid == self.dcid:
            if self.force:
                logs.warning("Cleaning my own inventory/routing")
            else:
                logs.warning("Will not clean own inventory/routing (--force to overwrite this)")
                return        
        if inv:
            logs.notice("** cleaning: %s" % node.dcid)
            for (ncode, nstart, network) in unWrapNSLC(inv.network, node.dcid, True):
                if not node.checkSpam(network.code, network):
                    logs.warning("I [=] %s %s is not know by the master table" % (ncode,nstart))
                    continue
                inv.remove_network(ncode, nstart)
                logs.notice("I [-] %s %s" % (ncode,nstart))
            if inv.stationGroup:
                for (sgcode,start,end) in node.stationGroupList():
                    if sgcode in inv.stationGroup:
                        inv.remove_stationGroup(sgcode)
                        logs.notice("Sg [-] %s" % (sgcode))
            else:
                logs.warning("** local machine has no station groups loaded")
        else:
            logs.warning("** local machine has no inventory loaded")

        if rtn:
            if not node.networks:
                logs.warning("** cleaning warning, node %s has no networks, cannot clean ROUTING" % node.dcid)
                return

            aCollection = []
            sCollection = []
            for (network, stations) in rtn.route.items():
                if network not in node.networks: continue
                for (station, locations) in stations.items():
                    for (location, streams) in locations.items():
                        for (stream, route) in streams.items():
                            ## Arclink
                            if route.arclink:
                                for (address, spams) in route.arclink.items():
                                    for (start, aroute) in spams.items():
                                        ## Arclink has start & end times so we check against node object to see if it fits
                                        if not node.checkSpam(network, aroute): continue
                                        aCollection.append((network, station, location, stream, address, start))
                            ## Seedlink
                            if route.seedlink:
                                for (address, sroute) in route.seedlink.items():
                                    ## BUG: removes what it need to check
                                    sCollection.append((network, station, location, stream, address))

            ## Real Delete Arclink
            for (n, s, l, c, a, st) in aCollection:
                logs.notice("A [-] ArcLink %s.%s.%s.%s / %s / %s" % (n, s, l, c, a, st))
                self.rtn.route[n][s][l][c].remove_arclink(a,st)

            ## Real Delete Seedlink
            for (n, s, l, c, a) in sCollection:
                logs.notice("S [-] SeedLink %s.%s.%s.%s / %s" % (n, s, l, c, a))
                self.rtn.route[n][s][l][c].remove_seedlink(a)
        else:
            logs.warning("** local machine has no routing loaded")

    def _cleanInstruments(self):
        if self.inv is None:
            return
        
        inv = self.inv
        inst = []
        
        ## Collect instruments public Ids in use
        for (ncode, nspam) in inv.network.items():
            for (nstart, network) in nspam.items():
                for (scode, sspam) in network.station.items():
                    for (sstart, station) in sspam.items():
                        for (lcode, lspam) in station.sensorLocation.items():
                            for (lstart, location) in lspam.items():
                                for (ccode, cspam) in location.stream.items():
                                    for (cstart, channel) in cspam.items():
                                        inst.append(channel.sensor)
                                        inst.append(channel.datalogger)

        ## Clean Sensor & Collect used publicIDs
        sref = []
        for (name, sensor) in inv.sensor.items():
            if sensor.publicID in inst:
                sref.append(sensor.response)
                continue
            logs.debug("[-] Sensor %s" % sensor.name)
            inv.remove_sensor(name)
        
        ## Clean Datalogger & Collect used publicIDs
        dref = []
        for (name,datalogger) in inv.datalogger.items():
            if datalogger.publicID in inst:
                for (srn, lsrn) in datalogger.decimation.items():
                    for (srd, decimation) in lsrn.items():
                        if decimation.analogueFilterChain:
                            dref.extend(decimation.analogueFilterChain.split(" "))
                        if decimation.digitalFilterChain:
                            dref.extend(decimation.digitalFilterChain.split(" "))
                continue
            logs.debug("[-] Datalogger %s" % datalogger.name)
            inv.remove_datalogger(name)

        ## Clean Paz
        for (name, rpaz) in inv.responsePAZ.items():
            if rpaz.publicID in sref:
                continue
            if rpaz.publicID in dref:
                continue
            logs.debug("[-] PAZ %s" % rpaz.name)
            inv.remove_responsePAZ(name)
        
        ## Clean Polynomial
        for (name, rpol) in inv.responsePolynomial.items():
            if rpol.publicID in sref:
                continue
            logs.debug("[-] Polynomial %s" % rpol.name)
            inv.remove_responsePolynomial(name)

        ## Clean Fir
        for (name, rfir) in inv.responseFIR.items():
            if rfir.publicID in dref:
                continue
            logs.debug("[-] FIR %s" % rfir.name)
            inv.remove_responseFIR(name)
        
        ## Clean Iir
        for (name, riir) in inv.responseIIR.items():
            if riir.publicID in sref:
                continue
            if riir.publicID in dref:
                continue
            logs.debug("[-] IIR %s" % riir.name)
            inv.remove_responseIIR(name)

        ## Todo
        ## Clean AuxSource
        ## Clean AuxDevice
        return

    def _cleanRoutings(self):
        ## After clean up check that we don't have any empty route entries
        collection = []
        for (network, stations) in self.rtn.route.items():
            for (station, locations) in stations.items():
                for (location, streams) in locations.items():
                    for (stream, route) in streams.items():
                        if len(route.arclink) == 0 and len(route.seedlink) == 0:
                            collection.append((network,station,location,stream))
                            

        for (n,s,l,c) in collection:
            logs.debug("R [-] Empty Route %s.%s.%s.%s" % (n,s,l,c))
            self.rtn.remove_route(n,s,l,c)
        return

    def _checkInventory(self, inv, node):
        ok = True
        logs.notice("** Checking Inventory for dcid = %s" % node.dcid)
        logs.notice("   [-] is missing on local inventory")
        logs.notice("   [+] is missing on master table for node %s" % node.dcid)
        logs.notice("   [=] is defined on master table and local inventory")
        
        ## Load a local list of networks
        ##
        list = unWrapNSLC(inv.network, node.dcid, True)
        
        ## Loop the master table and find the local entries if any
        ##
        for (n, s, e) in node.networkList():
            toremove = []
            found = False
            for (ncode, nstart, network) in list:
                if n == ncode and s == nstart:
                    toremove.append((ncode, nstart, network))
                    logs.notice("[=] %s,%d match the master table" % (ncode, nstart.year))
                    found = True
                    break
            if not found:
                logs.warning("[-] %s,%s" % (n, s))
                ok = False
            for (ncode, nstart, network) in toremove:
                list.remove((ncode, nstart, network))
            
        ## If there is entries on the list those are extra-defined
        ##
        for (n,s,ne) in list:
            logs.warning("[+] %s,%s" % (n, s))
            ok = False
        
        ### Check that we have the routings for networks
        ##
        logs.notice("** Checking Routing for dcid = %s" % node.dcid)
        logs.notice("   [-] is missing on local routing")
        logs.notice("   [=] is defined on master table and local inventory")
        for (nCode, nStart, nEnd) in node.networkList():
            ##
            ## Arclink
            found = False
            for (n, s, l, c, ad, st, arclink) in unWrapRTN(self.rtn, nCode, 'arclink'):
                if isInside(nStart, nEnd, st, arclink.end):
                    logs.notice("[=] [Arclink] %s.%s.%s.%s [%s] %s" % (n, s, l, c, arclink.priority, ad))
                    found = True
            if not found:
                logs.warning("[-] [Arclink] %s,%s" % (nCode, nStart))
                ok = False
            ##
            ## Seedlink
            if not nEnd:
                found = False
                for (n, s, l, c, ad, seedlink) in unWrapRTN(self.rtn, nCode, 'seedlink'):
                    found = True
                    if found:
                        logs.notice("[=] [Seedlink] %s.%s.%s.%s [%s] %s" % (n, s, l, c, seedlink.priority, ad))
                if not found:
                    logs.warning("[-] [Seedlink] %s.%s" % (nCode, nStart))
                    ok = False
                
        ### Check that we have the routings for networks
        ##
        if len(node.stationGroupList()) > 0:
            logs.notice("** Checking Virtual network for dcid = %s" % node.dcid)
            logs.notice("   [-] is missing on local inventory")
            logs.notice("   [=] is defined on master table and local inventory")
            for (sgroup, s, e) in node.stationGroupList():
                found = False
                for (sgcode, sg) in inv.stationGroup.items():
                    ##
                    ## Missing check of dates
                    ##
                    if sgcode == sgroup:
                        logs.notice("[=] [StationGroup] %s,%s" % (sgroup, s))
                        found=True
                        break
                if not found:
                    logs.warning("[-] [StationGroup] %s,%s" % (sgroup, s))
                    ok = False

        if ok:
            logs.notice("No Warnings found.")
        return ok

    def _sanityCheck(self, inv, nodes, clean = False):
        issues = False
        
        logs.notice("** Performing a sanity check")
        nCollection = [] 
        aCollection = []
        sCollection = []
        for (ncode, nstart, net) in unWrapNSLC(inv.network):

            # Check that we know the archive
            if net.archive not in nodes:
                if net.shared == True:
                    logs.warning("Network %s,%d archive (%s) not listed in node list." % (ncode, nstart.year, net.archive))
                    nCollection.append((ncode, nstart))
                    issues = True
                else:
                    logs.debug("Network %s,%d archive (%s) not listed in node list (but ... shared is false)." % (ncode, nstart.year, net.archive))
            else:
                # And it is valid
                if not nodes[net.archive].checkSpam(ncode, net):
                    if net.shared == True:
                        logs.warning("Network %s,%d listed as %s but it does not fit this node." % (ncode, nstart.year, net.archive))
                        nCollection.append((ncode, nstart))
                        issues = True                        
                    else:
                        logs.debug("Network %s,%d listed as %s but it does not fit this node (but ... shared is false)." % (ncode, nstart.year, net.archive))

            for (archive, node) in nodes.items():
                ## Check if net fits this node
                if node.checkSpam(ncode, net):
                    ## if it fits the archive should match
                    if node.dcid != net.archive:
                        logs.warning("Network %s,%d belongs node %s not %s ?" % (ncode, nstart.year, archive, net.archive))
                else:
                    ## if it does not fit check if it overlaps
                    if node.overlaps(ncode, net):
                        logs.warning("Network %s,%d overlaps network defined for node %s" % (ncode, nstart.year, archive))
                        issues = True

        for (network, stations) in self.rtn.route.items():
            for (station, locations) in stations.items():
                for (location, streams) in locations.items():
                    for (stream, route) in streams.items():
                        if route.arclink:
                            if route.seedlink:
                                for (address, sroute) in route.seedlink.items():
                                    try:
                                        if not network: raise Exception("")
                                        found = False
                                        for (net) in inv.network[network].values():
                                            if not net.end and (net.code, net.start) not in nCollection: 
                                                found = True
                                        if not found: raise Exception("")
                                    except Exception,e:
                                        issues = True
                                        logs.warning("Seedlink Routing %s.%s.%s.%s / %s does not match a any loaded valid network" % (network, station, location, stream, address))
                                        sCollection.append((network, station, location, stream, address))

                            for (address, spams) in route.arclink.items():
                                for (start, aroute) in spams.items():
                                    try:
                                        if not network: raise Exception("") 
                                        found = False
                                        for (net) in inv.network[network].values():
                                            if isInside(net.start, net.end, aroute.start, aroute.end) and (net.code, net.start) not in nCollection:
                                                found = True
                                        if not found: raise Exception("")
                                    except Exception,e:
                                        issues = True
                                        logs.warning("Arclink Routing %s.%s.%s.%s / %s / %s does not match any loaded valid network" % (network, station, location, stream, address, start))
                                        aCollection.append((network, station, location, stream, address, start))

        if clean and (nCollection or aCollection or sCollection):
            logs.notice("** Sanity Clean")

            self.sendOnDone = True
            
            for (ncode, nstart) in nCollection:
                logs.notice("[-] %s,%d cleaning network" % (ncode, nstart.year))
                self.inv.remove_network(ncode, nstart)

            for (n, s, l, c, a) in sCollection:
                logs.notice("S [-] SeedLink %s.%s.%s.%s / %s" % (n, s, l, c, a))
                self.rtn.route[n][s][l][c].remove_seedlink(a)
                
            for (n, s, l, c, a, st) in aCollection:
                logs.notice("A [-] ArcLink %s.%s.%s.%s / %s / %s" % (n, s, l, c, a, st))
                self.rtn.route[n][s][l][c].remove_arclink(a,st)

        if not issues:
            logs.notice("No warnings found")

    def _restartReqHandler(self):
        logs.notice("** Re-start ArcLink request handlers")
        
        user = os.getenv("USER")
        cmdl = "ps -o pid,command -u " + user + " | grep reqhandler | grep -v grep"
        count = 0
        for line in os.popen(cmdl):
            (pid, command) = line.strip().split(" ", 1)

            try:
                # Make sure that number is int
                int(pid)
            except:
                pid = ""
                pass

            if pid:
                count = count + 1
                logs.notice("Re-starting PID = %s" % pid)
                logs.debug (" Command: %s" % command)
                os.popen("kill " + pid)

        if count == 0:
            logs.notice("No req-handlers found for re-starting.")

    def _doGroupRebuild(self, node):
        if node is None or not isinstance(node, SyncNode):
            raise Exception("Invalid node object")
        
        if node.dcid == self.dcid:
            if self.force:
                logs.warning("Performing a station group rebuild against myself")
            else:
                logs.warning("Will not rebuild station group against myself (--force to overwrite this)")
                return

        logs.notice("** rebuilding station group from %s (%s:%d)" % (node.dcid, node.address, node.port))

        if len(node.stationGroupList()) == 0:
            logs.debug ("Nothing to do, no station groups defined for node %s")
            return

        sginv = node.getStationGroup(self.dcid)
        if sginv is None:
            logs.error("Could not fetch stationGroups from node: %s" % node.dcid)
            return
        sgmap = prepareMap(sginv)

        inv = self.inv

        for (sgcode, sgstart, sgend) in node.stationGroupList():
            # Check the reference
            try:
                sgref = sginv.stationGroup[sgcode]
            except KeyError:
                if sgcode in inv.stationGroup:
                    inv.remove_stationGroup(sgcode)
                    logs.warning("Sg [-] %s" % sgcode)
                continue

            # Check the current
            try:
                sg = inv.stationGroup[sgcode]
                logs.debug("Sg [=] %s" % sgcode)
            except KeyError:
                sg = inv.insert_stationGroup(sgcode)
                logs.warning("Sg [+] %s" % sgcode)

            # Update attributes
            sg.start = sgref.start
            sg.end = sgref.end
            sg.type = sgref.type
            sg.description = sgref.description
            sg.latitude = sgref.latitude
            sg.longitude = sgref.longitude
            sg.elevation = sgref.elevation

            # Make a map and sanitize the stationGroup so that we can sync
            sidToRemove = []
            invmap = prepareMap(inv)

            for sid in sg.stationReference:
                if sid not in invmap:
                    logs.warning("Sg [!] cleaning missing reference for public id %s" % sid)
                    sidToRemove.append(sid)
            for sid in sidToRemove:
                sg.remove_stationReference(sid)

            # Remove not listed references, and clean conflict ones
            sidToRemove = []
            for sid in sg.stationReference:
                try:
                    srref = sgref.stationReference[sid]
                    if invmap[sid] != sgmap[sid]:
                        logs.warning("Sg [!] %s key conflict error (%s/%s/%s/%s)" % ((sgcode,)+invmap[sid]))
                        raise KeyError()
                except KeyError:
                    sidToRemove.append(sid)
            for sid in sidToRemove:
                sg.remove_stationReference(sid)
                logs.warning("Sg [-] %s %s/%s/%s/%s" % ((sgcode,)+invmap[sid]))

            # Add the missing or removed entries again when they are valid
            for sid in sgref.stationReference:
                try:
                    sr = sg.stationReference[sid]
                    logs.debug("Sg [=] %s %s/%s/%s/%s" % ((sgcode,) + sgmap[sid]))
                except KeyError:
                    newSID = findSID(invmap, sgmap[sid])
                    if newSID is None:
                        logs.warning("Sg [!] %s Skipping station reference %s/%s/%s/%s" % ((sgcode,)+sgmap[sid]))
                        continue
                    if newSID != sid: 
                        logs.warning("Sg  [!] Linking to a different publicID %s -> %s" % (sid, newSID))
                    sg.insert_stationReference(newSID)
                    logs.warning("Sg [+] %s %s %s %s %s" % ((sgcode,)+invmap[newSID]))

        if self.keep:
            tmpinv = GInventory()
            tmpinv.stationGroup = inv.stationGroup
            tmpinv.save_xml("StationGroup-SYNC.xml", False)

    def run(self):
        sc3wrap.dbQuery = self.query()
        table = None

        try:
            self.dcid = self.configGetString("datacenterID")
        except Config.Exception:
            logs.error("Cannot find local datacenter ID (dcid)")
            return False
        logs.notice("** Running on DataCenter: %s" % self.dcid)
        logs.notice("** Sync_arc version: %s" % VERSION)
        
        if not self.doErase:
            try:
                table = self._loadMasterTable()
            except Exception, e:
                logs.error(str(e))
                return False
        
        if not self.doList:
            try:
                self.inv = self._loadLocalInventory()
            except Exception,e:
                logs.error(str(e))
                return False
            
            try:
                self.rtn = self._loadLocalRouting()
            except Exception:
                logs.error("Cannot load local routing")
                return False
        
        ## Process the request
        if self.doCheck:
            for node in table.values():
                self._checkInventory(self.inv, node)
        
        elif self.doList:
            for node in table.values():
                node.info()

        elif self.doSanityClean:
            self._sanityCheck(self.inv, table, True)

        elif self.doSanity:
            self._sanityCheck(self.inv, table, False)
        
        elif self.doErase:
            self._fullClean()
            self.sendOnDone = True

        elif self.doRemove:
            for node in table.values():
                self._clean(node)
            self.sendOnDone = True

        elif self.doMerge:
            for (dcid, node) in table.items():
                try:
                    self._mInventory(node)
                    self._mRouting(node)
                except Exception,e:
                    logs.error("[%s] %s" % (dcid, e))
            if self.stationGroup:
                for (dcid, node) in table.items():
                    self._doGroupRebuild(node)
            self.sendOnDone = True
            self.tidyOnDone = True

        else:
            return False
        
        return True

    def tidyPool(self):
        if self.poolKeep == 0: return

        logs.notice("** Performing a poolFolder cleaning")
        logs.notice("   Keeping %s files per node" % self.poolKeep)
        
        files = []
        keysKeep = []
        
        for file in glob.glob(self.poolFolder + "/Inventory-*-*-RAW.xml"):
            try:
                aux = file.split("-")
                id = int(aux[2])
                files.append((id, file))
                if id not in keysKeep:
                    keysKeep.append(id)
            except Exception,e:
                logs.warning("Invalid file %s" % file)

        for file in glob.glob(self.poolFolder + "/Routing-*-*-RAW.xml"):
            try:
                aux = file.split("-")
                id = int(aux[2])
                files.append((id, file))
                if id not in keysKeep:
                    logs.warning("File %s has not Inventory associated, and will be removed." % file)
            except Exception,e:
                logs.warning("Invalid file %s" % file)


        keysKeep.sort()
        keysKeep =  keysKeep[-self.poolKeep:]

        for (id, file) in files:
            if id in keysKeep: continue
            try:
                logs.debug("Removing file %s" % file)
                os.unlink(file)
            except Exception,e:
                logs.warning("Error removing file %s" % file)
        return

    def done(self):
        if not self.dryrun:
            if self.sendOnDone:
                sent = 0
                if self.inv is not None: 
                    logs.notice("** cleaning Instruments")
                    self._cleanInstruments()
                    
                    logs.notice("** sending Inventory Notifiers")
                    self.inv.flush()
                    sent = sent + self.send_notifiers("INVENTORY")
        
                if self.rtn is not None:
                    logs.notice("** cleaning Routings")
                    self._cleanRoutings()
                    
                    logs.notice("** sending Routing Notifiers")
                    self.rtn.flush()
                    sent = sent + self.send_notifiers("ROUTING")
    
                if sent > 0:
                    self._restartReqHandler()
        else:
            logs.warning("*** Will not send notifiers DRY RUN *** ")

        if self.tidyOnDone:
            self.tidyPool()

        return Client.Application.done(self)

if __name__ == "__main__":
    logs.debug = Logging.debug
    logs.info = Logging.info
    logs.notice = Logging.notice
    logs.warning = Logging.warning
    logs.error = Logging.error
    app = ArclinkSynchronizer(len(sys.argv), sys.argv)
    sys.exit(app())
