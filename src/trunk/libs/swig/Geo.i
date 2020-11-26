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

%module (package="seiscomp3", docstring="Codes for working with geo features (e.g. polygons)") Geo
%{
/* headers to be included in the wrapper code */
#include "seiscomp3/geo/coordinate.h"
#include "seiscomp3/geo/boundingbox.h"
#include "seiscomp3/geo/feature.h"
#include "seiscomp3/geo/featureset.h"

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

#include "seiscomp3/core/typedarray.h"
#include "seiscomp3/core/record.h"
#include "seiscomp3/core/greensfunction.h"
#include "seiscomp3/core/genericrecord.h"
#include "seiscomp3/core/genericmessage.h"
#include "seiscomp3/core/datamessage.h"
#include "seiscomp3/core/interruptible.h"
%}


%include "stl.i"
%include "std_vector.i"
%import "Math.i"
%feature ("autodoc", "1");
%include "seiscomp3/core.h"

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

%include "seiscomp3/geo/coordinate.h"
%include "seiscomp3/geo/boundingbox.h"
%include "seiscomp3/geo/feature.h"
%include "seiscomp3/geo/featureset.h"

%template(Categories) std::vector<Seiscomp::Geo::Category*>;
%template(GeoFeatures) std::vector<Seiscomp::Geo::GeoFeature*>;
%template(Vertices) std::vector<Seiscomp::Geo::GeoCoordinate>;
%template(Indexes) std::vector<size_t>;
