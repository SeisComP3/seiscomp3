################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# Common utility functions
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################

import socket, traceback

from twisted.internet import reactor, defer
from twisted.python.failure import Failure

from seiscomp3 import Logging
from seiscomp3.Client import Application
from seiscomp3.Core import Time
from seiscomp3.IO import ExportSink


#-------------------------------------------------------------------------------
# Tests if a SC3 inventory object is restricted
def isRestricted(obj):
	try:
		return obj.restricted()
	except ValueError:
		return False


#-------------------------------------------------------------------------------
# Thread-safe write of data using reactor main thread
def writeTS(req, data):
	reactor.callFromThread(req.write, data)


#-------------------------------------------------------------------------------
# Finish requests deferred to threads
def onFinish(result, req):
	Logging.debug("finish value = %s" % str(result))
	if isinstance(result, Failure):
		err = result.value
		if isinstance(err, defer.CancelledError):
			Logging.error("request canceled")
			return
		Logging.error("%s %s" % (result.getErrorMessage(),
		              traceback.format_tb(result.getTracebackObject())))
	else:
		if result:
			Logging.debug("request successfully served")
		else:
			Logging.debug("request failed")

	reactor.callFromThread(req.finish)
	#req.finish()


#-------------------------------------------------------------------------------
# Handle connection errors
def onCancel(failure, req):
	if failure:
		Logging.error("%s %s" % (failure.getErrorMessage(),
		              traceback.format_tb(failure.getTracebackObject())))
	else:
		Logging.error("request canceled")
	req.cancel()


#-------------------------------------------------------------------------------
# Handle premature connection reset
def onResponseFailure(err, call):
	Logging.error("response canceled")
	call.cancel()


#-------------------------------------------------------------------------------
# Renders error page if the result set exceeds the configured maximum number
# objects
def accessLog(req, ro, code, length, err):
	logger = Application.Instance()._accessLog
	if logger is None:
		return

	logger.log(AccessLogEntry(req, ro, code, length, err))


################################################################################
class Sink(ExportSink):
	def __init__(self, request):
		ExportSink.__init__(self)
		self.request = request
		self.written = 0

	def write(self, data, size):
		if self.request._disconnected:
			return -1
		writeTS(self.request, data[:size])
		self.written += size
		return size


################################################################################
class AccessLogEntry:
	def __init__(self, req, ro, code, length, err):
		# user agent
		agent = req.getHeader("User-Agent")
		if agent is None:
			agent = ""
		else:
			agent = agent[:100].replace('|', ' ')

		if err is None:
			err = ""

		service, user, accessTime, procTime = "", "", "", 0
		net, sta, loc, cha = "", "", "", ""
		if ro is not None:
			# processing time in milliseconds
			procTime = int((Time.GMT() - ro.accessTime).length() * 1000.0)

			service = ro.service
			if ro.userName is not None:
				user = ro.userName
			accessTime = str(ro.accessTime)

			if ro.channel is not None:
				if ro.channel.net is not None:
					net = ",".join(ro.channel.net)
				if ro.channel.sta is not None:
					sta = ",".join(ro.channel.sta)
				if ro.channel.loc is not None:
					loc = ",".join(ro.channel.loc)
				if ro.channel.cha is not None:
					cha = ",".join(ro.channel.cha)

		# The host name of the client is resolved in the __str__ method by the
		# logging thread so that a long running DNS reverse lookup may not slow
		# down the request
		self.msgPrefix = "%s|%s|%s|" % (service, req.getRequestHostname(),
		                                accessTime)
		self.clientIP = req.getClientIP()
		self.msgSuffix = "|%s|%i|%i|%s|%s|%i|%s|%s|%s|%s|%s||" % (
		                 self.clientIP, length, procTime, err, agent, code,
		                 user, net, sta, loc, cha)

	def __str__(self):
		try:
			clientName = socket.gethostbyaddr(self.clientIP)[0]
		except socket.herror:
			clientName = ""
		return self.msgPrefix + clientName + self.msgSuffix


