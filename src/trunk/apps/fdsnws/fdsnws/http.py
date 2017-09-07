################################################################################
# Copyright (C) 2013-2014 by gempa GmbH
#
# HTTP -- Utility methods which generate HTTP result strings
#
# Author:  Stephan Herrnkind
# Email:   herrnkind@gempa.de
################################################################################

from twisted.web import http, resource, server, static, util

from seiscomp3 import Core, Logging

import utils
import json
import gnupg
import base64
import hashlib
import random
import os
import time
import datetime
import sys
try:
	import dateutil.parser
except ImportError, e:
	sys.exit("%s\nIs python-dateutil installed?" % str(e))

VERSION = "1.2.0"

################################################################################
class HTTP:

	#---------------------------------------------------------------------------
	@staticmethod
	def renderErrorPage(request, code, msg, ro=None):
		resp = """\
Error %i: %s

%s

Usage details are available from %s

Request:
%s

Request Submitted:
%s

Service Version:
%s
"""

		# rewrite response code if requested and no data was found
		if ro is not None and code == http.NO_CONTENT:
			code = ro.noData

		# status code 204 requires no message body
		request.setResponseCode(code)
		if code == http.NO_CONTENT:
			return None

		request.setHeader('Content-Type', 'text/plain')

		reference = "%s/" % (request.path.rpartition('/')[0])

		codeStr = http.RESPONSES[code]
		Logging.warning("responding with error: %i (%s)" % (code, codeStr))
		date = Core.Time.GMT().toString("%FT%T.%f")
		response = resp % (code, codeStr, msg, reference, request.uri, date,
		                   VERSION)
		utils.accessLog(request, ro, code, len(response), msg)
		return response


	#---------------------------------------------------------------------------
	@staticmethod
	def renderNotFound(request):
		msg = "The requested resource does not exist on this server."
		return HTTP.renderErrorPage(request, http.NOT_FOUND, msg)

	#---------------------------------------------------------------------------
	# Renders error page if the result set exceeds the configured maximum number
	# objects
	@staticmethod
	def checkObjects(req, objCount, maxObj):
		if objCount <= maxObj:
			return True

		msg = "The result set of your request exceeds the configured maximum " \
		      "number of objects (%i). Refine your request parameters." % maxObj
		utils.writeTS(req, HTTP.renderErrorPage(
		              req, http.REQUEST_ENTITY_TOO_LARGE, msg))
		return False




################################################################################
class ServiceVersion(resource.Resource):
	isLeaf = True

	#---------------------------------------------------------------------------
	def render(self, request):
		return VERSION


################################################################################
class NoResource(resource.Resource):

	#---------------------------------------------------------------------------
	def render(self, request):
		return HTTP.renderNotFound(request)


	#---------------------------------------------------------------------------
	def getChild(self, chnam, request):
		return self


################################################################################
class ListingResource(resource.Resource):

	html = """<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="author" content="gempa GmbH">
  <title>SeisComP3 FDSNWS Implementation</title>
</head>
<body>
  <h1>SeisComP3 FDSNWS Web Service</h1>
  <p>Index of %s</p>
  <ul>
%s
  </ul>
</body>"""

	#---------------------------------------------------------------------------
	def render(self, request):
		lis = ""
		if request.path[-1:] != '/':
			return util.redirectTo(request.path + '/', request)

		for child in self.children.items():
			if child[1].isLeaf:
				continue
			if hasattr(child[1], 'hideInListing') and child[1].hideInListing:
				continue
			name = child[0]
			lis += """<li><a href="%s/">%s/</a></li>\n""" % (name, name)
		return ListingResource.html % (request.path, lis)


	#---------------------------------------------------------------------------
	def getChild(self, chnam, request):
		if not chnam:
			return self
		return NoResource()



################################################################################
class DirectoryResource(static.File):

	#---------------------------------------------------------------------------
	def __init__(self, fileName):
		static.File.__init__(self, fileName)
		self.childNotFound = NoResource()


	#---------------------------------------------------------------------------
	def render(self, request):
		if request.path[-1:] != '/':
			return util.redirectTo(request.path + '/', request)
		return static.File.render(self, request)


	#---------------------------------------------------------------------------
	def getChild(self, chnam, request):
		if not chnam:
			return self
		return NoResource()



################################################################################
class AuthResource(resource.Resource):
	isLeaf = True

	def __init__(self, gnupghome, userdb):
		resource.Resource.__init__(self)
		self.__gpg = gnupg.GPG(gnupghome=gnupghome)
		self.__userdb = userdb

	#---------------------------------------------------------------------------
	def render_POST(self, request):
		request.setHeader('Content-Type', 'text/plain')

		try:
			verified = self.__gpg.decrypt(request.content.getvalue())

		except Exception, e:
			msg = "invalid token"
			Logging.warning("%s: %s" % (msg, str(e)))
			return HTTP.renderErrorPage(request, http.BAD_REQUEST, msg, None)

		if verified.trust_level is None or verified.trust_level < verified.TRUST_FULLY:
			msg = "token has invalid signature"
			Logging.warning(msg)
			return HTTP.renderErrorPage(request, http.BAD_REQUEST, msg, None)

		try:
			attributes = json.loads(verified.data)
			td = dateutil.parser.parse(attributes['valid_until']) - \
					datetime.datetime.now(dateutil.tz.tzutc())
			lifetime = td.seconds + td.days * 24 * 3600

		except Exception, e:
			msg = "token has invalid validity"
			Logging.warning("%s: %s" % (msg, str(e)))
			return HTTP.renderErrorPage(request, http.BAD_REQUEST, msg, None)

		if lifetime <= 0:
			msg = "token is expired"
			Logging.warning(msg)
			return HTTP.renderErrorPage(request, http.BAD_REQUEST, msg, None)

		userid = base64.urlsafe_b64encode(hashlib.sha256(verified.data).digest()[:18])
		password = self.__userdb.addUser(userid, attributes, time.time() + min(lifetime, 24 * 3600), verified.data)
		utils.accessLog(request, None, http.OK, len(userid)+len(password)+1, None)
		return '%s:%s' % (userid, password)



################################################################################
class Site(server.Site):

	#---------------------------------------------------------------------------
	def getResourceFor(self, request):
		Logging.debug("request (%s): %s" % (request.getClientIP(),
		              request.uri))
		request.setHeader('Server', "SeisComP3-FDSNWS/%s" % VERSION)
		request.setHeader('Access-Control-Allow-Origin', '*')
		request.setHeader('Access-Control-Allow-Headers', 'Authorization')
		request.setHeader('Access-Control-Expose-Headers', 'WWW-Authenticate')
		return server.Site.getResourceFor(self, request)

