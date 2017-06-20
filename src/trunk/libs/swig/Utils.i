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

%module Utils

%{
#include "seiscomp3/utils/files.h"
#include "seiscomp3/utils/timer.h"
%}

%include "seiscomp3/core.h"
%include "seiscomp3/utils/files.h"
%include "seiscomp3/utils/timer.h"

%newobject Seiscomp::Util::stringToStreambuf;
%newobject Seiscomp::Util::file2ostream;
%newobject Seiscomp::Util::file2istream;
