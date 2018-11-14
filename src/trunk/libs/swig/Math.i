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

%module(package="seiscomp3", docstring="Codes for various geographical computations and filters") Math
%{
/* headers to be included in the wrapper code */
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
%import "Core.i"
%feature ("autodoc", "1");
%include "seiscomp3/core.h"

namespace std {
   %template(vectorf) vector<float>;
   %template(vectord) vector<double>;
};

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

%apply double *OUTPUT { double *out_dist,  double *out_azi1, double *out_azi2 };
%apply double *OUTPUT { double *out_lat,   double *out_lon };
%apply double *OUTPUT { double *out_latx1, double *out_lonx1,
                        double *out_latx2, double *out_lonx2 };
%apply double *OUTPUT { double *dist, double *azi };

%ignore nearestHotspot(double, double, double, int, const NamedCoordD*, double *, double *);

%include "seiscomp3/math/math.h"
%include "seiscomp3/math/coord.h"

%template(CoordF) Seiscomp::Math::Geo::Coord<float>;
%template(CoordD) Seiscomp::Math::Geo::Coord<double>;

%template(NamedCoordF) Seiscomp::Math::Geo::NamedCoord<float>;
%template(NamedCoordD) Seiscomp::Math::Geo::NamedCoord<double>;

%template(HotspotListF) std::vector<Seiscomp::Math::Geo::NamedCoordF>;
%template(HotspotListD) std::vector<Seiscomp::Math::Geo::NamedCoordD>;

%template(CityF) Seiscomp::Math::Geo::City<float>;
%template(CityD) Seiscomp::Math::Geo::City<double>;

%template(CityListF) std::vector<Seiscomp::Math::Geo::CityF>;
%template(CityListD) std::vector<Seiscomp::Math::Geo::CityD>;

%include "seiscomp3/math/filter.h"

%template(InPlaceFilterF) Seiscomp::Math::Filtering::InPlaceFilter<float>;
%template(InPlaceFilterD) Seiscomp::Math::Filtering::InPlaceFilter<double>;

%include "seiscomp3/math/filter/average.h"

%template(AverageFilterF) Seiscomp::Math::Filtering::Average<float>;
%template(AverageFilterD) Seiscomp::Math::Filtering::Average<double>;

%include "seiscomp3/math/filter/stalta.h"

%template(STALTAFilterF) Seiscomp::Math::Filtering::STALTA<float>;
%template(STALTAFilterD) Seiscomp::Math::Filtering::STALTA<double>;

%include "seiscomp3/math/filter/rmhp.h"

%template(RunningMeanFilterF) Seiscomp::Math::Filtering::RunningMean<float>;
%template(RunningMeanFilterD) Seiscomp::Math::Filtering::RunningMean<double>;

%template(RunningMeanHighPassFilterF) Seiscomp::Math::Filtering::RunningMeanHighPass<float>;
%template(RunningMeanHighPassFilterD) Seiscomp::Math::Filtering::RunningMeanHighPass<double>;

%include "seiscomp3/math/filter/taper.h"

%template(InitialTaperFilterF) Seiscomp::Math::Filtering::InitialTaper<float>;
%template(InitialTaperFilterD) Seiscomp::Math::Filtering::InitialTaper<double>;

%include "seiscomp3/math/filter/biquad.h"

%template(BiquadCascadeF) Seiscomp::Math::Filtering::IIR::BiquadCascade<float>;
%template(BiquadCascadeD) Seiscomp::Math::Filtering::IIR::BiquadCascade<double>;

%include "seiscomp3/math/filter/butterworth.h"

%template(ButterworthLowpassF) Seiscomp::Math::Filtering::IIR::ButterworthLowpass<float>;
%template(ButterworthLowpassD) Seiscomp::Math::Filtering::IIR::ButterworthLowpass<double>;

%template(ButterworthHighpassF) Seiscomp::Math::Filtering::IIR::ButterworthHighpass<float>;
%template(ButterworthHighpassD) Seiscomp::Math::Filtering::IIR::ButterworthHighpass<double>;

%template(ButterworthBandpassF) Seiscomp::Math::Filtering::IIR::ButterworthBandpass<float>;
%template(ButterworthBandpassD) Seiscomp::Math::Filtering::IIR::ButterworthBandpass<double>;

%include "seiscomp3/math/filter/chainfilter.h"

%apply SWIGTYPE *DISOWN { Seiscomp::Math::Filtering::InPlaceFilter<float> *filter };
%apply SWIGTYPE *DISOWN { Seiscomp::Math::Filtering::InPlaceFilter<double> *filter };

%newobject Seiscomp::Math::Filtering::ChainFilter<float>::take;
%newobject Seiscomp::Math::Filtering::ChainFilter<double>::take;

%template(ChainFilterF) Seiscomp::Math::Filtering::ChainFilter<float>;
%template(ChainFilterD) Seiscomp::Math::Filtering::ChainFilter<double>;

%newobject Seiscomp::Math::Filtering::InplaceFilter<float>::Create;
%newobject Seiscomp::Math::Filtering::InplaceFilter<double>::Create;

%include "seiscomp3/math/filter/seismometers.h"

%template(WWSSN_SPF) Seiscomp::Math::Filtering::IIR::WWSSN_SP_Filter<float>;
%template(WWSSN_SPD) Seiscomp::Math::Filtering::IIR::WWSSN_SP_Filter<double>;

%include "seiscomp3/math/geo.h"

%template(deg2km) Seiscomp::Math::Geo::deg2km<double>;

%rename(TransferFunctionPAZ) Seiscomp::Math::Restitution::FFT::PolesAndZeros;

%include "seiscomp3/math/restitution/types.h"
%include "seiscomp3/math/restitution/transferfunction.h"

%template(vectorc) std::vector<Seiscomp::Math::Complex>;
