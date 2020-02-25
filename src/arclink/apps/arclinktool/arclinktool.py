#!/usr/bin/env python
#***************************************************************************** 
# arclinktool.py
#
# Simple ArcLink command-line client
#
# (c) 2004 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import sys
import re
import getpass
from optparse import OptionParser
from seiscomp.arclink.client import *

VERSION = "1.0 (2010.256)"

arcl = Arclink()
block = False

def submit(file):
    reqf = open(file)
    try:
        req_id = arcl.submit(reqf)
    finally:
        reqf.close()
    
    print("Request successfully submitted")
    print("Request ID:", req_id)

def show_status(req_id):
    status = arcl.get_status(req_id)

    for req in status.request:
        if req.error:
            req_status = "ERROR"
        elif req.ready:
            req_status = "READY"
        else:
            req_status = "PROCESSING"

        print("Request ID: %s, Label: %s, Type: %s, Encrypted: %s, Args: %s" % \
            (req.id, req.label, req.type, req.encrypted, req.args) )
        print("Status: %s, Size: %d, Info: %s" % \
            (req_status, req.size, req.message) )

        if req.user != "":
            print("User: %s, Institution: %s" % (req.user, req.institution))

        for vol in req.volume:
            print("    volume ID: %s, dcid: %s, Status: %s, Size: %d, Encrypted: %s, Info: %s " % \
            (vol.id, vol.dcid, arclink_status_string(vol.status), vol.size, vol.encrypted, vol.message))

            for rqln in vol.line:
                print("        request: %s" % (rqln.content,))
                print("        status: %s, Size: %d, Info: %s" % \
                    (arclink_status_string(rqln.status), rqln.size, rqln.message))

def download(req_vol):
    rv = req_vol.split(".", 1)
    req_id = rv[0]
    if len(rv) == 1:
        vol_id = None
    else:
        vol_id = rv[1]

    to = 0
    if block:
        to = 60000

    outfd = open(outf, "w")
    try:
        arcl.download_data(outfd, req_id, vol_id, timeout=to, password=SSLpassword)
    finally:
        outfd.close()

    print("Download successful")

def purge(req_id):
    arcl.purge(req_id)
    print("Product successfully deleted")

def process_options():
    parser = OptionParser(usage="usage: %prog -u user [-i institution] [-o file] [-b] [-w password] {-r|-s|-d|-p} host:port",
      version="%prog v" + VERSION)

    parser.add_option("-u", "--user", type="string", dest="user",
      help="user's e-mail address")

    parser.add_option("-i", "--institution", type="string", dest="inst",
      help="user's institution")

    parser.add_option("-o", "--output-file", type="string", dest="outf",
      help="file where downloaded data is written")
# This is handled automatically by the client.
#    parser.add_option("-c", "--decompress", type="string", dest="decomp",
#      help="compression type for decompression")

    parser.add_option("-w","--SSLpassword", type="string", dest="SSLpassword",
      help="password to decrypt the received file (default %default)")

    parser.add_option("-r", "--submit", type="string", dest="request_file",
      help="submit request")

    parser.add_option("-s", "--status", type="string", dest="status_id",
      help="check status")

    parser.add_option("-d", "--download", type="string", dest="download_id",
      help="download product [-b blocks]")

    parser.add_option("-b", "--blocking", action="store_true", dest="block", default=False, help="use blocking download")

    parser.add_option("-p", "--purge", type="string", dest="purge_id",
      help="delete product from the server")

    (options, args) = parser.parse_args()

    if len(args) != 1:
        parser.error("incorrect number of arguments")

    m = re.compile(r'([^:]+):([0-9]{1,5})').match(args[0])
    if m is None:
        parser.error("address not in form of host:port")

    (host, port) = m.groups()
    
    if options.user is None:
        parser.error("username required")
    
    action = None
    action_arg = None

    if options.request_file is not None:
        action = submit
        action_arg = options.request_file

    if options.status_id is not None:
        if action is not None:
            parser.error("conflicting options");
            
        action = show_status
        action_arg = options.status_id

    if options.download_id is not None:
        if action is not None:
            parser.error("conflicting options");

        if options.outf is None:
            parser.error("output file required")
        
        block = options.block
        action = download
        action_arg = options.download_id

    if options.purge_id is not None:
        if action is not None:
            parser.error("conflicting options");

        action = purge
        action_arg = options.purge_id

    if action is None:
        parser.error("one of -r/-s/-l/-d/-p must be given")

    return (options.SSLpassword, options.user, options.inst, options.outf,
      host, int(port), action, action_arg, options.block)

(SSLpassword, user, inst, outf, host, port, action, action_arg, block) = process_options()

try:
    #passwd = getpass.getpass().strip()
    #if len(passwd) == 0:
    passwd = None

    arcl.open_connection(host, port, user, passwd, inst)
    print("Connected to", arcl.software, "at", arcl.organization)
    action(action_arg)
    sys.exit(0)

except ArclinkAuthFailed:
    print("Authentification failed")

except ArclinkCommandNotAccepted:
    print("Error:", arcl.get_errmsg())

except ArclinkError as e:
    print("Error:", str(e))

sys.exit(1)

