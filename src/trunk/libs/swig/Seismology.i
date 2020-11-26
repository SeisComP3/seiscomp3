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

%module (package="seiscomp3", docstring="Codes for various seismological computations") Seismology
%{
/* headers to be included in the wrapper code */
#include "seiscomp3/core/typedarray.h"
#include "seiscomp3/core/genericrecord.h"
#include "seiscomp3/core/exceptions.h"
#include "seiscomp3/core/datamessage.h"
#include "seiscomp3/core/greensfunction.h"
#include "seiscomp3/math/geo.h"
#include "seiscomp3/math/coord.h"
#include "seiscomp3/math/math.h"
#include "seiscomp3/math/filter.h"
#include "seiscomp3/math/filter/rmhp.h"
#include "seiscomp3/math/filter/taper.h"
#include "seiscomp3/math/filter/average.h"
#include "seiscomp3/math/filter/stalta.h"
#include "seiscomp3/math/filter/chainfilter.h"
#include "seiscomp3/math/filter/biquad.h"
#include "seiscomp3/math/filter/butterworth.h"
#include "seiscomp3/math/filter/taper.h"
#include "seiscomp3/math/filter/seismometers.h"
#include "seiscomp3/math/restitution/transferfunction.h"
#include "seiscomp3/io/recordstream.h"
#include "seiscomp3/io/recordinput.h"
#include "seiscomp3/io/recordfilter.h"
#include "seiscomp3/io/recordfilter/demux.h"
#include "seiscomp3/io/recordfilter/iirfilter.h"
#include "seiscomp3/io/recordfilter/resample.h"
#include "seiscomp3/io/importer.h"
#include "seiscomp3/io/exporter.h"
#include "seiscomp3/io/gfarchive.h"
#include "seiscomp3/io/archive/binarchive.h"
#include "seiscomp3/io/archive/xmlarchive.h"
#include "seiscomp3/io/records/mseedrecord.h"
#include "seiscomp3/io/recordstream/file.h"
#include "seiscomp3/io/recordstream/slconnection.h"
#include "seiscomp3/io/recordstream/arclink.h"
#include "seiscomp3/io/recordstream/combined.h"

#include "seiscomp3/datamodel/publicobjectcache.h"
#include "seiscomp3/datamodel/messages.h"
#include "seiscomp3/datamodel/databasequery.h"
#include "seiscomp3/datamodel/eventparameters_package.h"
#include "seiscomp3/datamodel/inventory_package.h"
#include "seiscomp3/datamodel/config_package.h"
#include "seiscomp3/datamodel/routing_package.h"

#include "seiscomp3/seismology/regions.h"
#include "seiscomp3/seismology/locator/locsat.h"
#include "seiscomp3/seismology/ttt.h"
%}

%include "std_string.i"
%include "std_list.i"
%include "std_vector.i"

%import "IO.i"
%import "DataModel.i"
%import "../../../system/libs/swig/Config.i"

%template(TravelTimeList_internal) std::list<Seiscomp::TravelTime>;

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

%newobject Seiscomp::TravelTimeTableInterface::Create;
%newobject Seiscomp::TravelTimeTableInterface::compute;
%newobject Seiscomp::TravelTimeTable::compute;

%include "seiscomp3/core.h"
%include "seiscomp3/seismology/regions.h"
%include "seiscomp3/seismology/locatorinterface.h"
%include "seiscomp3/seismology/locator/locsat.h"
%include "seiscomp3/seismology/ttt.h"
