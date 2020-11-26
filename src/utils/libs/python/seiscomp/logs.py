#***************************************************************************** 
# logs.py
#
# SeisComP log handlers
#
# (c) 2005 Andres Heinloo, GFZ Potsdam
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version. For more information, see http://www.gnu.org/
#*****************************************************************************

from __future__ import absolute_import, division, print_function

import sys as _sys
import traceback as _traceback

class _Logf(object):
    def write(self, s):
        error(s.rstrip())

def print_exc():
    _traceback.print_exc(file=_Logf())

# Default handlers, to be overridden by packages, eg.:
#
# def log_info(s):
#     print time.ctime() + " - trigger: " + s
#     sys.stdout.flush()
#
# seiscomp.logs.info = log_info

def debug(s):
    _sys.stdout.write(s + "\n")
    _sys.stdout.flush()

def info(s):
    _sys.stdout.write(s + "\n")
    _sys.stdout.flush()

def notice(s):
    _sys.stdout.write(s + "\n")
    _sys.stdout.flush()

def warning(s):
    _sys.stdout.write(s + "\n")
    _sys.stdout.flush()

def error(s):
    _sys.stdout.write(s + "\n")
    _sys.stdout.flush()

