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


import os, sys, time, fnmatch, base64

try:
	from twisted.cred import checkers, credentials, error, portal
	from twisted.internet import reactor, defer, task
	from twisted.web import guard, resource, server, static
	from twisted.python import log, failure
	from zope.interface import implements
except ImportError, e:
	sys.exit("%s\nIs python twisted installed?" % str(e))

try:
	from seiscomp3 import Core, DataModel, Logging
	from seiscomp3.Client import Application, Inventory
	from seiscomp3.System import Environment
	from seiscomp3.Config import Exception as ConfigException
	from seiscomp3.Core import ValueException
except ImportError, e:
	sys.exit("%s\nIs the SeisComP environment set correctly?" % str(e))

from seiscomp3.fdsnws.dataselect import FDSNDataSelect, FDSNDataSelectRealm, FDSNDataSelectAuthRealm
from seiscomp3.fdsnws.event import FDSNEvent
from seiscomp3.fdsnws.station import FDSNStation
from seiscomp3.fdsnws.http import DirectoryResource, ListingResource, NoResource, \
                                  Site, ServiceVersion, AuthResource
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
		task.LoopingCall(self.__expireUsers).start(60, False)

	#---------------------------------------------------------------------------
	def __expireUsers(self):
		for (user, (password, attributes, expires)) in self.__users.items():
			if time.time() > expires:
				del self.__users[user]

	#---------------------------------------------------------------------------
	def addUser(self, name, attributes, expires):
		try:
			password = self.__users[name][0]

		except KeyError:
			password = base64.urlsafe_b64encode(os.urandom(12))

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

			except Core.ValueException:
				end = None

			self.__access.setdefault((net, sta, loc, cha), []) \
				.append((user, start, end))

	#---------------------------------------------------------------------------
	def __matchTime(self, t1, t2, accessStart, accessEnd):
		return (not accessStart or (t1 and t1 >= accessStart)) and \
			(not accessEnd or (t2 and t2 <= accessEnd))

	#---------------------------------------------------------------------------
	def __matchUser(self, emailAddress, accessUser):
		return (emailAddress.upper() == accessUser.upper() or \
				(accessUser[:1] == '@' and emailAddress[:1] != '@' and \
					emailAddress.upper().endswith(accessUser.upper())))

	#---------------------------------------------------------------------------
	def authorize(self, user, net, sta, loc, cha, t1, t2):
		try:
			# OID 0.9.2342.19200300.100.1.3 (RFC 2798)
			emailAddress = user['mail']

		except KeyError:
			return False

		for (u, start, end) in self.__access.get((net, '', '', ''), []):
			if self.__matchTime(t1, t2, start, end) and self.__matchUser(emailAddress, u):
				return True

		for (u, start, end) in self.__access.get((net, sta, '', ''), []):
			if self.__matchTime(t1, t2, start, end) and self.__matchUser(emailAddress, u):
				return True

		for (u, start, end) in self.__access.get((net, sta, loc, cha), []):
			if self.__matchTime(t1, t2, start, end) and self.__matchUser(emailAddress, u):
				return True

		return False


################################################################################
class FDSNWS(Application):

	#---------------------------------------------------------------------------
	def __init__(self):
		Application.__init__(self, len(sys.argv), sys.argv)
		self.setMessagingEnabled(True)
		self.setDatabaseEnabled(True, True)
		self.setRecordStreamEnabled(True)
		self.setLoadInventoryEnabled(True)

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
		self._stationFilter         = None
		self._dataSelectFilter      = None
		self._debugFilter           = False

		self._accessLog     = None

		self._fileNamePrefix = 'fdsnws'

		self._trackdbEnabled = False
		self._trackdbDefaultUser = 'fdsnws'

		self._authEnabled   = False
		self._authGnupgHome = '@ROOTDIR@/var/lib/gpg'

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
		except ConfigException: pass
		try: self._port = self.configGetInt('port')
		except ConfigException: pass

		# maximum number of connections
		try: self._connections = self.configGetInt('connections')
		except ConfigException: pass

		# maximum number of objects per query, used in fdsnws-station and
		# fdsnws-event to limit main memory consumption
		try: self._queryObjects = self.configGetInt('queryObjects')
		except ConfigException: pass

		# restrict end time of request to now-realtimeGap seconds, used in
		# fdsnws-dataselect
		try: self._realtimeGap = self.configGetInt('realtimeGap')
		except ConfigException: pass

		# maximum number of samples (in units of million) per query, used in
		# fdsnws-dataselect to limit bandwidth
		try: self._samplesM = self.configGetDouble('samplesM')
		except ConfigException: pass

		# location of htpasswd file
		try:
			self._htpasswd = self.configGetString('htpasswd')
		except ConfigException: pass
		self._htpasswd = Environment.Instance().absolutePath(self._htpasswd)

		# location of access log file
		try:
			self._accessLogFile = Environment.Instance().absolutePath(
			                      self.configGetString('accessLog'))
		except ConfigException: pass

		# access to restricted inventory information
		try: self._allowRestricted = self.configGetBool('allowRestricted')
		except: pass

		# services to enable
		try: self._serveDataSelect = self.configGetBool('serveDataSelect')
		except: pass
		try: self._serveEvent = self.configGetBool('serveEvent')
		except: pass
		try: self._serveStation = self.configGetBool('serveStation')
		except: pass

		# event filter
		try: self._hideAuthor = self.configGetBool('hideAuthor')
		except: pass
		try:
			name = self.configGetString('evaluationMode')
			if name.lower() == DataModel.EEvaluationModeNames.name(DataModel.MANUAL):
				self._evaluationMode = DataModel.MANUAL
			elif name.lower() == DataModel.EEvaluationModeNames.name(DataModel.AUTOMATIC):
				self._evaluationMode = DataModel.AUTOMATIC
			else:
				print >> sys.stderr, "invalid evaluation mode string: %s" % name
				return False
		except: pass
		try:
			strings = self.configGetStrings('eventType.whitelist')
			if len(strings) > 1 or len(strings[0]):
				self._eventTypeWhitelist = [ s.lower() for s in strings ]
		except: pass
		try:
			strings = self.configGetStrings('eventType.blacklist')
			if len(strings) > 0 or len(strings[0]):
				self._eventTypeBlacklist = [ s.lower() for s in strings ]
		except: pass

		# station filter
		try: self._stationFilter = Environment.Instance().absolutePath(self.configGetString('stationFilter'))
		except ConfigException: pass

		# dataSelect filter
		try: self._dataSelectFilter = Environment.Instance().absolutePath(self.configGetString('dataSelectFilter'))
		except ConfigException: pass

		# output filter debug information
		try: self._debugFilter = self.configGetBool('debugFilter')
		except ConfigException: pass

		# prefix to be used as default for output filenames
		try: self._fileNamePrefix = self.configGetString('fileNamePrefix')
		except ConfigException: pass

		# save request logs in database?
		try: self._trackdbEnabled = self.configGetBool('trackdb.enable')
		except ConfigException: pass

		# default user
		try: self._trackdbDefaultUser = self.configGetString('trackdb.defaultUser')
		except ConfigException: pass

		# enable authentication extension?
		try: self._authEnabled = self.configGetBool('auth.enable')
		except ConfigException: pass

		# GnuPG home directory
		try: self._authGnupgHome = self.configGetString('auth.gnupgHome')
		except ConfigException: pass
		self._authGnupgHome = Environment.Instance().absolutePath(self._authGnupgHome)

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
		               "    gnupgHome     : %s\n" % (
		               self._serveDataSelect, self._serveEvent,
		               self._serveStation, self._listenAddress, self._port,
		               self._connections, self._htpasswd, self._accessLogFile,
		               self._queryObjects, self._realtimeGap, self._samplesM,
		               self._allowRestricted, self._hideAuthor, modeStr,
		               whitelistStr, blacklistStr, stationFilterStr,
		               dataSelectFilterStr, self._debugFilter,
		               self._trackdbEnabled, self._trackdbDefaultUser,
		               self._authEnabled, self._authGnupgHome))

		if not self._serveDataSelect and not self._serveEvent and \
		   not self._serveStation:
			Logging.error("all services disabled through configuration")
			return False

		# access logger if requested
		if self._accessLogFile:
			self._accessLog = Log(self._accessLogFile)

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
				return False

		if self._serveDataSelect:
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

		# right now service version is shared by all services
		serviceVersion = ServiceVersion()

		# dataselect
		if self._serveDataSelect:
			dataselect = ListingResource()
			prefix.putChild('dataselect', dataselect)
			dataselect1 = DirectoryResource(os.path.join(shareDir, 'dataselect.html'))
			dataselect.putChild('1', dataselect1)

			dataselect1.putChild('query', FDSNDataSelect(dataSelectInv, self._access))
			msg = 'authorization for restricted time series data required'
			authSession = self._getAuthSessionWrapper(dataSelectInv, msg)
			dataselect1.putChild('queryauth', authSession)
			dataselect1.putChild('version', serviceVersion)
			fileRes = static.File(os.path.join(shareDir, 'dataselect.wadl'))
			fileRes.childNotFound = NoResource()
			dataselect1.putChild('application.wadl', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'dataselect-builder.html'))
			fileRes.childNotFound = NoResource()
			dataselect1.putChild('builder', fileRes)

			if self._authEnabled:
				dataselect1.putChild('auth', AuthResource(self._authGnupgHome,
									  self._userdb))

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
			fileRes = static.File(os.path.join(shareDir, 'event-builder.html'))
			fileRes.childNotFound = NoResource()
			event1.putChild('builder', fileRes)

		# station
		if self._serveStation:
			station = ListingResource()
			prefix.putChild('station', station)
			station1 = DirectoryResource(os.path.join(shareDir, 'station.html'))
			station.putChild('1', station1)

			station1.putChild('query', FDSNStation(stationInv,
			                                       self._allowRestricted,
			                                       self._queryObjects))
			station1.putChild('version', serviceVersion)
			fileRes = static.File(os.path.join(shareDir, 'station.wadl'))
			fileRes.childNotFound = NoResource()
			station1.putChild('application.wadl', fileRes)
			fileRes = static.File(os.path.join(shareDir, 'station-builder.html'))
			fileRes.childNotFound = NoResource()
			station1.putChild('builder', fileRes)


		# static files
		fileRes = static.File(os.path.join(shareDir, 'js'))
		fileRes.childNotFound = NoResource()
		fileRes.hideInListing = True
		prefix.putChild('js', fileRes)

		fileRes = static.File(os.path.join(shareDir, 'css'))
		fileRes.childNotFound = NoResource()
		fileRes.hideInListing = True
		prefix.putChild('css', fileRes)

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
			except ValueException: netRestricted = None
			try: netShared = net.shared()
			except ValueException: netShared = None

			# stations
			iSta = 0
			while iSta < net.stationCount():
				sta = net.station(iSta)
				staCode = "%s.%s" % (net.code(), sta.code())

				try: staRestricted = sta.restricted()
				except ValueException: staRestricted = None
				try: staShared = sta.shared()
				except ValueException: staShared = None

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
								except ValueException:
									if staRestricted != None:
										if sta.Restricted != rule.Restricted:
											continue
									elif netRestricted == None or \
									        netRestricted != rule.Restricted:
										continue

							# shared
							if rule.shared is not None:
								try:
									if cha.shared() != rule.shared:
										continue
								except ValueException:
									if staShared != None:
										if sta.Shared != rule.Shared:
											continue
									elif netShared == None or \
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
		if self._authEnabled:  # auth extension
			realm = FDSNDataSelectAuthRealm(inv, self._access, self._userdb)
			checker = UsernamePasswordChecker(self._userdb)

		else:  # htpasswd
			realm = FDSNDataSelectRealm(inv, self._access)
			checker = checkers.FilePasswordDB(self._htpasswd)

		p = portal.Portal(realm, [checker])
		f = guard.DigestCredentialFactory('MD5', msg)
		f.digest = BugfixedDigest('MD5', msg)
		return HTTPAuthSessionWrapper(p, [f])



app = FDSNWS()
sys.exit(app())


# vim: ts=4 noet
