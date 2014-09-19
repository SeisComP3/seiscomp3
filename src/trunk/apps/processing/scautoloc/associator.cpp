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




#define SEISCOMP_COMPONENT Autoloc
#include <seiscomp3/logging/log.h>

#include <math.h>
#include <seiscomp3/seismology/ttt.h>

#include "util.h"
#include "sc3adapters.h"
#include "associator.h"

using namespace std;

namespace Autoloc {

#define AFFMIN 0.1

Associator::Associator()
{
	_origins = 0;
	_stations = 0;

	// The order of the phases is crucial!
	_phases.push_back( Phase("P",      0, 180) );
	_phases.push_back( Phase("PcP",   25,  55) );
	_phases.push_back( Phase("ScP",   25,  55) );
	_phases.push_back( Phase("PP",    60, 160) );
// FIXME: For these, there are no tables in LocSAT!
	_phases.push_back( Phase("SKP",  120, 150) );
	_phases.push_back( Phase("PKKP",  80, 130) );
	_phases.push_back( Phase("PKiKP", 30, 120) );
	_phases.push_back( Phase("SKKP", 110, 152) );
// TODO: make the phase set configurable
}

Associator::~Associator()
{
}

void
Associator::setStations(const StationDB *stations)
{
	_stations = stations;
}

void
Associator::setOrigins(const OriginDB *origins)
{
	_origins = origins;
}


void
Associator::reset()
{
	_associations.clear();
}


void
Associator::shutdown()
{
	reset();
}


bool
Associator::feed(const Pick* pick)
{
	_associations.clear();

	if ( ! _origins)
		return false;

	static Seiscomp::TravelTimeTable ttt;

	int count = 0;

	for(OriginDB::const_iterator
	    it=_origins->begin(); it != _origins->end(); ++it) {

		const Origin  *origin = (*it).get();
		const Station *station = pick->station();

		double delta, az, baz;
		delazi(origin, station, delta, az, baz);

		Seiscomp::TravelTimeList
			*ttlist = ttt.compute(origin->lat, origin->lon, origin->dep,
			                      station->lat, station->lon, 0);

		// An imported origin is treated as if it had a very high
		// score. => Anything can be associated with it.
		double origin_score = origin->imported ? 1000 : origin->score;

		for(vector<Phase>::iterator
		    it = _phases.begin(); it != _phases.end(); ++it) {

			const Phase &phase = *it;

			// TODO: make this configurable
//			if (origin->definingPhaseCount() < (phase.code=="P" ? 8 : 30))
			if (origin_score < (phase.code=="P" ? 20 : 50))
				continue;

			if (delta < phase.dmin || delta > phase.dmax)
				continue;

			double ttime = -1, x = 1;

			if (phase.code == "P") {
				for (Seiscomp::TravelTimeList::iterator
				     it = ttlist->begin(); it != ttlist->end(); ++it) {

					const Seiscomp::TravelTime &tt = *it;
					if (delta < 114) {
						// for  distances < 114, allways take 1st arrival
						ttime = tt.time;
						break;
					}
					if (tt.phase.substr(0,2) != "PK")
						// for distances >= 114, skip Pdiff etc., take first
						// PKP*, PKiKP*
						continue;
					ttime = tt.time;
					break;
				}
				// Weight residuals at regional distances "a bit" lower
				// This is quite hackish!
				x = 1 + 0.6*exp(-0.003*delta*delta) + 0.5*exp(-0.03*(15-delta)*(15-delta));
			}
			else {
				for (Seiscomp::TravelTimeList::iterator
				     it = ttlist->begin(); it != ttlist->end(); ++it) {

					const Seiscomp::TravelTime &tt = *it;
					if (tt.phase.substr(0, phase.code.size()) == phase.code) {
						ttime = tt.time;
						break;
					}
				}
			}

			if (ttime == -1) // phase not found
				continue;

			// compute "affinity" based on distance and residual
			double affinity = 0;
			double residual = double(pick->time - (origin->time + ttime));
			if ( origin->imported ) {
				// This is a clear-cut decision: If the pick is within the interval, associate it with affinity 1, otherwise skip
				if (residual < -20 || residual > 30) // TODO: Make this configurable
					continue;
				affinity = 1;
			}
			else {
				residual = residual/x;
				residual /= 10;       // normalize residual

				affinity = avgfn(residual); // test if exp(-residual**2) if better
				if (affinity < AFFMIN)
					continue;
			}
			string phcode = phase.code;
			if (phcode=="P" && ttime > 960)
				phcode = "PKP";
			Association asso(origin, pick, phcode, residual, affinity);
			asso.distance = delta;
			asso.azimuth = az;
			_associations.push_back(asso);
			count++;

			break; // ensure no more than one association per origin
		}

		delete ttlist;
	}

	return (_associations.size() > 0);
}

const AssociationVector &
Associator::associations() const
{
	return _associations;
}


Association*
Associator::associate(Origin *origin, const Pick *pick, const string &phase)
{
	return NULL;
}


Associator::Phase::Phase(const string &code, double dmin, double dmax)
	: code(code), dmin(dmin), dmax(dmax) {}


}  // namespace Autoloc
