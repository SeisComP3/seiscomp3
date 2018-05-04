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

import time
import sys
import os
import time
import seiscomp3.Client
from seiscomp3.scbulletin import Bulletin, stationCount


class ProcAlert(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)

        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)

        self.setAutoApplyNotifierEnabled(True)
        self.setInterpretNotifierEnabled(True)

        self.setPrimaryMessagingGroup("LISTENER_GROUP")
        self.addMessagingSubscription("EVENT")
        self.addMessagingSubscription("LOCATION")
        self.addMessagingSubscription("MAGNITUDE")

        self.maxAgeDays = 1.
        self.minPickCount = 25

        self.procAlertScript = ""

        ep = seiscomp3.DataModel.EventParameters()

    def createCommandLineDescription(self):
        try:
            self.commandline().addGroup("Publishing")
            self.commandline().addIntOption("Publishing", "min-arr",
                                            "Minimum arrival count of a published origin", self.minPickCount)
            self.commandline().addDoubleOption("Publishing", "max-age",
                                               "Maximum age in days of published origins", self.maxAgeDays)
            self.commandline().addStringOption("Publishing", "procalert-script",
                                               "Specify the script to publish an event. The ProcAlert file and the event id are passed as parameter $1 and $2")
            self.commandline().addOption("Publishing", "test",
                                         "Test mode, no messages are sent")
        except:
            seiscomp3.Logging.warning(
                "caught unexpected error %s" % sys.exc_info())

    def initConfiguration(self):
        if not seiscomp3.Client.Application.initConfiguration(self):
            return False

        try:
            self.procAlertScript = self.configGetString("scripts.procAlert")
        except:
            pass

        try:
            self.minPickCount = self.configGetInt("minArrivals")
        except:
            pass

        try:
            self.maxAgeDays = self.configGetDouble("maxAgeDays")
        except:
            pass

        return True

    def init(self):
        if not seiscomp3.Client.Application.init(self):
            return False

        try:
            self.procAlertScript = self.commandline().optionString("procalert-script")
        except:
            pass

        try:
            self.minPickCount = self.commandline().optionInt("min-arr")
        except:
            pass

        try:
            self.maxAgeDays = self.commandline().optionDouble("max-age")
        except:
            pass

        self.bulletin = Bulletin(self.query(), "autoloc1")
        self.cache = seiscomp3.DataModel.PublicObjectRingBuffer(
            self.query(), 100)

        if not self.procAlertScript:
            seiscomp3.Logging.warning("No procalert script given")
        else:
            seiscomp3.Logging.info(
                "Using procalert script: %s" % self.procAlertScript)

        return True

    def addObject(self, parentID, obj):
        org = seiscomp3.DataModel.Origin.Cast(obj)
        if org:
            self.cache.feed(org)
            seiscomp3.Logging.info("Received origin %s" % org.publicID())
            return

        self.updateObject(parentID, obj)

    def updateObject(self, parentID, obj):
        try:
            evt = seiscomp3.DataModel.Event.Cast(obj)
            if evt:
                orid = evt.preferredOriginID()

                org = self.cache.get(seiscomp3.DataModel.Origin, orid)
                if not org:
                    seiscomp3.Logging.error("Unable to fetch origin %s" % orid)
                    return

                if org.arrivalCount() == 0:
                    self.query().loadArrivals(org)
                if org.stationMagnitudeCount() == 0:
                    self.query().loadStationMagnitudes(org)
                if org.magnitudeCount() == 0:
                    self.query().loadMagnitudes(org)

                if not self.originMeetsCriteria(org, evt):
                    seiscomp3.Logging.warning("Origin %s not published" % orid)
                    return

                txt = self.bulletin.printEvent(evt)

                for line in txt.split("\n"):
                    line = line.rstrip()
                    seiscomp3.Logging.info(line)
                seiscomp3.Logging.info("")

                if not self.commandline().hasOption("test"):
                    self.send_procalert(txt, evt.publicID())

                return

        except:
            sys.stderr.write("%s\n" % sys.exc_info())

    def hasValidNetworkMagnitude(self, org, evt):
        nmag = org.magnitudeCount()
        for imag in range(nmag):
            mag = org.magnitude(imag)
            if mag.publicID() == evt.preferredMagnitudeID():
                return True
        return False

    def send_procalert(self, txt, evid):
        if self.procAlertScript:
            tmp = "/tmp/yyy%s" % evid.replace("/", "_").replace(":", "-")
            f = file(tmp, "w")
            f.write("%s" % txt)
            f.close()

            os.system(self.procAlertScript + " " + tmp + " " + evid)

    def coordinates(self, org):
        return org.latitude().value(), org.longitude().value(), org.depth().value()

    def originMeetsCriteria(self, org, evt):
        publish = True

        lat, lon, dep = self.coordinates(org)

        if 43 < lat < 70 and -10 < lon < 60 and dep > 200:
            seiscomp3.Logging.error("suspicious region/depth - ignored")
            publish = False

        if stationCount(org) < self.minPickCount:
            seiscomp3.Logging.error("too few picks - ignored")
            publish = False

        now = seiscomp3.Core.Time.GMT()
        if (now - org.time().value()).seconds()/86400. > self.maxAgeDays:
            seiscomp3.Logging.error("origin too old - ignored")
            publish = False

        try:
            if org.evaluationMode() == seiscomp3.DataModel.MANUAL:
                publish = True
        except:
            pass

        try:
            if org.evaluationStatus() == seiscomp3.DataModel.CONFIRMED:
                publish = True
        except:
            pass

        if not self.hasValidNetworkMagnitude(org, evt):
            seiscomp3.Logging.error("no network magnitude - ignored")
            publish = False

        return publish


app = ProcAlert(len(sys.argv), sys.argv)
sys.exit(app())
