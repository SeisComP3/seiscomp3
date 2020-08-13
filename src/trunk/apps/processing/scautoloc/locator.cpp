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




#define SEISCOMP_COMPONENT Autoloc
#include <seiscomp3/logging/log.h>

#include <string>
#include <vector>
#include <map>

#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/seismology/locator/locsat.h>
#include "util.h"
#include "sc3adapters.h"
#include "locator.h"

using namespace std;

namespace Autoloc {

MySensorLocationDelegate::~MySensorLocationDelegate() {
	_sensorLocations.clear();
}

void MySensorLocationDelegate::setStation(const Autoloc::Station *station) {
	string key = station->net + "." + station->code;

	Seiscomp::DataModel::SensorLocationPtr
		sloc = Seiscomp::DataModel::SensorLocation::Create();

	sloc->setLatitude(  station->lat  );
	sloc->setLongitude( station->lon  );
	sloc->setElevation( station->alt  );
	_sensorLocations.insert(pair<string,Seiscomp::DataModel::SensorLocationPtr>(key, sloc));
}

Seiscomp::DataModel::SensorLocation *
MySensorLocationDelegate::getSensorLocation(Seiscomp::DataModel::Pick *pick) const {
	if ( !pick ) return NULL;

	std::string key = pick->waveformID().networkCode() + "." + pick->waveformID().stationCode();

	SensorLocationList::const_iterator it = _sensorLocations.find(key);
	if ( it != _sensorLocations.end() )
		return it->second.get();

	return NULL;
}

Locator::Locator()
{
}

bool Locator::init()
{
	const std::string locator = "LOCSAT";
	_sc3locator =
		Seiscomp::Seismology::LocatorInterface::Create(locator.c_str());
	if (!_sc3locator) {
		SEISCOMP_ERROR_S("Could not create "+locator+" instance");
		exit(-1);
	}
	_sc3locator->useFixedDepth(false);
	_locatorCallCounter = 0;
	_minDepth = 5;
	setFixedDepth(_minDepth, false);

	sensorLocationDelegate = new MySensorLocationDelegate;
	_sc3locator->setSensorLocationDelegate(sensorLocationDelegate.get());

	return true;
}

Locator::~Locator()
{
	SEISCOMP_INFO("Locator instance called %ld times", _locatorCallCounter);
}

void Locator::setStation(const Autoloc::Station *station) {
	sensorLocationDelegate->setStation(station);
}


void Locator::setMinimumDepth(double depth)
{
	_minDepth = depth;
}


static bool hasFixedDepth(const Origin *origin)
{
	switch(origin->depthType) {
		case Origin::DepthManuallyFixed:
		case Origin::DepthDefault:
			return true;
		default:
			break;
	}
	return false;
}


Origin *Locator::relocate(const Origin *origin)
{
	_locatorCallCounter++;

// vvvvvvvvvvvvvvvvv
// FIXME: This is still needed, but it would be better to get rid of it!
	// if the origin to relocate has a fixed depth, keep it fixed!
	if (hasFixedDepth(origin)) {
		setFixedDepth(origin->dep);
	}
// ^^^^^^^^^^^^^^^^
/*
	else
		useFixedDepth(false);
*/


	Origin* relo = _sc3relocate(origin);
	if (relo == NULL)
		return NULL;

	if (relo->dep <= _minDepth &&
	    relo->depthType != Origin::DepthManuallyFixed &&
	    ! _sc3locator->usingFixedDepth()) {

			// relocate again, this time fixing the depth to _minDepth
			// NOTE: This reconfigures the locator temporarily!
			setFixedDepth(_minDepth, true);
			Origin *relo2 = _sc3relocate(origin);
			useFixedDepth(false); // restore free depth

			if (relo2 != NULL) {
				delete relo;
				relo = relo2;
				relo->depthType = Origin::DepthMinimum;
			}
			else {
				delete relo;
				return NULL;
			}
	}

	OriginQuality &q = relo->quality;
	if ( ! determineAzimuthalGaps(relo, &q.aziGapPrimary, &q.aziGapSecondary))
		q.aziGapPrimary = q.aziGapSecondary = 360.;

	return relo;
}


Origin* Locator::_sc3relocate(const Origin *origin)
{
	// convert origin to SC3, relocate, and convert the result back

	Seiscomp::DataModel::OriginPtr sc3origin = convertToSC3(origin);
	if ( sc3origin==NULL ) {
		// give up
		SEISCOMP_ERROR("Unexpected failure to relocate origin");
		return NULL;
	}

	Seiscomp::DataModel::TimeQuantity sc3tq;
	Seiscomp::DataModel::RealQuantity sc3rq;

/*
	if(fixedDepth>=0) {
		setFixedDepth(fixedDepth);
	}
	else if(_useFixedDepth==true) {
		setFixedDepth(origin->dep);
	}
	else
		releaseDepth();
*/

	// Store SC3 Picks/Stations here so that they can be found
	// by LocSAT via SC3 PublicObject lookup
	vector<Seiscomp::DataModel::PublicObjectPtr> sc3objects;

	int arrivalCount = origin->arrivals.size();
	for (int i=0; i<arrivalCount; i++) {

		const Arrival &arr = origin->arrivals[i];
		const Seiscomp::DataModel::Phase phase(arr.phase);

		Seiscomp::DataModel::PickPtr
			sc3pick = Seiscomp::DataModel::Pick::Find(arr.pick->id);

		if ( sc3pick == NULL ) {
			sc3pick = Seiscomp::DataModel::Pick::Create(arr.pick->id);
			if ( sc3pick == NULL ) {
				SEISCOMP_ERROR_S("Locator::_sc3relocate(): Failed to create pick "+arr.pick->id+" - giving up");
				return NULL;
			}
			const Station *sta = arr.pick->station();
			Seiscomp::DataModel::WaveformStreamID wfid(sta->net, sta->code, "", "XYZ", "");
			sc3pick->setWaveformID(wfid);
			sc3tq.setValue(sc3time(arr.pick->time));
			sc3pick->setTime(sc3tq);
			sc3pick->setPhaseHint(phase);
			sc3pick->setEvaluationMode(Seiscomp::DataModel::EvaluationMode(Seiscomp::DataModel::AUTOMATIC));
		}
		sc3objects.push_back(sc3pick);
	}


	// 
	// try the actual relocation
	//
	Seiscomp::DataModel::OriginPtr sc3relo;
	try {
		// FIXME| It is strange: sometimes LocSAT requires a second
		// FIXME| invocation to produce a decent result. Reason TBD
		Seiscomp::DataModel::OriginPtr temp;
		temp    = _sc3locator->relocate(sc3origin.get());
		if (!temp) return NULL;
		sc3relo = _sc3locator->relocate(temp.get());
		if (!sc3relo) return NULL;
	}
	catch(Seiscomp::Seismology::LocatorException) {
		return NULL;
	}
	catch(Seiscomp::Seismology::PickNotFoundException) {
		SEISCOMP_WARNING("Unsuccessful location due to PickNotFoundException");
		return NULL;
	}



	// 
	// Now get the relocated origin back from SC3
	// TODO: put it into sc3adapters.cpp
	// HOWEVER: here a copy of the original origin is just updated
	//
	// A copy is made of the input origins, i.e. the Arrival attributes
	// don't get lost or have to be searched for in a complicated manner.
	// However, this relies on the order of the arrivals as returned by
	// LocSAT being the same as in the input. If not, this is absolutely
	// fatal.
	//
	Origin *relo = new Origin(*origin);
	if ( ! relo)
		return NULL;

	relo->lat     = sc3relo->latitude().value();
	try { relo->laterr  = 0.5*(sc3relo->latitude().lowerUncertainty()+sc3relo->latitude().upperUncertainty()); }
	catch ( ... ) { relo->laterr  = sc3relo->latitude().uncertainty(); }
	relo->lon     = sc3relo->longitude().value();
	try { relo->lonerr  = 0.5*(sc3relo->longitude().lowerUncertainty()+sc3relo->longitude().upperUncertainty()); }
	catch ( ... ) { relo->lonerr  = sc3relo->longitude().uncertainty(); }
	relo->dep     = sc3relo->depth().value();
	try { relo->deperr  = 0.5*(sc3relo->depth().lowerUncertainty()+sc3relo->depth().upperUncertainty()); }
	catch ( ... ) { relo->deperr  = sc3relo->depth().uncertainty(); }
	relo->time    = double(sc3relo->time().value() - Seiscomp::Core::Time());
	try { relo->timeerr = 0.5*(sc3relo->time().lowerUncertainty()+sc3relo->time().upperUncertainty()); }
	catch ( ... ) { relo->timeerr = sc3relo->time().uncertainty(); }

	relo->methodID = sc3relo->methodID();
	relo->earthModelID = sc3relo->earthModelID();

	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = relo->arrivals[i];
		const string &pickID = sc3relo->arrival(i)->pickID();

		if (arr.pick->id != pickID) {
			// If this should ever happen, let it bang loudly!
			SEISCOMP_ERROR("Locator: FATAL ERROR: Inconsistent arrival order");
			exit(1);
		}

		arr.residual = sc3relo->arrival(i)->timeResidual();
		arr.distance = sc3relo->arrival(i)->distance();
		arr.azimuth  = sc3relo->arrival(i)->azimuth();

		if ( (arr.phase == "P" || arr.phase == "P1") && arr.distance > 115)
			arr.phase = "PKP";

//		if (arr.residual == -999.)
//			arr.residual = 0; // FIXME preliminary cosmetics;

// We do not copy the weight back, because it is still there in the original arrival
//		arr.weight   = sc3relo->arrival(i)->weight();
/*
		if ( arr.residual > 800 && ( arr.phase=="P" || arr.phase=="Pdiff" ) && \
		     arr.distance > 104 && arr.distance < 112) {

			Seiscomp::TravelTime tt;
			if ( ! travelTimeP(arr.distance, origin->dep, tt))
				continue;
			arr.residual = arr.pick->time - (origin->time + tt.time);
		}
*/
	}

	relo->error.sdobs  = 1; // FIXME
	double norm        = 1./relo->error.sdobs;
	relo->error.sdepth = norm*sc3relo->depth().uncertainty() * 1.8;
	relo->error.stime  = norm*sc3relo->time().uncertainty()  * 1.8;
	relo->error.conf   = 0; // FIXME

	return relo;
}


bool determineAzimuthalGaps(const Origin *origin, double *primary, double *secondary)
{
	vector<double> azi;

	int arrivalCount = origin->arrivals.size();
	for (int i=0; i<arrivalCount; i++) {

		const Arrival &arr = origin->arrivals[i];
	
		if (arr.excluded)
			continue;

		azi.push_back(arr.azimuth);
	}

	if (azi.size() < 2)
		return false;

	sort(azi.begin(), azi.end());

	*primary = *secondary = 0.;
	int aziCount = azi.size();
	azi.push_back(azi[0] + 360.);
	azi.push_back(azi[1] + 360.);

	for (int i=0; i<aziCount; i++) {
		double gap = azi[i+1]-azi[i];
		if (gap > *primary)
			*primary = gap;
		gap = azi[i+2]-azi[i];
		if (gap > *secondary)
			*secondary = gap;
	}
	
	return true;
}



}  // namespace Autoloc
