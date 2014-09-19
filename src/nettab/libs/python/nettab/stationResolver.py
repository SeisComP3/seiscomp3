import time, datetime

def _cmptime(t1, t2):
	if t1 is None and t2 is None:
		return 0
	elif t2 is None or (t1 is not None and t1 < t2):
		return -1
	elif t1 is None or (t2 is not None and t1 > t2):
		return 1
	return 0

def _time2datetime(t):
	result = datetime.datetime(*time.strptime(t.toString("%Y-%m-%dT%H:%M:00Z"), "%Y-%m-%dT%H:%M:%SZ")[0:6])
	result += datetime.timedelta(microseconds=float(t.toString("%S.%f")) * 1000000)

class StationResolver(object):
	def __init__(self):
		self.stationMap = {}
		self.initialStations = set()

	def collectStations(self, inventory, initial = False):
		for ni in xrange(inventory.networkCount()):
			n = inventory.network(ni)
			for si in xrange(n.stationCount()):
				s = n.station(si)

				try:
					if initial:
						self.initialStations.add((n.code(), s.code()))

					else:
						self.initialStations.remove((n.code(), s.code()))
						del self.stationMap[(n.code(), s.code())]

				except KeyError:
					pass

				try:
					item = self.stationMap[(n.code(), s.code())]

				except KeyError:
					item = []
					self.stationMap[(n.code(), s.code())] = item
				
				start = _time2datetime(s.start())
				try: end = _time2datetime(s.end())
				except: end = None

				item.append((start, end, s.publicID()))

	def resolveStation(self, ncode, scode, start, end):
		result = set()
		try:
			for (s, e, publicID) in self.stationMap[(ncode, scode)]:
				if _cmptime(start, e) <= 0 and _cmptime(end, s) >= 0:
					result.add(publicID)

		except KeyError:
			pass

		if not result:
			raise Exception("Station reference %s,%s cannot be resolved" % (ncode, scode))

		return result

