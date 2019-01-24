/***************************************************************************
 * Copyright (C) 2018 by gempa GmbH                                        *
 *                                                                         *
 * All Rights Reserved.                                                    *
 *                                                                         *
 * NOTICE: All information contained herein is, and remains                *
 * the property of gempa GmbH and its suppliers, if any. The intellectual  *
 * and technical concepts contained herein are proprietary to gempa GmbH   *
 * and its suppliers.                                                      *
 * Dissemination of this information or reproduction of this material      *
 * is strictly forbidden unless prior written permission is obtained       *
 * from gempa GmbH.                                                        *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
 ***************************************************************************/


#define SEISCOMP_COMPONENT iLoc

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/version.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/system/environment.h>


#if SC_API_VERSION >= SC_API_VERSION_CHECK(10,0,0)

#include "iloc.h"
#include <iomanip>


using namespace std;
using namespace Seiscomp;


namespace Gempa {
namespace Plugins {

namespace {


float getTimeError(const Seiscomp::DataModel::Pick *pick,
                   double defaultTimeError,
                   bool useUncertainties) {
	if ( useUncertainties ) {
		try {
			return pick->time().uncertainty();
		}
		catch ( ... ) {
			try {
				return 0.5 * (pick->time().lowerUncertainty() + pick->time().upperUncertainty());
			}
			catch ( ... ) {}
		}
	}

	return defaultTimeError;
}


struct Assoc : ILOC_ASSOC {
	Assoc() {
		arid = 0;
		StaInd = 0;
		PhaseHint[0] = '\0';
		Phase[0] = '\0';
		Delta = ILOC_NULLVAL;
		Esaz = ILOC_NULLVAL;
		Seaz = ILOC_NULLVAL;
		ArrivalTime = ILOC_NULLVAL;
		Deltim = ILOC_NULLVAL;
		TimeRes = ILOC_NULLVAL;
		Timedef = 0;
		BackAzimuth = ILOC_NULLVAL;
		Delaz = ILOC_NULLVAL;
		AzimRes = ILOC_NULLVAL;
		Azimdef = 0;
		Slowness = ILOC_NULLVAL;
		Delslo = ILOC_NULLVAL;
		SlowRes = ILOC_NULLVAL;
		Slowdef = 0;
		Vmodel[0] = '\0';
	}
};


void initConfig(ILOC_CONF &cfg) {
	string auxdir = Environment::Instance()->shareDir() + "/iloc";

	// directory of auxiliary data files
	strcpy(cfg.auxdir, auxdir.c_str());
	cfg.Verbose = 1;

	// Travel time predictions
	strcpy(cfg.TTmodel, "iasp91");

	strcpy(cfg.LocalVmodel, "");
	cfg.MaxLocalTTDelta = 3.;

	// ETOPO parameters
	strcpy(cfg.EtopoFile, "etopo5_bed_g_i2.bin");
	cfg.EtopoNlon = 4321;
	cfg.EtopoNlat = 2161;
	cfg.EtopoRes = 0.0833333;

	// NA search parameters
	cfg.DoGridSearch = 1;
	cfg.NAsearchRadius = 5.;
	cfg.NAsearchDepth = 300.;
	cfg.NAsearchOT = 30.;
	cfg.NAlpNorm = 1.;
	cfg.NAiterMax = 5;
	cfg.NAcells = 25;
	cfg.NAinitialSample = 1000;
	cfg.NAnextSample = 100;

	// depth resolution
	cfg.MinDepthPhases = 3;
	cfg.MaxLocalDistDeg = 0.2;
	cfg.MinLocalStations = 2;
	cfg.MaxSPDistDeg = 2.;
	cfg.MinSPpairs = 3;
	cfg.MinCorePhases = 3;
	cfg.MaxShallowDepthError = 30.;
	cfg.MaxDeepDepthError = 60.;

	// Linearized inversion
	cfg.DoCorrelatedErrors = 1;
	cfg.SigmaThreshold = 6.;
	cfg.AllowDamping = 1;
	cfg.MinIterations = 4;
	cfg.MaxIterations = 20;
	cfg.MinNdefPhases = 4;
	cfg.DoNotRenamePhases = 0;

	// RSTT
	strcpy(cfg.RSTTmodel, (auxdir + "/RSTTmodel/rstt201404um.geotess").c_str());
	//strcpy(cfg.RSTTmodel, "");
	cfg.UseRSTTPnSn = 1;
	cfg.UseRSTTPgLg = 1;
	cfg.UseRSTT = 0;

	if ( strlen(cfg.LocalVmodel) > 1 && cfg.UseLocalTT )
		cfg.UseLocalTT = 1;
	else
		cfg.UseLocalTT = 0;
}


}


ILoc::IDList ILoc::_allowedParameters;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ILoc::AuxData::AuxData()
: tablesTT(NULL)
, tablesLocalTT(NULL)
, ec(NULL)
, useRSTT(false)
, valid(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ILoc::AuxData::~AuxData() {
	free();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ILoc::AuxData::read(const iLocConfig *config) {
	if ( valid ) free();

	if ( iLoc_ReadAuxDataFiles(const_cast<iLocConfig*>(config), &infoPhaseId,
	                           &fe, &defaultDepth, &variogram,
	                           &infoTT, &tablesTT, &ec,
	                           &infoLocalTT, &tablesLocalTT) ) {
		throw Seismology::LocatorException("iLoc: failed to read aux files");
	}

	valid = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ILoc::AuxData::free() {
	if ( !valid ) return;

	iLoc_FreeAuxData(&infoPhaseId, &fe, &defaultDepth,
	                 &variogram, &infoTT,
	                 tablesTT, ec,
	                 &infoLocalTT, tablesLocalTT,
	                 useRSTT
	                 );
	tablesTT = NULL;
	tablesLocalTT = NULL;
	ec = NULL;
	valid = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ILoc::ILoc() {
	_name = "iLoc";
	_defaultPickUncertainty = 1.0;

	if ( _allowedParameters.empty() ) {
		_allowedParameters.push_back("Verbose");
		_allowedParameters.push_back("UsePickUncertainties");
		_allowedParameters.push_back("FixOriginTime");
		_allowedParameters.push_back("FixLocation");
		_allowedParameters.push_back("DoGridSearch");
		_allowedParameters.push_back("DoNotRenamePhases");
		_allowedParameters.push_back("UseRSTT");
		_allowedParameters.push_back("MaxLocalTTDelta");
		_allowedParameters.push_back("UseLocalTT");
		_allowedParameters.push_back("MinIterations");
		_allowedParameters.push_back("MaxIterations");
		_allowedParameters.push_back("MinNdefPhases");
		_allowedParameters.push_back("SigmaThreshold");
		_allowedParameters.push_back("DoCorrelatedErrors");
		_allowedParameters.push_back("AllowDamping");
		_allowedParameters.push_back("MaxLocalDistDeg");
		_allowedParameters.push_back("MinLocalStations");
		_allowedParameters.push_back("MaxSPDistDeg");
		_allowedParameters.push_back("MinSPpairs");
		_allowedParameters.push_back("MinCorePhases");
		_allowedParameters.push_back("MinDepthPhases");
		_allowedParameters.push_back("MaxShallowDepthError");
		_allowedParameters.push_back("MaxDeepDepthError");
		_allowedParameters.push_back("DefaultPickUncertainty");
	}

	initConfig(_config);
	_usePickUncertainties = false;
	_fixTime = false;
	_fixLocation = false;

	_auxDirty = true;

	_profiles.push_back("iasp91");
	_profiles.push_back("ak135");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ILoc::~ILoc() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ILoc::init(const Seiscomp::Config::Config &config) {
	string auxdir;

	try {
		auxdir = Seiscomp::Environment::Instance()->absolutePath(config.getString("iLoc.auxDir"));
	}
	catch ( ... ) {
		auxdir = Environment::Instance()->shareDir() + "/iloc";
	}

	strcpy(_config.auxdir, auxdir.c_str());

	try {
		_profiles = config.getStrings("iLoc.profiles");
	}
	catch ( ... ) {
		_profiles.clear();
		_profiles.push_back("iasp91");
		_profiles.push_back("ak135");
	}

	try {
		_defaultPickUncertainty = config.getDouble("iLoc.defaultTimeError");
	}
	catch ( ... ) {
		_defaultPickUncertainty = 1.0;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ILoc::IDList ILoc::parameters() const {
	return _allowedParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string ILoc::parameter(const string &name) const {
#define RET_STRING(NAME) if ( name == #NAME ) return Core::toString(_config.NAME)

	     RET_STRING(Verbose);
	else if ( name == "UsePickUncertainties" )
		return Core::toString(_usePickUncertainties);
	else if ( name == "FixOriginTime" )
		return Core::toString(_fixTime);
	else if ( name == "FixLocation" )
		return Core::toString(_fixLocation);
	else RET_STRING(DoGridSearch);
	else RET_STRING(DoNotRenamePhases);
	else RET_STRING(UseRSTT);
	else RET_STRING(MaxLocalTTDelta);
	else RET_STRING(UseLocalTT);
	else RET_STRING(MinIterations);
	else RET_STRING(MaxIterations);
	else RET_STRING(MinNdefPhases);
	else RET_STRING(SigmaThreshold);
	else RET_STRING(DoCorrelatedErrors);
	else RET_STRING(AllowDamping);
	else RET_STRING(MaxLocalDistDeg);
	else RET_STRING(MinLocalStations);
	else RET_STRING(MaxSPDistDeg);
	else RET_STRING(MinSPpairs);
	else RET_STRING(MinCorePhases);
	else RET_STRING(MinDepthPhases);
	else RET_STRING(MaxShallowDepthError);
	else RET_STRING(MaxDeepDepthError);
	else if ( name == "DefaultPickUncertainty" )
		return Core::toString(_defaultPickUncertainty);

	return string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ILoc::setParameter(const string &name, const string &value) {
#define INP_STRING(NAME, TYPE) if ( name == #NAME ) { \
	TYPE v;\
	if ( !Core::fromString(v, value) )\
		return false;\
	_config.NAME = v;\
}

	     INP_STRING(Verbose, int)
	else if ( name == "UsePickUncertainties" ) {
		bool v;
		if ( !Core::fromString(v, value) )
			return false;

		_usePickUncertainties = v;
	}
	else if ( name == "FixOriginTime" ) {
		bool v;
		if ( !Core::fromString(v, value) )
			return false;

		_fixTime = v;
	}
	else if ( name == "FixLocation" ) {
		bool v;
		if ( !Core::fromString(v, value) )
			return false;

		_fixLocation = v;
	}
	else INP_STRING(DoGridSearch, int)
	else INP_STRING(DoNotRenamePhases, int)
	if ( name == "UseRSTT" ) {
		int v;
		if ( !Core::fromString(v, value) )
			return false;

		v = v ? 1 : 0;

		if ( _config.UseRSTT != v ) {
			_config.UseRSTT = v;
			// We need to re-read the aux files
			_auxDirty = true;
		}
	}
	else INP_STRING(UseRSTT, int)
	else INP_STRING(MaxLocalTTDelta, double)
	else INP_STRING(UseLocalTT, int)
	else INP_STRING(MinIterations, int)
	else INP_STRING(MaxIterations, int)
	else INP_STRING(MinNdefPhases, int)
	else INP_STRING(SigmaThreshold, double)
	else INP_STRING(DoCorrelatedErrors, int)
	else INP_STRING(AllowDamping, int)
	else INP_STRING(MaxLocalDistDeg, double)
	else INP_STRING(MinLocalStations, int)
	else INP_STRING(MaxSPDistDeg, double)
	else INP_STRING(MinSPpairs, int)
	else INP_STRING(MinCorePhases, int)
	else INP_STRING(MinDepthPhases, int)
	else INP_STRING(MaxShallowDepthError, double)
	else INP_STRING(MaxDeepDepthError, double)
	else if ( name == "DEFAULT_PICK_UNCERTAINTY" ) {
		double dpu;

		if ( !Core::fromString(dpu, value) )
			return false;

		_defaultPickUncertainty = dpu;
	}
	else
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ILoc::IDList ILoc::profiles() const {
	return _profiles;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ILoc::setProfile(const string &name) {
	if ( !strcmp(_config.TTmodel, name.c_str()) ) return;

	memset(_config.TTmodel, '\0', sizeof(_config.TTmodel));
	strncpy(_config.TTmodel, name.c_str(), sizeof(_config.TTmodel)-1);
	_auxDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ILoc::capabilities() const {
	return InitialLocation | FixedDepth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::DataModel::Origin *ILoc::locate(ILoc::PickList &pickList)
#if SC_API_VERSION < SC_API_VERSION_CHECK(11,0,0)
throw(Seiscomp::Core::GeneralException)
#endif
{
	throw Core::GeneralException("Not yet implemented");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SC_API_VERSION < 0x010C00
Seiscomp::DataModel::Origin *ILoc::locate(ILoc::PickList &pickList,
                                          double initLat, double initLon, double initDepth,
                                          Core::Time &initTime)
throw(Core::GeneralException) {
#else
Seiscomp::DataModel::Origin *ILoc::locate(ILoc::PickList &pickList,
                                          double initLat, double initLon, double initDepth,
                                          const Seiscomp::Core::Time &initTime)
#if SC_API_VERSION < SC_API_VERSION_CHECK(11,0,0)
throw(Seiscomp::Core::GeneralException)
#endif
{
#endif
	prepareAuxFiles();
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::DataModel::Origin *ILoc::relocate(const Seiscomp::DataModel::Origin *inputOrigin)
#if SC_API_VERSION < SC_API_VERSION_CHECK(11,0,0)
throw(Seiscomp::Core::GeneralException)
#endif
{
	prepareAuxFiles();

	ILOC_HYPO hypoCenter;

	hypoCenter.isManMade = 0; // ???
	hypoCenter.Time = inputOrigin->time().value();
	hypoCenter.Lat = inputOrigin->latitude().value();
	hypoCenter.Lon = inputOrigin->longitude().value();

	if ( usingFixedDepth() ) {
		hypoCenter.FixDepth = 1;
		hypoCenter.Depth = fixedDepth();
	}
	else {
		hypoCenter.FixDepth = 0;

		try {
			hypoCenter.Depth = inputOrigin->depth().value();
		}
		catch ( ... ) {
			hypoCenter.Depth = 0;
		}
	}

	hypoCenter.FixOT = _fixTime ? 1 : 0;
	hypoCenter.FixLat = _fixLocation ? 1 : 0;
	hypoCenter.FixLon = _fixLocation ? 1 : 0;
	hypoCenter.FixHypo = 0;
	hypoCenter.numPhase = inputOrigin->arrivalCount();

	vector<Assoc> assocs;
	vector<ILOC_STA> stalocs;
	map<string, int> siteIDs;

	assocs.resize(inputOrigin->arrivalCount());

	for ( size_t i = 0; i < inputOrigin->arrivalCount(); ++i ) {
		DataModel::Arrival *arrival = inputOrigin->arrival(i);

		DataModel::Pick *pick = getPick(arrival);

		if ( pick == NULL )
			throw Seismology::PickNotFoundException("pick '" + arrival->pickID() + "' not found");

		bool timeUsed = true;

#if SC3_LOCATOR_INTERFACE_VERSION >= 2
		try {
			timeUsed = arrival->timeUsed();
		}
		catch ( ... ) {}
#else
		try{
			double arrivalWeight = arrival->weight();
			if ( arrivalWeight <= _minArrivalWeight )
				timeUsed = false;
		}
		catch (...) {}
#endif

		DataModel::SensorLocation *sloc = getSensorLocation(pick);
		if ( sloc == NULL ) {
			if ( pick->waveformID().locationCode().empty() )
				throw Seismology::StationNotFoundException(
					"station '" + pick->waveformID().networkCode() +
					"." + pick->waveformID().stationCode() + "' not found"
				);
			else
				throw Seismology::StationNotFoundException(
					"sensor location '" + pick->waveformID().networkCode() +
					"." + pick->waveformID().stationCode() +
					"." + pick->waveformID().locationCode() + "' not found"
				);
		}

		float lat, lon, elev;

		try {
			lat = sloc->latitude();
		}
		catch ( ... ) {
			throw Seismology::StationNotFoundException(
				"station latitude '" + pick->waveformID().networkCode() +
				"." + pick->waveformID().stationCode() + "' not set"
			);
		}

		try {
			lon = sloc->longitude();
		}
		catch ( ... ) {
			throw Seismology::StationNotFoundException(
				"station longitude '" + pick->waveformID().networkCode() +
				"." + pick->waveformID().stationCode() + "' not set"
			);
		}

		try {
			elev = sloc->elevation();
		}
		catch ( ... ) {
			elev = 0;
		}

		std::string phaseCode;
		try {
			phaseCode = arrival->phase().code();
		}
		catch (...) {
			try {
				phaseCode = pick->phaseHint().code();
			}
			catch (...) {
				phaseCode = "P";
			}
		}

		string siteID = pick->waveformID().networkCode() + "." +
		                pick->waveformID().stationCode();

		if ( !pick->waveformID().locationCode().empty() ) {
			siteID +=  ".";
			siteID += pick->waveformID().locationCode();
		}

		siteID += "-";
		siteID += sloc->start().iso();

		int sensorID;

		map<string, int>::iterator sit = siteIDs.find(siteID);
		if ( sit != siteIDs.end() )
			sensorID = sit->second;
		else {
			sensorID = (int)stalocs.size();
			siteIDs[siteID] = sensorID;

			// Add a new location
			stalocs.push_back(ILOC_STA());
			ILOC_STA &staloc = stalocs.back();
			staloc.StaLat = lat;
			staloc.StaLon = lon;
			staloc.StaElevation = elev;
		}

		ILOC_ASSOC &assoc = assocs[i];
		assoc.arid = i;
		assoc.phaseFixed = 0;
		assoc.StaInd = sensorID;
		assoc.ArrivalTime = (double)pick->time().value();
		assoc.Deltim = getTimeError(pick, _defaultPickUncertainty, _usePickUncertainties);
		assoc.BackAzimuth = ILOC_NULLVAL;
		assoc.Delaz = ILOC_NULLVAL;
		assoc.Slowness = ILOC_NULLVAL;
		assoc.Delslo = ILOC_NULLVAL;
		assoc.Timedef = timeUsed?1:0;
		assoc.Azimdef = 0;
		assoc.Slowdef = 0;
		strcpy(assoc.PhaseHint, phaseCode.c_str());

		// Set backazimuth
#if SC3_LOCATOR_INTERFACE_VERSION >= 2
		bool backazimuthUsed = true;
		try {
			backazimuthUsed = arrival->backazimuthUsed();
		}
		catch ( ... ) {}

		if ( backazimuthUsed ) {
			try {
				float az = pick->backazimuth().value();
				float delaz;

				try { delaz = pick->backazimuth().uncertainty(); }
				// Default delaz
				catch ( ... ) { delaz = 0; }

				assoc.Azimdef = 1;
				assoc.BackAzimuth = az;
				assoc.Delaz = delaz;
			}
			catch ( ... ) {}
		}
#else
		try {
			float az = pick->backazimuth().value();
			float delaz;
			try { delaz = pick->backazimuth().uncertainty(); }
			// Default delaz
			catch ( ... ) { delaz = 0; }

			assoc.Azimdef = 1;
			assoc.BackAzimuth = az;
			assoc.Delaz = delaz;
		}
		catch ( ... ) {}
#endif

		// Set slowness
#if SC3_LOCATOR_INTERFACE_VERSION >= 2
		bool horizontalSlownessUsed = true;
		try {
			horizontalSlownessUsed = arrival->horizontalSlownessUsed();
		}
		catch ( ... ) {}

		if ( horizontalSlownessUsed ) {
			try {
				float slo = pick->horizontalSlowness().value();
				float delslo;

				try { delslo = pick->horizontalSlowness().uncertainty(); }
				// Default delaz
				catch ( ... ) { delslo = 0; }

				assoc.Slowdef = 1;
				assoc.Slowness = slo;
				assoc.Delslo = delslo;
			}
			catch ( ... ) {}
		}
#else
		try {
			float slo = pick->horizontalSlowness().value();
			float delslo;

			try { delslo = pick->horizontalSlowness().uncertainty(); }
			// Default delaz
			catch ( ... ) { delslo = 0; }

			assoc.Slowdef = 1;
			assoc.Slowness = slo;
			assoc.Delslo = delslo;
		}
		catch ( ... ) {}
#endif
	}

	hypoCenter.numSta = (int)stalocs.size();

	if ( stalocs.empty() || assocs.empty() )
		throw Seismology::LocatorException("iLoc: too few usable data");

	int res;

	res = iLoc_Locator(&_config, &_aux.infoPhaseId, &_aux.fe, &_aux.defaultDepth,
	                   &_aux.variogram, _aux.ec,
	                   &_aux.infoTT, _aux.tablesTT,
	                   &_aux.infoLocalTT, _aux.tablesLocalTT,
	                   &hypoCenter, &assocs[0], &stalocs[0]);

	if ( !res ) {
		DataModel::Origin* origin = DataModel::Origin::Create();
		if (!origin) return NULL;

		DataModel::CreationInfo ci;
		ci.setCreationTime(Core::Time().gmt());
		origin->setCreationInfo(ci);

		origin->setMethodID("iLoc");

		OPT(double) confidenceLevel = 90;

		if ( hypoCenter.Errors[2] >= 0 && hypoCenter.Errors[2] != ILOC_NULLVAL )
			origin->setLatitude(DataModel::RealQuantity(hypoCenter.Lat, Math::Geo::deg2km(hypoCenter.Errors[2]), Core::None, Core::None, confidenceLevel));
		else
			origin->setLatitude(DataModel::RealQuantity(hypoCenter.Lat));

		if ( hypoCenter.Errors[1] >= 0 && hypoCenter.Errors[1] != ILOC_NULLVAL )
			origin->setLongitude(DataModel::RealQuantity(hypoCenter.Lon, Math::Geo::deg2km(hypoCenter.Errors[1]), Core::None, Core::None, confidenceLevel));
		else
			origin->setLongitude(DataModel::RealQuantity(hypoCenter.Lon));

		if ( hypoCenter.Errors[3] >= 0 && hypoCenter.Errors[3] != ILOC_NULLVAL )
			origin->setDepth(DataModel::RealQuantity(hypoCenter.Depth, hypoCenter.Errors[3], Core::None, Core::None, confidenceLevel));
		else
			origin->setDepth(DataModel::RealQuantity(hypoCenter.Depth));

		if ( hypoCenter.Errors[0] >= 0 && hypoCenter.Errors[0] != ILOC_NULLVAL )
			origin->setTime(DataModel::TimeQuantity(Core::Time(hypoCenter.Time), hypoCenter.Errors[0], Core::None, Core::None, confidenceLevel));
		else
			origin->setTime(DataModel::TimeQuantity(Core::Time(hypoCenter.Time)));

		double rms = 0;
		int phaseAssocCount = 0;
		int usedAssocCount = 0;
		int rmsCount = 0;
		std::vector<double> dist;
		std::vector<double> azi;
		int depthPhaseCount = hypoCenter.DepthDp;

		std::set<std::string> stationsUsed;
		std::set<std::string> stationsAssociated;

		string vmodel;
		bool ambiguousVmodel = false;

		if ( fixedDepth() )
			origin->setDepthType(DataModel::OriginDepthType(DataModel::OPERATOR_ASSIGNED));

		for ( size_t i = 0; i < assocs.size(); ++i ) {
			++phaseAssocCount;

			const ILOC_ASSOC &assoc = assocs[i];

			DataModel::ArrivalPtr arrival = new DataModel::Arrival();

			arrival->setTimeUsed(assoc.Timedef ? true : false);
			arrival->setBackazimuthUsed(assoc.Azimdef ? true : false);
			arrival->setHorizontalSlownessUsed(assoc.Slowdef ? true : false);

			bool isUsed = arrival->timeUsed() || arrival->backazimuthUsed() || arrival->horizontalSlownessUsed();

			DataModel::Pick *pick = getPick(inputOrigin->arrival(assoc.arid));

			arrival->setPickID(pick->publicID());

			stationsAssociated.insert(pick->waveformID().networkCode() + "." + pick->waveformID().stationCode());

			if ( isUsed ) {
				stationsUsed.insert(pick->waveformID().networkCode() + "." + pick->waveformID().stationCode());
				arrival->setWeight(1.0);
				if ( arrival->timeUsed() ) {
					rms += (assoc.TimeRes * assoc.TimeRes);
					++rmsCount;
				}
				dist.push_back(assoc.Delta);
				azi.push_back(assoc.Esaz);
				++usedAssocCount;
			}
			else
				arrival->setWeight(0.0);

			arrival->setDistance(assoc.Delta);
			if ( assoc.TimeRes > -999.0 )
				arrival->setTimeResidual(assoc.TimeRes);
			arrival->setAzimuth(assoc.Esaz);
			arrival->setPhase(DataModel::Phase(assoc.Phase));

			// Populate horizontal slowness residual
			if ( assoc.SlowRes > -999.0 )
				arrival->setHorizontalSlownessResidual(assoc.SlowRes);

			// Populate backazimuth residual
			if ( assoc.AzimRes > -999.0 )
				arrival->setBackazimuthResidual(assoc.AzimRes);

			if ( !origin->add(arrival.get()) )
				SEISCOMP_DEBUG("arrival not added for some reason");

			if ( isUsed ) {
				if ( vmodel.empty() )
					vmodel = assoc.Vmodel;
				else if ( vmodel != assoc.Vmodel ) {
					SEISCOMP_DEBUG("Ambiguous earth model: %s != %s", vmodel.c_str(), assoc.Vmodel);
					ambiguousVmodel = true;
				}
			}
		}

		if ( !ambiguousVmodel )
			origin->setEarthModelID(vmodel);

		origin->setQuality(DataModel::OriginQuality());
		DataModel::OriginQuality &originQuality = origin->quality();

		originQuality.setAssociatedPhaseCount(phaseAssocCount);
		originQuality.setUsedPhaseCount(usedAssocCount);
		originQuality.setDepthPhaseCount(depthPhaseCount);

		/*
		if ( rmsCount > 0 ) {
			rms /= rmsCount;
			originQuality.setStandardError(sqrt(rms));
		}
		*/
		originQuality.setStandardError(hypoCenter.uRMS);

		if ( !azi.empty() ) {
			std::sort(azi.begin(), azi.end());
			azi.push_back(azi.front()+360.);
			double azGap = 0.;
			if ( azi.size() > 2 )
				for ( size_t i = 0; i < azi.size()-1; ++i )
					azGap = (azi[i+1]-azi[i]) > azGap ? (azi[i+1]-azi[i]) : azGap;
			if ( 0. < azGap && azGap < 360. )
				originQuality.setAzimuthalGap(azGap);
		}

		if ( !dist.empty() ) {
			std::sort(dist.begin(), dist.end());
			originQuality.setMinimumDistance(dist.front());
			originQuality.setMaximumDistance(dist.back());
			originQuality.setMedianDistance(dist[dist.size()/2]);
		}

		originQuality.setUsedStationCount(stationsUsed.size());
		originQuality.setAssociatedStationCount(stationsAssociated.size());

		return origin;
	}

	switch ( res ) {
		case ILOC_CANNOT_OPEN_FILE:
		case ILOC_MEMORY_ALLOCATION_ERROR:
		case ILOC_STRING_TOO_LONG:
		case ILOC_INCOMPLETE_INPUT_DATA:
		case ILOC_INSUFFICIENT_NUMBER_OF_PHASES:
		case ILOC_INSUFFICIENT_NUMBER_OF_INDEPENDENT_PHASES:
		case ILOC_PHASE_LOSS:
		case ILOC_SLOW_CONVERGENCE:
		case ILOC_SINGULAR_MATRIX:
		case ILOC_ABNORMALLY_ILL_CONDITIONED_PROBLEM:
		case ILOC_DIVERGING_SOLUTION:
		case ILOC_OUT_OF_RANGE:
		case ILOC_INVALID_DEPTH:
		case ILOC_INVALID_DELTA:
		case ILOC_INVALID_PHASE:
		case ILOC_RSTT_ERROR:
		case ILOC_NO_TRAVELTIME:
		case ILOC_UNKNOWN_ERROR:
		default:
			throw Seismology::LocatorException("iLoc: no solution found");
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string ILoc::lastMessage(MessageType) const {
	return string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ILoc::prepareAuxFiles() {
	if ( !_auxDirty ) return;
	SEISCOMP_DEBUG("Read AUX files");
	_aux.read(&_config);
	_auxDirty = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_LOCATOR(ILoc, "iLoc");

}
}


#endif
