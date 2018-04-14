#***************************************************************************** 
# routing.py
#
# XML I/O for Routing (new schema)
#
# (c) 2006 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import xmlwrap as _xmlwrap
from seiscomp.db import DBError

try:
    from xml.etree import cElementTree as ET  # Python 2.5?
except ImportError:
    import cElementTree as ET

_root_tag = "{http://geofon.gfz-potsdam.de/ns/Routing/1.0/}routing"
 
#***************************************************************************** 
# XML IN
#***************************************************************************** 

def _arclink_in(xarclink, route):
    if xarclink.action == "delete":
        try:
            route.remove_arclink(xarclink.address, xarclink.start)
        except KeyError:
            pass
        
        return

    try:
        arclink = route.arclink[xarclink.address][xarclink.start]

    except KeyError:
        arclink = route.insert_arclink(xarclink.address, xarclink.start)
        
    xarclink._copy_to(arclink)

def _seedlink_in(xseedlink, route):
    if xseedlink.action == "delete":
        try:
            route.remove_seedlink(xseedlink.address)
        except KeyError:
            pass
        
        return

    try:
        seedlink = route.seedlink[xseedlink.address]

    except KeyError:
        seedlink = route.insert_seedlink(xseedlink.address)
        
    xseedlink._copy_to(seedlink)

def _route_in(xroute, routing):
    if xroute.action == "delete":
        try:
            routing.remove_route(xroute.networkCode, xroute.stationCode, xroute.locationCode, xroute.streamCode)
        except KeyError:
            pass
        
        return

    try:
        route = routing.route[xroute.networkCode][xroute.stationCode][xroute.locationCode][xroute.streamCode]
        if route.publicID != xroute.publicID:
            routing.remove_route(xroute.networkCode, xroute.stationCode, xroute.locationCode, xroute.streamCode)
            raise KeyError

    except KeyError:
        route = routing.insert_route(xroute.networkCode, xroute.stationCode, xroute.locationCode, xroute.streamCode, publicID=xroute.publicID)
        xroute.publicID = route.publicID
        
    xroute._copy_to(route)

    for xarclink in xroute.arclink:
        _arclink_in(xarclink, route)
   
    for xseedlink in xroute.seedlink:
        _seedlink_in(xseedlink, route)
   
def _access_in(xacc, routing):
    if xacc.action == "delete":
        try:
            routing.remove_access(xacc.networkCode, xacc.stationCode, xacc.locationCode,
                xacc.streamCode, xacc.user, xacc.start)
        except KeyError:
            pass

        return

    try:
        acc = routing.access[xacc.networkCode][xacc.stationCode][xacc.locationCode][xacc.streamCode][xacc.user][xacc.start]
            
    except KeyError:
        acc = routing.insert_access(xacc.networkCode, xacc.stationCode, xacc.locationCode,
            xacc.streamCode, xacc.user, xacc.start)

    xacc._copy_to(acc)
    
def _xmldoc_in(xrouting, routing, use_access):
    for xroute in xrouting.route:
        _route_in(xroute, routing)

    if use_access:
        for xacc in xrouting.access:
            _access_in(xacc, routing)
    
#***************************************************************************** 
# XML OUT
#***************************************************************************** 

def _arclink_out(xroute, arclink, modified_after):
    if modified_after is None or arclink.last_modified >= modified_after:
        xarclink = xroute._new_arclink()
        xarclink._copy_from(arclink)
        xroute._append_child(xarclink)
        return True

    return False

def _seedlink_out(xroute, seedlink, modified_after):
    if modified_after is None or seedlink.last_modified >= modified_after:
        xseedlink = xroute._new_seedlink()
        xseedlink._copy_from(seedlink)
        xroute._append_child(xseedlink)
        return True

    return False

def _route_out(xrouting, route, modified_after):
    xroute = xrouting._new_route()

    if modified_after is None or route.last_modified >= modified_after:
        xroute._copy_from(route)
        retval = True
    else:
        xroute.networkCode = route.networkCode
        xroute.stationCode = route.stationCode
        retval = False

    for i in route.arclink.itervalues():
        for j in i.itervalues():
            retval |= _arclink_out(xroute, j, modified_after)

    for i in route.seedlink.itervalues():
        retval |= _seedlink_out(xroute, i, modified_after)

    if retval:
        xrouting._append_child(xroute)
    
    return retval

def _access_out(xrouting, acc, modified_after):
    xacc = xrouting._new_access()

    if modified_after is None or acc.last_modified >= modified_after:
        xacc._copy_from(acc)
        xrouting._append_child(xacc)
        return True

    return False

def _xmldoc_out(xrouting, routing, use_access, modified_after):
    for i in routing.route.itervalues():
        for j in i.itervalues():
            for k in j.itervalues():
                for l in k.itervalues():
                    _route_out(xrouting, l, modified_after)

    if use_access:
        for i in routing.access.itervalues():
            for j in i.itervalues():
                for k in j.itervalues():
                    for l in k.itervalues():
                        for m in l.itervalues():
                            for n in m.itervalues():
                                _access_out(xrouting, n, modified_after)

#***************************************************************************** 
# Incremental Parser
#*****************************************************************************

class _IncrementalParser(object):
    def __init__(self, routing, use_access):
        self.__routing = routing
        self.__use_access = use_access
        self.__p = ET.XMLTreeBuilder()

    def feed(self, s):
        self.__p.feed(s)

    def close(self):
        root = self.__p.close()
        if root.tag != _root_tag:
            raise DBError, "unrecognized root element: " + root.tag

        xrouting = _xmlwrap.xml_Routing(root)
        _xmldoc_in(xrouting, self.__routing, self.__use_access)
        
#***************************************************************************** 
# Public functions
#*****************************************************************************

def make_parser(routing, use_access=False):
    return _IncrementalParser(routing, use_access)

def xml_in(routing, src, use_access=False):
    doc = ET.parse(src)
    root = doc.getroot()
    if root.tag != _root_tag:
        raise DBError, "unrecognized root element: " + root.tag

    xrouting = _xmlwrap.xml_Routing(root)
    _xmldoc_in(xrouting, routing, use_access)

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

def xml_out(routing, dest, use_access=False, modified_after=None, stylesheet=None, indent=True):
    xrouting = _xmlwrap.xml_Routing()

    _xmldoc_out(xrouting, routing, use_access, modified_after)

    if isinstance(dest, basestring):
        fd = file(dest, "w")
    elif hasattr(dest, "write"):
        fd = dest
    else:
        raise TypeError, "invalid file object"

    try:
        filename = fd.name
    except AttributeError:
        filename = '<???>'

    fd.write('<?xml version="1.0" encoding="utf-8"?>\n')

    if stylesheet != None:
        fd.write('<?xml-stylesheet type="application/xml" href="%s"?>\n' % \
            (stylesheet,))
    
    if indent is True:
        _indent(xrouting._element)
    
    ET.ElementTree(xrouting._element).write(fd, encoding="utf-8")
    
    if isinstance(dest, basestring):
        fd.close()

