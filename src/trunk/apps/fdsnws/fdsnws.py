#!/usr/bin/env python

################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# FDSNWS -- Implements FDSN Web Service interface, see
# http://www.fdsn.org/webservices/
#
# Implemented Services:
#   fdsnws-dataselect
#   fdsnws-event
#   fdsnws-station
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################


import os, sys

try:
	from twisted.cred import checkers, credentials, error, portal
	from twisted.internet import reactor, defer
	from twisted.web import guard, resource, server, static
	from twisted.python import log
except ImportError, e:
	sys.exit("%s\nIs python twisted installed?" % str(e))

try:
	from seiscomp3 import Core, DataModel, Logging
	from seiscomp3.Client import Application
	from seiscomp3.System import Environment
	from seiscomp3.Config import Exception as ConfigException
except ImportError, e:
	sys.exit("%s\nIs the SeisComP environment set correctly?" % str(e))

from seiscomp3.fdsnws.dataselect import FDSNDataSelect, FDSNDataSelectRealm
from seiscomp3.fdsnws.event import FDSNEvent
from seiscomp3.fdsnws.station import FDSNStation
from seiscomp3.fdsnws.http import DirectoryResource, ListingResource, NoResource, \
                                  Site, ServiceVersion
from seiscomp3.fdsnws.log import Log

def logSC3(entry):
	try:
		isError = entry['isError']
		msg = entry['message']
		if isError:
			for l in msg:
				Logging.error("[reactor] %s" % l)
		else:
			for l in msg:
				Logging.info("[reactor] %s" % l)
	except:
		pass


################################################################################
# Fixes bug of DigestCredentialFactory by overriding decode method,
# see http://twistedmatrix.com/trac/ticket/6445
class BugfixedDigest(credentials.DigestCredentialFactory):
	def decode(self, response, method, host):
		response = ' '.join(response.splitlines())

		# split comma separated parameters, don't split quoted strings, remove
		# quotes
		quoted = False
		parts = []
		p = ""
		for c in response:
			if c == '"':
				quoted = not quoted
			elif not quoted and c == ',':
				parts.append(p)
				p = ""
			else:
				p += c
		if p:
			parts.append(p)

		auth = {}

		for (k, v) in [p.split('=', 1) for p in parts]:
			auth[k.strip()] = v.strip()

		username = auth.get('username')
		if not username:
			raise error.LoginFailed("invalid response, no user name given")

		if 'opaque' not in auth:
			raise error.LoginFailed("invalid response, no opaque given")

		if 'nonce' not in auth:
			raise error.LoginFailed("invalid response, no nonce given.")

		# Now verify the nonce/opaque values for this client
		if self._verifyOpaque(auth.get('opaque'), auth.get('nonce'), host):
			return credentials.DigestedCredentials(username, method,
			                                       self.authenticationRealm,
			                                       auth)



################################################################################
class FDSNWS(Application):

	#---------------------------------------------------------------------------
	def __init__(self):
		Application.__init__(self, len(sys.argv), sys.argv)
		self.setMessagingEnabled(False)
		self.setDatabaseEnabled(True, True)
		self.setRecordStreamEnabled(True)

		self._serverRoot    = os.path.dirname(__file__)
		self._listenAddress = '0.0.0.0' # all interfaces
		self._port          = 8080
		self._connections   = 5
		self._queryObjects  = 100000    # maximum number of objects per query
		self._realtimeGap   = None      # minimum data age: 5min
		self._samplesM      = None      # maximum number of samples per query
		self._htpasswd      = '@CONFIGDIR@/fdsnws.htpasswd'
		self._accessLogFile = ''

		self._allowRestricted   = True
		self._serveDataSelect   = True
		self._serveEvent        = True
		self._serveStation      = True

		self._hideAuthor            = False
		self._evaluationMode        = None
		self._eventTypeWhitelist    = None
		self._eventTypeBlacklist    = None

		self._accessLog     = None
		self._inv           = None

		self._fileNamePrefix = 'fdsnws'

		# Leave signal handling to us
		Application.HandleSignals(False, False)


	#---------------------------------------------------------------------------
	def initConfiguration(self):
		if not Application.initConfiguration(self):
			return False

		cfg = self.configuration()

		# bind address and port
		try: self._listenAddress = cfg.getString('listenAddress')
		except ConfigException: pass
		try: self._port = cfg.getInt('port')
		except ConfigException: pass

		# maximum number of connections
		try: self._connections = cfg.getInt('connections')
		except ConfigException: pass

		# maximum number of objects per query, used in fdsnws-station and
		# fdsnws-event to limit main memory consumption
		try: self._queryObjects = cfg.getInt('queryObjects')
		except ConfigException: pass

		# restrict end time of request to now-realtimeGap seconds, used in
		# fdsnws-dataselect
		try: self._realtimeGap = cfg.getInt('realtimeGap')
		except ConfigException: pass

		# maximum number of samples (in units of million) per query, used in
		# fdsnws-dataselect to limit bandwidth
		try: self._samplesM = cfg.getDouble('samplesM')
		except ConfigException: pass

		# location of htpasswd file
		try:
			self._htpasswd = cfg.getString('htpasswd')
		except ConfigException: pass
		self._htpasswd = Environment.Instance().absolutePath(self._htpasswd)

		# location of access log file
		try:
			self._accessLogFile = Environment.Instance().absolutePath(
			                      cfg.getString('accessLog'))
		except ConfigException: pass

		# access to restricted inventory information
		try: self._allowRestricted = cfg.getBool('allowRestricted')
		except: pass

		# services to enable
		try: self._serveDataSelect = cfg.getBool('serveDataSelect')
		except: pass
		try: self._serveEvent = cfg.getBool('serveEvent')
		except: pass
		try: self._serveStation = cfg.getBool('serveStation')
		except: pass

		# event filter
		try: self._hideAuthor = cfg.getBool('hideAuthor')
		except: pass
		try:
			name = cfg.getString('evaluationMode')
			if name.lower() == DataModel.EEvaluationModeNames.name(DataModel.MANUAL):
				self._evaluationMode = DataModel.MANUAL
			elif name.lower() == DataModel.EEvaluationModeNames.name(DataModel.AUTOMATIC):
				self._evaluationMode = DataModel.AUTOMATIC
			else:
				print >> sys.stderr, "invalid evaluation mode string: %s" % name
				return False
		except: pass
		try:
			strings = cfg.getStrings('eventType.whitelist')
			if len(strings) > 1 or len(strings[0]):
				self._eventTypeWhitelist = [ s.lower() for s in strings ]
		except: pass
		try:
			strings = cfg.getStrings('eventType.blacklist')
			if len(strings) > 0 or len(strings[0]):
				self._eventTypeBlacklist = [ s.lower() for s in strings ]
		except: pass

		# prefix to be used as default for output filenames
		try: self._fileNamePrefix = cfg.getString('fileNamePrefix')
		except ConfigException: pass

		return True


	#---------------------------------------------------------------------------
	# Signal handling in Python and fork in wrapped C++ code is not a good
	# combination. Without digging too much into the problem, forking the
	# process with os.fork() helps
	def forkProcess(self):
		cp = os.fork()
		if cp < 0: return False
		elif cp == 0: return True
		elif cp > 0:
			sys.exit(0)


	#---------------------------------------------------------------------------
	def run(self):
		modeStr = None
		if self._evaluationMode is not None:
			modeStr = DataModel.EEvaluationModeNames.name(self._evaluationMode)
		whitelistStr = "<None>"
		if self._eventTypeWhitelist is not None:
			whitelistStr = ", ".join(self._eventTypeWhitelist)
		blacklistStr = "<None>"
		if self._eventTypeBlacklist is not None:
			blacklistStr = ", ".join(self._eventTypeBlacklist)
		Logging.notice("\n" \
		               "configuration read:\n" \
		               "  serve\n" \
		               "    dataselect    : %s\n" \
		               "    event         : %s\n" \
		               "    station       : %s\n" \
		               "  listenAddress   : %s\n" \
		               "  port            : %i\n" \
		               "  connections     : %i\n" \
		               "  htpasswd        : %s\n" \
		               "  accessLog       : %s\n" \
		               "  queryObjects    : %i\n" \
		               "  realtimeGap     : %s\n" \
		               "  samples (M)     : %s\n" \
		               "  allowRestricted : %s\n" \
		               "  hideAuthor      : %s\n" \
		               "  evaluationMode  : %s\n" \
		               "  eventType\n" \
		               "    whitelist     : %s\n" \
		               "    blacklist     : %s\n" % (
		               self._serveDataSelect, self._serveEvent,
		               self._serveStation, self._listenAddress, self._port,
		               self._connections, self._htpasswd, self._accessLogFile,
		               self._queryObjects, self._realtimeGap, self._samplesM,
		               self._allowRestricted, self._hideAuthor, modeStr,
		               whitelistStr, blacklistStr))

		if not self._serveDataSelect and not self._serveEvent and \
		   not self._serveStation:
			Logging.error("all services disabled through configuration")
			return False

		# access logger if requested
		if self._accessLogFile:
			self._accessLog = Log(self._accessLogFile)

		# load inventory needed by DataSelect and Station service
		if self._serveDataSelect or self._serveStation:
			self._loadInventory()

		DataModel.PublicObject.SetRegistrationEnabled(False)

		shareDir = os.path.join(Environment.Instance().shareDir(), 'fdsnws')

		# Overwrite/set mime type of *.wadl and *.xml documents. Instead of
		# using the official types defined in /etc/mime.types 'application/xml'
		# is used as enforced by the FDSNWS spec.
		static.File.contentTypes['.wadl'] = 'application/xml'
		static.File.contentTypes['.xml'] = 'application/xml'

		# create resource tree /fdsnws/...
		root = ListingResource()

		fileName = os.path.join(shareDir, 'favicon.ico')
		fileRes = static.File(fileName, 'image/x-icon')
		fileRes.childNotFound = NoResource()
		fileRes.isLeaf = True
		root.putChild('favicon.ico', fileRes)

		prefix = ListingResource()
		root.putChild('fdsnws', prefix)

		# right now service version is shared by all services
		serviceVersion = ServiceVersion()

		# dataselect
		if self._serveDataSelect:
			dataselect = ListingResource()
			prefix.putChild('dataselect', dataselect)
			dataselect1 = DirectoryResource(os.path.join(shareDir, 'dataselect.html'))
			dataselect.putChild('1', dataselect1)

			dataselect1.putChild('query', FDSNDataSelect())
			msg = 'authorization for restricted time series data required'
			authSession = self._getAuthSessionWrapper(FDSNDataSelectRealm(), msg)
			dataselect1.putChild('queryauth', authSession)
			dataselect1.putChild('version', serviceVersion)
			fileRes = static.File(os.path.join(shareDir, 'dataselect.wadl'))
			fileRes.childNotFound = NoResource()
			dataselect1.putChild('application.wadl', fileRes)

		# event
		if self._serveEvent:
			event = ListingResource()
			prefix.putChild('event', event)
			event1 = DirectoryResource(os.path.join(shareDir, 'event.html'))
			event.putChild('1', event1)

			event1.putChild('query', FDSNEvent(self._hideAuthor,
			                                   self._evaluationMode,
			                                   self._eventTypeWhitelist,
			                                   self._eventTypeBlacklist))
			fileRes = static.File(os.path.join(shareDir, 'catalogs.xml'))
			fileRes.childNotFound = NoResource()
			event1.putChild('catalogs', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'contributors.xml'))
			fileRes.childNotFound = NoResource()
			event1.putChild('contributors', fileRes)
			event1.putChild('version', serviceVersion)
			fileRes = static.File(os.path.join(shareDir, 'event.wadl'))
			fileRes.childNotFound = NoResource()
			event1.putChild('application.wadl', fileRes)

		# station
		if self._serveStation:
			station = ListingResource()
			prefix.putChild('station', station)
			station1 = DirectoryResource(os.path.join(shareDir, 'station.html'))
			station.putChild('1', station1)

			station1.putChild('query', FDSNStation(self._inv, self._allowRestricted, self._queryObjects))
			station1.putChild('version', serviceVersion)
			fileRes = static.File(os.path.join(shareDir, 'station.wadl'))
			fileRes.childNotFound = NoResource()
			station1.putChild('application.wadl', fileRes)

		retn = False
		try:
			# start listen for incoming request
			reactor.listenTCP(self._port, Site(root), self._connections,
			                  self._listenAddress)

			# start processing
			Logging.info("start listening")
			log.addObserver(logSC3)

			reactor.run()
			retn = True
		except Exception, e:
			Logging.error(str(e))

		return retn


	#---------------------------------------------------------------------------
	def _loadInventory(self):
		Logging.notice("loading inventory")
		dbr = DataModel.DatabaseReader(self.database())
		self._inv = DataModel.Inventory()

		# Load networks and stations
		staCount = 0
		for i in xrange(dbr.loadNetworks(self._inv)):
			staCount += dbr.load(self._inv.network(i))
		Logging.debug("loaded %i stations from %i networks" % (
		              staCount, self._inv.networkCount()))

		# Load sensors, skip calibrations (not needed by StationXML exporter)
		Logging.debug("loaded %i sensors" % dbr.loadSensors(self._inv))

		# Load datalogger and its decimations, skip calibrations (not needed by
		# StationXML exporter)
		deciCount = 0
		for i in xrange(dbr.loadDataloggers(self._inv)):
			deciCount += dbr.loadDecimations(self._inv.datalogger(i))
		Logging.debug("loaded %i decimations from %i dataloggers" % (
		              deciCount, self._inv.dataloggerCount()))

		# Load responses
		resPAZCount = dbr.loadResponsePAZs(self._inv)
		resFIRCount = dbr.loadResponseFIRs(self._inv)
		resPolCount = dbr.loadResponsePolynomials(self._inv)
		resCount = resPAZCount + resFIRCount + resPolCount
		Logging.debug("loaded %i responses (PAZ: %i, FIR: %i, Poly: %i)" % (
		              resCount, resPAZCount, resFIRCount, resPolCount))
		Logging.info("inventory loaded")


	#---------------------------------------------------------------------------
	def _getAuthSessionWrapper(self, realm, msg):
		checker = checkers.FilePasswordDB(self._htpasswd)
		p = portal.Portal(realm, [checker])
		f = guard.DigestCredentialFactory('md5', msg)
		f.digest = BugfixedDigest('md5', msg)
		return guard.HTTPAuthSessionWrapper(p, [f])



app = FDSNWS()
sys.exit(app())
