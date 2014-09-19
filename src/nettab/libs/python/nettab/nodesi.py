from lineType import Dl, Se, Ff, Pz, Cl
from basesc3 import sc3
import sys

class prefixable(object):
	def adjust(self, prefix):
		if prefix:
			self.id = "%s:%s" % (prefix, self.id)

class Instruments(object):
	def __init__(self, prefix=""):
		self.keys = []
		self.ses = {}
		self.dls = {}
		self.fls = {}
		self.cls = {}
		self._sensors = {}
		self._datalogger = {}
		self._filters = {}
		self._Cal = {}
		self._prefix = prefix

	def sc3Objs(self):
		objs = []
		
		for s in self._sensors.values():
			objs.append(s.sc3Obj(self))

		for s in self._datalogger.values():
			objs.append(s.sc3Obj(self))

		for s in self._filters.values():
			objs.append(s.sc3Obj(self))

		return objs

	def add(self, obj):
		where = None

		if isinstance(obj, Se):
			where = self.ses
		elif isinstance(obj, Dl):
			where = self.dls
		elif isinstance(obj, Cl):
			where = self.cls 
		elif isinstance(obj, Ff) or isinstance(obj, Pz):
			where = self.fls
		else:
			raise Exception("Object type %s doesn't fir this class" % type(obj))
		
		if obj.id in self.keys:
			raise Exception("Object id %s already exist." % (obj)) 

		self.keys.append(obj.id)
		where[obj.id] = obj
	
		return
	
	def instrumentId(self, iid, gain):
		if gain is None:
			if iid in self.dls:
				gain = self.dls[iid].gain
			elif iid in self.ses:
				gain = self.ses[iid].gain
			else:
				raise Exception("Instrument iid not found")

		siid = "%s/g=%s" % (iid, int(float(gain)))
		return siid
	
	def loadDataloggerCalibrations(self, dsm, dsn, dch, dsg, start, end, dd):
		cls = []
		for cl in self.cls.itervalues():
			if cl.type != "L": continue
			if cl.match(dsm, dsn):
				cls.append(Calibration(cl, dch, start, end))

		if len(cls) == 0:
			if dsn in self.cls:
				print >> sys.stderr,"[%s] No calibrations found for serial number %s and model %s " % (dsm, dsn, dsm)
			return

		diid = self.instrumentId(dsm, dsg)
		try:
			datalogger = self._datalogger[diid].sc3Obj(self)
			if dd != datalogger.publicID():
				raise Exception("Public Id doesn't match")
		except:
			raise Exception("[%s] Could not retrieve datalogger %s" % (dsm, diid))
	
		for cl in cls:
			if (dsm, dsn, dch, start, end) in self._Cal: 
				## print >> sys.stderr,"[%s] Skiping calibration channel %s" %  (dsm, cl.channel)
				continue
			## print >> sys.stderr,"[%s] Adding calibration %s (%s)" % (dsm, cl.channel, dd)
			datalogger.add(cl.sc3Obj(self))
			self._Cal[(dsm, dsn, dch, start, end)] = cl

	def loadSensorCalibrations(self, ssm, ssn, sch, ssg, start, end, ss):
		cls = []
		for cl in self.cls.itervalues():
			if cl.type != "S": continue
			if cl.match(ssm, ssn):
				cls.append(Calibration(cl, sch, start, end))

		if len(cls) == 0:
			if ssn in self.cls:
				print >> sys.stderr,"[%s] No calibrations found for serial number %s and model %s " % (ssm,ssn, ssm)
			return
		
		siid = self.instrumentId(ssm, ssg)	
		try:
			sensor = self._sensors[siid].sc3Obj(self)
			if ss != sensor.publicID():
				raise Exception("Public Id doesn't match")
		except:
			raise Exception("[%s] Could not retrieve sensor %s" % (ssm, siid))

		for cl in cls:
			if (ssm, ssn, sch, start, end) in self._Cal: 
				## print >> sys.stderr,"[%s] Skiping calibration channel %s" %  (ssm, cl.channel)
				continue
			## print >> sys.stderr,"[%s] Adding calibration %s channel %s start %s" % (ssm, ssn, cl.channel, start)
			sensor.add(cl.sc3Obj(self))
			self._Cal[(ssm, ssn, sch, start, end)] = cl
	
	def check(self, networks):
		error = []
		
		# Dataloggers check
		error.append("* Dataloggers:")
		for dl in self.dls.itervalues():
			error.extend(dl.check(self))
		error.append("")
		
		# Check fir filters
		error.append("* Filters:")
		for f in self.fls.itervalues():
			c = False
			for dl in self.dls.itervalues():
				c = c or dl.use(f)
				if c: break
			if not c: error.append(" [%s] filter is not used" % f.id)
		error.append("")


		# Check the calibrations
		error.append("* Calibrations:")
		for cl in self.cls.itervalues():
			error.extend(cl.check(self))
		error.append("")
		
		
		error.append("* Sensors:")
		for f in self.ses.itervalues():
			c = False
			for network in networks.itervalues():
				for station in network.stations:
					for location in station.locations:
						for channel in location.channels:
							c = c or channel.use(f)
							if c: break
						if c: break
					if c: break
				if c: break
			if not c: error.append(" [%s] sensor is not used" % f.id)
		error.append("")

		error.append("* Dataloggers:")
		for f in self.dls.itervalues():
			c = False
			for network in networks.itervalues():
				c = c or network.use(f)
				if c: break
			if not c: error.append(" [%s] datalogger is not used" % f.id)
		error.append("")
		
		return error

	def filterType(self, iid):
		if iid not in self.keys:
			raise Exception("[%s] Filter id not found" % iid)
		
		if iid not in self.fls:
			raise Exception("[%s] Object is not a filter" % iid)
	
		obj = self.fls[iid]
		if isinstance(obj, Ff):
			fType = 'D'
		elif isinstance(obj, Pz):
			fType = obj.type
		
		return fType
	
	def filterID(self, iid):
		if iid not in self.keys:
			raise Exception("[%s] Filter id not found" % iid)
		
		if iid not in self.fls:
			raise Exception("[%s] Object is not a filter" % iid)
	
		if iid not in self._filters:
			obj = self.fls[iid]
			if isinstance(obj, Pz):
				## print >> sys.stderr," Generating new Filter (PZ): %s %s" % (iid,obj.type)
				newFilter = Paz(obj)
			elif isinstance(obj, Ff):
				## print >> sys.stderr," Generating new Filter (Fir): %s" % (iid)
				newFilter = Fir(obj)
			newFilter.adjust(self._prefix)
			if newFilter.id != self.prefix(iid):
				raise Exception("Invalid filter created %s" % (iid))
			self._filters[iid] = newFilter

		return self._filters[iid].sc3ID(self)
	
	def prefix(self, iid):
		if self._prefix:
			iid = "%s:%s" % (self._prefix, iid)
		return iid

	def dataloggerID(self, iid, gain = None):
		if iid not in self.keys:
			raise Exception("Object not found.")

		if iid not in self.dls:
			raise Exception("[%s] Object is not a datalogger" % iid)

		diid = self.instrumentId(iid, gain)
		
		if diid not in self._datalogger:
			## print >> sys.stderr,"Generating datalogger %s -> %s" % (iid, diid)
			newDatalogger = Dataloger(self.dls[iid], gain)
			newDatalogger.adjust(self._prefix)
			if newDatalogger.id != self.prefix(diid):
				raise Exception("Invalid datalogger created %s %s" % (iid, diid))
			self._datalogger[diid] = newDatalogger
		
		return self._datalogger[diid].sc3ID(self)
		
	def sensorID(self, iid, gain = None):
		if iid not in self.keys:
			raise Exception("Object not found.")

		if iid not in self.ses:
			raise Exception("[%s] Object is not a sensor" % iid)

		diid = self.instrumentId(iid, gain)

		if diid not in self._sensors:
			## print >> sys.stderr,"Generating Sensor %s -> %s" % (iid, diid)
			newSensor = Sensor(self.ses[iid], gain)
			newSensor.adjust(self._prefix)
			if newSensor.id != self.prefix(diid):
				raise Exception("Invalid sensor created %s %s" % (iid, diid))
			self._sensors[diid] = newSensor
		
		return self._sensors[diid].sc3ID(self)

	def _findObject(self, objID, where):
		obj = None
		for ob in where.itervalues():
			obj = ob.sc3Obj(self)
			if obj.publicID() == objID:
				break;
		if not obj:
			raise Exception("Object not found: %s " % objID)
		return obj

	def _findCallibration(self, obj, count, serialNumber, channel, start):
		if serialNumber is None: 
			return None
		if channel is None:
			return None
		
		for cal in [obj(i) for i in xrange(0, count)]:
			if cal.serialNumber() == serialNumber and cal.channel() == channel:
				return cal.gain()
		return None
	
	def _sensorGain(self, seID, serialNumber, channel, start):
		sensor = self._findObject(seID, self._sensors)
		if not sensor: 
			raise Exception("Not found %s" % seID)

		sensorFilter = self._findObject(sensor.response(), self._filters)
		if not sensorFilter: 
			raise Exception("Not found %s" % seID)

		gainFrequency = sensorFilter.gainFrequency()
		try:
			gainUnit = sensor.unit()
		except:
			print >> sys.stderr,"[%s] No gain unit supplied" % seID
			gainUnit = None
		
		gain = self._findCallibration(sensor.sensorCalibration, sensor.sensorCalibrationCount(), serialNumber, channel, start)
		if gain is not None:
			## print >> sys.stderr,'[%s] Using sensor gain from calibration %s' % (serialNumber, gain)
			pass
		else:
			gain = sensorFilter.gain()
		
		return (gain, gainFrequency, gainUnit)
	
	def _dataloggerGain(self, dtID, serialNumber, channel, Numerator, Denominator, start):
		datalogger = self._findObject(dtID, self._datalogger)
		gain = self._findCallibration(datalogger.dataloggerCalibration, datalogger.dataloggerCalibrationCount(), serialNumber, channel, start)
		if gain is not None:
			##print >> sys.stderr,'[%s] Using datalogger gain from calibration %s' % (serialNumber, gain)
			pass
		else:
			gain = datalogger.gain()
		
		decimation = None
		for i in xrange(0,datalogger.decimationCount()):
			decimation = datalogger.decimation(i)
			if decimation.sampleRateNumerator() == Numerator and decimation.sampleRateDenominator() == Denominator:
				break
			decimation = None
		
		if not decimation:
			raise Exception("Decimation not found %s/%s" % (Numerator, Denominator))

		af = decimation.analogueFilterChain().content().split()
		df = decimation.digitalFilterChain().content().split()

		for fiID in af:
			g = self._findObject(fiID, self._filters).gain()
			#print >> sys.stderr,"Multiplying by %s %s" % (fiID, g)
			gain = gain * g

		for fiID in df:
			g = self._findObject(fiID, self._filters).gain()
			#print >> sys.stderr,"Multiplying by %s %s" % (fiID, g)
			gain = gain * g

		return gain

	def getChannelGainAttribute(self, dtID, seID, dtSerialNumber, seSerialNumber, dtChannel, seChannel, Numerator, Denominator, channelStart):
		if not dtID or not seID:
			raise Exception("Empty instruments ID supplied.")
		
		(sensorGain, sensorFrequency,sensorUnit) = self._sensorGain(seID, seSerialNumber, seChannel, channelStart)
		dataloggerGain =  self._dataloggerGain(dtID, dtSerialNumber, dtChannel, Numerator, Denominator, channelStart)
		
		att = {}
		att['Gain'] = sensorGain * dataloggerGain
		if sensorFrequency is not None:
			att['GainFrequency'] = sensorFrequency
		if sensorUnit is not None:
			att['GainUnit'] = sensorUnit
		return att

class Paz(sc3, prefixable):
	def __init__(self, pz):
		sc3.__init__(self, 'paz')
		self.id = pz.id
		self.att = pz.getAttributes()

	def sc3Att(self):
		att = {}
		att['Name'] = self.id

		for (key,value) in self.att.iteritems():
			if not self.sc3ValidKey(key) or key in att:
				print >> sys.stderr," [%s] [%s] Ignoring Attribute %s = %s " % (self.sc3Mode, self.id, key,value)
				continue
			att[key] = value

		return att

class Sensor(sc3, prefixable):
	def __init__(self, se, gain = None):
		sc3.__init__(self, 'sensor')
		self.baseid = se.id
		self.att = se.getAttributes()

		self.pz = se.generatePz(gain)
		
		self.id = "%s/g=%s" % (self.baseid, int(float(self.pz.gain)))

	def sc3Resolv(self, inventory):
		try:
			self.att['Response'] = inventory.filterID(self.pz.id)
			## print >> sys.stderr,"Re-used a sensor pole-zero"
		except:
			inventory.add(self.pz)
			self.att['Response'] = inventory.filterID(self.pz.id)

	def sc3Att(self):
		att = {}
		
		att['Name'] = self.id
		for (key, value) in self.att.iteritems():
			if not self.sc3ValidKey(key) or key in att:
				print >> sys.stderr," [%s] [%s] ignoring Attribute %s = %s " % (self.sc3Mode, self.id, key, value)
				continue
			att[key] = value

		## Forcing needed description on the sensor
		if 'Description' not in att:
			att['Description'] = self.id

		return att

class Fir(sc3, prefixable):
	def __init__(self, ff):
		sc3.__init__(self, 'fir')
		self.id = ff.id
		self.gain = ff.gain
		self.att = ff.getAttributes()

	def sc3Att(self):
		att = {}
		att['Name'] = self.id
		
		for (key,value) in self.att.iteritems():
			if not self.sc3ValidKey(key) or key in att :
				print >> sys.stderr," [%s] [%s] Ignoring Attribute %s = %s " % (self.sc3Mode, self.id, key,value)
				continue
			att[key] = value
		return att
	
class Decimation(sc3):
	def __init__(self, numerator, decimator, dl):
		sc3.__init__(self, 'decimation')
		self._numerator = numerator
		self._denominator = decimator
		self.chains = dl.chains[(numerator, decimator)]
		self.att = {}

	def sc3Resolv(self, inventory):
		sequence = {}
		sequence['A'] = []
		sequence['D'] = []
		
		
		for stage in self.chains:
			sid = inventory.filterID(stage)
			ADtype = inventory.filterType(stage)
			sequence[ADtype].append(sid)

		self.att['AnalogueFilterChain'] = " ".join(sequence['A'])
		self.att['DigitalFilterChain'] = " ".join(sequence['D'])

	def sc3Att(self):
		att = {}
		att['SampleRateNumerator'] = self._numerator
		att['SampleRateDenominator'] = self._denominator
		att.update(self.att)
		return att

class Dataloger(sc3, prefixable):
	def __init__(self, dl, gain = None):
		dcs = []
		sc3.__init__(self, 'datalogger', dcs)
		
		if gain:
			self.gain = gain
		else:
			self.gain = dl.gain
		
		self.att = dl.getAttributes()
		
		self.id = "%s/g=%s" % (dl.id, int(float(self.gain)))
		self.maxClockDrift = dl.mcld
		
		if dl.chains:
			for (num, dec) in dl.chains:
				dcs.append(Decimation(num, dec, dl))
			self.dcs = dcs
		else:
			print >>sys.stderr, "[%s] Datalogger %s has no stages." % (self.id, dl)

	def sc3Att(self):
		att = {}
		att['Name'] = self.id
		att['Gain'] = self.gain
		att['MaxClockDrift'] = self.maxClockDrift
		
		for (key,value) in self.att.iteritems():
			if not self.sc3ValidKey(key) or key in att:
				print >> sys.stderr," [%s] [%s] ignoring Attribute %s = %s " % (self.sc3Mode, self.id, key, value)
				continue
			att[key] = value

		## Forcing needed description on the sensor
		if 'Description' not in att:
			att['Description'] = self.id

		return att

class Calibration(sc3):
	def __init__(self, cl, channel, start, end):
		if cl.type == "S":
			sc3.__init__(self, "sensorCalibration")
		else:
			sc3.__init__(self, "dataloggerCalibration")
		
		if channel < 0 or channel >= cl.channelCount:
			raise Exception("Invalid channel for calibration [%s]" % channel)

		self.start = start
		self.end = end
		self.channel = channel
		self.id = cl.id
		self.att = cl.getAttributes(channel)
	
	def sc3Att(self):
		att = {}
		att['SerialNumber'] = self.id
		att['Start'] = self.start
		if self.end:
			att['End'] = self.end

		for (key, value) in self.att.iteritems():
			if not self.sc3ValidKey(key) or key in att:
				print >> sys.stderr," [%s] [%s] Ignoring Attribute %s = %s " % (self.sc3Mode, self.id, key,value)
				continue
			att[key] = value
		return att
