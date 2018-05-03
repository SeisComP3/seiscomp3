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

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/publicobject.h>

#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/mean.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "sc3adapters.h"
#include "app.h"
#include "util.h"
#include "datamodel.h"

namespace Autoloc {

void delazi(double lat1, double lon1, double lat2, double lon2,
	    double &delta, double &az1, double &az2)
{
	Seiscomp::Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, &az1, &az2);
}

void delazi(const Hypocenter *hypo, const Station *station,
	    double &delta, double &az1, double &az2)
{
	Seiscomp::Math::Geo::delazi(
		   hypo->lat,    hypo->lon,
		station->lat, station->lon,
		&delta, &az1, &az2);	
}

Seiscomp::Core::Time sc3time(const Time &time)
{
	return Seiscomp::Core::Time() + Seiscomp::Core::TimeSpan(time);
}


} // namespace Autoloc

Seiscomp::DataModel::Origin *Autoloc::convertToSC3(const Autoloc::Origin* origin, bool allPhases)
{
	Seiscomp::DataModel::Origin *sc3origin
	    = Seiscomp::DataModel::Origin::Create();

	Seiscomp::DataModel::TimeQuantity sc3tq;
	Seiscomp::DataModel::RealQuantity sc3rq;

	sc3origin->setTime(Seiscomp::DataModel::TimeQuantity(Autoloc::sc3time(origin->time), origin->timeerr, Seiscomp::Core::None, Seiscomp::Core::None, Seiscomp::Core::None));
	sc3origin->setLatitude(Seiscomp::DataModel::RealQuantity(origin->lat, origin->laterr, Seiscomp::Core::None, Seiscomp::Core::None, Seiscomp::Core::None));
	sc3origin->setLongitude(Seiscomp::DataModel::RealQuantity(origin->lon, origin->lonerr, Seiscomp::Core::None, Seiscomp::Core::None, Seiscomp::Core::None));
	sc3origin->setDepth(Seiscomp::DataModel::RealQuantity(origin->dep, origin->deperr, Seiscomp::Core::None, Seiscomp::Core::None, Seiscomp::Core::None));

	sc3origin->setMethodID(origin->methodID);
	sc3origin->setEarthModelID(origin->earthModelID);

	sc3origin->setEvaluationMode(Seiscomp::DataModel::EvaluationMode(Seiscomp::DataModel::AUTOMATIC));
	if ( origin->preliminary )
		sc3origin->setEvaluationStatus(Seiscomp::DataModel::EvaluationStatus(Seiscomp::DataModel::PRELIMINARY));

	switch ( origin->depthType ) {
	case Autoloc::Origin::DepthFree:
			sc3origin->setDepthType(Seiscomp::DataModel::OriginDepthType(Seiscomp::DataModel::FROM_LOCATION));
			break;

	case Autoloc::Origin::DepthMinimum:
			break;

	case Autoloc::Origin::DepthDefault:
			break;

	case Autoloc::Origin::DepthManuallyFixed:
			sc3origin->setDepthType(Seiscomp::DataModel::OriginDepthType(Seiscomp::DataModel::OPERATOR_ASSIGNED));
			break;
	default:
			break;
	}

	// This is a preliminary fix which prevents autoloc from producing
	// origins with fixed depth, as this caused some problems at BMG
	// where the fixed-depth checkbox was not unchecked and an incorrect
	// depth was retained. Need a better way, though.
//	sc3origin->setDepthType(Seiscomp::DataModel::OriginDepthType(Seiscomp::DataModel::FROM_LOCATION));


	// Store SC3 Picks/Stations here so that they can be found
	// via SC3 PublicObject lookup
	std::vector<Seiscomp::DataModel::PublicObjectPtr> sc3objects;

	int arrivalCount = origin->arrivals.size();

	for (int i=0; i<arrivalCount; i++) {
		const Autoloc::Arrival &arr = origin->arrivals[i];

		// If not all (automatic) phases are requested, only include P and PKP
		if ( !allPhases && automatic(arr.pick.get()) && arr.phase != "P" && arr.phase != "PKP") {
			SEISCOMP_DEBUG_S("SKIPPING 1  "+arr.pick->id);
			continue;
		}

		// Don't include arrivals with huge residuals as (unless by
		// accident) these are excluded from the location anyway.
/*
		if (arr.excluded && fabs(arr.residual) > 30.) { // FIXME: quick+dirty fix
			SEISCOMP_DEBUG_S("SKIPPING 1  "+arr.pick->id);
			continue;
		}
*/
		const Seiscomp::DataModel::Phase phase(arr.phase);
		Seiscomp::DataModel::ArrivalPtr sc3arr 
		    = new Seiscomp::DataModel::Arrival();
		sc3arr->setPickID(   arr.pick->id);
		sc3arr->setDistance( arr.distance);
		sc3arr->setAzimuth(  arr.azimuth);
		sc3arr->setTimeResidual( arr.residual);
		sc3arr->setTimeUsed(arr.excluded == Arrival::NotExcluded);
		sc3arr->setWeight(arr.excluded == Arrival::NotExcluded ? 1. : 0.);
		sc3arr->setPhase(phase);

		Seiscomp::DataModel::PickPtr sc3pick
		    = Seiscomp::DataModel::Pick::Cast(
		        Seiscomp::DataModel::PublicObject::Find(arr.pick->id));

		if ( sc3pick == NULL ) {
			sc3pick = Seiscomp::DataModel::Pick::Create(arr.pick->id);
			const Autoloc::Station *sta = arr.pick->station();
			Seiscomp::DataModel::WaveformStreamID wfid(sta->net, sta->code, "", "XYZ", "");
			sc3pick->setWaveformID(wfid);
			sc3tq.setValue(Autoloc::sc3time(arr.pick->time));
			sc3pick->setTime(sc3tq);
			sc3pick->setPhaseHint(phase);

			if (arr.pick->status == Autoloc::Pick::Manual)
				sc3pick->setEvaluationMode(Seiscomp::DataModel::EvaluationMode(Seiscomp::DataModel::MANUAL));
			else
				sc3pick->setEvaluationMode(Seiscomp::DataModel::EvaluationMode(Seiscomp::DataModel::AUTOMATIC));
		}
		sc3objects.push_back(sc3pick);

		sc3origin->add(sc3arr.get());
	}

	Seiscomp::DataModel::OriginQuality oq;
	oq.setAssociatedPhaseCount(sc3origin->arrivalCount());
	oq.setUsedPhaseCount(origin->definingPhaseCount());
	oq.setAssociatedStationCount(origin->associatedStationCount());
	oq.setUsedStationCount(origin->definingStationCount());
	double msd = origin->medianStationDistance();
	if (msd>0)
		oq.setMedianDistance(msd);
	oq.setStandardError(origin->rms());

	double minDist, maxDist, aziGap;
	origin->geoProperties(minDist, maxDist, aziGap);

	oq.setMinimumDistance(minDist);
	oq.setMaximumDistance(maxDist);
	oq.setAzimuthalGap(aziGap);

	sc3origin->setQuality(oq);

	return sc3origin;
}


Autoloc::Origin *Seiscomp::Applications::Autoloc::App::convertFromSC3(const Seiscomp::DataModel::Origin *sc3origin)
{
	double lat  = sc3origin->latitude().value();
	double lon  = sc3origin->longitude().value();
	double dep  = sc3origin->depth().value();
	double time = double(sc3origin->time().value() - Seiscomp::Core::Time());

	::Autoloc::Origin *origin = new ::Autoloc::Origin(lat, lon, dep, time);

	try { origin->laterr  = 0.5*(sc3origin->latitude().lowerUncertainty() + sc3origin->latitude().upperUncertainty()); }
	catch ( ... ) {
		try { origin->laterr  = sc3origin->latitude().uncertainty(); }
		catch ( ... ) { origin->laterr = 0; }
	}

	try { origin->lonerr  = 0.5*(sc3origin->longitude().lowerUncertainty()+ sc3origin->longitude().upperUncertainty()); }
	catch ( ... ) {
		try { origin->lonerr  = sc3origin->longitude().uncertainty(); }
		catch ( ... ) { origin->lonerr = 0; }
	}

	try { origin->deperr  = 0.5*(sc3origin->depth().lowerUncertainty()    + sc3origin->depth().upperUncertainty()); }
	catch ( ... ) {
		try { origin->deperr  = sc3origin->depth().uncertainty(); }
		catch ( ... ) { origin->deperr = 0; }
	}

	try { origin->timeerr = 0.5*(sc3origin->time().lowerUncertainty()     + sc3origin->time().upperUncertainty()); }
	catch ( ... ) {
		try { origin->timeerr = sc3origin->time().uncertainty(); }
		catch ( ... ) { origin->timeerr = 0; }
	}

	int arrivalCount = sc3origin->arrivalCount();
	for (int i=0; i<arrivalCount; i++) {

		const std::string &pickID = sc3origin->arrival(i)->pickID();
/*
		Seiscomp::DataModel::Pick *sc3pick = Seiscomp::DataModel::Pick::Cast( Seiscomp::DataModel::PublicObject::Find(pickID) );
		if ( ! sc3pick) {
			SEISCOMP_ERROR_S("Pick " + pickID + " not found - cannot convert origin");
			delete origin;
			return NULL;
			// TODO:
			// Trotzdem mal schauen, ob wir den Pick nicht
			// als Autoloc-Pick schon haben
		}
*/
		const ::Autoloc::Pick *pick = ::Autoloc::Autoloc3::pick(pickID);
		if ( ! pick ) {
			// XXX FIXME: This may also happen after Autoloc cleaned up older picks, so the pick isn't available any more!
			SEISCOMP_ERROR_S("Pick " + pickID + " not found in internal pick pool - SKIPPING this pick");
			if (Seiscomp::DataModel::PublicObject::Find(pickID))
				SEISCOMP_ERROR("HOWEVER, this pick is present in pool of public objects");
			// This actually IS an error but we try to work around
			// it instead of giving up in this origin completely.
			continue;
//			delete origin;
//			return NULL;
		}

		::Autoloc::Arrival arr(pick /* , const std::string &phase="P", double residual=0 */ );
		try { arr.residual = sc3origin->arrival(i)->timeResidual(); }
		catch(...) { arr.residual = 0; SEISCOMP_WARNING("got arrival with timeResidual not set"); }
		try { arr.distance = sc3origin->arrival(i)->distance(); }
		catch(...) { arr.distance = 0; SEISCOMP_WARNING("got arrival with distance not set"); }
		try { arr.azimuth  = sc3origin->arrival(i)->azimuth(); }
		catch(...) { arr.azimuth = 0; SEISCOMP_WARNING("got arrival with azimuth not set"); }

		if (sc3origin->evaluationMode() == DataModel::MANUAL) {
			// for manual origins we do allow secondary phases like pP
			arr.phase = sc3origin->arrival(i)->phase();

			try {
				if (sc3origin->arrival(i)->timeUsed() == false)
					arr.excluded = ::Autoloc::Arrival::ManuallyExcluded;
			}
			catch(...) {
				// In a manual origin in which the time is not
				// explicitly used we treat the arrival as if
				// it was explicitly excluded.
				arr.excluded = ::Autoloc::Arrival::ManuallyExcluded;
			}
		}

		origin->arrivals.push_back(arr);
	}

	origin->publicID = sc3origin->publicID();
	try {
	// FIXME: In scolv the Origin::depthType is not set!
	Seiscomp::DataModel::OriginDepthType dtype = sc3origin->depthType();
	if ( dtype == Seiscomp::DataModel::OriginDepthType(Seiscomp::DataModel::FROM_LOCATION) )
		origin->depthType = ::Autoloc::Origin::DepthFree;
	
	else if ( dtype == Seiscomp::DataModel::OriginDepthType(Seiscomp::DataModel::OPERATOR_ASSIGNED) ) 
		origin->depthType = ::Autoloc::Origin::DepthManuallyFixed;
	}
	catch(...) {
		SEISCOMP_WARNING("Origin::depthType is not set!");
		if (sc3origin->evaluationMode() == DataModel::MANUAL &&
		    _config.adoptManualDepth == true) {
			// This is a hack! We cannot know wether the operator
			// assigned a depth manually, but we can assume the
			// depth to be opperator approved and this is better
			// than nothing.
			// TODO: Make this behavior configurable?
			origin->depthType = ::Autoloc::Origin::DepthManuallyFixed;
			SEISCOMP_WARNING("Treating depth as if it was manually fixed");
		}
		else {
			origin->depthType = ::Autoloc::Origin::DepthFree;
			SEISCOMP_WARNING("Leaving depth free");
		}
	}

	return origin;
}

Autoloc::Pick *Seiscomp::Applications::Autoloc::App::convertFromSC3(const Seiscomp::DataModel::Pick *sc3pick)
{
	const std::string &id  = sc3pick->publicID();
	const std::string &net = sc3pick->waveformID().networkCode();
	const std::string &sta = sc3pick->waveformID().stationCode();
	::Autoloc::Time time = ::Autoloc::Time(sc3pick->time().value());

	// FIXME: XXX XXX XXX   This will create duplicates if a pick with
	// that ID exists already. Make sure this cannot happen!
	::Autoloc::Pick* pick = new ::Autoloc::Pick(id,net,sta,time);

	pick->status = ::Autoloc::Utils::status(sc3pick);
	pick->loc = sc3pick->waveformID().locationCode();
	pick->cha = sc3pick->waveformID().channelCode();

	if (pick->loc=="")
		pick->loc = "__";

	pick->attachment = sc3pick;

	return pick;
}

