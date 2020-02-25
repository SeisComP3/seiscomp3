#!/usr/bin/env python
"""
Copyright
---------
This file is part of the Virtual Seismologist (VS) software package.
VS is free software: you can redistribute it and/or modify it under
the terms of the "SED Public License for Seiscomp Contributions"

VS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the SED Public License for Seiscomp
Contributions for more details.

You should have received a copy of the SED Public License for Seiscomp
Contributions with VS. If not, you can find it at
http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt

Author of the Software: Yannik Behr
Copyright (C) 2006-2013 by Swiss Seismological Service
"""

import sys, traceback, seiscomp3.Client
from seiscomp3.DataModel import PublicObjectTimeSpanBuffer, EventParameters
from seiscomp3.Core import TimeSpan, Time
from seiscomp3 import Config, System
import smtplib
from email.mime.text import MIMEText
import os
import datetime
from dateutil import tz
import random


class Listener(seiscomp3.Client.Application):

    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)
        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)
        self.setPrimaryMessagingGroup(seiscomp3.Communication.Protocol.LISTENER_GROUP)
        self.addMessagingSubscription("LOCATION")
        self.addMessagingSubscription("MAGNITUDE")
        self.addMessagingSubscription("PICK")
        self.addMessagingSubscription("EVENT")

        self.cache = PublicObjectTimeSpanBuffer()
        self.expirationtime = 3600.
        self.event_dict = {}
        self.origin_lookup = {}
        self.event_lookup = {}
        self.latest_event = Time.Null
        # report settings
        self.ei = System.Environment.Instance()
        self.report_head = "Mag.|Lat.  |Lon.   |tdiff |Depth |creation time (UTC)" + " "*6 + "|"
        self.report_head += "origin time (UTC)" + " "*8 + "|likeh." + "|#st.(org.) "
        self.report_head += "|#st.(mag.)" + "\n"
        self.report_head += "-"*114 + "\n"
        self.report_directory = os.path.join(self.ei.logDir(), 'VS_reports')
        # email settings
        self.sendemail = True
        self.smtp_server = None
        self.email_port = None
        self.hostname = None
        self.tls = False
        self.ssl = False
        self.credentials = None
        self.email_sender = None
        self.email_recipients = None
        self.email_subject = None
        self.username = None
        self.password = None
        self.auth = False
        self.magThresh = 0.0
        # UserDisplay interface
        self.udevt = None

    def handleTimeout(self):
        self.hb.send_hb()

    def validateParameters(self):
        try:
            if seiscomp3.Client.Application.validateParameters(self) == False:
                return False
            if self.commandline().hasOption('savedir'):
                self.report_directory = self.commandline().optionString('savedir')
            if self.commandline().hasOption('playback'):
                self.setDatabaseEnabled(False, False)
            return True
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)
            return False

    def createCommandLineDescription(self):
        """
        Setup command line options.
        """
        try:
            self.commandline().addGroup("Reports")
            self.commandline().addStringOption("Reports", "savedir", "Directory to save reports to.")
            self.commandline().addGroup("Mode")
            self.commandline().addOption("Mode", "playback", "Disable database connection.")
        except:
            seiscomp3.Logging.warning("caught unexpected error %s" % sys.exc_info())
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)
        return True

    def initConfiguration(self):
        """
        Read configuration file.
        """
        if not seiscomp3.Client.Application.initConfiguration(self):
            return False

        try:
            self.sendemail = self.configGetBool("email.activate")
            self.smtp_server = self.configGetString("email.smtpserver")
            self.email_port = self.configGetInt("email.port")
            self.tls = self.configGetBool("email.usetls")
            self.ssl = self.configGetBool("email.usessl")
            if self.tls and self.ssl:
                seiscomp.Logging.warning('TLS and SSL cannot be both true!')
                self.tls = False
                self.ssl = False
            self.auth = self.configGetBool("email.authenticate")
            if (self.auth):
                cf = seiscomp3.Config.Config()
                cf.readConfig(self.configGetString("email.credentials"))
                self.username = cf.getString('username')
                self.password = cf.getString('password')
            self.email_sender = self.configGetString("email.senderaddress")
            self.email_recipients = self.configGetStrings("email.recipients")
            self.email_subject = self.configGetString("email.subject")
            self.hostname = self.configGetString("email.host")
            self.magThresh = self.configGetDouble("email.magThresh")
        except Exception as e:
            seiscomp3.Logging.info('Some configuration parameters could not be read: %s' % e)
            self.sendemail = False

        try:
            self.amqHost = self.configGetString('ActiveMQ.hostname')
            self.amqPort = self.configGetInt('ActiveMQ.port')
            self.amqTopic = self.configGetString('ActiveMQ.topic')
            self.amqHbTopic = self.configGetString('ActiveMQ.hbtopic')
            self.amqUser = self.configGetString('ActiveMQ.username')
            self.amqPwd = self.configGetString('ActiveMQ.password')
            self.amqMsgFormat = self.configGetString('ActiveMQ.messageFormat')
        except:
            pass

        try:
            self.storeReport = self.configGetBool("report.activate")
            self.expirationtime = self.configGetDouble("report.eventbuffer")
            self.report_directory = self.ei.absolutePath(self.configGetString("report.directory"))
        except:
            pass
        return True

    def init(self):
        """
        Initialization.
        """
        if not seiscomp3.Client.Application.init(self):
            return False

        if not self.sendemail:
            seiscomp3.Logging.info('Sending email has been disabled.')
        else:
            self.sendMail({}, '', test=True)

        self.cache.setTimeSpan(TimeSpan(self.expirationtime))
        if self.isDatabaseEnabled():
            self.cache.setDatabaseArchive(self.query());
            seiscomp3.Logging.info('Cache connected to database.')
        if self.storeReport:
            seiscomp3.Logging.info("Reports are stored in %s" % self.report_directory)
        else:
            seiscomp3.Logging.info("Saving reports to disk has been DISABLED!")
        try:
            import ud_interface
            self.udevt = ud_interface.CoreEventInfo(self.amqHost, self.amqPort,
                                                    self.amqTopic, self.amqUser,
                                                    self.amqPwd,
                                                    self.amqMsgFormat)
            self.hb = ud_interface.HeartBeat(self.amqHost, self.amqPort,
                                             self.amqHbTopic, self.amqUser,
                                             self.amqPwd, self.amqMsgFormat)
            self.enableTimer(5)
            seiscomp3.Logging.info('ActiveMQ interface is running.')
        except Exception as e:
            seiscomp3.Logging.warning('ActiveMQ interface cannot be loaded: %s' % e)
        return True

    def generateReport(self, evID):
        """
        Generate a report for an event, write it to disk and optionally send
        it as an email.
        """
        sout = self.report_head
        threshold_exceeded = False
        for _i in sorted(self.event_dict[evID]['updates'].keys()):
            ed = self.event_dict[evID]['updates'][_i]
            mag = ed['magnitude']
            if mag > self.magThresh:
                threshold_exceeded = True
            sout += "%4.2f|" % mag
            sout += "%6.2f|" % ed['lat']
            sout += "%7.2f|" % ed['lon']
            sout += "%6.2f|" % ed['diff'].toDouble()
            sout += "%6.2f|" % ed['depth']
            sout += "%s|" % ed['ts']
            sout += "%s|" % ed['ot']
            sout += "%6.2f|" % ed['likelihood']
            sout += "%11d|" % ed['nstorg']
            sout += "%10d\n" % ed['nstmag']

        if self.storeReport:
            self.event_dict[evID]['report'] = sout
            if not os.path.isdir(self.report_directory):
                os.makedirs(self.report_directory)
            f = open(os.path.join(self.report_directory,
                                  '%s_report.txt' % evID.replace('/', '_')), 'w')
            f.writelines(self.event_dict[evID]['report'])
            f.close()
        self.event_dict[evID]['magnitude'] = ed['magnitude']
        seiscomp3.Logging.info("\n" + sout)
        if self.sendemail and threshold_exceeded:
            self.sendMail(self.event_dict[evID], evID)
        self.event_dict[evID]['published'] = True

    def handleMagnitude(self, magnitude, parentID):
        """
        Generate an origin->magnitude lookup table.
        """
        try:
            if magnitude.type() == 'MVS':
                seiscomp3.Logging.debug("Received MVS magnitude for origin %s" % parentID)
                self.origin_lookup[magnitude.publicID()] = parentID
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)

    def handleOrigin(self, org, parentID):
        """
        Add origins to the cache.
        """
        try:
            seiscomp3.Logging.debug("Received origin %s" % org.publicID())
            self.cache.feed(org)
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)

    def handlePick(self, pk, parentID):
        """
        Add picks to the cache.
        """
        try:
            seiscomp3.Logging.debug("Received pick %s" % pk.publicID())
            self.cache.feed(pk)
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)

    def garbageCollector(self):
        """
        Remove outdated events from the event dictionary.
        """
        tcutoff = self.latest_event - TimeSpan(self.expirationtime)
        for evID in self.event_dict.keys():
            if self.event_dict[evID]['timestamp'] < tcutoff:
                self.event_dict.pop(evID)

    def handleEvent(self, evt):
        """
        Add events to the event dictionary and generate an
        event->origin lookup table.
        """
        evID = evt.publicID()
        seiscomp3.Logging.debug("Received event %s" % evID)
        self.event_lookup[evt.preferredOriginID()] = evID
        if evID not in self.event_dict.keys():
            self.event_dict[evID] = {}
            self.event_dict[evID]['published'] = False
            self.event_dict[evID]['updates'] = {}
            try:
                self.event_dict[evID]['timestamp'] = \
                evt.creationInfo().modificationTime()
            except:
                self.event_dict[evID]['timestamp'] = \
                evt.creationInfo().creationTime()
            if self.event_dict[evID]['timestamp'] > self.latest_event:
                self.latest_event = self.event_dict[evID]['timestamp']
        # delete old events
        self.garbageCollector()

    def sendMail(self, evt, evID, test=False):
        """
        Email reports.
        """
        if test:
            msg = MIMEText('scvsmaglog was started.')
            msg['Subject'] = 'scvsmaglog startup message'
        else:
            msg = MIMEText(evt['report'])
            subject = self.email_subject + ' / %s / ' % self.hostname
            subject += '%.2f / %s' % (evt['magnitude'], evID)
            msg['Subject'] = subject
        msg['From'] = self.email_sender
        msg['To'] = self.email_recipients[0]
        utc_date = datetime.datetime.utcnow()
        utc_date.replace(tzinfo=tz.gettz('UTC'))
        msg['Date'] = utc_date.strftime("%a, %d %b %Y %T %z")
        try:
            if self.ssl:
                s = smtplib.SMTP_SSL(host=self.smtp_server,
                                     port=self.email_port, timeout=5)
            else:
                s = smtplib.SMTP(host=self.smtp_server, port=self.email_port,
                                 timeout=5)
        except Exception as e:
            seiscomp3.Logging.warning('Cannot connect to smtp server: %s' % e)
            return
        try:
            if self.tls:
                s.starttls()
            if self.auth:
                s.login(self.username, self.password)
            s.sendmail(self.email_sender, self.email_recipients, msg.as_string())
        except Exception as e:
            seiscomp3.Logging.warning('Email could not be sent: %s' % e)
        s.quit()

    def handleComment(self, comment, parentID):
        """
        Update or publish events based on incoming MVS magnitude comments.
        """
        try:
            if comment.id() == 'update':
                seiscomp3.Logging.debug("update comment received for magnitude %s " % parentID)
                magID = parentID
                orgID = self.origin_lookup[magID]
                evID = self.event_lookup[orgID]
                org = self.cache.get(seiscomp3.DataModel.Origin, orgID)
                if not org:
                    # error messages
                    not_in_cache = "Object %s not found in cache!\n" % orgID
                    not_in_cache += "Is the cache size big enough?\n"
                    not_in_cache += "Have you subscribed to all necessary message groups?"
                    seiscomp3.Logging.warning(not_in_cache)
                    return
                mag = self.cache.get(seiscomp3.DataModel.Magnitude, magID)
                if not mag:
                    # error messages
                    not_in_cache = "Object %s not found in cache!\n" % magID
                    not_in_cache += "Is the cache size big enough?\n"
                    not_in_cache += "Have you subscribed to all necessary message groups?"
                    seiscomp3.Logging.warning(not_in_cache)
                    return
                updateno = int(comment.text())
                if updateno in self.event_dict[evID]['updates'].keys():
                    if not self.event_dict[evID]['published']:
                        self.generateReport(evID)
                    else:
                        seiscomp3.Logging.info("event %s has already been published" % evID)
                else:
                    self.event_dict[evID]['updates'][updateno] = {}
                    self.event_dict[evID]['updates'][updateno]['magnitude'] = mag.magnitude().value()
                    self.event_dict[evID]['updates'][updateno]['lat'] = org.latitude().value()
                    self.event_dict[evID]['updates'][updateno]['lon'] = org.longitude().value()
                    self.event_dict[evID]['updates'][updateno]['depth'] = org.depth().value()
                    self.event_dict[evID]['updates'][updateno]['nstorg'] = org.arrivalCount()
                    self.event_dict[evID]['updates'][updateno]['nstmag'] = mag.stationCount()
                    try:
                        self.event_dict[evID]['updates'][updateno]['ts'] = \
                        mag.creationInfo().modificationTime().toString("%FT%T.%4fZ")
                        difftime = mag.creationInfo().modificationTime() - org.time().value()
                    except:
                        self.event_dict[evID]['updates'][updateno]['ts'] = \
                        mag.creationInfo().creationTime().toString("%FT%T.%4fZ")
                        difftime = mag.creationInfo().creationTime() - org.time().value()
                    self.event_dict[evID]['updates'][updateno]['diff'] = difftime
                    self.event_dict[evID]['updates'][updateno]['ot'] = \
                    org.time().value().toString("%FT%T.%4fZ")
                    seiscomp3.Logging.info("updatenumber: %d" % updateno)
                    seiscomp3.Logging.info("lat: %f; lon: %f; mag: %f; ot: %s" % \
                                           (org.latitude().value(),
                                            org.longitude().value(),
                                            mag.magnitude().value(),
                                            org.time().value().toString("%FT%T.%4fZ")))

            if comment.id() == 'likelihood':
                seiscomp3.Logging.debug("likelihood comment received for magnitude %s " % parentID)
                ep = EventParameters()
                magID = parentID
                orgID = self.origin_lookup[magID]
                evID = self.event_lookup[orgID]
                evt = self.cache.get(seiscomp3.DataModel.Event, evID)
                if evt:
                    evt.setPreferredMagnitudeID(parentID)
                    ep.add(evt)
                else:
                    seiscomp3.Logging.debug("Cannot find event %s in cache." % evID)
                org = self.cache.get(seiscomp3.DataModel.Origin, orgID)
                if org:
                    ep.add(org)
                    for _ia in xrange(org.arrivalCount()):
                        pk = self.cache.get(seiscomp3.DataModel.Pick, org.arrival(_ia).pickID())
                        if not pk:
                            seiscomp3.Logging.debug("Cannot find pick %s in cache." % org.arrival(_ia).pickID())
                        else:
                            ep.add(pk)
                else:
                    seiscomp3.Logging.debug("Cannot find origin %s in cache." % orgID)
                # if there are updates attach the likelihood to the most recent one
                if len(self.event_dict[evID]['updates'].keys()) > 0:
                    idx = sorted(self.event_dict[evID]['updates'].keys())[-1]
                    self.event_dict[evID]['updates'][idx]['likelihood'] = float(comment.text())
                    if self.udevt is not None:
                        self.udevt.send(self.udevt.message_encoder(ep))
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: seiscomp3.Logging.error(i)

    def updateObject(self, parentID, object):
        """
        Call-back function if an object is updated.
        """
        pk = seiscomp3.DataModel.Pick.Cast(object)
        if pk:
            self.handlePick(pk, parentID)

        mag = seiscomp3.DataModel.Magnitude.Cast(object)
        if mag:
            self.handleMagnitude(mag, parentID)

        event = seiscomp3.DataModel.Event.Cast(object)
        if event:
            evt = self.cache.get(seiscomp3.DataModel.Event, event.publicID())
            self.handleEvent(evt)

        comment = seiscomp3.DataModel.Comment.Cast(object)
        if comment:
            self.handleComment(comment, parentID)

    def addObject(self, parentID, object):
        """
        Call-back function if a new object is received.
        """
        pk = seiscomp3.DataModel.Pick.Cast(object)
        if pk:
            self.handlePick(pk, parentID)

        mag = seiscomp3.DataModel.Magnitude.Cast(object)
        if mag:
            self.cache.feed(mag)
            self.handleMagnitude(mag, parentID)

        origin = seiscomp3.DataModel.Origin.Cast(object)
        if origin:
            self.handleOrigin(origin, parentID)

        event = seiscomp3.DataModel.Event.Cast(object)
        if event:
            self.cache.feed(event)
            self.handleEvent(event)

        comment = seiscomp3.DataModel.Comment.Cast(object)
        if comment:
            self.handleComment(comment, parentID)

    def run(self):
        """
        Start the main loop.
        """
        seiscomp3.Logging.info("scvsmag logging is running.")
        return seiscomp3.Client.Application.run(self)

if __name__ == '__main__':
    import sys
    app = Listener(len(sys.argv), sys.argv)
    sys.exit(app())
