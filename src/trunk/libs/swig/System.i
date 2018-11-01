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

%module(package="seiscomp3") System

//
// includes
//
%{
#include "seiscomp3/system/environment.h"
#include "seiscomp3/system/schema.h"
#include "seiscomp3/system/model.h"
#include "seiscomp3/core/array.h"
#include "seiscomp3/core/enumeration.h"
#include "seiscomp3/core/typedarray.h"
#include "seiscomp3/core/record.h"
#include "seiscomp3/core/greensfunction.h"
#include "seiscomp3/core/genericrecord.h"
#include "seiscomp3/core/interruptible.h"
#include "seiscomp3/core/datamessage.h"
#include "seiscomp3/core/status.h"
%}

%exception {
  try {
    $action
  }
  catch ( const Seiscomp::Core::ValueException &e) {
    SWIG_exception(SWIG_ValueError, e.what());
  }
  catch ( const std::exception &e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
  catch ( ... ) {
    SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
  }
}

// 
// typemaps
//
%include <std_string.i>
%include <std_map.i>
%include <std_set.i>
%include <std_vector.i>
%import "Core.i"
%import "../../../system/libs/swig/Config.i"
%include "seiscomp3/core.h"

optional(bool);

%include "seiscomp3/system/environment.h"
%include "seiscomp3/system/schema.h"
%include "seiscomp3/system/model.h"

%template(BindingMap) std::map<Seiscomp::System::StationID, Seiscomp::System::ModuleBindingPtr>;
%template(GroupVector) std::vector<Seiscomp::System::GroupPtr>;
%template(ParameterVector) std::vector<Seiscomp::System::ParameterPtr>;
%template(StructureVector) std::vector<Seiscomp::System::StructurePtr>;
