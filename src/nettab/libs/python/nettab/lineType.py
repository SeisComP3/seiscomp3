from __future__ import print_function
from datetime import datetime
import decimal
import shlex
import sys
import re
from helpers import parsers

verboseFlag = 0

class Nw(object):
	def __str__(self):
		return "nw/%s(%s::%s)" % (self.code, self.start, self.end)

	def __init__(self, line):
		self.att = {}
		try:
			print("Network: %s" % line, file=sys.stderr)
			items = shlex.split(line)
			items.reverse()
			self.code  = items.pop()
			self.start = parsers.parseDate(items.pop())
			# Optional End data
			try:
				self.end = parsers.parseDate(items.pop())
			except IndexError:
				self.end = None
			# Start < End
			if self.end and self.start > self.end:
				raise Exception("End Date before Start Date at line, %s %s" % (self.start, self.end))
		except Exception as e:
			raise Exception("Invalid Network header line," + str(e))
	
	def getNetworkAttributes(self):
		att = {}
		att.update(self.att)
		return att
	
	def Na(self, na):
		if not isinstance(na, Na):
			raise Exception("Wrong type of Network attribute line")
		if verboseFlag:
			if na.Key in self.att:
				print(" Overriding key %s=%s with %s" % (na.Key,self.att[na.Key], na.Value), file=sys.stderr)
			else:
				print(" Adding key %s to %s" % (na.Key, self.code), file=sys.stderr)
		self.att[na.Key] = na.Value

class Sg(object):
	def __str__(self):
		return "sg/%s(%s::%s)" % (self.code, self.start, self.end)

	def __init__(self, line):
		self.att = {}
		try:
			#print("Network: %s" % line, file=sys.stderr)
			items = shlex.split(line)
			items.reverse()
			self.code  = items.pop()
			self.start = parsers.parseDate(items.pop())
			# Optional End data
			try:
				self.end = parsers.parseDate(items.pop())
			except:
				self.end = None
			# Start < End
			if self.end and self.start > self.end:
				raise Exception("End Date before Start Date at line, %s %s" % (self.start, self.end))
		except Exception as e:
			raise Exception("Invalid Station Group header line," + str(e))
	
	def getStationGroupAttributes(self):
		att = {}
		att.update(self.att)
		return att
	
	def Na(self, na):
		if not isinstance(na, Na):
			raise Exception("Wrong type of Network attribute line")
		if verboseFlag:
			if na.Key in self.att:
				print(" Overriding key %s=%s with %s" % (na.Key,self.att[na.Key], na.Value), file=sys.stderr)
			else:
				print(" Adding key %s to %s" % (na.Key, self.code), file=sys.stderr)
		self.att[na.Key] = na.Value

class Sr(object):
	def __str__(self):
		return "sr/%s,%s(%s::%s)" % (self.ncode, self.scode, self.start, self.end)

	def __init__(self, line):
		self.ncode = None
		self.scode = None
		self.start = datetime(1980,1,1)
		self.end = None
		#print("  Station Attribute: %s" % line, file=sys.stderr)
		# Sa: Key=Value Net,Station,Location,Channel to=<Date> from=<Date>
		try:
			items = shlex.split(line)
			items.reverse()
			ns = items.pop()
			(self.ncode, self.scode) = ns.split(",",1)
			
			for item in items:
				if item[0:3] == "to=":
					self.end = parsers.parseDate(item[3:])
				elif item[0:5] == "from=":
					self.start = parsers.parseDate(item[5:])
				else:
					raise Exception("invalid station reference")

			if self.start and self.end and self.start > self.end:
				raise Exception("station reference has invalid dates.")
			
		except Exception as e:
			raise Exception("Invalid Station Attribute line, " + str(e))
	
	def _matchTime(self, start, end):
		if not self.start and not self.end:
			return True
		
		# |-------------------------------------------  Infinite 
		if self.start and not self.end:
			if start < self.start:
				if not end or end > self.start:
					raise Exception("Cross Start")
				else:
					return False
			else:
				return True
		
		# infinite ----------------------------------| 
		if self.end and not self.start:
			if start >= self.end:
				return False
			else:
				if not end or end > self.end:
					raise Exception("Cross End")
				else:
					return True
		
		#  |----------------------------------|
		if self.start and self.end:
			if start < self.start:
				if not end or end > self.start:
					raise Exception ("Cross Start")
				return False
			elif start <= self.end:
				if not end or end > self.end:
					raise Exception("Cross end")
				return True
			else:
				return False

	def match(self, ncode, scode, start, end):
		A = False
		B = False
		if ncode is not None and scode is not None:
			A = (ncode == self.ncode and scode == self.scode)
		else:
			raise Exception("Invalid Nslc supplied.")
		
		if A: B = self._matchTime(start, end)
		return A and B

class Sl(object):
	def __str__(self):
		return "sl/%s(%s::%s)" % (self.code, self.start, self.end)
	
	def __init__(self, line):
		# This are other default values
		self.compression = "2"
		self.gainCode = "H"
		self.location = ""
		self.channels = {}
		self.orientations = {}
		self.attStation = {}
		self.attLocation = {}
		try:
			items = shlex.split(line)
			items.reverse()
			self.code = parsers.parseStationCode(items.pop())
			_pc = items.pop()
			if _pc.find("/") != -1:
				(self._place, self._country) = _pc.split("/",1)
				self._place = self._place.strip()
				self._country = self._country.strip()				
			else:
				self._place = _pc.strip()
				self._country = ""
			(self._datalogger, self._dataloggerSerialNumber, self.dataloggerGain) = self._parseInstrument(items.pop())
			(self._sensor, self._sensorSerialNumber, self.sensorGain) = self._parseInstrument(items.pop())
			self._sampling = items.pop()
			self._orientation = items.pop()
			self._latitude = parsers.parseLatitude(items.pop())
			self._longitude = parsers.parseLongitude(items.pop())
			self._elevation = parsers.parseElevation(items.pop())
			self._depth = parsers.parseDepth(items.pop())
			self.start = parsers.parseDate(items.pop())
			# Optional End data
			try:
				self.end = parsers.parseDate(items.pop())
			except IndexError:
				self.end = None
			# Start < End
			if self.end and self.start > self.end:
				raise Exception("End Date before Start Date at line, %s %s" % (self.start, self.end))
			# Parse the Sampling string
			self._parseOrientation(self._orientation)
			# Parse the Sampling string
			self._parseSampling(self._sampling)

			if verboseFlag: print("Station Line:", self.code, self.start,"->",self.end, self._sampling, self._orientation, file=sys.stderr)
		except Exception as e:
			raise Exception("Invalid Station line, " + str(e))

	def _parseInstrument(self, inst):
		try:
			items = inst.split("%")
			i = items.pop(0)
			try:
				sn = items.pop(0)
				if len(sn) == 0: sn = None
			except IndexError:
				sn = None
			try:
				gain = parsers.parseGain(items.pop(0))
			except IndexError:
				gain = None
		except Exception as e:
			raise Exception("Invalid Instrument %s" % e)		
		return (i,sn,gain)
	
	def _parseOrientation(self, orientation):
		att = {}
		regex = re.compile(r'(?P<channelCode>[A-Z0-9])(?:[(](?P<channelOrientation>.*?)[)]){0,1}')
		pos = 0
		zne = [0,0,0]
		while pos < len(orientation):
			if len(att) >=3:
				raise Exception("Each Sl should define a maximum of three channels")
			
			ma = regex.match(orientation,pos)
			if not ma:
				raise Exception("Error at:",orientation[pos:])
			code = ma.groupdict()["channelCode"]
			if ma.groupdict()["channelOrientation"]:
				if code in ["Z", "N", "E"]:
					raise Exception("You should not supply a Azimuth/Dip for ZNE channels.")
				try:
					(dip,azimuth) = ma.groupdict()["channelOrientation"].split(",")
					dip = float(dip)
					azimuth = float(azimuth)
					if azimuth < 0.0: azimuth += 360
					if azimuth < 0.0 or azimuth > 360:
						raise Exception("Azimuth should be from 0 <= A <= 360.0")
					if dip < -90.0 or dip > 90:
						raise Exception("Dip should be from -90.0 <= D <= 90.0")
				except:
					raise Exception("Invalid format, expected Dip, Azimuth for channel, got %s" % ma.groupdict()["channelOrientation"])
			else:
				if code == "Z":
					dip = -90.0
					azimuth = 0.0
				elif code == "N" or code == "1":
					dip = 0.0
					azimuth = 0.0
				elif code == "E" or code == "2":
					dip = 0.0
					azimuth = 90.0
				else:
					raise Exception("Dip,Azimuth not specified and code != Z/N/E/1/2")

			# Datalogger/Sensor Channel
			if code == "Z":
				if zne[0]:
					raise Exception("Cannot define more than one '0' channel Z")
				dataloggerChannel = 0
				sensorChannel = 0
				zne[0] += 1
			elif code == "N" or code == "1":
				if zne[1]:
					raise Exception("Cannot define more than one '1' channels 1/N.")
				dataloggerChannel = 1
				sensorChannel = 1
				zne[1] += 1
			elif code == "E" or code == "2":
				if zne[2]:
					raise Exception("Cannot define more than one '2' channels 2/E.")
				dataloggerChannel = 2
				sensorChannel = 2
				zne[2] += 1
			else:
				for i in [0,1,2]:
					if zne[i]: continue
					dataloggerChannel = i
					sensorChannel = i
					zne[i] += 1
					break
				
			# Assembly
			if code not in att:
				att[code] = {} 
				att[code]["Dip"] = dip
				att[code]["Azimuth"] = azimuth
				att[code]["DataloggerChannel"] = dataloggerChannel
				att[code]["SensorChannel"] = sensorChannel
				pos = ma.end()
			else:
				raise Exception("Cannot create two %s channels while parsing channel Orientations at: %s" % (code, orientation))
		self.orientations = att
	
	def _parseSampling(self, sampling):
		_rx_samp = re.compile(r'(?P<band_code>[A-Z])?(?P<sample_rate>.*)$')
		endPreamble = sampling.find('_')
		if endPreamble > 0:
			for x in sampling[:endPreamble].split('/'):
				if x[0] == 'F':
					self.compression = x[1:]
				elif x[0] == 'L':
					self.location = x[1:]
				elif x[0] == 'T':
					self.gainCode = x[1:]
				else:
					raise Exception("unknown Preemble code %s in %s" % (x[0], sampling))
			
		for x in sampling[endPreamble+1:].split('/'):
			m = _rx_samp.match(x)
			if not m:
				raise Exception("error parsing sampling %s at %s" % (sampling, x))
				
			try:
				sample_rate = decimal.Decimal(m.group('sample_rate'))
			except decimal.InvalidOperation:
				raise Exception("error parsing sampling %s at %s" % (sampling, x))
			
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
					raise Exception("could not determine band code for %s in %s" (x, sampling))
			
			for orientation in self.orientations:
				code = band_code + self.gainCode + orientation
				if code in self.channels:
					raise Exception("channel code %s aready exists for rate %s " % (band_code, sample_rate))
				try:
					(numerator, denominator) = parsers._rational(sample_rate)
				except Exception as e:
					raise Exception("Error converting sample_rate (%s)" % e)
				self.channels[code] = { 'SampleRateNumerator': numerator,
										'SampleRateDenominator': denominator }
				self.channels[code].update(self.orientations[orientation])
		
	def Sa(self, sa):
		if not isinstance(sa, Sa):
			raise Exception("Wrong type of Station Attribute line")
		
		if sa.match(self.code, None, None, self.start, self.end):
			if verboseFlag:
				if sa.Key in self.attStation:
					print("Overridding Station key %s=%s with %s" % (sa.Key, self.attStation[sa.Key], sa.Value), file=sys.stderr)
				else:
					print("Adding Station key %s to %s" % (sa.Key, self.code), file=sys.stderr)
			self.attStation[sa.Key] = sa.Value

		if sa.match(self.code, self.location, None, self.start, self.end):
			if verboseFlag:
				if sa.Key in self.attLocation:
					print("Overriding Location key %s=%s with %s" % (sa.Key, self.attLocation[sa.Key], sa.Value), file=sys.stderr)
				else:
					print("Adding Location key %s to %s" % (sa.Key, self.code), file=sys.stderr)
			self.attLocation[sa.Key] = sa.Value

		for channel in self.channels.keys():
			if sa.match(self.code, self.location, channel, self.start, self.end):
				if verboseFlag:
					if sa.Key in self.channels[channel].keys():
						print("Overriding Channel key %s=%s with %s" % (sa.Key, self.channels[channel][sa.Key], sa.Value), file=sys.stderr)
					else:
						print("Adding Channel key %s to %s/%s" % (sa.Key, self.code, channel), file=sys.stderr)
				self.channels[channel][sa.Key] = sa.Value
	
	def getStationAttributes(self):
		att = {}
		
		att["Latitude"] = self._latitude
		att["Longitude"] = self._longitude
		att["Elevation"] = self._elevation
		att['Place'] = self._place
		att['Country'] = self._country
		att.update(self.attStation)

		if 'Description' not in att:
			att['Description'] = ""
			if 'Affiliation' in self.attStation:
				att['Description'] += self.attStation['Affiliation']

			if self._place:
				att['Description'] += " Station %s" % self._place

			if self._country:
				att['Description'] += ", %s" % self._country
		return att
	
	def getLocationAttributes(self):
		att = {}
		att["Latitude"] = self._latitude
		att["Longitude"] = self._longitude
		att["Elevation"] = self._elevation
		att.update(self.attLocation)
		return att

	def getChannelAttributes(self, code):
		if code not in self.channels:
			raise Exception("Invalid channel code")
		att = {}
		att["Datalogger"] = self._datalogger
		att["Sensor"] = self._sensor

		if self._dataloggerSerialNumber:
			att["DataloggerSerialNumber"] = self._dataloggerSerialNumber

		if self._sensorSerialNumber:
			att["SensorSerialNumber"] = self._sensorSerialNumber

		att["Depth"] = self._depth
		att["Format"] = 'Steim%s' % self.compression
		att.update(self.channels[code])
		return att

class Na(object):
	def __str__(self):
		return "na/%s=%s" % (self.Key, self.Value)

	def __init__(self, line):
		#print("Network Attribute:", line, file=sys.stderr)
		try:
			items = shlex.split(line)
			(key, value) = items[0].split("=", 1)
			(self.Key, self.Value) = self._validate(key, value)
		except Exception as e:
			raise Exception("Invalid Na line, " + str(e))

	# Validate the line in terms of the seiscomp3 inventory definition.
	# Also should make sure that the variable types are correct and in sync with the sc3 inventory.	
	def _validate(self, key, value):
		key = key.strip()
		value = value.strip()
		if len(key) == 0 or len(value) == 0:
			raise Exception("Key/value should not be empty") 
		return (key,value)

class Sa(object):
	def __str__(self):
		return "sa/%s=%s(%s,%s)" % (self.Key, self.Value, self.start, self.end)

	def __init__(self, line):
		self.items = []
		self.start = None
		self.end = None
		#print("  Station Attribute:", line, file=sys.stderr)
		# Sa: Key=Value Net,Station,Location,Channel to=<Date> from=<Date>
		try:
			items = shlex.split(line)
			items.reverse()
			keypair=items.pop()
			(key,value) = keypair.split("=",1)
			for item in items:
				if item[0:3] == "to=":
					self.end = parsers.parseDate(item[3:])
				elif item[0:5] == "from=":
					self.start = parsers.parseDate(item[5:])
				else:
					items = item.split(",")
					items.reverse()
					try:
						station = parsers.parseStationCode(items.pop())
					except IndexError:
						station = None
					try:
						location = parsers.parseLocationCode(items.pop())
					except IndexError:
						location = None
					try:
						channel = parsers.parseChannelCode(items.pop())
					except IndexError:
						channel = None

					(self.Key, self.Value) = self._validate(key, value, station, location, channel)
					## print("Adding %s: %s %s %s" % (self.Key, station,location,channel), file=sys.stderr)
					self.items.append((station,location,channel))
			if self.start and self.end and self.start > self.end:
				raise Exception("attribute has invalid dates.")
			if len(self.items) == 0:
				raise Exception("no channel pattern specified.")
			
		except Exception as e:
			raise Exception("Invalid Station Attribute line, " + str(e))
	
	def _validate(self, key, value, station, location, channel):
		
		# Validate the nslc
		
		key = key.strip()
		value = value.strip()
		if len(key) == 0 or len(value) == 0:
			raise Exception("Key/value should not be empty")
		if channel is None and location is None and station is None: 
			raise Exception("%s/%s pair should apply to a least one node" % (key, value))
		return (key, value)
	
	def _regexCompare(self, pattern, search):
		pattern = '^' + pattern.replace("?",".").replace("*",".*") + '$'
		return (re.search(pattern, search) is not None)

	def _matchStation(self, scode):
		for (sta, loc, cha) in self.items:
			if cha is not None: continue
			if loc is not None: continue
			if self._regexCompare(sta, scode):
				return True
		return False
	
	def _matchLocation(self, scode, lcode):
		for (sta, loc, cha) in self.items:
			if cha is not None: continue
			if loc is None: continue
			if self._regexCompare(sta, scode) and self._regexCompare(loc, lcode):
				return True
		return False
	
	def _matchChannel(self, scode, lcode, ccode):
		for (sta, loc, cha) in self.items:
			if cha is None: continue
			if self._regexCompare(sta, scode) and self._regexCompare(loc, lcode) and self._regexCompare(cha, ccode):
				return True
		return False

	def _matchTimeRelaxed(self, start, end):
		'''
		This method is more relaxed about time constrains and only match start date supplied.
		When the start date supplied is inside the range defined in this object, accept, otherwise
		reject
		'''
		if not self.start and not self.end:
			return True

		if not start: raise Exception("Cannot compare with an node without start")

		# |-------------------------------------------  Infinite
		if self.start and not self.end:
			if start >= self.start: return True
			return False

		# infinite ----------------------------------| 
		if self.end and not self.start:
			if start > self.end: return False
			return True

		#  |----------------------------------|
		if self.start and self.end:
			if start < self.start: return False
			if start > self.end: return False
			return True

	def _matchTime(self, start, end):
		if not self.start and not self.end:
			return True
		
		if not start: raise Exception("Cannot compare with an node without start")
		
		# |-------------------------------------------  Infinite 
		if self.start and not self.end:
			if start < self.start:
				if not end or end > self.start:
					raise Exception("Object cross attribute (Sa) start")
				else:
					return False
			else:
				return True
		
		# infinite ----------------------------------| 
		if self.end and not self.start:
			if start >= self.end:
				return False
			else:
				if not end or end > self.end:
					raise Exception("Object cross attribute (Sa) end")
				else:
					return True
		
		#  |----------------------------------|
		if self.start and self.end:
			if start < self.start:
				if not end or end > self.start:
					raise Exception ("Object cross attribute (Sa) start")
				return False
			elif start <= self.end:
				if not end or end > self.end:
					raise Exception("Object cross attribute (Sa) end")
				return True
			else:
				return False

	def match(self, scode, lcode, ccode, start, end, relaxed = False):
		A = False
		B = False
		if ccode is not None and lcode is not None and scode is not None:
			A =  self._matchChannel(scode, lcode, ccode)
		elif lcode is not None and scode is not None:
			A = self._matchLocation(scode, lcode)
		elif scode is not None:
			A = self._matchStation(scode)
		else:
			raise Exception("Invalid Nslc supplied.")
		
		if A:
			if not relaxed:
				B = self._matchTime(start, end)
			else:
				B = self._matchTimeRelaxed(start, end)
		
		return A and B

class Ia(object):
	def __str__(self):
		return "ia/%s=%s" % (self.Key, self.Value)

	def _regexCompare(self, pattern, search):
		pattern = '^' + pattern.replace("?",".").replace("*",".*") + '$'
		return (re.search(pattern, search) is not None)

	def match(self, elementID, element):
		try:
			rs = self.ids[elementID]
			if len(rs) == 0: return True
			for r in rs:
				if type(element) == str:
					if r.__name__ == element: return True
				else:
					if isinstance(element,r): return True
		except:
			pass

		try:
			for (pattern, rs) in self.patterns.iteritems():
				if self._regexCompare(pattern, elementID):
					if len(rs) == 0: return True
					for r in rs:
						if type(element) == str:
							if r.__name__ == element: return True
						else:
							if isinstance(element, r): return True
		except:
			pass
		
		return False

	def __init__(self, line):
		_rx_item = re.compile(r'((?P<restriction>.*)::)?(?P<pattern>.*)$')
		self.ids = {}
		self.patterns = {}
		items = shlex.split(line)
		items.reverse()
		try:
			(self.Key, self.Value) = items.pop().split("=")
			if len(items) == 0:
				raise Exception("Attribute without ids.")
			
			while items:
				item = items.pop()
				m = _rx_item.match(item)
				if not m:
					Exception("Invalid id specification for attribute %s" % item)
					
				restriction = m.group('restriction')
				pattern = m.group('pattern')
				
				if pattern is None or len(pattern) == 0:
					raise Exception("Empty id for attribute %s" % item)

				if pattern.find("*") > -1 or pattern.find("?") > -1:
					idList = self.patterns
				else:
					idList = self.ids
				
				if pattern not in idList:
					idList[pattern] = []
				
				if restriction:
					if restriction not in ["Se", "Dl", "Cl", "Ff", "Pz", "If" ]:
						raise Exception("Invalid line restriction type %s" % restriction)
					try:
						idList[pattern].append(globals()[restriction])
					except KeyError as e:
						raise Exception("Invalid restriction type [%s] on instrument attribute %s" % (restriction, self.Key))

		except Exception as e:
			raise Exception("Error parsing Instrument Attribution line, %s" % e)

class Se(object):
	def __str__(self):
		return "se/%s " % self.id

	def __init__(self, line):
		self.att = {}
		self.iaList = []
		items = line.split()
		try:
			self.id = items.pop(0)
			self.gain = items[0]
			self._generator = " ".join(items) 
		except Exception as e:
			print("Error parsing Seismometer line, %s" % e, file=sys.stderr)
	
	def getAttributes(self):
		att = {}
		att.update(self.att)
		return att

	def generatePz(self, gain=None):
		pz = None
		
		pazid = "BADpazID"
		try:
			pz = Pz(" ".join([pazid, self._generator]), 'A', gain)
		except Exception as e:
			raise Exception("Error %s" % e)
		
		for ia in self.iaList:
			pz.Ia(ia)
		
		_gain = pz.gain
		pz.id = "%s/g=%s" % (self.id, int(float(_gain)))

		return pz

	def Ia(self, ia):
		if not isinstance(ia, Ia):
			raise Exception("Wrong type of Instrument attribute line")

		self.iaList.append(ia)

		if not ia.match(self.id, self):
			return
	
		if ia.Key in self.att:
			if verboseFlag: print(" [%s] Overriding key %s=%s with %s" % (self.id, ia.Key, self.att[ia.Key], ia.Value), file=sys.stderr)

		self.att[ia.Key] = ia.Value

class Dl(object):
	def __str__(self):
		return "dl/%s " % self.id
	
	def __init__(self, line):
		self.att = {}
		items = line.split()
		items.reverse()
		try:
			self.id = items.pop()
			self.gain = float(items.pop())
			self.spfr = decimal.Decimal(items.pop())
			self.mcld = float(items.pop())

			self.chains = None
			if items:
				filterIndex = items.pop()
				self.chains = self._parseStages(filterIndex, items.pop())
			
		except Exception as e:
			raise Exception("[%s] Error parsing Datalogger line, %s" % (self.id, e))
		
	def _parseStages(self, filterIndex, line):
		chains = {}
		decimations = line.split(",")
		for decimation in decimations:
			if decimation.find("_") == -1:
				rate = decimation
				stringstages = None
			else:
				(rate, stringstages) = decimation.split("_")
			(num, den) =  parsers._rational(decimal.Decimal(rate))
			stages = []
			if stringstages:
				for stage in stringstages.split("/"):
					if len(stage) and stage[0] == 'A':
						stages.append(str(filterIndex + "_digipaz_" + stage[1:]))
					elif len(stage) and  stage[0] == 'I':
						stages.append(str(filterIndex + "_iirpaz_" + stage[1:]))
					elif len(stage):
						stages.append(str(filterIndex + "_FIR_" + stage))
					else:
						raise Exception("Invalid Elo-In-Chain")
			if (num, den) in chains:
				raise Exception("%s has a duplicate rate chain Numerator=%s Denominator=%s" % (self.id, num, den))
			else:
				chains[(num, den)] = stages
		return chains

	def Ia(self, ia):
		if not isinstance(ia, Ia):
			raise Exception("Wrong type of Instrument attribute line")

		if not ia.match(self.id, self):
			return

		if ia.Key in self.att:
			if verboseFlag: print(" Overriding key %s=%s with %s" % (ia.Key,self.att[ia.Key], ia.Value), file=sys.stderr)

		self.att[ia.Key] = ia.Value

	def check(self, instruments):
		error = []
		
		# Check that we can find my stages
		if self.chains:
			for chain in self.chains.itervalues():
				for stage in chain:
					if stage not in instruments.keys:
						error.append(" [%s] Missing stage %s" % (self.id, stage))
		return error

	def use(self, genericFilter):
		"""
		Check that the supplied filter is used by this instance.
		"""
		if not self.chains:
			return False
		
		if isinstance(genericFilter, Ff) or isinstance(genericFilter, Pz):
			for chain in self.chains.itervalues():
				if genericFilter.id in chain:
					return True

		return False
	
	def getAttributes(self):
		att = {}
		att.update(self.att)
		return att

class Cl(object):
	def __str__(self):
		return "cl/%s " % self.id

	def __init__(self, line):
		self.att = {}
		items = line.split()
		items.reverse()
		try:
			self.id = items.pop()
			self.date = parsers.parseDate("1980/001")
			self.gain = []
			self.gain.append(float(items.pop())) 
			self.gain.append(float(items.pop())) 
			self.gain.append(float(items.pop()))
			
			self.channelCount = len(self.gain)

			(self.type, self.instruments) = items.pop().split("_")
			self.instruments = self.instruments.split(",")
			
			if self.type != "S" and self.type != "L":
				raise Exception("Unknown calibration type %s" % self.type)
			
		except Exception as e:
			print("Error parsing Calibration line, %s" % e, file=sys.stderr)
			raise Exception("Error!")

	def check(self, instruments):
		error = []
		for i in self.instruments:
			if i not in instruments.keys:
				error.append(" [%s] calibration attributed to an invalid instrument %s" % (self.id, i))
		return error
		
	def getAttributes(self, channel):
		att = {}
		
		att['Gain'] = self.gain[int(channel)]
		att['Channel'] = channel
		att.update(self.att)
		return att

	def match(self, instrumentID, serialNumber):
		return (instrumentID in self.instruments) and (self.id == serialNumber)

	def Ia(self, ia):
		if not isinstance(ia, Ia):
			raise Exception("Wrong type of Instrument attribute line")

		if not ia.match(self.id, self):
			return

		if ia.Key in self.att:
			if verboseFlag: print(" Overriding key %s=%s with %s" % (ia.Key,self.att[ia.Key], ia.Value), file=sys.stderr)

		self.att[ia.Key] = ia.Value

class Ff(object):
	def __str__(self):
		return "ff/%s " % (self.id)

	def _loadCoeficients(self, filterpath, relaxed = False):
		if not filterpath:
			raise Exception("Need a filter folder to load the coeficients files %s" % self.filename)
		try:
			fd = open(filterpath.rstrip(" /") + "/" + self.filename)
		except IOError as  e:
			raise Exception("cannot read %s%s: %s" % (filterpath, self.filename, str(e)))

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
					raise Exception("invalid number of coefficients in %s/%s" % (self.id, self.filename))

				coeff_strlist = [ coeff_split[3 * i + 1] for i in range(real_ncf) ]
				coeff = map(float, coeff_strlist)

			except (TypeError, ValueError) as e:
				raise Exception("error reading %s/%s: %s"  % (self.id, self.filename, str(e)))

		finally:
			fd.close()

		i = 0
		while 2 * i < real_ncf:
			if coeff[i] != coeff[real_ncf - 1 - i]:
				break
			i += 1

		#print("I=%s NCF=%s" % (i,real_ncf), file=sys.stderr)
		if 2 * i > real_ncf:
			real_ncf = i
			real_sym = "B"
		elif 2 * i == real_ncf:
			real_ncf = i
			real_sym = "C"
		else:
			real_ncf = real_ncf
			real_sym = "A"
		
		if self.sym != real_sym:
			if verboseFlag: print(" [%s] Warning: Symmetry code %s doesn't match (%s)" % (self.id, self.sym, self.filename), file=sys.stderr)
			self.sym = real_sym
			if not relaxed:
				raise Exception(" [%s] Symmetry code %s doesn't match (%s)" % (self.id, self.sym,self.filename))

		if self.ncf != real_ncf:
			if verboseFlag: print(" [%s] Warning: Coefficients number %s differs (%s)" % (self.id, self.ncf, self.filename), file=sys.stderr)
			self.ncf = real_ncf
			if not relaxed:
				raise Exception(" [%s] Coefficients number %s differs (%s)" % (self.id, self.ncf, self.filename))
		
		return coeff_strlist[:self.ncf]

	def __init__(self, filterpath, line):
		self.att = {}
		self.ratt= {}
		items = line.split()
		items.reverse()
		try:
			self.id = items.pop()
			self.filename = items.pop()
			self.sym = items.pop()
			self.ncf = int(items.pop())
			nullo = float(items.pop())
			_inrate = decimal.Decimal(items.pop())
			fac = int(items.pop())
			
			delay = float(items.pop())  * float(_inrate)
			corrtn = float(items.pop()) * float(_inrate)
			
			self.gain = float(items.pop())
			_frg = float(items.pop())
		except Exception as e:
			print("[%s] Error parsing FIR Filter line, %s" % (self.id, e), file=sys.stderr)
			raise Exception("Error!")
		
		# Load the coefficients
		self.coefficients = self._loadCoeficients(filterpath, relaxed = True)
			
		if self.sym == 'B':
			real_delay = (float(self.ncf) - 1.0) / float(_inrate)
		elif self.sym == 'C':
			real_delay = (float(self.ncf) - 0.5) / float(_inrate)

		if self.sym in ('B', 'C') and abs(delay - real_delay) > 0.001:
			if 0: print(" [%s] Warning: delay=%g (estimated %g)" % (self.id, delay, real_delay), file=sys.stderr)

		self.ratt['Gain'] = self.gain
		self.ratt['DecimationFactor'] = fac
		self.ratt['Delay'] = delay
		self.ratt['Correction'] = corrtn
		self.ratt['NumberOfCoefficients'] = self.ncf
		self.ratt['Symmetry'] = self.sym
		self.ratt['Coefficients'] = self.coefficients

		if self.ncf != len(self.coefficients):
			raise Exception("Wrong number of coefficients")

	def getAttributes(self):
		att = {}
		att.update(self.att)
		att.update(self.ratt)
		return att
	
	def Ia(self, ia):
		if not isinstance(ia, Ia):
			raise Exception("Wrong type of Instrument attribute line")

		if not ia.match(self.id, self):
			return

		if ia.Key in self.ratt:
			raise Exception(" [%s] Cannot override a read-only attribute '%s'" % (self.id, ia.Key))

		if ia.Key in self.att:
			if verboseFlag: print(" Overriding key %s=%s with %s" % (ia.Key,self.att[ia.Key], ia.Value), file=sys.stderr)

		self.att[ia.Key] = ia.Value

class Pz(object):
	def __str__(self):
		return "pz/%s(%s) " % (self.id, self.type)

	def __init__(self, line, pzType = None, gain = None):
		if not pzType:
			print("Type should be supplied as 'D' == for Iir filters or type 'A' for digiPaz filter on datalogger", file=sys.stderr)
			raise Exception("Error.")

		## Attributes array
		self.att = {}

		## non-overwriting attributes array
		self.ratt = {}

		items = line.split()
		items.reverse()
		try:
			self.id = items.pop()

			self.ratt['Type'] = pzType
			## Copying for access
			self.type = self.ratt['Type']

			self.ratt['Gain'] = float(items.pop()) 
			# Overriding default gain
			if gain:
				self.ratt['Gain'] = float(gain)
			## Copying for access
			self.gain = self.ratt['Gain']

			self.ratt['GainFrequency'] = float(items.pop())
			self.ratt['NormalizationFactor'] = float(items.pop())
			self.ratt['NormalizationFrequency'] = float(items.pop())

			_nz= int(items.pop())
			self.ratt['NumberOfZeros'] = _nz

			_np = int(items.pop())
			self.ratt['NumberOfPoles'] = _np

			items.reverse()
			paz = parsers._parse_paz(_nz + _np, " ".join(items))
			_zeros = paz[:_nz]
			_poles = paz[_nz:]
			self.ratt['Zeros'] = _zeros 
			self.ratt['Poles'] = _poles 

			if len(_poles) != _np or len(_zeros) != _nz:
				raise Exception("Wrong number of poles & Zeros")
		except Exception as e:
			print("Error parsing Poles and Zeros Filter line, %s" % e, file=sys.stderr)
			raise Exception("Error!")

	def getAttributes(self):
		att = {}
		att.update(self.att)
		att.update(self.ratt)
		return att

	def Ia(self, ia):
		if not isinstance(ia, Ia):
			raise Exception("Wrong type of Instrument attribute line")

		if not ia.match(self.id, self):
			return

		if ia.Key in self.ratt:
			raise Exception(" [%s] Cannot override a read-only attribute '%s'" % (self.id, ia.Key))

		if ia.Key in self.att:
			if verboseFlag: print(" Overriding key %s=%s with %s" % (ia.Key,self.att[ia.Key], ia.Value), file=sys.stderr)

		self.att[ia.Key] = ia.Value
