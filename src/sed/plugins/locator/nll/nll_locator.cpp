/***************************************************************************
* Copyright (C) 2010 by Jan Becker, gempa GmbH                             *
* EMail: jabe@gempa.de                                                     *
*                                                                          *
* This code has been developed for the SED/ETH Zurich and is               *
* released under the SeisComP Public License.                              *
***************************************************************************/


#define SEISCOMP_COMPONENT NLLocator
#define EXTERN_MODE

#include "nll_locator.h"

extern "C" {

#include "GridLib.h"
#include "ran1/ran1.h"
#include "velmod.h"
#include "GridMemLib.h"
#include "calc_crust_corr.h"
#include "phaseloclist.h"
#include "otime_limit.h"
#include "NLLocLib.h"

}

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/vector3.h>
#include <seiscomp3/utils/files.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>


ADD_SC_PLUGIN(
	"Locator implementation using NonLinLoc by Anthony Lomax "
	"(http://alomax.free.fr/nlloc/)",
	"Jan Becker, gempa GmbH",
	0, 7, 2
)


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;


namespace Seiscomp {

namespace Seismology {

namespace Plugins {

namespace {


// Global region class defining a rectangular region
// by latmin, lonmin, latmax, lonmax.
struct GlobalRegion : public NLLocator::Region {
	GlobalRegion() {
		isEmpty = true;
	}

	bool isGlobal() const { return isEmpty; }

	bool init(const Config::Config &config, const std::string &prefix) {
		vector<string> region;
		try { region = config.getStrings(prefix + "region"); }
		catch ( ... ) {}

		if ( region.empty() )
			isEmpty = true;
		else {
			isEmpty = false;

			// Parse region
			if ( region.size() != 4 ) {
				SEISCOMP_ERROR("%s: expected 4 values in region definition, got %d",
				               prefix.c_str(), (int)region.size());
				return false;
			}

			if ( !fromString(latMin, region[0]) ||
			     !fromString(lonMin, region[1]) ||
			     !fromString(latMax, region[2]) ||
			     !fromString(lonMax, region[3]) ) {
				SEISCOMP_ERROR("%s: invalid region value(s)", prefix.c_str());
				return false;
			}
		}

		return true;
	}

	bool isInside(double lat, double lon) const {
		if ( isEmpty ) return true;

		double len, dist;

		if ( lat < latMin || lat > latMax ) return false;

		len = lonMax - lonMin;
		if ( len < 0 )
			len += 360.0;

		dist = lon - lonMin;
		if ( dist < 0 )
			dist += 360.0;

		return dist <= len;
	}

	bool isEmpty;
	double latMin, lonMin;
	double latMax, lonMax;
};


// Class that implementes the SIMPLE transformation as documented
// here: http://alomax.free.fr/nlloc/
// It expects the region to be a grid 
struct SimpleTransformedRegion : public NLLocator::Region {
	bool init(const Config::Config &config, const std::string &prefix) {
		vector<string> list;

		try { list = config.getStrings(prefix + "origin"); }
		catch ( ... ) {
			SEISCOMP_ERROR("%s: missing origin definition for simple transformation",
			               prefix.c_str());
			return false;
		}

		if ( list.size() != 2 ) {
			SEISCOMP_ERROR("%s: expected 2 values in origin definition for simple transformation, got %d",
			               prefix.c_str(), (int)list.size());
			return false;
		}

		if ( !fromString(lat0, list[0]) ||
		     !fromString(lon0, list[1]) ) {
			SEISCOMP_ERROR("%s: invalid origin value(s)", prefix.c_str());
			return false;
		}

		try {
			angle = config.getDouble(prefix + "rotation");
		}
		catch ( ... ) {
			SEISCOMP_ERROR("%s: missing rotation definition for simple transformation",
			               prefix.c_str());
			return false;
		}

		try { list = config.getStrings(prefix + "region"); }
		catch ( ... ) {
			SEISCOMP_ERROR("%s: missing region definition for simple transformation",
			               prefix.c_str());
			return false;
		}

		// Parse region
		if ( list.size() != 4 ) {
			SEISCOMP_ERROR("%s: expected 4 values in region definition for simple transformation, got %d",
			               prefix.c_str(), (int)list.size());
			return false;
		}

		if ( !fromString(xmin, list[0]) ||
		     !fromString(ymin, list[1]) ||
		     !fromString(xmax, list[2]) ||
		     !fromString(ymax, list[3]) ) {
			SEISCOMP_ERROR("%s: invalid region value(s)", prefix.c_str());
			return false;
		}

		return true;
	}

	bool isInside(double lat, double lon) const {
		double lonDiff = lon - lon0;
		if ( lonDiff < -180 )
			lonDiff += 360;
		else if ( lonDiff > 180 )
			lonDiff -= 360;

		double x = Math::Geo::deg2km(lonDiff) * cos(deg2rad(lat));
		double y = Math::Geo::deg2km(lat - lat0);

		double cosa = cos(-deg2rad(angle));
		double sina = sin(-deg2rad(angle));

		double tx = x * cosa - y * sina;
		double ty = y * cosa + x * sina;

		if ( tx < xmin ) return false;
		if ( ty < ymin ) return false;
		if ( tx > xmax ) return false;
		if ( ty > ymax ) return false;

		return true;
	}

	double lat0, lon0;
	double angle;

	double xmin, xmax;
	double ymin, ymax;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void makeUpper(std::string &dest, const std::string &src) {
	dest = src;
	for ( size_t i = 0; i < src.size(); ++i )
		dest[i] = toupper(src[i]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double timeError(const TimeQuantity &t, double defaultValue) {
	try {
		return quantityUncertainty(t);
	}
	catch ( ... ) {
		return defaultValue;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void replaceWeight(vector<string> &observations, const std::string staCode,
                   double weight) {
	for ( vector<string>::iterator it = observations.begin();
	      it != observations.end(); ++it ) {
		size_t pos = it->find(' ');
		if ( pos == string::npos ) continue;
		if ( it->compare(0, pos, staCode) != 0 ) continue;

		pos = it->rfind(' ');
		if ( pos == string::npos ) continue;

		it->replace(pos+1, it->size(), Core::toString(weight)+"\n");
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double normalizeAz(double az) {
	if ( az < 0 )
		az += 360.0;
	else if ( az >= 360.0 )
		az -= 360.0;
	return az;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double normalizeLon(double lon) {
	while ( lon < -180.0 ) lon += 360.0;
	while ( lon >  180.0 ) lon -= 360.0;
	return lon;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // private namespace
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_LOCATOR(NLLocator, "NonLinLoc");

NLLocator::IDList NLLocator::_allowedParameters;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NLLocator::NLLocator() {
	_name = "NonLinLoc";
	_publicIDPattern = "NLL.@time/%Y%m%d%H%M%S.%f@.@id@";

	if ( _allowedParameters.empty() ) {
		_allowedParameters.push_back("CONTROL");
		_allowedParameters.push_back("LOCGRID");
		_allowedParameters.push_back("LOCGAU");
		_allowedParameters.push_back("LOCGAU2");
		_allowedParameters.push_back("LOCELEVCORR");
		_allowedParameters.push_back("LOCSEARCH");
		_allowedParameters.push_back("LOCMETH");
	}

	_defaultPickError = 0.5;
	_fixedDepthGridSpacing = 0.1;
	_allowMissingStations = true;
	_enableSEDParameters = false;
	_enableNLLOutput = true;
	_enableNLLSaveInput = true;

	_SEDdiffMaxLikeExpectTag = "SED.diffMaxLikeExpect";
	_SEDqualityTag = "SED.quality";

	// Set allowed parameters to unset
	for ( IDList::iterator it = _allowedParameters.begin();
	      it != _allowedParameters.end(); ++it ) {
		_parameters[*it] = "";
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NLLocator::~NLLocator() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NLLocator::init(const Config::Config &config) {
	Environment *env = Environment::Instance();

	try {
		_publicIDPattern = config.getString("NonLinLoc.publicID");
	}
	catch ( ... ) {
		_publicIDPattern = "NLL.@time/%Y%m%d%H%M%S.%f@.@id@";
	}

	try {
		_outputPath = env->absolutePath(config.getString("NonLinLoc.outputPath"));
	}
	catch ( ... ) {
		_outputPath = "/tmp/sc3.nll";
	}

	if ( !Util::pathExists(_outputPath) ) {
		if ( ! Util::createPath(_outputPath) ) {
			SEISCOMP_ERROR("NonLinLoc.outputPath: failed to create path %s", _outputPath.c_str());
			return false;
		}
	}

	if ( _outputPath.size() > 0 ) {
		if ( _outputPath[_outputPath.size()-1] != '/' )
			_outputPath += '/';
	}

	try {
		_controlFilePath = env->absolutePath(config.getString("NonLinLoc.controlFile"));
		/*
		if ( !Util::fileExists(_controlFilePath) ) {
			SEISCOMP_ERROR("NonLinLoc.controlFile: file %s does not exist",
			               _controlFilePath.c_str());
			return false;
		}
		*/
	}
	catch ( ... ) {
		_controlFilePath = "";
	}

	_currentProfile = NULL;
	bool result = true;

	_profileNames.clear();
	try { _profileNames = config.getStrings("NonLinLoc.profiles"); }
	catch ( ... ) {}

	for ( IDList::iterator it = _profileNames.begin();
	      it != _profileNames.end(); ) {

		Profile prof;
		string prefix = string("NonLinLoc.profile.") + *it + ".";

		prof.name = *it;

		try { prof.earthModelID = config.getString(prefix + "earthModelID"); }
		catch ( ... ) {}

		try { prof.methodID = config.getString(prefix + "methodID"); }
		catch ( ... ) {
			prof.methodID = "NonLinLoc";
		}

		try { prof.tablePath = env->absolutePath(config.getString(prefix + "tablePath")); }
		catch ( ... ) {}

		if ( prof.tablePath.empty() ) {
			SEISCOMP_ERROR("NonLinLoc.profile.%s: none or empty tablePath", it->c_str());
			it = _profileNames.erase(it);
			result = false;
			continue;
		}


		string regionType;
		try {
			makeUpper(regionType, config.getString(prefix + "transform"));
		}
		catch ( ... ) {
			regionType = "GLOBAL";
		}


		if ( regionType == "GLOBAL" )
			prof.region = new GlobalRegion;
		else if ( regionType == "SIMPLE" )
			prof.region = new SimpleTransformedRegion;

		if ( prof.region == NULL ) {
			SEISCOMP_ERROR("NonLinLoc.profile.%s: invalid transformation: %s",
			               it->c_str(), regionType.c_str());
			it = _profileNames.erase(it);
			result = false;
			continue;
		}

		if ( !prof.region->init(config, prefix) ) {
			SEISCOMP_ERROR("NonLinLoc.profile.%s: invalid region parameters", it->c_str());
			it = _profileNames.erase(it);
			result = false;
			continue;
		}

		try {
			prof.controlFile = env->absolutePath(config.getString(prefix + "controlFile"));
		}
		catch ( ... ) {}

		if ( prof.controlFile.empty() )
			prof.controlFile = _controlFilePath;

		if ( !Util::fileExists(prof.controlFile) ) {
			SEISCOMP_ERROR("NonLinLoc.profile.%s.controlFile: file %s does not exist",
			               it->c_str(), prof.controlFile.c_str());
			it = _profileNames.erase(it);
			result = false;
			continue;
		}

		_profiles.push_back(prof);

		++it;
	}

	_profileNames.insert(_profileNames.begin(), "automatic");

	try {
		_enableNLLOutput = config.getBool("NonLinLoc.saveIntermediateOutput");
	}
	catch ( ... ) {
		_enableNLLOutput = true;
	}

	try {
		_enableNLLSaveInput = config.getBool("NonLinLoc.saveInput");
	}
	catch ( ... ) {
		_enableNLLSaveInput = true;
	}

	try {
		_defaultPickError = config.getDouble("NonLinLoc.defaultPickError");
	}
	catch ( ... ) {
		_defaultPickError = 0.5;
	}

	try {
		_fixedDepthGridSpacing = config.getDouble("NonLinLoc.fixedDepthGridSpacing");
	}
	catch ( ... ) {
		_fixedDepthGridSpacing = 0.1;
	}

	try {
		_allowMissingStations = config.getBool("NonLinLoc.allowMissingStations");
	}
	catch ( ... ) {
		_allowMissingStations = true;
	}

	try {
		_enableSEDParameters = config.getBool("NonLinLoc.enableSEDParameters");
	}
	catch ( ... ) {
		_enableSEDParameters = false;
	}

	try {
		_SEDqualityTag = config.getString("NonLinLoc.commentSEDQuality");
	}
	catch ( ... ) {
		_SEDqualityTag = "SED.quality";
	}

	try {
		_SEDdiffMaxLikeExpectTag = config.getString("NonLinLoc.commentSEDDiffMaxLikeExpect");
	}
	catch ( ... ) {
		_SEDdiffMaxLikeExpectTag = "SED.diffMaxLikeExpect";
	}

	updateProfile("");

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NLLocator::IDList NLLocator::parameters() const {
	return _allowedParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string NLLocator::parameter(const string &name) const {
	ParameterMap::const_iterator it = _parameters.find(name);
	if ( it != _parameters.end() )
		return it->second;

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NLLocator::setParameter(const string &name,
                             const string &value) {
	ParameterMap::iterator it = _parameters.find(name);
	if ( it == _parameters.end() )
		return false;

	it->second = value;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NLLocator::IDList NLLocator::profiles() const {
	return _profileNames;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NLLocator::setProfile(const string &name) {
	if ( find(_profileNames.begin(), _profileNames.end(), name) ==
	     _profileNames.end() )
		return;

	if ( name == "automatic" )
		updateProfile("");
	else
		updateProfile(name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int NLLocator::capabilities() const {
	return FixedDepth | DistanceCutOff;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* NLLocator::locate(PickList &pickList) throw(Core::GeneralException) {
	_lastWarning = "";

	if ( pickList.empty() )
		throw LocatorException("Empty observation set");

	if ( _currentProfile == NULL ) {
		throw GeneralException("No profile set");
	}

	if ( _controlFile.empty() ) {
		throw GeneralException("Invalid control file");
	}

	SEISCOMP_DEBUG("requested earth model: %s", _currentProfile->earthModelID.c_str());

	string earthModelPath;
	bool globalMode = false;

	earthModelPath = _currentProfile->tablePath;
	globalMode = _currentProfile->region->isGlobal();

	if ( earthModelPath.empty() ) {
		if ( _profileNames.empty() )
			throw GeneralException("No earth model configured");
		else
			throw GeneralException("Wrong earth model set");
	}


	TextLines obs, params;
	std::vector<string> observationIDs;

	// copy user set parameters to param strings
	for ( ParameterMap::iterator it = _parameters.begin();
	      it != _parameters.end(); ++it )
		if ( !it->second.empty() )
			params.push_back(it->first + " " + it->second);

	PickList usedPicks;

	// create observation buffer
	for ( PickList::iterator it = pickList.begin();
	      it != pickList.end(); ++it )
	{
		Pick *pick = it->first.get();

		SensorLocation *sloc = getSensorLocation(pick);
		if ( sloc == NULL ) {
			if ( _allowMissingStations ) {
				// Append a new line to the warning message
				if ( !_lastWarning.empty() )
					_lastWarning += "\n";
				_lastWarning += "Sensor location ";
				_lastWarning += pick->waveformID().networkCode() + "." +
				                pick->waveformID().stationCode() + "." +
				                pick->waveformID().locationCode() + " is not available and ignored";
				continue;
			}
			else {
				throw StationNotFoundException(
					(string("station ") + pick->waveformID().networkCode() +
				  "." + pick->waveformID().stationCode() +
				  " not found").c_str()
				);
			}
		}

		usedPicks.push_back(*it);

		// create the LOCSRCE entries 
		params.push_back(string("LOCSRCE ") +
		                 pick->waveformID().stationCode() +
		                 " LATLON " +
		                 toString(sloc->latitude()) + " " +
		                 toString(sloc->longitude()) + " 0 " +
		                 toString(sloc->elevation()*0.001));
		stringstream ss;
		ss << setprecision(2);
		ss.setf(ios_base::scientific, ios_base::floatfield);

		ss // station code
		   << left << setw(6) << pick->waveformID().stationCode()
		   << internal << setw(0) << " "
		   // instrument
		   << "? "
		   // component
		   << (pick->waveformID().channelCode().size() > 2
		       ?
		         pick->waveformID().channelCode().substr(pick->waveformID().channelCode().size()-1)
		       :
		         "?"
		       ) << " "
		   // P phase onset (i, e)
		   << "? "
		   // phase descriptor
		   << pick->phaseHint().code() << " "
		   // first motion
		   << "? "
		   // date
		   << pick->time().value().toString("%Y%m%d") << " "
		   // hour Minute
		   << pick->time().value().toString("%H%M") << " "
		   // seconds
		   << pick->time().value().toString("%S.%4f") << " "
		   // error type
		   << "GAU "
		   // error magnitude
		   << timeError(pick->time(), _defaultPickError) << " "
		   // coda duration
		   << -1.0 << " "
		   // amplitude
		   << -1.0 << " "
		   // period
		   << -1.0 << " "
		   // priorWt
		   << it->second << endl;

		// NOTE: The weight must be the last value otherwise the method "replaceWeight"
		//       needs to be changed.
		obs.push_back(ss.str());
	}

	if ( obs.empty() )
		throw LocatorException("Empty observation set due to missing stations");

	Origin *origin;
	if ( !_publicIDPattern.empty() ) {
		origin = Origin::Create("");
		PublicObject::GenerateId(origin, _publicIDPattern);
	}
	else
		origin = Origin::Create();

	SEISCOMP_DEBUG("New origin publicID: %s", origin->publicID().c_str());
	string outputPath = _outputPath + origin->publicID();

	if ( globalMode )
		params.push_back("LOCFILES - NLLOC_OBS " + earthModelPath + " " + outputPath + " 1");
	else
		params.push_back("LOCFILES - NLLOC_OBS " + earthModelPath + " " + outputPath);

	if ( globalMode )
		params.push_back("TRANS GLOBAL");

	if ( _usingFixedDepth ) {
		bool foundLocGrid = false;

		for ( TextLines::iterator it = params.begin(); it != params.end(); ++it ) {
			size_t pos = it->find_first_of(" \t\r\n");
			if ( pos != string::npos ) {
				if ( it->compare(0, pos, "LOCGRID") != 0 )
					continue;
			}
			else if ( *it != "LOCGRID") continue;

			// Modify LOCGRID element
			vector<string> toks;
			Core::split(toks, it->c_str(), " \t\r\n", true);

			if ( toks.size() < 10 ) {
				delete origin;
				throw GeneralException((string("Unable to fix depth: invalid LOCGRID line in ") +
				                        _currentProfile->controlFile).c_str());
			}
			// modify num_grid_z, orig_grid_z and d_grid_z

			toks[3] = "2";
			toks[6] = Core::toString(_fixedDepth);
			toks[9] = Core::toString(_fixedDepthGridSpacing);

			*it = "";
			for ( size_t i = 0; i < toks.size(); ++i ) {
				if ( i > 0 ) *it += ' ';
				*it += toks[i];
			}

			foundLocGrid = true;
		}

		if ( !foundLocGrid ) {
			delete origin;
			throw GeneralException((string("Unable to fix depth: LOCGRID line not found in ") +
			                        _currentProfile->controlFile).c_str());
		}
	}

	// Suppress physical NLL output it will be done later manually
	if ( _enableSEDParameters )
		params.push_back("LOCHYPOUT NONE CALC_SED_ORIGIN");
	else
		params.push_back("LOCHYPOUT NONE");

	std::vector<char*> obs_buf, control_buf;

	obs_buf.resize(obs.size());
	for ( size_t i = 0; i < obs.size(); ++i )
		obs_buf[i] = &obs[i][0];

	control_buf.resize(_controlFile.size() + params.size());
	for ( size_t i = 0; i < _controlFile.size(); ++i )
		control_buf[i] = &_controlFile[i][0];

	for ( size_t i = 0; i < params.size(); ++i )
		control_buf[_controlFile.size()+i] = &params[i][0];

	// call taken from NLL_func_test
	int return_locations = 1;
	int return_oct_tree_grid = 1;
	int return_scatter_sample = 1;
	LocNode *loc_list_head = NULL;

	int istat = NLLoc(NULL, NULL,
	                  &control_buf[0], (int)control_buf.size(),
	                  &obs_buf[0], (int)obs_buf.size(), return_locations,
	                  return_oct_tree_grid, return_scatter_sample, &loc_list_head);

	SEISCOMP_DEBUG("NLLoc returned with code %d", istat);

	int id = 0;
	LocNode *locNode = getLocationFromLocList(loc_list_head, id);
	bool validOrigin = false;

	for ( ; locNode != NULL; locNode = getLocationFromLocList(loc_list_head, ++id) ) {
		SEISCOMP_DEBUG("Processing node");

		validOrigin = NLL2SC3(origin, _lastWarning, locNode, usedPicks, _usingFixedDepth);

		if ( validOrigin ) {
			bool rejectedLocation = false;
			try {
				rejectedLocation = origin->evaluationStatus() == REJECTED;
			}
			catch ( ... ) {}

			if ( _enableDistanceCutOff && !rejectedLocation ) {
				// Update input weights for stations within distance
				// greater that the cut-off
				for ( PickList::iterator it = usedPicks.begin();
				      it != usedPicks.end(); ++it ) {
					Pick *pick = it->first.get();

					SensorLocation *sloc = getSensorLocation(pick);
					if ( sloc == NULL ) continue;
					double dist, az, baz;

					// Compute distance from origin to station
					Math::Geo::delazi(origin->latitude(), origin->longitude(),
					                  sloc->latitude(), sloc->longitude(),
					                  &dist, &az, &baz);

					dist = Math::Geo::deg2km(dist);
					if ( dist > _distanceCutOff )
						replaceWeight(obs, pick->waveformID().stationCode(), 0);
				}

				// Rebuild observation buffer
				for ( size_t i = 0; i < obs.size(); ++i )
					obs_buf[i] = &obs[i][0];

				// Free previous results
				freeLocList(loc_list_head, 1);

				// call NLL again
				loc_list_head = NULL;
				id = 0;
				istat = NLLoc(NULL, NULL,
				              &control_buf[0], (int)control_buf.size(),
				              &obs_buf[0], (int)obs_buf.size(), return_locations,
				              return_oct_tree_grid, return_scatter_sample, &loc_list_head);

				SEISCOMP_DEBUG("NLLoc 2nd call returned with code %d", istat);

				validOrigin = false;
				locNode = getLocationFromLocList(loc_list_head, id);

				if ( locNode ) {
					// Create a new origin with the same publicID
					std::string publicID = origin->publicID();
					delete origin;
					origin = Origin::Create(publicID);
					validOrigin = NLL2SC3(origin, _lastWarning, locNode, usedPicks, _usingFixedDepth);
				}
				else {
					delete origin;
					freeLocList(loc_list_head, 1);
					throw LocatorException("Distance cut-off failed: empty location");
				}

				if ( !validOrigin ) break;
			}

			// fill additional values
			origin->setEarthModelID(_currentProfile?_currentProfile->earthModelID:"");
			origin->setMethodID(_currentProfile?_currentProfile->methodID:"NonLinLoc");

			if ( _enableNLLOutput ) {
				// write NLLoc Hypocenter-Phase file to disk
				if ( WriteLocation(NULL, locNode->plocation->phypo, locNode->plocation->parrivals,
				                   locNode->plocation->narrivals, const_cast<char*>((outputPath + ".loc.hyp").c_str()),
				                   1, 1, 0, locNode->plocation->pgrid, 0) < 0 )
					SEISCOMP_ERROR("Failed writing location to event file: %s", (outputPath + ".loc.hyp").c_str());

				// write NLLoc location Grid Header file to disk
				if ( WriteGrid3dHdr(locNode->plocation->pgrid, NULL,
				                    const_cast<char*>(outputPath.c_str()),
				                    const_cast<char*>("loc")) < 0 )
					SEISCOMP_ERROR("Failed writing grid header to disk: %s", outputPath.c_str());

				// write NLLoc location Oct tree structure of locaiton likelihood values to disk
				if ( return_oct_tree_grid ) {
					FILE *fpio = fopen((outputPath + ".loc.octree").c_str(), "w");
					if ( fpio != NULL ) {
						istat = writeTree3D(fpio, locNode->plocation->poctTree);
						fclose(fpio);
						SEISCOMP_INFO("Oct tree structure written to file: %d nodes", istat);
					}
					else
						SEISCOMP_ERROR("Failed writing octree grid: %s", (outputPath + ".loc.octree").c_str());
				}

				// write NLLoc binary Scatter file to disk
				if ( return_scatter_sample ) {
					FILE *fpio = fopen((outputPath + ".loc.scat").c_str(), "w");
					if ( fpio != NULL ) {
						// write scatter file header informaion
						fseek(fpio, 0, SEEK_SET);
						fwrite(&(locNode->plocation->phypo->nScatterSaved), sizeof(int), 1, fpio);
						float ftemp = (float) locNode->plocation->phypo->probmax;
						fwrite(&ftemp, sizeof(float), 1, fpio);
						// skip header record
						fseek(fpio, 4 * sizeof(float), SEEK_SET);
						// write scatter samples
						fwrite(locNode->plocation->pscatterSample, 4 * sizeof(float), locNode->plocation->phypo->nScatterSaved, fpio);
						fclose(fpio);
					}
					else
						SEISCOMP_ERROR("Failed writing scatter file: %s", (outputPath + ".loc.scat").c_str());
				}
			}
		}

		// only the first location is taken into account for now
		break;
	}

	if ( _enableNLLSaveInput ) {
		// Save NLL observation input
		ofstream obsOut((outputPath + ".obs").c_str());
		for ( size_t i = 0; i < obs_buf.size(); ++i )
			obsOut << obs_buf[i];
		obsOut.close();

		// Save NLL control input
		ofstream controlOut((outputPath + ".conf").c_str());
		for ( size_t i = 0; i < control_buf.size(); ++i )
			controlOut << control_buf[i] << endl;
		controlOut.close();
	}

	// clean up
	freeLocList(loc_list_head, 1);

	if ( !validOrigin ) {
		delete origin;
		origin = NULL;
	}

	if ( locNode == NULL )
		throw LocatorException("Empty location");

	return origin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* NLLocator::locate(PickList& pickList,
                           double initLat, double initLon, double initDepth,
                           Time &initTime) throw(Core::GeneralException) {
	return locate(pickList);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* NLLocator::relocate(const Origin* origin) throw(Core::GeneralException) {
	_lastWarning = "";

	if ( origin == NULL ) return NULL;

	double lat = origin->latitude().value();
	double lon = origin->longitude().value();
	bool   emptyProfile = false;

	SEISCOMP_DEBUG("Relocating origin with publicID: %s", origin->publicID().c_str());

	if ( _currentProfile == NULL ) {
		// Find best earth model based on region information and
		// the initial origin
		emptyProfile = true;
		for ( Profiles::iterator it = _profiles.begin();
		      it != _profiles.end(); ++it ) {
			// if epicenter is inside the configured region, use it
			if ( it->region->isInside(lat, lon) ) {
				updateProfile(it->name);
				break;
			}
		}

		SEISCOMP_DEBUG("matching earth model: %s",
		               _currentProfile == NULL?"none":_currentProfile->earthModelID.c_str());
	}

	if ( _currentProfile == NULL ) {
		throw LocatorException(
			string("No matching earth model found "
			       "for location (lat: ") + Core::toString(lat) +
			       ", lon: " + Core::toString(lon) + ")");
	}

	PickList picks;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		double weight = 1.0;
		try {
			if ( origin->arrival(i)->weight() <= 0 )
				weight = 0.0;
		}
		catch ( ... ) {}

		PickPtr pick = getPick(origin->arrival(i));
		if ( pick != NULL ) {
			try {
				// Phase definition of arrival and pick different?
				if ( pick->phaseHint().code() != origin->arrival(i)->phase().code() ) {
					PickPtr np = new Pick(*pick);
					np->setPhaseHint(origin->arrival(i)->phase());
					pick = np;
				}
			}
			catch ( ... ) {
				// Pick has no phase hint?
				PickPtr np = new Pick(*pick);
				np->setPhaseHint(origin->arrival(i)->phase());
				pick = np;
			}

			picks.push_back(WeightedPick(pick,weight));

		}
		else {
			if ( emptyProfile ) _currentProfile = NULL;

			throw PickNotFoundException(
				"pick '" + origin->arrival(i)->pickID() + "' not found"
			);
		}
	}

	Origin *org;

	/*
	// NOTE: DEBUG code
	if ( picks.size() > 20 )
		throw LocatorException("Too many picks (only 20 picks allowed)");
	*/

	try {
		org = locate(picks);
	}
	catch ( GeneralException &exc ) {
		if ( emptyProfile ) _currentProfile = NULL;
		throw exc;
	}

	// Only try another profile if the automatic profile is active
	if ( org && emptyProfile ) {
		vector<RegionPtr> blacklist;

		while ( true ) {
			// Origin.evaluationStatus != REJECT send this one
			try {
				if ( org->evaluationStatus() != REJECTED )
					break;
			}
			catch ( ... ) {
				break;
			}

			Origin *lastWorkingOrg = org;
			SEISCOMP_DEBUG("Try to find another locator profile");

			// Otherwise find a less restrictive profile based on the
			// input region
			blacklist.push_back(_currentProfile->region);
			_currentProfile = NULL;

			for ( Profiles::iterator it = _profiles.begin();
			      it != _profiles.end(); ++it ) {
				// Skip profiles with blacklisted regions
				if ( std::find(blacklist.begin(), blacklist.end(), it->region) != blacklist.end() )
					continue;

				// if epicenter is inside the configured region, use it
				if ( it->region->isInside(lat, lon) ) {
					updateProfile(it->name);
					break;
				}
			}

			if ( _currentProfile ) {
				SEISCOMP_DEBUG("next earth model: %s", _currentProfile->earthModelID.c_str());

				try {
					org = locate(picks);
				}
				catch ( GeneralException &exc ) {
					delete lastWorkingOrg;
					_currentProfile = NULL;
					throw exc;
				}

				if ( org == NULL )
					org = lastWorkingOrg;
				else {
					// New origin available => delete the old one
					delete lastWorkingOrg;
					lastWorkingOrg = NULL;
				}
			}
			else {
				SEISCOMP_DEBUG("No profile found: returning the last origin with id: %s",
				               lastWorkingOrg->publicID().c_str());
				org = lastWorkingOrg;
				break;
			}
		}

		_currentProfile = NULL;
	}

	return org;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string NLLocator::lastMessage(MessageType type) const {
	if ( type == Warning )
		return _lastWarning;

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NLLocator::updateProfile(const std::string &name) {
	_currentProfile = NULL;

	Profile *prof = NULL;
	for ( Profiles::iterator it = _profiles.begin();
	      it != _profiles.end(); ++it ) {
		if ( it->name != name ) continue;

		prof = &(*it);
		break;
	}

	if ( prof == _currentProfile ) return;

	_currentProfile = prof;
	_controlFile.clear();

	// Unset all parameters
	for ( ParameterMap::iterator it = _parameters.begin();
	      it != _parameters.end(); ++it )
		it->second = "";

	if ( _currentProfile ) {
		string controlFile;
		if ( !_currentProfile->controlFile.empty() )
			controlFile = _currentProfile->controlFile;
		else if ( !_controlFilePath.empty() )
			controlFile = _controlFilePath;

		if ( !controlFile.empty() ) {
			SEISCOMP_DEBUG("Reading control file: %s", controlFile.c_str());
			ifstream f(controlFile.c_str());
			if ( !f.is_open() ) {
				SEISCOMP_ERROR("NonLinLoc: unable to open control file at %s",
				               controlFile.c_str());
				return;
			}

			while ( f.good() ) {
				string line;
				getline(f, line);
				Core::trim(line);
				// ignore empty lines
				if ( line.empty() ) continue;
				// ignore comments
				if ( line[0] == '#' ) continue;

				size_t pos = line.find_first_of(" \t\r\n");
				if ( pos != string::npos ) {
					string param = line.substr(0, pos);
					// Lookup the parameter name in the local parameter map
					// If not available pass it to NLL directly without being able
					// to modify it.
					ParameterMap::iterator it = _parameters.find(param);
					if ( it == _parameters.end() )
						_controlFile.push_back(line);
					else {
						it->second = line.substr(pos+1);
						Core::trim(it->second);
					}
				}
				else
					_controlFile.push_back(line);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NLLocator::NLL2SC3(Origin *origin, string &locComment, const void *vnode,
                        const NLLocator::PickList &picks,
                        bool depthFixed) {
	const LocNode *node = static_cast<const LocNode*>(vnode);
	set<Pick*> associatedPicks;

	SEISCOMP_DEBUG("Convert NLL origin to SC3");
	locComment = "";

	HypoDesc *phypo = node->plocation->phypo;

	Time ot;
	ot.set(phypo->year, phypo->month, phypo->day,
	       phypo->hour, phypo->min, (int)floor(phypo->sec),
	       (int)((phypo->sec-floor(phypo->sec))*1.0E6));

	double lat_error = sqrt(phypo->cov.yy);
	double lon_error = sqrt(phypo->cov.xx);
	double dep_error = sqrt(phypo->cov.zz);

	//bool globalMode = GeometryMode == MODE_GLOBAL;

	CreationInfo ci;
	ci.setCreationTime(Core::Time().gmt());
	origin->setCreationInfo(ci);

	origin->setTime(ot);
	origin->setLatitude(RealQuantity(phypo->dlat, lat_error, None, None, None));
	origin->setLongitude(RealQuantity(normalizeLon(phypo->dlong), lon_error, None, None, None));

	if ( depthFixed )
		origin->setDepth(RealQuantity(phypo->depth));
	else
		origin->setDepth(RealQuantity(phypo->depth, dep_error, None, None, None));

	for ( int i = 0; i < node->plocation->narrivals; ++i ) {
		ArrivalDesc *parr = node->plocation->parrivals + i;

		if ( parr->original_obs_index < 0 ||
		     parr->original_obs_index >= (int)picks.size() ) {
			SEISCOMP_WARNING("NLL: ignoring invalid arrival for observation id %d",
			                 parr->original_obs_index);
			continue;
		}

		PickPtr pick = picks[parr->original_obs_index].first;
		associatedPicks.insert(pick.get());

		DataModel::ArrivalPtr arr = new DataModel::Arrival;
		arr->setPickID(pick->publicID());
		arr->setPhase(Phase(parr->phase));
		arr->setTimeResidual(parr->residual);
		if ( parr->ray_dip >= 0 && parr->ray_qual > iAngleQualityMin )
			arr->setTakeOffAngle(parr->ray_dip);
		arr->setWeight(parr->weight);
		arr->setDistance(Math::Geo::km2deg(parr->dist));
		arr->setAzimuth(normalizeAz(rect2latlonAngle(0,parr->azim)));
		arr->setTimeCorrection(parr->delay);

		origin->add(arr.get());
	}

	// Associate all remaining picks with unknown residual and weight 0
	for ( size_t i = 0; i < picks.size(); ++i ) {
		if ( associatedPicks.find(picks[i].first.get()) != associatedPicks.end() )
			continue;

		PickPtr pick = picks[i].first;

		// Skip unknown station
		SensorLocation *sloc = getSensorLocation(pick.get());
		if ( sloc == NULL ) continue;

		// Compute distance and azimuth
		double dist, az, baz;
		Math::Geo::delazi(phypo->dlat, phypo->dlong,
		                  sloc->latitude(), sloc->longitude(),
		                  &dist, &az, &baz);

		std::cout << "delazi: " << origin->latitude() << "," << origin->longitude()
		          << " - " << sloc->latitude() << "," << sloc->longitude()
		          << " = " << az << std::endl;

		DataModel::ArrivalPtr arr = new DataModel::Arrival;
		arr->setPickID(pick->publicID());
		arr->setPhase(Phase(pick->phaseHint()));
		arr->setWeight(0.0);
		arr->setDistance(dist);
		arr->setAzimuth(normalizeAz(az));

		origin->add(arr.get());
	}


	// Set origin quality
	OriginQuality oq;
	oq.setStandardError(phypo->rms);
	oq.setAzimuthalGap(phypo->gap);
	oq.setSecondaryAzimuthalGap(phypo->gap_secondary);

	if ( phypo->usedStationCount >= 0 )
		oq.setUsedStationCount(phypo->usedStationCount);

	if ( phypo->associatedStationCount >= 0 )
		oq.setAssociatedStationCount(phypo->associatedStationCount);

	if ( phypo->associatedPhaseCount >= 0 )
		oq.setAssociatedPhaseCount(phypo->associatedPhaseCount);

	if ( phypo->depthPhaseCount >= 0 )
		oq.setDepthPhaseCount(phypo->depthPhaseCount);

	oq.setUsedPhaseCount(phypo->nreadings);
	oq.setMinimumDistance(Math::Geo::km2deg(phypo->minimumDistance));
	oq.setMaximumDistance(Math::Geo::km2deg(phypo->maximumDistance));
	oq.setMedianDistance(Math::Geo::km2deg(phypo->medianDistance));

	oq.setGroundTruthLevel(phypo->groundTruthLevel);

	// Set origin uncertainty
	OriginUncertainty ou;
	ConfidenceEllipsoid ce;

	// Create confidence ellipsoid
	ce.setSemiMajorAxisLength(phypo->ellipsoid.len3);
	ce.setSemiMinorAxisLength(phypo->ellipsoid.len1);
	ce.setSemiIntermediateAxisLength(phypo->ellipsoid.len2);

	Math::Vector3d a,b,c;

	double ellipsoid_az1 = rect2latlonAngle(0, phypo->ellipsoid.az1);
	double ellipsoid_az2 = rect2latlonAngle(0, phypo->ellipsoid.az2);

	a.fromAngles(deg2rad(ellipsoid_az1), deg2rad(phypo->ellipsoid.dip1));
	b.fromAngles(deg2rad(ellipsoid_az2), deg2rad(phypo->ellipsoid.dip2));
	c.cross(a,b);

	double az,dip;
	c.toAngles(az,dip);

	ce.setMajorAxisPlunge(rad2deg(dip));
	ce.setMajorAxisAzimuth(rad2deg(az));
	ce.setMajorAxisRotation(90-phypo->ellipsoid.dip1);

	ou.setConfidenceEllipsoid(ce);

	ou.setHorizontalUncertainty(phypo->ellipse.len2);
	ou.setMinHorizontalUncertainty(phypo->ellipse.len1);
	ou.setMaxHorizontalUncertainty(phypo->ellipse.len2);

	double azMaxHorUnc = phypo->ellipse.az1 + 90.0;
	if ( azMaxHorUnc >= 360.0 )
		azMaxHorUnc -= 360.0;
	if ( azMaxHorUnc >= 180.0 )
		azMaxHorUnc -= 180.0;

	ou.setAzimuthMaxHorizontalUncertainty(azMaxHorUnc);

	origin->setUncertainty(ou);
	origin->setQuality(oq);

	if ( phypo->qualitySED != '\0' ) {
		CommentPtr comment = new Comment;
		comment->setId(_SEDqualityTag);
		comment->setText(string(1, phypo->qualitySED));
		origin->add(comment.get());

		comment = new Comment;
		comment->setId(_SEDdiffMaxLikeExpectTag);
		comment->setText(toString(phypo->diffMaxLikeExpect));
		origin->add(comment.get());
	}

	if ( strcmp(phypo->locStat, "LOCATED") != 0 ) {
		origin->setEvaluationStatus(EvaluationStatus(REJECTED));
		locComment = phypo->locStatComm;
		SEISCOMP_WARNING("NonLinLoc: origin %s has been rejected: %s",
		                 origin->publicID().c_str(), locComment.c_str());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}

}
