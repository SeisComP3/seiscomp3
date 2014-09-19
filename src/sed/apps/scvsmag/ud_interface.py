#!/usr/bin/env python
"""
Interface to the UserDisplay.
Created on May 7, 2013

@author: behry
"""

from stompy.simple import Client
import xml.etree.ElementTree as ET
import cStringIO
import datetime
import time
import seiscomp3

class UDException(Exception): pass

class UDConnection:

    def __init__(self, host=None, port=None, topic=None, username=None,
                 password=None):
        try:
            self.topic = topic
            self.username = username
            self.password = password
            self.host = host
            self.port = port
            self.stomp = Client(host=self.host, port=self.port)
            self.stomp.connect(username=self.username, password=self.password)
        except Exception, e:
            raise UDException('Cannot connect to message broker (%s@%s:%d): %s.'\
                               % (username, host, port, e))

    def send(self, msg):
        try:
            self.stomp.put(msg, destination=self.topic)
        except Exception, e:
            seiscomp3.Logging.error("ActiveMQ connection lost.")
            # Wait for a bit in case the ActiveMQ broker is restarting
            time.sleep(10)
            try:
                del self.stomp
                self.stomp = Client(host=self.host, port=self.port)
                self.stomp.connect(username=self.username, password=self.password)
            except Exception, e:
                raise UDException('Cannot reconnect to server: %s' % e)
            seiscomp3.Logging.info('Connection re-established.')


class HeartBeat(UDConnection):

    def __init__(self, host, port, topic, username, password):
        UDConnection.__init__(self, host, port, topic, username, password)

    def send_hb(self):
        dt = datetime.datetime.utcnow()
        now = dt.strftime('%a %B %d %H:%M:%S %Y')
        root = ET.Element('hb')
        root.set('originator', 'vs.9')
        root.set('sender', 'vs.9')
        root.set('timestamp', now)
        tree = ET.ElementTree(root)
        f = cStringIO.StringIO()
        tree.write(f, encoding="UTF-8", xml_declaration=True, method='xml')
        msg = f.getvalue()
        self.send(msg)
        return msg

class CoreEventInfo(UDConnection):

    def __init__(self, host, port, topic, username, password):
        self.id = -9
        self.mag = -9.9
        self.mag_uncer = -9.9
        self.lat = -999.9
        self.lat_uncer = -999.9
        self.lon = -999.9
        self.lon_uncer = -999.9
        self.dep = -9.9
        self.dep_uncer = -9.9
        self.o_time = datetime.datetime.utcnow().strftime('%Y-%m-%dT%H:%M:%SZ')
        self.o_time_uncer = -9.9
        self.likelihood = -9.9
        self.type = 0
        self.ver = 0
        self.mag_units = "Mw"
        self.mag_uncer_units = "Mw"
        self.lat_units = "deg"
        self.lat_uncer_units = "deg"
        self.lon_units = "deg"
        self.lon_uncer_units = "deg"
        self.dep_units = "km"
        self.dep_uncer_units = "km"
        self.o_time_units = "UTC"
        self.o_time_uncer_units = "sec"
        self.MessageTypeString = [ "new", "update", "delete" ]
        UDConnection.__init__(self, host, port, topic, username, password)

    def indent(self, elem, level=0, space=''):
        i = "\n" + level * space
        if len(elem):
            if not elem.text or not elem.text.strip():
                elem.text = i + space
            if not elem.tail or not elem.tail.strip():
                elem.tail = i
            for elem in elem:
                self.indent(elem, level + 1, space)
            if not elem.tail or not elem.tail.strip():
                elem.tail = i
        else:
            if level and (not elem.tail or not elem.tail.strip()):
                elem.tail = i

    def DMMessageEncoder(self, test=False):
        root = ET.Element('event_message')
        if test:
            root.set('category', 'test')
        root.set('message_type', self.MessageTypeString[self.type])
        root.set('orig_sys', 'dm')
        root.set('version', str(self.ver))
        ci = ET.SubElement(root, 'core_info')
        ci.set('id', str(self.id))
        mg = ET.SubElement(ci, 'mag')
        mg.set('units', self.mag_units)
        mg.text = str(self.mag)
        mgs = ET.SubElement(ci, 'mag_uncer')
        mgs.set('units', self.mag_uncer_units)
        mgs.text = str(self.mag_uncer)
        lat = ET.SubElement(ci, 'lat')
        lat.set('units', self.lat_units)
        lat.text = str(self.lat)
        lats = ET.SubElement(ci, 'lat_uncer')
        lats.set('units', self.lat_uncer_units)
        lats.text = str(self.lat_uncer)
        lon = ET.SubElement(ci, 'lon')
        lon.set('units', self.lon_units)
        lon.text = str(self.lon)
        lons = ET.SubElement(ci, 'lon_uncer')
        lons.set('units', self.lon_uncer_units)
        lons.text = str(self.lon_uncer)
        dp = ET.SubElement(ci, 'depth')
        dp.set('units', self.dep_units)
        dp.text = str(self.dep)
        dps = ET.SubElement(ci, 'depth_uncer')
        dps.set('units', self.dep_uncer_units)
        dps.text = str(self.dep_uncer)
        ot = ET.SubElement(ci, 'orig_time')
        ot.set('units', self.o_time_units)
        ot.text = str(self.o_time)
        ots = ET.SubElement(ci, 'orig_time_uncer')
        ots.set('units', self.o_time_uncer_units)
        ots.text = str(self.o_time_uncer)
        lh = ET.SubElement(ci, 'likelihood')
        lh.text = str(self.likelihood)
        self.indent(root, space='    ')
        tree = ET.ElementTree(root)
        f = cStringIO.StringIO()
        tree.write(f, encoding="UTF-8", xml_declaration=True, method='xml')
        msg = f.getvalue()
        return msg

if __name__ == '__main__':
    pass
