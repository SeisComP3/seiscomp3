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


#include <vector> 
#include <algorithm> 

#include <seiscomp3/qc/qcprocessor_spike.h>
#include <seiscomp3/math/filter/butterworth.h>


namespace Seiscomp {
namespace Processing {

	
IMPLEMENT_SC_CLASS_DERIVED(QcProcessorSpike, QcProcessor, "QcProcessorSpike");


QcProcessorSpike::QcProcessorSpike() : QcProcessor() {
	_initFilter = false; // should we use filtered data ?
}

QcProcessorSpike::~QcProcessorSpike() {}

bool QcProcessorSpike::feed(const Record *record) {
	if (_initFilter)
		_setFilter(record->samplingFrequency());

	return WaveformProcessor::feed(record);
}

void QcProcessorSpike::_setFilter(double fsamp) {
	_initFilter = false;

	Math::Filtering::InPlaceFilter<double>* f =
		new Math::Filtering::IIR::ButterworthHighpass<double>(2, (fsamp / 2.0) * 0.8);

	setFilter(f);
	initFilter(fsamp);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//! *** P R E L I M I N A R Y ***
//! spike finder test --> TODO replace with better one...
bool QcProcessorSpike::setState(const Record *rec, const DoubleArray &data) {
	int size = data.size();
	double fsamp = rec->samplingFrequency();

	if (size < 3 || fsamp <= 0.0)
		return false;

	Spikes spikes;

	//! rms and mean from filtered data
	double mean = data.mean();
	double rms = data.rms(mean);

	double p1, p2;
	int last_i = (int)(-fsamp/2 - 1);

	for ( int i = 0; i < size; i++ ) {

		if (i != 0) _stream.lastSample = data[i-1];

		if (i < size -1) {
			p1 = (_stream.lastSample-mean) - (data[i]-mean);
			p2 = (data[i]-mean) - (data[i+1]-mean);
		}
		else p1 = p2 = 0.0;

		//! use heuristic limits ...
		if (p1*p2 < -1e6 && (data[i]-mean) > 5.0*rms &&(i - last_i) > (int)(fsamp/2)) {
			spikes[rec->startTime() + Core::TimeSpan((double)(i/fsamp))] = data[i];
			last_i = i;
		}
	}

	_stream.lastSample = data[size-1];

	if (!spikes.empty()) {
		_qcp->parameter = spikes;
		return true;
	}

	return false;
}

QcProcessorSpike::Spikes QcProcessorSpike::getSpikes(){
	try {
		return boost::any_cast<Spikes>(_qcp->parameter);
	}
	catch (const boost::bad_any_cast &) {
		throw Core::ValueException("no data");
	}
}


}
}
