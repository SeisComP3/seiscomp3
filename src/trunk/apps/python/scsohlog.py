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

import sys
import os
import time
import re
import seiscomp3.Client
import seiscomp3.System


"""
Monitor application that connects to the messaging and collects all
information on the STATUS_GROUP to create an XML file ever N seconds.
It can furthermore call a configured script to trigger processing of the
produced XML file.
"""

inputRegEx = re.compile("in\((?P<params>[^\)]*)\)")
outputRegEx = re.compile("out\((?P<params>[^\)]*)\)")


# Define all units of measure for available system SOH tags. Tags that are
# not given here are not processed.
Tests = {
    "cpuusage": "%",
    "clientmemoryusage": "kB",
    "sentmessages": "cnt",
    "receivedmessages": "cnt",
    "messagequeuesize": "cnt",
    "summedmessagequeuesize": "cnt",
    "summedmessagesize": "Byte",
    "objectcount": "cnt",
    "uptime": "s"
}


#----------------------------------------------------------------------------
# Class TestLog to hold the properties of a test. It also creates XML.
#----------------------------------------------------------------------------
class TestLog:
    def __init__(self):
        self.value = None
        self.uom = None
        self.update = None

    def toXML(self, f, name):
        f.write('<test name="%s"' % name)
        if self.value:
            try:
                # Try to convert to float
                fvalue = float(self.value)
                if fvalue % 1.0 >= 1E-6:
                    f.write(' value="%f"' % fvalue)
                else:
                    f.write(' value="%d"' % int(fvalue))
            except:
                f.write(' value="%s"' % self.value)
        if self.uom:
            f.write(' uom="%s"' % self.uom)
        if self.update:
            f.write(' updateTime="%s"' % self.update)
        f.write('/>')


#----------------------------------------------------------------------------
# Class ObjectLog to hold the properties of a object log. It also creates
# XML.
#----------------------------------------------------------------------------
class ObjectLog:
    def __init__(self):
        self.count = None
        self.average = None
        self.timeWindow = None
        self.last = None
        self.update = None

    def toXML(self, f, name, channel):
        f.write('<object')
        if name:
            f.write(' name="%s"' % name)
        if channel:
            f.write(' channel="%s"' % channel)
        if not self.count is None:
            f.write(' count="%s"' % self.count)
        if not self.timeWindow is None:
            f.write(' timeWindow="%s"' % self.timeWindow)
        if not self.average is None:
            f.write(' average="%s"' % self.average)
        if self.last:
            f.write(' lastTime="%s"' % self.last)
        f.write(' updateTime="%s"' % self.update)
        f.write('/>')


#----------------------------------------------------------------------------
# Class Client that holds all tests and object logs of a particular client
# (messaging user name).
#----------------------------------------------------------------------------
class Client:
    def __init__(self):
        self.pid = None
        self.progname = None
        self.host = None

        self.inputLogs = dict()
        self.outputLogs = dict()
        self.tests = dict()

    #----------------------------------------------------------------------------
    # Update/add (system) tests based on the passed tests dictionary retrieved
    # from a status message.
    #----------------------------------------------------------------------------
    def updateTests(self, updateTime, tests):
        for name, value in list(tests.items()):
            if name == "pid":
                self.pid = value
            elif name == "programname":
                self.progname = value
            elif name == "hostname":
                self.host = value

            if name not in Tests:
                continue

            # Convert d:h:m:s to seconds
            if name == "uptime":
                try:
                    t = [int(v) for v in value.split(":")]
                except:
                    continue
                if len(t) != 4:
                    continue
                value = str(t[0]*86400+t[1]*3600+t[2]*60+t[3])

            if name not in self.tests:
                log = TestLog()
                log.uom = Tests[name]
                self.tests[name] = log
            else:
                log = self.tests[name]
            log.value = value
            log.update = updateTime

    #----------------------------------------------------------------------------
    # Update/add object logs based on the passed log text. The content is parsed.
    #----------------------------------------------------------------------------
    def updateObjects(self, updateTime, log):
        # Check input structure
        v = inputRegEx.search(log)
        if not v:
            # Check out structure
            v = outputRegEx.search(log)
            if not v:
                return
            logs = self.outputLogs
        else:
            logs = self.inputLogs

        try:
            tmp = v.group('params').split(',')
        except:
            return

        params = dict()
        for p in tmp:
            try:
                param, value = p.split(':', 1)
            except:
                continue
            params[param] = value

        name = params.get("name", "")
        channel = params.get("chan", "")
        if (name, channel) not in logs:
            logObj = ObjectLog()
            logs[(name, channel)] = logObj
        else:
            logObj = logs[(name, channel)]

        logObj.update = updateTime
        logObj.count = params.get("cnt")
        logObj.average = params.get("avg")
        logObj.timeWindow = params.get("tw")
        logObj.last = params.get("last")

    def toXML(self, f, name):
        f.write('<service name="%s"' % name)
        if self.host:
            f.write(' host="%s"' % self.host)
        if self.pid:
            f.write(' pid="%s"' % self.pid)
        if self.progname:
            f.write(' prog="%s"' % self.progname)
        f.write('>')
        for name, log in list(self.tests.items()):
            log.toXML(f, name)
        if len(self.inputLogs) > 0:
            f.write('<input>')
            for id, log in list(self.inputLogs.items()):
                log.toXML(f, id[0], id[1])
            f.write('</input>')
        if len(self.outputLogs) > 0:
            f.write('<output>')
            for id, log in list(self.outputLogs.items()):
                log.toXML(f, id[0], id[1])
            f.write('</output>')
        f.write("</service>")


#----------------------------------------------------------------------------
# SC3 application class Monitor
#----------------------------------------------------------------------------
class Monitor(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)
        self.setDatabaseEnabled(False, False)
        self.addMessagingSubscription(
            seiscomp3.Communication.Protocol.STATUS_GROUP)
        self.setMessagingUsername("")
        self.setPrimaryMessagingGroup("LISTENER_GROUP")
        self._clients = dict()
        self._outputScript = None
        self._outputFile = "@LOGDIR@/server.xml"
        self._outputInterval = 60

    def createCommandLineDescription(self):
        try:
            self.commandline().addGroup("Output")
            self.commandline().addStringOption("Output", "file,o",
                                               "Specify the output file to create")
            self.commandline().addIntOption("Output", "interval,i",
                                            "Specify the output interval in seconds (default: 60)")
            self.commandline().addStringOption("Output", "script",
                                               "Specify an output script to be called after the output file is generated")
        except:
            seiscomp3.Logging.warning(
                "caught unexpected error %s" % sys.exc_info())
        return True

    def initConfiguration(self):
        if not seiscomp3.Client.Application.initConfiguration(self):
            return False

        try:
            self._outputFile = self.configGetString("monitor.output.file")
        except:
            pass

        try:
            self._outputInterval = self.configGetInt("monitor.output.interval")
        except:
            pass

        try:
            self._outputScript = self.configGetString("monitor.output.script")
        except:
            pass

        return True

    def init(self):
        if not seiscomp3.Client.Application.init(self):
            return False

        try:
            self._outputFile = self.commandline().optionString("file")
        except:
            pass

        try:
            self._outputInterval = self.commandline().optionInt("interval")
        except:
            pass

        try:
            self._outputScript = self.commandline().optionString("script")
        except:
            pass

        self._outputFile = seiscomp3.System.Environment.Instance().absolutePath(self._outputFile)
        seiscomp3.Logging.info("Output file: %s" % self._outputFile)

        if self._outputScript:
            self._outputScript = seiscomp3.System.Environment.Instance(
            ).absolutePath(self._outputScript)
            seiscomp3.Logging.info("Output script: %s" % self._outputScript)

        self._monitor = self.addInputObjectLog(
            "status", seiscomp3.Communication.Protocol.STATUS_GROUP)
        self.enableTimer(self._outputInterval)
        seiscomp3.Logging.info(
            "Starting output timer with %d secs" % self._outputInterval)

        return True

    def handleNetworkMessage(self, msg):
        # A state of health message
        if msg.type() == seiscomp3.Communication.Protocol.STATE_OF_HEALTH_RESPONSE_MSG:
            data = [_f for _f in msg.data().split("&") if _f]
            self.updateStatus(msg.clientName(), data)

        # If a client disconnected, remove it from the list
        elif msg.type() == seiscomp3.Communication.Protocol.CLIENT_DISCONNECTED_MSG:
            if msg.clientName() in self._clients:
                del self._clients[msg.clientName()]

    def handleDisconnect(self):
        # If we got disconnected all client states are deleted
        self._clients = dict()

    #----------------------------------------------------------------------------
    # Timeout handler called by the Application class.
    # Write XML to configured output file and trigger configured script.
    #----------------------------------------------------------------------------
    def handleTimeout(self):
        if self._outputFile == "-":
            self.toXML(sys.stdout)
            sys.stdout.write("\n")
            return

        try:
            f = open(self._outputFile, "w")
        except:
            seiscomp3.Logging.error(
                "Unable to create output file: %s" % self._outputFile)
            return

        self.toXML(f)
        f.close()

        if self._outputScript:
            os.system(self._outputScript + " " + self._outputFile)

    #----------------------------------------------------------------------------
    # Write XML to stream f
    #----------------------------------------------------------------------------
    def toXML(self, f):
        f.write('<?xml version="1.0" encoding="UTF-8"?>')
        f.write('<server name="seiscomp" host="%s">' % self.messagingHost())
        for name, client in list(self._clients.items()):
            client.toXML(f, name)
        f.write('</server>')

    def updateStatus(self, name, items):
        if name not in self._clients:
            self._clients[name] = Client()

        now = seiscomp3.Core.Time.GMT()
        client = self._clients[name]
        self.logObject(self._monitor, now)

        params = dict()
        objs = []

        for t in items:
            try:
                param, value = t.split("=", 1)
                params[param] = value
            except:
                objs.append(t)

        if "time" in params:
            update = params["time"]
            del params["time"]
        else:
            update = now.iso()

        client.updateTests(update, params)
        for o in objs:
            client.updateObjects(update, o)
        #client.toXML(sys.stdout, name)


app = Monitor(len(sys.argv), sys.argv)
sys.exit(app())
