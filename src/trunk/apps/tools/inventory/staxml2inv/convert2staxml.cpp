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


#define SEISCOMP_COMPONENT STAXML
#include "convert2staxml.h"

#include <stationxml/stamessage.h>
#include <stationxml/network.h>
#include <stationxml/station.h>
#include <stationxml/stationepoch.h>
#include <stationxml/channel.h>
#include <stationxml/channelepoch.h>
#include <stationxml/response.h>
#include <stationxml/coefficients.h>
#include <stationxml/fir.h>
#include <stationxml/numeratorcoefficient.h>
#include <stationxml/polynomial.h>
#include <stationxml/polynomialcoefficient.h>
#include <stationxml/polesandzeros.h>
#include <stationxml/poleandzero.h>

#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/logging/log.h>

#include <iostream>

using namespace std;


#define UNDEFINED   ""
#define ACCEL1      "M/S**2"
#define ACCEL2      "M/S2"
#define VELOCITY    "M/S"
#define DISPLACE    "M"
#define CURRENT     "V"
#define DIGITAL     "COUNTS"
#define TEMPERATURE "C"
#define PRESSURE    "PA"


namespace Seiscomp {

namespace {

StationXML::Network *findNetwork(StationXML::StaMessage *msg,
                                 const string &code, const Core::Time &start) {
	for ( size_t i = 0; i < msg->networkCount(); ++i ) {
		StationXML::Network *net = msg->network(i);
		if ( net->code() == code && net->start() == start )
			return net;
	}

	return NULL;
}

StationXML::Station *findStation(StationXML::Network *net,
                                 const string &code) {
	for ( size_t i = 0; i < net->stationCount(); ++i ) {
		StationXML::Station *sta = net->station(i);
		if ( sta->code() == code ) return sta;
	}

	return NULL;
}

StationXML::StationEpoch *findEpoch(StationXML::Station *sta,
                                    const Core::Time &start) {
	for ( size_t i = 0; i < sta->epochCount(); ++i ) {
		StationXML::StationEpoch *epoch = sta->epoch(i);
		if ( epoch->start() == start ) return epoch;
	}

	return NULL;
}

StationXML::Channel *findChannel(StationXML::StationEpoch *epoch,
                                 const string &loccode, const string &code) {
	for ( size_t i = 0; i < epoch->channelCount(); ++i ) {
		StationXML::Channel *chan = epoch->channel(i);
		if ( chan->code() == code && chan->locationCode() == loccode )
			return chan;
	}

	return NULL;
}

StationXML::ChannelEpoch *findEpoch(StationXML::Channel *chan,
                                    const Core::Time &start) {
	for ( size_t i = 0; i < chan->epochCount(); ++i ) {
		StationXML::ChannelEpoch *epoch = chan->epoch(i);
		if ( epoch->start() == start ) return epoch;
	}

	return NULL;
}


DataModel::Datalogger *findDatalogger(const DataModel::Inventory *inv,
                                      const string &publicID) {
	for ( size_t i = 0; i < inv->dataloggerCount(); ++i ) {
		DataModel::Datalogger *d = inv->datalogger(i);
		if ( d->publicID() == publicID )
			return d;
	}

	return NULL;
}


DataModel::Sensor *findSensor(const DataModel::Inventory *inv,
                              const string &publicID) {
	for ( size_t i = 0; i < inv->sensorCount(); ++i ) {
		DataModel::Sensor *s = inv->sensor(i);
		if ( s->publicID() == publicID )
			return s;
	}

	return NULL;
}


DataModel::ResponseFIR *findFIR(const DataModel::Inventory *inv,
                                const string &publicID) {
	for ( size_t i = 0; i < inv->responseFIRCount(); ++i ) {
		DataModel::ResponseFIR *fir = inv->responseFIR(i);
		if ( fir->publicID() == publicID )
			return fir;
	}

	return NULL;
}


DataModel::ResponsePAZ *findPAZ(const DataModel::Inventory *inv,
                                const string &publicID) {
	for ( size_t i = 0; i < inv->responsePAZCount(); ++i ) {
		DataModel::ResponsePAZ *paz = inv->responsePAZ(i);
		if ( paz->publicID() == publicID )
			return paz;
	}

	return NULL;
}


DataModel::ResponsePolynomial *findPoly(const DataModel::Inventory *inv,
                                        const string &publicID) {
	for ( size_t i = 0; i < inv->responsePolynomialCount(); ++i ) {
		DataModel::ResponsePolynomial *poly = inv->responsePolynomial(i);
		if ( poly->publicID() == publicID )
			return poly;
	}

	return NULL;
}


// SC3 FIR responses are converted to StationXML FIR responses to
// use optimizations (symmetry) though all StationXML files in the
// wild are using Coefficient responses. This can be changed on
// request with the penalty of resolving the symmetry here.
StationXML::ResponsePtr convert(const DataModel::ResponseFIR *fir) {
	double gain = 0;
	try { gain = fir->gain(); } catch ( ... ) {}

	StationXML::FrequencyType freq;
	StationXML::FloatType ft;
	StationXML::ResponsePtr sx_resp = new StationXML::Response;

	freq.setValue(0);
	ft.setValue(0);

	sx_resp->stageSensitivity().setSensitivityValue(gain);
	sx_resp->stageSensitivity().setSensitivityUnits(DIGITAL);
	sx_resp->stageSensitivity().setFrequency(freq);

	sx_resp->setDecimation(StationXML::Decimation());

	try { sx_resp->decimation().setFactor(fir->decimationFactor()); }
	catch ( ... ) { sx_resp->decimation().setFactor(0); }

	sx_resp->decimation().setOffset(0);

	try { ft.setValue(fir->delay()); }
	catch ( ... ) { ft.setValue(0); }
	sx_resp->decimation().setDelay(ft);

	try { ft.setValue(fir->correction()); }
	catch ( ... ) { ft.setValue(0); }
	sx_resp->decimation().setCorrection(ft);

	// Update it later
	freq.setValue(0);
	sx_resp->decimation().setInputSampleRate(freq);

	StationXML::FIRPtr sx_fir = new StationXML::FIR;
	sx_fir->setResponseName(fir->name());

	if ( fir->symmetry() == "A" )
		sx_fir->setSymmetry(StationXML::SymmetryType(StationXML::ST_NONE));
	else if ( fir->symmetry() == "B" )
		sx_fir->setSymmetry(StationXML::SymmetryType(StationXML::ST_ODD));
	else if ( fir->symmetry() == "C" )
		sx_fir->setSymmetry(StationXML::SymmetryType(StationXML::ST_EVEN));

	sx_fir->setInputUnits(DIGITAL);
	sx_fir->setOutputUnits(DIGITAL);

	try {
		int idx = 0;
		const vector<double> &coeff = fir->coefficients().content();
		for ( size_t c = 0; c < coeff.size(); ++c ) {
			StationXML::NumeratorCoefficientPtr fc = new StationXML::NumeratorCoefficient;
			fc->setI(idx++);
			fc->setValue(coeff[c]);
			sx_fir->addNumeratorCoefficient(fc.get());
		}
	}
	catch ( ... ) {}

	sx_resp->addFIR(sx_fir.get());

	return sx_resp;
}


StationXML::ResponsePtr convert(const DataModel::ResponsePAZ *paz,
                                const std::string &inputUnit,
                                const std::string &outputUnit) {
	StationXML::ResponsePtr sx_resp = new StationXML::Response;
	try { sx_resp->stageSensitivity().setSensitivityValue(paz->gain()); }
	catch ( ... ) { sx_resp->stageSensitivity().setSensitivityValue(0); }

	StationXML::FrequencyType freq;

	try {
		freq.setValue(paz->gainFrequency());
		sx_resp->stageSensitivity().setFrequency(freq);
	}
	catch ( ... ) {}

	sx_resp->stageSensitivity().setSensitivityUnits(inputUnit);

	StationXML::PolesAndZerosPtr sx_paz = new StationXML::PolesAndZeros;
	try { sx_paz->setNormalizationFactor(paz->normalizationFactor()); }
	catch ( ... ) { sx_paz->setNormalizationFactor(0); }

	try { freq.setValue(paz->normalizationFrequency()); }
	catch ( ... ) { freq.setValue(0); }
	sx_paz->setNormalizationFreq(freq);

	sx_paz->setInputUnits(inputUnit);
	sx_paz->setOutputUnits(outputUnit);

	if ( paz->type() == "A" )
		sx_paz->setPzTransferFunctionType(StationXML::PZTFT_LAPLACE_RAD);
	else if ( paz->type() == "B" )
		sx_paz->setPzTransferFunctionType(StationXML::PZTFT_LAPLACE_HZ);
	else if ( paz->type() == "D" )
		sx_paz->setPzTransferFunctionType(StationXML::PZTFT_DIGITAL_Z_TRANSFORM);
	else
		sx_paz->setPzTransferFunctionType(StationXML::PZTFT_LAPLACE_RAD);

	int idx = 0;
	try {
		const vector< complex<double> > &poles = paz->poles().content();
		for ( size_t i = 0; i < poles.size(); ++i ) {
			StationXML::PoleAndZeroPtr pole = new StationXML::PoleAndZero;
			pole->setNumber(idx++);
			pole->setReal(poles[i].real());
			pole->setImaginary(poles[i].imag());
			sx_paz->addPole(pole.get());
		}
	}
	catch ( ... ) {}

	try {
		const vector< complex<double> > &zeros = paz->zeros().content();
		for ( size_t i = 0; i < zeros.size(); ++i ) {
			StationXML::PoleAndZeroPtr zero = new StationXML::PoleAndZero;
			zero->setNumber(idx++);
			zero->setReal(zeros[i].real());
			zero->setImaginary(zeros[i].imag());
			sx_paz->addZero(zero.get());
		}
	}
	catch ( ... ) {}

	sx_resp->addPolesAndZeros(sx_paz.get());
	return sx_resp;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Convert2StaXML::Convert2StaXML(StationXML::StaMessage *msg)
: _msg(msg), _inv(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2StaXML::push(const DataModel::Inventory *inv) {
	if ( _msg == NULL ) return false;
	_inv = inv;

	_dataloggerLookup.clear();
	for ( size_t i = 0; i < inv->dataloggerCount(); ++i ) {
		DataModel::Datalogger *d = inv->datalogger(i);
		_dataloggerLookup[d->publicID()] = d;
	}

	_sensorLookup.clear();
	for ( size_t i = 0; i < inv->sensorCount(); ++i ) {
		DataModel::Sensor *s = inv->sensor(i);
		_sensorLookup[s->publicID()] = s;
	}

	_firLookup.clear();
	for ( size_t i = 0; i < inv->responseFIRCount(); ++i ) {
		DataModel::ResponseFIR *r = inv->responseFIR(i);
		_firLookup[r->publicID()] = r;
	}

	_pazLookup.clear();
	for ( size_t i = 0; i < inv->responsePAZCount(); ++i ) {
		DataModel::ResponsePAZ *r = inv->responsePAZ(i);
		_pazLookup[r->publicID()] = r;
	}

	_polyLookup.clear();
	for ( size_t i = 0; i < inv->responsePolynomialCount(); ++i ) {
		DataModel::ResponsePolynomial *r = inv->responsePolynomial(i);
		_polyLookup[r->publicID()] = r;
	}

	for ( size_t n = 0; n < inv->networkCount(); ++n ) {
		if ( _interrupted ) break;

		DataModel::Network *net = inv->network(n);

		SEISCOMP_DEBUG("Processing network %s (%s)",
		               net->code().c_str(), net->start().toString("%F %T").c_str());

		StationXML::NetworkPtr sx_net;
		sx_net = findNetwork(_msg, net->code(), net->start());
		if ( sx_net == NULL ) {
			sx_net = new StationXML::Network;
			sx_net->setCode(net->code());
			sx_net->setStart(StationXML::DateTime(net->start()));
			_msg->addNetwork(sx_net.get());
		}

		try { sx_net->setEnd(StationXML::DateTime(net->end())); }
		catch ( ... ) { sx_net->setEnd(Core::None); }

		sx_net->setDescription(net->description());
		// Leave out totalNumberOfStations because we don't know if the
		// inventory is complete or filtered.
		// SelectedNumberOfStations is updated at the end to reflect the
		// numbers of stations added to this network in this run

		for ( size_t s = 0; s < net->stationCount(); ++s ) {
			DataModel::Station *sta = net->station(s);
			process(sx_net.get(), sta);
		}
	}

	_inv = NULL;

	return !_interrupted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2StaXML::process(StationXML::Network *sx_net,
                          const DataModel::Station *sta) {
	StationXML::StationPtr sx_sta = findStation(sx_net, sta->code());
	// Create station object
	if ( sx_sta == NULL ) {
		sx_sta = new StationXML::Station;
		sx_sta->setNetCode(sx_net->code());
		sx_sta->setCode(sta->code());
		sx_net->addStation(sx_sta.get());
	}

	SEISCOMP_DEBUG("Processing station %s/%s (%s)",
	               sx_net->code().c_str(), sta->code().c_str(),
	               sta->start().toString("%F %T").c_str());

	// Update station object
	sx_sta->setAgencyCode(sta->affiliation());

	StationXML::StationEpochPtr sx_staepoch = findEpoch(sx_sta.get(), sta->start());
	// Create station epoch
	if ( sx_staepoch == NULL ) {
		sx_staepoch = new StationXML::StationEpoch;
		sx_staepoch->setStart(sta->start());
		sx_sta->addEpoch(sx_staepoch.get());
	}

	sx_staepoch->setCreationDate(sx_staepoch->start());

	try { sx_staepoch->setEnd(StationXML::DateTime(sta->end())); }
	catch ( ... ) { sx_staepoch->setEnd(Core::None); }

	StationXML::LatType lat;
	StationXML::LonType lon;
	StationXML::DistanceType elev;
	try { lat.setValue(sta->latitude()); } catch ( ... ) { lat.setValue(0); }
	try { lon.setValue(sta->longitude()); } catch ( ... ) { lon.setValue(0); }
	try { elev.setValue(sta->elevation()); } catch ( ... ) { elev.setValue(0); }

	sx_staepoch->setLatitude(lat);
	sx_staepoch->setLongitude(lon);
	sx_staepoch->setElevation(elev);

	// Site is mandatory
	StationXML::Site site;
	if ( !sta->country().empty() )
		site.setCountry(sta->country());
	else if ( !sta->description().empty() )
		site.setCountry(sta->description());
	else
		site.setCountry(sx_net->code());
	site.setName(sx_net->code() + "-" + sx_sta->code());
	site.setDescription(sta->description());
	sx_staepoch->setSite(site);

	for ( size_t l = 0; l < sta->sensorLocationCount(); ++l ) {
		if ( _interrupted ) break;
		DataModel::SensorLocation *loc = sta->sensorLocation(l);
		for ( size_t s = 0; s < loc->streamCount(); ++s ) {
			if ( _interrupted ) break;
			DataModel::Stream *stream = loc->stream(s);
			process(sx_staepoch.get(), loc, stream);
		}
	}

	return !_interrupted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2StaXML::process(StationXML::StationEpoch *sx_staepoch,
                             const DataModel::SensorLocation *loc,
                             const DataModel::Stream *stream) {
	StationXML::ChannelPtr sx_chan = findChannel(sx_staepoch, loc->code(), stream->code());
	if ( sx_chan == NULL ) {
		sx_chan = new StationXML::Channel;
		sx_chan->setCode(stream->code());
		sx_chan->setLocationCode(loc->code());
		sx_staepoch->addChannel(sx_chan.get());
	}

	sx_chan->setCreationDate(stream->start());

	try { sx_chan->setRestricted(stream->restricted()?1:0); }
	catch ( ...) { sx_chan->setRestricted(Core::None); }

	StationXML::ChannelEpochPtr sx_epoch = findEpoch(sx_chan.get(), stream->start());
	if ( sx_epoch == NULL ) {
		sx_epoch = new StationXML::ChannelEpoch;
		sx_epoch->setStart(stream->start());
		sx_chan->addEpoch(sx_epoch.get());
	}

	try { sx_epoch->setEnd(StationXML::DateTime(stream->end())); }
	catch ( ... ) { sx_epoch->setEnd(Core::None); }

	StationXML::LatType lat;
	StationXML::LonType lon;
	StationXML::DistanceType elev, depth;
	try { lat.setValue(loc->latitude()); } catch ( ... ) { lat.setValue(0); }
	try { lon.setValue(loc->longitude()); } catch ( ... ) { lon.setValue(0); }
	try { elev.setValue(loc->elevation()); } catch ( ... ) { elev.setValue(0); }
	try { depth.setValue(stream->depth()); } catch ( ... ) { depth.setValue(0); }

	sx_epoch->setLatitude(lat);
	sx_epoch->setLongitude(lon);
	sx_epoch->setElevation(elev);
	sx_epoch->setDepth(depth);

	try {
		StationXML::AzimuthType azi;
		azi.setValue(stream->azimuth());
		sx_epoch->setAzimuth(azi);
	}
	catch ( ... ) {
		sx_epoch->setAzimuth(Core::None);
	}

	try {
		StationXML::DipType dip;
		dip.setValue(stream->dip());
		sx_epoch->setDip(dip);
	}
	catch ( ... ) {
		sx_epoch->setDip(Core::None);
	}

	try {
		StationXML::SampleRateType sr;
		sr.setValue((double)stream->sampleRateNumerator() /
		            (double)stream->sampleRateDenominator());
		sx_epoch->setSampleRate(sr);
	}
	catch ( ... ) {
		sx_epoch->setSampleRate(Core::None);
	}

	sx_epoch->setStorageFormat(stream->format());

	// Remove all existing responses and regenerate them with the
	// following information
	while ( sx_epoch->responseCount() > 0 )
		sx_epoch->removeResponse(size_t(0));

	try {
		StationXML::Sensitivity sensitivity;
		sensitivity.setSensitivityValue(stream->gain());
		sensitivity.setFrequency(stream->gainFrequency());

		if ( stream->gainUnit().empty() ) {
			SEISCOMP_WARNING("%s.%s.%s.%s: gainUnit not set, assuming m/s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), stream->code().c_str());
			sensitivity.setSensitivityUnits("M/S");
		}
		else
			sensitivity.setSensitivityUnits(stream->gainUnit());

		sx_epoch->setInstrumentSensitivity(sensitivity);

		StationXML::ResponsePtr stage0 = new StationXML::Response;
		StationXML::Sensitivity gainSensitivity;
		StationXML::FrequencyType freq;
		stage0->setStage(0);
		gainSensitivity.setSensitivityValue(sensitivity.sensitivityValue());
		freq.setValue(sensitivity.frequency());
		gainSensitivity.setFrequency(freq);
		gainSensitivity.setSensitivityUnits(sensitivity.sensitivityUnits());
		stage0->setStageSensitivity(gainSensitivity);
		sx_epoch->addResponse(stage0.get());
	}
	catch ( Core::GeneralException &exc ) {
		sx_epoch->setInstrumentSensitivity(Core::None);
	}

	const DataModel::Datalogger *datalogger = NULL;
	const DataModel::Sensor *sensor = NULL;

	if ( !stream->datalogger().empty() )
		datalogger = findDatalogger(stream->datalogger());

	if ( !stream->sensor().empty() )
		sensor = findSensor(stream->sensor());

	if ( sensor ) {
		if ( !sensor->type().empty() ||
		     !sensor->model().empty() ||
		     !sensor->manufacturer().empty() ) {
			sx_epoch->setSensor(StationXML::Equipment());
			string type = sensor->type();
			if ( type.empty() ) {
				type = sensor->manufacturer();
				if ( !sensor->model().empty() ) {
					if ( !type.empty() ) type += " ";
					type += sensor->model();
				}
			}

			sx_epoch->sensor().setEquipType(type);
			sx_epoch->sensor().setDescription(sensor->description());
			sx_epoch->sensor().setManufacturer(sensor->manufacturer());
			sx_epoch->sensor().setModel(sensor->model());

			// Special import serial number
			if ( stream->sensorSerialNumber() != "yyyy" )
				sx_epoch->sensor().setSerialNumber(stream->sensorSerialNumber());
		}

		process(sx_epoch.get(), stream, sensor);
	}

	if ( datalogger ) {
		if ( !datalogger->digitizerModel().empty() ||
		     !datalogger->digitizerManufacturer().empty() ) {
			sx_epoch->setDatalogger(StationXML::Datalogger());
			string type = datalogger->digitizerManufacturer();
			if ( !datalogger->digitizerModel().empty() ) {
				if ( !type.empty() ) type += " ";
				type += datalogger->digitizerModel();
			}

			sx_epoch->datalogger().setEquipType(type);
			sx_epoch->datalogger().setDescription(datalogger->description());
			sx_epoch->datalogger().setManufacturer(datalogger->digitizerManufacturer());
			sx_epoch->datalogger().setModel(datalogger->digitizerModel());

			// Special import serial number
			if ( stream->dataloggerSerialNumber() != "xxxx" )
				sx_epoch->datalogger().setSerialNumber(stream->dataloggerSerialNumber());
		}

		// Restore clock drift
		try {
			double clockDrift = datalogger->maxClockDrift();
			double sr = sx_epoch->sampleRate().value();
			StationXML::ClockDriftType drift;
			drift.setValue(clockDrift/sr);
			sx_epoch->setClockDrift(drift);
		}
		catch ( ... ) {
			sx_epoch->setClockDrift(Core::None);
		}

		process(sx_epoch.get(), stream, datalogger);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2StaXML::process(StationXML::ChannelEpoch *sx_epoch,
                             const DataModel::Stream *stream,
                             const DataModel::Datalogger *datalogger) {
	// No stage0 response -> sorry
	if ( sx_epoch->responseCount() < 1 ) return false;

	double gain = 0;
	try { gain = datalogger->gain(); } catch ( ... ) {}

	StationXML::ResponsePtr sx_resp0 = new StationXML::Response;
	StationXML::FrequencyType freq;
	StationXML::FloatType ft;

	// Initialize zeros
	freq.setValue(0);
	ft.setValue(0);

	sx_resp0->setStage(sx_epoch->responseCount());
	sx_resp0->stageSensitivity().setSensitivityValue(gain);
	sx_resp0->stageSensitivity().setSensitivityUnits(CURRENT);
	sx_resp0->stageSensitivity().setFrequency(freq);

	StationXML::CoefficientsPtr coeff0 = new StationXML::Coefficients;
	coeff0->setInputUnits(CURRENT);
	coeff0->setOutputUnits(DIGITAL);
	coeff0->setCfTransferFunctionType(StationXML::CFTFT_DIGITAL);

	sx_resp0->addCoefficients(coeff0.get());

	sx_resp0->setDecimation(StationXML::Decimation());
	sx_resp0->decimation().setFactor(1);
	sx_resp0->decimation().setOffset(0);
	sx_resp0->decimation().setDelay(ft);
	sx_resp0->decimation().setCorrection(ft);
	sx_resp0->decimation().setInputSampleRate(freq);
	// Input sample rate will be derived from all subsequent stages and
	// set later

	int numerator, denominator;

	try {
		numerator = stream->sampleRateNumerator();
		denominator = stream->sampleRateDenominator();
	}
	catch ( ... ) {
		return false;
	}

	double finalSR = (double)numerator / (double)denominator;

	sx_epoch->addResponse(sx_resp0.get());

	DataModel::Decimation *deci = datalogger->decimation(DataModel::DecimationIndex(numerator, denominator));
	if ( deci != NULL ) {

		try {
			string analogueFilterChain = deci->analogueFilterChain().content();
			vector<string> filters;
			Core::split(filters, analogueFilterChain.c_str(), " ");

			for ( size_t i = 0; i < filters.size(); ++i ) {
				if ( filters[i].empty() ) continue;
				const DataModel::ResponsePAZ *paz = findPAZ(filters[i]);
				if ( paz == NULL ) {
					SEISCOMP_WARNING("PAZ response not found in inventory: %s",
									 filters[i].c_str());
					SEISCOMP_WARNING("Stopping at response stage %d",
									 (int)sx_epoch->responseCount());
					return false;
				}

				StationXML::ResponsePtr sx_resp = convert(paz, CURRENT, CURRENT);

				sx_resp->setStage(sx_epoch->responseCount());
				sx_epoch->addResponse(sx_resp.get());
			}
		}
		catch ( ... ) {}

		try {
			string digitialFilterChain = deci->digitalFilterChain().content();
			vector<string> filters;
			Core::split(filters, digitialFilterChain.c_str(), " ");
			for ( size_t i = 0; i < filters.size(); ++i ) {
				if ( filters[i].empty() ) continue;

				StationXML::ResponsePtr sx_resp;

				const DataModel::ResponseFIR *fir = findFIR(filters[i]);
				if ( fir != NULL )
					sx_resp = convert(fir);
				else {
					const DataModel::ResponsePAZ *paz = findPAZ(filters[i]);
					if ( paz != NULL )
						sx_resp = convert(paz, DIGITAL, DIGITAL);
					else {
						SEISCOMP_WARNING("FIR response not found in inventory: %s",
										 filters[i].c_str());
						SEISCOMP_WARNING("Stopping at response stage %d",
										 (int)sx_epoch->responseCount());
						return false;
					}
				}

				sx_resp->setStage(sx_epoch->responseCount());
				sx_epoch->addResponse(sx_resp.get());
			}
		}
		catch ( ... ) {}
	}

	double outputSR = finalSR;
	double inputSR;

	for ( int r = sx_epoch->responseCount()-1; r >= sx_resp0->stage(); --r ) {
		StationXML::Response *resp = sx_epoch->response(r);
		try {
			inputSR = outputSR * resp->decimation().factor();
			freq.setValue(inputSR);
			resp->decimation().setInputSampleRate(freq);
			resp->decimation().delay().setValue( resp->decimation().delay() / inputSR );
			resp->decimation().correction().setValue( resp->decimation().correction() / inputSR );
			outputSR = inputSR;
		}
		catch ( ... ) {}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2StaXML::process(StationXML::ChannelEpoch *sx_epoch,
                             const DataModel::Stream *stream,
                             const DataModel::Sensor *sensor) {
	// No stage0 response -> sorry
	if ( sx_epoch->responseCount() < 1 ) return false;

	string unit = sensor->unit();
	if ( unit.empty() ) {
		SEISCOMP_WARNING("Sensor %s: unit not set, assuming m/s",
		                 sensor->publicID().c_str());
		unit = "M/S";
	}

	const DataModel::ResponsePAZ *paz = findPAZ(sensor->response());
	if ( paz == NULL ) {
		const DataModel::ResponsePolynomial *poly = findPoly(sensor->response());
		if ( poly == NULL ) return false;

		// Insert poly response
		StationXML::ResponsePtr sx_resp = new StationXML::Response;
		sx_resp->setStage(sx_epoch->responseCount());
		try { sx_resp->stageSensitivity().setSensitivityValue(poly->gain()); }
		catch ( ... ) { sx_resp->stageSensitivity().setSensitivityValue(0); }

		StationXML::FrequencyType freq;

		try {
			freq.setValue(poly->gainFrequency());
			sx_resp->stageSensitivity().setFrequency(freq);
		}
		catch ( ... ) {}

		sx_resp->stageSensitivity().setSensitivityUnits(unit);
		StationXML::PolynomialPtr sx_poly = new StationXML::Polynomial;

		sx_poly->setInputUnits(unit);
		sx_poly->setOutputUnits(CURRENT);

		StationXML::ApproximationType at;
		if ( at.fromString(poly->approximationType().c_str()) )
			sx_poly->setApproximationType(at);
		else
			sx_poly->setApproximationType(StationXML::AT_MACLAURIN);

		try { freq.setValue(poly->approximationLowerBound()); }
		catch ( ... ) { freq.setValue(0); }
		sx_poly->setFreqLowerBound(freq);

		try { freq.setValue(poly->approximationUpperBound()); }
		catch ( ... ) { freq.setValue(0); }
		sx_poly->setFreqUpperBound(freq);

		try { freq.setValue(poly->approximationError()); }
		catch ( ... ) { freq.setValue(0); }
		sx_poly->setMaxError(freq);

		int idx = 0;
		try {
			const vector<double> &coeff = poly->coefficients().content();
			for ( size_t i = 0; i < coeff.size(); ++i ) {
				StationXML::PolynomialCoefficientPtr c = new StationXML::PolynomialCoefficient;
				c->setNumber(idx++);
				c->setValue(coeff[i]);
				sx_poly->addCoefficient(c.get());
			}
		}
		catch ( ... ) {}

		sx_resp->addPolynomial(sx_poly.get());
		sx_epoch->addResponse(sx_resp.get());
	}
	else {
		// Insert PAZ response
		StationXML::ResponsePtr sx_resp = convert(paz, unit, CURRENT);
		sx_resp->setStage(sx_epoch->responseCount());
		sx_epoch->addResponse(sx_resp.get());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::Datalogger *
Convert2StaXML::findDatalogger(const std::string &publicID) {
	ObjectLookup::iterator it = _dataloggerLookup.find(publicID);
	if ( it == _dataloggerLookup.end() ) return NULL;
	return (const DataModel::Datalogger*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::Sensor *
Convert2StaXML::findSensor(const std::string &publicID) {
	ObjectLookup::iterator it = _sensorLookup.find(publicID);
	if ( it == _sensorLookup.end() ) return NULL;
	return (const DataModel::Sensor*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponseFIR *
Convert2StaXML::findFIR(const std::string &publicID) {
	ObjectLookup::iterator it = _firLookup.find(publicID);
	if ( it == _firLookup.end() ) return NULL;
	return (const DataModel::ResponseFIR*)it->second;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponsePAZ *
Convert2StaXML::findPAZ(const std::string &publicID) {
	ObjectLookup::iterator it = _pazLookup.find(publicID);
	if ( it == _pazLookup.end() ) return NULL;
	return (const DataModel::ResponsePAZ*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponsePolynomial *
Convert2StaXML::findPoly(const std::string &publicID) {
	ObjectLookup::iterator it = _polyLookup.find(publicID);
	if ( it == _polyLookup.end() ) return NULL;
	return (const DataModel::ResponsePolynomial*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
