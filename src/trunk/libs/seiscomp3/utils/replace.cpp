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


#define SEISCOMP_COMPONENT Utils/Replace
#include <seiscomp3/utils/replace.h>
#include <stdlib.h>

/*
#include <seiscomp3/core/platform/platform.h>
#ifdef __APPLE__
    // On OSX HOST_NAME_MAX is not define in sys/params.h. Therefoere we use MAXHOSTNAMELEN
    // instead on this platform.
    #include <sys/param.h>
    #define HOST_NAME_MAX MAXHOSTNAMELEN
#endif
*/
#include <seiscomp3/core/system.h>

#include <seiscomp3/logging/log.h>
#include <iostream>
#include <errno.h>
#include <cstdlib>

namespace Seiscomp {
namespace Util {

bool VariableResolver::resolve(std::string& variable) const {
	if ( variable == "hostname" ) {
		variable = Core::getHostname();
	}
	else if ( variable == "user" ) {
		variable = Core::getLogin();
		if ( variable.empty() ){
			char* tmp;
			tmp = getenv("USER");
			if( tmp == NULL )
				return false;
			else
				variable = tmp;
		}
	}
	else
		return false;

	return true;
}

namespace {

VariableResolver defaultResolver;

}


std::string replace(const std::string& input) {
	return replace(input, defaultResolver);
}


std::string replace(const std::string &input,
                    const VariableResolver &resolver) {
	return replace(input, resolver, "@", "@", "@");
}


std::string replace(const std::string &input,
                    const VariableResolver &resolver,
                    const std::string &prefix, const std::string &postfix,
                    const std::string &emptyValue) {
	std::string::size_type lastPos = 0, pos = 0;
	std::string result;

	while ( (pos = input.find(prefix, pos)) != std::string::npos ) {
		std::string::size_type endPos;
		endPos = input.find(postfix, pos+prefix.size());
		if ( endPos == std::string::npos )
			break;

		// skip leading '@'
		std::string placeHolder = input.substr(pos+prefix.size(), endPos-pos-prefix.size());

		if ( placeHolder.empty() )
			placeHolder = emptyValue;
		else if ( !resolver.resolve(placeHolder) ) {
			pos = endPos+postfix.size();
			continue;
		}

		result.append(input, lastPos, pos-lastPos);
		result += placeHolder;

		pos = endPos+postfix.size();
		lastPos = pos;
	}

	result.append(input, lastPos, input.size()-lastPos);

	return result;
}


std::string replace(const char* input,
                    const VariableResolver& resolver) {
	return replace(std::string(input), resolver);
}


}
}
