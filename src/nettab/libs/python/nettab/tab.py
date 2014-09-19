from lineType import Nw, Sg, Sr, Sl, Sa, Na, Dl, Se, Ff, Pz, Ia, Cl
from nodesi import Instruments
from nodesnslc import Network, StationGroup, DontFit
from seiscomp3 import DataModel, IO, Client
from stationResolver import StationResolver
import sys
import os
import glob
import re

__VERSION__ = "0.1"

class Tab(object):
	def version(self):
		return __VERSION__

	def __init__(self, instrumentPrefix = None, defaultsFile = None, filterFolder = None, xmlFolder = None, database = None):
		self.i = Instruments(instrumentPrefix)
		self.n = {}
		self.g = {}
		self.sas = []
		self.nas = []
		self.ias = []
		self.stationResolver = StationResolver()
		
		self._filterFolder = None
		
		print >>sys.stderr, "Starting tab2inv version %s" % self.version()
		
		if not filterFolder:
			print >>sys.stderr, " Warning, not filter folder supplied."
		else:  
			if not os.path.isdir(filterFolder):
				raise Exception("Filter folder does not exist.")

		self._filterFolder = filterFolder

		if defaultsFile is not None:
			self._defaults(defaultsFile)

		if database is not None:
			self._loadDatabase(database)
	
		if xmlFolder is not None:
			self._loadXml(xmlFolder)

	def _defaults(self, filename):
		sas = []
		ias = []
		nas = []
		try:
			fd = open(filename)
			print >> sys.stderr, " Parsing defaults file: %s" % (filename)
			for line in fd:
				line = line.strip()
				if not line or line[0] == "#": continue
				(Type, Content) = line.split(":",1)
				if Type == "Nw":
					raise Exception("Defaults file can only contain attributes")
				elif Type == "Na":
					nas.append(Na(Content))
				elif Type == "Sa":
					sas.append(Sa(Content))
				elif Type == "Sl":
					raise Exception("Defaults file can only contain attributes")
				elif Type == "Ia":
					ias.append(Ia(Content))
				elif Type == "Se":
					raise Exception("Defaults file can only contain attributes")
				elif Type == "Dl":
					raise Exception("Defaults file can only contain attributes")
				elif Type == "Cl":
					raise Exception("Defaults file can only contain attributes")
				elif Type == "Ff":
					raise Exception("Defaults file can only contain attributes")
				elif Type == "If":
					raise Exception("Defaults file can only contain attributes")
				elif Type == "Pz":
					raise Exception("Defaults file can only contain attributes")
				else:
					print >> sys.stderr," Ignored line", line
			fd.close()
		except Exception, e:
			print >> sys.stderr, " Warning: %s" % e
			pass
		
		self.sas = sas
		self.nas = nas
		self.ias = ias

	def _loadDatabase(self, dbUrl):
		m = re.match("(?P<dbDriverName>^.*):\/\/(?P<dbAddress>.+?:.+?@.+?\/.+$)", dbUrl)
		if not m:
			raise Exception("error in parsing SC3 DB url")
		
		db = m.groupdict()
		
		try:
			registry = Client.PluginRegistry.Instance()
			registry.addPluginName("dbmysql")
			registry.loadPlugins()
		except Exception,e:
			raise("Cannot load database driver: %s" % e)
		
		dbDriver = IO.DatabaseInterface.Create(db["dbDriverName"])
		if dbDriver is None:
			raise Exception("Cannot find database driver " + db["dbDriverName"])

		if not dbDriver.connect(db["dbAddress"]):
			raise Exception("Cannot connect to database at " + db["dbAddress"])

		dbQuery = DataModel.DatabaseQuery(dbDriver)
		if dbQuery is None:
			raise Exception("Cannot get DB query object")

		print >> sys.stderr," Loading inventory from database ... ",
		inventory = DataModel.Inventory()
		dbQuery.loadNetworks(inventory)
		for ni in xrange(inventory.networkCount()):
			dbQuery.loadStations(inventory.network(ni))
		print >> sys.stderr,"Done."
		if inventory:
			self.stationResolver.collectStations(inventory, True)

	def _loadXml(self, folder):
		print >> sys.stderr," Loading inventory from XML file ... ",
		for f in glob.glob(os.path.join(folder, "*.xml")):
			ar = IO.XMLArchive()
			ar.open(f)
			inventory = DataModel.Inventory_Cast(ar.readObject())
			ar.close()

			if inventory:
				self.stationResolver.collectStations(inventory)
		print >> sys.stderr, "Done."

	def digest(self, tabFilename):
		sas = []
		ias = []
		nw = None
		
		n = None
		g = None
		print >> sys.stderr," Parsing file: %s" % (tabFilename)

		if not tabFilename or not os.path.isfile(tabFilename):
			raise Exception("Supplied filename is invalid.")
		
		if tabFilename in self.n.keys() or tabFilename in self.g.keys():
			raise Exception("File %s is already digested." % tabFilename)
		filename = 1
		try:
			fd = open(tabFilename)
			for line in fd:
				obj = None
				line = line.strip()
				if not line or line[0] == "#": continue
				if str(line).find(":") == -1:
					raise Exception("Invalid line format '%s'" % line)
				(Type, Content) = line.split(":",1)

				if Type == "Nw":
					if n or g:
						raise Exception("Network or Station Group already defined, only one Hr line should be defined per file.")
					try:
						nw = Nw(Content)
					except Exception,e:
						raise Exception("Error while creating nw from '%s': %s" % (Content, e))
					try:
						for na in self.nas:	nw.Na(na) # Defaults
					except Exception,e:
						raise Exception("Error while loading (defaults) %s into %s: %s" % (na, nw, e))

				elif Type == "Sg":
					if n or g:
						raise Exception("Network or Station Group already defined, only one Hr line should be defined per file.")

					try:
						sg = Sg(Content)
					except Exception,e:
						raise Exception("Error while creating sg from '%s': %s" % (Content, e))
					try:
						for na in self.nas:	sg.Na(na) # Defaults
					except Exception,e:
						raise Exception("Error while loading (defaults) %s into %s: %s" % (na, sg, e))

				elif Type == "Na":
					if not nw and not sg:
						raise Exception("No network defined, no Na line before a Hr line.")
					if n or g:
						raise Exception("No Na lines after a Sl line. Network has already been defined.")
					try:
						na = Na(Content)
					except Exception,e:
						raise Exception("Error while creating na from '%s': %s" % (Content, e))
					if nw:
						try:
							nw.Na(na)
						except Exception,e:
							raise Exception("Error while adding %s to %s: %s" % (na, nw, e))
					else:
						try:
							sg.Na(na)
						except Exception,e:
							raise Exception("Error while adding %s to %s: %s" % (na, sg, e))


				elif Type == "Sa":
					if not nw:
						raise Exception("Not Sa line before a hr line allowed.")
					try:
						sas.append(Sa(Content))
					except Exception,e:
						raise Exception("Error while creating Sa from '%s': %s" % (Content,e))

				elif Type == "Sl":
					if not n:
						if not nw:
							raise Exception("No network defined, Hr line should come before station line.")
						else:
							n = Network(nw)
							for (filename, network) in self.n.iteritems():
								if network.conflict(n):
									raise Exception("Network already defined %s (%s)-(%s) by file %s." % (network.code, network.start, network.end, filename))
					try:
						sl = Sl(Content)
					except Exception,e:
						raise Exception("Error while creating sl from '%s': %s" % (Content, e))
					# Fill in attributes
					try:
						for sa in self.sas: sl.Sa(sa) # Defaults
					except Exception,e:
						raise Exception("Error while loading (default) %s into %s: %s" % (sa, sl, e))
					try:
						for sa in sas: sl.Sa(sa) # Collected
					except Exception,e:
						raise Exception("Error while loading %s into %s: %s" % (str(sa), str(sl), e))
					# Digest by Station
					try:
						n.Sl(sl)
					except DontFit:
						raise Exception("%s does not fit in %s" % (sl, n))
					except Exception,e:
						raise Exception("Error while loading %s into %s: %s" % (sl, n, e))

				elif Type == "Sr":
					if not g:
						if not sg:
							raise Exception("No station group defined, Sg line should come before station reference line.")
						else:
							g = StationGroup(sg)
							for (filename, stationGroup) in self.g.iteritems():
								if stationGroup.conflict(g):
									raise Exception("Station group already defined %s (%s)-(%s) by file %s." % (stationGroup.code, stationGroup.start, stationGroup.end, filename))
							for (filename, network) in self.n.iteritems():
								if network.conflict(g):
									raise Exception("Station group conflict network already defined %s (%s)-(%s) by file %s." % (network.code, network.start, network.end, filename))

					try:
						sr = Sr(Content)
					except Exception,e:
						raise Exception("Error while creating sr from '%s': %s" % (Content, e))
					# Digest by Station Reference
					try:
						g.Sr(sr)
					except DontFit:
						raise Exception("%s does not fit in %s" % (sr, n))
					except Exception,e:
						raise Exception("Error while loading %s into %s: %s" % (sr, n, e))

				elif Type == "Ia":
					ias.append(Ia(Content))

				elif Type == "Se":
					obj = Se(Content)

				elif Type == "Dl":
					obj = Dl(Content)

				elif Type == "Cl":
					obj = Cl(Content)

				elif Type == "Ff":
					obj = Ff(self._filterFolder, Content)

				elif Type == "If":
					obj = Pz(Content,'D')

				elif Type == "Pz":
					obj = Pz(Content,'A')
				else:
					print >> sys.stderr," Ignored line", line
	
				## Process Instrument
				if obj:
					try:
						for ia in self.ias: obj.Ia(ia) # Defaults
					except Exception,e:
						raise Exception("Error while loading (defaults) %s into %s: %s" % (ia, obj, e)) 
					try:
						for ia in ias: obj.Ia(ia) # Collected
					except Exception,e:
						raise Exception("Error while loading %s into %s: %s" % (ia, obj, e)) 
					try:
						self.i.add(obj)
					except Exception,e:
						raise Exception("Error while loading %s into Instruments db: %s" % (obj, e))
					obj = None
	
			# Process Network
			if n:
				self.n[tabFilename] = n

			# Process Station Group
			if g:
				self.g[tabFilename] = g

		except Exception,e:
			raise e

		finally:
			if fd:
				fd.close()
	
	def check(self):
		# Instrument alone check
		if self.i.keys:
			print >> sys.stderr,"\nCheking Instruments Loaded:\n"
			error = self.i.check(self.n)
			if error:
				for e in error: print >>sys.stderr,e
		else:
			print >> sys.stderr,"\nNo instruments loaded"

		# Cross Check
		error = []
		if self.n:
			print >> sys.stderr,"\nChecking Networks Loaded:\n"
			for network in self.n.itervalues():
				error.extend(network.check(self.i))
			if error:
				for e in error: print >> sys.stderr,e
		else:
			print >> sys.stderr,"\nNo network/stations loaded."
	
	def sc3Obj(self, sc3i = None):
		if not sc3i:
			sc3i = DataModel.Inventory()
			
		for network in self.n.values():
			sc3n = network.sc3Obj(self.i)
			sc3i.add(sc3n)
		
		for sc3o in self.i.sc3Objs():
			sc3i.add(sc3o)
		
		self.stationResolver.collectStations(sc3i)

		for stationGroup in self.g.values():
			sc3g = stationGroup.sc3Obj(self.stationResolver)
			sc3i.add(sc3g)
		
		return sc3i
