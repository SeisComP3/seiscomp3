/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

%module Logging
%{
#include "seiscomp3/logging/output.h"
#include "seiscomp3/logging/log.h"
#include "seiscomp3/logging/channel.h"
#include "seiscomp3/logging/node.h"
#include "seiscomp3/logging/fd.h"
#include "seiscomp3/logging/file.h"
#include "seiscomp3/logging/filerotator.h"
#include "seiscomp3/logging/syslog.h"
%}

%include "seiscomp3/core.h"
%include "seiscomp3/logging/log.h"
%include "seiscomp3/logging/output.h"
%include "seiscomp3/logging/fd.h"
%include "seiscomp3/logging/file.h"
%include "seiscomp3/logging/filerotator.h"
%include "seiscomp3/logging/syslog.h"
