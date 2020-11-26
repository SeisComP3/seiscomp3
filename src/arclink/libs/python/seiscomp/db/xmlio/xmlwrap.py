# This file was created by a source code generator:
# genxml2wrap.py 
# Do not modify. Change the definition and
# run the generator again!
#
# (c) 2010 Mathias Hoffmann, GFZ Potsdam
#
#

from __future__ import absolute_import, division, print_function
import re
import datetime

def Property(func):
        return property(**func())

try:
	import xml.etree.cElementTree as ET  # Python 2.5?
except ImportError:
	import cElementTree as ET

try:
	from decimal import Decimal   # Python 2.4
	_have_decimal = True
except ImportError:
	_have_decimal = False         # Will use float for decimal, hope it works

def _string_fromxml(val):
	if val is None:
		return ""

	return val.encode("utf-8", "replace").strip()

def _string_toxml(val):
	if val is None:
		return ""

	if isinstance(val, str):
		try:
			return val.decode("utf-8")
		except UnicodeDecodeError:
			return val.decode("iso-8859-1", "replace")

	return unicode(val)

def _int_fromxml(val):
	if val is None or val == "":
		return None

	return int(val)

_int_toxml = _string_toxml

def _float_fromxml(val):
	if val is None or val == "":
		return None

	return float(val)

_float_toxml = _string_toxml

if _have_decimal:
	def _decimal_fromxml(val):
		if val is None or val == "":
			return None

		return Decimal(val)
else:
	_decimal_fromxml = _float_fromxml

_decimal_toxml = _string_toxml

def _boolean_fromxml(val):
	if val == "True" or val == "true":
		return True

	return False

def _boolean_toxml(val):
	if val:
		return "true"

	return "false"

# DataModel.DEPLOYMENT == 0
# DataModel.ARRAY == 1
def _StationGroupType_toxml(val):
	if val == 0:
		return "DEPLOYMENT"
	else:
		return "ARRAY"

def _StationGroupType_fromxml(val):
	if val == "DEPLOYMENT":
		return 0
	else:
		return 1

_rx_datetime = re.compile("([0-9]{4})-([0-9]{2})-([0-9]{2})T" \
	"([0-9]{2}):([0-9]{2}):([0-9]{2})(\.([0-9]*))?" \
	"(Z|([+-])([0-9]{2}):([0-9]{2}))?$")

_rx_date = re.compile("([0-9]*)-([0-9]*)-([0-9]*)" \
	"(Z|([+-])([0-9]{2}):([0-9]{2}))?$")

def _datetime_fromxml(val = ""):
	if val is None or val == "":
		return None
		
	m = _rx_datetime.match(val)
	if m == None:
		m = _rx_date.match(val)
		if m == None:
			raise ValueError("invalid datetime: " + val)

		(year, month, mday, tz, plusminus, tzhours, tzminutes) = m.groups()

		try:
			# ignore time zone
			obj = datetime.datetime(int(year), int(month), int(mday), 0, 0, 0)
		except ValueError:
			raise ValueError("invalid datetime: " + val)
	else:
		(year, month, mday, hour, min, sec, sfdot, sfract,
			tz, plusminus, tzhours, tzminutes) = m.groups()

		if sfract is None:
			sfract = "0"
		
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
			raise ValueError("invalid datetime: " + val)

	return obj

def _datetime_toxml(val):
	if isinstance(val, datetime.datetime):
		return "%04d-%02d-%02dT%02d:%02d:%02d.%04dZ" % 			(val.year, val.month, val.day, val.hour, val.minute,
			val.second, val.microsecond / 100)
	elif isinstance(val, datetime.date):
		return "%04d-%02d-%02d" % (val.year, val.month, val.day)

	elif val is None:
		return ""

	raise ValueError("invalid date or datetime object")

def _get_blob(e, name):
	return _string_fromxml(e.findtext(name)).strip()
	
def _set_blob(e, name, value):
	text = _string_toxml(value).strip()
	e1 = ET.Element(name)
	e1.text = text
	e.append(e1)

def _get_quantity(e, name):
	e1 = e.find(name)
	if e1 is None:
		return quantity()

	return quantity(_float_fromxml(e1.text), unit=_string_fromxml(e1.get("unit")), error=_float_fromxml(e1.get("error")))

def _set_quantity(e, name, value):
	e1 = ET.Element(name)
	e1.text = _float_toxml(value)
	e1.set("unit", _string_toxml(value.unit))
	e1.set("error", _float_toxml(value.error))
	e.append(e1)
#
#




# Inventory::Comment
class xml_Comment(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Comment._xmlns + "comment")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.text = src.text
        self.id = src.id
        self.start = src.start
        self.end = src.end
        # creationInfo is an XML attribute, but we are missing
        # CreationInfo_fromxml() [string->object] and _CreationInfo_toxml()
        # [object->string] serialization within the creationInfo() property.
        # Could be JSON??? Ignore CreationInfo for now.
        #self.creationInfo = src.creationInfo

    def _copy_to(self, dest):
        if self._element.get("text") is not None:
            dest.text = self.text
        if self._element.get("id") is not None:
            dest.id = self.id
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("creationInfo") is not None:
            dest.creationInfo = self.creationInfo

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def text():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("text"))
        def fset(self, value):
            self._element.set("text", _string_toxml(value))
        return locals()

    @Property
    def id():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("id"))
        def fset(self, value):
            self._element.set("id", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def creationInfo():
    # type: CreationInfo
        def fget(self):
            return _CreationInfo_fromxml(self._element.get("creationInfo"))
        def fset(self, value):
            self._element.set("creationInfo", _CreationInfo_toxml(value))
        return locals()


# QualityControl::QCLog
class xml_QCLog(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/QualityControl/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_QCLog._xmlns + "qCLog")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.networkCode = src.networkCode
        self.stationCode = src.stationCode
        self.streamCode = src.streamCode
        self.locationCode = src.locationCode
        self.creatorID = src.creatorID
        self.created = src.created
        self.start = src.start
        self.end = src.end
        self.message = src.message
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("networkCode") is not None:
            dest.networkCode = self.networkCode
        if self._element.get("stationCode") is not None:
            dest.stationCode = self.stationCode
        if self._element.get("streamCode") is not None:
            dest.streamCode = self.streamCode
        if self._element.get("locationCode") is not None:
            dest.locationCode = self.locationCode
        if self._element.get("creatorID") is not None:
            dest.creatorID = self.creatorID
        if self._element.get("created") is not None:
            dest.created = self.created
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("message") is not None:
            dest.message = self.message
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def networkCode():
        def fget(self):
            return _string_fromxml(self._element.get("networkCode"))
        def fset(self, value):
            self._element.set("networkCode", _string_toxml(value))
        return locals()

    @Property
    def stationCode():
        def fget(self):
            return _string_fromxml(self._element.get("stationCode"))
        def fset(self, value):
            self._element.set("stationCode", _string_toxml(value))
        return locals()

    @Property
    def streamCode():
        def fget(self):
            return _string_fromxml(self._element.get("streamCode"))
        def fset(self, value):
            self._element.set("streamCode", _string_toxml(value))
        return locals()

    @Property
    def locationCode():
        def fget(self):
            return _string_fromxml(self._element.get("locationCode"))
        def fset(self, value):
            self._element.set("locationCode", _string_toxml(value))
        return locals()

    @Property
    def creatorID():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("creatorID"))
        def fset(self, value):
            self._element.set("creatorID", _string_toxml(value))
        return locals()

    @Property
    def created():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("created"))
        def fset(self, value):
            self._element.set("created", _datetime_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def message():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("message"))
        def fset(self, value):
            self._element.set("message", _string_toxml(value))
        return locals()


# QualityControl::WaveformQuality
class xml_WaveformQuality(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/QualityControl/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_WaveformQuality._xmlns + "waveformQuality")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.networkCode = src.networkCode
        self.stationCode = src.stationCode
        self.streamCode = src.streamCode
        self.locationCode = src.locationCode
        self.creatorID = src.creatorID
        self.created = src.created
        self.start = src.start
        self.end = src.end
        self.type = src.type
        self.parameter = src.parameter
        self.value = src.value
        self.lowerUncertainty = src.lowerUncertainty
        self.upperUncertainty = src.upperUncertainty
        self.windowLength = src.windowLength

    def _copy_to(self, dest):
        if self._element.get("networkCode") is not None:
            dest.networkCode = self.networkCode
        if self._element.get("stationCode") is not None:
            dest.stationCode = self.stationCode
        if self._element.get("streamCode") is not None:
            dest.streamCode = self.streamCode
        if self._element.get("locationCode") is not None:
            dest.locationCode = self.locationCode
        if self._element.get("creatorID") is not None:
            dest.creatorID = self.creatorID
        if self._element.get("created") is not None:
            dest.created = self.created
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("type") is not None:
            dest.type = self.type
        if self._element.get("parameter") is not None:
            dest.parameter = self.parameter
        if self._element.get("value") is not None:
            dest.value = self.value
        if self._element.get("lowerUncertainty") is not None:
            dest.lowerUncertainty = self.lowerUncertainty
        if self._element.get("upperUncertainty") is not None:
            dest.upperUncertainty = self.upperUncertainty
        if self._element.get("windowLength") is not None:
            dest.windowLength = self.windowLength

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def networkCode():
        def fget(self):
            return _string_fromxml(self._element.get("networkCode"))
        def fset(self, value):
            self._element.set("networkCode", _string_toxml(value))
        return locals()

    @Property
    def stationCode():
        def fget(self):
            return _string_fromxml(self._element.get("stationCode"))
        def fset(self, value):
            self._element.set("stationCode", _string_toxml(value))
        return locals()

    @Property
    def streamCode():
        def fget(self):
            return _string_fromxml(self._element.get("streamCode"))
        def fset(self, value):
            self._element.set("streamCode", _string_toxml(value))
        return locals()

    @Property
    def locationCode():
        def fget(self):
            return _string_fromxml(self._element.get("locationCode"))
        def fset(self, value):
            self._element.set("locationCode", _string_toxml(value))
        return locals()

    @Property
    def creatorID():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("creatorID"))
        def fset(self, value):
            self._element.set("creatorID", _string_toxml(value))
        return locals()

    @Property
    def created():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("created"))
        def fset(self, value):
            self._element.set("created", _datetime_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def type():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("type"))
        def fset(self, value):
            self._element.set("type", _string_toxml(value))
        return locals()

    @Property
    def parameter():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("parameter"))
        def fset(self, value):
            self._element.set("parameter", _string_toxml(value))
        return locals()

    @Property
    def value():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("value"))
        def fset(self, value):
            self._element.set("value", _float_toxml(value))
        return locals()

    @Property
    def lowerUncertainty():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("lowerUncertainty"))
        def fset(self, value):
            self._element.set("lowerUncertainty", _float_toxml(value))
        return locals()

    @Property
    def upperUncertainty():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("upperUncertainty"))
        def fset(self, value):
            self._element.set("upperUncertainty", _float_toxml(value))
        return locals()

    @Property
    def windowLength():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("windowLength"))
        def fset(self, value):
            self._element.set("windowLength", _float_toxml(value))
        return locals()


# QualityControl::Outage
class xml_Outage(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/QualityControl/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Outage._xmlns + "outage")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.networkCode = src.networkCode
        self.stationCode = src.stationCode
        self.streamCode = src.streamCode
        self.locationCode = src.locationCode
        self.creatorID = src.creatorID
        self.created = src.created
        self.start = src.start
        self.end = src.end

    def _copy_to(self, dest):
        if self._element.get("networkCode") is not None:
            dest.networkCode = self.networkCode
        if self._element.get("stationCode") is not None:
            dest.stationCode = self.stationCode
        if self._element.get("streamCode") is not None:
            dest.streamCode = self.streamCode
        if self._element.get("locationCode") is not None:
            dest.locationCode = self.locationCode
        if self._element.get("creatorID") is not None:
            dest.creatorID = self.creatorID
        if self._element.get("created") is not None:
            dest.created = self.created
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def networkCode():
        def fget(self):
            return _string_fromxml(self._element.get("networkCode"))
        def fset(self, value):
            self._element.set("networkCode", _string_toxml(value))
        return locals()

    @Property
    def stationCode():
        def fget(self):
            return _string_fromxml(self._element.get("stationCode"))
        def fset(self, value):
            self._element.set("stationCode", _string_toxml(value))
        return locals()

    @Property
    def streamCode():
        def fget(self):
            return _string_fromxml(self._element.get("streamCode"))
        def fset(self, value):
            self._element.set("streamCode", _string_toxml(value))
        return locals()

    @Property
    def locationCode():
        def fget(self):
            return _string_fromxml(self._element.get("locationCode"))
        def fset(self, value):
            self._element.set("locationCode", _string_toxml(value))
        return locals()

    @Property
    def creatorID():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("creatorID"))
        def fset(self, value):
            self._element.set("creatorID", _string_toxml(value))
        return locals()

    @Property
    def created():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("created"))
        def fset(self, value):
            self._element.set("created", _datetime_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()


# QualityControl::QualityControl
class xml_QualityControl(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/QualityControl/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_QualityControl._xmlns + "qualityControl")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        pass


    def _copy_to(self, dest):
        pass

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    # Aggregation: QCLog
    def _new_log(self):
        return xml_QCLog(ET.Element(xml_QCLog._xmlns + "log"))
    @property
    def log(self):
        for e1 in self._element.findall(xml_QCLog._xmlns + "log"):
            yield xml_QCLog(e1)

    # Aggregation: WaveformQuality
    def _new_waveformQuality(self):
        return xml_WaveformQuality(ET.Element(xml_WaveformQuality._xmlns + "waveformQuality"))
    @property
    def waveformQuality(self):
        for e1 in self._element.findall(xml_WaveformQuality._xmlns + "waveformQuality"):
            yield xml_WaveformQuality(e1)

    # Aggregation: Outage
    def _new_outage(self):
        return xml_Outage(ET.Element(xml_Outage._xmlns + "outage"))
    @property
    def outage(self):
        for e1 in self._element.findall(xml_Outage._xmlns + "outage"):
            yield xml_Outage(e1)


# Inventory::StationReference
class xml_StationReference(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_StationReference._xmlns + "stationReference")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.stationID = src.stationID

    def _copy_to(self, dest):
        if self._element.get("stationID") is not None:
            dest.stationID = self.stationID

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def stationID():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("stationID"))
        def fset(self, value):
            self._element.set("stationID", _string_toxml(value))
        return locals()


# Inventory::StationGroup
class xml_StationGroup(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_StationGroup._xmlns + "stationGroup")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.type = src.type
        self.code = src.code
        self.start = src.start
        self.end = src.end
        self.description = src.description
        self.latitude = src.latitude
        self.longitude = src.longitude
        self.elevation = src.elevation
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("type") is not None:
            dest.type = self.type
        if self._element.get("code") is not None:
            dest.code = self.code
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("description") is not None:
            dest.description = self.description
        if self._element.get("latitude") is not None:
            dest.latitude = self.latitude
        if self._element.get("longitude") is not None:
            dest.longitude = self.longitude
        if self._element.get("elevation") is not None:
            dest.elevation = self.elevation
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def type():
    # type: StationGroupType
        def fget(self):
            return _StationGroupType_fromxml(self._element.get("type"))
        def fset(self, value):
            self._element.set("type", _StationGroupType_toxml(value))
        return locals()

    @Property
    def code():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("code"))
        def fset(self, value):
            self._element.set("code", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def description():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("description"))
        def fset(self, value):
            self._element.set("description", _string_toxml(value))
        return locals()

    @Property
    def latitude():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("latitude"))
        def fset(self, value):
            self._element.set("latitude", _float_toxml(value))
        return locals()

    @Property
    def longitude():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("longitude"))
        def fset(self, value):
            self._element.set("longitude", _float_toxml(value))
        return locals()

    @Property
    def elevation():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("elevation"))
        def fset(self, value):
            self._element.set("elevation", _float_toxml(value))
        return locals()

    # Aggregation: StationReference
    def _new_stationReference(self):
        return xml_StationReference(ET.Element(xml_StationReference._xmlns + "stationReference"))
    @property
    def stationReference(self):
        for e1 in self._element.findall(xml_StationReference._xmlns + "stationReference"):
            yield xml_StationReference(e1)


# Inventory::AuxSource
class xml_AuxSource(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_AuxSource._xmlns + "auxSource")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.description = src.description
        self.unit = src.unit
        self.conversion = src.conversion
        self.sampleRateNumerator = src.sampleRateNumerator
        self.sampleRateDenominator = src.sampleRateDenominator
        self.remark = src.remark

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("description") is not None:
            dest.description = self.description
        if self._element.get("unit") is not None:
            dest.unit = self.unit
        if self._element.get("conversion") is not None:
            dest.conversion = self.conversion
        if self._element.get("sampleRateNumerator") is not None:
            dest.sampleRateNumerator = self.sampleRateNumerator
        if self._element.get("sampleRateDenominator") is not None:
            dest.sampleRateDenominator = self.sampleRateDenominator
        if self._element.find(xml_AuxSource._xmlns + "remark") is not None:
            dest.remark = self.remark

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def description():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("description"))
        def fset(self, value):
            self._element.set("description", _string_toxml(value))
        return locals()

    @Property
    def unit():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("unit"))
        def fset(self, value):
            self._element.set("unit", _string_toxml(value))
        return locals()

    @Property
    def conversion():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("conversion"))
        def fset(self, value):
            self._element.set("conversion", _string_toxml(value))
        return locals()

    @Property
    def sampleRateNumerator():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("sampleRateNumerator"))
        def fset(self, value):
            self._element.set("sampleRateNumerator", _int_toxml(value))
        return locals()

    @Property
    def sampleRateDenominator():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("sampleRateDenominator"))
        def fset(self, value):
            self._element.set("sampleRateDenominator", _int_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_AuxSource._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_AuxSource._xmlns + "remark", value)
        return locals()


# Inventory::AuxDevice
class xml_AuxDevice(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_AuxDevice._xmlns + "auxDevice")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.description = src.description
        self.model = src.model
        self.manufacturer = src.manufacturer
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("description") is not None:
            dest.description = self.description
        if self._element.get("model") is not None:
            dest.model = self.model
        if self._element.get("manufacturer") is not None:
            dest.manufacturer = self.manufacturer
        if self._element.find(xml_AuxDevice._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def description():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("description"))
        def fset(self, value):
            self._element.set("description", _string_toxml(value))
        return locals()

    @Property
    def model():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("model"))
        def fset(self, value):
            self._element.set("model", _string_toxml(value))
        return locals()

    @Property
    def manufacturer():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("manufacturer"))
        def fset(self, value):
            self._element.set("manufacturer", _string_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_AuxDevice._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_AuxDevice._xmlns + "remark", value)
        return locals()

    # Aggregation: AuxSource
    def _new_source(self):
        return xml_AuxSource(ET.Element(xml_AuxSource._xmlns + "source"))
    @property
    def source(self):
        for e1 in self._element.findall(xml_AuxSource._xmlns + "source"):
            yield xml_AuxSource(e1)


# Inventory::SensorCalibration
class xml_SensorCalibration(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_SensorCalibration._xmlns + "sensorCalibration")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.serialNumber = src.serialNumber
        self.channel = src.channel
        self.start = src.start
        self.end = src.end
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.remark = src.remark

    def _copy_to(self, dest):
        if self._element.get("serialNumber") is not None:
            dest.serialNumber = self.serialNumber
        if self._element.get("channel") is not None:
            dest.channel = self.channel
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.find(xml_SensorCalibration._xmlns + "remark") is not None:
            dest.remark = self.remark

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def serialNumber():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("serialNumber"))
        def fset(self, value):
            self._element.set("serialNumber", _string_toxml(value))
        return locals()

    @Property
    def channel():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("channel"))
        def fset(self, value):
            self._element.set("channel", _int_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_SensorCalibration._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_SensorCalibration._xmlns + "remark", value)
        return locals()


# Inventory::Sensor
class xml_Sensor(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Sensor._xmlns + "sensor")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.description = src.description
        self.model = src.model
        self.manufacturer = src.manufacturer
        self.type = src.type
        self.unit = src.unit
        self.lowFrequency = src.lowFrequency
        self.highFrequency = src.highFrequency
        self.response = src.response
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("description") is not None:
            dest.description = self.description
        if self._element.get("model") is not None:
            dest.model = self.model
        if self._element.get("manufacturer") is not None:
            dest.manufacturer = self.manufacturer
        if self._element.get("type") is not None:
            dest.type = self.type
        if self._element.get("unit") is not None:
            dest.unit = self.unit
        if self._element.get("lowFrequency") is not None:
            dest.lowFrequency = self.lowFrequency
        if self._element.get("highFrequency") is not None:
            dest.highFrequency = self.highFrequency
        if self._element.get("response") is not None:
            dest.response = self.response
        if self._element.find(xml_Sensor._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def description():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("description"))
        def fset(self, value):
            self._element.set("description", _string_toxml(value))
        return locals()

    @Property
    def model():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("model"))
        def fset(self, value):
            self._element.set("model", _string_toxml(value))
        return locals()

    @Property
    def manufacturer():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("manufacturer"))
        def fset(self, value):
            self._element.set("manufacturer", _string_toxml(value))
        return locals()

    @Property
    def type():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("type"))
        def fset(self, value):
            self._element.set("type", _string_toxml(value))
        return locals()

    @Property
    def unit():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("unit"))
        def fset(self, value):
            self._element.set("unit", _string_toxml(value))
        return locals()

    @Property
    def lowFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("lowFrequency"))
        def fset(self, value):
            self._element.set("lowFrequency", _float_toxml(value))
        return locals()

    @Property
    def highFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("highFrequency"))
        def fset(self, value):
            self._element.set("highFrequency", _float_toxml(value))
        return locals()

    @Property
    def response():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("response"))
        def fset(self, value):
            self._element.set("response", _string_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_Sensor._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_Sensor._xmlns + "remark", value)
        return locals()

    # Aggregation: SensorCalibration
    def _new_calibration(self):
        return xml_SensorCalibration(ET.Element(xml_SensorCalibration._xmlns + "calibration"))
    @property
    def calibration(self):
        for e1 in self._element.findall(xml_SensorCalibration._xmlns + "calibration"):
            yield xml_SensorCalibration(e1)


# Inventory::ResponsePAZ
class xml_ResponsePAZ(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_ResponsePAZ._xmlns + "responsePAZ")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.type = src.type
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.normalizationFactor = src.normalizationFactor
        self.normalizationFrequency = src.normalizationFrequency
        self.numberOfZeros = src.numberOfZeros
        self.numberOfPoles = src.numberOfPoles
        self.zeros = src.zeros
        self.poles = src.poles
        self.remark = src.remark
        self.decimationFactor = src.decimationFactor
        self.delay = src.delay
        self.correction = src.correction
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("type") is not None:
            dest.type = self.type
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.get("normalizationFactor") is not None:
            dest.normalizationFactor = self.normalizationFactor
        if self._element.get("normalizationFrequency") is not None:
            dest.normalizationFrequency = self.normalizationFrequency
        if self._element.get("numberOfZeros") is not None:
            dest.numberOfZeros = self.numberOfZeros
        if self._element.get("numberOfPoles") is not None:
            dest.numberOfPoles = self.numberOfPoles
        if self._element.find(xml_ResponsePAZ._xmlns + "zeros") is not None:
            dest.zeros = self.zeros
        if self._element.find(xml_ResponsePAZ._xmlns + "poles") is not None:
            dest.poles = self.poles
        if self._element.find(xml_ResponsePAZ._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("decimationFactor") is not None:
            dest.decimationFactor = self.decimationFactor
        if self._element.get("delay") is not None:
            dest.delay = self.delay
        if self._element.get("correction") is not None:
            dest.correction = self.correction
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def type():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("type"))
        def fset(self, value):
            self._element.set("type", _string_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def normalizationFactor():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("normalizationFactor"))
        def fset(self, value):
            self._element.set("normalizationFactor", _float_toxml(value))
        return locals()

    @Property
    def normalizationFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("normalizationFrequency"))
        def fset(self, value):
            self._element.set("normalizationFrequency", _float_toxml(value))
        return locals()

    @Property
    def numberOfZeros():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("numberOfZeros"))
        def fset(self, value):
            self._element.set("numberOfZeros", _int_toxml(value))
        return locals()

    @Property
    def numberOfPoles():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("numberOfPoles"))
        def fset(self, value):
            self._element.set("numberOfPoles", _int_toxml(value))
        return locals()

    @Property
    def zeros():
    # type: ComplexArray
        def fget(self):
            return _get_blob(self._element, xml_ResponsePAZ._xmlns + "zeros")
        def fset(self, value):
            _set_blob(self._element, xml_ResponsePAZ._xmlns + "zeros", value)
        return locals()

    @Property
    def poles():
    # type: ComplexArray
        def fget(self):
            return _get_blob(self._element, xml_ResponsePAZ._xmlns + "poles")
        def fset(self, value):
            _set_blob(self._element, xml_ResponsePAZ._xmlns + "poles", value)
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_ResponsePAZ._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_ResponsePAZ._xmlns + "remark", value)
        return locals()

    @Property
    def decimationFactor():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("decimationFactor"))
        def fset(self, value):
            self._element.set("decimationFactor", _int_toxml(value))
        return locals()

    @Property
    def delay():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("delay"))
        def fset(self, value):
            self._element.set("delay", _float_toxml(value))
        return locals()

    @Property
    def correction():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("correction"))
        def fset(self, value):
            self._element.set("correction", _float_toxml(value))
        return locals()


# Inventory::ResponsePolynomial
class xml_ResponsePolynomial(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_ResponsePolynomial._xmlns + "responsePolynomial")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.frequencyUnit = src.frequencyUnit
        self.approximationType = src.approximationType
        self.approximationLowerBound = src.approximationLowerBound
        self.approximationUpperBound = src.approximationUpperBound
        self.approximationError = src.approximationError
        self.numberOfCoefficients = src.numberOfCoefficients
        self.coefficients = src.coefficients
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.get("frequencyUnit") is not None:
            dest.frequencyUnit = self.frequencyUnit
        if self._element.get("approximationType") is not None:
            dest.approximationType = self.approximationType
        if self._element.get("approximationLowerBound") is not None:
            dest.approximationLowerBound = self.approximationLowerBound
        if self._element.get("approximationUpperBound") is not None:
            dest.approximationUpperBound = self.approximationUpperBound
        if self._element.get("approximationError") is not None:
            dest.approximationError = self.approximationError
        if self._element.get("numberOfCoefficients") is not None:
            dest.numberOfCoefficients = self.numberOfCoefficients
        if self._element.find(xml_ResponsePolynomial._xmlns + "coefficients") is not None:
            dest.coefficients = self.coefficients
        if self._element.find(xml_ResponsePolynomial._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def frequencyUnit():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("frequencyUnit"))
        def fset(self, value):
            self._element.set("frequencyUnit", _string_toxml(value))
        return locals()

    @Property
    def approximationType():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("approximationType"))
        def fset(self, value):
            self._element.set("approximationType", _string_toxml(value))
        return locals()

    @Property
    def approximationLowerBound():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("approximationLowerBound"))
        def fset(self, value):
            self._element.set("approximationLowerBound", _float_toxml(value))
        return locals()

    @Property
    def approximationUpperBound():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("approximationUpperBound"))
        def fset(self, value):
            self._element.set("approximationUpperBound", _float_toxml(value))
        return locals()

    @Property
    def approximationError():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("approximationError"))
        def fset(self, value):
            self._element.set("approximationError", _float_toxml(value))
        return locals()

    @Property
    def numberOfCoefficients():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("numberOfCoefficients"))
        def fset(self, value):
            self._element.set("numberOfCoefficients", _int_toxml(value))
        return locals()

    @Property
    def coefficients():
    # type: RealArray
        def fget(self):
            return _get_blob(self._element, xml_ResponsePolynomial._xmlns + "coefficients")
        def fset(self, value):
            _set_blob(self._element, xml_ResponsePolynomial._xmlns + "coefficients", value)
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_ResponsePolynomial._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_ResponsePolynomial._xmlns + "remark", value)
        return locals()


# Inventory::ResponseFAP
class xml_ResponseFAP(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_ResponseFAP._xmlns + "responseFAP")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.numberOfTuples = src.numberOfTuples
        self.tuples = src.tuples
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.get("numberOfTuples") is not None:
            dest.numberOfTuples = self.numberOfTuples
        if self._element.find(xml_ResponseFAP._xmlns + "tuples") is not None:
            dest.tuples = self.tuples
        if self._element.find(xml_ResponseFAP._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def numberOfTuples():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("numberOfTuples"))
        def fset(self, value):
            self._element.set("numberOfTuples", _int_toxml(value))
        return locals()

    @Property
    def tuples():
    # type: RealArray
        def fget(self):
            return _get_blob(self._element, xml_ResponseFAP._xmlns + "tuples")
        def fset(self, value):
            _set_blob(self._element, xml_ResponseFAP._xmlns + "tuples", value)
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_ResponseFAP._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_ResponseFAP._xmlns + "remark", value)
        return locals()


# Inventory::ResponseFIR
class xml_ResponseFIR(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_ResponseFIR._xmlns + "responseFIR")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.decimationFactor = src.decimationFactor
        self.delay = src.delay
        self.correction = src.correction
        self.numberOfCoefficients = src.numberOfCoefficients
        self.symmetry = src.symmetry
        self.coefficients = src.coefficients
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.get("decimationFactor") is not None:
            dest.decimationFactor = self.decimationFactor
        if self._element.get("delay") is not None:
            dest.delay = self.delay
        if self._element.get("correction") is not None:
            dest.correction = self.correction
        if self._element.get("numberOfCoefficients") is not None:
            dest.numberOfCoefficients = self.numberOfCoefficients
        if self._element.get("symmetry") is not None:
            dest.symmetry = self.symmetry
        if self._element.find(xml_ResponseFIR._xmlns + "coefficients") is not None:
            dest.coefficients = self.coefficients
        if self._element.find(xml_ResponseFIR._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def decimationFactor():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("decimationFactor"))
        def fset(self, value):
            self._element.set("decimationFactor", _int_toxml(value))
        return locals()

    @Property
    def delay():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("delay"))
        def fset(self, value):
            self._element.set("delay", _float_toxml(value))
        return locals()

    @Property
    def correction():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("correction"))
        def fset(self, value):
            self._element.set("correction", _float_toxml(value))
        return locals()

    @Property
    def numberOfCoefficients():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("numberOfCoefficients"))
        def fset(self, value):
            self._element.set("numberOfCoefficients", _int_toxml(value))
        return locals()

    @Property
    def symmetry():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("symmetry"))
        def fset(self, value):
            self._element.set("symmetry", _string_toxml(value))
        return locals()

    @Property
    def coefficients():
    # type: RealArray
        def fget(self):
            return _get_blob(self._element, xml_ResponseFIR._xmlns + "coefficients")
        def fset(self, value):
            _set_blob(self._element, xml_ResponseFIR._xmlns + "coefficients", value)
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_ResponseFIR._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_ResponseFIR._xmlns + "remark", value)
        return locals()


# Inventory::ResponseIIR
class xml_ResponseIIR(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_ResponseIIR._xmlns + "responseIIR")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.type = src.type
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.decimationFactor = src.decimationFactor
        self.delay = src.delay
        self.correction = src.correction
        self.numberOfNumerators = src.numberOfNumerators
        self.numberOfDenominators = src.numberOfDenominators
        self.numerators = src.numerators
        self.denominators = src.denominators
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("type") is not None:
            dest.type = self.type
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.get("decimationFactor") is not None:
            dest.decimationFactor = self.decimationFactor
        if self._element.get("delay") is not None:
            dest.delay = self.delay
        if self._element.get("correction") is not None:
            dest.correction = self.correction
        if self._element.get("numberOfNumerators") is not None:
            dest.numberOfNumerators = self.numberOfNumerators
        if self._element.get("numberOfDenominators") is not None:
            dest.numberOfDenominators = self.numberOfDenominators
        if self._element.find(xml_ResponseIIR._xmlns + "numerators") is not None:
            dest.numerators = self.numerators
        if self._element.find(xml_ResponseIIR._xmlns + "denominators") is not None:
            dest.denominators = self.denominators
        if self._element.find(xml_ResponseIIR._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def type():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("type"))
        def fset(self, value):
            self._element.set("type", _string_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def decimationFactor():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("decimationFactor"))
        def fset(self, value):
            self._element.set("decimationFactor", _int_toxml(value))
        return locals()

    @Property
    def delay():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("delay"))
        def fset(self, value):
            self._element.set("delay", _float_toxml(value))
        return locals()

    @Property
    def correction():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("correction"))
        def fset(self, value):
            self._element.set("correction", _float_toxml(value))
        return locals()

    @Property
    def numberOfNumerators():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("numberOfNumerators"))
        def fset(self, value):
            self._element.set("numberOfNumerators", _int_toxml(value))
        return locals()

    @Property
    def numberOfDenominators():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("numberOfDenominators"))
        def fset(self, value):
            self._element.set("numberOfDenominators", _int_toxml(value))
        return locals()

    @Property
    def numerators():
    # type: RealArray
        def fget(self):
            return _get_blob(self._element, xml_ResponseIIR._xmlns + "numerators")
        def fset(self, value):
            _set_blob(self._element, xml_ResponseIIR._xmlns + "numerators", value)
        return locals()

    @Property
    def denominators():
    # type: RealArray
        def fget(self):
            return _get_blob(self._element, xml_ResponseIIR._xmlns + "denominators")
        def fset(self, value):
            _set_blob(self._element, xml_ResponseIIR._xmlns + "denominators", value)
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_ResponseIIR._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_ResponseIIR._xmlns + "remark", value)
        return locals()


# Inventory::DataloggerCalibration
class xml_DataloggerCalibration(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_DataloggerCalibration._xmlns + "dataloggerCalibration")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.serialNumber = src.serialNumber
        self.channel = src.channel
        self.start = src.start
        self.end = src.end
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.remark = src.remark

    def _copy_to(self, dest):
        if self._element.get("serialNumber") is not None:
            dest.serialNumber = self.serialNumber
        if self._element.get("channel") is not None:
            dest.channel = self.channel
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.find(xml_DataloggerCalibration._xmlns + "remark") is not None:
            dest.remark = self.remark

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def serialNumber():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("serialNumber"))
        def fset(self, value):
            self._element.set("serialNumber", _string_toxml(value))
        return locals()

    @Property
    def channel():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("channel"))
        def fset(self, value):
            self._element.set("channel", _int_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_DataloggerCalibration._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_DataloggerCalibration._xmlns + "remark", value)
        return locals()


# Inventory::Decimation
class xml_Decimation(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Decimation._xmlns + "decimation")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.sampleRateNumerator = src.sampleRateNumerator
        self.sampleRateDenominator = src.sampleRateDenominator
        self.analogueFilterChain = src.analogueFilterChain
        self.digitalFilterChain = src.digitalFilterChain

    def _copy_to(self, dest):
        if self._element.get("sampleRateNumerator") is not None:
            dest.sampleRateNumerator = self.sampleRateNumerator
        if self._element.get("sampleRateDenominator") is not None:
            dest.sampleRateDenominator = self.sampleRateDenominator
        if self._element.find(xml_Decimation._xmlns + "analogueFilterChain") is not None:
            dest.analogueFilterChain = self.analogueFilterChain
        if self._element.find(xml_Decimation._xmlns + "digitalFilterChain") is not None:
            dest.digitalFilterChain = self.digitalFilterChain

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def sampleRateNumerator():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("sampleRateNumerator"))
        def fset(self, value):
            self._element.set("sampleRateNumerator", _int_toxml(value))
        return locals()

    @Property
    def sampleRateDenominator():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("sampleRateDenominator"))
        def fset(self, value):
            self._element.set("sampleRateDenominator", _int_toxml(value))
        return locals()

    @Property
    def analogueFilterChain():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_Decimation._xmlns + "analogueFilterChain")
        def fset(self, value):
            _set_blob(self._element, xml_Decimation._xmlns + "analogueFilterChain", value)
        return locals()

    @Property
    def digitalFilterChain():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_Decimation._xmlns + "digitalFilterChain")
        def fset(self, value):
            _set_blob(self._element, xml_Decimation._xmlns + "digitalFilterChain", value)
        return locals()


# Inventory::Datalogger
class xml_Datalogger(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Datalogger._xmlns + "datalogger")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.name = src.name
        self.description = src.description
        self.digitizerModel = src.digitizerModel
        self.digitizerManufacturer = src.digitizerManufacturer
        self.recorderModel = src.recorderModel
        self.recorderManufacturer = src.recorderManufacturer
        self.clockModel = src.clockModel
        self.clockManufacturer = src.clockManufacturer
        self.clockType = src.clockType
        self.gain = src.gain
        self.maxClockDrift = src.maxClockDrift
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("name") is not None:
            dest.name = self.name
        if self._element.get("description") is not None:
            dest.description = self.description
        if self._element.get("digitizerModel") is not None:
            dest.digitizerModel = self.digitizerModel
        if self._element.get("digitizerManufacturer") is not None:
            dest.digitizerManufacturer = self.digitizerManufacturer
        if self._element.get("recorderModel") is not None:
            dest.recorderModel = self.recorderModel
        if self._element.get("recorderManufacturer") is not None:
            dest.recorderManufacturer = self.recorderManufacturer
        if self._element.get("clockModel") is not None:
            dest.clockModel = self.clockModel
        if self._element.get("clockManufacturer") is not None:
            dest.clockManufacturer = self.clockManufacturer
        if self._element.get("clockType") is not None:
            dest.clockType = self.clockType
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("maxClockDrift") is not None:
            dest.maxClockDrift = self.maxClockDrift
        if self._element.find(xml_Datalogger._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def name():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("name"))
        def fset(self, value):
            self._element.set("name", _string_toxml(value))
        return locals()

    @Property
    def description():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("description"))
        def fset(self, value):
            self._element.set("description", _string_toxml(value))
        return locals()

    @Property
    def digitizerModel():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("digitizerModel"))
        def fset(self, value):
            self._element.set("digitizerModel", _string_toxml(value))
        return locals()

    @Property
    def digitizerManufacturer():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("digitizerManufacturer"))
        def fset(self, value):
            self._element.set("digitizerManufacturer", _string_toxml(value))
        return locals()

    @Property
    def recorderModel():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("recorderModel"))
        def fset(self, value):
            self._element.set("recorderModel", _string_toxml(value))
        return locals()

    @Property
    def recorderManufacturer():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("recorderManufacturer"))
        def fset(self, value):
            self._element.set("recorderManufacturer", _string_toxml(value))
        return locals()

    @Property
    def clockModel():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("clockModel"))
        def fset(self, value):
            self._element.set("clockModel", _string_toxml(value))
        return locals()

    @Property
    def clockManufacturer():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("clockManufacturer"))
        def fset(self, value):
            self._element.set("clockManufacturer", _string_toxml(value))
        return locals()

    @Property
    def clockType():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("clockType"))
        def fset(self, value):
            self._element.set("clockType", _string_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def maxClockDrift():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("maxClockDrift"))
        def fset(self, value):
            self._element.set("maxClockDrift", _float_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_Datalogger._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_Datalogger._xmlns + "remark", value)
        return locals()

    # Aggregation: DataloggerCalibration
    def _new_calibration(self):
        return xml_DataloggerCalibration(ET.Element(xml_DataloggerCalibration._xmlns + "calibration"))
    @property
    def calibration(self):
        for e1 in self._element.findall(xml_DataloggerCalibration._xmlns + "calibration"):
            yield xml_DataloggerCalibration(e1)

    # Aggregation: Decimation
    def _new_decimation(self):
        return xml_Decimation(ET.Element(xml_Decimation._xmlns + "decimation"))
    @property
    def decimation(self):
        for e1 in self._element.findall(xml_Decimation._xmlns + "decimation"):
            yield xml_Decimation(e1)


# Inventory::AuxStream
class xml_AuxStream(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_AuxStream._xmlns + "auxStream")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.code = src.code
        self.start = src.start
        self.end = src.end
        self.device = src.device
        self.deviceSerialNumber = src.deviceSerialNumber
        self.source = src.source
        self.format = src.format
        self.flags = src.flags
        self.restricted = src.restricted
        self.shared = src.shared

    def _copy_to(self, dest):
        if self._element.get("code") is not None:
            dest.code = self.code
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("device") is not None:
            dest.device = self.device
        if self._element.get("deviceSerialNumber") is not None:
            dest.deviceSerialNumber = self.deviceSerialNumber
        if self._element.get("source") is not None:
            dest.source = self.source
        if self._element.get("format") is not None:
            dest.format = self.format
        if self._element.get("flags") is not None:
            dest.flags = self.flags
        if self._element.get("restricted") is not None:
            dest.restricted = self.restricted
        if self._element.get("shared") is not None:
            dest.shared = self.shared

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def code():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("code"))
        def fset(self, value):
            self._element.set("code", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def device():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("device"))
        def fset(self, value):
            self._element.set("device", _string_toxml(value))
        return locals()

    @Property
    def deviceSerialNumber():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("deviceSerialNumber"))
        def fset(self, value):
            self._element.set("deviceSerialNumber", _string_toxml(value))
        return locals()

    @Property
    def source():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("source"))
        def fset(self, value):
            self._element.set("source", _string_toxml(value))
        return locals()

    @Property
    def format():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("format"))
        def fset(self, value):
            self._element.set("format", _string_toxml(value))
        return locals()

    @Property
    def flags():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("flags"))
        def fset(self, value):
            self._element.set("flags", _string_toxml(value))
        return locals()

    @Property
    def restricted():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("restricted"))
        def fset(self, value):
            self._element.set("restricted", _boolean_toxml(value))
        return locals()

    @Property
    def shared():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("shared"))
        def fset(self, value):
            self._element.set("shared", _boolean_toxml(value))
        return locals()


# Inventory::Stream
class xml_Stream(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Stream._xmlns + "stream")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.code = src.code
        self.start = src.start
        self.end = src.end
        self.datalogger = src.datalogger
        self.dataloggerSerialNumber = src.dataloggerSerialNumber
        self.dataloggerChannel = src.dataloggerChannel
        self.sensor = src.sensor
        self.sensorSerialNumber = src.sensorSerialNumber
        self.sensorChannel = src.sensorChannel
        self.clockSerialNumber = src.clockSerialNumber
        self.sampleRateNumerator = src.sampleRateNumerator
        self.sampleRateDenominator = src.sampleRateDenominator
        self.depth = src.depth
        self.azimuth = src.azimuth
        self.dip = src.dip
        self.gain = src.gain
        self.gainFrequency = src.gainFrequency
        self.gainUnit = src.gainUnit
        self.format = src.format
        self.flags = src.flags
        self.restricted = src.restricted
        self.shared = src.shared
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("code") is not None:
            dest.code = self.code
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("datalogger") is not None:
            dest.datalogger = self.datalogger
        if self._element.get("dataloggerSerialNumber") is not None:
            dest.dataloggerSerialNumber = self.dataloggerSerialNumber
        if self._element.get("dataloggerChannel") is not None:
            dest.dataloggerChannel = self.dataloggerChannel
        if self._element.get("sensor") is not None:
            dest.sensor = self.sensor
        if self._element.get("sensorSerialNumber") is not None:
            dest.sensorSerialNumber = self.sensorSerialNumber
        if self._element.get("sensorChannel") is not None:
            dest.sensorChannel = self.sensorChannel
        if self._element.get("clockSerialNumber") is not None:
            dest.clockSerialNumber = self.clockSerialNumber
        if self._element.get("sampleRateNumerator") is not None:
            dest.sampleRateNumerator = self.sampleRateNumerator
        if self._element.get("sampleRateDenominator") is not None:
            dest.sampleRateDenominator = self.sampleRateDenominator
        if self._element.get("depth") is not None:
            dest.depth = self.depth
        if self._element.get("azimuth") is not None:
            dest.azimuth = self.azimuth
        if self._element.get("dip") is not None:
            dest.dip = self.dip
        if self._element.get("gain") is not None:
            dest.gain = self.gain
        if self._element.get("gainFrequency") is not None:
            dest.gainFrequency = self.gainFrequency
        if self._element.get("gainUnit") is not None:
            dest.gainUnit = self.gainUnit
        if self._element.get("format") is not None:
            dest.format = self.format
        if self._element.get("flags") is not None:
            dest.flags = self.flags
        if self._element.get("restricted") is not None:
            dest.restricted = self.restricted
        if self._element.get("shared") is not None:
            dest.shared = self.shared
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def code():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("code"))
        def fset(self, value):
            self._element.set("code", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def datalogger():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("datalogger"))
        def fset(self, value):
            self._element.set("datalogger", _string_toxml(value))
        return locals()

    @Property
    def dataloggerSerialNumber():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("dataloggerSerialNumber"))
        def fset(self, value):
            self._element.set("dataloggerSerialNumber", _string_toxml(value))
        return locals()

    @Property
    def dataloggerChannel():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("dataloggerChannel"))
        def fset(self, value):
            self._element.set("dataloggerChannel", _int_toxml(value))
        return locals()

    @Property
    def sensor():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("sensor"))
        def fset(self, value):
            self._element.set("sensor", _string_toxml(value))
        return locals()

    @Property
    def sensorSerialNumber():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("sensorSerialNumber"))
        def fset(self, value):
            self._element.set("sensorSerialNumber", _string_toxml(value))
        return locals()

    @Property
    def sensorChannel():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("sensorChannel"))
        def fset(self, value):
            self._element.set("sensorChannel", _int_toxml(value))
        return locals()

    @Property
    def clockSerialNumber():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("clockSerialNumber"))
        def fset(self, value):
            self._element.set("clockSerialNumber", _string_toxml(value))
        return locals()

    @Property
    def sampleRateNumerator():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("sampleRateNumerator"))
        def fset(self, value):
            self._element.set("sampleRateNumerator", _int_toxml(value))
        return locals()

    @Property
    def sampleRateDenominator():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("sampleRateDenominator"))
        def fset(self, value):
            self._element.set("sampleRateDenominator", _int_toxml(value))
        return locals()

    @Property
    def depth():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("depth"))
        def fset(self, value):
            self._element.set("depth", _float_toxml(value))
        return locals()

    @Property
    def azimuth():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("azimuth"))
        def fset(self, value):
            self._element.set("azimuth", _float_toxml(value))
        return locals()

    @Property
    def dip():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("dip"))
        def fset(self, value):
            self._element.set("dip", _float_toxml(value))
        return locals()

    @Property
    def gain():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gain"))
        def fset(self, value):
            self._element.set("gain", _float_toxml(value))
        return locals()

    @Property
    def gainFrequency():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("gainFrequency"))
        def fset(self, value):
            self._element.set("gainFrequency", _float_toxml(value))
        return locals()

    @Property
    def gainUnit():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("gainUnit"))
        def fset(self, value):
            self._element.set("gainUnit", _string_toxml(value))
        return locals()

    @Property
    def format():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("format"))
        def fset(self, value):
            self._element.set("format", _string_toxml(value))
        return locals()

    @Property
    def flags():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("flags"))
        def fset(self, value):
            self._element.set("flags", _string_toxml(value))
        return locals()

    @Property
    def restricted():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("restricted"))
        def fset(self, value):
            self._element.set("restricted", _boolean_toxml(value))
        return locals()

    @Property
    def shared():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("shared"))
        def fset(self, value):
            self._element.set("shared", _boolean_toxml(value))
        return locals()

    # Aggregation: Comment
    def _new_comment(self):
        return xml_Comment(ET.Element(xml_Comment._xmlns + "comment"))
    @property
    def comment(self):
        for e1 in self._element.findall(xml_Comment._xmlns + "comment"):
            yield xml_Comment(e1)


# Inventory::SensorLocation
class xml_SensorLocation(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_SensorLocation._xmlns + "sensorLocation")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.code = src.code
        self.start = src.start
        self.end = src.end
        self.latitude = src.latitude
        self.longitude = src.longitude
        self.elevation = src.elevation
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("code") is not None:
            dest.code = self.code
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("latitude") is not None:
            dest.latitude = self.latitude
        if self._element.get("longitude") is not None:
            dest.longitude = self.longitude
        if self._element.get("elevation") is not None:
            dest.elevation = self.elevation
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def code():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("code"))
        def fset(self, value):
            self._element.set("code", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def latitude():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("latitude"))
        def fset(self, value):
            self._element.set("latitude", _float_toxml(value))
        return locals()

    @Property
    def longitude():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("longitude"))
        def fset(self, value):
            self._element.set("longitude", _float_toxml(value))
        return locals()

    @Property
    def elevation():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("elevation"))
        def fset(self, value):
            self._element.set("elevation", _float_toxml(value))
        return locals()

    # Aggregation: Comment
    def _new_comment(self):
        return xml_Comment(ET.Element(xml_Comment._xmlns + "comment"))
    @property
    def comment(self):
        for e1 in self._element.findall(xml_Comment._xmlns + "comment"):
            yield xml_Comment(e1)

    # Aggregation: AuxStream
    def _new_auxStream(self):
        return xml_AuxStream(ET.Element(xml_AuxStream._xmlns + "auxStream"))
    @property
    def auxStream(self):
        for e1 in self._element.findall(xml_AuxStream._xmlns + "auxStream"):
            yield xml_AuxStream(e1)

    # Aggregation: Stream
    def _new_stream(self):
        return xml_Stream(ET.Element(xml_Stream._xmlns + "stream"))
    @property
    def stream(self):
        for e1 in self._element.findall(xml_Stream._xmlns + "stream"):
            yield xml_Stream(e1)


# Inventory::Station
class xml_Station(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Station._xmlns + "station")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.code = src.code
        self.start = src.start
        self.end = src.end
        self.description = src.description
        self.latitude = src.latitude
        self.longitude = src.longitude
        self.elevation = src.elevation
        self.place = src.place
        self.country = src.country
        self.affiliation = src.affiliation
        self.type = src.type
        self.archive = src.archive
        self.archiveNetworkCode = src.archiveNetworkCode
        self.restricted = src.restricted
        self.shared = src.shared
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("code") is not None:
            dest.code = self.code
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("description") is not None:
            dest.description = self.description
        if self._element.get("latitude") is not None:
            dest.latitude = self.latitude
        if self._element.get("longitude") is not None:
            dest.longitude = self.longitude
        if self._element.get("elevation") is not None:
            dest.elevation = self.elevation
        if self._element.get("place") is not None:
            dest.place = self.place
        if self._element.get("country") is not None:
            dest.country = self.country
        if self._element.get("affiliation") is not None:
            dest.affiliation = self.affiliation
        if self._element.get("type") is not None:
            dest.type = self.type
        if self._element.get("archive") is not None:
            dest.archive = self.archive
        if self._element.get("archiveNetworkCode") is not None:
            dest.archiveNetworkCode = self.archiveNetworkCode
        if self._element.get("restricted") is not None:
            dest.restricted = self.restricted
        if self._element.get("shared") is not None:
            dest.shared = self.shared
        if self._element.find(xml_Station._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def code():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("code"))
        def fset(self, value):
            self._element.set("code", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def description():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("description"))
        def fset(self, value):
            self._element.set("description", _string_toxml(value))
        return locals()

    @Property
    def latitude():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("latitude"))
        def fset(self, value):
            self._element.set("latitude", _float_toxml(value))
        return locals()

    @Property
    def longitude():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("longitude"))
        def fset(self, value):
            self._element.set("longitude", _float_toxml(value))
        return locals()

    @Property
    def elevation():
    # type: float
        def fget(self):
            return _float_fromxml(self._element.get("elevation"))
        def fset(self, value):
            self._element.set("elevation", _float_toxml(value))
        return locals()

    @Property
    def place():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("place"))
        def fset(self, value):
            self._element.set("place", _string_toxml(value))
        return locals()

    @Property
    def country():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("country"))
        def fset(self, value):
            self._element.set("country", _string_toxml(value))
        return locals()

    @Property
    def affiliation():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("affiliation"))
        def fset(self, value):
            self._element.set("affiliation", _string_toxml(value))
        return locals()

    @Property
    def type():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("type"))
        def fset(self, value):
            self._element.set("type", _string_toxml(value))
        return locals()

    @Property
    def archive():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("archive"))
        def fset(self, value):
            self._element.set("archive", _string_toxml(value))
        return locals()

    @Property
    def archiveNetworkCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("archiveNetworkCode"))
        def fset(self, value):
            self._element.set("archiveNetworkCode", _string_toxml(value))
        return locals()

    @Property
    def restricted():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("restricted"))
        def fset(self, value):
            self._element.set("restricted", _boolean_toxml(value))
        return locals()

    @Property
    def shared():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("shared"))
        def fset(self, value):
            self._element.set("shared", _boolean_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_Station._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_Station._xmlns + "remark", value)
        return locals()

    # Aggregation: Comment
    def _new_comment(self):
        return xml_Comment(ET.Element(xml_Comment._xmlns + "comment"))
    @property
    def comment(self):
        for e1 in self._element.findall(xml_Comment._xmlns + "comment"):
            yield xml_Comment(e1)

    # Aggregation: SensorLocation
    def _new_sensorLocation(self):
        return xml_SensorLocation(ET.Element(xml_SensorLocation._xmlns + "sensorLocation"))
    @property
    def sensorLocation(self):
        for e1 in self._element.findall(xml_SensorLocation._xmlns + "sensorLocation"):
            yield xml_SensorLocation(e1)


# Inventory::Network
class xml_Network(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Network._xmlns + "network")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.code = src.code
        self.start = src.start
        self.end = src.end
        self.description = src.description
        self.institutions = src.institutions
        self.region = src.region
        self.type = src.type
        self.netClass = src.netClass
        self.archive = src.archive
        self.restricted = src.restricted
        self.shared = src.shared
        self.remark = src.remark
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("code") is not None:
            dest.code = self.code
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("description") is not None:
            dest.description = self.description
        if self._element.get("institutions") is not None:
            dest.institutions = self.institutions
        if self._element.get("region") is not None:
            dest.region = self.region
        if self._element.get("type") is not None:
            dest.type = self.type
        if self._element.get("netClass") is not None:
            dest.netClass = self.netClass
        if self._element.get("archive") is not None:
            dest.archive = self.archive
        if self._element.get("restricted") is not None:
            dest.restricted = self.restricted
        if self._element.get("shared") is not None:
            dest.shared = self.shared
        if self._element.find(xml_Network._xmlns + "remark") is not None:
            dest.remark = self.remark
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def code():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("code"))
        def fset(self, value):
            self._element.set("code", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def description():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("description"))
        def fset(self, value):
            self._element.set("description", _string_toxml(value))
        return locals()

    @Property
    def institutions():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("institutions"))
        def fset(self, value):
            self._element.set("institutions", _string_toxml(value))
        return locals()

    @Property
    def region():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("region"))
        def fset(self, value):
            self._element.set("region", _string_toxml(value))
        return locals()

    @Property
    def type():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("type"))
        def fset(self, value):
            self._element.set("type", _string_toxml(value))
        return locals()

    @Property
    def netClass():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("netClass"))
        def fset(self, value):
            self._element.set("netClass", _string_toxml(value))
        return locals()

    @Property
    def archive():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("archive"))
        def fset(self, value):
            self._element.set("archive", _string_toxml(value))
        return locals()

    @Property
    def restricted():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("restricted"))
        def fset(self, value):
            self._element.set("restricted", _boolean_toxml(value))
        return locals()

    @Property
    def shared():
    # type: boolean
        def fget(self):
            return _boolean_fromxml(self._element.get("shared"))
        def fset(self, value):
            self._element.set("shared", _boolean_toxml(value))
        return locals()

    @Property
    def remark():
    # type: Blob
        def fget(self):
            return _get_blob(self._element, xml_Network._xmlns + "remark")
        def fset(self, value):
            _set_blob(self._element, xml_Network._xmlns + "remark", value)
        return locals()

    # Aggregation: Comment
    def _new_comment(self):
        return xml_Comment(ET.Element(xml_Comment._xmlns + "comment"))
    @property
    def comment(self):
        for e1 in self._element.findall(xml_Comment._xmlns + "comment"):
            yield xml_Comment(e1)

    # Aggregation: Station
    def _new_station(self):
        return xml_Station(ET.Element(xml_Station._xmlns + "station"))
    @property
    def station(self):
        for e1 in self._element.findall(xml_Station._xmlns + "station"):
            yield xml_Station(e1)


# Inventory::Inventory
class xml_Inventory(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Inventory/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Inventory._xmlns + "inventory")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        pass


    def _copy_to(self, dest):
        pass

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    # Aggregation: StationGroup
    def _new_stationGroup(self):
        return xml_StationGroup(ET.Element(xml_StationGroup._xmlns + "stationGroup"))
    @property
    def stationGroup(self):
        for e1 in self._element.findall(xml_StationGroup._xmlns + "stationGroup"):
            yield xml_StationGroup(e1)

    # Aggregation: AuxDevice
    def _new_auxDevice(self):
        return xml_AuxDevice(ET.Element(xml_AuxDevice._xmlns + "auxDevice"))
    @property
    def auxDevice(self):
        for e1 in self._element.findall(xml_AuxDevice._xmlns + "auxDevice"):
            yield xml_AuxDevice(e1)

    # Aggregation: Sensor
    def _new_sensor(self):
        return xml_Sensor(ET.Element(xml_Sensor._xmlns + "sensor"))
    @property
    def sensor(self):
        for e1 in self._element.findall(xml_Sensor._xmlns + "sensor"):
            yield xml_Sensor(e1)

    # Aggregation: Datalogger
    def _new_datalogger(self):
        return xml_Datalogger(ET.Element(xml_Datalogger._xmlns + "datalogger"))
    @property
    def datalogger(self):
        for e1 in self._element.findall(xml_Datalogger._xmlns + "datalogger"):
            yield xml_Datalogger(e1)

    # Aggregation: ResponsePAZ
    def _new_responsePAZ(self):
        return xml_ResponsePAZ(ET.Element(xml_ResponsePAZ._xmlns + "responsePAZ"))
    @property
    def responsePAZ(self):
        for e1 in self._element.findall(xml_ResponsePAZ._xmlns + "responsePAZ"):
            yield xml_ResponsePAZ(e1)

    # Aggregation: ResponseFIR
    def _new_responseFIR(self):
        return xml_ResponseFIR(ET.Element(xml_ResponseFIR._xmlns + "responseFIR"))
    @property
    def responseFIR(self):
        for e1 in self._element.findall(xml_ResponseFIR._xmlns + "responseFIR"):
            yield xml_ResponseFIR(e1)

    # Aggregation: ResponseIIR
    def _new_responseIIR(self):
        return xml_ResponseIIR(ET.Element(xml_ResponseIIR._xmlns + "responseIIR"))
    @property
    def responseIIR(self):
        for e1 in self._element.findall(xml_ResponseIIR._xmlns + "responseIIR"):
            yield xml_ResponseIIR(e1)

    # Aggregation: ResponsePolynomial
    def _new_responsePolynomial(self):
        return xml_ResponsePolynomial(ET.Element(xml_ResponsePolynomial._xmlns + "responsePolynomial"))
    @property
    def responsePolynomial(self):
        for e1 in self._element.findall(xml_ResponsePolynomial._xmlns + "responsePolynomial"):
            yield xml_ResponsePolynomial(e1)

    # Aggregation: ResponseFAP
    def _new_responseFAP(self):
        return xml_ResponseFAP(ET.Element(xml_ResponseFAP._xmlns + "responseFAP"))
    @property
    def responseFAP(self):
        for e1 in self._element.findall(xml_ResponseFAP._xmlns + "responseFAP"):
            yield xml_ResponseFAP(e1)

    # Aggregation: Network
    def _new_network(self):
        return xml_Network(ET.Element(xml_Network._xmlns + "network"))
    @property
    def network(self):
        for e1 in self._element.findall(xml_Network._xmlns + "network"):
            yield xml_Network(e1)


# Routing::RouteArclink
class xml_RouteArclink(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Routing/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_RouteArclink._xmlns + "routeArclink")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.address = src.address
        self.start = src.start
        self.end = src.end
        self.priority = src.priority

    def _copy_to(self, dest):
        if self._element.get("address") is not None:
            dest.address = self.address
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end
        if self._element.get("priority") is not None:
            dest.priority = self.priority

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def address():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("address"))
        def fset(self, value):
            self._element.set("address", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()

    @Property
    def priority():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("priority"))
        def fset(self, value):
            self._element.set("priority", _int_toxml(value))
        return locals()


# Routing::RouteSeedlink
class xml_RouteSeedlink(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Routing/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_RouteSeedlink._xmlns + "routeSeedlink")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.address = src.address
        self.priority = src.priority

    def _copy_to(self, dest):
        if self._element.get("address") is not None:
            dest.address = self.address
        if self._element.get("priority") is not None:
            dest.priority = self.priority

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def address():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("address"))
        def fset(self, value):
            self._element.set("address", _string_toxml(value))
        return locals()

    @Property
    def priority():
    # type: int
        def fget(self):
            return _int_fromxml(self._element.get("priority"))
        def fset(self, value):
            self._element.set("priority", _int_toxml(value))
        return locals()


# Routing::Route
class xml_Route(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Routing/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Route._xmlns + "route")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.networkCode = src.networkCode
        self.stationCode = src.stationCode
        self.locationCode = src.locationCode
        self.streamCode = src.streamCode
        self.publicID = src.publicID

    def _copy_to(self, dest):
        if self._element.get("networkCode") is not None:
            dest.networkCode = self.networkCode
        if self._element.get("stationCode") is not None:
            dest.stationCode = self.stationCode
        if self._element.get("locationCode") is not None:
            dest.locationCode = self.locationCode
        if self._element.get("streamCode") is not None:
            dest.streamCode = self.streamCode
        if self._element.get("publicID") is not None:
            dest.publicID = self.publicID

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def networkCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("networkCode"))
        def fset(self, value):
            self._element.set("networkCode", _string_toxml(value))
        return locals()

    @Property
    def stationCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("stationCode"))
        def fset(self, value):
            self._element.set("stationCode", _string_toxml(value))
        return locals()

    @Property
    def locationCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("locationCode"))
        def fset(self, value):
            self._element.set("locationCode", _string_toxml(value))
        return locals()

    @Property
    def streamCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("streamCode"))
        def fset(self, value):
            self._element.set("streamCode", _string_toxml(value))
        return locals()

    # Aggregation: RouteArclink
    def _new_arclink(self):
        return xml_RouteArclink(ET.Element(xml_RouteArclink._xmlns + "arclink"))
    @property
    def arclink(self):
        for e1 in self._element.findall(xml_RouteArclink._xmlns + "arclink"):
            yield xml_RouteArclink(e1)

    # Aggregation: RouteSeedlink
    def _new_seedlink(self):
        return xml_RouteSeedlink(ET.Element(xml_RouteSeedlink._xmlns + "seedlink"))
    @property
    def seedlink(self):
        for e1 in self._element.findall(xml_RouteSeedlink._xmlns + "seedlink"):
            yield xml_RouteSeedlink(e1)


# Routing::Access
class xml_Access(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Routing/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Access._xmlns + "access")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        self.networkCode = src.networkCode
        self.stationCode = src.stationCode
        self.locationCode = src.locationCode
        self.streamCode = src.streamCode
        self.user = src.user
        self.start = src.start
        self.end = src.end

    def _copy_to(self, dest):
        if self._element.get("networkCode") is not None:
            dest.networkCode = self.networkCode
        if self._element.get("stationCode") is not None:
            dest.stationCode = self.stationCode
        if self._element.get("locationCode") is not None:
            dest.locationCode = self.locationCode
        if self._element.get("streamCode") is not None:
            dest.streamCode = self.streamCode
        if self._element.get("user") is not None:
            dest.user = self.user
        if self._element.get("start") is not None:
            dest.start = self.start
        if self._element.get("end") is not None:
            dest.end = self.end

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    @Property
    def networkCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("networkCode"))
        def fset(self, value):
            self._element.set("networkCode", _string_toxml(value))
        return locals()

    @Property
    def stationCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("stationCode"))
        def fset(self, value):
            self._element.set("stationCode", _string_toxml(value))
        return locals()

    @Property
    def locationCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("locationCode"))
        def fset(self, value):
            self._element.set("locationCode", _string_toxml(value))
        return locals()

    @Property
    def streamCode():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("streamCode"))
        def fset(self, value):
            self._element.set("streamCode", _string_toxml(value))
        return locals()

    @Property
    def user():
    # type: string
        def fget(self):
            return _string_fromxml(self._element.get("user"))
        def fset(self, value):
            self._element.set("user", _string_toxml(value))
        return locals()

    @Property
    def start():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("start"))
        def fset(self, value):
            self._element.set("start", _datetime_toxml(value))
        return locals()

    @Property
    def end():
    # type: datetime
        def fget(self):
            return _datetime_fromxml(self._element.get("end"))
        def fset(self, value):
            self._element.set("end", _datetime_toxml(value))
        return locals()


# Routing::Routing
class xml_Routing(object):
    _xmlns = "{http://geofon.gfz-potsdam.de/ns/Routing/1.0/}"
    def __init__(self, e = None):
        if e is None:
            self._element = ET.Element(xml_Routing._xmlns + "routing")
        else:
            self._element = e

    def _append_child(self, obj):
        self._element.append(obj._element)

    def _copy_from(self, src):
        pass


    def _copy_to(self, dest):
        pass

    @Property
    def publicID():
        def fget(self):
            return _string_fromxml(self._element.get("publicID"))
        def fset(self, value):
            self._element.set("publicID", _string_toxml(value))
        return locals()

    @Property
    def action():
        def fget(self):
            return _string_fromxml(self._element.get("action"))
        def fset(self, value):
            self._element.set("action", _string_toxml(value))
        return locals()

    # Aggregation: Route
    def _new_route(self):
        return xml_Route(ET.Element(xml_Route._xmlns + "route"))
    @property
    def route(self):
        for e1 in self._element.findall(xml_Route._xmlns + "route"):
            yield xml_Route(e1)

    # Aggregation: Access
    def _new_access(self):
        return xml_Access(ET.Element(xml_Access._xmlns + "access"))
    @property
    def access(self):
        for e1 in self._element.findall(xml_Access._xmlns + "access"):
            yield xml_Access(e1)
