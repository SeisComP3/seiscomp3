################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# RequestOptions -- HTTP GET request parameters
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################

from __future__ import absolute_import, division, print_function

import fnmatch
import math
import re

from twisted.web import http

from seiscomp3.Core import Time
from seiscomp3 import Logging, Math

from .utils import py3ustr, py3ustrlist


class RequestOptions:

    # the match() method matched only patterns at the beginning of a string,
    # since we have to ensure that no invalid character is present we use the
    # search() method in combination with a negated pattern instead
    FloatChars = re.compile(r'[^-0-9.]').search
    ChannelChars = re.compile(r'[^A-Za-z0-9*?]').search
    ChannelExtChars = re.compile(r'[^A-Za-z0-9*?+\-_]').search
    TimeFormats = ['%FT%T.%f',    # YYYY-MM-DDThh:mm:ss.ssssss
                   '%Y-%jT%T.%f',  # YYYY-DDDThh:mm:ss.ssssss
                   '%FT%T',       # YYYY-MM-DDThh:mm:ss
                   '%Y-%jT%T',    # YYYY-DDDThh:mm:ss
                   '%FT%R',       # YYYY-MM-DDThh:mm
                   '%Y-%jT%R',    # YYYY-DDDThh:mm
                   '%FT%H',       # YYYY-MM-DDThh
                   '%Y-%jT%H',    # YYYY-DDDThh
                   '%F',          # YYYY-MM-DD
                   '%Y-%j',       # YYYY-DDD
                   '%Y',          # YYYY
                   ]
    BooleanTrueValues = ['1', 'true', 't', 'yes', 'y']
    BooleanFalseValues = ['0', 'false', 'f', 'no', 'n']
    OutputFormats = []  # override in derived classes

    PStart = ['starttime', 'start']
    PEnd = ['endtime', 'end']
    PStartBefore = ['startbefore']
    PStartAfter = ['startafter']
    PEndBefore = ['endbefore']
    PEndAfter = ['endafter']
    SimpleTimeParams = PStart + PEnd
    WindowTimeParams = PStartBefore + PStartAfter + PEndBefore + PEndAfter
    TimeParams = SimpleTimeParams + WindowTimeParams

    PNet = ['network', 'net']
    PSta = ['station', 'sta']
    PLoc = ['location', 'loc']
    PCha = ['channel', 'cha']
    StreamParams = PNet + PSta + PLoc + PCha

    PMinLat = ['minlatitude', 'minlat']
    PMaxLat = ['maxlatitude', 'maxlat']
    PMinLon = ['minlongitude', 'minlon']
    PMaxLon = ['maxlongitude', 'maxlon']
    PLat = ['latitude', 'lat']
    PLon = ['longitude', 'lon']
    PMinRadius = ['minradius']
    PMaxRadius = ['maxradius']
    GeoRectParams = PMinLat + PMaxLat + PMinLon + PMaxLon
    GeoCircleParams = PLat + PLon + PMinRadius + PMaxRadius
    GeoParams = GeoRectParams + GeoCircleParams

    PFormat = ['format']
    PNoData = ['nodata']
    OutputParams = PFormat + PNoData

    POSTParams = OutputParams

    #---------------------------------------------------------------------------
    class Channel:
        def __init__(self):
            self.net = None
            self.sta = None
            self.loc = None
            self.cha = None

        def matchNet(self, value):
            return self.match(value, self.net)

        def matchSta(self, value):
            return self.match(value, self.sta)

        def matchLoc(self, value):
            return self.match(value, self.loc, True)

        def matchCha(self, value):
            return self.match(value, self.cha)

        @staticmethod
        def match(value, globList, testEmpty=False):
            if not globList:
                return True

            for glob in globList:
                if testEmpty and value == '' and glob == '--':
                    return True
                if fnmatch.fnmatchcase(value, glob):
                    return True

            return False

    #---------------------------------------------------------------------------
    class Time:

        def __init__(self):
            self.simpleTime = True
            self.start = None
            self.end = None
            # window time only
            self.startBefore = None
            self.startAfter = None
            self.endBefore = None
            self.endAfter = None

        # used by FDSN Station and DataSelect
        def match(self, start, end=None):
            # simple time: limit to epochs intersecting with the specified time
            # range
            res = (self.start is None or end is None or end >= self.start) and \
                  (self.end is None or start <= self.end)

            # window time: limit to epochs strictly starting or ending before or
            # after a specified time value
            if not self.simpleTime:
                res = res and \
                    (self.startBefore is None or (
                        start is not None and start < self.startBefore)) and \
                    (self.startAfter is None or (
                        start is not None and start > self.startAfter)) and \
                    (self.endBefore is None or (
                        end is not None and end < self.endBefore)) and \
                    (self.endAfter is None or end is None or end > self.endAfter)

            return res

    #---------------------------------------------------------------------------
    class Geo:

        #-----------------------------------------------------------------------
        class BBox:

            def __init__(self):
                self.minLat = None
                self.maxLat = None
                self.minLon = None
                self.maxLon = None

            def dateLineCrossing(self):
                return self.minLon and self.maxLon and self.minLon > self.maxLon

        #-----------------------------------------------------------------------
        class BCircle:

            def __init__(self):
                self.lat = None
                self.lon = None
                self.minRad = None
                self.maxRad = None

            #-------------------------------------------------------------------
            # Calculates outer bounding box
            def calculateBBox(self):
                def rad(degree):
                    return math.radians(degree)

                def deg(radians):
                    return math.degrees(radians)

                b = RequestOptions.Geo.BBox()
                if self.maxRad is None or self.maxRad >= 180:
                    return b

                b.minLat = self.lat - self.maxRad
                b.maxLat = self.lat + self.maxRad
                if b.minLat > -90 and b.maxLat < 90:
                    dLon = deg(math.asin(math.sin(rad(self.maxRad) /
                                                  math.cos(rad(self.lat)))))
                    b.minLon = self.lon - dLon
                    if b.minLon < -180:
                        b.minLon += 360
                    b.maxLon = self.lon + dLon
                    if b.maxLon > 180:
                        b.maxLon -= 360
                else:
                    # pole within distance: one latitude and no longitude
                    # restrictions remains
                    if b.minLat <= -90:
                        b.minLat = None
                    else:
                        b.maxLat = None
                    b.minLon = None
                    b.maxLon = None

                return b

        #-----------------------------------------------------------------------
        def __init__(self):
            self.bBox = None
            self.bCircle = None

        #-----------------------------------------------------------------------
        def match(self, lat, lon):
            if self.bBox is not None:
                b = self.bBox
                if b.minLat is not None and lat < b.minLat:
                    return False
                if b.maxLat is not None and lat > b.maxLat:
                    return False
                # date line crossing if minLon > maxLon
                if b.dateLineCrossing():
                    return lon >= b.minLon or lon <= b.maxLon
                if b.minLon is not None and lon < b.minLon:
                    return False
                if b.maxLon is not None and lon > b.maxLon:
                    return False
                return True
            elif self.bCircle:
                c = self.bCircle
                dist = Math.delazi(c.lat, c.lon, lat, lon)
                if c.minRad is not None and dist[0] < c.minRad:
                    return False
                if c.maxRad is not None and dist[0] > c.maxRad:
                    return False
                return True

            return False

    #---------------------------------------------------------------------------
    def __init__(self, args=None):
        self.service = ""
        self.accessTime = Time.GMT()
        self.userName = None

        self.time = None
        self.channel = None
        self.geo = None

        self.noData = http.NO_CONTENT
        self.format = None

        # transform keys to lower case
        self._args = {}
        if args is not None:
            for k, v in args.items():
                self._args[py3ustr(k.lower())] = py3ustrlist(v)

        self.streams = []  # 1 entry for GET, multipl

    #---------------------------------------------------------------------------
    def parseOutput(self):
        # nodata
        code = self.parseInt(self.PNoData)
        if code is not None:
            if code != http.NO_CONTENT and code != http.NOT_FOUND:
                self.raiseValueError(self.PNoData[0])
            self.noData = code

        # format
        key, value = self.getFirstValue(self.PFormat)
        if value is None:
            # no format specified: default to first in list if available
            if len(self.OutputFormats) > 0:
                self.format = self.OutputFormats[0]
        else:
            value = value.lower()
            if value in self.OutputFormats:
                self.format = value
            else:
                self.raiseValueError(key)

    #---------------------------------------------------------------------------
    def parseChannel(self):
        c = RequestOptions.Channel()

        c.net = self.parseChannelChars(self.PNet, False, True)
        c.sta = self.parseChannelChars(self.PSta)
        c.loc = self.parseChannelChars(self.PLoc, True)
        c.cha = self.parseChannelChars(self.PCha)

        if c.net or c.sta or c.loc or c.cha:
            self.channel = c

    #---------------------------------------------------------------------------
    def parseTime(self, parseWindowTime=False):
        t = RequestOptions.Time()

        # start[time], end[time]
        t.start = self.parseTimeStr(self.PStart)
        t.end = self.parseTimeStr(self.PEnd)

        simpleTime = t.start is not None or t.end is not None

        # [start,end][before,after]
        if parseWindowTime:
            t.startBefore = self.parseTimeStr(self.PStartBefore)
            t.startAfter = self.parseTimeStr(self.PStartAfter)
            t.endBefore = self.parseTimeStr(self.PEndBefore)
            t.endAfter = self.parseTimeStr(self.PEndAfter)

            windowTime = t.startBefore is not None or t.startAfter is not None or \
                t.endBefore is not None or t.endAfter is not None
            if simpleTime or windowTime:
                self.time = t
                self.time.simpleTime = not windowTime

        elif simpleTime:
            self.time = t
            self.time.simpleTime = True

    #---------------------------------------------------------------------------
    def parseGeo(self):
        # bounding box (optional)
        b = RequestOptions.Geo.BBox()
        b.minLat = self.parseFloat(self.PMinLat, -90, 90)
        b.maxLat = self.parseFloat(self.PMaxLat, -90, 90)
        if b.minLat is not None and b.maxLat is not None and \
           b.minLat > b.maxLat:
            raise ValueError("%s exceeds %s" % (self.PMinLat[0],
                                                self.PMaxLat[0]))

        b.minLon = self.parseFloat(self.PMinLon, -180, 180)
        b.maxLon = self.parseFloat(self.PMaxLon, -180, 180)
        # maxLon < minLon -> date line crossing

        hasBBoxParam = b.minLat is not None or b.maxLat is not None or \
                       b.minLon is not None or b.maxLon is not None

        # bounding circle (optional)
        c = RequestOptions.Geo.BCircle()
        c.lat = self.parseFloat(self.PLat, -90, 90)
        c.lon = self.parseFloat(self.PLon, -180, 180)
        c.minRad = self.parseFloat(self.PMinRadius, 0, 180)
        c.maxRad = self.parseFloat(self.PMaxRadius, 0, 180)
        if c.minRad is not None and c.maxRad is not None and \
           c.minRad > c.maxRad:
            raise ValueError("%s exceeds %s" % (self.PMinRadius[0],
                                                self.PMaxRadius[0]))

        hasBCircleRadParam = c.minRad is not None or c.maxRad is not None
        hasBCircleParam = c.lat is not None or c.lon is not None or \
            hasBCircleRadParam

        # bounding box and bounding circle may not be combined
        if hasBBoxParam and hasBCircleParam:
            raise ValueError("bounding box and bounding circle parameters " \
                             "may not be combined")
        elif hasBBoxParam:
            self.geo = RequestOptions.Geo()
            self.geo.bBox = b
        elif hasBCircleRadParam:
            self.geo = RequestOptions.Geo()
            if c.lat is None:
                c.lat = .0
            if c.lon is None:
                c.lon = .0
            self.geo.bCircle = c

    #---------------------------------------------------------------------------
    def _assertValueRange(self, key, v, minValue, maxValue):
        if (minValue is not None and v < minValue) or \
           (maxValue is not None and v > maxValue):
            minStr, maxStr = '-inf', 'inf'
            if minValue is not None:
                minStr = str(minValue)
            if maxValue is not None:
                maxStr = str(maxValue)
            raise ValueError("parameter not in domain [%s,%s]: %s" % (
                             minStr, maxStr, key))

    #---------------------------------------------------------------------------
    def raiseValueError(self, key):
        raise ValueError("invalid value in parameter: %s" % key)

    #---------------------------------------------------------------------------
    def getFirstValue(self, keys):
        for key in keys:
            if key in self._args:
                return key, self._args[key][0].strip()

        return None, None

    #---------------------------------------------------------------------------
    def getValues(self, keys):
        v = []
        for key in keys:
            if key in self._args:
                v += self._args[key]
        return v

    #---------------------------------------------------------------------------
    def getListValues(self, keys, lower=False):
        values = set()
        for key in keys:
            if not key in self._args:
                continue
            for vList in self._args[key]:
                for v in vList.split(','):
                    if v is None:
                        continue
                    v = v.strip()
                    if lower:
                        v = v.lower()
                    values.add(v)

        return values

    #---------------------------------------------------------------------------
    def parseInt(self, keys, minValue=None, maxValue=None):
        key, value = self.getFirstValue(keys)

        if value is None:
            return None

        try:
            i = int(value)
        except ValueError:
            raise ValueError("invalid integer value in parameter: %s" % key)
        self._assertValueRange(key, i, minValue, maxValue)
        return i

    #---------------------------------------------------------------------------
    def parseFloat(self, keys, minValue=None, maxValue=None):
        key, value = self.getFirstValue(keys)

        if value is None:
            return None

        if self.FloatChars(value):
            raise ValueError("invalid characters in float parameter: %s " \
                             "(scientific notation forbidden by spec)" % key)

        try:
            f = float(value)
        except ValueError:
            raise ValueError("invalid float value in parameter: %s" % key)
        self._assertValueRange(key, f, minValue, maxValue)
        return f

    #---------------------------------------------------------------------------
    def parseBool(self, keys):
        key, value = self.getFirstValue(keys)

        if value is None:
            return None

        value = value.lower()
        if value in self.BooleanTrueValues:
            return True
        if value in self.BooleanFalseValues:
            return False

        raise ValueError("invalid boolean value in parameter: %s" % key)

    #---------------------------------------------------------------------------
    def parseTimeStr(self, keys):
        key, value = self.getFirstValue(keys)

        if value is None:
            return None

        time = Time()
        timeValid = False
        for fmt in RequestOptions.TimeFormats:
            if time.fromString(value, fmt):
                timeValid = True
                break

        if not timeValid:
            raise ValueError("invalid date format in parameter: %s" % key)

        return time

    #---------------------------------------------------------------------------
    def parseChannelChars(self, keys, allowEmpty=False, useExtChars=False):
        # channel parameters may be specified as a comma separated list and may
        # be repeated several times
        values = None
        for vList in self.getValues(keys):
            if values is None:
                values = []
            for v in vList.split(','):
                v = v.strip()
                if allowEmpty and (v == '--' or len(v) == 0):
                    values.append('--')
                    continue

                if (useExtChars and self.ChannelExtChars(v)) or \
                   (not useExtChars and self.ChannelChars(v)):
                    raise ValueError("invalid characters in parameter: " \
                                     "%s" % keys[0])
                values.append(v)

        return values

    #---------------------------------------------------------------------------
    def parsePOST(self, content):
        nLine = 0

        for line in content:
            nLine += 1
            line = py3ustr(line.strip())

            # ignore empty and comment lines
            if len(line) == 0 or line[0] == '#':
                continue

            # collect parameter (non stream lines)
            toks = line.split("=", 1)
            if len(toks) > 1:
                key = toks[0].strip().lower()

                isPOSTParam = False
                for p in self.POSTParams:
                    if p == key:
                        if key not in self._args:
                            self._args[key] = []
                        self._args[key].append(toks[1].strip())
                        isPOSTParam = True
                        break

                if isPOSTParam:
                    continue

                # time parameters not allowed in POST header
                for p in self.TimeParams:
                    if p == key:
                        raise ValueError("time parameter in line %i not " \
                                         "allowed in POST request" % nLine)

                # stream parameters not allowed in POST header
                for p in self.StreamParams:
                    if p == key:
                        raise ValueError("stream parameter in line %i not " \
                                         "allowed in POST request" % nLine)

                raise ValueError("invalid parameter in line %i" % nLine)

            else:
                # stream parameters
                toks = line.split()
                nToks = len(toks)
                if nToks != 5 and nToks != 6:
                    raise ValueError("invalid number of stream components " \
                                     "in line %i" % nLine)

                ro = RequestOptions()

                # net, sta, loc, cha
                ro.channel = RequestOptions.Channel()
                ro.channel.net = toks[0].split(',')
                ro.channel.sta = toks[1].split(',')
                ro.channel.loc = toks[2].split(',')
                ro.channel.cha = toks[3].split(',')

                msg = "invalid %s value in line %i"
                for net in ro.channel.net:
                    if ro.ChannelChars(net):
                        raise ValueError(msg % ('network', nLine))
                for sta in ro.channel.sta:
                    if ro.ChannelChars(sta):
                        raise ValueError(msg % ('station', nLine))
                for loc in ro.channel.loc:
                    if loc != "--" and ro.ChannelChars(loc):
                        raise ValueError(msg % ('location', nLine))
                for cha in ro.channel.cha:
                    if ro.ChannelChars(cha):
                        raise ValueError(msg % ('channel', nLine))

                # start/end time
                ro.time = RequestOptions.Time()
                ro.time.start = Time()
                for fmt in RequestOptions.TimeFormats:
                    if ro.time.start.fromString(toks[4], fmt):
                        break
                logEnd = "-"
                if len(toks) > 5:
                    ro.time.end = Time()
                    for fmt in RequestOptions.TimeFormats:
                        if ro.time.end.fromString(toks[5], fmt):
                            break
                    logEnd = ro.time.end.iso()

                Logging.debug("ro: %s.%s.%s.%s %s %s" % (ro.channel.net,
                                                         ro.channel.sta, ro.channel.loc, ro.channel.cha,
                                                         ro.time.start.iso(), logEnd))
                self.streams.append(ro)

        if len(self.streams) == 0:
            raise ValueError("at least one stream line is required")


# vim: ts=4 et
