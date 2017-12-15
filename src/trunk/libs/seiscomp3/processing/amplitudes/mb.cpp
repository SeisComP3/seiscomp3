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



#define SEISCOMP_COMPONENT Amplitudemb

#include <seiscomp3/processing/amplitudes/mb.h>
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

IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_mb, AmplitudeProcessor, "AmplitudeProcessor_mb");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_mb, "mb");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mb::AmplitudeProcessor_mb()
: AmplitudeProcessor("mb") {
	setSignalEnd(30);
	setMinSNR(0);
	setMinDist(5);
	setMaxDist(105);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mb::AmplitudeProcessor_mb(const Core::Time& trigger)
: AmplitudeProcessor(trigger, "mb") {
	setSignalEnd(30);
	setMinSNR(0);
	setMinDist(5);
	setMaxDist(105);
	computeTimeWindow();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_mb::initFilter(double fsamp) {
	AmplitudeProcessor::setFilter(
		new Filtering::IIR::WWSSN_SP_Filter<double>(Velocity)
	);
	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_mb::computeAmplitude(
	const DoubleArray &data,
	size_t i1, size_t i2,
	size_t si1, size_t si2,
	double offset,
	AmplitudeIndex *dt,
	AmplitudeValue *amplitude,
	double *period, double *snr)
{
	const int n = data.size();
	const double *f = static_cast<const double*>(data.data());
	std::vector<double> d(n);

	// We want to find (A/T)_max, so we locate the max. of the derivative
	for (int i=1; i<n-1; i++)
		d[i] = 0.5*(f[i+1] - f[i-1]);
	d[0] = d[n-1] = 0;

	// find the max. amplitude in the *derivative*
	int    imax = find_absmax(n, &d[0], si1, si2, offset);
//	double amax = fabs(f[imax] - offset);
	double pmax = -1; // dominant period around maximum
	double pstd =  0; // standard error of period

	// measure period in the original trace but at the position of the max. amplitude of its *derivative*
	if ( !measure_period(n, f, imax, offset, &pmax, &pstd) )
		pmax = -1;

	// finally relocate the max. amplitude in the original trace at the position of the max. amplitude of its *derivative*
	imax = find_absmax(n, f, imax-(int)pmax, imax+(int)pmax, offset);
	double amax = fabs(f[imax] - offset);

	// string sta = _records->back()->stationCode();
	// cerr << sta << " mb ampl="<<amax<<"  period=" << pmax << "   +/- " << pstd << endl;

#ifdef WRITE_ASCII_FILES_DEBUG
	ofstream of((sta+".asc").c_str());
	for(int i=-1000;i<=1000;i++) {
		if(imax+i<0 || imax+i >n)
			continue;
		of << i << " " << f[imax+i]-offset << endl;
	}
	of.close();
#endif

	// Bei Mwp bestimmt man amax zur Berechnung des SNR. Die eigentliche
	// Amplitude ist aber was anderes! Daher ist SNR-Bestimmung auch
	// magnitudenspezifisch!

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	// SNR check
	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = imax;

	if(pmax>0)
		*period = pmax;

	amplitude->value = amax;

	if ( _streamConfig[_usedComponent].gain != 0.0 )
		amplitude->value /= _streamConfig[_usedComponent].gain;
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// Convert m to nm
	amplitude->value *= 1.E9;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
