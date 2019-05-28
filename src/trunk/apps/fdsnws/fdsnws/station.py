################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# FDSNStation -- Implements the fdsnws-station Web service, see
#   http://www.fdsn.org/webservices/
#
# Feature notes:
#   - 'updatedafter' request parameter not implemented: The last modification
#     time in SeisComP is tracked on the object level. If a child of an object
#     is updated the update time is not propagated to all parents. In order to
#     check if a station was updated all children must be evaluated recursively.
#     This operation would be much to expensive.
#   - additional request parameters:
#     - formatted:       boolean, default: false
#   - additional values of request parameters:
#     - format
#       - standard:      [xml, text]
#       - additional:    [fdsnxml (=xml), stationxml, sc3ml]
#       - default:       xml
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################

from twisted.internet.threads import deferToThread
from twisted.web import http, resource, server

from seiscomp3 import DataModel, Logging
from seiscomp3.Client import Application
from seiscomp3.Core import Time
from seiscomp3.IO import Exporter, ExportObjectList

from http import BaseResource
from request import RequestOptions
import utils

VERSION = "1.1.0"

################################################################################
class _StationRequestOptions(RequestOptions):

	Exporters      = { 'xml'       : 'fdsnxml',
	                   'fdsnxml'   : 'fdsnxml',
	                   'stationxml': 'staxml',
	                   'sc3ml'     : 'trunk' }
	MinTime        = Time(0, 1)

	VText          = [ 'text' ]
	OutputFormats  = Exporters.keys() + VText

	PLevel                = [ 'level' ]
	PIncludeRestricted    = [ 'includerestricted' ]
	PIncludeAvailability  = [ 'includeavailability' ]
	PUpdateAfter          = [ 'updateafter' ]
	PMatchTimeSeries      = [ 'matchtimeseries' ]

	# non standard parameters
	PFormatted     = [ 'formatted' ]

	POSTParams     = RequestOptions.POSTParams + RequestOptions.GeoParams + \
	                 PLevel + PIncludeRestricted + PIncludeAvailability + \
	                 PUpdateAfter + PMatchTimeSeries + PFormatted


	#---------------------------------------------------------------------------
	def __init__(self, args=None):
		RequestOptions.__init__(self, args)
		self.service = 'fdsnws-station'

		self.includeSta = True
		self.includeCha = False
		self.includeRes = False

		self.restricted      = None
		self.availability    = None
		self.updatedAfter    = None
		self.matchTimeSeries = None

		# non standard parameters
		self.formatted  = None


	#---------------------------------------------------------------------------
	def parse(self):
		self.parseTime(True)
		self.parseChannel()
		self.parseGeo()
		self.parseOutput()

		# level: [network, station, channel, response]
		key, value = self.getFirstValue(self.PLevel)
		if value is not None:
			value = value.lower()
			if value ==  'network' or value == 'net':
				self.includeSta = False
			elif value == 'channel' or value == 'cha' or value == 'chan':
				self.includeCha = True
			elif value == 'response' or value == 'res' or value == 'resp':
				self.includeCha = True
				self.includeRes = True
			elif value != 'station' and value != 'sta':
				self.raiseValueError(key)

		# includeRestricted (optional)
		self.restricted = self.parseBool(self.PIncludeRestricted)

		# includeAvailability (optional)
		self.availability = self.parseBool(self.PIncludeAvailability)

		# updatedAfter (optional), currently not supported
		self.updatedAfter = self.parseTimeStr(self.PUpdateAfter)

		# includeAvailability (optional)
		self.matchTimeSeries = self.parseBool(self.PMatchTimeSeries)

		# format XML
		self.formatted = self.parseBool(self.PFormatted)


	#---------------------------------------------------------------------------
	def networkIter(self, inv, matchTime=False):
		for i in xrange(inv.networkCount()):
			net = inv.network(i)

			for ro in self.streams:
				# network code
				if ro.channel and not ro.channel.matchNet(net.code()):
					continue

				# start and end time
				if matchTime and ro.time:
					try: end = net.end()
					except ValueError: end = None
					if not ro.time.match(net.start(), end):
						continue

				yield net
				break


	#---------------------------------------------------------------------------
	def stationIter(self, net, matchTime=False):
		for i in xrange(net.stationCount()):
			sta = net.station(i)

			# geographic location
			if self.geo:
				try:
					lat = sta.latitude()
					lon = sta.longitude()
				except ValueError: continue
				if not self.geo.match(lat, lon):
					continue

			for ro in self.streams:
				# station code
				if ro.channel and (not ro.channel.matchSta(sta.code()) or \
				                   not ro.channel.matchNet(net.code())):
					continue

				# start and end time
				if matchTime and ro.time:
					try: end = sta.end()
					except ValueError: end = None
					if not ro.time.match(sta.start(), end):
						continue

				yield sta
				break


	#---------------------------------------------------------------------------
	def locationIter(self, net, sta, matchTime=False):
		for i in xrange(sta.sensorLocationCount()):
			loc = sta.sensorLocation(i)

			for ro in self.streams:
				# location code
				if ro.channel and (not ro.channel.matchLoc(loc.code()) or \
				                   not ro.channel.matchSta(sta.code()) or \
				                   not ro.channel.matchNet(net.code())):
					continue

				# start and end time
				if matchTime and ro.time:
					try: end = loc.end()
					except ValueError: end = None
					if not ro.time.match(loc.start(), end):
						continue

				yield loc
				break


	#---------------------------------------------------------------------------
	def streamIter(self, net, sta, loc, matchTime, dac):
		for i in xrange(loc.streamCount()):
			stream = loc.stream(i)

			for ro in self.streams:
				# stream code
				if ro.channel and (not ro.channel.matchCha(stream.code()) or \
				                   not ro.channel.matchLoc(loc.code()) or \
				                   not ro.channel.matchSta(sta.code()) or \
				                   not ro.channel.matchNet(net.code())):
					continue

				# start and end time
				if matchTime and ro.time:
					try: end = stream.end()
					except ValueError: end = None
					if not ro.time.match(stream.start(), end):
						continue

				# match data availability extent
				if dac is not None and ro.matchTimeSeries:
					extent = dac.extent(net.code(), sta.code(), loc.code(),
					                    stream.code())
					if extent is None or ( ro.time and \
					   not ro.time.match(extent.start(), extent.end()) ):
						continue

				yield stream
				break


################################################################################
class FDSNStation(BaseResource):
	isLeaf = True

	#---------------------------------------------------------------------------
	def __init__(self, inv, restricted, maxObj, daEnabled):
		BaseResource.__init__(self, VERSION)
		self._inv = inv
		self._allowRestricted = restricted
		self._maxObj = maxObj
		self._daEnabled = daEnabled

		# additional object count dependent on detail level
		self._resLevelCount = inv.responsePAZCount() + inv.responseFIRCount() \
		                      + inv.responsePolynomialCount() + inv.responseIIRCount() \
		                      + inv.responseFAPCount()
		for i in xrange(inv.dataloggerCount()):
			self._resLevelCount += inv.datalogger(i).decimationCount()


	#---------------------------------------------------------------------------
	def render_OPTIONS(self, req):
		req.setHeader('Access-Control-Allow-Origin', '*')
		req.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
		req.setHeader('Access-Control-Allow-Headers', 'Accept, Content-Type, X-Requested-With, Origin')
		req.setHeader('Content-Type', 'text/plain')
		return ""


	#---------------------------------------------------------------------------
	def render_GET(self, req):
		# Parse and validate GET parameters
		ro = _StationRequestOptions(req.args)
		try:
			ro.parse()
			# the GET operation supports exactly one stream filter
			ro.streams.append(ro)
		except ValueError, e:
			Logging.warning(str(e))
			return self.renderErrorPage(req, http.BAD_REQUEST, str(e), ro)

		return self._prepareRequest(req, ro)


	#---------------------------------------------------------------------------
	def render_POST(self, req):
		# Parse and validate POST parameters
		ro = _StationRequestOptions()
		try:
			ro.parsePOST(req.content)
			ro.parse()
		except ValueError, e:
			Logging.warning(str(e))
			return self.renderErrorPage(req, http.BAD_REQUEST, str(e), ro)

		return self._prepareRequest(req, ro)


	#---------------------------------------------------------------------------
	def _prepareRequest(self, req, ro):
		if ro.availability and not self._daEnabled:
			msg = "including of availability information not supported"
			return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		if ro.updatedAfter:
			msg = "filtering based on update time not supported"
			return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		if ro.matchTimeSeries and not self._daEnabled:
			msg = "filtering based on available time series not supported"
			return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		# load data availability if requested
		dac = None
		if ro.availability or ro.matchTimeSeries:
			dac = Application.Instance().getDACache()
			if dac is None or len(dac.extents()) == 0:
				msg = "no data availabiltiy extent information found"
				return self.renderErrorPage(req, http.NO_CONTENT, msg, ro)

		# Exporter, 'None' is used for text output
		if ro.format in ro.VText:
			if ro.includeRes:
				msg = "response level output not available in text format"
				return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)
			req.setHeader('Content-Type', 'text/plain')
			d = deferToThread(self._processRequestText, req, ro, dac)
		else:
			exp = Exporter.Create(ro.Exporters[ro.format])
			if exp is None:
				msg = "output format '%s' no available, export module '%s' " \
				      "could not be loaded." % (
				      ro.format, ro.Exporters[ro.format])
				return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

			req.setHeader('Content-Type', 'application/xml')
			exp.setFormattedOutput(bool(ro.formatted))
			d = deferToThread(self._processRequestExp, req, ro, exp, dac)

		req.notifyFinish().addErrback(utils.onCancel, d)
		d.addBoth(utils.onFinish, req)

		# The request is handled by the deferred object
		return server.NOT_DONE_YET


	#---------------------------------------------------------------------------
	def _processRequestExp(self, req, ro, exp, dac):
		if req._disconnected: return False

		staCount, locCount, chaCount, extCount, objCount = 0, 0, 0, 0, 0

		DataModel.PublicObject.SetRegistrationEnabled(False)
		newInv = DataModel.Inventory()
		dataloggers, sensors, extents = set(), set(), set()

		skipRestricted = not self._allowRestricted or \
		                 (ro.restricted is not None and not ro.restricted)
		levelNet = not ro.includeSta
		levelSta = ro.includeSta and not ro.includeCha

		# iterate over inventory networks
		for net in ro.networkIter(self._inv, levelNet):
			if req._disconnected: return False
			if skipRestricted and utils.isRestricted(net): continue
			newNet = DataModel.Network(net)

			# Copy comments
			for i in xrange(net.commentCount()):
				newNet.add(DataModel.Comment(net.comment(i)))

			# iterate over inventory stations of current network
			for sta in ro.stationIter(net, levelSta):
				if req._disconnected:
					return False
				if skipRestricted and utils.isRestricted(sta):
					continue
				if not self.checkObjects(req, objCount, self._maxObj):
					return False

				if ro.includeCha:
					numCha, numLoc, d, s, e = \
						self._processStation(newNet, net, sta, ro, dac,
						                     skipRestricted)
					if numCha > 0:
						locCount += numLoc
						chaCount += numCha
						extCount += len(e)
						objCount += numLoc + numCha + extCount
						if not self.checkObjects(req, objCount, self._maxObj):
							return False
						dataloggers |= d
						sensors |= s
						extents |= e
				elif self._matchStation(net, sta, ro, dac):
					if ro.includeSta:
						newSta = DataModel.Station(sta)
						# Copy comments
						for i in xrange(sta.commentCount()):
							newSta.add(DataModel.Comment(sta.comment(i)))
						newNet.add(newSta)
					else:
						# no station output requested: one matching station
						# is sufficient to include the network
						newInv.add(newNet)
						objCount += 1
						break

			if newNet.stationCount() > 0:
				newInv.add(newNet)
				staCount += newNet.stationCount()
				objCount += staCount + 1

		# Return 204 if no matching inventory was found
		if newInv.networkCount() == 0:
			msg = "no matching inventory found"
			data = self.renderErrorPage(req, http.NO_CONTENT, msg, ro)
			if data:
				utils.writeTS(req, data)
			return True

		# Copy references (dataloggers, responses, sensors)
		decCount, resCount = 0, 0
		if ro.includeCha:
			decCount = self._copyReferences(newInv, req, objCount, self._inv,
			           ro, dataloggers, sensors, self._maxObj)
			if decCount is None:
				return False
			else:
				resCount = newInv.responsePAZCount() + \
				           newInv.responseFIRCount() + \
				           newInv.responsePolynomialCount() + \
				           newInv.responseFAPCount() + \
				           newInv.responseIIRCount()
				objCount += resCount + decCount + newInv.dataloggerCount() + \
				            newInv.sensorCount()

		# Copy data extents
		objOut = newInv
		if len(extents) > 0:
			objCount += 1
			da = DataModel.DataAvailability()
			for e in extents:
				da.add(DataModel.DataExtent(e))
			objOut = ExportObjectList()
			objOut.append(newInv)
			objOut.append(da)

		sink = utils.Sink(req)
		if not exp.write(sink, objOut):
			return False

		Logging.debug("%s: returned %iNet, %iSta, %iLoc, %iCha, " \
		               "%iDL, %iDec, %iSen, %iRes, %iDAExt (total objects/" \
		               "bytes: %i/%i) " % (ro.service, newInv.networkCount(),
		               staCount, locCount, chaCount, newInv.dataloggerCount(),
		               decCount, newInv.sensorCount(), resCount, extCount,
		               objCount, sink.written))
		utils.accessLog(req, ro, http.OK, sink.written, None)
		return True


	#---------------------------------------------------------------------------
	def _formatEpoch(self, obj):
		df = "%FT%T"
		dfMS = "%FT%T.%f"

		if obj.start().microseconds() > 0:
			start = obj.start().toString(dfMS)
		else:
			start = obj.start().toString(df)

		try:
			if obj.end().microseconds() > 0:
				end = obj.end().toString(dfMS)
			else:
				end = obj.end().toString(df)
		except ValueError:
			end = ''

		return start, end


	#---------------------------------------------------------------------------
	def _processRequestText(self, req, ro, dac):
		if req._disconnected: return False

		skipRestricted = not self._allowRestricted or \
		                 (ro.restricted is not None and not ro.restricted)

		data = ""
		lines = []

		# level = network
		if not ro.includeSta:
			data = "#Network|Description|StartTime|EndTime|TotalStations\n"

			# iterate over inventory networks
			for net in ro.networkIter(self._inv, True):
				if req._disconnected: return False
				if skipRestricted and utils.isRestricted(net): continue

				# at least one matching station is required
				stationFound = False
				for sta in ro.stationIter(net, False):
					if req._disconnected: return False
					if self._matchStation(net, sta, ro, dac) and \
					   not (skipRestricted and utils.isRestricted(sta)):
						stationFound = True
						break
				if not stationFound: continue

				start, end = self._formatEpoch(net)
				lines.append(("%s %s" % (net.code(), start),
				              "%s|%s|%s|%s|%i\n" % (
				                  net.code(), net.description(), start, end,
				                  net.stationCount())))

		# level = station
		elif not ro.includeCha:
			data = "#Network|Station|Latitude|Longitude|Elevation|" \
			       "SiteName|StartTime|EndTime\n"

			# iterate over inventory networks
			for net in ro.networkIter(self._inv, False):
				if req._disconnected: return False
				if skipRestricted and utils.isRestricted(net): continue
				# iterate over inventory stations
				for sta in ro.stationIter(net, True):
					if req._disconnected: return False
					if not self._matchStation(net, sta, ro, dac) or \
					   (skipRestricted and utils.isRestricted(sta)): continue

					try: lat = str(sta.latitude())
					except ValueError: lat = ''
					try: lon = str(sta.longitude())
					except ValueError: lon = ''
					try: elev = str(sta.elevation())
					except ValueError: elev = ''
					try: desc = sta.description()
					except ValueError: desc = ''

					start, end = self._formatEpoch(sta)
					lines.append(("%s.%s %s" % (net.code(), sta.code(), start),
					              "%s|%s|%s|%s|%s|%s|%s|%s\n" % (
					                  net.code(), sta.code(), lat, lon, elev,
					                  desc, start, end)))

		# level = channel (resonse level not supported in text format)
		else:
			data = "#Network|Station|Location|Channel|Latitude|Longitude|" \
			       "Elevation|Depth|Azimuth|Dip|SensorDescription|Scale|" \
			       "ScaleFreq|ScaleUnits|SampleRate|StartTime|EndTime\n"

			# iterate over inventory networks
			for net in ro.networkIter(self._inv, False):
				if req._disconnected: return False
				if skipRestricted and utils.isRestricted(net): continue
				# iterate over inventory stations, locations, streams
				for sta in ro.stationIter(net, False):
					if req._disconnected: return False
					if skipRestricted and utils.isRestricted(sta): continue
					for loc in ro.locationIter(net, sta, True):
						for stream in ro.streamIter(net, sta, loc, True, dac):
							if skipRestricted and utils.isRestricted(stream): continue

							try: lat = str(loc.latitude())
							except ValueError: lat = ''
							try: lon = str(loc.longitude())
							except ValueError: lon = ''
							try: elev = str(loc.elevation())
							except ValueError: elev = ''
							try: depth = str(stream.depth())
							except ValueError: depth = ''
							try: azi = str(stream.azimuth())
							except ValueError: azi = ''
							try: dip = str(stream.dip())
							except ValueError: dip = ''

							desc = ''
							try:
								sensor = self._inv.findSensor(stream.sensor())
								if sensor is not None:
									desc = sensor.description()
							except ValueError: pass

							try: scale = str(stream.gain())
							except ValueError: scale = ''
							try: scaleFreq = str(stream.gainFrequency())
							except ValueError: scaleFreq = ''
							try: scaleUnit = str(stream.gainUnit())
							except ValueError: scaleUnit = ''
							try:
								sr = str(stream.sampleRateNumerator() /
								     float(stream.sampleRateDenominator()))
							except ValueError, ZeroDevisionError:
								sr = ''

							start, end = self._formatEpoch(stream)
							lines.append(("%s.%s.%s.%s %s" % (
							                  net.code(), sta.code(),
							                  loc.code(), stream.code(), start),
							              "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|" \
							              "%s|%s|%s|%s|%s|%s\n" % (
							                  net.code(), sta.code(),
							                  loc.code(), stream.code(), lat,
							                  lon, elev, depth, azi, dip, desc,
							                  scale, scaleFreq, scaleUnit, sr,
							                  start, end)))

		# sort lines and append to final data string
		lines.sort(key = lambda line: line[0])
		for line in lines:
			data += line[1]

		# Return 204 if no matching inventory was found
		if len(lines) == 0:
			msg = "no matching inventory found"
			data = self.renderErrorPage(req, http.NO_CONTENT, msg, ro)
			if data:
				utils.writeTS(req, data)
			return False

		utils.writeTS(req, data)
		Logging.debug("%s: returned %i lines (total bytes: %i)" % (
		              ro.service, len(lines), len(data)))
		utils.accessLog(req, ro, http.OK, len(data), None)
		return True


	#---------------------------------------------------------------------------
	# Checks if at least one location and channel combination matches the
	# request options
	@staticmethod
	def _matchStation(net, sta, ro, dac):
		# No filter: return true immediately
		if dac is None and \
		   ( not ro.channel or ( not ro.channel.loc and not ro.channel.cha ) ):
			return True

		for loc in ro.locationIter(net, sta, False):
			if dac is None and not ro.channel.cha and not ro.time:
				return True

			for stream in ro.streamIter(net, sta, loc, False, dac):
				return True

		return False


	#---------------------------------------------------------------------------
	# Adds a deep copy of the specified station to the new network if the
	# location and channel combination matches the request options (if any)
	@staticmethod
	def _processStation(newNet, net, sta, ro, dac, skipRestricted):
		chaCount = 0
		dataloggers, sensors, extents = set(), set(), set()
		newSta = DataModel.Station(sta)
		includeAvailability = dac is not None and ro.availability

		# Copy comments
		for i in xrange(sta.commentCount()):
			newSta.add(DataModel.Comment(sta.comment(i)))

		for loc in ro.locationIter(net, sta, True):
			newLoc = DataModel.SensorLocation(loc)
			# Copy comments
			for i in xrange(loc.commentCount()):
				newLoc.add(DataModel.Comment(loc.comment(i)))

			for stream in ro.streamIter(net, sta, loc, True, dac):
				if skipRestricted and utils.isRestricted(stream): continue
				newCha = DataModel.Stream(stream)
				# Copy comments
				for i in xrange(stream.commentCount()):
					newCha.add(DataModel.Comment(stream.comment(i)))
				newLoc.add(newCha)
				dataloggers.add(stream.datalogger())
				sensors.add(stream.sensor())
				if includeAvailability:
					ext = dac.extent(net.code(), sta.code(), loc.code(),
					                 stream.code())
					if ext is not None:
						extents.add(ext)

			if newLoc.streamCount() > 0:
				newSta.add(newLoc)
				chaCount += newLoc.streamCount()

		if newSta.sensorLocationCount() > 0:
			newNet.add(newSta)
			return chaCount, newSta.sensorLocationCount(), dataloggers, \
			       sensors, extents

		return 0, 0, [], [], []


	#---------------------------------------------------------------------------
	# Copy references (data loggers, sensors, responses) depended on request
	# options
	@staticmethod
	def _copyReferences(newInv, req, objCount, inv, ro, dataloggers, sensors,
	                    maxObj):

		responses = set()
		decCount = 0

		# datalogger
		for i in xrange(inv.dataloggerCount()):
			if req._disconnected: return None
			logger = inv.datalogger(i)
			if logger.publicID() not in dataloggers:
				continue
			newLogger = DataModel.Datalogger(logger)
			newInv.add(newLogger)
			# decimations are only needed for responses
			if ro.includeRes:
				for j in xrange(logger.decimationCount()):
					decimation = logger.decimation(j)
					newLogger.add(DataModel.Decimation(decimation))

					# collect response ids
					filterStr = ""
					try: filterStr = decimation.analogueFilterChain().content() + " "
					except ValueError: pass
					try: filterStr += decimation.digitalFilterChain().content()
					except ValueError: pass
					for resp in filterStr.split():
						responses.add(resp)
				decCount += newLogger.decimationCount()

		objCount += newInv.dataloggerCount() + decCount
		resCount = len(responses)
		if not self.checkObjects(req, objCount + resCount, maxObj):
			return None

		# sensor
		for i in xrange(inv.sensorCount()):
			if req._disconnected: return None
			sensor = inv.sensor(i)
			if sensor.publicID() not in sensors:
				continue
			newSensor = DataModel.Sensor(sensor)
			newInv.add(newSensor)
			resp = newSensor.response()
			if resp:
				if ro.includeRes:
					responses.add(resp)
				else:
					# no responses: remove response reference to avoid missing
					# response warning of exporter
					newSensor.setResponse("")

		objCount += newInv.sensorCount()
		resCount = len(responses)
		if not self.checkObjects(req, objCount + resCount, maxObj):
			return None

		# responses
		if ro.includeRes:
			if req._disconnected: return None
			for i in xrange(inv.responsePAZCount()):
				resp = inv.responsePAZ(i)
				if resp.publicID() in responses:
					newInv.add(DataModel.ResponsePAZ(resp))
			if req._disconnected: return None
			for i in xrange(inv.responseFIRCount()):
				resp = inv.responseFIR(i)
				if resp.publicID() in responses:
					newInv.add(DataModel.ResponseFIR(resp))
			if req._disconnected: return None
			for i in xrange(inv.responsePolynomialCount()):
				resp = inv.responsePolynomial(i)
				if resp.publicID() in responses:
					newInv.add(DataModel.ResponsePolynomial(resp))
			if req._disconnected: return None
			for i in xrange(inv.responseFAPCount()):
				resp = inv.responseFAP(i)
				if resp.publicID() in responses:
					newInv.add(DataModel.ResponseFAP(resp))
			if req._disconnected: return None
			for i in xrange(inv.responseIIRCount()):
				resp = inv.responseIIR(i)
				if resp.publicID() in responses:
					newInv.add(DataModel.ResponseIIR(resp))

		return decCount


# vim: ts=4 noet
