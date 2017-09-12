#***************************************************************************** 
# handler.py
#
# ArcLink request handler library
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
# (c) 2007 Mathias Hoffmann, GFZ Potsdam (SC3+Track2DB)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import threading
import re
import os
import sys
import bz2
import time
import datetime
import fnmatch
from shutil import copyfile, copyfileobj
from tempfile import TemporaryFile, NamedTemporaryFile
from seiscomp import logs
from seiscomp.db import DBError
from seiscomp.mseedlite import Input as MSeedInput, MSeedError
from seiscomp.fseed import SEEDVolume, SEEDError

from seiscomp.db.seiscomp3.routing import Routing as _Routing
from seiscomp.db.seiscomp3.inventory import Inventory as _Inventory
from seiscomp.db.seiscomp3.qc import QualityControl as _QualityControl
from seiscomp.db.seiscomp3 import sc3wrap

from seiscomp3 import DataModel, Core, IO

_VERSEED_BIN = "/usr/local/bin/verseed"

_INPUT_FD  = 62
_OUTPUT_FD = 63
_LINESIZE  = 1024

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

class ArclinkHandlerError(Exception):
    pass

class ArclinkStatus(object):
    _keyword = None
    volume = None
    message = None
    size = None

    def __init__(self, volume = None, message = None, size = None):
        self.volume = volume
        self.message = message
        self.size = size

    def __eq__(self, other):
        return isinstance(self, other)
    
    def __ne__(self, other):
        return not isinstance(self, other)
    
class Arclink_OK(ArclinkStatus):
    _keyword = "OK"
    pass

class Arclink_WARN(ArclinkStatus):
    _keyword = "WARN"
    pass

class Arclink_ERROR(ArclinkStatus):
    _keyword = "ERROR"
    pass

class Arclink_RETRY(ArclinkStatus):
    _keyword = "RETRY"
    pass

class Arclink_DENIED(ArclinkStatus):
    _keyword = "DENIED"
    pass

class Arclink_NODATA(ArclinkStatus):
    _keyword = "NODATA"
    pass

class _Logs(object):
    def __init__(self, verbosity):
        self.__verbosity = verbosity
        self.__req_id = None
    
    def __logmsg(self, func, msg):
        func("[" + str(self.__req_id) + "] " + msg)
    
    def set_id(self, req_id):
        self.__req_id = req_id
    
    def debug_out(self, msg):
        if self.__verbosity >= 2:
            self.__logmsg(logs.debug, "< " + msg)

    def debug_in(self, msg):
        if self.__verbosity >= 2:
            self.__logmsg(logs.debug, "> " + msg)

    def nodebug(self, msg):
        if self.__verbosity == 1:
            self.__logmsg(logs.info, msg)
    
    def info(self, msg):
        if self.__verbosity >= 1:
            self.__logmsg(logs.info, msg)

    def notice(self, msg):
        self.__logmsg(logs.notice, msg)

    def warning(self, msg):
        self.__logmsg(logs.warning, msg)

    def error(self, msg):
        self.__logmsg(logs.error, msg)

class _Responder(object):
    def __init__(self, fd, logs):
        self.__fd = fd
        self.__logs = logs

    def __send_cmd(self, cmd):
        self.__logs.debug_out(cmd)
        self.__fd.write(cmd + "\n")
        self.__fd.flush()
    
    def __send_status_line(self, ref, kwd):
        if isinstance(ref, basestring):
            self.__send_cmd("STATUS VOLUME %s %s" % (ref, kwd))
        else:
            self.__send_cmd("STATUS LINE %s %s" % (ref, kwd))

    def __set_status(self, ref, kwd, size, msg):
        if msg is not None:
            self.__logs.nodebug(msg)
            self.__send_status_line(ref, "MESSAGE %s" % (msg,))

        if size is not None:
            self.__send_status_line(ref, "SIZE %d" % (size,))

        self.__send_status_line(ref, kwd)
    
    def set_id(self, req_id):
        self.__req_id = req_id
    
    def processing(self, ref, vol_id):
        self.__send_status_line(ref, "PROCESSING %s" % (vol_id,))

    def cancel(self, ref):
        self.__send_status_line(ref, "CANCEL")

    def set_status(self, status, size = None, msg = None, ref = None):
        if size is None:
            size = status.size
        
        if msg is None:
            msg = status.message

        if ref is None:
            ref = status.volume

        self.__set_status(ref, status._keyword, size, msg)

    def req_message(self, msg):
        self.__logs.nodebug(msg)
        self.__send_cmd("MESSAGE %s" % (msg,))

    def req_cancel(self):
        self.__send_cmd("CANCEL")

    def req_error(self):
        self.__send_cmd("ERROR")

    # BIANCHI|ENCRYPTION: Reply arclink server saying that the request contains
    # restricted data.
    def req_restricted(self):
        self.__send_cmd("RESTRICTED")

    def req_end(self):
        self.__send_cmd("END")

_rx_verseed_error = re.compile("Error", re.M | re.I)



# original RequestTracker: write logs to reqtrack directory
class _RequestTrackerDIR(object):
    def __init__(self, dir, req_id, req_type, user, header, label, user_ip, client_ip):
        if not dir:
            self.__dir = None
            return

        self.__dir = dir + "/" + user + "/" + req_id + "." + req_type
            
        if label:    
            self.__dir += ("." + label)
        
        if not os.path.exists(self.__dir):
            os.makedirs(self.__dir)

        fd = open(self.__dir + "/" + "status", "a")
        print >>fd, "USER " + user

        if user_ip:
            print >>fd, "USER_IP " + user_ip

        if client_ip:
            print >>fd, "CLIENT_IP " + client_ip

        if label:
            print >>fd, "LABEL " + label

        print >>fd, header
        fd.close()

    def line_status(self, start_time, end_time, network, station, channel,
        location, restricted, net_class, shared, constraints, volume, status, size, message):
        
        if not self.__dir:
            return
        
        if network is None or network == "":
            network = "."

        if station is None or station == "":
            station = "."

        if channel is None or channel == "":
            channel = "."

        if location is None or location == "":
            location = "."

        if volume is None:
            volume = "NODATA"

        if size is None:
            size = 0

        if message is None:
            message = ""

        if isinstance(start_time, basestring):
            stime = start_time
        else:
            stime = "%d,%d,%d,%d,%d,%d" % (start_time.year, start_time.month,
                start_time.day, start_time.hour, start_time.minute,
                start_time.second)

        if isinstance(end_time, basestring):
            etime = end_time
        else:
            etime = "%d,%d,%d,%d,%d,%d" % (end_time.year, end_time.month,
                end_time.day, end_time.hour, end_time.minute,
                end_time.second)

        if isinstance(constraints, list):
            constr = " ".join(constraints)
        else:
            constr = " ".join([ a+"="+b for (a, b) in constraints.iteritems() ])

        fd = open(self.__dir + "/" + "status", "a")
        print >>fd, "%s %s %s %s %s %s (%s) %s %s %d %s" % (stime, etime,
            network, station, channel, location, constr,
            volume, status, size, message)
            
        fd.close()
    
    def volume_status(self, volume, status, size, message):
        if not self.__dir:
            return
        
        if volume is None:
            volume = "NODATA"

        if size is None:
            size = 0

        if message is None:
            message = ""

        fd = open(self.__dir + "/" + "status", "a")
        print >>fd, "VOLUME %s %s %d %s" % (volume, status, size, message)
        fd.close()

    def request_status(self, status, message):
        if not self.__dir:
            return
        
        if message is None:
            message = ""

        fd = open(self.__dir + "/" + "status", "a")
        print >>fd, "%s %s" % (status, message)
        fd.close()
    
    def __verseed_errors(self, volume):
        fd = open(self.__dir + "/" + volume + ".verseed")

        try:
            return _rx_verseed_error.search(fd.read()) is not None

        finally:
            fd.close()
    
    def verseed(self, volume, file):
        if not self.__dir:
            return
        
        if not os.path.exists(_VERSEED_BIN):
            return

        os.system("%s -4 %s >>\"%s\" 2>&1" % (_VERSEED_BIN,
            file.replace('"', '\\"').replace('$', '\\$'),
            self.__dir + "/" + volume + ".verseed"))
            
        if self.__verseed_errors(volume):
            copyfile(file, self.__dir + "/" + volume + ".data")



def enableNotifier(f):
    def wrap(*args, **kwargs):
        saveState = DataModel.Notifier.IsEnabled()
        DataModel.Notifier.SetEnabled(True)
        f(*args, **kwargs)
        DataModel.Notifier.SetEnabled(saveState)
    return wrap


# new RequestTracker: generate data-base logging entries
class _RequestTrackerDB(object):

    @enableNotifier
    def __init__(self, dir, req_id, req_type, user, header, label, user_ip, client_ip):
        self.arclinkRequest = DataModel.ArclinkRequest.Create()
        self.arclinkRequest.setCreated(Core.Time.GMT())
        self.arclinkRequest.setRequestID(req_id)
        self.arclinkRequest.setUserID(user) # HACK
        if user_ip: self.arclinkRequest.setUserIP(user_ip)
        if client_ip: self.arclinkRequest.setClientIP(client_ip)
        self.arclinkRequest.setType(req_type)
        self.arclinkRequest.setLabel(str(label))
        self.arclinkRequest.setHeader(header)

        self.averageTimeWindow = Core.TimeSpan(0.)
        self.totalLineCount = 0
        self.okLineCount = 0

        al = DataModel.ArclinkLog()
        al.add(self.arclinkRequest)



    @enableNotifier
    def send(self):
        msg = DataModel.Notifier.GetMessage(True)
        if msg:
            sc3App.connection().send("LOGGING", msg)



    @enableNotifier
    def line_status(self, start_time, end_time, network, station, channel,
        location, restricted, net_class, shared, constraints, volume, status, size, message):
        if network is None or network == "":
            network = "."
        if station is None or station == "":
            station = "."
        if channel is None or channel == "":
            channel = "."
        if location is None or location == "":
            location = "."
        if volume is None:
            volume = "NODATA"
        if size is None:
            size = 0
        if message is None:
            message = ""

        if isinstance(start_time, basestring):
            stime = start_time
        else:
            stime = "%d,%d,%d,%d,%d,%d" % (start_time.year, start_time.month,
                start_time.day, start_time.hour, start_time.minute,
                start_time.second)
        startTime = Core.Time.FromString(stime, "%Y,%m,%d,%H,%M,%S")
        if isinstance(end_time, basestring):
            etime = end_time
        else:
            etime = "%d,%d,%d,%d,%d,%d" % (end_time.year, end_time.month,
                end_time.day, end_time.hour, end_time.minute,
                end_time.second)
        endTime = Core.Time.FromString(etime, "%Y,%m,%d,%H,%M,%S")

        if isinstance(constraints, list):
            constr = " ".join(constraints)
        else:
            constr = " ".join([ a+"="+b for (a, b) in constraints.iteritems() ])

        arclinkRequestLine = DataModel.ArclinkRequestLine()
        arclinkRequestLine.setStart(startTime)
        arclinkRequestLine.setEnd(endTime)
        arclinkRequestLine.setStreamID(DataModel.WaveformStreamID(network[:8], station[:8], location[:8], channel[:8], ""))
        arclinkRequestLine.setConstraints(constr)
        if isinstance(restricted, bool): arclinkRequestLine.setRestricted(restricted)
        arclinkRequestLine.setNetClass(net_class)
        if isinstance(shared, bool):arclinkRequestLine.setShared(shared)
        #
        arclinkStatusLine = DataModel.ArclinkStatusLine()
        arclinkStatusLine.setVolumeID(volume)
        arclinkStatusLine.setStatus(status)
        arclinkStatusLine.setSize(size)
        arclinkStatusLine.setMessage(message)
        #
        arclinkRequestLine.setStatus(arclinkStatusLine)
        self.arclinkRequest.add(arclinkRequestLine)

        self.averageTimeWindow += endTime - startTime
        self.totalLineCount += 1
        if status == "OK": self.okLineCount += 1


    @enableNotifier
    def volume_status(self, volume, status, size, message):
        if volume is None:
            volume = "NODATA"
        if size is None:
            size = 0
        if message is None:
            message = ""

        arclinkStatusLine = DataModel.ArclinkStatusLine()
        arclinkStatusLine.setVolumeID(volume)
        arclinkStatusLine.setStatus(status)
        arclinkStatusLine.setSize(size)
        arclinkStatusLine.setMessage(message)
        self.arclinkRequest.add(arclinkStatusLine)


    @enableNotifier
    def request_status(self, status, message):
        if message is None:
            message = ""

        self.arclinkRequest.setStatus(status)
        self.arclinkRequest.setMessage(message)

        ars = DataModel.ArclinkRequestSummary()
        tw = self.averageTimeWindow.seconds()
        if self.totalLineCount > 0:
            tw = self.averageTimeWindow.seconds() / self.totalLineCount # avarage request time window
        if tw > 2**32: tw = -1 # prevent 32bit int overflow
        ars.setAverageTimeWindow(tw)
        ars.setTotalLineCount(self.totalLineCount)
        ars.setOkLineCount(self.okLineCount)
        self.arclinkRequest.setSummary(ars)

        self.send()


    def __verseed_errors(self, volume):
        pass

    def verseed(self, volume, file):
        pass




# execute multiple RequestTrackers
class _RequestTracker(object):
    def __init__(self, (dir, db), req_id, req_type, user, header, label, user_ip, client_ip):
        self.trackerList = list()
        if db: self.trackerList.append(_RequestTrackerDB(db, req_id, req_type, user, header, label, user_ip, client_ip))
        if dir: self.trackerList.append(_RequestTrackerDIR(dir, req_id, req_type, user, header, label, user_ip, client_ip))

    def __getattr__(self, name):
        self.name = name
        return self.call

    def call(self, *args, **kwargs):
        [ eval("func."+self.name+"(*args, **kwargs)") for func in self.trackerList ]




class _Request(object):
    def __init__(self, resp, logs):
        self._resp = resp
        self._logs = logs
        self._line = 0
        self._errors = 0

    def line_error(self, errmsg):
        self._resp.processing(self._line, "ERROR")
        self._resp.set_status(Arclink_ERROR, msg=errmsg, ref=self._line)
        self._resp.set_status(Arclink_ERROR, ref="ERROR")
        self._line += 1
        self._errors += 1

class _FSEED_Volume(object):
    def __init__(self, name, fd, req, tracker, inventory, organization, label,
        resp_dict):
        self.__fd = fd
        self.__name = name
        self.__req = req
        self.__tracker = tracker
        self.__inv = inventory
        self.__org = organization
        self.__label = label
        self.__resp_dict = resp_dict
        self.__mseed_fd = TemporaryFile()
        self.status = None

        # strip webinterface UUID
        if label.startswith("WI:"):
            try:
                self.__label = label.split(':', 2)[2]

            except IndexError:
                pass

    def write(self, data):
        self.__mseed_fd.write(data)

    def close(self, status, error=False):
        try:
            self.status = status
            tmpfd = None

            if not error:
                tmpfd = NamedTemporaryFile()

                try:
                    seed_volume = SEEDVolume(self.__inv, self.__org,
                        self.__label, self.__resp_dict)

                    self.__mseed_fd.seek(0)
                    for rec in MSeedInput(self.__mseed_fd):
                        seed_volume.add_data(rec)

                    seed_volume.output(tmpfd)
                    tmpfd.seek(0)
                    copyfileobj(tmpfd, self.__fd)
                    self.__tracker.verseed(self.__name, tmpfd.name)

                except (MSeedError, SEEDError, DBError), e:
                    self.status = Arclink_ERROR(message=str(e))
                
        finally:
            self.__fd.close()
            self.__mseed_fd.close()
            
            if tmpfd is not None:
                tmpfd.close()
            
            self.__req.close_volume_callback(self.__name, self)

class _MSEED_Volume(object):
    def __init__(self, name, fd, req):
        self.__name = name
        self.__req = req
        self.__fd = fd
        self.status = None

    def write(self, data):
        self.__fd.write(data)

    def close(self, status, error=False):
        try:
            self.status = status

        finally:
            self.__fd.close()
            self.__req.close_volume_callback(self.__name, self)

class _WaveformVolumeFactory(object):
    def __init__(self, req, tracker, fileprefix, format, compression,
        inventory, organization, label, resp_dict):
        self.__req = req
        self.__tracker = tracker
        self.__fileprefix = fileprefix
        self.__format = format
        self.__compression = compression
        self.__inv = inventory
        self.__org = organization
        self.__label = label
        self.__resp_dict = resp_dict
    
    def open(self, name):
        if self.__compression == "bzip2":
            fd = bz2.BZ2File(self.__fileprefix + name, "w")
        else:
            fd = file(self.__fileprefix + name, "w")

        if self.__format == "FSEED":
            vol = _FSEED_Volume(name, fd, self.__req, self.__tracker,
                self.__inv, self.__org, self.__label, self.__resp_dict)
        else:
            vol = _MSEED_Volume(name, fd, self.__req)

        return vol

class _WaveformRequest(_Request):
    def __init__(self, rtn, resp, logs, req_dir, req_id, req_args, tracker,
        user, wf, inv, organization, label):
        _Request.__init__(self, resp, logs)
        self.__tracker = tracker
        self.__user = user
        self.__rtn = rtn
        self.__wf = wf
        self.__inv = inv
        self.__org = organization
        self.__label = label
        self.__fileprefix = req_dir + "/" + str(req_id) + "."
        self.__not_reported = True

        format = req_args.get("format", "FSEED")
        if format != "FSEED" and format != "MSEED":
            resp.req_message("format '%s' is unknown, defaulting to FSEED" %
                (format,))
            format = "FSEED"

        compression = req_args.get("compression", "none")
        if compression != "bzip2" and compression != "none":
            resp.req_message("compression '%s' is unknown, defaulting to none" %
                (compression,))
            compression = "none"
        
        resp_dict = (req_args.get("resp_dict", "true") == "true")
        
        try:
            self.__volume_factory = _WaveformVolumeFactory(self, self.__tracker,
                self.__fileprefix, format, compression, inv,
                organization, label, resp_dict)
        
            wf.start_request(self.__volume_factory)

        except (OSError, IOError), e:
            raise ArclinkHandlerError, str(e)

    def __check_access(self, net_code, sta_code, str_code, loc_code,
        start_time, end_time):

        routing = self.__rtn
        
        RequestHandler.dataLock.acquire()
        try:
            routing.load_access(net_code, sta_code, str_code, loc_code,
                start_time, end_time)
        
        finally:
            RequestHandler.dataLock.release()
        
        for acc_net in routing.access.itervalues():
            for acc_sta in acc_net.itervalues():
                for acc_loc in acc_sta.itervalues():
                    for acc_strm in acc_loc.itervalues():
                        for acc_user in acc_strm.itervalues():
                            for acc in acc_user.itervalues():
                                if ':' in acc.user:
                                    # for fdsnws only
                                    continue

                                if (self.__user.upper() == acc.user.upper() or \
                                        (acc.user[:1] == '@' and self.__user[:1] != '@' and \
                                            self.__user.upper().endswith(acc.user.upper()))) and \
                                    start_time >= acc.start and \
                                    (acc.end is None or end_time <= acc.end):
                                    return True

        return False
    
    def add_line(self, start_time, end_time, network, station, channel,
        location, constraints):
        
        # Check for allowed wildcards
        if network is None or station is None or channel is None or \
            network.find("*") != -1 or station.find("*") != -1:
            raise ArclinkHandlerError, "incorrectly formulated request"

        # QC constraints
        consdict = dict([cons for cons in constraints.iteritems() 
                        if (cons[0][-4:] == "_min" or cons[0][-4:] == "_max") and cons[1].strip()])

        if location is None and channel is not None:
            location = ""
        
        # temporary hack for SC3
        channel = channel.replace('?', '*')
        
        try:
            # Expand wildcards and check access rights
            access_denied = False
            dcid = None
            expanded = set()
            inv = _Inventory(self.__inv.obj)
            shared = False
            restricted = False
            net_class = ""

            RequestHandler.dataLock.acquire()
            try:
                inv.load_stations(network, station, channel, location,
                    start_time, end_time, qc_constraints=consdict)

            finally:
                RequestHandler.dataLock.release()

            for net in sum([i.values() for i in inv.network.itervalues()], []):
                dcid = net.archive
                net_class = net.netClass
                for sta in sum([i.values() for i in net.station.itervalues()], []):
                    for loc in sum([i.values() for i in sta.sensorLocation.itervalues()], []):
                        if not fnmatch.fnmatchcase(loc.code, location):
                            continue

                        for strm in sum([i.values() for i in loc.stream.itervalues()], []):
                            if not fnmatch.fnmatchcase(strm.code, channel):
                                continue
                            restricted |= strm.restricted or sta.restricted or net.restricted
                            shared |= strm.shared or sta.shared or net.shared

                            if strm.restricted and \
                                not self.__check_access(net.code, sta.code,
                                strm.code, loc.code, start_time, end_time):
                                access_denied = True
                                continue

                            # Check and report restrictions
                            if strm.restricted and self.__not_reported:
                                self._resp.req_restricted()
                                self.__not_reported = False

                            expanded.add((start_time, end_time, net.code, sta.code,
                                strm.code, loc.code, sta.archiveNetworkCode))

                        for strm in sum([i.values() for i in loc.auxStream.itervalues()], []):
                            if not fnmatch.fnmatchcase(strm.code, channel):
                                continue
                            restricted |= strm.restricted
                            #shared |= strm.shared

                            if strm.restricted and \
                                not self.__check_access(net.code, sta.code,
                                strm.code, loc.code, start_time, end_time):
                                access_denied = True
                                continue

                            # Check and report restrictions
                            if strm.restricted and self.__not_reported:
                                self._resp.req_restricted()
                                self.__not_reported = False


                            expanded.add((start_time, end_time, net.code, sta.code,
                                strm.code, loc.code, sta.archiveNetworkCode))

            RequestHandler.dataLock.acquire()
            try:
                self.__rtn.clear_access()

            finally:
                RequestHandler.dataLock.release()

            if len(expanded) == 0:
                self._resp.processing(self._line, "NODATA")
                self._resp.set_status(Arclink_NODATA, ref="NODATA")

                if access_denied:
                    self._resp.set_status(Arclink_DENIED, ref=self._line,
                        msg="access denied for user " + self.__user)
                        
                    self.__tracker.line_status(start_time, end_time, network,
                        station, channel, location, restricted, net_class, shared, constraints, "NODATA",
                        "DENIED", 0, "access denied for user " + self.__user)

                elif len(consdict) > 0:
                    self._resp.set_status(Arclink_NODATA, ref=self._line,
                        msg="metadata not found in the database or QC constraints not satisfied")

                    self.__tracker.line_status(start_time, end_time, network,
                        station, channel, location, restricted, net_class, shared, constraints, "NODATA",
                        "NODATA", 0, "metadata not found in the database or QC constraints not satisfied")

                else:
                    self._resp.set_status(Arclink_NODATA, ref=self._line,
                        msg="metadata not found in the database")

                    self.__tracker.line_status(start_time, end_time, network,
                        station, channel, location, restricted, net_class, shared, constraints, "NODATA",
                        "NODATA", 0, "metadata not found in the database")

                self._line += 1
                return

            # Load inventory for full SEED
            RequestHandler.dataLock.acquire()
            try:
                self.__inv.load_stations(network, station, channel, location,
                    start_time, end_time)

            finally:
                RequestHandler.dataLock.release()

            status = self.__wf.add_wins(dcid, list(expanded))

        except (OSError, IOError, SEEDError, DBError), e:
            raise ArclinkHandlerError, str(e)

        if status.volume is not None:
            self._resp.processing(self._line, status.volume)

        self._resp.set_status(status, ref=self._line)
        self._line += 1

        self.__tracker.line_status(start_time, end_time, network,
            station, channel, location, restricted, net_class, shared, constraints, status.volume,
            status._keyword, status.size, status.message)
 
    def close_volume_callback(self, name, vol):
        size = None
        if vol.status == Arclink_OK or vol.status == Arclink_WARN:
            try:
                size = os.stat(self.__fileprefix + name).st_size
            except OSError, e:
                self._logs.error(str(e))
                self._resp.set_status(Arclink_NODATA, msg=str(e), ref=name)
                self.__tracker.volume_status(name, "NODATA", 0, str(e))
                return
        
        self._resp.set_status(vol.status, size, ref=name)
        self.__tracker.volume_status(name, vol.status._keyword, size,
            vol.status.message)

    def execute(self):
        try:
            self.__wf.end_request()
                
        finally:
            RequestHandler.dataLock.acquire()
            try:
                self.__inv.clear_stations()

            finally:
                RequestHandler.dataLock.release()
    
class _ResponseRequest(_Request):
    def __init__(self, resp, logs, req_dir, req_id, req_args, tracker, inv,
        organization, label):
        _Request.__init__(self, resp, logs)
        self.__req_dir = req_dir
        self.__req_id = req_id
        self.__tracker = tracker
        self.__inv = inv
        self.__org = organization
        self.__label = label

        self.__compression = req_args.get("compression", "none")
        if self.__compression != "bzip2" and self.__compression != "none":
            resp.req_message("compression '%s' is unknown, defaulting to none" %
                (self.__compression,))
            self.__compression = "none"
        
        self.__resp_dict = (req_args.get("resp_dict", "true") == "true")

    def add_line(self, start_time, end_time, network, station, channel,
        location, constraints):
        
        # Check for allowed wildcards and constraints
        if network is None or station is None or channel is None or \
            network.find("*") != -1 or len(constraints) != 0:
            raise ArclinkHandlerError, "incorrectly formulated request"

        if location is None and channel is not None:
            location = ""
        
        self._resp.processing(self._line, "dataless")

        try:
            RequestHandler.dataLock.acquire()
            try:
                self.__inv.load_stations(network, station, channel, location,
                    start_time, end_time)
            
            finally:
                RequestHandler.dataLock.release()
            
        except DBError, e:
            raise ArclinkHandlerError, str(e)

        self._resp.set_status(Arclink_OK, ref=self._line)
        self._line += 1

        self.__tracker.line_status(start_time, end_time, network,
            station, channel, location, "", "", "", constraints, "dataless",
            "OK", 0, "")

    def execute(self):
        try:
            if self._line == self._errors:
                return

            try:
                warn_msg = None
                seed_volume = SEEDVolume(self.__inv, self.__org, self.__label, self.__resp_dict)
                for net in sum([i.values() for i in self.__inv.network.itervalues()], []):
                    for sta in sum([i.values() for i in net.station.itervalues()], []):
                        for loc in sum([i.values() for i in sta.sensorLocation.itervalues()], []):
                            for strm in sum([i.values() for i in loc.stream.itervalues()], []):
                                try:
                                    seed_volume.add_chan(net.code, sta.code, loc.code,
                                        strm.code, strm.start, strm.end)
                                
                                except SEEDError, e:
                                    warn_msg = str(e)


                filename = self.__req_dir + "/" + str(self.__req_id) + ".dataless"
                if self.__compression == "bzip2":
                    fd = bz2.BZ2File(filename, "w")
                    try:
                        tmpfd = NamedTemporaryFile()
                        try:
                            seed_volume.output(tmpfd)
                            tmpfd.seek(0)
                            copyfileobj(tmpfd, fd)
                            self.__tracker.verseed("dataless", tmpfd.name)
                        finally:
                            tmpfd.close()
                    finally:
                        fd.close()
                else:
                    fd = file(filename, "w")
                
                    try:
                        seed_volume.output(fd)
                        self.__tracker.verseed("dataless", filename)
                    finally:
                        fd.close()

                size = os.stat(filename).st_size

                if warn_msg is None:
                    self._resp.set_status(Arclink_OK, size, ref="dataless")
                    self.__tracker.volume_status("dataless", "OK", size, "")
                
                else:
                    self._resp.set_status(Arclink_WARN, size, warn_msg, "dataless")
                    self.__tracker.volume_status("dataless", "WARN", size, warn_msg)

            except (SEEDError, OSError, IOError), e:
                self._resp.set_status(Arclink_ERROR, msg=str(e), ref="dataless")
                self.__tracker.volume_status("dataless", "ERROR", 0, str(e))

        finally:
            RequestHandler.dataLock.acquire()
            try:
                self.__inv.clear_stations()

            finally:
                RequestHandler.dataLock.release()

class _InventoryRequest(_Request):
    def __init__(self, resp, logs, req_dir, req_id, req_args, tracker, inv):
        _Request.__init__(self, resp, logs)
        self.__req_dir = req_dir
        self.__req_id = req_id
        self.__tracker = tracker
        self.__inv = inv

        instr = req_args.get("instruments", "false")
        if instr == "true":
            self.__instr = 1
        else:
            self.__instr = 0

        self.__compression = req_args.get("compression", "none")
        if self.__compression != "bzip2" and self.__compression != "none":
            resp.req_message("compression '%s' is unknown, defaulting to none" %
                (self.__compression,))
            self.__compression = "none"

        modified_after = req_args.get("modified_after")
        self.__modified_after = None
        if modified_after is not None:
            try:
                self.__modified_after = _parse_datetime(modified_after)
            except ValueError:
                resp.req_message("invalid time, ignoring modified_after")
        
        self.__self_dcid = req_args.get("self_dcid")
        self.__sync_dcid = req_args.get("sync_dcid")
        
    def add_line(self, start_time, end_time, network, station, channel,
        location, constraints):
        latmin = constraints.get("latmin")
        latmax = constraints.get("latmax")
        lonmin = constraints.get("lonmin")
        lonmax = constraints.get("lonmax")
        sensortype = constraints.get("sensortype")
        permanent = constraints.get("permanent")
        restricted = constraints.get("restricted") 

        # QC constraints
        consdict = dict([cons for cons in constraints.iteritems() 
                        if (cons[0][-4:] == "_min" or cons[0][-4:] == "_max") and cons[1].strip()])

        if location is None and channel is not None:
            location = ""
        
        self._resp.processing(self._line, "inventory")

        try:
            if latmin is not None:
                latmin = float(latmin)
            
            if latmax is not None:
                latmax = float(latmax)
            
            if lonmin is not None:
                lonmin = float(lonmin)

            if lonmax is not None:
                lonmax = float(lonmax)

        except ValueError:
            raise ArclinkHandlerError, "incorrectly formulated request"
        
        if permanent == "true":
            permanent = True
        elif permanent == "false":
            permanent = False
        else:
            permanent = None

        if restricted == "true":
            restricted = True
        elif restricted == "false":
            restricted = False
        else:
            restricted = None

        try:
            RequestHandler.dataLock.acquire()
            try:
                self.__inv.load_stations(network, station, channel, location,
                                        start_time, end_time, sensortype, latmin, latmax, lonmin,
                                        lonmax, permanent, restricted, self.__modified_after,
                                        self.__self_dcid, self.__sync_dcid, qc_constraints = consdict)

            finally:
                RequestHandler.dataLock.release()
            
        except DBError, e:
            raise ArclinkHandlerError, str(e)

        self._resp.set_status(Arclink_OK, ref=self._line)
        self._line += 1

        self.__tracker.line_status(start_time, end_time, network,
            station, channel, location, "", "", "", constraints, "inventory",
            "OK", 0, "")

    def execute(self):
        try:
            if self._line == self._errors:
                return

            try:
                filename = self.__req_dir + "/" + str(self.__req_id) + ".inventory"
                if self.__compression == "bzip2":
                    fd = bz2.BZ2File(filename, "w")
                else:
                    fd = file(filename, "w")
                
                try:
                    self.__inv.save_xml(fd, instr = self.__instr,
                        modified_after = self.__modified_after)
                    
                finally:
                    fd.close()

                size = os.stat(filename).st_size
                self._resp.set_status(Arclink_OK, size, ref="inventory")
                self.__tracker.volume_status("inventory", "OK", size, "")

            except (DBError, OSError, IOError), e:
                self._resp.set_status(Arclink_ERROR, msg=str(e), ref="inventory")
                self.__tracker.volume_status("inventory", "ERROR", 0, str(e))

        finally:
            RequestHandler.dataLock.acquire()
            try:
                self.__inv.clear_stations()

            finally:
                RequestHandler.dataLock.release()

class _RoutingRequest(_Request):
    def __init__(self, resp, logs, req_dir, req_id, req_args, tracker, rtn):
        _Request.__init__(self, resp, logs)
        self.__req_dir = req_dir
        self.__req_id = req_id
        self.__tracker = tracker
        self.__routing = rtn

        self.__compression = req_args.get("compression", "none")
        if self.__compression != "bzip2" and self.__compression != "none":
            resp.req_message("compression '%s' is unknown, defaulting to none" %
                (self.__compression,))
            self.__compression = "none"

        modified_after = req_args.get("modified_after")
        self.__modified_after = None
        if modified_after is not None:
            try:
                self.__modified_after = _parse_datetime(modified_after)
            except ValueError:
                resp.req_message("invalid time, ignoring modified_after")
        
    def add_line(self, start_time, end_time, network, station, channel,
        location, constraints):

        if network is None:
            network = ""

        if station is None:
            station = ""
        
        if channel is None:
            channel = "*"
        
        if location is None:
            location = ""
        
        self._resp.processing(self._line, "routing")

        try:
            RequestHandler.dataLock.acquire()
            try:
                self.__routing.load_routes(network, station, channel, location, start_time, end_time)

            finally:
                RequestHandler.dataLock.release()

        except DBError, e:
            raise ArclinkHandlerError, str(e)

        self._resp.set_status(Arclink_OK, ref=self._line)
        self._line += 1

        # where the route points to
        try: route = self.__routing.route[network][station][location][channel].arclink.keys()[::-1]
        except KeyError:
            try: route = self.__routing.route[network][station][location][""].arclink.keys()[::-1]
            except KeyError:
                try: route = self.__routing.route[network][station][""][""].arclink.keys()[::-1]
                except KeyError:
                    try: route = self.__routing.route[network][""][""][""].arclink.keys()[::-1]
                    except KeyError:
                        try: route = self.__routing.route[""][""][""][""].arclink.keys()[::-1]
                        except KeyError: route = ""

        self.__tracker.line_status(start_time, end_time, network,
            station, channel, location, "", "", "", constraints, "routing",
            "OK", 0, " ".join(route))

    def execute(self):
        try:
            if self._line == self._errors:
                return

            try:
                filename = self.__req_dir + "/" + str(self.__req_id) + ".routing"
                if self.__compression == "bzip2":
                    fd = bz2.BZ2File(filename, "w")
                else:
                    fd = file(filename, "w")
                        
                try:
                    self.__routing.save_xml(fd, modified_after = self.__modified_after)

                finally:
                    fd.close()

                size = os.stat(filename).st_size
                self._resp.set_status(Arclink_OK, size, ref="routing")
                self.__tracker.volume_status("routing", "OK", size, "")
                
            except (DBError, OSError, IOError), e:
                self._resp.set_status(Arclink_ERROR, msg=str(e), ref="routing")
                self.__tracker.volume_status("routing", "ERROR", 0, str(e))

        finally:
            RequestHandler.dataLock.acquire()
            try:
                self.__routing.clear_routes()

            finally:
                RequestHandler.dataLock.release()

class _QCRequest(_Request):
    def __init__(self, resp, logs, req_dir, req_id, req_args, tracker):
        _Request.__init__(self, resp, logs)
        self.__req_dir = req_dir
        self.__req_id = req_id
        self.__tracker = tracker
        self.__qc = _QualityControl(DataModel.QualityControl())

        self.__compression = req_args.get("compression", "none")
        if self.__compression != "bzip2" and self.__compression != "none":
            resp.req_message("compression '%s' is unknown, defaulting to none" %
                (self.__compression,))
            self.__compression = "none"

        parameters = req_args.get("parameters")
        if parameters is not None:
            self.__parameters = [p.strip().replace("_"," ") for p in parameters.split(',')]

        self.__logs = (req_args.get("logs") == "true")
        self.__outages = (req_args.get("outages") == "true")
        
    def __load_logs(self, start_time, end_time, network, station, channel, location):
        pass
    
    # TEST too
    def __load_outages(self, start_time, end_time, network, station, channel, location):
        wfid = DataModel.WaveformStreamID(network, station, location, channel, "")
        
        it = sc3wrap.dbQuery.getOutage(wfid, Core.Time.FromString(str(start_time), "%Y-%m-%d %H:%M:%S"), 
                                    Core.Time.FromString(str(end_time), "%Y-%m-%d %H:%M:%S"))
        while it.get():
            outage = DataModel.Outage.Cast(it.get())
            self.__qc.obj.add(outage)
            it.step()
    
    # TEST
    def __load_wfq(self, start_time, end_time, network, station, channel, location):
        wfid = DataModel.WaveformStreamID(network, station, location, channel, "")
        
        for p in self.__parameters:
            it = sc3wrap.dbQuery.getWaveformQuality(wfid,p,Core.Time.FromString(str(start_time),"%Y-%m-%d %H:%M:%S"),Core.Time.FromString(str(end_time),"%Y-%m-%d %H:%M:%S"))

            while it.get():
                wfq = DataModel.WaveformQuality.Cast(it.get())
                self.__qc.obj.add(wfq)
                it.step()

    def add_line(self, start_time, end_time, network, station, channel,
        location, constraints):

        if network is None:
            network = ""

        if station is None:
            station = ""
        
        if channel is None:
            channel = ""

        if location is None:
            location = ""
        
        self._resp.processing(self._line, "qc")

        try:
            RequestHandler.dataLock.acquire()
            try:
                if self.__logs:
                    self.__load_logs(start_time, end_time, network, station,
                        channel, location)

                if self.__outages:
                    self.__load_outages(start_time, end_time, network, station,
                        channel, location)

                if self.__parameters:
                    self.__load_wfq(start_time, end_time, network, station,
                        channel, location)

            finally:
                RequestHandler.dataLock.release()

        except DBError, e:
            raise ArclinkHandlerError, str(e)

        self._resp.set_status(Arclink_OK, ref=self._line)
        self._line += 1

        self.__tracker.line_status(start_time, end_time, network,
            station, channel, location, "", "", "", constraints, "qc",
            "OK", 0, "")

    def execute(self):
        try:
            if self._line == self._errors:
                return

            try:
                filename = self.__req_dir + "/" + str(self.__req_id) + ".qc"
                if self.__compression == "bzip2":
                    fd = bz2.BZ2File(filename, "w")
                else:
                    fd = file(filename, "w")
                        
                try:
                    self.__qc.load_logs()
                    self.__qc.load_outages()
                    self.__qc.load_waveform_quality()
                    self.__qc.save_xml(fd)

                finally:
                    fd.close()

                size = os.stat(filename).st_size
                self._resp.set_status(Arclink_OK, size, ref="qc")
                self.__tracker.volume_status("qc", "OK", size, "")
                
            except (DBError, OSError, IOError), e:
                self._resp.set_status(Arclink_ERROR, msg=str(e), ref="qc")
                self.__tracker.volume_status("qc", "ERROR", 0, str(e))

        finally:
            self.__qc = None

class _GreensfuncRequest(_Request):
    def __init__(self, resp, logs, req_dir, req_id, req_args, tracker, gfa):
        _Request.__init__(self, resp, logs)
        self.__req_dir = req_dir
        self.__req_id = req_id
        self.__tracker = tracker
        self.__gfa = gfa

        self.__compression = req_args.get("compression", "none")
        if self.__compression != "bzip2" and self.__compression != "none":
            resp.req_message("compression '%s' is unknown, defaulting to none" %
                (self.__compression,))
            self.__compression = "none"

    def add_line(self, start_time, end_time, network, station, channel,
        location, constraints):

        if network is not None or station is not None or \
            channel is not None or location is not None:
            raise ArclinkHandlerError, "incorrectly formulated request"

        try:
            id = constraints["id"]
            model = constraints["model"]
            distance = float(constraints["distance"])
            depth = float(constraints["depth"])
            
            if "span" in constraints:
                span = Core.TimeSpan(float(constraints["span"]))
            
            else:
                t1 = Core.Time.FromString(str(start_time), "%Y-%m-%d %H:%M:%S")
                t2 = Core.Time.FromString(str(end_time), "%Y-%m-%d %H:%M:%S")
                span = t2 - t1

        except (KeyError, TypeError, ValueError):
            raise ArclinkHandlerError, "incorrectly formulated request"

        self._resp.processing(self._line, "gf")

        self.__gfa.addRequest(id, model, distance, depth, span)

        self._resp.set_status(Arclink_OK, ref=self._line)
        self._line += 1

        self.__tracker.line_status(start_time, end_time, network,
            station, channel, location, "", "", "", constraints, "gf",
            "OK", 0, "")

    def execute(self):
        try:
            if self._line == self._errors:
                return

            try:
                tmpfd = NamedTemporaryFile()
                
                try:
                    gf = self.__gfa.get()
                    if gf:
                        output = IO.BinaryArchive()
                        output.create(tmpfd.name)
                        while gf:
                            output.writeObject(gf)
                            gf = self.__gfa.get()
                        output.close()

                    filename = self.__req_dir + "/" + str(self.__req_id) + ".gf"
                    if self.__compression == "bzip2":
                        fd = bz2.BZ2File(filename, "w")
                    else:
                        fd = file(filename, "w")
                            
                    try:
                        tmpfd.seek(0)
                        copyfileobj(tmpfd, fd)

                    finally:
                        fd.close()

                finally:
                    tmpfd.close()

                size = os.stat(filename).st_size
                self._resp.set_status(Arclink_OK, size, ref="gf")
                self.__tracker.volume_status("gf", "OK", size, "")
                
            except (OSError, IOError), e:
                self._resp.set_status(Arclink_ERROR, msg=str(e), ref="gf")
                self.__tracker.volume_status("gf", "ERROR", 0, str(e))

        finally:
            self.__gfa = None

class RequestHandler(object):
    dataLock = threading.RLock()
    def __init__(self, inv, rtn, wf, gfa, rqdir, track, verbosity,
        organization, label):
        self.__inv = inv
        self.__rtn = rtn
        self.__wf = wf
        self.__gfa = gfa
        self.__rqdir = rqdir
        self.__track = track
        self.__org = organization
        self.__default_label = label
        self.__label = label
        self.__req = None
        self.__tracker = None
        self.__user = None
        self.__user_ip = None
        self.__client_ip = None
        self.__institution = None
        self.__updateNeeded = False
        self.__fdin = os.fdopen(_INPUT_FD)
        fdout = os.fdopen(_OUTPUT_FD, "w")
        self.__logs = _Logs(verbosity)
        self.__resp = _Responder(fdout, self.__logs)

    def updateInventory(self):
        self.__updateNeeded = True

    def __get_line(self):
        rqstr = self.__fdin.readline(_LINESIZE)
        if len(rqstr) > 1 and rqstr[-2:] == "\r\n":
            self.__logs.debug_in(rqstr[:-2])
        elif len(rqstr) > 0 and rqstr[-1:] == "\n":
            self.__logs.debug_in(rqstr[:-1])
        elif len(rqstr) == 0:
            logs.error("ArcLink server seems to have disappeared")
            sys.exit(1)
        else:
            raise ArclinkHandlerError, "invalid request line"

        return rqstr.split()

    def __process_line(self, rqline):
        if self.__req is None:
            if rqline[0] == "USER":
                self.__user = rqline[1]
                
            if rqline[0] == "USER_IP":
                self.__user_ip = rqline[1]
                
            if rqline[0] == "CLIENT_IP":
                self.__client_ip = rqline[1]
                
            elif rqline[0] == "INSTITUTION":
                self.__institution = " ".join(rqline[1:])

            elif rqline[0] == "LABEL":
                self.__label = rqline[1]

            elif rqline[0] == "REQUEST":
                if len(rqline) < 3:
                    raise ArclinkHandlerError, "invalid beginning of request"
        
                req_type = rqline[1]
                req_id = rqline[2]
                req_args = {}
                for arg in rqline[3:]:
                    pv = arg.split('=', 1)
                    if len(pv) != 2:
                        raise ArclinkHandlerError, "invalid beginning of request"
                    
                    req_args[pv[0]] = pv[1]

                self.__tracker = _RequestTracker(self.__track, req_id,
                    req_type, self.__user, " ".join(rqline), self.__label, self.__user_ip, self.__client_ip)
                
                if self.__label is None:
                    self.__label = self.__default_label

                if req_type == "WAVEFORM":
                    self.__req = _WaveformRequest(self.__rtn, self.__resp, self.__logs,
                        self.__rqdir, req_id, req_args, self.__tracker,
                        self.__user, self.__wf, self.__inv, self.__org,
                        self.__label)
                elif req_type == "RESPONSE":
                    self.__req = _ResponseRequest(self.__resp, self.__logs,
                        self.__rqdir, req_id, req_args, self.__tracker,
                        self.__inv, self.__org, self.__label)
                elif req_type == "INVENTORY":
                    self.__req = _InventoryRequest(self.__resp, self.__logs,
                        self.__rqdir, req_id, req_args, self.__tracker,
                        self.__inv)
                elif req_type == "ROUTING":
                    self.__req = _RoutingRequest(self.__resp, self.__logs,
                        self.__rqdir, req_id, req_args, self.__tracker,
                        self.__rtn)
                elif req_type == "QC":
                    self.__req = _QCRequest(self.__resp, self.__logs,
                        self.__rqdir, req_id, req_args, self.__tracker)
                elif req_type == "GREENSFUNC" and self.__gfa is not None:
                    self.__req = _GreensfuncRequest(self.__resp, self.__logs,
                        self.__rqdir, req_id, req_args, self.__tracker,
                        self.__gfa)
                else:
                    raise ArclinkHandlerError, "unknown request type"
                    
                self.__logs.set_id(req_id)
                self.__logs.info("new %s request from %s, %s" % 
                    (req_type, self.__user, self.__institution))
            
        elif rqline[0] == "END":
            try:
                self.__req.execute()
                self.__tracker.request_status("END", "")
                self.__resp.req_end()
                
            except ArclinkHandlerError, e:
                self.__logs.error(str(e))
                self.__resp.req_message(str(e))
                self.__resp.req_error()
                self.__tracker.request_status("ERROR", str(e))

            self.__user = None
            self.__user_ip = None
            self.__client_ip = None
            self.__institution = None
            self.__req = None
            self.__label = None
            self.__tracker = None
            self.__logs.set_id(None)

        else:
            if len(rqline) < 3:
                raise ArclinkHandlerError, "invalid request syntax"

            try:
                start_time = datetime.datetime(*map(int, rqline[0].split(",")))
                end_time = datetime.datetime(*map(int, rqline[1].split(",")))
            except ValueError, e:
                raise ArclinkHandlerError, "syntax error: " + str(e)

            network = None
            station = None
            channel = None
            location = None

            i = 2
            if len(rqline) > 2 and rqline[2] != ".":
                network = rqline[2]
                i += 1
                if len(rqline) > 3 and rqline[3] != ".":
                    station = rqline[3]
                    i += 1
                    if len(rqline) > 4 and rqline[4] != ".":
                        channel = rqline[4]
                        i += 1
                        if len(rqline) > 5 and rqline[5] != ".":
                            location = rqline[5]
                            i += 1
                        
            while len(rqline) > i and rqline[i] == ".":
                i += 1
            
            constraints = {}
            for arg in rqline[i:]:
                pv = arg.split('=', 1)
                if len(pv) != 2:
                    raise ArclinkHandlerError, "invalid request syntax"
                
                constraints[pv[0]] = pv[1]

            self.__req.add_line(start_time, end_time, network, station, channel,
                location, constraints)



    
    def start(self):
        try:
            RequestHandler.dataLock.acquire()
            try:
                self.__inv.load_instruments()

            finally:
                RequestHandler.dataLock.release()

            error_message = None

            while True:
                try:
                    rqline = self.__get_line()
                    
                    # FIXME: this here, because readline still blocks...
                    # FIXME: --> non-blocking select ?!? TBD
                    if rqline[0] == "REQUEST":
                        if self.__updateNeeded:
                            RequestHandler.dataLock.acquire()
                            try:
                                self.__logs.info("updating instrument data")
                                self.__inv.clear_instruments()
                                self.__inv.load_instruments()
                                self.__updateNeeded = False

                            finally:
                                RequestHandler.dataLock.release()
                    
                    if len(rqline) == 0:
                        continue

                    if error_message is not None:
                        if rqline[0] == "END":
                            self.__resp.req_message(error_message)
                            self.__resp.req_error()
                            self.__tracker.request_status("ERROR", error_message)
                            error_message = None

                        continue

                    self.__process_line(rqline)

                except ArclinkHandlerError, e:
                    self.__logs.error(str(e))
                    if self.__req is not None:
                        # avoid error from below
                        if len(rqline) < 6:
                            rqline += (6 - len(rqline)) * ['.']

                        self.__req.line_error(str(e))
                        self.__tracker.line_status(rqline[0], rqline[1], rqline[2],
                            rqline[3], rqline[4], rqline[5], "", "", "", rqline[6:],
                            "ERROR", "ERROR", 0, str(e))
                    else:
                        error_message = str(e)
        
        except SystemExit:
            pass

        except:
            self.__resp.req_message("internal error")
            self.__resp.req_error()
            if self.__tracker is not None:
                self.__tracker.request_status("ERROR", "internal error")

            raise

