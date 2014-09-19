#***************************************************************************** 
# filecontainer.py
#
# A dictionary based on files
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

import os
import cPickle

try:
    import fcntl             # missing in jython
    _have_fcntl = True
except ImportError:
    _have_fcntl = False
    
class FileContainer(object):
    def __init__(self, dir):
        self.__dir = dir
    
    def __setitem__(self, key, value):
        fd = open(self.__dir + "/" + key, "w")
        try:
            if _have_fcntl:
                fcntl.lockf(fd, fcntl.LOCK_EX)
                
            try:
                cPickle.dump(value, fd)
            finally:
                if _have_fcntl:
                    fcntl.lockf(fd, fcntl.LOCK_UN)
        finally:
            fd.close()

    def __getitem__(self, key):
        try:
            fd = open(self.__dir + "/" + key, "r")
            try:
                if _have_fcntl:
                    fcntl.lockf(fd, fcntl.LOCK_SH)
                    
                try:
                    return cPickle.load(fd)
                finally:
                    if _have_fcntl:
                        fcntl.lockf(fd, fcntl.LOCK_UN)
            finally:
                fd.close()
        except IOError:
            raise KeyError, key
    
    def __delitem__(self, key):
        try:
            os.unlink(self.__dir + "/" + key)
        except OSError:
            raise KeyError, key

    def __contains__(self, key):
        try:
            os.stat(self.__dir + "/" + key)
            return True
        except OSError:
            return False
    
    def get(self, key, default = None):
        try:
            return self[key]
        except KeyError:
            return default

