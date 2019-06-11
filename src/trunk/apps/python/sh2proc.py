#!/usr/bin/env python
# -*- coding: utf-8 -*-

###############################################################################
# Copyright (C) 2017 by gempa GmbH                                            #
#                                                                             #
# You can redistribute and/or modify this program under the                   #
# terms of the SeisComP Public License.                                       #
#                                                                             #
# This program is distributed in the hope that it will be useful,             #
# but WITHOUT ANY WARRANTY; without even the implied warranty of              #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               #
# SeisComP Public License for more details.                                   #
#                                                                             #
# Author: Alexander Jaeger, Stephan Herrnkind, Lukas Lehmann, Dirk Roessler   #
# Email: herrnkind@gempa.de                                                   #
#                                                                             #
# Converts SeismicHandler (http://www.seismic-handler.org/) event data to     #
# SeisComP3. Data is read from input file or stdin if no input file is        #
# specified. The result is available on stdout.                               #
#                                                                             #
#   seiscomp exec sh2proc shm.evt > sc3.xml                                   #
#                                                                             #
# Since SeismicHandler only specifies station and component codes, a          #
# mapping to SeisComP3 network, location and channel codes is necessary.      #
# The script assumes that the same station code is not used in different      #
# networks. In case an ambiguous id is found a warning is printed and the     #
# first network code is used. The channel and stream code is extracted        #
# from the dectecStream and detecLocid configured in the global binding.      #
# In case no configuration module is available the first location and         #
# stream is used.                                                             #
###############################################################################


from seiscomp3 import Client, Core, DataModel, IO, Logging, Math
from time import strptime
import sys, traceback

TimeFormats = [
	'%d-%b-%Y_%H:%M:%S.%f',
	'%d-%b-%Y_%H:%M:%S'
]


## SC3 has more event types available in the datamodel
EventTypes = {
	'teleseismic quake'  : DataModel.EARTHQUAKE,
	'local quake'        : DataModel.EARTHQUAKE,
	'regional quake'     : DataModel.EARTHQUAKE,
	'quarry blast'       : DataModel.QUARRY_BLAST,
	'nuclear explosion'  : DataModel.NUCLEAR_EXPLOSION,
	'mining event'       : DataModel.MINING_EXPLOSION
}

def wfs2Str(wfsID):
	return '%s.%s.%s.%s' % (wfsID.networkCode(), wfsID.stationCode(),
	                        wfsID.locationCode(), wfsID.channelCode())



###############################################################################
class SH2Proc(Client.Application):

	###########################################################################
	def __init__(self):
		Client.Application.__init__(self, len(sys.argv), sys.argv)
		self.setMessagingEnabled(True)
		self.setDatabaseEnabled(True, True)
		self.setLoadInventoryEnabled(True)
		self.setLoadConfigModuleEnabled(True)
		self.setDaemonEnabled(False)

		self.inputFile = '-'



	###########################################################################
	def initConfiguration(self):
		if not Client.Application.initConfiguration(self):
			return False

		# If the database connection is passed via command line or configuration
		# file then messaging is disabled. Messaging is only used to get
		# the configured database connection URI.
		if self.databaseURI() != '':
			self.setMessagingEnabled(False)
		else:
			# A database connection is not required if the inventory is loaded
			# from file
			if not self.isInventoryDatabaseEnabled():
				self.setMessagingEnabled(False)
				self.setDatabaseEnabled(False, False)

		return True



	###########################################################################
	def validateParameters(self):
		if not Client.Application.validateParameters(self):
			return False

		for opt in self.commandline().unrecognizedOptions():
			if len(opt) > 1 and opt.startswith('-'):
				continue

			self.inputFile = opt
			break

		return True



	###########################################################################
	def loadStreams(self):
		now = Core.Time.GMT()
		inv = Client.Inventory.Instance()

		self.streams = {}

		# try to load streams by detecLocid and detecStream
		mod = self.configModule()
		if mod is not None and mod.configStationCount() > 0:
			Logging.info('loading streams using detecLocid and detecStream')
			for i in range(mod.configStationCount()):
				cfg = mod.configStation(i)
				net = cfg.networkCode()
				sta = cfg.stationCode()
				if self.streams.has_key(sta):
					Logging.warning('ambiguous stream id found for station ' \
					                '%s.%s' % (net, sta))
					continue

				setup = DataModel.findSetup(cfg, self.name(), True)
				if not setup:
					Logging.warning('could not find station setup for %s.%s' % (
					                net, sta))
					continue

				params = DataModel.ParameterSet.Find(setup.parameterSetID())
				if not params:
					Logging.warning('could not find station parameters for ' \
					                '%s.%s' % (net, sta))
					continue

				detecLocid = ''
				detecStream = None

				for j in xrange(params.parameterCount()):
					param = params.parameter(j)
					if param.name() == 'detecStream':
						detecStream = param.value()
					elif param.name() == 'detecLocid':
						detecLocid = param.value()

				if detecStream is None:
					Logging.warning('could not find detecStream for %s.%s' % (
					                net, sta))
					continue

				loc = inv.getSensorLocation(net, sta, detecLocid, now)
				if loc is None:
					Logging.warning('could not find preferred location for ' \
					                '%s.%s' % (net, sta))
					continue

				components = {}
				tc = DataModel.ThreeComponents()
				DataModel.getThreeComponents(tc, loc, detecStream[:2], now)
				if tc.vertical():
					cha = tc.vertical()
					wfsID = DataModel.WaveformStreamID(net, sta, loc.code(),
					                                   cha.code(), '')
					components[cha.code()[-1]] = wfsID
					Logging.debug('add stream %s (vertical)' % wfs2Str(wfsID))
				if tc.firstHorizontal():
					cha = tc.firstHorizontal()
					wfsID = DataModel.WaveformStreamID(net, sta, loc.code(),
					                                   cha.code(), '')
					components[cha.code()[-1]] = wfsID
					Logging.debug('add stream %s (first horizontal)' % wfs2Str(wfsID))
				if tc.secondHorizontal():
					cha = tc.secondHorizontal()
					wfsID = DataModel.WaveformStreamID(net, sta, loc.code(),
					                                   cha.code(), '')
					components[cha.code()[-1]] = wfsID
					Logging.debug('add stream %s (second horizontal)' % wfs2Str(wfsID))
				if len(components) > 0:
					self.streams[sta] = components

			return


		# fallback loading streams from inventory
		Logging.warning('no configuration module available, loading streams ' \
		                'from inventory and selecting first available stream ' \
		                'matching epoch')
		for iNet in xrange(inv.inventory().networkCount()):
			net = inv.inventory().network(iNet)
			Logging.debug('network %s: loaded %i stations' % (
			              net.code(), net.stationCount()))
			for iSta in xrange(net.stationCount()):
				sta = net.station(iSta)
				try:
					start = sta.start()
					if not start <= now:
						continue
				except:
					continue

				try:
					end = sta.end()
					if not now <= end:
						continue
				except:
					pass

				for iLoc in xrange(sta.sensorLocationCount()):
					loc = sta.sensorLocation(iLoc)
					for iCha in range(loc.streamCount()):
						cha = loc.stream(iCha)

						wfsID = DataModel.WaveformStreamID(net.code(),
						        sta.code(), loc.code(), cha.code(), '')
						comp = cha.code()[2]
						if not self.streams.has_key(sta.code()):
							components = {}
							components[comp] = wfsID
							self.streams[sta.code()] = components
						else:
							# Seismic Handler does not support network,
							# location and channel code: make sure network and
							# location codes match first item in station
							# specific steam list
							oldWfsID = self.streams[sta.code()].values()[0]
							if net.code() != oldWfsID.networkCode() or \
							   loc.code() != oldWfsID.locationCode() or \
							   cha.code()[:2] != oldWfsID.channelCode()[:2]:
								Logging.warning('ambiguous stream id found ' \
								                'for station %s, ignoring %s' \
								                % (sta.code(), wfs2Str(wfsID)))
								continue

							self.streams[sta.code()][comp] = wfsID

						Logging.debug('add stream %s' % wfs2Str(wfsID))



	###########################################################################
	def parseTime(self, timeStr):
		time = Core.Time()
		for fmt in TimeFormats:
			if time.fromString(timeStr, fmt):
				break
		return time



	###########################################################################
	def parseMagType(self, value):
		if value == 'm':
			return 'M'
		elif value == 'ml':
			return 'ML'
		elif value == 'mb':
			return 'mb'
		elif value == 'ms':
			return 'Ms(BB)'
		elif value == 'mw':
			return 'Mw'
		elif value == 'bb':
			return 'mB'

		return ''



	###########################################################################
	def sh2proc(self, file):
		ep             = DataModel.EventParameters()
		origin         = DataModel.Origin.Create()
		event          = DataModel.Event.Create()

		origin.setCreationInfo(DataModel.CreationInfo())
		origin.creationInfo().setCreationTime(Core.Time.GMT())

		originQuality  = None
		originCE       = None
		latFound       = False
		lonFound       = False
		depthError     = None
		originComments = {}

		# variables, reset after 'end of phase'
		pick           = None
		stationMag     = None
		staCode        = None
		compCode       = None
		stationMagBB   = None

		amplitudeDisp  = None
		amplitudeVel   = None
		amplitudeSNR   = None
		amplitudeBB    = None

		magnitudeMB    = None
		magnitudeML    = None
		magnitudeMS    = None
		magnitudeBB    = None

		km2degFac      = 1.0 / Math.deg2km(1.0)

		# read file line by line, split key and value at colon
		iLine = 0
		for line in file:
			iLine += 1
			a = line.split(':', 1)
			key = a[0].strip()
			keyLower = key.lower()
			value = None

			# empty line
			if len(keyLower) == 0:
				continue

			# end of phase
			elif keyLower == '--- end of phase ---':
				if pick is None:
					Logging.warning('Line %i: found empty phase block' % iLine)
					continue

				if staCode is None or compCode is None:
					Logging.warning('Line %i: end of phase, stream code ' \
					                'incomplete' % iLine)
					continue

				if not self.streams.has_key(staCode):
					Logging.warning('Line %i: end of phase, station code %s ' \
					                'not found in inventory' % (iLine, staCode))
					continue

				if not self.streams[staCode].has_key(compCode):
					Logging.warning('Line %i: end of phase, component %s of ' \
					                'station %s not found in inventory' % (
					                iLine, compCode, staCode))
					continue

				streamID = self.streams[staCode][compCode]

				pick.setWaveformID(streamID)
				ep.add(pick)

				arrival.setPickID(pick.publicID())
				arrival.setPhase(phase)
				origin.add(arrival)


				if amplitudeSNR is not None:
					amplitudeSNR.setPickID(pick.publicID())
					amplitudeSNR.setWaveformID(streamID)
					ep.add(amplitudeSNR)

				if amplitudeBB is not None:
					amplitudeBB.setPickID(pick.publicID())
					amplitudeBB.setWaveformID(streamID)
					ep.add(amplitudeBB)

				if stationMagBB is not None:
					stationMagBB.setWaveformID(streamID)
					origin.add(stationMagBB)
					stationMagContrib = DataModel.StationMagnitudeContribution()
					stationMagContrib.setStationMagnitudeID(stationMagBB.publicID())
					if magnitudeBB is None:
						magnitudeBB = DataModel.Magnitude.Create()
					magnitudeBB.add(stationMagContrib)

				if stationMag is not None:
					if stationMag.type() in ['mb', 'ML'] and amplitudeDisp is not None:
						amplitudeDisp.setPickID(pick.publicID())
						amplitudeDisp.setWaveformID(streamID)
						amplitudeDisp.setPeriod(DataModel.RealQuantity(ampPeriod))
						amplitudeDisp.setType(stationMag.type())
						ep.add(amplitudeDisp)

					if stationMag.type() in ['Ms(BB)'] and amplitudeVel is not None:
						amplitudeVel.setPickID(pick.publicID())
						amplitudeVel.setWaveformID(streamID)
						amplitudeVel.setPeriod(DataModel.RealQuantity(ampPeriod))
						amplitudeVel.setType(stationMag.type())
						ep.add(amplitudeVel)

					stationMag.setWaveformID(streamID)
					origin.add(stationMag)

					stationMagContrib = DataModel.StationMagnitudeContribution()
					stationMagContrib.setStationMagnitudeID(stationMag.publicID())

					magType = stationMag.type()
					if magType == 'ML':
						if magnitudeML is None:
							magnitudeML = DataModel.Magnitude.Create()
						magnitudeML.add(stationMagContrib)

					elif magType == 'Ms(BB)':
						if magnitudeMS is None:
							magnitudeMS = DataModel.Magnitude.Create()
						magnitudeMS.add(stationMagContrib)

					elif magType == 'mb':
						if magnitudeMB is None:
							magnitudeMB = DataModel.Magnitude.Create()
						magnitudeMB.add(stationMagContrib)


				pick          = None
				staCode       = None
				compCode      = None
				stationMag    = None
				stationMagBB  = None
				amplitudeDisp = None
				amplitudeVel  = None
				amplitudeSNR  = None
				amplitudeBB   = None
				continue

			# empty key
			elif len(a) == 1:
				Logging.warning('Line %i: key without value' % iLine)
				continue

			value = a[1].strip()
			if pick is None:
				pick = DataModel.Pick.Create()
				arrival = DataModel.Arrival()

			try:
				##############################################################
				# station parameters

				# station code
				if keyLower == 'station code':
					staCode = value

				# pick time
				elif keyLower == 'onset time':
					pick.setTime(DataModel.TimeQuantity(self.parseTime(value)))

				# pick onset type
				elif keyLower == 'onset type':
					found = False
					for onset in [ DataModel.EMERGENT, DataModel.IMPULSIVE,
					               DataModel.QUESTIONABLE ]:
						if value == DataModel.EPickOnsetNames_name(onset):
							pick.setOnset(onset)
							found = True
							break
					if not found:
						raise Exception('Unsupported onset value')

				# phase code
				elif keyLower == 'phase name':
					phase = DataModel.Phase()
					phase.setCode(value)
					pick.setPhaseHint(phase)

				# event type
				elif keyLower == 'event type':
					evttype = EventTypes[value]
					event.setType(evttype)
					originComments[key] = value

				# filter ID
				elif keyLower == 'applied filter':
					pick.setFilterID(value)

				# channel code, prepended by configured Channel prefix if only
				# one character is found
				elif keyLower == 'component':
					compCode = value

				# pick evaluation mode
				elif keyLower == 'pick type':
					found = False
					for mode in [ DataModel.AUTOMATIC, DataModel.MANUAL ]:
						if value == DataModel.EEvaluationModeNames_name(mode):
							pick.setEvaluationMode(mode)
							found = True
							break
					if not found:
						raise Exception('Unsupported evaluation mode value')

				# pick author
				elif keyLower == 'analyst':
					creationInfo = DataModel.CreationInfo()
					creationInfo.setAuthor(value)
					pick.setCreationInfo(creationInfo)

				# pick polarity
				# isn't tested
				elif keyLower == 'sign':
					if value == 'positive':
						sign = '0' # positive
					elif value == 'negative':
						sign = '1' # negative
					else:
						sign =  '2' # unknown
					pick.setPolarity(float(sign))

				# arrival weight
				elif keyLower == 'weight':
					arrival.setWeight(float(value))

				# arrival azimuth
				elif keyLower == 'theo. azimuth (deg)':
					arrival.setAzimuth(float(value))

				# pick theo backazimuth
				elif keyLower == 'theo. backazimuth (deg)':
					if pick.slownessMethodID() == 'corrected':
						Logging.debug('Line %i: ignoring parameter: %s' % (
						              iLine, key))
					else:
						pick.setBackazimuth(DataModel.RealQuantity(float(value)))
						pick.setSlownessMethodID('theoretical')

				# pick beam slowness
				elif keyLower == 'beam-slowness (sec/deg)':
					if pick.slownessMethodID() == 'corrected':
						Logging.debug('Line %i: ignoring parameter: %s' % (
						              iLine, key))
					else:
						pick.setHorizontalSlowness(DataModel.RealQuantity(float(value)))
						pick.setSlownessMethodID('Array Beam')

				# pick beam backazimuth
				elif keyLower == 'beam-azimuth (deg)':
					if pick.slownessMethodID() == 'corrected':
						Logging.debug('Line %i: ignoring parameter: %s' % (
						              iLine, key))
					else:
						pick.setBackazimuth(DataModel.RealQuantity(float(value)))

				# pick epi slowness
				elif keyLower == 'epi-slowness (sec/deg)':
					pick.setHorizontalSlowness(DataModel.RealQuantity(float(value)))
					pick.setSlownessMethodID('corrected')

				# pick epi backazimuth
				elif keyLower == 'epi-azimuth (deg)':
					pick.setBackazimuth(DataModel.RealQuantity(float(value)))

				# arrival distance degree
				elif keyLower == 'distance (deg)':
					arrival.setDistance(float(value))

				# arrival distance km, recalculates for degree
				elif keyLower == 'distance (km)':
					if isinstance(arrival.distance(), float):
						Logging.debug('Line %i: ignoring parameter: %s' % (
						              iLine-1, 'distance (deg)'))
					arrival.setDistance(float(value) * km2degFac)

				# arrival time residual
				elif keyLower == 'residual time':
					arrival.setTimeResidual(float(value))

				# amplitude snr
				elif keyLower == 'signal/noise':
					amplitudeSNR = DataModel.Amplitude.Create()
					amplitudeSNR.setType('SNR')
					amplitudeSNR.setAmplitude(DataModel.RealQuantity(float(value)))

				# amplitude period
				elif keyLower.startswith('period'):
					ampPeriod = float(value)

				# amplitude value for displacement
				elif keyLower == 'amplitude (nm)':
					amplitudeDisp = DataModel.Amplitude.Create()
					amplitudeDisp.setAmplitude(DataModel.RealQuantity(float(value)))
					amplitudeDisp.setUnit('nm')

				# amplitude value for velocity
				elif keyLower.startswith('vel. amplitude'):
					amplitudeVel = DataModel.Amplitude.Create()
					amplitudeVel.setAmplitude(DataModel.RealQuantity(float(value)))
					amplitudeVel.setUnit('nm/s')

				elif keyLower == 'bb amplitude (nm/sec)':
					amplitudeBB = DataModel.Amplitude.Create()
					amplitudeBB.setAmplitude(DataModel.RealQuantity(float(value)))
					amplitudeBB.setType('mB')
					amplitudeBB.setUnit('nm/s')
					amplitudeBB.setPeriod(DataModel.RealQuantity(ampBBPeriod))

				elif keyLower == 'bb period (sec)':
					ampBBPeriod = float(value)

				elif keyLower == 'broadband magnitude':
					magType = self.parseMagType('bb')
					stationMagBB = DataModel.StationMagnitude.Create()
					stationMagBB.setMagnitude(DataModel.RealQuantity(float(value)))
					stationMagBB.setType(magType)
					stationMagBB.setAmplitudeID(amplitudeBB.publicID())

				# ignored
				elif keyLower == 'quality number':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				# station magnitude value and type
				elif keyLower.startswith('magnitude '):
					magType = self.parseMagType(key[10:])
					stationMag = DataModel.StationMagnitude.Create()
					stationMag.setMagnitude(DataModel.RealQuantity(float(value)))

					if len(magType) > 0:
						stationMag.setType(magType)
					if magType == 'mb':
						stationMag.setAmplitudeID(amplitudeDisp.publicID())

					elif magType == 'MS(BB)':
						stationMag.setAmplitudeID(amplitudeVel.publicID())
					else:
						Logging.debug('Line %i: Magnitude Type not known %s.' % (
						              iLine, magType))


				###############################################################
				# origin parameters

				# event ID, added as origin comment later on
				elif keyLower == 'event id':
					originComments[key] = value

				# magnitude value and type
				elif keyLower == 'mean bb magnitude':
					magType = self.parseMagType('bb')
					if magnitudeBB is None:
						magnitudeBB = DataModel.Magnitude.Create()
					magnitudeBB.setMagnitude(DataModel.RealQuantity(float(value)))
					magnitudeBB.setType(magType)


				elif keyLower.startswith('mean magnitude '):
					magType = self.parseMagType(key[15:])

					if magType =='ML':
						if magnitudeML is None:
							magnitudeML = DataModel.Magnitude.Create()
						magnitudeML.setMagnitude(DataModel.RealQuantity(float(value)))
						magnitudeML.setType(magType)

					elif magType == 'Ms(BB)':
						if magnitudeMS is None:
							magnitudeMS = DataModel.Magnitude.Create()
						magnitudeMS.setMagnitude(DataModel.RealQuantity(float(value)))
						magnitudeMS.setType(magType)

					elif magType == 'mb':
						if magnitudeMB is None:
							magnitudeMB = DataModel.Magnitude.Create()
						magnitudeMB.setMagnitude(DataModel.RealQuantity(float(value)))
						magnitudeMB.setType(magType)

					else:
						Logging.warning('Line %i: Magnitude type %s not defined yet.' % (
						                iLine, magType))

				# latitude
				elif keyLower == 'latitude':
					origin.latitude().setValue(float(value))
					latFound = True
				elif keyLower == 'error in latitude (km)':
					origin.latitude().setUncertainty(float(value))

				# longitude
				elif keyLower == 'longitude':
					origin.longitude().setValue(float(value))
					lonFound = True
				elif keyLower == 'error in longitude (km)':
					origin.longitude().setUncertainty(float(value))

				# depth
				elif keyLower == 'depth (km)':
					origin.setDepth(DataModel.RealQuantity(float(value)))
					if depthError is not None:
						origin.depth().setUncertainty(depthError)
				elif keyLower == 'depth type':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))
				elif keyLower == 'error in depth (km)':
					depthError = float(value)
					try: origin.depth().setUncertainty(depthError)
					except Core.ValueException: pass

				# time
				elif keyLower == 'origin time':
					origin.time().setValue(self.parseTime(value))
				elif keyLower == 'error in origin time':
					origin.time().setUncertainty(float(value))

				# location method
				elif keyLower == 'location method':
					origin.setMethodID(str(value))

				# region table, added as origin comment later on
				elif keyLower == 'region table':
					originComments[key] = value

				# region table, added as origin comment later on
				elif keyLower == 'region id':
					originComments[key] = value

				# source region, added as origin comment later on
				elif keyLower == 'source region':
					originComments[key] = value

				# used station count
				elif keyLower == 'no. of stations used':
					if originQuality is None:
						originQuality = DataModel.OriginQuality()
					originQuality.setUsedStationCount(int(value))

				# ignored
				elif keyLower == 'reference location name':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				# confidence ellipsoid major axis
				elif keyLower == 'error ellipse major':
					if originCE is None:
						originCE = DataModel.ConfidenceEllipsoid()
					originCE.setSemiMajorAxisLength(float(value))

				# confidence ellipsoid minor axis
				elif keyLower == 'error ellipse minor':
					if originCE is None:
						originCE = DataModel.ConfidenceEllipsoid()
					originCE.setSemiMinorAxisLength(float(value))

				# confidence ellipsoid rotation
				elif keyLower == 'error ellipse strike':
					if originCE is None:
						originCE = DataModel.ConfidenceEllipsoid()
					originCE.setMajorAxisRotation(float(value))

				# azimuthal gap
				elif keyLower == 'max azimuthal gap (deg)':
					if originQuality is None:
						originQuality = DataModel.OriginQuality()
					originQuality.setAzimuthalGap(float(value))

				# creation info author
				elif keyLower == 'author':
					origin.creationInfo().setAuthor(value)

				# creation info agency
				elif keyLower == 'source of information':
					origin.creationInfo().setAgencyID(value)

				# earth model id
				elif keyLower == 'velocity model':
					origin.setEarthModelID(value)

				# standard error
				elif keyLower == 'rms of residuals (sec)':
					if originQuality is None:
						originQuality = DataModel.OriginQuality()
					originQuality.setStandardError(float(value))

				# ignored
				elif keyLower == 'phase flags':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				# ignored
				elif keyLower == 'location input params':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				### missing keys
				elif keyLower == 'ampl&period source':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				elif keyLower == 'location quality':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				elif keyLower == 'reference latitude':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				elif keyLower == 'reference longitude':
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))

				elif keyLower.startswith('amplitude time'):
					Logging.debug('Line %i: ignoring parameter: %s' % (
					              iLine, key))


				# unknown key
				else:
					Logging.warning('Line %i: ignoring unknown parameter: %s' \
					                % (iLine, key))

			except ValueError, ve:
				Logging.warning('Line %i: can not parse %s value' % (
				                iLine, key))
			except Exception:
				Logging.error('Line %i: %s' % (iLine,
				                               str(traceback.format_exc())))
				return None

		# check
		if not latFound:
			Logging.warning('could not add origin, missing latitude parameter')
		elif not lonFound:
			Logging.warning('could not add origin, missing longitude parameter')
		elif not origin.time().value().valid():
			Logging.warning('could not add origin, missing origin time parameter')
		else:
			if magnitudeMB is not None:
				origin.add(magnitudeMB)
			if magnitudeML is not None:
				origin.add(magnitudeML)
			if magnitudeMS is not None:
				origin.add(magnitudeMS)
			if magnitudeBB is not None:
				origin.add(magnitudeBB)

			ep.add(event)
			ep.add(origin)

			if originQuality is not None:
				origin.setQuality(originQuality)

			if originCE is not None:
				uncertainty = DataModel.OriginUncertainty()
				uncertainty.setConfidenceEllipsoid(originCE)
				origin.setUncertainty(uncertainty)

			for k,v in originComments.iteritems():
				comment = DataModel.Comment()
				comment.setId(k)
				comment.setText(v)
				origin.add(comment)


		return ep


	###########################################################################
	def run(self):
		self.loadStreams()

		try:
			if self.inputFile == '-':
				f = sys.stdin
			else:
				f = open(self.inputFile)
		except IOError, e:
			Logging.error(str(e))
			return False

		ep = self.sh2proc(f)
		if ep is  None:
			return False

		ar = IO.XMLArchive()
		ar.create('-')
		ar.setFormattedOutput(True)
		ar.writeObject(ep)
		ar.close()

		return True


###############################################################################
def main():
	try:
		app = SH2Proc()
		return app()
	except:
		sys.stderr.write(str(traceback.format_exc()))

	return 1

if __name__ == '__main__':
	sys.exit(main())



# vim: ts=4 noet
