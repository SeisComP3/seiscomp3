################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# FDSNEvent -- Implements the fdsnws-event Web service, see
#   http://www.fdsn.org/webservices/
#
# Feature notes:
#   - SeisComP does not distinguish between catalogs and contributors, but
#     supports agencyIDs. Hence, if specified, the value of the 'contributor'
#     parameter is mapped to the agencyID.
#   - origin and magnitude filter parameters are always applied to
#     preferred origin resp. preferred magnitude
#   - 'updateafter' request parameter not implemented
#   - additional request parameters:
#     - includepicks:    boolean, default: true,
#                        available only in combination with includearrivals=true
#     - includecomments: boolean, default: false
#     - formatted:       boolean, default: false
#   - additional values of request parameters:
#     - format
#       - standard:      [xml, text]
#       - additional:    [qml (=xml), qml-rt, sc3ml, csv]
#       - default:       xml
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################

from twisted.internet.threads import deferToThread
from twisted.web import http, resource, server

from seiscomp3 import DataModel, Logging
from seiscomp3.Client import Application
from seiscomp3.IO import DatabaseInterface, Exporter

from http import HTTP
from request import RequestOptions
import utils


DBMaxUInt = 18446744073709551615 # 2^64 - 1

VERSION = "1.2.0"

################################################################################
class _EventRequestOptions(RequestOptions):

	Exporters = { 'xml'    : 'qml1.2',
	              'qml'    : 'qml1.2',
	              'qml-rt' : 'qml1.2rt',
	              'sc3ml'  : 'trunk',
	              'csv'    : 'csv' }
	VText          = [ 'text' ]
	VOrderBy       = [ 'time', 'time-asc', 'magnitude', 'magnitude-asc' ]
	OutputFormats  = Exporters.keys() + VText


	PMinDepth      = [ 'mindepth' ]
	PMaxDepth      = [ 'maxdepth' ]
	PMinMag        = [ 'minmagnitude', 'minmag' ]
	PMaxMag        = [ 'maxmagnitude', 'maxmag' ]
	PMagType       = [ 'magnitudetype', 'magtype' ]

	PAllOrigins    = [ 'includeallorigins', 'allorigins' ]
	PAllMags       = [ 'includeallmagnitudes', 'allmagnitudes', 'allmags' ]
	PArrivals      = [ 'includearrivals', 'allarrivals' ]

	PEventID       = [ 'eventid' ]

	PLimit         = [ 'limit' ]
	POffset        = [ 'offset' ]
	POrderBy       = [ 'orderby' ]

	PContributor   = [ 'contributor' ]
	PCatalog       = [ 'catalog' ]
	PUpdateAfter   = [ 'updateafter' ]

	# non standard parameters
	PPicks         = [ 'includepicks', 'picks' ]
	PFM            = [ 'includefocalmechanism', 'focalmechanism', 'fm' ]
	PAllFMs        = [ 'includeallfocalmechanisms', 'allfocalmechanisms', 'allfms' ]
	PStaMTs        = [ 'includestationmts', 'stationmts', 'stamts' ]
	PComments      = [ 'includecomments', 'comments' ]
	PFormatted     = [ 'formatted' ]


	#---------------------------------------------------------------------------
	class Depth:
		def __init__(self):
			self.min = None
			self.max = None

	#---------------------------------------------------------------------------
	class Magnitude:
		def __init__(self):
			self.min  = None
			self.max  = None
			self.type = None


	#---------------------------------------------------------------------------
	def __init__(self, args):
		RequestOptions.__init__(self, args)
		self.service = 'fdsnws-event'

		self.depth        = None
		self.mag          = None

		self.allOrigins   = None
		self.allMags      = None
		self.arrivals     = None

		self.limit        = None   # event limit, if defined: min 1
		self.offset       = None   # start at specific event count position,
		                           # the spec uses a weird offset definition
		                           # where an offset of '1' returns the first
		                           # element, not the second one
		self.orderBy      = None   # [time, time-asc, magnitude, magnitude-asc]
		self.catalogs     = []     # not supported
		self.contributors = []     # mapped to agency id
		self.updatedAfter = None

		self.eventIDs     = []     # specific event filter may not be
		                           # combined with above filter criteria


		# non standard parameters
		self.comments     = None
		self.formatted    = None
		self.picks        = None
		self.fm           = None
		self.allFMs       = None
		self.staMTs       = None


	#---------------------------------------------------------------------------
	def parse(self):
		self.parseTime()
		self.parseGeo()
		self.parseOutput()

		# depth
		d = self.Depth()
		d.min = self.parseFloat(self.PMinDepth)
		d.max = self.parseFloat(self.PMaxDepth)
		if d.min is not None and d.max is not None and d.min > d.max:
			raise ValueError, "%s exceeds %s" % (
			                  self.PMinDepth[0], self.PMaxDepth[0])
		if d.min is not None or d.max:
			self.depth = d

		# magnitude
		m = self.Magnitude()
		m.min = self.parseFloat(self.PMinMag)
		m.max = self.parseFloat(self.PMaxMag)
		if m.min is not None and m.max is not None and m.min > m.max:
			raise ValueError, "%s exceeds %s" % (
			                  self.PMinMag[0], self.PMaxMag[0])
		key, m.type = self.getFirstValue(self.PMagType)
		if m.min is not None or m.max is not None or m.type is not None:
			self.mag = m

		# output components
		self.allOrigins = self.parseBool(self.PAllOrigins)
		self.allMags    = self.parseBool(self.PAllMags)
		self.arrivals   = self.parseBool(self.PArrivals)
		self.picks      = self.parseBool(self.PPicks)
		self.fm         = self.parseBool(self.PFM)
		self.allFMs     = self.parseBool(self.PAllFMs)
		self.staMTs     = self.parseBool(self.PStaMTs)
		self.comments   = self.parseBool(self.PComments)

		# limit, offset, orderBy, updatedAfter
		self.limit = self.parseInt(self.PLimit, 1, DBMaxUInt)
		self.offset = self.parseInt(self.POffset, 1, DBMaxUInt)
		if self.offset is not None:
			# the offset is decremented by one since spec uses a weird offset
			# where an offset of '1' should return the first element instead of
			# the second one
			self.offset -= 1
		key, value = self.getFirstValue(self.POrderBy)
		if value is not None:
			if value in self.VOrderBy:
				self.orderBy = value
			else:
				self.raiseValueError(key)

		# catalogs and contributors
		self.catalogs = self.getValues(self.PCatalog)
		self.contributors = self.getValues(self.PContributor)
		self.updatedAfter = self.parseTimeStr(self.PUpdateAfter)

		# eventID(s)
		filterParams = self.time or self.geo or self.depth or self.mag or \
		               self.limit is not None or self.offset is not None or \
		               self.orderBy or self.catalogs or self.contributors or \
		               self.updatedAfter
		self.eventIDs = self.getValues(self.PEventID)
		# eventID, MUST NOT be combined with above parameters
		if filterParams and self.eventIDs:
			raise ValueError, "invalid mixture of parameters, the parameter " \
			      "'%s' may only be combined with: %s, %s, %s, %s, %s" % (
			      self.PEventID[0], self.PAllOrigins[0], self.PAllMags[0],
			      self.PArrivals[0], self.PPicks[0], self.PFM[0],
			      self.PAllFMs[0], self.PAllMTs[0], self.PComments[0])

		# format XML
		self.formatted = self.parseBool(self.PFormatted)


################################################################################
class FDSNEvent(resource.Resource):
	isLeaf = True
	version = VERSION

	#---------------------------------------------------------------------------
	def __init__(self, hideAuthor = False, evaluationMode = None,
	             eventTypeWhitelist = None, eventTypeBlacklist = None,
	             formatList = None):
		self._hideAuthor = hideAuthor
		self._evaluationMode = evaluationMode
		self._eventTypeWhitelist = eventTypeWhitelist
		self._eventTypeBlacklist = eventTypeBlacklist
		self._formatList = formatList


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
		# Parse and validate GET parameters
		ro = _EventRequestOptions(req.args)
		try:
			ro.parse()
		except ValueError, e:
			Logging.warning(str(e))
			return HTTP.renderErrorPage(req, http.BAD_REQUEST, str(e), ro)

		# Catalog filter is not supported
		if ro.catalogs:
			msg = "catalog filter not supported"
			return HTTP.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		# updateafter not implemented
		if ro.updatedAfter:
			msg = "filtering based on update time not supported"
			return HTTP.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		if self._formatList is not None and ro.format not in self._formatList:
			msg = "output format '%s' not available" % ro.format
			return HTTP.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		# Exporter, 'None' is used for text output
		if ro.format in ro.VText:
			exp = None
		else:
			exp = Exporter.Create(ro.Exporters[ro.format])
			if exp:
				exp.setFormattedOutput(bool(ro.formatted))
			else:
				msg = "output format '%s' not available, export module '%s' could " \
				      "not be loaded." % (ro.format, ro.Exporters[ro.format])
				return HTTP.renderErrorPage(req, http.BAD_REQUEST, msg, ro)

		# Create database query
		db = DatabaseInterface.Open(Application.Instance().databaseURI())
		if db is None:
			msg = "could not connect to database: %s" % dbq.errorMsg()
			return HTTP.renderErrorPage(req, http.SERVICE_UNAVAILABLE, msg, ro)

		dbq = DataModel.DatabaseQuery(db)

		# Process request in separate thread
		d = deferToThread(self._processRequest, req, ro, dbq, exp)
		req.notifyFinish().addErrback(utils.onCancel, d)
		d.addBoth(utils.onFinish, req)

		# The request is handled by the deferred object
		return server.NOT_DONE_YET


	#---------------------------------------------------------------------------
	def _removeAuthor(self, obj):
		try:
			ci = obj.creationInfo()
			ci.setAuthor("")
			ci.setAuthorURI("")
		except ValueError: pass


	#---------------------------------------------------------------------------
	def _loadComments(self, dbq, obj):
		cnt = dbq.loadComments(obj)
		if self._hideAuthor:
			for iComment in xrange(cnt):
				self._removeAuthor(obj.comment(iComment))
		return cnt


	#---------------------------------------------------------------------------
	def _processRequestExp(self, req, ro, dbq, exp, ep):
		objCount = ep.eventCount()
		maxObj = Application.Instance()._queryObjects

		if not HTTP.checkObjects(req, objCount, maxObj):
			return False

		pickIDs = set()
		if ro.picks is None:
			ro.picks = True

		# add related information
		for iEvent in xrange(ep.eventCount()):
			if req._disconnected:
				return False
			e = ep.event(iEvent)
			if self._hideAuthor:
				self._removeAuthor(e)

			originIDs = set()
			magIDs = set()
			magIDs.add(e.preferredMagnitudeID())

			# eventDescriptions and comments
			objCount += dbq.loadEventDescriptions(e)
			if ro.comments:
				objCount += self._loadComments(dbq, e)
			if not HTTP.checkObjects(req, objCount, maxObj):
				return False

			# origin references: either all or preferred only
			dbIter = dbq.getObjects(e, DataModel.OriginReference.TypeInfo())
			for obj in dbIter:
				oRef = DataModel.OriginReference.Cast(obj)
				if oRef is None:
					continue
				if ro.allOrigins:
					e.add(oRef)
					originIDs.add(oRef.originID())
				elif oRef.originID() == e.preferredOriginID():
					e.add(oRef)
					originIDs.add(oRef.originID())
					dbIter.close()

			objCount += e.originReferenceCount()

			# focalMechanism references: either none, preferred only or all
			if ro.fm or ro.allFMs:
				dbIter = dbq.getObjects(e, DataModel.FocalMechanismReference.TypeInfo())
				for obj in dbIter:
					fmRef = DataModel.FocalMechanismReference.Cast(obj)
					if fmRef is None:
						continue
					if ro.allFMs:
						e.add(fmRef)
					elif fmRef.focalMechanismID() == e.preferredFocalMechanismID():
						e.add(fmRef)
						dbIter.close()

			objCount += e.focalMechanismReferenceCount()

			if not HTTP.checkObjects(req, objCount, maxObj):
				return False

			# focal mechanisms: process before origins to add derived origin to
			# originID list since it may be missing from origin reference list
			for iFMRef in xrange(e.focalMechanismReferenceCount()):
				if req._disconnected:
					return False
				fmID = e.focalMechanismReference(iFMRef).focalMechanismID()
				obj = dbq.getObject(DataModel.FocalMechanism.TypeInfo(), fmID)
				fm = DataModel.FocalMechanism.Cast(obj)
				if fm is None:
					continue

				ep.add(fm)
				objCount += 1
				if self._hideAuthor:
					self._removeAuthor(fm)

				# comments
				if ro.comments:
					objCount += self._loadComments(dbq, fm)

				# momentTensors
				objCount += dbq.loadMomentTensors(fm)

				if not HTTP.checkObjects(req, objCount, maxObj):
					return False

				for iMT in xrange(fm.momentTensorCount()):
					mt = fm.momentTensor(iMT)

					originIDs.add(mt.derivedOriginID())
					magIDs.add(mt.momentMagnitudeID())

					if self._hideAuthor:
						self._removeAuthor(mt)

					if ro.comments:
						for iMT in xrange(fm.momentTensorCount()):
							objCount += self._loadComments(dbq, mt)

					objCount += dbq.loadDataUseds(mt);
					objCount += dbq.loadMomentTensorPhaseSettings(mt);
					if ro.staMTs:
						objCount += dbq.loadMomentTensorStationContributions(mt);
						for iStaMT in xrange(mt.momentTensorStationContributionCount()):
							objCount += dbq.load(mt.momentTensorStationContribution(iStaMT))

					if not HTTP.checkObjects(req, objCount, maxObj):
						return False

			# find ID of origin containing preferred Magnitude
			if e.preferredMagnitudeID():
				obj = dbq.getObject(DataModel.Magnitude.TypeInfo(),
				                    e.preferredMagnitudeID())
				m = DataModel.Magnitude.Cast(obj)
				if m is not None:
					oID = dbq.parentPublicID(m)
					if oID:
						originIDs.add(oID)

			# origins
			for oID in originIDs:
				if req._disconnected:
					return False
				obj = dbq.getObject(DataModel.Origin.TypeInfo(), oID)
				o = DataModel.Origin.Cast(obj)
				if o is None:
					continue

				ep.add(o)
				objCount += 1
				if self._hideAuthor:
					self._removeAuthor(o)

				# comments
				if ro.comments:
					objCount += self._loadComments(dbq, o)
				if not HTTP.checkObjects(req, objCount, maxObj):
					return False

				# magnitudes
				dbIter = dbq.getObjects(oID, DataModel.Magnitude.TypeInfo())
				for obj in dbIter:
					mag = DataModel.Magnitude.Cast(obj)
					if mag is None:
						continue
					if ro.allMags:
						o.add(mag)
					elif mag.publicID() in magIDs:
						o.add(mag)
						dbIter.close()

					if self._hideAuthor:
						self._removeAuthor(mag)

				objCount += o.magnitudeCount()
				if ro.comments:
					for iMag in xrange(o.magnitudeCount()):
						objCount += self._loadComments(dbq, o.magnitude(iMag))
				if not HTTP.checkObjects(req, objCount, maxObj):
					return False

				# TODO station magnitudes, amplitudes
				# - added pick id for each pick referenced by amplitude

				# arrivals
				if ro.arrivals:
					objCount += dbq.loadArrivals(o)
					if self._removeAuthor:
						for iArrival in xrange(o.arrivalCount()):
							self._removeAuthor(o.arrival(iArrival))

					# collect pick IDs if requested
					if ro.picks:
						for iArrival in xrange(o.arrivalCount()):
							pickIDs.add(o.arrival(iArrival).pickID())

				if not HTTP.checkObjects(req, objCount, maxObj):
					return False

		# picks
		if pickIDs:
			objCount += len(pickIDs)
			if not HTTP.checkObjects(req, objCount, maxObj):
				return False

			for pickID in pickIDs:
				obj = dbq.getObject(DataModel.Pick.TypeInfo(), pickID)
				pick = DataModel.Pick.Cast(obj)
				if pick is not None:
					if self._hideAuthor:
						self._removeAuthor(pick)
					if ro.comments:
						objCount += self._loadComments(dbq, pick)
					ep.add(pick)
				if not HTTP.checkObjects(req, objCount, maxObj):
					return False

		# write response
		sink = utils.Sink(req)
		if not exp.write(sink, ep):
			return False
		Logging.debug("%s: returned %i events and %i origins (total " \
		               "objects/bytes: %i/%i)" % (ro.service, ep.eventCount(),
		               ep.originCount(), objCount, sink.written))
		utils.accessLog(req, ro, http.OK, sink.written, None)
		return True


	#---------------------------------------------------------------------------
	def _processRequestText(self, req, ro, dbq, ep):
		lineCount = 0

		line = "#EventID|Time|Latitude|Longitude|Depth/km|Author|Catalog|" \
		       "Contributor|ContributorID|MagType|Magnitude|MagAuthor|" \
		       "EventLocationName\n"
		df = "%FT%T.%f"
		utils.writeTS(req, line)
		byteCount = len(line)

		# add related information
		for iEvent in xrange(ep.eventCount()):
			e = ep.event(iEvent)
			eID = e.publicID()

			# query for preferred origin
			obj = dbq.getObject(DataModel.Origin.TypeInfo(),
			                    e.preferredOriginID())
			o = DataModel.Origin.Cast(obj)
			if o is None:
				Logging.warning("preferred origin of event '%s' not found: %s" % (
				                eID, e.preferredOriginID()))
				continue

			# depth
			try: depth = str(o.depth().value())
			except ValueError: depth = ''

			# author
			if self._hideAuthor:
				author = ''
			else:
				try: author = o.creationInfo().author()
				except ValueError: author = ''

			# contributor
			try: contrib = e.creationInfo().agencyID()
			except ValueError: contrib = ''

			# query for preferred magnitude (if any)
			mType, mVal, mAuthor = '', '', ''
			if e.preferredMagnitudeID():
				obj = dbq.getObject(DataModel.Magnitude.TypeInfo(),
				                    e.preferredMagnitudeID())
				m = DataModel.Magnitude.Cast(obj)
				if m is not None:
					mType = m.type()
					mVal = str(m.magnitude().value())
					if self._hideAuthor:
						mAuthor = ''
					else:
						try: mAuthor = m.creationInfo().author()
						except ValueError: pass

			# event description
			dbq.loadEventDescriptions(e)
			region = ''
			for i in xrange(e.eventDescriptionCount()):
				ed = e.eventDescription(i)
				if ed.type() == DataModel.REGION_NAME:
					region = ed.text()
					break

			if req._disconnected:
				return False
			line = "%s|%s|%f|%f|%s|%s||%s|%s|%s|%s|%s|%s\n" % (
			       eID, o.time().value().toString(df), o.latitude().value(),
			       o.longitude().value(), depth, author, contrib, eID,
			       mType, mVal, mAuthor, region)
			utils.writeTS(req, line)
			lineCount +=1
			byteCount += len(line)

		# write response
		Logging.debug("%s: returned %i events (total bytes: %i) " % (
		               ro.service, lineCount, byteCount))
		utils.accessLog(req, ro, http.OK, byteCount, None)
		return True


	#---------------------------------------------------------------------------
	def _processRequest(self, req, ro, dbq, exp):
		if req._disconnected:
			return False

		DataModel.PublicObject.SetRegistrationEnabled(False)

		# query event(s)
		ep = DataModel.EventParameters()
		if ro.eventIDs:
			for eID in ro.eventIDs:
				obj = dbq.getEventByPublicID(eID)
				e = DataModel.Event.Cast(obj)
				if not e:
					continue

				if self._eventTypeWhitelist or self._eventTypeBlacklist:
					eType = None
					try: eType = DataModel.EEventTypeNames_name(e.type())
					except ValueError: pass
					if self._eventTypeWhitelist and \
					   not eType in self._eventTypeWhitelist: continue
					if self._eventTypeBlacklist and \
					   eType in self._eventTypeBlacklist: continue

				if self._evaluationMode is not None:
					obj = dbq.getObject(DataModel.Origin.TypeInfo(),
					                    e.preferredOriginID())
					o = DataModel.Origin.Cast(obj)
					try:
						if o is None or \
						   o.evaluationMode() != self._evaluationMode:
							continue
					except ValueError:
						continue

				ep.add(e)
		else:
			self._findEvents(ep, ro, dbq)

		if ep.eventCount() == 0:
			msg = "no matching events found"
			data = HTTP.renderErrorPage(req, http.NO_CONTENT, msg, ro)
			if data:
				utils.writeTS(req, data)
			return True

		Logging.debug("events found: %i" % ep.eventCount())

		if ro.format == 'csv' or not exp:
			req.setHeader('Content-Type', 'text/plain')
		else:
			req.setHeader('Content-Type', 'application/xml')

		if exp:
			return self._processRequestExp(req, ro, dbq, exp, ep)

		return self._processRequestText(req, ro, dbq, ep)


	#---------------------------------------------------------------------------
	def _findEvents(self, ep, ro, dbq):
		db = Application.Instance().database()
		def _T(name):
			return db.convertColumnName(name)
		def _time(time):
			return db.timeToString(time)

		orderByMag = ro.orderBy and ro.orderBy.startswith('magnitude')
		reqMag = ro.mag or orderByMag
		reqDist = ro.geo and ro.geo.bCircle
		colPID = _T('publicID')
		colTime = _T('time_value')
		colMag = _T('magnitude_value')
		if orderByMag:
			colOrderBy = "m.%s" % colMag
		else:
			colOrderBy = "o.%s" % colTime

		bBox = None
		if ro.geo:
			colLat, colLon = _T('latitude_value'), _T('longitude_value')
			if ro.geo.bBox:
				bBox = ro.geo.bBox
			else:
				bBox = ro.geo.bCircle.calculateBBox()

		# SELECT --------------------------------
		q = "SELECT DISTINCT pe.%s, e.*, %s" % (colPID, colOrderBy)
		if reqDist: # Great circle distance calculated by Haversine formula
			c = ro.geo.bCircle
			q += ", DEGREES(ACOS(" \
			     "COS(RADIANS(o.%s)) * COS(RADIANS(%s)) * " \
			     "COS(RADIANS(o.%s) - RADIANS(%s)) + SIN(RADIANS(o.%s)) * " \
			     "SIN(RADIANS(%s)))) AS distance" % (
			     colLat, c.lat, colLon, c.lon, colLat, c.lat)

		# FROM ----------------------------------
		q += " FROM Event AS e, PublicObject AS pe" \
		     ", Origin AS o, PublicObject AS po"
		if reqMag:
			q += ", Magnitude AS m, PublicObject AS pm"

		# WHERE ---------------------------------
		q += " WHERE e._oid = pe._oid"

		# event information filter
		if self._eventTypeWhitelist:
			q += " AND e.%s IN ('%s')" % (
			     _T('type'), "', '".join(self._eventTypeWhitelist))
		if self._eventTypeBlacklist:
			q += " AND (e.%s IS NULL OR e.%s NOT IN ('%s'))" % (
			     _T('type'), _T('type'),
			     "', '".join(self._eventTypeBlacklist))
		if ro.contributors:
			q += " AND e.%s AND upper(e.%s) IN('%s')" % (
			     _T('creationinfo_used'), _T('creationinfo_agencyid'),
			     "', '".join(ro.contributors).upper())

		# origin information filter
		q += " AND o._oid = po._oid AND po.%s = e.%s" % (
		       colPID, _T('preferredOriginID'))

		# evaluation mode config parameter
		if self._evaluationMode is not None:
			colEvalMode = _T('evaluationMode')
			q += " AND o.%s = '%s'" % ( colEvalMode,
			     DataModel.EEvaluationModeNames.name(self._evaluationMode))

		# time
		if ro.time:
			colTimeMS = _T('time_value_ms')
			if ro.time.start is not None:
				t = _time(ro.time.start)
				ms = ro.time.start.microseconds()
				q += " AND (o.%s > '%s' OR (o.%s = '%s' AND o.%s >= %i))" % (
				     colTime, t, colTime, t, colTimeMS, ms)
			if ro.time.end is not None:
				t = _time(ro.time.end)
				ms = ro.time.end.microseconds()
				q += " AND (o.%s < '%s' OR (o.%s = '%s' AND o.%s <= %i))" % (
				     colTime, t, colTime, t, colTimeMS, ms)

		# bounding box
		if bBox:
			if bBox.minLat is not None:
				q += " AND o.%s >= %s" % (colLat, bBox.minLat)
			if bBox.maxLat is not None:
				q += " AND o.%s <= %s" % (colLat, bBox.maxLat)
			if bBox.dateLineCrossing():
				q += " AND (o.%s >= %s OR o.%s <= %s)" % (
				     colLon, bBox.minLon, colLon, bBox.maxLon)
			else:
				if bBox.minLon is not None:
					q += " AND o.%s >= %s" % (colLon, bBox.minLon)
				if bBox.maxLon is not None:
					q += " AND o.%s <= %s" % (colLon, bBox.maxLon)

		# depth
		if ro.depth:
			q += " AND o.%s" % _T("depth_used")
			colDepth = _T('depth_value')
			if ro.depth.min is not None:
				q += " AND o.%s >= %s" % (colDepth, ro.depth.min)
			if ro.depth.max is not None:
				q += " AND o.%s <= %s" % (colDepth, ro.depth.max)

		# updated after
		if ro.updatedAfter:
			t = _time(ro.updatedAfter)
			ms = ro.updatedAfter.microseconds()
			colCTime   = _T('creationinfo_creationtime')
			colCTimeMS = _T('creationinfo_creationtime_ms')
			colMTime   = _T('creationinfo_modificationtime')
			colMTimeMS = _T('creationinfo_modificationtime_ms')
			tFilter = "(o.%s > '%s' OR (o.%s = '%s' AND o.%s > %i))"

			q += " AND ("
			q += tFilter % (colCTime, t, colCTime, t, colCTimeMS, ms) + " OR "
			q += tFilter % (colMTime, t, colMTime, t, colMTimeMS, ms) + ")"

		# magnitude information filter
		if reqMag:
			q += " AND m._oid = pm._oid AND "
			if ro.mag and ro.mag.type:
				# join magnitude table on oID of origin and magnitude type
				q += "m._parent_oid = o._oid AND m.%s = '%s'" % (_T('type'),
				                                                 dbq.toString(ro.mag.type))
			else:
				# join magnitude table on preferred magnitude id of event
				q += "pm.%s = e.%s" % (colPID, _T('preferredMagnitudeID'))

			if ro.mag and ro.mag.min is not None:
				q += " AND m.%s >= %s" % (colMag, ro.mag.min)
			if ro.mag and ro.mag.max is not None:
				q += " AND m.%s <= %s" % (colMag, ro.mag.max)

		# ORDER BY ------------------------------
		q += " ORDER BY %s" % colOrderBy
		if ro.orderBy and ro.orderBy.endswith('-asc'):
			q += " ASC"
		else:
			q += " DESC"

		# SUBQUERY distance (optional) ----------
		if reqDist:
			q = "SELECT * FROM (%s) AS subquery WHERE distance " % q
			c = ro.geo.bCircle
			if c.minRad is not None:
				q += ">= %s" % c.minRad
			if c.maxRad is not None:
				if c.minRad is not None:
					q += " AND distance "
				q += "<= %s" % c.maxRad

		# LIMIT/OFFSET --------------------------
		if ro.limit is not None or ro.offset is not None:
			# Postgres allows to omit the LIMIT parameter for offsets, MySQL
			# does not. According to the MySQL manual a very large number should
			# be used for this case.
			l = DBMaxUInt
			if ro.limit is not None:
				l = ro.limit
			q += " LIMIT %i" % l
			if ro.offset is not None:
				q += " OFFSET %i" % ro.offset

		Logging.debug("event query: %s" % q)

		for e in dbq.getObjectIterator(q, DataModel.Event.TypeInfo()):
			ep.add(DataModel.Event.Cast(e))


# vim: ts=4 noet
