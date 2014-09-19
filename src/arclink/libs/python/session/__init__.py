#***************************************************************************** 
# session.py
#
# Quick hack to get Pickle working with current mod_python
#
# (c) 2008 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

from seiscomp.filecontainer import *

SESSDIR = "/tmp/arclink/"

class AuthData(object):
    def __init__(self, user, passwd):
        self.user = user
        self.passwd = passwd

class SessionData(object):
    def __init__(self):
        self.timestamp = None   # for session expiration
        self.__auth = {}

    def get_auth(self, addr):
        return self.__auth.get(addr)

    def add_auth(self, addr, auth):
        self.__auth[addr] = auth

sessions = FileContainer(SESSDIR)

def have_key(sesskey):
    return sesskey in sessions

def get_data(sesskey):
    sessdata = sessions.get(sesskey)
    if sessdata == None:
        sessdata = SessionData()
        sessions[sesskey] = sessdata

    return sessdata

def set_data(sesskey, sessdata):
    sessions[sesskey] = sessdata

def new_auth(user, passwd):
    return AuthData(user, passwd)

