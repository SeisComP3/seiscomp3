/***************************************************************************
 * Copyright (C) 2009 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 *
 * Modifications 2010 - 2011 by Stefan Heimers <heimers@sed.ethz.ch>
 * Modifications 2014 - 2015 by  JeromeSalichon 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/

#define SEISCOMP_COMPONENT MLR

#include "mlr.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/processing/magnitudeprocessor.h>
#include <seiscomp3/math/geo.h>

#include <iostream>

#include <boost/bind.hpp>
#include <unistd.h>

// Only valid within 0-20 degrees
#define DELTA_MIN 0.
#define DELTA_MAX 20

// Only valid for depths up to 800 km
#define DEPTH_MAX 800

#define MAG_TYPE "MLr"

using namespace std;


using namespace Seiscomp;
using namespace Seiscomp::Processing;


ADD_SC_PLUGIN(
	"MLr GNS magnitude, J. Ristau method",
	"Derived from MLh magnitude & Sc3 Tutorial methods, J.Salichon, GNS Science New Zealand, J.Becker, Gempa", 
	0, 0, 1
);


class Magnitude_MLR : public Processing::MagnitudeProcessor {
	public:
		struct param_struct {
			double dist;
			double A;
			bool nomag;
		};

		list<param_struct> list_of_parametersets;
		param_struct selected_parameterset;

		Magnitude_MLR() : Processing::MagnitudeProcessor(MAG_TYPE) {}

		//Same amplitude measurement as MLv (ML Sc3 amplitude on vertical)
		std::string amplitudeType() const {
			return "MLv" ;
		}

		// Do not need to compute another amplitude.

		bool setup(const Settings &settings) {
			// Reset the configuration 
			list_of_parametersets.clear();
			try {
				// Read the settings variable MLr.params. This can be found in the applications
				// configuration file at:
				// module.trunk.global.MLr.params
				//   or per station (highest priority)
				// module.trunk.NZ.WEL.MLr.params
				if ( !initParameters(list_of_parametersets, settings.getString("MLr.params")) )
					return false;
			}
			catch ( ... ) {}

			return true;
		}

		MagnitudeProcessor::Status computeMagnitude(
		      double amplitude,   // in milimeters
		      double period,      // in seconds
		      double delta,       // in degrees
		      double depth,       // in kilometers
		      const DataModel::Origin *, const DataModel::SensorLocation *,
		      double &value)
		{
			if ( delta < DELTA_MIN || delta > DELTA_MAX )
				return DistanceOutOfRange;

			if ( depth > DEPTH_MAX )
				return DepthOutOfRange;

			return compute_ML(amplitude, delta, depth, &value);
		}

	private:
		// create a C++ list of structs containing all configured
		// parameter sets.
		// run this once at program start
		bool initParameters(list<param_struct> &paramlist, const string &params) {
			string paramset, range_str, minrange_str;
			string A_str;

			// for each parameter set
			istringstream iss_params(params);
			while ( getline(iss_params,paramset,';') ) {
				// extract the parameters from this parameter set
				istringstream iss_paramset(paramset);
				iss_paramset >> range_str;
				iss_paramset >> A_str;

				param_struct new_paramset;
				if ( !Core::fromString(new_paramset.dist, range_str) ) {
					SEISCOMP_ERROR("MLr: %s: range is not a numeric value",
					               params.c_str());
					return false;
				}

				if ( A_str == "nomag" ) {
					new_paramset.A     = 0;
					new_paramset.nomag = true;
				}
				else {
					if ( !Core::fromString(new_paramset.A, A_str) ) {
						SEISCOMP_ERROR("MLr: %s: not a numeric value",
						               A_str.c_str());
						return false;
					}

					new_paramset.nomag = false;
				}

				paramlist.push_back(new_paramset);
			}

			return true;
		}


		// select the right parameter set for a given distance. init_parameters() must
		// have been called before using this function.
		param_struct selectParameters(double distance, const list<param_struct> &paramlist) {
			double last_dist = 1000000; // arbitrary number larger than any expected distance;
			param_struct selected_parameters;

			// defaults in case the distance is out of the definded ranges
			selected_parameters.nomag = true;
			selected_parameters.dist = 0;
			selected_parameters.A = 0;

			list<param_struct>::const_iterator paramit;
			for ( paramit = paramlist.begin(); paramit != paramlist.end(); ++paramit ) {
				if ( (paramit->dist < last_dist) && (paramit->dist >= distance) ) {
					selected_parameters = *paramit;
					last_dist = paramit->dist;
				}
			}

			return selected_parameters;
		}

		MagnitudeProcessor::Status compute_ML(
		        double amplitude, // in micrometers
		        double delta,     // in degrees
		        double depth,     // in kilometers
		        double *mag) {
			float epdistkm,hypdistkm;
			float LogAmpREF; //MLr parameters
			float magTemp ; //MLr parameters

			if ( amplitude <= 0. ) {
				*mag = 0;
				return Error;
			}

			// examples:
			// epdistkm <= 60 km =>    mag=log10(waampl) + 0.018 *epdistkm+1.77 ;
			// 60 < epdistkm <= 700 => mag=log10(waampl) + 0.0038*epdistkm+2.62 ;
			// more generic: mag = log10(waampl) + A * epdistkm
			//
			// a list of distance ranges and corresponding values for A
			// is read from a config file.

			// calculate the distance in kilometers from the distance in degrees
			epdistkm = Math::Geo::deg2km(delta);
			hypdistkm = sqrt(epdistkm*epdistkm + depth * depth);

			// read the values for A and epdistkm from the config file and
			// select the right set depending on the distance
			selected_parameterset = selectParameters(hypdistkm, list_of_parametersets);

			SEISCOMP_DEBUG("epdistkm: %f\n",epdistkm);
			SEISCOMP_DEBUG("hypdistkm: %f\n",hypdistkm);

			if ( selected_parameterset.nomag ) {
				SEISCOMP_DEBUG( "epicenter distance out of configured range, no magnitude");
				return DistanceOutOfRange;
			}
			else {
				SEISCOMP_DEBUG("The selected range is: %f", selected_parameterset.dist);
				SEISCOMP_DEBUG("A:     %f", selected_parameterset.A);
				//SEISCOMP_DEBUG("MLr: station %s.%s: %s",settings.networkCode.c_str(), settings.stationCode.c_str(),s.c_str());

				// ORI(sed): *mag = log10(amplitude)  + selected_parameterset.A * hypdistkm + selected_parameterset.B;
				// JRistau: *mag = log10(amplitude) - Log(Amp(hypdistkm)) + Selected_parameterset.A  ;
				//
				// Computing terms of magnitude
				LogAmpREF = 0.2869 - (1.272 * 1e-3) * hypdistkm - (1.493 * log10(hypdistkm) ) ;

				// magTemp = log10(amplitude) - LogAmpREF + StatCorr ;
				// A is  defined as STATION dependent ie. module.trunk.NZ.WEL.MLr.params ;
				// Not corrected stations: Default is 0. Use nomag option to disable; 
				magTemp = log10(amplitude) - LogAmpREF + selected_parameterset.A ;
				*mag = magTemp ;

				SEISCOMP_DEBUG("Mlr:     %f", *mag);
				return OK;
			}
		}
};

REGISTER_MAGNITUDEPROCESSOR(Magnitude_MLR, MAG_TYPE);
