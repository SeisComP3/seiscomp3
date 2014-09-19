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


#define SEISCOMP_COMPONENT GFArchive

#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/gfarchive.h>
#include <seiscomp3/core/interfacefactory.ipp>

#include <string.h>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::IO::GFArchive, SC_SYSTEM_CORE_API);


namespace Seiscomp {
namespace IO {


IMPLEMENT_SC_ABSTRACT_CLASS(GFArchive, "GFArchive");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GFArchive::GFArchive() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GFArchive::~GFArchive() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GFArchive* GFArchive::Create(const char* service) {
	return GFArchiveFactory::Create(service);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GFArchive* GFArchive::Open(const char* url) {
	const char* tmp;
	std::string service;
	std::string source;
	std::string type;

	tmp = strstr(url, "://");
	if ( tmp ) {
		std::copy(url, tmp, std::back_inserter(service));
		url = tmp + 3;
	}

	source = url;

	if ( service.empty() ) {
		SEISCOMP_ERROR("empty gfarchive service passed");
		return NULL;
	}

	SEISCOMP_DEBUG("trying to open archive %s://%s%s%s",
	               service.c_str(), source.c_str(), type.empty()?"":"#", type.c_str());

	GFArchive* ar;

	ar = Create(service.c_str());

	if ( !ar ) {
		SEISCOMP_DEBUG("gfarchive backend '%s' does not exist",
		               service.c_str());
		return NULL;
	}

	if ( !ar->setSource(source.c_str()) ) {
		SEISCOMP_DEBUG("gfarchive '%s' failed to set source",
		               source.c_str());
		delete ar;
		ar = NULL;
	}

	return ar;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<std::string> GFArchive::availableModels() const {
	return std::list<std::string>();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<double> GFArchive::availableDepths(const std::string &model) const {
	return std::list<double>();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
