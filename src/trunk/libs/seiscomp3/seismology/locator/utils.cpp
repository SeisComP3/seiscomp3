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
 ***************************************************************************/


#include <seiscomp3/seismology/locator/utils.h>

#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>

#include <set>
#include <algorithm>


namespace Seiscomp{


void compile(DataModel::OriginQuality &quality, const DataModel::Origin *origin) {
	int usedPhases = 0;
	int usedDepthPhases = 0;

	std::set<std::string> stationsUsed;
	std::set<std::string> stationsAssociated;

	bool validStationCounts = true;

	std::vector<double> dist;
	std::vector<double> azi;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		DataModel::Arrival *arrival = origin->arrival(i);
		DataModel::Pick *pick = DataModel::Pick::Find(arrival->pickID());

		if ( !pick )
			validStationCounts = false;
		else
			stationsAssociated.insert(pick->waveformID().networkCode() + "." + pick->waveformID().stationCode());

		try {
			if ( arrival->weight() <= 0 ) {
				continue;
			}
		}
		catch ( ... ) {}

		++usedPhases;

		if ( !arrival->phase().code().empty() ) {
			if ( arrival->phase().code()[0] == 'p'
			  || arrival->phase().code()[0] == 's' )
				++usedDepthPhases;
		}

		try { dist.push_back(arrival->distance()); }
		catch ( ... ) {}

		try { azi.push_back(arrival->azimuth()); }
		catch ( ... ) {}

		if ( pick )
			stationsUsed.insert(pick->waveformID().networkCode() + "." + pick->waveformID().stationCode());
	}

	if ( !azi.empty() ) {
		std::sort(azi.begin(), azi.end());
		azi.push_back(azi.front()+360.);
		double azGap = 0.;
		if ( azi.size() > 2 )
			for ( size_t i = 0; i < azi.size()-1; ++i )
				azGap = (azi[i+1]-azi[i]) > azGap ? (azi[i+1]-azi[i]) : azGap;

		if ( 0. < azGap && azGap < 360. )
			quality.setAzimuthalGap(azGap);
	}

	if ( !dist.empty() ) {
		std::sort(dist.begin(), dist.end());
		quality.setMinimumDistance(dist.front());
		quality.setMaximumDistance(dist.back());
		quality.setMedianDistance(dist[dist.size()/2]);
	}

	quality.setAssociatedPhaseCount(int(origin->arrivalCount()));
	quality.setUsedPhaseCount(usedPhases);
	quality.setDepthPhaseCount(usedDepthPhases);

	if ( validStationCounts ) {
		quality.setAssociatedStationCount(int(stationsAssociated.size()));
		quality.setUsedStationCount(int(stationsUsed.size()));
	}
}


void populateQuality(DataModel::Origin *origin) {
	try {
		compile(origin->quality(), origin);
	}
	catch ( ... ) {
		DataModel::OriginQuality quality;
		compile(quality, origin);
		origin->setQuality(quality);
	}
}


}
