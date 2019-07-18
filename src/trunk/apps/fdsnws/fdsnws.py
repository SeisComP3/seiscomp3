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


import os, sys, time, fnmatch, base64, signal, re

try:
	from twisted.cred import checkers, credentials, error, portal
	from twisted.internet import reactor, defer, task
	from twisted.web import guard, resource, server, static
	from twisted.python import log, failure
	from zope.interface import implements
except ImportError, e:
	sys.exit("%s\nIs python twisted installed?" % str(e))

try:
	from seiscomp3 import Core, DataModel, IO, Logging
	from seiscomp3.Client import Application, Inventory
	from seiscomp3.System import Environment
except ImportError, e:
	sys.exit("%s\nIs the SeisComP environment set correctly?" % str(e))

from seiscomp3.fdsnws import utils
from seiscomp3.fdsnws.dataselect import FDSNDataSelect, FDSNDataSelectRealm, FDSNDataSelectAuthRealm
from seiscomp3.fdsnws.dataselect import VERSION as DataSelectVersion
from seiscomp3.fdsnws.event import FDSNEvent
from seiscomp3.fdsnws.event import VERSION as EventVersion
from seiscomp3.fdsnws.station import FDSNStation
from seiscomp3.fdsnws.station import VERSION as StationVersion
from seiscomp3.fdsnws.availability import AvailabilityExtent, AvailabilityQuery
from seiscomp3.fdsnws.availability import VERSION as AvailabilityVersion
from seiscomp3.fdsnws.http import DirectoryResource, ListingResource, NoResource, \
                                  Site, ServiceVersion, AuthResource, WADLFilter
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
# Make CORS work with queryauth
class HTTPAuthSessionWrapper(guard.HTTPAuthSessionWrapper):
	def __init__(self, *args, **kwargs):
		guard.HTTPAuthSessionWrapper.__init__(self, *args, **kwargs)

	def render(self, request):
		if request.method == 'OPTIONS':
			request.setHeader('Allow', 'GET,HEAD,POST,OPTIONS')
			return ''

		else:
			return guard.HTTPAuthSessionWrapper.render(self, request)


################################################################################
class UsernamePasswordChecker(object):
	implements(checkers.ICredentialsChecker)

	credentialInterfaces = (credentials.IUsernamePassword,
				credentials.IUsernameHashedPassword)

	#---------------------------------------------------------------------------
	def __init__(self, userdb):
		self.__userdb = userdb

	#---------------------------------------------------------------------------
	def __cbPasswordMatch(self, matched, username):
		if matched:
			return username
		else:
			return failure.Failure(error.UnauthorizedLogin())

	#---------------------------------------------------------------------------
	def requestAvatarId(self, credentials):
		return defer.maybeDeferred(self.__userdb.checkPassword, credentials) \
			.addCallback(self.__cbPasswordMatch, str(credentials.username))


################################################################################
class UserDB(object):

	#---------------------------------------------------------------------------
	def __init__(self):
		self.__users = {}
		self.__blacklist = set()
		task.LoopingCall(self.__expireUsers).start(60, False)

	#---------------------------------------------------------------------------
	def __expireUsers(self):
		for (name, (password, attributes, expires)) in self.__users.items():
			if time.time() > expires:
				Logging.info("de-registering %s" % name)
				del self.__users[name]

	#---------------------------------------------------------------------------
	def blacklistUser(self, name):
		Logging.info("blacklisting %s" % name)
		self.__blacklist.add(name)

	#---------------------------------------------------------------------------
	def addUser(self, name, attributes, expires, data):
		try:
			password = self.__users[name][0]

		except KeyError:
			bl = " (blacklisted)" if name in self.__blacklist else ""
			Logging.notice("registering %s%s %s" % (name, bl, data))
			password = base64.urlsafe_b64encode(os.urandom(12))

		attributes['blacklisted'] = name in self.__blacklist
		self.__users[name] = (password, attributes, expires)
		return password

	#---------------------------------------------------------------------------
	def checkPassword(self, credentials):
		try:
			pw = self.__users[str(credentials.username)][0]

		except KeyError:
			return False

		return credentials.checkPassword(pw)

	#---------------------------------------------------------------------------
	def getAttributes(self, name):
		return self.__users[name][1]

	#---------------------------------------------------------------------------
	def dump(self):
		Logging.info("known users:")

		for name, user in self.__users.items():
			Logging.info(" %s %s %d" % (name, user[1], user[2]))


################################################################################
class Access(object):

	#---------------------------------------------------------------------------
	def __init__(self):
		self.__access = {}

	#---------------------------------------------------------------------------
	def initFromSC3Routing(self, routing):
		for i in xrange(routing.accessCount()):
			acc = routing.access(i)
			net = acc.networkCode()
			sta = acc.stationCode()
			loc = acc.locationCode()
			cha = acc.streamCode()
			user = acc.user()
			start = acc.start()

			try:
				end = acc.end()

			except ValueError:
				end = None

			self.__access.setdefault((net, sta, loc, cha), []) \
				.append((user, start, end))

	#---------------------------------------------------------------------------
	def __matchTime(self, t1, t2, accessStart, accessEnd):
		return (not accessStart or (t1 and t1 >= accessStart)) and \
			(not accessEnd or (t2 and t2 <= accessEnd))

	#---------------------------------------------------------------------------
	def __matchEmail(self, emailAddress, accessUser):
		defaultPrefix = "mail:"

		if accessUser.startswith(defaultPrefix):
			accessUser = accessUser[len(defaultPrefix):]

		return (emailAddress.upper() == accessUser.upper() or \
				(accessUser[:1] == '@' and emailAddress[:1] != '@' and \
					emailAddress.upper().endswith(accessUser.upper())))

	#---------------------------------------------------------------------------
	def __matchAttribute(self, attribute, accessUser):
		return (attribute.upper() == accessUser.upper())

	#---------------------------------------------------------------------------
	def authorize(self, user, net, sta, loc, cha, t1, t2):
		if user['blacklisted']:
			return False

		matchers = []

		try:
			# OID 0.9.2342.19200300.100.1.3 (RFC 2798)
			emailAddress = user['mail']
			matchers.append((self.__matchEmail, emailAddress))

		except KeyError:
			pass

		try:
			# B2ACCESS
			for memberof in user['memberof'].split(';'):
				matchers.append((self.__matchAttribute, "group:" + memberof))

		except KeyError:
			pass

		for m in matchers:
			for (u, start, end) in self.__access.get((net, '', '', ''), []):
				if self.__matchTime(t1, t2, start, end) and m[0](m[1], u):
					return True

			for (u, start, end) in self.__access.get((net, sta, '', ''), []):
				if self.__matchTime(t1, t2, start, end) and m[0](m[1], u):
					return True

			for (u, start, end) in self.__access.get((net, sta, loc, cha), []):
				if self.__matchTime(t1, t2, start, end) and m[0](m[1], u):
					return True

		return False


################################################################################
class DataAvailabilityCache(object):

	#---------------------------------------------------------------------------
	def __init__(self, app, da, validUntil):
		self._da            = da
		self._validUntil    = validUntil
		self._extents       = {}
		self._extentsSorted = []
		self._extentsOID    = {}

		for i in xrange(self._da.dataExtentCount()):
			ext = self._da.dataExtent(i)
			wid = ext.waveformID()
			sid = "%s.%s.%s.%s" % (wid.networkCode(), wid.stationCode(),
			                       wid.locationCode(), wid.channelCode())
			restricted = app._openStreams is None or sid not in app._openStreams
			if restricted and not app._allowRestricted:
				continue
			self._extents[sid] = (ext, restricted)
			#Logging.debug("%s: %s ~ %s" % (sid, ext.start().iso(),
			#                               ext.end().iso()))

		if app._serveAvailability:
			# load data attribute extents if availability is served
			for i in xrange(da.dataExtentCount()):
				extent = da.dataExtent(i)
				app.query().loadDataAttributeExtents(extent)

			# create a list of (extent, oid, restricted) tuples sorted by stream
			self._extentsSorted = [ (e, app.query().getCachedId(e), res) \
			        for wid, (e, res) in sorted(self._extents.iteritems(),
			                                    key=lambda t: t[0]) ]

			# create a dictionary of object ID to extents
			self._extentsOID = dict((oid, (e, res)) \
			        for (e, oid, res) in self._extentsSorted)

		Logging.info("loaded %i extents" % len(self._extents))

	#---------------------------------------------------------------------------
	def validUntil(self):
		return self._validUntil

	#---------------------------------------------------------------------------
	def extent(self, net, sta, loc, cha):
		wid = "%s.%s.%s.%s" % (net, sta, loc, cha)
		if wid in self._extents:
			return self._extents[wid][0]

		return None

	#---------------------------------------------------------------------------
	def extents(self):
		return self._extents

	#---------------------------------------------------------------------------
	def extentsSorted(self):
		return self._extentsSorted

	#---------------------------------------------------------------------------
	def extentsOID(self):
		return self._extentsOID

	#---------------------------------------------------------------------------
	def dataAvailability(self):
		return self._da


################################################################################
class FDSNWS(Application):

	#---------------------------------------------------------------------------
	def __init__(self):
		Application.__init__(self, len(sys.argv), sys.argv)
		self.setMessagingEnabled(True)
		self.setDatabaseEnabled(True, True)
		self.setRecordStreamEnabled(True)
		self.setLoadInventoryEnabled(True)

		self._serverRoot     = os.path.dirname(__file__)
		self._listenAddress  = '0.0.0.0' # all interfaces
		self._port           = 8080
		self._connections    = 5
		self._queryObjects   = 100000    # maximum number of objects per query
		self._realtimeGap    = None      # minimum data age: 5min
		self._samplesM       = None      # maximum number of samples per query
		self._recordBulkSize = 102400    # desired record bulk size
		self._htpasswd       = '@CONFIGDIR@/fdsnws.htpasswd'
		self._accessLogFile  = ''
		self._requestLogFile  = ''

		self._allowRestricted   = True
		self._useArclinkAccess  = False
		self._serveDataSelect   = True
		self._serveEvent        = True
		self._serveStation      = True
		self._serveAvailability = False
		self._daEnabled         = False
		self._daCacheDuration   = 300
		self._daCache           = None
		self._openStreams       = None
		self._daRepositoryName  = 'primary'
		self._daDCCName         = 'DCC'

		self._hideAuthor            = False
		self._evaluationMode        = None
		self._eventTypeWhitelist    = None
		self._eventTypeBlacklist    = None
		self._eventFormats          = None
		self._stationFilter         = None
		self._dataSelectFilter      = None
		self._debugFilter           = False

		self._accessLog     = None

		self._fileNamePrefix = 'fdsnws'

		self._trackdbEnabled = False
		self._trackdbDefaultUser = 'fdsnws'

		self._authEnabled   = False
		self._authGnupgHome = '@ROOTDIR@/var/lib/gpg'
		self._authBlacklist = []

		self._userdb        = UserDB()
		self._access        = Access()

		# Leave signal handling to us
		Application.HandleSignals(False, False)


	#---------------------------------------------------------------------------
	def initConfiguration(self):
		if not Application.initConfiguration(self):
			return False

		# bind address and port
		try: self._listenAddress = self.configGetString('listenAddress')
		except Exception: pass
		try: self._port = self.configGetInt('port')
		except Exception: pass

		# maximum number of connections
		try: self._connections = self.configGetInt('connections')
		except Exception: pass

		# maximum number of objects per query, used in fdsnws-station and
		# fdsnws-event to limit main memory consumption
		try: self._queryObjects = self.configGetInt('queryObjects')
		except Exception: pass

		# restrict end time of request to now-realtimeGap seconds, used in
		# fdsnws-dataselect
		try: self._realtimeGap = self.configGetInt('realtimeGap')
		except Exception: pass

		# maximum number of samples (in units of million) per query, used in
		# fdsnws-dataselect to limit bandwidth
		try: self._samplesM = self.configGetDouble('samplesM')
		except Exception: pass

		try: self._recordBulkSize = self.configGetInt('recordBulkSize')
		except Exception: pass

		if self._recordBulkSize < 1:
			print >> sys.stderr, "Invalid recordBulkSize, must be larger than 0"
			return False

		# location of htpasswd file
		try:
			self._htpasswd = self.configGetString('htpasswd')
		except Exception: pass
		self._htpasswd = Environment.Instance().absolutePath(self._htpasswd)

		# location of access log file
		try:
			self._accessLogFile = Environment.Instance().absolutePath(
			                      self.configGetString('accessLog'))
		except Exception: pass

		# location of request log file
		try:
			self._requestLogFile = Environment.Instance().absolutePath(
			                       self.configGetString('requestLog'))
		except Exception: pass

		# access to restricted inventory information
		try: self._allowRestricted = self.configGetBool('allowRestricted')
		except Exception: pass

		# use arclink-access bindings
		try: self._useArclinkAccess = self.configGetBool('useArclinkAccess')
		except Exception: pass

		# services to enable
		try: self._serveDataSelect = self.configGetBool('serveDataSelect')
		except Exception: pass
		try: self._serveEvent = self.configGetBool('serveEvent')
		except Exception: pass
		try: self._serveStation = self.configGetBool('serveStation')
		except Exception: pass
		try: self._serveAvailability = self.configGetBool('serveAvailability')
		except Exception: pass

		# data availability
		try: self._daEnabled = self.configGetBool('dataAvailability.enable')
		except Exception: pass
		try: self._daCacheDuration = self.configGetInt('dataAvailability.cacheDuration')
		except Exception: pass
		try: self._daRepositoryName = self.configGetString('dataAvailability.repositoryName')
		except Exception: pass
		try: self._daDCCName = self.configGetString('dataAvailability.dccName')
		except Exception: pass

		if self._serveAvailability and not self._daEnabled:
			print >> sys.stderr, "can't serve availabilty without " \
			                     "dataAvailability.enable set to true"
			return False
		if not bool(re.match(r'^[a-zA-Z0-9_\ -]*$', self._daRepositoryName)):
			print >> sys.stderr, "invalid characters in dataAvailability.repositoryName"
			return False
		if not bool(re.match(r'^[a-zA-Z0-9_\ -]*$', self._daDCCName)):
			print >> sys.stderr, "invalid characters in dataAvailability.dccName"
			return False

		# event filter
		try: self._hideAuthor = self.configGetBool('hideAuthor')
		except Exception: pass
		try:
			name = self.configGetString('evaluationMode')
			if name.lower() == DataModel.EEvaluationModeNames.name(DataModel.MANUAL):
				self._evaluationMode = DataModel.MANUAL
			elif name.lower() == DataModel.EEvaluationModeNames.name(DataModel.AUTOMATIC):
				self._evaluationMode = DataModel.AUTOMATIC
			else:
				print >> sys.stderr, "invalid evaluation mode string: %s" % name
				return False
		except Exception: pass
		try:
			strings = self.configGetStrings('eventType.whitelist')
			if len(strings) > 1 or len(strings[0]):
				self._eventTypeWhitelist = [ s.lower() for s in strings ]
		except Exception: pass
		try:
			strings = self.configGetStrings('eventType.blacklist')
			if len(strings) > 0 or len(strings[0]):
				self._eventTypeBlacklist = [ s.lower() for s in strings ]
		except Exception: pass
		try:
			strings = self.configGetStrings('eventFormats')
			if len(strings) > 1 or len(strings[0]):
				self._eventFormats = [ s.lower() for s in strings ]
		except Exception: pass

		# station filter
		try: self._stationFilter = Environment.Instance().absolutePath(self.configGetString('stationFilter'))
		except Exception: pass

		# dataSelect filter
		try: self._dataSelectFilter = Environment.Instance().absolutePath(self.configGetString('dataSelectFilter'))
		except Exception: pass

		# output filter debug information
		try: self._debugFilter = self.configGetBool('debugFilter')
		except Exception: pass

		# prefix to be used as default for output filenames
		try: self._fileNamePrefix = self.configGetString('fileNamePrefix')
		except Exception: pass

		# save request logs in database?
		try: self._trackdbEnabled = self.configGetBool('trackdb.enable')
		except Exception: pass

		# default user
		try: self._trackdbDefaultUser = self.configGetString('trackdb.defaultUser')
		except Exception: pass

		# enable authentication extension?
		try: self._authEnabled = self.configGetBool('auth.enable')
		except Exception: pass

		# GnuPG home directory
		try: self._authGnupgHome = self.configGetString('auth.gnupgHome')
		except Exception: pass
		self._authGnupgHome = Environment.Instance().absolutePath(self._authGnupgHome)

		# blacklist of users/tokens
		try:
			strings = self.configGetStrings('auth.blacklist')
			if len(strings) > 1 or len(strings[0]):
				self._authBlacklist = strings
		except Exception: pass

		# If the database connection is passed via command line or configuration
		# file then messaging is disabled. Messaging is only used to get
		# the configured database connection URI.
		if self.databaseURI() != "":
			self.setMessagingEnabled(self._trackdbEnabled)
		else:
			# Without the event service, a database connection is not
			# required if the inventory is loaded from file and no data
			# availability information should be processed
			if not self._serveEvent and not self._useArclinkAccess and \
			   ( not self._serveStation or ( \
			     not self.isInventoryDatabaseEnabled() and not self._daEnabled ) ):
				self.setMessagingEnabled(self._trackdbEnabled)
				self.setDatabaseEnabled(False, False)

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
	def getDACache(self):
		if not self._daEnabled:
			return None

		now = Core.Time.GMT()
		# check if cache is still valid
		if self._daCache is None or now > self._daCache.validUntil():

			if self.query() is None or \
			   not self.query().driver().isConnected():
				dbInt = IO.DatabaseInterface.Open(self.databaseURI())
				if dbInt is None:
					Logging.error('failed to connect to database')
					return self._daCache
				else:
					self.setDatabase(dbInt)

			da = DataModel.DataAvailability()
			self.query().loadDataExtents(da)
			validUntil = now + Core.TimeSpan(self._daCacheDuration, 0)
			self._daCache = DataAvailabilityCache(self, da, validUntil)

		return self._daCache


	#---------------------------------------------------------------------------
	def _site(self):
		modeStr = None
		if self._evaluationMode is not None:
			modeStr = DataModel.EEvaluationModeNames.name(self._evaluationMode)
		whitelistStr = "<None>"
		if self._eventTypeWhitelist is not None:
			whitelistStr = ", ".join(self._eventTypeWhitelist)
		blacklistStr = "<None>"
		if self._eventTypeBlacklist is not None:
			blacklistStr = ", ".join(self._eventTypeBlacklist)
		stationFilterStr = "<None>"
		if self._stationFilter is not None:
			stationFilterStr = self._stationFilter
		dataSelectFilterStr = "<None>"
		if self._dataSelectFilter is not None:
			dataSelectFilterStr = self._dataSelectFilter
		Logging.debug("\n" \
		               "configuration read:\n" \
		               "  serve\n" \
		               "    dataselect    : %s\n" \
		               "    event         : %s\n" \
		               "    station       : %s\n" \
		               "    availability  : %s\n" \
		               "  listenAddress   : %s\n" \
		               "  port            : %i\n" \
		               "  connections     : %i\n" \
		               "  htpasswd        : %s\n" \
		               "  accessLog       : %s\n" \
		               "  queryObjects    : %i\n" \
		               "  realtimeGap     : %s\n" \
		               "  samples (M)     : %s\n" \
		               "  recordBulkSize  : %i\n" \
		               "  allowRestricted : %s\n" \
		               "  useArclinkAccess: %s\n" \
		               "  hideAuthor      : %s\n" \
		               "  evaluationMode  : %s\n" \
		               "  data availability\n" \
		               "    enabled       : %s\n" \
		               "    cache duration: %i\n" \
		               "    repo name     : %s\n" \
		               "    dcc name      : %s\n" \
		               "  eventType\n" \
		               "    whitelist     : %s\n" \
		               "    blacklist     : %s\n" \
		               "  inventory filter\n" \
		               "    station       : %s\n" \
		               "    dataSelect    : %s\n" \
		               "    debug enabled : %s\n" \
		               "  trackdb\n" \
		               "    enabled       : %s\n" \
		               "    defaultUser   : %s\n" \
		               "  auth\n" \
		               "    enabled       : %s\n" \
		               "    gnupgHome     : %s\n" \
		               "  requestLog      : %s\n" % (
		               self._serveDataSelect, self._serveEvent,
		               self._serveStation, self._serveAvailability,
		               self._listenAddress, self._port, self._connections,
		               self._htpasswd, self._accessLogFile, self._queryObjects,
		               self._realtimeGap, self._samplesM, self._recordBulkSize,
		               self._allowRestricted, self._useArclinkAccess,
		               self._hideAuthor, modeStr, self._daEnabled,
		               self._daCacheDuration, self._daRepositoryName,
		               self._daDCCName, whitelistStr, blacklistStr,
		               stationFilterStr, dataSelectFilterStr,
		               self._debugFilter, self._trackdbEnabled,
		               self._trackdbDefaultUser, self._authEnabled,
		               self._authGnupgHome, self._requestLogFile))

		if not self._serveDataSelect and not self._serveEvent and \
		   not self._serveStation:
			Logging.error("all services disabled through configuration")
			return None

		# access logger if requested
		if self._accessLogFile:
			self._accessLog = Log(self._accessLogFile)

		# request logger if requested
		if self._requestLogFile:
			# import here, so we don't depend on GeoIP if request log is not needed
			from seiscomp3.fdsnws.reqlog import RequestLog
			self._requestLog = RequestLog(self._requestLogFile)

		# load inventory needed by DataSelect and Station service
		stationInv = dataSelectInv = None
		if self._serveDataSelect or self._serveStation:
			retn = False
			stationInv = dataSelectInv = Inventory.Instance().inventory()
			Logging.info("inventory loaded")

			if self._serveDataSelect and self._serveStation:
				# clone inventory if station and dataSelect filter are distinct
				# else share inventory between both services
				if self._stationFilter != self._dataSelectFilter:
					dataSelectInv = self._cloneInventory(stationInv)
					retn = self._filterInventory(stationInv, self._stationFilter, "station") and \
					       self._filterInventory(dataSelectInv, self._dataSelectFilter, "dataSelect")
				else:
					retn = self._filterInventory(stationInv, self._stationFilter)
			elif self._serveStation:
				retn = self._filterInventory(stationInv, self._stationFilter)
			else:
				retn = self._filterInventory(dataSelectInv, self._dataSelectFilter)

			if not retn:
				return None

		self._access = Access()

		if self._serveDataSelect and self._useArclinkAccess:
			self._access.initFromSC3Routing(self.query().loadRouting())

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

		# dataselect
		if self._serveDataSelect:
			dataselect = ListingResource(DataSelectVersion)
			prefix.putChild('dataselect', dataselect)
			lstFile = os.path.join(shareDir, 'dataselect.html')
			dataselect1 = DirectoryResource(lstFile, DataSelectVersion)
			dataselect.putChild('1', dataselect1)

			dataselect1.putChild('query', FDSNDataSelect(dataSelectInv, self._recordBulkSize))
			msg = 'authorization for restricted time series data required'
			authSession = self._getAuthSessionWrapper(dataSelectInv, msg)
			dataselect1.putChild('queryauth', authSession)
			dataselect1.putChild('version', ServiceVersion(DataSelectVersion))
			fileRes = static.File(os.path.join(shareDir, 'dataselect.wadl'))
			fileRes.childNotFound = NoResource(DataSelectVersion)
			dataselect1.putChild('application.wadl', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'dataselect-builder.html'))
			fileRes.childNotFound = NoResource(DataSelectVersion)
			dataselect1.putChild('builder', fileRes)

			if self._authEnabled:
				dataselect1.putChild('auth', AuthResource(
				    DataSelectVersion, self._authGnupgHome, self._userdb))

		# event
		if self._serveEvent:
			event = ListingResource(EventVersion)
			prefix.putChild('event', event)
			lstFile = os.path.join(shareDir, 'event.html')
			event1 = DirectoryResource(lstFile, EventVersion)
			event.putChild('1', event1)

			event1.putChild('query', FDSNEvent(self._hideAuthor,
			                                   self._evaluationMode,
			                                   self._eventTypeWhitelist,
			                                   self._eventTypeBlacklist,
			                                   self._eventFormats))
			fileRes = static.File(os.path.join(shareDir, 'catalogs.xml'))
			fileRes.childNotFound = NoResource(EventVersion)
			event1.putChild('catalogs', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'contributors.xml'))
			fileRes.childNotFound = NoResource(EventVersion)
			event1.putChild('contributors', fileRes)
			event1.putChild('version', ServiceVersion(EventVersion))
			fileRes = static.File(os.path.join(shareDir, 'event.wadl'))
			fileRes.childNotFound = NoResource(EventVersion)
			event1.putChild('application.wadl', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'event-builder.html'))
			fileRes.childNotFound = NoResource(EventVersion)
			event1.putChild('builder', fileRes)

		# station
		if self._serveStation:
			station = ListingResource(StationVersion)
			prefix.putChild('station', station)
			lstFile = os.path.join(shareDir, 'station.html')
			station1 = DirectoryResource(lstFile, StationVersion)
			station.putChild('1', station1)

			station1.putChild('query', FDSNStation(stationInv,
			                                       self._allowRestricted,
			                                       self._queryObjects,
			                                       self._daEnabled))
			station1.putChild('version', ServiceVersion(StationVersion))

			# wadl, optionally filtered
			filterList = [] if self._daEnabled else [ 'name="matchtimeseries"' ]
			try:
				fileRes = WADLFilter(os.path.join(shareDir, 'station.wadl'),
				                     filterList)
			except:
				fileRes = NoResource(StationVersion)
			station1.putChild('application.wadl', fileRes)

			fileRes = static.File(os.path.join(shareDir, 'station-builder.html'))
			fileRes.childNotFound = NoResource(StationVersion)
			station1.putChild('builder', fileRes)

		# availability
		if self._serveAvailability:

			# create a set of waveformIDs which represent open channels
			if self._serveDataSelect:
				openStreams = set()
				for iNet in xrange(dataSelectInv.networkCount()):
					net = dataSelectInv.network(iNet)
					if utils.isRestricted(net): continue
					for iSta in xrange(net.stationCount()):
						sta = net.station(iSta)
						if utils.isRestricted(sta): continue
						for iLoc in xrange(sta.sensorLocationCount()):
							loc = sta.sensorLocation(iLoc)
							for iCha in xrange(loc.streamCount()):
								cha = loc.stream(iCha)
								if utils.isRestricted(cha): continue
								openStreams.add("{0}.{1}.{2}.{3}".format(
								                net.code(), sta.code(),
								                loc.code(), cha.code()))
				self._openStreams = openStreams
			else:
				self._openStreams = None

			ext = ListingResource()
			prefix.putChild('ext', ext)
			availability = ListingResource(AvailabilityVersion)
			ext.putChild('availability', availability)
			lstFile = os.path.join(shareDir, 'availability.html')
			availability1 = DirectoryResource(lstFile, AvailabilityVersion)
			availability.putChild('1', availability1)

			availability1.putChild('extent', AvailabilityExtent())
			availability1.putChild('query', AvailabilityQuery())
			availability1.putChild('version', ServiceVersion(AvailabilityVersion))
			fileRes = static.File(os.path.join(shareDir, 'station.wadl'))
			fileRes.childNotFound = NoResource(AvailabilityVersion)
			availability1.putChild('availability.wadl', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'availability-extent-builder.html'))
			fileRes.childNotFound = NoResource(AvailabilityVersion)
			availability1.putChild('builder-extent', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'availability-builder.html'))
			fileRes.childNotFound = NoResource(AvailabilityVersion)
			availability1.putChild('builder', fileRes)


		# static files
		fileRes = static.File(os.path.join(shareDir, 'js'))
		fileRes.childNotFound = NoResource()
		fileRes.hideInListing = True
		prefix.putChild('js', fileRes)

		fileRes = static.File(os.path.join(shareDir, 'css'))
		fileRes.childNotFound = NoResource()
		fileRes.hideInListing = True
		prefix.putChild('css', fileRes)

		return Site(root)


	#---------------------------------------------------------------------------
	def _reloadTask(self):
		if not self.__sighup:
			return

		self.__sighup = False

		Logging.info("reloading inventory")
		self.reloadInventory()

		site = self._site()

		if site:
			self.__tcpPort.factory = site
			Logging.info("reload successful")

		else:
			Logging.info("reload failed")

		self._userdb.dump()


	#---------------------------------------------------------------------------
	def _sighupHandler(self, signum, frame):
		Logging.info("SIGHUP received")
		self.__sighup = True


	#---------------------------------------------------------------------------
	def run(self):
		retn = False
		try:
			for user in self._authBlacklist:
				self._userdb.blacklistUser(user)

			site = self._site()

			if not site:
				return False

			# start listen for incoming request
			self.__tcpPort = reactor.listenTCP(self._port,
			                                   site,
	 		                                   self._connections,
			                                   self._listenAddress)

			# setup signal handler
			self.__sighup = False
			signal.signal(signal.SIGHUP, self._sighupHandler)
			task.LoopingCall(self._reloadTask).start(60, False)

			# start processing
			Logging.info("start listening")
			log.addObserver(logSC3)

			reactor.run()
			retn = True
		except Exception, e:
			Logging.error(str(e))

		return retn


	#---------------------------------------------------------------------------
	def _cloneInventory(self, inv):
		wasEnabled = DataModel.PublicObject.IsRegistrationEnabled()
		DataModel.PublicObject.SetRegistrationEnabled(False)
		inv2 = DataModel.Inventory.Cast(inv.clone())

		for iNet in xrange(inv.networkCount()):
			net = inv.network(iNet)
			net2 = DataModel.Network.Cast(net.clone())
			inv2.add(net2)

			for iSta in xrange(net.stationCount()):
				sta = net.station(iSta)
				sta2 = DataModel.Station.Cast(sta.clone())
				net2.add(sta2)

				for iLoc in xrange(sta.sensorLocationCount()):
					loc = sta.sensorLocation(iLoc)
					loc2 = DataModel.SensorLocation.Cast(loc.clone())
					sta2.add(loc2)

					for iCha in xrange(loc.streamCount()):
						cha = loc.stream(iCha)
						cha2 = DataModel.Stream.Cast(cha.clone())
						loc2.add(cha2)

		DataModel.PublicObject.SetRegistrationEnabled(wasEnabled)
		return inv2


	#---------------------------------------------------------------------------
	def _filterInventory(self, inv, fileName, serviceName=""):
		if not fileName:
			return True

		class FilterRule:
			def __init__(self, name, code):
				self.name       = name
				self.exclude    = name.startswith("!")
				self.code       = code

				self.restricted = None
				self.shared     = None
				self.netClass   = None
				self.archive    = None

		# read filter configuration from INI file
		filter = []
		includeRuleDefined = False
		try:
			import ConfigParser
		except ImportError, ie:
			Logging.error("could not load 'ConfigParser' Python module")
			return False

		try:
			cp = ConfigParser.ConfigParser()
			Logging.notice("reading inventory filter file: %s" % fileName)
			cp.readfp(open(fileName, 'r'))
			if len(cp.sections()) == 0:
				return True

			# check for mandatory code attribute
			for sectionName in cp.sections():
				code = ""
				try:
					code = cp.get(sectionName, "code")
				except:
					Logging.error("missing 'code' attribute in section %s of " \
					              "inventory filter file %s" % (
					              sectionName, fileName))
					return False

				rule = FilterRule(sectionName, str(code))

				try:
					rule.restricted = cp.getboolean(sectionName, 'restricted')
				except: pass

				try:
					rule.shared = cp.getboolean(sectionName, 'shared')
				except: pass

				try:
					rule.netClass = str(cp.get(sectionName, 'netClass'))
				except: pass

				try:
					rule.archive = str(cp.get(sectionName, 'archive'))
				except: pass

				includeRuleDefined |= not rule.exclude
				filter.append(rule)

		except Exception, e:
			Logging.error("could not read inventory filter file %s: %s" % (
			              fileName, str(e)))
			return False

		# apply filter
		# networks
		if self._debugFilter:
			debugLines = []
		delNet = delSta = delLoc = delCha = 0
		iNet = 0
		while iNet < inv.networkCount():
			net = inv.network(iNet)

			try: netRestricted = net.restricted()
			except ValueError: netRestricted = None
			try: netShared = net.shared()
			except ValueError: netShared = None

			# stations
			iSta = 0
			while iSta < net.stationCount():
				sta = net.station(iSta)
				staCode = "%s.%s" % (net.code(), sta.code())

				try: staRestricted = sta.restricted()
				except ValueError: staRestricted = None
				try: staShared = sta.shared()
				except ValueError: staShared = None

				# sensor locations
				iLoc = 0
				while iLoc < sta.sensorLocationCount():
					loc = sta.sensorLocation(iLoc)
					locCode = "%s.%s" % (staCode, loc.code())

					# channels
					iCha = 0
					while iCha < loc.streamCount():
						cha = loc.stream(iCha)
						code = "%s.%s" % (locCode, cha.code())

						# evaluate rules until matching code is found
						match = False
						for rule in filter:
							# code
							if not fnmatch.fnmatchcase(code, rule.code):
								continue

							# restricted
							if rule.restricted is not None:
								try:
									if cha.restricted() != rule.restricted:
										continue
								except ValueError:
									if staRestricted is not None:
										if sta.Restricted != rule.Restricted:
											continue
									elif netRestricted is None or \
									        netRestricted != rule.Restricted:
										continue

							# shared
							if rule.shared is not None:
								try:
									if cha.shared() != rule.shared:
										continue
								except ValueError:
									if staShared is not None:
										if sta.Shared != rule.Shared:
											continue
									elif netShared is None or \
									     netShared != rule.Shared:
										continue

							# netClass
							if rule.netClass is not None and \
							   net.netClass() != rule.netClass:
								continue

							# archive
							if rule.archive is not None and \
							   net.archive() != rule.archive:
								continue

							# the rule matched
							match = True
							break

						if (match and rule.exclude) or \
						   (not match and includeRuleDefined):
							loc.removeStream(iCha)
							delCha += 1
							reason = "no matching include rule"
							if match:
								reason = "'%s'" % rule.name
							if self._debugFilter:
								debugLines.append("%s [-]: %s" % (code, reason))
						else:
							iCha += 1
							reason = "no matching exclude rule"
							if match:
								reason = "'%s'" % rule.name
							if self._debugFilter:
								debugLines.append("%s [+]: %s" % (code, reason))

					# remove empty sensor locations
					if loc.streamCount() == 0:
						sta.removeSensorLocation(iLoc)
						delLoc += 1
					else:
						iLoc += 1

				# remove empty stations
				if sta.sensorLocationCount() == 0:
					delSta += 1
					net.removeStation(iSta)
				else:
					iSta += 1

			# remove empty networks
			if net.stationCount() == 0:
				delNet += 1
				inv.removeNetwork(iNet)
			else:
				iNet += 1

		if serviceName:
			serviceName += ": "
		Logging.debug("%sremoved %i networks, %i stations, %i locations, "
		              "%i streams" % (serviceName, delNet, delSta, delLoc,
		               delCha))
		if self._debugFilter:
			debugLines.sort()
			Logging.notice("%sfilter decisions based on file %s:\n%s" % (
			               serviceName, fileName, str("\n".join(debugLines))))

		return True


	#---------------------------------------------------------------------------
	def _getAuthSessionWrapper(self, inv, msg):
		if self._useArclinkAccess:
			access = self._access

		else:
			access = None

		if self._authEnabled:  # auth extension
			access = self._access  # requires useArclinkAccess for security reasons
			realm = FDSNDataSelectAuthRealm(inv, self._recordBulkSize, access, self._userdb)
			checker = UsernamePasswordChecker(self._userdb)

		else:  # htpasswd
			realm = FDSNDataSelectRealm(inv, self._recordBulkSize, access)
			checker = checkers.FilePasswordDB(self._htpasswd)

		p = portal.Portal(realm, [checker])
		f = guard.DigestCredentialFactory('MD5', msg)
		f.digest = BugfixedDigest('MD5', msg)
		return HTTPAuthSessionWrapper(p, [f])



app = FDSNWS()
sys.exit(app())


# vim: ts=4 noet
