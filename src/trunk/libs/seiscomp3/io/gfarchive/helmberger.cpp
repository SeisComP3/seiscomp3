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



#define SEISCOMP_COMPONENT Helmberger


#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/greensfunction.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/io/gfarchive/helmberger.h>

#include <iostream>
#include <fstream>

#include <boost/version.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>


namespace fs = boost::filesystem;

namespace Seiscomp {
namespace IO {


REGISTER_GFARCHIVE(HelmbergerArchive, "helmberger");


namespace {


void interpolate(Core::GreensFunction *gf1, const Core::GreensFunction *gf2,
                 double dist, double lower, double upper) {
	double coeff2 = (dist-lower) / (upper-lower);
	double coeff1 = 1.0 - coeff2;

	//std::cerr << "Interpolate: " << lower << ", " << dist << ", " << upper << ": "
	//          << coeff1 << ", " << coeff2 << std::endl;

	for ( int i = 0; i < 8; ++i ) {
		FloatArray *ar1 = (FloatArray*)gf1->data(i);
		FloatArray *ar2 = (FloatArray*)gf2->data(i);

		if ( ar1->size() != ar2->size() )
			SEISCOMP_ERROR("GF: Interpolation sizes do not match");

		for ( int s = 0; s < ar1->size(); ++s )
			(*ar1)[s] = coeff1*(*ar1)[s] + coeff2*(*ar2)[s];
	}
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HelmbergerArchive::HelmbergerArchive() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HelmbergerArchive::HelmbergerArchive(const std::string &baseDirectory) {
	setSource(baseDirectory);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HelmbergerArchive::~HelmbergerArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HelmbergerArchive::setSource(std::string source) {
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

			std::ifstream ifDists;
			std::ifstream ifDepths;
			std::ifstream ifVel;

			double velocity = 9;
			ifVel.open((_baseDirectory + "/" + name + ".vel").c_str());
			if ( ifVel.is_open() )
				ifVel >> velocity;

			ifDists.open((_baseDirectory + "/" + name + ".dists").c_str());
			ifDepths.open((_baseDirectory + "/" + name + ".depths").c_str());

			if ( !ifDists.good() || !ifDepths.good() ) {
				SEISCOMP_WARNING("Unable to find distance or depth config for matching directory: %s",
				                 name.c_str());
				continue;
			}

			SEISCOMP_DEBUG("model: %s, velocity: %.2f", model.c_str(), velocity);

			_models[model].velocity = velocity;
			DoubleList &dists = _models[model].distances;
			DoubleList &depths = _models[model].depths;

			double value;
			ifDists >> value;
			while ( ifDists.good() ) {
				dists.insert(value);
				ifDists >> value;
			}

			ifDepths >> value;
			while ( ifDepths.good() ) {
				depths.insert(value);
				ifDepths >> value;
			}

			if ( dists.empty() || depths.empty() ) {
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
void HelmbergerArchive::close() {
	_requests.clear();
	_models.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<std::string> HelmbergerArchive::availableModels() const {
	std::list<std::string> models;
	for ( ModelMap::const_iterator it = _models.begin(); it != _models.end(); ++it )
		models.push_back(it->first);
	return models;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<double> HelmbergerArchive::availableDepths(const std::string &model) const {
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
bool HelmbergerArchive::hasModel(const std::string &m) const {
	return _models.find(m) != _models.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HelmbergerArchive::setTimeSpan(const Core::TimeSpan &span) {
	_defaultTimespan = span;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HelmbergerArchive::addRequest(const std::string &id,
                                   const std::string &model,
                                   double distance, double depth) {
	if ( !hasModel(model) ) {
		SEISCOMP_DEBUG("Wrong model: %s", model.c_str());
		return false;
	}

	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().model = model;
	_requests.back().distance = distance;
	_requests.back().depth = depth;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HelmbergerArchive::addRequest(const std::string &id,
                                   const std::string &model,
                                   double distance, double depth,
                                   const Core::TimeSpan &span) {
	if ( !hasModel(model) ) {
		SEISCOMP_DEBUG("Wrong model: %s", model.c_str());
		return false;
	}

	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().model = model;
	_requests.back().distance = distance;
	_requests.back().depth = depth;
	_requests.back().timeSpan = span;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::GreensFunction* HelmbergerArchive::get() {
	while ( !_requests.empty() ) {
		Request req = _requests.front();
		_requests.pop_front();

		std::string modelprefix = req.model/* + "_efl"*/;
		std::string pathprefix = _baseDirectory + "/" + modelprefix + "/" +
		                         modelprefix/* + "_tmp"*/;

		int distKm = (int)req.distance;
		int iDepth = (int)req.depth;

		ModelMap::iterator mit = _models.find(req.model);
		if ( mit == _models.end() ) {
			SEISCOMP_DEBUG("helmberger: req dropped, model %s not available",
			                req.model.c_str());
			continue;
		}

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
			dist2 = *ubdist;

			double maxDistError = dist2 - dist1;
			if ( dist1 - distKm > maxDistError ) {
				SEISCOMP_DEBUG("helmberger: distance too low: %d km", distKm);
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
				SEISCOMP_DEBUG("helmberger: distance too high: %d km", distKm);
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
			dep2 = *ubdep;

			double maxDepError = dep2 - dep1;
			if ( dep1 - iDepth > maxDepError ) {
				SEISCOMP_DEBUG("helmberger: depth too low: %d km", iDepth);
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
			if ( iDepth - dep2 > maxDepError ) {
				SEISCOMP_DEBUG("helmberger: depth too high: %d km", iDepth);
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

		if ( fabs(iDepth - dep1) < fabs(iDepth - dep2) ) {
			dep = dep1;
		}
		else {
			dep = dep2;
		}

		/*
		if ( fabs(depError) > maxDepError || fabs(distError) > maxDistError ) {
		}
		*/

		Core::TimeSpan ts = _defaultTimespan;
		if ( req.timeSpan ) ts = req.timeSpan;

		double ofs = _models[req.model].velocity != 0?dist / _models[req.model].velocity:0;

		if ( dist1 == dist || dist2 == dist ) {
			std::string file = pathprefix + Core::toString(dist) + "d" + Core::toString(dep) + ".disp";
			//std::cout << file << std::endl;

			Core::GreensFunction *gf = read(file, ts, ofs);
			if ( gf ) {
				gf->setId(req.id);
				gf->setModel(req.model);
				gf->setDepth(dep);
				gf->setDistance(dist);
				//SEISCOMP_DEBUG("GF: dist = %.2f, ofs = %.2f", gf->distance(), gf->timeOffset());
				return gf;
			}
		}
		else {
			std::string file = pathprefix + Core::toString(dist1) + "d" + Core::toString(dep) + ".disp";
			Core::GreensFunction *gf1 = read(file, ts, ofs);

			file = pathprefix + Core::toString(dist2) + "d" + Core::toString(dep) + ".disp";
			Core::GreensFunction *gf2 = read(file, ts, ofs);

			if ( gf1 && gf2 ) {
				gf1->setId(req.id);
				gf1->setModel(req.model);
				gf1->setDepth(dep);
				gf1->setDistance(distKm);

				interpolate(gf1, gf2, distKm, dist1, dist2);
				delete gf2;

				//SEISCOMP_DEBUG("GF: dist = %.2f, ofs = %.2f", gf1->distance(), gf1->timeOffset());
				return gf1;
			}
			else {
				SEISCOMP_ERROR("Unable to read %s or %s",
				               (pathprefix + Core::toString(dist1) + "d" + Core::toString(dep) + ".disp").c_str(),
				               file.c_str());
				if ( gf1 ) delete gf1;
				if ( gf2 ) delete gf2;
			}
		}

		//SEISCOMP_DEBUG("No greensfunction found");
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::GreensFunction* HelmbergerArchive::read(const std::string &file,
                                              const Core::TimeSpan &ts,
                                              double timeOfs) {
	if ( timeOfs >= (double)ts )
		return NULL;

	std::ifstream ifs(file.c_str());
	if ( !ifs.good() ) {
		//SEISCOMP_DEBUG("%s: not found", file.c_str());
		return NULL;
	}

	int components = 0;
	ifs >> components;
	if ( components < 8 ) {
		SEISCOMP_WARNING("%s: invalid number of components: %d, need 8",
		                 file.c_str(), components);
		return NULL;
	}

	std::string format, tmp;
	ifs >> format;
	//std::cout << "Format: " << format << std::endl;

	boost::smatch what;
	if ( !boost::regex_match(format, what, boost::regex("^\\(([0-9]*)e([0-9]*)\\.([0-9]*)\\)")) ) {
		SEISCOMP_WARNING("%s: wrong format: %s", file.c_str(), format.c_str());
		return NULL;
	}

	if ( what.size() != 4 ) {
		SEISCOMP_WARNING("%s: wrong format: %s", file.c_str(), format.c_str());
		return NULL;
	}

	int numTokens, tokenSize;

	if ( !Core::fromString(numTokens, what.str(1)) ) {
		SEISCOMP_WARNING("%s: wrong format: %s", file.c_str(), format.c_str());
		return NULL;
	}

	if ( !Core::fromString(tokenSize, what.str(2)) ) {
		SEISCOMP_WARNING("%s: wrong format: %s", file.c_str(), format.c_str());
		return NULL;
	}

	//std::cout << "number of tokens: " << numTokens << std::endl;
	//std::cout << "token size: " << tokenSize << std::endl;

	std::getline(ifs, tmp);

	Core::GreensFunction *gf = NULL;
	int maxSamples = 0;

	Core::GreensFunctionComponent orderedComps[8] = {
		Core::TSS,
		Core::TDS,
		Core::RSS,
		Core::RDS,
		Core::RDD,
		Core::ZSS,
		Core::ZDS,
		Core::ZDD
	};

	for ( int c = 0; c < 8; ++c ) {
		int numSamples;
		double samplingFrequency;

		// --- from here
		std::getline(ifs, tmp);
		ifs >> numSamples >> samplingFrequency;
		std::getline(ifs, tmp);

		samplingFrequency = 1.0/samplingFrequency;

		//std::cout << "number of samples: " << numSamples << std::endl;
		//std::cout << "sampling frequency: " << samplingFrequency << std::endl;

		if ( gf == NULL ) {
			gf = new Core::GreensFunction();
			gf->setSamplingFrequency(samplingFrequency);
			gf->setTimeOffset(timeOfs);

			double neededLength = (double)ts - timeOfs;
			maxSamples = neededLength * samplingFrequency;

		}
		else if ( samplingFrequency != gf->samplingFrequency() ) {
			SEISCOMP_ERROR("%s: mismatching sampling frequencies between components",
			               file.c_str());
			delete gf;
			return NULL;
		}

		int completeLines = numSamples / numTokens;
		int count = 0;

		if ( maxSamples > numSamples ) maxSamples = numSamples;

		FloatArrayPtr arr = new FloatArray(maxSamples);

		for ( int i = 0; i < completeLines; ++i ) {
			tmp.resize(tokenSize);
			for ( int j = 0; j < numTokens; ++j ) {
				int idx = 0;
				while ( idx < tokenSize ) {
					int read = ifs.readsome(&tmp[idx], tokenSize-idx);
					if ( read <= 0 ) {
						SEISCOMP_ERROR("%s: read error", file.c_str());
						delete gf;
						return NULL;
					}
					idx += read;
				}

				//std::cout << tmp << "|" << std::flush;

				float value;
				if ( !Core::fromString(value, tmp) ) {
					SEISCOMP_ERROR("%s: invalid numeric value %s at index %d", file.c_str(), tmp.c_str(), count);
					delete gf;
					return NULL;
				}

				if ( count < arr->size() ) (*arr)[count] = value;
				++count;
			}

			//std::cout << std::endl;

			std::getline(ifs, tmp);
		}

		tmp.resize(tokenSize);
		for ( int i = count; i < numSamples; ++i ) {
			int idx = 0;
			while ( idx < tokenSize ) {
				int read = ifs.readsome(&tmp[idx], tokenSize-idx);
				if ( read <= 0 ) {
					SEISCOMP_ERROR("%s: read error", file.c_str());
					delete gf;
					return NULL;
				}
				idx += read;
			}

			float value;
			if ( !Core::fromString(value, tmp) ) {
				SEISCOMP_ERROR("%s: invalid numeric value %s at index %d", file.c_str(), tmp.c_str(), count);
				delete gf;
				return NULL;
			}

			if ( count < arr->size() ) (*arr)[count] = value;
			++count;
		}

		//SEISCOMP_DEBUG("%s: read %d samples", file.c_str(), arr->size());

		if ( orderedComps[c] == Core::ZSS ||
		     orderedComps[c] == Core::ZDS ||
		     orderedComps[c] == Core::ZDD )
		{
			for ( int i = 0; i < arr->size(); ++i )
				(*arr)[i] *= -1.0f;
		}

		gf->setData(orderedComps[c], arr.get());

		std::getline(ifs, tmp);
		// --- to here
	}

	return gf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
