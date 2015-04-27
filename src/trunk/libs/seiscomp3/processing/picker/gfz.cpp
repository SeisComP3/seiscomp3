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

#define SEISCOMP_COMPONENT Picker

#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/processing/picker/gfz.h>


using namespace std;

namespace Seiscomp {

namespace Processing {

REGISTER_POSTPICKPROCESSOR(GFZPicker, "GFZ");

namespace {


//
// Repicker using the simple non-AR algorithm of Maeda (1985),
// see paper of Zhang et al. (2003) in BSSA
//

template<typename TYPE>
static void
maeda_aic(int n, TYPE *data, int &kmin, double &snr, int margin=10)
{
	// expects a properly filtered and demeaned trace

	// square trace in place
	for ( int i=0; i<n; ++i ) {
		data[i] = data[i]*data[i];
	}

	// windowed sum for variance computation
	double sumwin1 = 0, sumwin2 = 0, minaic = 0;
	int imin = margin, imax = n-margin;
	for ( int i = 0; i < n; ++i ) {
		if ( i < imin )
			sumwin1 += data[i];
		else
			sumwin2 += data[i];
	}
	
	for ( int k = imin; k < imax; ++k ) {
		double var1 = sumwin1/(k-1),
		       var2 = sumwin2/(n-k-1);
		double aic = k*log10(var1) + (n-k-1)*log10(var2);

		sumwin1 += data[k];
		sumwin2 -= data[k];

		data[k] = aic;
		if ( k == imin ) minaic = aic;
		if ( aic < minaic ) {
			minaic = aic;
			kmin = k;
			snr = var2/var1;
		}
	}

	double maxaic = data[imin] > data[imax-1] ? data[imin] : data[imax-1];

	for ( int k = 0; k < imin; ++k ) {
		data[k] = data[n-k-1] = maxaic;
	}
}


template<typename TYPE>
static void
maeda_aic_const(int n, const TYPE *data, int &kmin, double &snr, int margin=10)
{
	// expects a properly filtered and demeaned trace

	// windowed sum for variance computation
	double sumwin1 = 0, sumwin2 = 0, minaic = 0;
	int imin = margin, imax = n-margin;
	for ( int i = 0; i < n; ++i ) {
		TYPE squared = data[i]*data[i];
		if ( i < imin )
			sumwin1 += squared;
		else
			sumwin2 += squared;
	}
	
	for ( int k = imin; k < imax; ++k ) {
		double var1 = sumwin1/(k-1),
		       var2 = sumwin2/(n-k-1);
		double aic = k*log10(var1) + (n-k-1)*log10(var2);
		TYPE squared = data[k]*data[k];

		sumwin1 += squared;
		sumwin2 -= squared;

		if ( k == imin ) minaic = aic;
		if ( aic < minaic ) {
			minaic = aic;
			kmin = k;
			snr = var2/var1;
		}
	}
}


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GFZPicker::GFZPicker() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GFZPicker::GFZPicker(const Core::Time& trigger)
 : Picker(trigger) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GFZPicker::~GFZPicker() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GFZPicker::setup(const Settings &settings) {
	if ( !Picker::setup(settings) ) return false;

	settings.getValue(_config.signalBegin, "picker.GFZ.signalBegin");
	settings.getValue(_config.signalEnd, "picker.GFZ.signalEnd");
	settings.getValue(_config.snrMin, "picker.GFZ.minSNR");

	_usedFilter.clear();
	settings.getValue(_usedFilter, "picker.GFZ.filter");
	if ( !_usedFilter.empty() ) {
		string error;
		Filter *f = Filter::Create(_usedFilter, &error);
		if ( f == NULL ) {
			SEISCOMP_ERROR("failed to create filter '%s': %s",
			               _usedFilter.c_str(), error.c_str());
			return false;
		}
		setFilter(f);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &GFZPicker::methodID() const {
	static string method = "GFZ";
	return method;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &GFZPicker::filterID() const {
	return _usedFilter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GFZPicker::calculatePick(int ndata, const double *data,
                              int signalStartIndex, int signalEndIndex,
                              int &onsetIndex, int &lowerUncertainty,
                              int &upperUncertainty, double &snr)
// Initially, onsetIndex contains the index of the triggering sample.
{
	const int     n = signalEndIndex - signalStartIndex;
	const double *f = data+signalStartIndex;

	if (n<=10) {
		SEISCOMP_INFO("GFZPicker::calculatePick: not enough data");
		return false;
	}

	// Here we assume that the first third of the seismogram contains only noise.
	// FIXME: somewhat hackish
	int nnoise = n/3;

	// determine offset
	double offset = 0;
	for (int i=0; i<nnoise; i++)
		offset += f[i];
	offset /= nnoise;

	vector<double> tmp(n);
	for (int i=0; i<n; i++)
		tmp[i] = f[i]-offset;

	double fc = 1; // center frequency
	double bw = 2; // bandwidth
	double f1 = fc/sqrt(bw), f2 = fc*sqrt(bw);
//	Math::Filtering::IIR::ButterworthBandpass<double> filter(3, f1, f2, _fsamp);
	Filter *filter = new Math::Filtering::IIR::ButterworthBandpass<double>(3, f1, f2, _stream.fsamp);
//	filter.apply(tmp);
//	filter->apply(tmp);
	delete filter;

	int onset = onsetIndex-signalStartIndex;
	maeda_aic_const(n, &tmp[0], onset, snr);
	if (onset==-1) {
		SEISCOMP_INFO("GFZPicker::calculatePick: no onset found: n=%d fs=%g %g %g %g    %d -> -1", n, _stream.fsamp, _config.signalBegin, _config.signalEnd, offset, onsetIndex-signalStartIndex);
		return false;
	}

	SEISCOMP_INFO("GFZPicker::calculatePick n=%d fs=%g %g %g %g    %d -> %d", n, _stream.fsamp, _config.signalBegin, _config.signalEnd, offset, onsetIndex-signalStartIndex, onset);

	onsetIndex = onset + signalStartIndex;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
