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

#define SEISCOMP_COMPONENT KIWI
#include <seiscomp3/logging/log.h>

#include <seiscomp3/core/plugin.h>
#include <seiscomp3/core/system.h>

#include <boost/version.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "kiwiarchive.h"


namespace fs = boost::filesystem;


REGISTER_GFARCHIVE(KIWIArchive, "kiwi");
ADD_SC_PLUGIN("KIWI hdf5 greensfunction access", "gempa GmbH <jabe@gempa.de>", 0, 1, 0)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
KIWIArchive::KIWIArchive() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
KIWIArchive::KIWIArchive(const std::string &url) {
	setSource(url);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
KIWIArchive::~KIWIArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool KIWIArchive::setSource(std::string source) {
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

			DB *db = new DB;
			if ( db->open(_baseDirectory + "/" + name) ) {
				_models[name] = db;
			}
			else
				delete db;
		}
	}
	catch ( ... ) {}

	return !_models.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void KIWIArchive::close() {
	for ( std::map<std::string,DB*>::iterator it = _models.begin();
	      it != _models.end(); ++it )
		delete it->second;

	_models.clear();
	_requests.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<std::string> KIWIArchive::availableModels() const {
	std::list<std::string> models;

	for ( std::map<std::string,DB*>::const_iterator it = _models.begin();
	      it != _models.end(); ++it )
		models.push_back(it->first);

	return models;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<double> KIWIArchive::availableDepths(const std::string &model) const {
	std::list<double> res;

	std::map<std::string, DB*>::const_iterator it =
		_models.find(model);

	if ( it == _models.end() ) {
		SEISCOMP_DEBUG("Model %s not available, empty depth set", model.c_str());
		return res;
	}

	SEISCOMP_DEBUG("Got %d depth steps", it->second->nz);
	float startDepth = 0;
	for ( int i = 0; i < it->second->nz; ++i ) {
		res.push_back(startDepth);
		startDepth += it->second->dz * 0.001;
	}

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool KIWIArchive::setTimeSpan(const Seiscomp::Core::TimeSpan &span) {
	_defaultTimespan = span;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool KIWIArchive::addRequest(const std::string &id,
                             const std::string &model, double distance,
                             double depth) {
	if ( _models.find(model) == _models.end() ) {
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
bool KIWIArchive::addRequest(const std::string &id,
                             const std::string &model, double distance,
                             double depth,
                             const Seiscomp::Core::TimeSpan &span) {
	if ( _models.find(model) == _models.end() ) {
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
Seiscomp::Core::GreensFunction* KIWIArchive::get() {
	while ( !_requests.empty() ) {
		std::map<std::string, DB*>::iterator it =
			_models.find(_requests.front().model);

		if ( it == _models.end() ) {
			_requests.pop_front();
			continue;
		}

		Seiscomp::Core::GreensFunction *gf =
			it->second->readGF(_requests.front().distance,
			                   _requests.front().depth,
			                   _requests.front().timeSpan?_requests.front().timeSpan:_defaultTimespan);

		if ( gf ) {
			gf->setModel(it->first);
			gf->setId(_requests.front().id);
			_requests.pop_front();
			return gf;
		}

		_requests.pop_front();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
