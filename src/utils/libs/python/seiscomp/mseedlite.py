#***************************************************************************** 
# mseedlite.py
#
# Python-only Mini-SEED module with limited functionality
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import datetime
import struct
import cStringIO
from seiscomp import logs

_FIXHEAD_LEN = 48
_BLKHEAD_LEN = 4
_BLK1000_LEN = 4
_BLK1001_LEN = 4
_MAX_RECLEN = 4096

_doy = (0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365)

def _is_leap(y):
    """True if y is a leap year."""
    return (y % 400 == 0 or (y % 4 == 0 and y % 100 != 0))

def _ldoy(y, m):
    """The day of the year of the first day of month m, in year y.
    Note: for January, m=0; for December, m=11.

    Examples:

    _ldoy(1900, 3) = 90
    _ldoy(1900, 0) = 0
    _ldoy(1999, 3) = 90
    _ldoy(2004, 3) = 91
    _ldoy(2000, 3) = 91

    """
    return _doy[m] + (_is_leap(y) and m >= 2)

def _dy2mdy(doy, year):
    month = 1
    while doy > _ldoy(year, month):
        month += 1

    mday = doy - _ldoy(year, month - 1)
    return (month, mday)

def _mdy2dy(month, day, year):
    return _ldoy(year, month - 1) + day

class MSeedError(Exception):
    pass

class MSeedNoData(MSeedError):
    pass

class Record(object):
    def __init__(self, src):
        if isinstance(src, basestring):
            fd = cStringIO.StringIO(src)
        elif hasattr(src, "read"):
            fd = src
        else:
            raise TypeError, "argument is neither string nor file object"

        self.header = ""
        fixhead = fd.read(_FIXHEAD_LEN)

        if len(fixhead) == 0:
            raise StopIteration

        if len(fixhead) < _FIXHEAD_LEN:
            raise MSeedError, "unexpected end of header"

        (recno_str, self.rectype, sta, loc, cha, net, bt_year, bt_doy, bt_hour,
            bt_minute, bt_second, bt_tms, self.nsamp, self.sr_factor,
            self.sr_mult, self.aflgs, self.cflgs, self.qflgs, self.__num_blk,
            self.time_correction, self.__pdata, self.__pblk) = \
            struct.unpack(">6scx5s2s3s2s2H3Bx2H2h4Bl2H", fixhead)

        self.header += fixhead

        if self.rectype != 'D' and self.rectype != 'R' and self.rectype != 'Q' and self.rectype != 'M':
            fd.read(_MAX_RECLEN - _FIXHEAD_LEN)
            raise MSeedNoData, "non-data record"

        if self.__pdata < _FIXHEAD_LEN or self.__pdata >= _MAX_RECLEN or \
            (self.__pblk != 0 and \
            (self.__pblk < _FIXHEAD_LEN or self.__pblk >= self.__pdata)):
            raise MSeedError, "invalid pointers"

        if self.__pblk == 0:
            blklen = 0
        else:
            blklen = self.__pdata - self.__pblk
            gaplen = self.__pblk - _FIXHEAD_LEN
            gap = fd.read(gaplen)
            if len(gap) < gaplen:
                raise MSeedError, "unexpected end of data"

            self.header += gap

        # defaults
        self.encoding = 11
        self.byteorder = 1
        rec_len_exp = 12
        self.time_quality = -1
        micros = 0
        self.nframes = None
        self.__rec_len_exp_idx = None
        self.__micros_idx = None
        self.__nframes_idx = None

        pos = 0
        while pos < blklen:
            blkhead = fd.read(_BLKHEAD_LEN)
            if len(blkhead) < _BLKHEAD_LEN:
                raise MSeedError, "unexpected end of blockettes at ", \
                    pos + len(blkhead)

            (blktype, nextblk) = struct.unpack(">2H", blkhead)
            self.header += blkhead
            pos += _BLKHEAD_LEN

            if blktype == 1000:
                blk1000 = fd.read(_BLK1000_LEN)
                if len(blk1000) < _BLK1000_LEN:
                    raise MSeedError, "unexpected end of blockettes at ", \
                        pos + len(blk1000)

                (self.encoding, self.byteorder, rec_len_exp) = \
                    struct.unpack(">3Bx", blk1000)

                self.__rec_len_exp_idx = self.__pblk + pos + 2
                self.header += blk1000
                pos += _BLK1000_LEN

            elif blktype == 1001:
                blk1001 = fd.read(_BLK1001_LEN)
                if len(blk1001) < _BLK1001_LEN:
                    raise MSeedError, "unexpected end of blockettes at ", \
                        pos + len(blk1001)

                (self.time_quality, micros, self.nframes) = \
                    struct.unpack(">BbxB", blk1001)

                self.__micros_idx = self.__pblk + pos + 1
                self.__nframes_idx = self.__pblk + pos + 3
                self.header += blk1001
                pos += _BLK1001_LEN

            if nextblk == 0:
                break

            if nextblk < self.__pblk + pos or nextblk >= self.__pdata:
                raise MSeedError, "invalid pointers"

            gaplen = nextblk - (self.__pblk + pos)
            gap = fd.read(gaplen)
            if len(gap) < gaplen:
                raise MSeedError, "unexpected end of data"

            self.header += gap
            pos += gaplen

        if pos > blklen:
            raise MSeedError, "corrupt record"

        gaplen = self.__pdata - len(self.header)
        gap = fd.read(gaplen)
        if len(gap) < gaplen:
            raise MSeedError, "unexpected end of data"

        self.header += gap
        pos += gaplen

        self.recno = int(recno_str)
        self.net = net.strip()
        self.sta = sta.strip()
        self.loc = loc.strip()
        self.cha = cha.strip()

        if self.sr_factor > 0 and self.sr_mult > 0:
            self.samprate_num = self.sr_factor * self.sr_mult
            self.samprate_denom = 1
        elif self.sr_factor > 0 and self.sr_mult < 0:
            self.samprate_num = self.sr_factor
            self.samprate_denom = -self.sr_mult
        elif self.sr_factor < 0 and self.sr_mult > 0:
            self.samprate_num = self.sr_mult
            self.samprate_denom = -self.sr_factor
        elif self.sr_factor < 0 and self.sr_mult < 0:
            self.samprate_num = 1
            self.samprate_denom = self.sr_factor * self.sr_mult
        else:
            self.samprate_num = 0
            self.samprate_denom = 1

        self.fsamp = float(self.samprate_num) / float(self.samprate_denom)

        # quick fix to avoid exception from datetime
        if bt_second > 59:
            self.leap = bt_second - 59
            bt_second = 59
        else:
            self.leap = 0

        try:
            (month, day) = _dy2mdy(bt_doy, bt_year)
            self.begin_time = datetime.datetime(bt_year, month, day, bt_hour,
                bt_minute, bt_second)

            self.begin_time += datetime.timedelta(microseconds = bt_tms * 100 + micros)

            if self.nsamp != 0 and self.fsamp != 0:
                self.end_time = self.begin_time + \
                    datetime.timedelta(microseconds = 1000000 * self.nsamp / self.fsamp)
            else:
                self.end_time = self.begin_time

        except ValueError, e:
            logs.error("tms = " + str(bt_tms) + ", micros = " + str(micros))
            raise MSeedError, "invalid time: " + str(e)

        self.size = 1 << rec_len_exp
        if self.size < len(self.header) or self.size > _MAX_RECLEN:
            raise MSeedError, "invalid record size"

        datalen = self.size - self.__pdata
        self.data = fd.read(datalen)
        if len(self.data) < datalen:
            raise MSeedError, "unexpected end of data"

        if len(self.header) + len(self.data) != self.size:
            raise MSeedError, "internal error"

        (self.X0, self.Xn) = struct.unpack(">ll", self.data[4:12])

        (w0,) = struct.unpack(">L", self.data[:4])
        (w3,) = struct.unpack(">L", self.data[12:16])
        c3 = (w0 >> 24) & 0x3
        d0 = None

        if self.encoding == 10:
            """STEIM (1) Compression?"""
            if c3 == 1:
                d0 = (w3 >> 24) & 0xff
                if d0 > 0x7f:
                    d0 -= 0x100
            elif c3 == 2:
                d0 = (w3 >> 16) & 0xffff
                if d0 > 0x7fff:
                    d0 -= 0x10000
            elif c3 == 3:
                d0 = w3 & 0xffffffff
                if d0 > 0x7fffffff:
                    d0 -= 0xffffffff
                    d0 -= 1

        elif self.encoding == 11:
            """STEIM (2) Compression?"""
            if c3 == 1:
                d0 = (w3 >> 24) & 0xff
                if d0 > 0x7f:
                    d0 -= 0x100
            elif c3 == 2:
                dnib = (w3 >> 30) & 0x3
                if dnib == 1:
                    d0 = w3 & 0x3fffffff
                    if d0 > 0x1fffffff:
                        d0 -= 0x40000000
                elif dnib == 2:
                    d0 = (w3 >> 15) & 0x7fff
                    if d0 > 0x3fff:
                        d0 -= 0x8000
                elif dnib == 3:
                    d0 = (w3 >> 20) & 0x3ff
                    if d0 > 0x1ff:
                        d0 -= 0x400
            elif c3 == 3:
                dnib = (w3 >> 30) & 0x3
                if dnib == 0:
                    d0 = (w3 >> 24) & 0x3f
                    if d0 > 0x1f:
                        d0 -= 0x40
                elif dnib == 1:
                    d0 = (w3 >> 25) & 0x1f
                    if d0 > 0xf:
                        d0 -= 0x20
                elif dnib == 2:
                    d0 = (w3 >> 24) & 0xf
                    if d0 > 0x7:
                        d0 -= 0x10

        if d0 is not None:
            self.X_minus1 = self.X0 - d0
        else:
            self.X_minus1 = None

        if self.nframes is None or self.nframes == 0:
            i = 0
            self.nframes = 0
            while i < len(self.data):
                if self.data[i] == "\0":
                    break

                i += 64
                self.nframes += 1

    def merge(self, rec):
        """
        Caller is expected to check for contiguity of data
        Check if rec.nframes * 64 <= len(data)?

        """
        (self.Xn,) = struct.unpack(">l", rec.data[8:12])
        self.data += rec.data[:rec.nframes * 64]
        self.nframes += rec.nframes
        self.nsamp += rec.nsamp
        self.size = len(self.header) + len(self.data)
        self.end_time = rec.end_time

    def write(self, fd, rec_len_exp):
        if self.size > (1 << rec_len_exp):
            raise MSeedError, "record is larger than requested write size"

        recno_str = "%06d" % (self.recno,)
        sta = "%-5.5s" % (self.sta,)
        loc = "%-2.2s" % (self.loc,)
        cha = "%-3.3s" % (self.cha,)
        net = "%-2.2s" % (self.net,)
        bt_year = self.begin_time.year
        bt_doy = _mdy2dy(self.begin_time.month, self.begin_time.day,
            self.begin_time.year)
        bt_hour = self.begin_time.hour
        bt_minute = self.begin_time.minute
        bt_second = self.begin_time.second + self.leap
        bt_tms = self.begin_time.microsecond // 100
        micros = self.begin_time.microsecond % 100

        buf = struct.pack(">6s2c5s2s3s2s2H3Bx2H2h4Bl2H", recno_str,
            self.rectype, ' ', sta, loc, cha, net, bt_year, bt_doy, bt_hour,
            bt_minute, bt_second, bt_tms, self.nsamp, self.sr_factor,
            self.sr_mult, self.aflgs, self.cflgs, self.qflgs, self.__num_blk,
            self.time_correction, self.__pdata, self.__pblk)

        fd.write(buf)

        buf = list(self.header[_FIXHEAD_LEN:])

        if self.__rec_len_exp_idx is not None:
            buf[self.__rec_len_exp_idx - _FIXHEAD_LEN] = struct.pack(">B", rec_len_exp)

        if self.__micros_idx is not None:
            buf[self.__micros_idx - _FIXHEAD_LEN] = struct.pack(">b", micros)

        if self.__nframes_idx is not None:
            buf[self.__nframes_idx - _FIXHEAD_LEN] = struct.pack(">B", self.nframes)

        fd.write(''.join(buf))

        buf = self.data[:4] + struct.pack(">ll", self.X0, self.Xn) + \
            self.data[12:] + ((1 << rec_len_exp) - self.size) * '\0'

        fd.write(buf)

class _Iter(object):
    def __init__(self, fd):
        self.__fd = fd

    def next(self):
        while True:
            try:
                return Record(self.__fd)

            except MSeedError, e:
                logs.error(str(e))

            except MSeedNoData:
                pass

class Input(object):
    def __init__(self, fd):
        self.__fd = fd

    def __iter__(self):
        return _Iter(self.__fd)

