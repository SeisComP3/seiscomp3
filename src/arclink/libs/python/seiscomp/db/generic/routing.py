# This file was created by a source code generator:
# genxml2wrap.py 
# Do not modify. Change the definition and
# run the generator again!
#
# (c) 2010 Mathias Hoffmann, GFZ Potsdam
#
#

from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

#import genwrap as _genwrap
import datetime
from seiscomp.db.xmlio import routing as _xmlio
from seiscomp.db import DBError
#
#


# ---------------------------------------------------------------------------------------
class _RouteArclink(object):
	__slots__ = (
		"myRoute",
		"object",
		"address",
		"start",
		"end",
		"priority",
		"last_modified",
	)

	def __init__(self, myRoute, address, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.address = ""
		self.start = None
		self.end = None
		self.priority = None
		self.myRoute = myRoute
		self.object = {}

		for (a, v) in args.items():
			self.__setattr__(a, v)

		self.address = address
		self.start = start


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _RouteSeedlink(object):
	__slots__ = (
		"myRoute",
		"object",
		"address",
		"priority",
		"last_modified",
	)

	def __init__(self, myRoute, address, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.address = ""
		self.priority = None
		self.myRoute = myRoute
		self.object = {}

		for (a, v) in args.items():
			self.__setattr__(a, v)

		self.address = address


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Route(object):
	__slots__ = (
		"my",
		"object",
		"publicID",
		"networkCode",
		"stationCode",
		"locationCode",
		"streamCode",
		"last_modified",
		"arclink",
		"seedlink",
	)

	def __init__(self, my, networkCode, stationCode, locationCode, streamCode, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.networkCode = ""
		self.stationCode = ""
		self.locationCode = ""
		self.streamCode = ""
		self.my = my
		self.object = {}

		for (a, v) in args.items():
			self.__setattr__(a, v)

		self.networkCode = networkCode
		self.stationCode = stationCode
		self.locationCode = locationCode
		self.streamCode = streamCode

		self.arclink = {}
		self.seedlink = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_arclink(self, address, start, **args):
		if address not in self.arclink:
			self.arclink[address] = {}
		if start in self.arclink[address]:
			raise DBError("RouteArclink [%s][%s] already defined" % (address, start))
		obj = _RouteArclink(self, address, start, args)
		self.arclink[address][start] = obj
		return obj

	def remove_arclink(self, address, start):
		try:
			del self.arclink[address][start]
			if len(self.arclink[address]) == 0:
				del self.arclink[address]
		except KeyError:
			raise DBError("RouteArclink [%s][%s] not found" % (address, start))

	def insert_seedlink(self, address, **args):
		if address in self.seedlink:
			raise DBError("RouteSeedlink %s already defined" % address)
		obj = _RouteSeedlink(self, address, args)
		self.seedlink[address] = obj
		return obj

	def remove_seedlink(self, address):
		try:
			del self.seedlink[address]
		except KeyError:
			raise DBError("RouteSeedlink [%s] not found" % (address))
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class _Access(object):
	__slots__ = (
		"my",
		"object",
		"networkCode",
		"stationCode",
		"locationCode",
		"streamCode",
		"user",
		"start",
		"end",
		"last_modified",
	)

	def __init__(self, my, networkCode, stationCode, locationCode, streamCode, user, start, args):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.networkCode = ""
		self.stationCode = ""
		self.locationCode = ""
		self.streamCode = ""
		self.user = ""
		self.start = None
		self.end = None
		self.my = my
		self.object = {}

		for (a, v) in args.items():
			self.__setattr__(a, v)

		self.networkCode = networkCode
		self.stationCode = stationCode
		self.locationCode = locationCode
		self.streamCode = streamCode
		self.user = user
		self.start = start


	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())
# ---------------------------------------------------------------------------------------




# ---------------------------------------------------------------------------------------
class Routing(object):
	__slots__ = (
		"object",
		"publicID",
		"last_modified",
		"route",
		"access",
	)

	def __init__(self):
		self.last_modified = datetime.datetime(1970, 1, 1, 0, 0, 0)
		self.publicID = ""
		self.object = {}


		self.route = {}
		self.access = {}

	def __setattr__(self, name, value):
		object.__setattr__(self, name, value)
		object.__setattr__(self, "last_modified", datetime.datetime.utcnow())

	def insert_route(self, networkCode, stationCode, locationCode, streamCode, **args):
		if networkCode not in self.route:
			self.route[networkCode] = {}
		if stationCode not in self.route[networkCode]:
			self.route[networkCode][stationCode] = {}
		if locationCode not in self.route[networkCode][stationCode]:
			self.route[networkCode][stationCode][locationCode] = {}
		if streamCode in self.route[networkCode][stationCode][locationCode]:
			raise DBError("Route [%s][%s][%s][%s] already defined" % (networkCode, stationCode, locationCode, streamCode))
		obj = _Route(self, networkCode, stationCode, locationCode, streamCode, args)
		self.route[networkCode][stationCode][locationCode][streamCode] = obj
		self.object[obj.publicID] = obj
		return obj

	def remove_route(self, networkCode, stationCode, locationCode, streamCode):
		try:
			del self.route[networkCode][stationCode][locationCode][streamCode]
			if len(self.route[networkCode][stationCode][locationCode]) == 0:
				del self.route[networkCode][stationCode][locationCode]
			if len(self.route[networkCode][stationCode]) == 0:
				del self.route[networkCode][stationCode]
			if len(self.route[networkCode]) == 0:
				del self.route[networkCode]
		except KeyError:
			raise DBError("Route [%s][%s][%s][%s] not found" % (networkCode, stationCode, locationCode, streamCode))

	def insert_access(self, networkCode, stationCode, locationCode, streamCode, user, start, **args):
		if networkCode not in self.access:
			self.access[networkCode] = {}
		if stationCode not in self.access[networkCode]:
			self.access[networkCode][stationCode] = {}
		if locationCode not in self.access[networkCode][stationCode]:
			self.access[networkCode][stationCode][locationCode] = {}
		if streamCode not in self.access[networkCode][stationCode][locationCode]:
			self.access[networkCode][stationCode][locationCode][streamCode] = {}
		if user not in self.access[networkCode][stationCode][locationCode][streamCode]:
			self.access[networkCode][stationCode][locationCode][streamCode][user] = {}
		if start in self.access[networkCode][stationCode][locationCode][streamCode][user]:
			raise DBError("Access [%s][%s][%s][%s][%s][%s] already defined" % (networkCode, stationCode, locationCode, streamCode, user, start))
		obj = _Access(self, networkCode, stationCode, locationCode, streamCode, user, start, args)
		self.access[networkCode][stationCode][locationCode][streamCode][user][start] = obj
		return obj

	def remove_access(self, networkCode, stationCode, locationCode, streamCode, user, start):
		try:
			del self.access[networkCode][stationCode][locationCode][streamCode][user][start]
			if len(self.access[networkCode][stationCode][locationCode][streamCode][user]) == 0:
				del self.access[networkCode][stationCode][locationCode][streamCode][user]
			if len(self.access[networkCode][stationCode][locationCode][streamCode]) == 0:
				del self.access[networkCode][stationCode][locationCode][streamCode]
			if len(self.access[networkCode][stationCode][locationCode]) == 0:
				del self.access[networkCode][stationCode][locationCode]
			if len(self.access[networkCode][stationCode]) == 0:
				del self.access[networkCode][stationCode]
			if len(self.access[networkCode]) == 0:
				del self.access[networkCode]
		except KeyError:
			raise DBError("Access [%s][%s][%s][%s][%s][%s] not found" % (networkCode, stationCode, locationCode, streamCode, user, start))

	def clear_routes(self):
		self.route = {}

	def clear_access(self):
		self.access = {}
	
	def load_xml(self, src, use_access=False):
		_xmlio.xml_in(self, src, use_access)

	def save_xml(self, dest, use_access=False, modified_after=None, stylesheet=None):
		_xmlio.xml_out(self, dest, use_access, modified_after, stylesheet)

	def make_parser(self):
		return _xmlio.make_parser(self)

# ---------------------------------------------------------------------------------------





