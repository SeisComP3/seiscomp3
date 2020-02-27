#*****************************************************************************
# fseed.py
#
# SEED builder for SeisComP
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import sys
import re
import json
import datetime
import mseedlite as mseed
from tempfile import TemporaryFile
from shutil import copyfileobj
from seiscomp import logs

_RECLEN_EXP = 12

def _min_data_gap(fsamp):
    return datetime.timedelta(microseconds=1000000/(fsamp * 10))

#_min_ts_gap = datetime.timedelta(minutes=1)
_min_ts_gap = datetime.timedelta(days=3650)

class SEEDError(Exception):
    pass

_rx_coeff = re.compile(r'\s*(\S+)\s*')

def _mkseedcoeff_fir(nblk, nfld, ncoeff, s):
    pos = 0
    n = 0
    c = ""

    while pos < len(s):
        m = _rx_coeff.match(s, pos)
        if m is None:
            raise SEEDError, "blockette %d, field %d: error parsing FIR coefficients at '%s'" % (nblk, nfld, s[pos:])

        try:
            v = float(m.group(1))
        except ValueError:
            raise SEEDError, "blockette %d, field %d: error parsing FIR coefficients at '%s'" % (nblk, nfld, s[pos:])

        c += "%14.7E" % (v,)
        n += 1
        pos = m.end()

    if n != ncoeff:
        raise SEEDError, "blockette %d, field %d: expected %d coefficients, found %d" % (nblk, nfld, ncoeff, n)

    return c

def _mkseedcoeff_iir(nblk, nfld, ncoeff, s):
    pos = 0
    n = 0
    c = ""

    while pos < len(s):
        m = _rx_coeff.match(s, pos)
        if m is None:
            raise SEEDError, "blockette %d, field %d: error parsing IIR coefficients at '%s'" % (nblk, nfld, s[pos:])

        try:
            v = float(m.group(1))
        except ValueError:
            raise SEEDError, "blockette %d, field %d: error parsing IIR coefficients at '%s'" % (nblk, nfld, s[pos:])

        c += "%12.5E%12.5E" % (v,0)
        n += 1
        pos = m.end()

    if n != ncoeff:
        raise SEEDError, "blockette %d, field %d: expected %d coefficients, found %d" % (nblk, nfld, ncoeff, n)

    return c

def _mkseedcoeff_polynomial(nblk, nfld, ncoeff, s, gain=1.0):
    pos = 0
    n = 0
    c = ""

    while pos < len(s):
        m = _rx_coeff.match(s, pos)
        if m is None:
            raise SEEDError, "blockette %d, field %d: error parsing polynomial coefficients at '%s'" % (nblk, nfld, s[pos:])

        try:
            v = float(m.group(1))
        except ValueError:
            raise SEEDError, "blockette %d, field %d: error parsing polynomial coefficients at '%s'" % (nblk, nfld, s[pos:])

        c += "%12.5E%12.5E" % (v/(gain**n),0)
        n += 1
        pos = m.end()

    if n != ncoeff:
        raise SEEDError, "blockette %d, field %d: expected %d coefficients, found %d" % (nblk, nfld, ncoeff, n)

    return c

_rx_paz = re.compile(r'\s*([0-9]*)\(\s*([^,]+),\s*([^)]+)\)\s*')

def _mkseedpaz(nblk, nfld, npaz, s):
    pos = 0
    n = 0
    c = ""

    l = 0
    if not s is None: l = len(s)

    while pos < l:
        m = _rx_paz.match(s, pos)
        if m is None:
            raise SEEDError, "blockette %d, field %d: error parsing PAZ at '%s'" % (nblk, nfld, s[pos:])

        try:
            if len(m.group(1)) > 0:
                x = int(m.group(1))
            else:
                x = 1

            rv = float(m.group(2))
            iv = float(m.group(3))

        except ValueError:
            raise SEEDError, "blockette %d, field %d: error parsing PAZ at '%s'" % (nblk, nfld, s[pos:])

        for i in xrange(0, x):
            c += "%12.5E%12.5E 0.00000E-00 0.00000E-00" % (rv, iv)

        n += x
        pos = m.end()

    if not npaz is None and (n != npaz):
        raise SEEDError, "blockette %d, field %d: expected %d PAZ, found %d" % (nblk, nfld, npaz, n)

    return (c,n)

def _mkseedfap(nblk, nfld, nfap, s):
    pos = 0
    n = 0
    c = ""

    values = s.split()
    l = len(values)
    while pos < l:
      try:
        f = float(values[pos])
      except:
        raise SEEDError, "blockette %d, field %d: error parsing FAP at '%s'" % (nblk, nfld, values[pos])

      pos += 1

      try:
        a = float(values[pos])
      except:
        raise SEEDError, "blockette %d, field %d: error parsing FAP at '%s'" % (nblk, nfld, values[pos])

      pos += 1

      try:
        p = float(values[pos])
      except:
        raise SEEDError, "blockette %d, field %d: error parsing FAP at '%s'" % (nblk, nfld, values[pos])

      pos += 1

      c += "%12.5E%12.5E 0.00000E-00%12.5E 0.00000E-00" % (f, a, p)

      n += 1

    if not nfap is None and (n != nfap):
        raise SEEDError, "blockette %d, field %d: expected %d FAP, found %d" % (nblk, nfld, nfap, n)

    return (c,n)

def _mkseedstring(nblk, nfld, s, min_length, max_length, flags):
    U = L = N = P = S = X = False
    rx_list = []

    if flags.find("U") != -1:
        U = True
        rx_list.append("[A-Z]")

    if flags.find("L") != -1:
        L = True
        rx_list.append("[a-z]")

    if flags.find("N") != -1:
        N = True
        rx_list.append("[0-9]")

    if flags.find("P") != -1:
        P = True
        rx_list.append("[^A-Za-z0-9 ]")

    if flags.find("S") != -1:
        S = True
        rx_list.append(" ")

    if flags.find("_") != -1:
        X = True
        rx_list.append("_")

    sn = s.strip()[:max_length]

    if U and not L:
        sn = sn.upper()
    elif L and not U:
        sn = sn.lower()

    if S and not X:
        sn = sn.replace("_", " ")
    elif X and not S:
        sn = sn.replace(" ", "_")

    rx = "|".join(rx_list)
    sn = "".join(re.findall(rx, sn))

    if re.match("(" + rx + ")*$", sn) is None:
        raise SEEDError, "blockette %d, field %d: cannot convert string \"%s\" with flags %s" % \
          (nblk, nfld, s, flags)

    if len(sn) < min_length:
        if min_length != max_length:
            raise SEEDError, "blockette %d, field %d: cannot extend string \"%s\" to minimum length %d with flags %s" % \
              (nblk, nfld, s, min_length, flags)
        else:
            sn = (sn + min_length * " ")[:min_length]

    if min_length != max_length:
        sn += "~"

    return sn

def _mkseedtime(nblk, nfld, t):
    if t is None:
        return "~"

    if isinstance(t, datetime.datetime):
        tt = t.utctimetuple()
        return "%04d,%03d,%02d:%02d:%02d.%04d~" % (t.year, tt[7],
            t.hour, t.minute, t.second, t.microsecond // 100)
    elif isinstance(t, datetime.date):
        tt = datetime.datetime.combine(t, datetime.time(0, 0, 0)).utctimetuple()
        return "%04d,%03d~" % (t.year, tt[7])

    raise SEEDError, "blockette %d, field %d: invalid time object: %s" % (nblk, nfld, str(t))

def _cmptime(t1, t2):
    if t1 is None and t2 is None:
        return 0
    elif t2 is None or (t1 is not None and t1 < t2):
        return -1
    elif t1 is None or (t2 is not None and t1 > t2):
        return 1

    return 0

def _is_fir_response(obj):
    return hasattr(obj, "symmetry")

def _is_iir_response(obj):
    return hasattr(obj, "denominators")

def _is_paz_response(obj):
    return hasattr(obj, "poles")

def _is_poly_response(obj):
    return hasattr(obj, "approximationType")

def _is_fap_response(obj):
    return hasattr(obj, "tuples")

class _Blockette10(object):
    def __init__(self, record_length, start_time, end_time, vol_time,
        organization, label):
        self.__record_length = record_length
        self.__start_time = _mkseedtime(10, 5, start_time)
        self.__end_time = _mkseedtime(10, 6, end_time)
        self.__vol_time = _mkseedtime(10, 7, vol_time)
        self.__organization = _mkseedstring(10, 8, organization, 1, 80, "UNLPS_")
        self.__label = _mkseedstring(10, 9, label, 0, 80, "UNLPS_")
        self.__len = 13 + len(self.__start_time) + len(self.__end_time) + \
            len(self.__vol_time) + len(self.__organization) + \
            len(self.__label)

    def output(self, f):
        blk = "010%4d 2.3%2d%s%s%s%s%s" % (self.__len, self.__record_length,
            self.__start_time, self.__end_time, self.__vol_time,
            self.__organization, self.__label)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 10 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette11(object):
    def __init__(self):
        self.__nstations = 0
        self.__stat_rec = ()
        self.__len = 10

    def add_station(self, code, recno):
        self.__stat_rec += (_mkseedstring(11, 4, code, 5, 5, "UN"), recno)
        self.__nstations += 1
        self.__len += 11

    def __output_huge(self, f):
        n = 0
        while n < self.__nstations:
            ns = min(self.__nstations - n, 908)
            blen = 10 + 11 * ns

            blk = ("011%4d%3d" + ns * "%s%6d") % \
                ((blen, ns) + self.__stat_rec[2*n:2*(n+ns)])

            if len(blk) != blen:
                raise SEEDError, "blockette 11 has invalid length: %d instead of %d" % (len(blk), blen)

            f.write_blk(blk)
            n += ns

    def output(self, f):
        if self.__len > 9999:
            self.__output_huge(f)
            return

        blk = ("011%4d%3d" + self.__nstations * "%s%6d") % \
            ((self.__len, self.__nstations) + self.__stat_rec)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 11 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette12(object):
    def __init__(self):
        self.__nspans = 0
        self.__span_rec = ()
        self.__len = 11

    def add_span(self, begin, end, recno):
        self.__span_rec += (_mkseedtime(12, 4, begin), _mkseedtime(12, 5, end), recno)
        self.__nspans += 1
        self.__len += 52

    def __output_huge(self, f):
        n = 0
        while n < self.__nspans:
            ns = min(self.__nspans - n, 192)
            blen = 11 + 52 * ns

            blk = ("012%4d%4d" + ns * "%s%s%6d") % \
                ((blen, ns) + self.__span_rec[3*n:3*(n+ns)])

            if len(blk) != blen:
                raise SEEDError, "blockette 12 has invalid length: %d instead of %d" % (len(blk), blen)

            f.write_blk(blk)
            n += ns

    def output(self, f):
        if self.__len > 9999:
            self.__output_huge(f)
            return

        blk = ("012%4d%4d" + self.__nspans * "%s%s%6d") % \
            ((self.__len, self.__nspans) + self.__span_rec)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 12 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette30(object):
    def __init__(self, name, key, family, ddl):
        self.__name = _mkseedstring(30, 3, name, 1, 50, "UNLPS")
        self.__key = key
        self.__family = family
        self.__ddl = "~".join(ddl) + "~"
        self.__nddl = len(ddl)
        self.__len = 16 + len(self.__name) + len(self.__ddl)

    def output(self, f):
        blk = "030%4d%s%4d%3d%2d%s" % (self.__len, self.__name,
            self.__key, self.__family, self.__nddl, self.__ddl)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 30 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette31(object):
    def __init__(self, key, cclass, comment):
        self.__key = key
        self.__cclass = cclass
        self.__comment = _mkseedstring(31, 5, comment, 1, 70, "UNLPS")
        self.__len = 15 + len(self.__comment)

    def output(self, f):
        blk = "031%4d%4d%s%s  0" % (self.__len, self.__key, self.__cclass, self.__comment)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 31 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette33(object):
    def __init__(self, key, desc):
        self.__key = key
        self.__desc = _mkseedstring(33, 4, desc, 1, 50, "UNLPS")
        self.__len = 10 + len(self.__desc)

    def output(self, f):
        blk = "033%4d%3d%s" % (self.__len, self.__key, self.__desc)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 33 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette34(object):
    def __init__(self, key, name, desc):
        self.__key = key
        self.__name = _mkseedstring(34, 4, name, 1, 20, "UNP")
        self.__desc = _mkseedstring(34, 5, desc, 1, 50, "UNLPS")
        self.__len = 10 + len(self.__name) + len(self.__desc)

    def output(self, f):
        blk = "034%4d%3d%s%s" % (self.__len, self.__key, self.__name,
            self.__desc)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 34 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette41(object):
    def __init__(self, key, name, symmetry, input_units, output_units, ncoeff,
        coeff):

        self.__key = key
        self.__name = _mkseedstring(41, 4, name, 1, 25, "UN")
        self.__symmetry = _mkseedstring(41, 5, symmetry, 1, 1, "U")
        self.__input_units = input_units
        self.__output_units = output_units
        self.__ncoeff = ncoeff
        self.__coeff = _mkseedcoeff_fir(41, 9, ncoeff, coeff)
        self.__len = 22 + 14 * ncoeff + len(self.__name)

    def __output_huge(self, f):
        n = 0
        while n < self.__ncoeff:
            nc = min(self.__ncoeff - n, (9977 - len(self.__name)) // 14)
            blen = 22 + 14 * nc + len(self.__name)

            blk = "041%4d%4d%s%s%3d%3d%4d%s" % (blen, self.__key,
                self.__name, self.__symmetry, self.__input_units,
                self.__output_units, self.__ncoeff, self.__coeff[14*n:14*(n+nc)])

            if len(blk) != blen:
                raise SEEDError, "blockette 41 has invalid length: %d instead of %d" % (len(blk), blen)

            f.write_blk(blk)
            n += nc

    def output(self, f):
        if self.__len > 9999:
            self.__output_huge(f)
            return

        blk = "041%4d%4d%s%s%3d%3d%4d%s" % (self.__len, self.__key,
            self.__name, self.__symmetry, self.__input_units,
            self.__output_units, self.__ncoeff, self.__coeff)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 41 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette42(object):
    def __init__(self, key, name, input_units, output_units, freq_unit, low_freq,
       high_freq, approx_type, approx_lower_bound, approx_upper_bound,
       approx_error, ncoeff, coeff):

        self.__key = key
        self.__name = _mkseedstring(42, 4, name, 1, 25, "UN")
        self.__input_units = input_units
        self.__output_units = output_units
        self.__freq_unit = freq_unit
        self.__low_freq = low_freq
        self.__high_freq = high_freq
        self.__approx_type = approx_type
        self.__approx_lower_bound = approx_lower_bound
        self.__approx_upper_bound = approx_upper_bound
        self.__approx_error = approx_error
        self.__ncoeff = ncoeff
        self.__coeff = _mkseedcoeff_polynomial(42, 16, ncoeff, coeff)
        self.__len = 83 + 24 * ncoeff + len(self.__name)

    def output(self, f):
        blk = "042%4d%4d%sP%3d%3d%1s%1s%12.5E%12.5E%12.5E%12.5E%12.5E%3d%s" % (self.__len,
            self.__key, self.__name, self.__input_units, self.__output_units,
            self.__approx_type, self.__freq_unit, self.__low_freq, self.__high_freq,
            self.__approx_lower_bound, self.__approx_upper_bound, self.__approx_error,
            self.__ncoeff, self.__coeff)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 42 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette43(object):
    def __init__(self, key, name, type, input_units, output_units, norm_fac,
        norm_freq, nzeros, zeros, npoles, poles):

        self.__key = key
        self.__name = _mkseedstring(43, 4, name, 1, 25, "UN")
        self.__type = _mkseedstring(43, 5, type, 1, 1, "U")
        self.__input_units = input_units
        self.__output_units = output_units
        self.__norm_fac = norm_fac
        self.__norm_freq = norm_freq
        self.__zeros, nzeros = _mkseedpaz(43, 11, nzeros, zeros)
        self.__nzeros = nzeros
        self.__poles, npoles = _mkseedpaz(43, 16, npoles, poles)
        self.__npoles = npoles
        self.__len = 48 + 48 * (nzeros + npoles) + len(self.__name)

    def output(self, f):
        blk = "043%4d%4d%s%s%3d%3d%12.5E%12.5E%3d%s%3d%s" % \
            (self.__len, self.__key, self.__name, self.__type,
            self.__input_units, self.__output_units, self.__norm_fac,
            self.__norm_freq, self.__nzeros, self.__zeros, self.__npoles,
            self.__poles)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 43 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette44(object):
    def __init__(self, key, name, type, input_units, output_units,
        n_numerators=0, numerators="", n_denominators=0, denominators=""):

        self.__key = key
        self.__name = _mkseedstring(44, 4, name, 1, 25, "UN")
        self.__type = _mkseedstring(44, 5, type, 1, 1, "U")
        self.__input_units = input_units
        self.__output_units = output_units
        self.__n_numerators = n_numerators
        self.__numerators = _mkseedcoeff_iir(54, 8, n_numerators, numerators)
        self.__n_denominators = n_denominators
        self.__denominators = _mkseedcoeff_iir(54, 11, n_denominators, denominators)
        self.__len = 26 + len(self.__name) + 24 * n_numerators + 24 * n_denominators

    def output(self, f):
        blk = "044%4d%4d%s%s%3d%3d%4d%s%4d%s" % (self.__len, self.__key,
            self.__name, self.__type, self.__input_units, self.__output_units,
            self.__n_numerators, self.__numerators, self.__n_denominators, self.__denominators)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 44 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette45(object):
    def __init__(self, key, name, type, input_units, output_units, ntuples, tuples):

        self.__key = key
        self.__name = _mkseedstring(45, 4, name, 1, 25, "UN")
        self.__input_units = input_units
        self.__output_units = output_units
        self.__tuples, ntuples = _mkseedfap(55, 7, ntuples, tuples)
        self.__ntuples = ntuples
        self.__len = 21 + 60 * ntuples + len(self.__name)

    def output(self, f):
        if self.__len < 10000:
            blk = "055%4d%4d%s%3d%3d%4d%s" % \
                (self.__len, self.__key, self.__name,
                 self.__input_units, self.__output_units,
                 self.__ntuples, self.__tuples)

            if len(blk) != self.__len:
                raise SEEDError, "blockette 45 has invalid length: %d instead of %d" % (len(blk), self.__len)

            f.write_blk(blk)
        else:
            btuples = int((9999-21-len(self.__name))/60)
            l = 21 + 60 * btuples + len(self.__name)
            blk = "055%4d%4d%s%3d%3d%4d%s" % \
                (l, self.__key, self.__name,
                 self.__input_units, self.__output_units,
                 btuples, self.__tuples[:btuples*60])

            if len(blk) != l:
                raise SEEDError, "blockette 45 has invalid length: %d instead of %d" % (len(blk), l)

            f.write_blk(blk)

            left_tuples = self.__ntuples-btuples
            ofs = btuples*60
            while left_tuples > 0:
                if left_tuples > btuples:
                    ntuples = btuples
                else:
                    ntuples = left_tuples
                l = 21 + ntuples * 60 + len(self.__name)
                blk = "055%4d%4d%s%3d%3d%4d" % \
                (l, self.__key, self.__name,
                 self.__input_units, self.__output_units,
                 ntuples, self.__tuples[ofs:ofs+ntuples*60])

                if len(blk) != l:
                    raise SEEDError, "blockette 45 has invalid length: %d instead of %d" % (len(blk), l)

                f.write_blk(blk)

                ofs += ntuples*60
                left_tuples -= ntuples

class _Blockette47(object):
    def __init__(self, key, name, input_rate, deci_fac, deci_offset,
        delay, correction):

        self.__key = key
        self.__name = _mkseedstring(47, 4, name, 1, 25, "UN")
        self.__input_rate = input_rate
        self.__deci_fac = deci_fac
        self.__deci_offset = deci_offset
        self.__delay = delay
        self.__correction = correction
        self.__len = 53 + len(self.__name)

    def output(self, f):
        blk = "047%4d%4d%s%10.4E%5d%5d%11.4E%11.4E" % (self.__len,
            self.__key, self.__name, self.__input_rate, self.__deci_fac,
            self.__deci_offset, self.__delay, self.__correction)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 47 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette48(object):
    def __init__(self, key, name, gain, gain_freq):
        self.__key = key
        self.__name = _mkseedstring(48, 4, name, 1, 25, "UN")
        self.__gain = gain
        self.__gain_freq = gain_freq
        self.__len = 37 + len(self.__name)

    def output(self, f):
        blk = "048%4d%4d%s%12.5E%12.5E 0" % (self.__len, self.__key,
            self.__name, self.__gain, self.__gain_freq)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 48 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette50(object):
    def __init__(self, stat_code, latitude, longitude, elevation,
        site_name, net_id, net_code, start_date, end_date):
        self.__stat_code = _mkseedstring(50, 3, stat_code, 5, 5, "UN")
        self.__latitude = latitude
        self.__longitude = longitude
        self.__elevation = elevation
        self.__site_name = _mkseedstring(50, 9, site_name, 1, 60, "UNLPS")
        self.__net_id = net_id
        self.__start_date = _mkseedtime(50, 13, start_date)
        self.__end_date = _mkseedtime(50, 14, end_date)
        self.__net_code = _mkseedstring(50, 16, net_code, 2, 2, "UN")
        self.__len = 59 + len(self.__site_name) + len(self.__start_date) + \
            len(self.__end_date)

        error = False
        if not isinstance(self.__stat_code, str):
            print >> sys.stderr, "blockette 50 - station code not set or not of type string"
            error = True
        if not isinstance(self.__latitude, float):
            print >> sys.stderr, "blockette 50 - latitude not set or not of type float"
            error = True
        if not isinstance(self.__longitude, float):
            print >> sys.stderr, "blockette 50 - longitude not set or not of type float"
            error = True
        if not isinstance(self.__elevation, float):
            print >> sys.stderr, "blockette 50 - elevation not set or not of type float"
            error = True

        if error:
            raise SEEDError, "blockette 50 - provide correct values. Adjust your inventory XML!"

    def output(self, f):
        blk = "050%4d%s%10.6f%11.6f%7.1f       %s%3d321010%s%sN%s" % \
            (self.__len, self.__stat_code, self.__latitude, self.__longitude,
            self.__elevation, self.__site_name, self.__net_id,
            self.__start_date, self.__end_date, self.__net_code)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 50 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette51(object):
    def __init__(self, start_time, end_time, comment_key):
        self.__start_time = _mkseedtime(51, 3, start_time)
        self.__end_time = _mkseedtime(51, 4, end_time)
        self.__comment_key = comment_key
        self.__len = 17 + len(self.__start_time) + len(self.__end_time)

    def output(self, f):
        blk = "051%4d%s%s%4d     0" % (self.__len, self.__start_time,
            self.__end_time, self.__comment_key)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 51 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette52(object):
    def __init__(self, loc_id, chan_id, instr_id, comment, signal_units,
        calibration_units, latitude, longitude, elevation, local_depth,
        azimuth, dip, data_format, record_length, sample_rate, clock_drift,
        flags, start_date, end_date):
        self.__loc_id = _mkseedstring(52, 3, loc_id, 2, 2, "UN")
        self.__chan_id = _mkseedstring(52, 4, chan_id, 3, 3, "UN")
        self.__instr_id = instr_id
        self.__comment = _mkseedstring(52, 7, comment, 0, 30, "UNLPS")
        self.__signal_units = signal_units
        self.__calibration_units = calibration_units
        self.__latitude = latitude
        self.__longitude = longitude
        self.__elevation = elevation
        self.__local_depth = local_depth
        self.__azimuth = azimuth
        self.__dip = dip
        self.__data_format = data_format
        self.__record_length = record_length
        self.__sample_rate = sample_rate
        self.__clock_drift = clock_drift
        self.__flags = _mkseedstring(52, 21, flags, 0, 26, "U")
        self.__raw_start_date = start_date
        self.__raw_end_date = end_date
        self.__len = 0

        error = False
        if not isinstance(self.__loc_id, str):
            print >> sys.stderr, "blockette 52 - location ID not set or not of type string"
            error = True
        if not isinstance(self.__chan_id, str):
            print >> sys.stderr, "blockette 52 - channel ID not set or not of type string"
            error = True
        if not isinstance(self.__len, int):
            print >> sys.stderr, "blockette 52 - len not set or not of type int"
            error = True
        if not isinstance(self.__instr_id, int):
            print >> sys.stderr, "blockette 52 - instrument ID not set or not of type integer"
            error = True
        if not isinstance(self.__comment, str):
            print >> sys.stderr, "blockette 52 - comment not set or not of type sting"
            error = True
        if not isinstance(self.__signal_units, int):
            print >> sys.stderr, "blockette 52 - signal_units not set or not of type integer"
            error = True
        if not isinstance(self.__calibration_units, int):
            print >> sys.stderr, "blockette 52 - calibration units not set or not of type integer"
            error = True
        if not isinstance(self.__latitude, float):
            print >> sys.stderr, "blockette 52 - latitude not set or not of type float"
            error = True
        if not isinstance(self.__longitude, float):
            print >> sys.stderr, "blockette 52 - longitude not set or not of type float"
            error = True
        if not isinstance(self.__elevation, float):
            print >> sys.stderr, "blockette 52 - elevation not set or not of type float"
            error = True
        if not isinstance(self.__local_depth, float):
            print >> sys.stderr, "Warning: blockette 52 - local depth not set or not of type float"
            error = True
        if not isinstance(self.__azimuth, float):
            print >> sys.stderr, "blockette 52 - azimuth not set or not of type float"
            error = True
        if not isinstance(self.__dip, float):
            print >> sys.stderr, "blockette 52 - dip not set or not of type float"
            error = True
        if not isinstance(self.__data_format, int):
            print >> sys.stderr, "blockette 52 - data format not set or not of type integer"
            error = True
        if not isinstance(self.__record_length, int):
            print >> sys.stderr, "blockette 52 - record length not set or not of type integer"
            error = True
        if not isinstance(self.__sample_rate, float):
            print >> sys.stderr, "blockette 52 - sample rate not set or not of type float"
            error = True
        if not isinstance(self.__clock_drift, float):
            print >> sys.stderr, "Warning: blockette 52 - clock drift not set or not of type float"
            error = True

        if error:
            raise SEEDError, "blockette 52 - provide correct values. Adjust your inventory XML!"

    def set_vol_span(self, vol_start, vol_end):
        # make verseed happy
        if _cmptime(self.__raw_end_date, vol_end) > 0:
            self.__raw_end_date = vol_end

        if _cmptime(self.__raw_start_date, vol_start) < 0:
            self.__raw_start_date = vol_start

    def output(self, f):
        self.__start_date = _mkseedtime(52, 22, self.__raw_start_date)
        self.__end_date = _mkseedtime(52, 23, self.__raw_end_date)
        self.__len = 99 + len(self.__comment) + len(self.__flags) + \
            len(self.__start_date) + len(self.__end_date)

        blk = "052%4d%s%s   0%3d%s%3d%3d%10.6f%11.6f%7.1f%5.1f%5.1f%5.1f%4d%2d%10.4E%10.4E    %s%s%sN" % \
            (self.__len, self.__loc_id, self.__chan_id, self.__instr_id,
            self.__comment, self.__signal_units, self.__calibration_units,
            self.__latitude, self.__longitude, self.__elevation,
            self.__local_depth, self.__azimuth, self.__dip, self.__data_format,
            self.__record_length, self.__sample_rate, self.__clock_drift,
            self.__flags, self.__start_date, self.__end_date)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 52 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette53(object):
    def __init__(self, type, input_units, output_units, norm_fac,
        norm_freq, nzeros, zeros, npoles, poles):

        self.__type = _mkseedstring(53, 3, type, 1, 1, "U")
        self.__stage = 0
        self.__input_units = input_units
        self.__output_units = output_units
        self.__norm_fac = norm_fac
        self.__norm_freq = norm_freq
        self.__zeros, nzeros = _mkseedpaz(53, 10, nzeros, zeros)
        self.__nzeros = nzeros
        self.__poles, npoles = _mkseedpaz(53, 15, npoles, poles)
        self.__npoles = npoles
        self.__len = 46 + 48 * (nzeros + npoles)

    def set_stage(self, stage):
        self.__stage = stage

    def output(self, f):
        blk = "053%4d%s%2d%3d%3d%12.5E%12.5E%3d%s%3d%s" % \
            (self.__len, self.__type, self.__stage,
            self.__input_units, self.__output_units, self.__norm_fac,
            self.__norm_freq, self.__nzeros, self.__zeros, self.__npoles,
            self.__poles)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 53 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette54(object):
    def __init__(self, type, input_units, output_units,
        n_numerators=0, numerators="", n_denominators=0, denominators=""):

        self.__type = _mkseedstring(54, 3, type, 1, 1, "U")
        self.__stage = 0
        self.__input_units = input_units
        self.__output_units = output_units
        self.__n_numerators = n_numerators
        self.__numerators = _mkseedcoeff_iir(54, 8, n_numerators, numerators)
        self.__n_denominators = n_denominators
        self.__denominators = _mkseedcoeff_iir(54, 11, n_denominators, denominators)
        self.__len = 24 + 24 * n_numerators + 24 * n_denominators

    def set_stage(self, stage):
        self.__stage = stage

    def output(self, f):
        blk = "054%4d%s%2d%3d%3d%4d%s%4d%s" % (self.__len,
            self.__type, self.__stage, self.__input_units, self.__output_units,
            self.__n_numerators, self.__numerators, self.__n_denominators, self.__denominators)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 54 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette55(object):
    def __init__(self, input_units, output_units, ntuples, tuples):
        self.__stage = 0
        self.__input_units = input_units
        self.__output_units = output_units
        self.__tuples, ntuples = _mkseedfap(55, 7, ntuples, tuples)
        self.__ntuples = ntuples
        self.__len = 19 + 60 * ntuples

    def set_stage(self, stage):
        self.__stage = stage

    def output(self, f):
        if self.__len < 10000:
            blk = "055%4d%2d%3d%3d%4d%s" % \
                (self.__len, self.__stage,
                 self.__input_units, self.__output_units,
                 self.__ntuples, self.__tuples)

            if len(blk) != self.__len:
                raise SEEDError, "blockette 55 has invalid length: %d instead of %d" % (len(blk), self.__len)

            f.write_blk(blk)
        else:
            l = 19 + 166*60
            blk = "055%4d%2d%3d%3d%4d%s" % \
                (l, self.__stage,
                 self.__input_units, self.__output_units,
                 166, self.__tuples[:166*60])

            if len(blk) != l:
                raise SEEDError, "blockette 55 has invalid length: %d instead of %d" % (len(blk), l)

            f.write_blk(blk)

            left_tuples = self.__ntuples-166
            ofs = 166*60
            while left_tuples > 0:
                if left_tuples > 166:
                    ntuples = 166
                else:
                    ntuples = left_tuples
                l = 19 + ntuples * 60
                blk = "055%4d%2d%3d%3d%4d%s" % \
                (l, self.__stage,
                 self.__input_units, self.__output_units,
                 ntuples, self.__tuples[ofs:ofs+ntuples*60])

                if len(blk) != l:
                    raise SEEDError, "blockette 55 has invalid length: %d instead of %d" % (len(blk), l)

                f.write_blk(blk)

                ofs += ntuples*60
                left_tuples -= ntuples

class _Blockette57(object):
    def __init__(self, input_rate, deci_fac, deci_offset, delay, correction):
        self.__stage = 0
        self.__input_rate = input_rate
        self.__deci_fac = deci_fac
        self.__deci_offset = deci_offset
        self.__delay = delay
        self.__correction = correction
        self.__len = 51

    def set_stage(self, stage):
        self.__stage = stage

    def output(self, f):
        blk = "057%4d%2d%10.4E%5d%5d%11.4E%11.4E" % (self.__len,
            self.__stage, self.__input_rate, self.__deci_fac,
            self.__deci_offset, self.__delay, self.__correction)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 57 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette58(object):
    def __init__(self, gain, gain_freq):
        self.__stage = 0
        self.__gain = gain
        self.__gain_freq = gain_freq
        self.__len = 35

    def set_stage(self, stage):
        self.__stage = stage

    def output(self, f):
        blk = "058%4d%2d%12.5E%12.5E 0" % (self.__len, self.__stage,
            self.__gain, self.__gain_freq)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 58 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette59(object):
    def __init__(self, start_time, end_time, comment_key):
        self.__start_time = _mkseedtime(51, 3, start_time)
        self.__end_time = _mkseedtime(51, 4, end_time)
        self.__comment_key = comment_key
        self.__len = 17 + len(self.__start_time) + len(self.__end_time)

    def output(self, f):
        blk = "059%4d%s%s%4d     0" % (self.__len, self.__start_time,
            self.__end_time, self.__comment_key)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 59 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette60(object):
    def __init__(self, start_stage):
        self.__start_stage = start_stage
        self.__ref_list = []
        self.__len = 9

    def add_stage(self, *keyref):
        self.__ref_list.append(keyref)
        self.__len += 4 + 4 * len(keyref)

    def output(self, f):
        blk = "060%4d%2d" % (self.__len, len(self.__ref_list))

        for (n, r) in enumerate(self.__ref_list):
            blk += ("%2d%2d" + len(r) * "%4d") % \
               ((self.__start_stage + n, len(r)) + r)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 60 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette61(object):
    def __init__(self, name, symmetry, input_units, output_units, ncoeff,
        coeff):

        self.__stage = 0
        self.__name = _mkseedstring(61, 4, name, 1, 25, "UN")
        self.__symmetry = _mkseedstring(61, 5, symmetry, 1, 1, "U")
        self.__input_units = input_units
        self.__output_units = output_units
        self.__ncoeff = ncoeff
        self.__coeff = _mkseedcoeff_fir(61, 9, ncoeff, coeff)
        self.__len = 20 + 14 * ncoeff + len(self.__name)

    def set_stage(self, stage):
        self.__stage = stage

    def __output_huge(self, f):
        n = 0
        while n < self.__ncoeff:
            nc = min(self.__ncoeff - n, (9977 - len(self.__name)) // 14)
            blen = 20 + 14 * nc + len(self.__name)

            blk = "061%4d%2d%s%s%3d%3d%4d%s" % (blen, self.__stage,
                self.__name, self.__symmetry, self.__input_units,
                self.__output_units, nc, self.__coeff[14*n:14*(n+nc)])

            if len(blk) != blen:
                raise SEEDError, "blockette 61 has invalid length: %d instead of %d" % (len(blk), self.__len)

            f.write_blk(blk)
            n += nc

    def output(self, f):
        if self.__len > 9999:
            self.__output_huge(f)
            return

        blk = "061%4d%2d%s%s%3d%3d%4d%s" % (self.__len, self.__stage,
            self.__name, self.__symmetry, self.__input_units,
            self.__output_units, self.__ncoeff, self.__coeff)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 61 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette62(object):
    def __init__(self, input_units, output_units, freq_unit, low_freq,
        high_freq, approx_type, approx_lower_bound, approx_upper_bound,
        approx_error, ncoeff, coeff, gain=1.0):

        self.__stage = 0
        self.__input_units = input_units
        self.__output_units = output_units
        self.__freq_unit = freq_unit
        self.__low_freq = low_freq
        self.__high_freq = high_freq
        self.__approx_type = approx_type
        self.__approx_lower_bound = approx_lower_bound
        self.__approx_upper_bound = approx_upper_bound
        self.__approx_error = approx_error
        self.__ncoeff = ncoeff
        self.__coeff = _mkseedcoeff_polynomial(62, 15, ncoeff, coeff, gain)
        self.__len = 81 + 24 * ncoeff

    def set_stage(self, stage):
        self.__stage = stage

    def output(self, f):
        blk = "062%4dP%2d%3d%3d%1s%1s%12.5E%12.5E%12.5E%12.5E%12.5E%3d%s" % (self.__len,
            self.__stage, self.__input_units, self.__output_units,
            self.__approx_type[:1], self.__freq_unit[:1], self.__low_freq, self.__high_freq,
            self.__approx_lower_bound, self.__approx_upper_bound, self.__approx_error,
            self.__ncoeff, self.__coeff)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 62 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette70(object):
    def __init__(self, flag, begin, end):
        self.__flag = flag
        self.__begin = _mkseedtime(70, 4, begin)
        self.__end = _mkseedtime(70, 5, end)
        self.__len = 54

    def output(self, f):
        blk = "070%4d%c%s%s" % (self.__len, self.__flag, self.__begin,
            self.__end)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 70 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _Blockette74(object):
    def __init__(self, net_code, stat_code, loc_id, chan_id,
        start_time, start_recno, end_time, end_recno):
        self.__net_code = _mkseedstring(74, 16, net_code, 2, 2, "UN")
        self.__stat_code = _mkseedstring(74, 3, stat_code, 5, 5, "UN")
        self.__loc_id = _mkseedstring(74, 4, loc_id, 2, 2, "UN")
        self.__chan_id = _mkseedstring(74, 5, chan_id, 3, 3, "UN")
        self.__start_time = _mkseedtime(74, 6, start_time)
        self.__start_recno = start_recno
        self.__end_time = _mkseedtime(74, 9, end_time)
        self.__end_recno = end_recno
        self.__len = 84

    def add_accelerator(self, recno):
        pass

    def output(self, f):
        blk = "074%4d%s%s%s%s%6d 1%s%6d 1  0%s" % (self.__len,
            self.__stat_code, self.__loc_id, self.__chan_id,
            self.__start_time, self.__start_recno, self.__end_time,
            self.__end_recno, self.__net_code)

        if len(blk) != self.__len:
            raise SEEDError, "blockette 74 has invalid length: %d instead of %d" % (len(blk), self.__len)

        f.write_blk(blk)

class _FormatDict(object):
    __formats = {
        "Steim1": ("Steim1 Integer Compression Format", 50,
            "F1 P4 W4 D C2 R1 P8 W4 D C2",
            "P0 W4 N15 S2,0,1",
            "T0 X W4",
            "T1 Y4 W7 D C2",
            "T2 Y2 W2 D C2",
            "T3 N0 W4 D C2"),

        "steim1": ("Steim1 Integer Compression Format", 50,
            "F1 P4 W4 D C2 R1 P8 W4 D C2",
            "P0 W4 N15 S2,0,1",
            "T0 X W4",
            "T1 Y4 W7 D C2",
            "T2 Y2 W2 D C2",
            "T3 N0 W4 D C2"),

        "Steim2": ("Steim2 Integer Compression Format", 50,
            "F1 P4 W4 D C2 R1 P8 W4 D C2",
            "P0 W4 N15 S2,0,1",
            "T0 X W4",
            "T1 Y4 W1 D C2",
            "T2 W4 I D2",
            "K0 X D30",
            "K1 N0 D30 C2",
            "K2 Y2 D15 C2",
            "K3 Y3 D10 C2",
            "T3 W4 I D2",
            "K0 Y5 D6 C2",
            "K1 Y6 D5 C2",
            "K2 X D2 Y7 D4 C2",
            "K3 X D30"),

        "steim2": ("Steim2 Integer Compression Format", 50,
            "F1 P4 W4 D C2 R1 P8 W4 D C2",
            "P0 W4 N15 S2,0,1",
            "T0 X W4",
            "T1 Y4 W1 D C2",
            "T2 W4 I D2",
            "K0 X D30",
            "K1 N0 D30 C2",
            "K2 Y2 D15 C2",
            "K3 Y3 D10 C2",
            "T3 W4 I D2",
            "K0 Y5 D6 C2",
            "K1 Y6 D5 C2",
            "K2 X D2 Y7 D4 C2",
            "K3 X D30"),

        "mseed10": ("Steim1 Integer Compression Format", 50,
            "F1 P4 W4 D C2 R1 P8 W4 D C2",
            "P0 W4 N15 S2,0,1",
            "T0 X W4",
            "T1 Y4 W7 D C2",
            "T2 Y2 W2 D C2",
            "T3 N0 W4 D C2"),

        "mseed11": ("Steim2 Integer Compression Format", 50,
            "F1 P4 W4 D C2 R1 P8 W4 D C2",
            "P0 W4 N15 S2,0,1",
            "T0 X W4",
            "T1 Y4 W1 D C2",
            "T2 W4 I D2",
            "K0 X D30",
            "K1 N0 D30 C2",
            "K2 Y2 D15 C2",
            "K3 Y3 D10 C2",
            "T3 W4 I D2",
            "K0 Y5 D6 C2",
            "K1 Y6 D5 C2",
            "K2 X D2 Y7 D4 C2",
            "K3 X D30"),

        "mseed13": ("GEOSCOPE Multiplexed Format 16 bit gain ranged, 3 bit exponent", 1,
            "M0",
            "W2 D0-11 A-2048",
            "D12-14",
            "E2:0:-1"),

        "mseed14": ("GEOSCOPE Multiplexed Format 16 bit gain ranged, 4 bit exponent", 1,
            "M0",
            "W2 D0-11 A-2048",
            "D12-15",
            "E2:0:-1"),

        "mseed0": ("ASCII console log", 80, ""),

        "ASCII": ("ASCII console log", 80, ""),

        "unknown": ("unknown", 99, "") }

    def __init__(self):
        self.__num = 0
        self.__used = {}
        self.__blk = []

    def lookup(self, name):
        if not name:
            name = "unknown"

        try:
            return self.__used[name]

        except KeyError:
            pass

        self.__num += 1
        k = self.__num
        self.__used[name] = k

        try:
            f = self.__formats[name]

        except KeyError:
            f = (name, 99, "")

        b = _Blockette30(name = f[0], key = k, family = f[1], ddl = f[2:])
        self.__blk.append(b)
        return k

    def output(self, f):
        for b in self.__blk:
            b.output(f)

class _UnitDict(object):
    __units = {
        "COUNTS": "Digital Counts",
        "COUNTS/V": "Counts per Volt",
        "M": "Displacement in Meters",
        "M/S": "Velocity in Meters per Second",
        "M/S**2": "Acceleration in Meters per Second per Second",
        "RAD/S": "Angular Velocity in Radians per Second",
        "V": "Volts",
        "A": "Amperes",
        "PA": "Pascal",
        "C": "Degree Celsius",
        "DEG": "Degree" }

    def __init__(self):
        self.__num = 0
        self.__used = {}
        self.__blk = []

    def lookup(self, name, remark=None):
        if not name:
            name = "unknown"

        try:
            return self.__used[(name, remark)]

        except KeyError:
            pass

        self.__num += 1
        k = self.__num
        self.__used[(name, remark)] = k

        try:
            desc = json.loads(remark)['unit']

        except Exception:
            try:
                desc = self.__units[name]

            except KeyError:
                desc = name

        b = _Blockette34(key = k, name = name, desc = desc)
        self.__blk.append(b)
        return k

    def output(self, f):
        for b in self.__blk:
            b.output(f)

class _CommentDict(object):
    def __init__(self):
        self.__num = 0
        self.__used = {}
        self.__blk = []

    def lookup(self, cclass, comment):
        k = self.__used.get((cclass, comment))
        if k is not None:
            return k

        self.__num += 1
        k = self.__num
        self.__used[(cclass, comment)] = k

        b = _Blockette31(key = k, cclass = cclass, comment = comment)
        self.__blk.append(b)
        return k

    def output(self, f):
        for b in self.__blk:
            b.output(f)

class _GenericAbbreviationDict(object):
    def __init__(self, inventory):
        self.__inventory = inventory
        self.__num = 0
        self.__used_sensor = {}
        self.__used_network = {}
        self.__blk = []   # blk33

    def lookup_sensor(self, name):     # instrument id for blk52
        sensor = self.__inventory.object.get(name)
        if sensor is None:
            raise SEEDError, "unknown sensor: " + name

        desc = sensor.description
        if not desc:
            desc = "unknown"

        k = self.__used_sensor.get(desc)
        if k is not None:
            return k

        self.__num += 1
        k = self.__num
        self.__used_sensor[desc] = k

        self.__blk.append(_Blockette33(k, desc))
        return k

    def lookup_network(self, code, start):    # network id for blk50
        k = self.__used_network.get((code, start))
        if k is not None:
            return k

        self.__num += 1
        k = self.__num
        self.__used_network[(code, start)] = k

        net_tp = self.__inventory.network.get(code)
        if net_tp is None:
            raise SEEDError, "unknown network: %s" % (code,)

        netcfg = net_tp.get(start)
        if net_tp is None:
            raise SEEDError, "unknown network: %s.%s" % \
                (code, start.isoformat())

        self.__blk.append(_Blockette33(k, netcfg.description))
        return k

    def output(self, f):
        for b in self.__blk:
            b.output(f)

class _ResponseContainer(object):
    def __init__(self, fac):
        self.__fac = fac

    def add_sensor(self, name, dev_id, compn):
        (x1, x2, sens, sens_freq) = self.__fac._lookup_sensor(name,
            dev_id, compn)

        if x2 is not None:
            self._add_stage(x1, x2)

        else:
            self._add_stage(x1)

        return (sens, sens_freq)

    def add_analogue_paz(self, name):
        (x1, x2, gain) = self.__fac._lookup_analogue_paz(name)

        self._add_stage(x1, x2)
        return gain

    def add_analogue_iir(self, name):
        (x1, x2, gain) = self.__fac._lookup_analogue_iir(name)

        self._add_stage(x1, x2)
        return gain

    def add_analogue_fap(self, name):
        (x1, x2, gain) = self.__fac._lookup_analogue_fap(name)

        self._add_stage(x1, x2)
        return gain

    def add_digitizer(self, name, dev_id, compn, sample_rate, sample_rate_div):
        (x1, x2, x3, rate, gain) = self.__fac._lookup_digitizer(name,
            dev_id, compn, sample_rate, sample_rate_div)

        self._add_stage(x1, x2, x3)
        return (rate, gain)

    def add_digital_paz(self, name, input_rate):
        (x1, x2, x3, rate, gain) = self.__fac._lookup_digital_paz(name, input_rate)

        self._add_stage(x1, x2, x3)
        return (rate, gain)

    def add_digital_iir(self, name, input_rate):
        (x1, x2, x3, rate, gain) = self.__fac._lookup_digital_iir(name, input_rate)

        self._add_stage(x1, x2, x3)
        return (rate, gain)

    def add_fir(self, name, input_rate):
        (x1, x2, x3, rate, gain) = self.__fac._lookup_fir(name, input_rate)

        self._add_stage(x1, x2, x3)
        return (rate, gain)

class _Response4xContainer(_ResponseContainer):
    def __init__(self, fac):
        _ResponseContainer.__init__(self, fac)
        self.__blk = _Blockette60(1)

    def _add_stage(self, *blkref):
        self.__blk.add_stage(*blkref)

    def output(self, f):
        self.__blk.output(f)

class _Response5xContainer(_ResponseContainer):
    def __init__(self, fac):
        _ResponseContainer.__init__(self, fac)
        self.__stage = 1
        self.__blk = []

    def _add_stage(self, *blk):
        for b in blk:
            b.set_stage(self.__stage)
            self.__blk.append(b)

        self.__stage += 1

    def output(self, f):
        for b in self.__blk:
            b.output(f)

class _Response4xFactory(object):
    def __init__(self, inventory, unit_dict):
        self.__inventory = inventory
        self.__unit_dict = unit_dict
        self.__num = 0
        self.__used_sensor = {}
        self.__used_sensor_calib = {}
        self.__used_digitizer = {}
        self.__used_digitizer_calib = {}
        self.__used_analogue_paz = {}
        self.__used_analogue_iir = {}
        self.__used_analogue_fap = {}
        self.__used_digital_paz = {}
        self.__used_digital_iir = {}
        self.__used_fir = {}
        self.__used_fir_deci = {}
        self.__blk41 = []
        self.__blk42 = []
        self.__blk43 = []
        self.__blk44 = []
        self.__blk45 = []
        self.__blk47 = []
        self.__blk48 = []

    def new_response(self):
        return _Response4xContainer(self)

    def _lookup_sensor(self, name, dev_id, compn):
        sensor = self.__inventory.object.get(name)
        if sensor is None:
            raise SEEDError, "unknown sensor: " + name

        resp = self.__inventory.object.get(sensor.response)
        if resp is None:
            raise SEEDError, "cannot find response for sensor " + sensor.name

        k1 = self.__used_sensor.get(name)
        if k1 is None:
            unit = None
            try:
                unit = sensor.unit

            except AttributeError:
                pass

            if unit:
                input_units = self.__unit_dict.lookup(unit, sensor.remark)

            elif _is_paz_response(resp) and resp.numberOfZeros == 0:
                input_units = self.__unit_dict.lookup("M/S**2")

            else:
                input_units = self.__unit_dict.lookup("M/S")

            k1 = self.__num + 1

            if _is_paz_response(resp):
                if resp.type != "A" and resp.type != "B":
                    raise SEEDError, "invalid PAZ response type of " + resp.name

                b1 = _Blockette43(key = k1,
                    name = "RS" + name,
                    type = resp.type,
                    input_units = input_units,
                    output_units = self.__unit_dict.lookup("V"),
                    norm_fac = resp.normalizationFactor,
                    norm_freq = resp.normalizationFrequency,
                    nzeros = resp.numberOfZeros,
                    zeros = resp.zeros,
                    npoles = resp.numberOfPoles,
                    poles = resp.poles)

                self.__blk43.append(b1)

            elif _is_iir_response(resp):
                if resp.type != "A" and resp.type != "B":
                    raise SEEDError, "invalid IIR response type of " + resp.name

                b1 = _Blockette44(key = k1,
                    name = "RS" + name,
                    type = resp.type,
                    input_units = input_units,
                    output_units = self.__unit_dict.lookup("V"),
                    n_numerators = resp.numberOfNumerators,
                    numerators = resp.numerators,
                    n_denominators = resp.numberOfDenominators,
                    denominators = resp.denominators)

                self.__blk44.append(b1)

            elif _is_poly_response(resp):
                b1 = _Blockette42(key = k1,
                    name = "RS" + name,
                    input_units = input_units,
                    output_units = self.__unit_dict.lookup("V"),
                    freq_unit = resp.frequencyUnit,
                    low_freq = sensor.lowFrequency,
                    high_freq = sensor.highFrequency,
                    approx_type = resp.approximationType,
                    approx_lower_bound = resp.approximationLowerBound,
                    approx_upper_bound = resp.approximationUpperBound,
                    approx_error = resp.approximationError,
                    ncoeff = resp.numberOfCoefficients,
                    coeff = resp.coefficients)

                self.__blk42.append(b1)

            elif _is_fap_response(resp):
                b1 = _Blockette45(key = k1,
                    name = "RS" + name,
                    input_units = input_units,
                    output_units = self.__unit_dict.lookup("V"),
                    ntuples = resp.numberOfTuples,
                    tuples = resp.tuples)

                self.__blk45.append(b1)

            else:
                raise SEEDError, "unknown response type of sensor " + sensor.name

            self.__num += 1
            self.__used_sensor[name] = k1

        try:
            calib = sensor.calibration[dev_id][compn]

        except KeyError:
            calib = None

        if calib is not None and len(calib) > 0:
            calib_list = calib.items()
            calib_list.sort()
            resp_name = "GS" + sensor.name + "_" + dev_id
            gain = calib_list[-1][1].gain
        else:
            calib_list = []
            resp_name = "GS" + sensor.name
            gain = resp.gain
            dev_id = None
            compn = None

        if gain == 0.0 or gain is None or resp.gainFrequency is None:
            return (k1, None, 1.0, 0.0)

        k2 = self.__used_sensor_calib.get((name, dev_id, compn))
        if k2 is not None:
            return (k1, k2, gain, resp.gainFrequency)

        k2 = self.__num + 1

        b2 = _Blockette48(key = k2,
            name = resp_name,
            gain = gain,
            gain_freq = resp.gainFrequency) #,
            #calib_list = calib_list)

        self.__blk48.append(b2)
        self.__num += 1
        self.__used_sensor_calib[(name, dev_id, compn)] = k2
        return (k1, k2, gain, resp.gainFrequency)

    def _lookup_analogue_paz(self, name):
        resp_paz = self.__inventory.object.get(name)
        if resp_paz is None:
            raise SEEDError, "unknown PAZ response: " + name

        k = self.__used_analogue_paz.get(name)
        if k is not None:
            (k1, k2) = k
            return (k1, k2, resp_paz.gain)

        #if resp_paz.deci_fac is not None:
        #    raise SEEDError, "expected analogue response, found digital"

        if resp_paz.type != "A" and resp_paz.type != "B":
            raise SEEDError, "invalid PAZ response type of " + resp_paz.name

        k1 = self.__num + 1
        k2 = self.__num + 2

        b1 = _Blockette43(key = k1,
            name = "RA" + name,
            type = resp_paz.type,
            input_units = self.__unit_dict.lookup("V"),
            output_units = self.__unit_dict.lookup("V"),
            norm_fac = resp_paz.normalizationFactor,
            norm_freq = resp_paz.normalizationFrequency,
            nzeros = resp_paz.numberOfZeros,
            zeros = resp_paz.zeros,
            npoles = resp_paz.numberOfPoles,
            poles = resp_paz.poles)

        b2 = _Blockette48(key = k2,
            name = "GA" + name,
            gain = resp_paz.gain,
            gain_freq = resp_paz.gainFrequency)

        self.__blk43.append(b1)
        self.__blk48.append(b2)
        self.__num += 2
        self.__used_analogue_paz[name] = (k1, k2)
        return (k1, k2, resp_paz.gain)

    def _lookup_analogue_iir(self, name):
        resp_iir = self.__inventory.object.get(name)
        if resp_iir is None:
            raise SEEDError, "unknown IIR response: " + name

        k = self.__used_analogue_iir.get(name)
        if k is not None:
            (k1, k2) = k
            return (k1, k2, resp_iir.gain)

        #if resp_iir.deci_fac is not None:
        #    raise SEEDError, "expected analogue response, found digital"

        if resp_iir.type != "A" and resp_iir.type != "B":
            raise SEEDError, "invalid IIR response type of " + resp_iir.name

        k1 = self.__num + 1
        k2 = self.__num + 2

        b1 = _Blockette44(key = k1,
            name = "RA" + name,
            type = resp_iir.type,
            input_units = self.__unit_dict.lookup("V"),
            output_units = self.__unit_dict.lookup("V"),
            n_numerators = resp_iir.numberOfNumerators,
            numerators = resp_iir.numerators,
            n_denominators = resp_iir.numberOfDenominators,
            denominators = resp_iir.denominators)

        b2 = _Blockette48(key = k2,
            name = "GA" + name,
            gain = resp_iir.gain,
            gain_freq = resp_iir.gainFrequency)

        self.__blk44.append(b1)
        self.__blk48.append(b2)
        self.__num += 2
        self.__used_analogue_iir[name] = (k1, k2)
        return (k1, k2, resp_iir.gain)

    def _lookup_analogue_fap(self, name):
        resp_fap = self.__inventory.object.get(name)
        if resp_fap is None:
            raise SEEDError, "unknown FAP response: " + name

        k = self.__used_analogue_fap.get(name)
        if k is not None:
            (k1, k2) = k
            return (k1, k2, resp_fap.gain)

        k1 = self.__num + 1
        k2 = self.__num + 2

        b1 = _Blockette45(key = k1,
            name = "RA" + name,
            input_units = self.__unit_dict.lookup("V"),
            output_units = self.__unit_dict.lookup("V"),
            ntuples = resp_fap.numberOfTuples,
            tuples = resp_fap.tuples)

        b2 = _Blockette48(key = k2,
            name = "GA" + name,
            gain = resp_fap.gain,
            gain_freq = resp_fap.gainFrequency)

        self.__blk45.append(b1)
        self.__blk48.append(b2)
        self.__num += 2
        self.__used_analogue_fap[name] = (k1, k2)
        return (k1, k2, resp_fap.gain)

    def _lookup_digitizer(self, name, dev_id, compn, sample_rate, sample_rate_div):
        digi = self.__inventory.object.get(name)
        if digi is None:
            raise SEEDError, "unknown datalogger: " + name

        input_rate = float(sample_rate) / float(sample_rate_div)

        try:
            stream_deci = digi.decimation[sample_rate][sample_rate_div]
            if stream_deci.digitalFilterChain and \
                len(stream_deci.digitalFilterChain) > 0:
                for f in stream_deci.digitalFilterChain.split():
                    obj = self.__inventory.object[f]
                    input_rate *= (obj.decimationFactor or 1)

        except KeyError:
            pass

        k = self.__used_digitizer.get((name, input_rate))
        if k is None:
            k1 = self.__num + 1
            k2 = self.__num + 2

            b1 = _Blockette44(key = k1,
                name = "RL" + name,
                type = "D",
                input_units = self.__unit_dict.lookup("V"),
                output_units = self.__unit_dict.lookup("COUNTS"))

            b2 = _Blockette47(key = k2,
                name = "DL" + name,
                input_rate = input_rate,
                deci_fac = 1,
                deci_offset = 0,
                delay = 0,
                correction = 0)

            self.__blk44.append(b1)
            self.__blk47.append(b2)
            self.__num += 2
            self.__used_digitizer[(name, input_rate)] = (k1, k2)
        else:
            (k1, k2) = k

        try:
            calib = digi.calibration[dev_id][compn]

        except KeyError:
            calib = None

        if calib is not None and len(calib) > 0:
            calib_list = calib.items()
            calib_list.sort()
            resp_name = "GL" + digi.name + "_" + dev_id
            gain = calib_list[-1][1].gain
        else:
            calib_list = []
            resp_name = "GL" + digi.name
            gain = digi.gain
            dev_id = None
            compn = None

        k3 = self.__used_digitizer_calib.get((name, dev_id, compn))
        if k3 is not None:
            return (k1, k2, k3, input_rate, gain)

        k3 = self.__num + 1

        b3 = _Blockette48(key = k3,
            name = resp_name,
            gain = gain,
            gain_freq = 0) #,
            #calib_list = calib_list)

        self.__blk48.append(b3)
        self.__num += 1
        self.__used_digitizer_calib[(name, dev_id, compn)] = k3
        return (k1, k2, k3, input_rate, gain)

    def _lookup_digital_paz(self, name, input_rate):
        resp_paz = self.__inventory.object.get(name)
        if resp_paz is None:
            raise SEEDError, "unknown PAZ response: " + name

        deci_fac = resp_paz.decimationFactor or 1
        delay = (resp_paz.delay or 0.0) / input_rate
        correction = (resp_paz.correction or 0.0) / input_rate
        gain = resp_paz.gain or 1.0
        gain_freq = resp_paz.gainFrequency or 0.0

        k = self.__used_digital_paz.get((name, input_rate))
        if k is not None:
            (k1, k2, k3) = k
            return (k1, k2, k3, input_rate / deci_fac, resp_paz.gain)

        #if resp_paz.deci_fac is None:
        #    raise SEEDError, "expected digital response, found analogue"

        if resp_paz.type != "D":
            raise SEEDError, "invalid PAZ response type of " + resp_paz.name

        k1 = self.__num + 1
        k2 = self.__num + 2
        k3 = self.__num + 3

        b1 = _Blockette43(key = k1,
            name = "RD" + name,
            type = "D",
            input_units = self.__unit_dict.lookup("COUNTS"),
            output_units = self.__unit_dict.lookup("COUNTS"),
            norm_fac = resp_paz.normalizationFactor,
            norm_freq = resp_paz.normalizationFrequency,
            nzeros = resp_paz.numberOfZeros,
            zeros = resp_paz.zeros,
            npoles = resp_paz.numberOfPoles,
            poles = resp_paz.poles)

        b2 = _Blockette47(key = k2,
            name = "DD" + name,
            input_rate = input_rate,
            deci_fac = deci_fac,
            deci_offset = 0,
            delay = delay,
            correction = correction)

        b3 = _Blockette48(key = k3,
            name = "GD" + name,
            gain = gain,
            gain_freq = gain_freq)

        self.__blk43.append(b1)
        self.__blk47.append(b2)
        self.__blk48.append(b3)
        self.__num += 3
        self.__used_digital_paz[(name, input_rate)] = (k1, k2, k3)
        return (k1, k2, k3, input_rate / deci_fac, resp_paz.gain)

    def _lookup_digital_iir(self, name, input_rate):
        resp_iir = self.__inventory.object.get(name)
        if resp_iir is None:
            raise SEEDError, "unknown IIR response: " + name

        deci_fac = resp_iir.decimationFactor or 1
        delay = (resp_iir.delay or 0.0) / input_rate
        correction = (resp_iir.correction or 0.0) / input_rate
        gain = resp_iir.gain or 1.0
        gain_freq = resp_iir.gainFrequency or 0.0

        k = self.__used_digital_iir.get((name, input_rate))
        if k is not None:
            (k1, k2, k3) = k
            return (k1, k2, k3, input_rate / deci_fac, resp_iir.gain)

        #if resp_iir.deci_fac is None:
        #    raise SEEDError, "expected digital response, found analogue"

        if resp_iir.type != "D":
            raise SEEDError, "invalid IIR response type of " + resp_iir.name

        k1 = self.__num + 1
        k2 = self.__num + 2
        k3 = self.__num + 3

        b1 = _Blockette44(key = k1,
            name = "RD" + name,
            type = "D",
            input_units = self.__unit_dict.lookup("COUNTS"),
            output_units = self.__unit_dict.lookup("COUNTS"),
            n_numerators = resp_iir.numberOfNumerators,
            numerators = resp_iir.numerators,
            n_denominators = resp_iir.numberOfDenominators,
            denominators = resp_iir.denominators)

        b2 = _Blockette47(key = k2,
            name = "DD" + name,
            input_rate = input_rate,
            deci_fac = deci_fac,
            deci_offset = 0,
            delay = delay,
            correction = correction)

        b3 = _Blockette48(key = k3,
            name = "GD" + name,
            gain = gain,
            gain_freq = gain_freq)

        self.__blk44.append(b1)
        self.__blk47.append(b2)
        self.__blk48.append(b3)
        self.__num += 3
        self.__used_digital_iir[(name, input_rate)] = (k1, k2, k3)
        return (k1, k2, k3, input_rate / deci_fac, resp_iir.gain)

    def _lookup_fir(self, name, input_rate):
        resp_fir = self.__inventory.object.get(name)
        if resp_fir is None:
            raise SEEDError, "unknown FIR response: " + name

        deci_fac = resp_fir.decimationFactor or 1
        delay = (resp_fir.delay or 0.0) / input_rate
        correction = (resp_fir.correction or 0.0) / input_rate
        gain = resp_fir.gain or 1.0
        gain_freq = resp_fir.gainFrequency or 0.0

        k = self.__used_fir.get(name)
        if k is None:
            k1 = self.__num + 1
            k3 = self.__num + 2

            b1 = _Blockette41(key = k1,
                name = "RF" + name,
                symmetry = resp_fir.symmetry,
                input_units = self.__unit_dict.lookup("COUNTS"),
                output_units = self.__unit_dict.lookup("COUNTS"),
                ncoeff = resp_fir.numberOfCoefficients,
                coeff = resp_fir.coefficients)

            b3 = _Blockette48(key = k3,
                name = "GF" + name,
                gain = gain,
                gain_freq = gain_freq)

            self.__blk41.append(b1)
            self.__blk48.append(b3)
            self.__num += 2
            self.__used_fir[name] = (k1, k3)
        else:
            (k1, k3) = k

        k2 = self.__used_fir_deci.get((name, input_rate))
        if k2 is None:
            k2 = self.__num + 1
            b2 = _Blockette47(key = k2,
                name = "DF" + name + "_" + str(input_rate).replace(".", "_"),
                input_rate = input_rate,
                deci_fac = deci_fac,
                deci_offset = 0,
                delay = delay,
                correction = correction)

            self.__blk47.append(b2)
            self.__num += 1
            self.__used_fir_deci[(name, input_rate)] = k2

        return (k1, k2, k3, input_rate / deci_fac, resp_fir.gain)

    def output(self, f):
        for b in self.__blk41:
            b.output(f)

        for b in self.__blk42:
            b.output(f)

        for b in self.__blk43:
            b.output(f)

        for b in self.__blk44:
            b.output(f)

        for b in self.__blk45:
            b.output(f)

        for b in self.__blk47:
            b.output(f)

        for b in self.__blk48:
            b.output(f)

class _Response5xFactory(object):
    def __init__(self, inventory, unit_dict):
        self.__inventory = inventory
        self.__unit_dict = unit_dict

    def new_response(self):
        return _Response5xContainer(self)

    def _lookup_sensor(self, name, dev_id, compn):
        sensor = self.__inventory.object.get(name)
        if sensor is None:
            raise SEEDError, "unknown sensor: " + name

        resp = self.__inventory.object.get(sensor.response)
        if resp is None:
            raise SEEDError, "cannot find response for sensor " + sensor.name

        unit = None
        try:
            unit = sensor.unit

        except AttributeError:
            pass

        if unit:
            input_units = self.__unit_dict.lookup(unit, sensor.remark)

        elif _is_paz_response(resp) and resp.numberOfZeros == 0:
            input_units = self.__unit_dict.lookup("M/S**2")

        else:
            input_units = self.__unit_dict.lookup("M/S")

        if _is_paz_response(resp):
            if resp.type != "A" and resp.type != "B":
                raise SEEDError, "invalid PAZ response type of " + resp.name

            b1 = _Blockette53(type = resp.type,
                input_units = input_units,
                output_units = self.__unit_dict.lookup("V"),
                norm_fac = resp.normalizationFactor,
                norm_freq = resp.normalizationFrequency,
                nzeros = resp.numberOfZeros,
                zeros = resp.zeros,
                npoles = resp.numberOfPoles,
                poles = resp.poles)

        elif _is_iir_response(resp):
            if resp.type != "A" and resp.type != "B":
                raise SEEDError, "invalid IIR response type of " + resp.name

            b1 = _Blockette54(type = resp.type,
                input_units = input_units,
                output_units = self.__unit_dict.lookup("V"),
                n_numerators = resp.numberOfNumerators,
                numerators = resp.numerators,
                n_denominators = resp.numberOfDenominators,
                denominators = resp.denominators)

            self.__blk54.append(b1)

        elif _is_poly_response(resp):
            b1 = _Blockette62(input_units = input_units,
                output_units = self.__unit_dict.lookup("V"),
                freq_unit = resp.frequencyUnit,
                low_freq = sensor.lowFrequency,
                high_freq = sensor.highFrequency,
                approx_type = resp.approximationType,
                approx_lower_bound = resp.approximationLowerBound,
                approx_upper_bound = resp.approximationUpperBound,
                approx_error = resp.approximationError,
                ncoeff = resp.numberOfCoefficients,
                coeff = resp.coefficients)

        elif _is_fap_response(resp):
            b1 = _Blockette55(input_units = input_units,
                output_units = self.__unit_dict.lookup("V"),
                ntuples = resp.numberOfTuples,
                tuples = resp.tuples)

        else:
            raise SEEDError, "unknown response type of sensor " + sensor.name

        try:
            calib = sensor.calibration[dev_id][compn]

        except KeyError:
            calib = None

        if calib is not None and len(calib) > 0:
            calib_list = calib.items()
            calib_list.sort()
            gain = calib_list[-1][1].gain
        else:
            calib_list = []
            gain = resp.gain
            dev_id = None
            compn = None

        if gain == 0.0 or gain is None or resp.gainFrequency is None:
            return (b1, None, 1.0, 0.0)

        b2 = _Blockette58(gain = gain,
            gain_freq = resp.gainFrequency) #,
            #calib_list = calib_list)

        return (b1, b2, gain, resp.gainFrequency)

    def _lookup_analogue_paz(self, name):
        resp_paz = self.__inventory.object.get(name)
        if resp_paz is None:
            raise SEEDError, "unknown PAZ response: " + name

        if resp_paz.type != "A" and resp_paz.type != "B":
            raise SEEDError, "invalid PAZ response type of " + resp_paz.name

        b1 = _Blockette53(type = resp_paz.type,
            input_units = self.__unit_dict.lookup("V"),
            output_units = self.__unit_dict.lookup("V"),
            norm_fac = resp_paz.normalizationFactor,
            norm_freq = resp_paz.normalizationFrequency,
            nzeros = resp_paz.numberOfZeros,
            zeros = resp_paz.zeros,
            npoles = resp_paz.numberOfPoles,
            poles = resp_paz.poles)

        b2 = _Blockette58(gain = resp_paz.gain,
            gain_freq = resp_paz.gainFrequency)

        return (b1, b2, resp_paz.gain)

    def _lookup_analogue_iir(self, name):
        resp_iir = self.__inventory.object.get(name)
        if resp_iir is None:
            raise SEEDError, "unknown IIR response: " + name

        if resp_iir.type != "A" and resp_iir.type != "B":
            raise SEEDError, "invalid IIR response type of " + resp_iir.name

        b1 = _Blockette54(type = resp_iir.type,
            input_units = self.__unit_dict.lookup("V"),
            output_units = self.__unit_dict.lookup("V"),
            n_numerators = resp_iir.numberOfNumerators,
            numerators = resp_iir.numerators,
            n_denominators = resp_iir.numberOfDenominators,
            denominators = resp_iir.denominators)

        b2 = _Blockette58(gain = resp_iir.gain,
            gain_freq = resp_iir.gainFrequency)

        return (b1, b2, resp_iir.gain)

    def _lookup_analogue_fap(self, name):
        resp_fap = self.__inventory.object.get(name)
        if resp_fap is None:
            raise SEEDError, "unknown FAP response: " + name

        gain = resp_fap.gain
        if gain is None: gain = 1.0

        b1 = _Blockette55(input_units = self.__unit_dict.lookup("V"),
            output_units = self.__unit_dict.lookup("V"),
            ntuples = resp_fap.numberOfTuples, tuples = resp_fap.tuples)

        b2 = _Blockette58(gain = gain,
            gain_freq = resp_fap.gainFrequency)

        return (b1, b2, gain)

    def _lookup_digitizer(self, name, dev_id, compn, sample_rate, sample_rate_div):
        digi = self.__inventory.object.get(name)
        if digi is None:
            raise SEEDError, "unknown datalogger: " + name

        input_rate = float(sample_rate) / float(sample_rate_div)

        try:
            stream_deci = digi.decimation[sample_rate][sample_rate_div]
            if stream_deci.digitalFilterChain and \
                len(stream_deci.digitalFilterChain) > 0:
                for f in stream_deci.digitalFilterChain.split():
                    obj = self.__inventory.object[f]
                    input_rate *= (obj.decimationFactor or 1)

        except KeyError:
            pass

        b1 = _Blockette54(type = "D",
            input_units = self.__unit_dict.lookup("V"),
            output_units = self.__unit_dict.lookup("COUNTS"))

        b2 = _Blockette57(input_rate = input_rate,
            deci_fac = 1,
            deci_offset = 0,
            delay = 0,
            correction = 0)

        try:
            calib = digi.calibration[dev_id][compn]

        except KeyError:
            calib = None

        if calib is not None and len(calib) > 0:
            calib_list = calib.items()
            calib_list.sort()
            gain = calib_list[-1][1].gain
        else:
            calib_list = []
            gain = digi.gain
            dev_id = None
            compn = None

        b3 = _Blockette58(gain = gain,
            gain_freq = 0) #,
            #calib_list = calib_list)

        return (b1, b2, b3, input_rate, gain)

    def _lookup_digital_paz(self, name, input_rate):
        resp_paz = self.__inventory.object.get(name)
        if resp_paz is None:
            raise SEEDError, "unknown PAZ response: " + name

        if resp_paz.type != "D":
            raise SEEDError, "invalid PAZ response type of " + resp_paz.name

        deci_fac = resp_paz.decimationFactor or 1
        delay = (resp_paz.delay or 0.0) / input_rate
        correction = (resp_paz.correction or 0.0) / input_rate
        gain = resp_paz.gain or 1.0
        gain_freq = resp_paz.gainFrequency or 0.0

        b1 = _Blockette53(type = "D",
            input_units = self.__unit_dict.lookup("COUNTS"),
            output_units = self.__unit_dict.lookup("COUNTS"),
            norm_fac = resp_paz.normalizationFactor,
            norm_freq = resp_paz.normalizationFrequency,
            nzeros = resp_paz.numberOfZeros,
            zeros = resp_paz.zeros,
            npoles = resp_paz.numberOfPoles,
            poles = resp_paz.poles)

        b2 = _Blockette57(input_rate = input_rate,
            deci_fac = deci_fac,
            deci_offset = 0,
            delay = delay,
            correction = correction)

        b3 = _Blockette58(gain = gain,
            gain_freq = gain_freq)

        return (b1, b2, b3, input_rate / deci_fac, resp_paz.gain)

    def _lookup_digital_iir(self, name, input_rate):
        resp_iir = self.__inventory.object.get(name)
        if resp_iir is None:
            raise SEEDError, "unknown IIR response: " + name

        if resp_iir.type != "D":
            raise SEEDError, "invalid IIR response type of " + resp_iir.name

        deci_fac = resp_iir.decimationFactor or 1
        delay = (resp_iir.delay or 0.0) / input_rate
        correction = (resp_iir.correction or 0.0) / input_rate
        gain = resp_iir.gain or 1.0
        gain_freq = resp_iir.gainFrequency or 0.0

        b1 = _Blockette54(type = "D",
            input_units = self.__unit_dict.lookup("COUNTS"),
            output_units = self.__unit_dict.lookup("COUNTS"),
            n_numerators = resp_iir.numberOfNumerators,
            numerators = resp_iir.numerators,
            n_denominators = resp_iir.numberOfDenominators,
            denominators = resp_iir.denominators)

        b2 = _Blockette57(input_rate = input_rate,
            deci_fac = deci_fac,
            deci_offset = 0,
            delay = delay,
            correction = correction)

        b3 = _Blockette58(gain = gain,
            gain_freq = gain_freq)

        return (b1, b2, b3, input_rate / deci_fac, resp_iir.gain)

    def _lookup_fir(self, name, input_rate):
        resp_fir = self.__inventory.object.get(name)
        if resp_fir is None:
            raise SEEDError, "unknown FIR response: " + name

        deci_fac = resp_fir.decimationFactor or 1
        delay = (resp_fir.delay or 0.0) / input_rate
        correction = (resp_fir.correction or 0.0) / input_rate
        gain = resp_fir.gain or 1.0
        gain_freq = resp_fir.gainFrequency or 0.0

        b1 = _Blockette61(name = "RF" + name,
            symmetry = resp_fir.symmetry,
            input_units = self.__unit_dict.lookup("COUNTS"),
            output_units = self.__unit_dict.lookup("COUNTS"),
            ncoeff = resp_fir.numberOfCoefficients,
            coeff = resp_fir.coefficients)

        b2 = _Blockette57(input_rate = input_rate,
            deci_fac = deci_fac,
            deci_offset = 0,
            delay = delay,
            correction = correction)

        b3 = _Blockette58(gain = gain,
            gain_freq = gain_freq)

        return (b1, b2, b3, input_rate / deci_fac, resp_fir.gain)

    def output(self, f):
        pass

class _Channel(object):
    def __init__(self, inventory, strmcfg, format_dict, unit_dict,
        comment_dict, gen_dict, resp_container):

        loccfg = strmcfg.mySensorLocation
        statcfg = loccfg.myStation
        netcfg = statcfg.myNetwork

        self.__id = (loccfg.code, strmcfg.code, strmcfg.start)
        self.__resp_container = resp_container
        self.__comment_dict = comment_dict
        self.__comment_blk = []

        sensor = inventory.object.get(strmcfg.sensor)
        if sensor is None:
            raise SEEDError, "unknown sensor: " + strmcfg.sensor

        resp = inventory.object.get(sensor.response)
        if resp is None:
            raise SEEDError, "cannot find response for sensor " + sensor.name

        digi = inventory.object.get(strmcfg.datalogger)
        #if digi is None:
        #    raise SEEDError, "unknown datalogger referenced in channel %s: %s" % (strmcfg.code, strmcfg.datalogger)

        stream_deci = None
        if digi:
            try:
                stream_deci = digi.decimation[strmcfg.sampleRateNumerator][strmcfg.sampleRateDenominator]

            except KeyError:
                raise SEEDError, "cannot find filter chain for stream " + \
                    str(strmcfg.sampleRateNumerator) + "/" + \
                    str(strmcfg.sampleRateDenominator) + " of datalogger " + \
                    digi.name

        unit = None
        try:
            unit = sensor.unit

        except AttributeError:
            pass

        if unit:
            signal_units = unit_dict.lookup(unit, sensor.remark)

        elif _is_paz_response(resp) and resp.numberOfZeros == 0:
            signal_units = unit_dict.lookup("M/S**2")

        else:
            signal_units = unit_dict.lookup("M/S")

        if strmcfg.sampleRateNumerator == 0 or \
            strmcfg.sampleRateDenominator == 0:
            raise SEEDError, "invalid sample rate %d/%d" % \
                (strmcfg.sampleRateNumerator, strmcfg.sampleRateDenominator)

        sample_rate = float(strmcfg.sampleRateNumerator) / \
            float(strmcfg.sampleRateDenominator)

        clock_drift = float(0)
        if digi:
            if digi.maxClockDrift is not None:
                clock_drift = digi.maxClockDrift / sample_rate

        self.__chan_blk = _Blockette52(loc_id = loccfg.code,
            chan_id = strmcfg.code,
            instr_id = gen_dict.lookup_sensor(strmcfg.sensor),
            comment = "",
            signal_units = signal_units,
            calibration_units = 0,
            latitude = loccfg.latitude,
            longitude = loccfg.longitude,
            elevation = loccfg.elevation,
            local_depth = strmcfg.depth,
            azimuth = strmcfg.azimuth,
            dip = strmcfg.dip,
            data_format = format_dict.lookup(strmcfg.format),
            record_length = 12,
            sample_rate = sample_rate,
            clock_drift = clock_drift,
            flags = strmcfg.flags,
            start_date = strmcfg.start,
            end_date = strmcfg.end)

        (sens, sens_freq) = resp_container.add_sensor(strmcfg.sensor,
            strmcfg.sensorSerialNumber, strmcfg.sensorChannel)

        if stream_deci:
            if stream_deci.analogueFilterChain:
                if len(stream_deci.analogueFilterChain) > 0:
                    for f in stream_deci.analogueFilterChain.split():
                        obj = inventory.object[f]
                        if _is_paz_response(obj):
                            gain = resp_container.add_analogue_paz(f)
                            sens *= gain
                        elif _is_iir_response(obj):
                            gain = resp_container.add_analogue_iir(f)
                            sens *= gain
                        elif _is_fap_response(obj):
                            gain = resp_container.add_analogue_fap(f)
                            sens *= gain
                        else:
                            raise SEEDError, "invalid filter type: %s (%s)" % (f, obj.name)

            (rate, gain) = resp_container.add_digitizer(strmcfg.datalogger,
                strmcfg.dataloggerSerialNumber, strmcfg.dataloggerChannel,
                strmcfg.sampleRateNumerator, strmcfg.sampleRateDenominator)

            sens *= gain

            if stream_deci.digitalFilterChain:
                if len(stream_deci.digitalFilterChain) > 0:
                    for f in stream_deci.digitalFilterChain.split():
                        obj = inventory.object[f]
                        if _is_paz_response(obj):
                            (rate, gain) = resp_container.add_digital_paz(f, rate)
                        elif _is_iir_response(obj):
                            (rate, gain) = resp_container.add_digital_iir(f, rate)
                        elif _is_fir_response(obj):
                            (rate, gain) = resp_container.add_fir(f, rate)
                        else:
                            raise SEEDError, "invalid filter type: %s (%s)" % (f, obj.name)

                        sens *= gain

            if sens_freq > rate / 5:
                sens_freq = rate / 5

        #if sample_rate != rate:
        #    print digi.name, netcfg.code, statcfg.code, strmcfg.code, "expected sample rate", sample_rate, "actual", rate

        if strmcfg.gain is None:
            strmcfg.gain = sens

        if strmcfg.gainFrequency is None:
            strmcfg.gainFrequency = sens_freq

        if _is_poly_response(resp):
            self.__stage0_blk = _Blockette62(input_units = signal_units,
                output_units = unit_dict.lookup("COUNTS"),
                freq_unit = resp.frequencyUnit,
                low_freq = sensor.lowFrequency,
                high_freq = sensor.highFrequency,
                approx_type = resp.approximationType,
                approx_lower_bound = resp.approximationLowerBound,
                approx_upper_bound = resp.approximationUpperBound,
                approx_error = resp.approximationError,
                ncoeff = resp.numberOfCoefficients,
                coeff = resp.coefficients,
                gain = sens)

        else:
            self.__stage0_blk = _Blockette58(gain = strmcfg.gain,
                gain_freq = strmcfg.gainFrequency)

    def __cmp__(self, other):
        if(self.__id < other.__id):
            return -1

        if(self.__id > other.__id):
            return 1

        return 0

    def add_comment(self, com):
        self.__comment_blk.append(_Blockette59(start_time = com.start,
            end_time = com.end,
            comment_key = self.__comment_dict.lookup('C', com.text)))

    def output(self, f, vol_start, vol_end):
        self.__chan_blk.set_vol_span(vol_start, vol_end)
        self.__chan_blk.output(f)
        self.__resp_container.output(f)
        self.__stage0_blk.output(f)

        for b in self.__comment_blk:
            b.output(f)

class _Station(object):
    def __init__(self, inventory, statcfg, format_dict, unit_dict,
        comment_dict, gen_dict, resp_fac):
        self.__inventory = inventory
        self.__statcfg = statcfg
        self.__format_dict = format_dict
        self.__unit_dict = unit_dict
        self.__comment_dict = comment_dict
        self.__gen_dict = gen_dict
        self.__resp_fac = resp_fac
        self.__recno = 0
        self.__id = (statcfg.myNetwork.code, statcfg.myNetwork.start,
          statcfg.code, statcfg.start)
        self.__channel = {}
        self.__comment_blk = []

        site_name = statcfg.description
        if not site_name:
            if statcfg.place and statcfg.country:
                site_name = statcfg.place + ", " + statcfg.country
            else:
                site_name = statcfg.code

        self.__stat_blk = _Blockette50(stat_code = statcfg.code,
            latitude = statcfg.latitude,
            longitude = statcfg.longitude,
            elevation = statcfg.elevation,
            site_name = site_name,
            net_id = gen_dict.lookup_network(statcfg.myNetwork.code,
              statcfg.myNetwork.start),
            net_code = statcfg.myNetwork.code,
            start_date = statcfg.start,
            end_date = statcfg.end)

        for com in statcfg.comment.itervalues():
            self.__comment_blk.append(_Blockette51(start_time = com.start,
                end_time = com.end,
                comment_key = self.__comment_dict.lookup('S', com.text)))

    def __cmp__(self, other):
        if(self.__id < other.__id):
            return -1

        if(self.__id > other.__id):
            return 1

        return 0

    def add_chan(self, strmcfg):
        loccfg = strmcfg.mySensorLocation

        if (loccfg.code, strmcfg.code, strmcfg.start) in self.__channel:
            return

        cha = _Channel(self.__inventory, strmcfg, self.__format_dict,
            self.__unit_dict, self.__comment_dict, self.__gen_dict,
            self.__resp_fac.new_response())

        self.__channel[(loccfg.code, strmcfg.code, strmcfg.start)] = cha

        for com in strmcfg.comment.itervalues():
            cha.add_comment(com)

    def get_id(self):
        return self.__id

    def get_recno(self):
        return self.__recno

    def output(self, f, vol_start, vol_end):
        self.__recno = f.get_recno()
        self.__stat_blk.output(f)
        for b in self.__comment_blk:
            b.output(f)

        chan_list = self.__channel.values()
        chan_list.sort()
        for c in chan_list:
            c.output(f, vol_start, vol_end)

        f.flush()

class _TimeSeries(object):
    def __init__(self, span, net_code, stat_code, loc_id, chan_id,
        start_time, end_time, recno):
        self.__span = span
        self.__net_code = net_code
        self.__stat_code = stat_code
        self.__loc_id = loc_id
        self.__chan_id = chan_id
        self.__start_time = start_time
        self.__start_recno = recno
        self.__end_time = end_time
        self.__end_recno = recno

    def extend(self, start_time, end_time, recno):
        if start_time < self.__start_time:
            self.__start_time = start_time

        if end_time > self.__end_time:
            self.__end_time = end_time

        self.__end_recno = recno
        self.__span.extend(start_time, end_time)

    def get_series_data(self):
        return (self.__net_code, self.__stat_code, self.__loc_id,
          self.__chan_id, self.__start_time, self.__end_time)

    def output(self, f, data_start):
        b = _Blockette74(self.__net_code, self.__stat_code, self.__loc_id,
            self.__chan_id, self.__start_time, self.__start_recno + data_start,
            self.__end_time, self.__end_recno + data_start)

        b.output(f)

class _Timespan(object):
    def __init__(self):
        self.__start_time = None
        self.__end_time = None
        self.__recno = 0
        self.__series = []

    def new_time_series(self, net, sta, loc, cha, start_time, end_time, recno):
        if len(self.__series) == 0:
            self.__start_time = start_time
            self.__end_time = end_time
        else:
            self.extend(start_time, end_time)

        ts = _TimeSeries(self, net, sta, loc, cha, start_time, end_time, recno)
        self.__series.append(ts)
        return ts

    def overlap(self, start_time, end_time):
        return self.__start_time - _min_ts_gap <= start_time <= self.__end_time + _min_ts_gap or \
            self.__start_time - _min_ts_gap <= end_time <= self.__end_time + _min_ts_gap

    def extend(self, start_time, end_time):
        if start_time < self.__start_time:
            self.__start_time = start_time

        if end_time > self.__end_time:
            self.__end_time = end_time

    def get_span_data(self):
        return (self.__start_time, self.__end_time, self.__recno)

    def get_series_data(self):
        return [ s.get_series_data() for s in self.__series ]

    def output_index(self, f, data_start):
        self.__recno = f.get_recno()

        b = _Blockette70("P", self.__start_time, self.__end_time)
        b.output(f)

        for s in self.__series:
            s.output(f, data_start)

        f.flush()

class _WaveformData(object):
    def __init__(self):
        self.__fd = TemporaryFile()
        self.__recno = 0
        self.__cur_rec = None
        self.__cur_series = None
        self.__span = []

    def __get_time_series(self, rec):
        for s in self.__span:
            if s.overlap(rec.begin_time, rec.end_time):
                break
        else:
            s = _Timespan()
            self.__span.append(s)

        return s.new_time_series(rec.net, rec.sta, rec.loc, rec.cha,
            rec.begin_time, rec.end_time, self.__recno)

    def add_data(self, rec):
        if self.__cur_rec is None:
            self.__cur_rec = rec
            self.__cur_series = self.__get_time_series(rec)

            #if rec.encoding != 10 and rec.encoding != 11:
            #    logs.warning("%s %s %s %s cannot merge records with encoding %d" % \
            #        (rec.net, rec.sta, rec.loc, rec.cha, rec.encoding))

            return

        if self.__cur_rec.net == rec.net and self.__cur_rec.sta == rec.sta and \
            self.__cur_rec.loc == rec.loc and self.__cur_rec.cha == rec.cha:

            contiguous = True
            if rec.encoding == 10 or rec.encoding == 11:
                if abs(rec.begin_time - self.__cur_rec.end_time) > _min_data_gap(rec.fsamp):
                    contiguous = False

                if rec.X_minus1 is None:
                    logs.debug("%s %s %s %s X[-1] not defined" %
                        (rec.net, rec.sta, rec.loc, rec.cha))
                    contiguous = False
            else:
                contiguous = False

            if self.__cur_rec.fsamp != rec.fsamp:
                logs.debug("%s %s %s %s sample rate changed from %f to %f" %
                    (rec.net, rec.sta, rec.loc, rec.cha, self.__cur_rec.fsamp,
                    rec.fsamp))
                contiguous = False

            if self.__cur_rec.encoding != rec.encoding:
                logs.debug("%s %s %s %s encoding changed from %d to %d" %
                    (rec.net, rec.sta, rec.loc, rec.cha, self.__cur_rec.encoding,
                    rec.encoding))
                contiguous = False

            if contiguous and self.__cur_rec.Xn != rec.X_minus1:
                logs.debug("%s %s %s %s non-contiguous data: %d != %d" %
                    (rec.net, rec.sta, rec.loc, rec.cha, self.__cur_rec.Xn,
                    rec.X_minus1))
                contiguous = False

            if contiguous and self.__cur_rec.size + rec.nframes * 64 <= (1 << _RECLEN_EXP):
                self.__cur_rec.merge(rec)

            else:
                self.__recno += 1

                if abs(rec.begin_time - self.__cur_rec.end_time) <= _min_ts_gap:
                    self.__cur_series.extend(rec.begin_time, rec.end_time,
                        self.__recno)
                else:
                    self.__cur_series = self.__get_time_series(rec)

                self.__cur_rec.write(self.__fd, _RECLEN_EXP)
                self.__cur_rec = rec

        else:
            self.__recno += 1
            self.__cur_series = self.__get_time_series(rec)
            self.__cur_rec.write(self.__fd, _RECLEN_EXP)
            self.__cur_rec = rec

    def get_series_data(self):
        return sum([ s.get_series_data() for s in self.__span ], [])

    def output_vol(self, f):
        b = _Blockette12()
        for s in self.__span:
            b.add_span(*s.get_span_data())

        b.output(f)

    def output_index(self, f, data_start):
        for s in self.__span:
            s.output_index(f, data_start)

    def output_data(self, fd, data_start):
        if self.__cur_rec is not None:
            self.__cur_rec.write(self.__fd, _RECLEN_EXP)
            self.__cur_rec = None
            self.__cur_series = None

        self.__fd.seek(0)
        #copyfileobj(self.__fd, fd)

        i = 0
        for rec in mseed.Input(self.__fd):
            rec.recno = data_start + i
            rec.write(fd, _RECLEN_EXP)
            i += 1

        self.__fd.close()

class _RecordBuilder(object):
    def __init__(self, type, fd):
        self.__recno = 1
        self.__type = type
        self.__fd = fd
        self.__buf = None

    def flush(self):
        if self.__buf is not None:
            self.__buf += ((1 << _RECLEN_EXP) - len(self.__buf)) * " "
            self.__fd.write(self.__buf)
            self.__buf = None

    def reset(self, type, fd, recno = None):
        self.flush()
        self.__type = type
        self.__fd = fd
        if recno is not None:
            self.__recno = recno

    def get_recno(self):
        return self.__recno

    def write_blk(self, s):
        if self.__buf is None:
            self.__buf = "%06d%c " % (self.__recno, self.__type)
            self.__recno += 1

        b = 0
        while len(s) - b > (1 << _RECLEN_EXP) - len(self.__buf):
            e = b + (1 << _RECLEN_EXP) - len(self.__buf)
            self.__buf += s[b:e]
            self.__fd.write(self.__buf)

            self.__buf = "%06d%c*" % (self.__recno, self.__type)
            self.__recno += 1
            b = e

        self.__buf += s[b:]

        if len(self.__buf) > (1 << _RECLEN_EXP) - 8:
            self.flush()

class SEEDVolume(object):
    def __init__(self, inventory, organization, label, resp_dict=True):
        self.__inventory = inventory
        self.__organization = organization
        self.__label = label or ""
        self.__vol_start_time = datetime.datetime(2100,1,1,0,0,0)
        self.__vol_end_time = datetime.datetime(1971,1,1,0,0,0)
        self.__format_dict = _FormatDict()
        self.__unit_dict = _UnitDict()
        self.__comment_dict = _CommentDict()
        self.__gen_dict = _GenericAbbreviationDict(inventory)
        self.__station = {}
        self.__waveform_data = None

        if resp_dict:
            self.__resp_fac = _Response4xFactory(inventory, self.__unit_dict)

        else:
            self.__resp_fac = _Response5xFactory(inventory, self.__unit_dict)

    def add_chan(self, net_code, stat_code, loc_id, chan_id, start_time, end_time, strict=False):
        found = False

        net_tp = self.__inventory.network.get(net_code)
        if net_tp is not None:
            for netcfg in net_tp.itervalues():
#               if _cmptime(start_time, netcfg.end) <= 0 and \
#                   _cmptime(end_time, netcfg.start) >= 0:

                    sta_tp = netcfg.station.get(stat_code)
                    if sta_tp is not None:
                        for statcfg in sta_tp.itervalues():
#                           if _cmptime(start_time, statcfg.end) <= 0 and \
#                               _cmptime(end_time, statcfg.start) >= 0:

                                sta = self.__station.get((net_code, netcfg.start, stat_code, statcfg.start))
                                if sta is None:
                                    sta = _Station(self.__inventory, statcfg, self.__format_dict,
                                        self.__unit_dict, self.__comment_dict, self.__gen_dict,
                                        self.__resp_fac)
                                    self.__station[(net_code, netcfg.start, stat_code, statcfg.start)] = sta

                                loc_tp = statcfg.sensorLocation.get(loc_id)
                                if loc_tp is not None:
                                    for loccfg in loc_tp.itervalues():
#                                       if _cmptime(start_time, strmcfg.end) <= 0 and \
#                                           _cmptime(end_time, strmcfg.start) >= 0:

                                            strm_tp = loccfg.stream.get(chan_id)
                                            if strm_tp is not None:
                                                for strmcfg in strm_tp.itervalues():
                                                    if _cmptime(start_time, strmcfg.end) <= 0 and \
                                                        _cmptime(end_time, strmcfg.start) >= 0:

                                                        if _cmptime(start_time, self.__vol_start_time) < 0:
                                                            self.__vol_start_time = start_time

                                                        if _cmptime(end_time, self.__vol_end_time) > 0:
                                                            self.__vol_end_time = end_time

                                                        sta.add_chan(strmcfg)
                                                        found = True

        if not found:
            if strict:
                raise SEEDError, "cannot find %s %s %s %s %s %s" % \
                    (net_code, stat_code, loc_id, chan_id, start_time, end_time)
            else:
                logs.warning("cannot find %s %s %s %s %s %s" %
                    (net_code, stat_code, loc_id, chan_id, start_time, end_time))

    def add_data(self, rec):
        if self.__waveform_data is None:
            self.__waveform_data = _WaveformData()

        self.__waveform_data.add_data(rec)

    def __output_vol(self, vol_creat_time, sta_list, rb):
        b1 = _Blockette10(record_length = _RECLEN_EXP,
            start_time = self.__vol_start_time,
            end_time = self.__vol_end_time,
            vol_time = vol_creat_time,
            organization = self.__organization,
            label = self.__label)

        b2 = _Blockette11()

        for sta in sta_list:
            (net_code, net_start, stat_code, stat_start) = sta.get_id()
            b2.add_station(stat_code, sta.get_recno())

        b1.output(rb)
        b2.output(rb)

        if self.__waveform_data is not None:
            self.__waveform_data.output_vol(rb)

        rb.flush()

    def output(self, dest, strict=False):
        vol_creat_time = datetime.datetime.utcnow()

        if self.__waveform_data is not None:
            for (net_code, stat_code, loc_id, chan_id, start_time, end_time) in \
                self.__waveform_data.get_series_data():
                try:
                    self.add_chan(net_code, stat_code, loc_id, chan_id, start_time, end_time, strict)

                except SEEDError as e:
                    if strict:
                        raise SEEDError, "%s.%s.%s.%s.%s: %s" % \
                            (net_code, stat_code, loc_id, chan_id, start_time.isoformat(), e)

                    logs.warning("%s.%s.%s.%s.%s: %s" %
                        (net_code, stat_code, loc_id, chan_id, start_time.isoformat(), e))

        sta_list = self.__station.values()
        sta_list.sort()

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

        rb = _RecordBuilder("V", fd)
        self.__output_vol(vol_creat_time, sta_list, rb)

        rb.reset("A", fd)
        self.__format_dict.output(rb)
        self.__comment_dict.output(rb)
        self.__gen_dict.output(rb)
        self.__unit_dict.output(rb)
        self.__resp_fac.output(rb)
        rb.flush()

        rb.reset("S", fd)
        for sta in sta_list:
            sta.output(rb, self.__vol_start_time, self.__vol_end_time)

        if self.__waveform_data is not None:
            index_start = rb.get_recno()
            rb.reset("T", fd)
            self.__waveform_data.output_index(rb, 0)

            data_start = rb.get_recno()
            rb.reset("T", fd, index_start)
            fd.seek((1 << _RECLEN_EXP) * (index_start - 1), 0)
            self.__waveform_data.output_index(rb, data_start)
            self.__waveform_data.output_data(fd, data_start)

        fd.seek(0, 0)
        rb.reset("V", fd, 1)
        self.__output_vol(vol_creat_time, sta_list, rb)

        if isinstance(dest, basestring):
            fd.close()
