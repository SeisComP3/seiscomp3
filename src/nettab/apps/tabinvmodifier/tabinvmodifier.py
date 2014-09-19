#!/usr/bin/env python

import os
import sys
import datetime, time
from seiscomp3 import DataModel, IO, Client, Core, Logging
from nettab.lineType import Nw, Sa, Na, Ia
from nettab.basesc3 import sc3

class Rules(object):
    def __init__(self, relaxed = False):
        self.relaxed = relaxed
        self.attributes = {}
        self.iattributes = []
        return

    @staticmethod
    def _overlaps(pstart, pend, cstart, cend):
        if pend:
            if pend > cstart:
                if not cend or pstart < cend:
                    return True
        else:
            if not cend or pstart < cend:
                return True
        return False

    def Nw(self, nw):
        key = (nw.code, nw.start, nw.end)
        if key in self.attributes:
            raise Exception("Nw (%s/%s-%s) is already defined." % key)
        self.attributes[key] = {}
        self.attributes[key]["Sa"] = []
        self.attributes[key]["Na"] = []
        return key

    def Sa(self, key, sa):
        try:
            items = self.attributes[key]["Sa"]
        except KeyError:
            raise Exception ("Nw %s/%s-%s not found in Ruleset" % key)
        items.append(sa)

    def Na(self, key, na):
        try:
            items = self.attributes[key]["Na"]
        except KeyError:
            raise Exception ("Nw %s/%s-%s not found in Ruleset" % key)
        items.append(na)

    def Ia(self, ia):
        self.iattributes.append(ia);

    def findKey(self, ncode, nstart, nend):
        for (code, start, end) in self.attributes:
            if code == ncode and self._overlaps(start, end, nstart, nend):
                return (code, start, end)
        return None

    def getInstrumentsAttributes(self, elementId, elementType):
        att = {}
        for item in self.iattributes:
            if item.match(elementId, elementType):
                att[item.Key] = item.Value
        return att

    def getNetworkAttributes(self, key):
        att = {}
        for item in self.attributes[key]["Na"]:
            att[item.Key] = item.Value 
        return att

    def getStationAttributes(self, key, ncode, scode, lcode, ccode, start, end):
        att = {}
        for item in self.attributes[key]["Sa"]:
            if item.match(scode, lcode, ccode, start, end, self.relaxed):
                att[item.Key] = item.Value 
        return att

class InventoryModifier(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)
        self.setMessagingUsername("iModify")

        self.rules = None
        self.relaxed = False
        self.outputFile = None

    def _digest(self, tabFilename, rules = None):
        if not tabFilename or not os.path.isfile(tabFilename):
            raise Exception("Supplied filename is invalid.")
        
        if not rules:
            rules = Rules(self.relaxed)
    
        try:
            fd = open(tabFilename)
            for line in fd:
                obj = None
                line = line.strip()
                if not line or line[0] == "#": continue
                if str(line).find(":") == -1:
                    raise Exception("Invalid line format '%s'" % line)
                (Type, Content) = line.split(":",1)
    
                if Type == "Nw":
                    nw = Nw(Content)
                    key = rules.Nw(nw)
                elif Type == "Sg":
                    raise Exception("Type not supported.")
                elif Type == "Na":
                    na = Na(Content)
                    rules.Na(key, na)
                elif Type == "Sa":
                    sa = Sa(Content)
                    rules.Sa(key, sa)
                elif Type == "Sr":
                    raise Exception("Type not supported.")
                elif Type == "Ia":
                    ia = Ia(Content)
                    rules.Ia(ia)
                elif Type == "Se":
                    raise Exception("Type not supported.")
                elif Type == "Dl":
                    raise Exception("Type not supported.")
                elif Type == "Cl":
                    raise Exception("Type not supported.")
                elif Type == "Ff":
                    raise Exception("Type not supported.")
                elif Type == "If":
                    raise Exception("Type not supported.")
                elif Type == "Pz":
                    raise Exception("Type not supported.")
        except Exception,e:
            raise e
    
        finally:
            if fd:
                fd.close()
        return rules

    def validateParameters(self):
        outputFile = None
        rulesFile  = None

        if self.commandline().hasOption("rules"):
            rulesFile = self.commandline().optionString("rules")

        if self.commandline().hasOption("output"):
            outputFile = self.commandline().optionString("output")

        if self.commandline().hasOption("relaxed"):
            self.relaxed = True

        if self.commandline().hasOption("inventory-db") and outputFile is None:
            print >>sys.stderr,"Cannot send notifiers when loading inventory from file."
            return False

        if self.commandline().unrecognizedOptions():
            print >>sys.stderr,"Invalid options: ",
            for i in self.commandline().unrecognizedOptions():
                print >>sys.stderr,i,
            print >>sys.stderr,""
            return False

        if not rulesFile:
            print >>sys.stderr,"No rule file was supplied for processing"
            return False

        if self.commandline().hasOption("inventory-db"):
            self.setDatabaseEnabled(False, False)
            self.setMessagingEnabled(False)

        self.rules = self._digest(rulesFile, self.rules)
        self.outputFile = outputFile
        return True

    def createCommandLineDescription(self):
        Client.Application.createCommandLineDescription(self)

        self.commandline().addGroup("Rules")
        self.commandline().addStringOption("Rules", "rules,r", "Input XML filename")
        self.commandline().addOption("Rules", "relaxed,e", "Relax rules for matching NSLC items")

        self.commandline().addGroup("Dump")
        self.commandline().addStringOption("Dump", "output,o", "Output XML filename")

    def initConfiguration(self):
        value = Client.Application.initConfiguration(self)
        self.setLoggingToStdErr(True)
        self.setDatabaseEnabled(True, True)
        self.setMessagingEnabled(True)
        self.setLoadInventoryEnabled(True)
        return value

    def send(self, *args):
        while not self.connection().send(*args):
            Logging.warning("send failed, retrying")
            time.sleep(1)

    def send_notifiers(self, group):
        Nsize = DataModel.Notifier.Size()

        if Nsize > 0:
            Logging.info("trying to apply %d change%s" % (Nsize,"s" if Nsize != 1 else "" ))
        else:
            Logging.info("no changes to apply")
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
                        Logging.debug("sending message (%5.1f %%)" % (sent / float(Nsize) * 100.0))
                        self.send(group, msg)
                        msg.clear()
                        mcount = 0
                        self.sync()
                    it.next()
            except:
                pass
        finally:
            if msg.size():
                Logging.debug("sending message (%5.1f %%)" % 100.0)
                self.send(group, msg)
                msg.clear()
            self.sync()
        Logging.info("done")
        return mcount

    @staticmethod
    def _loop(obj, count):
        return [ obj(i) for i in range(count) ]

    @staticmethod
    def _collect(obj):
        code  = obj.code()
        start = datetime.datetime.strptime(obj.start().toString("%Y %m %d %H %M %S"), "%Y %m %d %H %M %S")
        try:
            end = obj.end()
            end = datetime.datetime.strptime(end.toString("%Y %m %d %H %M %S"), "%Y %m %d %H %M %S")
        except:
            end = None
        return (code, start, end)

    @staticmethod
    def _modifyInventory(mode, obj, att):
        valid = sc3._findValidOnes(mode)
        if att:
            for (k,p) in att.iteritems():
                p = valid['attributes'][k]['validator'](p)
                getattr(obj, 'set'+k)(p)
            obj.update()
        return

    def run(self):
        rules = self.rules
        iv = Client.Inventory.Instance().inventory()

        if not rules:
            return 1

        if not iv:
            return 1 

        Logging.debug("Loaded %d networks" % iv.networkCount())
        if self.outputFile is None:
            DataModel.Notifier.Enable()
            self.setInterpretNotifierEnabled(True)

        for net in self._loop(iv.network, iv.networkCount()):
            (ncode, nstart, nend) = self._collect(net)
            key = rules.findKey(ncode, nstart, nend)
            if not key: continue
            att = rules.getNetworkAttributes(key)
            self._modifyInventory("network", net, att)
            Logging.info("%s %s" % (ncode, att))
            for sta in self._loop(net.station, net.stationCount()):
                (scode, sstart, send) = self._collect(sta)
                att = rules.getStationAttributes(key, ncode, scode, None, None, sstart, send)
                self._modifyInventory("station", sta, att)
                if att: Logging.info(" %s %s" % (scode, att))
                for loc in self._loop(sta.sensorLocation, sta.sensorLocationCount()):
                    (lcode, lstart, lend) = self._collect(loc)
                    att = rules.getStationAttributes(key, ncode, scode, lcode, None, lstart, lend)
                    self._modifyInventory("location", loc, att)
                    if att: Logging.info("  %s %s" % (lcode, att))
                    for cha in self._loop(loc.stream, loc.streamCount()):
                        (ccode, cstart, cend) = self._collect(cha)
                        att = rules.getStationAttributes(key, ncode, scode, lcode, ccode, cstart, cend)
                        self._modifyInventory("channel", cha, att)
                        if att: Logging.info("   %s %s" % (ccode, att))

        for sensor in self._loop(iv.sensor, iv.sensorCount()):
            att = rules.getInstrumentsAttributes(sensor.name(), "Se")
            self._modifyInventory("sensor", sensor, att)

        for datalogger in self._loop(iv.datalogger, iv.dataloggerCount()):
            att = rules.getInstrumentsAttributes(datalogger.name(), "Dl")
            self._modifyInventory("datalogger", datalogger, att)

        return 0

    def done(self):
        if self.outputFile:
            ar = IO.XMLArchive()
            ar.create(self.outputFile)
            ar.setFormattedOutput(True)
            ar.writeObject(Client.Inventory.Instance().inventory())
            ar.close()
        else:
            self.send_notifiers("INVENTORY")
        Client.Application.done(self)

if __name__ == "__main__":
    app = InventoryModifier(len(sys.argv), sys.argv)
    sys.exit(app())
