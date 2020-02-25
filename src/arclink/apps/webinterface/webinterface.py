#***************************************************************************** 
# webinterface.py
#
# ArcLink web interface (prototype)
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import sys
import os
import time
import datetime
import random
import fcntl
import socket
import urllib
import base64
import cStringIO
import syslog
import session
from sets import Set
from mod_python import apache
from seiscomp import logs
from seiscomp.arclink.client import *
from seiscomp.arclink.manager import *
from html_chunks import *

DEFAULT_ADDR = "localhost:18001"
DEFAULT_USER = "guest@webdc"
EMAIL_ADDR = "breqfast@webdc.eu"
EMAIL_FROM = "WebDC <breqfast@webdc.eu>"
EMAIL_SUBJECT = "ArcLink request confirmation"
FORMAIL_BIN = "/usr/bin/formail"
SENDMAIL_BIN = "/usr/sbin/sendmail"
ARCLINK_TIMEOUT = 100
ARCLINK_TIMEOUT_CHECK = 5
INVENTORY_WAIT = 1
INVENTORY_RETRY = 20
LABEL = "WebDC"

VERSION = "1.0 (2010.256)"

nettypes = [
    ("perm", "Permanent nets", True, None),
    ("open", "All public nets", None, False),
    ("tempa", "Temporary nets", False, None),
    ("tempo", "Public temp. nets", False, False),
    ("tempr", "Non-public nets", None, True)]

def make_logger(prio):
    def log_print(s):
        syslog.syslog(prio, s)
    
    return log_print

logs.debug = make_logger(syslog.LOG_DEBUG)
logs.info = make_logger(syslog.LOG_INFO)
logs.notice = make_logger(syslog.LOG_NOTICE)
logs.warning = make_logger(syslog.LOG_WARNING)
logs.error = make_logger(syslog.LOG_ERR)


arcl = Arclink()
mgr = ArclinkManager(DEFAULT_ADDR, DEFAULT_USER)

def init_session(obj, args):
    sesskey = args.get("sesskey")
    myaddr = args.get("arclink")
    user = args.get("user")                            #TEMP
    
    if sesskey is None:
        while True:
            sesskey = "%08x" % (random.randint(0, 0xffffffff),)
            if not session.have_key(sesskey):
                break

        if obj.args is None:
            url = "http://%s%s?sesskey=%s" % \
                (obj.hostname, obj.uri, sesskey)
        else:
            url = "http://%s%s?sesskey=%s&%s" % \
                (obj.hostname, obj.uri, sesskey, obj.args)
            
        obj.headers_out['Location'] = url
        raise apache.SERVER_RETURN, apache.HTTP_MOVED_PERMANENTLY
        
    if myaddr is None:
        return (sesskey, None, None)

    sessdata = session.get_data(sesskey)
    authdata = sessdata.get_auth(myaddr)
    if authdata is None:                               #TEMP
        authdata = sessdata.get_auth(DEFAULT_ADDR)     #TEMP

    if authdata is None:
        if user is not None:                           #TEMP
            authdata = session.new_auth(user, None)    #TEMP
            sessdata.add_auth(DEFAULT_ADDR, authdata)  #TEMP
        else:                                          #TEMP
            authdata = session.new_auth(None, None)
            sessdata.add_auth(myaddr, authdata)
    else:
        auth = obj.headers_in.get('Authorization')
        if auth is not None:
            (authdata.user, authdata.passwd) = base64.decodestring(auth.split()[1]).split(':')

    sessdata.timestamp = time.time()      # refresh
    session.set_data(sesskey, sessdata)

    return (sesskey, authdata.user, authdata.passwd)
    
def query(obj, args):
    nettype = args.get("nettype", "perm")
    network = args.get("network", "*")
    station = args.get("station", "*")

    (sesskey, user, passwd) = init_session(obj, args)

    net_arg = ""
    if network != "*":
        net_arg = "&network=" + network

    permanent = None
    restricted = None
    if nettype is not None:
        for (nt, net_desc, perm, restr) in nettypes:
            if nt == nettype:
                permanent = perm
                restricted = restr
    
    db = mgr.get_inventory(network, station, permanent=permanent,
        restricted=restricted, allnet=True) 
    
    obj.content_type = "text/html; charset=UTF-8"
    obj.write(query_head)
    obj.write('<b>Received inventory data from ArcLink. '
        'Generating request form at ' + time.ctime() + '</b><br>')
    
    obj.write(query_chunk1)
    for (nt, net_desc, parm, restr) in nettypes:
        if nt == nettype:
            sel = "selected"
        else:
            sel = ""
            
        obj.write('<option value="query?sesskey=%s&nettype=%s"%s>%s</option>' % \
            (sesskey, nt, sel, net_desc))

    obj.write(query_chunk2)
    netlist = db.network.keys()
    statlist = []

    if network == "*":
        sel = "selected"
        stats = Set()
        for net_code in netlist:
            for net in db.network[net_code].values():
                for sta_code in net.station:
                    stats.add((sta_code, net_code))

        statlist = list(stats)
    else:
        sel = ""
    
    obj.write('<option value="query?sesskey=%s&nettype=%s"%s>%s</option>' % \
        (sesskey, nettype, sel, "all"))

    arch_netlist = []
    for net_code in netlist:
        net_start = db.network[net_code].keys()
        net_start.sort()
        net = db.network[net_code][net_start[-1]]
        arch_netlist.append(((net.netClass == 't'), net.code, net.archive,
            net.restricted))

    arch_netlist.sort()
    for (temp, net_code, arch_code, restr) in arch_netlist:
        if net_code == network:
            sel = "selected"
            stats = Set()
            for net in db.network[net_code].values():
                for sta_code in net.station:
                    stats.add(sta_code)

            statlist = list(stats)
        else:
            sel = ""

        net_start = db.network[net_code].keys()
        net_start.sort()
        net = db.network[net_code][net_start[-1]]
        
        if temp:
            code = net.code + '*'
        else:
            code = net.code

        if restr:
            desc = net.description + " (" + net.archive + ", restricted)"
        else:
            desc = net.description + " (" + net.archive + ")"

        obj.write('<option value="query?sesskey=%s&nettype=%s&network=%s"%s>%s</option>' % \
            (sesskey, nettype, net_code, sel, code + " - " + desc))

    obj.write(query_chunk3)
    if station == "*":
        sel = "selected"
    else:
        sel = ""

    obj.write('<option value="query?sesskey=%s&nettype=%s%s"%s>%s</option>' % \
        (sesskey, nettype, net_arg, sel, "all"))

    statlist.sort()
    if network == "*":
        for (sta_code, net_code) in statlist:
            obj.write('<option value="query?sesskey=%s&nettype=%s&network=%s&station=%s">%s</option>' % \
                (sesskey, nettype, net_code, sta_code, sta_code + "/" + net_code))
    else:
        for sta_code in statlist:
            if sta_code == station:
                sel = "selected"
            else:
                sel = ""

            obj.write('<option value="query?sesskey=%s&nettype=%s%s&station=%s"%s>%s</option>' % \
                (sesskey, nettype, net_arg, sta_code, sel, sta_code))

    obj.write(query_chunk4)
    obj.write('<input name="sesskey" value="%s" type="hidden">' % (sesskey,))
    obj.write('<input name="nettype" value="%s" type="hidden">' % (nettype,))

    if network != "*":
        obj.write('<input name="network" value="%s" type="hidden">' % (network,))

    if station != "*":
        obj.write('<input name="station" value="%s" type="hidden">' % (station,))
    
    obj.write(query_tail)
        
def select(obj, args):
    nettype = args.get("nettype", "perm")
    network = args.get("network", "*")
    station = args.get("station", "*")
    stream = args.get("stream", "*")
    loc_id = args.get("loc_id", "*")
    start_date = args.get("start_date")
    end_date = args.get("end_date")
    latmin = args.get("latmin")
    latmax = args.get("latmax")
    lonmin = args.get("lonmin")
    lonmax = args.get("lonmax")
    sensortype = args.get("sensor")

    qc_constraints = dict([ arg for arg in args.items() if arg[0] not in
        set(["nettype", "network", "station", "stream", "loc_id", "start_date",
        "end_date", "latmin", "latmax", "lonmin", "lonmax", "sensor",
        "user", "sesskey", "arclink", "submit" ]) and arg[1].strip() ])
    
    if len(loc_id) == 0:
        loc_id = None
    
    if sensortype == "all":
        sensortype = None
    
    begin = None
    if start_date is not None:
        try:
            begin = datetime.datetime(*time.strptime(start_date, "%Y-%m-%d")[0:3])
        except ValueError:
            raise ArclinkError, "invalid start date: " + start_date
    
    end = None
    if end_date is not None:
        try:
            end = datetime.datetime(*time.strptime(end_date, "%Y-%m-%d")[0:3])
        except ValueError:
            raise ArclinkError, "invalid end date: " + end_date
    
    (sesskey, user, passwd) = init_session(obj, args)

    if nettype is not None:
        for (nt, net_desc, perm, restr) in nettypes:
            if nt == nettype:
                permanent = perm
                restricted = restr
    
    db = mgr.get_inventory(network, station, stream, loc_id, begin, end,
        sensortype, permanent, restricted, latmin, latmax, lonmin, lonmax,
        qc_constraints = qc_constraints) 
    
    obj.content_type = "text/html; charset=UTF-8"
    obj.write(select_head)
    obj.write('<b>Received inventory data from ArcLink. '
        'Generating request form at ' + time.ctime() + '</b><br>')
    
    obj.write(select_chunk1)
    obj.write('<input name="sesskey" value="%s" type="hidden">' % (sesskey,))

    netlist = db.network.keys()
    netlist.sort()
    for net_code in netlist:
        net_start = db.network[net_code].keys()
        net_start.sort()
        net = db.network[net_code][net_start[-1]]

        if net.netClass == 't':
            code = net.code + '*'
        else:
            code = net.code

        if net.restricted:
            desc = net.description + " (" + net.archive + ", restricted)"
        else:
            desc = net.description + " (" + net.archive + ")"

        obj.write('<tr><td><b>%s</b></td><td><b>%s</b></td></tr>\n' % (code, desc))
        statlist = net.station.keys()
        statlist.sort()
        for sta_code in statlist:
            sta_start = net.station[sta_code].keys()
            sta_start.sort()
            sta = net.station[sta_code][sta_start[-1]]
            obj.write('<tr><td>%s</td><td>%s</td></tr>\n' % (sta.code, sta.description))
            streamset = Set()
            for net in db.network[net_code].values():
                for sta in net.station[sta_code].values():
                    for loc_code in sta.sensorLocation.keys():
                        for loc in sta.sensorLocation[loc_code].values():
			    for str_code in loc.stream.keys():
	                        streamset.add((loc_code, str_code))
                                streamset.add((loc_code, str_code[:-1] + "*"))
                    
			    for str_code in loc.auxStream:
                                streamset.add((loc_code, str_code))

            streamlist = list(streamset)
            streamlist.sort()
            lastloc = streamlist[0][0]   # assume streamlist not empty
            laststream = streamlist[0][1][:2]
            obj.write('<tr><td></td><td><table><tbody>\n<tr>')
            for s in streamlist:
                if len(s[0]) == 0:
                    streamname = s[1]
                else:
                    streamname = s[0] + "." + s[1]
                
                if s[0] != lastloc or s[1][:2] != laststream:
                    obj.write('</tr>\n<tr>')
                    lastloc = s[0]
                    laststream = s[1][:2]
                    
                obj.write('<td><input type="checkbox" name="rq" value="%s_%s_%s_%s">'
                    '%s</input></td>' % (net.code, sta.code, s[0], s[1], streamname))
            
            obj.write('</tr>\n</tbody></table></td></tr>\n')
            
    obj.write(select_chunk2)

    t = time.time()
    start_gmt = time.gmtime(t - 20)
    end_gmt = time.gmtime(t - 10)

    start_date = time.strftime("%Y-%m-%d", start_gmt)
    start_time = time.strftime("%H:%M:%S", start_gmt)
    end_date = time.strftime("%Y-%m-%d", end_gmt)
    end_time = time.strftime("%H:%M:%S", end_gmt)
    
    obj.write(select_chunk3)
    obj.write('Start time: <INPUT type="text" value="%s" size=10 maxlength=10 min=10 name="start_date"> - '
        '<INPUT type="text" value="%s" size=8 maxlength=8 min=8 name="start_time">'
        'End time: <INPUT type="text" value="%s" size=10 maxlength=10 min=10 name="end_date"> - '
        '<INPUT type="text" value="%s" size=8 maxlength=8 min=8 name="end_time">' % \
        (start_date, start_time, end_date, end_time))
    
    obj.write(select_tail)
        
def submit(obj, args):
    myaddr = args.get("arclink")
    name = args.get("name")
    inst = args.get("inst")
    mail = args.get("mail")
    user = args.get("user")                            #TEMP
    request = args.get("rq")
    start_date = args.get("start_date")
    start_time = args.get("start_time")
    end_date = args.get("end_date")
    end_time = args.get("end_time")
    format = args.get("format")
    compression = args.get("compression")
    resp_dict = args.get("resp_dict")

    if myaddr is None or len(myaddr) == 0:
        raise ArclinkError, "missing arclink address"
    
    try:
        (host, port) = myaddr.split(':')
        port = int(port)
    except (AttributeError, ValueError):
        raise ArclinkError, "invalid ArcLink address"

    if name is None or len(name) == 0:
        raise ArclinkError, "missing full name"
        
    if inst is None or len(inst) == 0:
        raise ArclinkError, "missing institution"
    
    if request is None or len(request) == 0:
        raise ArclinkError, "empty request"
    
    if start_date is None or len(start_date) == 0:
        raise ArclinkError, "missing start date"
    else:
        try:
            bdate = datetime.date(*time.strptime(start_date, "%Y-%m-%d")[0:3])
        except ValueError:
            raise ArclinkError, "invalid start date: " + start_date
    
    if start_time is None or len(start_time) == 0:
        raise ArclinkError, "missing start time"
    else:
        try:
            btime = datetime.time(*time.strptime(start_time, "%H:%M:%S")[3:6])
        except ValueError:
            raise ArclinkError, "invalid start time: " + start_time
    
    begin = datetime.datetime.combine(bdate, btime)
    
    if end_date is None or len(end_date) == 0:
        raise ArclinkError, "missing end date"
    else:
        try:
            edate = datetime.date(*time.strptime(end_date, "%Y-%m-%d")[0:3])
        except ValueError:
            raise ArclinkError, "invalid end date: " + end_date
    
    if end_time is None or len(end_time) == 0:
        raise ArclinkError, "missing end time"
    else:
        try:
            etime = datetime.time(*time.strptime(end_time, "%H:%M:%S")[3:6])
        except ValueError:
            raise ArclinkError, "invalid end time: " + end_time
    
    end = datetime.datetime.combine(edate, etime)
    
    if format is None or len(format) == 0:
        raise ArclinkError, "missing format"
    
    req_args = {}
    if compression == "bzip2":
        req_args['compression'] = "bzip2"
    elif compression is not None and compression != "none":
        raise ArclinkError, "unsupported compression: " + compression
    
    (sesskey, user, passwd) = init_session(obj, args)

    mgr = ArclinkManager(myaddr, user)

    if format == "MSEED":
        req_args['format'] = 'MSEED'
        req = mgr.new_request("WAVEFORM", req_args, LABEL)
    elif format == "FSEED":
        req_args['format'] = 'FSEED'
        req = mgr.new_request("WAVEFORM", req_args, LABEL)
    elif format == "DSEED":
        req = mgr.new_request("RESPONSE", req_args, LABEL)
    elif format == "INV":
        req_args['instruments'] = 'true'
        req = mgr.new_request("INVENTORY", req_args, LABEL)
    else:
        raise ArclinkError, "unsupported format: " + format

    if resp_dict == "true":
        req_args['resp_dict'] = 'true'
    else:
        req_args['resp_dict'] = 'false'
    
    for req_item in request:
        try:
            (network, station, loc_id, stream) = req_item.split("_")
        except (AttributeError, ValueError):
            raise ArclinkError, "invalid request: " + req_item
        
        req.add(network, station, stream, loc_id, begin, end, {})

    if format == "INV" or format == "DSEED":  # no need to route
#       req.submit(DEFAULT_ADDR, user, passwd)
# send to primary nodei
        req.submit(myaddr, user, passwd)
        req_ok = [ req ]
        req_fail = None
    else:
        (req_ok, req_fail) = mgr.route_request(req)

    obj.content_type = "text/html; charset=UTF-8"
    obj.write(submit_head)

    for req in req_ok:
        obj.write('<b>Request routed to %s at %s, ID=%s</b><br>\n' % \
            (req.dcname, time.ctime(), req.id))
    
        obj.write('Click <a href="status?sesskey=%s&arclink=%s">here</a> '
            'to see the status of all your requests at %s.<br>\n' % \
            (sesskey, req.address, req.dcname))
        
        obj.write('Confirmation will be sent to <b>%s</b>.<br>\n' % (user,))
        obj.write('Below is the ArcLink request you just submitted.<br>\n')
        obj.write('<pre>\n')
        req.dump(obj)
        obj.write('\n</pre>\n')
        fd = os.popen("%s -I'From: %s' -I'To: %s' -I'Subject: %s' -a'Message-ID:' -A'X-Loop: %s' | %s -f%s %s" % \
            (FORMAIL_BIN, EMAIL_FROM, user, EMAIL_SUBJECT, EMAIL_ADDR,
            SENDMAIL_BIN, EMAIL_ADDR, user), "w")

        try:
            print("The following ArcLink request has been sent to %s\n" % (req.dcname), file=fd)

            req.dump(fd)
            print("\nClick here to see the status of all your requests at %s:" % (req.dcname), file=fd)
            print("http://%s%s/status?arclink=%s&user=%s" % (obj.hostname, obj.uri.rsplit("/", 1)[0], req.address, user), file=fd)
        
        finally:
            fd.close()

    if req_fail is not None:
        obj.write('The following data appears to be temporarily '
            'unavailable, because no ArcLink connection could be '
            'established to servers where this data is located.<br>')
        
        obj.write('<pre>\n')
        req_fail.dump(obj)
        obj.write('\n</pre>\n')
        
        obj.write('Click <a href="submit?sesskey=%s&arclink=%s&name=%s'
            '&inst=%s&mail=%s&start_date=%s&start_time=%s&end_date=%s'
            '&end_time=%s&format=%s&compression=%s%s">here</a> to retry.<br>\n' % \
            (sesskey, myaddr, name, inst, mail, start_date, start_time,
            end_date, end_time, format, compression,
            "".join([ "&rq=%s_%s_%s_%s" % (x[0], x[1], x[3], x[2]) \
                for x in req_fail.content ])))
        
    obj.write(submit_tail)
 
def status(obj, args):
    myaddr = args.get("arclink")
    req_id = args.get("req_id", "ALL")
    
    try:
        (host, port) = myaddr.split(':')
        port = int(port)
    except (AttributeError, ValueError):
        raise ArclinkError, "invalid ArcLink address"

    (sesskey, user, passwd) = init_session(obj, args)
    
    arcl.open_connection(host, port, user, passwd, timeout=ARCLINK_TIMEOUT)
    try:
        arcl.send_command("USER_IP %s" % obj.remote_host)
    except:
        pass

    try:
        obj.content_type = "text/html; charset=UTF-8"
        obj.write(status_head)
    
        logs.debug("connected to %s at %s" % (arcl.software, arcl.organization))

        arcl_status = arcl.get_status(req_id)
        logs.debug("got request status")

        for req in arcl_status.request:
            if req.error:
                req_status = "ERROR"
            elif req.ready:
                req_status = "READY"
            else:
                req_status = "PROCESSING"

            obj.write('Request ID: %s, Type: %s, Args: %s<br>\n' % \
                (req.id, req.type, req.args))
            obj.write('Status: %s, Size: %d, Info: %s<br>\n' % \
                (req_status, req.size, req.message))
            
            if req.user != "":
                obj.write('User: %s, Institution: %s<br>\n' % (req.user, req.institution))

            if req.ready and not req.error and req.size > 0:
                obj.write('<a href="download?sesskey=%s&arclink=%s&req_id=%s">Download!</a>&nbsp;' % \
                    (sesskey, myaddr, req.id))

            obj.write('<a href="purge?sesskey=%s&arclink=%s&req_id=%s">Delete!</a><br>' % \
                (sesskey, myaddr, req.id))

            for vol in req.volume:
                obj.write(4 * '&nbsp;' + 'Volume ID: %s, Status: %s, Size: %d, Info: %s<br>\n' % \
                    (vol.id, arclink_status_string(vol.status), vol.size, vol.message))

                if vol.status == STATUS_OK or vol.status == STATUS_WARN:
                    obj.write(4 * '&nbsp;' + '<a href="download?sesskey=%s&arclink=%s&req_id=%s&vol_id=%s">Download!</a><br>' % \
                        (sesskey, myaddr, req.id, vol.id))
                
                for rqln in vol.line:
                    obj.write('<br>\n')
                    obj.write(8 * '&nbsp;' + '<font face="courier">%s</font><br>\n' % (rqln.content,))
                    obj.write(8 * '&nbsp;' + 'Status: %s, Size: %d, Info: %s<br>\n' % \
                        (arclink_status_string(rqln.status), rqln.size, rqln.message))
     
            obj.write('<br>\n')
            
    finally:
        logs.debug("closing connection")
        arcl.close_connection()

    obj.write(status_tail)
 
def download(obj, args):
    myaddr = args.get("arclink")
    req_id = args.get("req_id")
    vol_id = args.get("vol_id")
    
    try:
        (host, port) = myaddr.split(':')
        port = int(port)
    except (AttributeError, ValueError):
        raise ArclinkError, "invalid ArcLink address"

    if req_id is None or len(req_id) == 0:
        raise ArclinkError, "missing request ID "

    (sesskey, user, passwd) = init_session(obj, args)
    
    arcl.open_connection(host, port, user, passwd, timeout=ARCLINK_TIMEOUT)
    try:
        logs.debug("connected to %s at %s" % (arcl.software, arcl.organization))

        arcl_status = arcl.get_status(req_id)
        logs.debug("got request status")

        req_type = arcl_status.request[0].type
        fext = ''
        if "compression=bzip2" in arcl_status.request[0].args.split():
            obj.content_type = "application/x-bzip2"
            fext = '.bz2'
        elif req_type == "WAVEFORM" or req_type == "RESPONSE":
            obj.content_type = "application/x-seed"
        else:
            obj.content_type = "application/xml"
            fext = '.xml'
        obj.headers_out['Content-Disposition'] = 'attachment; filename=ArclinkRequest_%s%s' % (str(req_id), fext)

        
        arcl.download_data(obj, req_id, vol_id)
        logs.debug("download finished")

    finally:
        logs.debug("closing connection")
        arcl.close_connection()
 
def purge(obj, args):
    myaddr = args.get("arclink")
    req_id = args.get("req_id")
    
    try:
        (host, port) = myaddr.split(':')
        port = int(port)
    except (AttributeError, ValueError):
        raise ArclinkError, "invalid ArcLink address"

    if req_id is None or len(req_id) == 0:
        raise ArclinkError, "missing request ID "

    (sesskey, user, passwd) = init_session(obj, args)
    
    arcl.open_connection(host, port, user, passwd, timeout=ARCLINK_TIMEOUT)
    try:
        logs.debug("connected to %s at %s" % (arcl.software, arcl.organization))

        arcl_status = arcl.purge(req_id)
        logs.debug("request deleted")

    finally:
        logs.debug("closing connection")
        arcl.close_connection()

    url = "http://%s%s/status?sesskey=%s&arclink=%s" % \
        (obj.hostname, obj.uri.rsplit("/", 1)[0], sesskey, myaddr)
        
    obj.headers_out['Location'] = url
    raise apache.SERVER_RETURN, apache.HTTP_MOVED_PERMANENTLY
 
def newuser(obj, args):
    obj.content_type = "text/plain"
    obj.write("Not implemented")

def nettab(obj, args):
    network = args.get("network", "*")

    db = mgr.get_inventory(network, "*", "*", "*") 
    
    obj.content_type = "application/xml"
    db.save_xml(obj, stylesheet = "http://st82/nettab.xsl")

action_table = {
    "query":    (query,    Set()),
    "select":   (select,   Set()),
    "submit":   (submit,   Set(["rq"])),
    "status":   (status,   Set()),
    "download": (download, Set()),
    "purge":    (purge,    Set()),
    "newuser":  (newuser,  Set()),
    "nettab":   (nettab,   Set())
}

def handler(obj):
    try:
        fname = obj.filename.split("/")[-1]
        item = action_table.get(fname)
        if item is None:
            return apache.HTTP_NOT_FOUND

        (action, multipar) = item

        args = {}
        if obj.args is not None:
            for arg in obj.args.split('&'):
                pv = arg.split('=', 1)
                if pv[0] in multipar:
                    if pv[0] not in args:
                        args[pv[0]] = []
                        
                    if len(pv) == 1:
                        args[pv[0]].append("")
                    else:
                        args[pv[0]].append(urllib.unquote(pv[1]))
                else:
                    if len(pv) == 1:
                        args[pv[0]] = ""
                    else:
                        args[pv[0]] = urllib.unquote(pv[1])

        action(obj, args)

    except ArclinkAuthFailed as e:
        obj.err_headers_out['WWW-Authenticate'] = 'Basic realm="%s"' % (e.dcname,)
        logs.debug("unauthorized")
        return apache.HTTP_UNAUTHORIZED
    except (ArclinkError, socket.error) as e:
        #
        # PLE 2013-03-13: This is where that message
        # "Error: missing user ID (email address)"
        # escapes from, but how does it get raised?
        # 
        logs.error(fname+":"+str(e))
        obj.content_type = "text/plain"
        obj.write("Error: " + "(%s) " % fname + str(e))
    except apache.SERVER_RETURN:
        raise
    except Exception as e:
        logs.error(str(e))
        raise

    return apache.OK

syslog.openlog("webinterface", syslog.LOG_PID, syslog.LOG_LOCAL0)
logs.info("ArcLink webinterface v" + VERSION + " started")

