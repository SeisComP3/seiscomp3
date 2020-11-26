/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/



#define SEISCOMP_COMPONENT Amplitudemsbb

#include <seiscomp3/processing/amplitudes/msbb.h>
#include <seiscomp3/math/filter/seismometers.h>

#include <limits>


using namespace Seiscomp::Math;


namespace {

bool measure_period(int n, const double *f, int i0, double offset,
                    double *per, double *std) {

	//  Measures the period of an approximately sinusoidal signal f about
	//  the sample with index i0. It does so by measuring the zero
	//  crossings of the signal as well as the position of its extrema.
	//
	//  To be improved e.g. by using splines

	// TODO offset!
	int ip1, ip2, in1, in2;

	double f0 = f[i0];

// ******* find zero crossings **********************************

	// first previous
	for (ip1=i0;   ip1>=0 && (f[ip1]-offset)*f0 >= 0;  ip1--);
	// second previous
	for (ip2=ip1;  ip2>=0 && (f[ip2]-offset)*f0 <  0;  ip2--);

	// first next
	for (in1=i0;   in1<n  && (f[in1]-offset)*f0 >= 0;  in1++);
	// second next
	for (in2=in1;  in2<n  && (f[in2]-offset)*f0 <  0;  in2++);

	double wt = 0, pp = 0;

	// for computing the standard deviation, we need:
	double m[5];
	int nm=0;

	if (ip2>=0) {
		wt += 0.5;
		pp += 0.5*(ip1 -ip2);
		m[nm++] = ip1 -ip2;

		int imax = find_absmax(n, f, ip2, ip1, offset);
		wt += 1;
		pp += i0 -imax;
		m[nm++] = i0 -imax;
	}
	if (ip1>=0 && in1<n) {
		wt += 1;
		pp += in1 -ip1;
		m[nm++] = in1 -ip1;
	}
	if (in2<n) {
		wt += 0.5;
		pp += 0.5*(in2 -in1);
		m[nm++] = in2 -in1;

		int imax = find_absmax(n, f, in1, in2, offset);
		wt += 1;
		pp += imax - i0;
		m[nm++] = imax - i0;

	}

	// compute standard deviation of period
	if (nm>=3) {
		double avg = 0, sum = 0;
		for(int i=0; i<nm; i++) avg += m[i];
		avg /= nm;
		for(int i=0; i<nm; i++) sum += (m[i]-avg)*(m[i]-avg);
		*std = 2*sqrt(sum/(nm-1));
	}
	else    *std = 0;


	if (wt < 0.9) return false;

	*per = 2*pp/wt;

	return true;
}

}


namespace Seiscomp {

namespace Processing {

IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_msbb, AmplitudeProcessor, "AmplitudeProcessor_msbb");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_msbb, "Ms(BB)");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_msbb::AmplitudeProcessor_msbb()
: AmplitudeProcessor("Ms(BB)") {
	setSignalEnd(3600.);
	setMinSNR(0);
	setMinDist(2);
	setMaxDist(160);
	setMaxDepth(100);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_msbb::AmplitudeProcessor_msbb(const Core::Time& trigger, double duration)
: AmplitudeProcessor(trigger, "Ms(BB)") {
	setSignalEnd(3600.);
	setMinSNR(0);
	setMinDist(2);
	setMaxDist(160);
	setMaxDepth(100);
	computeTimeWindow();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_msbb::computeAmplitude(const DoubleArray &data,
                                               size_t i1, size_t i2,
                                               size_t si1, size_t si2, double offset,
                                               AmplitudeIndex *dt,
                                               AmplitudeValue *amplitude,
                                               double *period, double *snr) {
	/*
	* Low-level signal amplitude computation. This is magnitude specific.
	*
	* Input:
	*      f           double array of length n
	*      i1,i2       indices defining the measurement window,
	*                  0 <= i1 < i2 <= n
	*      offset      this is subtracted from the samples in f before
	*                  computation
	*
	* Output:
	*      dt          Point at which the measurement was mad/completed. May
	*                  be the peak time or end of integration.
	*      amplitude   amplitude. This may be a peak amplitude as well as a
	*                  sum or integral.
	*      period      dominant period of the signal. Optional. If the period
	*                  is not computed, set it to -1.
	*/

	size_t imax = find_absmax(data.size(), (const double*)data.data(), si1, si2, offset);
	double amax = fabs(data[imax] - offset);
	double pmax = -1;
	double pstd =  0; // standard error of period
	if ( !measure_period(data.size(), static_cast<const double*>(data.data()), imax, offset, &pmax, &pstd) )
		pmax = -1;

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = imax;
	*period = pmax;
	amplitude->value = amax;

	if ( _usedComponent <= SecondHorizontal ) {
		if ( _streamConfig[_usedComponent].gain != 0.0 )
			amplitude->value /= _streamConfig[_usedComponent].gain;
		else {
			setStatus(MissingGain, 0.0);
			return false;
		}
	}
	else
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double AmplitudeProcessor_msbb::timeWindowLength(double distance_deg) const {
	// Minimal S/SW group velocity.
	//
	// This is very approximate and may need refinement. Usually the Lg
	// group velocity is around 3.2-3.6 km/s. By setting v_min to 3 km/s,
	// we are probably on the safe side. We add 30 s to coount for rupture
	// duration, which may, however, nit be sufficient.
	double v_min = 3.5;

	double distance_km = distance_deg*111.2; 
	double windowLength = distance_km/v_min + 30;  
	return windowLength < _config.signalEnd ? windowLength :_config.signalEnd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
