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

%module(directors="1") Config

//
// includes
//
%{
#include "seiscomp3/config/config.h"
#include "seiscomp3/config/symboltable.h"
%}


%feature("director") Seiscomp::Config::Logger;


// 
// typemaps
//
%include <typemaps.i>
%include <std_string.i>
%include <std_vector.i>

%include "seiscomp3/config/api.h"
%include "seiscomp3/config/log.h"
%include "seiscomp3/config/exceptions.h"

%exception {
  try {
    $action
  }
  catch ( const Seiscomp::Config::OptionNotFoundException &e) {
    SWIG_exception(SWIG_ValueError, e.what());
  }
  catch ( const std::exception &e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
  catch ( ... ) {
    SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
  }
}

%include "seiscomp3/config/symboltable.h"
%include "seiscomp3/config/config.h"

%template(vectorStr) std::vector<std::string>;
%template(vectorInt) std::vector<int>;
%template(vectorDouble) std::vector<double>;
%template(vectorBool) std::vector<bool>;
