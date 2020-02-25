from __future__ import print_function
from .lineType import Sl, Nw, Sr, Sg
from .nodesi import Instruments
from .basesc3 import sc3
import sys

debug = 0

class DontFit(Exception):
	def __init__(self, message):
		Exception.__init__(self, message)

class nslc(object):
	def __init__(self):
		self.start = None
		self.end = None
		self.code = None

	def __overlap__(self, another):
		if self.end:
			if self.end > another.start:
				if not another.end or self.start < another.end:
					return True
		else:
			if not another.end or self.start < another.end:
				return True
		return False

	def _span(self):
		return "%s / %s" % (self.start, self.end) 

	def sc3Att(self):
		att = {}

		att['Start'] = self.start
		if self.end:
			att['End'] =  self.end
		att['Code'] = self.code

		for (key,value) in self.att.items():
			if not self.sc3ValidKey(key) or key in att:
				print("[%s] type %s ignoring attribute %s = %s " % (self.code, self.sc3Mode, key,value), file=sys.stderr)
				continue

			att[key] = value
		return att

def _cmptime(t1, t2):
	if t1 is None and t2 is None:
		return 0
	elif t2 is None or (t1 is not None and t1 < t2):
		return -1
	elif t1 is None or (t2 is not None and t1 > t2):
		return 1
	return 0

class StationGroup(nslc,sc3):
	def __str__(self):
		return "%s" % (self.code)

	def __init__(self, sg):
		if not isinstance(sg,Sg):
			return False

		self.stationReferences = []
		sc3.__init__(self, 'stationGroup', self.stationReferences)

		self.code = sg.code
		self.start = sg.start
		self.end = sg.end
		self.att = sg.getStationGroupAttributes()
		self.srdata = []

	def __match__(self, sr):
		if not isinstance(sr,Sr):
			return False
		
		return (_cmptime(sr.start, self.end) <= 0 and _cmptime(sr.end, self.start) >= 0)

	def conflict(self, another):
		if self.code != another.code:
			return False
		
		if self.end:
			if self.end <= another.start:
				return False
			if another.end and another.end <= self.start:
				return False
		else:
			if another.end and another.end <= self.start:
				return False
		
		return True

	def Sr(self, sr):
		self.srdata.append((sr.ncode, sr.scode, sr.start, sr.end))

	def sc3Resolv(self, inventory):
		for (ncode, scode, start, end) in self.srdata:
			for stationID in inventory.resolveStation(ncode, scode, start, end):
				st = StationReference(self, stationID)
				self.stationReferences.append(st)

class StationReference(sc3):
	def __str__(self):
		return "%s" % (self.att["StationID"])

	def __init__(self, stationGroup, stationID):
		self.stationGroup = stationGroup
		sc3.__init__(self, 'stationReference')

		self.att = { "StationID": stationID }

	def sc3Att(self):
		return self.att

class Network(nslc, sc3):
	def __str__(self):
		return "%s" % (self.code)

	def __init__(self, nw):
		if not isinstance(nw,Nw):
			return False

		self.stations = []
		sc3.__init__(self, 'network', self.stations)

		nslc.__init__(self)
		self.code = nw.code
		self.start = nw.start
		self.end = nw.end
		self.att = nw.getNetworkAttributes()

	def __match__(self, sl):
		if not isinstance(sl,Sl):
			return False
		
		if sl.start < self.start:
			return False
		if self.end:
			if not sl.end or sl.end > self.end:
				return False 
		return True

	def conflict(self, another):
		if self.code != another.code:
			return False
		
		if self.end:
			if self.end <= another.start:
				return False
			if another.end and another.end <= self.start:
				return False
		else:
			if another.end and another.end <= self.start:
				return False
		
		return True

	def Sl(self, sl):
		if not self.__match__(sl):
			raise DontFit(" Object doesn't fit this network object.")
		inserted = False
		for sta in self.stations:
			try:
				where = "%s" % (sta._span())
				sta.Sl(sl)
				if debug: print("[%s] inserted at %s -> %s" % (self, where, sta._span()), file=sys.stderr)
				inserted = True
				for other in self.stations:
					if other is sta: continue
					if other.conflict(sta):
						raise Exception("I Station conflict with already existing station (%s/%s/%s)" % (other, other.start, other.end))
				break
			except DontFit:
				pass
		if not inserted:
			st = Station(self, sl)
			if debug: print("[%s] created new station %s %s" % (self, st, st._span()), file=sys.stderr)
			for sta in self.stations:
				if sta.conflict(st):
					raise Exception("Station conflict with already existing station (%s/%s/%s)" % (sta, sta.start, sta.end))
			self.stations.append(st)

	def check(self, i):
		error = []
		for station in self.stations:
			error.extend(station.check(i))
		return error
	
	def use(self, iid):
		c = False
		for station in self.stations:
			c = c or station.use(iid)
			if c: break
		return c

class Station(nslc, sc3):
	def __str__(self):
		return "%s.%s" % (self.network.code, self.code)

	def __init__(self, network, sl):
		if not isinstance(sl,Sl):
			return False

		self.locations = []
		self.network = network
		sc3.__init__(self, 'station', self.locations)

		# I load myself as a station
		nslc.__init__(self)
		self.code = sl.code
		self.start = sl.start
		self.end = sl.end
		self.att = sl.getStationAttributes()

		# Further parse to generate my locations
		self.Sl(sl)
	
	def __match__(self, obj):
		if not isinstance(obj,Sl):
			return False
		# Check code
		if obj.code != self.code:
			return False
		# Attributes
		att = obj.getStationAttributes()
		for at in att:
			# Make sure that all attributes in Sl-line are here
			if at not in self.att:
				return False
			# And they match
			if att[at] != self.att[at]:
				return False
		# Make sure that there is no other attribute here that is not on Sl-line
		for at in self.att:
			if at not in att:
				return False

		return True
	
	def __adjustTime__(self, sl):
		if sl.start < self.start:
			self.start = sl.start
		if not self.end:
			return
		if sl.end and sl.end < self.end:
			return
		self.end = sl.end

	def conflict(self, another):
		if not isinstance(another, Station):
			raise Exception("Cannot compare myself with %s" % type(another))
		if self.code != another.code:
			return False
		if not self.__overlap__(another):
			return False
		return True

	def use(self, iid):
		c = False
		for location in self.locations:
			c = c or location.use(iid)
			if c: break
		return c

	def check(self, i):
		error = []
		for location in self.locations:
			error.extend(location.check(i))
		return error

	def Sl(self, sl):
		if not self.__match__(sl):
			raise DontFit(" sl doesn't fit this station %s/%s_%s." % (self.code, self.start, self.end))
		# Handle Time Adjustments
		self.__adjustTime__(sl)
		# Handle Locations
		inserted = False
		for loc in self.locations:
			try:
				where = loc._span()
				loc.Sl(sl)
				if debug: print(" [%s] inserted at %s -> %s" % (self, where, loc._span()), file=sys.stderr)
				inserted = True
				for other in self.locations:
					if other is loc: continue
					if other.conflict(loc):
						raise Exception("Location conflict with already existing location")
				break
			except DontFit:
				pass
		
		if not inserted:
			loc = Location(self, sl)
			if debug: print(" [%s] created new location %s %s" % (self, loc, loc._span()), file=sys.stderr)
			for lc in self.locations:
				if lc.conflict(loc):
					raise Exception("Location conflict with already existing location")
			self.locations.append(loc)

	def sc3Att(self):
		att = nslc.sc3Att(self)
		
		## Make sure that we set the Remark
		if 'ArchiveNetworkCode' not in att:
			att['ArchiveNetworkCode'] = self.network.code

		if 'Remark' not in att:
			att['Remark'] = ""
		return att

class Location(nslc, sc3):
	def __str__(self):
		return "%s.%s.%s" % (self.station.network.code, self.station.code, self.code)

	def __init__(self, station, sl):
		if not isinstance(sl, Sl):
			return False
		self.channels = []
		sc3.__init__(self, 'location', self.channels)
		
		nslc.__init__(self)
		self.station = station
		self.code = sl.location
		self.start = sl.start
		self.end = sl.end
		self.att = sl.getLocationAttributes()
		self.Sl(sl)
	
	def __adjustTime__(self, sl):
		if sl.start < self.start:
			self.start = sl.start
		if not self.end:
			return
		if sl.end and sl.end < self.end:
			return
		self.end = sl.end
	
	def __match__(self, obj):
		if not isinstance(obj, Sl):
			return False
		if obj.location != self.code:
			return False
		# Attributes
		att = obj.getLocationAttributes()
		for at in att:
			# Make sure that all attributes in Sl-line are here
			if at not in self.att:
				return False
			# And they match
			if att[at] != self.att[at]:
				return False
		# Make sure that there is no other attribute here that is not on Sl-line
		for at in self.att:
			if at not in att:
				return False
		return True

	def conflict(self, another):
		if not isinstance(another, Location):
			raise Exception("Cannot compare myself with %s" % type(another))
		if self.code != another.code:
			return False
		if not self.__overlap__(another):
			return False
		return True

	def use(self, iid):
		c = False
		for channel in self.channels:
			c = c or channel.use(iid)
			if c: break
		return c

	def check(self, i):
		error = []
		for channel in self.channels:
			error.extend(channel.check(i))
		return error

	def Sl(self, sl):
		if not self.__match__(sl):
			raise DontFit(" This obj doesn't match this Location '%s'" % self.code)
		
		# Handle Time Adjustments
		self.__adjustTime__(sl)
		
		# Create Channels
		for code in sl.channels:
			channel = (Channel(self, code, sl))
			if debug: print("  [%s] created new channel %s/%s" % (self, channel, channel._span()), file=sys.stderr)
			for echan in self.channels:
				if echan.conflict(channel):
					raise Exception("[%s] channel %s conflict with already existing channel" % (self, code))
			#print >>sys.stderr," Channel %s appended at '%s'" % (code, self.code)
			self.channels.append(channel)

class Channel(nslc, sc3):
	def __str__(self):
		return "%s.%s.%s.%s" % (self.location.station.network.code, self.location.station.code, self.location.code, self.code)

	def __init__(self, location, code, sl):
		sc3.__init__(self, 'channel')
		self.location = location

		nslc.__init__(self)
		self.code = code
		self.start = sl.start
		self.end = sl.end
		self.att = sl.getChannelAttributes(self.code)
		
		## Bring the Instrument gains to the channel level
		self._sensorGain = sl.sensorGain
		self._dataloggerGain = sl.dataloggerGain
	
	def conflict(self, another):
		if not isinstance(another, Channel):
			raise Exception("Cannot compare myself with %s" % type(another))
		if self.code != another.code:
			return False
		if not self.__overlap__(another):
			return False
		return True

	def use(self, iid):
		if 'Datalogger' in self.att and iid == self.att['Datalogger']: return True
		if 'Sesor' in self.att and iid == self.att['Sensor']: return True
		return False

	def check(self, i):
		good = []
		
		if not isinstance(i, Instruments):
			raise Exception("Invalid instrument object")

		if not self.att['Datalogger'] in i.keys:
			good.append("no Datalogger") 
			
		if not self.att['Sensor'] in i.keys:
			good.append("no Sensor") 
		
		if good:
			good = [ " [%s] %s" % (self, "/".join(good)) ]
		
		return good

	def sc3Resolv(self, inventory):
		if not inventory:
			print("[%s] Warning, inventory not supplied" % self.code, file=sys.stderr) 
			return
	
		try:
			ssm = self.att['Sensor']
			ssg = self._sensorGain
			sch = self.att['SensorChannel']
			ssn = self.att["SensorSerialNumber"] if "SensorSerialNumber" in self.att else None
			# Sensor publicID
			ss = inventory.sensorID(ssm, ssg)
			self.att['Sensor'] = ss
			
			# Sensor Calibration
			inventory.loadSensorCalibrations(ssm, ssn, sch, ssg, self.start, self.end, ss)
		except Exception as e:
			print("[%s] Sensor Resolution Error %s" % (self, e), file=sys.stderr)
			ss = None
		
		try:
			dsm = self.att['Datalogger']
			dsg = self._dataloggerGain
			dch = self.att['DataloggerChannel']
			dsn = self.att['DataloggerSerialNumber'] if 'DataloggerSerialNumber' in self.att else None

			dt = inventory.dataloggerID(dsm, dsg)
			self.att['Datalogger'] = dt
			inventory.loadDataloggerCalibrations(dsm, dsn, dch, dsg, self.start, self.end, dt)
		except Exception as e:
			print("[%s] Datalogger Resolution Error %s" % (self, e), file=sys.stderr)
			dt = None
		
		try:
			up = self.att['SampleRateNumerator']
			down = self.att['SampleRateDenominator']
			self.att.update(inventory.getChannelGainAttribute(dt, ss, dsn, ssn, dch, sch, up, down, self.start))
		except Exception as e:
			print("[%s] Cannot find gain back for the channel: %s" % (self,e), file=sys.stderr)
