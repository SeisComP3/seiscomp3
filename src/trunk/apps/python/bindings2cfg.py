#!/usr/bin/env seiscomp-python
# -*- coding: utf-8 -*-

############################################################################
#    Copyright (C) by gempa GmbH                                           #
#    Author: Jan Becker <jabe@gempa.de>                                    #
#                                                                          #
#    You can redistribute and/or modify this program under the             #
#    terms of the SeisComP Public License.                                 #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    SeisComP Public License for more details.                             #
############################################################################

import seiscomp3.bindings2cfg
import sys

sys.exit(seiscomp3.bindings2cfg.main())
