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



#define SEISCOMP_COMPONENT LocSAT
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/seismology/locsat.h>
#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/math/geo.h>
#include "locsat_internal.h"

#include <stdlib.h>
#include <math.h>
#include <list>
#include <algorithm>
#include <fstream>
#include <sstream>

// IGN additions for OriginUncertainty computation
#include "eigv.h"
#include "chi2.h"
#define rad2deg 57.29577951308232

//#define LOCSAT_TESTING

using namespace Seiscomp::DataModel;
using namespace Seiscomp::Seismology;


namespace Seiscomp{


std::string LocSAT::_defaultTablePrefix = "iasp91";
LocSAT::IDList LocSAT::_allowedParameters;

REGISTER_LOCATOR(LocSAT, "LOCSAT");


LocSAT::~LocSAT(){
	delete _locator_params;
	if (_locateEvent)
		delete _locateEvent;
}

LocSAT::LocSAT() {
	_name = "LOCSAT";
	_newOriginID = "";
	_computeConfidenceEllipsoid = false;

	if ( _allowedParameters.empty() ) {
		_allowedParameters.push_back("MAX_ITERATIONS");
		_allowedParameters.push_back("NUM_DEG_FREEDOM");
	}

	_locator_params = new Internal::Locator_params;
	_locator_params->outfile_name = new char[1024];
	_locator_params->prefix = new char[1024];

	_locateEvent = NULL;
	_profiles.push_back("iasp91");
	_profiles.push_back("tab");
	setProfile(_defaultTablePrefix);
	setDefaultLocatorParams();
}


bool LocSAT::init(const Config::Config &config) {
	try {
		_profiles = config.getStrings("LOCSAT.profiles");
	}
	catch ( ... ) {
		_profiles.clear();
		_profiles.push_back("iasp91");
		_profiles.push_back("tab");
	}

	try {
		_computeConfidenceEllipsoid = config.getBool("LOCSAT.enableConfidenceEllipsoid");
	}
	catch ( ... ) {}

	return true;
}


LocatorInterface::IDList LocSAT::parameters() const {
	return _allowedParameters;
}


std::string LocSAT::parameter(const std::string &name) const {
	if ( name == "MAX_ITERATIONS" )
		return getLocatorParams(LP_MAX_ITERATIONS);
	else if ( name == "NUM_DEG_FREEDOM" )
		return getLocatorParams(LP_NUM_DEG_FREEDOM);

	return std::string();
}


bool LocSAT::setParameter(const std::string &name,
                          const std::string &value) {
	if ( name == "MAX_ITERATIONS" )
		setLocatorParams(LP_MAX_ITERATIONS, value.c_str());
	else if ( name == "NUM_DEG_FREEDOM" )
		setLocatorParams(LP_NUM_DEG_FREEDOM, value.c_str());
	else
		return false;

	return true;
}


void LocSAT::setNewOriginID(const std::string& newOriginID) {
	_newOriginID = newOriginID;
}


int LocSAT::capabilities() const {
	return InitialLocation | FixedDepth | IgnoreInitialLocation;
}


DataModel::Origin* LocSAT::locate(PickList& pickList,
                                  double initLat, double initLon, double initDepth,
                                  Seiscomp::Core::Time& initTime) throw(Core::GeneralException) {
	if (_locateEvent) delete _locateEvent;
	_locateEvent = new Internal::LocSAT;
	_locateEvent->setOrigin(initLat, initLon, initDepth);
	_locateEvent->setOriginTime((double)initTime);

	if ( isInitialLocationIgnored() )
		setLocatorParams(LP_USE_LOCATION, "n");
	else
		setLocatorParams(LP_USE_LOCATION, "y");

	return fromPicks(pickList);
}


DataModel::Origin* LocSAT::locate(PickList& picks) throw(Core::GeneralException) {
	if (_locateEvent) delete _locateEvent;
	_locateEvent = new Internal::LocSAT;
	_locateEvent->setOrigin(0.0, 0.0, 0.0);
	_locateEvent->setOriginTime(0.0);
	setLocatorParams(LP_USE_LOCATION, "n");

	return fromPicks(picks);
}


DataModel::Origin* LocSAT::fromPicks(PickList& picks){
	if ( _usingFixedDepth ) {
		_locator_params->fixing_depth = _fixedDepth;
		_locator_params->fix_depth = 'y';
	}
	else
		_locator_params->fix_depth = 'n';

	_locateEvent->setLocatorParams(_locator_params);

	int i = 0;

	for (PickList::iterator it = picks.begin(); it != picks.end(); ++it){
		DataModel::Pick* pick = it->first.get();
		DataModel::SensorLocation* sloc = getSensorLocation(pick);

		if ( sloc ) {
			std::string stationID = pick->waveformID().networkCode()+"."+
			                        pick->waveformID().stationCode();

			if ( !pick->waveformID().locationCode().empty() ) {
				stationID +=  ".";
				stationID += pick->waveformID().locationCode();
			}

			_locateEvent->addSite(stationID.c_str(),
			                      sloc->latitude(), sloc->longitude(),
			                      sloc->elevation());

			std::string phase;
			try { phase = pick->phaseHint().code(); }
			catch(...) { phase = "P"; }

			double cor = stationCorrection(stationID, pick->waveformID().stationCode(), phase);
			_locateEvent->addArrival(i++, stationID.c_str(),
			                         phase.c_str(),
			                         (double)pick->time().value()-cor,
			                         ARRIVAL_TIME_ERROR,
			                         it->second <= _minArrivalWeight?0:1);

			// Set backazimuth
			try {
				float az = pick->backazimuth().value();
				float delaz;
				try { delaz = pick->backazimuth().uncertainty(); }
				// Default delaz
				catch ( ... ) { delaz = 0; }
				_locateEvent->setArrivalAzimuth(az,delaz,1);
			}
			catch ( ... ) {}

			// Set slowness
			try {
				float slo = pick->horizontalSlowness().value();
				float delslo;

				try { delslo = pick->horizontalSlowness().uncertainty(); }
				// Default delaz
				catch ( ... ) { delslo = 0; }

				_locateEvent->setArrivalSlowness(slo, delslo, 1);
			}
			catch ( ... ) {}

#ifdef LOCSAT_TESTING
			SEISCOMP_DEBUG("pick station: %s", stationID.c_str());
			SEISCOMP_DEBUG("station lat: %.2f", (double)sloc->latitude());
			SEISCOMP_DEBUG("station lon: %.2f", (double)sloc->longitude());
			SEISCOMP_DEBUG("station elev: %.2f", (double)sloc->elevation());
#endif
		}
		else
			throw StationNotFoundException("station '" + pick->waveformID().networkCode() +
			                               "." + pick->waveformID().stationCode() +
			                               "." + pick->waveformID().locationCode() + "' not found");
	}

	_locateEvent->setLocatorParams(_locator_params);
	Internal::Loc* newLoc = _locateEvent->doLocation();

	DataModel::Origin* origin = loc2Origin(newLoc);

	if ( origin ) {
		std::set<std::string> stationsUsed;
		std::set<std::string> stationsAssociated;

		for ( int i = 0; i < newLoc->arrivalCount; ++i ) {
			size_t arid = (size_t)newLoc->arrival[i].arid;
			if ( arid >= picks.size() ) continue;
			Pick* p = picks[arid].first.get();

			if ( (size_t)i < origin->arrivalCount() ) {
				origin->arrival(i)->setPickID(p->publicID());

				stationsAssociated.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());

				try {
					if ( origin->arrival(i)->weight() == 0 ) continue;
				}
				catch ( ... ) {}

				stationsUsed.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());
			}
		}

		try {
			origin->quality().setUsedStationCount(stationsUsed.size());
			origin->quality().setAssociatedStationCount(stationsAssociated.size());
		}
		catch ( ... ) {
			OriginQuality oq;
			oq.setUsedStationCount(stationsUsed.size());
			oq.setAssociatedStationCount(stationsAssociated.size());
			origin->setQuality(oq);
		}
	}

	if ( newLoc) free(newLoc);
	delete _locateEvent;
	_locateEvent = NULL;

	if ( origin && _useArrivalRMSAsTimeError ) {
		double sig = 0.0;
		int used = 0;
		for ( size_t i = 0; i < origin->arrivalCount(); ++i )
			if ( origin->arrival(i)->weight() >= 0.5 ) {
				++used;
				sig += origin->arrival(i)->timeResidual() * origin->arrival(i)->timeResidual();
			}

		DataModel::OriginPtr originPtr(origin);
		origin = relocate(origin, used > 4?sqrt(sig / (used - 4)):ARRIVAL_TIME_ERROR);
	}

	return origin;
}


DataModel::Origin* LocSAT::relocate(const DataModel::Origin* origin, double timeError) {
	if ( origin == NULL ) return NULL;

	if ( isInitialLocationIgnored() )
		setLocatorParams(LP_USE_LOCATION, "n");
	else
		setLocatorParams(LP_USE_LOCATION, "y");

	if (_locateEvent) delete _locateEvent;
	_locateEvent = new Internal::LocSAT;

	double depth = 0.0;
	try { depth = origin->depth().value(); } catch (...){}

	_locateEvent->setOrigin(origin->latitude().value(),
	                        origin->longitude().value(),
	                        depth );

	_locateEvent->setOriginTime((double)origin->time().value());

	if ( !loadArrivals(origin, timeError)) {
		delete _locateEvent;
		_locateEvent = NULL;
		return NULL;
	}

	if ( _usingFixedDepth ) {
		_locator_params->fixing_depth = _fixedDepth;
		_locator_params->fix_depth = 'y';
	}
	else
		_locator_params->fix_depth = 'n';

	_locateEvent->setLocatorParams(_locator_params);
	Internal::Loc* newLoc = _locateEvent->doLocation();

	DataModel::Origin* result = loc2Origin(newLoc);

	if ( result ) {
		std::set<std::string> stationsUsed;
		std::set<std::string> stationsAssociated;

		for ( int i = 0; i < newLoc->arrivalCount; ++i ) {
			size_t arid = (size_t)newLoc->arrival[i].arid;
			if ( arid >= origin->arrivalCount() ) continue;

			if ( (size_t)i < result->arrivalCount() ) {
				result->arrival(i)->setPickID(origin->arrival(arid)->pickID());
				DataModel::Pick* p = Pick::Find(result->arrival(i)->pickID());

				if ( p != NULL )
					stationsAssociated.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());

				try {
					if ( result->arrival(i)->weight() == 0 ) continue;
				}
				catch ( ... ) {}

				if ( p != NULL )
					stationsUsed.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());
			}
		}

		try {
			result->quality().setUsedStationCount(stationsUsed.size());
			result->quality().setAssociatedStationCount(stationsAssociated.size());
		}
		catch ( ... ) {
			OriginQuality oq;
			oq.setUsedStationCount(stationsUsed.size());
			oq.setAssociatedStationCount(stationsAssociated.size());
			result->setQuality(oq);
		}
	}

	if ( newLoc) free(newLoc);
	delete _locateEvent;
	_locateEvent = NULL;

	return result;
}


DataModel::Origin* LocSAT::relocate(const DataModel::Origin* origin) throw(Core::GeneralException) {
	DataModel::Origin* o = relocate(origin, ARRIVAL_TIME_ERROR);
	if ( o && _useArrivalRMSAsTimeError ) {
		double sig = 0.0;
		int used = 0;
		for ( size_t i = 0; i < o->arrivalCount(); ++i )
			if ( o->arrival(i)->weight() >= 0.5 ) {
				++used;
				sig += o->arrival(i)->timeResidual() * o->arrival(i)->timeResidual();
			}

		DataModel::OriginPtr origin(o);
		o = relocate(o, used > 4?sqrt(sig / (used - 4)):ARRIVAL_TIME_ERROR);
	}

	return o;
}


static bool atTransitionPtoPKP(const DataModel::Arrival* arrival)
{
	return (arrival->distance() > 106.9 && arrival->distance() < 111.1);
}


// Let this be a local hack for the time being. See the same routine in Autoloc
static bool travelTimeP(double lat, double lon, double depth, double delta, double azi, TravelTime &tt)
{
	static Seiscomp::TravelTimeTable ttt;

	double lat2, lon2;
	Math::Geo::delandaz2coord(delta, azi, lat, lon, &lat2, &lon2);

	Seiscomp::TravelTimeList
		*ttlist = ttt.compute(lat, lon, depth, lat2, lon2, 0);

	if ( ttlist == NULL || ttlist->empty() )
		return false;

	for (Seiscomp::TravelTimeList::iterator
	     it = ttlist->begin(); it != ttlist->end(); ++it) {
		tt = *it;
		if (delta < 114)
			// for  distances < 114, allways take 1st arrival
			break;
		if (tt.phase.substr(0,2) != "PK")
			// for  distances >= 114, skip Pdiff etc., take first
			// PKP*, PKiKP*
			continue;
		break;
	}
	delete ttlist;

	return true;
}


double LocSAT::stationCorrection(const std::string &staid,
                                 const std::string &stacode,
                                 const std::string &phase) const {
	StationCorrectionMap::const_iterator it = _stationCorrection.find(staid);
	if ( it != _stationCorrection.end() ) {
		PhaseCorrectionMap::const_iterator pit = it->second.find(phase);
		if ( pit != it->second.end() ) {
			SEISCOMP_DEBUG("LOCSAT: stacorr(%s,%s) = %f", staid.c_str(), phase.c_str(), pit->second);
			return pit->second;
		}
	}

	it = _stationCorrection.find(stacode);
	if ( it != _stationCorrection.end() ) {
		PhaseCorrectionMap::const_iterator pit = it->second.find(phase);
		if ( pit != it->second.end() ) {
			SEISCOMP_DEBUG("LOCSAT: stacorr(%s,%s) = %f", stacode.c_str(), phase.c_str(), pit->second);
			return pit->second;
		}
	}

	return 0.0;
}


bool LocSAT::loadArrivals(const DataModel::Origin* origin, double timeError) {
	if ( ! origin)
		return false;

#ifdef LOCSAT_TESTING
	SEISCOMP_DEBUG("Load arrivals:");
#endif
	for (unsigned int i = 0; i < origin->arrivalCount(); i++){
		DataModel::Arrival* arrival = origin->arrival(i);
		DataModel::Pick* pick = getPick(arrival);
		if (!pick){
			throw PickNotFoundException("pick '" + arrival->pickID() + "' not found");
		}
		double traveltime = double(pick->time().value() - origin->time().value());

		int defining = 1;

		try{
			double arrivalWeight = arrival->weight();
// 			double arrivalWeight = rand()/(RAND_MAX + 1.0);
			if (arrivalWeight <= _minArrivalWeight){
				defining = 0;
			}
			// work around problem related to discontinuity in the travel-time tables
			// at the P->PKS transition
			if (atTransitionPtoPKP(arrival))
				defining = 0;
		}
		catch (...) {}

		DataModel::SensorLocation *sloc = getSensorLocation(pick);
		if (!sloc){
			throw StationNotFoundException("station '" + pick->waveformID().networkCode() +
			                               "." + pick->waveformID().stationCode() + "' not found");
		}

		std::string phaseCode;
		try {
			phaseCode = arrival->phase().code();
			// Rename P to PKP where appropriate. A first arrival
			// with a traveltime of > 1000 s observed at distances
			// > 110 deg is definitely not P but PKP, as PKP
			// traveltime is always well above 1000 s and P traveltime
			// is always well below.
			if ((phaseCode=="P" || phaseCode=="P1") && arrival->distance() > 110 && traveltime > 1000) {
				phaseCode="PKP";
			}
		}
		catch (...) {
			try {
				phaseCode = pick->phaseHint().code();
			}
			catch (...) {
				phaseCode = "P";
			}
		}

		/*
		// This is essentially a whitelist for automatic phases. Only
		// P and PKP are currently deemed acceptable automatic phases.
		// Others may be associated but cannot be used because of
		// this:
		if (pick->evaluationMode() == DataModel::AUTOMATIC &&
		    ! ( phaseCode == "P" || phaseCode == "PKP") ) // TODO make this configurable
			defining=0;
		*/

		std::string stationID = pick->waveformID().networkCode()+"."+
		                        pick->waveformID().stationCode();

		if ( !pick->waveformID().locationCode().empty() ) {
			stationID +=  ".";
			stationID += pick->waveformID().locationCode();
		}

#ifdef LOCSAT_TESTING
		SEISCOMP_DEBUG(" [%s] set phase to %s", stationID.c_str(), phaseCode.c_str());
#endif

		_locateEvent->addSite(stationID.c_str(),
		                      sloc->latitude(), sloc->longitude(), sloc->elevation());

		double cor = stationCorrection(stationID, pick->waveformID().stationCode(), phaseCode);

		_locateEvent->addArrival(i, stationID.c_str(),
		                         phaseCode.c_str(),
		                         (double)pick->time().value()-cor,
		                         timeError, defining);

		// Set backazimuth
		try {
			float az = pick->backazimuth().value();
			float delaz;
			try { delaz = pick->backazimuth().uncertainty(); }
			// Default delaz
			catch ( ... ) { delaz = 0; }
			_locateEvent->setArrivalAzimuth(az,delaz,1);
		}
		catch ( ... ) {}

		// Set slowness
		try {
			float slo = pick->horizontalSlowness().value();
			float delslo;

			try { delslo = pick->horizontalSlowness().uncertainty(); }
			// Default delaz
			catch ( ... ) { delslo = 0; }

			_locateEvent->setArrivalSlowness(slo, delslo,1);
		}
		catch ( ... ) {}
	}

	return true;
}


DataModel::Origin* LocSAT::loc2Origin(Internal::Loc* loc){
	if ( loc == NULL ) return NULL;

	DataModel::Origin* origin = _newOriginID.empty()
	             ?DataModel::Origin::Create()
	             :DataModel::Origin::Create(_newOriginID);
	if (!origin) return NULL;

	DataModel::CreationInfo ci;
	ci.setCreationTime(Core::Time().gmt());
	origin->setCreationInfo(ci);

	origin->setMethodID("LOCSAT");
	origin->setEarthModelID(_tablePrefix);
	origin->setLatitude(DataModel::RealQuantity(loc->origin->lat, sqrt(loc->origerr->syy), Core::None, Core::None, Core::None));
	origin->setLongitude(DataModel::RealQuantity(loc->origin->lon, sqrt(loc->origerr->sxx), Core::None, Core::None, Core::None));
	origin->setDepth(DataModel::RealQuantity(loc->origin->depth, sqrt(loc->origerr->szz), Core::None, Core::None, Core::None));

	origin->setTime(DataModel::TimeQuantity(Core::Time(loc->origin->time), sqrt(loc->origerr->stt), Core::None, Core::None, Core::None));


	double rms = 0;
	int phaseAssocCount = 0;
	int usedAssocCount = 0;
	std::vector<double> dist;
	std::vector<double> azi;
	int depthPhaseCount = 0;

	for ( int i = 0; i < loc->arrivalCount; ++i ) {
		++phaseAssocCount;

		ArrivalPtr arrival = new DataModel::Arrival();
		// To have different pickID's just generate some based on
		// the index. They become set correctly later on.
		arrival->setPickID(Core::toString(i));

		if (loc->locator_errors[i].arid != 0 ||
		    !strcmp(loc->assoc[i].timedef, "n") ){
			arrival->setWeight(0.0);
// 			SEISCOMP_DEBUG("arrival %d : setting weight to 0.0 -  because it was not used in locsat", i);
		}
		else {
			arrival->setWeight(1.0);
			rms += (loc->assoc[i].timeres * loc->assoc[i].timeres);
			dist.push_back(loc->assoc[i].delta);
			azi.push_back(loc->assoc[i].esaz);
			++usedAssocCount;
		}

		arrival->setDistance(loc->assoc[i].delta);
		arrival->setTimeResidual(loc->assoc[i].timeres < -990. ? 0. : loc->assoc[i].timeres);
		arrival->setAzimuth(loc->assoc[i].esaz);
		arrival->setPhase(Phase(loc->assoc[i].phase));
		if (arrival->phase().code()[0] == 'p' || arrival->phase().code()[0] == 's')
			if (arrival->weight() > 0.5)
				depthPhaseCount++;

		// This is a workaround for what seems to be a problem with LocSAT,
		// namely, that in a narrow distance range around 108 degrees
		// sometimes picks suddenly have a residual of > 800 sec after
		// relocation. The reason is not clear.
		if ( arrival->timeResidual() > 800 && \
		   ( arrival->phase().code()=="P" || arrival->phase().code()=="Pdiff" ) && \
		     atTransitionPtoPKP(arrival.get())) {

			TravelTime tt;
			if ( travelTimeP(origin->latitude(), origin->longitude(), origin->depth(), arrival->distance(), arrival->azimuth(), tt) ) {
				double res = loc->arrival[i].time - double(origin->time().value() + Core::TimeSpan(tt.time));
				arrival->setTimeResidual(res);
			}
		}

		// Populate horizontal slowness residual
		if ( loc->assoc[i].slores > -990. ) {
			arrival->setHorizontalSlownessResidual(loc->assoc[i].slores);
			arrival->setHorizontalSlownessUsed(true);
		}

		// Populate backazimuth residual
		if ( loc->assoc[i].azres > -990. ) {
			arrival->setBackazimuthResidual(loc->assoc[i].azres);
			arrival->setBackazimuthUsed(true);
		}

		if ( !origin->add(arrival.get()) )
			SEISCOMP_DEBUG("arrival not added for some reason");
	}

	DataModel::OriginQuality originQuality;

	originQuality.setAssociatedPhaseCount(phaseAssocCount);
	originQuality.setUsedPhaseCount(usedAssocCount);
	originQuality.setDepthPhaseCount(depthPhaseCount);

	if (phaseAssocCount > 0) {
		rms /= usedAssocCount;
		originQuality.setStandardError(sqrt(rms));
	}

	std::sort(azi.begin(), azi.end());
	azi.push_back(azi.front()+360.);
	double azGap = 0.;
	if (azi.size() > 2)
		for (size_t i = 0; i < azi.size()-1; i++)
			azGap = (azi[i+1]-azi[i]) > azGap ? (azi[i+1]-azi[i]) : azGap;
	if (0. < azGap && azGap < 360.)
		originQuality.setAzimuthalGap(azGap);

	std::sort(dist.begin(), dist.end());
	originQuality.setMinimumDistance(dist.front());
	originQuality.setMaximumDistance(dist.back());
	originQuality.setMedianDistance(dist[dist.size()/2]);

// #ifdef LOCSAT_TESTING
//	SEISCOMP_DEBUG("--- Confidence region at %4.2f level: ----------------", 0.9);
//	SEISCOMP_DEBUG("Semi-major axis:   %8.2f km", loc->origerr->smajax);
//	SEISCOMP_DEBUG("Semi-minor axis:   %8.2f km", loc->origerr->sminax);
//	SEISCOMP_DEBUG("Major axis strike: %8.2f deg. clockwise from North", loc->origerr->strike);
//	SEISCOMP_DEBUG("Depth error:       %8.2f km", loc->origerr->sdepth);
//	SEISCOMP_DEBUG("Orig. time error:  %8.2f sec", loc->origerr->stime);
//	SEISCOMP_DEBUG("--- OriginQuality ------------------------------------");
//	SEISCOMP_DEBUG("DefiningPhaseCount: %d", originQuality.definingPhaseCount());
//	SEISCOMP_DEBUG("PhaseAssociationCount: %d", originQuality.phaseAssociationCount());
//// 	SEISCOMP_DEBUG("ArrivalCount: %d", originQuality.stationCount());
//	SEISCOMP_DEBUG("Res. RMS: %f sec", originQuality.rms());
//	SEISCOMP_DEBUG("AzimuthalGap: %f deg", originQuality.azimuthalGap());
//	SEISCOMP_DEBUG("originQuality.setMinimumDistance: %f deg", originQuality.minimumDistance());
//	SEISCOMP_DEBUG("originQuality.setMaximumDistance: %f deg", originQuality.maximumDistance());
//	SEISCOMP_DEBUG("originQuality.setMedianDistance:  %f deg", originQuality.medianDistance());
//	SEISCOMP_DEBUG("------------------------------------------------------");
// #endif


	if ( _computeConfidenceEllipsoid ) {

	// IGN additions: OriginUncertainty computation

	// X axis in LocSAT is W-E and Y axis is S-N. We'll take X axis as S-N and Y axis as W-E
	// LocSAT covariance matrix is something like s2*inv(GG.T),
	// where G is the proper cov matrix and s2 the variance

	// M4d is the 4D matrix coming from LocSAT with the axis changed properly
	double M4d[16] = {	loc->origerr->syy, loc->origerr->sxy, loc->origerr->syz,loc->origerr->sty,
						loc->origerr->sxy, loc->origerr->sxx, loc->origerr->sxz,loc->origerr->stx,
						loc->origerr->syz, loc->origerr->sxz, loc->origerr->szz,loc->origerr->stz,
						loc->origerr->sty, loc->origerr->stx, loc->origerr->stz,loc->origerr->stt
					 };

	// M3d is the matrix in space
	double M3d[9] = {
						M4d[0], M4d[1], M4d[2],
						M4d[4], M4d[5], M4d[6],
						M4d[8], M4d[9], M4d[10]
					};

	// M2d is the matrix in the XY plane
	double M2d[4] = {
						M4d[0], M4d[1],
						M4d[4], M4d[5]
					};

	// Diagonalize 3D and 2D matrixes
	// We use EISPACK code

	// compute 3D and 2D eigenvalues, eigenvectors
	// EISPACK sort eigenvalues from min to max
	int ierr3;
	double eigvec3d[9];
	double eigval3d[3];
	ierr3 = rs(3, M3d, eigval3d, eigvec3d);

	int ierr2;
	double eigvec2d[4];
	double eigval2d[2];
	ierr2 = rs(2, M2d, eigval2d, eigvec2d);


	/*
	* Confidence coefficients for 1D, 2D and 3D
	* LocSAT assumes complete uncertainty knowledge.
	*
	* We use ASA091 code
	*
	* The following table summarizes confidence coefficients for 0.90 (LocSAT) and 0.68 (NonLinLoc) confidence levels
	*
	* confidenceLevel      1D             2D              3D
	*      0.90        1.6448536270    2.1459660263    2.5002777108
	*      0.68        0.9944578832    1.5095921855    1.8724001591
	*
	*
	*/

	if (ierr3 == 0 && ierr2 == 0) {
		double kppf[3];
		double g;
		int ifault;
		double dof;
		for (int i = 0; i < 3; i++) {
			dof = i + 1;
			g = alngam(dof/2.0, &ifault);
			kppf[i] = pow(ppchi2(_locator_params->conf_level, dof, g, &ifault), 0.5);
		}

		double sx, sy, smajax, sminax, strike;

		// 1D confidence intervals
		sx     = kppf[0] * pow(M4d[0], 0.5); // sxx
		sy     = kppf[0] * pow(M4d[5], 0.5); // syy


		// 1D confidence intervals
		origin->setTime(DataModel::TimeQuantity(Core::Time(loc->origin->time), sqrt(loc->origerr->stt) * kppf[0], Core::None, Core::None, _locator_params->conf_level * 100.0));
		origin->setLatitude(DataModel::RealQuantity(loc->origin->lat, sqrt(loc->origerr->syy) * kppf[0], Core::None, Core::None, _locator_params->conf_level * 100.0));
		origin->setLongitude(DataModel::RealQuantity(loc->origin->lon, sqrt(loc->origerr->sxx) * kppf[0], Core::None, Core::None, _locator_params->conf_level * 100.0));
		origin->setDepth(DataModel::RealQuantity(loc->origin->depth, sqrt(loc->origerr->szz) * kppf[0], Core::None, Core::None, _locator_params->conf_level * 100.0));

		// 2D confidence intervals
		sminax = kppf[1] * pow(eigval2d[0], 0.5);
		smajax = kppf[1] * pow(eigval2d[1], 0.5);
		strike = rad2deg * atan(eigvec2d[3] / eigvec2d[2]);
		// give the strike in the [0.0, 180.0] interval
		if (strike < 0.0) {
			strike += 180.0;
		}
		if (strike > 180.0) {
			strike -= 180.0;
		}

		// 3D confidence intervals
		double s3dMajAxis, s3dMinAxis, s3dIntAxis, MajAxisPlunge, MajAxisAzimuth, MajAxisRotation;

		s3dMinAxis = kppf[2] * pow(eigval3d[0], 0.5);
		s3dIntAxis = kppf[2] * pow(eigval3d[1], 0.5);
		s3dMajAxis = kppf[2] * pow(eigval3d[2], 0.5);

		MajAxisPlunge   = rad2deg * atan(eigvec3d[8] / pow(pow(eigvec3d[6], 2.0) + pow(eigvec3d[7], 2.0), 0.5));
		if (MajAxisPlunge < 0.0) {
			MajAxisPlunge += 180.0;
		}
		if (MajAxisPlunge > 180.0) {
			MajAxisPlunge -= 180.0;
		}

		MajAxisAzimuth  = rad2deg * atan(eigvec3d[7] / eigvec3d[6]);
		if (MajAxisAzimuth < 0.0) {
			MajAxisAzimuth += 180.0;
		}
		if (MajAxisAzimuth > 180.0) {
			MajAxisAzimuth -= 180.0;
		}

		MajAxisRotation = rad2deg * atan(eigvec3d[2] / pow(pow(eigvec3d[0], 2.0) + pow(eigvec3d[1], 2.0), 0.5));
		if (loc->origerr->szz == 0.0) {
			MajAxisRotation = 0.0;
		}
		if (MajAxisRotation < 0.0) {
			MajAxisRotation += 180.0;
		}
		if (MajAxisRotation > 180.0) {
			MajAxisRotation -= 180.0;
		}



		DataModel::ConfidenceEllipsoid confidenceEllipsoid;
		DataModel::OriginUncertainty originUncertainty;

		confidenceEllipsoid.setSemiMinorAxisLength(s3dMinAxis * 1000.0);
		confidenceEllipsoid.setSemiIntermediateAxisLength(s3dIntAxis * 1000.0);
		confidenceEllipsoid.setSemiMajorAxisLength(s3dMajAxis* 1000.0);
		confidenceEllipsoid.setMajorAxisPlunge(MajAxisPlunge);
		confidenceEllipsoid.setMajorAxisAzimuth(MajAxisAzimuth);
		confidenceEllipsoid.setMajorAxisRotation(MajAxisRotation);

		// QuakeML, horizontalUncertainty: Circular confidence region, given by single value of horizontal uncertainty.
		// Acordingly, 1D horizontal errors quadratic mean is given
		originUncertainty.setHorizontalUncertainty(sqrt(pow(sx, 2) + pow(sy, 2)));
		originUncertainty.setMinHorizontalUncertainty(sminax);
		originUncertainty.setMaxHorizontalUncertainty(smajax);
		originUncertainty.setAzimuthMaxHorizontalUncertainty(strike);
		originUncertainty.setConfidenceEllipsoid(confidenceEllipsoid);
		originUncertainty.setPreferredDescription(Seiscomp::DataModel::OriginUncertaintyDescription(Seiscomp::DataModel::ELLIPSOID));

		origin->setUncertainty(originUncertainty);

#ifdef LOCSAT_TESTING
		SEISCOMP_DEBUG("Origin quality:");
		SEISCOMP_DEBUG("    Orig. time error:       %+17.8f s", loc->origerr->stime);
		SEISCOMP_DEBUG("    Semi-major axis:        %+17.8f km", loc->origerr->smajax);
		SEISCOMP_DEBUG("    Semi-minor axis:        %+17.8f km", loc->origerr->sminax);
		SEISCOMP_DEBUG("    Major axis strike:      %+17.8f deg clockwise from North", loc->origerr->strike);
		SEISCOMP_DEBUG("    Depth error:            %+17.8f km", loc->origerr->sdepth);
		SEISCOMP_DEBUG("    AzimuthalGap:           %+17.8f deg", originQuality.azimuthalGap());
		SEISCOMP_DEBUG("    setMinimumDistance:     %+17.8f deg", originQuality.minimumDistance());
		SEISCOMP_DEBUG("    setMaximumDistance:     %+17.8f deg", originQuality.maximumDistance());
		SEISCOMP_DEBUG("    setMedianDistance:      %+17.8f deg", originQuality.medianDistance());
		SEISCOMP_DEBUG("IGN's origin uncertainty computation:");
		SEISCOMP_DEBUG("  LocSAT uncentainties:");
		SEISCOMP_DEBUG("    Semi-Major axis:        %+17.8f m", loc->origerr->smajax * 1000.0);
		SEISCOMP_DEBUG("    Semi-minor axis:        %+17.8f m", loc->origerr->sminax * 1000.0);
		SEISCOMP_DEBUG("    Major axis strike:      %+17.8f deg clockwise from North", loc->origerr->strike);
		SEISCOMP_DEBUG("    Depth error:            %+17.8f m", loc->origerr->sdepth * 1000.0);
		SEISCOMP_DEBUG("    Orig. time error:       %+17.8f s", loc->origerr->stime);
		SEISCOMP_DEBUG("  Confidence level: %4.2f %%", _locator_params->conf_level * 100.0);
		SEISCOMP_DEBUG("  Covariance matrix:");
		SEISCOMP_DEBUG("    1D uncertainties:");
		SEISCOMP_DEBUG("       Orig. time error:    %+17.8f s", stime);
		SEISCOMP_DEBUG("       X axis (S-N):        %+17.8f m", sx * 1000.0);
		SEISCOMP_DEBUG("       Y axis (W-E):        %+17.8f m", sy * 1000.0);
		SEISCOMP_DEBUG("       Depth error:         %+17.8f m", sdepth * 1000.0);
		SEISCOMP_DEBUG("    2D uncertainties:");
		SEISCOMP_DEBUG("       Semi-major axis:     %+17.8f m", smajax * 1000.0);
		SEISCOMP_DEBUG("       Semi-minor axis:     %+17.8f m", sminax * 1000.0);
		SEISCOMP_DEBUG("       Major axis strike:   %+17.8f deg clockwise from North", strike);
		SEISCOMP_DEBUG("    3D uncertainties / QuakeML:");
		SEISCOMP_DEBUG("       semiMinorAxisLength: %+17.8f m", s3dMinAxis * 1000.0);
		SEISCOMP_DEBUG("       semiMajorAxisLength: %+17.8f m", s3dMajAxis * 1000.0);
		SEISCOMP_DEBUG("       semiInterAxisLength: %+17.8f m", s3dIntAxis * 1000.0);
		SEISCOMP_DEBUG("       majorAxisPlunge:     %+17.8f deg", MajAxisPlunge);
		SEISCOMP_DEBUG("       majorAxisAzimuth:    %+17.8f deg", MajAxisAzimuth);
		SEISCOMP_DEBUG("       majorAxisRotation:   %+17.8f deg", MajAxisRotation);
		SEISCOMP_DEBUG("DEBUG INFO:");
		SEISCOMP_DEBUG("  LocSAT values:");
		SEISCOMP_DEBUG("    stx: %+17.8f", loc->origerr->stx);
		SEISCOMP_DEBUG("    sty: %+17.8f", loc->origerr->sty);
		SEISCOMP_DEBUG("    stz: %+17.8f", loc->origerr->stz);
		SEISCOMP_DEBUG("    stt: %+17.8f", loc->origerr->stt);
		SEISCOMP_DEBUG("    sxx: %+17.8f", loc->origerr->sxx);
		SEISCOMP_DEBUG("    sxy: %+17.8f", loc->origerr->sxy);
		SEISCOMP_DEBUG("    sxz: %+17.8f", loc->origerr->sxz);
		SEISCOMP_DEBUG("    syy: %+17.8f", loc->origerr->syy);
		SEISCOMP_DEBUG("    syz: %+17.8f", loc->origerr->syz);
		SEISCOMP_DEBUG("    szz: %+17.8f", loc->origerr->szz);
		SEISCOMP_DEBUG("  4D matrix:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[0], M4d[1], M4d[2], M4d[3]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[4], M4d[5], M4d[6], M4d[7]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[8], M4d[9], M4d[10], M4d[11]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[12], M4d[13], M4d[14], M4d[15]);
		SEISCOMP_DEBUG("  3D matrix:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", M3d[0], M3d[1], M3d[2]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", M3d[3], M3d[4], M3d[5]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", M3d[6], M3d[7], M3d[8]);
		SEISCOMP_DEBUG("  2D matrix:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", M2d[0], M2d[1]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", M2d[2], M2d[3]);
		SEISCOMP_DEBUG("  3D eigenvalues:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigval3d[0], eigval3d[1],eigval3d[2]);
		SEISCOMP_DEBUG("  3D eigenvectors:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigvec3d[0], eigvec3d[3],eigvec3d[6]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigvec3d[1], eigvec3d[4],eigvec3d[7]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigvec3d[2], eigvec3d[5],eigvec3d[8]);
		SEISCOMP_DEBUG("  2D eigenvalues:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", eigval2d[0], eigval2d[1]);
		SEISCOMP_DEBUG("  2D eigenvectors:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", eigvec2d[0], eigvec2d[2]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", eigvec2d[1], eigvec2d[3]);
		SEISCOMP_DEBUG("  Chi2 Percent point function:");
		SEISCOMP_DEBUG("    1D: %+10.5f", kppf[0]);
		SEISCOMP_DEBUG("    2D: %+10.5f", kppf[1]);
		SEISCOMP_DEBUG("    3D: %+10.5f", kppf[2]);
#endif


	} else {
		SEISCOMP_DEBUG("Unable to calculate eigenvalues/eigenvectors. No Origin uncertainty will be computed");
	}

	} // Closing bracket for _computeConfidenceEllipsoid == true

	origin->setQuality(originQuality);

	_errorEllipsoid.smajax = loc->origerr->smajax * 1000.0;
	_errorEllipsoid.sminax = loc->origerr->sminax * 1000.0;
	_errorEllipsoid.strike = loc->origerr->strike;
	_errorEllipsoid.sdepth = loc->origerr->sdepth * 1000.0;
	_errorEllipsoid.stime  = loc->origerr->stime;
	_errorEllipsoid.sdobs  = loc->origerr->sdobs;
	_errorEllipsoid.conf   = loc->origerr->conf * 100.0;

	return origin;
}

	//! --------------------------------------------------

void LocSAT::setDefaultLocatorParams(){
	_locator_params->cor_level		= 0;
	_locator_params->use_location 	= TRUE;
	_locator_params->fix_depth		= 'n';
	_locator_params->fixing_depth	= 20.0;
	_locator_params->verbose		= 'n';
	_locator_params->lat_init		= 999.9;
	_locator_params->lon_init		= 999.9;
	_locator_params->depth_init		= 20.0;
	_locator_params->conf_level		= 0.90;
	_locator_params->damp			= -1.00;
	_locator_params->est_std_error	= 1.00;
	_locator_params->num_dof		= 9999;
	_locator_params->max_iterations	= 100;
	_minArrivalWeight				= 0.5;
	_useArrivalRMSAsTimeError		= false;

	strcpy(_locator_params->outfile_name, "ls.out");
}


void LocSAT::setDefaultProfile(const std::string &prefix) {
	_defaultTablePrefix = prefix;
}


std::string LocSAT::currentDefaultProfile() {
	return _defaultTablePrefix;
}


LocatorInterface::IDList LocSAT::profiles() const {
	return _profiles;
}

void LocSAT::setProfile(const std::string &prefix) {
	if ( prefix.empty() ) return;

	_stationCorrection.clear();
	_tablePrefix = prefix;
	strcpy(_locator_params->prefix, (Environment::Instance()->shareDir() + "/locsat/tables/" + _tablePrefix).c_str());

	std::ifstream ifs;
	ifs.open((Environment::Instance()->shareDir() + "/locsat/tables/" + _tablePrefix + ".stacor").c_str());
	if ( !ifs.is_open() )
		SEISCOMP_DEBUG("LOCSAT: no station corrections used for profile %s", _tablePrefix.c_str());
	else {
		std::string line;
		int lc = 1;
		int cnt = 0;
		for ( ; std::getline(ifs, line); ++lc ) {
			Core::trim(line);
			if ( line.empty() ) continue;
			if ( line[0] == '#' ) continue;

			std::vector<std::string> toks;

			Core::split(toks, line.c_str(), " \t");

			if ( toks.size() != 5 ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: expected 5 columns", lc);
				continue;
			}

			if ( toks[0] != "LOCDELAY" ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: expected LOCDELAY", lc);
				continue;
			}

			int num_phases;
			double correction;

			if ( !Core::fromString(num_phases, toks[3]) ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: 4th column is not an integer", lc);
				continue;
			}

			if ( !Core::fromString(correction, toks[4]) ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: 5th column is not a double", lc);
				continue;
			}

			_stationCorrection[toks[1]][toks[2]] = correction;
			++cnt;
		}

		SEISCOMP_DEBUG("LOCSAT: loaded %d station corrections from %d configuration lines",
		               cnt, lc);
	}
}


const char* LocSAT::getLocatorParams(int param) const {
	char* value = new char[1024];

	switch(param){

	case LP_USE_LOCATION:
		if (_locator_params->use_location == TRUE)
			strcpy(value, "y");
		else
			strcpy(value, "n");
		break;

	case LP_FIX_DEPTH:
		value[0] = _locator_params->fix_depth;
		value[1] = '\0';
		break;

	case LP_FIXING_DEPTH:
		sprintf(value, "%7.2f", _locator_params->fixing_depth);
		break;

	case LP_VERBOSE:
		strcpy(value, &_locator_params->verbose);
		break;

	case LP_PREFIX:
		strcpy(value, _locator_params->prefix);
		break;

	case LP_MAX_ITERATIONS:
		sprintf(value, "%d", _locator_params->max_iterations);
		break;

	case LP_EST_STD_ERROR:
		sprintf(value, "%7.2f", _locator_params->est_std_error);
		break;

	case LP_NUM_DEG_FREEDOM:
		sprintf(value, "%d", _locator_params->num_dof);
		break;

	case LP_MIN_ARRIVAL_WEIGHT:
		sprintf(value, "%7.2f", _minArrivalWeight);
		break;

	default:
		SEISCOMP_ERROR("getLocatorParam: wrong Parameter: %d", param);
		return "error";
	}
	return value;
}


void LocSAT::setLocatorParams(int param, const char* value){
	switch ( param ) {
		case LP_USE_LOCATION:
			if (!strcmp(value, "y"))
				_locator_params->use_location = TRUE;
			else
				_locator_params->use_location = FALSE;
			break;

		case LP_FIX_DEPTH:
			_locator_params->fix_depth = value[0];
			break;

		case LP_FIXING_DEPTH:
			_locator_params->fixing_depth = atof(value);
			break;

		case LP_VERBOSE:
			if ( !strcmp(value, "y") )
				_locator_params->verbose = 'y';
			else
				_locator_params->verbose = 'n';
			break;

		case LP_PREFIX:
			strcpy(_locator_params->prefix, value);
			break;

		case LP_MAX_ITERATIONS:
			_locator_params->max_iterations = atoi(value);
			break;

		case LP_EST_STD_ERROR:
			_locator_params->est_std_error = atof(value);
			break;

		case LP_NUM_DEG_FREEDOM:
			_locator_params->num_dof = atoi(value);
			break;

		case LP_MIN_ARRIVAL_WEIGHT:
			_minArrivalWeight = atof(value);
			break;

		case LP_RMS_AS_TIME_ERROR:
			_useArrivalRMSAsTimeError = !strcmp(value, "y");
			break;

		default:
			SEISCOMP_ERROR("setLocatorParam: wrong Parameter: %d", param);
	}
}


}// of namespace Seiscomp
