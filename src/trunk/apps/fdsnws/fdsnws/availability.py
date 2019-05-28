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

from seiscomp3 import DataModel, IO, Logging
from seiscomp3.Client import Application
from seiscomp3.Core import Time

from http import BaseResource
from request import RequestOptions

import utils


DBMaxUInt = 18446744073709551615 # 2^64 - 1
VERSION = "0.1.0"

################################################################################
class _AvailabilityRequestOptions(RequestOptions):

	VFormatText         = 'text'
	VFormatGeoCSV       = 'geocsv'
	VFormatJSON         = 'json'
	VFormatSync         = 'sync'

	OutputFormats       = [ VFormatText, VFormatGeoCSV, VFormatJSON,
	                        VFormatSync ]

	PQuality            = [ 'quality' ]
	PMergeQuality       = [ 'mergequality' ]
	PMergeSampleRate    = [ 'mergesamplerate' ]
	PIncludeRestricted  = [ 'includerestricted' ]

	POSTParams          = RequestOptions.POSTParams + PQuality + \
	                      PMergeQuality + PMergeSampleRate+ PIncludeRestricted


	#---------------------------------------------------------------------------
	def __init__(self, args=None):
		RequestOptions.__init__(self, args)

		self.service           = 'availability-base'

		self.quality           = None
		self.mergeQuality      = None
		self.mergeSampleRate   = None
		self.showLastUpdate    = None
		self.includeRestricted = None


	#---------------------------------------------------------------------------
	def parse(self):
		self.parseTime()
		self.parseChannel()
		self.parseOutput()

		# quality: D, M, Q, R, * (optional)
		foundAny = False
		for vList in self.getValues(self.PQuality):
			for v in vList.split(','):
				v = v.strip()
				if len(v) == 1:
					if v[0] == '*':
						foundAny = True
						break
					elif v[0].isupper():
						if self.quality is None:
							self.quality = [ v ]
						else:
							self.quality.append(v)
						continue
				self.raiseValueError(self.PQuality[0])
			if foundAny:
				self.quality = None
				break

		# mergeQuality (optional)
		self.mergeQuality = self.parseBool(self.PMergeQuality)

		# mergeSampleRate (optional)
		self.mergeSampleRate = self.parseBool(self.PMergeSampleRate)

		# includeRestricted (optional)
		self.includeRestricted = self.parseBool(self.PIncludeRestricted)
		if self.includeRestricted is None:
			self.includeRestricted = True

		# sync format implies printing of last update and forces merging of
		# quality
		if self.format == self.VFormatSync:
			self.showLastUpdate = True
			self.mergeQuality   = True



	#---------------------------------------------------------------------------
	def extentIter(self, dac):
		# tupel: extent, oid, restricted
		for e in dac.extentsSorted():
			ext = e[0]
			restricted = e[2]
			for ro in self.streams:
				if ro.channel:
					wid = ext.waveformID()
					if not ro.channel.matchNet(wid.networkCode()) or \
					   not ro.channel.matchSta(wid.stationCode()) or \
					   not ro.channel.matchLoc(wid.locationCode()) or \
					   not ro.channel.matchCha(wid.channelCode()):
						continue

				if ro.time and not ro.time.match(ext.start(), ext.end()):
					continue

				if not ro.includeRestricted and restricted:
					continue

				yield e



################################################################################
class _Availability(BaseResource):
	isLeaf = True

	#---------------------------------------------------------------------------
	def __init__(self):
		BaseResource.__init__(self, VERSION)


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
		ro = self._createRequestOptions(req.args)
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
		ro = self._createRequestOptions()
		try:
			ro.parsePOST(req.content)
			ro.parse()
		except ValueError, e:
			Logging.warning(str(e))
			return self.renderErrorPage(req, http.BAD_REQUEST, str(e), ro)

		return self._prepareRequest(req, ro)


	#---------------------------------------------------------------------------
	def _prepareRequest(self, req, ro):

		dac = Application.Instance().getDACache()

		if ro.format == ro.VFormatJSON:
			contentType = 'application/json'
			extension = 'json'
		elif ro.format == ro.VFormatGeoCSV:
			contentType = 'application/csv'
			extension = 'csv'
		else:
			contentType = 'text/plain'
			extension = 'txt'

		req.setHeader('Content-Type', contentType)
		req.setHeader('Content-Disposition',
		              'inline; filename=fdsnws-ext-availability_{0}.{1}' \
		              .format(Time.GMT().iso(), extension))

		d = deferToThread(self._processRequest, req, ro, dac)

		req.notifyFinish().addErrback(utils.onCancel, d)
		d.addBoth(utils.onFinish, req)

		# The request is handled by the deferred object
		return server.NOT_DONE_YET


	#---------------------------------------------------------------------------
	def _formatTime(self, time, ms=False):
		if ms:
			return '{0}.{1:06d}Z'.format(time.toString('%FT%T'),
			                             time.microseconds())
		return time.toString('%FT%TZ')


	#---------------------------------------------------------------------------
	def _writeLines(self, req, lines, ro):
		if ro.format == ro.VFormatText:
			return self._writeFormatText(req, lines, ro)
		elif ro.format == ro.VFormatGeoCSV:
			return self._writeFormatGeoCSV(req, lines, ro)
		elif ro.format == ro.VFormatJSON:
			return self._writeFormatJSON(req, lines, ro)
		elif ro.format == ro.VFormatSync:
			return self._writeFormatSync(req, lines, ro)

		raise Exception, "unknown reponse format: %s" % ro.format


	#---------------------------------------------------------------------------
	def _writeFormatText(self, req, lines, ro):
		byteCount   = 0
		lineCount   = 0

		# the extent service uses a different update column name and alignment
		isExtentReq = ro.__class__ == _AvailabilityExtentRequestOptions

		nslc        = '{0: <2} {1: <5} {2: <2} {3: <3}'
		quality     = ' {0: <1}'
		sampleRate  = ' {0: >11}'
		sampleRateF = ' {0: >11.1f}'
		time        = ' {0: >27} {1: >27}'
		updated     = ' {0: >20}' if isExtentReq else ' {0: <20}'
		timeSpans   = ' {0: >10}'
		restriction = ' {0: <11}'

		header = nslc.format('#n', 's', 'l', 'c')
		if not ro.mergeQuality:
			header += quality.format('q')
		if not ro.mergeSampleRate:
			header += sampleRate.format('sample-rate')
		header += time.format('earliest', 'latest')
		if ro.showLastUpdate:
			header += updated.format('updated' if isExtentReq else \
			                       'most-recent-update')
		if isExtentReq:
			if ro.showTimeSpanCount:
				header += timeSpans.format('time-spans')
			if ro.showRestriction:
				header += restriction.format('restriction')
		header += '\n'

		first = True
		for line in lines:
			if first:
				first = False
				utils.writeTS(req, header)
				byteCount += len(header)

			wid = line[0].waveformID()
			e   = line[1]
			loc = wid.locationCode() if wid.locationCode() else "--"
			data = nslc.format(wid.networkCode(), wid.stationCode(),
			                   loc, wid.channelCode())
			if not ro.mergeQuality:
				data += quality.format(e.quality())
			if not ro.mergeSampleRate:
				data += sampleRateF.format(e.sampleRate())
			data += time.format(self._formatTime(e.start(), True),
			                    self._formatTime(e.end(), True))
			if ro.showLastUpdate:
				data += updated.format(self._formatTime(e.updated()))
			if isExtentReq:
				if ro.showTimeSpanCount:
					data += timeSpans.format(e.segmentCount())
				if ro.showRestriction:
					data += restriction.format('RESTRICTED' if line[2] else \
					                           'OPEN')
			data += '\n'

			utils.writeTS(req, data)
			byteCount += len(data)
			lineCount += 1

		return byteCount, lineCount


	#---------------------------------------------------------------------------
	def _writeFormatGeoCSV(self, req, lines, ro):
		byteCount   = 0
		lineCount   = 0

		# the extent service uses a different update column name and alignment
		isExtentReq = ro.__class__ == _AvailabilityExtentRequestOptions

		nslc        = '{0}|{1}|{2}|{3}'
		time        = '|{0}|{1}'

		# header
		fieldUnit = '#field_unit: unitless | unitless | unitless | unitless'
		fieldType = '#field_type: string | string | string | string'
		fieldName = 'network|station|location|channel'

		if not isExtentReq:
			fieldUnit += ' | unitless'
			fieldType += ' | string'
			fieldName += '|repository'
			repoName = '|' + Application.Instance()._daRepositoryName

		if not ro.mergeQuality:
			fieldUnit += ' | unitless'
			fieldType += ' | string'
			fieldName += '|quality'

		if not ro.mergeSampleRate:
			fieldUnit += ' | unitless'
			fieldType += ' | float'
			fieldName += '|sample_rate' if isExtentReq else \
			             '|samplerate'

		fieldUnit += ' | ISO_8601 | ISO_8601'
		fieldType += ' | datetime | datetime'
		fieldName += '|earliest|latest' if isExtentReq else \
		             '|starttime|endtime'

		if ro.showLastUpdate:
			fieldUnit += ' | ISO_8601'
			fieldType += ' | datetime'
			fieldName += '|updated' if isExtentReq else \
			             '|lastupdate'

		if isExtentReq:
			if ro.showTimeSpanCount:
				fieldUnit += ' | unitless'
				fieldType += ' | integer'
				fieldName += '|timespans'

			if ro.showRestriction:
				fieldUnit += ' | unitless'
				fieldType += ' | string'
				fieldName += '|restriction'

		header = '#dataset: GeoCSV 2.0\n' \
		         '#delimiter: |\n'
		header += '{0}\n{1}\n{2}\n'.format(fieldUnit, fieldType, fieldName)

		first = True
		for line in lines:
			if first:
				first = False
				utils.writeTS(req, header)
				byteCount += len(header)

			wid = line[0].waveformID()
			e   = line[1]
			data = nslc.format(wid.networkCode(), wid.stationCode(),
			                   wid.locationCode(), wid.channelCode())
			if not isExtentReq:
				data += repoName
			if not ro.mergeQuality:
				data += '|' + e.quality()
			if not ro.mergeSampleRate:
				data += '|{0:.1f}'.format(e.sampleRate())
			data += time.format(self._formatTime(e.start(), True),
			                    self._formatTime(e.end(), True))
			if ro.showLastUpdate:
				data += '|' + self._formatTime(e.updated())
			if isExtentReq:
				if ro.showTimeSpanCount:
					data += '|{0:d}'.format(e.segmentCount())
				if ro.showRestriction:
					data += '|RESTRICTED' if line[2] else '|OPEN'
			data += '\n'

			utils.writeTS(req, data)
			byteCount += len(data)
			lineCount += 1

		return byteCount, lineCount


	#---------------------------------------------------------------------------
	def _writeFormatJSON(self, req, lines, ro):
		byteCount = 0
		lineCount = 0

		now       = self._formatTime(Time.GMT())
		repoName  = Application.Instance()._daRepositoryName
		header    = '{{' \
		              '"created":"{0}",' \
		              '"repository":[{{' \
		                '"repository_name":"{1}",' \
		                '"channels":['.format(now, repoName)
		footer    = ']}]}'

		return self._writeJSONChannels(req, header, footer, lines, ro)


	#---------------------------------------------------------------------------
	def _writeFormatSync(self, req, lines, ro):
		byteCount = 0
		lineCount = 0
		dccName   = Application.Instance()._daDCCName
		repoName  = Application.Instance()._daRepositoryName

		updated = None


		header = '{0}|{1}\n'.format(dccName, Time.GMT().toString('%Y,%j'))

		first = True
		for line in lines:
			if first:
				first = False
				utils.writeTS(req, header)
				byteCount += len(header)

			wid = line[0].waveformID()
			e   = line[1]

			start = e.start()
			end   = e.end()

			# truncate start and end time to requested time frame
			if ro.time:
				if ro.time.start and ro.time.start > start:
					start = ro.time.start
				if ro.time.end and ro.time.end < end:
					end = ro.time.end

			sr  = "" if ro.mergeSampleRate \
			         else '{0:.1f}'.format(e.sampleRate())
			data = '{0}|{1}|{2}|{3}|{4}|{5}||{6}||||||{7}||\n'.format(
			       wid.networkCode(), wid.stationCode(), wid.locationCode(),
			       wid.channelCode(), start.toString('%Y,%j,%T'),
			       end.toString('%Y,%j,%T'), sr, repoName)
			utils.writeTS(req, data)
			byteCount += len(data)
			lineCount += 1

		return byteCount, lineCount


################################################################################
class _AvailabilityExtentRequestOptions(_AvailabilityRequestOptions):

	VFormatRequest      = 'request'

	OutputFormats       = _AvailabilityRequestOptions.OutputFormats + \
	                      [ VFormatRequest ]

	VOrderByNSLC        = 'nslc_time_quality_samplerate'
	VOrderByCount       = 'timespancount'
	VOrderByCountDesc   = 'timespancount_desc'
	VOrderByUpdate      = 'latestupdate'
	VOrderByUpdateDesc  = 'latestupdate_desc'
	VOrderBy            = [ VOrderByNSLC, VOrderByCount, VOrderByCountDesc,
	                        VOrderByUpdate, VOrderByUpdateDesc ]

	VShowLatestUpdate   = 'latestupdate'
	VShowTimeSpanCount  = 'timespancount'
	VShowRestriction    = 'restriction'

	POrderBy            = [ 'orderby' ]
	PRowLimit           = [ 'rowlimit' ]
	PShow               = [ 'show' ]

	POSTParams          = _AvailabilityRequestOptions.POSTParams + \
	                      POrderBy + PRowLimit + PShow


	#---------------------------------------------------------------------------
	def __init__(self, args=None):
		_AvailabilityRequestOptions.__init__(self, args)
		self.service    = 'availability-extent'

		self.orderBy           = None
		self.rowLimit          = None
		self.showTimeSpanCount = False
		self.showRestriction   = False


	#---------------------------------------------------------------------------
	def parse(self):
		_AvailabilityRequestOptions.parse(self)

		# orderby
		key, value = self.getFirstValue(self.POrderBy)
		if value is None:
			self.orderBy = self.VOrderBy[0]
		else:
			if value in self.VOrderBy:
				self.orderBy = value
			else:
				self.raiseValueError(key)

		# rowlimit
		self.rowLimit = self.parseInt(self.PRowLimit, 1, DBMaxUInt)

		# show
		for vList in self.getValues(self.PShow):
			for v in vList.split(','):
				v = v.strip()
				if v == self.VShowLatestUpdate:
					self.showLastUpdate = True
				elif v == self.VShowTimeSpanCount:
					self.showTimeSpanCount = True
				elif v == self.VShowRestriction:
					self.showRestriction = True
				else:
					self.raiseValueError(self.PShow[0])

		# request format implies no extra columns and forces merging of
		# quality and sample rate
		if self.format == self.VFormatRequest:
			self.showLastUpdate    = False
			self.showTimeSpanCount = False
			self.showRestriction   = False
			self.mergeQuality      = True
			self.mergeSampleRate   = True


	#---------------------------------------------------------------------------
	def attributeExtentIter(self, ext):

		for i in xrange(ext.dataAttributeExtentCount()):
			e = ext.dataAttributeExtent(i)

			if self.time and not self.time.match(e.start(), e.end()):
				continue

			if self.quality and e.quality() not in self.quality:
				continue

			yield e


################################################################################
class AvailabilityExtent(_Availability):
	isLeaf = True

	#---------------------------------------------------------------------------
	def __init__(self):
		_Availability.__init__(self)


	#---------------------------------------------------------------------------
	def _createRequestOptions(self, args=None):
		return _AvailabilityExtentRequestOptions(args)


	#---------------------------------------------------------------------------
	def _mergeExtents(self, attributeExtents):

		merged = None
		cloned = False

		# Create a copy of the extent only if more than 1 element is found. The
		# number of elements is not known in advance since attributeExtents
		# might only be an iterator.
		for e in attributeExtents:
			if merged is None:
				merged = e
			else:
				if not cloned:
					merged = DataModel.DataAttributeExtent(merged)
					cloned = True

				if e.start() < merged.start():
					merged.setStart(e.start())
				if e.end() > merged.end():
					merged.setEnd(e.end())
				if e.updated() > merged.updated():
					merged.setUpdated(e.updated())
				merged.setSegmentCount(merged.segmentCount() + e.segmentCount())

		return merged


	#---------------------------------------------------------------------------
	def _writeLines(self, req, lines, ro):
		if ro.format == ro.VFormatRequest:
			return self._writeFormatRequest(req, lines, ro), len(lines)
		else:
			return _Availability._writeLines(self, req, lines, ro)


	#---------------------------------------------------------------------------
	def _writeFormatRequest(self, req, lines, ro):
		byteCount   = 0

		for line in lines:
			wid   = line[0].waveformID()
			e     = line[1]
			loc   = wid.locationCode() if wid.locationCode() else "--"
			start = e.start()
			end   = e.end()

			# truncate start and end time to requested time frame
			if ro.time:
				if ro.time.start and ro.time.start > start:
					start = ro.time.start
				if ro.time.end and ro.time.end < end:
					end = ro.time.end

			data = '{0} {1} {2} {3} {4} {5}\n'.format(
			       wid.networkCode(), wid.stationCode(), loc,
			       wid.channelCode(), self._formatTime(start, True),
			       self._formatTime(end, True))

			utils.writeTS(req, data)
			byteCount += len(data)

		return byteCount


	#---------------------------------------------------------------------------
	def _writeJSONChannels(self, req, header, footer, lines, ro):
		byteCount   = 0

		nslc        = '{{' \
		                '"net":"{0}",' \
		                '"sta":"{1}",' \
		                '"loc":"{2}",' \
		                '"cha":"{3}"'
		quality     =   ',"quality":"{0}"'
		sampleRate  =   ',"sample_rate":{0}'
		time        =   ',"earliest":"{0}","latest":"{1}"'
		updated     =   ',"updated":"{0}"'
		timeSpans   =   ',"timespans":{0}'
		restriction =   ',"restriction":"{0}"'

		utils.writeTS(req, header)
		byteCount += len(header)

		data = None
		for line in lines:
			wid = line[0].waveformID()
			e   = line[1]
			data = "," if data is not None else ""
			data += nslc.format(wid.networkCode(), wid.stationCode(),
			                    wid.locationCode(), wid.channelCode())
			if not ro.mergeQuality:
				data += quality.format(e.quality())
			if not ro.mergeSampleRate:
				data += sampleRate.format(e.sampleRate())
			data += time.format(self._formatTime(e.start(), True),
			                    self._formatTime(e.end(), True))
			if ro.showLastUpdate:
				data += updated.format(self._formatTime(e.updated()))
			if ro.showTimeSpanCount:
				data += timeSpans.format(e.segmentCount())
			if ro.showRestriction:
				data += restriction.format('RESTRICTED' if line[2] else 'OPEN')
			data += '}'

			utils.writeTS(req, data)
			byteCount += len(data)

		utils.writeTS(req, footer)
		byteCount += len(footer)

		return byteCount, len(lines)


	#---------------------------------------------------------------------------
	def _processRequest(self, req, ro, dac):
		if req._disconnected: return False

		data = ""
		restriction = None

		# tuples: wid, attribute extent, restricted status
		lines = [ ]

		mergeAll = ro.mergeQuality and ro.mergeSampleRate
		mergeNone = not ro.mergeQuality and not ro.mergeSampleRate
		mergeOne = not mergeAll and not mergeNone

		# iterate extents
		for ext, objID, restricted in ro.extentIter(dac):
			if req._disconnected: return False

			# iterate attribute extents and merge them if requested
			if mergeNone:
				for e in ro.attributeExtentIter(ext):
					lines.append((ext, e, restricted))
			elif mergeAll:
				e = self._mergeExtents(ro.attributeExtentIter(ext))
				if e is not None:
					lines.append((ext, e, restricted))
			elif ro.mergeQuality:
				eDict = {} # key=sampleRate
				for e in ro.attributeExtentIter(ext):
					if e.sampleRate() in eDict:
						eDict[e.sampleRate()].append(e)
					else:
						eDict[e.sampleRate()] = [ e ]
				for eList in eDict.itervalues():
					e = self._mergeExtents(eList)
					lines.append((ext, e, restricted))
			else:
				eDict = {} # key=quality
				for e in ro.attributeExtentIter(ext):
					if e.quality() in eDict:
						eDict[e.quality()].append(e)
					else:
						eDict[e.quality()] = [ e ]
				for eList in eDict.itervalues():
					e = self._mergeExtents(eList)
					lines.append((ext, e, restricted))


		# Return 204 if no matching availability information was found
		if len(lines) == 0:
			msg = "no matching availabilty information found"
			data = self.renderErrorPage(req, http.NO_CONTENT, msg, ro)
			if data:
				utils.writeTS(req, data)
			return True

		# sort lines
		self._sortLines(lines, ro)

		# truncate lines to requested row limit
		if ro.rowLimit:
			del lines[ro.rowLimit:]

		byteCount, extCount = self._writeLines(req, lines, ro)

		Logging.debug("%s: returned %i extents (total bytes: %i)" % (
		              ro.service, extCount, byteCount))
		utils.accessLog(req, ro, http.OK, byteCount, None)
		return True



	#---------------------------------------------------------------------------
	def _sortLines(self, lines, ro):

		def compareNSLC(l1, l2):
			if l1[0] is not l2[0]:
				# The lines are expected to be sorted according NSLC
				return 0

			e1 = l1[1]
			e2 = l2[1]

			if e1.start() < e2.start():
				return -1
			elif e1.start() > e2.start():
				return 1
			elif e1.end() < e2.end():
				return -1
			elif e1.end() > e2.end():
				return 1

			if not ro.mergeQuality:
				if e1.quality() < e2.quality():
					return -1
				if e1.quality() > e2.quality():
					return 1

			if not ro.mergeSampleRate:
				if e1.sampleRate() < e2.sampleRate():
					return -1
				if e1.sampleRate() > e2.sampleRate():
					return 1

			return 0

		def compareCount(l1, l2):
			c1 = l1[1].segmentCount()
			c2 = l2[1].segmentCount()

			return -1 if c1 < c2 else \
			        1 if c1 > c2 else \
			        compareNSLC(l1, l2)

		def compareCountDesc(l1, l2):
			c1 = l1[1].segmentCount()
			c2 = l2[1].segmentCount()

			return -1 if c1 > c2 else \
			        1 if c1 < c2 else \
			        compareNSLC(l1, l2)

		def compareUpdate(l1, l2):
			c1 = l1[1].updated()
			c2 = l2[1].updated()

			return -1 if c1 < c2 else \
			        1 if c1 > c2 else \
			        compareNSLC(l1, l2)

		def compareUpdateDesc(l1, l2):
			c1 = l1[1].updated()
			c2 = l2[1].updated()

			return -1 if c1 > c2 else \
			        1 if c1 < c2 else \
			        compareNSLC(l1, l2)

		comparator = compareNSLC if ro.orderBy == ro.VOrderByNSLC else \
		             compareCount if ro.orderBy == ro.VOrderByCount else \
		             compareCountDesc if ro.orderBy == ro.VOrderByCountDesc else \
		             compareUpdate if ro.orderBy == ro.VOrderByUpdate else \
		             compareUpdateDesc
		lines.sort(cmp=comparator)



################################################################################
class _AvailabilityQueryRequestOptions(_AvailabilityRequestOptions):

	PMergeOverlap       = [ 'mergeoverlap' ]
	PMergeTolerance     = [ 'mergetolerance' ]
	PShowLastUpdate     = [ 'showlastupdate' ]
	PExcludeTooLarge    = [ 'excludetoolarge' ]

	POSTParams          = _AvailabilityRequestOptions.POSTParams + \
	                      PMergeOverlap + PMergeTolerance + PShowLastUpdate + \
	                      PExcludeTooLarge


	#---------------------------------------------------------------------------
	def __init__(self, args=None):
		_AvailabilityRequestOptions.__init__(self, args)
		self.service    = 'availability-query'

		self.mergeOverlap    = None
		self.mergeTolerance  = None
		self.excludeTooLarge = None


	#---------------------------------------------------------------------------
	def parse(self):
		_AvailabilityRequestOptions.parse(self)

		self.mergeOverlap    = self.parseBool(self.PMergeOverlap)
		self.mergeTolerance  = self.parseFloat(self.PMergeTolerance, 0, 86400)
		self.showLastUpdate  = self.parseBool(self.PShowLastUpdate)
		self.excludeTooLarge = self.parseBool(self.PExcludeTooLarge)
		if self.excludeTooLarge is None:
			self.excludeTooLarge = True

		if self.channel is None:
			raise ValueError, 'Request contains no selections'

		if self.mergeTolerance is not None and not self.mergeOverlap:
			raise ValueError, 'mergetolerance given ({0:.1f}) but ' \
			                  'mergeoverlap is not specified'.format(
			                  self.mergeTolerance)


################################################################################
class AvailabilityQuery(_Availability):
	isLeaf = True

	#---------------------------------------------------------------------------
	def __init__(self):
		_Availability.__init__(self)

	#---------------------------------------------------------------------------
	def _createRequestOptions(self, args=None):
		return _AvailabilityQueryRequestOptions(args)


	#---------------------------------------------------------------------------
	def _writeJSONChannels(self, req, header, footer, lines, ro):
		byteCount   = 0
		lineCount   = 0

		nslc        = '{{' \
		                '"net":"{0}",' \
		                '"sta":"{1}",' \
		                '"loc":"{2}",' \
		                '"cha":"{3}"'
		quality     =   ',"quality":"{0}"'
		sampleRate  =   ',"sample_rate":{0}'
		updated     =   ',"updated":"{0}"'
		timeSpans   =   ',"timespans":[['
		seg         =   '"{0}","{1}"]'
		segUpdated  =   '"{0}","{1}","{2}"]'

		class SegGroup:
			def __init__(self, segments, updated):
				self.segments = segments
				self.updated = updated

		def writeSegments(segments):
			first     = True
			byteCount = 0

			if ro.showLastUpdate:
				for s in segments:
					if first:
						first = False
						data = timeSpans
					else:
						data = ",["

					data += segUpdated.format(
					        self._formatTime(s.start(), True),
					        self._formatTime(s.end(), True),
					        self._formatTime(s.updated()))
					utils.writeTS(req, data)
					byteCount += len(data)
			else:
				for s in segments:
					if first:
						first = False
						data = timeSpans
					else:
						data = ",["

					data += seg.format(
					        self._formatTime(s.start(), True),
					        self._formatTime(s.end(), True))
					utils.writeTS(req, data)
					byteCount += len(data)

			return byteCount

		ext = None

		# merge of quality and sample rate: all timespans of one stream are
		# grouped into one channel
		if ro.mergeQuality and ro.mergeSampleRate:
			lastUpdate = None
			segments   = None
			for line in lines:
				s = line[1]
				lineCount += 1

				if line[0] is not ext:
					if ext is not None:
						if byteCount == 0:
							utils.writeTS(req, header)
							byteCount += len(header)
							data = ''
						else:
							data = ']},'

						wid = ext.waveformID()
						data += nslc.format(wid.networkCode(), wid.stationCode(),
						                    wid.locationCode(), wid.channelCode())
						if ro.showLastUpdate:
							data += updated.format(self._formatTime(lastUpdate))
						utils.writeTS(req, data)
						byteCount += len(data)
						byteCount += writeSegments(segments)

					ext = line[0]
					lastUpdate = s.updated()
					segments = [ s ]
				else:
					if s.updated() > lastUpdate:
						lastUpdate = s.updated()
					segments.append(s)

			# handle last extent
			if ext is not None:
				if byteCount == 0:
					utils.writeTS(req, header)
					byteCount += len(header)
					data = ''
				else:
					data = ']},'

				wid = ext.waveformID()
				data += nslc.format(wid.networkCode(), wid.stationCode(),
				                    wid.locationCode(), wid.channelCode())
				if ro.showLastUpdate:
					data += updated.format(self._formatTime(lastUpdate))
				utils.writeTS(req, data)
				byteCount += len(data)
				byteCount += writeSegments(segments)

				data = ']}';
				utils.writeTS(req, data)
				byteCount += len(data)
				utils.writeTS(req, footer)
				byteCount += len(footer)

		# merge of quality: all timespans of one stream are grouped by
		# sample rate
		elif ro.mergeQuality:
			segGroups = None
			for line in lines:
				s = line[1]
				lineCount += 1

				if line[0] is not ext:
					if ext is not None:
						wid = ext.waveformID()
						for sr, sg in segGroups.iteritems():
							if req._disconnected: return False
							if byteCount == 0:
								utils.writeTS(req, header)
								byteCount += len(header)
								data = ''
							else:
								data = ']},'

							data += nslc.format(wid.networkCode(),
							                    wid.stationCode(),
							                    wid.locationCode(),
							                    wid.channelCode())
							data += sampleRate.format(sr)
							if ro.showLastUpdate:
								data += updated.format(
								        self._formatTime(sg.updated))
							utils.writeTS(req, data)
							byteCount += len(data)
							byteCount += writeSegments(sg.segments)

					ext = line[0]
					segGroups = {}
					segGroups[s.sampleRate()] = SegGroup([ s ], s.updated())
				else:
					if s.sampleRate() in segGroups:
						segGroup = segGroups[s.sampleRate()]
						if s.updated() > segGroup.updated:
							segGroup.updated = s.updated()
						segGroup.segments.append(s)
					else:
						segGroups[s.sampleRate()] = SegGroup([ s ], s.updated())

			# handle last extent
			if ext is not None:
				wid = ext.waveformID()
				for sr, sg in segGroups.iteritems():
					if req._disconnected: return False
					if byteCount == 0:
						utils.writeTS(req, header)
						byteCount += len(header)
						data = ''
					else:
						data = ']},'

					data += nslc.format(wid.networkCode(), wid.stationCode(),
					                    wid.locationCode(), wid.channelCode())
					data += sampleRate.format(sr)
					if ro.showLastUpdate:
						data += updated.format(self._formatTime(sg.updated))
					utils.writeTS(req, data)
					byteCount += len(data)
					byteCount += writeSegments(sg.segments)

				data = ']}';
				utils.writeTS(req, data)
				byteCount += len(data)
				utils.writeTS(req, footer)
				byteCount += len(footer)

		# merge of sample rate: all timespans of one stream are grouped by
		# quality
		elif ro.mergeSampleRate:
			segGroups = None
			for line in lines:
				s = line[1]
				lineCount += 1

				if line[0] is not ext:
					if ext is not None:
						wid = ext.waveformID()
						for q, sg in segGroups.iteritems():
							if req._disconnected: return False
							if byteCount == 0:
								utils.writeTS(req, header)
								byteCount += len(header)
								data = ''
							else:
								data = ']},'

							data += nslc.format(wid.networkCode(),
							                    wid.stationCode(),
							                    wid.locationCode(),
							                    wid.channelCode())
							data += quality.format(q)
							if ro.showLastUpdate:
								data += updated.format(
								        self._formatTime(sg.updated))
							utils.writeTS(req, data)
							byteCount += len(data)
							byteCount += writeSegments(sg.segments)

					ext = line[0]
					segGroups = {}
					segGroups[s.quality()] = SegGroup([ s ], s.updated())
				else:
					if s.quality() in segGroups:
						segGroup = segGroups[s.quality()]
						if s.updated() > segGroup.updated:
							segGroup.updated = s.updated()
						segGroup.segments.append(s)
					else:
						segGroups[s.quality()] = SegGroup([ s ], s.updated())

			# handle last extent
			if ext is not None:
				wid = ext.waveformID()
				for q, sg in segGroups.iteritems():
					if req._disconnected: return False
					if byteCount == 0:
						utils.writeTS(req, header)
						byteCount += len(header)
						data = ''
					else:
						data = ']},'

					data += nslc.format(wid.networkCode(), wid.stationCode(),
					                    wid.locationCode(), wid.channelCode())
					data += quality.format(q)
					if ro.showLastUpdate:
						data += updated.format(self._formatTime(sg.updated))
					utils.writeTS(req, data)
					byteCount += len(data)
					byteCount += writeSegments(sg.segments)

				data = ']}';
				utils.writeTS(req, data)
				byteCount += len(data)
				utils.writeTS(req, footer)
				byteCount += len(footer)

		# no merge: all timespans of one stream are grouped by tuple of
		# quality and sampleRate
		else:
			segGroups = None
			for line in lines:
				s = line[1]
				lineCount += 1

				if line[0] is not ext:
					if ext is not None:
						wid = ext.waveformID()
						for (q, sr), sg in segGroups.iteritems():
							if req._disconnected: return False
							if byteCount == 0:
								utils.writeTS(req, header)
								byteCount += len(header)
								data = ''
							else:
								data = ']},'

							data += nslc.format(wid.networkCode(),
							                    wid.stationCode(),
							                    wid.locationCode(),
							                    wid.channelCode())
							data += quality.format(q)
							data += sampleRate.format(sr)
							if ro.showLastUpdate:
								data += updated.format(
								        self._formatTime(sg.updated))
							utils.writeTS(req, data)
							byteCount += len(data)
							byteCount += writeSegments(sg.segments)

					ext = line[0]
					segGroups = {}
					segGroups[(s.quality(), s.sampleRate())] = \
					        SegGroup([ s ], s.updated())
				else:
					t = (s.quality(), s.sampleRate())
					if t in segGroups:
						segGroup = segGroups[t]
						if s.updated() > segGroup.updated:
							segGroup.updated = s.updated()
						segGroup.segments.append(s)
					else:
						segGroups[t] = SegGroup([ s ], s.updated())

			# handle last extent
			if ext is not None:
				wid = ext.waveformID()
				for (q, sr), sg in segGroups.iteritems():
					if req._disconnected: return False
					if byteCount == 0:
						utils.writeTS(req, header)
						byteCount += len(header)
						data = ''
					else:
						data = ']},'

					data += nslc.format(wid.networkCode(), wid.stationCode(),
					                    wid.locationCode(), wid.channelCode())
					data += quality.format(q)
					data += sampleRate.format(sr)
					if ro.showLastUpdate:
						data += updated.format(self._formatTime(sg.updated))
					utils.writeTS(req, data)
					byteCount += len(data)
					byteCount += writeSegments(sg.segments)

				data = ']}';
				utils.writeTS(req, data)
				byteCount += len(data)
				utils.writeTS(req, footer)
				byteCount += len(footer)


		return byteCount, lineCount


	#---------------------------------------------------------------------------
	def _processRequest(self, req, ro, dac):
		if req._disconnected: return False

		# tuples: wid, segment, restricted status
		lines = [ ]

		mergeAll = ro.mergeQuality and ro.mergeSampleRate
		mergeNone = not ro.mergeQuality and not ro.mergeSampleRate
		mergeOne = not mergeAll and not mergeNone

		byteCount   = 0

		# iterate extents and create IN clauses of parent_oids in bunches
		# of 1000 because the query size is limited
		parentOIDs, idList, tooLarge = [], [], []
		i = 0
		for ext, objID, restricted in ro.extentIter(dac):
			if req._disconnected: return False

			if ro.excludeTooLarge:
				if ext.segmentOverflow():
					continue
			elif ext.segmentOverflow():
				tooLarge.append(ext)
				continue
			elif tooLarge:
				continue

			if i < 1000:
				idList.append(objID)
				i += 1
			else:
				parentOIDs.append(idList)
				idList = [ objID ]
				i = 1

		if not ro.excludeTooLarge and tooLarge:
			extents = ', '.join('{0}.{1}.{2}.{3}'.format(
			        e.waveformID().networkCode(),
			        e.waveformID().stationCode(),
			        e.waveformID().locationCode(),
			        e.waveformID().channelCode()) for e in tooLarge)

			msg = 'Unable to process request due to database limitations. ' \
			      'Some selections have too many segments to process. ' \
			      'Rejected extents: {{{0}}}. This limitation may be ' \
			      'resolved in a future version of this webservice.' \
			      .format(extents)
			data = self.renderErrorPage(req, http.REQUEST_ENTITY_TOO_LARGE,
			                            msg, ro)
			if data:
				utils.writeTS(req, data)
			return False
		elif len(idList) > 0:
			parentOIDs.append(idList)
		else:
			msg = "no matching availabilty information found"
			data = self.renderErrorPage(req, http.NO_CONTENT, msg, ro)
			if data:
				utils.writeTS(req, data)
			return False

		db = IO.DatabaseInterface.Open(Application.Instance().databaseURI())
		if db is None:
			msg = "could not connect to database"
			return self.renderErrorPage(req, http.SERVICE_UNAVAILABLE, msg, ro)

		lines = self._lineIter(db, parentOIDs, req, ro, dac.extentsOID())

		byteCount, segCount = self._writeLines(req, lines, ro)

		# Return 204 if no matching availability information was found
		if segCount <= 0:
			msg = "no matching availabilty information found"
			data = self.renderErrorPage(req, http.NO_CONTENT, msg, ro)
			if data:
				utils.writeTS(req, data)
			return True

		Logging.debug("%s: returned %i segments (total bytes: %i)" % (
		              ro.service, segCount, byteCount))
		utils.accessLog(req, ro, http.OK, byteCount, None)

		return True


	#---------------------------------------------------------------------------
	def _lineIter(self, db, parentOIDs, req, ro, oIDs):

		def _T(name):
			return db.convertColumnName(name)

		dba = DataModel.DatabaseArchive(db)

		for idList in parentOIDs:
			if req._disconnected: raise StopIteration

			# build SQL query
			q = 'SELECT * from DataSegment ' \
			    'WHERE _parent_oid IN ({0}) ' \
			    .format(','.join(str(x) for x in idList))
			if ro.time:
				if ro.time.start is not None:
					if ro.time.start.microseconds() == 0:
						q += "AND {0} >= '{1}' " \
						     .format(_T('end'), db.timeToString(ro.time.start))
					else:
						q += "AND ({0} > '{1}' OR (" \
						          "{0} = '{1}' AND end_ms >= {2})) " \
						     .format(_T('end'), db.timeToString(ro.time.start),
						             ro.time.start.microseconds())
				if ro.time.end is not None:
					if ro.time.end.microseconds() == 0:
						q += "AND {0} < '{1}' " \
						     .format(_T('start'), db.timeToString(ro.time.end))
					else:
						q += "AND ({0} < '{1}' OR (" \
						          "{0} = '{1}' AND start_ms < {2})) " \
						     .format(_T('start'), db.timeToString(ro.time.end),
						             ro.time.end.microseconds())
			if ro.quality:
					q += "AND {0} IN ('{1}') ".format(_T('quality'),
					     "', '".join(ro.quality))
			q += 'ORDER BY _parent_oid, {0}, {1}' \
			     .format(_T('start'), _T('start_ms'))

			segIt = dba.getObjectIterator(q, DataModel.DataSegment.TypeInfo())
			if segIt is None or not segIt.valid():
				raise StopIteration

			if not ro.mergeOverlap:
				# No merge of overlap requested, segment can be instantly
				# processed.
				while not req._disconnected:
					s = DataModel.DataSegment.Cast(segIt.get())
					if s is None:
						break
					try:
						ext, restricted = oIDs[segIt.parentOid()]
					except KeyError:
						Logging.warning("parent object id not found: %i",
						                segIt.parentOid())
						continue
					yield (ext, s, restricted)

					if not segIt.next():
						break

			else:
				# Merge overlapping segments (gaps and overlaps): segment can
				# only be yielded if
				# - extent changed
				# - quality changed and merging of quality was not requested
				# - sample rate changed and merging of sample rate was not
				#   requested
				# - mergetolerance is exceeded
				seg   = None
				ext   = None
				while not req._disconnected and (seg is None or segIt.next()):
					s = DataModel.DataSegment.Cast(segIt.get())
					if s is None:
						break

					try:
						ext, restricted = oIDs[segIt.parentOid()]
					except KeyError:
						Logging.warning("parent object id not found: %i",
						                segIt.parentOid())
						continue

					if seg is None:
						seg = s
						ext = e
					elif e is ext and \
					     (ro.mergeQuality or s.quality() == seg.quality()) and \
					     (ro.mergeSampleRate or \
					          s.sampleRate() == seg.sampleRate()) and \
					     abs(float(s.start() - seg.end())) <= ro.mergeTolerance:

						seg.setEnd(s.end())
						if s.updated() > seg.updated():
							seg.setUpdated(s.updated())
					else:
						yield (ext, s, restricted)
						seg = s
						ext = e

				if seg is not None:
					yield (ext, seg, restriction)



# vim: ts=4 noet
