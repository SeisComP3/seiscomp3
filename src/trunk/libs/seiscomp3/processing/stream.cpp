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
		if ( (gainFreq > 0) && (gainFreq != 1.0) && proc_resp ) {
			Math::Restitution::FFT::TransferFunctionPtr tf =
				proc_resp->getTransferFunction();

			if ( tf ) {
				// Compute response at gain frequency and at 1Hz
				Math::Complex r1, r2;
				double targetGainFreq = 1.0;

				tf->evaluate(&r1, 1, &gainFreq);
				tf->evaluate(&r2, 1, &targetGainFreq);

				double scale = abs(r2) / abs(r1);

				if ( scale != 1.0 )
					SEISCOMP_DEBUG("%s.%s.%s.%s: gain correction = %f",
					               stream->sensorLocation()->station()->network()->code().c_str(),
					               stream->sensorLocation()->station()->code().c_str(),
					               stream->sensorLocation()->code().c_str(),
					               stream->code().c_str(),
					               scale);
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
