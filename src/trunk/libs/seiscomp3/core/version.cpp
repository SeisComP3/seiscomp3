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


#include <seiscomp3/core/version.h>
#include <seiscomp3/core/strings.h>
#include <iostream>


#ifdef WITH_SVN_REVISION
extern SC_SYSTEM_CORE_API const char* git_revision();
#endif
#ifdef WITH_BUILD_INFOS
extern SC_SYSTEM_CORE_API const char* build_system();
extern SC_SYSTEM_CORE_API const char* compiler_version();
extern SC_SYSTEM_CORE_API const char* os_version();
#endif


namespace Seiscomp {
namespace Core {


FrameworkVersion CurrentVersion;


FrameworkVersion::FrameworkVersion() {
	_text = "Jakarta 2015.070";
}


std::string FrameworkVersion::toString() const {
	return _text;
}


std::string FrameworkVersion::systemInfo() const {
	std::string s;
#ifndef WIN32
#ifdef WITH_BUILD_INFOS
	s += std::string("Compiler: ") + compiler_version() + "\n";
	s += std::string("Build system: ") + build_system() + "\n";
	s += std::string("OS: ") + os_version();
#endif
#endif
	return s;
}


std::string Version::toString() const {
	return Core::toString(majorTag()) + "." + Core::toString(minorTag());
}


bool Version::fromString(const std::string &str) {
	size_t pos = str.find('.');
	if ( pos == std::string::npos ) return false;

	int maj, min;
	if ( !Core::fromString(maj, str.substr(0, pos)) ||
		 !Core::fromString(min, str.substr(pos+1)) ) {
		return false;
	}

	if ( (maj & 0xFFFF0000) || (min & 0xFFFF0000) )
		return false;

	packed = pack(maj, min);
	return true;
}


}
}
