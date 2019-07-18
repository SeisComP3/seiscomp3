################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# FDSNDataSelect -- Implements the fdsnws-dataselect Web service, see
#   http://www.fdsn.org/webservices/
#
# Feature notes:
#   - 'quality' request parameter not implemented (information not available in
#     SeisComP)
#   - 'minimumlength' parameter is not implemented
#   - 'longestonly' parameter is not implemented
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################

from twisted.cred import portal
from twisted.web import http, resource, server
from twisted.internet import interfaces, reactor

from zope.interface import implements

from seiscomp3 import Logging
from seiscomp3.Client import Application
from seiscomp3.Core import Array, Record, Time
from seiscomp3.IO import RecordInput, RecordStream

from http import HTTP, BaseResource
from request import RequestOptions
import utils

import time
import dateutil.parser
import cStringIO
from reqtrack import RequestTrackerDB
from fastsds import SDS
from seiscomp import mseedlite

VERSION = "1.1.0"

################################################################################
class _DataSelectRequestOptions(RequestOptions):

	MinTime        = Time(0, 1)

	PQuality       = [ 'quality' ]
	PMinimumLength = [ 'minimumlength' ]
	PLongestOnly   = [ 'longestonly' ]

	QualityValues  = [ 'B', 'D', 'M', 'Q', 'R' ]
	OutputFormats  = [ 'miniseed', 'mseed' ]

	POSTParams     = RequestOptions.POSTParams + \
	                 PQuality + PMinimumLength + PLongestOnly


	#---------------------------------------------------------------------------
	def __init__(self, args=None):
		RequestOptions.__init__(self, args)
		self.service = 'fdsnws-dataselect'

		self.quality = self.QualityValues[0]
		self.minimumLength = None
		self.longestOnly = None


	#---------------------------------------------------------------------------
	def _checkTimes(self, realtimeGap):
		maxEndTime = Time(self.accessTime)
		if realtimeGap is not None:
			maxEndTime -= Time(realtimeGap, 0)

		for ro in self.streams:
			# create time if non was specified
			if ro.time is None:
				ro.time = RequestOptions.Time()
			# restrict time to 1970 - now
			if ro.time.start is None or ro.time.start < self.MinTime:
				ro.time.start = self.MinTime
			if ro.time.end is None or ro.time.end > maxEndTime:
				ro.time.end = maxEndTime

		# remove items with start time >= end time
		self.streams = [ x for x in self.streams if x.time.start < x.time.end ]


	#---------------------------------------------------------------------------
	def parse(self):
		# quality (optional), currently not supported
		key, value = self.getFirstValue(self.PQuality)
		if value is not None:
			value = value.upper()
			if value in self.QualityValues:
				self.quality = value
			else:
				self.raiseValueError(key)

		# minimumlength(optional), currently not supported
		self.minimumLength = self.parseFloat(self.PMinimumLength, 0)

		# longestonly (optional), currently not supported
		self.longestOnly = self.parseBool(self.PLongestOnly)

		# generic parameters
		self.parseTime()
		self.parseChannel()
		self.parseOutput()


################################################################################
class _MyRecordStream(object):
	def __init__(self, url, trackerList, bufferSize):
		self.__url = url
		self.__trackerList = trackerList
		self.__bufferSize = bufferSize
		self.__tw = []


	def addStream(self, net, sta, loc, cha, startt, endt, restricted, archNet):
		self.__tw.append((net, sta, loc, cha, startt, endt, restricted, archNet))


	def __override_network(self, data, net):
		inp = cStringIO.StringIO(data)
		out = cStringIO.StringIO()

		for rec in mseedlite.Input(inp):
			rec.net = net
			rec_len_exp = 9

			while (1 << rec_len_exp) < rec.size:
				rec_len_exp += 1

			rec.write(out, rec_len_exp)

		return out.getvalue()


	def input(self):
		fastsdsPrefix = 'fastsds://'

		if self.__url.startswith(fastsdsPrefix):
			fastsds = SDS(self.__url[len(fastsdsPrefix):])

		else:
			fastsds = None

		for (net, sta, loc, cha, startt, endt, restricted, archNet) in self.__tw:
			if not archNet:
				archNet = net

			size = 0

			if fastsds:
				start = dateutil.parser.parse(startt.iso()).replace(tzinfo=None)
				end = dateutil.parser.parse(endt.iso()).replace(tzinfo=None)

				for data in fastsds.getRawBytes(start, end, archNet, sta, loc, cha, self.__bufferSize):
					size += len(data)

					if archNet == net:
						yield data

					else:
						try:
							yield self.__override_network(data, net)

						except Exception, e:
							Logging.error("could not override network code: %s" % str(e))

			else:
				rs = RecordStream.Open(self.__url)

				if rs is None:
					Logging.error("could not open record stream")
					break

				rs.addStream(archNet, sta, loc, cha, startt, endt)
				rsInput = RecordInput(rs, Array.INT, Record.SAVE_RAW)
				eof = False

				while not eof:
					data = ""

					while len(data) < self.__bufferSize:
						try:
							rec = rsInput.next()

						except Exception, e:
							Logging.error("%s" % str(e))
							eof = True
							break

						if rec is None:
							eof = True
							break

						data += rec.raw().str()

					if data:
						size += len(data)

						if archNet == net:
							yield data

						else:
							try:
								yield self.__override_network(data, net)

							except Exception, e:
								Logging.error("could not override network code: %s" % str(e))

			for tracker in self.__trackerList:
				net_class = 't' if net[0] in "0123456789XYZ" else 'p'

				if size == 0:
					tracker.line_status(startt, endt, net, sta, cha, loc,
					                    restricted, net_class, True, [],
					                    "fdsnws", "NODATA", 0, "")

				else:
					tracker.line_status(startt, endt, net, sta, cha, loc,
					                    restricted, net_class, True, [],
					                    "fdsnws", "OK", size, "")


################################################################################
class _WaveformProducer(object):
	implements(interfaces.IPushProducer)

	def __init__(self, req, ro, rs, fileName, trackerList):
		self.req = req
		self.ro = ro
		self.it = rs.input()

		self.fileName = fileName
		self.written = 0

		self.trackerList = trackerList
		self.paused = False
		self.stopped = False
		self.running = False


	def _flush(self, data):
		if self.stopped:
			return

		if not self.paused:
			reactor.callInThread(self._collectData)

		else:
			self.running = False

		if self.written == 0:
			self.req.setHeader('Content-Type', 'application/vnd.fdsn.mseed')
			self.req.setHeader('Content-Disposition', "attachment; " \
			                   "filename=%s" % self.fileName)

		self.req.write(data)
		self.written += len(data)


	def _finish(self):
		if self.stopped:
			return

		if self.written == 0:
			msg = "no waveform data found"
			errorpage = HTTP.renderErrorPage(self.req, http.NO_CONTENT, msg,
			                                 VERSION, self.ro)

			if errorpage:
				self.req.write(errorpage)

			for tracker in self.trackerList:
				tracker.volume_status("fdsnws", "NODATA", 0, "")
				tracker.request_status("END", "")

		else:
			Logging.debug("%s: returned %i bytes of mseed data" % (
			              self.ro.service, self.written))
			utils.accessLog(self.req, self.ro, http.OK, self.written, None)

			for tracker in self.trackerList:
				tracker.volume_status("fdsnws", "OK", self.written, "")
				tracker.request_status("END", "")

		self.req.unregisterProducer()
		self.req.finish()


	def _collectData(self):
		try:
			reactor.callFromThread(self._flush, self.it.next())

		except StopIteration:
			reactor.callFromThread(self._finish)


	def pauseProducing(self):
		self.paused = True


	def resumeProducing(self):
		self.paused = False

		if not self.running:
			self.running = True
			reactor.callInThread(self._collectData)


	def stopProducing(self):
		self.stopped = True

		Logging.debug("%s: returned %i bytes of mseed data (not completed)" % (
		              self.ro.service, self.written))
		utils.accessLog(self.req, self.ro, http.OK, self.written, "not completed")

		for tracker in self.trackerList:
			tracker.volume_status("fdsnws", "ERROR", self.written, "")
			tracker.request_status("END", "")

		self.req.unregisterProducer()
		self.req.finish()


################################################################################
class FDSNDataSelectRealm(object):
	implements(portal.IRealm)

	#---------------------------------------------------------------------------
	def __init__(self, inv, bufferSize, access):
		self.__inv = inv
		self.__bufferSize = bufferSize
		self.__access = access

	#---------------------------------------------------------------------------
	def requestAvatar(self, avatarId, mind, *interfaces):
		if resource.IResource in interfaces:
			return (resource.IResource,
			        FDSNDataSelect(self.__inv, self.__bufferSize, self.__access,
			                       {"mail": avatarId, "blacklisted": False}),
			        lambda: None)

		raise NotImplementedError()



################################################################################
class FDSNDataSelectAuthRealm(object):
	implements(portal.IRealm)

	#---------------------------------------------------------------------------
	def __init__(self, inv, bufferSize, access, userdb):
		self.__inv = inv
		self.__bufferSize = bufferSize
		self.__access = access
		self.__userdb = userdb

	#---------------------------------------------------------------------------
	def requestAvatar(self, avatarId, mind, *interfaces):
		if resource.IResource in interfaces:
			return (resource.IResource,
			        FDSNDataSelect(self.__inv, self.__bufferSize, self.__access,
			                       self.__userdb.getAttributes(avatarId)),
			        lambda: None)

		raise NotImplementedError()



################################################################################
class FDSNDataSelect(BaseResource):
	isLeaf = True

	#---------------------------------------------------------------------------
	def __init__(self, inv, bufferSize, access=None, user=None):
		BaseResource.__init__(self, VERSION)
		self._rsURL = Application.Instance().recordStreamURL()
		self.__inv = inv
		self.__access = access
		self.__user = user
		self.__bufferSize = bufferSize


	#---------------------------------------------------------------------------
	def render_OPTIONS(self, req):
		req.setHeader('Access-Control-Allow-Origin', '*')
		req.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
		req.setHeader('Access-Control-Allow-Headers',
		              'Accept, Content-Type, X-Requested-With, Origin')
		req.setHeader('Content-Type', 'text/plain')
		return ""


	#---------------------------------------------------------------------------
	def render_GET(self, req):
		# Parse and validate POST parameters
		ro = _DataSelectRequestOptions(req.args)
		ro.userName = self.__user and self.__user.get('mail')
		try:
			ro.parse()
			# the GET operation supports exactly one stream filter
			ro.streams.append(ro)
		except ValueError, e:
			Logging.warning(str(e))
			return self.renderErrorPage(req, http.BAD_REQUEST, str(e), ro)

		return self._processRequest(req, ro)


	#---------------------------------------------------------------------------
	def render_POST(self, req):
		# Parse and validate POST parameters
		ro = _DataSelectRequestOptions()
		ro.userName = self.__user and self.__user.get('mail')
		try:
			ro.parsePOST(req.content)
			ro.parse()
		except ValueError, e:
			Logging.warning(str(e))
			return self.renderErrorPage(req, http.BAD_REQUEST, str(e), ro)

		return self._processRequest(req, ro)


	#-----------------------------------------------------------------------
	def _networkIter(self, ro):
		for i in xrange(self.__inv.networkCount()):
			net = self.__inv.network(i)

			# network code
			if ro.channel and not ro.channel.matchNet(net.code()):
				continue

			# start and end time
			if ro.time:
				try: end = net.end()
				except ValueError: end = None
				if not ro.time.match(net.start(), end):
					continue

			yield net


	#---------------------------------------------------------------------------
	def _stationIter(self, net, ro):
		for i in xrange(net.stationCount()):
			sta = net.station(i)

			# station code
			if ro.channel and not ro.channel.matchSta(sta.code()):
				continue

			# start and end time
			if ro.time:
				try: end = sta.end()
				except ValueError: end = None
				if not ro.time.match(sta.start(), end):
					continue

			yield sta


	#---------------------------------------------------------------------------
	def _locationIter(self, sta, ro):
		for i in xrange(sta.sensorLocationCount()):
			loc = sta.sensorLocation(i)

			# location code
			if ro.channel and not ro.channel.matchLoc(loc.code()):
				continue

			# start and end time
			if ro.time:
				try: end = loc.end()
				except ValueError: end = None
				if not ro.time.match(loc.start(), end):
					continue

			yield loc


	#---------------------------------------------------------------------------
	def _streamIter(self, loc, ro):
		for i in xrange(loc.streamCount()):
			stream = loc.stream(i)

			# stream code
			if ro.channel and not ro.channel.matchCha(stream.code()):
				continue

			# start and end time
			if ro.time:
				try: end = stream.end()
				except ValueError: end = None
				if not ro.time.match(stream.start(), end):
					continue

			yield stream


	#---------------------------------------------------------------------------
	def _processRequest(self, req, ro):

		if ro.quality != 'B' and ro.quality != 'M':
			msg = "quality other than 'B' or 'M' not supported"
			return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		if ro.minimumLength:
			msg = "enforcing of minimum record length not supported"
			return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		if ro.longestOnly:
			msg = "limitation to longest segment not supported"
			return self.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		app = Application.Instance()
		ro._checkTimes(app._realtimeGap)

		maxSamples = None
		if app._samplesM is not None:
			maxSamples = app._samplesM * 1000000
			samples = 0

		trackerList = []

		if app._trackdbEnabled or app._requestLog:
			xff = req.requestHeaders.getRawHeaders("x-forwarded-for")
			if xff:
				userIP = xff[0].split(",")[0].strip()
			else:
				userIP = req.getClientIP()

			clientID = req.getHeader("User-Agent")
			if clientID:
				clientID = clientID[:80]
			else:
				clientID = "fdsnws"

		if app._trackdbEnabled:
			if ro.userName:
				userID = ro.userName
			else:
				userID = app._trackdbDefaultUser

			reqID = 'ws' + str(int(round(time.time() * 1000) - 1420070400000))
			tracker = RequestTrackerDB(clientID, app.connection(), reqID,
			                           "WAVEFORM", userID,
			                           "REQUEST WAVEFORM " + reqID,
			                           "fdsnws", userIP, req.getClientIP())

			trackerList.append(tracker)

		if app._requestLog:
			tracker = app._requestLog.tracker(ro.service, ro.userName, userIP, clientID)
			trackerList.append(tracker)

		# Open record stream
		rs = _MyRecordStream(self._rsURL, trackerList, self.__bufferSize)

		forbidden = None

		# Add request streams
		# iterate over inventory networks
		for s in ro.streams:
			for net in self._networkIter(s):
				netRestricted = utils.isRestricted(net)
				if not trackerList and netRestricted and not self.__user:
					forbidden = forbidden or (forbidden is None)
					continue
				for sta in self._stationIter(net, s):
					staRestricted = utils.isRestricted(sta)
					if not trackerList and staRestricted and not self.__user:
						forbidden = forbidden or (forbidden is None)
						continue
					for loc in self._locationIter(sta, s):
						for cha in self._streamIter(loc, s):
							try:
								start_time = max(cha.start(), s.time.start)

							except Exception:
								start_time = s.time.start

							try:
								end_time = min(cha.end(), s.time.end)

							except Exception:
								end_time = s.time.end

							if ( netRestricted or staRestricted or \
							     utils.isRestricted(cha) ) and \
							    (not self.__user or (self.__access and
							     not self.__access.authorize(self.__user,
							         net.code(), sta.code(), loc.code(),
							         cha.code(), start_time, end_time))):

								for tracker in trackerList:
									net_class = 't' if net.code()[0] in "0123456789XYZ" else 'p'
									tracker.line_status(start_time, end_time,
									    net.code(), sta.code(), cha.code(), loc.code(),
									    True, net_class, True, [],
									    "fdsnws", "DENIED", 0, "")

								forbidden = forbidden or (forbidden is None)
								continue

							forbidden = False

							# enforce maximum sample per request restriction
							if maxSamples is not None:
								try:
									n = cha.sampleRateNumerator()
									d = cha.sampleRateDenominator()
								except ValueError:
									msg = "skipping stream without sampling " \
									      "rate definition: %s.%s.%s.%s" % (
									      net.code(), sta.code(), loc.code(),
									      cha.code())
									Logging.warning(msg)
									continue

								# calculate number of samples for requested
								# time window
								diffSec = (end_time - start_time).length()
								samples += int(diffSec * n / d)
								if samples > maxSamples:
									msg = "maximum number of %sM samples " \
									      "exceeded" % str(app._samplesM)
									return self.renderErrorPage(req,
									       http.REQUEST_ENTITY_TOO_LARGE, msg,
									       ro)

							Logging.debug("adding stream: %s.%s.%s.%s %s - %s" \
							              % (net.code(), sta.code(), loc.code(),
							                 cha.code(), start_time.iso(),
							                 end_time.iso()))
							rs.addStream(net.code(), sta.code(), loc.code(),
							             cha.code(), start_time, end_time,
							             utils.isRestricted(cha),
							             sta.archiveNetworkCode())

		if forbidden:
			for tracker in trackerList:
				tracker.volume_status("fdsnws", "DENIED", 0, "")
				tracker.request_status("END", "")

			msg = "access denied"
			return self.renderErrorPage(req, http.FORBIDDEN, msg, ro)

		elif forbidden is None:
			for tracker in trackerList:
				tracker.volume_status("fdsnws", "NODATA", 0, "")
				tracker.request_status("END", "")

			msg = "no metadata found"
			return self.renderErrorPage(req, http.NO_CONTENT, msg, ro)

		# Build output filename
		fileName = Application.Instance()._fileNamePrefix.replace("%time",
		           time.strftime('%Y-%m-%dT%H:%M:%S')) + '.mseed'

		# Create producer for async IO
		prod = _WaveformProducer(req, ro, rs, fileName, trackerList)
		req.registerProducer(prod, True)
		prod.resumeProducing()

		# The request is handled by the deferred object
		return server.NOT_DONE_YET


# vim: ts=4 noet
