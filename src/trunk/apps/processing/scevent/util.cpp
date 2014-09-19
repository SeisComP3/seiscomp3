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




#include "util.h"
#include "string.h"

#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/databasearchive.h>
#include <seiscomp3/seismology/regions.h>

#define SEISCOMP_COMPONENT SCEVENT
#include <seiscomp3/logging/log.h>


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;


namespace Seiscomp {
namespace Private {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string makeUpper(const std::string &src) {
	string dest = src;
	for ( size_t i = 0; i < src.size(); ++i )
		dest[i] = toupper(src[i]);
	return dest;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string encode(uint64_t x, const char *sym, int numsym,
                   int len, uint64_t *width) {
	string enc;

	if ( len == 0 ) len = 4;

	uint64_t dx = uint64_t(((370*24)*60)*60)*1000;
	uint64_t w, rng = 1, tmp = rng;

	for ( int i = 0; i < len; ++i ) {
		tmp = rng * numsym;
		if ( tmp > rng ) rng = tmp;
		else {
			len = i;
			break;
		}
	}

	w = dx / rng;
	if ( w == 0 ) w = 1;

	if ( dx >= rng )
		x /= w;
	else
		x *= (rng / dx);

	for ( int i = 0; i < len; ++i ) {
		uint64_t d = x / numsym;
		uint64_t r = x % numsym;
		enc += sym[r];
		x = d;
	}

	if ( width ) *width = w;

	return string(enc.rbegin(), enc.rend());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string encodeChar(uint64_t x, int len, uint64_t *width) {
	return encode(x, "abcdefghijklmnopqrstuvwxyz", 26, len, width);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string encodeInt(uint64_t x, int len, uint64_t *width) {
	return encode(x, "0123456789", 10, len, width);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string encodeHex(uint64_t x, int len, uint64_t *width) {
	return encode(x, "0123456789abcdef", 16, len, width);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string generateEventID(int year, uint64_t x, const std::string &prefix,
                            const string &pattern, std::string &textBlock, uint64_t *width) {
	string evtID;
	textBlock = "";

	for ( size_t i = 0; i < pattern.size(); ++i ) {
		if ( pattern[i] != '%' )
			evtID += pattern[i];
		else {
			++i;
			int len = 0;
			while ( i < pattern.size() ) {
				if ( pattern[i] >= '0' && pattern[i] <= '9' ) {
					len *= 10;
					len += int(pattern[i] - '0');
					++i;
					continue;
				}
				else if ( pattern[i] == '%' ) {
					evtID += pattern[i];
				}
				else if ( pattern[i] == 'c' ) {
					textBlock = encodeChar(x, len, width);
					evtID += textBlock;
				}
				else if ( pattern[i] == 'C' ) {
					textBlock = makeUpper(encodeChar(x, len, width));
					evtID += textBlock;
				}
				else if ( pattern[i] == 'd' ) {
					textBlock = encodeInt(x, len, width);
					evtID += textBlock;
				}
				else if ( pattern[i] == 'x' ) {
					textBlock = encodeHex(x, len, width);
					evtID += textBlock;
				}
				else if ( pattern[i] == 'X' ) {
					textBlock = makeUpper(encodeHex(x, len, width));
					evtID += textBlock;
				}
				else if ( pattern[i] == 'Y' )
					evtID += toString(year);
				else if ( pattern[i] == 'p' )
					evtID += prefix;
				else
					return "";

				break;
			}
		}
	}

	return evtID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string allocateEventID(DatabaseArchive *ar, const Origin *origin,
                       const string &prefix, const string &pattern,
                       const Client::Config::StringSet *blackList) {
	if ( !origin )
		return "";

	int year, yday, hour, min, sec, usec;

	if ( !origin->time().value().get2(&year, &yday, &hour, &min, &sec, &usec) )
		return "";

	uint64_t width;
	// Maximum precission is 1 millisecond
	uint64_t x = uint64_t((((yday*24)+hour)*60+min)*60+sec)*1000 + usec/1000;

	string text;
	string eventID = generateEventID(year, x, prefix, pattern, text, &width);
	ObjectPtr o = ar?ar->getObject(Event::TypeInfo(), eventID):NULL;
	bool blocked = (blackList != NULL) && (blackList->find(text) != blackList->end());

	if ( !o && !blocked )
		return eventID;

	if ( blocked ) SEISCOMP_WARNING("Blocked ID: %s (rejected %s)", eventID.c_str(), text.c_str());

	for ( int i = 1; i < 5; ++i ) {
		eventID = generateEventID(year, x+i*width, prefix, pattern, text);
		blocked = (blackList != NULL) && (blackList->find(text) != blackList->end());
		o = ar?ar->getObject(Event::TypeInfo(), eventID):NULL;
		if ( !o && !blocked )
			return eventID;
		if ( blocked ) SEISCOMP_WARNING("Blocked ID: %s (rejected %s)", eventID.c_str(), text.c_str());
	}

	for ( int i = 1; i < 5; ++i ) {
		eventID = generateEventID(year, x-i*width, prefix, pattern, text);
		blocked = (blackList != NULL) && (blackList->find(text) != blackList->end());
		o = ar?ar->getObject(Event::TypeInfo(), eventID):NULL;
		if ( !o && !blocked )
			return eventID;
		if ( blocked ) SEISCOMP_WARNING("Blocked ID: %s (rejected %s)", eventID.c_str(), text.c_str());
	}

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string region(const Origin *origin) {
	return Regions::getRegionName(origin->latitude(), origin->longitude());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double arrivalWeight(const DataModel::Arrival *arr, double defaultWeight) {
	try {
		return arr->weight();
	}
	catch ( Core::ValueException& ) {
		return defaultWeight;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int stationCount(const DataModel::Magnitude *mag) {
	try {
		return mag->stationCount();
	}
	catch ( ... ) {
		return -1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int priority(const Origin *origin) {
	EvaluationMode mode = AUTOMATIC;
	try {
		mode = origin->evaluationMode();
	}
	catch ( ValueException& ) {}

	switch ( mode ) {
		case MANUAL:

			try {

				switch ( origin->evaluationStatus() ) {
					case PRELIMINARY: return 0;
					case CONFIRMED: return 1;
					case REVIEWED: return 2;
					case FINAL: return 3;
					case REPORTED: return -1;
					case REJECTED: return -100;
					default: break;
				}

			}
			catch ( ValueException& ) {
				return 1;
			}

			break;

		case AUTOMATIC:
		default:

			try {

				switch ( origin->evaluationStatus() ) {
					case PRELIMINARY: return 0;
					case CONFIRMED: return 1;
					case REVIEWED: return 2;
					case FINAL: return 3;
					case REPORTED: return -1;
					case REJECTED: return -100;
					default: break;
				}

			}
			catch ( ValueException& ) {
				return 0;
			}

			break;
	}

	SEISCOMP_WARNING("Origin %s has unknown evaluation mode", origin->publicID().c_str());

	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int priority(const DataModel::FocalMechanism *fm) {
	EvaluationMode mode = AUTOMATIC;
	try {
		mode = fm->evaluationMode();
	}
	catch ( ValueException& ) {}

	switch ( mode ) {
		case MANUAL:

			try {

				switch ( fm->evaluationStatus() ) {
					case PRELIMINARY: return 0;
					case CONFIRMED: return 1;
					case REVIEWED: return 2;
					case FINAL: return 3;
					case REPORTED: return -1;
					case REJECTED: return -100;
					default: break;
				}

			}
			catch ( ValueException& ) {
				return 1;
			}

			break;

		case AUTOMATIC:
		default:

			try {

				switch ( fm->evaluationStatus() ) {
					case PRELIMINARY: return 0;
					case CONFIRMED: return 1;
					case REVIEWED: return 2;
					case FINAL: return 3;
					case REPORTED: return -1;
					case REJECTED: return -100;
					default: break;
				}

			}
			catch ( ValueException& ) {
				return 0;
			}

			break;
	}

	/*
	EvaluationMode mode = AUTOMATIC;
	try {
		mode = fm->evaluationMode();
	}
	catch ( ValueException& ) {}

	switch ( mode ) {
		case MANUAL:
			return 1;

		case AUTOMATIC:
		default:
			return 0;
	}
	*/

	SEISCOMP_WARNING("FocalMechanism %s has unknown evaluation mode", fm->publicID().c_str());

	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int definingPhaseCount(const Origin *origin) {
	try {
		return origin->quality().usedPhaseCount();
	}
	catch ( ValueException& ) {
		int n = 0;
		for ( unsigned int i = 0; i < origin->arrivalCount(); ++i ) {
			try {
				if ( origin->arrival(i)->weight() > 0 )
					++n;
			}
			catch ( ValueException& ) {
				++n;
			}
		}

		return n;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double rms(const DataModel::Origin *origin) {
	try {
		return origin->quality().standardError();
	}
	catch ( ValueException& ) {
		return -1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::EventDescription *
eventRegionDescription(DataModel::Event *ev) {
	for ( size_t i = 0; i < ev->eventDescriptionCount(); ++i ) {
		DataModel::EventDescription *ed = ev->eventDescription(i);
		if ( ed->type() == REGION_NAME )
			return ed;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int magnitudePriority(const std::string &magType, const Client::Config &config) {
	int n = config.magTypes.size();
	for ( Client::Config::StringList::const_iterator it = config.magTypes.begin();
	      it != config.magTypes.end(); ++it, --n ) {
		if ( magType == *it )
			break;
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int agencyPriority(const std::string &agencyID, const Client::Config &config) {
	int n = config.agencies.size();
	for ( Client::Config::StringList::const_iterator it = config.agencies.begin();
	      it != config.agencies.end(); ++it, --n ) {
		if ( agencyID == *it )
			break;
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int authorPriority(const std::string &author, const Client::Config &config) {
	int n = config.authors.size();
	for ( Client::Config::StringList::const_iterator it = config.authors.begin();
	      it != config.authors.end(); ++it, --n ) {
		if ( author == *it )
			break;
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int methodPriority(const std::string &methodID, const Client::Config &config) {
	int n = config.methods.size();
	for ( Client::Config::StringList::const_iterator it = config.methods.begin();
	      it != config.methods.end(); ++it, --n ) {
		if ( methodID == *it )
			break;
	}

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int goodness(const Magnitude *netmag, int mbcount,
             double mbval, const Client::Config &config) {
	if ( !netmag )
		return -1;

	size_t mcount = stationCount(netmag);
	double mval = netmag->magnitude().value();
	
	if ( mcount < config.minStationMagnitudes )
		return 0;

	// Special Mw(mB) criterion
	
	if (isMw(netmag)) {
		if (mcount < config.minMwCount) return 0;

		if (mcount < config.mbOverMwCount) {
			if ( (mval+mbval)/2 < config.mbOverMwValue) return 0;
			if ( (int)mcount<mbcount/2 ) return 0;
		}
	}

	return magnitudePriority(netmag->type(), config);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool isMw(const Magnitude *netmag) {
	return (string(netmag->type(), 0, 2) == "Mw");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
