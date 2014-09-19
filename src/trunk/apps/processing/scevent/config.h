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




#ifndef __SEISCOMP_APPLICATIONS_EVENTTOOL_CONFIG_H__
#define __SEISCOMP_APPLICATIONS_EVENTTOOL_CONFIG_H__

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/datamodel/types.h>
#include <vector>
#include <string>


namespace Seiscomp {

namespace Client {


struct Config {
	DEFINE_SMARTPOINTER(Region);
	struct Region : public Core::BaseObject {
		virtual bool init(const Seiscomp::Config::Config &config, const std::string &prefix) = 0;
		virtual bool isInside(double lat, double lon) const = 0;
	};

	struct RegionFilter {
		RegionPtr   region;
		OPT(double) minDepth;
		OPT(double) maxDepth;
	};

	typedef std::vector<RegionFilter> RegionFilters;

	struct EventFilter {
		OPT(std::string) agencyID;
		OPT(std::string) author;
		OPT(DataModel::EvaluationMode) evaluationMode;
	};

	typedef std::vector<std::string> StringList;
	typedef std::set<std::string> StringSet;

	size_t          minAutomaticArrivals;
	size_t          minMatchingPicks;
	double          maxMatchingPicksTimeDiff;
	bool            matchingPicksTimeDiffAND;
	size_t          minStationMagnitudes;
	double          maxDist;
	Core::TimeSpan  maxTimeDiff;
	Core::TimeSpan  eventTimeBefore;
	Core::TimeSpan  eventTimeAfter;

	RegionFilters   regionFilter;

	size_t          mbOverMwCount;
	double          mbOverMwValue;
	size_t          minMwCount;

	bool            enableFallbackPreferredMagnitude;
	bool            updatePreferredSolutionAfterMerge;

	std::string     eventIDPrefix;
	std::string     eventIDPattern;
	StringList      magTypes;
	StringList      agencies;
	StringList      authors;
	StringList      methods;
	std::string     score;
	StringSet       blacklistIDs;

	StringList      priorities;

	EventFilter     delayFilter;
	int             delayTimeSpan;

	int             delayPrefFocMech;
	bool            ignoreMTDerivedOrigins;
};


}

}


#endif
