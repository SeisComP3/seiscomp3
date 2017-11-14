#!/usr/bin/env python
#############################################################################
# reqhandler.py                                                             #
#                                                                           #
# ArcLink request handler                                                   #
#                                                                           #
# (c) 2004 Joachim Saul, GFZ Potsdam                                        #
# (c) 2005 Andres Heinloo, GFZ Potsdam                                      #
# (c) 2006 Doreen Pahlke, GFZ Potsdam                                       #
# (c) 2007 Mathias Hoffmann, GFZ Potsdam                                    #
#                                                                           #
# This program is free software; you can redistribute it and/or modify it   #
# under the terms of the GNU General Public License as published by the     #
# Free Software Foundation; either version 2, or (at your option) any later #
# version. For more information, see http://www.gnu.org/                    #
#                                                                           #
#############################################################################

import threading
import os
import sys
import time
import datetime
from optparse import OptionParser
from seiscomp import logs, sds
from seiscomp.mseedlite import Record, Input as MSeedInput, MSeedError
from seiscomp.fseed import SEEDError
from seiscomp.db import DBError
from seiscomp.arclink.manager import *
from seiscomp.arclink.handler import *
from seiscomp.arclink import handler

from seiscomp3 import Core, Config, Client, DataModel, Logging, System
from seiscomp.db.seiscomp3 import sc3wrap
from seiscomp.db.seiscomp3.routing import Routing
from seiscomp.db.seiscomp3.inventory import Inventory

VERSION = "1.1 (2013.084)"
DEFAULT_USER = "arclink@webdc"
DEFAULT_LABEL = ""
SOCKET_TIMEOUT = 300
DOWNLOAD_RETRY = 5

class RequestThread(threading.Thread):
    lock = threading.RLock()

    class SafeVolume(object):
        def __init__(self, obj):
            self.__obj = obj

        def write(self, data):
            RequestThread.lock.acquire()
            try:
                return self.__obj.write(data)

            finally:
                RequestThread.lock.release()

        def close(self, status, error=False):
            RequestThread.lock.acquire()
            try:
                return self.__obj.close(status, error)

            finally:
                RequestThread.lock.release()

    class SafeVolumeFactory(object):
        def __init__(self, obj):
            self.__obj = obj

        def open(self, name):
            RequestThread.lock.acquire()
            try:
                return RequestThread.SafeVolume(self.__obj.open(name))

            finally:
                RequestThread.lock.release()

    def __init__(self, req, dcid, addr, volume_factory):
        threading.Thread.__init__(self)
        self.__req = req
        self.__dcid = dcid
        self.__addr = addr
        self.__volume_factory = RequestThread.SafeVolumeFactory(volume_factory)

    def run(self):
        fd = self.__volume_factory.open(self.__dcid)
        self.__req.submit(self.__addr, DEFAULT_USER, None)
        if self.__req.error:
            logs.warning("error submitting request to %s: %s" %
                (self.__dcid, self.__req.error))

            fd.close(Arclink_ERROR(message=self.__req.error))
            return

        try:
            self.__req.download_data(fd, True, False)
            logs.info("%s: request %s ready" % (self.__req.address, self.__req.id))
                
            rqstat = self.__req.status()
            for vol in rqstat.volume:
                if vol.status != STATUS_OK:
                    if vol.message:
                        fd.close(Arclink_WARN(message=vol.message))
                    else:
                        fd.close(Arclink_WARN)

                    break

            else:
                if vol.message:
                    fd.close(Arclink_OK(message=vol.message))

                else:
                    fd.close(Arclink_OK)

            self.__req.purge()
            
        except ArclinkTimeout, e:
            logs.warning("%s: %s" % (self.__req.address, str(e)))
            fd.close(Arclink_RETRY(message="timeout"))

        except ArclinkError, e:
            try:
                rqstat = self.__req.status()
                if rqstat.error:
                    logs.warning("%s: request %s failed: %s" %
                        (self.__req.address, str(rqstat.id), str(rqstat.message)))
                    
                    fd.close(Arclink_ERROR(message=rqstat.message))
                
                else:
                    logs.warning("%s: request %s returned no data (%s)" %
                        (self.__req.address, str(rqstat.id), str(e)))

                    fd.close(Arclink_NODATA)

                self.__req.purge()

            except (ArclinkError, socket.error), e:
                logs.warning("%s: error: %s" % (self.__req.address, str(e)))
                fd.close(Arclink_RETRY(message=str(e)))

        except socket.error, e:
            logs.warning("%s: error: %s" % (self.__req.address, str(e)))
            fd.close(Arclink_RETRY(message=str(e)))

class WiggleFetcher(object):
    def __init__(self, nrtdir, archdir, isodir, filedb, max_size, dcid, subnode_addr,
        dcid_override):
        self.__volume_factory = None
        self.__local_volume_fd = None
        self.__local_volume_size = 0
        self.__request_size = 0
        self.__sds = sds.SDS(nrtdir, archdir, isodir, filedb)
        self.__arcl = ArclinkManager(None, socket_timeout = SOCKET_TIMEOUT,
            download_retry = DOWNLOAD_RETRY)
        self.__max_size = max_size
        self.__dcid = dcid
        self.__subnode_addr = subnode_addr
        self.__dcid_override = dcid_override
        self.__foreign_req = {}

    def __estimate_size(self, chan, t1, t2):
        """
        Computes the estimated request size.

        @arguments: channel, start, end
        @return: The estimated request size in bytes.
        """
        samp_dict = {"B": 20, "E": 100, "H": 100, "L": 1, "S": 50, "U": 0.01, "V": 0.1, "U": 0.01}
        
        tdiff = t2 - t1
        tdiff = tdiff.days*86400+tdiff.seconds
        samp = samp_dict.get(chan.upper(), 20)
        return int(tdiff*samp*1.5)
        
    # Arclink has a concept of "data volume". The idea is that if parts of
    # request take different time to extract, the request will be split
    # into several volumes. For example, extracting data from online SDS
    # will be fast, while extracting data from archive may involve reading
    # tapes (nowadays not the case anymore), which is slow. Moreover, it
    # may increase performance if several volumes are extracted in
    # parallel. The user can start downloading data volumes before the
    # whole request is ready. If the user chooses to download the complete
    # product, then volumes are simply appended to each other.

    def start_request(self, volume_factory):
        self.__volume_factory = volume_factory
        
    # add_win() must decide into which volume the time window belongs.
    # The volume name (arbitrary identifier) is returned via the "volume"
    # attribute of status object.
    
    def add_wins(self, dcid, wins):
        """Add time windows (single expanded request line) to the request.

        dcid - data center (archive) ID 
        wins - sequence of (t1, t2, net, sta, cha, loc), where
        t1, t2 - begin and end time as datetime.datetime objects
        net, sta, cha, loc - network, station, channel (stream) and location
            codes, no wildcards

        Return value should be an object of ArclinkStatus.
        """
        
        estimated_size = 0
        for (t1, t2, net, sta, cha, loc, arch_net) in wins:
            if t1 >= t2 or t2 - t1 > datetime.timedelta(days=999):
                return Arclink_ERROR(volume=dcid,
                    message="invalid time window")
                
            estimated_size += self.__estimate_size(cha, t1, t2)
            
        try:
            dcid = self.__dcid_override[(net,sta)]

        except KeyError:
            pass
        
        if dcid not in self.__subnode_addr and self.__local_volume_fd is None:
            self.__local_volume_fd = self.__volume_factory.open(self.__dcid)
        
        if self.__request_size + estimated_size > self.__max_size:
            self.__request_size += estimated_size
            return Arclink_ERROR(volume=dcid, size=self.__request_size,
                message="maximum request size exceeded")

        if dcid in self.__subnode_addr:
            for (t1, t2, net, sta, cha, loc, arch_net) in wins:
                try:
                    req = self.__foreign_req[dcid]

                except KeyError:
                    req = self.__arcl.new_request("WAVEFORM",
                        {"format": "MSEED"})
                    
                    self.__foreign_req[dcid] = req
                
                req.add(net, sta, cha, loc, t1, t2)

            self.__request_size += estimated_size
            
            return Arclink_OK(volume=dcid, size=estimated_size,
                message="request queued for forwarding")
                
        dcid = self.__dcid
        real_size = 0

        try:
            for (t1, t2, net, sta, cha, loc, arch_net) in wins:
                if not arch_net:
                    arch_net = net

                try:
                    for rawrec in self.__sds.iterdata(t1, t2, arch_net, sta, cha, loc):
                        if arch_net != net:
                            rec = Record(rawrec)
                            rec.net = net
                            rec_len_exp = 9
                            while (1 << rec_len_exp) < rec.size:
                                rec_len_exp += 1
                
                            rec.write(self.__local_volume_fd, rec_len_exp)

                        else:
                            self.__local_volume_fd.write(rawrec)

                        real_size += len(rawrec)

                        if self.__request_size + real_size > self.__max_size:
                            self.__request_size += real_size
                            return Arclink_ERROR(volume=dcid, size=self.__request_size,
                                message="maximum request size exceeded")

                except sds.TemporaryUnavailabilityException:
                    return Arclink_RETRY(volume=dcid, size=0,
                        message="data temporarily unavailable, try again later")

        except (OSError, IOError, MSeedError, SEEDError, DBError), e:
            return Arclink_ERROR(volume=dcid, message=str(e))
        
        self.__local_volume_size += real_size
        self.__request_size += real_size

        if real_size == 0:
            return Arclink_NODATA(volume=dcid)

        return Arclink_OK(volume=dcid, size=real_size)

    def end_request(self):
        try:
            if self.__request_size > self.__max_size:
                ex = "maximum request size exceeded"
                if self.__local_volume_fd is not None:
                    if self.__local_volume_size == 0:
                        self.__local_volume_fd.close(Arclink_ERROR(message=ex))
                    else:
                        self.__local_volume_fd.close(Arclink_WARN(message=ex))
                        
                for (dcid, req) in self.__foreign_req.iteritems():
                    fd = self.__volume_factory.open(dcid)
                    fd.close(Arclink_ERROR(message=ex))
                
                return
            
            if self.__local_volume_fd is not None:
                if self.__local_volume_size == 0:
                    self.__local_volume_fd.close(Arclink_NODATA)
                else:
                    self.__local_volume_fd.close(Arclink_OK)

            dcid_req_pending = []
            for (dcid, req) in self.__foreign_req.iteritems():
                addr = self.__subnode_addr.get(dcid)
                if addr is None:
                    logs.warning("error submitting request to %s: routing failed" %
                        (dcid,))
                    
                    fd = self.__volume_factory.open(dcid)
                    fd.close(Arclink_ERROR(message="routing failed"))
                    continue
        
                thr = RequestThread(req, dcid, addr, self.__volume_factory)
                thr.start()
                dcid_req_pending.append(thr)
            
            for thr in dcid_req_pending:
                thr.join()
        
        finally:
            self.__volume_factory = None
            self.__local_volume_fd = None
            self.__local_volume_size = 0
            self.__request_size = 0
            self.__foreign_req = {}

class MessageThread(threading.Thread):
    def __init__(self, conn, rh):
        threading.Thread.__init__(self)
        self.__conn = conn
        self.__rh = rh

    def run(self):
        while True:
            msg = self.__conn.readMessage(False)
            if msg is None:
                time.sleep(1)
                continue
            
            for obj in msg:
                notifier = DataModel.Notifier_Cast(obj)
                if notifier:
                    RequestHandler.dataLock.acquire()
                    try:
                        notifier.apply()
                        self.__rh.updateInventory()

                    finally:
                        RequestHandler.dataLock.release()

class RequestHandlerApp(Client.Application):
    def __init__(self, argc, argv):
        Client.Application.__init__(self, argc, argv)

        self.setLoggingToStdErr(True)

        self.setMessagingEnabled(True)
        self.setDatabaseEnabled(True, True)

        self.setAutoApplyNotifierEnabled(False)
        self.setInterpretNotifierEnabled(False)

        self.setMessagingUsername(str(os.getpid())[:5] + "arc")
        self.setPrimaryMessagingGroup("LISTENER_GROUP")
        self.addMessagingSubscription("INVENTORY")
        self.addMessagingSubscription("ROUTING")

    def initConfiguration(self):
        if not Client.Application.initConfiguration(self):
            return False

        # Get the environment
        e = System.Environment_Instance()

        # force logging to stderr even if logging.file = 1
        self.setLoggingToStdErr(True)


        ## Those are necessary.
        try:
            self.organization = self.configGetString("organization")
        except Exception:
            logs.error("organization name not specified")
            return False

        try:
            self.reqdir = e.absolutePath(self.configGetString("request_dir"))
        except Exception:
            logs.error("request data directory not specified")
            return False

        try:
            self.dcid = self.configGetString("datacenterID")
        except Exception:
            logs.error("datacenter/archive ID not specified")
            return False

        ## From those we can supply a default
        try:
            self.trackdir = e.absolutePath(self.configGetString("reqhandler.trackdir"))
        except Exception:
            self.trackdir = None

        try:
            self.trackdb = self.configGetString("reqhandler.trackdb")
            if self.trackdb.lower() == "true":
                self.trackdb = True
            else:
                self.trackdb = False
        except Exception:
            self.trackdb = False

        try:
            self.nrtdir = e.absolutePath(self.configGetString("reqhandler.nrtdir"))
        except Exception:
            self.nrtdir = e.absolutePath("@ROOTDIR@/var/lib/archive")

        try:
            self.archdir = e.absolutePath(self.configGetString("reqhandler.archdir"))
        except Exception:
            self.archdir = "/iso_sds"

        try:
            self.isodir = e.absolutePath(self.configGetString("reqhandler.isodir"))
        except Exception:
            self.isodir = "/iso_arc"

        try:
            self.filedb = e.absolutePath(self.configGetString("reqhandler.filedb"))
        except Exception:
            self.filedb = None

        try:
            self.subnodelist = e.absolutePath(self.configGetString("reqhandler.subnodelist"))
        except Exception:
            self.subnodelist = None

        try:
            self.maxsize = self.configGetInt("reqhandler.maxsize")
        except Core.TypeConversionException:
            logs.error("invalid maximum request size")
            return False
        except Exception:
            self.maxsize = 500

        return True

    def run(self):
        try:
            logs.info("ArcLink request handler v" + VERSION + " started")
            logs.info("Configuration: ")
            logs.info("Request handler for %s (organization) at %s (Datacenter Id)" % (self.organization, self.dcid))
            logs.info("Request Dir: %s" % self.reqdir)
            logs.info("Max request size: %s" % self.maxsize)
            logs.info("Archive Dir: %s" % self.archdir)
            logs.info("ISO Dir: %s" % self.isodir)
            logs.info("NRT Dir: %s" % self.nrtdir)
            logs.info("Trackdb is %s @ %s" % (self.trackdb, self.trackdir))
            logs.info("Subnodelist: %s" % self.subnodelist)
            logs.info("File Database: %s" % self.filedb)

            subnode_addr = {}
            dcid_override = {}
            if self.subnodelist is not None:
                fd = open(self.subnodelist)
                line = fd.readline()
                while line:
                    try:
                        (dcid, addr) = line.split()
                        subnode_addr[dcid] = addr
                    
                    except ValueError:
                        (dcid, addr, net, sta) = line.split()
                        subnode_addr[dcid] = addr
                        dcid_override[(net,sta)] = dcid

                    line = fd.readline()

                fd.close()

            sc3wrap.dbQuery = self.query()
            handler.sc3App = self

            inv = Inventory(self.query().loadInventory())
            rtn = Routing(self.query().loadRouting())

            wf = WiggleFetcher(self.nrtdir, self.archdir, self.isodir, self.filedb,
                1024*1024*self.maxsize, self.dcid, subnode_addr, dcid_override)

            rh = RequestHandler(inv, rtn, wf, self.reqdir, (self.trackdir, self.trackdb),
                5, self.organization, DEFAULT_LABEL)

            mt = MessageThread(self.connection(), rh)
            mt.start()
            rh.start()

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

    ## Request handler is a "plugin" for arclink
    argv = sys.argv
    argv[0] = "arclink"
    app = RequestHandlerApp(len(argv), argv)
    sys.exit(app())
