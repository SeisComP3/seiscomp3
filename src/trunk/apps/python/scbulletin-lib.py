#!/usr/bin/env python
# -*- coding: utf-8 -*-

############################################################################
#    Copyright (C) by GFZ Potsdam                                          #
#                                                                          #
#    You can redistribute and/or modify this program under the             #
#    terms of the SeisComP Public License.                                 #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    SeisComP Public License for more details.                             #
############################################################################
#
# 2010/09/23 Marco Olivieri
# minimum weight is now configurable and not fixed to 0.5 that remains the default value.
# If assigning --weight 0.0, scbulletin will count and write out all the picks that where
# associated to the corresponding location
# By default minweight = 0.5 and scbulletin reports only picks with small residuals.
#
# 2017/11/02 Dirk Roessler
# enhanced option: High-precision output for bulletins with precisions given in
# meter or milliseconds. Considered useful for bulletins of local earthquakes.

import sys, traceback
import seiscomp3.Client, seiscomp3.Seismology

minDepthPhaseCount = 3
minweight = 0.5

def time2str(time):
    """
    Convert a seiscomp3.Core.Time to a string
    """
    return time.toString("%Y-%m-%d %H:%M:%S.%f000000")[:23]


def lat2str(lat, enhanced = False):
    if enhanced:
        s = "%.4f " % abs(lat)
    else:
        s = "%.2f " % abs(lat)
    if lat>=0: s+="N"
    else: s+="S"
    return s


def lon2str(lon, enhanced = False):
    if enhanced:
        s = "%.4f " % abs(lon)
    else:
        s = "%.2f " % abs(lon)
    if lon>=0: s+="E"
    else: s+="W"
    return s


def stationCount(org):
    count = 0
    for i in xrange(org.arrivalCount()):
        arr = org.arrival(i)
        #   if arr.weight()> 0.5:
        if arr.weight()> minweight:
            count += 1
    return count



class Bulletin(object):

    def __init__(self, dbq, long=True):
        self._dbq = dbq
        self._long = long
        self._evt = None
        self.format = "autoloc1"
        self.enhanced = False
        self.polarities = False
        self.useEventAgencyID = False

    def _getDistancesArrivalsSorted(self, org):
        # sort arrival list by distance
        dist_arr = []
        for i in xrange(org.arrivalCount()):
            arr = org.arrival(i)
            dist_arr.append((arr.distance(), arr))
        dist_arr.sort()
        return dist_arr


    def _getPicks(self, org):
        if self._dbq is None: return {}
        orid = org.publicID()
        pick = {}
        for obj in self._dbq.getPicks(orid):
            p = seiscomp3.DataModel.Pick.Cast(obj)
            key = p.publicID()
            pick[key] = p
        return pick


    def _getAmplitudes(self, org):
        if self._dbq is None: return {}
        orid = org.publicID()
        ampl = {}
        for obj in self._dbq.getAmplitudesForOrigin(orid):
            amp = seiscomp3.DataModel.Amplitude.Cast(obj)
            key = amp.publicID()
            ampl[key] = amp
        return ampl


    def _printOriginAutoloc3(self, org, extra=False):
        orid = org.publicID()

        dist_arr = self._getDistancesArrivalsSorted(org)
        pick = self._getPicks(org)
        ampl = self._getAmplitudes(org)
        enhanced = self.enhanced


        try:
            depthPhaseCount = org.quality().depthPhaseCount()
        except:
            depthPhaseCount = 0
            for dist, arr in dist_arr:
                wt  = arr.weight()
                pha = arr.phase().code()
                #  if (pha[0] in ["p","s"] and wt >= 0.5 ):
                if (pha[0] in ["p","s"] and wt >= minweight ):
                    depthPhaseCount += 1

        txt = ""

        evt = self._evt
        if not evt and self._dbq:
            evt = self._dbq.getEvent(orid)

        if evt:
            txt += "Event:\n"
            txt += "    Public ID              %s\n" % evt.publicID()
            if extra:
                txt += "    Preferred Origin ID    %s\n" % evt.preferredOriginID()
                txt += "    Preferred Magnitude ID %s\n" % evt.preferredMagnitudeID()
            try:
                type = evt.type()
                txt += "    Type                   %s\n" % seiscomp3.DataModel.EEventTypeNames.name(type)
            except: pass
            txt += "    Description\n"
            for i in xrange(evt.eventDescriptionCount()):
                evtd = evt.eventDescription(i)
                evtdtype = seiscomp3.DataModel.EEventDescriptionTypeNames.name(evtd.type())
                txt += "      %s: %s" % (evtdtype, evtd.text())

            if extra:
                try: txt += "\n    Creation time          %s\n" % evt.creationInfo().creationTime().toString("%Y-%m-%d %H:%M:%S")
                except: pass
            txt += "\n"
            preferredMagnitudeID = evt.preferredMagnitudeID()
        else:
            preferredMagnitudeID = ""

        tim = org.time().value()
        lat = org.latitude().value()
        lon = org.longitude().value()
        dep = org.depth().value()
        try: timerr = 0.5*(org.time().lowerUncertainty()+org.time().upperUncertainty())
        except:
            try: timerr = org.time().uncertainty()
            except: timerr = None
        try: laterr = 0.5*(org.latitude().lowerUncertainty()+org.latitude().upperUncertainty())
        except:
            try: laterr = org.latitude().uncertainty()
            except: laterr = None
        try: lonerr = 0.5*(org.longitude().lowerUncertainty()+org.longitude().upperUncertainty())
        except:
            try: lonerr = org.longitude().uncertainty()
            except: lonerr = None
        try: deperr = 0.5*(org.depth().lowerUncertainty()+org.depth().upperUncertainty())
        except:
            try: deperr = org.depth().uncertainty()
            except: deperr = None
        tstr = time2str(tim)

        originHeader = "Origin:\n"
        if evt:
            if org.publicID() != evt.preferredOriginID():
                originHeader = "Origin (NOT the preferred origin of this event):\n"

        txt += originHeader
        if extra:
            txt += "    Public ID              %s\n" % org.publicID()
        txt += "    Date                   %s\n" % tstr[:10]
        if timerr:
            if enhanced:
                txt += "    Time                   %s  +/- %6.3f s\n" % (tstr[11:], timerr)
            else:
                txt += "    Time                   %s  +/- %6.1f s\n" % (tstr[11:-2], timerr)
        else:
            if enhanced:
                txt += "    Time                   %s\n" % tstr[11:]
            else:
                txt += "    Time                   %s\n" % tstr[11:-2]

        if laterr:
            if enhanced:
                txt += "    Latitude              %11.6f deg  +/- %7.3f km\n" % (lat, laterr)
            else:
                txt += "    Latitude              %7.2f deg  +/- %4.0f km\n" % (lat, laterr)
        else:
            if enhanced:
                txt += "    Latitude              %11.6f deg\n" % lat
            else:
                txt += "    Latitude              %7.2f deg\n" % lat
        if lonerr:
            if enhanced:
                txt += "    Longitude             %11.6f deg  +/- %7.3f km\n" % (lon, lonerr)
            else:
                txt += "    Longitude             %7.2f deg  +/- %4.0f km\n" % (lon, lonerr)
        else:
            if enhanced:
                txt += "    Longitude             %11.6f deg\n" % lon
            else:
                txt += "    Longitude             %7.2f deg\n" % lon
        if enhanced:
            txt += "    Depth                 %11.3f km" % dep
        else:
            txt += "    Depth                 %7.0f km" % dep
        if not deperr:
            txt += "\n"
        elif deperr==0:
            txt +="   (fixed)\n"
        else:
            if depthPhaseCount >= minDepthPhaseCount:
                if enhanced:
                    txt += "   +/- %7.3f km  (%d depth phases)\n" % (deperr, depthPhaseCount)
                else:
                    txt += "   +/- %4.0f km  (%d depth phases)\n" % (deperr, depthPhaseCount)
            else:
                if enhanced:
                    txt += "   +/- %7.3f km\n" % deperr
                else:
                    txt += "   +/- %4.0f km\n" % deperr

        agencyID = ""
        if self.useEventAgencyID:
            try: agencyID = evt.creationInfo().agencyID()
            except: pass
        else:
            try: agencyID = org.creationInfo().agencyID()
            except: pass
        txt += "    Agency                 %s\n" % agencyID
        if extra:
            try:    authorID = org.creationInfo().author()
            except: authorID = "NOT SET"
            txt += "    Author                 %s\n" % authorID
        txt += "    Mode                   "
        try:    txt += "%s\n" % seiscomp3.DataModel.EEvaluationModeNames.name(org.evaluationMode())
        except: txt += "NOT SET\n"
        txt += "    Status                 "
        try:    txt += "%s\n" % seiscomp3.DataModel.EEvaluationStatusNames.name(org.evaluationStatus())
        except: txt += "NOT SET\n"

        if extra:
            txt += "    Creation time          "
            try:    txt += "%s\n" % org.creationInfo().creationTime().toString("%Y-%m-%d %H:%M:%S")
            except: txt += "NOT SET\n"

        try:
            if enhanced:
                txt += "    Residual RMS            %7.3f s\n" % org.quality().standardError()
            else:
                txt += "    Residual RMS            %6.2f s\n" % org.quality().standardError()
        except: pass

        try:
            if enhanced:
                txt += "    Azimuthal gap           %6.1f deg\n" % org.quality().azimuthalGap()
            else:
                txt += "    Azimuthal gap           %5.0f deg\n" % org.quality().azimuthalGap()
        except: pass

        txt += "\n"

        netmag = {}
        nmag = org.magnitudeCount()
        tmptxt = txt
        txt = ""
        foundPrefMag = False
        for i in xrange(nmag):
            mag = org.magnitude(i)
            val = mag.magnitude().value()
            typ = mag.type()
            netmag[typ] = mag
            err = ""

            # FIXME vvv
            # This test is necessary because if lowerUncertainty/upperUncertainty
            # are not set, the resulting C++ exception is currently not propagated
            # to Python and therefore cannot be caught, resulting in a crash.
            if mag.stationCount() >= 4:
                # This only works as long as 4 is the minimum magnitude count for
                # which errors are computed by scmag.
                # FIXME ^^^
                try:
                    m = mag.magnitude()
                    try: err = "+/- %.2f" % (0.5*(m.lowerUncertainty()+m.upperUncertainty()))
                    except: err = "+/- %.2f" % m.uncertainty()
                except ValueError:
                    pass # just don't write any error, that's it
                except Exception, e:
                    sys.stderr.write("_printOriginAutoloc3: caught unknown exception, type='%s', text='%s'\n" % (type(e),str(e)))
            if mag.publicID() == preferredMagnitudeID:
                    preferredMarker = "preferred"
                    foundPrefMag = True
            else:   preferredMarker = "         "
            if extra:
                try: agencyID = mag.creationInfo().agencyID()
                except: pass
            else:
                agencyID = ""
            txt += "    %-8s %5.2f %8s %3d %s  %s\n" % \
                    (typ, val, err, mag.stationCount(), preferredMarker, agencyID)

        if not foundPrefMag and preferredMagnitudeID != "":
            mag = seiscomp3.DataModel.Magnitude.Find(preferredMagnitudeID)
            if mag is None and self._dbq:
                o = self._dbq.loadObject(seiscomp3.DataModel.Magnitude.TypeInfo(), preferredMagnitudeID)
                mag = seiscomp3.DataModel.Magnitude.Cast(o)

            if mag:
                val = mag.magnitude().value()
                typ = mag.type()
                netmag[typ] = mag
                err = ""

                try:
                    m = mag.magnitude()
                    try: err = "+/- %.2f" % (0.5*(m.lowerUncertainty()+m.upperUncertainty()))
                    except: err = "+/- %.2f" % m.uncertainty()
                except ValueError:
                    pass # just don't write any error, that's it
                except Exception, e:
                    sys.stderr.write("_printOriginAutoloc3: caught unknown exception, type='%s', text='%s'\n" % (type(e),str(e)))
                preferredMarker = "preferred"
                if extra:
                    try: agencyID = mag.creationInfo().agencyID()
                    except: pass
                else:
                    agencyID = ""
                txt += "    %-8s %5.2f %8s %3d %s  %s\n" % \
                        (typ, val, err, mag.stationCount(), preferredMarker, agencyID)
                nmag = nmag + 1

        txt = tmptxt + "%d Network magnitudes:\n" % nmag + txt

        if not self._long:
            return txt

        dist_azi = {}
        lines = []

        for dist, arr in dist_arr:
            p = seiscomp3.DataModel.Pick.Find(arr.pickID())
            if p is None:
                lines.append( (180, "    ## missing pick %s\n" % arr.pickID()) )
                continue

            wfid = p.waveformID()
            net = wfid.networkCode()
            sta = wfid.stationCode()
            try:
                if enhanced:
                    azi = "%5.1f" % arr.azimuth()
                else:
                    azi = "%3.0f" % arr.azimuth()
            except: azi = "N/A"
            dist_azi[net+"_"+sta] = (dist, azi)
            wt  = arr.weight()
            if enhanced:
                tstr = time2str(p.time().value())[11:]
            else:
                tstr = time2str(p.time().value())[11:-2]
            try:
                if enhanced:
                    res = "%7.3f" % arr.timeResidual()
                else:
                    res = "%5.1f" % arr.timeResidual()
            except: res = "  N/A"
            pha = arr.phase().code()
            flag = "X "[wt>0.1]
            try:
                status = seiscomp3.DataModel.EEvaluationModeNames.name(p.evaluationMode())[0].upper()
            except:
                status = "-"
            if self.polarities:
                try:
                    pol = seiscomp3.DataModel.EPickPolarityNames.name(p.polarity())
                except:
                    pol = None
                if pol:
                    if   pol=="positive":    pol="u"
                    elif pol=="negative":    pol="d"
                    elif pol=="undecidable": pol="x"
                    else: pol="."
                else: pol="."
                if enhanced:
                    line = "    %-5s %-2s  %10.6f %s  %-7s %s %s %1s%1s %5.3f  %s %-5s\n" \
                        % (sta, net, dist, azi, pha, tstr, res, status, flag, wt, pol, sta)
                else:
                    line = "    %-5s %-2s  %5.1f %s  %-7s %s %s %1s%1s %3.1f  %s %-5s\n" \
                        % (sta, net, dist, azi, pha, tstr, res, status, flag, wt, pol, sta)
            else:
                if enhanced:
                    line = "    %-5s %-2s  %10.6f %s  %-7s %s %s %1s%1s %3.1f  %-5s\n" \
                        % (sta, net, dist, azi, pha, tstr, res, status, flag, wt, sta)
                else:
                    line = "    %-5s %-2s  %5.1f %s  %-7s %s %s %1s%1s %3.1f  %-5s\n" \
                        % (sta, net, dist, azi, pha, tstr, res, status, flag, wt, sta)
            lines.append( (dist, line) )

        lines.sort()

        txt += "\n"
        txt += "%d Phase arrivals:\n" % org.arrivalCount()
        txt += "    sta  net   dist azi  phase   time         res     wt  "
        if self.polarities: txt += "  "
        txt += "sta\n"
        for dist,line in lines:
            txt += line
        txt += "\n"

        nmag = org.stationMagnitudeCount()
        mags = {}

        for i in xrange(nmag):
            mag = org.stationMagnitude(i)
            typ = mag.type()
            if typ not in netmag: continue
            if typ not in mags:
                mags[typ] = []

            mags[typ].append(mag)

        lines = []

        for typ in mags:
            for mag in mags[typ]:

                key = mag.amplitudeID()
                amp = seiscomp3.DataModel.Amplitude.Find(key)
                if amp is None and self._dbq:
                    # Bei einem manuellen Pick muss eine gefundene, automatische
                    # Amplitude in eine neue, "manuelle" Amplitude gefaked werden,
                    # die dann auf den manuellen Pick verweist. -> MagTool
                    #
                    # Oder man verzichtet auf das Faken und gibt die Amplitude
                    # einfach als N/A an.
                    seiscomp3.Logging.debug("missing station amplitude '%s'" % key)

                    # FIXME really slow!!!
                    obj = self._dbq.loadObject(seiscomp3.DataModel.Amplitude.TypeInfo(), key)
                    amp = seiscomp3.DataModel.Amplitude.Cast(obj)

                p = a = "N/A"
                if amp:
                    #if typ in ["mb", "mB", "Ms", "ML"]:
                    if typ in ["mb", "mB", "Ms", "Ms(BB)", "ML", "MLv", "Mwp"]:
                        try:    a = "%g" % amp.amplitude().value()
                        except: a = "N/A"

                        if typ in ["mb", "Ms", "Ms(BB)"]:
                            try:    p = "%.2f" % amp.period().value()
                            except: p = "N/A"
                        else:
                            p = ""

                wfid = mag.waveformID()
                net = wfid.networkCode()
                sta = wfid.stationCode()

                try:    dist, azi = dist_azi[net+"_"+sta]
                except: dist, azi = 0, "N/A"

                val = mag.magnitude().value()
                res = val - netmag[typ].magnitude().value()


                line = "    %-5s %-2s  %5.1f %s  %-6s %5.2f %5.2f   %8s %4s \n" % \
                        (sta, net, dist, azi, typ, val, res, a, p)
                lines.append( (dist, line) )

        lines.sort()

        txt += "%d Station magnitudes:\n" % nmag
        txt += "    sta  net   dist azi  type   value   res        amp per\n"
        for dist,line in lines:
            txt += line

        return txt


    def _printOriginAutoloc1(self, org):
        evt = self._evt
        enhanced = self.enhanced

        if not evt and self._dbq:
            evt = self._dbq.getEvent(org.publicID())
        if evt:
            evid = evt.publicID()
            pos = evid.find("#") # XXX Hack!!!
            if pos != -1:
                evid = evid[:pos]
            prefMagID = evt.preferredMagnitudeID()
        else:
            evid = "..."
            prefMagID = ""

        txt = ""

        reg = seiscomp3.Seismology.Regions()
        if enhanced:
            depth = org.depth().value()
            sTime = org.time().value().toString("%Y/%m/%d  %H:%M:%S.%f")[:24]
        else:
            depth = int(org.depth().value()+0.5)
            sTime = org.time().value().toString("%Y/%m/%d  %H:%M:%S.%f")[:22]

        tmp = {
            "evid":evid,
            "nsta":stationCount(org),
            "time":sTime,
            "lat":lat2str(org.latitude().value(),enhanced),
            "lon":lon2str(org.longitude().value(),enhanced),
            "dep":depth,
            "reg":reg.getRegionName(org.latitude().value(), org.longitude().value()),
            # changed to properly report location method. (Marco Olivieri 21/06/2010)
            "method":org.methodID(),
            "model":org.earthModelID(),
            # end (MO)
            "stat":"A"
        }

        try:
            if org.evaluationMode() == seiscomp3.DataModel.MANUAL:
                tmp["stat"] = "M"
        except: pass

        # dummy default
        tmp["mtyp"] = "M"
        tmp["mval"] = 0.

        foundMag = False
        nmag = org.magnitudeCount()
        for i in xrange( org.magnitudeCount() ):
            mag = org.magnitude(i)
            if mag.publicID() == prefMagID:
                if mag.type() in ["mb","mB","Mwp","ML","MLv", "Mjma"]:
                    tmp["mtyp"] = mag.type()
                tmp["mval"] = mag.magnitude().value()
                foundMag = True
                break;

        if not foundMag and prefMagID != "":
            mag = seiscomp3.DataModel.Magnitude.Find(prefMagID)
            if mag is None and self._dbq:
                o = self._dbq.loadObject(seiscomp3.DataModel.Magnitude.TypeInfo(), prefMagID)
                mag = seiscomp3.DataModel.Magnitude.Cast(o)

            if mag :
                tmp["mtyp"] = mag.type()
                tmp["mval"] = mag.magnitude().value()

# changed to properly report location method. (Marco Olivieri 21/06/2010)
#        txt += """
# Autoloc alert %(evid)s: determined by %(nsta)d stations, type %(stat)s
#
# LOCSAT solution (with start solution, %(nsta)d stations used, weight %(nsta)d):
#
#  %(reg)s  %(mtyp)s=%(mval).1f  %(time)s  %(lat)s  %(lon)s   %(dep)d km
#
#  Stat  Net   Date       Time          Amp    Per   Res  Dist  Az mb  ML  mB
#""" % tmp
        if enhanced:
            txt += """
  Alert %(evid)s: determined by %(nsta)d stations, type %(stat)s

 %(method)s solution with earthmodel %(model)s (with start solution, %(nsta)d stations used, weight %(nsta)d):

  %(reg)s  %(mtyp)s=%(mval).1f  %(time)s  %(lat)s  %(lon)s   %(dep).3f km

  Stat  Net   Date       Time          Amp    Per   Res  Dist  Az mb  ML  mB
""" % tmp
        else:
            txt += """
  Alert %(evid)s: determined by %(nsta)d stations, type %(stat)s

 %(method)s solution with earthmodel %(model)s (with start solution, %(nsta)d stations used, weight %(nsta)d):

  %(reg)s  %(mtyp)s=%(mval).1f  %(time)s  %(lat)s  %(lon)s   %(dep)d km

  Stat  Net   Date       Time          Amp    Per   Res  Dist  Az mb  ML  mB
""" % tmp

# end (MO)
        dist_arr = self._getDistancesArrivalsSorted(org)
        pick = self._getPicks(org)
        ampl = self._getAmplitudes(org)

        mags = {}
        nmag = org.stationMagnitudeCount()
        for i in xrange(nmag):
            mag = org.stationMagnitude(i)
            typ = mag.type()
            if typ == "MLv": typ = "ML"
            if typ not in mags:
                mags[typ] = {}

            sta = mag.waveformID().stationCode()
            mags[typ][sta] = mag

        for dist, arr in dist_arr:
            if arr.weight() < minweight :
                continue

            p = seiscomp3.DataModel.Pick.Find(arr.pickID())
            if p is None:
                txt += "  ## missing pick %s\n" % arr.pickID()
                continue

            wfid = p.waveformID()
            net = wfid.networkCode()
            sta = wfid.stationCode()
            if enhanced:
                tstr = p.time().value().toString("%y/%m/%d  %H:%M:%S.%f")[:22]
            else:
                tstr = p.time().value().toString("%y/%m/%d  %H:%M:%S.%f00")[:20]
            try:
                if enhance:
                    res = "%7.3f" % arr.timeResidual()
                else:
                    res = "%5.1f" % arr.timeResidual()
            except: res = "  N/A"
            try:
                if enhance:
                    azi = "%5.1f" % arr.azimuth()
                else:
                    azi = "%3.0f" % arr.azimuth()
            except: azi = "N/A"
            pha = arr.phase().code()
            mstr = ""
            amp = per = 0.
            for typ in ["mb","ML","mB"]:
                mag = 0.
                try:
                    m = mags[typ][sta]
                    mag = m.magnitude().value()
                    if typ=="mb":
                        ampid = m.amplitudeID()
                        a = seiscomp3.DataModel.Amplitude.Find(ampid)
                        if a is None and self._dbq:
                            obj = self._dbq.loadObject(seiscomp3.DataModel.Amplitude.TypeInfo(), ampid)
                            a   = seiscomp3.DataModel.Amplitude.Cast(obj)
                        if a:
                            per = a.period().value()
                            try: amp = a.amplitude().value()
                            except: amp = -1
                except KeyError:
                    pass
                mstr += " %3.1f" % mag
            txt += "  %-5s %-2s  %s  %10.1f %4.1f %s %5.1f %s%s\n" \
                % (sta, net, tstr, amp, per, res, dist, azi, mstr)

        if enhanced:
            txt += "\n RMS-ERR:         %.3f\n\n" % org.quality().standardError()
        else:
            txt += "\n RMS-ERR:         %.2f\n\n" % org.quality().standardError()

        try:
            if enhanced:
                tm = evt.creationInfo().creationTime().toString("%Y/%m/%d %H:%M:%S.%f")
            else:
                tm = evt.creationInfo().creationTime().toString("%Y/%m/%d %H:%M:%S")
            txt += " First location:  %s\n" % tm
        except: pass

        try:
            if enhanced:
                tm = org.creationInfo().creationTime().toString("%Y/%m/%d %H:%M:%S.%f")
            else:
                tm = org.creationInfo().creationTime().toString("%Y/%m/%d %H:%M:%S")
            txt += " This location:   %s\n" % tm
        except: pass

        return txt


    def printOrigin(self, origin):
        org = None
        if isinstance(origin, seiscomp3.DataModel.Origin):
            org = origin
        elif isinstance(origin, str):
            if self._dbq:
                org = self._dbq.loadObject(seiscomp3.DataModel.Origin.TypeInfo(), origin)
                org = seiscomp3.DataModel.Origin.Cast(org)
            if not org:
                seiscomp3.Logging.error("origin '%s' not loaded" % origin)
                return
        else:
            raise TypeError, "illegal type for origin"

        if self.format == "autoloc1":
            return self._printOriginAutoloc1(org)
        elif self.format == "autoloc3":
            return self._printOriginAutoloc3(org, extra=False)
        elif self.format == "autoloc3extra":
            return self._printOriginAutoloc3(org, extra=True)
        else:
            pass


    def printEvent(self, event):
        try:
            evt = None
            if isinstance(event, seiscomp3.DataModel.Event):
                self._evt = event
                org = seiscomp3.DataModel.Origin.Find(event.preferredOriginID())
                if not org:
                    org = event.preferredOriginID()
                return self.printOrigin(org)
            elif isinstance(event, str):
                if self._dbq:
                    evt = self._dbq.loadObject(seiscomp3.DataModel.Event.TypeInfo(), event)
                    evt = seiscomp3.DataModel.Event.Cast(evt)
                    self._evt = evt
                if evt is None:
                    raise TypeError, "unknown event '" + event + "'"
                return self.printOrigin(evt.preferredOriginID())
            else:
                raise TypeError, "illegal type for event"
        finally:
            self._evt = None


def usage(exitcode=0):
    usagetext = """
 scbulletin [ -E event-id | -O origin-os | --db-type database-type | --db-parameters database-connection]
    """
    sys.stderr.write("%s\n" % usagetext)
    sys.exit(exitcode)


class BulletinApp(seiscomp3.Client.Application):
    def __init__(self, argc, argv):
        seiscomp3.Client.Application.__init__(self, argc, argv)
        self.setMessagingEnabled(False)
        self.setDatabaseEnabled(True, True)
        self.setDaemonEnabled(False)
        self.setLoggingToStdErr(True)
        self.setLoadRegionsEnabled(True)

        self.format = "autoloc1"

    def createCommandLineDescription(self):
        try:
            try:
                self.commandline().addGroup("Dump")
                self.commandline().addStringOption("Dump", "event,E", "ID of event to dump")
                self.commandline().addStringOption("Dump", "origin,O", "ID of origin to dump")
                self.commandline().addStringOption("Dump", "weight,w", "weight threshold for printed and counted picks")
                self.commandline().addOption("Dump", "autoloc1,1", "autoloc1 format")
                self.commandline().addOption("Dump", "autoloc3,3", "autoloc3 format")
                self.commandline().addOption("Dump", "extra,x", "extra detailed autoloc3 format")
                self.commandline().addOption("Dump", "enhanced,e", "enhanced output precision for local earthquakes")
                self.commandline().addOption("Dump", "polarities,p", "dump onset polarities")
                self.commandline().addOption("Dump", "first-only", "dump only the first event/origin")
                self.commandline().addOption("Dump", "event-agency-id", "use agency ID information from event instead of preferred origin")

                self.commandline().addGroup("Input")
                self.commandline().addStringOption("Input", "format,f", "input format to use (xml [default], zxml (zipped xml), binary)")
                self.commandline().addStringOption("Input", "input,i", "input file, default: stdin")
            except:
                seiscomp3.Logging.warning("caught unexpected error %s" % sys.exc_info())

            return True
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)
            sys.exit(-1)


    def validateParameters(self):
        try:
            if seiscomp3.Client.Application.validateParameters(self) == False:
                return False

            if not self.commandline().hasOption("event") and not self.commandline().hasOption("origin"):
                self.setDatabaseEnabled(False, False)

            return True
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)
            sys.exit(-1)

    def run(self):
        try:
            evid = None
            orid = None
            mw   = None

            try: evid = self.commandline().optionString("event")
            except: pass

            try: orid = self.commandline().optionString("origin")
            except: pass

            if evid != "" or orid != "":
                dbq = seiscomp3.DataModel.DatabaseQuery(self.database())
            else:
                dbq = None


            try: mw = self.commandline().optionString("weight")
            except: pass

            if mw != "" and mw is not None:
                global minweight
                minweight = float(mw)

            bulletin = Bulletin(dbq)
            bulletin.format = "autoloc1"

            if self.commandline().hasOption("autoloc1"):
                bulletin.format = "autoloc1"
            elif self.commandline().hasOption("autoloc3"):
                if self.commandline().hasOption("extra"):
                    bulletin.format = "autoloc3extra"
                else:
                    bulletin.format = "autoloc3"

            if self.commandline().hasOption("enhanced"):
                    bulletin.enhanced = True

            if self.commandline().hasOption("polarities"):
                bulletin.polarities = True
            if self.commandline().hasOption("event-agency-id"):
                bulletin.useEventAgencyID = True

            try:
                if evid:
                    txt = bulletin.printEvent(evid)
                elif orid:
                    txt = bulletin.printOrigin(orid)
                else:
                    inputFormat = "xml"
                    inputFile = "-"

                    try: inputFile = self.commandline().optionString("input")
                    except: pass

                    try: inputFormat = self.commandline().optionString("format")
                    except: pass

                    if inputFormat == "xml":
                        ar = seiscomp3.IO.XMLArchive()
                    elif inputFormat == "zxml":
                        ar = seiscomp3.IO.XMLArchive()
                        ar.setCompression(True)
                    elif inputFormat == "binary":
                        ar = seiscomp3.IO.BinaryArchive()
                    else:
                        raise TypeError, "unknown input format '" + inputFormat + "'"

                    if ar.open(inputFile) == False:
                        raise IOError, inputFile + ": unable to open"

                    obj = ar.readObject()
                    if obj is None:
                        raise TypeError, inputFile + ": invalid format"

                    ep = seiscomp3.DataModel.EventParameters.Cast(obj)
                    if ep is None:
                        raise TypeError, inputFile + ": no eventparameters found"

                    if ep.eventCount() <= 0:
                        if ep.originCount() <= 0:
                            raise TypeError, inputFile + ": no origin and no event in eventparameters found"
                        else:
                            if self.commandline().hasOption("first-only"):
                                org = ep.origin(0)
                                txt = bulletin.printOrigin(org)
                            else:
                                txt = ""
                                for i in xrange(ep.originCount()):
                                    org = ep.origin(i)
                                    txt += bulletin.printOrigin(org)
                    else:
                        if self.commandline().hasOption("first-only"):
                            ev = ep.event(0)
                            if ev is None:
                               raise TypeError, inputFile + ": invalid event"

                            txt = bulletin.printEvent(ev)
                        else:
                            txt = ""
                            for i in xrange(ep.eventCount()):
                                ev = ep.event(i)
                                txt += bulletin.printEvent(ev)

            except Exception, exc:
                sys.stderr.write("ERROR: " + str(exc) + "\n")
                return False

            if txt:
                sys.stdout.write("%s\n" % txt)

            return True
        except:
            info = traceback.format_exception(*sys.exc_info())
            for i in info: sys.stderr.write(i)
            sys.exit(-1)


def main():
    app = BulletinApp(len(sys.argv), sys.argv)
    return app()


if __name__ == "__main__":
    sys.exit(main())
