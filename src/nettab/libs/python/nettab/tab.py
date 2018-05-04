from .lineType import Nw, Sg, Sr, Sl, Sa, Na, Dl, Se, Ff, Pz, Ia, Cl
from .nodesi import Instruments
from .nodesnslc import Network, StationGroup, DontFit
from seiscomp3 import DataModel, IO, Client
from .stationResolver import StationResolver
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
		
		print("Starting tab2inv version %s" % self.version(), file=sys.stderr)
		
		if not filterFolder:
			print(" Warning, not filter folder supplied.", file=sys.stderr)
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
			print(" Parsing defaults file: %s" % (filename), file=sys.stderr)
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
					print(" Ignored line", line, file=sys.stderr)
			fd.close()
		except Exception as e:
			print(" Warning: %s" % e, file=sys.stderr)
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
		except Exception as e:
			raise "Cannot load database driver: %s"
		
		dbDriver = IO.DatabaseInterface.Create(db["dbDriverName"])
		if dbDriver is None:
			raise Exception("Cannot find database driver " + db["dbDriverName"])

		if not dbDriver.connect(db["dbAddress"]):
			raise Exception("Cannot connect to database at " + db["dbAddress"])

		dbQuery = DataModel.DatabaseQuery(dbDriver)
		if dbQuery is None:
			raise Exception("Cannot get DB query object")

		print(" Loading inventory from database ... ", end=' ', file=sys.stderr)
		inventory = DataModel.Inventory()
		dbQuery.loadNetworks(inventory)
		for ni in range(inventory.networkCount()):
			dbQuery.loadStations(inventory.network(ni))
		print("Done.", file=sys.stderr)
		if inventory:
			self.stationResolver.collectStations(inventory, True)

	def _loadXml(self, folder):
		print(" Loading inventory from XML file ... ", end=' ', file=sys.stderr)
		for f in glob.glob(os.path.join(folder, "*.xml")):
			ar = IO.XMLArchive()
			ar.open(f)
			inventory = DataModel.Inventory_Cast(ar.readObject())
			ar.close()

			if inventory:
				self.stationResolver.collectStations(inventory)
		print("Done.", file=sys.stderr)

	def digest(self, tabFilename):
		sas = []
		ias = []
		nw = None
		
		n = None
		g = None
		print(" Parsing file: %s" % (tabFilename), file=sys.stderr)

		if not tabFilename or not os.path.isfile(tabFilename):
			raise Exception("Supplied filename is invalid.")
		
		if tabFilename in list(self.n.keys()) or tabFilename in list(self.g.keys()):
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
					except Exception as e:
						raise Exception("Error while creating nw from '%s': %s" % (Content, e))
					try:
						for na in self.nas:	nw.Na(na) # Defaults
					except Exception as e:
						raise Exception("Error while loading (defaults) %s into %s: %s" % (na, nw, e))

				elif Type == "Sg":
					if n or g:
						raise Exception("Network or Station Group already defined, only one Hr line should be defined per file.")

					try:
						sg = Sg(Content)
					except Exception as e:
						raise Exception("Error while creating sg from '%s': %s" % (Content, e))
					try:
						for na in self.nas:	sg.Na(na) # Defaults
					except Exception as e:
						raise Exception("Error while loading (defaults) %s into %s: %s" % (na, sg, e))

				elif Type == "Na":
					if not nw and not sg:
						raise Exception("No network defined, no Na line before a Hr line.")
					if n or g:
						raise Exception("No Na lines after a Sl line. Network has already been defined.")
					try:
						na = Na(Content)
					except Exception as e:
						raise Exception("Error while creating na from '%s': %s" % (Content, e))
					if nw:
						try:
							nw.Na(na)
						except Exception as e:
							raise Exception("Error while adding %s to %s: %s" % (na, nw, e))
					else:
						try:
							sg.Na(na)
						except Exception as e:
							raise Exception("Error while adding %s to %s: %s" % (na, sg, e))


				elif Type == "Sa":
					if not nw:
						raise Exception("Not Sa line before a hr line allowed.")
					try:
						sas.append(Sa(Content))
					except Exception as e:
						raise Exception("Error while creating Sa from '%s': %s" % (Content,e))

				elif Type == "Sl":
					if not n:
						if not nw:
							raise Exception("No network defined, Hr line should come before station line.")
						else:
							n = Network(nw)
							for (filename, network) in self.n.items():
								if network.conflict(n):
									raise Exception("Network already defined %s (%s)-(%s) by file %s." % (network.code, network.start, network.end, filename))
					try:
						sl = Sl(Content)
					except Exception as e:
						raise Exception("Error while creating sl from '%s': %s" % (Content, e))
					# Fill in attributes
					try:
						for sa in self.sas: sl.Sa(sa) # Defaults
					except Exception as e:
						raise Exception("Error while loading (default) %s into %s: %s" % (sa, sl, e))
					try:
						for sa in sas: sl.Sa(sa) # Collected
					except Exception as e:
						raise Exception("Error while loading %s into %s: %s" % (str(sa), str(sl), e))
					# Digest by Station
					try:
						n.Sl(sl)
					except DontFit:
						raise Exception("%s does not fit in %s" % (sl, n))
					except Exception as e:
						raise Exception("Error while loading %s into %s: %s" % (sl, n, e))

				elif Type == "Sr":
					if not g:
						if not sg:
							raise Exception("No station group defined, Sg line should come before station reference line.")
						else:
							g = StationGroup(sg)
							for (filename, stationGroup) in self.g.items():
								if stationGroup.conflict(g):
									raise Exception("Station group already defined %s (%s)-(%s) by file %s." % (stationGroup.code, stationGroup.start, stationGroup.end, filename))
							for (filename, network) in self.n.items():
								if network.conflict(g):
									raise Exception("Station group conflict network already defined %s (%s)-(%s) by file %s." % (network.code, network.start, network.end, filename))

					try:
						sr = Sr(Content)
					except Exception as e:
						raise Exception("Error while creating sr from '%s': %s" % (Content, e))
					# Digest by Station Reference
					try:
						g.Sr(sr)
					except DontFit:
						raise Exception("%s does not fit in %s" % (sr, n))
					except Exception as e:
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
					print(" Ignored line", line, file=sys.stderr)
	
				## Process Instrument
				if obj:
					try:
						for ia in self.ias: obj.Ia(ia) # Defaults
					except Exception as e:
						raise Exception("Error while loading (defaults) %s into %s: %s" % (ia, obj, e)) 
					try:
						for ia in ias: obj.Ia(ia) # Collected
					except Exception as e:
						raise Exception("Error while loading %s into %s: %s" % (ia, obj, e)) 
					try:
						self.i.add(obj)
					except Exception as e:
						raise Exception("Error while loading %s into Instruments db: %s" % (obj, e))
					obj = None
	
			# Process Network
			if n:
				self.n[tabFilename] = n

			# Process Station Group
			if g:
				self.g[tabFilename] = g

		except Exception as e:
			raise e

		finally:
			if fd:
				fd.close()
	
	def check(self):
		# Instrument alone check
		if self.i.keys:
			print("\nCheking Instruments Loaded:\n", file=sys.stderr)
			error = self.i.check(self.n)
			if error:
				for e in error: print(e, file=sys.stderr)
		else:
			print("\nNo instruments loaded", file=sys.stderr)

		# Cross Check
		error = []
		if self.n:
			print("\nChecking Networks Loaded:\n", file=sys.stderr)
			for network in self.n.values():
				error.extend(network.check(self.i))
			if error:
				for e in error: print(e, file=sys.stderr)
		else:
			print("\nNo network/stations loaded.", file=sys.stderr)
	
	def sc3Obj(self, sc3i = None):
		if not sc3i:
			sc3i = DataModel.Inventory()
			
		for network in list(self.n.values()):
			sc3n = network.sc3Obj(self.i)
			sc3i.add(sc3n)
		
		for sc3o in self.i.sc3Objs():
			sc3i.add(sc3o)
		
		self.stationResolver.collectStations(sc3i)

		for stationGroup in list(self.g.values()):
			sc3g = stationGroup.sc3Obj(self.stationResolver)
			sc3i.add(sc3g)
		
		return sc3i
