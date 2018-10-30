import re
from datetime import datetime
import string

class parsers(object):

    @staticmethod
    def parseString(val):
        return val.strip()
    
    @staticmethod
    def _parse_paz(npaz, s):
        _rx_paz = re.compile(r'\s*([0-9]*)\(\s*([^,]+),\s*([^)]+)\)\s*')
        pos = 0
        n = 0
        c = []
        while pos < len(s):
            m = _rx_paz.match(s, pos)
            if m is None:
                raise Exception("error parsing PAZ at '" + s[pos:] + "'")
    
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
                raise Exception("error parsing PAZ at '" + s[pos:] + "'")
    
            for i in xrange(0, x):
                c.append((rv, iv))
                i = i
            
            n += x
            pos = m.end()
        
        if n != npaz:
            raise Exception("expected %d PAZ, found %d" % (npaz, n))
        return c

    @staticmethod
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
    
    @staticmethod
    def _rational(x):
        sign, mantissa, exponent = x.as_tuple()
        sign = (1, -1)[sign]
        mantissa = sign * reduce(lambda a, b: 10 * a + b, mantissa)
        if exponent < 0:
            return parsers._normalize(mantissa, 10 ** (-exponent))
        else:
            return (mantissa * 10 ** exponent, 1)

    @staticmethod
    def _parseFloat(val, mi=None , ma= None):
        number = float(val)
        if (mi and number < mi) or (ma and number > ma):
            raise Exception("Invalid Range")
        return number
    
    @staticmethod
    def parseGain(val):
        try:
            return parsers._parseFloat(val, 0.0, None)
        except Exception,e:
            raise Exception("Invalid Gain: %s" % e)
    
    @staticmethod
    def parseLongitude(val):
        try:
            return parsers._parseFloat(val, -180.0, 180.0)
        except Exception,e:
            raise Exception("Invalid Longitude: %s" % e)
    
    @staticmethod
    def parseLatitude(val):
        try:
            return parsers._parseFloat(val, -90.0, 90.0)
        except Exception,e:
            raise Exception("Invalid Latitude: %s" % e)
    
    @staticmethod
    def parseDepth(val):
        # Deepest mine ~ 5000 m
        try:
            return parsers._parseFloat(val, 0.0, 5000)
        except Exception,e:
            raise Exception("Invalid Depth: %s" % e)
    
    @staticmethod
    def parseElevation(val):
        # Highest Everest ~8500 m
        # Deepest Mariana ~11000 m
        try:
            return parsers._parseFloat(val, -11000, 9000)
        except Exception,e:
            raise Exception("Invalid Elevation: %s" % e)

    @staticmethod    
    def parseDate(val):
        date=val.replace("/", "-")
        formats={ len("YYYY-JJJ") : "%Y-%j",
                len("YYYY-MM-DD") : "%Y-%m-%d",
                len("YYYY-JJJ:HHMM") : "%Y-%j:%H%M",
                len("YYYY-JJJTHH:MM") : "%Y-%jT%H:%M",
                len("YYYY-MM-DDTHH:MM") : "%Y-%m-%dT%H:%M",
                len("YYYY-JJJTHH:MM:SS") : "%Y-%jT%H:%M:%S",
                len("YYYY-MM-DDTHH:MM:SS") : "%Y-%m-%dT%H:%M:%S"}
        try:
            return datetime.strptime(date, formats[len(date)])
        except Exception, e:
            raise ValueError, "invalid date: " + date + str(e)

    @staticmethod    
    def parseLocationCode(val):
        Code = val.strip()
        if len(Code) > 2 or len(re.sub("[A-Z0-9-*?]","",Code)) > 0:
            raise Exception("wrong code for location: %s" % Code)
        return Code

    @staticmethod    
    def parseStationCode(val):
        Code = val.strip()
        if not Code or len(Code) > 5 or  len(re.sub("[A-Z0-9*?]","",Code)) > 0:
            raise Exception("Wrong code for station: %s" % Code)
        return Code

    @staticmethod    
    def parseChannelCode(val):
        Code = val.strip()
        if not Code or len(Code) > 3 or len(re.sub("[A-Z0-9*?]","",Code)) > 0:
            raise Exception("Wrong code for channel: %s" % Code)
        return Code

    @staticmethod
    def parseNetworkCode(val):
        Code = val.strip()
        if not Code or len(Code) > 2 or len(re.sub("[A-Z0-9*?]","",Code)) > 0:
            raise Exception("Wrong code for network: %s" % Code)
        return Code
