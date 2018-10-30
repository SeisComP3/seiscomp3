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

import os, sys, subprocess, traceback
import seiscomp3.Client, seiscomp3.Seismology, seiscomp3.System

class VoiceAlert(seiscomp3.Client.Application):

    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)

        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setLoadRegionsEnabled(True)
        self.setMessagingUsername("")
        self.setPrimaryMessagingGroup(seiscomp3.Communication.Protocol.LISTENER_GROUP)
        self.addMessagingSubscription("EVENT")
        self.addMessagingSubscription("LOCATION")
        self.addMessagingSubscription("MAGNITUDE")

        self.setAutoApplyNotifierEnabled(True)
        self.setInterpretNotifierEnabled(True)

        self.setLoadCitiesEnabled(True)
        self.setLoadRegionsEnabled(True)

        self._ampType = "snr"
        self._citiesMaxDist = 20
        self._citiesMinPopulation = 50000

        self._eventDescriptionPattern = None
        self._ampScript = None
        self._alertScript = None
        self._eventScript = None

        self._ampProc = None
        self._alertProc = None
        self._eventProc = None

        self._newWhenFirstSeen = False
        self._prevMessage = {}
        self._agencyIDs = []


    def createCommandLineDescription(self):
        self.commandline().addOption("Generic", "first-new", "calls an event a new event when it is seen the first time")
        self.commandline().addGroup("Alert")
        self.commandline().addStringOption("Alert", "amp-type", "specify the amplitude type to listen to", self._ampType)
        self.commandline().addStringOption("Alert", "amp-script", "specify the script to be called when a stationamplitude arrived, network-, stationcode and amplitude are passed as parameters $1, $2 and $3")
        self.commandline().addStringOption("Alert", "alert-script", "specify the script to be called when a preliminary origin arrived, latitude and longitude are passed as parameters $1 and $2")
        self.commandline().addStringOption("Alert", "event-script", "specify the script to be called when an event has been declared; the message string, a flag (1=new event, 0=update event), the EventID, the arrival count and the magnitude (optional when set) are passed as parameter $1, $2, $3, $4 and $5")
        self.commandline().addGroup("Cities")
        self.commandline().addStringOption("Cities", "max-dist", "maximum distance for using the distance from a city to the earthquake")
        self.commandline().addStringOption("Cities", "min-population", "minimum population for a city to become a point of interest")
        self.commandline().addGroup("Debug")
        self.commandline().addStringOption("Debug", "eventid,E", "specify Event ID")
        return True


    def init(self):
        if not seiscomp3.Client.Application.init(self): return False

        try: self._newWhenFirstSeen = self.configGetBool("firstNew");
        except: pass

        try: self._agencyIDs = [ self.configGetString("agencyID") ]
        except: pass

        try:
            agencyIDs = self.configGetStrings("agencyIDs")
            for item in agencyIDs:
                item = item.strip()
                if item not in self._agencyIDs:
                    self._agencyIDs.append(item)
        except: pass

        try:
            if self.commandline().hasOption("first-new"): self._newWhenFirstSeen = True
        except: pass

        try: self._eventDescriptionPattern = self.configGetString("poi.message")
        except: pass

        try: self._citiesMaxDist = self.configGetDouble("poi.maxDist")
        except: pass

        try: self._citiesMaxDist = self.commandline().optionDouble("max-dist")
        except: pass

        try: self._citiesMinPopulation = self.configGetInt("poi.minPopulation")
        except: pass

        try: self._citiesMinPopulation = self.commandline().optionInt("min-population")
        except: pass

        try: self._ampType = self.commandline().optionString("amp-type")
        except: pass

        try: self._ampScript = self.commandline().optionString("amp-script")
        except:
            try: self._ampScript = self.configGetString("scripts.amplitude")
            except: seiscomp3.Logging.warning("No amplitude script defined")

        if self._ampScript:
            self._ampScript = seiscomp3.System.Environment.Instance().absolutePath(self._ampScript)

        try: self._alertScript = self.commandline().optionString("alert-script")
        except:
            try: self._alertScript = self.configGetString("scripts.alert")
            except: seiscomp3.Logging.warning("No alert script defined") 

        if self._alertScript:
            self._alertScript = seiscomp3.System.Environment.Instance().absolutePath(self._alertScript)

        try: self._eventScript = self.commandline().optionString("event-script")
        except:
            try:
                self._eventScript = self.configGetString("scripts.event")
                seiscomp3.Logging.info("Using event script: %s" % self._eventScript)
            except: seiscomp3.Logging.warning("No event script defined")

        self._eventScript = seiscomp3.System.Environment.Instance().absolutePath(self._eventScript)

        seiscomp3.Logging.info("Creating ringbuffer for 100 objects")
        if not self.query():
            seiscomp3.Logging.warning("No valid database interface to read from")
        self._cache = seiscomp3.DataModel.PublicObjectRingBuffer(self.query(), 100)

        if self._ampScript and self.connection():
            self.connection().subscribe("AMPLITUDE")

        if self._newWhenFirstSeen:
            seiscomp3.Logging.info("A new event is declared when I see it the first time")

        if not self._agencyIDs:
            seiscomp3.Logging.info("agencyIDs: []")
        else:
            seiscomp3.Logging.info("agencyIDs: %s" % (" ".join(self._agencyIDs)))

        return True


    def run(self):
        try:
            try:
                eventID = self.commandline().optionString("eventid")
                event = self._cache.get(seiscomp3.DataModel.Event, eventID)
                if event:
                    self.notifyEvent(event)
            except: pass

            return seiscomp3.Client.Application.run(self)
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)
            return False


    def runAmpScript(self, net, sta, amp):
        if not self._ampScript: return

        if self._ampProc is not None:
            if self._ampProc.poll() is None:
                seiscomp3.Logging.warning("AmplitudeScript still in progress -> skipping message")
                return
        try:
            self._ampProc = subprocess.Popen([self._ampScript, net, sta, "%.2f" % amp])
            seiscomp3.Logging.info("Started amplitude script with pid %d" % self._ampProc.pid)
        except:
            seiscomp3.Logging.error("Failed to start amplitude script '%s'" % self._ampScript)


    def runAlert(self, lat, lon):
        if not self._alertScript: return

        if self._alertProc is not None:
            if self._alertProc.poll() is None:
                seiscomp3.Logging.warning("AlertScript still in progress -> skipping message")
                return
        try:
            self._alertProc = subprocess.Popen([self._alertScript, "%.1f" % lat, "%.1f" % lon])
            seiscomp3.Logging.info("Started alert script with pid %d" % self._alertProc.pid)
        except:
            seiscomp3.Logging.error("Failed to start alert script '%s'" % self._alertScript)



    def handleMessage(self, msg):
        try:
            dm = seiscomp3.Core.DataMessage.Cast(msg)
            if dm:
                for att in dm:
                    org = seiscomp3.DataModel.Origin.Cast(att)
                    if org:
                        try:
                            if org.evaluationStatus() == seiscomp3.DataModel.PRELIMINARY:
                                self.runAlert(org.latitude().value(), org.longitude().value())
                        except: pass

            #ao = seiscomp3.DataModel.ArtificialOriginMessage.Cast(msg)
            #if ao:
            #  org = ao.origin()
            #  if org:
            #    self.runAlert(org.latitude().value(), org.longitude().value())
            #  return

            seiscomp3.Client.Application.handleMessage(self, msg)
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)


    def addObject(self, parentID, object):
        try:
            obj = seiscomp3.DataModel.Amplitude.Cast(object)
            if obj:
                if obj.type() == self._ampType:
                    seiscomp3.Logging.debug("got new %s amplitude '%s'" % (self._ampType, obj.publicID()))
                    self.notifyAmplitude(obj)

            obj = seiscomp3.DataModel.Origin.Cast(object)
            if obj:
                self._cache.feed(obj)
                seiscomp3.Logging.debug("got new origin '%s'" % obj.publicID())

                try:
                    if obj.evaluationStatus() == seiscomp3.DataModel.PRELIMINARY:
                        self.runAlert(obj.latitude().value(), obj.longitude().value())
                except: pass

                return

            obj = seiscomp3.DataModel.Magnitude.Cast(object)
            if obj:
                self._cache.feed(obj)
                seiscomp3.Logging.debug("got new magnitude '%s'" % obj.publicID())
                return

            obj = seiscomp3.DataModel.Event.Cast(object)
            if obj:
                org = self._cache.get(seiscomp3.DataModel.Origin, obj.preferredOriginID())
                agencyID = org.creationInfo().agencyID()
                seiscomp3.Logging.debug("got new event '%s'" % obj.publicID())
                if not self._agencyIDs or agencyID in self._agencyIDs:
                    self.notifyEvent(obj, True)
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)


    def updateObject(self, parentID, object):
        try:
            obj = seiscomp3.DataModel.Event.Cast(object)
            if obj:
                org = self._cache.get(seiscomp3.DataModel.Origin, obj.preferredOriginID())
                agencyID = org.creationInfo().agencyID()
                seiscomp3.Logging.debug("update event '%s'" % obj.publicID())
                if not self._agencyIDs or agencyID in self._agencyIDs:
                    self.notifyEvent(obj, False)
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)


    def notifyAmplitude(self, amp):
        self.runAmpScript(amp.waveformID().networkCode(), amp.waveformID().stationCode(), amp.amplitude().value())


    def notifyEvent(self, evt, newEvent=True, dtmax=3600):
        try:
            org = self._cache.get(seiscomp3.DataModel.Origin, evt.preferredOriginID())
            if not org:
                seiscomp3.Logging.warning("unable to get origin %s, ignoring event message" % evt.preferredOriginID())
                return

            preliminary = False
            try:
                if org.evaluationStatus() == seiscomp3.DataModel.PRELIMINARY:
                    preliminary = True
            except: pass

            if preliminary == False:
                nmag = self._cache.get(seiscomp3.DataModel.Magnitude, evt.preferredMagnitudeID())
                if nmag:
                    mag = nmag.magnitude().value()
                    mag = "magnitude %.1f" % mag
                else:
                    if len(evt.preferredMagnitudeID()) > 0:
                        seiscomp3.Logging.warning("unable to get magnitude %s, ignoring event message" % evt.preferredMagnitudeID())
                    else:
                        seiscomp3.Logging.warning("no preferred magnitude yet, ignoring event message")
                    return

            # keep track of old events
            if self._newWhenFirstSeen:
                if evt.publicID() in self._prevMessage:
                    newEvent = False
                else:
                    newEvent = True

            dsc = seiscomp3.Seismology.Regions.getRegionName(org.latitude().value(), org.longitude().value())

            if self._eventDescriptionPattern:
                try:
                    city,dist,azi = self.nearestCity(org.latitude().value(), org.longitude().value(), self._citiesMaxDist, self._citiesMinPopulation)
                    if city:
                        dsc = self._eventDescriptionPattern
                        region = seiscomp3.Seismology.Regions.getRegionName(org.latitude().value(), org.longitude().value())
                        distStr = str(int(seiscomp3.Math.deg2km(dist)))
                        dsc = dsc.replace("@region@", region).replace("@dist@", distStr).replace("@poi@", city.name())
                except: pass

            seiscomp3.Logging.debug("desc: %s" % dsc)

            dep = org.depth().value()
            now = seiscomp3.Core.Time.GMT()
            otm = org.time().value()

            dt = (now - otm).seconds()

    #   if dt > dtmax:
    #       return

            if dt > 3600:
                dt = "%d hours %d minutes ago" % (dt/3600, (dt%3600)/60)
            elif dt > 120:
                dt = "%d minutes ago" % (dt/60)
            else:
                dt = "%d seconds ago" % dt

            if preliminary == True:
                message = "earthquake, preliminary, %%s, %s" % dsc
            else:
                message = "earthquake, %%s, %s, %s, depth %d kilometers" % (dsc, mag, int(dep+0.5))
            # at this point the message lacks the "ago" part

            if evt.publicID() in self._prevMessage and self._prevMessage[evt.publicID()] == message:
                seiscomp3.Logging.info("Suppressing repeated message '%s'" % message)
                return

            self._prevMessage[evt.publicID()] = message
            message = message % dt # fill the "ago" part
            seiscomp3.Logging.info(message)

            if not self._eventScript: return

            if self._eventProc is not None:
                if self._eventProc.poll() is None:
                    seiscomp3.Logging.warning("EventScript still in progress -> skipping message")
                    return

            try:
                param2 = 0
                param3 = 0
                param4 = ""
                if newEvent: param2 = 1

                org = self._cache.get(seiscomp3.DataModel.Origin, evt.preferredOriginID())
                if org:
                    try: param3 = org.quality().associatedPhaseCount()
                    except: pass

                nmag = self._cache.get(seiscomp3.DataModel.Magnitude, evt.preferredMagnitudeID())
                if nmag:
                    param4 = "%.1f" % nmag.magnitude().value()

                self._eventProc = subprocess.Popen([self._eventScript, message, "%d" % param2, evt.publicID(), "%d" % param3, param4])
                seiscomp3.Logging.info("Started event script with pid %d" % self._eventProc.pid)
            except:
                seiscomp3.Logging.error("Failed to start event script '%s %s %d %d %s'" % (self._eventScript, message, param2, param3, param4))
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)


app = VoiceAlert(len(sys.argv), sys.argv)
sys.exit(app())
