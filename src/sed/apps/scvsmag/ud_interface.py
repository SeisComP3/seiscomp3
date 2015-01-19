#!/usr/bin/env python
"""
Interface to the UserDisplay.
Created on May 7, 2013

@author: behry
"""

from stompy.simple import Client
import cStringIO
import datetime
import time
import seiscomp3
from seiscomp3.IO import Exporter, ExportSink
import cStringIO
import lxml.etree as ET
import os


class UDException(Exception): pass


class Sink(ExportSink):
    def __init__(self, buf):
        ExportSink.__init__(self)
        self.buf = buf
        self.written = 0

    def write(self, data, size):
        self.buf.write(data[:size])
        self.written += size
        return size


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

    def __init__(self, host, port, topic, username, password,
                 format='qml1.2-rt'):
        UDConnection.__init__(self, host, port, topic, username, password)
        self.format = format

    def send_hb(self):
        dt = datetime.datetime.utcnow()
        root = ET.Element('hb')
        root.set('originator', 'vssc3')
        root.set('sender', 'vssc3')
        if self.format == 'qml1.2-rt':
            now = dt.strftime('%Y-%m-%dT%H:%M:%S.%fZ')
            root.set('xmlns', 'http://heartbeat.reakteu.org')
        else:
            now = dt.strftime('%a %B %d %H:%M:%S %Y')
        root.set('timestamp', now)
        tree = ET.ElementTree(root)
        f = cStringIO.StringIO()
        tree.write(f, encoding="UTF-8", xml_declaration=True, method='xml')
        msg = f.getvalue()
        self.send(msg)
        return msg


class CoreEventInfo(UDConnection):

    def __init__(self, host, port, topic, username, password,
                 format='qml1.2-rt'):
        UDConnection.__init__(self, host, port, topic, username, password)
        ei = seiscomp3.System.Environment.Instance()
        self.transform = None
        if format == 'qml1.2-rt':
            xslt = ET.parse(os.path.join(ei.shareDir(), 'scvsmaglog',
                                         'sc3ml_0.7__quakeml_1.2-RT_eewd.xsl'))
            self.transform = ET.XSLT(xslt)
        elif format == 'shakealert':
            xslt = ET.parse(os.path.join(ei.shareDir(), 'scvsmaglog',
                            'sc3ml_0.7__shakealert.xsl'))
            self.transform = ET.XSLT(xslt)
        elif format == 'sc3ml':
            pass
        else:
            seiscomp3.Logging.error('Currently supported AMQ message formats \
            are sc3ml, qml1.2-rt, and shakealert.')

    def message_encoder(self, ep, pretty_print=True):
        exp = Exporter.Create('trunk')
        io = cStringIO.StringIO()
        sink = Sink(io)
        exp.write(sink, ep)
        # apply XSLT
        dom = ET.fromstring(io.getvalue())
        if self.transform is not None:
            dom = self.transform(dom)
        return ET.tostring(dom, pretty_print=pretty_print)

if __name__ == '__main__':
    pass
