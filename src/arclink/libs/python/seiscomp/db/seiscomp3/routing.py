#***************************************************************************** 
# routing.py
#
# seiscomp3-based Routing implementation
#
# (c) 2006 Andres Heinloo, GFZ Potsdam
# (c) 2007 Mathias Hoffmann, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import re
import datetime
from seiscomp.db.seiscomp3 import sc3wrap as _sc3wrap
from seiscomp.db.xmlio import routing as _xmlio
from seiscomp.db import DBError
from seiscomp import logs

#
# arclink's  r o u t i n g  representation
#
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class _Arclink(_sc3wrap.base_routearclink):
	def __init__(self, so):
		_sc3wrap.base_routearclink.__init__(self, so)
		self.myroute = None

	def __cmp__(self, other):
		if(self.priority < other.priority):
			return -1
	
		if(self.priority > other.priority):
			return 1

		return 0
	
	def flush(self):
		self._sync_update()

class _Seedlink(_sc3wrap.base_routeseedlink):
	def __init__(self, so):
		_sc3wrap.base_routeseedlink.__init__(self, so)
		self.myroute = None

	def __cmp__(self, other):
		if(self.priority < other.priority):
			return -1
	
		if(self.priority > other.priority):
			return 1

		return 0
	
	def flush(self):
		self._sync_update()

class _Route(_sc3wrap.base_route):
	def __init__(self, so):
		_sc3wrap.base_route.__init__(self, so)
		self.arclink = {}
		self.seedlink = {}

	def _link_arclink(self, obj):
		if obj.address not in self.arclink:
			self.arclink[obj.address] = {}

		self.arclink[obj.address][obj.start] = obj
		obj.myroute = self

	def _link_seedlink(self, obj):
		self.seedlink[obj.address] = obj
		obj.myroute = self
	
	def insert_arclink(self, address, start, **args):
		obj = _Arclink(self._new_routearclink(address=address, start=start,
			**args))
		self._link_arclink(obj)
		return obj
	
	def insert_seedlink(self, address, **args):
		obj = _Seedlink(self._new_routeseedlink(address=address, **args))
		self._link_seedlink(obj)
		return obj

	def remove_arclink(self, address, start):
		try:
			self.arclink[address][start]._delete()
			del self.arclink[address][start]

			if len(self.arclink[address]) == 0:
				del self.arclink[address]

		except KeyError:
			raise DBError, "ArcLink route (%s,%s) not found" % \
				(address, start)
	
	def remove_seedlink(self, address):
		try:
			self.seedlink[address]._delete()
			del self.seedlink[address]

		except KeyError:
			raise DBError, "SeedLink route %s not found" % (address,)
	
	def flush(self):
		self._sync_update()
		
		for i in self.arclink.itervalues():
			for j in i.itervalues():
				j.flush()

		for i in self.seedlink.itervalues():
			i.flush()

class _Access(_sc3wrap.base_access):
	def __init__(self, so):
		_sc3wrap.base_access.__init__(self, so)

	def flush(self):
		self._sync_update()
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#
# conditional functions
#
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
def _retr(val):
	if val is None:
		val = '*'
	s = re.sub("^[?]$", "*", val)       # single ? --> *
	s = re.sub("[?]", ".", s)           # SN?? --> SN..
	s = re.sub("[+]", ".+", s)          # + --> .+
	s = re.sub("[\*]", ".*", "^"+s+"$") # S*AA --> S.*AA
	return s

def _modifiedAfter(obj, modified_after):
	if (obj.last_modified is None) or (modified_after is None):
		return True
	return (obj.last_modified >= modified_after)

def _inPattern(code, pat):
	#logs.info("_inPattern: %s %s" % (str(code), str(_retr(pat))))
	return re.match(_retr(pat), code)
	
def _inTimeSpan(obj, start_time, end_time):
	start = True
	end = True
	if (obj.start is not None) and (end_time is not None):
		start = (obj.start <= end_time)
	if (obj.end is not None) and (start_time is not None):
		end = (obj.end >= start_time)
	return (start and end)
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



#
# arclink's  r o u t i n g  implementation
#
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class Routing(_sc3wrap.base_routing):
	def __init__(self, sc3Routing):
		_sc3wrap.base_routing.__init__(self, sc3Routing)
		self.route = {}
		self.access = {}
		self.__route_so = {}
		
	def _link_route(self, obj):
		if obj.networkCode not in self.route:
			self.route[obj.networkCode] = {}

		if obj.stationCode not in self.route[obj.networkCode]:
			self.route[obj.networkCode][obj.stationCode] = {}

		if obj.locationCode not in self.route[obj.networkCode][obj.stationCode]:
			self.route[obj.networkCode][obj.stationCode][obj.locationCode] = {}

		self.route[obj.networkCode][obj.stationCode][obj.locationCode][obj.streamCode] = obj
		
	def _link_access(self, obj):
		if obj.networkCode not in self.access:
			self.access[obj.networkCode] = {}

		if obj.stationCode not in self.access[obj.networkCode]:
			self.access[obj.networkCode][obj.stationCode] = {}

		if obj.locationCode not in self.access[obj.networkCode][obj.stationCode]:
			self.access[obj.networkCode][obj.stationCode][obj.locationCode] = {}

		if obj.streamCode not in self.access[obj.networkCode][obj.stationCode][obj.locationCode]:
			self.access[obj.networkCode][obj.stationCode][obj.locationCode][obj.streamCode] = {}

		if obj.user not in self.access[obj.networkCode][obj.stationCode][obj.locationCode][obj.streamCode]:
			self.access[obj.networkCode][obj.stationCode][obj.locationCode][obj.streamCode][obj.user] = {}

		self.access[obj.networkCode][obj.stationCode][obj.locationCode][obj.streamCode][obj.user][obj.start] = obj
	
	def insert_route(self, networkCode, stationCode, locationCode, streamCode, **args):
		obj = _Route(self._new_route(networkCode=networkCode, stationCode=stationCode,
			locationCode=locationCode, streamCode=streamCode, **args))
		self._link_route(obj)
		return obj

	def insert_access(self, networkCode, stationCode, locationCode, streamCode,
		user, start, **args):
		obj = _Access(self._new_access(networkCode=networkCode, stationCode=stationCode,
			locationCode=locationCode, streamCode=streamCode, user=user, start=start, **args))
		self._link_access(obj)
		return obj
	
	def remove_route(self, networkCode, stationCode, locationCode, streamCode):
		try:
			self.route[networkCode][stationCode][locationCode][streamCode]._delete()
			del self.route[networkCode][stationCode][locationCode][streamCode]

			if len(self.route[networkCode][stationCode][locationCode]) == 0:
				del self.route[networkCode][stationCode][locationCode]

			if len(self.route[networkCode][stationCode]) == 0:
				del self.route[networkCode][stationCode]

			if len(self.route[networkCode]) == 0:
				del self.route[networkCode]

		except KeyError:
			raise DBError, "route to stream (%s,%s,%s,%s) not found" % (networkCode, stationCode,
				locationCode, streamCode)
	
	def remove_access(self, networkCode, stationCode, locationCode, streamCode, user, start):
		try:
			self.access[networkCode][stationCode][locationCode][streamCode][user][start]._delete()
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
			raise DBError, "access rule (%s,%s,%s,%s,%s,%s) not found" % \
				(networkCode, stationCode, locationCode, streamCode, user, start)

	def flush(self):
		for i in self.route.itervalues():
			for j in i.itervalues():
				for k in j.itervalues():
					for l in k.itervalues():
						l.flush()

		for i in self.access.itervalues():
			for j in i.itervalues():
				for k in j.itervalues():
					for l in k.itervalues():
						for m in l.itervalues():
							for n in m.itervalues():
								n.flush()



	def load_routes(self, net_pat = None, sta_pat = None, cha_pat = None, loc_pat = None,
		start_time = None, end_time = None, modified_after = None):

		for item in (net_pat, sta_pat, cha_pat, loc_pat):
			if item is None or item.find("*") >= 0 or item.find("?") >= 0:
				# wildcards detected, use slow version
				route_iter = self._route
				break

		else:
			# no wildcards, trigger a hack to speed things up
			route_iter = []

			idx = set()
			for x in (15, 14, 13, 11, 7, 12, 10, 9, 6, 5, 3, 8, 4, 2, 1, 0):
				net = (net_pat, "")[not (x & 8)]
				sta = (sta_pat, "")[not (x & 4)]
				cha = (cha_pat, "")[not (x & 2)]
				loc = (loc_pat, "")[not (x & 1)]

				# note the different order of loc, cha
				idx.add((net, sta, loc, cha))

			for i in idx:
				obj = self.obj.route(_sc3wrap.DataModel.RouteIndex(*i))
				if obj:
					obj.lastModified = _sc3wrap.Core.Time.GMT()
					route_iter.append(_sc3wrap.base_route(obj))
		
		for route in route_iter:
			
#			if net_pat is None and sta_pat is None:
#				routeFound = True
#			else:
#				routeFound = False

			routeFound = True
			
			if not _modifiedAfter(route, modified_after):
				continue
			if not _inPattern(route.networkCode, net_pat):
				continue
			if route.stationCode != "" and not _inPattern(route.stationCode, sta_pat):
				continue
			
			try:
				R = self.route[route.networkCode][route.stationCode][route.locationCode][route.streamCode]
			except:
				R = _Route(route.obj)

			for routeArclink in R._routeArclink:
				if not _modifiedAfter(routeArclink, modified_after):
					continue
				if not _inTimeSpan(routeArclink, start_time, end_time):
					continue
				R._link_arclink(_Arclink(routeArclink.obj))
				routeFound = True
				
			for routeSeedlink in R._routeSeedlink:
				if not _modifiedAfter(routeSeedlink, modified_after):
					continue
				R._link_seedlink(_Seedlink(routeSeedlink.obj))
				routeFound = True
			
			if routeFound:
				self._link_route(R)

	def clear_routes(self):
		self.flush()
		self.route = {}




	def load_access(self, net_pat = None, sta_pat = None, str_pat = None,
		loc_pat = None, start_time = None, end_time = None, modified_after = None):

		for access in self._access:
			if not _modifiedAfter(access, modified_after):
				continue
			if not _inTimeSpan(access, start_time, end_time):
				continue
			
			if access.networkCode and not _inPattern(access.networkCode, net_pat):
				continue
			if access.stationCode and not _inPattern(access.stationCode, sta_pat):
				continue
			if access.streamCode and not _inPattern(access.streamCode, str_pat):
				continue
			if access.locationCode and not _inPattern(access.locationCode, loc_pat):
				continue
			
			A = _Access(access.obj)
			self._link_access(A)

	
	def clear_access(self):
		self.flush()
		self.access = {}
	
	
	
	
	def load_xml(self, src, use_access=False):
		_xmlio.xml_in(self, src, use_access)

	def save_xml(self, dest, use_access=False, modified_after=None, stylesheet=None):
		_xmlio.xml_out(self, dest, use_access, modified_after, stylesheet)

	def make_parser(self):
		return _xmlio.make_parser(self)
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
