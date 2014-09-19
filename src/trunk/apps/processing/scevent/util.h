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




#ifndef __SEISCOMP_APPLICATIONS_EVENTTOOL_UTIL_H__
#define __SEISCOMP_APPLICATIONS_EVENTTOOL_UTIL_H__


#include <string>
#include <stdint.h>
#include "config.h"


namespace Seiscomp {

namespace DataModel {

class Arrival;
class Origin;
class FocalMechanism;
class Event;
class EventDescription;
class Magnitude;
class DatabaseArchive;

}

namespace Private {

#define ORIGIN_PRIORITY_MAX  100
#define FOCALMECHANISM_PRIORITY_MAX  100

std::string encode(uint64_t x, const char *sym, int numsym, int len,
                   uint64_t *width = NULL);
std::string encodeChar(uint64_t x, int len, uint64_t *width = NULL);
std::string encodeInt(uint64_t x, int len, uint64_t *width = NULL);
std::string encodeHex(uint64_t x, int len, uint64_t *width = NULL);
std::string generateEventID(int year, uint64_t x,
                            const std::string &prefix,
                            const std::string &pattern,
                            std::string &textBlock,
                            uint64_t *width = NULL);

std::string allocateEventID(DataModel::DatabaseArchive *,
                            const DataModel::Origin *origin,
                            const std::string &prefix,
                            const std::string &pattern,
                            const Client::Config::StringSet *blackList = NULL);

std::string region(const DataModel::Origin *origin);

double arrivalWeight(const DataModel::Arrival *arr, double defaultWeight=1.);
int stationCount(const DataModel::Magnitude *mag);
int priority(const DataModel::Origin *origin);
int priority(const DataModel::FocalMechanism *fm);
int definingPhaseCount(const DataModel::Origin *origin);
double rms(const DataModel::Origin *origin);

template <typename T>
bool isRejected(T *obj);

template <typename T>
Core::Time created(T *obj);

DataModel::EventDescription *eventRegionDescription(DataModel::Event *ev);

int magnitudePriority(const std::string &magType, const Client::Config &config);
int agencyPriority(const std::string &agencyID, const Client::Config &config);
int authorPriority(const std::string &author, const Client::Config &config);
int methodPriority(const std::string &methodID, const Client::Config &config);

int goodness(const DataModel::Magnitude *netmag, int mbcount,
             double mbval, const Client::Config &config);
bool isMw(const DataModel::Magnitude *netmag);



template <typename T>
Core::Time created(T *obj) {
	try {
		return obj->creationInfo().creationTime();
	}
	catch ( ... ) {
		return Core::Time();
	}
}


template <typename T>
bool isRejected(T *obj) {
	try {
		return obj->evaluationStatus() == DataModel::REJECTED;
	}
	catch ( ... ) {
		return false;
	}
}


}
}


#endif
