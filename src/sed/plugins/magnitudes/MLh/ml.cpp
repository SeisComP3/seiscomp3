/***************************************************************************
 * Copyright (C) 2009 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 *
 * Modifications 2010 - 2011 by Stefan Heimers <heimers@sed.ethz.ch>
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

#define SEISCOMP_COMPONENT MLH

#include "ml.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/processing/amplitudes/MLv.h>
#include <seiscomp3/processing/magnitudeprocessor.h>
#include <seiscomp3/seismology/magnitudes.h>
#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/math/geo.h>

#include <iostream>

#include <boost/bind.hpp>
#include <unistd.h>

// Only valid within 0-20 degrees
#define DELTA_MIN 0.
#define DELTA_MAX 20

// Only valid for depths up to 80 km
#define DEPTH_MAX 80

#define MAG_TYPE "MLh"

using namespace std;

using namespace std;

using namespace Seiscomp;
using namespace Seiscomp::Processing;

namespace {

AmplitudeProcessor::AmplitudeValue average(
	const AmplitudeProcessor::AmplitudeValue &v0,
	const AmplitudeProcessor::AmplitudeValue &v1)
{
	AmplitudeProcessor::AmplitudeValue v;
	// Average both values
	v.value = (v0.value + v1.value) * 0.5;

	// Compute lower and upper uncertainty
	double v0l = v0.value;
	double v0u = v0.value;
	double v1l = v1.value;
	double v1u = v1.value;

	if ( v0.lowerUncertainty ) v0l -= *v0.lowerUncertainty;
	if ( v0.upperUncertainty ) v0u += *v0.upperUncertainty;
	if ( v1.lowerUncertainty ) v1l -= *v1.lowerUncertainty;
	if ( v1.upperUncertainty ) v1u += *v1.upperUncertainty;

	double l = 0, u = 0;

	l = std::max(l, v.value - v0l);
	l = std::max(l, v.value - v0u);
	l = std::max(l, v.value - v1l);
	l = std::max(l, v.value - v1u);

	u = std::max(l, v0l - v.value);
	u = std::max(l, v0u - v.value);
	u = std::max(l, v1l - v.value);
	u = std::max(l, v1u - v.value);

	v.lowerUncertainty = l;
	v.upperUncertainty = u;

	return v;
}


AmplitudeProcessor::AmplitudeTime average(
	const AmplitudeProcessor::AmplitudeTime &t0,
	const AmplitudeProcessor::AmplitudeTime &t1)
{
	AmplitudeProcessor::AmplitudeTime t;
	t.reference = Core::Time((double(t0.reference) + double(t1.reference)) * 0.5);

	// Compute lower and upper uncertainty
	Core::Time t0b = t0.reference + Core::TimeSpan(t0.begin);
	Core::Time t0e = t0.reference + Core::TimeSpan(t0.end);
	Core::Time t1b = t1.reference + Core::TimeSpan(t1.begin);
	Core::Time t1e = t1.reference + Core::TimeSpan(t1.end);

	Core::Time minTime = t.reference;
	Core::Time maxTime = t.reference;

	minTime = std::min(minTime, t0b);
	minTime = std::min(minTime, t0e);
	minTime = std::min(minTime, t1b);
	minTime = std::min(minTime, t1e);

	maxTime = std::max(maxTime, t0b);
	maxTime = std::max(maxTime, t0e);
	maxTime = std::max(maxTime, t1b);
	maxTime = std::max(maxTime, t1e);

	t.begin = (double)(minTime - t.reference);
	t.end = (double)(maxTime - t.reference);

	return t;
}

}


ADD_SC_PLUGIN(
	"MLh magnitude method (max of both horizontal compontents)",
	"gempa GmbH, modified by Stefan Heimers at the SED",
	0, 0, 7
)


class AmplitudeProcessor_MLh : public AmplitudeProcessor_MLv {
	public:
		double ClippingThreshold; // will be set by the setup method in AmplitudeProcessor_ML

		AmplitudeProcessor_MLh() : AmplitudeProcessor_MLv() {
			_type = MAG_TYPE;
			setMinSNR(0);
			setMinDist(DELTA_MIN);
			setMaxDist(DELTA_MAX);
			setMaxDepth(DEPTH_MAX);
		}

	private:
		// Discard clipped signals
		// TODO: test and improve this!
		void fill(size_t n, double *samples) {
			SEISCOMP_DEBUG("AmplitudeProcessor_MLh:fill() was used with limit %f!",ClippingThreshold);
			for ( size_t i = 0; i < n; ++i ) {
				if ( abs(samples[i]) > ClippingThreshold ) {
					setStatus(DataClipped, samples[i]);
					SEISCOMP_DEBUG("AmplitudeProcessor_MLh:fill(): DataClipped at index %ld, value %f",(long int)i,samples[i]);
					break;
				}
			}
			AmplitudeProcessor_MLv::fill(n, samples); // this will apply a filter
		}

		friend class AmplitudeProcessor_ML;
};


class AmplitudeProcessor_ML : public AmplitudeProcessor {
	private:
		MAKEENUM(
			CombinerProc,
			EVALUES(
				// Take max of both amplitudes
				Maximum,
				// Average both amplitudes
				Average,
				// Take min of both amplitudes
				Minimum
			),
			// Names for enum used in config file or database
			ENAMES(
				"max",
				"avg",
				"min"
			)
		);

	public:
		AmplitudeProcessor_ML() : AmplitudeProcessor(MAG_TYPE) {
			setMinSNR(0);
			setMinDist(DELTA_MIN);
			setMaxDist(DELTA_MAX);
			setMaxDepth(DEPTH_MAX);

			setUsedComponent(Horizontal);

			_ampN.setUsedComponent(FirstHorizontal);
			_ampE.setUsedComponent(SecondHorizontal);

			_ampN.setPublishFunction(boost::bind(&AmplitudeProcessor_ML::newAmplitude, this, _1, _2));
			_ampE.setPublishFunction(boost::bind(&AmplitudeProcessor_ML::newAmplitude, this, _1, _2));

			_combiner = Maximum;
		}

		// Method to describe the capabilities for manual analysis
		int capabilities() const {
			return _ampN.capabilities() | Combiner;
		}

		// Returns a value list for a given capability.
		AmplitudeProcessor::IDList
		capabilityParameters(Capability cap) const {
			if ( cap == Combiner ) {
				IDList params;
				params.push_back("Max");
				params.push_back("Average");
				params.push_back("Min");
				return params;
			}

			return _ampN.capabilityParameters(cap);
		}

		// Sets the value for a given capability
		bool setParameter(Capability cap, const string &value) {
			if ( cap == Combiner ) {
				if ( value == "Min" ) {
					_combiner = Minimum;
					return true;
				}
				else if ( value == "Max" ) {
					_combiner = Maximum;
					return true;
				}
				else if ( value == "Average" ) {
					_combiner = Average;
					return true;
				}

				return false;
			}

			_ampN.setParameter(cap, value);
			return _ampE.setParameter(cap, value);
		}


		// Method called by the application to setup the processor.
		// StreamConfigs are expected to be setup already.
		bool setup(const Settings &settings) {
			// Copy the stream configurations (gain, orientation, responses, ...) to
			// the horizontal processors
			_ampN.streamConfig(FirstHorizontalComponent) = streamConfig(FirstHorizontalComponent);
			_ampE.streamConfig(SecondHorizontalComponent) = streamConfig(SecondHorizontalComponent);

			if ( !AmplitudeProcessor::setup(settings) ) return false;

			// Setup each component
			if ( !_ampN.setup(settings) || !_ampE.setup(settings) ) return false;

			// Read the settings variable MLh.maxavg. This can be found in the applications
			// configuration file at:
			// module.trunk.global.MLh.maxavg
			//   or per station (highest priority)
			// module.trunk.CH.AIGLE.MLh.maxavg
			try {
				string s = settings.getString("MLh.maxavg");
				if ( !_combiner.fromString(s.c_str()) ) {
					SEISCOMP_ERROR("MLh: invalid combiner type for station %s.%s: %s",
					               settings.networkCode.c_str(), settings.stationCode.c_str(),
					               s.c_str());
					return false;
				}
			}
			catch ( ... ) {}

			// get the clipping threshold from the config file
			try {
				_ampN.ClippingThreshold=settings.getDouble("MLh.ClippingThreshold");
				_ampE.ClippingThreshold=_ampN.ClippingThreshold;
			}
			catch ( ... ) {
				SEISCOMP_DEBUG("Failed to read MLh.ClippingThreshold from config file, using defaults");
				_ampN.ClippingThreshold=99999999999.0;	// default if not set in config file
				_ampE.ClippingThreshold=_ampN.ClippingThreshold;
			}
			return true;
		}

		// Returns the component processor for a given component. This
		// method is only used for interactive analysis.
		const AmplitudeProcessor *componentProcessor(Component comp) const {
			switch ( comp ) {
			case FirstHorizontalComponent:
				return &_ampN;
			case SecondHorizontalComponent:
				return &_ampE;
			default:
				break;
			}

			return NULL;
		}

		// Returns the processed data array for a given component. This
		// method is only used for interactive analysis.
		const DoubleArray *processedData(Component comp) const {
			switch ( comp ) {
			case FirstHorizontalComponent:
				return _ampN.processedData(comp);
			case SecondHorizontalComponent:
				return _ampE.processedData(comp);
			default:
				break;
			}

			return NULL;
		}


		void setTrigger(const Core::Time &trigger) throw(Core::ValueException) {
			// Set the trigger in 'this' as well to be able to query it
			// correctly from outside.
			AmplitudeProcessor::setTrigger(trigger);
			_ampE.setTrigger(trigger);
			_ampN.setTrigger(trigger);
		}

		void computeTimeWindow() {
			// Copy configuration to each component
			_ampN.setConfig(config());
			_ampE.setConfig(config());

			_ampE.computeTimeWindow();
			_ampN.computeTimeWindow();
			setTimeWindow(_ampE.timeWindow() | _ampN.timeWindow());
		}

		double timeWindowLength(double distance_deg) const {
			double endN = _ampN.timeWindowLength(distance_deg);
			double endE = _ampE.timeWindowLength(distance_deg);
			_ampN.setSignalEnd(endN);
			_ampE.setSignalEnd(endE);
			return std::max(endN, endE);
		}

		void reset() {
			AmplitudeProcessor::reset();

			_results[0] = _results[1] = Core::None;

			_ampE.reset();
			_ampN.reset();
		}

		void close() {
			// TODO: Check for best available amplitude here
		}

		bool feed(const Record *record) {
			// Both processors finished already?
			if ( _ampE.isFinished() && _ampN.isFinished() ) return false;

			// Did an error occur?
			if ( status() > Finished ) return false;

			if ( record->channelCode() == _streamConfig[FirstHorizontalComponent].code() ) {
				if ( !_ampN.isFinished() ) {
					_ampN.feed(record);
					if ( _ampN.status() == InProgress )
						setStatus(WaveformProcessor::InProgress, _ampN.statusValue());
					else if ( _ampN.isFinished() && _ampE.isFinished() ) {
						if ( !isFinished() ) {
							if ( _ampE.status() == Finished )
								setStatus(_ampN.status(), _ampN.statusValue());
							else
								setStatus(_ampE.status(), _ampE.statusValue());
						}
					}
				}
			}
			else if ( record->channelCode() == _streamConfig[SecondHorizontalComponent].code() ) {
				if ( !_ampE.isFinished() ) {
					_ampE.feed(record);
					if ( _ampE.status() == InProgress )
						setStatus(WaveformProcessor::InProgress, _ampE.statusValue());
					else if ( _ampE.isFinished() && _ampN.isFinished() ) {
						if ( !isFinished() ) {
							if ( _ampN.status() == Finished )
								setStatus(_ampE.status(), _ampE.statusValue());
							else
								setStatus(_ampN.status(), _ampN.statusValue());
						}
					}
				}
			}

			return true;
		}


		bool computeAmplitude(const DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt,
		                      AmplitudeValue *amplitude,
		                      double *period, double *snr) {
			return false;
		}

		void reprocess(OPT(double) searchBegin, OPT(double) searchEnd) {
			setStatus(WaitingForData, 0);
			_ampN.setConfig(config());
			_ampE.setConfig(config());

			_results[0] = _results[1] = Core::None;

			_ampN.reprocess(searchBegin, searchEnd);
			_ampE.reprocess(searchBegin, searchEnd);

			if ( !isFinished() ) {
				if ( _ampN.status() > Finished )
					setStatus(_ampN.status(), _ampN.statusValue());
				else
					setStatus(_ampE.status(), _ampE.statusValue());
			}
		}


	private:
		void newAmplitude(const AmplitudeProcessor *proc,
		                  const AmplitudeProcessor::Result &res) {
			if ( isFinished() ) return;

			int idx = 0;

			if ( proc == &_ampE )
				idx = 0;
			else if ( proc == &_ampN )
				idx = 1;

			_results[idx] = ComponentResult();
			_results[idx]->value = res.amplitude;
			_results[idx]->time = res.time;

			if ( _results[0] && _results[1] ) {
				setStatus(Finished, 100.);
				Result newRes;
				newRes.record = res.record;
				newRes.component = Horizontal;

				switch ( _combiner ) {
					case Average:
						// original was calculating the average
						newRes.amplitude = average(_results[0]->value, _results[1]->value);
						newRes.time = average(_results[0]->time, _results[1]->time);
						newRes.component = Horizontal;
						break;
					case Maximum:
						// calculating the max
						if ( _results[0]->value.value >= _results[1]->value.value ) {
							newRes.amplitude =  _results[0]->value;
							newRes.time = _results[0]->time;
							newRes.component = _ampE.usedComponent();
						}
						else {
							newRes.amplitude =  _results[1]->value;
							newRes.time = _results[1]->time;
							newRes.component = _ampN.usedComponent();
						}
						break;
					case Minimum:
						// calculating the min
						if ( _results[0]->value.value <= _results[1]->value.value ) {
							newRes.amplitude =  _results[0]->value;
							newRes.time = _results[0]->time;
							newRes.component = _ampE.usedComponent();
						}
						else {
							newRes.amplitude =  _results[1]->value;
							newRes.time = _results[1]->time;
							newRes.component = _ampN.usedComponent();
						}
						break;
						// Not a correct type? Do nothing.
					default:
						return;
				}

				// zero to peak amplitude
				newRes.amplitude.value *= 0.5;
				newRes.period = -1;
				newRes.snr = -1;
				emitAmplitude(newRes);
			}
		}

	private:
		struct ComponentResult {
			AmplitudeValue value;
			AmplitudeTime  time;
		};

		mutable
		AmplitudeProcessor_MLh _ampE, _ampN;
		CombinerProc           _combiner;
		OPT(ComponentResult)   _results[2];
};



class MagnitudeProcessor_ML : public MagnitudeProcessor {
public:
    struct param_struct {
        double dist;
        double A;
        double B;
        bool nomag;
    };

    list<param_struct> list_of_parametersets;
    param_struct selected_parameterset;

    MagnitudeProcessor_ML() : MagnitudeProcessor(MAG_TYPE) {}


    bool setup(const Settings &settings) {
        try {
            // Read the settings variable MLh.params. This can be found in the applications
            // configuration file at:
            // module.trunk.global.MLh.params
            //   or per station (highest priority)
            // module.trunk.CH.AIGLE.MLh.params
            if ( !initParameters(list_of_parametersets, settings.getString("MLh.params")) )
                return false;
        }
        catch ( ... ) {}

        return true;
    }


    string amplitudeType() const {
        return MAG_TYPE;
    }


    MagnitudeProcessor::Status computeMagnitude(
        double amplitude,   // in milimeters per second
        double period,      // in seconds
        double delta,       // in degrees
        double depth,       // in kilometers
        double &value)
    {
        if ( delta < DELTA_MIN || delta > DELTA_MAX )
            return DistanceOutOfRange;

        if ( depth > DEPTH_MAX )
            return DepthOutOfRange;

        return compute_ML_sed(amplitude, delta, depth, &value);
    }

private:
    // create a C++ list of structs containing all configured
    // parameter sets.
    // run this once at program start
    bool initParameters(list<param_struct> &paramlist, const string &params) {
        string paramset, range_str,minrange_str;
        string A_str, B_str;

        // for each parameter set
        istringstream iss_params(params);
        while ( getline(iss_params,paramset,';') ) {
            // extract the parameters from this parameter set
            istringstream iss_paramset(paramset);
            iss_paramset >> range_str;
            iss_paramset >> A_str;

            param_struct new_paramset;
            if ( !Core::fromString(new_paramset.dist, range_str) ) {
                SEISCOMP_ERROR("MLh: %s: range is not a numeric value",
                               params.c_str());
                return false;
            }

            if ( A_str == "nomag" ) {
                new_paramset.A     = 0;
                new_paramset.B     = 0;
                new_paramset.nomag = true;
            }
            else {
                if ( !Core::fromString(new_paramset.A, A_str) ) {
                    SEISCOMP_ERROR("MLh: %s: not a numeric value",
                                   A_str.c_str());
                    return false;
                }
                iss_paramset >> B_str;
                if ( !Core::fromString(new_paramset.B, B_str) ) {
                    SEISCOMP_ERROR("MLh: %s: not a numeric value",
                                   B_str.c_str());
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
        selected_parameters.nomag=true;
        selected_parameters.dist=0;
        selected_parameters.A=0;
        selected_parameters.B=0;

        list<param_struct>::const_iterator paramit;
        for ( paramit = paramlist.begin(); paramit != paramlist.end(); ++paramit ) {
            if ( (paramit->dist < last_dist) && (paramit->dist >= distance) ) {
                selected_parameters = *paramit;
                last_dist = paramit->dist;
            }
        }
        return selected_parameters;
    }


    MagnitudeProcessor::Status compute_ML_sed(
        double amplitude, // in micrometers
        double delta,     // in degrees
        double depth,     // in kilometers
        double *mag) {

        float epdistkm,hypdistkm;

        if (amplitude <= 0. ) {
            *mag = 0;
            return Error;
        }

        // examples:
        // epdistkm <= 60 km =>    mag=log10(waampl) + 0.018 *epdistkm+1.77 + 0.40;
        // 60 < epdistkm <= 700 => mag=log10(waampl) + 0.0038*epdistkm+2.62 + 0.40;
        //
        // more generic: mag = log10(waampl) + A * epdistkm + B
        // a list of distance ranges and corresponding values for A and B
        // is read from the config file.

        // calculate the distance in kilometers from the distance in degrees
        epdistkm = Math::Geo::deg2km(delta);
        hypdistkm = sqrt(epdistkm*epdistkm + depth * depth);

        // read the values for A, B and epdistkm from the config file and
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
            SEISCOMP_DEBUG("B:     %f", selected_parameterset.B);
            *mag = log10(amplitude)  + selected_parameterset.A * hypdistkm + selected_parameterset.B;
            return OK;
        }
    }
};


REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_ML, MAG_TYPE);
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_ML, MAG_TYPE);
