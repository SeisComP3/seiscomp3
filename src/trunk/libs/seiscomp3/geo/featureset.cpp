/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <seiscomp3/geo/featureset.h>

#define SEISCOMP_COMPONENT GeoPolygonSet
#define CONFIG_BASE "geo.feature"

#include <seiscomp3/system/environment.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/logging/log.h>

#include <boost/version.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <sstream>
#include <errno.h>


using namespace Seiscomp::Geo;
namespace fs = boost::filesystem;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSet &GeoFeatureSetSingleton::getInstance() {
	static GeoFeatureSetSingleton instance;
	return instance._geoFeatureSet;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSetSingleton::GeoFeatureSetSingleton() {
	_geoFeatureSet.load();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSetObserver::GeoFeatureSetObserver() : _observedSet(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSetObserver::~GeoFeatureSetObserver() {
	if ( _observedSet )
		_observedSet->unregisterObserver(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSet::GeoFeatureSet() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSet::GeoFeatureSet(const GeoFeatureSet &) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSet::~GeoFeatureSet() {
	clear();

	ObserverList::iterator it;
	for ( it = _observers.begin(); it != _observers.end(); ++it )
		(*it)->_observedSet = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::registerObserver(GeoFeatureSetObserver *observer) {
	ObserverList::iterator it = std::find(_observers.begin(),
	                                      _observers.end(),
	                                      observer);
	if ( it != _observers.end() )
		return false;

	observer->_observedSet = this;
	_observers.push_back(observer);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::unregisterObserver(GeoFeatureSetObserver *observer) {
	ObserverList::iterator it = std::find(_observers.begin(),
	                                      _observers.end(),
	                                      observer);
	if ( it == _observers.end() )
		return false;

	observer->_observedSet = NULL;
	_observers.erase(it);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureSet::clear() {
	// Delete all GeoFeatures
	for ( size_t i = 0; i < _features.size(); ++i ) {
		delete _features[i];
	}
	_features.clear();

	// Delete all Categories
	for ( size_t i = 0; i < _categories.size(); ++i ) {
		delete _categories[i];
	}
	_categories.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureSet::load() {
	clear();

	Seiscomp::Environment* env = Seiscomp::Environment::Instance();
	if ( !readBNADir(env->configDir() + "/bna") )
		readBNADir(env->shareDir() + "/bna");

	ObserverList::iterator it;
	for ( it = _observers.begin(); it != _observers.end(); ++it )
		(*it)->geoFeatureSetUpdated();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t GeoFeatureSet::readBNADir(const std::string& dirPath) {
	// Clear the current GeoFeatureSet
	clear();

	// Get the config base directory
	fs::path directory;
	try {
		directory = SC_FS_PATH(dirPath);
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Invalid path '%s'", dirPath.c_str());
		return 0;
	}

	// Read the BNA directory recursively
	Core::Time start = Core::Time::GMT();
	size_t fileCount = readBNADirRecursive(directory, addNewCategory(""));
	SEISCOMP_INFO("%s in %fs", initStatus(dirPath, fileCount).c_str(),
	              (Core::Time::GMT()-start).length());

	// Sort the features according to their rank
 	std::sort(_features.begin(), _features.end(), compareByRank);

	return fileCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t GeoFeatureSet::readBNADirRecursive(const fs::path &directory,
                                          Category *category) {
	// store directory path the data was read from
	category->dataDir = directory.string();

	size_t fileCount = 0;

	try {
		fs::directory_iterator end_itd;
		fs::directory_iterator itd(directory);

		std::string fileName;
		for ( ; itd != end_itd; ++itd ) {
			fileName = SC_FS_IT_LEAF(itd);

			// If the current directory entry is a directory, then
			// descend into this directory
			if (fs::is_directory(*itd))
				fileCount += readBNADirRecursive(*itd,
					addNewCategory(fileName, category));
			else if ( fileName.length() < 4 ||
			          fileName.substr(fileName.length() - 4) != ".bna")
				continue;
			else if ( readBNAFile(SC_FS_IT_STR(itd), category) )
				++fileCount;
			else
				SEISCOMP_ERROR("Error reading file: %s", SC_FS_IT_STR(itd).c_str());
		}
	}
	catch (const std::exception& ex) {}

	return fileCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Category* GeoFeatureSet::addNewCategory(const std::string name,
                                        const Category* parent) {
	Category* category = new Category(_categories.size(),
		parent == NULL || parent->name == "" ? name :
		parent->name + "." + name, parent);
	category->localName = name;
	_categories.push_back(category);
	return category;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string GeoFeatureSet::initStatus(const std::string &directory,
                                            unsigned int fileCount) const {
	unsigned int vertexCount = 0;

	std::vector<GeoFeature*>::const_iterator itf;

	for ( itf = _features.begin(); itf != _features.end(); ++itf ) {
		vertexCount += (*itf)->vertices().size();
	}

	std::ostringstream buffer;
	buffer << "Read " << _features.size()
	       << " segment(s) with a total number of "
	       << vertexCount << " vertice(s) from " << fileCount
	       << " BNA file(s) found under " << directory;

	return buffer.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** Reads the BNA-header, e.g. "segment name","rank 3",123 */
bool GeoFeatureSet::readBNAHeader(std::string& segment, unsigned int& rank,
                                  unsigned int& points, bool& isClosed,
                                  std::string& error, const std::string &line) const {
	size_t pos1, pos2;
	std::string tmpStr;

	// segment
	if ( (pos1 = line.find('"')) == std::string::npos ||
	     (pos2 = line.find('"', pos1+1)) == std::string::npos ) {
		error = "missing quote sign in first header field";
		return false;
	}
	segment = line.substr(pos1+1, pos2-pos1-1);
	Core::trim(segment);

	// rank
	if ( (pos1 = line.find('"', pos2+1)) == std::string::npos ||
	     (pos2 = line.find('"', pos1+1)) == std::string::npos ) {
		error = "missing quote sign in second header field";
		return false;
	}
	tmpStr = line.substr(pos1+1, pos2-pos1-1);

	if ( tmpStr.length() >= 6 && strncmp(tmpStr.c_str(), "rank ", 5) == 0 ) {
		rank = atoi(tmpStr.substr(5, tmpStr.length()-5).c_str());
	}
	else {
		rank = 1;
	}

	// points
	if ( (pos1 = line.rfind(',')) == std::string::npos || pos1 <= pos2 ) {
		error = "could not find position of length field";
		return false;
	}
	tmpStr = line.substr(pos1+1);

	Seiscomp::Core::trim(tmpStr);
	int p;
	if ( !Seiscomp::Core::fromString(p, tmpStr) ) {
		error = "invalid number format in length field";
		return false;
	}
	if ( p >= 0 ) {
		points = p;
		isClosed = true;
	}
	else {
		points = -p;
		isClosed = false;
	}
	return true;
}

/**
 * Reads one BNA file. A BNA file may contain multiple segments consisting of
 * multiple points which define a non closed polyline. A new segment is
 * introduced by the following line:
 *   '"arbitrary segment name","rank <#rank>",<#points>'
 * e.g.
 *   '"test segment","rank 1",991'
 *
 * A point or vertex is defined by the following line:
 *   '<latitude>,<longitude>'
 * e.g.
 *   '31.646944,25.151389'
 *
 * In addition the BNA file format supports complex areas such as main land and
 * islands. The coordinate pairs representing the islands are separated from
 * each other by repeating the initial coordinate of the main area. The
 * following sample file illustrates a complex area:
 *
 *   "Test Area","rank 1",17
 *   2,2 --begin main area
 *   2,1
 *   1,1
 *   1,2
 *   2,2 --end of main area
 *   4,4 --begin island #1
 *   4,3
 *   3,3
 *   3,4
 *   4,4 --end island #1
 *   2,2 --end main area
 *   7,7 --begin island #2
 *   7,5
 *   6,4
 *   5,6
 *   7,7 --end of island #2
 *   2,2 --end of main area
 */
bool GeoFeatureSet::readBNAFile(const std::string &filename,
                                const Category *category) {
	SEISCOMP_DEBUG("Reading segments from file: %s", filename.c_str());

	std::ifstream infile(filename.c_str());

	if ( infile.fail() ) {
		SEISCOMP_WARNING("Could not open segment file for reading: %s",
		                 filename.c_str());
		return false;
	}

	std::vector<GeoFeature*> features;
	GeoFeature *feature;
	unsigned int lineNum = 0, rank, points;
	std::string line, segment, error;
	const char *nptr;
	char *endptr;
	bool isClosed;
	GeoCoordinate v;
	bool startSubFeature;

	bool fileValid = true;

	while ( infile.good() && fileValid ) {
		++lineNum;

		// read BNA header
		std::getline(infile, line);
		if ( Core::trim(line).empty() ) {
			continue;
		}

		if ( !readBNAHeader(segment, rank, points, isClosed, error, line) ) {
			SEISCOMP_ERROR("error reading BNA header in file %s at line %i: %s",
			               filename.c_str(), lineNum, error.c_str());
			fileValid = false;
			break;
		}
		startSubFeature = false;

		feature = new GeoFeature(segment, category, rank);
		features.push_back(feature);
		if ( isClosed )
			feature->setClosedPolygon(true);

		// read vertices, expected format:
		//   "lon1,lat1 lon2,lat2 ... lon_i,lat_i\n"
		//   "lon_i+1,lat_i+1 lon_i+2,lat_i+2 ... \n
		nptr = NULL;
		unsigned int pi = 0;
		while ( true ) {
			if ( nptr == NULL ) {
				// stop if all points have been read
				if ( pi == points ) break;

				// read next line
				if ( infile.good() ) {
					++lineNum;
					std::getline(infile, line);
					nptr = line.c_str();
				}
				else {
					SEISCOMP_ERROR("to few vertices (%i/%i) for feature "
					               "starting at line %i",
					               pi, points, lineNum - pi);
					fileValid = false;
					break;
				}
			}

			// advance nptr to next none white space
			while ( isspace(*nptr) ) ++nptr;

			// read next line if end of line is reached
			if ( *nptr == '\0' ) {
				nptr = NULL;
				continue;
			}

			// file invalid if extra characters are found after last vertex
			if ( pi == points ) {
				SEISCOMP_ERROR("extra characters after last vertex (%i) of "
				              "feature starting at line %i",
				               pi, lineNum - pi);
				fileValid = false;
				break;
			}

			// read longitude
			endptr = NULL;
			errno = 0;
			v.lon = strtof(nptr, &endptr);
			if ( errno != 0 || endptr == NULL || endptr == nptr ||
			     v.lon < -180 || v.lon > 180) {
				SEISCOMP_ERROR("invalid longitude in file %s at line %i",
				               filename.c_str(), lineNum);
				fileValid = false;
				break;
			}

			// search for comma
			nptr = strchr(endptr, ',');
			if ( nptr == NULL ) {
				SEISCOMP_ERROR("invalid coordinate separator in file %s at line %i",
				               filename.c_str(), lineNum);
				fileValid = false;
				break;
			}

			// read latitude
			endptr = NULL; nptr += 1;
			v.lat = strtof(nptr, &endptr);
			if ( errno != 0 || endptr == NULL || endptr == nptr ||
			     v.lat < -90 || v.lat > 90) {
				SEISCOMP_ERROR("invalid latitude in file %s at line %i",
				               filename.c_str(), lineNum);
				fileValid = false;
				break;
			}
			nptr = endptr;

			while ( isspace(*nptr) ) ++nptr;

			// Skip comments
			if ( strncmp(nptr, "--", 2) == 0 )
				nptr = NULL;

			// increase number of succesfully read points
			pi += 1;

			if ( !feature->vertices().empty() ) {
				// check if the current vertex marks the end of a (sub-) or
				// feature and if so don't add it to the vertex vector but mark
				// the next vertex as the starting point of a new sub feature
				if ( v == feature->vertices().front() ) {
					startSubFeature = true;
					continue;
				}
				// Don't add the vertex if it is equal to the start point of
				// the current subfeature
				else if ( !startSubFeature &&
				          !feature->subFeatures().empty() &&
				          v == feature->vertices()[feature->subFeatures().back()] ) {
					continue;
				}
			}

			feature->addVertex(v, startSubFeature);
			startSubFeature = false;
		}

		if ( fileValid ) {
			feature->updateBoundingBox();
			if ( feature->area() < 0 )
				feature->invertOrder();
		}
	}

	if ( fileValid ) {
		_features.insert(_features.end(), features.begin(), features.end());
		return true;
	}
	else {
		for ( size_t i = 0; i < features.size(); ++i ) {
			delete features[i];
		}
		return false;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::addFeature(GeoFeature *feature) {
	_features.push_back(feature);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const bool GeoFeatureSet::compareByRank(const GeoFeature* gf1,
                                        const GeoFeature* gf2 ) {
  	return gf1->rank() < gf2->rank();
}
