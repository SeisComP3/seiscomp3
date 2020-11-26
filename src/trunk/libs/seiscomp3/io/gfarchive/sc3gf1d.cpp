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


#define SEISCOMP_COMPONENT SC3GF1D


#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/greensfunction.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/io/gfarchive/sc3gf1d.h>
#include <seiscomp3/io/records/sacrecord.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <iostream>
#include <fstream>

#include <boost/version.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>


/******************************************************************************
 * To be used along with rapidjson. To make these macros work it is expected
 * to have a variable itr of type
 * rapidjson::Value::ConstMemberIterator. Use JINIT for that.
 * Supported type names are:
 * - Null
 * - False
 * - True
 * - Bool
 * - String
 * - Object
 * - Array
 * - Number
 * - Int
 * - Uint
 * - Int64
 * - Uint64
 * - Double
 * To retrieve the value, use Get[type]() on the Value object.
 * ----------------------------------------------------------------------------
 * Usage example:
 *
 * void parseSomething() {
 *     rapidjson::Document doc;
 *     JINIT; // Create temporary iterator
 *     doc.Parse("...");
 *     // Check for member "test" with type Object. If thats fails, return
 *     if ( JFAIL(doc, "test", Object) )
 *         return;
 *     // Store the last found member value
 *     const rapidjson::Value &test = JVAL;
 *     if ( JOK(test, "attrib1", String) )
 *         cout << JNAME.GetString() << " = " << JVAL.GetString() << endl;
 *     ...
 * }
 ******************************************************************************/
#define JINIT rapidjson::Value::ConstMemberIterator jitr
#define JFAIL(node, member, type) ((jitr = node.FindMember(member)) == node.MemberEnd() || !jitr->value.Is##type())
#define JOK(node, member, type) ((jitr = node.FindMember(member)) != node.MemberEnd() && jitr->value.Is##type())
#define JNAME jitr->name
#define JVAL jitr->value


namespace fs = boost::filesystem;

namespace Seiscomp {
namespace IO {


namespace {


double getValue(const std::map<double, double> &map, double ref) {
	// Get element after the distance
	std::map<double, double>::const_iterator it_to = map.lower_bound(ref);
	std::map<double, double>::const_iterator it_from = it_to;

	// After supported key range
	if ( it_to == map.end() )
		return -1;

	double toRef = it_to->first;
	double fromRef = toRef;

	// Before supported key range
	if ( it_to == map.begin() ) {
		if ( toRef > ref )
			return -1;

		return it_to->second;
	}

	--it_from;
	fromRef = it_from->first;

	double ref1 = it_from->second;
	double ref2 = it_to->second;

	// Interpolate value
	return (ref1 * (toRef-ref) + ref2 * (ref-fromRef)) / (toRef - fromRef);
}


}


REGISTER_GFARCHIVE(SC3GF1DArchive, "sc3gf1d");
// For portability: deprecated!
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::GFArchive, SC3GF1DArchive>
DeprecatedInterface("saul");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


bool interpolate(Core::GreensFunction *gf1, const Core::GreensFunction *gf2,
                 const Core::GreensFunction *gf3, const Core::GreensFunction *gf4,
                 double dist, double lowerDist, double upperDist,
                 double depth, double lowerDepth, double upperDepth) {

	double coeffDist1, coeffDist2;
	double coeffDepth1, coeffDepth2;

	if ( upperDist == lowerDist ) {
		coeffDist1 = 1.0;
		coeffDist2 = 0.0;
	}
	else {
		coeffDist2 = (dist-lowerDist) / (upperDist-lowerDist);
		coeffDist1 = 1.0 - coeffDist2;
	}

	if ( upperDepth == lowerDepth ) {
		coeffDepth1 = 1.0;
		coeffDepth2 = 0.0;
	}
	else {
		coeffDepth2 = (depth-lowerDepth) / (upperDepth-lowerDepth);
		coeffDepth1 = 1.0 - coeffDepth2;
	}

	//std::cerr << "Interpolate: " << lower << ", " << dist << ", " << upper << ": "
	//          << coeff1 << ", " << coeff2 << std::endl;

	for ( int i = 0; i < 8; ++i ) {
		FloatArray *ar1 = (FloatArray*)gf1->data(i);
		FloatArray *ar2 = (FloatArray*)gf2->data(i);

		FloatArray *ar3 = (FloatArray*)gf3->data(i);
		FloatArray *ar4 = (FloatArray*)gf4->data(i);

		if ( ar1->size() != ar2->size() ) {
			SEISCOMP_ERROR("GF: Interpolation sizes do not match");
			return false;
		}

		if ( ar2->size() != ar3->size() ) {
			SEISCOMP_ERROR("GF: Interpolation sizes do not match");
			return false;
		}

		if ( ar3->size() != ar4->size() ) {
			SEISCOMP_ERROR("GF: Interpolation sizes do not match");
			return false;
		}

		for ( int s = 0; s < ar1->size(); ++s )
			(*ar1)[s] = (coeffDist1*(*ar1)[s] + coeffDist2*(*ar2)[s])*coeffDepth1 +
			            (coeffDist1*(*ar3)[s] + coeffDist2*(*ar4)[s])*coeffDepth2;
	}

	return true;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SC3GF1DArchive::SC3GF1DArchive() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SC3GF1DArchive::SC3GF1DArchive(const std::string &baseDirectory) {
	setSource(baseDirectory);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SC3GF1DArchive::~SC3GF1DArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SC3GF1DArchive::setSource(std::string source) {
	fs::path directory;
	try {
		directory = SC_FS_PATH(source);
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Unable to open directory: %s", source.c_str());
		return false;
	}

	_baseDirectory = source;

	fs::directory_iterator end_itr;
	try {
		for ( fs::directory_iterator itr(directory); itr != end_itr; ++itr ) {
			if ( !fs::is_directory(*itr) ) continue;

			std::string name = SC_FS_IT_LEAF(itr);

			/*
			size_t pos = name.rfind("_efl");
			if ( pos == std::string::npos ) continue;
			*/

			std::string model = name/*.substr(0, pos)*/;

			std::ifstream ifDesc;

			int depthFrom = -1, depthTo = -1;
			double depthSpacing = -1;
			int distanceFrom = -1, distanceTo = -1, distanceSpacing = -1;
			std::string line;

			ifDesc.open((_baseDirectory + "/" + name + ".desc").c_str());
			if ( !ifDesc.is_open() ) {
				SEISCOMP_WARNING("Unable to find model description, skipping directory: %s",
				                 name.c_str());
				continue;
			}

			bool validModel = true;

			DoubleList &depths = _models[model].depths;
			DoubleList &dists = _models[model].distances;

			while ( getline(ifDesc, line) ) {
				Core::trim(line);
				if ( line.empty() ) continue;
				if ( line[0] == '#' ) continue;
				std::stringstream ss(line);
				ss >> line;
				if ( line == "depth" ) {
					ss >> depthFrom >> depthTo >> depthSpacing;

					if ( (depthSpacing < 0) || (depthFrom > depthTo) ) {
						SEISCOMP_WARNING("Invalid description format, skipping directory: %s",
						                 name.c_str());
						validModel = false;
						break;
					}

					if ( depthSpacing == 0 )
						depths.insert(depthFrom);
					else {
						for ( double i = depthFrom; i <= depthTo; i += depthSpacing ) {
							depths.insert(i);
						}
					}

				}
				else if ( line == "distance" ) {
					ss >> distanceFrom >> distanceTo >> distanceSpacing;

					if ( (distanceSpacing < 0) || (distanceFrom > distanceTo) ) {
						SEISCOMP_WARNING("Invalid description format, skipping directory: %s",
						                 name.c_str());
						validModel = false;
						break;
					}

					if ( distanceSpacing == 0 )
						dists.insert(distanceFrom);
					else {
						for ( int i = distanceFrom; i <= distanceTo; i += distanceSpacing ) {
							dists.insert(i);
						}
					}
				}
			}

			if ( !validModel )
				_models.erase(_models.find(model));
			else if ( dists.empty() || depths.empty() ) {
				SEISCOMP_WARNING("Empty distances or depths for matching directory: %s",
				                 name.c_str());
				_models.erase(_models.find(model));
			}
		}
	}
	catch ( ... ) {}

	return !_models.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SC3GF1DArchive::close() {
	_requests.clear();
	_models.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<std::string> SC3GF1DArchive::availableModels() const {
	std::list<std::string> models;
	for ( ModelMap::const_iterator it = _models.begin(); it != _models.end(); ++it )
		models.push_back(it->first);
	return models;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<double> SC3GF1DArchive::availableDepths(const std::string &model) const {
	ModelMap::const_iterator it = _models.find(model);
	if ( it == _models.end() )
		return std::list<double>();

	std::list<double> depths;
	for ( DoubleList::const_iterator dit = it->second.depths.begin();
	      dit != it->second.depths.end(); ++dit )
		depths.push_back(*dit);

	return depths;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SC3GF1DArchive::hasModel(const std::string &m) const {
	return _models.find(m) != _models.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SC3GF1DArchive::setTimeSpan(const Core::TimeSpan &span) {
	_defaultTimespan = span;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SC3GF1DArchive::addRequest(const std::string &id,
                                const std::string &model,
                                const GFSource &source,
		                        const GFReceiver &receiver) {
	if ( !hasModel(model) ) {
		SEISCOMP_DEBUG("Wrong model: %s", model.c_str());
		return false;
	}

	double dist, az, baz;
	Math::Geo::delazi_wgs84(source.lat, source.lon,
	                        receiver.lat, receiver.lon,
	                        &dist, &az, &baz);

	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().model = model;
	_requests.back().distance = Math::Geo::deg2km(dist);
	_requests.back().depth = source.depth;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SC3GF1DArchive::addRequest(const std::string &id,
                                const std::string &model,
                                const GFSource &source, const GFReceiver &receiver,
                                const Core::TimeSpan &span) {
	if ( !hasModel(model) ) {
		SEISCOMP_DEBUG("Wrong model: %s", model.c_str());
		return false;
	}

	double dist, az, baz;
	Math::Geo::delazi_wgs84(source.lat, source.lon,
	                        receiver.lat, receiver.lon,
	                        &dist, &az, &baz);

	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().model = model;
	_requests.back().distance = Math::Geo::deg2km(dist);
	_requests.back().depth = source.depth;
	_requests.back().timeSpan = span;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::GreensFunction* SC3GF1DArchive::get() {
	while ( !_requests.empty() ) {
		Request req = _requests.front();
		_requests.pop_front();

		std::string pathprefix = _baseDirectory + "/" + req.model + "/";

		int distKm = (int)req.distance;
		double fDepth = req.depth;

		ModelMap::iterator mit = _models.find(req.model);
		if ( mit == _models.end() ) continue;

		DoubleList::iterator lbdist = mit->second.distances.lower_bound(distKm);
		DoubleList::iterator ubdist = lbdist--;
		DoubleList::iterator lbdep = mit->second.depths.lower_bound(req.depth);
		DoubleList::iterator ubdep = lbdep--;

		double dist1, dist2, dist;
		double dep1, dep2, dep;

		// Distance is lower than the first stored value
		if ( ubdist == mit->second.distances.begin() ) {
			dist1 = *ubdist;
			++ubdist;
			if ( ubdist != mit->second.distances.end() )
				dist2 = *ubdist;
			else
				dist2 = dist1;

			double maxDistError = dist2 - dist1;
			if ( dist1 - distKm > maxDistError ) {
				SEISCOMP_DEBUG("Distance too low: %d km", distKm);
				continue;
			}

			dist2 = dist1;
		}
		// Distance is greater than the last stored value
		else if ( ubdist == mit->second.distances.end() ) {
			dist2 = *lbdist;
			--lbdist;
			dist1 = *lbdist;

			double maxDistError = dist2 - dist1;
			if ( distKm - dist2 > maxDistError ) {
				SEISCOMP_DEBUG("Distance too high: %d km", distKm);
				continue;
			}

			dist2 = dist1;
		}
		else {
			dist1 = *lbdist;
			dist2 = *ubdist;
		}

		// Depth is lower than the first stored value
		if ( ubdep == mit->second.depths.begin() ) {
			dep1 = dep2 = *ubdep;

			dep1 = *ubdep;
			++ubdep;
			if ( ubdep != mit->second.depths.end() )
				dep2 = *ubdep;
			else
				dep2 = dep1;

			double maxDepError = dep2 - dep1;
			if ( dep1 - fDepth > maxDepError ) {
				SEISCOMP_DEBUG("Depth too low: %f km < %d km", fDepth, (int)dep1);
				continue;
			}

			dep2 = dep1;

		}
		// Depth is greater than the last stored value
		else if ( ubdep == mit->second.depths.end() ) {
			dep2 = *lbdep;
			--lbdep;
			dep1 = *lbdep;

			double maxDepError = dep2 - dep1;
			if ( fDepth - dep2 > maxDepError ) {
				SEISCOMP_DEBUG("Depth too high: %f km", fDepth);
				continue;
			}

			dep2 = dep1;
		}
		else {
			dep1 = *lbdep;
			dep2 = *ubdep;
		}

		if ( fabs(distKm - dist1) < fabs(distKm - dist2) ) {
			dist = dist1;
		}
		else {
			dist = dist2;
		}

		if ( fabs(fDepth - dep1) < fabs(fDepth - dep2) ) {
			dep = dep1;
		}
		else {
			dep = dep2;
		}

		/*
		if ( fabs(depError) > maxDepError || fabs(distError) > maxDistError ) {
		}
		*/

		// For greens functions for bilinear interpolation
		Core::GreensFunction *gf_11;
		Core::GreensFunction *gf_12;
		Core::GreensFunction *gf_21;
		Core::GreensFunction *gf_22;

		if ( (dist == dist1) || (dist == dist2) || (dist1 == dist2) ) {
			char dep_str[10], dist_str[10];
			snprintf(dep_str, 10, "%04d", (int)dep*10);
			snprintf(dist_str, 10, "%05d", (int)dist);
			std::string file = pathprefix + dep_str + "/" + dist_str + "/" + dep_str + "." + dist_str + ".";

			Core::TimeSpan ts = _defaultTimespan;
			if ( req.timeSpan ) ts = req.timeSpan;

			//double ofs = dist / _models[req.model].velocity;
			double ofs = 0;

			Core::GreensFunction *gf = read(file, ts, ofs);
			if ( gf ) {
				gf->setId(req.id);
				gf->setModel(req.model);
				gf->setDepth(dep);
				gf->setDistance(dist);
				gf_11 = gf_12 = gf;
			}
			else {
				SEISCOMP_ERROR("Unable to read %s", file.c_str());
				continue;
			}
		}
		else {
			char dep_str[10], dist1_str[10], dist2_str[10];
			snprintf(dep_str, 10, "%04d", (int)dep*10);
			snprintf(dist1_str, 10, "%05d", (int)dist1);
			snprintf(dist2_str, 10, "%05d", (int)dist2);
			std::string file1 = pathprefix + dep_str + "/" + dist1_str + "/" + dep_str + "." + dist1_str + ".";
			std::string file2 = pathprefix + dep_str + "/" + dist2_str + "/" + dep_str + "." + dist2_str + ".";

			Core::TimeSpan ts = _defaultTimespan;
			if ( req.timeSpan ) ts = req.timeSpan;

			//double ofs = dist / _models[req.model].velocity;
			double ofs = 0;

			Core::GreensFunction *gf1 = read(file1, ts, ofs);
			Core::GreensFunction *gf2 = read(file2, ts, ofs);
			if ( gf1 && gf2 ) {
				gf1->setId(req.id);
				gf1->setModel(req.model);
				gf1->setDepth(dep);
				gf1->setDistance(distKm);

				gf_11 = gf1;
				gf_12 = gf2;
			}
			else {
				SEISCOMP_ERROR("Unable to read %s or %s",
				               file1.c_str(), file2.c_str());
				if ( gf1 ) delete gf1;
				if ( gf2 ) delete gf2;

				continue;
			}
		}


		double alt_dep = dep;
		if ( dep != dep1 )
			alt_dep = dep1;
		else if ( dep != dep2 )
			alt_dep = dep2;

		if ( dep == alt_dep ) {
			gf_21 = gf_11;
			gf_22 = gf_12;
		}
		else {
			if ( (dist == dist1) || (dist == dist2) || (dist1 == dist2) ) {
				char dep_str[10], dist_str[10];
				snprintf(dep_str, 10, "%04d", (int)alt_dep*10);
				snprintf(dist_str, 10, "%05d", (int)dist);
				std::string file = pathprefix + dep_str + "/" + dist_str + "/" + dep_str + "." + dist_str + ".";

				Core::TimeSpan ts = _defaultTimespan;
				if ( req.timeSpan ) ts = req.timeSpan;

				//double ofs = dist / _models[req.model].velocity;
				double ofs = 0;

				Core::GreensFunction *gf = read(file, ts, ofs);
				if ( gf ) {
					gf_21 = gf_22 = gf;
				}
				else {
					if ( gf_11 ) delete gf_11;
					if ( gf_12 && (gf_11 != gf_12) ) delete gf_12;

					SEISCOMP_ERROR("Unable to read %s", file.c_str());
					continue;
				}
			}
			else {
				char dep_str[10], dist1_str[10], dist2_str[10];
				snprintf(dep_str, 10, "%04d", (int)alt_dep*10);
				snprintf(dist1_str, 10, "%05d", (int)dist1);
				snprintf(dist2_str, 10, "%05d", (int)dist2);
				std::string file1 = pathprefix + dep_str + "/" + dist1_str + "/" + dep_str + "." + dist1_str + ".";
				std::string file2 = pathprefix + dep_str + "/" + dist2_str + "/" + dep_str + "." + dist2_str + ".";

				Core::TimeSpan ts = _defaultTimespan;
				if ( req.timeSpan ) ts = req.timeSpan;

				//double ofs = dist / _models[req.model].velocity;
				double ofs = 0;

				Core::GreensFunction *gf1 = read(file1, ts, ofs);
				Core::GreensFunction *gf2 = read(file2, ts, ofs);
				if ( gf1 && gf2 ) {
					gf_21 = gf1;
					gf_22 = gf2;
				}
				else {
					SEISCOMP_ERROR("Unable to read %s or %s",
					               file1.c_str(), file2.c_str());
					if ( gf1 ) delete gf1;
					if ( gf2 ) delete gf2;

					if ( gf_11 ) delete gf_11;
					if ( gf_12 && (gf_11 != gf_12) ) delete gf_12;

					continue;
				}
			}

			gf_11->setDepth(fDepth);
		}

		if ( !interpolate(gf_11, gf_12, gf_21, gf_22,
		                  distKm, dist1, dist2, fDepth, dep, alt_dep) ) {
			SEISCOMP_ERROR("Interpolation for %d / %f failed", distKm, fDepth);

			if ( gf_11 ) delete gf_11;
			if ( gf_12 && (gf_11 != gf_12) ) delete gf_12;

			if ( gf_21 && ((gf_21 != gf_11) && (gf_21 != gf_12)) ) delete gf_21;
			if ( gf_22 && ((gf_22 != gf_11) && (gf_22 != gf_12) && (gf_22 != gf_21)) ) delete gf_22;
		}
		else {
			if ( gf_12 && (gf_12 != gf_11) ) delete gf_12;
			if ( gf_21 && ((gf_21 != gf_11) && (gf_21 != gf_12)) ) delete gf_21;
			if ( gf_22 && ((gf_22 != gf_11) && (gf_22 != gf_12) && (gf_22 != gf_21)) ) delete gf_22;

			return gf_11;
		}

		SEISCOMP_DEBUG("No greensfunction found");
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::GreensFunction* SC3GF1DArchive::read(const std::string &file,
                                        const Core::TimeSpan &ts,
                                        double timeOfs) {
#if SC_API_VERSION >= SC_API_VERSION_CHECK(13,0,0)
#define GF_COMPS 10
#else
#define GF_COMPS 8
#endif

	Core::GreensFunctionComponent comps[GF_COMPS] = {
		Core::TSS,
		Core::TDS,
		Core::RSS,
		Core::RDS,
		Core::RDD,
		Core::ZSS,
		Core::ZDS,
		Core::ZDD
#if SC_API_VERSION >= SC_API_VERSION_CHECK(13,0,0)
		,
		Core::ZEP,
		Core::REP
#endif
	};

	Core::GreensFunction *gf = NULL;

	for ( int i = 0; i < GF_COMPS; ++i ) {
		std::string filename = file + comps[i].toString();
		std::ifstream ifs(filename .c_str());
		if ( !ifs.good() ) {
			SEISCOMP_DEBUG("%s: not found", filename.c_str());
			return NULL;
		}

		IO::SACRecord sac;
		try {
			sac.read(ifs);
		}
		catch ( std::exception &exc ) {
			SEISCOMP_ERROR("%s: %s", filename.c_str(), exc.what());
			if ( gf ) delete gf;
			return NULL;
		}

		if ( sac.startTime() >= ts ) {
			SEISCOMP_ERROR("%s: requested timespan not within range (%s < %s",
			               filename.c_str(), Core::Time(ts).iso().c_str(), sac.startTime().iso().c_str());
			if ( gf ) delete gf;
			return NULL;
		}

		timeOfs = (double)sac.startTime();
		double cutSeconds = (double)ts - timeOfs;

		if ( gf && gf->timeOffset() != timeOfs ) {
			SEISCOMP_ERROR("%s: mismatching start times, last component = %.4f, this = %.4f",
			               filename.c_str(), gf->timeOffset(), timeOfs);
			delete gf;
			return NULL;
		}

		FloatArray *data = FloatArray::Cast(sac.data());
		if ( data == NULL ) {
			SEISCOMP_ERROR("%s: invalid data, expected float array", filename.c_str());
			if ( gf ) delete gf;
			return NULL;
		}

		/*
		int sampleOfs = (int)(timeOfs * sac.samplingFrequency());
		if ( sampleOfs < 0 ) {
			SEISCOMP_ERROR("%s: negative sampling frequencies are not allowed",
			               filename.c_str());
			if ( gf ) delete gf;
			return NULL;
		}
		*/
		int sampleOfs = 0;

		if ( sampleOfs > data->size() ) {
			SEISCOMP_ERROR("%s: not enough data, time-ofs = %.2f, sr = %.4f, samples = %d",
			               filename.c_str(), timeOfs, sac.samplingFrequency(), data->size());
			if ( gf ) delete gf;
			return NULL;
		}

		if ( gf == NULL ) {
			gf = new Core::GreensFunction();
			gf->setSamplingFrequency(sac.samplingFrequency());
			gf->setTimeOffset(timeOfs);
		}
		else if ( gf->samplingFrequency() != sac.samplingFrequency() ) {
			SEISCOMP_ERROR("%s: mismatching sampling frequencies, last component = %.4f, this = %.4f",
			               filename.c_str(), gf->samplingFrequency(), sac.samplingFrequency());
			delete gf;
			return NULL;
		}

		int sampleCount = std::min(data->size()-sampleOfs, (int)(cutSeconds * gf->samplingFrequency()));
		if ( sampleCount <= 0 ) {
			SEISCOMP_WARNING("%s: skipping empty result", filename.c_str());
			if ( gf ) delete gf;
			return NULL;
		}

		FloatArrayPtr arr = data->slice(sampleOfs, sampleOfs + sampleCount);

		if ( (comps[i] == Core::ZDS) || (comps[i] == Core::TSS) || (comps[i] == Core::RDS) )
		{
			for ( int i = 0; i < arr->size(); ++i )
				(*arr)[i] *= -1.0f;
		}

		gf->setData(comps[i], arr.get());
	}

	return gf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(double) SC3GF1DArchive::getTravelTime(const std::string &phase,
                                          const std::string &model,
                                          const GFSource &source,
                                          const GFReceiver &receiver) {
	ModelMap::iterator it = _models.find(model);
	if ( it == _models.end() )
		return Core::None;

	ModelConfig &config = it->second;
	if ( !config.travelTimesInitialized ) {
		// Read travel time table
		config.travelTimesInitialized = true;

		std::string tttfile = _baseDirectory + "/" + model + ".ttt";
		std::ifstream ifttt;
		ifttt.open(tttfile.c_str());

		if ( !ifttt )
			return Core::None;

		rapidjson::IStreamWrapper isw(ifttt);
		rapidjson::Document doc;
		doc.ParseStream(isw);
		if ( doc.HasParseError() ) {
			SEISCOMP_ERROR("%s/%s.ttt: invalid JSON document",
			               _baseDirectory.c_str(), model.c_str());
			return Core::None;
		}

		std::vector<std::string> phaseMap;
		std::vector<double> distanceMap;
		std::vector<double> depthMap;

		JINIT;

		if ( JFAIL(doc, "phases", Array) ) {
			SEISCOMP_ERROR("%s/%s.ttt: missing 'phases' array attribute",
			               _baseDirectory.c_str(), model.c_str());
			return Core::None;
		}

		const rapidjson::Value &vPhases = JVAL;

		if ( JFAIL(doc, "distances", Array) ) {
			SEISCOMP_ERROR("%s/%s.ttt: missing 'distances' array attribute",
			               _baseDirectory.c_str(), model.c_str());
			return Core::None;
		}

		const rapidjson::Value &vDistances = JVAL;

		if ( JFAIL(doc, "depths", Array) ) {
			SEISCOMP_ERROR("%s/%s.ttt: missing 'depths' array attribute",
			               _baseDirectory.c_str(), model.c_str());
			return Core::None;
		}

		const rapidjson::Value &vDepths = JVAL;

		if ( JFAIL(doc, "travelTimes", Array) ) {
			SEISCOMP_ERROR("%s/%s.ttt: missing 'travelTimes' array attribute",
			               _baseDirectory.c_str(), model.c_str());
			return Core::None;
		}

		const rapidjson::Value &vTT = JVAL;

		for ( rapidjson::SizeType i = 0; i < vPhases.Size(); ++i ) {
			const rapidjson::Value &ph = vPhases[i];
			if ( !ph.IsString() ) {
				SEISCOMP_ERROR("%s/%s.ttt: phase at index %d is not a string",
				               _baseDirectory.c_str(), model.c_str(), i);
				return Core::None;
			}

			phaseMap.push_back(ph.GetString());
		}

		for ( rapidjson::SizeType i = 0; i < vDistances.Size(); ++i ) {
			const rapidjson::Value &d = vDistances[i];
			if ( !d.IsNumber() ) {
				SEISCOMP_ERROR("%s/%s.ttt: distance at index %d is not a number",
				               _baseDirectory.c_str(), model.c_str(), i);
				return Core::None;
			}

			distanceMap.push_back(d.GetDouble());
		}

		for ( rapidjson::SizeType i = 0; i < vDepths.Size(); ++i ) {
			const rapidjson::Value &d = vDepths[i];
			if ( !d.IsNumber() ) {
				SEISCOMP_ERROR("%s/%s.ttt: depth at index %d is not a number",
				               _baseDirectory.c_str(), model.c_str(), i);
				return Core::None;
			}

			depthMap.push_back(d.GetDouble());
		}

		if ( vTT.Size() != depthMap.size() ) {
			SEISCOMP_ERROR("%s/%s.ttt: travelTimes are of wrong dimension: %d != %d",
			               _baseDirectory.c_str(), model.c_str(),
			               (int)vTT.Size(), (int)distanceMap.size());
			return Core::None;
		}

		for ( rapidjson::SizeType i = 0; i < vTT.Size(); ++i ) {
			const rapidjson::Value &distances = vTT[i];
			if ( !distances.IsArray() ) {
				SEISCOMP_ERROR("%s/%s.ttt: travelTimes at index %d are not a distance array",
				               _baseDirectory.c_str(), model.c_str(), i);
				return Core::None;
			}

			if ( distances.Size() != distanceMap.size() ) {
				SEISCOMP_ERROR("%s/%s.ttt: travelTimes distance array at index %d has wrong dimension: %d != %d",
				               _baseDirectory.c_str(), model.c_str(), i,
				               (int)distances.Size(), (int)distanceMap.size());
				return Core::None;
			}

			for ( rapidjson::SizeType j = 0; j < distances.Size(); ++j ) {
				const rapidjson::Value &phases = distances[j];
				if ( !phases.IsArray() ) {
					SEISCOMP_ERROR("%s/%s.ttt: travelTimes depth phases at index %d/%d are not an array",
					               _baseDirectory.c_str(), model.c_str(), i, j);
					return Core::None;
				}

				if ( phases.Size() != phaseMap.size() ) {
					SEISCOMP_ERROR("%s/%s.ttt: travelTimes depth phase array at index %d/%d has wrong dimension: %d != %d",
					               _baseDirectory.c_str(), model.c_str(), i, j,
					               (int)phases.Size(), (int)phaseMap.size());
					return Core::None;
				}

				for ( rapidjson::SizeType k = 0; k < phases.Size(); ++k ) {
					const rapidjson::Value &tt = phases[k];
					if ( !tt.IsNumber() ) {
						SEISCOMP_ERROR("%s/%s.ttt: travelTimes depth phase at index %d/%d/%d is not a number",
						               _baseDirectory.c_str(), model.c_str(), i, j, k);
						return Core::None;
					}

					// Populate table
					config.travelTimes[phaseMap[k]][distanceMap[j]][depthMap[i]] = tt.GetDouble();
				}
			}
		}
	}

	TTPhases::iterator pit = config.travelTimes.find(phase);
	if ( pit == config.travelTimes.end() )
		return Core::None;

	TTDistance &distanceDepths = pit->second;

	double dist, az, baz;
	Math::Geo::delazi_wgs84(source.lat, source.lon,
	                        receiver.lat, receiver.lon,
	                        &dist, &az, &baz);

	dist = Math::Geo::deg2km(dist);

	// Get element after the distance
	TTDistance::iterator it_dist_to = distanceDepths.lower_bound(dist);
	TTDistance::iterator it_dist_from = it_dist_to;

	// Distance out of range
	if ( it_dist_to == distanceDepths.end() )
		return Core::None;

	double toDist = it_dist_to->first;
	double fromDist = toDist;

	// Before supported distance
	if ( it_dist_to == distanceDepths.begin() ) {
		if ( toDist > dist )
			return Core::None;

		return getValue(it_dist_to->second, source.depth);
	}

	--it_dist_from;
	fromDist = it_dist_from->first;

	double tt1 = getValue(it_dist_from->second, source.depth);
	double tt2 = getValue(it_dist_to->second, source.depth);

	if ( tt1 < 0 || tt2 < 0 )
		return Core::None;

	// Interpolate distances
	return (tt1 * (toDist-dist) + tt2 * (dist-fromDist)) / (toDist - fromDist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
