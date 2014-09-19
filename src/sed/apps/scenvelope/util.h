/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#ifndef __SEISCOMP_APPLICATIONS_SCENVELOPE_UTIL_H__
#define __SEISCOMP_APPLICATIONS_SCENVELOPE_UTIL_H__


#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/processing/waveformprocessor.h>
#include <string>
#include <set>
#include <map>


namespace Seiscomp {
namespace Private {


typedef std::set<std::string> StringSet;
typedef std::map<std::string, bool> StringPassMap;

struct StringFirewall {
	StringSet allow;
	StringSet deny;
	mutable StringPassMap cache;

	bool isAllowed(const std::string &s) const;
	bool isBlocked(const std::string &s) const;
};


std::string
toStreamID(const DataModel::WaveformStreamID &wfid);


DataModel::Stream*
findStreamMaxSR(DataModel::Station *station, const Core::Time &time,
                Processing::WaveformProcessor::SignalUnit requestedUnit,
                const StringFirewall *firewall);


}
}


#endif

