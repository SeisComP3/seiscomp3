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



#define SEISCOMP_COMPONENT Stream

#include <seiscomp3/processing/stream.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/logging/log.h>

#include <iostream>
#include <algorithm>


namespace Seiscomp {
namespace Processing  {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Stream::Stream() {
	gain = 0.0;
	azimuth = 999;
	dip = 999;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::init(const DataModel::Stream *stream) {
	gain = 0.0;
	setCode(stream->code());
	epoch = Core::TimeWindow();
	_sensor = NULL;

	double gainFreq = -1.0;

	epoch.setStartTime(stream->start());
	try { epoch.setEndTime(stream->end()); }
	catch ( ... ) {}

	try {
		gain = stream->gain();
	}
	catch ( ... ) {}

	try {
		gainFreq = stream->gainFrequency();
	}
	catch ( ... ) {}

	gainUnit = stream->gainUnit();
	std::transform(gainUnit.begin(), gainUnit.end(), gainUnit.begin(), ::toupper);

	try {
		azimuth = stream->azimuth();
	}
	catch ( ... ) {
		azimuth = 999;
	}

	try {
		dip = stream->dip();
	}
	catch ( ... ) {
		dip = 999;
	}

	DataModel::Sensor *sensor = DataModel::Sensor::Find(stream->sensor());
	if ( sensor ) {
		SensorPtr proc_sensor = new Sensor;
		proc_sensor->setModel(sensor->model());
		proc_sensor->setManufacturer(sensor->manufacturer());
		proc_sensor->setType(sensor->type());
		proc_sensor->setUnit(sensor->unit());

		try { proc_sensor->setLowFrequency(sensor->lowFrequency()); } catch ( ... ) {}
		try { proc_sensor->setHighFrequency(sensor->highFrequency()); } catch ( ... ) {}

		setSensor(proc_sensor.get());

		DataModel::ResponsePAZ *paz = DataModel::ResponsePAZ::Find(sensor->response());

		if ( paz ) {
			// Only PaZ types A and B are currently supported
			if ( (paz->type() != "A") && (paz->type() != "B") ) {
				SEISCOMP_WARNING("response.type = %s: ignored",
				                 paz->type().c_str());
			}
			else {
				ResponsePAZPtr proc_response = new ResponsePAZ;
				try { proc_response->setNormalizationFactor(paz->normalizationFactor()); } catch ( ... ) {}
				try { proc_response->setNormalizationFrequency(paz->normalizationFrequency()); } catch ( ... ) {}
				try { proc_response->setPoles(paz->poles().content()); } catch ( ... ) {}
				try { proc_response->setZeros(paz->zeros().content()); } catch ( ... ) {}

				if ( paz->type() == "B")
					proc_response->convertFromHz();

				proc_sensor->setResponse(proc_response.get());
			}
		}

		ResponsePtr proc_resp = proc_sensor->response();
		if ( gainFreq > 0 && proc_resp ) {
			Math::Restitution::FFT::TransferFunctionPtr tf =
				proc_resp->getTransferFunction();

			if ( tf ) {
				// Compute response at gain frequency
				Math::Complex r;
				tf->evaluate(&r, 1, &gainFreq);

				double scale = 1.0 / abs(r);

				/*
				std::cerr << networkCode << "." << stationCode << "."
				          << locationCode << "." << channelCode << ": "
				          << "gain scale: " << scale
				          << " at " << gainFreq << "Hz"
				          << std::endl;
				*/

				// NOTE: The following block tries to find the maximum
				//       response and use it as the response of the
				//       plateau. But this doesn't work reliable in praxis
				//       that we rely on a properly configured normalization
				//       factor.
				/*
				// Assume a default sampling frequency of 100 Hz
				double fsamp = 100.0;
				try {
					fsamp = (double)stream->sampleRateNumerator() /
					        (double)stream->sampleRateDenominator();
				}
				catch ( ... ) {}

				double nyquistFreq = fsamp * 0.5;
				double logNyquistFreq = log10(nyquistFreq);
				double lf = -3;
				double maxResp = 0;
				while ( lf < logNyquistFreq ) {
					double f = pow10(lf);
					tf->evaluate(&r, 1, &f);
					double a = abs(r);
					if ( a > maxResp ) maxResp = a;
					lf += 0.1;
				}

				if ( maxResp > 0 ) {
					//std::cerr << " plateau response: " << maxResp << std::endl;
					scale *= maxResp;
				}
				*/

				gain *= scale;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::init(const std::string &networkCode,
                  const std::string &stationCode,
                  const std::string &locationCode,
                  const std::string &channelCode,
                  const Core::Time &time) {
	gain = 0.0;
	setCode(channelCode);
	epoch = Core::TimeWindow();
	_sensor = NULL;

	Client::Inventory *inv = Client::Inventory::Instance();
	if ( !inv ) return;

	DataModel::Stream *stream = inv->getStream(networkCode, stationCode, locationCode, channelCode, time);
	if ( !stream ) return;

	init(stream);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::setCode(const std::string &code) {
	_code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Stream::code() const {
	return _code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Sensor *Stream::sensor() const {
	return _sensor.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::setSensor(Sensor *sensor) {
	_sensor = sensor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::applyGain(int n, double *inout) {
	double scale = 1.0 / gain;
	for ( int i = 0; i < n; ++i )
		inout[i] *= scale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::applyGain(DoubleArray &inout) {
	applyGain(inout.size(), inout.typedData());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::removeGain(int n, double *inout) {
	for ( int i = 0; i < n; ++i )
		inout[i] *= gain;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Stream::removeGain(DoubleArray &inout) {
	removeGain(inout.size(), inout.typedData());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
