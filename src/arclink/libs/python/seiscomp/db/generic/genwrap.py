# This file was created by a source code generator:
# genxml2wrap.py 
# Do not modify. Change the definition and
# run the generator again!
#
# (c) 2010 Mathias Hoffmann, GFZ Potsdam
#
#
import datetime

class _TrackedObject(object):
	def __init__(self):
		self.__dict__['last_modified'] = datetime.datetime(1970, 1, 1, 0, 0, 0)

	def __setattr__(self, name, value):
		if name not in self.__dict__ or self.__dict__[name] != value:
			self.__dict__[name] = value
			self.__dict__['last_modified'] = datetime.datetime.utcnow()
#
#


# QualityControl::QCLog
class base_QCLog(_TrackedObject):
	publicID = ""
	networkCode = ""
	stationCode = ""
	streamCode = ""
	locationCode = ""
	creatorID = ""
	created = None
	start = None
	end = None
	message = ""

# QualityControl::WaveformQuality
class base_WaveformQuality(_TrackedObject):
	networkCode = ""
	stationCode = ""
	streamCode = ""
	locationCode = ""
	creatorID = ""
	created = None
	start = None
	end = None
	type = ""
	parameter = ""
	value = None
	lowerUncertainty = None
	upperUncertainty = None
	windowLength = None

# QualityControl::Outage
class base_Outage(_TrackedObject):
	networkCode = ""
	stationCode = ""
	streamCode = ""
	locationCode = ""
	creatorID = ""
	created = None
	start = None
	end = None


# Inventory::StationReference
class base_StationReference(_TrackedObject):
	stationID = ""

# Inventory::StationGroup
class base_StationGroup(_TrackedObject):
	publicID = ""
	type = None
	code = ""
	start = None
	end = None
	description = ""
	latitude = None
	longitude = None
	elevation = None
	# StationReference = ""

# Inventory::AuxSource
class base_AuxSource(_TrackedObject):
	name = ""
	description = ""
	unit = ""
	conversion = ""
	sampleRateNumerator = None
	sampleRateDenominator = None
	remark = ""

# Inventory::AuxDevice
class base_AuxDevice(_TrackedObject):
	publicID = ""
	name = ""
	description = ""
	model = ""
	manufacturer = ""
	remark = ""
	# AuxSource = ""

# Inventory::SensorCalibration
class base_SensorCalibration(_TrackedObject):
	serialNumber = ""
	channel = None
	start = None
	end = None
	gain = None
	gainFrequency = None
	remark = ""

# Inventory::Sensor
class base_Sensor(_TrackedObject):
	publicID = ""
	name = ""
	description = ""
	model = ""
	manufacturer = ""
	type = ""
	unit = ""
	lowFrequency = None
	highFrequency = None
	response = ""
	remark = ""
	# SensorCalibration = ""

# Inventory::ResponsePAZ
class base_ResponsePAZ(_TrackedObject):
	publicID = ""
	name = ""
	type = ""
	gain = None
	gainFrequency = None
	normalizationFactor = None
	normalizationFrequency = None
	numberOfZeros = None
	numberOfPoles = None
	zeros = ""
	poles = ""
	remark = ""

# Inventory::ResponsePolynomial
class base_ResponsePolynomial(_TrackedObject):
	publicID = ""
	name = ""
	gain = None
	gainFrequency = None
	frequencyUnit = ""
	approximationType = ""
	approximationLowerBound = None
	approximationUpperBound = None
	approximationError = None
	numberOfCoefficients = None
	coefficients = ""
	remark = ""

# Inventory::DataloggerCalibration
class base_DataloggerCalibration(_TrackedObject):
	serialNumber = ""
	channel = None
	start = None
	end = None
	gain = None
	gainFrequency = None
	remark = ""

# Inventory::Decimation
class base_Decimation(_TrackedObject):
	sampleRateNumerator = None
	sampleRateDenominator = None
	analogueFilterChain = ""
	digitalFilterChain = ""

# Inventory::Datalogger
class base_Datalogger(_TrackedObject):
	publicID = ""
	name = ""
	description = ""
	digitizerModel = ""
	digitizerManufacturer = ""
	recorderModel = ""
	recorderManufacturer = ""
	clockModel = ""
	clockManufacturer = ""
	clockType = ""
	gain = None
	maxClockDrift = None
	remark = ""
	# DataloggerCalibration = ""
	# Decimation = ""

# Inventory::ResponseFIR
class base_ResponseFIR(_TrackedObject):
	publicID = ""
	name = ""
	gain = None
	decimationFactor = None
	delay = None
	correction = None
	numberOfCoefficients = None
	symmetry = ""
	coefficients = ""
	remark = ""

# Inventory::AuxStream
class base_AuxStream(_TrackedObject):
	code = ""
	start = None
	end = None
	device = ""
	deviceSerialNumber = ""
	source = ""
	format = ""
	flags = ""
	restricted = None

# Inventory::Stream
class base_Stream(_TrackedObject):
	code = ""
	start = None
	end = None
	datalogger = ""
	dataloggerSerialNumber = ""
	dataloggerChannel = None
	sensor = ""
	sensorSerialNumber = ""
	sensorChannel = None
	clockSerialNumber = ""
	sampleRateNumerator = None
	sampleRateDenominator = None
	depth = None
	azimuth = None
	dip = None
	gain = None
	gainFrequency = None
	gainUnit = ""
	format = ""
	flags = ""
	restricted = None
	shared = None

# Inventory::SensorLocation
class base_SensorLocation(_TrackedObject):
	publicID = ""
	code = ""
	start = None
	end = None
	latitude = None
	longitude = None
	elevation = None
	# AuxStream = ""
	# Stream = ""

# Inventory::Station
class base_Station(_TrackedObject):
	publicID = ""
	code = ""
	start = None
	end = None
	description = ""
	latitude = None
	longitude = None
	elevation = None
	place = ""
	country = ""
	affiliation = ""
	type = ""
	archive = ""
	archiveNetworkCode = ""
	restricted = None
	shared = None
	remark = ""
	# SensorLocation = ""

# Inventory::Network
class base_Network(_TrackedObject):
	publicID = ""
	code = ""
	start = None
	end = None
	description = ""
	institutions = ""
	region = ""
	type = ""
	netClass = ""
	archive = ""
	restricted = None
	shared = None
	remark = ""
	# Station = ""


# Routing::RouteArclink
class base_RouteArclink(_TrackedObject):
	address = ""
	start = None
	end = None
	priority = None

# Routing::RouteSeedlink
class base_RouteSeedlink(_TrackedObject):
	address = ""
	priority = None

# Routing::Route
class base_Route(_TrackedObject):
	publicID = ""
	networkCode = ""
	stationCode = ""
	locationCode = ""
	streamCode = ""
	# RouteArclink = ""
	# RouteSeedlink = ""

# Routing::Access
class base_Access(_TrackedObject):
	networkCode = ""
	stationCode = ""
	locationCode = ""
	streamCode = ""
	user = ""
	start = None
	end = None


