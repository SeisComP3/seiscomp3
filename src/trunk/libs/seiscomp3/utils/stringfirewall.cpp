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
 *                                                                         *
 *   Author: Jan Becker <jabe@gempa.de>                                    *
 ***************************************************************************/


#include "stringfirewall.h"

#include <seiscomp3/core/strings.h>


using namespace std;


namespace Seiscomp {
namespace Util {


namespace {


bool passes(const StringFirewall::StringSet &ids, const std::string &id) {
	StringFirewall::StringSet::iterator it;
	for ( it = ids.begin(); it != ids.end(); ++it ) {
		if ( Core::wildcmp(*it, id) )
			return true;
	}

	return false;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StringFirewall::isAllowed(const std::string &s) const {
	return (allow.empty()?true:allow.find(s) != allow.end())
	    && (deny.empty()?true:deny.find(s) == deny.end());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StringFirewall::isDenied(const std::string &s) const {
	return !isAllowed(s);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WildcardStringFirewall::WildcardStringFirewall()
: _enableCaching(true) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WildcardStringFirewall::clearCache() const {
	_cache.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WildcardStringFirewall::setCachingEnabled(bool enable) {
	_enableCaching = enable;
	if ( !_enableCaching )
		_cache.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WildcardStringFirewall::isAllowed(const std::string &s) const {
	// If no rules are configured, don't cache anything and just return true
	if ( allow.empty() && deny.empty() ) return true;

	if ( _enableCaching ) {
		StringPassMap::const_iterator it = _cache.find(s);

		// Not yet cached, evaluate the string
		if ( it == _cache.end() ) {
			bool check = (allow.empty()?true:passes(allow, s))
			          && (deny.empty()?true:!passes(deny, s));
			_cache[s] = check;
			return check;
		}

		// Return cached result
		return it->second;
	}
	else {
		return (allow.empty()?true:passes(allow, s))
		    && (deny.empty()?true:!passes(deny, s));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
