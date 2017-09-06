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


#define SEISCOMP_COMPONENT FDSNXML
#include "convert2fdsnxml.h"

#include <fdsnxml/fdsnstationxml.h>
#include <fdsnxml/network.h>
#include <fdsnxml/station.h>
#include <fdsnxml/channel.h>
#include <fdsnxml/response.h>
#include <fdsnxml/responsestage.h>
#include <fdsnxml/coefficients.h>
#include <fdsnxml/fir.h>
#include <fdsnxml/numeratorcoefficient.h>
#include <fdsnxml/polynomial.h>
#include <fdsnxml/polynomialcoefficient.h>
#include <fdsnxml/polesandzeros.h>
#include <fdsnxml/poleandzero.h>
#include <fdsnxml/responselist.h>
#include <fdsnxml/responselistelement.h>

#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/logging/log.h>

#include <rapidjson/document.h>

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

FDSNXML::Network *findNetwork(FDSNXML::FDSNStationXML *msg,
                                 const string &code, const Core::Time &start) {
	for ( size_t i = 0; i < msg->networkCount(); ++i ) {
		FDSNXML::Network *net = msg->network(i);
		if ( net->code() == code && net->startDate() == start )
			return net;
	}

	return NULL;
}

FDSNXML::Station *findStation(FDSNXML::Network *net,
                              const string &code, const Core::Time &start) {
	for ( size_t i = 0; i < net->stationCount(); ++i ) {
		FDSNXML::Station *sta = net->station(i);
		if ( sta->code() == code && sta->startDate() == start ) return sta;
	}

	return NULL;
}

FDSNXML::Channel *findChannel(FDSNXML::Station *sta,
                              const string &loccode, const string &code,
                              const Core::Time &start) {
	for ( size_t i = 0; i < sta->channelCount(); ++i ) {
		FDSNXML::Channel *chan = sta->channel(i);
		if ( chan->code() == code && chan->locationCode() == loccode &&
		     chan->startDate() == start )
			return chan;
	}

	return NULL;
}


template <typename T>
void populateStageGain(FDSNXML::ResponseStage *stage, T *resp) {
	FDSNXML::Gain stageGain;
	OPT(double) gain, gainFrequency;

	try { gain = resp->gain(); } catch ( ... ) {}
	try { gainFrequency = resp->gainFrequency(); } catch ( ... ) {}

	if ( gain || gainFrequency ) {
		if ( gain )
			stageGain.setValue(*gain);
		else
			stageGain.setValue(0);

		if ( gainFrequency )
			stageGain.setFrequency(*gainFrequency);
		else
			stageGain.setFrequency(0);

		stage->setStageGain(stageGain);
	}
	else
		stage->setStageGain(Core::None);
}


// SC3 FIR responses are converted to fdsnxml FIR responses to
// use optimizations (symmetry) though all fdsnxml files in the
// wild are using Coefficient responses. This can be changed on
// request with the penalty of resolving the symmetry here.
FDSNXML::ResponseStagePtr convert(const DataModel::ResponseFIR *fir,
                                  const std::string &inputUnit,
                                  const std::string &outputUnit) {
	FDSNXML::FrequencyType freq;
	FDSNXML::FloatType ft;
	FDSNXML::ResponseStagePtr sx_resp = new FDSNXML::ResponseStage;
	populateStageGain(sx_resp.get(), fir);

	ft.setValue(0);

	OPT(int) decimationFactor;
	try { decimationFactor = fir->decimationFactor(); }
	catch ( ... ) {}

	if ( decimationFactor ) {
		sx_resp->setDecimation(FDSNXML::Decimation());

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
	}

	sx_resp->setFIR(FDSNXML::FIR());
	FDSNXML::FIR &sx_fir = sx_resp->fIR();

	sx_fir.setResourceId(fir->publicID());
	sx_fir.setName(fir->name());

	if ( fir->symmetry() == "A" )
		sx_fir.setSymmetry(FDSNXML::SymmetryType(FDSNXML::ST_NONE));
	else if ( fir->symmetry() == "B" )
		sx_fir.setSymmetry(FDSNXML::SymmetryType(FDSNXML::ST_ODD));
	else if ( fir->symmetry() == "C" )
		sx_fir.setSymmetry(FDSNXML::SymmetryType(FDSNXML::ST_EVEN));

	sx_fir.setInputUnits(inputUnit);
	sx_fir.setOutputUnits(outputUnit);

	try {
		const vector<double> &coeff = fir->coefficients().content();
		for ( size_t c = 0; c < coeff.size(); ++c ) {
			FDSNXML::NumeratorCoefficientPtr fc = new FDSNXML::NumeratorCoefficient;
			fc->setValue(coeff[c]);
			sx_fir.addNumeratorCoefficient(fc.get());
		}
	}
	catch ( ... ) {}

	return sx_resp;
}


FDSNXML::ResponseStagePtr convert(const DataModel::ResponseIIR *iir,
                                  const std::string &inputUnit,
                                  const std::string &outputUnit) {
	FDSNXML::FrequencyType freq;
	FDSNXML::FloatType ft;
	FDSNXML::ResponseStagePtr sx_resp = new FDSNXML::ResponseStage;
	populateStageGain(sx_resp.get(), iir);

	ft.setValue(0);

	OPT(int) decimationFactor;
	try { decimationFactor = iir->decimationFactor(); }
	catch ( ... ) {}

	if ( decimationFactor ) {
		sx_resp->setDecimation(FDSNXML::Decimation());

		try { sx_resp->decimation().setFactor(*decimationFactor); }
		catch ( ... ) { sx_resp->decimation().setFactor(0); }

		sx_resp->decimation().setOffset(0);

		try { ft.setValue(iir->delay()); }
		catch ( ... ) { ft.setValue(0); }
		sx_resp->decimation().setDelay(ft);

		try { ft.setValue(iir->correction()); }
		catch ( ... ) { ft.setValue(0); }
		sx_resp->decimation().setCorrection(ft);

		// Update it later
		freq.setValue(0);
		sx_resp->decimation().setInputSampleRate(freq);
	}

	sx_resp->setCoefficients(FDSNXML::Coefficients());
	FDSNXML::Coefficients &sx_iir = sx_resp->coefficients();

	sx_iir.setResourceId(iir->publicID());
	sx_iir.setName(iir->name());

	sx_iir.setInputUnits(inputUnit);
	sx_iir.setOutputUnits(outputUnit);

	if ( iir->type() == "A" )
		sx_iir.setCfTransferFunctionType(FDSNXML::CFTFT_ANALOG_RAD);
	else if ( iir->type() == "B" )
		sx_iir.setCfTransferFunctionType(FDSNXML::CFTFT_ANALOG_HZ);
	else if ( iir->type() == "D" )
		sx_iir.setCfTransferFunctionType(FDSNXML::CFTFT_DIGITAL);
	else {
		SEISCOMP_WARNING("Type 'C' (composite) response coefficients are not supported");
		return NULL;
	}

	try {
		const vector<double> &numerators = iir->numerators().content();
		for ( size_t c = 0; c < numerators.size(); ++c ) {
			FDSNXML::FloatTypePtr fv = new FDSNXML::FloatType;
			fv->setValue(numerators[c]);
			sx_iir.addNumerator(fv.get());
		}
	}
	catch ( ... ) {}

	try {
		const vector<double> &denominators = iir->denominators().content();
		for ( size_t c = 0; c < denominators.size(); ++c ) {
			FDSNXML::FloatTypePtr fv = new FDSNXML::FloatType;
			fv->setValue(denominators[c]);
			sx_iir.addDenominator(fv.get());
		}
	}
	catch ( ... ) {}

	return sx_resp;
}


FDSNXML::ResponseStagePtr convert(const DataModel::ResponsePAZ *paz,
                                  const std::string &inputUnit,
                                  const std::string &outputUnit) {
	FDSNXML::FrequencyType freq;
	FDSNXML::ResponseStagePtr sx_resp = new FDSNXML::ResponseStage;
	populateStageGain(sx_resp.get(), paz);

	sx_resp->setPolesZeros(FDSNXML::PolesAndZeros());
	FDSNXML::PolesAndZeros &sx_paz = sx_resp->polesZeros();

	sx_paz.setResourceId(paz->publicID());
	sx_paz.setName(paz->name());

	try { sx_paz.setNormalizationFactor(paz->normalizationFactor()); }
	catch ( ... ) { sx_paz.setNormalizationFactor(0); }

	try { freq.setValue(paz->normalizationFrequency()); }
	catch ( ... ) { freq.setValue(0); }
	sx_paz.setNormalizationFrequency(freq);

	sx_paz.setInputUnits(FDSNXML::UnitsType(inputUnit));
	sx_paz.setOutputUnits(FDSNXML::UnitsType(outputUnit));

	OPT(int) decimationFactor;
	try { decimationFactor = paz->decimationFactor(); }
	catch ( ... ) {}

	if ( decimationFactor ) {
		FDSNXML::FrequencyType freq;
		FDSNXML::FloatType ft;

		freq.setValue(0);
		ft.setValue(0);

		sx_resp->setDecimation(FDSNXML::Decimation());

		try { sx_resp->decimation().setFactor(*decimationFactor); }
		catch ( ... ) { sx_resp->decimation().setFactor(0); }

		sx_resp->decimation().setOffset(0);

		try { ft.setValue(paz->delay()); }
		catch ( ... ) { ft.setValue(0); }
		sx_resp->decimation().setDelay(ft);

		try { ft.setValue(paz->correction()); }
		catch ( ... ) { ft.setValue(0); }
		sx_resp->decimation().setCorrection(ft);

		// Update it later
		freq.setValue(0);
		sx_resp->decimation().setInputSampleRate(freq);
	}

	if ( paz->type() == "A" )
		sx_paz.setPzTransferFunctionType(FDSNXML::PZTFT_LAPLACE_RAD);
	else if ( paz->type() == "B" )
		sx_paz.setPzTransferFunctionType(FDSNXML::PZTFT_LAPLACE_HZ);
	else if ( paz->type() == "D" )
		sx_paz.setPzTransferFunctionType(FDSNXML::PZTFT_DIGITAL_Z_TRANSFORM);
	else
		sx_paz.setPzTransferFunctionType(FDSNXML::PZTFT_LAPLACE_RAD);

	int idx = 0;
	try {
		const vector< complex<double> > &poles = paz->poles().content();
		for ( size_t i = 0; i < poles.size(); ++i ) {
			FDSNXML::PoleAndZeroPtr pole = new FDSNXML::PoleAndZero;
			pole->setNumber(idx++);
			pole->setReal(poles[i].real());
			pole->setImaginary(poles[i].imag());
			sx_paz.addPole(pole.get());
		}
	}
	catch ( ... ) {}

	try {
		const vector< complex<double> > &zeros = paz->zeros().content();
		for ( size_t i = 0; i < zeros.size(); ++i ) {
			FDSNXML::PoleAndZeroPtr zero = new FDSNXML::PoleAndZero;
			zero->setNumber(idx++);
			zero->setReal(zeros[i].real());
			zero->setImaginary(zeros[i].imag());
			sx_paz.addZero(zero.get());
		}
	}
	catch ( ... ) {}

	return sx_resp;
}


FDSNXML::ResponseStagePtr convert(const DataModel::ResponseFAP *fap,
                                  const std::string &inputUnit,
                                  const std::string &outputUnit) {
	FDSNXML::ResponseStagePtr sx_resp = new FDSNXML::ResponseStage;
	populateStageGain(sx_resp.get(), fap);
	sx_resp->setResponseList(FDSNXML::ResponseList());
	FDSNXML::ResponseList &sx_fap = sx_resp->responseList();

	sx_fap.setResourceId(fap->publicID());
	sx_fap.setName(fap->name());

	sx_fap.setInputUnits(FDSNXML::UnitsType(inputUnit));
	sx_fap.setOutputUnits(FDSNXML::UnitsType(outputUnit));

	try {
		const vector<double> &tuples = fap->tuples().content();
		for ( size_t i = 0; i < tuples.size(); i += 3 ) {
			FDSNXML::ResponseListElementPtr c = new FDSNXML::ResponseListElement;
			c->frequency().setValue(tuples[i+0]);
			c->amplitude().setValue(tuples[i+1]);
			c->phase().setValue(tuples[i+2]);
			sx_fap.addElement(c.get());
		}
	}
	catch ( ... ) {}

	return sx_resp;
}


FDSNXML::ResponseStagePtr convert(const DataModel::ResponsePolynomial *poly,
                                  const std::string &inputUnit,
                                  const std::string &outputUnit) {
	// Insert poly response
	FDSNXML::ResponseStagePtr sx_resp = new FDSNXML::ResponseStage;
	populateStageGain(sx_resp.get(), poly);
	sx_resp->setPolynomial(FDSNXML::Polynomial());
	FDSNXML::Polynomial &sx_poly = sx_resp->polynomial();

	sx_poly.setResourceId(poly->publicID());
	sx_poly.setName(poly->name());

	sx_poly.setInputUnits(FDSNXML::UnitsType(inputUnit));
	sx_poly.setOutputUnits(FDSNXML::UnitsType(outputUnit));

	FDSNXML::ApproximationType at;
	if ( at.fromString(poly->approximationType().c_str()) )
		sx_poly.setApproximationType(at);
	else
		sx_poly.setApproximationType(FDSNXML::AT_MACLAURIN);

	try { sx_poly.setApproximationLowerBound(poly->approximationLowerBound()); }
	catch ( ... ) { sx_poly.setApproximationLowerBound(0); }

	try { sx_poly.setApproximationUpperBound(poly->approximationUpperBound()); }
	catch ( ... ) { sx_poly.setApproximationUpperBound(0); }

	try { sx_poly.setMaximumError(poly->approximationError()); }
	catch ( ... ) { sx_poly.setMaximumError(0); }

	int idx = 0;
	try {
		const vector<double> &coeff = poly->coefficients().content();
		for ( size_t i = 0; i < coeff.size(); ++i ) {
			FDSNXML::PolynomialCoefficientPtr c = new FDSNXML::PolynomialCoefficient;
			c->setNumber(idx++);
			c->setValue(coeff[i]);
			sx_poly.addCoefficient(c.get());
		}
	}
	catch ( ... ) {}

	return sx_resp;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Convert2FDSNStaXML::Convert2FDSNStaXML(FDSNXML::FDSNStationXML *msg)
: _msg(msg), _inv(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2FDSNStaXML::push(const DataModel::Inventory *inv) {
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

	_iirLookup.clear();
	for ( size_t i = 0; i < inv->responseIIRCount(); ++i ) {
		DataModel::ResponseIIR *r = inv->responseIIR(i);
		_iirLookup[r->publicID()] = r;
	}

	_pazLookup.clear();
	for ( size_t i = 0; i < inv->responsePAZCount(); ++i ) {
		DataModel::ResponsePAZ *r = inv->responsePAZ(i);
		_pazLookup[r->publicID()] = r;
	}

	_fapLookup.clear();
	for ( size_t i = 0; i < inv->responseFAPCount(); ++i ) {
		DataModel::ResponseFAP *r = inv->responseFAP(i);
		_fapLookup[r->publicID()] = r;
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

		FDSNXML::NetworkPtr sx_net;
		sx_net = findNetwork(_msg, net->code(), net->start());
		if ( sx_net == NULL ) {
			sx_net = new FDSNXML::Network;
			sx_net->setCode(net->code());
			sx_net->setStartDate(FDSNXML::DateTime(net->start()));
			_msg->addNetwork(sx_net.get());
		}

		try { sx_net->setEndDate(FDSNXML::DateTime(net->end())); }
		catch ( ... ) { sx_net->setEndDate(Core::None); }

		sx_net->setDescription(net->description());

		try { sx_net->setRestrictedStatus(FDSNXML::RestrictedStatusType(net->restricted()?FDSNXML::RST_CLOSED:FDSNXML::RST_OPEN)); }
		catch ( ... ) { sx_net->setRestrictedStatus(Core::None); }

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
bool Convert2FDSNStaXML::process(FDSNXML::Network *sx_net,
                                 const DataModel::Station *sta) {
	FDSNXML::StationPtr sx_sta = findStation(sx_net, sta->code(), sta->start());
	// Create station object
	if ( sx_sta == NULL ) {
		sx_sta = new FDSNXML::Station;
		sx_sta->setCode(sta->code());
		sx_sta->setStartDate(FDSNXML::DateTime(sta->start()));

		sx_net->addStation(sx_sta.get());
	}

	SEISCOMP_DEBUG("Processing station %s/%s (%s)",
	               sx_net->code().c_str(), sta->code().c_str(),
	               sta->start().toString("%F %T").c_str());

	// Update station object
	sx_sta->setCreationDate(FDSNXML::DateTime(sta->start()));

	try { sx_sta->setEndDate(FDSNXML::DateTime(sta->end())); }
	catch ( ... ) { sx_sta->setEndDate(Core::None); }

	try { sx_sta->setRestrictedStatus(FDSNXML::RestrictedStatusType(sta->restricted()?FDSNXML::RST_CLOSED:FDSNXML::RST_OPEN)); }
	catch ( ... ) { sx_sta->setRestrictedStatus(Core::None); }

	FDSNXML::LatitudeType lat;
	FDSNXML::LongitudeType lon;
	FDSNXML::DistanceType elev;
	try { lat.setValue(sta->latitude()); } catch ( ... ) { lat.setValue(0); }
	try { lon.setValue(sta->longitude()); } catch ( ... ) { lon.setValue(0); }
	try { elev.setValue(sta->elevation()); } catch ( ... ) { elev.setValue(0); }

	sx_sta->setLatitude(lat);
	sx_sta->setLongitude(lon);
	sx_sta->setElevation(elev);

	// Site is mandatory
	FDSNXML::Site site;
	if ( !sta->country().empty() )
		site.setCountry(sta->country());
	if ( sta->description().empty() )
		site.setName(sx_net->code() + "-" + sx_sta->code());
	else
		site.setName(sta->description());
	sx_sta->setSite(site);

	for ( size_t l = 0; l < sta->sensorLocationCount(); ++l ) {
		if ( _interrupted ) break;
		DataModel::SensorLocation *loc = sta->sensorLocation(l);
		for ( size_t s = 0; s < loc->streamCount(); ++s ) {
			if ( _interrupted ) break;
			DataModel::Stream *stream = loc->stream(s);
			process(sx_sta.get(), loc, stream);
		}
	}

	return !_interrupted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2FDSNStaXML::process(FDSNXML::Station *sx_sta,
                                 const DataModel::SensorLocation *loc,
                                 const DataModel::Stream *stream) {
	FDSNXML::ChannelPtr sx_chan = findChannel(sx_sta, loc->code(), stream->code(), stream->start());
	if ( sx_chan == NULL ) {
		sx_chan = new FDSNXML::Channel;
		sx_chan->setCode(stream->code());
		sx_chan->setStartDate(FDSNXML::DateTime(stream->start()));
		sx_chan->setLocationCode(loc->code());
		sx_sta->addChannel(sx_chan.get());
	}

	try { sx_chan->setRestrictedStatus(FDSNXML::RestrictedStatusType(stream->restricted()?FDSNXML::RST_CLOSED:FDSNXML::RST_OPEN)); }
	catch ( ... ) { sx_chan->setRestrictedStatus(Core::None); }

	try { sx_chan->setEndDate(FDSNXML::DateTime(stream->end())); }
	catch ( ... ) { sx_chan->setEndDate(Core::None); }

	FDSNXML::LatitudeType lat;
	FDSNXML::LongitudeType lon;
	FDSNXML::DistanceType elev, depth;
	try { lat.setValue(loc->latitude()); } catch ( ... ) { lat.setValue(0); }
	try { lon.setValue(loc->longitude()); } catch ( ... ) { lon.setValue(0); }
	try { elev.setValue(loc->elevation()); } catch ( ... ) { elev.setValue(0); }
	try { depth.setValue(stream->depth()); } catch ( ... ) { depth.setValue(0); }

	sx_chan->setLatitude(lat);
	sx_chan->setLongitude(lon);
	sx_chan->setElevation(elev);
	sx_chan->setDepth(depth);

	try {
		FDSNXML::AzimuthType azi;
		azi.setValue(stream->azimuth() < 0 ? 360.0 + fmod(stream->azimuth(), 360.0) : stream->azimuth());
		sx_chan->setAzimuth(azi);
	}
	catch ( ... ) {
		sx_chan->setAzimuth(Core::None);
	}

	try {
		FDSNXML::DipType dip;
		dip.setValue(stream->dip());
		sx_chan->setDip(dip);
	}
	catch ( ... ) {
		sx_chan->setDip(Core::None);
	}

	try {
		FDSNXML::SampleRateType sr;
		FDSNXML::SampleRateRatioType srr;
		sr.setValue((double)stream->sampleRateNumerator() /
		            (double)stream->sampleRateDenominator());
		srr.setNumberSamples(stream->sampleRateNumerator());
		srr.setNumberSeconds(stream->sampleRateDenominator());
		sx_chan->setSampleRate(sr);
		sx_chan->setSampleRateRatio(srr);
	}
	catch ( ... ) {
		sx_chan->setSampleRate(Core::None);
		sx_chan->setSampleRateRatio(Core::None);
	}

	sx_chan->setStorageFormat(stream->format());

	// Remove all existing responses and regenerate them with the
	// following information
	sx_chan->setResponse(FDSNXML::Response());
	FDSNXML::Response &resp = sx_chan->response();

	const DataModel::Datalogger *datalogger = NULL;
	const DataModel::Sensor *sensor = NULL;

	if ( !stream->datalogger().empty() )
		datalogger = findDatalogger(stream->datalogger());

	if ( !stream->sensor().empty() )
		sensor = findSensor(stream->sensor());

	try {
		FDSNXML::Sensitivity sensitivity;
		sensitivity.setValue(stream->gain());
		sensitivity.setFrequency(stream->gainFrequency());

		FDSNXML::UnitsType inputUnits;
		if ( stream->gainUnit().empty() ) {
			SEISCOMP_WARNING("%s.%s.%s.%s: gainUnit not set, assuming m/s",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(),
			                 loc->code().c_str(), stream->code().c_str());
			inputUnits.setName("M/S");
		}
		else
			inputUnits.setName(stream->gainUnit());

		if ( sensor ) {
			try {
				const DataModel::Blob &blob = sensor->remark();
				rapidjson::Document json;
				if ( !json.Parse(blob.content().c_str()).HasParseError() ) {
					rapidjson::Value::ConstMemberIterator jit = json.FindMember("unit");
					if ( jit != json.MemberEnd() && jit->value.IsString() )
						inputUnits.setDescription(jit->value.GetString());
				}
			}
			catch ( ... ) {}
		}

		sensitivity.setInputUnits(inputUnits);
		resp.setInstrumentSensitivity(sensitivity);
	}
	catch ( Core::GeneralException &exc ) {
		resp.setInstrumentSensitivity(Core::None);
	}

	if ( sensor ) {
		sx_chan->setSensor(FDSNXML::Equipment());
		FDSNXML::Equipment &equip = sx_chan->sensor();
		equip.setResourceId(sensor->publicID());

		string type = sensor->type();
		if ( type.empty() ) {
			type = sensor->manufacturer();
			if ( !sensor->model().empty() ) {
				if ( !type.empty() ) type += " ";
				type += sensor->model();
			}
		}

		equip.setType(type);
		equip.setDescription(sensor->description());
		equip.setManufacturer(sensor->manufacturer());
		equip.setModel(sensor->model());

		// Special import serial number
		if ( stream->sensorSerialNumber() != "yyyy" )
			equip.setSerialNumber(stream->sensorSerialNumber());

		process(sx_chan.get(), stream, sensor);
	}
	else
		sx_chan->setSensor(Core::None);

	if ( datalogger ) {
		sx_chan->setDataLogger(FDSNXML::Equipment());
		FDSNXML::Equipment &equip = sx_chan->dataLogger();
		equip.setResourceId(datalogger->publicID());

		string type = datalogger->digitizerManufacturer();
		if ( !datalogger->digitizerModel().empty() ) {
			if ( !type.empty() ) type += " ";
			type += datalogger->digitizerModel();
		}

		equip.setType(type);
		equip.setDescription(datalogger->description());
		equip.setManufacturer(datalogger->digitizerManufacturer());
		equip.setModel(datalogger->digitizerModel());

		// Special import serial number
		if ( stream->dataloggerSerialNumber() != "xxxx" )
			equip.setSerialNumber(stream->dataloggerSerialNumber());

		// Restore clock drift
		try {
			double clockDrift = datalogger->maxClockDrift();
			double sr = sx_chan->sampleRate().value();
			FDSNXML::ClockDriftType drift;
			drift.setValue(clockDrift/sr);
			sx_chan->setClockDrift(drift);
		}
		catch ( ... ) {
			sx_chan->setClockDrift(Core::None);
		}

		process(sx_chan.get(), stream, datalogger);
	}
	else
		sx_chan->setDataLogger(Core::None);

	// Set output unit of last response stage
	try {
		if ( resp.stageCount() > 0 ) {
			FDSNXML::ResponseStage *stage = resp.stage(resp.stageCount()-1);
			if ( stage != NULL ) {
				FDSNXML::Sensitivity &sensitivity = resp.instrumentSensitivity();
				FDSNXML::UnitsType outputUnits;

				try { outputUnits = stage->polesZeros().outputUnits(); }
				catch ( ... ) {
					try { outputUnits = stage->coefficients().outputUnits(); }
					catch ( ... ) {
						try { outputUnits = stage->responseList().outputUnits(); }
						catch ( ... ) {
							try { outputUnits = stage->fIR().outputUnits(); }
							catch ( ... ) {
								try { outputUnits = stage->polynomial().outputUnits(); }
								catch ( ... ) {}
							}
						}
					}
				}

				sensitivity.setOutputUnits(outputUnits);
			}
		}
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2FDSNStaXML::process(FDSNXML::Channel *sx_chan,
                             const DataModel::Stream *stream,
                             const DataModel::Datalogger *datalogger) {
	if ( datalogger->decimationCount() == 0 )
		return true;

	// No stage0 response -> sorry
	FDSNXML::Response *resp = NULL;
	try { resp = &sx_chan->response(); } catch ( ...) {}

	double gain = 0;
	try { gain = datalogger->gain(); } catch ( ... ) {}

	FDSNXML::ResponseStagePtr sx_stage;
	FDSNXML::FrequencyType freq;
	FDSNXML::FloatType ft;
	FDSNXML::CounterType cnt;
	FDSNXML::ResponseStagePtr sx_stage0;

	// Initialize zeros
	freq.setValue(0);
	ft.setValue(0);

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

	DataModel::Decimation *deci = datalogger->decimation(DataModel::DecimationIndex(numerator, denominator));
	if ( deci != NULL ) {
		try {
			string analogueFilterChain = deci->analogueFilterChain().content();
			vector<string> filters;
			Core::split(filters, analogueFilterChain.c_str(), " ");

			for ( size_t i = 0; i < filters.size(); ++i ) {
				if ( filters[i].empty() ) continue;

				sx_stage = NULL;

				const DataModel::ResponsePAZ *paz = findPAZ(filters[i]);
				if ( paz != NULL )
					sx_stage = convert(paz, CURRENT, CURRENT);
				else {
					const DataModel::ResponseFAP *fap = findFAP(filters[i]);
					if ( fap != NULL )
						sx_stage = convert(fap, CURRENT, CURRENT);
					else {
						const DataModel::ResponsePolynomial *poly = findPoly(filters[i]);
						if ( poly != NULL )
							sx_stage = convert(poly, CURRENT, CURRENT);
						else {
							const DataModel::ResponseFIR *fir = findFIR(filters[i]);
							if ( fir != NULL )
								sx_stage = convert(fir, CURRENT, CURRENT);
							else {
								const DataModel::ResponseIIR *iir = findIIR(filters[i]);
								if ( iir != NULL )
									sx_stage = convert(iir, CURRENT, CURRENT);
								else {
									SEISCOMP_WARNING("Response not found in inventory: %s",
									                 filters[i].c_str());
									SEISCOMP_WARNING("Stopping at response stage %d",
									                 (int)resp->stageCount());
									return false;
								}
							}
						}
					}
				}

				cnt.setValue(resp->stageCount()+1);
				sx_stage->setNumber(cnt);
				resp->addStage(sx_stage.get());
			}
		}
		catch ( ... ) {}
	}

	cnt.setValue(resp->stageCount()+1);

	sx_stage = new FDSNXML::ResponseStage;
	FDSNXML::Gain stageGain;
	sx_stage->setNumber(cnt);
	stageGain.setValue(gain);
	stageGain.setFrequency(freq);
	sx_stage->setStageGain(stageGain);

	sx_stage->setCoefficients(FDSNXML::Coefficients());
	FDSNXML::Coefficients &coeff0 = sx_stage->coefficients();
	coeff0.setInputUnits(FDSNXML::UnitsType(CURRENT));
	coeff0.setOutputUnits(FDSNXML::UnitsType(DIGITAL));
	coeff0.setCfTransferFunctionType(FDSNXML::CFTFT_DIGITAL);

	sx_stage->setDecimation(FDSNXML::Decimation());
	sx_stage->decimation().setFactor(1);
	sx_stage->decimation().setOffset(0);
	sx_stage->decimation().setDelay(ft);
	sx_stage->decimation().setCorrection(ft);
	sx_stage->decimation().setInputSampleRate(freq);

	resp->addStage(sx_stage.get());

	sx_stage0 = sx_stage;

	if ( deci != NULL ) {
		try {
			string digitialFilterChain = deci->digitalFilterChain().content();
			vector<string> filters;
			Core::split(filters, digitialFilterChain.c_str(), " ");
			for ( size_t i = 0; i < filters.size(); ++i ) {
				if ( filters[i].empty() ) continue;

				sx_stage = NULL;

				const DataModel::ResponseFIR *fir = findFIR(filters[i]);
				if ( fir != NULL )
					sx_stage = convert(fir, DIGITAL, DIGITAL);
				else {
					const DataModel::ResponseIIR *iir = findIIR(filters[i]);
					if ( iir != NULL )
						sx_stage = convert(iir, DIGITAL, DIGITAL);
					else {
						const DataModel::ResponsePAZ *paz = findPAZ(filters[i]);
						if ( paz != NULL )
							sx_stage = convert(paz, DIGITAL, DIGITAL);
						else {
							const DataModel::ResponseFAP *fap = findFAP(filters[i]);
							if ( fap != NULL )
								sx_stage = convert(fap, DIGITAL, DIGITAL);
							else {
								const DataModel::ResponsePolynomial *poly = findPoly(filters[i]);
								if ( poly != NULL )
									sx_stage = convert(poly, DIGITAL, DIGITAL);
								else {
									SEISCOMP_WARNING("FIR response not found in inventory: %s",
									                 filters[i].c_str());
									SEISCOMP_WARNING("Stopping at response stage %d",
									                 (int)resp->stageCount());
									return false;
								}
							}
						}
					}
				}

				cnt.setValue(resp->stageCount()+1);
				sx_stage->setNumber(cnt);
				resp->addStage(sx_stage.get());
			}
		}
		catch ( ... ) {}
	}

	double finalSR = (double)numerator / (double)denominator;
	double outputSR = finalSR;
	double inputSR = outputSR;

	for ( int r = resp->stageCount()-1; r >= sx_stage0->number(); --r ) {
		FDSNXML::ResponseStage *stage = resp->stage(r);
		try {
			inputSR = outputSR * stage->decimation().factor();
			freq.setValue(inputSR);
			stage->decimation().setInputSampleRate(freq);
			stage->decimation().delay().setValue(stage->decimation().delay() / inputSR);
			stage->decimation().correction().setValue(stage->decimation().correction() / inputSR);
			outputSR = inputSR;
		}
		catch ( ... ) {}
	}

	freq = FDSNXML::FrequencyType();
	freq.setValue(inputSR);
	sx_stage0->decimation().setInputSampleRate(freq);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2FDSNStaXML::process(FDSNXML::Channel *sx_chan,
                                 const DataModel::Stream *stream,
                                 const DataModel::Sensor *sensor) {
	FDSNXML::Response *resp = NULL;
	try { resp = &sx_chan->response(); } catch ( ...) {}

	// No stage0 response -> sorry
	if ( resp == NULL ) return false;

	string unit = sensor->unit();
	if ( unit.empty() ) {
		SEISCOMP_WARNING("Sensor %s: unit not set, assuming m/s",
		                 sensor->publicID().c_str());
		unit = "M/S";
	}

	const DataModel::ResponsePAZ *paz = findPAZ(sensor->response());
	if ( paz == NULL ) {
		const DataModel::ResponsePolynomial *poly = findPoly(sensor->response());
		if ( poly == NULL ) {
			const DataModel::ResponseFAP *fap = findFAP(sensor->response());
			if ( fap == NULL )
				return false;

			// Insert ResponseList response
			FDSNXML::ResponseStagePtr sx_stage = convert(fap, unit, CURRENT);
			FDSNXML::CounterType cnt;
			cnt.setValue(resp->stageCount()+1);
			sx_stage->setNumber(cnt);
			resp->addStage(sx_stage.get());
		}
		else {
			// Insert ResponseList response
			FDSNXML::ResponseStagePtr sx_stage = convert(poly, unit, CURRENT);

			// Set lower and upper frequency bound
			FDSNXML::FrequencyType freq;

			try {
				freq.setValue(sensor->lowFrequency());
			}
			catch ( ... ) {
				freq.setValue(0);
			}

			sx_stage->polynomial().setFrequencyLowerBound(freq);

			try {
				freq.setValue(sensor->highFrequency());
			}
			catch ( ... ) {
				freq.setValue(0);
			}

			sx_stage->polynomial().setFrequencyUpperBound(freq);

			FDSNXML::CounterType cnt;
			cnt.setValue(resp->stageCount()+1);
			sx_stage->setNumber(cnt);
			resp->addStage(sx_stage.get());
		}
	}
	else {
		// Insert PAZ response
		FDSNXML::ResponseStagePtr sx_stage = convert(paz, unit, CURRENT);
		FDSNXML::CounterType cnt;
		cnt.setValue(resp->stageCount()+1);
		sx_stage->setNumber(cnt);
		resp->addStage(sx_stage.get());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::Datalogger *
Convert2FDSNStaXML::findDatalogger(const std::string &publicID) {
	ObjectLookup::iterator it = _dataloggerLookup.find(publicID);
	if ( it == _dataloggerLookup.end() ) return NULL;
	return (const DataModel::Datalogger*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::Sensor *
Convert2FDSNStaXML::findSensor(const std::string &publicID) {
	ObjectLookup::iterator it = _sensorLookup.find(publicID);
	if ( it == _sensorLookup.end() ) return NULL;
	return (const DataModel::Sensor*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponseFIR *
Convert2FDSNStaXML::findFIR(const std::string &publicID) {
	ObjectLookup::iterator it = _firLookup.find(publicID);
	if ( it == _firLookup.end() ) return NULL;
	return (const DataModel::ResponseFIR*)it->second;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponseIIR *
Convert2FDSNStaXML::findIIR(const std::string &publicID) {
	ObjectLookup::iterator it = _iirLookup.find(publicID);
	if ( it == _iirLookup.end() ) return NULL;
	return (const DataModel::ResponseIIR*)it->second;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponsePAZ *
Convert2FDSNStaXML::findPAZ(const std::string &publicID) {
	ObjectLookup::iterator it = _pazLookup.find(publicID);
	if ( it == _pazLookup.end() ) return NULL;
	return (const DataModel::ResponsePAZ*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponseFAP *
Convert2FDSNStaXML::findFAP(const std::string &publicID) {
	ObjectLookup::iterator it = _fapLookup.find(publicID);
	if ( it == _fapLookup.end() ) return NULL;
	return (const DataModel::ResponseFAP*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::ResponsePolynomial *
Convert2FDSNStaXML::findPoly(const std::string &publicID) {
	ObjectLookup::iterator it = _polyLookup.find(publicID);
	if ( it == _polyLookup.end() ) return NULL;
	return (const DataModel::ResponsePolynomial*)it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
