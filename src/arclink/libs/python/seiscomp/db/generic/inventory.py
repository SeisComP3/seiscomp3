# This file was created by a source code generator:
# genxml2wrap.py 
# Do not modify. Change the definition and
# run the generator again!
#
# (c) 2010 Mathias Hoffmann, GFZ Potsdam
#
#
#import genwrap as _genwrap
import datetime
from seiscomp.db.xmlio import inventory as _xmlio
from seiscomp.db import DBError
#
#


# ---------------------------------------------------------------------------------------
class _StationReference(object):
	__slots__ = (
		"myStationGroup",
		"object",
		"stationID",
		"last_modified",
	)

	def __init__(self, myStationGroup, stationID, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.stationID = ""
		self.myStationGroup = myStationGroup
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.stationID = stationID


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _StationGroup(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"type",
		"code",
		"start",
		"end",
		"description",
		"latitude",
		"longitude",
		"elevation",
		"last_modified",
		"stationReference",
	)

	def __init__(self, my, code, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.type = None
		self.code = ""
		self.start = None
		self.end = None
		self.description = ""
		self.latitude = None
		self.longitude = None
		self.elevation = None
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.code = code

		self.stationReference = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_stationReference(self, stationID, **args):
		if stationID in self.stationReference:
			raise DBError, "StationReference %s already defined" % stationID
		obj = _StationReference(self, stationID, args)
		self.stationReference[stationID] = obj
		return obj

	def remove_stationReference(self, stationID):
		try:
			del self.stationReference[stationID]
		except KeyError:
			raise DBError, "StationReference [%s] not found" % (stationID)
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _AuxSource(object):
	__slots__ = (
		"myAuxDevice",
		"object",
		"name",
		"description",
		"unit",
		"conversion",
		"sampleRateNumerator",
		"sampleRateDenominator",
		"remark",
		"last_modified",
	)

	def __init__(self, myAuxDevice, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.name = ""
		self.description = ""
		self.unit = ""
		self.conversion = ""
		self.sampleRateNumerator = None
		self.sampleRateDenominator = None
		self.remark = ""
		self.myAuxDevice = myAuxDevice
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _AuxDevice(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"name",
		"description",
		"model",
		"manufacturer",
		"remark",
		"last_modified",
		"source",
	)

	def __init__(self, my, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.name = ""
		self.description = ""
		self.model = ""
		self.manufacturer = ""
		self.remark = ""
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name

		self.source = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_source(self, name, **args):
		if name in self.source:
			raise DBError, "AuxSource %s already defined" % name
		obj = _AuxSource(self, name, args)
		self.source[name] = obj
		return obj

	def remove_source(self, name):
		try:
			del self.source[name]
		except KeyError:
			raise DBError, "AuxSource [%s] not found" % (name)
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _SensorCalibration(object):
	__slots__ = (
		"mySensor",
		"object",
		"serialNumber",
		"channel",
		"start",
		"end",
		"gain",
		"gainFrequency",
		"remark",
		"last_modified",
	)

	def __init__(self, mySensor, serialNumber, channel, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.serialNumber = ""
		self.channel = None
		self.start = None
		self.end = None
		self.gain = None
		self.gainFrequency = None
		self.remark = ""
		self.mySensor = mySensor
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.serialNumber = serialNumber
		self.channel = channel
		self.start = start


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Sensor(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"name",
		"description",
		"model",
		"manufacturer",
		"type",
		"unit",
		"lowFrequency",
		"highFrequency",
		"response",
		"remark",
		"last_modified",
		"calibration",
	)

	def __init__(self, my, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.name = ""
		self.description = ""
		self.model = ""
		self.manufacturer = ""
		self.type = ""
		self.unit = ""
		self.lowFrequency = None
		self.highFrequency = None
		self.response = ""
		self.remark = ""
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name

		self.calibration = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_calibration(self, serialNumber, channel, start, **args):
		if serialNumber not in self.calibration:
			self.calibration[serialNumber] = {}
		if channel not in self.calibration[serialNumber]:
			self.calibration[serialNumber][channel] = {}
		if start in self.calibration[serialNumber][channel]:
			raise DBError, "SensorCalibration [%s][%s][%s] already defined" % (serialNumber, channel, start)
		obj = _SensorCalibration(self, serialNumber, channel, start, args)
		self.calibration[serialNumber][channel][start] = obj
		return obj

	def remove_calibration(self, serialNumber, channel, start):
		try:
			del self.calibration[serialNumber][channel][start]
			if len(self.calibration[serialNumber][channel]) == 0:
				del self.calibration[serialNumber][channel]
			if len(self.calibration[serialNumber]) == 0:
				del self.calibration[serialNumber]
		except KeyError:
			raise DBError, "SensorCalibration [%s][%s][%s] not found" % (serialNumber, channel, start)
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _ResponsePAZ(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"name",
		"type",
		"gain",
		"gainFrequency",
		"normalizationFactor",
		"normalizationFrequency",
		"numberOfZeros",
		"numberOfPoles",
		"zeros",
		"poles",
		"remark",
		"decimationFactor",
		"delay",
		"correction",
		"last_modified",
	)

	def __init__(self, my, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.name = ""
		self.type = ""
		self.gain = None
		self.gainFrequency = None
		self.normalizationFactor = None
		self.normalizationFrequency = None
		self.numberOfZeros = None
		self.numberOfPoles = None
		self.zeros = ""
		self.poles = ""
		self.remark = ""
		self.decimationFactor = None
		self.delay = None
		self.correction = None
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _ResponsePolynomial(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"name",
		"gain",
		"gainFrequency",
		"frequencyUnit",
		"approximationType",
		"approximationLowerBound",
		"approximationUpperBound",
		"approximationError",
		"numberOfCoefficients",
		"coefficients",
		"remark",
		"last_modified",
	)

	def __init__(self, my, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.name = ""
		self.gain = None
		self.gainFrequency = None
		self.frequencyUnit = ""
		self.approximationType = ""
		self.approximationLowerBound = None
		self.approximationUpperBound = None
		self.approximationError = None
		self.numberOfCoefficients = None
		self.coefficients = ""
		self.remark = ""
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _ResponseFAP(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"name",
		"gain",
		"gainFrequency",
		"numberOfTuples",
		"tuples",
		"remark",
		"last_modified",
	)

	def __init__(self, my, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.name = ""
		self.gain = None
		self.gainFrequency = None
		self.numberOfTuples = None
		self.tuples = ""
		self.remark = ""
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _DataloggerCalibration(object):
	__slots__ = (
		"myDatalogger",
		"object",
		"serialNumber",
		"channel",
		"start",
		"end",
		"gain",
		"gainFrequency",
		"remark",
		"last_modified",
	)

	def __init__(self, myDatalogger, serialNumber, channel, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.serialNumber = ""
		self.channel = None
		self.start = None
		self.end = None
		self.gain = None
		self.gainFrequency = None
		self.remark = ""
		self.myDatalogger = myDatalogger
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.serialNumber = serialNumber
		self.channel = channel
		self.start = start


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Decimation(object):
	__slots__ = (
		"myDatalogger",
		"object",
		"sampleRateNumerator",
		"sampleRateDenominator",
		"analogueFilterChain",
		"digitalFilterChain",
		"last_modified",
	)

	def __init__(self, myDatalogger, sampleRateNumerator, sampleRateDenominator, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.sampleRateNumerator = None
		self.sampleRateDenominator = None
		self.analogueFilterChain = ""
		self.digitalFilterChain = ""
		self.myDatalogger = myDatalogger
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.sampleRateNumerator = sampleRateNumerator
		self.sampleRateDenominator = sampleRateDenominator


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Datalogger(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"name",
		"description",
		"digitizerModel",
		"digitizerManufacturer",
		"recorderModel",
		"recorderManufacturer",
		"clockModel",
		"clockManufacturer",
		"clockType",
		"gain",
		"maxClockDrift",
		"remark",
		"last_modified",
		"calibration",
		"decimation",
	)

	def __init__(self, my, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.name = ""
		self.description = ""
		self.digitizerModel = ""
		self.digitizerManufacturer = ""
		self.recorderModel = ""
		self.recorderManufacturer = ""
		self.clockModel = ""
		self.clockManufacturer = ""
		self.clockType = ""
		self.gain = None
		self.maxClockDrift = None
		self.remark = ""
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name

		self.calibration = {}
		self.decimation = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_calibration(self, serialNumber, channel, start, **args):
		if serialNumber not in self.calibration:
			self.calibration[serialNumber] = {}
		if channel not in self.calibration[serialNumber]:
			self.calibration[serialNumber][channel] = {}
		if start in self.calibration[serialNumber][channel]:
			raise DBError, "DataloggerCalibration [%s][%s][%s] already defined" % (serialNumber, channel, start)
		obj = _DataloggerCalibration(self, serialNumber, channel, start, args)
		self.calibration[serialNumber][channel][start] = obj
		return obj

	def remove_calibration(self, serialNumber, channel, start):
		try:
			del self.calibration[serialNumber][channel][start]
			if len(self.calibration[serialNumber][channel]) == 0:
				del self.calibration[serialNumber][channel]
			if len(self.calibration[serialNumber]) == 0:
				del self.calibration[serialNumber]
		except KeyError:
			raise DBError, "DataloggerCalibration [%s][%s][%s] not found" % (serialNumber, channel, start)

	def insert_decimation(self, sampleRateNumerator, sampleRateDenominator, **args):
		if sampleRateNumerator not in self.decimation:
			self.decimation[sampleRateNumerator] = {}
		if sampleRateDenominator in self.decimation[sampleRateNumerator]:
			raise DBError, "Decimation [%s][%s] already defined" % (sampleRateNumerator, sampleRateDenominator)
		obj = _Decimation(self, sampleRateNumerator, sampleRateDenominator, args)
		self.decimation[sampleRateNumerator][sampleRateDenominator] = obj
		return obj

	def remove_decimation(self, sampleRateNumerator, sampleRateDenominator):
		try:
			del self.decimation[sampleRateNumerator][sampleRateDenominator]
			if len(self.decimation[sampleRateNumerator]) == 0:
				del self.decimation[sampleRateNumerator]
		except KeyError:
			raise DBError, "Decimation [%s][%s] not found" % (sampleRateNumerator, sampleRateDenominator)
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _ResponseFIR(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"name",
		"gain",
		"decimationFactor",
		"delay",
		"correction",
		"numberOfCoefficients",
		"symmetry",
		"coefficients",
		"remark",
		"last_modified",
	)

	def __init__(self, my, name, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.name = ""
		self.gain = None
		self.decimationFactor = None
		self.delay = None
		self.correction = None
		self.numberOfCoefficients = None
		self.symmetry = ""
		self.coefficients = ""
		self.remark = ""
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.name = name


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _AuxStream(object):
	__slots__ = (
		"mySensorLocation",
		"object",
		"code",
		"start",
		"end",
		"device",
		"deviceSerialNumber",
		"source",
		"format",
		"flags",
		"restricted",
		"shared",
		"last_modified",
	)

	def __init__(self, mySensorLocation, code, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.code = ""
		self.start = None
		self.end = None
		self.device = ""
		self.deviceSerialNumber = ""
		self.source = ""
		self.format = ""
		self.flags = ""
		self.restricted = None
		self.shared = None
		self.mySensorLocation = mySensorLocation
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.code = code
		self.start = start


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Stream(object):
	__slots__ = (
		"mySensorLocation",
		"object",
		"code",
		"start",
		"end",
		"datalogger",
		"dataloggerSerialNumber",
		"dataloggerChannel",
		"sensor",
		"sensorSerialNumber",
		"sensorChannel",
		"clockSerialNumber",
		"sampleRateNumerator",
		"sampleRateDenominator",
		"depth",
		"azimuth",
		"dip",
		"gain",
		"gainFrequency",
		"gainUnit",
		"format",
		"flags",
		"restricted",
		"shared",
		"last_modified",
	)

	def __init__(self, mySensorLocation, code, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.code = ""
		self.start = None
		self.end = None
		self.datalogger = ""
		self.dataloggerSerialNumber = ""
		self.dataloggerChannel = None
		self.sensor = ""
		self.sensorSerialNumber = ""
		self.sensorChannel = None
		self.clockSerialNumber = ""
		self.sampleRateNumerator = None
		self.sampleRateDenominator = None
		self.depth = None
		self.azimuth = None
		self.dip = None
		self.gain = None
		self.gainFrequency = None
		self.gainUnit = ""
		self.format = ""
		self.flags = ""
		self.restricted = None
		self.shared = None
		self.mySensorLocation = mySensorLocation
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.code = code
		self.start = start


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _SensorLocation(object):
	__slots__ = (
		"myStation",
		"object",
		"publicID",
		"code",
		"start",
		"end",
		"latitude",
		"longitude",
		"elevation",
		"last_modified",
		"auxStream",
		"stream",
	)

	def __init__(self, myStation, code, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.code = ""
		self.start = None
		self.end = None
		self.latitude = None
		self.longitude = None
		self.elevation = None
		self.myStation = myStation
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.code = code
		self.start = start

		self.auxStream = {}
		self.stream = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_auxStream(self, code, start, **args):
		if code not in self.auxStream:
			self.auxStream[code] = {}
		if start in self.auxStream[code]:
			raise DBError, "AuxStream [%s][%s] already defined" % (code, start)
		obj = _AuxStream(self, code, start, args)
		self.auxStream[code][start] = obj
		return obj

	def remove_auxStream(self, code, start):
		try:
			del self.auxStream[code][start]
			if len(self.auxStream[code]) == 0:
				del self.auxStream[code]
		except KeyError:
			raise DBError, "AuxStream [%s][%s] not found" % (code, start)

	def insert_stream(self, code, start, **args):
		if code not in self.stream:
			self.stream[code] = {}
		if start in self.stream[code]:
			raise DBError, "Stream [%s][%s] already defined" % (code, start)
		obj = _Stream(self, code, start, args)
		self.stream[code][start] = obj
		return obj

	def remove_stream(self, code, start):
		try:
			del self.stream[code][start]
			if len(self.stream[code]) == 0:
				del self.stream[code]
		except KeyError:
			raise DBError, "Stream [%s][%s] not found" % (code, start)
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Station(object):
	__slots__ = (
		"myNetwork",
		"object",
		"publicID",
		"code",
		"start",
		"end",
		"description",
		"latitude",
		"longitude",
		"elevation",
		"place",
		"country",
		"affiliation",
		"type",
		"archive",
		"archiveNetworkCode",
		"restricted",
		"shared",
		"remark",
		"last_modified",
		"sensorLocation",
	)

	def __init__(self, myNetwork, code, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.code = ""
		self.start = None
		self.end = None
		self.description = ""
		self.latitude = None
		self.longitude = None
		self.elevation = None
		self.place = ""
		self.country = ""
		self.affiliation = ""
		self.type = ""
		self.archive = ""
		self.archiveNetworkCode = ""
		self.restricted = None
		self.shared = None
		self.remark = ""
		self.myNetwork = myNetwork
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.code = code
		self.start = start

		self.sensorLocation = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_sensorLocation(self, code, start, **args):
		if code not in self.sensorLocation:
			self.sensorLocation[code] = {}
		if start in self.sensorLocation[code]:
			raise DBError, "SensorLocation [%s][%s] already defined" % (code, start)
		obj = _SensorLocation(self, code, start, args)
		self.sensorLocation[code][start] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_sensorLocation(self, code, start):
		try:
			del self.sensorLocation[code][start]
			if len(self.sensorLocation[code]) == 0:
				del self.sensorLocation[code]
		except KeyError:
			raise DBError, "SensorLocation [%s][%s] not found" % (code, start)
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Network(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"code",
		"start",
		"end",
		"description",
		"institutions",
		"region",
		"type",
		"netClass",
		"archive",
		"restricted",
		"shared",
		"remark",
		"last_modified",
		"station",
	)

	def __init__(self, my, code, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.code = ""
		self.start = None
		self.end = None
		self.description = ""
		self.institutions = ""
		self.region = ""
		self.type = ""
		self.netClass = ""
		self.archive = ""
		self.restricted = None
		self.shared = None
		self.remark = ""
		self.my = my
		self.object = {}

		for (a, v) in args.iteritems():
			self.__setattr__(a, v)

		self.code = code
		self.start = start

		self.station = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_station(self, code, start, **args):
		if code not in self.station:
			self.station[code] = {}
		if start in self.station[code]:
			raise DBError, "Station [%s][%s] already defined" % (code, start)
		obj = _Station(self, code, start, args)
		self.station[code][start] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_station(self, code, start):
		try:
			del self.station[code][start]
			if len(self.station[code]) == 0:
				del self.station[code]
		except KeyError:
			raise DBError, "Station [%s][%s] not found" % (code, start)
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class Inventory(object):
	__slots__ = (
		"object",
		"publicID",
		"last_modified",
		"stationGroup",
		"auxDevice",
		"sensor",
		"datalogger",
		"responsePAZ",
		"responseFIR",
		"responsePolynomial",
		"responseFAP",
		"network",
	)

	def __init__(self):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.object = {}


		self.stationGroup = {}
		self.auxDevice = {}
		self.sensor = {}
		self.datalogger = {}
		self.responsePAZ = {}
		self.responseFIR = {}
		self.responsePolynomial = {}
		self.responseFAP = {}
		self.network = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_stationGroup(self, code, **args):
		if code in self.stationGroup:
			raise DBError, "StationGroup %s already defined" % code
		obj = _StationGroup(self, code, args)
		self.stationGroup[code] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_stationGroup(self, code):
		try:
			del self.stationGroup[code]
		except KeyError:
			raise DBError, "StationGroup [%s] not found" % (code)

	def insert_auxDevice(self, name, **args):
		if name in self.auxDevice:
			raise DBError, "AuxDevice %s already defined" % name
		obj = _AuxDevice(self, name, args)
		self.auxDevice[name] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_auxDevice(self, name):
		try:
			del self.auxDevice[name]
		except KeyError:
			raise DBError, "AuxDevice [%s] not found" % (name)

	def insert_sensor(self, name, **args):
		if name in self.sensor:
			raise DBError, "Sensor %s already defined" % name
		obj = _Sensor(self, name, args)
		self.sensor[name] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_sensor(self, name):
		try:
			del self.sensor[name]
		except KeyError:
			raise DBError, "Sensor [%s] not found" % (name)

	def insert_datalogger(self, name, **args):
		if name in self.datalogger:
			raise DBError, "Datalogger %s already defined" % name
		obj = _Datalogger(self, name, args)
		self.datalogger[name] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_datalogger(self, name):
		try:
			del self.datalogger[name]
		except KeyError:
			raise DBError, "Datalogger [%s] not found" % (name)

	def insert_responsePAZ(self, name, **args):
		if name in self.responsePAZ:
			raise DBError, "ResponsePAZ %s already defined" % name
		obj = _ResponsePAZ(self, name, args)
		self.responsePAZ[name] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_responsePAZ(self, name):
		try:
			del self.responsePAZ[name]
		except KeyError:
			raise DBError, "ResponsePAZ [%s] not found" % (name)

	def insert_responseFIR(self, name, **args):
		if name in self.responseFIR:
			raise DBError, "ResponseFIR %s already defined" % name
		obj = _ResponseFIR(self, name, args)
		self.responseFIR[name] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_responseFIR(self, name):
		try:
			del self.responseFIR[name]
		except KeyError:
			raise DBError, "ResponseFIR [%s] not found" % (name)

	def insert_responsePolynomial(self, name, **args):
		if name in self.responsePolynomial:
			raise DBError, "ResponsePolynomial %s already defined" % name
		obj = _ResponsePolynomial(self, name, args)
		self.responsePolynomial[name] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_responsePolynomial(self, name):
		try:
			del self.responsePolynomial[name]
		except KeyError:
			raise DBError, "ResponsePolynomial [%s] not found" % (name)

	def insert_responseFAP(self, name, **args):
		if name in self.responseFAP:
			raise DBError, "ResponseFAP %s already defined" % name
		obj = _ResponseFAP(self, name, args)
		self.responseFAP[name] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_responseFAP(self, name):
		try:
			del self.responseFAP[name]
		except KeyError:
			raise DBError, "ResponseFAP [%s] not found" % (name)

	def insert_network(self, code, start, **args):
		if code not in self.network:
			self.network[code] = {}
		if start in self.network[code]:
			raise DBError, "Network [%s][%s] already defined" % (code, start)
		obj = _Network(self, code, start, args)
		self.network[code][start] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_network(self, code, start):
		try:
			del self.network[code][start]
			if len(self.network[code]) == 0:
				del self.network[code]
		except KeyError:
			raise DBError, "Network [%s][%s] not found" % (code, start)

	def clear_instruments(self):
		self.stationGroup = {}
		self.auxDevice = {}
		self.sensor = {}
		self.datalogger = {}
		self.responsePAZ = {}
		self.responseFIR = {}
		self.responsePolynomial = {}
		self.responseFAP = {}

	def clear_stations(self):
		self.network = {}

	def load_xml(self, src):
		_xmlio.xml_in(self, src)

	def save_xml(self, dest, instr=0, modified_after=None, stylesheet=None):
		_xmlio.xml_out(self, dest, instr, modified_after, stylesheet)

	def make_parser(self):
		return _xmlio.make_parser(self)
# ---------------------------------------------------------------------------------------





