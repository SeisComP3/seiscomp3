# -*- coding: utf-8 -*-
#***************************************************************************** 
# reqtrack2db.py
#
# copy reqtrack dir entries into data base
#
# (c) 2010 Mathias Hoffmann, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************
import cPickle

import random
import re, os, sys
from datetime import datetime, timedelta, date
from seiscomp3 import DataModel, Core, IO, Client

SEISCOMP_ROOT = os.getenv("SEISCOMP_ROOT")
REQTRACK_DIR = os.path.join(SEISCOMP_ROOT, "var", "lib", "arclink", "reqtrack"
ARC_DIR =  os.path.join(SEISCOMP_ROOT, "var", "lib", "arclink", "reqtrackArchive"
#USERS_IGNORE = ["arclink_probe@gfz-potsdam.de", "andres@gfz-potsdam.de"]
#LABEL_IGNORE = ["breq_req"]
USERS_IGNORE = []
LABEL_IGNORE = []



def load(fname):
	f = open(fname, "rb")
	sys.stderr.write("loading from file:  %s\n" % fname)
	sys.stderr.flush()
	ret = cPickle.load(f)
	f.close()
	return ret


class RequestSummarizer(object):
	
	statusLineSelectorRe = (
		"^(?P<user>USER)\s+(?P<user_name>.*)$",
		"^(?P<label>LABEL)\s+(?P<label_name>.*)$",
		"^(?P<request>REQUEST)\s+(?P<request_type>\w+)\s+(?P<request_id>\w+)\s+(?P<constrains>.*)$",
		"^(?P<request_line>\d+),.*$",
		"^(?P<volume>VOLUME)\s+(?P<vol_type>\w+)\s+(?P<status>\w+)\s+(?P<size>\w+)\s*(?P<vol_error>.*)$",
		"^(?P<error>ERROR)\s+(?P<error_error>.*)$",
		"^(?P<end>END)\s*$"\
		)
	statusLineSelector = re.compile("|".join(statusLineSelectorRe))
	
	statusReqLineRe = (
		"^((?P<beg_2year>\d{2})|(?P<beg_4year>\d{4})),",
		"(?P<beg_month>\d{1,2}),",
		"(?P<beg_day>\d{1,2}),",
		"(?P<beg_hour>\d{1,2}),",
		"(?P<beg_min>\d{1,2}),",
		"(?P<beg_sec>\d{1,2})(\.\d*)?\s+",
		
		"((?P<end_2year>\d{2})|(?P<end_4year>\d{4})),",
		"(?P<end_month>\d{1,2}),",
		"(?P<end_day>\d{1,2}),",
		"(?P<end_hour>\d{1,2}),",
		"(?P<end_min>\d{1,2}),",
		"(?P<end_sec>\d{1,2})(\.\d*)?\s+",
		
		"(?P<network>[\w?\*\.]+)\s+",
		"(?P<station>[\w?\*\.]+)\s+",
		"(?P<channel>[\w?\*\.,\[\]]+)\s+",
		"(?P<location>[\w?\*\.\+]+)\s+",
		
		"\((?P<constrains>.*)\)\s+",
		"(?P<req_type>.*?)\s+",
		"(?P<status>.*?)\s+",
		"(?P<size>\d*)\s*",
		"(?P<message>.*)$"\
		)
	statusRequestLine = re.compile("".join(statusReqLineRe))


	def __new__(cls, *args, **kwargs):
		archiveDir = kwargs.get('archiveDir', ARC_DIR)
		startTime = args[0].replace(hour=0, minute=0, second=0, microsecond=0)
		endTime = args[0].replace(hour=23, minute=59, second=59, microsecond=999)
		fname = None
		
		if archiveDir:
			fname = "%s/ReqSumArc_%s" % (archiveDir, startTime.date())
			
		try:
			if endTime >= datetime.now():
				fname = None
				sys.stderr.write("day not complete: ")
				sys.stderr.flush()
				os.remove(fname)
				raise IOError
			obj = load(fname)
		except:
			obj = object.__new__(cls)
			obj.archiveName = fname
			obj.__init(*args, **kwargs)
		
		return obj
		

	def __init(self, day, reqTrackDir=REQTRACK_DIR, archiveDir=False):
		self.startTime = day.replace(hour=0, minute=0, second=0, microsecond=0)
		self.endTime = day.replace(hour=23, minute=59, second=59, microsecond=999)
		self.users = dict()
		self.reqTrackDir = reqTrackDir
		
		self.scan(self.reqTrackDir, self.startTime, self.endTime)
		
		if self.archiveName and len(self.users) > 0:
			store(self, self.archiveName)
		
		
	def setUser(self, userName):
		user = self.users.setdefault(userName, User(userName))
		return user
		
	def getUserNames(self):
		return self.users.keys()
	
	def getUsers(self):
		return self.users.values()
	
	
	
	# recursive scan of directories
	def scan(self, reqtrackDir, startTime=None, endTime=None):
		d = os.path.abspath(reqtrackDir)
		for f in [f for f in os.listdir(d) if not f in [".",".."]]:
			nfile = os.path.join(d,f)
			if os.path.isdir(nfile):
				self.scan(nfile, startTime, endTime)
			else:
				if f == "status":
					ftime = datetime.fromtimestamp(os.path.getmtime(nfile))
					if startTime and ftime < startTime: continue
					if endTime and ftime > endTime: continue
					self.processFile(nfile, ftime)

	
	def processFile(self, sFile, reqdate):
		#sys.stderr.write("scanning file: %s\n" % sFile)
		#sys.stderr.flush()
		try:
			sFile = open(sFile, "r")
			label = None
			for line in sFile:
				m = self.statusLineSelector.search(line)
				if m:
					d = m.groupdict()
					if d["user"] is not None:
						if d["user_name"] in USERS_IGNORE:
							#print("ignoring: %s" % d["user_name"])
							break
						user = self.setUser(d['user_name'])
					elif d["label"] is not None:
						label = d['label_name']
						if label in LABEL_IGNORE:
							print("ignoring: %s" % label)
							break
					elif d["request"] is not None:
						req = user.addRequest(reqdate, d['request_id'], d['request_type'], label)
						req.constrains.append(d["constrains"])
					elif d["request_line"] is not None:
						mm = self.statusRequestLine.search(line)
						if mm:
							dd = mm.groupdict()
							start = datetime(int(dd["beg_4year"]), int(dd["beg_month"]), int(dd["beg_day"]), int(dd["beg_hour"]), int(dd["beg_min"]), int(dd["beg_sec"]))
							end = datetime(int(dd["end_4year"]), int(dd["end_month"]), int(dd["end_day"]), int(dd["end_hour"]), int(dd["end_min"]), int(dd["end_sec"]))
							reqLine = RequestLine(start, end, dd["network"], dd["station"], dd["channel"], dd["location"], dd["constrains"], dd["req_type"], dd["status"], dd["size"], dd["message"])
							req.addRequestLine(reqLine)
						else:
							sys.stderr.write("file: %s\n---> error parsing request line: %s" % (sFile, str(line)))	
					elif d["volume"] is not None:
						vol = VolumeLine(d["vol_type"], d["status"], d["size"], d["vol_error"])
						req.addVolumeLine(vol)
					elif d["error"] is not None:
						err = ErrorLine(d["error_error"])
						req.addErrorLine(err)
				else:
					sys.stderr.write("file: %s\n---> error parsing status line: %s" % (sFile, str(line)))
				
			sFile.close()
		except: pass

#------------------------------------------------------------------------------





#------------------------------------------------------------------------------
class User(object):
	def __init__(self, userName):
		self.userName = userName
		self.requests = dict()
	
	def __repr__(self):
		return str(self.userName)

	def addRequest(self, reqDate, reqID, reqType, label):
		request = Request(reqDate, reqID, reqType, label)
		self.requests[(reqDate, reqID, reqType, label)] = request
		return request
		
	def getRequests(self, startTime=None, endTime=None, rtype=None, rid=None, label=None):
		if not startTime and not endTime and not rtype and not rid:
			return self.requests.values()
		retList = list()
		for req in self.requests.items():
			(reqDate, reqID, reqType, reqLabel) = req[0]
			if startTime and reqDate < startTime:
				continue
			if endTime and reqDate > endTime:
				continue
			if rtype and reqType != rtype:
				continue
			if rid and reqID != rid:
				continue
			if label and reqLabel != label:
				continue
			retList.append(req[1])
		return retList
		
	
	def requestCount(self):
		return len(self.requests)
	
#------------------------------------------------------------------------------




#------------------------------------------------------------------------------
class Request(object):
	def __init__(self, reqDate=None, reqID=None, reqType=None, label=None):
		self.reqDate = reqDate
		self.reqID = reqID
		self.reqType = reqType
		self.reqLabel = label
		self.constrains = list()
		self.requestLines = list()
		self.volumeLines = list()
		self.errorLines = list()

	def __repr__(self):
		ret = str()
		ret += " ".join(map(str, (self.reqDate, self.reqID, self.reqType, self.constrains)))
		for rl in self.requestLines:
			ret += rl
		return ret
	
	
	def totalLineCount(self):
		return self.requestLineCount() + self.volumeLineCount() + self.errorLineCount()
	
	
	def addRequestLine(self, line):
		self.requestLines.append(line)
	
	def getRequestLines(self):
		return self.requestLines
	
	def requestLineCount(self):
		try:
			return len(self.requestLines)
		except:
			return 0
	
	def requestLength(self):
		l = timedelta()
		for reqLine in self.getRequestLines():
				l += reqLine.length
		return l
	
	
	def addVolumeLine(self, line):
		self.volumeLines.append(line)
	
	def getVolumeLines(self):
		return self.volumeLines
	
	def volumeLineCount(self):
		try:
			return len(self.volumeLines)
		except:
			return 0
	
	def volumeSize(self):
		size = 0
		for line in self.volumeLines:
			size += int(line.size)
		return size
	
	
	def addErrorLine(self, line):
		self.errorLines.append(line)
	
	def getErrorLines(self):
		return self.errorLines
	
	def errorLineCount(self):
		try:
			return len(self.errorLines)
		except:
			return 0
		
	
	def getStreams(self):
		retList = list()
		for line in self.requestLines:
			td = line.end - line.start
			retList.append(("%s.%s.%s.%s" % (line.network, line.station, line.location, line.channel), td ))
		return retList
	
#------------------------------------------------------------------------------




#------------------------------------------------------------------------------
class RequestLine(object):
	def __init__(self, start, end, net, sta, cha, loc, constrains, data, status, size, message):
		self.start = start
		self.end = end
		self.network = net
		self.station = sta
		self.channel = cha
		self.location = loc
		self.constrains = constrains
		self.data = data
		self.status = status
		self.size = size
		self.message = message
		
		
	def __repr__(self):
		return " ".join(map(str, (self.start, "-", self.end, self.network, self.station, self.channel, self.location, self.constrains, self.data, self.status, self.size, self.message)))
	
	def _streamCode(self):
		return ".".join(net, sta, cha, loc)
	streamCode = property(_streamCode)
	
	def _length(self):
		td = self.end - self.start
		return td
	length = property(_length)
		
#------------------------------------------------------------------------------




#------------------------------------------------------------------------------
class VolumeLine(object):
	def __init__(self, data, status, size, message):
		self.data = data
		self.status = status
		self.size = size
		self.message = message
		
	def __repr__(self):
		return " ".join(map(str, (self.data, self.status, self.size, self.message)))
#------------------------------------------------------------------------------




#------------------------------------------------------------------------------
class ErrorLine(object):
	def __init__(self, message):
		self.message = message
		
	def __repr__(self):
		return str(self.message)
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
def catch(f):
    def wrap(*args, **kwargs):
		try:
			f(*args, **kwargs)
		except Exception as e:
			print("Hmm. An ERROR has occurred ...", e)
    return wrap
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
class RequestTrackerDB(object):

	def __init__(self, dbWrite, req, user):
		self.dbWrite = dbWrite
		self.arclinkRequest = DataModel.ArclinkRequest.Create()
		self.arclinkRequest.setCreated(Core.Time.FromString(str(req.reqDate), "%Y-%m-%d %H:%M:%S"))
		self.arclinkRequest.setRequestID(req.reqID)
		self.arclinkRequest.setUserID(user)
		self.arclinkRequest.setType(req.reqType)
		self.arclinkRequest.setLabel(str(req.reqLabel))
		self.arclinkRequest.setHeader(" ".join(req.constrains))

		self.averageTimeWindow = Core.TimeSpan(0.)
		self.totalLineCount = req.requestLineCount()
		#self.okLineCount = self.totalLineCount - req.errorLineCount()
		self.okLineCount = 0

		for line in req.getRequestLines():
			self.line_status(line)

		for line in req.getVolumeLines():
			self.volume_status(line)

		self.request_status("END", "") # FIXME


	@catch
	def send(self):
		print("writing request with ID %s to DB" % self.arclinkRequest.requestID())
		al = DataModel.ArclinkLog()
		al.add(self.arclinkRequest)
		self.arclinkRequest.accept(self.dbWrite)

	@catch
	def __call__(self):
		return self.arclinkRequest


	@catch
	def line_status(self, line):
		start_time = line.start
		end_time = line.end
		network = line.network
		station = line.station
		channel = line.channel
		location = line.location
		constraints = line.constrains
		volume = line.data
		status = line.status
		size = line.size
		message = line.message

		if network is None or network == "":
			network = "."
		if station is None or station == "":
			station = "."
		if channel is None or channel == "":
			channel = "."
		if location is None or location == "":
			location = "."
		if volume is None:
			volume = "NODATA"
		if size is None:
			size = 0
		if message is None:
			message = ""

		startTime = Core.Time.FromString(str(start_time), "%Y-%m-%d %H:%M:%S")
		endTime = Core.Time.FromString(str(end_time), "%Y-%m-%d %H:%M:%S")
		
		if startTime <= endTime: validTime = True
		else: validTime = False

		if isinstance(constraints, list):
			constraints = " ".join(constraints)

		arclinkRequestLine = DataModel.ArclinkRequestLine()
		arclinkRequestLine.setStart(startTime)
		arclinkRequestLine.setEnd(endTime)
		arclinkRequestLine.setStreamID(DataModel.WaveformStreamID(network, station, location, channel, ""))
		arclinkRequestLine.setConstraints(constraints)
		#
		arclinkStatusLine = DataModel.ArclinkStatusLine()
		arclinkStatusLine.setVolumeID(volume)
		arclinkStatusLine.setStatus(status)
		arclinkStatusLine.setSize(int(size))
		arclinkStatusLine.setMessage(message)
		#
		arclinkRequestLine.setStatus(arclinkStatusLine)
		self.arclinkRequest.add(arclinkRequestLine)

		if validTime: self.averageTimeWindow += endTime - startTime
		#self.totalLineCount += 1
		if status == "OK": self.okLineCount += 1

	@catch
	def volume_status(self, line):
		volume = line.data
		status = line.status
		size = int(line.size)
		message = line.message

		if volume is None:
			volume = "NODATA"
		if size is None:
			size = 0
		if message is None:
			message = ""

		arclinkStatusLine = DataModel.ArclinkStatusLine()
		arclinkStatusLine.setVolumeID(volume)
		arclinkStatusLine.setStatus(status)
		arclinkStatusLine.setSize(size)
		arclinkStatusLine.setMessage(message)
		print("adding Volume: ", volume)
		self.arclinkRequest.add(arclinkStatusLine)

	@catch
	def request_status(self, status, message):
		if message is None:
			message = ""

		self.arclinkRequest.setStatus(status)
		self.arclinkRequest.setMessage(message)

		ars = DataModel.ArclinkRequestSummary()
		tw = self.averageTimeWindow.seconds()
		if self.totalLineCount > 0:
			tw = self.averageTimeWindow.seconds() / self.totalLineCount # avarage request time window
		if tw > 2**32: tw = -1 # prevent 32bit int overflow
		ars.setAverageTimeWindow(tw)
		ars.setTotalLineCount(self.totalLineCount)
		ars.setOkLineCount(self.okLineCount)
		self.arclinkRequest.setSummary(ars)

		self.send()
#------------------------------------------------------------------------------





#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
class App(Client.Application):

#------------------------------------------------------------------------------
	def __init__(self, argc, argv):
		Client.Application.__init__(self, argc, argv)

		self.setMessagingEnabled(False)
		self.setDatabaseEnabled(True, True)
		self.setAutoApplyNotifierEnabled(False)
		self.setInterpretNotifierEnabled(False)
#------------------------------------------------------------------------------




#------------------------------------------------------------------------------
	def createCommandLineDescription(self):
		self.commandline().addGroup("Date")
		self.commandline().addStringOption("Date", "startTime,b", "start date: YYYY-MM-DD")
		self.commandline().addStringOption("Date", "endTime,e", "end date: YYYY-MM-DD")

		return True
#------------------------------------------------------------------------------




#------------------------------------------------------------------------------
	def initConfiguration(self):
		if not Client.Application.initConfiguration(self):
			return False

		return True
#------------------------------------------------------------------------------




#------------------------------------------------------------------------------
	def validateParameters(self):

		try:
			self.start = self.commandline().optionString("startTime")
			self.end = self.commandline().optionString("endTime")
		except:
			print("Error in start/end Time")
			return False

		return True
#------------------------------------------------------------------------------





#------------------------------------------------------------------------------
	def run(self):

		startTime = datetime.strptime(self.start,"%Y-%m-%d")
		endTime   = datetime.strptime(self.end,"%Y-%m-%d")

		dba = DataModel.DatabaseArchive(self.database())
		dbWrite = DataModel.DatabaseObjectWriter(dba, 100)
		
		month = startTime.month
		day = startTime
		while day <= endTime:
			print(str(day))
			#rs = RequestSummarizer(day)
			rs = RequestSummarizer(day, archiveDir=False)

			users = rs.getUsers()
			print(users)

			for user in users:
				requests = user.getRequests()
				for req in requests:
					rt = RequestTrackerDB(dbWrite, req, user.userName)

			day += timedelta(1)

		print("count : %d" % dbWrite.count())
		print("errors: %d" % dbWrite.errors())

		return True
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

###############################################################################

if __name__ == "__main__":

	app = App(len(sys.argv), sys.argv)
	sys.exit(app())

###############################################################################
