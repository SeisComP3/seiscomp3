import sys
import datetime
from re import split
from time import *
from types import *
from seiscomp import logs
from seiscomp.arclink.manager import *

if sys.version_info < (2,4): from sets import Set as set

class BreqParser(object):
	"""
	Parses the breq_fast email format using regular expressions.
	
	@classvaribales: __tokenrule, defines the syntax of the beginning of a Breq_fast header token
					 __tokenlist, specifies the tokens required for the ArcLink request
					 __reqlist,   defines the syntax for a request line in Breq_fast format
	"""
	__tokenrule="^\.[A-Z_]+[:]?\s"
	
	__tokenlist=(
				 "\.NAME\s+(?P<name>.+)",
				 "\.INST\s+(?P<institution>.+)?",
				 "\.EMAIL\s+(?P<email>.+[@].+)",
				 "\.LABEL\s+(?P<label>.+)?")
	
	__reqlist=("(?P<station>[\w?\*]+)",
			"(?P<network>[\w?]+)",
			"((?P<beg_2year>\d{2})|(?P<beg_4year>\d{4}))",
			"(?P<beg_month>\d{1,2})",
			"(?P<beg_day>\d{1,2})",
			"(?P<beg_hour>\d{1,2})",
			"(?P<beg_min>\d{1,2})",
			"(?P<beg_sec>\d{1,2})(\.\d*)?",
			"((?P<end_2year>\d{2})|(?P<end_4year>\d{4}))",
			"(?P<end_month>\d{1,2})",
			"(?P<end_day>\d{1,2})",
			"(?P<end_hour>\d{1,2})",
			"(?P<end_min>\d{1,2})",
			"(?P<end_sec>\d{1,2})(\.\d*)?",
			"(?P<cha_num>\d+)",
			"(?P<cha_list>[\w?\s*]+)")
	
	def __init__(self):
		"""
		Constructor.

		@instancevariables: head,      a string concatenation of certain Breq_fast header lines
                                    request,   a string concatenation of the Breq_fast request lines
                                    tokendict, a dictionary storing the ArcLink required matches
                                    reqlist,   a list of tuples containing the matched requests
                                    failstr,   a string documenting failed matches
		"""
		self.head = ""
		self.request = ""
		self.tokendict = {}
		self.reqlist = []
		self.failstr = ""

		try:
			self.mgr = ArclinkManager("%s:%d"%(DEFAULT_HOST, DEFAULT_PORT), DEFAULT_USER)
		except:
			pass

	def __parse_token(self, head):
		"""
		Gets the Breq_fast header to match it against the corresponding pattern.
		If successul sets the dictionary for storing the ArcLink required matches.
		
		@arguments: head, a string storing the email request header in Breq_fast format
		"""
		l = re.compile( "|".join(self.__tokenlist) )
		self.tokendict = dict()
		
		for line in split("\n", head):
			m = l.search(line)
			if m:
				for k,v in m.groupdict().items():
					if v is not None: self.tokendict[k] = v
		
		if not "name" in self.tokendict or not "email" in self.tokendict:
			self.failstr = "%sBreq_fast header must contain at least .NAME and .EMAIL arguments.\n" % self.failstr 

	def __expand_net_station(self, network, station, beg_time, end_time):
		"""
		Mathias, 31.05.2007
		
		Expands a possibly wildcarded station code field by looking up in the inventory.
		Allowed wildcards: '?' and '*'
		
		@arguments: string network:       lookup for stations in this network (e.g. GE or G* or ?? or * or *E )
					string station:       one station entry, (e.g. SNAA or SN?? or ???? or * or SN* or ? )
					beg_time, end_time:   begin & end time: limit search for net/stations to selected time window
		
		@return: list of expanded (network, station) codes tuples
		"""
		net_station_list = []
		
		if re.search(r'[?\*]+', station):
			logs.debug("*** expanding %s for network: %s" % (station, network))

			# FIXME: begin=None, end=None
			db = self.mgr.get_inventory(network=re.sub("[?]+","*",network), station=re.sub("[?]+","*",station), begin=None, end=None) 
	
			netlist = []
			netlist = db.network.keys()
			logs.debug("- netlist: %s" % netlist)
			
			for net_code in netlist:
				for net in db.network[net_code].values():
					for sta_code in net.station:
						#logs.debug("- net/stacode: %s %s" % (net_code, sta_code))
						s = re.sub("^[?]$", "*", station)    # single ? --> *
						s = re.sub("[?]", ".", s)            # SN?? --> SN..
						s = re.sub("[\*]", ".*", "^"+s+"$")  # S*AA --> S.*AA
						#logs.debug("- station filter: %s" % s)
						if re.match(s, sta_code):
							net_station_list.append((net_code, sta_code))

		else:
			net_station_list.append((network, station))
		
		return net_station_list

	def __parse_request(self, line):
		"""
		Gets a request line in Breq_fast format to match it against the corresponding pattern.
		If successful the request list will be completed; the fail string otherwise.

		@arguments: line, a request line in Breq_fast format
		"""
		loc = "*"
		self.request = "%s\n%s" % (self.request, line)
		m = re.search("\s+".join(self.__reqlist),line)
		if m:
			d = m.groupdict()

			logs.debug("request_line: %s" % line)
				
			# catch two digit year inputs
			if d["beg_2year"]:
				if int(d["beg_2year"]) > 50:
					d["beg_4year"] = "19%s" % d["beg_2year"]
				else:
					d["beg_4year"] = "20%s" % d["beg_2year"]
			if d["end_2year"]:
				if int(d["end_2year"]) > 50:
					d["end_4year"] = "19%s" % d["end_2year"]
				else:
					d["end_4year"] = "20%s" % d["end_2year"]

			# some users have problems with time...
			if int(d["beg_hour"]) > 23:
				d["beg_hour"] = "23"
			if int(d["end_hour"]) > 23:
				d["end_hour"] = "23"
			if int(d["beg_min"]) > 59:
				d["beg_min"] = "59"
			if int(d["end_min"]) > 59:
				d["end_min"] = "59"
			if int(d["beg_sec"]) > 59:
				d["beg_sec"] = "59"
			if int(d["end_sec"]) > 59:
				d["end_sec"] = "59"
			
			try:
				beg_time = datetime.datetime(int(d["beg_4year"]),int(d["beg_month"]),int(d["beg_day"]),
											int(d["beg_hour"]),int(d["beg_min"]),int(d["beg_sec"]))
				end_time = datetime.datetime(int(d["end_4year"]),int(d["end_month"]),int(d["end_day"]),
											int(d["end_hour"]),int(d["end_min"]),int(d["end_sec"]))
			except ValueError as e:
				self.failstr = "%s%s [error: wrong begin or end time: %s]\n" % (self.failstr, line, e)
				return
			
			# expand network and station
			for (network, station) in self.__expand_net_station(d["network"], d["station"], beg_time, end_time):

				cha_list = re.findall("([\w?\*]+)\s*",d["cha_list"])
				if len(cha_list) == int(d['cha_num'])+1:
					loc = cha_list.pop()
				for cha in cha_list:
					cha = re.sub("[?]+","*",cha)
					self.reqlist.append(RequestLine(beg_time,end_time,str(network),str(station),cha,loc))
					logs.debug("reqlist.append: %s %s %s %s" % (str(network),str(station),cha,loc))
		else:
			self.failstr = "%s%s\n" % (self.failstr,line)


	def parse_email(self, path):
		"""
		Parses the Breq_fast email and stores matches in structures required for the ArcLink request.
		
		@arguments: path, the absolute path to file containing the email Breq_fast request
		"""
		fh = file(path)
		
		try:
			self.parse_email_from_handler(fh)
		finally:
			fh.close()

	def parse_email_from_handler(self, fh):
		endtoken = False
		reqflag = False
		
		try:
			for line in fh:                    
				if re.match(self.__tokenrule,line):
					self.head = "".join((self.head,line))
					endtoken = True
				elif endtoken:
					line = line.rstrip("\n")
					if len(line) > 0:
						#reqflag = True
						self.__parse_request(line)
					# (new)lines following the request lines are ignored #
					else:
						if reqflag:
							break
		finally:
			fh.close()

		self.__parse_token(self.head)
