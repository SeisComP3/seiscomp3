#!/usr/bin/env python

import os
import sys
import time
import socket
import signal
import select
import datetime
import threading
import SocketServer
import cPickle
from optparse import OptionParser
from xml.etree import cElementTree as ET
from shutil import copyfileobj
from tempfile import TemporaryFile
from seiscomp import sds
from seiscomp.arclink.manager import *
from seiscomp3 import Logging

VERSION = "1.1 (2014.316)"

class InvalidCommandError(Exception):
    pass

def _checkdot(s):
    if s == ".":
        return ""

    return s

def _indent(elem, level=0):
    s = 1*"\t" # indent char
    i = "\n" + level*s
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + s
        for e in elem:
            _indent(e, level+1)
            if not e.tail or not e.tail.strip():
                e.tail = i + s
        if not e.tail or not e.tail.strip():
            e.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

class RequestProxy(object):
    @staticmethod
    def __tw_str(tw):
        (net, sta, strm, loc, begin, end, constraints, blacklist) = tw
        return ("%d,%d,%d,%d,%d,%d %d,%d,%d,%d,%d,%d %s %s %s %s %s" %
            (begin.year, begin.month, begin.day, begin.hour, begin.minute, begin.second,
            end.year, end.month, end.day, end.hour, end.minute, end.second, net, sta, strm, loc,
            " ".join([ "%s=%s" % (a, v) for a, v in constraints.iteritems() ]))).strip()

    @staticmethod
    def __estimate_size(tw_list):
        samp_dict = {"B": 20, "E": 100, "H": 100, "L": 1, "S": 50, "U": 0.01, "V": 0.1}
        
        fsize = 0
        for tw in tw_list:
            tdiff = tw[5] - tw[4]
            tdiff = tdiff.days * 86400 + tdiff.seconds
            samp = samp_dict.get(tw[2][0].upper(), 20)
            fsize += int(tdiff * samp * 1.5)
            
        return fsize

    def __init__(self, rtype, args, label, user, user_ip):
        self.__rtype = rtype
        self.__args = args
        self.__label = label
        self.__user = user
        self.__user_ip = user_ip
        self.__ready = False
        self.__error = None
        self.__tw_proxy = []
        self.__tw_local = []
        self.__req_sent = []
        self.__req_noroute = []
        self.__size = 0
        self.__encrypted = False
        self.__vol_map = {}
        self.__purged = False
        self.__used = 0
        self.__lock = threading.RLock()
        self.__cond = threading.Condition()
        self.__sds = sds.SDS(g_options.nrtdir, g_options.arcdir, g_options.isodir)

    def before_save_state(self):
        self.__lock = self.__cond = self.__sds = None

        for req in self.__req_sent:
            req.close()

    def after_restore_state(self):
        self.__lock = threading.RLock()
        self.__cond = threading.Condition()
        self.__sds = sds.SDS(g_options.nrtdir, g_options.arcdir, g_options.isodir)

    def incr_used(self):
        with self.__lock:
            self.__used += 1

    def decr_used(self):
        with self.__lock:
            self.__used -= 1

            if not self.__used:
                for req in self.__req_sent:
                    req.close()

                    # XXX check this
                    if self.__purged:
                        try:
                            req.purge()

                        except ArclinkError, e:
                            logs.error(str(e))
    
    def access(self, user):
        return user == self.__user
    
    def sameuser(self, req):
        return req.__user == self.__user
    
    def ready(self):
        return self.__ready

    def user(self):
        return self.__user
    
    def add(self, network, station, stream, loc_id, begin, end, constraints):
        tw_tuple = RequestLine(begin, end, network, station, stream, loc_id,
            constraints)

        if self.__rtype == "WAVEFORM" and (g_options.local or self.__sds.exists(begin, end, network, station, stream, _checkdot(loc_id))):
            self.__tw_local.append(tw_tuple)

        else:
            self.__tw_proxy.append(tw_tuple)

    def execute(self):
        self.incr_used()
        
        try:
            if self.__tw_proxy:
                mgr = ArclinkManager(g_options.address,
                    default_user = self.__user,
                    user_ip = self.__user_ip,
                    socket_timeout = g_options.socket_timeout,
                    request_timeout = g_options.request_timeout,
                    download_retry = g_options.download_retries)

                req = mgr.new_request(self.__rtype, self.__args, self.__label)
                req.content = self.__tw_proxy
                use_routing = self.__rtype not in ("INVENTORY", "RESPONSE", "ROUTING") and \
                    not g_options.proxychain

                try:
                    res = mgr.execute(req, False, use_routing)

                except Exception, e:
                    res = (None, [], req)
                    self.__error = str(e)

            else:
                res = (None, [], None)

            self.__req_sent = res[1]
            self.__req_noroute = res[2]
            self.__size = 0
            self.__vol_map = {}

            for req in self.__req_sent:
	    	try:
                    rs = req.status()
                    for vs in rs.volume:
                        vol_id = vs.id + "-" + rs.id
                        self.__vol_map[vol_id] = (req, vs.id, vs.size)
                        self.__size += int(vs.size)
                        self.__encrypted |= vs.encrypted
 
                except ArclinkError, e:
                    logs.error(str(e))

            with self.__cond:
                self.__ready = True
                self.__cond.notify_all()

        finally:
            self.decr_used()

    def download(self, fd, vol, pos, blocking, chunkmode):
        self.incr_used()
        
        try:
            with self.__cond:
                if vol is None:
                    if not blocking and not self.__ready:
                        return False

                    if self.__encrypted:
                        raise InvalidCommandError, "cannot download encrypted request with multiple volumes"
                    
                    if chunkmode:
                        for (net, sta, cha, loc, t1, t2, constraints, blacklist) in self.__tw_local:
                            for rec in self.__sds.iterdata(t1, t2, net, sta, cha, _checkdot(loc)):
                                fd.write("CHUNK %d\r\n" % (len(rec),))
                                fd.write(rec)

                    while not self.__ready:
                        self.__cond.wait()

                    if chunkmode:
                        if self.__size or not self.__tw_local:
                            fd.write("CHUNK " + str(self.__size) + "\r\n")
                    
                    else:
                        if self.__tw_local:
                            size = self.__size
                            tmpfd = TemporaryFile()

                            for (net, sta, cha, loc, t1, t2, constraints, blacklist) in self.__tw_local:
                                for rec in self.__sds.iterdata(t1, t2, net, sta, cha, _checkdot(loc)):
                                    tmpfd.write(rec)
                                    size += len(rec)

                            tmpfd.seek(0)
                            fd.write(str(size) + "\r\n")
                            copyfileobj(tmpfd, fd)
                            tmpfd.close()

                        else:
                            fd.write(str(self.__size) + "\r\n")

                    for req in self.__req_sent:
                        if req.status().size > 0:
                            ### exception?
                            req.download_data(fd, None, block=True, purge=False, raw=True)

                elif vol == "local":
                    if not self.__tw_local:
                        raise InvalidCommandError, "volume not found"

                    if chunkmode:
                        for (net, sta, cha, loc, t1, t2, constraints, blacklist) in self.__tw_local:
                            for rec in self.__sds.iterdata(t1, t2, net, sta, cha, _checkdot(loc)):
                                fd.write("CHUNK %d\r\n" % (len(rec),))
                                fd.write(rec)

                    else:
                        size = 0
                        tmpfd = TemporaryFile()

                        for (net, sta, cha, loc, t1, t2, constraints, blacklist) in self.__tw_local:
                            for rec in self.__sds.iterdata(t1, t2, net, sta, cha, _checkdot(loc)):
                                tmpfd.write(rec)
                                size += len(rec)

                        tmpfd.seek(0)
                        fd.write(str(size) + "\r\n")
                        copyfileobj(tmpfd, fd)
                        tmpfd.close()
                
                else:
                    try:
                        (req, vol_id, vol_size) = self.__vol_map[vol]

                    except KeyError:
                        raise InvalidCommandError, "volume not found"

                    if chunkmode:
                        fd.write("CHUNK " + str(vol_size) + "\r\n")

                    else:
                        fd.write(str(vol_size) + "\r\n")

                    if vol_size > 0:
                        ### exception?
                        req.download_data(fd, vol_id, block=True, purge=False, raw=True)

        finally:
            self.decr_used()

        return True

    def purge(self):
        self.__purged = True
        with self.__lock:
            if not self.__used:
                for req in self.__req_sent:
                    req.close()

                    # XXX check this
                    if self.__purged:
                        try:
                            req.purge()

                        except ArclinkError, e:
                            logs.error(str(e))

    def status_xml(self, et):
        ready = self.__ready

        if ready:
            size = self.__size

        else:
            size = 0

        xreq = ET.Element("request")
        xreq.set("id", self.id)
        xreq.set("type", self.__rtype)
        xreq.set("label", self.__label)
        xreq.set("args", " ".join([ "%s=%s" % (a, v) for a, v in self.__args.iteritems() ]))
        xreq.set("size", str(size))
        xreq.set("ready", ("false", "true")[ready])
        xreq.set("encrypted", ("false", "true")[self.__encrypted])

        if self.__error:
            xreq.set("error", "true")
            xreq.set("message", self.__error)
        
        else:
            xreq.set("error", "false")
            xreq.set("message", "")

        et.append(xreq)

        if ready:
            if self.__req_sent:
                for req in self.__req_sent:
                    rs = req.status()
                    for vs in rs.volume:
                        vol_id = vs.id + "-" + rs.id

                        xvol = ET.Element("volume")
                        xvol.set("id", vol_id)
                        xvol.set("dcid", vs.dcid)
                        xvol.set("encrypted", ("false", "true")[vs.encrypted])
                        xvol.set("status", arclink_status_string(vs.status))
                        xvol.set("size", str(vs.size))
                        xvol.set("message", vs.message)
                        xreq.append(xvol)
                        
                        for ls in vs.line:
                            xline = ET.Element("line")
                            xline.set("content", ls.content)
                            xline.set("status", arclink_status_string(ls.status))
                            xline.set("size", str(ls.size))
                            xline.set("message", ls.message)
                            xvol.append(xline)

            if self.__req_noroute:
                xvol = ET.Element("volume")
                xvol.set("id", "NODATA")
                xvol.set("dcid", g_options.dcid)
                xvol.set("restricted", "false")
                xvol.set("status", "NODATA")
                xvol.set("size", "0")
                xvol.set("message", "routing failed")
                xreq.append(xvol)

                for tw in self.__req_noroute.content:
                    xline = ET.Element("line")
                    xline.set("content", RequestProxy.__tw_str(tw))
                    xline.set("status", "NODATA")
                    xline.set("size", "0")
                    xline.set("message", "routing failed")
                    xvol.append(xline)
            
        else:
            xvol = ET.Element("volume")
            xvol.set("id", "UNSET")
            xvol.set("dcid", g_options.dcid)
            xvol.set("restricted", "false")
            xvol.set("status", "PROCESSING")
            xvol.set("size", "0")
            xvol.set("message", "")
            xreq.append(xvol)

            for tw in self.__tw_proxy:
                xline = ET.Element("line")
                xline.set("content", RequestProxy.__tw_str(tw))
                xline.set("status", "UNSET")
                xline.set("size", "0")
                xline.set("message", "")
                xvol.append(xline)

        if self.__tw_local:
            xvol = ET.Element("volume")
            xvol.set("id", "local")
            xvol.set("dcid", g_options.dcid)
            xvol.set("status", "OK")
            xvol.set("size", str(RequestProxy.__estimate_size(self.__tw_local)))
            xvol.set("message", "")
            xreq.append(xvol)

            for tw in self.__tw_local:
                xline = ET.Element("line")
                xline.set("content", RequestProxy.__tw_str(tw))
                xline.set("status", "OK")
                xline.set("size", str(RequestProxy.__estimate_size([tw])))
                xline.set("message", "")
                xvol.append(xline)
        
class RequestContainerThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.__running = False
        self.__id = 0
        self.__request = {}
        self.__executing = {}
        self.__queued = []
        self.__cond = threading.Condition()

    def save_state(self, file):
        for req in self.__request.itervalues():
            req.before_save_state()

        cPickle.dump((self.__id, self.__request, self.__queued), open(file, "w"))

    def restore_state(self, file):
        (self.__id, self.__request, self.__queued) = cPickle.load(open(file))

        for req in self.__request.itervalues():
            req.after_restore_state()

    def put(self, req):
        with self.__cond:
            if len(self.__queued) >= g_options.max_queued:
                raise InvalidCommandError, "maximum number of requests queued"

            if g_options.max_queued_per_user and reduce(lambda a,b: a+req.sameuser(b), self.__queued, 0) >= g_options.max_queued_per_user:
                logs.warning("maximum number of requests per user (%s) queued" % (req.user(),))
                raise InvalidCommandError, "maximum number of requests per user (%s) queued" % (req.user(),)

            req.id = str(self.__id)
            req.time = time.time()
            self.__request[str(self.__id)] = req
            self.__queued.append(req)
            self.__id = self.__id + 1

    def get(self, id):
        with self.__cond:
            return self.__request[id]

    def purge(self, id):
        with self.__cond:
            if not self.__request[id].ready():
                for i in range(len(self.__queued)):
                    if id == self.__queued[i].id:
                        del self.__queued[i]
                        break

            self.__request[id].purge()
            del self.__request[id]

    def stop(self):
        with self.__cond:
            self.__running = False
            self.__cond.notify()
    
    def status(self, id, user, fd):
        et = ET.Element("arclink")

        if id.upper() == "ALL":
            for req in self.__request.itervalues():
                if req.access(user):
                    req.status_xml(et)

        else:
            req = g_reqs.get(id)
            
            if not req.access(user):
                raise KeyError

            req.status_xml(et)

        _indent(et)
        fd.write("<?xml version=\"1.0\"?>\n")
        ET.ElementTree(et).write(fd, encoding="utf-8")
        fd.write("\r\n")

    def run(self):
        with self.__cond:
            self.__running = True
            while self.__running:
                curtime = time.time()
                for req in self.__request.values():
                    if req.ready() and curtime - req.time > g_options.max_age:
                        self.__request[req.id].purge()
                        del self.__request[req.id]
                
                for req in self.__executing.values():
                    if req.ready():
                        del self.__executing[req.id]

                while self.__queued and len(self.__executing) < g_options.max_executing:
                    req = self.__queued[0]
                    del self.__queued[0]
                    self.__executing[req.id] = req
                    threading.Thread(target=req.execute).start()

                self.__cond.wait(1)

class Session(object):
    __count = 0
    __cond = threading.Condition()

    @staticmethod
    def wait_for_termination():
        with Session.__cond:
            while Session.__count:
                Session.__cond.wait()
    
    @staticmethod
    def count():
        return Session.__count
    
    def __init__(self, peername, wfile):
        self.__peername = peername
        self.__wfile = wfile
        self.__user = None
        self.__institution = None
        self.__pwd = None
        self.__label = ""
        self.__current_req = None
        self.__req_lines = 0
        self.__req_error = None
        self.__reqs = []
        self.__user_ip = peername[0]  # default

    def __enter__(self):
        with Session.__cond:
            Session.__count += 1

        return self

    def __exit__(self, exc_type, exc_value, traceback):
        retval = False

        if exc_type == socket.timeout:
            logs.warning("%s:%s: timeout" % self.__peername)
            retval = True
        
        elif exc_type == socket.error:
            logs.warning("%s:%s: " % self.__peername + str(exc_value))
            retval = True

        elif exc_type == select.error:
            logs.warning("%s:%s: " % self.__peername + str(exc_value))
            retval = True

        for req in self.__reqs:
            req.decr_used()
        
        with Session.__cond:
            Session.__count -= 1
            Session.__cond.notify()

        return retval

    def __cmd_hello(self, cmd):
        return "ArcLink Proxy v" + VERSION + "\r\n" + g_options.organization
        
    def __cmd_user(self, cmd):
        if len(cmd) < 2 or len(cmd) > 3:
            raise InvalidCommandError, "syntax error"

        self.__user = cmd[1]

    def __cmd_institution(self, cmd):
        if len(cmd) < 2:
            raise InvalidCommandError, "syntax error"

        self.__institution = " ".join(cmd[1:])

    def __cmd_label(self, cmd):
        if len(cmd) != 2:
            raise InvalidCommandError, "syntax error"

        self.__label = cmd[1]

    def __cmd_user_ip(self, cmd):
        if len(cmd) != 2:
            raise InvalidCommandError, "syntax error"

        self.__user_ip = cmd[1]

    def __cmd_request(self, cmd):
        if len(cmd) < 2:
            raise InvalidCommandError, "syntax error"

        req_type = cmd[1].upper()

        if g_options.local and req_type != "WAVEFORM":
            raise InvalidCommandError, req_type + " request is disabled"

        req_args = {}
        for arg in cmd[2:]:
            pv = arg.split('=', 1)
            if len(pv) != 2:
                raise InvalidCommandError, "syntax error"
            
            req_args[pv[0]] = pv[1]

        self.__current_req = RequestProxy(req_type, req_args, self.__label,
            self.__user, self.__user_ip)

        self.__req_error = None
        self.__req_lines = 0
        
    def __cmd_end(self, cmd):
        if not self.__current_req:
            raise InvalidCommandError, "invalid command"

        if self.__req_error:
            raise InvalidCommandError, self.__req_error
        
        if self.__req_lines > g_options.max_lines:
            raise InvalidCommandError, "too many lines"
        
        try:
            g_reqs.put(self.__current_req)
            self.__current_req.incr_used()
            self.__reqs.append(self.__current_req)

            return self.__current_req.id

        finally:
            self.__current_req = None

    def __cmd_status(self, cmd):
        if len(cmd) != 2:
            raise InvalidCommandError, "syntax error"

        req_id = cmd[1]

        try:
            g_reqs.status(req_id, self.__user, self.__wfile)

        except KeyError:
            raise InvalidCommandError, "request not found or access denied"

        return "END"

    def __download(self, cmd, blocking, chunkmode):
        if len(cmd) < 2 or len(cmd) > 3:
            raise InvalidCommandError, "syntax error"

        req_vol = cmd[1].split(".", 1)

        req_id = req_vol[0]

        try:
            vol_id = req_vol[1]

        except IndexError:
            vol_id = None

        try:
            pos = cmd[2]

        except IndexError:
            pos = None

        except ValueError:
            raise InvalidCommandError, "syntax error"

        try:
            req = g_reqs.get(req_id)

        except KeyError:
            req = None

        if not req or not req.access(self.__user):
            raise InvalidCommandError, "request not found or access denied"

        if not req.download(self.__wfile, vol_id, pos, blocking, chunkmode):
            raise InvalidCommandError, "download failed"

        return "END"

    def __cmd_download(self, cmd):
        return self.__download(cmd, False, False)

    def __cmd_bdownload(self, cmd):
        return self.__download(cmd, True, False)

    def __cmd_cdownload(self, cmd):
        return self.__download(cmd, True, True)

    def __cmd_purge(self, cmd):
        if len(cmd) != 2:
            raise InvalidCommandError, "syntax error"

        req_id = cmd[1]

        try:
            req = g_reqs.get(req_id)

        except KeyError:
            req = None

        if not req or not req.access(self.__user):
            raise InvalidCommandError, "request not found or access denied"

        g_reqs.purge(req_id)

    def __add_line(self, reqline):
        if len(reqline) < 2:
            self.__error = "syntax error"
            return

        if self.__req_error or self.__req_lines > g_options.max_lines:
            return

        try:
            start_time = datetime.datetime(*map(int, reqline[0].split(",")))
            end_time = datetime.datetime(*map(int, reqline[1].split(",")))
        
        except ValueError:
            self.__req_error = "syntax error"
            return

        network = "."
        station = "."
        channel = "."
        location = "."

        i = 2
        if len(reqline) > 2 and reqline[2] != ".":
            network = reqline[2]
            i += 1
            if len(reqline) > 3 and reqline[3] != ".":
                station = reqline[3]
                i += 1
                if len(reqline) > 4 and reqline[4] != ".":
                    channel = reqline[4]
                    i += 1
                    if len(reqline) > 5 and reqline[5] != ".":
                        location = reqline[5]
                        i += 1
                    
        while len(reqline) > i and reqline[i] == ".":
            i += 1
        
        constraints = {}
        for arg in reqline[i:]:
            pv = arg.split('=', 1)
            if len(pv) != 2:
                self.__req_error = "syntax error"
                return
            
            constraints[pv[0]] = pv[1]

        self.__current_req.add(network, station, channel, location,
            start_time, end_time, constraints)
        
        self.__req_lines += 1

    __cmd = {
        "HELLO":       __cmd_hello,
        "USER":        __cmd_user,
        "INSTITUTION": __cmd_institution,
        "LABEL":       __cmd_label,
        "USER_IP":     __cmd_user_ip,
        "REQUEST":     __cmd_request,
        "END":         __cmd_end,
        "STATUS":      __cmd_status,
        "DOWNLOAD":    __cmd_download,
        "BDOWNLOAD":   __cmd_bdownload,
        "BCDOWNLOAD":  __cmd_cdownload,
        "PURGE":       __cmd_purge
    }

    def dispatch(self, cmd):
        if self.__current_req and cmd[0].upper() != "END":
            self.__add_line(cmd)
            return 
        
        else:
            try:
                return Session.__cmd[cmd[0].upper()](self, cmd) or "OK"

            except KeyError:
                raise InvalidCommandError, "invalid command"

class MyTCPServer(SocketServer.ThreadingTCPServer):
    def __init__(self, *args):
        SocketServer.ThreadingTCPServer.__init__(self, *args)
        self.allow_reuse_address = True
        self.timeout = 1

class MyTCPHandler(SocketServer.StreamRequestHandler):
    def handle(self):
        if Session.count() >= g_options.max_sessions:
            logs.error("maximum numer of sessions reached")
            return

        with Session(self.request.getpeername(), self.wfile) as sess:
            errmsg = "success"
            lastdata = ""

            timeout_counter = g_options.socket_timeout

            while g_running:
                try:
                    if not select.select([self.request], [], [], 1)[0]:
                        if not timeout_counter:
                            raise socket.timeout
                            
                        timeout_counter -= 1
                        continue
                
                except select.error, e:
                    if e[0] == 4: # EINTR
                        continue

                    raise
            
                timeout_counter = g_options.socket_timeout
            
                data = self.request.recv(1024)
                if not data:
                    break
                
                #lines = (lastdata + data).split('\r')
                lines = (lastdata + data).replace('\n', '\r').split('\r')
                lastdata = lines[-1]
                
                for line in lines[0:-1]:
                    cmd = line.split()
                    if not cmd:
                        continue

                    try:
                        if cmd[0].upper() == "BYE":
                            return

                        elif cmd[0].upper() == "SHOWERR":
                            self.wfile.write(errmsg + "\r\n")
                    
                        else:
                            res = sess.dispatch(cmd)
                            if res:
                                self.wfile.write(res + "\r\n")

                        errmsg = "success"

                    except InvalidCommandError, e:
                        errmsg = str(e)
                        self.wfile.write("ERROR\r\n")

g_verbosity = 1
g_running = True
g_reqs = RequestContainerThread()

def shutdown(*args):
    global g_running
    g_running = False

def add_verbosity(option, opt_str, value, parser):
    global g_verbosity
    g_verbosity += 1

def add_quietness(option, opt_str, value, parser):
    global g_verbosity
    g_verbosity -= 1

def process_options():
    parser = OptionParser(usage="usage: %prog [options]",
      version="%prog v" + VERSION)

    parser.set_defaults(address = "webdc.eu:18001",
                        proxyport = 18001,
                        organization = "proxy",
                        dcid = "GFZ",
                        max_sessions = 500,
                        max_queued = 500,
                        max_queued_per_user = 10,
                        max_executing = 10,
                        max_lines = 5000,
                        max_age = 860000,
                        socket_timeout = 300,
                        request_timeout = 300,
                        download_retries = 5,
                        proxychain = False,
                        local = False)

    parser.add_option("-a", "--address", type="string", dest="address",
      help="address of primary ArcLink node (default %default)")

    parser.add_option("-P", "--proxyport", type="int", dest="proxyport",
      help="proxy port (default %default)")

    parser.add_option("-O", "--organization", type="string", dest="organization",
      help="organization name (default %default)")

    parser.add_option("-d", "--dcid", type="string", dest="dcid",
      help="datacenter ID (default %default)")

    parser.add_option("-S", "--max-sessions", type="int", dest="max_sessions",
      help="maximum number of sessions (default %default)")

    parser.add_option("-Q", "--max-queued", type="int", dest="max_queued",
      help="maximum number of queued requests (default %default)")

    parser.add_option("-U", "--max-queued-per-user", type="int", dest="max_queued_per_user",
      help="maximum number of queued requests (default %default)")

    parser.add_option("-X", "--max-executing", type="int", dest="max_executing",
      help="maximum number of requests executed in parallel (default %default)")

    parser.add_option("-L", "--max-lines", type="int", dest="max_lines",
      help="maximum number of lines in request (default %default)")

    parser.add_option("-T", "--max-age", type="int", dest="max_age",
      help="maximum age of request (default %default)")

    parser.add_option("-t", "--socket-timeout", type="int", dest="socket_timeout",
      help="socket timeout in seconds (default %default)")

    #parser.add_option("-r, "--request-timeout", type="int", dest="request_timeout",
    #  help="request timeout in seconds (default %default)")

    parser.add_option("-R", "--download-retries", type="int", dest="download_retries",
      help="download retries (default %default)")

    parser.add_option("-N", "--nrtdir", type="string", dest="nrtdir",
      help="NRT SDS directory (default %default)")

    parser.add_option("-A", "--arcdir", type="string", dest="arcdir",
      help="SDS archive directory (default %default)")

    parser.add_option("-I", "--isodir", type="string", dest="isodir",
      help="ISO archive directory (default %default)")

    parser.add_option("-p", "--disable-routing", action="store_true", dest="proxychain",
      help="disable routing")

    parser.add_option("-l", "--local-only", action="store_true", dest="local",
      help="use local SDS only")

    parser.add_option("-x", "--state-file", type="string", dest="state_file",
      help="save/restore request information to this file")

    parser.add_option("-v", action="callback", callback=add_verbosity,
      help="increase verbosity level")
    
    parser.add_option("-q", action="callback", callback=add_quietness,
      help="decrease verbosity level")
    
    global g_options
    (g_options, args) = parser.parse_args()

    if args:
        parser.error("incorrect number of arguments")

    if g_options.nrtdir is None:
        g_options.nrtdir = os.getenv("SEISCOMP_ROOT", "") + "/var/lib/archive"

    if g_options.arcdir is None:
        g_options.arcdir = "/dummy"

    if g_options.isodir is None:
        g_options.isodir = "/dummy"

    if g_verbosity > 1:
        Logging.enableConsoleLogging(Logging.getGlobalChannel("debug"))
        Logging.enableConsoleLogging(Logging.getGlobalChannel("info"))
        Logging.enableConsoleLogging(Logging.getGlobalChannel("notice"))
        Logging.enableConsoleLogging(Logging.getGlobalChannel("warning"))
        Logging.enableConsoleLogging(Logging.getGlobalChannel("error"))

def main():
    process_options()
    
    server = MyTCPServer(("0.0.0.0", g_options.proxyport), MyTCPHandler)

    signal.signal(signal.SIGTERM, shutdown)
    signal.signal(signal.SIGINT, shutdown)
    
    if g_options.state_file:
        try:
            g_reqs.restore_state(g_options.state_file)

        except IOError:
            pass

    g_reqs.start()

    while g_running:
        try:
            server.handle_request()

        except select.error, e:
            if e[0] != 4:
                logs.error("select error: " + e[1])
    
    server.server_close()

    g_reqs.stop()
    g_reqs.join()

    Session.wait_for_termination()
    
    if g_options.state_file:
        g_reqs.save_state(g_options.state_file)

if __name__ == "__main__":
    def log_debug(s):
        if g_verbosity > 1:
            print >>sys.stderr, s
            sys.stderr.flush()

    def log_info(s):
        if g_verbosity > 0:
            print >>sys.stderr, s
            sys.stderr.flush()

    def log_other(s):
        print >>sys.stderr, s
        sys.stderr.flush()

    logs.debug = log_debug
    logs.info = log_info
    logs.notice = log_other
    logs.warning = log_other
    logs.error = log_other

    main()

