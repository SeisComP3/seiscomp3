#***************************************************************************** 
# nettab.py
#
# This implemented the first version of the nettab file fomart currently
# replaced by the nettab2 format. For more information about the nettab2
# format look on the lineType.py and nodesncsl.py and nodesi.py files
#
# inst.db and net.tab reader
#
# (c) 2009 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import re
import csv
import time
import datetime
import os.path
import decimal
import fnmatch
from seiscomp import logs

# Compatibillity for python 2.3
import sys
if sys.version_info < (2,4): from sets import Set as set

def isPyVersion(major, minor):
    return sys.version_info[0] == major and \
           sys.version_info[1] == minor
    

def getFieldNames(fd):
    tmp = fd.readline().split(',')
    fieldNames = []
    for i in tmp:
        fieldNames.append(i.strip())
    return fieldNames


# end compatibillity


_EPOCH_DATE = datetime.datetime(1980, 1, 1)

class NettabError(Exception):
    pass

_doy = (0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365)
_rx_datetime = re.compile(r'([0-9]+)/([0-9]+)(:([0-9]{2})([0-9]{2}))?$')
_rx_paz = re.compile(r'\s*([0-9]*)\(\s*([^,]+),\s*([^)]+)\)\s*')
_rx_samp = re.compile(r'(?P<band_code>[A-Z])?(?P<sample_rate>.*)$')
_rx_statmap = re.compile(r'\s*([^_]*)_([^=]*)=(\S*)\s*(from=([0-9]+)/([0-9]+))?\s*(to=([0-9]+)/([0-9]+))?\s*$')

def _normalize(num, denom):
    if num > denom:
        (a, b) = (num, denom)
    else:
        (a, b) = (denom, num)

    while b > 1:
        (a, b) = (b, a % b)

    if b == 0:
        return (num / a, denom / a)

    return (num, denom)

def _rational(x):
    sign, mantissa, exponent = x.as_tuple()
    sign = (1, -1)[sign]
    mantissa = sign * reduce(lambda a, b: 10 * a + b, mantissa)
    if exponent < 0:
        return _normalize(mantissa, 10 ** (-exponent))
    else:
        return (mantissa * 10 ** exponent, 1)

def _parse_paz(npaz, s):
    pos = 0
    n = 0
    c = []
    
    while pos < len(s):
        m = _rx_paz.match(s, pos)
        if m is None:
            raise NettabError, "error parsing PAZ at '" + s[pos:] + "'"

        try:
            if len(m.group(1)) > 0:
                x = int(m.group(1))
            else:
                x = 1
                
            rv = m.group(2)
            iv = m.group(3)

            float(rv)
            float(iv)

        except ValueError:
            raise NettabError, "error parsing PAZ at '" + s[pos:] + "'"

        for i in xrange(0, x):
            c.append((rv, iv))
        
        n += x
        pos = m.end()
    
    if n != npaz:
        raise NettabError, "expected %d PAZ, found %d" % (npaz, n)

    return c

def _is_leap(y):
    return (y % 400 == 0 or (y % 4 == 0 and y % 100 != 0))

def _ldoy(y, m):
    return _doy[m] + (_is_leap(y) and m >= 2)

def _dy2mdy(doy, year):
    month = 1
    while doy > _ldoy(year, month):
        month += 1

    mday = doy - _ldoy(year, month - 1)
    return (month, mday)

def _datetime(year, doy, hour = 0, minute = 0):
    (month, mday) = _dy2mdy(doy, year)
    return datetime.datetime(year, month, mday, hour, minute)

def _parse_date(datestr):
    if not datestr:
        return None

    m = _rx_datetime.match(datestr)
    if not m:
        raise ValueError, "invalid date: " + datestr

    try:
        year = int(m.group(1))
        doy = int(m.group(2))

        if m.group(3):
            hour = int(m.group(4))
            minute = int(m.group(5))

        else:
            hour = 0
            minute = 0

        return _datetime(year, doy, hour, minute)

    except ValueError:
        raise ValueError, "invalid date: " + datestr

def _cmptime(t1, t2):
    if t1 is None and t2 is None:
        return 0
    elif t2 is None or (t1 is not None and t1 < t2):
        return -1
    elif t1 is None or (t2 is not None and t1 > t2):
        return 1

    return 0

class _PAZ(object):
    def __init__(self, ns, id, p, o):
        self.ns = ns
        self.id = id
        self.gain = float(p[0])
        self.frgn = float(p[1])
        self.norm_fac = float(p[2])
        self.fnr = float(p[3])
        self.nz = int(p[4])
        self.np = int(p[5])
        paz = _parse_paz(self.nz + self.np, " ".join(p[6:]))
        self.zeros = paz[:self.nz]
        self.poles = paz[self.nz:]

    def update_inventory(self, inv, path):
        try:
            ipaz = inv.responsePAZ[self.ns + ":" + self.id]

        except KeyError:
            ipaz = inv.insert_responsePAZ(self.ns + ":" + self.id)
        
        ipaz.type = self.type
        ipaz.gain = self.gain
        ipaz.gainFrequency = self.frgn
        ipaz.normalizationFactor = self.norm_fac
        ipaz.normalizationFrequency = self.fnr
        ipaz.numberOfZeros = self.nz
        ipaz.numberOfPoles = self.np
        ipaz.zeros = " ".join([ "(%s,%s)" % x for x in self.zeros ])
        ipaz.poles = " ".join([ "(%s,%s)" % x for x in self.poles ])

        self.publicID = ipaz.publicID

class _DIGIPAZ(_PAZ):
    def __init__(self, ns, id, p, o):
        _PAZ.__init__(self, ns, id, p, o)
        self.type = 'A'
    
class _IIRPAZ(_PAZ):
    def __init__(self, ns, id, p, o):
        _PAZ.__init__(self, ns, id, p, o)
        self.type = 'D'
    
class _FIR(object):
    def __init__(self, ns, id, p, o):
        self.ns = ns
        self.id = id
        self.name = p[0]
        self.sym = p[1]
        self.ncf = int(p[2])
        self.null = float(p[3])
        self.inrate = decimal.Decimal(p[4])
        self.fac = int(p[5])
        self.delay = float(p[6])
        self.corrtn = float(p[7])
        self.gain = float(p[8])
        self.frg = float(p[9])
        self.coeff = []

        if self.sym == 'B':
            real_delay = (float(self.ncf) - 1.0) / float(self.inrate)

        elif self.sym == 'C':
            real_delay = (float(self.ncf) - 0.5) / float(self.inrate)

        if self.sym in ('B', 'C') and abs(self.delay - real_delay) > 0.001:
            #logs.warning("%s: delay=%g (estimated %g)" % (self.id, self.delay, real_delay))
            logs.debug("%s: delay=%g (estimated %g)" % (self.id, self.delay, real_delay))

    def update_inventory(self, inv, path):
        if not self.coeff:
            try:
                fd = open(path + self.name)

            except IOError, e:
                raise NettabError, "cannot read %s%s: %s" % (path, self.name, str(e))

            try:
                if self.sym == 'B':
                    real_ncf = 2 * self.ncf - 1
                
                elif self.sym == 'C':
                    real_ncf = 2 * self.ncf

                else:
                    real_ncf = self.ncf

                try:
                    coeff_split = fd.read().split()
                    if len(coeff_split) != real_ncf * 3:
                        raise NettabError, "invalid number of coefficients in %s%s" % (path, self.name)

                    coeff_strlist = [ coeff_split[3 * i + 1] for i in range(real_ncf) ]
                    coeff = map(float, coeff_strlist)

                except (TypeError, ValueError), e:
                    raise NettabError, "error reading %s%s: %s" % (path, self.name, str(e))

            finally:
                fd.close()

            i = 0
            while 2 * i < real_ncf:
                if coeff[i] != coeff[real_ncf - 1 - i]:
                    break

                i += 1

            if 2 * i > real_ncf:
                self.ncf = i
                real_sym = "B"

            elif 2 * i == real_ncf:
                self.ncf = i
                real_sym = "C"

            else:
                self.ncf = real_ncf
                real_sym = "A"

            if self.sym != real_sym:
                #logs.warning("%s: setting symmetry code to %s" % (self.id, real_sym))
                logs.info("%s: setting symmetry code to %s" % (self.id, real_sym))
                self.sym = real_sym

        try:
            ifir = inv.responseFIR[self.ns + ":" + self.id]

        except KeyError:
            ifir = inv.insert_responseFIR(self.ns + ":" + self.id)

        ifir.gain = self.gain
        ifir.decimationFactor = self.fac
        ifir.delay = self.delay * float(self.inrate)
        ifir.correction = self.corrtn * float(self.inrate)
        ifir.numberOfCoefficients = self.ncf
        ifir.symmetry = self.sym
        ifir.coefficients = " ".join(coeff_strlist[:self.ncf])

        self.publicID = ifir.publicID

class _Calibration(object):
    def __init__(self, ns, id, p, o):
        self.ns = ns
        self.id = id
        self.gain = map(float, p[:3])

    def update_inventory(self, obj, sn, gain_freq):
        for i in xrange(3):
           try:
               ical = obj.calibration[sn][i][_EPOCH_DATE]

           except KeyError:
               ical = obj.insert_calibration(sn, i, _EPOCH_DATE)
               ical.gain = self.gain[i]
               ical.gainFrequency = gain_freq
               ical.start = _EPOCH_DATE
               ical.end = None

class _Datalogger(object):
    def __init__(self, ns, id, p, o):
        self.ns = ns
        self.id = id
        self.gain = float(p[0])
        self.spfr = decimal.Decimal(p[1])
        self.mcld = float(p[2])
        self.decimation = {}
        self.__idl_cache = {}

        if len(p) > 4:
            for deci_spec in p[4].split(","):
                (rate, filters) = deci_spec.split("_")
                f_list = []
                deci_fac = 1
                for fn in filters.split("/"):
                    if fn[0] == "A":
                        f = o.get(_DIGIPAZ, p[3] + "_digipaz_" + fn[1:])
                    
                    elif fn[0] == "I":
                        f = o.get(_IIRPAZ, p[3] + "_iirpaz_" + fn[1:])

                    else:
                        f = o.get(_FIR, p[3] + "_FIR_" + fn)
                        deci_fac *= f.fac

                    f_list.append(f)

                pri_rate = decimal.Decimal(rate) * deci_fac

                try:
                    pri_deci = self.decimation[pri_rate]

                except KeyError:
                    pri_deci = {}
                    self.decimation[pri_rate] = pri_deci

                pri_deci[_rational(decimal.Decimal(rate))] = f_list

                if pri_rate != self.spfr:
                    logs.info("%s: setting spfr of %sHz to %sHz" % (id, rate,
                      str(pri_rate)))

                inrate = pri_rate
                delay = 0.0
                correction = 0.0
                for f in f_list:
                    if isinstance(f, _FIR):
                        if inrate != f.inrate:
                            logs.info("%s: setting inrate to %sHz" % (f.id,
                              str(inrate)))

                        inrate /= f.fac
                        delay += f.delay
                        correction += f.corrtn

                if abs(delay - correction) > 0.001:
                    #logs.warning("%s: %sHz stream has uncorrected time error %gs" %
                    #  (self.id, rate, delay - correction))
                    logs.debug("%s: %sHz stream has uncorrected time error %gs" %
                      (self.id, rate, delay - correction))
                    
    def __inst_dl(self, inv, name, gain, pri_rate, digitizer_model, digitizer_manufacturer,
        recorder_model, recorder_manufacturer, clock_model, clock_manufacturer, clock_type,
        remark):

        # NEW SCHEMA: primary rate no longer used
        # key = "%s/g=%d/f=%d" % (name, int(gain), int(pri_rate))
        key = "%s:%s/g=%d" % (self.ns, name, int(gain))

        try:
            idl = inv.datalogger[key]

        except KeyError:
            idl = inv.insert_datalogger(key)
        
        idl.description = self.id.replace('_', ' ')
        # NEW SCHEMA: primary rate no longer used
        # (idl.sampleRateNumerator, idl.sampleRateDenominator) = _rational(pri_rate)
        idl.gain = float(gain)
        idl.maxClockDrift = 0.0 # mcld
        idl.digitizerModel = digitizer_model
        idl.digitizerManufacturer = digitizer_manufacturer
        idl.recorderModel = recorder_model
        idl.recorderManufacturer = recorder_manufacturer
        idl.clockModel = clock_model
        idl.clockManufacturer = clock_manufacturer
        idl.clockType = clock_type
        idl.remark = remark

        return (key, idl)

    def __inst_deci(self, idl, rate, rate_div, f_list):
        try:
            deci = idl.decimation[rate][rate_div]

        except KeyError:
            deci = idl.insert_decimation(rate, rate_div)

        afc = []
        dfc = []
        gain = 1.0

        for f in f_list:
            if isinstance(f, _DIGIPAZ):
                if dfc:
                    raise NettabError, "%s: cannot add analogue filter %s after digital filters %s" % \
                      (self.id, f.id, str([ x.id for x in dfc ]))

                afc.append(f.publicID)

            else:
                dfc.append(f.publicID)

            gain *= f.gain

        deci.analogueFilterChain = " ".join(afc)
        deci.digitalFilterChain = " ".join(dfc)

        return gain

    def update_inventory(self, inv, name, sn, gain_mult, rate, rate_div, calib,
        digitizer_model = "",
        digitizer_manufacturer = "",
        recorder_model = "",
        recorder_manufacturer = "",
        clock_model = "",
        clock_manufacturer = "",
        clock_type = "",
        remark = ""):

        gain = self.gain * gain_mult

        for (pri_rate, pri_deci) in self.decimation.iteritems():
            try:
                f_list = pri_deci[(rate, rate_div)]
                break

            except:
                pass

        else:
            logs.warning("%s: %gHz filter chain is not defined" %
                (name, float(rate) / float(rate_div)))

            pri_rate = decimal.Decimal(rate) / decimal.Decimal(rate_div)

            try:
                pri_deci = self.decimation[pri_rate]

            except KeyError:
                pri_deci = {}
                self.decimation[pri_rate] = pri_deci

            f_list = []
            pri_deci[(rate, rate_div)] = f_list
      
        try:
            #(key, idl) = self.__idl_cache[(name, gain, pri_rate)]
            (key, idl) = self.__idl_cache[(name, gain)]

        except KeyError:
            (key, idl) = self.__inst_dl(inv, name, gain, pri_rate,
              digitizer_model, digitizer_manufacturer, recorder_model, recorder_manufacturer,
              clock_model, clock_manufacturer, clock_type, remark)

            #self.__idl_cache[(name, gain, pri_rate)] = (key, idl)
            self.__idl_cache[(name, gain)] = (key, idl)

        filter_gain = self.__inst_deci(idl, rate, rate_div, f_list)

        if calib:
            calib.update_inventory(idl, sn, 0)
            return (idl.publicID, [ x * filter_gain for x in calib.gain ])

        return (idl.publicID, 3 * [gain * filter_gain])
        
class _Seismometer(object):
    def __init__(self, ns, id, p, o):
        self.ns = ns
        self.id = id
        self.gain = float(p[0])
        self.frgn = float(p[1])
        self.norm_fac = float(p[2])
        self.fnr = float(p[3])
        self.nz = int(p[4])
        self.np = int(p[5])
        paz = _parse_paz(self.nz + self.np, " ".join(p[6:]))
        self.zeros = paz[:self.nz]
        self.poles = paz[self.nz:]

    def update_inventory(self, inv, name, sn, gain, calib,
        type = "VBB",
        unit = "",
        low_freq = None,
        high_freq = None,
        model = "",
        manufacturer = "",
        remark = ""):

        if gain:
            gain = float(gain)
        else:
            gain = self.gain

        key = "%s:%s/g=%d" % (self.ns, name, int(gain))

        try:
            ipaz = inv.responsePAZ[key]

        except KeyError:
            ipaz = inv.insert_responsePAZ(key)
        
        ipaz.type = "A"
        ipaz.gain = gain
        ipaz.gainFrequency = self.frgn
        ipaz.normalizationFactor = self.norm_fac
        ipaz.normalizationFrequency = self.fnr
        ipaz.numberOfZeros = self.nz
        ipaz.numberOfPoles = self.np
        ipaz.zeros = " ".join([ "(%s,%s)" % x for x in self.zeros ])
        ipaz.poles = " ".join([ "(%s,%s)" % x for x in self.poles ])

        try:
            ism = inv.sensor[key]

        except KeyError:
            ism = inv.insert_sensor(key)
        
        ism.description = self.id.replace('_', ' ')
        ism.response = ipaz.publicID
        ism.type = type
        ism.unit = unit
        ism.lowFrequency = low_freq
        ism.highFrequency = high_freq
        ism.model = model
        ism.manufacturer = manufacturer
        ism.remark = remark

        if calib:
            calib.update_inventory(ism, sn, self.frgn)
            return (ism.publicID, calib.gain, self.frgn, unit)

        return (ism.publicID, 3 * [gain], self.frgn, unit)

class _InstObject(object):
    def __init__(self, ns, prop):
        self.__ns = ns
        self.__prop = prop
        self.__obj = None

    def get(self, cls, id, objs):
        if self.__obj is None:
            try:
                self.__obj = cls(self.__ns, id, self.__prop, objs)
        
            except (TypeError, ValueError), e:
                raise NettabError, "error constructing %s %s: %s" % \
                  (cls.__name__[1:], id, str(e))

        elif self.__obj.__class__.__name__ != cls.__name__:
            raise NettabError, "cannot instantiate %s %s: already instantiated as %s" % \
              (cls.__name__[1:], id, self.__obj.__class__.__name__[1:])

        return self.__obj

class _InstObjects(object):
    def __init__(self, ns):
        self.__ns = ns
        self.__objs = {}

    def add(self, id, prop):
        if id in self.__objs:
            raise NettabError, "multiple definitions of %s" % (id,)

        self.__objs[id] = _InstObject(self.__ns, prop)

    def trytoget(self, cls, id):
        try:
            obj = self.__objs[id]

        except KeyError:
            return None

        return obj.get(cls, id, self)
    
    def get(self, cls, id):
        obj = self.trytoget(cls, id)
        if obj is None:
            raise NettabError, "cannot find %s %s" % (cls.__name__[1:], id)

        return obj

    def find(self, cls, name):
        found = None
        for (id, obj) in self.__objs.iteritems():
            if ('_' + id).endswith('_' + name):
                if found is not None:
                    raise NettabError, "%s matches multiple objects" % (name,)

                found = obj.get(cls, id, self)

        if found is None:
            raise NettabError, "cannot find %s %s" % (cls.__name__[1:], name)

        return found

class Instruments(object):
    def __init__(self, ns):
        self.__objects = _InstObjects(ns)
        self.__dl_attr = {}
        self.__sm_attr = {}
        self.__dlinst = {}
        self.__sminst = {}
        self.__flinst = set()
        self.__path = None

    def load_datalogger_attr(self, file):
        fd = open(file)
        try:
            try:
                fieldNames = None
                if isPyVersion(2, 3):
                    fieldNames = getFieldNames(fd)
                    
                for row in csv.DictReader(fd, fieldNames):
                    id = row['id']
                    if id in self.__dl_attr:
                        logs.warning("multiple %s found in %s" % (id, file))
                        continue

                    del row['id']
                    self.__dl_attr[id] = row

            except KeyError, e:
                raise NettabError, "column %s missing in %s" % (str(e), file)

            except (TypeError, ValueError), e:
                raise NettabError, "error reading %s: %s" % (file, str(e))

        finally:
            fd.close()

    def load_sensor_attr(self, file):
        fd = open(file)
        try:
            try:
                fieldNames = None
                if isPyVersion(2, 3):
                    fieldNames = getFieldNames(fd)

                for row in csv.DictReader(fd, fieldNames):
                    id = row['id']
                    if id in self.__sm_attr:
                        logs.warning("multiple %s found in %s" % (id, file))
                        continue

                    del row['id']

                    try:
                        if row['low_freq']:
                            row['low_freq'] = float(row['low_freq'])

                        else:
                            del row['low_freq']

                    except KeyError:
                        pass

                    try:
                        if row['high_freq']:
                            row['high_freq'] = float(row['high_freq'])

                        else:
                            del row['high_freq']

                    except KeyError:
                        pass

                    self.__sm_attr[id] = row

            except KeyError, e:
                raise NettabError, "column %s missing in %s" % (str(e), file)

            except (TypeError, ValueError), e:
                raise NettabError, "error reading %s: %s" % (file, str(e))

        finally:
            fd.close()

    def __get_dl_attr(self, id):
        try:
            return self.__dl_attr[id]

        except:
            logs.debug("using default attributes for datalogger %s" % (id,))
            self.__dl_attr[id] = dict()
            return self.__dl_attr[id]
    
    def __get_sm_attr(self, id):
        try:
            return self.__sm_attr[id]

        except:
            #logs.warning("using default attributes for sensor %s" % (id,))
            logs.debug("using default attributes for sensor %s" % (id,))
            self.__sm_attr[id] = dict()
            return self.__sm_attr[id]
    
    def update_inventory_dl(self, inv, dlname, dlsn, dlgain_mult, rate, rate_div):
        try:
            (logger_pubid, logger_gain) = self.__dlinst[(dlname, dlsn, dlgain_mult, rate, rate_div)]

        except KeyError:
            dl = self.__objects.find(_Datalogger, dlname)
            for (pri_rate, pri_deci) in dl.decimation.iteritems():
                for (f_rate, f_list) in pri_deci.iteritems():
                    for f in f_list:
                        if f.id not in self.__flinst:
                            f.update_inventory(inv, self.__path)

                        self.__flinst.add(f.id)

            (logger_pubid, logger_gain) = dl.update_inventory(inv, dlname, dlsn, dlgain_mult, rate, rate_div,
              self.__objects.trytoget(_Calibration, "Sngl-gain_" + dlsn),
              **self.__get_dl_attr(dlname))

            self.__dlinst[(dlname, dlsn, dlgain_mult, rate, rate_div)] = (logger_pubid, logger_gain)

        return (logger_pubid, logger_gain)

    def update_inventory_sm(self, inv, smname, smsn, smgain):
        try:
            (sensor_pubid, sensor_gain, gain_freq, gain_unit) = self.__sminst[(smname, smsn, smgain)]

        except KeyError:
            sm = self.__objects.find(_Seismometer, smname)
            (sensor_pubid, sensor_gain, gain_freq, gain_unit) = sm.update_inventory(inv, smname, smsn, smgain,
              self.__objects.trytoget(_Calibration, "Sngl-gain_" + smsn),
              **self.__get_sm_attr(smname))

            self.__sminst[(smname, smsn, smgain)] = (sensor_pubid, sensor_gain, gain_freq, gain_unit)

        return (sensor_pubid, sensor_gain, gain_freq, gain_unit)

    def update_inventory(self, inv, dlname, dlsn, dlgain_mult, smname, smsn, smgain, rate, rate_div):
        (logger_pubid, logger_gain) = self.update_inventory_dl(inv, dlname, dlsn, dlgain_mult, rate, rate_div)
        (sensor_pubid, sensor_gain, gain_freq, gain_unit) = self.update_inventory_sm(inv, smname, smsn, smgain)
        return (logger_pubid, logger_gain, sensor_pubid, sensor_gain, gain_freq, gain_unit)

    def load_db(self, file):
        self.__path = os.path.dirname(file) # to load filters
        if self.__path:
            self.__path += '/'

        fd = open(file)
        try:
            lineno = 0
            try:
                line = fd.readline()
                lineno += 1
                while line:
                    idline = line.split('>')
                    if idline:
                        id = idline[0].strip()
                        if id and id[0] != '#':
                            if len(idline) != 2:
                                raise NettabError, "parse error"

                            self.__objects.add(idline[0].strip(), idline[1].split())

                    line = fd.readline()
                    lineno += 1

            except (NettabError, TypeError, ValueError), e:
                raise NettabError, "%s:%d: %s" % (file, lineno, str(e))

        finally:
            fd.close()

class _Stream(object):
    def __init__(self, code, start):
        self.code = code
        self.start = start
        self.restricted = False
        self.shared = True

    def update_inventory(self, instdb, inv, iloc):
        try:
            (logger_pubid, logger_gain, sensor_pubid, sensor_gain, gain_freq, gain_unit) = instdb.update_inventory(inv,
                self.dlname, self.dlsn, self.dlgain_mult, self.smname, self.smsn, self.smgain,
                self.rate, self.rate_div)

        except NettabError, e:
            raise NettabError, "%s_%s_%s_%s: %s" % (iloc.myStation.myNetwork.code,
              iloc.myStation.code, iloc.code, self.code, str(e))

        try:
            istrm = iloc.stream[self.code][self.start]

        except KeyError:
            istrm = iloc.insert_stream(self.code, self.start)

        istrm.end = self.end
        istrm.datalogger = logger_pubid
        istrm.dataloggerSerialNumber = self.dlsn
        istrm.dataloggerChannel = self.channel
        istrm.sensor = sensor_pubid
        istrm.sensorSerialNumber = self.smsn
        istrm.sensorChannel = self.channel
        istrm.sampleRateNumerator = self.rate
        istrm.sampleRateDenominator = self.rate_div
        istrm.gainFrequency = gain_freq
        istrm.gainUnit = gain_unit
        istrm.depth = self.depth
        istrm.format = self.format
        istrm.flags = "GC"
        istrm.restricted = self.restricted
        istrm.shared = self.shared
        istrm.azimuth = self.azimuth
        istrm.dip = self.dip

        try:
            istrm.gain = sensor_gain[self.channel] * logger_gain[self.channel]
        
        except IndexError:
            logs.warning("%s_%s_%s_%s: channel %d is not defined, setting gain to 0" %
                (iloc.mystation.mynetwork.code, iloc.mystation.code, iloc.code,
                self.code, self.channel))

            istrm.gain = 0.0

class _SensorLocation(object):
    def __init__(self, code, lat, lon, elev):
        self.code = code
        self.start = None
        self.end = _EPOCH_DATE
        self.latitude = lat
        self.longitude = lon
        self.elevation = elev
        self.__stream = {}

    def add_stream(self, code, start, end, dlspec, smspec, gain_mult,
        rate, rate_div, format, lat, lon, elev, depth, channel, azimuth, dip):
            
        if _cmptime(self.start, start) > 0:
            self.start = start

        if _cmptime(self.end, end) < 0:
            self.end = end

        (dlname, dlsn) = dlspec.split('%')
        (smname, smsn, smgain) = (smspec.split('%') + [None])[:3]

        try:
            strm = self.__stream[(code, start)]

        except KeyError:
            strm = _Stream(code, start)
            self.__stream[(code, start)] = strm

        strm.end = end
        strm.dlname = dlname
        strm.dlsn = dlsn
        strm.dlgain_mult = gain_mult
        strm.smname = smname
        strm.smsn = smsn
        strm.smgain = smgain
        strm.rate = rate
        strm.rate_div = rate_div
        strm.depth = depth
        strm.format = format
        strm.latitude = lat
        strm.longitude = lon
        strm.eleation = elev
        strm.channel = channel
        strm.azimuth = azimuth
        strm.dip = dip

    def update_inventory(self, instdb, inv, ista, restricted, restricted_exc, shared):
        if not self.__stream:
            return

        try:
            iloc = ista.sensorLocation[self.code][self.start]

        except KeyError:
            iloc = ista.insert_sensorLocation(self.code, self.start)

        iloc.end = self.end
        iloc.latitude = self.latitude
        iloc.longitude = self.longitude
        iloc.elevation = self.elevation
        
        existing_streams = set()
        for strm in self.__stream.itervalues():
            strm.shared = shared
            for pat in restricted_exc.split():
                if fnmatch.fnmatchcase(strm.code, pat):
                    strm.restricted = not restricted
                    break
            else:
                strm.restricted = restricted

            strm.update_inventory(instdb, inv, iloc)
            existing_streams.add((strm.code, strm.start))

        for (strm_code, strm_tp) in iloc.stream.items():
            for strm_start in strm_tp.keys():
                if (strm_code, strm_start) not in existing_streams:
                    iloc.remove_stream(strm_code, strm_start)

class _Station(object):
    def __init__(self, code, map_net, map_from, map_to, lat, lon, elev):
        self.code = code
        self.map_net = map_net
        self.map_from = map_from
        self.map_to = map_to
        self.start = None
        self.end = _EPOCH_DATE
        self.latitude = lat
        self.longitude = lon
        self.elevation = elev
        self.restricted = False
        self.shared = True
        self.__sensor_location = {}
        self.__loc_coord = {}

    def update_inventory(self, instdb, inv, inet):
        if not self.__sensor_location:
            return

        try:
            ista = inet.station[self.code][self.start]

        except KeyError:
            ista = inet.insert_station(self.code, self.start)

        ista.end = self.end
        ista.description = self.description
        ista.latitude = self.latitude
        ista.longitude = self.longitude
        ista.elevation = self.elevation
        ista.place = self.place
        ista.country = self.country
        ista.affiliation = self.affiliation
        ista.restricted = self.restricted
        ista.shared = self.shared
        ista.archive = inet.archive
        ista.archiveNetworkCode = self.map_net
        ista.remark = self.remark
        
        existing_sensor_locations = set()
        for loc in self.__sensor_location.itervalues():
            loc.update_inventory(instdb, inv, ista, self.restricted, self.restricted_exc, 
                self.shared)
            existing_sensor_locations.add((loc.code, loc.start))
        
        for (loc_code, loc_tp) in ista.sensorLocation.items():
            for loc_start in loc_tp.keys():
                if (loc_code, loc_start) not in existing_sensor_locations:
                    ista.remove_sensorLocation(loc_code, loc_start)

    def __parse_sampling(self, sampling):
        compression_level = "2"
        instrument_code = "H"
        location_code = ""
        endPreamble = sampling.find('_')
        if endPreamble > 0:
            for x in sampling[:endPreamble].split('/'):
                if x[0] == 'F':
                    compression_level = x[1:]
                elif x[0] == 'L':
                    location_code = x[1:]
                elif x[0] == 'T':
                    instrument_code = x[1:]
                else:
                    raise NettabError, "unknown code %s in %s" % (x[0], sampling)
            
        for x in sampling[endPreamble+1:].split('/'):
            m = _rx_samp.match(x)
            if not m:
                raise NettabError, "error parsing sampling %s at %s" % (sampling, x)
                
            try:
                sample_rate = decimal.Decimal(m.group('sample_rate'))

            except decimal.InvalidOperation:
                raise NettabError, "error parsing sampling %s at %s" % (sampling, x)

            band_code = m.group('band_code')
            if not band_code:
                if sample_rate >= 80:
                    band_code = 'H'
                elif sample_rate >= 40:
                    band_code = 'S'
                elif sample_rate > 1:
                    band_code = 'B'
                elif sample_rate == 1:
                    band_code = 'L'
                elif sample_rate == decimal.Decimal("0.1"):
                    band_code = 'V'
                elif sample_rate == decimal.Decimal("0.01"):
                    band_code = 'U'
                else:
                    raise NettabError, "could not determine band code for %s in %s" (x, sampling)
            
            yield (( band_code + instrument_code, location_code ) +
                _rational(sample_rate) + ("Steim" + compression_level,))

    def __parse_orientation(self, orientation):
        n = 0
        for x in orientation.split(';'):
            try:
                (code, azimuth, dip) = x.split()
                yield (n, code, float(azimuth), float(dip))

            except (TypeError, ValueError):
                raise NettabError, "error parsing orientation %s at %s" % (orientation, x)

            n = n + 1

    def parse_tabline(self, code, desc, dlspec, smspec, gain_mult, samp, lat, lon, elev, depth, start, end,
        orientation,
        restricted = None,
        restricted_exc = "",
        place = "",
        country = "",
        affiliation = "",
        remark = ""):

        if _cmptime(self.map_from, end) > 0 or _cmptime(self.map_to, start) < 0:
            return

        if _cmptime(self.map_from, start) > 0:
            start = self.map_from

        if _cmptime(self.map_to, end) < 0:
            end = self.map_to

        if _cmptime(self.start, start) > 0:
            self.start = start

        if _cmptime(self.end, end) < 0:
            self.end = end

        self.description = desc.replace('_', ' ')
        self.restricted = restricted
        self.restricted_exc = restricted_exc
        self.place = place
        self.country = country
        self.affiliation = affiliation
        self.remark = remark
        
        for (chan, comp_code, azimuth, dip) in self.__parse_orientation(orientation):
            for (strm_code, loc_code, rate, rate_div, format) in self.__parse_sampling(samp):
                try:
                    if self.__loc_coord[(loc_code, start)] != (lat, lon, elev):
                        raise NettabError, "%s [%s] %d/%03d is already defined with different coordinates" % \
                            (code, loc_code, start.year, start.utctimetuple()[7])

                except KeyError:
                    self.__loc_coord[(loc_code, start)] = (lat, lon, elev)

                try:
                    loc = self.__sensor_location[(loc_code, lat, lon, elev)]

                except KeyError:
                    loc = _SensorLocation(loc_code, lat, lon, elev)
                    self.__sensor_location[(loc_code, lat, lon, elev)] = loc

                loc.add_stream(strm_code + comp_code, start, end, dlspec, smspec,
                    gain_mult, rate, rate_div, format, lat, lon, elev, depth,
                    chan, azimuth, dip)
            
class _Network(object):
    def __init__(self, code, sta_attr, statmap):
        self.code = code
        self.restricted = False
        self.shared = True
        self.__sta_attr = sta_attr
        self.__statmap = statmap
        self.stations = {}

    def update_inventory(self, instdb, inv):
        try:
            inet = inv.network[self.code][self.start]

        except KeyError:
            inet = inv.insert_network(self.code, self.start)

        inet.end = self.end
        inet.description = self.description
        inet.institutions = self.institutions
        inet.region = self.region
        inet.type = self.type
        inet.netClass = self.net_class
        inet.archive = self.archive
        inet.restricted = self.restricted
        inet.shared = self.shared
        inet.remark = self.remark

        existing_stations = set()
        for sta_list in self.stations.itervalues():
            for sta in sta_list:
                logs.debug("processing station %s %s %d/%03d" %
                  (self.code, sta.code, sta.start.year, sta.start.utctimetuple()[7]))

                sta.update_inventory(instdb, inv, inet)
                existing_stations.add((sta.code, sta.start))

        for (sta_code, sta_tp) in inet.station.items():
            for sta_start in sta_tp.keys():
                if (sta_code, sta_start) not in existing_stations:
                    inet.remove_station(sta_code, sta_start)
    
    def __get_sta_attr(self, net_code, sta_code, start):
        try:
            return self.__sta_attr[(net_code, sta_code, start)]

        except:
            if start != _EPOCH_DATE:
                start_str = " %d/%03d" % (start.year, start.utctimetuple()[7])
            
            else:
                start_str = ""

            logs.debug("using default attributes for station %s %s%s" %
              (net_code, sta_code, start_str))

            self.__sta_attr[(net_code, sta_code, start)] = dict()
            return self.__sta_attr[(net_code, sta_code, start)]
    
    def parse_tabline(self, code, desc, dlspec, smspec, gain_mult, samp, lat, lon, elev,
        depth_azimuth, start, end = None, orientation = "Z 0.0 -90.0; N 0.0 0.0; E 90.0 0.0"):

        lat = float(lat)
        lon = float(lon)
        elev = float(elev)
        gain_mult = float(gain_mult)
        start = _parse_date(start)
        end = _parse_date(end)

        try:
            depth = float(depth_azimuth)

        except (TypeError, ValueError):
            try:
                (depth, n_az, e_az) = depth_azimuth.split('/')
                depth = float(depth)
                n_az = float(n_az)
                e_az = float(e_az)

                if n_az < 0:
                    n_az += 360

                if e_az < 0:
                    e_az += 360

                orientation = "Z 0.0 -90.0; 1 %.1f 0.0; 2 %.1f 0.0" % (n_az, e_az)

            except (TypeError, ValueError):
                raise NettabError, "invalid depth/azimuth: " + depth_azimuth

        try:
            sta_list = self.stations[code]

        except KeyError:
            mapped_from = None
            mapped_to = _EPOCH_DATE
            sta_list = []
            mappings = self.__statmap.get((code, self.code))
            if mappings:
                for (map_from, map_to, map_net) in mappings:
                    sta_list.append(_Station(code, map_net, map_from, map_to,
                        lat, lon, elev))

                    if _cmptime(map_from, mapped_from) < 0:
                        mapped_from = map_from
                    
                    if _cmptime(map_to, mapped_to) > 0:
                        mapped_to = map_to
                    
                if mapped_from > _EPOCH_DATE:
                    sta_list.append(_Station(code, self.code, _EPOCH_DATE, mapped_from,
                        lat, lon, elev))

                if mapped_to is not None:
                    sta_list.append(_Station(code, self.code, mapped_to, None,
                        lat, lon, elev))

            else:
                sta_list.append(_Station(code, self.code, _EPOCH_DATE, None,
                    lat, lon, elev))

            self.stations[code] = sta_list

        for sta in sta_list:
            sta.parse_tabline(code, desc, dlspec, smspec, gain_mult, samp, lat, lon, elev, depth, start, end,
                orientation, **self.__get_sta_attr(self.code, sta.code, sta.map_from))
            
class _VirtualNetwork(object):
    def __init__(self, code, desc):
        self.code = code
        self.description = desc
        self.stations = set()

    def add_station(self, net_code, sta_code):
        self.stations.add((net_code, sta_code))

    def update_inventory(self, inv):
        try:
            igrp = inv.stationGroup[self.code]

        except KeyError:
            igrp = inv.insert_stationGroup(self.code)

        igrp.description = self.description
        igrp.type = 0

        existing_sref = set()
        for (net_code, sta_code) in self.stations:
            try:
               for net in inv.network[net_code].itervalues():
                   for sta in net.station[sta_code].itervalues():
                       try:
                           igrp.stationReference[sta.publicID]

                       except KeyError:
                           igrp.insert_stationReference(sta.publicID)

                       existing_sref.add(sta.publicID)

            except KeyError:
                logs.error("virtual net %s contains nonexistent station %s_%s" % \
                    (self.code, net_code, sta_code))
        
        for sta_id in igrp.stationReference.keys():
            if sta_id not in existing_sref:
                igrp.remove_stationReference(sta_id)

class Nettab(object):
    def __init__(self, dcid):
        self.__dcid = dcid
        self.__net_attr = {}
        self.__sta_attr = {}
        self.__statmap = {}
        self.__network = {}
        self.__vnet = {}
        self.__access_net = {}
        self.__access_sta = {}
        self.__access_all = []

    def load_network_attr(self, file):
        fd = open(file)
        try:
            try:
                fieldNames = None
                if isPyVersion(2, 3):
                    fieldNames = getFieldNames(fd)

                for row in csv.DictReader(fd, fieldNames):
                    net_code = row['net_code']
                    start = _parse_date(row['start'])

                    if start is None:
                        start = _EPOCH_DATE

                    if (net_code, start) in self.__net_attr:
                        logs.warning("multiple %s found in %s" %
                          (str((net_code, row['start'])), file))

                        continue

                    del row['net_code']
                    #del row['start']
                    row['start'] = start
                    row['end'] = _parse_date(row['end'])
                    row['restricted'] = bool(int(row['restricted']))
                    row['shared'] = bool(int(row['shared']))
                    #self.__net_attr[(net_code, start)] = row
                    self.__net_attr[(net_code, _EPOCH_DATE)] = row

            except KeyError, e:
                raise NettabError, "column %s missing in %s" % (str(e), file)

            except (TypeError, ValueError), e:
                raise NettabError, "error reading %s: %s" % (file, str(e))

        finally:
            fd.close()

    def load_station_attr(self, file):
        fd = open(file)
        try:
            try:
                fieldNames = None
                if isPyVersion(2, 3):
                    fieldNames = getFieldNames(fd)

                for row in csv.DictReader(fd, fieldNames):
                    net_code = row['net_code']
                    sta_code = row['sta_code']
                    start = _parse_date(row['start'])

                    if start is None:
                        start = _EPOCH_DATE

                    if (net_code, sta_code, start) in self.__sta_attr:
                        logs.warning("multiple %s found in %s" %
                          (str((net_code, sta_code, row['start'])), file))

                        continue

                    del row['net_code']
                    del row['sta_code']
                    del row['start']
                    if row['restricted']:
                        row['restricted'] = bool(int(row['restricted']))
                    else:
                        row['restricted'] = None

                    self.__sta_attr[(net_code, sta_code, start)] = row

            except KeyError, e:
                raise NettabError, "column %s missing in %s" % (str(e), file)

            except (TypeError, ValueError), e:
                raise NettabError, "error reading %s: %s" % (file, str(e))

        finally:
            fd.close()

    def load_statmap(self, file):
        fd = open(file)
        try:
            lineno = 0
            try:
                line = fd.readline()
                lineno = 1
                while line:
                    m = _rx_statmap.match(line)
                    if m is None:
                        raise NettabError, "parse error"

                    (sta, net, archive_net, from_def, from_year, from_doy, to_def, to_year, to_doy) = m.groups()

                    try:
                        sta_net = self.__statmap[(sta, net)]

                    except KeyError:
                        sta_net = []
                        self.__statmap[(sta, net)] = sta_net

                    if from_def:
                        from_date = _datetime(int(from_year), int(from_doy))
                    
                    else:
                        from_date = _EPOCH_DATE

                    if to_def:
                        to_date = _datetime(int(to_year), int(to_doy))

                    else:
                        to_date = None
                    
                    sta_net.append((from_date, to_date, archive_net))
                    line = fd.readline()
                    lineno += 1

            except (NettabError, TypeError, ValueError), e:
                raise NettabError, "%s:%d: %s" % (file, lineno, str(e))

        finally:
            fd.close()

    def __load_access(self, file, accmap):
        fd = open(file)
        try:
            lineno = 0
            try:
                line = fd.readline()
                lineno = 1
                while line:
                    splitline = line.split()
                    if splitline:
                        for code in splitline[1:]:
                            if code == "ALL":
                                self.__access_all.append(splitline[0])

                            else:
                                try:
                                    user_list = accmap[code]

                                except KeyError:
                                    user_list = []
                                    accmap[code] = user_list

                                user_list.append(splitline[0])

                    line = fd.readline()
                    lineno += 1

            except (NettabError, TypeError, ValueError), e:
                raise NettabError, "%s:%d: %s" % (file, lineno, str(e))

        finally:
            fd.close()

    def load_access_net(self, file):
        self.__load_access(file, self.__access_net)

    def load_access_stat(self, file):
        self.__load_access(file, self.__access_sta)

    def update_inventory(self, instdb, inv):
        updated_networks = set()
        for net in self.__network.itervalues():
            #net.restricted |= net.code in self.__access_net
            for sta_list in net.stations.itervalues():
                for sta in sta_list:
                    # sta.restricted = net.restricted # ??? or sta.code in self.__access_sta
                    if sta.restricted is None:
                        sta.restricted = net.restricted
                    sta.shared = net.shared
                    sta.archive = net.archive

            logs.debug("processing network %s" % (net.code,))
            net.update_inventory(instdb, inv)
            updated_networks.add(net.code)

        for (net_code, net_tp) in inv.network.items():
            for net_start in net_tp.keys():
                if net_code in updated_networks and \
                    (net_code, net_start) not in self.__network:
                    inv.remove_network(net_code, net_start)
        
        updated_vnets = set()
        for vnet in self.__vnet.itervalues():
            vnet.update_inventory(inv)
            updated_vnets.add(vnet.code)

        for vnet_code in inv.stationGroup.keys():
            if vnet_code not in updated_vnets:
                inv.remove_stationGroup(vnet_code)

    def __allow(self, rtn, net_code, sta_code, user_list, existing_access):
        for user in user_list:
            try:
                acc = rtn.access[net_code][sta_code][""][""][user][_EPOCH_DATE]

            except KeyError:
                acc = rtn.insert_access(net_code, sta_code, "", "", user, _EPOCH_DATE)

            acc.end = None
            existing_access.add((net_code, sta_code, "", "", user, _EPOCH_DATE))
    
    def update_access(self, rtn):
        access_net = dict(self.__access_net)
        access_sta = {}
        existing_access = set()

        for net in self.__network.itervalues():
            #if net.restricted:
            try:
                access_net[net.code] = self.__access_net[net.code] + self.__access_all

            except KeyError:
                access_net[net.code] = self.__access_all

            for sta_code in net.stations.iterkeys():
                try:
                    access_sta[(net.code, sta_code)] = self.__access_sta[sta_code]

                    #if not net.restricted:
                    #    access_sta[(net.code, sta_code)] += self.__access_all

                except KeyError:
                    pass

        for (net_code, user_list) in access_net.iteritems():
            self.__allow(rtn, net_code, "", user_list, existing_access)

        for ((net_code, sta_code), user_list) in access_sta.iteritems():
            self.__allow(rtn, net_code, sta_code, user_list, existing_access)

        for acc_net in rtn.access.values():
            for acc_sta in acc_net.values():
                for acc_loc in acc_sta.values():
                    for acc_user in acc_loc.values():
                        for acc_start in acc_user.values():
                            for acc in acc_start.values():
                                if (acc.networkCode, acc.stationCode, acc.locationCode, acc.streamCode, acc.user, acc.start) not in existing_access:
                                    rtn.remove_access(acc.networkCode, acc.stationCode, acc.locationCode, acc.streamCode, acc.user, acc.start)
        
    def __parse_netline_attr(self, desc, name, code, dc,
        start = _EPOCH_DATE,
        end = None,
        restricted = False,
        shared = True,
        net_class = "p",
        type = "",
        institutions = "",
        region = "",
        remark = ""):
        
        try:
            net = self.__network[(code, start)]

        except KeyError:
            net = _Network(code, self.__sta_attr, self.__statmap)
            self.__network[(code, start)] = net

        net.start = start
        net.end = end
        net.description = desc.replace('_', ' ')
        net.institutions = institutions
        net.region = region
        net.type = type
        net.net_class = net_class
        net.archive = self.__dcid
        net.restricted = restricted
        net.shared = shared
        net.remark = remark
        return net

    def __get_net_attr(self, code, start):
        try:
            return self.__net_attr[(code, start)]

        except:
            if start != _EPOCH_DATE:
                start_str = " %d/%03d" % (start.year, start.utctimetuple()[7])
            
            else:
                start_str = ""

            #logs.warning("using default attributes for network %s%s" %
            #  (code, start_str))
            logs.debug("using default attributes for network %s%s" %
              (code, start_str))

            self.__net_attr[(code, start)] = dict()
            return self.__net_attr[(code, start)]
    
    def __parse_netline(self, desc, name, code, dc, start = _EPOCH_DATE, end = None):
        return self.__parse_netline_attr(desc, name, code, dc, # start, end
            **self.__get_net_attr(code, start))
    
    def load_tab(self, file):
        fd = open(file)
        try:
            net = None
            lineno = 0
            try:
                line = fd.readline()
                lineno = 1
                while line:
                    splitline = line.split()
                    if splitline and splitline[0] and splitline[0][0] != '#':
                        if len(splitline) >= 11 and net is not None:
                            net.parse_tabline(*splitline[:12])
                            if len(splitline) > 12:
                                logs.debug("%s:%d: ignored unknown columns: %s" % \
                                  (file, lineno, " ".join(splitline[12:])))

                        elif len(splitline) >= 4:
                            net = self.__parse_netline(*splitline[:4])
                            if len(splitline) > 4:
                                logs.debug("%s:%d: ignored unknown columns: %s" % \
                                  (file, lineno, " ".join(splitline[4:])))

                        else:
                            raise NettabError, "invalid number of columns (%d)" % (len(splitline),)

                    line = fd.readline()
                    lineno += 1

            except (NettabError, TypeError, ValueError), e:
                raise NettabError, "%s:%d: %s" % (file, lineno, str(e))

        finally:
            fd.close()

    def load_vnet(self, file):
        fd = open(file)
        try:
            vnet = None
            lineno = 0
            try:
                line = fd.readline()
                lineno = 1
                while line:
                    splitline = [ s.strip() for s in line.split(" ", 1) ]
                    if splitline and splitline[0] and splitline[0][0] != '#':
                        if len(splitline) != 2:
                            logs.debug("%s:%d: invalid syntax: %s" % \
                                (file, lineno, line))

                        elif vnet is None:
                            vnet = _VirtualNetwork(*splitline)
                            self.__vnet[vnet.code] = vnet

                        else:
                            vnet.add_station(*splitline)

                    line = fd.readline()
                    lineno += 1

            except (NettabError, TypeError, ValueError), e:
                raise NettabError, "%s:%d: %s" % (file, lineno, str(e))

        finally:
            fd.close()


