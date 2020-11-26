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
#include <cmath>


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
	gainFrequency = Core::None;
	gainUnit = std::string();
	setCode(stream->code());
	epoch = Core::TimeWindow();
	_sensor = NULL;

	epoch.setStartTime(stream->start());
	try { epoch.setEndTime(stream->end()); }
	catch ( ... ) {}

	try {
		gain = stream->gain();
	}
	catch ( ... ) {}

	gainUnit = stream->gainUnit();
	std::transform(gainUnit.begin(), gainUnit.end(), gainUnit.begin(), ::toupper);
	try {
		gainFrequency = stream->gainFrequency();
	}
	catch ( ... ) {}

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
		Math::Restitution::FFT::TransferFunctionPtr tf;
		OPT(double) sensorGainFrequency;

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

				tf = proc_response->getTransferFunction();
			}

			try {
				sensorGainFrequency = paz->gainFrequency();
			}
			catch ( ... ) {}
		}
		else {
			DataModel::ResponseFAP *fap = DataModel::ResponseFAP::Find(sensor->response());
			if ( fap ) {
				ResponseFAPPtr proc_response = new ResponseFAP;

				try {
					sensorGainFrequency = fap->gainFrequency();
				}
				catch ( ... ) {}

				try {
					Math::SeismometerResponse::FAPs faps;

					const std::vector<double> &tuples = fap->tuples().content();
					for ( size_t i = 0; i < tuples.size(); i += 3 )
						faps.push_back(Math::SeismometerResponse::FAP(tuples[i], tuples[i+1], tuples[i+2]));

					std::sort(faps.begin(), faps.end());

					proc_response->setFAPs(faps);
				}
				catch ( ... ) {}

				proc_sensor->setResponse(proc_response.get());

				tf = proc_response->getTransferFunction();
			}
		}

		OPT(double) gainFrequency;

		try {
			gainFrequency = stream->gainFrequency();
		}
		catch ( ... ) {}

		if ( gainFrequency && sensorGainFrequency ) {
			if ( *gainFrequency != *sensorGainFrequency ) {
				SEISCOMP_DEBUG("%s.%s.%s.%s: sensor gain frequency does not match overall gain frequency: %f != %f",
				               stream->sensorLocation()->station()->network()->code().c_str(),
				               stream->sensorLocation()->station()->code().c_str(),
				               stream->sensorLocation()->code().c_str(),
				               stream->code().c_str(),
				               *sensorGainFrequency, *gainFrequency);

				if ( !tf ) {
					SEISCOMP_WARNING("%s.%s.%s.%s: no sensor transfer function available",
					                 stream->sensorLocation()->station()->network()->code().c_str(),
					                 stream->sensorLocation()->station()->code().c_str(),
					                 stream->sensorLocation()->code().c_str(),
					                 stream->code().c_str());
				}
				else {
					Math::Complex overallValue, sensorValue;

					tf->evaluate(&overallValue, 1, &*gainFrequency);
					tf->evaluate(&sensorValue, 1, &*sensorGainFrequency);

					double scale = abs(sensorValue) / abs(overallValue);
					if ( !std::isnormal(scale) ) {
						SEISCOMP_WARNING("%s.%s.%s.%s: invalid correction factor: %f",
						                 stream->sensorLocation()->station()->network()->code().c_str(),
						                 stream->sensorLocation()->station()->code().c_str(),
						                 stream->sensorLocation()->code().c_str(),
						                 stream->code().c_str(),
						                 scale);
					}
					else {
						SEISCOMP_DEBUG("%s.%s.%s.%s: correct gain by factor %f: %f -> %f",
						               stream->sensorLocation()->station()->network()->code().c_str(),
						               stream->sensorLocation()->station()->code().c_str(),
						               stream->sensorLocation()->code().c_str(),
						               stream->code().c_str(),
						               scale, gain, gain*scale);

						gain *= scale;
					}
				}
			}
		}
		else {
			if ( !gainFrequency && !sensorGainFrequency )
				SEISCOMP_WARNING("%s.%s.%s.%s: gain correction disabled: neither overall gain frequency nor sensor gain frequency defined",
				                 stream->sensorLocation()->station()->network()->code().c_str(),
				                 stream->sensorLocation()->station()->code().c_str(),
				                 stream->sensorLocation()->code().c_str(),
				                 stream->code().c_str());
			else if ( !gainFrequency )
				SEISCOMP_WARNING("%s.%s.%s.%s: gain correction disabled: overall gain frequency not defined",
				                 stream->sensorLocation()->station()->network()->code().c_str(),
				                 stream->sensorLocation()->station()->code().c_str(),
				                 stream->sensorLocation()->code().c_str(),
				                 stream->code().c_str());
			else
				SEISCOMP_WARNING("%s.%s.%s.%s: gain correction disabled: sensor gain frequency not defined",
				                 stream->sensorLocation()->station()->network()->code().c_str(),
				                 stream->sensorLocation()->station()->code().c_str(),
				                 stream->sensorLocation()->code().c_str(),
				                 stream->code().c_str());
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
