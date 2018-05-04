#***************************************************************************** 
# inventory.py
#
# seiscomp3-based QC implementation
#
# (c) 2009 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import datetime
from seiscomp.db.seiscomp3 import sc3wrap as _sc3wrap
from seiscomp.db.xmlio import qc as _xmlio
from seiscomp.db import DBError
from seiscomp import logs

#
# arclink's QC representation
#
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class _QCLog(_sc3wrap.base_qclog):
    def __init__(self, so):
        _sc3wrap.base_qclog.__init__(self, so)

    def flush(self):
        self._sync_update()

class _Outage(_sc3wrap.base_outage):
    def __init__(self, so):
        _sc3wrap.base_outage.__init__(self, so)

    def flush(self):
        self._sync_update()

class _WaveformQuality(_sc3wrap.base_waveformquality):
    def __init__(self, so):
        _sc3wrap.base_waveformquality.__init__(self, so)

    def flush(self):
        self._sync_update()

class QualityControl(_sc3wrap.base_qualitycontrol):
    def __init__(self, so):
        _sc3wrap.base_qualitycontrol.__init__(self, so)
        self.log = {}
        self.outage = {}
        self.waveform_quality = {}

    def _link_log(self, obj):
        if (obj.net_code, obj.sta_code, obj.str_code, obj.loc_code) not in self.log:
            self.log[(obj.net_code, obj.sta_code, obj.str_code, obj.loc_code)] = {}

        self.log[(obj.net_code, obj.sta_code, obj.str_code, obj.loc_code)][(obj.start, obj.end)] = obj

    def _link_outage(self, obj):
        if (obj.net_code, obj.sta_code, obj.str_code, obj.loc_code) not in self.outage:
            self.outage[(obj.net_code, obj.sta_code, obj.str_code, obj.loc_code)] = {}

        self.outage[(obj.net_code, obj.sta_code, obj.str_code, obj.loc_code)][obj.start] = obj

    def _link_waveform_quality(self, obj):
        if (obj.net_code, obj.sta_code, obj.str_code, obj.loc_code) not in self.waveform_quality:
            self.waveform_quality[(obj.net_code, obj.sta_code, obj.str_code, obj.loc_code)] = {}

        self.waveform_quality[(obj.net_code, obj.sta_code, obj.str_code, obj.loc_code)][(obj.start, obj.type, obj.parameter)] = obj

    def insert_log(self, net_code, sta_code, str_code, loc_code,
        start, end, **args):

        obj = _QCLog(self._new_qclog(net_code=net_code, sta_code=sta_code,
            str_code=str_code, loc_code=loc_code, start=start, end=end, **args))

        self._link_log(obj)
        return obj

    def insert_outage(self, net_code, sta_code, str_code, loc_code,
        start, **args):

        obj = _Outage(self._new_outage(net_code=net_code, sta_code=sta_code,
            str_code=str_code, loc_code=loc_code, start=start, **args))

        self._link_outage(obj)
        return obj

    def insert_waveform_quality(self, net_code, sta_code, str_code, loc_code,
        start, type, parameter, **args):

        obj = _WaveformQuality(self._new_waveformquality(net_code=net_code, sta_code=sta_code,
            str_code=str_code, loc_code=loc_code, start=start, type=type, parameter=parameter,
            **args))

        self._link_waveform_quality(obj)
        return obj

    def remove_log(self, net_code, sta_code, str_code, loc_code, start, end, **args):
        try:
            del self.log[(net_code, sta_code, str_code, loc_code)][(start, end)]
            if len(self.log[(net_code, sta_code, str_code, loc_code)]) == 0:
                del self.log[(net_code, sta_code, str_code, loc_code)]

        except KeyError:
            raise DBError("log entry (%s,%s,%s,%s,%s,%s) not found" % \
                (net_code, sta_code, str_code, loc_code, start, end))

    def remove_outage(self, net_code, sta_code, str_code, loc_code, start, **args):
        try:
            del self.outage[(net_code, sta_code, str_code, loc_code)][start]
            if len(self.outage[(net_code, sta_code, str_code, loc_code)]) == 0:
                del self.outage[(net_code, sta_code, str_code, loc_code)]

        except KeyError:
            raise DBError("outage (%s,%s,%s,%s,%s) not found" % \
                (net_code, sta_code, str_code, loc_code, start))

    def remove_waveform_quality(self, net_code, sta_code, str_code, loc_code, start,
        type, parameter, **args):

        try:
            del self.waveform_quality[(net_code, sta_code, str_code, loc_code)][(start, type, parameter)]
            if len(self.waveform_quality[(net_code, sta_code, str_code, loc_code)]) == 0:
                del self.waveform_quality[(net_code, sta_code, str_code, loc_code)]

        except KeyError:
            raise DBError("waveform quality (%s,%s,%s,%s,%s,%s,%s) not found" % \
                (net_code, sta_code, str_code, loc_code, start, type, parameter))

    def flush(self):
        for i in self.log.values():
            for j in i.values():
                j.flush()

        for i in self.outage.values():
            for j in i.values():
                j.flush()

        for i in self.waveform_quality.values():
            for j in i.values():
                j.flush()

    def load_logs(self):
        for log in self._qCLog:
            self._link_log(log)

    def load_outages(self):
        for outage in self._outage:
            self._link_outage(outage)

    def load_waveform_quality(self):
        for wfq in self._waveformQuality:
            self._link_waveform_quality(wfq)

    def clear_logs(self):
        self.flush()
        self.log = {}

    def clear_outages(self):
        self.flush()
        self.outage = {}
    
    def clear_waveform_quality(self):
        self.flush()
        self.waveform_quality = {}
    
    def load_xml(self, src):
        _xmlio.xml_in(self, src)

    def save_xml(self, dest, stylesheet=None):
        _xmlio.xml_out(self, dest, stylesheet)

    def make_parser(self):
        return _xmlio.make_parser(self)
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

