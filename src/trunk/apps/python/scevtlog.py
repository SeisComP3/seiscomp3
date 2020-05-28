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
import os
import time
import traceback
import re
import seiscomp3.Client
import seiscomp3.System
import seiscomp3.Utils
from seiscomp3.scbulletin import Bulletin


def time2str(time):
    """
    Convert a seiscomp3.Core.Time to a string
    """
    return time.toString("%Y-%m-%d %H:%M:%S.%f000000")[:23]


def createDirectory(dir):
    if os.access(dir, os.W_OK):
        return True

    try:
        os.makedirs(dir)
        return True
    except:
        return False


def originStatusToChar(org):
    # Manual origin are always tagged as M
    try:
        if org.evaluationMode() == seiscomp3.DataModel.MANUAL:
            return 'M'
    except:
        pass

    try:
        if org.evaluationStatus() == seiscomp3.DataModel.PRELIMINARY:
            return 'P'
        elif org.evaluationStatus() == seiscomp3.DataModel.CONFIRMED or \
                org.evaluationStatus() == seiscomp3.DataModel.REVIEWED or \
                org.evaluationStatus() == seiscomp3.DataModel.FINAL:
            return 'C'
        elif org.evaluationStatus() == seiscomp3.DataModel.REJECTED:
            return 'X'
        elif org.evaluationStatus() == seiscomp3.DataModel.REPORTED:
            return 'R'
    except:
        pass

    return 'A'


class CachePopCallback(seiscomp3.DataModel.CachePopCallback):
    def __init__(self, target):
        seiscomp3.DataModel.CachePopCallback.__init__(self)
        self.target = target

    def handle(self, obj):
        self.target.objectAboutToPop(obj)


class EventHistory(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)
        seiscomp3.DataModel.Notifier.SetEnabled(False)

        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setMessagingUsername("scevtlog")
        self.setPrimaryMessagingGroup(
            seiscomp3.Communication.Protocol.LISTENER_GROUP)
        self.addMessagingSubscription("EVENT")
        self.addMessagingSubscription("LOCATION")
        self.addMessagingSubscription("MAGNITUDE")

        self.setAutoApplyNotifierEnabled(True)
        self.setInterpretNotifierEnabled(True)

        # Create a callback object that gets called when an object
        # is going to be removed from the cache
        self._popCallback = CachePopCallback(self)

        # Create an object cache of half an hour
        self._cache = seiscomp3.DataModel.PublicObjectTimeSpanBuffer(
            self.query(), seiscomp3.Core.TimeSpan(30.0*60.0))
        self._cache.setPopCallback(self._popCallback)

        # Event progress counter
        self._eventProgress = dict()

        # Event-Origin mapping
        self._eventToOrg = dict()
        self._orgToEvent = dict()

        # Event-Magnitude mapping
        self._eventToMag = dict()
        self._magToEvent = dict()

        self._directory = "@LOGDIR@/events"
        self._format = "xml"
        self._currentDirectory = ""
        self._revisionFileExt = ".zip"
        self._useGZIP = False

    def createCommandLineDescription(self):
        try:
            self.commandline().addGroup("Storage")
            self.commandline().addStringOption(
                "Storage", "directory,o", "Specify the storage directory")
            self.commandline().addStringOption("Storage", "format,f",
                                               "Specify storage format (autoloc1, autoloc3, xml [default])")
        except:
            seiscomp3.Logging.warning(
                "caught unexpected error %s" % sys.exc_info())
        return True

    def initConfiguration(self):
        if not seiscomp3.Client.Application.initConfiguration(self):
            return False

        try:
            self._directory = self.configGetString("directory")
        except:
            pass

        try:
            self._format = self.configGetString("format")
        except:
            pass

        try:
            if self.configGetBool("gzip") == True:
                self._useGZIP = True
                self._revisionFileExt = ".gz"
        except:
            pass

        return True

    def init(self):
        if not seiscomp3.Client.Application.init(self):
            return False

        try:
            self._directory = self.commandline().optionString("directory")
        except:
            pass

        try:
            self._format = self.commandline().optionString("format")
        except:
            pass

        if self._format != "autoloc1" and self._format != "autoloc3" and self._format != "xml":
            self._format = "xml"

        try:
            if self._directory[-1] != "/":
                self._directory = self._directory + "/"
        except:
            pass

        if self._directory:
            self._directory = seiscomp3.System.Environment.Instance().absolutePath(self._directory)
            sys.stderr.write("Logging events to %s\n" % self._directory)

        self._cache.setDatabaseArchive(self.query())
        return True

    # def run(self):
        # obj = self._cache.get(seiscomp3.DataModel.Magnitude, "or080221153929#16#netMag.mb")

        # self.updateObject(obj)
        # return True

    def printEvent(self, evt, newEvent):
        if self._format != "xml":
            self.printEventProcAlert(evt, newEvent)
        else:
            self.printEventXML(evt, newEvent)
        self.advanceEventProgress(evt.publicID())

    def getSummary(self, time, org, mag):
        strTime = time.toString("%Y-%m-%d %H:%M:%S")
        summary = [strTime, "", "", "", "", "", "", "", "", ""]

        if org:
            tim = org.time().value()
            latency = time - tim

            summary[1] = "%5d.%02d" % (
                latency.seconds() / 60, (latency.seconds() % 60) * 100 / 60)

            lat = org.latitude().value()
            lon = org.longitude().value()

            dep = "%7s" % "---"
            try:
                dep = "%7.0f" % org.depth().value()
                summary[4] = dep
            except:
                summary[4] = "%7s" % ""

            phases = "%5s" % "---"
            try:
                phases = "%5d" % org.quality().usedPhaseCount()
                summary[5] = phases
            except:
                summary[5] = "%5s" % ""

            summary[2] = "%7.2f" % lat
            summary[3] = "%7.2f" % lon

            try:
                summary[9] = originStatusToChar(org)
            except:
                summary[9] = "-"

        if mag:
            summary[6] = "%12s" % mag.type()
            summary[7] = "%5.2f" % mag.magnitude().value()
            try:
                summary[8] = "%5d" % mag.stationCount()
            except:
                summary[8] = "     "
        else:
            summary[6] = "%12s" % ""
            summary[7] = "     "
            summary[8] = "     "

        return summary

    def printEventProcAlert(self, evt, newEvent):
        now = seiscomp3.Core.Time.GMT()

        org = self._cache.get(seiscomp3.DataModel.Origin,
                              evt.preferredOriginID())
        prefmag = self._cache.get(
            seiscomp3.DataModel.Magnitude, evt.preferredMagnitudeID())

        summary = self.getSummary(now, org, prefmag)

        # Load arrivals
        if org.arrivalCount() == 0:
            self.query().loadArrivals(org)

        # Load station magnitudes
        if org.stationMagnitudeCount() == 0:
            self.query().loadStationMagnitudes(org)

        # Load magnitudes
        if org.magnitudeCount() == 0:
            self.query().loadMagnitudes(org)

        picks = []
        amps = []

        if org:
            narr = org.arrivalCount()
            for i in range(narr):
                picks.append(self._cache.get(
                    seiscomp3.DataModel.Pick, org.arrival(i).pickID()))

            nstamags = org.stationMagnitudeCount()
            for i in range(nstamags):
                amps.append(self._cache.get(
                    seiscomp3.DataModel.Amplitude, org.stationMagnitude(i).amplitudeID()))

        netmag = {}
        nmag = org.magnitudeCount()

        bulletin = Bulletin(None, self._format)
        try:
            txt = bulletin.printEvent(evt)
        except:
            txt = ""

        if self._directory is None:
            sys.stdout.write("%s" % ("#<\n" + txt + "#>\n"))
            sys.stdout.flush()
        else:
            # Use created time to look up the proper directory
            try:
                arNow = evt.creationInfo().creationTime().get()
            # Otherwise use now (in case that event.created has not been set
            # which is always valid within the SC3 distribution
            except:
                arNow = now.get()
            seiscomp3.Logging.error("directory is " + self._directory + "/".join(
                ["%.2d" % i for i in arNow[1:4]]) + "/" + evt.publicID() + "/")

            directory = self._directory + \
                "/".join(["%.2d" % i for i in arNow[1:4]]) + \
                "/" + evt.publicID() + "/"
            if directory != self._currentDirectory:
                if createDirectory(directory) == False:
                    seiscomp3.Logging.error(
                        "Unable to create directory %s" % directory)
                    return

            self._currentDirectory = directory
            self.writeLog(self._currentDirectory + self.convertID(evt.publicID()) +
                          "." + ("%06d" % self.eventProgress(evt.publicID(), directory)), txt, "w")
            self.writeLog(self._currentDirectory +
                          self.convertID(evt.publicID()) + ".last", txt, "w")
            self.writeLog(self._directory + "last", txt, "w")
            self.writeLog(self._currentDirectory + self.convertID(evt.publicID()) + ".summary",
                          "|".join(summary), "a",
                          "# Layout: Timestamp, +OT (minutes, decimal), Latitude, Longitude, Depth, PhaseCount, MagType, Magnitude, MagCount")

        seiscomp3.Logging.info("cache size = %d" % self._cache.size())

    def printEventXML(self, evt, newEvent):
        now = seiscomp3.Core.Time.GMT()

        # Load comments
        if evt.commentCount() == 0:
            self.query().loadComments(evt)

        # Load origin references
        if evt.originReferenceCount() == 0:
            self.query().loadOriginReferences(evt)

        # Load event descriptions
        if evt.eventDescriptionCount() == 0:
            self.query().loadEventDescriptions(evt)

        org = self._cache.get(seiscomp3.DataModel.Origin,
                              evt.preferredOriginID())

        if evt.preferredFocalMechanismID():
            fm = self._cache.get(
                seiscomp3.DataModel.FocalMechanism, evt.preferredFocalMechanismID())
        else:
            fm = None

        # Load comments
        if org.commentCount() == 0:
            self.query().loadComments(org)

        # Load arrivals
        if org.arrivalCount() == 0:
            self.query().loadArrivals(org)
        prefmag = self._cache.get(
            seiscomp3.DataModel.Magnitude, evt.preferredMagnitudeID())

        wasEnabled = seiscomp3.DataModel.PublicObject.IsRegistrationEnabled()
        seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(False)

        ep = seiscomp3.DataModel.EventParameters()
        evt_cloned = seiscomp3.DataModel.Event.Cast(evt.clone())
        ep.add(evt_cloned)

        summary = self.getSummary(now, org, prefmag)

        if fm:
            ep.add(fm)

            seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(wasEnabled)

            # Load focal mechainsm references
            if evt.focalMechanismReferenceCount() == 0:
                self.query().loadFocalMechanismReferences(evt)

            # Load moment tensors
            if fm.momentTensorCount() == 0:
                self.query().loadMomentTensors(fm)

            seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(False)

            # Copy focal mechanism reference
            fm_ref = evt.focalMechanismReference(
                seiscomp3.DataModel.FocalMechanismReferenceIndex(fm.publicID()))
            if fm_ref:
                fm_ref_cloned = seiscomp3.DataModel.FocalMechanismReference.Cast(
                    fm_ref.clone())
                if fm_ref_cloned is None:
                    fm_ref_cloned = seiscomp3.DataModel.FocalMechanismReference(
                        fm.publicID())
                evt_cloned.add(fm_ref_cloned)

            nmt = fm.momentTensorCount()
            for i in range(nmt):
                mt = fm.momentTensor(i)
                if not mt.derivedOriginID():
                    continue

                # Origin already added
                if ep.findOrigin(mt.derivedOriginID()) is not None:
                    continue

                seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(
                    wasEnabled)
                derivedOrigin = self._cache.get(
                    seiscomp3.DataModel.Origin, mt.derivedOriginID())
                seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(False)

                if derivedOrigin is None:
                    seiscomp3.Logging.warning(
                        "derived origin for MT %s not found" % mt.derivedOriginID())
                    continue

                # Origin has been read from database -> read all childs
                if not self._cache.cached():
                    seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(
                        wasEnabled)
                    self.query().load(derivedOrigin)
                    seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(
                        False)

                # Add it to the event parameters
                ep.add(derivedOrigin)

        if org:
            seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(wasEnabled)

            # Load magnitudes
            if org.magnitudeCount() == 0:
                self.query().loadMagnitudes(org)

            if org.stationMagnitudeCount() == 0:
                self.query().loadStationMagnitudes(org)

            seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(False)

            # Copy event comments
            ncmts = evt.commentCount()
            for i in range(ncmts):
                cmt_cloned = seiscomp3.DataModel.Comment.Cast(
                    evt.comment(i).clone())
                evt_cloned.add(cmt_cloned)

            # Copy origin references
            org_ref = evt.originReference(
                seiscomp3.DataModel.OriginReferenceIndex(org.publicID()))
            if org_ref:
                org_ref_cloned = seiscomp3.DataModel.OriginReference.Cast(
                    org_ref.clone())
                if org_ref_cloned is None:
                    org_ref_cloned = seiscomp3.DataModel.OriginReference(
                        org.publicID())
                evt_cloned.add(org_ref_cloned)

            # Copy event descriptions
            for i in range(evt.eventDescriptionCount()):
                ed_cloned = seiscomp3.DataModel.EventDescription.Cast(
                    evt.eventDescription(i).clone())
                evt_cloned.add(ed_cloned)

            org_cloned = seiscomp3.DataModel.Origin.Cast(org.clone())
            ep.add(org_cloned)

            # Copy origin comments
            ncmts = org.commentCount()
            for i in range(ncmts):
                cmt_cloned = seiscomp3.DataModel.Comment.Cast(
                    org.comment(i).clone())
                org_cloned.add(cmt_cloned)

            # Copy arrivals
            narr = org.arrivalCount()
            for i in range(narr):
                arr_cloned = seiscomp3.DataModel.Arrival.Cast(
                    org.arrival(i).clone())
                org_cloned.add(arr_cloned)

                seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(
                    wasEnabled)
                pick = self._cache.get(
                    seiscomp3.DataModel.Pick, arr_cloned.pickID())
                seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(False)

                if pick:
                    pick_cloned = seiscomp3.DataModel.Pick.Cast(pick.clone())
                    ep.add(pick_cloned)

            # Copy network magnitudes
            nmag = org.magnitudeCount()
            for i in range(nmag):
                mag = org.magnitude(i)

                mag_cloned = seiscomp3.DataModel.Magnitude.Cast(mag.clone())

                seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(
                    wasEnabled)
                if mag.stationMagnitudeContributionCount() == 0:
                    self.query().loadStationMagnitudeContributions(mag)
                seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(False)

                # Copy magnitude references
                nmagref = mag.stationMagnitudeContributionCount()
                for j in range(nmagref):
                    mag_ref_cloned = seiscomp3.DataModel.StationMagnitudeContribution.Cast(
                        mag.stationMagnitudeContribution(j).clone())
                    mag_cloned.add(mag_ref_cloned)

                org_cloned.add(mag_cloned)

            # Copy station magnitudes and station amplitudes
            smag = org.stationMagnitudeCount()
            amp_map = dict()
            for i in range(smag):
                mag_cloned = seiscomp3.DataModel.StationMagnitude.Cast(
                    org.stationMagnitude(i).clone())
                org_cloned.add(mag_cloned)
                if (mag_cloned.amplitudeID() in amp_map) == False:
                    amp_map[mag_cloned.amplitudeID()] = True
                    seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(
                        wasEnabled)
                    amp = self._cache.get(
                        seiscomp3.DataModel.Amplitude, mag_cloned.amplitudeID())
                    seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(
                        False)
                    if amp:
                        amp_cloned = seiscomp3.DataModel.Amplitude.Cast(
                            amp.clone())
                        ep.add(amp_cloned)

        seiscomp3.DataModel.PublicObject.SetRegistrationEnabled(wasEnabled)

        # archive.create(event.publicID() + )
        ar = seiscomp3.IO.XMLArchive()
        ar.setFormattedOutput(True)

        if self._directory is None:
            sys.stdout.write("#<\n")
            ar.create("-")
            ar.writeObject(ep)
            ar.close()
            sys.stdout.write("#>\n")
            sys.stdout.flush()
        else:
            # Use created time to look up the proper directory
            try:
                arNow = evt.creationInfo().creationTime().get()
            # Otherwise use now (in case that event.created has not been set
            # which is always valid within the SC3 distribution
            except:
                arNow = now.get()

            directory = self._directory + \
                "/".join(["%.2d" % i for i in arNow[1:4]]) + \
                "/" + evt.publicID() + "/"
            if directory != self._currentDirectory:
                if createDirectory(directory) == False:
                    seiscomp3.Logging.error(
                        "Unable to create directory %s" % directory)
                    return

            self._currentDirectory = directory
            # self.writeLog(self._currentDirectory + evt.publicID(), "#<\n" + txt + "#>\n")
            #self.writeLog(self._currentDirectory + evt.publicID() + ".last", txt, "w")
            ar.create(self._currentDirectory + self.convertID(evt.publicID()) + "." + ("%06d" %
                                                                                       self.eventProgress(evt.publicID(), directory)) + ".xml" + self._revisionFileExt)
            ar.setCompression(True)
            if self._useGZIP:
                ar.setCompressionMethod(seiscomp3.IO.XMLArchive.GZIP)
            ar.writeObject(ep)
            ar.close()
            # Write last file to root
            ar.create(self._directory + "last.xml" + self._revisionFileExt)
            ar.setCompression(True)
            if self._useGZIP:
                ar.setCompressionMethod(seiscomp3.IO.XMLArchive.GZIP)
            ar.writeObject(ep)
            ar.close()
            # Write last xml
            ar.create(self._currentDirectory +
                      self.convertID(evt.publicID()) + ".last.xml")
            ar.setCompression(False)
            ar.writeObject(ep)
            ar.close()
            self.writeLog(self._currentDirectory + self.convertID(evt.publicID()) + ".summary",
                          "|".join(summary), "a",
                          "# Layout: Timestamp, +OT (minutes, decimal), Latitude, Longitude, Depth, PhaseCount, MagType, Magnitude, MagCount")

        del ep

    def convertID(self, id):
        '''Converts an ID containing slashes to one without slashes'''
        p = re.compile('/')
        return p.sub('_', id)

    def writeLog(self, file, text, mode="a", header=None):
        of = open(file, mode)
        if of:
            if of.tell() == 0 and not header is None:
                of.write(header+"\n")
            of.write(text+"\n")
            of.close()
        else:
            seiscomp3.Logging.error("Unable to write file: %s" % file)

    def objectAboutToPop(self, obj):
        try:
            evt = seiscomp3.DataModel.Event.Cast(obj)
            if evt:
                try:
                    self._orgToEvent.pop(evt.preferredOriginID())
                    self._eventToOrg.pop(evt.publicID())

                    self._magToEvent.pop(evt.preferredMagnitudeID())
                    self._eventToMag.pop(evt.publicID())

                    self._eventProgress.pop(evt.publicID())
                    return
                except:
                    pass

            org = seiscomp3.DataModel.Origin.Cast(obj)
            if org:
                try:
                    self._orgToEvent.pop(org.publicID())
                except:
                    pass
                return

            mag = seiscomp3.DataModel.Magnitude.Cast(obj)
            if mag:
                try:
                    self._magToEvent.pop(mag.publicID())
                except:
                    pass
                return
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info:
                sys.stderr.write(i)
            sys.exit(-1)

    def eventProgress(self, evtID, directory):
        # The progress is already stored
        if evtID in self._eventProgress:
            return self._eventProgress[evtID]

        # Find the maximum file counter
        maxid = -1
        files = os.listdir(directory)
        for file in files:
            if os.path.isfile(directory + file) == False:
                continue
            fid = file[len(evtID + '.'):len(file)]
            sep = fid.find('.')
            if sep == -1:
                sep = len(fid)
            fid = fid[0:sep]
            try:
                nid = int(fid)
            except:
                continue
            if nid > maxid:
                maxid = nid

        maxid = maxid + 1
        self._eventProgress[evtID] = maxid
        return maxid

    def advanceEventProgress(self, evtID):
        try:
            self._eventProgress[evtID] = self._eventProgress[evtID] + 1
        except:
            pass

    def addObject(self, parentID, object):
        try:
            obj = seiscomp3.DataModel.Event.Cast(object)
            if obj:
                self._cache.feed(obj)
                self._eventProgress[obj.publicID()] = 0
                self.printEvent(obj, True)
                self.updateCache(obj)
                return

            # New Magnitudes or Origins are not important for
            # the history update but we feed it into the cache to
            # access them faster later on in case they will become
            # preferred entities
            obj = seiscomp3.DataModel.Magnitude.Cast(object)
            if obj:
                self._cache.feed(obj)
                return

            obj = seiscomp3.DataModel.Origin.Cast(object)
            if obj:
                self._cache.feed(obj)
                return

            obj = seiscomp3.DataModel.Pick.Cast(object)
            if obj:
                self._cache.feed(obj)
                return

            obj = seiscomp3.DataModel.Amplitude.Cast(object)
            if obj:
                self._cache.feed(obj)
                return

        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info:
                sys.stderr.write(i)
            sys.exit(-1)

    def updateObject(self, parentID, object):
        try:
            obj = seiscomp3.DataModel.Event.Cast(object)
            if obj:
                self._cache.feed(obj)
                self.printEvent(obj, False)
                self.updateCache(obj)
                return

            # Updates of a Magnitude are only imported when it is
            # the preferred one.
            obj = seiscomp3.DataModel.Magnitude.Cast(object)
            if obj:
                try:
                    evtID = self._magToEvent[obj.publicID()]
                    if evtID:
                        self._cache.feed(obj)
                        evt = self._cache.get(seiscomp3.DataModel.Event, evtID)
                        if evt:
                            self.printEvent(evt, False)
                        else:
                            sys.stderr.write("Unable to fetch event for ID '%s' while update of magnitude '%s'\n" % (
                                evtID, obj.publicID()))
                    else:
                        # Magnitude has not been associated to an event yet
                        pass
                except:
                    # Search the corresponding event from the database
                    evt = self.query().getEventByPreferredMagnitudeID(obj.publicID())
                    # Associate the event (even if None) with the magnitude ID
                    if evt:
                        self._magToEvent[obj.publicID()] = evt.publicID()
                        self._cache.feed(obj)
                        self.printEvent(evt, False)
                    else:
                        self._magToEvent[obj.publicID()] = None
                return

            # Usually we do not update origins. To have it complete,
            # this case will be supported as well
            obj = seiscomp3.DataModel.Origin.Cast(object)
            if obj:
                try:
                    evtID = self._orgToEvent[obj.publicID()]
                    if evtID:
                        self._cache.feed(obj)
                        evt = self._cache.get(seiscomp3.DataModel.Event, evtID)
                        if evt:
                            self.printEvent(evt, False)
                        else:
                            sys.stderr.write("Unable to fetch event for ID '%s' while update of origin '%s'\n" % (
                                evtID, obj.publicID()))
                    else:
                        # Origin has not been associated to an event yet
                        pass
                except:
                    # Search the corresponding event from the database
                    evt = self.query().getEvent(obj.publicID())
                    if evt:
                        if evt.preferredOriginID() != obj.publicID():
                            evt = None

                    # Associate the event (even if None) with the origin ID
                    if evt:
                        self._orgToEvent[obj.publicID()] = evt.publicID()
                        self._cache.feed(obj)
                        self.printEvent(evt, False)
                    else:
                        self._orgToEvent[obj.publicID()] = None
                return

            return

        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info:
                sys.stderr.write(i)
            sys.exit(-1)

    def updateCache(self, evt):
        # Event-Origin update
        try:
            orgID = self._eventToOrg[evt.publicID()]
            if orgID != evt.preferredOriginID():
                self._orgToEvent.pop(orgID)
        except:
            # origin not yet registered
            pass

        # Bind the current preferred origin ID to the event and
        # vice versa
        self._orgToEvent[evt.preferredOriginID()] = evt.publicID()
        self._eventToOrg[evt.publicID()] = evt.preferredOriginID()

        # Event-Magnitude update
        try:
            magID = self._eventToMag[evt.publicID()]
            if magID != evt.preferredMagnitudeID():
                self._magToEvent.pop(magID)
        except:
            # not yet registered
            pass

        # Bind the current preferred magnitude ID to the event and
        # vice versa
        self._magToEvent[evt.preferredMagnitudeID()] = evt.publicID()
        self._eventToMag[evt.publicID()] = evt.preferredMagnitudeID()


app = EventHistory(len(sys.argv), sys.argv)
sys.exit(app())
