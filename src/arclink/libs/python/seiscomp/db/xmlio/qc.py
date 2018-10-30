#***************************************************************************** 
# qc.py
#
# XML I/O for Quality Control
#
# (c) 2009 Andres Heinloo, GFZ Potsdam
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

_root_tag = "{http://geofon.gfz-potsdam.de/ns/QualityControl/1.0/}qualityControl"
 
#***************************************************************************** 
# XML IN
#***************************************************************************** 

def _log_in(xlog, qc):
    if xlog.action == "delete":
        try:
            qc.remove_log(xlog.net_code, xlog.sta_code, xlog.str_code,
                xlog.loc_code, xlog.start, xlog.end)
        except KeyError:
            pass

        return

    try:
        log = qc.log[xlog.net_code, xlog.sta_code, xlog.str_code,
            xlog.loc_code][xlog.start, xlog.end]
            
    except KeyError:
        log = qc.insert_log(xlog.net_code, xlog.sta_code, xlog.str_code,
            xlog.loc_code, xlog.start, xlog.end)

    xlog._copy_to(log)

def _outage_in(xoutage, qc):
    if xoutage.action == "delete":
        try:
            qc.remove_outage(xoutage.net_code, xoutage.sta_code, xoutage.str_code,
                xoutage.loc_code, xoutage.start)
        except KeyError:
            pass

        return

    try:
        outage = qc.outage[xoutage.net_code, xoutage.sta_code, xoutage.str_code,
            xoutage.loc_code][xoutage.start]
            
    except KeyError:
        outage = qc.insert_outage(xoutage.net_code, xoutage.sta_code, xoutage.str_code,
            xoutage.loc_code, xoutage.start)

    xoutage._copy_to(outage)

def _wfq_in(xwfq, qc):
    if xwfq.action == "delete":
        try:
            qc.remove_waveform_quality(xwfq.net_code, xwfq.sta_code, xwfq.str_code,
                xwfq.loc_code, xwfq.start, xwfq.type, xwfq.parameter)
        except KeyError:
            pass

        return

    try:
        wfq = qc.waveform_quality[xwfq.net_code, xwfq.sta_code, xwfq.str_code,
            xwfq.loc_code][xwfq.start, xwfq.type, xwfq.parameter]
            
    except KeyError:
        wfq = qc.insert_waveform_quality(xwfq.net_code, xwfq.sta_code, xwfq.str_code,
            xwfq.loc_code, xwfq.start, xwfq.type, xwfq.parameter)

    xwfq._copy_to(wfq)

def _xmldoc_in(xqc, qc):
    for xlog in xqc.log:
        _log_in(xlog, qc)

    for xoutage in xqc.outage:
        _outage_in(xoutage, qc)

    for xwfq in xqc.waveform_quality:
        _wfq_in(xwfq, qc)

#***************************************************************************** 
# XML OUT
#***************************************************************************** 

def _log_out(xqc, log):
    xlog = xqc._new_log()
    xlog._copy_from(log)
    xqc._append_child(xlog)
    return True

def _outage_out(xqc, outage):
    xoutage = xqc._new_outage()
    xoutage._copy_from(outage)
    xqc._append_child(xoutage)
    return True

def _wfq_out(xqc, wfq):
    xwfq = xqc._new_waveform_quality()
    xwfq._copy_from(wfq)
    xqc._append_child(xwfq)
    return True

def _xmldoc_out(xqc, qc):
    for i in qc.log.itervalues():
        for j in i.itervalues():
            _log_out(xqc, j)

    for i in qc.outage.itervalues():
        for j in i.itervalues():
            _outage_out(xqc, j)

    for i in qc.waveform_quality.itervalues():
        for j in i.itervalues():
            _wfq_out(xqc, j)

#***************************************************************************** 
# Incremental Parser
#*****************************************************************************

class _IncrementalParser(object):
    def __init__(self, qc):
        self.__qc = qc
        self.__p = ET.XMLTreeBuilder()

    def feed(self, s):
        self.__p.feed(s)

    def close(self):
        root = self.__p.close()
        if root.tag != _root_tag:
            raise DBError, "unrecognized root element: " + root.tag

        xqc = _xmlwrap.xml_quality_control(root)
        _xmldoc_in(xqc, self.__qc)
        
#***************************************************************************** 
# Public functions
#*****************************************************************************

def make_parser(qc):
    return _IncrementalParser(qc)

def xml_in(qc, src):
    doc = ET.parse(src)
    root = doc.getroot()
    if root.tag != _root_tag:
        raise DBError, "unrecognized root element: " + root.tag

    xqc = _xmlwrap.xml_quality_control(root)
    _xmldoc_in(xqc, qc)

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

def xml_out(qc, dest, stylesheet=None, indent=True):
    xqc = _xmlwrap.xml_quality_control()

    _xmldoc_out(xqc, qc)

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

    if stylesheet is not None:
        fd.write('<?xml-stylesheet type="application/xml" href="%s"?>\n' % \
            (stylesheet,))
    
    if indent is True:
        _indent(xqc._element)
    
    ET.ElementTree(xqc._element).write(fd, encoding="utf-8")
    
    if isinstance(dest, basestring):
        fd.close()

