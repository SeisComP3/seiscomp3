#!/usr/bin/env python
#############################################################################
# breqfast.py                                                               #
#                                                                           #
# Processing breq_fast requests.                                            #
#                                                                           #
# (c) 2006 Doreen Pahlke, GFZ Potsdam                                       #
# (c) 2007 Mathias Hoffmann, GFZ Potsdam                                    #
#                                                                           #
# This program is free software; you can redistribute it and/or modify it   #
# under the terms of the GNU General Public License as published by the     #
# Free Software Foundation; either version 2, or (at your option) any later #
# version. For more information, see http://www.gnu.org/                    #
#                                                                           #
#############################################################################

#   changes 2011.234, MB&Andres
#   * Added support to multiples files in the case where no join is possible
#  (proxy + encryption). 
#   * Created a BASEDIR variable pointing to base installation

#   changes 2008.046, Mathias Hoffmann
#
# - allow trailing colon in header token (e.g. .MEDIA: )
# - allow network and station wildcards (limited to e.g.: ? * ??? * SN* *AA SN?? etc.)
# - more detailed error message, if request is too large
# - calculate file size only from available data
#
#

import sys
if sys.version_info < (2,4): from sets import Set as set
import os, shutil, datetime, poplib, smtplib, sys, StringIO, commands, fnmatch
from re import *
from time import *
from types import *
from email.MIMEText import MIMEText
from seiscomp import logs
from seiscomp.fseed import *
from seiscomp.arclink.manager import *
from seiscomp.db import DBError
from seiscomp.db.generic.inventory import Inventory
import seiscomp.mseedlite as mseed


DEFAULT_HOST = "localhost"
DEFAULT_PORT = 18001
DEFAULT_USER = "sysop"
ORGANIZATION = "WebDC"
SEED_LABEL = "WebDC SEED Volume"
ARCLINK_TIMEOUT = 100
ARCLINK_TIMEOUT_CHECK = 5
SOCKET_TIMEOUT = 100
REQUEST_TIMEOUT = 1800
REQUEST_WAIT = 10
MAX_LINES = 1000

BASEDIR = "/home/sysop/breqfast"

BREQ_DIR = BASEDIR+"/breq"
SPOOL_DIR = BASEDIR+"/spool"
ACC_DIR = BASEDIR+"/lib"

FTP_DIR = BASEDIR+"/data"
FTP_URL = "ftp://ftp.webdc.eu/breqfast"

#SMTP_SERVER = "smtp-server.gfz-potsdam.de"
#SMTP_SERVER = "localhost"
EMAIL_ADDR = "breqfast@webdc.eu"
EMAIL_FROM = "WebDC <breqfast@webdc.eu>"

LABEL = "breq_req"

FORMAIL_BIN = "/usr/bin/formail"
SENDMAIL_BIN = "/usr/sbin/sendmail"

VERSION = "0.13 (2017.221)"

class BreqParser(object):
	"""
	Parses the breq_fast email format using regular expressions.
	
	@classvariables: __tokenrule, defines the syntax of the beginning of a Breq_fast header token
					 __tokenlist, specifies the tokens required for the ArcLink request
					 __reqlist,   defines the syntax for a request line in Breq_fast format

	NOTE: parsing of e-mail addresses is rather more restrictive than allowed by RFC 5322/RFC 5321.
	In particular, white space should be allowed in the local part if escaped by '\' or quoted,
	and we may need to support Unicode characters in future.

	"""
	__tokenrule="^\.[A-Z_]+[:]?\s"
	
	__tokenlist=(
				 "\.NAME\s+(?P<name>.+)",
				 "\.INST\s+(?P<institution>.+)?",
				 "\.EMAIL\s+(?P<email>[A-Za-z0-9._+-]+[@][A-Za-z0-9.-]+)",
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
			self.mgr = ArclinkManager("%s:%d"%(DEFAULT_HOST,DEFAULT_PORT), DEFAULT_USER)
		except:
			pass
			


	def __parse_token(self, head):
		"""
		Gets the Breq_fast header to match it against the corresponding pattern.
		If successful, sets the dictionary for storing the ArcLink required matches.
		
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

		# Longest local part 64 octets, + "@" + longest domain part 255 octets.
		if self.tokendict.has_key("email") and len(self.tokendict["email"]) > 320:
			self.failstr = "%s.EMAIL argument is too long.\n" % self.failstr


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

                        n = re.sub("[?]+", "*", network)
                        s = re.sub("[?]+", "*", station)

			# FIXME: Following uses default values of
			# start_time=None, end_time=None,
			# but these should be set based on the time span
			# being requested.
			db = self.mgr.get_inventory(network=n, station=s)
	
			netlist = []
			netlist = db.network.keys()
			logs.debug("- netlist: %s" % netlist)
			
			for net_code in netlist:
				for net in db.network[net_code].itervalues():
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
					self.reqlist.append((str(network),str(station),cha,loc,beg_time,end_time, {}, set()))
					logs.debug("reqlist.append: %s %s %s %s" % (str(network),str(station),cha,loc))
		else:
			self.failstr = "%s%s\n" % (self.failstr,line)

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

					if len(self.reqlist) > MAX_LINES:
						break

		finally:
			fh.close()

		self.__parse_token(self.head)
		
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


class SeedOutput(object):
    def __init__(self, fd, inv, resp_dict = False):
        self.__fd = fd
        self.__inv = inv
        self.__resp_dict = resp_dict
        self.__mseed_fd = TemporaryFile()

    def write(self, data):
        self.__mseed_fd.write(data)

    def close(self):
        try:
            try:
                seed_volume = SEEDVolume(self.__inv, ORGANIZATION, LABEL,
                    self.__resp_dict)

                self.__mseed_fd.seek(0)
                for rec in mseed.Input(self.__mseed_fd):
                    seed_volume.add_data(rec)

                seed_volume.output(self.__fd)

            except (mseed.MSeedError, SEEDError, DBError), e:
                logs.error("error creating SEED volume: " + str(e))

        finally:
            self.__mseed_fd.close()
            self.__fd.close()


### private module methods for checking the data accessibility, availability and plausibility ###
# FIXME #
def _check_access(email_addr, elem, ext):
	"""
	Checks the access rights for a given user using the flat files access.net and ~.stat.

	@arguments: email_addr, a string defining the users email address
				elem,       a string giving the network or station code
				ext,        a string specifying the file extension of the access flat file
	@return: True, if the there does not exist any restriction or the user has the access rights
			False, otherwise
	"""
	domain = email_addr[email_addr.find("@")+1:]
	found = False
	
	fh = file("%s/access.%s" % (ACC_DIR,ext))
	try:
		line = fh.readline()
		while len(line) > 0:
			spl = line.split()
			if len(spl) > 1:
				if spl[1] == "ALL" and (email_addr == spl[0] or domain.endswith(spl[0])):
					return True
				elif spl[1] == elem:
					found = True
					if email_addr == spl[0] or domain.endswith(spl[0]):
						return True
			line = fh.readline()
	finally:
		fh.close()

	return not found


# FIXME #
def _check_availability(reqline):
	"""
	Checks for the given request line the availability using the 'Hemmleb'-database.

	@arguments: reqline: a tuple of request attributes (net, sta, cha, loc, start, end, constr)
	@return: True, if the requested time window or a part of it is available
			False, otherwise
	"""
	loc = 'all'
	if reqline[3] != "":
		loc = reqline[3]

	command = ACC_DIR + "/dbselect.pl -l all %s %s \"%s\" \"%s\" %s %s" % (reqline[0],reqline[1],loc,reqline[2],reqline[4].strftime("%Y%m%d%H%M%S"),reqline[5].strftime("%Y%m%d%H%M%S"))

	child = os.popen(command)
	result = child.read()
	err = child.close()
	if err:
		raise RuntimeError, '%s failed with exit code %d' % (command, err)

	m = re.search("(\d+) files found",str(result))

	if m:
		if int(m.group(1)) > 0: 
			return True
	return False


def _check_time(beg_time, end_time):
	"""
	Checks the validity of the given time span.
	
	@arguments: beg_time, a datetime object storing the start of time window
				end_time, a datetime object storing the end of time window
	@return: True if valid; False otherwise
	"""
	if beg_time > end_time or beg_time > datetime.datetime.today():
		return False
	
	return True


def _get_size(reqlist):
	"""
	Computes the expected file size of the full SEED volume.

	@arguments: reqlist, a list of request tuples
	@return: The expected full SEED volume file size in bytes.
	"""
	fsize = 0
	comp = 1
	samp_dict = {"B": 20, "E": 100, "H": 100, "L": 1, "S": 50, "U": 0.01, "V": 0.1}
	
	for req in reqlist:
		tdiff = req[5] - req[4]
		tdiff = tdiff.days*86400+tdiff.seconds
		samp = samp_dict.get(req[2][0].upper(), 20)
		if req[2].endswith("*"):
			comp = 3
		fsize += int(tdiff*samp*comp*1.5)
		
	return fsize


def _get_req_size(req):
	"""
	Computes the expected file size of the full SEED volume.

	@arguments: a request tuple
	@return: The expected full SEED volume file size in bytes.

	PLE 2011-08-10:
	This appears to be a grotesque over-estimate of the actual
	size, at least for large requests.
	It is based on 1.5 bytes per sample.
	"""
	fsize = 0
	comp = 1
	samp_dict = {"B": 20, "E": 100, "H": 100, "L": 1, "S": 50, "U": 0.01, "V": 0.1}
	
	tdiff = req[5] - req[4]
	tdiff = tdiff.days*86400+tdiff.seconds
	samp = samp_dict.get(req[2][0].upper(), 20)
	if req[2].endswith("*"):
		comp = 3
	fsize += int(tdiff*samp*comp*1.5)
	
	return fsize


def _check_size(reqlist):
	"""
	Checks the amount of the given request.

	changes: Mathias, 31.05.2007

	@arguments: reqlist, a list of request tuples
	@return: error_string   if the time window does exceed 5 days or
							the number of request lines is greater than 100
			"" otherwise
	"""
	tdiff = datetime.timedelta()
	for req in reqlist:
		tdiff = req[5] - req[4]
		if tdiff.days > 999:
			return "Max. time window of 999 days exceded. ["+req[0]+" "+req[1]+" "+req[2]+"]"
		
	if len(reqlist) > 1000:
		return "Number of requests exceeds limit of 1000."
	
	return ""
		
###############################################################################################
	

### (private) module methods and variables to process the Breq_fast request ###
### messages for status email ###
_emailextro = "In case you see any problems with your request or if you have other questions, please contact geofon_dc@gfz-potsdam.de."
_arctext = "Data found in GFZ archive:"
_badtext = "Invalid data request:"
_noctext1 = "Requests to restricted data:"
_noctext2 = "Contact geofon_dc@gfz-potsdam.de for access rights!"
_nodtext = "Data not found in GFZ archive:"
_toolarge = "Unfortunately your request is too large for automatic processing.\nPlease try again a smaller request."
_emailtext = "You have requested data from the WebDC data archive. This request is now queued for automatic execution. Depending on the amount of data and the load of the system it may take some time until it will be processed. You will get an email message when it is finished."
_nonetext = "None of your requested data have been found in our database. It will be forwarded to other data centers. Depending on the amount of data and the load of the system it may take some time until it will be processed. You will get an email message when it is finished."
### messages for resulting email ###
_oktext1 = "Your breq_fast request to the WebDC has been processed.\n\nPlease find the resulting data file in your personal ftp directory:"
_oktext1a = "Files with suffix .openssl contain restricted data and have been encrypted. Use 'openssl des-cbc -pass pass:<Password> -in <Input> -out <Output> -d' to decrypt. Remember that you should use the correct password based on the datacenter id of the file. The datacenter id is the field just before the '.seed' on the filename."
_oktext2 = "In case of problems please send a message to geofon_dc@gfz-potsdam.de.\n\nIf you use data from the WebDC for a publication, please acknowledge the GEOFON Program of GFZ Potsdam."
_noktext = "Your breq_fast request to the WebDC has been processed. We are very sorry, but unfortunately none of the requested data could be supplied."
_twok = "Time windows found:"
_twno = "Time windows NOT found:"
_rtno = "Routing NOT found:"

STATUS_TIMEOUT = 16
STATUS_ROUTING = 17
STATUS_SEED = 18
	
def _write_status_file(fbase, fext, ftext):
	"""
	Writes the specified text in the given file.


	@arguments: fbase, a string containing the file name
				fext,  a string defining the file extension
				ftext, a string storing the text which should be inserted in the file
	"""
	fname = fbase
	if fext != "":
		fname = ".".join((fbase,fext))

	fh = file(fname,"w+")
	try:
		fh.write(ftext)
	finally:
		fh.close()


def _status_to_string(status):
	"""
	Translates a given Arclink status value into a string.

	@arguments: status, an integer value specifying the Arclink status.
	@return: a string giving the translation of the status value
	"""
	if status == STATUS_PROC:   return "Request processing is not finished yet."
	elif status == STATUS_CANCEL: return "Request processing is cancelled:"
	elif status == STATUS_OK:     return _twok
	elif status == STATUS_WARN:   return "Request processing returns WARNING."
	elif status == STATUS_ERROR:  return "Request processing returns ERROR."
	elif status == STATUS_RETRY:  return "Request processing returns RETRY."
	elif status == STATUS_DENIED: return "Access denied:"
	elif status == STATUS_NODATA: return _twno
	elif status == STATUS_UNSET:  return "Request processing returns UNSET."
	elif status == STATUS_TIMEOUT: return "Request processing gets TIMEOUT."
	elif status == STATUS_ROUTING: return "Request routing failed."
	elif status == STATUS_SEED: return "Error creating SEED volume."


def _request_content_to_string(content):
	"""
	Converts the given list of request tuples to a string.

	@arguments: content, a list of request tuples (<net> <sta> <chan> <loc> <begtime> <endtime> <constr>)
	@return: the string representation
	"""
	retstr = ""

	for elem in content:
		retstr = "%s%s %s %s %s %s %s\n" % \
				(retstr,elem[0],elem[1],elem[2],elem[3],elem[4].strftime("%Y,%m,%d,%H,%M,%S"),elem[5].strftime("%Y,%m,%d,%H,%M,%S"))

	return retstr


def check_request(fname, basename, parser):
	"""
	Checks the availability, accessibility and plausibility of a single request line.
	Creates the corresponding files in the Breq_fast processing directory.
	Creates the file with specific suffix in the make-directory of SPOOL_DIR.
	Returns the email message containing the check status.
	
	@arguments: fname,    gives the path to the Breq_fast email
				basename, a string specifying the basename of the status files
				parser,   an object of BreqParser class
	@return: a string, giving the status email message
	"""
	arctext = badtext = noctext = nodtext = msg = ""
	ckname = os.path.join(os.path.dirname(fname),"check")
	fname = os.path.splitext(fname)[0]
	
	emailaddr = EMAIL_ADDR
	try:
		emailaddr = parser.tokendict["email"]
	except KeyError:
		pass

	requestSize = 0;
	for reqline in parser.reqlist:
		check = 0
		linestr = " ".join((reqline[0],reqline[1],reqline[2],reqline[3],
							reqline[4].strftime("%Y,%m,%d,%H,%M,%S"),reqline[5].strftime("%Y,%m,%d,%H,%M,%S")))
		# check the access rights #
		#if not _check_access(emailaddr,reqline[0],"net"):
		#	noctext = "%s%s\n" % (noctext,linestr)
		#	check = 1
		#if not check and not _check_access(emailaddr,reqline[1],"stat"):
		#	noctext = "%s%s\n" % (noctext,linestr)
		#	check = 1
		# check the time validity
		#if not check and not _check_time(reqline[4],reqline[5]):
		#	badtext = "%s%s\n" %(badtext,linestr)
		#	check = 1
		## check the availability #
		#if not check and not _check_availability(reqline):
		#	nodtext = "%s%s\n" % (nodtext,linestr)
		#	check = 1
		# write the archive text #
		if not check:
			requestSize += _get_req_size(reqline)
			arctext = "%s%s\n" % (arctext,linestr)

	
	sys.stderr.write( "--> Estimated total size of request: %f MByte\n" % (requestSize / 1024.0**2))
	gigabyte = 1024.0**3
	maxRequestSize = 3.0 * gigabyte
	if len(parser.reqlist) > MAX_LINES:
		sys.stderr.write( "--> this request is too large!!\n")
		msg = _toolarge + "\n----> reason for refusal: too many lines (after wildcard expansion)"
		_write_status_file(os.path.join(SPOOL_DIR,"make",basename),"too_large","")
	elif requestSize > maxRequestSize:
		sys.stderr.write( "--> this request is too large!!\n")
		msg = _toolarge + "\n----> reason for refusal: request too large! (%.1f > %.1f GByte)" % (requestSize / gigabyte,
													  maxRequestSize / gigabyte)
		_write_status_file(os.path.join(SPOOL_DIR,"make",basename),"too_large","")
	else:						
		### write the check status files ###
		_write_status_file(ckname,"arc",arctext)
		_write_status_file(ckname,"bad","%s%s" % (badtext,parser.failstr))
		_write_status_file(ckname,"noc",noctext)
		_write_status_file(ckname,"nod",nodtext)
		
		if arctext != "":
			arctext = "%s\n%s\n" % (_arctext,arctext)
		if badtext != "":
			badtext = "%s\n%s\n" % (_badtext,badtext)
		if parser.failstr != "":
			badtext = "%s\nThe following request lines are not Breq_fast conform:\n%s\n--> Look at http://ds.iris.edu/ds/nodes/dmc/manuals/breq_fast/ for the specification of BREQ_FAST requests.\n" % (badtext,parser.failstr)
		if noctext != "":
			noctext = "%s\n%s" % (_noctext1,noctext)
			noctext = "%s%s\n\n" % (noctext,_noctext2)
		if nodtext != "":
			nodtext = "%s\n%s\n" % (_nodtext,nodtext)
	
		# HACK, because of trouble with database connection to st7
		arctext = " "

		### create an email text containing the check status ###
		msg = "".join((arctext,nodtext,noctext,badtext))
		_write_status_file(os.path.join(SPOOL_DIR,"make",basename),"","")
		if arctext == "" and noctext == "" and badtext == "":
			msg = "\n%s\n%s" % (msg,_nonetext)
		elif arctext != "":
			msg = "\n%s\n%s" % (msg,_emailtext)
			
	### write the Breq_fast header file ###
	_write_status_file(fname,"head",parser.head)
	
	return msg


def show_status(rqstat):
    if rqstat.error:
        req_status = "ERROR"
    elif rqstat.ready:
        req_status = "READY"
    else:
        req_status = "PROCESSING"

    logs.info("Request ID: %s, Label: %s, Type: %s, Args: %s" % \
        (rqstat.id, rqstat.label, rqstat.type, rqstat.args))
    logs.info("Status: %s, Size: %d, Info: %s" % \
        (req_status, rqstat.size, rqstat.message))

    for vol in rqstat.volume:
        logs.info("    Volume ID: %s, Status: %s, Size: %d, Info: %s" % \
            (vol.id, arclink_status_string(vol.status), vol.size, vol.message))

        for rqln in vol.line:
            logs.info("        Request: %s" % (rqln.content,))
            logs.info("        Status: %s, Size: %d, Info: %s" % \
              (arclink_status_string(rqln.status), rqln.size, rqln.message))

    logs.info("")


def build_filename(encrypted, compressed, req_args):
    endung = ""
    if compressed is True:
        endung = '.bz2'
    elif compressed is None and req_args.has_key("compression"):
        endung = '.bz2'
    if encrypted:
        endung = endung + '.openssl'
    return endung;


def submit_request(parser, req_name, breq_id):
	"""
	Routes the request and analyses its results.
	Creates the corresponding files in the Breq_fast processing directory.
	Returns an email message containing the processing status of the breqfast request.

	@arguments: parser,  a BreqParser object
		    req_name, a string defining the request name
		    breq_id,  a string specifying the internal Breq_fast request ID
	@return: a string, giving the processing status email message
	"""
	emailaddr = EMAIL_ADDR
	try:
		emailaddr = parser.tokendict["email"]
	except KeyError:
		pass
	
	label = LABEL
	try:
		label = parser.tokendict["label"]
	except KeyError:
		pass
	
	label = re.sub("[^\w]", "_", str(label))

	arcl = ArclinkManager(DEFAULT_HOST + ":" + str(DEFAULT_PORT),emailaddr)
	# Default format is full SEED, however, we can request MSEED
	# and do the conversion here. In this case, we will end up
	# with a single SEED volume even if data comes from multiple
	# sources.
	wf_req = arcl.new_request("WAVEFORM",{"format": "FSEED"},label)
	for x in parser.reqlist:
	    wf_req.add(*x)
	
        # List of failed request lines associated to an error message.
	ok_content = []
	failed_content = {}
	emailmsg = ""
	emailmsg_extra = ""
	reqlogmsg = ""

	try:
		global logstream
		logstream = cStringIO.StringIO()
		try:
			(inv, req_sent, req_noroute, req_nodata) = arcl.execute(wf_req, True, True)
		
			logs.info("the following data requests were sent:")
			for req in req_sent:
				logs.info(req.dcname)
				show_status(req.status())
			
			if req_noroute:
				tmpstream = cStringIO.StringIO()
				req_noroute.dump(tmpstream)
				logs.info("the following entries could not be routed:")
				logs.info(tmpstream.getvalue())
			
			if req_nodata:
				tmpstream = cStringIO.StringIO()
				req_nodata.dump(tmpstream)
				logs.info("the following entries returned no data:")
				logs.info(tmpstream.getvalue())
			
		finally:
			reqlogmsg = logstream.getvalue()
			logstream = None
		
		if req_noroute:
			failed_content[STATUS_ROUTING] = req_noroute.content

		# This is necessary here because sometimes below we can't catch full empty requests
		if req_nodata:
			failed_content[STATUS_NODATA] = req_nodata.content
		
		if not os.path.exists("%s/%s" % (FTP_DIR, req_name)):
			os.mkdir("%s/%s" % (FTP_DIR, req_name))

		prefix = "%s/%s_%s" % (req_name, label, breq_id)
		urllist = []

		canJoin = True
		volumecounts = 0
		
		for req in req_sent:
			reqstatus = req.status()
			if reqstatus.encrypted:
				canJoin = False
			for vol in reqstatus.volume:
				if arclink_status_string(vol.status) == "OK" and vol.size > 0:
					volumecounts += 1
				if vol.encrypted and vol.size > 0:
					canJoin = False
		
		sufix = ""
		addname = ""
		fd_out = None

		logs.warning("Can Join is set to: %s" % canJoin)
		logs.warning("We have %s volumes to download" % volumecounts)

		if canJoin and volumecounts > 0:
			filename = FTP_DIR + '/' + prefix + '.seed'
			fd_out = open(filename, "wb")
			fd_out = SeedOutput(fd_out, inv)
		
		cset = set()
		
		# process resent requests before original failed requests
		req_sent.reverse()

		for req in req_sent:
			for vol in req.status().volume:
				if vol.size == 0:
					continue
				
				if not canJoin:
					addname = str(".%s.%s" % (req.id, vol.id))
					filename = FTP_DIR + '/' + prefix + addname + '.seed'
					fd_out = open(filename, "wb")
				vol_status = vol.status
				
				try:
				    req.download_data(fd_out, vol.id, block=True, purge=False)

				except (ArclinkError, socket.error), e:
					logs.error('error on downloading request: ' + str(e))
					if fd_out is not None: fd_out.close()
					raise

				except (IOError, OSError, DBError, SEEDError, mseed.MSeedError), e:
					logs.error("error creating SEED Volume: %s" % str(e))
					vol_status = STATUS_ERROR

				for rqln in vol.line:
					clist = rqln.content.split(" ")

					begtime = datetime.datetime(*[int(elem) for elem in clist[0].split(",")])
					endtime = datetime.datetime(*[int(elem) for elem in clist[1].split(",")])
					
					try:
						ctuple = tuple([str(clist[2]),str(clist[3]),str(clist[4]),str(clist[5]),begtime,endtime,{}])
					except IndexError:
						ctuple = tuple([str(clist[2]),str(clist[3]),str(clist[4]),"",begtime,endtime,{}])
						
					# ignore failed content that was resent
					if ctuple[:6] in cset:
						continue
					
					cset.add(ctuple[:6])
					
					if vol_status == STATUS_OK:
						status = rqln.status
					else:
						status = vol_status

					if status == STATUS_OK:
						ok_content += [ctuple]
					else:
						try:
							# And this sometimes catch two times what was already catch up.
							# I think that the problem is on the manager.
							failed_content[status] += [ctuple]
							
						except KeyError:
							failed_content[status] = [ctuple]
							
				if vol_status != STATUS_OK:
					if fd_out is not None: fd_out.close()
					continue
				
				if not canJoin and fd_out is not None:
					fd_out.close()
					endung = build_filename(req.encStatus, req.decStatus, req.args)
					if endung:
						os.rename(filename, filename+endung)
					urllist.append(FTP_URL + '/' + prefix + addname + '.seed' + endung)
			try:
				req.purge()
			except ArclinkError, e:
				logs.error('error on purging request: ' + str(e))
			
		
		if canJoin and fd_out is not None:
			fd_out.close()
			endung = build_filename(req.encStatus, req.decStatus, req.args)
			if endung:
				os.rename(filename, filename + endung)
			urllist.append(FTP_URL + '/' + prefix + '.seed' + endung)
		
	except (ArclinkError, socket.error), e:
		logs.warning("request failed: %s" % str(e))
		failed_content[STATUS_ERROR] = wf_req.content

		emailmsg_hint = ""
		# The "size exceeded" message is misleading - it's the number
		# of different streams which is too large, not the amount of data.
		# Adjust request_size in arclink.ini to change this at the server.
		if (search("size exceeded", str(e))):
			emailmsg_hint = "(By default, only 1000 streams can be requested at once.)"

		if (search("size exceeded", str(e)) or search("empty request", str(e))):
		        emailmsg_extra = """
Your Arclink request failed with the following message:
%s
%s
We hope that helps.\n\n""" % (str(e), emailmsg_hint)

		else:
			raise

	### formatting the output ###
	if len(ok_content) == 0:
		emailmsg = "%s\n\n" % _noktext

	rqlines = ""
	if len(ok_content) > 0:
		if canJoin:
			emailmsg = "%s\n\n%s\n\n%s\n\n%s\n" % (_oktext1,"\n".join(urllist),_oktext2,_twok)
		else:
			emailmsg = "%s\n\n%s\n\n%s\n\n%s\n\n%s\n" % (_oktext1,"\n".join(urllist),_oktext1a,_oktext2,_twok)
		rqlines = _request_content_to_string(ok_content)
		emailmsg = "%s%s\n" % (emailmsg, rqlines)
	_write_status_file(os.path.join(BREQ_DIR,req_name,breq_id,"proc"),"arc",rqlines)
	
	if len(failed_content) > 0:
		rqlines = ""
		try:
			rqlines = _request_content_to_string(failed_content[STATUS_NODATA])
			emailmsg = "%s%s\n%s\n" % (emailmsg, _twno, rqlines)
		except KeyError:
			pass
		_write_status_file(os.path.join(BREQ_DIR,req_name,breq_id,"proc"),"nod",rqlines)
		rqlines_all = ""
		for keystr in failed_content:
			if keystr != STATUS_NODATA:
				rqlines = _request_content_to_string(failed_content[keystr])
				emailmsg = "%s%s\n%s\n" % (emailmsg, _status_to_string(keystr), rqlines)
				rqlines_all = "%s%s\n" % (rqlines_all, rqlines)		
		_write_status_file(os.path.join(BREQ_DIR,req_name,breq_id,"proc"),"fail",rqlines_all)

	if len(emailmsg_extra) > 0:
		emailmsg = "%s%s\n" % (emailmsg, emailmsg_extra)

	emailmsg = "%s\n\nArcLink log:\n\n%s" % (emailmsg, reqlogmsg)
	return emailmsg


def submit_email(to, subj, text):
		"""
		Sends an email with given subject and text to a specified address using the specified SMTP host.

		@arguments: to,   a string storing the email address of the recipient
					subj, a string defining the email subject
					text, a string containing the email message text
		"""
		# I am not able to set sender to breqfast@webdc.eu with smtplib.
		# It is forced to sysop@webdc.eu, causing the message to be rejected
		# by the GFZ mail server. -Andres
		
		#msg = MIMEText("")
		#msg['Subject'] = subj
		#msg['From'] = EMAIL_ADDR
		#msg['To'] = to
		#msg.set_payload(text)            

		#server = smtplib.SMTP(SMTP_SERVER)
		#server.sendmail(EMAIL_ADDR,to,msg.as_string())
		#server.quit()        

		cmd = "%s -I'From: %s' -I'To: %s' -I'Subject: %s' -a'Message-ID:' -A'X-Loop: %s' | %s -f'%s' -- '%s'" % \
			(FORMAIL_BIN, EMAIL_FROM, to, subj, EMAIL_ADDR, SENDMAIL_BIN, EMAIL_ADDR, to)
			
		logs.debug("executing cmd: %s" % cmd)

		fd = os.popen(cmd, "w")
		try:
			fd.write(text)
		finally:
			fd.close()
			
def start():
	"""
	Checks request spool directory for files => iterating and processing
	"""

	while True:
		names = set()
		checklist = [ f for f in os.listdir(os.path.join(SPOOL_DIR,"check")) if os.path.isfile(os.path.join(SPOOL_DIR,"check",f)) and not f.endswith("_checking") ]

		if not checklist:
			break

		for fname in checklist:
			fname = os.path.join(SPOOL_DIR,"check",fname)
			basename = os.path.basename(fname)
			m = re.match("^.+/(?P<req_name>.+)[_](?P<breq_id>\w+[_]\d+)$",fname)
			if m:
				(req_name, breq_id) = (m.group("req_name"), m.group("breq_id"))
				if req_name in names:
					continue

				names.add(req_name)

				sys.stderr.write("working on: %s %s\n" % (req_name, breq_id))
			else:
				os.rename(fname,fname.replace("_checking","_fail"))
				logs.error("Parsing of Breq_fast name and ID in %s failed" % fname)
				continue
			
			### redirect the logging output to a logfile ###
			set_logger(os.path.join(BREQ_DIR,req_name,breq_id,"breq_mail.log"))
			
			### mark the processed file with suffix _checking ###
			logs.debug("checking file %s" % fname)
			os.rename(fname,"_".join((fname,"checking")))
			fname = "_".join((fname,"checking"))
			logs.debug("rename file in %s" % fname)
				
			### parse the original breq_fast email ###
			email = os.path.join(BREQ_DIR,req_name,breq_id,"breq_mail.org")
			parser = BreqParser()
			parser.parse_email(email)
			logs.debug("parsing email %s" % email)
			
			### create the response email message after checking this email ###
			emailmsg = check_request(email,basename,parser)
                        emailmsg = "%s\n\nThis request has the request ID: %s_%s\n\n%s\n" % (emailmsg,req_name,breq_id,_emailextro)
                        emailmsg = "%s\n\nbreq_fast request header:\n%s" % (emailmsg, parser.head)
                        emailmsg = "%s\nbreq_fast request lines:%s\n" % (emailmsg, parser.request)
			emailaddr = EMAIL_ADDR
			try:
				emailaddr = parser.tokendict["email"]
			except KeyError:
				pass

			errorstate = False
			if os.path.exists(os.path.join(SPOOL_DIR,"make",basename+"_running")):
				### email was sent before crash, don't send it again
				logs.debug("email notification was already sent")
				os.unlink(os.path.join(SPOOL_DIR,"make",basename+"_running"))
				errorstate = True

			else:
				submit_email(emailaddr,"breq_fast request %s_%s checked" % (req_name,breq_id),emailmsg)
				logs.debug("email submitted with message: %s" % emailmsg)

			### mark the processed file with suffix _done and move it to the check/done-dir in SPOOL_DIR ###
			shutil.move(fname,os.path.join(SPOOL_DIR,"check","done",basename+"_done"))
			logs.debug("move file %s to check/done dir" % fname)
			fname = os.path.join(SPOOL_DIR,"make",basename)
			logs.debug("now look for file %s" % fname)
			
			if (os.path.exists(fname)):
				### mark the processed file with suffix _running ###
				os.rename(fname,"_".join((fname,"running")))
				fname = "_".join((fname,"running"))
				logs.debug("rename file in %s" % fname)
				
				try:
					### submit the request to arclink server ###
					emailmsg = submit_request(parser,req_name,breq_id)

					### submit the email containing the processing status of the Breq_fast request
					submit_email(emailaddr,"breq_fast request %s_%s processed" % (req_name,breq_id),emailmsg)
					logs.debug("email submitted with message: %s" % emailmsg)

				except (ArclinkError, socket.error), e:
					logs.error("quit processing: " + str(e))

					if not errorstate:
						#submit_email("admin", "breqfast failure", str(e))
						pass

					shutil.move(os.path.join(SPOOL_DIR,"check","done",basename+"_done"), os.path.join(SPOOL_DIR,"check",basename))
					break
					
				if errorstate:
					#submit_email("admin", "breqfast OK", "")
					pass

				### mark the processed file with suffix _done and move it to the make/done-dir in SPOOL_DIR ###
				shutil.move(fname,os.path.join(SPOOL_DIR,"make","done",basename+"_done"))
				logs.debug("move file %s in make/done dir" % fname)

logstream = None

def make_logger(fname):
	def log_print(s):
		if logstream:
			logstream.write(s + "\n")

		else:
			try:
				fh = file(fname,"a")
				try:
					fh.write(s + "\n")
				finally:
					fh.close()
			except OSError:
				logs.error("Log file %s could not be opened!" % fname)
		
	return log_print

		
def set_logger(fname):
	logs.debug = make_logger(fname)
	logs.info = make_logger(fname)
	logs.notice = make_logger(fname)
	logs.warning = make_logger(fname)
	logs.error = make_logger(fname)

	
if __name__ == "__main__":
    start()

