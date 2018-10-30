#***************************************************************************** 
# xmlparser.py
#
# SeisComP XML parser
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import re
import datetime
import xml.sax
import xml.sax.handler
from seiscomp import logs

try:
    from decimal import Decimal   # Python 2.4
    _have_decimal = True
except ImportError:
    _have_decimal = False         # Will use float for decimal, hope it works

class _ForeignNamespace(Exception):
    pass

class _UnsupportedNamespace(Exception):
    pass

class _UnknownElement(Exception):
    pass

class _UnknownAttribute(Exception):
    pass

class _BadValue(Exception):
    pass

class XAttribute(object):
    def toxml(cls, val):
        if val is None:
            return ""

        return unicode(val)

    toxml = classmethod(toxml)
 
def _fix_attr(s): # fix bad attribute names (not complete)
    return s.replace('-', '_')

class XElement(object):
    xmlns = None

    def __init__(self, xdict = {}):
        self.__xdict = xdict
        self._cdata = ""
        
        for name, x in xdict.items():
            if isinstance(x, XAttribute):
                self.__dict__[_fix_attr(name)] = x.fromxml()
            else:
                self.__dict__[_fix_attr(name)] = []
 
    def __find_ctor(self, x, uri):
        if isinstance(x, tuple):
            for x1 in x:
                ctor = self.__find_ctor(x1, uri)
                if ctor is not None:
                    return ctor

        elif (x.xmlns is None and uri == self.xmlns) or uri == x.xmlns:
            return x
 
    def set_attr(self, uri, name, value):
        attr = self.__xdict.get(name)
        if attr is not None:
            if (uri is None or uri == self.xmlns):
                if isinstance(attr, XAttribute):
                    try:
                        obj = attr.fromxml(value)
                    except (TypeError, ValueError), e:
                        raise _BadValue, e.args
                
                    self.__dict__[_fix_attr(name)] = obj
                    return
                else:
                    raise _UnknownAttribute
            else:
                raise _UnsupportedNamespace
        elif uri is None or uri == self.xmlns:
            raise _UnknownAttribute
        else:
            raise _ForeignNamespace

    def add_child(self, uri, name):
        x = self.__xdict.get(name)
        if x is not None:
            ctor = self.__find_ctor(x, uri)
            if ctor is not None:
                if isinstance(ctor, type):
                    obj = ctor()
                    obj.xmlns = uri
                    self.__dict__[_fix_attr(name)].append(obj)
                    return obj
                else:
                    raise _UnknownElement
            else:
                raise _UnsupportedNamespace
        elif uri == self.xmlns:        
            raise _UnknownElement
        else:
            raise _ForeignNamespace

class _MyContentHandler(xml.sax.ContentHandler):
    def __init__(self, parser, element_stack, warn_unused):
        xml.sax.ContentHandler.__init__(self)
        self.__parser = parser
        self.__element_stack = element_stack
        self.__warn_unused = warn_unused

    def warning(self, s):
        logs.warning(str(self.__parser.getSystemId()) + ":" + \
            str(self.__parser.getLineNumber()) + ":" + \
            str(self.__parser.getColumnNumber()) + ": " + s)
    
    def characters(self, data):
        self.__element_stack[-1]._cdata += data

    def __start_element(self, uri, localname, attributes):
        try:
            chd = self.__element_stack[-1].add_child(uri, localname)
            for name, value in attributes.items():
                if isinstance(name, tuple):  # AttributesNS
                    attr_uri = name[0]
                    attr_localname = name[1]
                else:
                    attr_uri = uri
                    attr_localname = name

                try:
                    chd.set_attr(attr_uri, attr_localname, value)
                    
                except _UnknownAttribute:
                    if self.__warn_unused:
                        if attr_uri is not None:
                            self.warning("attribute '{%s}%s' ignored" % \
                                (attr_uri, attr_localname))
                        else:
                            self.warning("attribute '%s' ignored" % \
                                (attr_localname,))

                except _UnsupportedNamespace:
                    if self.__warn_unused:
                        if attr_uri is not None:
                            self.warning("attribute '{%s}%s' ignored, "
                                "unsupported namespace" % \
                                (attr_uri, attr_localname))
                        else:
                            self.warning("attribute '%s' ignored, "
                                "unsupported namespace" % \
                                (attr_localname,))

                except _ForeignNamespace:
                    pass
                    
                except _BadValue, e:
                    self.warning(str(e))

        except _UnknownElement:
            if self.__warn_unused:
                if uri is not None:
                    self.warning("element '{%s}%s' ignored" % (uri, localname))
                else:
                    self.warning("element '%s' ignored" % (localname,))

            chd = XElement()
            chd.xmlns = uri
            
        except _UnsupportedNamespace:
            if self.__warn_unused:
                if uri is not None:
                    self.warning("element '{%s}%s' ignored, "
                        "unsupported namespace" % (uri, localname))
                else:
                    self.warning("element '%s' ignored, "
                        "unsupported namespace" % (localname,))

            chd = XElement()
            chd.xmlns = self.__element_stack[-1].xmlns

        except _ForeignNamespace:
            chd = XElement()
            chd.xmlns = self.__element_stack[-1].xmlns

        self.__element_stack.append(chd)

    def startElement(self, localname, attributes):
        self.__start_element(None, localname, attributes)

    def endElement(self, localname):
        self.__element_stack.pop()

    def startElementNS(self, name, qname, attributes):
        (uri, localname) = name
        self.__start_element(uri, localname, attributes)
    
    def endElementNS(self, name, qname):
        self.__element_stack.pop()

class StringAttr(XAttribute):
    def __init__(self, default = ""):
        self._default = default

    def fromxml(self, val = ""):
        if val == "":
            return self._default

        return val

class IntAttr(XAttribute):
    def __init__(self, default = 0):
        self._default = default

    def fromxml(self, val = ""):
        if val == "":
            return self._default

        return int(val)

class FloatAttr(XAttribute):
    def __init__(self, default = 0.0):
        self._default = default

    def fromxml(self, val = ""):
        if val == "":
            return self._default

        return float(val)

if _have_decimal:
    class DecimalAttr(XAttribute):
        def __init__(self, default = 0):
            self._default = default

        def fromxml(self, val = ""):
            if val == "":
                return self._default

            return Decimal(val)
else:
    class DecimalAttr(FloatAttr):
        pass
        
class _Rational(object):
    def __init__(self, numerator, denominator):
        self.numerator = numerator
        self.denominator = denominator

class RationalAttr(XAttribute):
    def __init__(self, default_num = 0, default_denom = 1):
        self._default = _Rational(default_num, default_denom)

    def toxml(cls, num, denom):
        if denom == 1:
            return unicode(num)
        
        return unicode(num) + "/" + unicode(denom)

    toxml = classmethod(toxml)
    
    def fromxml(self, val = ""):
        if val == "":
            return self._default

        num_denom = val.split("/")
        if len(num_denom) == 1:
            return _Rational(int(num_denom[0]), 1)
        elif len(num_denom) == 2:
            return _Rational(int(num_denom[0]), int(num_denom[1]))
        else:
            raise ValueError, "invalid rational: " + str(val)

class BoolAttr(XAttribute):
    def __init__(self, default = False):
        self._default = default

    def toxml(cls, val):
        if val:
            return "true"

        return "false"

    toxml = classmethod(toxml)

    def fromxml(self, val = ""):
        if val == "":
            return self._default

        if val == "True" or val == "true":
            return True

        return False
        
_rx_datetime = re.compile("([0-9]{4})-([0-9]{2})-([0-9]{2})T" \
    "([0-9]{2}):([0-9]{2}):([0-9]{2}).([0-9]*)" \
    "(Z|([+-])([0-9]{2}):([0-9]{2}))?$")

_rx_date = re.compile("([0-9]*)-([0-9]*)-([0-9]*)" \
    "(Z|([+-])([0-9]{2}):([0-9]{2}))?$")

class DateTimeAttr(XAttribute):
    def __init__(self, default = None):
        self._default = None

    def toxml(cls, val):
        if isinstance(val, datetime.datetime):
            return "%04d-%02d-%02dT%02d:%02d:%02d.%04dZ" % \
                (val.year, val.month, val.day, val.hour, val.minute,
                val.second, val.microsecond / 100)
        elif isinstance(val, datetime.date):
            return "%04d-%02d-%02d" % (val.year, val.month, val.day)

        elif val is None:
            return ""

        raise ValueError, "invalid date or datetime object"

    toxml = classmethod(toxml)

    def fromxml(self, val = ""):
        if val == "":
            return self._default
            
        m = _rx_datetime.match(val)
        if m is None:
            m = _rx_date.match(val)
            if m is None:
                raise ValueError, "invalid datetime: " + val

            (year, month, mday, tz, plusminus, tzhours, tzminutes) = m.groups()

            try:
                # ignore time zone
                obj = datetime.date(int(year), int(month), int(mday))
            except ValueError:
                raise ValueError, "invalid datetime: " + val
        else:
            (year, month, mday, hour, min, sec, sfract,
                tz, plusminus, tzhours, tzminutes) = m.groups()

            try:
                obj = datetime.datetime(int(year), int(month), int(mday),
                    int(hour), int(min), int(sec), int((sfract + "000000")[:6]))
                if tz is not None and tz != "Z":
                    delta = datetime.timedelta(hours = int(tzhours),
                        minutes=int(tzminutes))
                    if plusminus == "-":
                        obj -= delta
                    else:
                        obj += delta
                    
            except ValueError:
                raise ValueError, "invalid datetime: " + val

        return obj

def read_xml(source, xdict, warn_unused = True):
    p = xml.sax.make_parser()
    element_stack = [XElement(xdict)]
    p.setFeature(xml.sax.handler.feature_namespaces, True)
    p.setContentHandler(_MyContentHandler(p, element_stack, warn_unused))
    p.parse(source)
    return element_stack[0]

