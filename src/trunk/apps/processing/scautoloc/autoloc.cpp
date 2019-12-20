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
//#define EXTRA_DEBUGGING
//#define LOG_RELOCATOR_CALLS
#include <seiscomp3/logging/log.h>
#include <seiscomp3/seismology/ttt.h>

#include "util.h"
#include "sc3adapters.h"
#include "nucleator.h"
#include "autoloc.h"

using namespace std;

namespace Autoloc {

static bool valid(const Pick *pick)
{
	// don't look any further at a pick for which we don't have station info
	if ( ! pick->station())
		return false;

	// any non-automatic pick is considered valid anyway
	if ( ! automatic(pick))
		return true;

	// the following is only relevant for automatic picks

	if (pick->snr <= 0 || pick->snr > 1.0E7) {
		if (pick->snr > 1.0E7)
			// If SNR is so high, something *must* be wrong
			SEISCOMP_WARNING("Pick %s with snr of %g was rejected", pick->id.c_str(), pick->snr);
		return false;
	}

	if ( ! hasAmplitude(pick))
		return false;

	return true;
}

static int arrivalWithLargestResidual(const Origin *origin)
{
	int arrivalCount = origin->arrivals.size(), imax=-1;
	double resmax = 0;
	for (int i=0; i<arrivalCount; i++) {

		const Arrival &arr = origin->arrivals[i];
		if (arr.excluded) continue;
		double absres = fabs(arr.residual);
		if (absres>resmax) {
			resmax = absres;
			imax = i;
		}
	}
	return imax;
}


Autoloc3::Autoloc3()
{
	_now = _nextCleanup = 0;
	_associator.setOrigins(&_origins);
	_relocator.setMinimumDepth(_config.minimumDepth);
}


Autoloc3::~Autoloc3()
{
}


bool Autoloc3::init()
{
        if ( ! _relocator.init())
                return false;

	_relocator.setMinimumDepth(_config.minimumDepth);

	if ( ! _config.staConfFile.empty()) {
		SEISCOMP_DEBUG_S("Reading station config from file '"+ _config.staConfFile +"'");

		if ( ! _stationConfig.read(_config.staConfFile) )
		    return false;
	}

        if ( ! _nucleator.init())
                return false;

	setLocatorProfile(_config.locatorProfile);

	return true; // ready to start processing
}


void Autoloc3::dumpState() const
{
	for (OriginVector::const_iterator
	     it = _origins.begin(); it != _origins.end(); ++it) {

		const Origin *origin = (*it).get();
		SEISCOMP_INFO_S(printOneliner(origin));
	}
}


bool Autoloc3::_report(const Origin *origin)
{
	// This is a dummy. Replace it by something suitable.
	SEISCOMP_INFO_S(" OUT " + printOneliner(origin));

	return true;
}


bool Autoloc3::report()
{
	for (OriginVector::iterator
	     it = _newOrigins.begin(); it != _newOrigins.end(); ) {

		Origin *origin = (*it).get();

		if (_nextDue.find(origin->id) == _nextDue.end())
			// first origin -> report immediately
			_nextDue[origin->id] = 0;

		_outgoing[origin->id] = origin;
		it = _newOrigins.erase(it);
	}

	_flush();

	return true;
}


void Autoloc3::_flush()
{
	Time t = now();
	vector<OriginID> ids;

	int dnmax = _config.publicationIntervalPickCount;

	for (map<int, OriginPtr>::iterator
	     it = _outgoing.begin(); it != _outgoing.end(); ++it) {

		const Origin *origin = (*it).second.get();
		double dt = t - _nextDue[origin->id];
		int dn = dnmax;

		if (_lastSent.find(origin->id) != _lastSent.end())
//			dn = origin->definingPhaseCount() - _lastSent[origin->id]->definingPhaseCount();
			dn = origin->phaseCount() - _lastSent[origin->id]->phaseCount();
		if (dt >= 0 || dn >= dnmax)
			ids.push_back(origin->id);
	}

	for(vector<OriginID>::iterator
	    it = ids.begin(); it != ids.end(); ++it) {

		OriginID id = *it;
		const Origin *origin = _outgoing[id].get();

		if ( ! _publishable(origin) ) {
			_outgoing.erase(id);
			continue;
		}

		// Test if we have previously sent an earlier version of this origin.
		// If so, test if the current version has improved.
		// TODO: perhaps move this test to _publishable()
		if (_lastSent.find(id) != _lastSent.end()) {
			const Origin *previous = _lastSent[id].get();

			// The main criterion is definingPhaseCount. However,
			// there may be origins with additional but excluded phases
			// like PKP and such origins should also be sent.
			if (origin->definingPhaseCount() <= previous->definingPhaseCount()) {

				if (origin->arrivals.size() <= previous->arrivals.size() ||
				    now() - previous->timestamp < 150) {  // TODO: make this configurable
					// ... some more robust criteria perhaps
					SEISCOMP_INFO("Origin %ld not sent (no improvement)", origin->id);
					_outgoing.erase(id);
					continue;
				}
			}
		}

		if (_report(origin)) {
			SEISCOMP_INFO_S(" OUT " + printOneliner(origin));

			// Compute the time at which the next origin in this
			// series would be due to be reported, if any.
			int N = origin->definingPhaseCount();
			// This defines the minimum time interval between
			// adjacent origins to be reported. Larger origins may
			// put a higher burden on the system, but change less,
			// so larger time intervals are justified. The time
			// interval is a linear function of the defining phase
			// count.
			double
				A  = _config.publicationIntervalTimeSlope,
				B  = _config.publicationIntervalTimeIntercept,
				dt = A*N + B;

//			if (dt < 0 || _config.playback) {
			if (dt < 0) {
				_nextDue[id] = 0;
				SEISCOMP_INFO("Autoloc3::_flush() origin=%ld  next due IMMEDIATELY", id);
			}
			else {
				_nextDue[id] = t + dt;
				SEISCOMP_INFO("Autoloc3::_flush() origin=%ld  next due: %s", id, time2str(_nextDue[id]).c_str());
			}

			// save a copy of the origin
			_lastSent[id] = new Origin(*origin);
			_lastSent[id]->timestamp = t;
			_outgoing.erase(id);
		}
	}
}


bool Autoloc3::_blacklisted(const Pick *pick) const
{
	if (_blacklist.find(pick) == _blacklist.end())
		// TODO to be implemented
		return false;

	return true;
}


void Autoloc3::_setBlacklisted(const Pick *pick, bool yes)
{
	if (yes) {
		SEISCOMP_INFO_S("process pick BLACKLISTING " + pick->id + " (manual pick)");
		// FIXME: memory leak!
		_blacklist.insert(pick);
		return;
	}
	// TODO else
}


bool Autoloc3::_addStationInfo(const Pick *pick)
{
	if (pick->station())
		return true; // nothing to do

	const string net_sta = pick->net + "." + pick->sta;
	StationMap::const_iterator it = _stations.find(net_sta);
	if (it == _stations.end()) {

		// remember missing stations already complained about
		if (_missingStations.find(net_sta) == _missingStations.end()) {
			SEISCOMP_ERROR_S("Autoloc3: MISSING STATION "+net_sta);
			_missingStations.insert(net_sta);
		}
		return false;
	}
	pick->setStation((*it).second.get());
	return true;
}


const Pick* Autoloc3::pick(const string &id) const
{
	PickMap::const_iterator it = _pick.find(id);
	if (it != _pick.end())
		return it->second.get();

	return NULL;
}


Time Autoloc3::now()
{
	if (_config.playback)
		return _now;

	return Time(Seiscomp::Core::Time::GMT());
}


bool Autoloc3::_store(const Pick *pick)
{
	if ( ! _addStationInfo(pick))
		return false;

	if ( ! pick->station())
		return false;

	if ( automatic(pick) && ! pick->station()->used)
		// This means that this pick is completely ignored!
		// Nevertheless, we might want to loosely associate it to an
		// origin, i.e. associate it without using it for location
		return false;
		// A manual pick, however, is processed, because we assume
		// that the analyst knows best!

	// pick too old? -> ignored completely
	if (pick->time < now() - _config.maxAge) {
		SEISCOMP_INFO_S("ignored old pick " + pick->id);
		return false;
	}

	// adjust time in offline mode
	if (_config.playback && pick->time > _now)
		_now = pick->time;

	// physically store the pick
	if ( Autoloc3::pick(pick->id) == NULL )
		_pick[ pick->id ] = pick;

	return true;
}


bool Autoloc3::feed(const Pick *pick)
{
	_newOrigins.clear();
	bool isnew = ( Autoloc3::pick(pick->id)==NULL );

	if ( ! _store(pick))
		return false;

	// Currently we require amplitudes to be present.
	// Otherwise the pick is ignored for the time being,
	// and processed once the amplitudes are present.
	if ( automatic(pick) ) {
		if ( ! hasAmplitude(pick)) {
			if (isnew)
				SEISCOMP_DEBUG("process pick %-35s %c   waiting for amplitude",
					       pick->id.c_str(), statusFlag(pick));
			return false;
		}
	}

	// A previous version of the new pick might have been updated in _store(); 
	if ( ! _process( Autoloc3::pick(pick->id) ))
		return false;

	report();
	cleanup();

	return true;
}


Origin *Autoloc3::_findMatchingOrigin(const Origin *origin)
{
	// find commonalities with existing origins
	// * identical picks
	// * similar picks (same stream but slightly different times)
	// replace similar picks by the ones found in the new origin, incl. weight
	Origin *found = 0;
	unsigned int bestmatch = 0;

	// iterate over existing origins
	for (OriginVector::iterator
	     it = _origins.begin(); it != _origins.end(); ++it) {
		Origin *existing = (*it).get();

		// It makes no sense to compare origins too different in time. This maximum time difference is for teleseismic worst case where we might need to associate origins wrongly located e.g. by using PKP as P where time differences of up to 20 minutes are possible. This time difference may be made configurable but this is not crucial.
		if (fabs(origin->time - existing->time) > 20*60) continue;

		unsigned int identical=0, similar=0;

		// go through this origin and look for manual picks
		for (unsigned int i1=0; i1<existing->arrivals.size(); i1++) {
			const Pick *pick = existing->arrivals[i1].pick.get();

			if (pick->station() == NULL) {
				const string net_sta = pick->net + "." + pick->sta;
				SEISCOMP_WARNING("Pick %3d   %s    %s  without station info",i1,
						 net_sta.c_str(),pick->id.c_str());
				continue;
			}

			// try to find a matching pick in our newly fed origin
			for (unsigned int i2=0; i2<origin->arrivals.size(); i2++) {
				const Pick *pick2 = origin->arrivals[i2].pick.get();

				// identical picks?
				if (pick2 == pick) {
					// found identical pick
					identical++;
					break;
					// TODO: adopt arrival weight etc.
				}

				// picks for same station
				if (pick2->station() == pick->station()) {
					double dt = pick2->time - pick->time;
					if ( dt >= -20 && dt <= 20 ) {
						// found similar pick
						similar++;
						break;
						// TODO: adopt arrival weight etc.
					}
				}
			}
		}

		if (identical+similar > 0) {
//			SEISCOMP_DEBUG("HURRA!!!! %9ld   identical %3d   similar %3d", existing->id,identical, similar);
			if (identical+similar > bestmatch) {
				bestmatch = identical+similar;
				found = existing;
			}
		}
	}

	return found;
}


bool Autoloc3::feed(Origin *origin)
{
	if ( origin->imported ) {
		// external origin from trusted agency for passive association only
		_store(origin);
		return true;
	}

	// At this point, any origin that was NOT IMPORTED is expected to be MANUAL.

	const Origin *manualOrigin = origin;

	if ( manualOrigin->arrivals.size() == 0 ) {
		SEISCOMP_WARNING("Ignoring manual origin without arrivals");
		return false;
	}

	SEISCOMP_INFO("processing manual origin z=%.3fkm   dtype=%d", manualOrigin->dep, manualOrigin->depthType);

	// Look for a matching (autoloc) origin. Our intention is to find the
	// best-matching origin and merge it with the just received manual
	// origin (adopt picks, fixed focal depth etc.)
	Origin *found = _findMatchingOrigin(manualOrigin);

	if (found) {
		SEISCOMP_DEBUG("found matching origin with id=%ld  z=%.3fkm", found->id, found->dep);

		// update existing origin with information from received origin
		OriginID id = found->id;
		ArrivalVector arrivals;

		for(unsigned int i=0; i<manualOrigin->arrivals.size(); i++) {
			Arrival arr = manualOrigin->arrivals[i];
			if ( ! arr.pick->station())
				continue;
			arrivals.push_back(arr);
		}

		// merge origin
		for(unsigned int i=0; i<found->arrivals.size(); i++) {
			Arrival arr = found->arrivals[i];
			if ( ! arr.pick->station()) {
				// This should actually NEVER happen at this point...
				SEISCOMP_ERROR("Arrival referencing a pick without station information");
				continue;
			}

			// Do we have an arrival for this station already?
			// We have to look for arrivals that either reference the same pick
			// or arrivals for the same station/phase combination. The latter
			// is still risky if two nearby picks of the same onset are assigned
			// different phase codes, e.g. P/Pn or P/PKP; in that case we end up
			// with both picks forming part of the solution.
			bool have=false;
			for(unsigned int k=0; k<arrivals.size(); k++) {
				Arrival _arr = arrivals[k];

				if (_arr.pick == arr.pick) {
					have = true;
					break;
				}

				if (_arr.pick->station() == arr.pick->station() && _arr.phase == arr.phase) {
					have = true;
					break;
				}
			}
			if (have) continue; // skip this arrival

			arrivals.push_back(arr);
		}
		arrivals.sort();

		*found = *manualOrigin;
		found->arrivals = arrivals;
		found->id = id;

		switch (manualOrigin->depthType) {
		case Origin::DepthManuallyFixed:
			_relocator.useFixedDepth(true);
			break;
		case Origin::DepthPhases:
		case Origin::DepthFree:
		default:
			_relocator.useFixedDepth(false);
		}

		// TODO: consider making this relocation optional
		OriginPtr relo = _relocator.relocate(found);
		if (relo) {
			found->updateFrom(relo.get());
			_store(found);
			report();
			cleanup();
		}
		else
			SEISCOMP_WARNING("RELOCATION FAILED @Autoloc3::feed(Origin*) (not critical)");
	}
	else {
		SEISCOMP_DEBUG("no matching origin found");
		// TODO: create a new internal origin
	}

	return true;
}


double Autoloc3::_score(const Origin *origin) const
{
	// compute the score of the origin as if there were no other origins
	double score = originScore(origin);

	// see how many of the picks may be secondary phases of a previous origin
	// TODO
	return score;
}


bool Autoloc3::_log(const Pick *pick)
{
	if (_pickLogFilePrefix != "") {
		Time now = Time(Seiscomp::Core::Time::GMT());
		setPickLogFileName(_pickLogFilePrefix+"."+sc3time(now).toString("%F"));
	}

	if ( ! _pickLogFile.good())
		return false;

	char line[200];
	string loc = pick->loc == "" ? "__" : pick->loc;
	sprintf(line, "%s %-2s %-6s %-3s %-2s %6.1f %10.3f %4.1f %c %s",
	      time2str(pick->time).c_str(),
	      pick->net.c_str(), pick->sta.c_str(), pick->cha.c_str(), loc.c_str(),
	      pick->snr, pick->amp, pick->per, statusFlag(pick),
	      pick->id.c_str());
	_pickLogFile << line << endl;

	SEISCOMP_INFO("%s", line);

	return true;
}


static bool mightBeAssociated(const Pick *pick, const Origin *origin)
{
	// a crude first check
	double dt = pick->time - origin->time;
	if (dt < -10 || dt > 1300)
		return false;
	return true;
}


bool Autoloc3::_tooLowSNR(const Pick *pick) const
{
	if ( ! automatic(pick))
		return false;

	if (pick->snr < _config.minPickSNR)
		return true;

	return false;
}


bool Autoloc3::_tooManyRecentPicks(const Pick *newPick) const
{
	if ( ! automatic(newPick))
		return false;

	double weightedSum = 0, prevThreshold = 0, timeSpan = _config.dynamicPickThresholdInterval;

	if (timeSpan <= 0)
		return false;

	if (newPick->snr <= 0.) {
		SEISCOMP_DEBUG("_tooManyRecentPicks: new pick without snr amplitude: %s -> ignored  (%g)", newPick->id.c_str(), newPick->snr);
		return true;
	}

	for (PickMap::const_iterator
	    it = _pick.begin(); it != _pick.end(); ++it) {

		const Pick *oldPick = (*it).second.get();

		if (oldPick->station() != newPick->station())
			continue;

		if ( !_config.useManualPicks && manual(oldPick) && !_config.useManualOrigins )
			continue;

		double dt = newPick->time - oldPick->time;
		if (dt < 0 || dt > timeSpan)
			continue;

/*		if ( newPick->origin() )  // associated?
			continue;
*/
		double snr = oldPick->snr;
		if (snr > 15)  snr = 15;
		if (snr <  3)  snr =  3;
		weightedSum += snr * (1-dt/timeSpan);


		// not well tested:
		double x = snr * (1-dt/_config.xxlDeadTime);
		if (x>prevThreshold) prevThreshold = x;
	}

	// These criteria mean that if within the time span there
	// were 10 Picks with SNR X
	weightedSum *= 2*0.07; // TODO: Make 0.07 configurable?
	if (newPick->snr < weightedSum) {
		SEISCOMP_DEBUG("_tooManyRecentPicks: %-35s      %.2f < %.2f",
			      newPick->id.c_str(), newPick->snr, weightedSum);
		return true;
	}

	if (newPick->snr < prevThreshold) {
		SEISCOMP_DEBUG("_tooManyRecentPicks: %-35s   XX %.2f < %.2f",
			       newPick->id.c_str(), newPick->snr, prevThreshold);
		return true;
	}

	return false; // OK, pick passes check
}




Origin *Autoloc3::merge(const Origin *origin1, const Origin *origin2)
{
	// The second origin is merged into the first. A new instance
	// is returned that has the ID of the first.
	OriginID id = origin1->id;

	// make origin1 the better origin
	if (_score(origin2) > _score(origin1)) {
//	if (origin2->definingPhaseCount() > origin1->definingPhaseCount()) {
		const Origin *tmp = origin1;
		origin1 = origin2;
		origin2 = tmp;
	}

	Origin *combined = new Origin(*origin1);
	combined->id = id;

	SEISCOMP_DEBUG_S(" MRG1 " + printOneliner(origin1));
	SEISCOMP_DEBUG_S(" MRG2 " + printOneliner(origin2));

	// This is a brute-force merge! Put everything into one origin.
	int arrivalCount2 = origin2->arrivals.size();
	int commonPickCount = 0;
	int commonUsedCount = 0;
	for(int i2=0; i2<arrivalCount2; i2++) {
		Arrival arr = origin2->arrivals[i2];
		// Skip pick if an arrival already references it
		bool found = combined->findArrival(arr.pick.get()) != -1;
		if (found) {
			commonPickCount++;
			if ( ! arr.excluded )
				commonUsedCount++;
			continue;
		}
		arr.excluded = Arrival::TemporarilyExcluded;
		// XXX The phase ID may be wrong.
		combined->add(arr);
		SEISCOMP_DEBUG(" MRG %ld->%ld added %s", origin2->id, origin1->id, arr.pick->id.c_str());
	}

#ifdef LOG_RELOCATOR_CALLS
	SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
	// This was previously missing:
	_relocator.useFixedDepth(false);  // TODO: extensive testing!

	OriginPtr relo = _relocator.relocate(combined);
	if ( ! relo) {
		// Actually we expect the relocation to always succeed,
		// because the temporarily excluded new arrivals should
		// not influence the solution. It does happen, rarely,
		// but is not critical.
		SEISCOMP_WARNING("THIS SHOULD NEVER HAPPEN @merge (not critical)");
		SEISCOMP_WARNING_S("Failed to relocate this one:\n"+printDetailed(combined));
		return combined;
		// The returned origin is the better of the two original
		// origins with the merged arrivals now TemporarilyExcluded.
	}

	combined->updateFrom(relo.get());

	// now see which of the temporarily excluded new arrivals have
	// acceptable residuals
	for (ArrivalVector::iterator
	     it = combined->arrivals.begin(); it != combined->arrivals.end(); ++it) {

		Arrival &arr = *it;

		if ( arr.excluded == Arrival::TemporarilyExcluded)
			arr.excluded =  _residualOK(arr, 1.3, 1.8) ? Arrival::NotExcluded : Arrival::LargeResidual;
	}
		
	_trimResiduals(combined);

	return combined;
}


bool Autoloc3::_followsBiggerPick(const Pick *newPick) const
{
	// Check whether this pick is within a short time after an XXL pick from the same station
	for (PickMap::const_iterator
		it = _pick.begin(); it != _pick.end(); ++it) {

		const Pick *pick = (*it).second.get();

		if (pick == newPick)
			continue;

		if ( ! pick->xxl)
			continue;

		if (pick->station() != newPick->station())
			continue;

		double dt = newPick->time - pick->time;
		if (dt < 0 || dt > _config.xxlDeadTime)
			continue;

		SEISCOMP_INFO_S("process pick IGNORING " + newPick->id + " (following XXL pick" + pick->id + ")");
		return true;
	}	

	return false;
}


bool Autoloc3::_perhapsPdiff(const Pick *pick) const
{
	// This is a very crude test that won't harm. if at all, only a few
	// picks with low SNR following a large event are affected.

	if (pick->snr > 6) // TODO: make this configurable? not very important
		return false;

	bool result = false;

	for (OriginVector::const_iterator
	     it = _origins.begin(); it != _origins.end(); ++it) {

		const Origin *origin = (*it).get();
		const Station *station = pick->station();

		if (pick->time - origin->time >  1000)
			continue;

		if (origin->score < 100) // TODO: make this configurable? not very important
			continue;

		double delta, az, baz;
		delazi(origin, station, delta, az, baz);

		if (delta < 98 || delta > 120)
			continue;

		Seiscomp::TravelTimeTable ttt;
		Seiscomp::TravelTimeList *ttlist = ttt.compute(origin->lat, origin->lon, origin->dep, station->lat, station->lon, 0);
		const Seiscomp::TravelTime *tt;
		if ( (tt = getPhase(ttlist, "Pdiff")) == NULL ) {
			delete ttlist;
			continue;
		}
		delete ttlist;

		double dt = pick->time - (origin->time + tt->time);
		if (dt > 0 && dt < 150) {
			SEISCOMP_DEBUG("Pick %s in Pdiff coda of origin %ld", pick->id.c_str(), origin->id);
			result = true;
		}
	}

	return result;
}


OriginPtr Autoloc3::_xxlPreliminaryOrigin(const Pick *newPick)
{
	if ( ! newPick->xxl)
		return 0; // nothing else to do for this pick

	OriginPtr newOrigin = 0;

	vector<const Pick*> xxlpicks;
	const Pick *earliest = newPick;
	xxlpicks.push_back(newPick);
	for (PickMap::const_iterator
	    it = _pick.begin(); it != _pick.end(); ++it) {

		const Pick *oldPick = (*it).second.get();

		if ( ! oldPick->xxl)
			continue;

		if ( ignored(oldPick))
			continue;
				
		if ( newPick->station() == oldPick->station() )
			continue;

		double dt = newPick->time - oldPick->time;
		double dx = distance(oldPick->station(), newPick->station());

		if (fabs(dt) > 10+13.7*_config.xxlMaxStaDist)
			continue;

		if ( dx > _config.xxlMaxStaDist )
			continue;

		if ( !_config.useManualPicks && manual(oldPick) && ! _config.useManualOrigins )
			continue;

		// make sure we don't have two picks of the same station
		bool duplicate_station = false;
		for (vector<const Pick*>::const_iterator
			it = xxlpicks.begin(); it != xxlpicks.end(); ++it) {
			if ((*it)->station() == oldPick->station()) {
				duplicate_station = true;
				break;
			}
		}
		if ( duplicate_station )
			continue;

		xxlpicks.push_back(oldPick);

		if ( oldPick->time < earliest->time )
			earliest = oldPick;
	}

	SEISCOMP_DEBUG("Number of XXL picks=%ld", (long int)xxlpicks.size());
	if (xxlpicks.size() < _config.xxlMinPhaseCount)
		return NULL;

	double lat = earliest->station()->lat+0.03;
	double lon = earliest->station()->lon+0.03;
	double tim = earliest->time-0.05;
	double dep;

	// loop over several trial depths, which are multiples of the default depth
	vector<double> trialDepths;
	for (int i=0; dep <= _config.xxlMaxDepth; i++) {
		dep = _config.defaultDepth*(1+i);
		trialDepths.push_back(dep);

		// in case of "sticky" default depth, we don't need any more trial depths
		if (_config.defaultDepthStickiness > 0.9)
			break;
	}
	
	for (unsigned int i=0; i<trialDepths.size(); i++) {
		dep = trialDepths[i];
		OriginPtr origin = new Origin(lat, lon, dep, tim);

		for(vector<const Pick*>::iterator it=xxlpicks.begin(); it!=xxlpicks.end(); ++it) {
			const Pick *pick = *it;
			double delta, az, baz;
			Arrival arr(pick);
			delazi(origin.get(), arr.pick->station(), delta, az, baz);
                	arr.distance = delta;
                	arr.azimuth = az;
			arr.excluded = Arrival::NotExcluded;
			origin->arrivals.push_back(arr);
		}
		_relocator.setFixedDepth(dep);
		_relocator.useFixedDepth(true);
		SEISCOMP_DEBUG("Trying to relocate possible XXL origin; trial depth %g km", dep);
		SEISCOMP_DEBUG_S(printDetailed(origin.get()));
#ifdef LOG_RELOCATOR_CALLS
		SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
		OriginPtr relo = _relocator.relocate(origin.get());
		if ( ! relo) {
			SEISCOMP_DEBUG("FAILED to relocate possible XXL origin");
			continue; // to next fixed depth
		}
		SEISCOMP_DEBUG_S("XXL " + printOneliner(relo.get()));

		bool ignore = false;
		int arrivalCount = relo->arrivals.size();
		for (int i=0; i<arrivalCount; i++) {

			Arrival &arr = relo->arrivals[i];
			if (arr.distance > _config.xxlMaxStaDist)
				ignore = true;
		}
		if (relo->rms() > _config.maxRMS)
			ignore = true;
		if (ignore)
			continue;

		SEISCOMP_INFO("RELOCATED XXL ALERT");
		origin->updateFrom(relo.get());
		origin->preliminary = true;
		origin->depthType = _config.defaultDepthStickiness > 0.9
			? Origin::DepthDefault
			: Origin::DepthManuallyFixed;
		SEISCOMP_INFO_S(printOneliner(origin.get()));

		// TODO: The _depthIsResolvable part needs review and could probably be cleaned up a bit...
		if (_config.defaultDepthStickiness < 0.9 && _depthIsResolvable(origin.get())) {
			_relocator.useFixedDepth(false);
#ifdef LOG_RELOCATOR_CALLS
		SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
			relo = _relocator.relocate(origin.get());
			if (relo)
				origin->updateFrom(relo.get());
		}

		newOrigin = origin;
		break;
	}

	if (newOrigin) {
		newOrigin->id = _newOriginID();
		newOrigin->arrivals.sort();
		return newOrigin;
	}

	return NULL;
}


OriginID Autoloc3::_newOriginID()
{
	static OriginID id = 0;
	return ++id;
}


OriginPtr Autoloc3::_tryAssociate(const Pick *pick)
{
	//
	// Try to associate the pick with existing, qualified origins.
	// Currently it is assumed that the Pick is a P phase.
	//
	double associatedOriginLargestScore = 0;

	OriginPtr origin = 0;
	if ( ! _associator.feed(pick) )
		return 0;

	const AssociationVector &associations = _associator.associations();

	// logging only
	if (associations.size() > 0)
		SEISCOMP_INFO("resulting in %ld associations", (long int)associations.size());

	// logging all associations
	for (AssociationVector::const_iterator
	     it=associations.begin(); it!=associations.end(); ++it) {

		const Association &asso = (*it);
		SEISCOMP_INFO_S("     " + printOneliner(asso.origin.get()) + "  ph="+asso.phase);
		SEISCOMP_INFO  ("     aff=%.2f res=%.2f", asso.affinity, asso.residual);
	}

	// 
	// loop through the associations
	//

	// first look for imported origins
	for (AssociationVector::const_iterator
	     it = associations.begin(); it != associations.end(); ++it) {

		const Association &asso = (*it);
		if ( ! (asso.origin->imported))
			continue;
		OriginPtr associatedOrigin = new Origin(*asso.origin.get());

		bool success = _associate(associatedOrigin.get(), pick, asso.phase);
		if ( ! success)
			continue;
		int index = associatedOrigin->findArrival(pick);
		if (index==-1) {
			SEISCOMP_ERROR("THIS SHOULD NEVER HAPPEN @_tryAssociate");
			return NULL;
		}
		Arrival &arr = associatedOrigin->arrivals[index];
		SEISCOMP_INFO("IMP associated pick %s to origin %ld   phase=%s aff=%.4f dist=%.1f wt=%d",
			  pick->id.c_str(), associatedOrigin->id,
			  arr.phase.c_str(), arr.affinity, arr.distance, arr.excluded?0:1);
		origin = associatedOrigin;
	}

	// If at this point we already have found an associated origin, which
	// must be an imported origin, we return it and don't try any further.
	if (origin) return origin;


	// If no imported origin was found, search for own origins.
	for (AssociationVector::const_iterator
	     it = associations.begin(); it != associations.end(); ++it) {

		const Association &asso = (*it);
		OriginPtr associatedOrigin = new Origin(*asso.origin.get());

		// this is the main criteria
		if (asso.affinity < _config.minPickAffinity)
			continue; 

		// do not relocate imported origins, only associate picks
		if (associatedOrigin->imported)
			break;

		if (asso.phase == "P" || asso.phase == "PKP") {
			SEISCOMP_DEBUG_S(" *** " + pick->id);
			SEISCOMP_DEBUG_S(" *** " + printOneliner(associatedOrigin.get())+"  ph="+asso.phase);
			bool success = _associate(associatedOrigin.get(), pick, asso.phase);
			string oneliner = printOneliner(associatedOrigin.get())+"  ph="+asso.phase;

			if (success) {
				SEISCOMP_DEBUG_S(" +++ " + oneliner);
			}
			else {
				SEISCOMP_DEBUG_S(" --- " + oneliner);
				continue;
			}
		}
		else {
			Arrival arr = asso;
			arr.excluded = Arrival::UnusedPhase;
			associatedOrigin->add(arr);
		}

		{ // logging only
		int index = associatedOrigin->findArrival(pick);
		if (index==-1) {
			SEISCOMP_ERROR("THIS SHOULD NEVER HAPPEN @_tryAssociate B");
			return NULL;
		}
		Arrival &arr = associatedOrigin->arrivals[index];
		SEISCOMP_INFO("associated pick %s to origin %ld   phase=%s aff=%.4f dist=%.1f wt=%d",
			  pick->id.c_str(), associatedOrigin->id, arr.phase.c_str(), asso.affinity, arr.distance, arr.excluded?0:1);
		}

		if ( ! _passedFilter(associatedOrigin.get()))
			continue;

		int phaseCount = associatedOrigin->definingPhaseCount();
		if (phaseCount > associatedOriginLargestScore) {
			associatedOriginLargestScore = phaseCount;
			origin = associatedOrigin;
		}
	}

	if (origin)
		return origin;

	return 0;
}


OriginPtr Autoloc3::_tryNucleate(const Pick *pick)
{
	if ( ! _nucleator.feed(pick))
		return NULL;

	//
	// The following will only be executed if the nucleation of a new
	// origin succeeded.
	// 
	// Examine the candidate origins suggested by the nucleator one-by-one
	// The aim is to find an acceptable new origin.
	//
	OriginPtr newOrigin = 0;
	OriginVector candidates = _nucleator.newOrigins();

	SEISCOMP_DEBUG("Autoloc3::_tryNucleate A  candidate origins: %d", int(candidates.size()));

	double bestScore = 0;
	for (OriginVector::iterator
	     it = candidates.begin(); it != candidates.end(); ++it) {

		Origin *candidate = (*it).get();

		// We are in a dilemma here: We may have a new origin with a
		// bad RMS due to a single outlier or simply bad picks (like
		// for emergent regional Pn). So the origin may actually be
		// resonably good, but the RMS is bad. So. for the very first
		// origin, we allow a somewhat larger RMS. Though "somewhat"
		// has yet to be quantified.

		if (candidate->rms() > 3*_config.maxRMS)
			continue;

		if ( ! newOrigin)
			newOrigin = candidate;
		else {
			double score = _score(candidate);
			if (score>bestScore) {
				bestScore = score;
				newOrigin = candidate;
			}
		}
//		break; // XXX HACKISH XXX

		//
		// We thus only get ONE origin out of the Nucleator!
		// This is *usually* OK, but we might want to try more.
		// TODO: testing needed
		//
	}

	if ( ! newOrigin)
		return NULL;

	newOrigin->id = _newOriginID();
	newOrigin->arrivals.sort();

	// Try to find the best Origin which might belong to same event
	// TODO avoid the ugly cast...
	Origin *bestEquivalentOrigin = const_cast<Origin*>(_origins.bestEquivalentOrigin(newOrigin.get()));
	
	if ( bestEquivalentOrigin != NULL ) {
 		double rms = bestEquivalentOrigin->rms(), score = _score(bestEquivalentOrigin);

		OriginPtr temp = merge(bestEquivalentOrigin, newOrigin.get());

		double epsilon = 1.E-07;
		if (fabs(temp->rms()-rms)/rms < epsilon &&
		    fabs(_score(temp.get())-score)/score < epsilon) {

			SEISCOMP_DEBUG_S(" MRG " + printOneliner(temp.get()) + " UNCHANGED");
		}
		else {
			SEISCOMP_DEBUG_S(" MRG " + printOneliner(temp.get()));
			bestEquivalentOrigin->updateFrom(temp.get());
			if ( _passedFilter(bestEquivalentOrigin) )
				return bestEquivalentOrigin; // FIXME memory leak!!!
		}
	}
	else {  // This is a new origin fresh from the nucleator.

		if ( _passedFilter(newOrigin.get()) )
			return newOrigin; // FIXME memory leak!!!
	}

//	delete newOrigin; // XXX XXX XXX Warum knallt das?
	return NULL;
}


static int countCommonPicks(const Origin *origin1, const Origin *origin2)
{
	int commonPickCount = 0;
	int arrivalCount1 = origin1->arrivals.size();
	int arrivalCount2 = origin2->arrivals.size();

	for(int i1=0; i1 < arrivalCount1; i1++) {
		for(int i2=0; i2<arrivalCount2; i2++)
			if (origin1->arrivals[i1].pick == origin2->arrivals[i2].pick)
				commonPickCount++;
	}

	return commonPickCount;
}


Origin *Autoloc3::_findEquivalent(const Origin *origin)
{
	Origin *result = 0;

	for (OriginVector::iterator
	     it = _origins.begin(); it != _origins.end(); ++it) {

		Origin *other = (*it).get();

		int count = countCommonPicks(origin, other);
		if (count >= 3) {
			if (result) {
				if (other->score > result->score)
					result = other;
			}
			else	result = other;
		}
	}

	return result;
}


bool Autoloc3::_process(const Pick *pick)
{
	// process a pick

	if ( ! valid(pick) ) {
		SEISCOMP_DEBUG("invalid pick %-35s", pick->id.c_str());
		return false;
	}

	if ( automatic(pick) && _tooLowSNR(pick) )
		return false;

	// A pick is tagged as XXL pick if it exceeds BOTH the configured XXL
	// minimum amplitude and XXL minimum SNR threshold.
	if ( _config.xxlEnabled && pick->amp >= _config.xxlMinAmplitude && pick->snr > _config.xxlMinSNR ) {
		const_cast<Pick*>(pick)->xxl = true;
	}

	double normalizationAmplitude = 2000.; // arbitrary choice: TODO: review, perhaps make configurable
	if ( _config.xxlEnabled )
		normalizationAmplitude = _config.xxlMinAmplitude;
	const_cast<Pick*>(pick)->normamp = pick->amp/normalizationAmplitude;

	if ( automatic(pick) && _tooManyRecentPicks(pick) ) {
		const_cast<Pick*>(pick)->status = Pick::IgnoredAutomatic;
		return false;
	}


	_log(pick);

	if ( _blacklisted(pick) ) {
		SEISCOMP_INFO("process pick %-35s %c blacklisted -> ignored",
			       pick->id.c_str(), statusFlag(pick));
		return false;
	}

	if ( manual(pick) ) {

		if ( ! _config.useManualPicks) {
			if ( _config.useManualOrigins ) {
				// If we want to consider only associated manual picks,
				// i.e. picks that come along with a manual origin that
				// uses them, we stop here because we don't want to feed
				// it into the associator/nucleator.
				return true;
			}
			else {
				_setBlacklisted(pick);
				return false;
			}
		}
	}

	SEISCOMP_INFO("process pick %-35s %s", pick->id.c_str(), (pick->xxl ? " XXL":""));

	if ( _followsBiggerPick(pick) )
		return false;

	if ( _perhapsPdiff(pick) )
		return false;



	// try to associate this pick to some existing origin

	OriginPtr origin;
	origin = _tryAssociate(pick);
	if ( origin ) {
		// if we associated the pick with an imported origin, we can stop here
		if ( origin->imported ) {
			_store(origin.get());
			return true;
		}

		_rework(origin.get());
		if ( _passedFilter(origin.get()) ) {
			_store(origin.get());
		}
		else
			origin = NULL;
	}

	if ( origin && origin->score >= _config.minScoreBypassNucleator )
		return true;  // bypass the nucleator

	// The following will only be executed if the association with an
	// existing origin failed or if the score of the best associated
	// origin s too small.
	//
	// In that case, feed the new pick to the nucleator.
	// The result may be several candidate origins; in a loop we examine
	// each of them until the result is satisfactory.

	if ( origin ) {
		// Feed the pick to the Nucleator but ignore result
		// TODO: Review!
		// _tryNucleate(pick);
		return true;
	}

	origin = _tryNucleate(pick);
	if ( origin ) {
		_rework(origin.get());
		if ( _passedFilter(origin.get()) ) {
			_store(origin.get());
			return true;
		}
	}

	// If up to now we haven't successfully procesed the new pick,
	// finally try the XXL hack (if enabled).

	if (_config.xxlEnabled) {

		OriginPtr origin = _xxlPreliminaryOrigin(pick);
		if ( origin ) {
			OriginPtr equivalent = _findEquivalent(origin.get());
			if (equivalent) {
				equivalent->updateFrom(origin.get());
				origin = equivalent;
			}

			_rework(origin.get());
			if ( _passedFilter(origin.get()) ) {
				_store(origin.get());
				return true;
			}
		}
	}

	return false;
}


static int depthPhaseCount(Origin *origin)
{
	int arrivalCount = origin->arrivals.size();
	int _depthPhaseCount = 0;
	for (int i=0;i<arrivalCount;i++) {
		Arrival &arr = origin->arrivals[i];
		if ( arr.excluded )
			continue;
		if ( arr.phase == "pP" || arr.phase == "sP" )
			_depthPhaseCount++;
	}
	return _depthPhaseCount;
}


bool Autoloc3::_setDefaultDepth(Origin *origin)
// Set origin depth to the configured default depth and relocate.
// May be set in an origin far outside the network where depth resolution is expected to be poor,
// or in testing that depth resolution.
{
	OriginPtr test = new Origin(*origin);

	_relocator.setFixedDepth(_config.defaultDepth);
	_relocator.useFixedDepth(true);
#ifdef LOG_RELOCATOR_CALLS
	SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
	OriginPtr relo = _relocator.relocate(test.get());
	if ( ! relo) {
		SEISCOMP_WARNING("_setDefaultDepth: failed relocation");
		return false;
	}

	origin->updateFrom(relo.get());
	origin->depthType = Origin::DepthDefault;

	return true;
}

bool Autoloc3::_setTheRightDepth(Origin *origin)
{
	if ( ! _config.tryDefaultDepth)
		return false;

	if (origin->depthType == Origin::DepthPhases)
		return false;


	// dann aber auch mal testen, ob man mit freier Tiefe evtl. weiter kommt.
	// Sonst bleibt das immer bei der Default-Tiefe haengen!
	if (origin->depthType == Origin::DepthDefault) {
		// XXX BAUSTELLE XXX
		OriginPtr test = new Origin(*origin);
		test->depthType = Origin::DepthFree;

		_relocator.useFixedDepth(false);
		OriginPtr relo = _relocator.relocate(test.get());
		if ( ! relo) {
			SEISCOMP_WARNING("_setDefaultDepth: failed relocation");
			return false;
		}

		double radius = 5*(relo->dep >= _config.defaultDepth ? relo->dep : _config.defaultDepth)/111.2;
		
		// XXX This is a hack, but better than nothing:
		// if there are at least 2 stations within 5 times the source depth, we assume sufficient depth resolution.
		if (relo->definingPhaseCount(0, radius) >= 2) {
			origin->updateFrom(relo.get());
			return false;
		}

		return true;
		// XXX BAUSTELLE XXX
	}

	// XXX This is a hack, but better than nothing:
	// if there are at least 2 stations within 5 times the source depth, we assume sufficient depth resolution.
	if (origin->definingPhaseCount(0, (5*origin->dep)/111.2) >= 2)
		return false;

	OriginPtr test = new Origin(*origin);
	if ( ! _setDefaultDepth(test.get()))
		return false; // relocation using default depth failed

	// test origin now has the default depth (fixed)


	// regarding the default depth "stickiness", we currently distinguish three cases:
	// stickiness >= 0.9: force use of default depth; might make a deep origin unrelocatable!
	// 0.1 < stickiness < 0.9: try default depth vs. free depth
	// stickiness <= 0.1 never use default depth - TODO

	if (_config.defaultDepthStickiness < 0.9) {
		// only then we need to try another depth

		double rms1 = origin->rms();      // current rms
		double rms2 = test->rms();        // rms with z=default

		// if setting z=default increases the rms "significantly"...
		if ( rms2 > 1.2*rms1 && rms2 > _config.goodRMS ) {
#ifdef EXTRA_DEBUGGING
			SEISCOMP_DEBUG("_testDepthResolution good for origin %ld (rms criterion)", origin->id);
#endif
			return false;
		}


		double score1 = _score(origin);      // current score
		double score2 = _score(test.get());  // score with z=default

		// if setting z=default decreases the score "significantly"...
		if ( score2 < 0.9*score1-5 ) {
#ifdef EXTRA_DEBUGGING
			SEISCOMP_DEBUG("_testDepthResolution good for origin %ld (score criterion)", origin->id);
#endif
			return false;
		}

		if (origin->dep != test->dep)
			SEISCOMP_INFO("Origin %ld: changed depth from %.1f to default of %.1f   score: %.1f -> %.1f   rms: %.1f -> %.1f", origin->id, origin->dep, test->dep, score1, score2, rms1, rms2);
	}

	origin->updateFrom(test.get());
	origin->depthType = Origin::DepthDefault;
	_updateScore(origin); // why here?

	return true;
}


bool Autoloc3::_epicenterRequiresDefaultDepth(const Origin *origin) const
{
	// TODO ;)
	return false;
}


void Autoloc3::_ensureConsistentArrivals(Origin *origin)
{
	// ensure consistent distances/azimuths of the arrivals
	int arrivalCount = origin->arrivals.size();
	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = origin->arrivals[i];
		double delta, az, baz;
		delazi(origin, arr.pick->station(), delta, az, baz);
		arr.distance = delta;
		arr.azimuth = az;
	}

	origin->arrivals.sort();
}


void Autoloc3::_ensureAcceptableRMS(Origin *origin, bool keepDepth)
{
	int minPhaseCount = 20; // TODO: make this configurable

	if (origin->definingPhaseCount() < minPhaseCount)
		return;

	if (origin->rms() <= _config.maxRMS)
		return;

	SEISCOMP_DEBUG("_ensureAcceptableRMS rms loop begin");
	
	while (origin->rms() > _config.maxRMS) {
		SEISCOMP_DEBUG("_ensureAcceptableRMS rms loop %.2f > %.2f", origin->rms(), 0.9*_config.maxRMS);

		int definingPhaseCount = origin->definingPhaseCount();

		if (definingPhaseCount < minPhaseCount)
			break;

		if (definingPhaseCount < 50) { // TODO: make this configurable
			// instead of giving up, try to enhance origin
			// This is rather costly, so we do it only up
			// to 50 defining picks, as then usually the
			// solution is so consolidated that switching
			// to removal of the pick with largest residual
			// is a safe bet.
			if ( ! _enhanceScore(origin, 1))
				break;
		}
		else {
			int worst = arrivalWithLargestResidual(origin);
			origin->arrivals[worst].excluded = Arrival::LargeResidual;
			_relocator.useFixedDepth(keepDepth ? true : false);
#ifdef LOG_RELOCATOR_CALLS
			SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
			OriginPtr relo = _relocator.relocate(origin);
			if ( ! relo) {
				SEISCOMP_WARNING("Relocation failed in _ensureAcceptableRMS for origin %ld", origin->id);
				break;
			}
			origin->updateFrom(relo.get());
		}

	}
	SEISCOMP_DEBUG("_ensureAcceptableRMS rms loop end");
}


void Autoloc3::_updateScore(Origin *origin)
{
	origin->score = _score(origin);
}


bool Autoloc3::_rework(Origin *origin)
{
#ifdef EXTRA_DEBUGGING
	SEISCOMP_DEBUG("_rework begin   deperr=%.1f", origin->deperr);
#endif
	// This is the minimum requirement
	if (origin->definingPhaseCount() < _config.minPhaseCount) {
		return false;
	}

	// There are several possible conditions that may require use of
	// the default depth for this origin. Check if any of these is met.
	bool enforceDefaultDepth = false;
	bool adoptManualDepth = false;

	// TODO: put all this depth related stuff into _setTheRightDepth()
	if (_config.adoptManualDepth && (
			origin->depthType == Origin::DepthManuallyFixed ||
			origin->depthType == Origin::DepthPhases ) ) {
			SEISCOMP_INFO("Adopting depth of %g km from manual origin", origin->dep);
			adoptManualDepth = true;
	}
	else { 
		if ( _config.defaultDepthStickiness >= 0.9 ) {
			enforceDefaultDepth = true;
			SEISCOMP_INFO("Enforcing default depth due to stickiness");
		}
		else if (_epicenterRequiresDefaultDepth(origin) && _setDefaultDepth(origin) ) {
			enforceDefaultDepth = true;
			SEISCOMP_INFO("Enforcing default depth due to epicenter location");
		}
		else if ( _setTheRightDepth(origin) ) {
			enforceDefaultDepth = true;
			SEISCOMP_INFO("Enforcing default depth due to epicenter-station geometry");
		}
		else
			SEISCOMP_INFO("Not fixing depth");
	}

	// The _enhance_score() call is slow for origins with many phases, while
	// the improvement becomes less. So at some point, we don't want to
	// call _enhance_score() too often or not at all.
	if (origin->definingPhaseCount() < 30) // TODO: make this configurable
		_enhanceScore(origin);

	if (enforceDefaultDepth)
		_relocator.setFixedDepth(_config.defaultDepth);

	bool keepDepth = adoptManualDepth || enforceDefaultDepth;

	_relocator.useFixedDepth(keepDepth ? true : false);
	_trimResiduals(origin);  // calls _relocator

	// only use nearest stations
	// TODO: need to review this...
	while (origin->definingPhaseCount(0, _config.maxStaDist) > _config.minPhaseCount) {
		int arrivalCount = origin->arrivals.size();
		double dmax=0;
		int imax=-1;
		// find the farthest used station
		for (int i=0; i<arrivalCount; i++) {

			Arrival &arr = origin->arrivals[i];
			if (arr.excluded)
				continue;
			if (arr.distance > dmax) {
				dmax = arr.distance;
				imax = i;
			}
		}

		Arrival &arr = origin->arrivals[imax];
		if (arr.distance < _config.maxStaDist)
			break;
		arr.excluded = Arrival::StationDistance;

		// relocate once
#ifdef LOG_RELOCATOR_CALLS
		SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
		OriginPtr relo = _relocator.relocate(origin);
		if ( ! relo) {
			SEISCOMP_WARNING("A relocation failed in _rework for origin %ld", origin->id);
			break;
		}

		origin->updateFrom(relo.get());
	}

	_ensureAcceptableRMS(origin, keepDepth);
	_addMorePicks(origin, keepDepth);


	_trimResiduals(origin); // again!
	_removeWorstOutliers(origin);
	_excludeDistantStations(origin);
	_excludePKP(origin);

	if (origin->dep != _config.defaultDepth && origin->depthType == Origin::DepthDefault)
		origin->depthType = Origin::DepthFree;

	// once more (see also above)
	if (origin->definingPhaseCount() < _config.minPhaseCount) {
		return false;
	}
#ifdef EXTRA_DEBUGGING
	SEISCOMP_DEBUG("_rework end");
#endif
	return true;
}


bool Autoloc3::_excludePKP(Origin *origin)
{
	if (origin->definingPhaseCount(0,105.) < _config.minStaCountIgnorePKP)
		// no need to do anything
		return false;

	bool relocate = false;
	int arrivalCount = origin->arrivals.size();
	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = origin->arrivals[i];
		if (arr.excluded)       continue;
		if (arr.distance  < 105)   continue;
		// TODO: how about PKiKP?
		if (arr.phase == "P" || arr.phase == "PKP" /* || arr.phase == "PKiKP" */ ) {
			// for times > 960, we expect P to be PKP
			if (arr.pick->time - origin->time > 960) {
				arr.excluded = Arrival::UnusedPhase;
				relocate = true;
			}
		}
	}

	if ( ! relocate)
		return false;

	// relocate once
#ifdef LOG_RELOCATOR_CALLS
	SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
	OriginPtr relo = _relocator.relocate(origin);
	if ( ! relo) {
		SEISCOMP_WARNING("A relocation failed in _excludePKP for origin %ld", origin->id);
		return false;
	}

	origin->updateFrom(relo.get());

	return true;
}

bool Autoloc3::_excludeDistantStations(Origin *origin)
{
	double q = 4;
	vector<double> distance;

	int arrivalCount = origin->arrivals.size();
	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = origin->arrivals[i];

		// ignore excluded arrivals except those that were previously
		// excluded because of the distance criterion, because the
		// latter may no longer hold (i.e. more distant stations)
		if (arr.excluded && arr.excluded != Arrival::StationDistance)
			continue;
		// ignore PKP, *may* be a bit risky -> checks required!
		if (arr.distance > 110)
			continue;
		distance.push_back(arr.distance);
	}
	int distanceCount = distance.size();
	if (distanceCount < 4)
		return false;

	sort(distance.begin(), distance.end());

	int nx = 0.1*distanceCount > 2 ? int(0.1*distanceCount) : 2;
//	double medDistance=Seiscomp::Math::Statistics::median(distance);
	double maxDistance=distance[distanceCount-nx];

	for (int i=distanceCount-nx+1; i<distanceCount; i++) {
		if(distance[i] > q*maxDistance)
			break;
		maxDistance = distance[i];
	}

	int excludedCount = 0;
	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = origin->arrivals[i];
		if (arr.excluded) continue;
		if (arr.distance > maxDistance) {
			arr.excluded = Arrival::StationDistance;
			excludedCount++;
			SEISCOMP_DEBUG("_excludeDistantStations origin %ld exc %s", origin->id, arr.pick->id.c_str());
		}
	}
	if (excludedCount) {
#ifdef LOG_RELOCATOR_CALLS
		SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
		OriginPtr relo = _relocator.relocate(origin);
		if (relo) {
			origin->updateFrom(relo.get());
			return true;
		}
	}

	return false;
}

bool Autoloc3::_passedFinalCheck(const Origin *origin)
{
// Do not execute the check here. It may result in missing origins which are
// correct after relocation, move the check to: Autoloc3::_publishable
//	if (origin->dep > _config.maxDepth) {
//		SEISCOMP_DEBUG("Ignore origin %ld: depth %.3f km > maxDepth %.3f km",
//		               origin->id, origin->dep, _config.maxDepth);
//		return false;
//	}

	if ( ! origin->preliminary &&
	     origin->definingPhaseCount() < _config.minPhaseCount)
		return false;

	return true;
}

bool Autoloc3::_passedFilter(Origin *origin)
{
	if (_config.offline || _config.test) {
		SEISCOMP_DEBUG_S(" TRY " + printOneliner(origin));
		SEISCOMP_DEBUG_S(printDetailed(origin));
	}

/*
	//////////////////////////////////////////////////////////////////
	// new distance vs. min. pick count criterion
	int arrivalCount = origin->arrivals.size();
	int phaseCount = origin->definingPhaseCount();
	int consistentPhaseCount = 0;
	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = origin->arrivals[i];
		if (arr.excluded)
			continue;
		if (arr.phase != "P" && arr.phase != "PKP")
			continue;

		// compute min. phase count of origin for this pick to be consistent with that origin
		int minPhaseCount = _config.minPhaseCount + (arr.distance-arr.pick->station()->maxNucDist)*_config.distSlope;

		SEISCOMP_DEBUG(" AAA origin=%d pick=%s  %d  %d", origin->id, arr.pick->id.c_str(), phaseCount, minPhaseCount);
		if (phaseCount < minPhaseCount) {
//			if (_config.offline || _config.test)
				SEISCOMP_DEBUG(" XXX inconsistent origin=%d pick=%s", origin->id, arr.pick->id.c_str());
			continue;
		}

		consistentPhaseCount++;
	}
	if (consistentPhaseCount < _config.minPhaseCount) {
//		if (_config.offline || _config.test)
			SEISCOMP_DEBUG_S(" XXX " + printOneliner(origin));
		return false;
	}
	//////////////////////////////////////////////////////////////////
*/

	double fakeProbability = _testFake(origin);
	if (fakeProbability > _config.maxAllowedFakeProbability) {
		SEISCOMP_DEBUG_S(printDetailed(origin));
		SEISCOMP_DEBUG("Probable fake origin: %ld - prob=%.3f", origin->id, fakeProbability);
		return false;
	}

	if ( ! _passedFinalCheck(origin))
		return false;

	_ensureConsistentArrivals(origin);

	return true;
}

bool Autoloc3::_publishable(const Origin *origin) const
{
	if (origin->quality.aziGapSecondary > _config.maxAziGapSecondary) {
		SEISCOMP_INFO("Origin %ld not sent (too large SGAP of %3.0f > %3.0f)",
			      origin->id, origin->quality.aziGapSecondary, _config.maxAziGapSecondary);
		return false;
	}

	if (origin->score < _config.minScore) {
		SEISCOMP_INFO("Origin %ld not sent (too low score of %.1f < %.1f)",
			      origin->id, origin->score, _config.minScore);
		return false;
	}

	if (origin->rms() > _config.maxRMS) {
		SEISCOMP_INFO("Origin %ld not sent (too large RMS of %.1f > %.1f)",
			      origin->id, origin->rms(), _config.maxRMS);
		return false;
	}


	if (origin->dep > _config.maxDepth) {
		SEISCOMP_INFO("Origin %ld too deep: %.1f km > %.1f km (maxDepth)",
			      origin->id, origin->dep, _config.maxDepth);
		return false;
	}

	return true;
}

bool Autoloc3::_store(Origin *origin)
{
	_rename_P_PKP(origin);

	if ( origin->imported ) { // FIXME
		SEISCOMP_INFO_S(" IMP " + printOneliner(origin));
		// TODO: Grab all matching picks for this origin
		_addMorePicks(origin);
	}
	else _updateScore(origin);


	if (depthPhaseCount(origin))
		origin->depthType = Origin::DepthPhases;

	Origin *existing = _origins.find(origin->id);
	if (existing) {
		existing->updateFrom(origin);
		origin = existing;
		SEISCOMP_INFO_S(" UPD " + printOneliner(origin));
	}
	else {
		SEISCOMP_INFO_S(" NEW " + printOneliner(origin));
		_origins.push_back(origin);
	}

	if (_config.offline || _config.test)  // this won't harm
		SEISCOMP_DEBUG_S(printDetailed(origin));

	if ( ! origin->imported  &&  origin->definingPhaseCount() >= _config.minPhaseCount)
		origin->preliminary = false;

	if (origin->depthType == Origin::DepthDefault &&
	    origin->dep       != _config.defaultDepth)
		origin->depthType = Origin::DepthFree;

	if ( ! _newOrigins.find(origin))
		_newOrigins.push_back(origin);

	return true;
}

// TODO:
// Einfach an _associate einen Arrival uebergeben, denn der hat ja alles schon an Bord.
bool Autoloc3::_associate(Origin *origin, const Pick *pick, const string &phase)
{
	// first crude check
	if ( ! mightBeAssociated(pick, origin))
		return false; // warning?

	// PKP pick is always > 1000 after O.T.
	if (phase=="PKP" && pick->time-origin->time < 1000)
		return false;

	const Station* station = pick->station();
	int index = origin->findArrival(pick);
	if (index != -1)
		return false; // pick already present -> warning?

	double delta, az, baz;
	delazi(origin, station, delta, az, baz);
	TravelTime tt;

	if (phase=="P" || phase == "PKP") {
		if ( !travelTimeP(origin->lat, origin->lon, origin->dep, station->lat, station->lon, 0, delta, tt))
			return false;
	}
	else {
		SEISCOMP_WARNING_S("_associate got " + phase + " phase - ignored");
		return false;
	}

	double residual = pick->time - origin->time - tt.time;
	Arrival arr(pick, phase, residual);
	if ( ! _residualOK(arr, 0.9, 1.3))
		return false;

	// FIXME: But if it is a very early origin, the location may not be
	//        very good (esp. depth!) and the residual rather meaningless
	//        as criterion.

	// XXX THIS IS A NEW AND EXPERIMENTAL CRITERION XXX
	int minPhaseCount = _config.minPhaseCount + (delta - station->maxNucDist)*_config.distSlope;
	if (origin->phaseCount() < minPhaseCount) {
		if (phase == "PKP" && _config.aggressivePKP) {
			// This is a common case, e.g. small Fiji-Tonga events
			// not picked at many stations at P range, but
			// recorded at many stations in Europe. In that case
			// the minPhaseCount criterion might be counter
			// productive, prevent the association as PKP and
			// ultimately result in fake events.
#ifdef EXTRA_DEBUGGING
			SEISCOMP_DEBUG_S("aggressive PKP association for station "+station->code);
#endif
		}
		else {
#ifdef EXTRA_DEBUGGING
			SEISCOMP_DEBUG("_associate origin=%d pick=%s weakly associated because origin.phaseCount=%d < minPhaseCount=%d", int(origin->id), pick->id.c_str(), origin->phaseCount(), minPhaseCount);
#endif

//			return false;
			arr.excluded = Arrival::TemporarilyExcluded;
		}
	}

	// passive association to imported origin
	if (origin->imported)
		arr.excluded = Arrival::UnusedPhase;

	OriginPtr copy = new Origin(*origin);
	double original_score = _score(copy.get());
	double original_rms   = copy->rms();

	arr.distance = delta;
	arr.azimuth  = az;

	// PKP phases are only used if absolutely needed
	if (arr.phase == "P" || arr.phase == "PKP") {
		if (delta > 105 && copy->definingPhaseCount(0,105.) > _config.minStaCountIgnorePKP)
			arr.excluded = Arrival::UnusedPhase;
		else if (delta > 105 && delta < 125) {
			// FIXME: This may actually be avoided by using separate P and PKP tables
			SEISCOMP_INFO("origin %ld: excluding pick %s because 105<delta<125", copy->id, pick->id.c_str());
			arr.excluded = Arrival::UnusedPhase;
		}
	}
	else {
		arr.excluded = Arrival::UnusedPhase;
	}

	copy->add(arr); // <<== the actual association


	if ( ! origin->imported ) {
		OriginPtr relo = 0;
		if ( arr.excluded != Arrival::UnusedPhase) {

//			double score2 = _score(copy.get()); // score before relocation
//			double rms2   = copy->rms();  // rms before relocation

			// Relocate and test if score improves, otherwise
			// leave pick only loosely associated.
			// TODO: Check for RMS improvement as well.

			// relocate once with fixed depth and if case of failure use free depth
			bool fixed = false;
			if (_config.defaultDepthStickiness > 0.9) {
				fixed = true;
				_relocator.setFixedDepth(_config.defaultDepth);
			}

//			else if (origin->depthType == Origin::DepthManuallyFixed || origin->depthType == Origin::DepthPhases) {
			else if (origin->depthType == Origin::DepthManuallyFixed) {
				fixed = true;
				_relocator.setFixedDepth(origin->dep);
			}
			_relocator.useFixedDepth(fixed);
#ifdef LOG_RELOCATOR_CALLS
			SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
			relo = _relocator.relocate(copy.get());
			if ( ! relo ) {
				if ( fixed )
					return false;
				else {
					_relocator.setFixedDepth(origin->dep);
					_relocator.useFixedDepth(true);
#ifdef LOG_RELOCATOR_CALLS
				SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
					relo = _relocator.relocate(copy.get());
					if ( ! relo) // if 2nd relocation attempt also fails
						return false;
				}
			}

			double score2 = _score(relo.get()); // score after relocation
			double rms2   = relo->rms();  // rms after relocation
#ifdef EXTRA_DEBUGGING
			SEISCOMP_DEBUG("_associate trying pick id=%s  as %s   res=%.1f",
			       arr.pick->id.c_str(), phase.c_str(), arr.residual);
			SEISCOMP_DEBUG("_associate score: %.1f -> %.1f    rms: %.2f -> %.2f ",
			       original_score, score2, original_rms, rms2);
#endif
			if (score2 < original_score || rms2 > original_rms + 3./sqrt(10.+copy->arrivals.size())) { // no improvement

				int index = copy->findArrival(pick);
				if (index==-1) {
					SEISCOMP_ERROR("THIS SHOULD NEVER HAPPEN @_associate A");
					return false;
				}
				Arrival &arr = copy->arrivals[index];
				arr.excluded = Arrival::LargeResidual;

				// relocate anyway, to get consistent residuals even for the unused picks
#ifdef EXTRA_DEBUGGING
				SEISCOMP_DEBUG("_associate trying again with fixed depth of z=%g", origin->dep);
#endif
				_relocator.setFixedDepth(origin->dep);
				_relocator.useFixedDepth(true);
#ifdef LOG_RELOCATOR_CALLS
				SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
				relo = _relocator.relocate(copy.get());
				if ( ! relo) {
					SEISCOMP_ERROR("THIS SHOULD NEVER HAPPEN @_associate B");
				}
				else {
					double score3 = _score(relo.get()); // score after relocation
#ifdef EXTRA_DEBUGGING
					double rms3   = copy->rms();
					SEISCOMP_DEBUG("_associate score: %.1f -> %.1f -> %.1f   rms: %.2f -> %.2f -> %.2f",
							original_score, score2, score3, original_rms, rms2, rms3);
#endif
					if (score3 < original_score) // still no improvement
						relo = 0;
				}
			}

			if (relo) {
				int index = relo->findArrival(pick);
				if (index==-1) {
					SEISCOMP_ERROR("THIS SHOULD NEVER HAPPEN @_associate C");
					return false;
				}
				Arrival &arr = relo->arrivals[index];
				if (fabs(arr.residual) > _config.maxResidualUse) {
					// added arrival but pick is not used due to large residual
					arr.excluded = Arrival::LargeResidual;
					origin->add(arr);
					return true;
				}
			}

		}

		if (relo) {
			origin->updateFrom(relo.get());
		}
		else {
			copy = new Origin(*origin);
			if (arr.excluded != Arrival::UnusedPhase)
				arr.excluded = Arrival::DeterioratesSolution;
			copy->add(arr);
			origin->updateFrom(copy.get());
		}
	}
	else {
		origin->updateFrom(copy.get());
	}

	SEISCOMP_DEBUG_S(" ADD " + printOneliner(origin) + " add " + arr.pick->id + " " + arr.phase);
	return true;
}


bool Autoloc3::_addMorePicks(Origin *origin, bool keepDepth)
// associate all matching picks
{
	set<string> have;
	int arrivalCount = origin->arrivals.size();
	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = origin->arrivals[i];
		if (arr.excluded)
			continue;

		const Pick *pick = arr.pick.get();
		if ( ! pick->station() )
			continue;
		string x = pick->station()->net + "." + pick->station()->code + ":" + arr.phase;
		have.insert(x);
	}

#ifdef EXTRA_DEBUGGING
	SEISCOMP_DEBUG("_addMorePicks begin");
#endif
	int picksAdded = 0;
	for(PickMap::iterator
	    it = _pick.begin(); it != _pick.end(); ++it) {

		const Pick *pick = (*it).second.get();

		if ( ! pick->station() ) // better if blacklisted
			continue;
		if ( !_config.useManualPicks && manual(pick))  // FIXME: maybe not needed here
			continue;
		if ( ignored(pick))
			continue;

		// check if for that station we already have a P/PKP pick
		string x = pick->station()->net + "." + pick->station()->code + ":";
		if (have.count(x+"P") || have.count(x+"PKP"))
			continue;

		if ( pick->amp <= 0. || pick->snr <= 0.)
			continue;
		if (_tooLowSNR(pick))
			continue;
		if (_blacklisted(pick)) 
			continue;
		if (pick->origin()) // associated to another origin?
			continue;
		if ( ! _associate(origin, pick, "P") &&
		     ! _associate(origin, pick, "PKP"))
			continue;

		picksAdded ++;
	}

#ifdef EXTRA_DEBUGGING
	SEISCOMP_DEBUG("_addMorePicks end   added %d", picksAdded);
#endif

	if ( ! picksAdded)
		return false;

	_rename_P_PKP(origin);

	return true;
}


bool Autoloc3::_enhanceScore(Origin *origin, int maxloops)
{
	// TODO: make sure that the RMS doesn't increase too badly!
	int count = 0, loops = 0;

	// a very early origin
	if (origin->definingPhaseCount() < 1.5*_config.minPhaseCount) {

		// count XXL picks
		unsigned int xxlcount = 0;
		PickCPtr earliestxxl = 0;
		int arrivalCount = origin->arrivals.size();
		for (int i=0; i<arrivalCount; i++) {
			Arrival &arr = origin->arrivals[i];
			if (arr.pick->xxl) {
				xxlcount++;
				if (earliestxxl==0)
					earliestxxl = arr.pick;
				else if (arr.pick->time < earliestxxl->time)
					earliestxxl = arr.pick;
			}
		}

		// if there are enough XXL picks, only use these
		if (xxlcount >= _config.xxlMinPhaseCount) {

			OriginPtr copy = new Origin(*origin);
			// exclude those picks which are (in time) before the
			// earliest XXL pick
			int excludedcount = 0;
			for (int i=0; i<arrivalCount; i++) {
				Arrival &arr = origin->arrivals[i];
				if ( ! arr.pick->xxl && arr.pick->time < earliestxxl->time) {
					copy->arrivals[i].excluded = Arrival::ManuallyExcluded;
					excludedcount++;
				}
			}
			if (excludedcount) {
				if (_config.defaultDepthStickiness > 0.9) {
					_relocator.useFixedDepth(true);
				}
				else
					_relocator.useFixedDepth(false);

				copy->depthType = Origin::DepthFree;
				copy->lat = earliestxxl->station()->lat;
				copy->lon = earliestxxl->station()->lon;
#ifdef LOG_RELOCATOR_CALLS
				SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
				OriginPtr relo = _relocator.relocate(copy.get());
				if (relo) {
					origin->updateFrom(relo.get());
					SEISCOMP_INFO_S(" XXL " + printOneliner(origin));
					return true;
				}
			}
		}
	}


	// try to enhance score by excluding outliers
//	while (origin->definingPhaseCount() >= _config.minPhaseCount) {
	for (int loop=0; loop<0; loop++) {

		if (maxloops > 0 && ++loops > maxloops)
			break;

		double currentScore = _score(origin);
		double bestScore = currentScore;
		int    bestExcluded = -1;

		int arrivalCount = origin->arrivals.size();
		for (int i=0; i<arrivalCount; i++) {

			Arrival &arr = origin->arrivals[i];
			if (arr.excluded)
				continue;

			OriginPtr copy = new Origin(*origin);
			copy->arrivals[i].excluded = Arrival::ManuallyExcluded;

			_relocator.useFixedDepth(false);
#ifdef LOG_RELOCATOR_CALLS
			SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
			OriginPtr relo = _relocator.relocate(copy.get());
			if ( ! relo) {
				// try again, now using fixed depth (this sometimes helps)
				// TODO: figure out why this sometimes helps and whether there is a better way
				_relocator.useFixedDepth(true);
#ifdef LOG_RELOCATOR_CALLS
				SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
				relo = _relocator.relocate(copy.get());
				if ( ! relo)
					continue;
			}

			double score = _score(relo.get());

			if (score>bestScore) {
				bestScore = score;
				bestExcluded = i;
			}

			arr.excluded = Arrival::NotExcluded;
		}

		if (bestExcluded == -1)
			break;

		// new experimental criterion to avoid endless exclusions followed by
		// inclusions of the same picks.
		// TODO: review this criterion
		if (bestScore < currentScore+0.2)
			break;

		OriginPtr copy = new Origin(*origin);
		Arrival &arr = copy->arrivals[bestExcluded];
		arr.excluded = Arrival::LargeResidual;

		_relocator.useFixedDepth(false);
#ifdef LOG_RELOCATOR_CALLS
		SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
		OriginPtr relo = _relocator.relocate(copy.get());
		if ( ! relo) {
			// try again, now using fixed depth (this sometimes helps)
			_relocator.useFixedDepth(true);
#ifdef LOG_RELOCATOR_CALLS
			SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
			relo = _relocator.relocate(copy.get());
			if ( ! relo) // give up if fixing depth didn't help
				continue;
		}

		if (bestScore > 5)  // don't spoil the log
			SEISCOMP_DEBUG_S(" ENH " + printOneliner(relo.get()) + " exc " + arr.pick->id);

		origin->updateFrom(relo.get());
		count ++;
	}

	return (count > 0);
}



void Autoloc3::_rename_P_PKP(Origin *origin)
{
	// get phase naming right
	// FIXME: Somewhat hackish! Shouldn't be needed at this point any more

	int arrivalCount = origin->arrivals.size();
	for (int i=0; i<arrivalCount; i++) {

		Arrival &arr = origin->arrivals[i];
		const Pick *pick = arr.pick.get();
		double dt = pick->time-origin->time;

		if ( arr.distance > 105 && dt > 1000 && arr.phase == "P" ) {
			arr.phase = "PKP";
		}
		if ( arr.distance < 125 && dt < 1000 && arr.phase == "PKP" ) {
			arr.phase = "P";
		}
	}
}


double Autoloc3::_testFake(Origin *origin) const
{
	// Perform a series of tests to figure out of this origin is possibly
	// a fake origin resulting from wrong phase identification. It
	// measures how many of the picks may be misassociated.

//	TODO: DISABLE THE FOLLOWING FOR NON-TELESEISMIC NETWORKS

	if ( origin->imported )
		return 0.;

	if ( origin->score > 80 ) {
		// can safely skip this test
		return 0.;
	}

	double maxProbability = 0;
	int arrivalCount = origin->arrivals.size();

	for (OriginVector::const_iterator
	     it = _origins.begin(); it != _origins.end(); ++it) {

		const Origin *otherOrigin = (*it).get();
		int count = 0;

		// first very crude checks

		// we want to compare this origin with other *previous*
		// origins, so we restrict the time window accordingly
		if (otherOrigin->time < origin->time-1800 || otherOrigin->time > origin->time+600)
			continue;

		// we want to compare this origin with origins that
		// have significantly more picks, as otherwise the chance for
		// enough secondary phases is small anyway.
		// XXX XXX XXX
		// This is risky, because a new origin naturally has few picks initially
		if (otherOrigin->definingPhaseCount() < 2*origin->definingPhaseCount()) {
			continue;
		}

		// now, for our origin, count the possible conincidences with
		// later phases of the other origin
		int definingPhaseCount = origin->definingPhaseCount();
		for (int i=0; i<arrivalCount; i++) {

			Arrival &arr = origin->arrivals[i];
//			if (arr.excluded)
//				continue;

			// see if otherOrigin references this pick already
			int iarr = otherOrigin->findArrival(arr.pick.get());
			if (iarr != -1) {
				const Arrival &oarr = otherOrigin->arrivals[iarr];
//				if ( ! arr.excluded) {
					arr.excluded = Arrival::DeterioratesSolution;
					SEISCOMP_DEBUG("_testFake: doubly associated pick %s", oarr.pick->id.c_str());
					count ++;
					continue;
//				}
			}


			// now test for various phases
			const Station *sta = arr.pick->station();
			double delta, az, baz, depth=otherOrigin->dep;
			delazi(otherOrigin, sta, delta, az, baz);
			Seiscomp::TravelTimeTable ttt;
			Seiscomp::TravelTimeList *ttlist = ttt.compute(otherOrigin->lat, otherOrigin->lon, otherOrigin->dep, sta->lat, sta->lon, 0);
			if (delta > 30) {
				const Seiscomp::TravelTime *tt = getPhase(ttlist, "PP");
				if (tt != NULL && ! arr.pick->xxl && arr.score < 1) {
					double dt = arr.pick->time - (otherOrigin->time + tt->time);
					if (dt > -20 && dt < 30) {
						if (fabs(dt) < fabs(arr.residual))
							arr.excluded = Arrival::DeterioratesSolution;
						SEISCOMP_DEBUG("_testFake: %-6s %5lu %5lu PP   dt=%.1f", sta->code.c_str(),origin->id, otherOrigin->id, dt);
						count ++;
						delete ttlist;
						continue;
					}
				}
			}

			if (delta > 100) {
				const Seiscomp::TravelTime *tt = getPhase(ttlist, "PKP");
				if (tt != NULL && ! arr.pick->xxl) {
					double dt = arr.pick->time - (otherOrigin->time + tt->time);
					if (dt > -20 && dt < 50) { // a bit more generous for PKP
						if (fabs(dt) < fabs(arr.residual))
							arr.excluded = Arrival::DeterioratesSolution;
						SEISCOMP_DEBUG("_testFake: %-6s %5lu %5lu PKP  dt=%.1f", sta->code.c_str(),origin->id, otherOrigin->id, dt);
						count ++;
						delete ttlist;
						continue;
					}
				}
			}

			if (delta > 120 && delta < 142) {
				const Seiscomp::TravelTime *tt = getPhase(ttlist, "SKP");
				if (tt != NULL && ! arr.pick->xxl) {
					double dt = arr.pick->time - (otherOrigin->time + tt->time);
					if (dt > -20 && dt < 50) { // a bit more generous for SKP
						if (fabs(dt) < fabs(arr.residual))
							arr.excluded = Arrival::DeterioratesSolution;
						SEISCOMP_DEBUG("_testFake: %-6s %5lu %5lu SKP  dt=%.1f", sta->code.c_str(),origin->id, otherOrigin->id, dt);
						count ++;
						delete ttlist;
						continue;
					}
				}
			}

			if (delta > 100 && delta < 130) { // preliminary! TODO: need to check amplitudes
				const Seiscomp::TravelTime *tt = getPhase(ttlist, "PKKP");
				if (tt != NULL && ! arr.pick->xxl) {
					double dt = arr.pick->time - (otherOrigin->time + tt->time);
					if (dt > -20 && dt < 50) { // a bit more generous for PKKP
						if (fabs(dt) < fabs(arr.residual))
							arr.excluded = Arrival::DeterioratesSolution;
						SEISCOMP_DEBUG("_testFake: %-6s %5lu %5lu PKKP dt=%.1f", sta->code.c_str(),origin->id, otherOrigin->id, dt);
						count ++;
						delete ttlist;
						continue;
					}
				}
			}

			if (delta > 25 && depth > 60) {
				const Seiscomp::TravelTime *tt = getPhase(ttlist, "pP");
				if (tt) {
					double dt = arr.pick->time - (otherOrigin->time + tt->time);
					if (dt > -20 && dt < 30) {
						if (fabs(dt) < fabs(arr.residual))
							arr.excluded = Arrival::DeterioratesSolution;
						SEISCOMP_DEBUG("_testFake: %-6s %5lu %5lu pP   dt=%.1f", sta->code.c_str(),origin->id, otherOrigin->id, dt);
						count ++;
						delete ttlist;
						continue;
					}
				}

				tt = getPhase(ttlist, "sP");
				if (tt) {
					double dt = arr.pick->time - (otherOrigin->time + tt->time);
					if (dt > -20 && dt < 30) {
						if (fabs(dt) < fabs(arr.residual))
							arr.excluded = Arrival::DeterioratesSolution;
						SEISCOMP_DEBUG("_testFake: %-6s %5lu %5lu sP   dt=%.1f", sta->code.c_str(),origin->id, otherOrigin->id, dt);
						count ++;
						delete ttlist;
						continue;
					}
				}
			}

			if (delta < 110) {
				const Seiscomp::TravelTime *tt = getPhase(ttlist, "S"); // includes SKS!
				if (tt != NULL && ! arr.pick->xxl && arr.score < 1) {
					double dt = arr.pick->time - (otherOrigin->time + tt->time);
					if (dt > -20 && dt < 30) {
						if (fabs(dt) < fabs(arr.residual))
							arr.excluded = Arrival::DeterioratesSolution;
						SEISCOMP_DEBUG("_testFake: %-6s %5lu %5lu S    dt=%.1f", sta->code.c_str(),origin->id, otherOrigin->id, dt);
						count ++;
						delete ttlist;
						continue;
					}
				}
			}

			// TODO: We might actually be able to skip the phase test here
			// if we can more generously associate phases to the "good" origin
			// (loose association). In that case we only need to test if
			// a pick is referenced by an origin with a (much) higher score.
			delete ttlist;
		}

		if (count) {
			SEISCOMP_DEBUG("_testFake: %ld -> %ld, %d/%d", origin->id, otherOrigin->id, count, definingPhaseCount);
		}

		double probability = double(count)/definingPhaseCount;
//		if (count > maxCount)
//			maxCount = count;
		if (probability > maxProbability)
			maxProbability = probability;
	}

/*
	if (maxCount) {
//		double probability = double(maxCount)/origin->definingPhaseCount();
		double probability = double(maxCount)/arrivalCount;
		return probability;
	}
*/

	return maxProbability;
}


int Autoloc3::_removeWorstOutliers(Origin *origin)
{
#ifdef EXTRA_DEBUGGING
	string info_before = printDetailed(origin);
	SEISCOMP_DEBUG("_removeWorstOutliers begin");
#endif
	int count = 0;
	vector<string> removed;

	for (ArrivalVector::iterator
	     it = origin->arrivals.begin(); it != origin->arrivals.end();) {

		Arrival &arr = *it;

		if (arr.excluded && fabs(arr.residual) > _config.maxResidualKeep) {

			arr.pick->setOrigin(NULL); // disassociate the pick
			removed.push_back(arr.pick->id);
			it = origin->arrivals.erase(it);
			count++;
			// TODO try to re-associate the released pick with other origin
		}
		else ++it;
	}

	if (count==0)
		return 0;

#ifdef EXTRA_DEBUGGING
	string info_after = printDetailed(origin);
	// logging only
	int n = removed.size();
	string tmp=removed[0];
	for(int i=1; i<n; i++)
		tmp += ", "+removed[i];
	SEISCOMP_DEBUG("removed outlying arrivals from origin %ld: %s", origin->id, tmp.c_str());
	SEISCOMP_DEBUG_S("Before:\n" + info_before);
	SEISCOMP_DEBUG_S("After:\n"  + info_after);
	SEISCOMP_DEBUG("_removeWorstOutliers end");
#endif
	return count;
}


static bool is_P_arrival(const Arrival &arr)
{
	return (arr.phase=="P"  ||
		arr.phase=="Pn" ||
		arr.phase=="Pg" ||
		arr.phase=="Pb");
}

static bool is_PKP_arrival(const Arrival &arr)
{
	return (arr.phase == "PKP"   ||
		arr.phase == "PKPab" ||
		arr.phase == "PKPdf" ||
		arr.phase == "PKiKP");
}


bool Autoloc3::_residualOK(const Arrival &arr, double minFactor, double maxFactor) const
{
	double minResidual = -minFactor*_config.maxResidualUse;
	double maxResidual =  maxFactor*_config.maxResidualUse;

	if ( _config.aggressivePKP && is_PKP_arrival(arr) ) {
		minResidual *= 2;
		maxResidual *= 2;
	}

	if ( is_P_arrival(arr) ) {
		// Autoloc 2 hack for regional phases, to allow use of Pg (sometimes even S)
		// as Pn by increasing maxResidual. Which --in principle-- is bad but may be
		// better than leaving those phases out completely.
		double regionalWeight = 1.+0.7*exp(-arr.distance*arr.distance/50.);
		maxResidual *= regionalWeight;
	}

	if (arr.residual < minResidual || arr.residual > maxResidual)
		return false;

	return true;
}


bool Autoloc3::_trimResiduals(Origin *origin)
{
	int arrivalCount = origin->arrivals.size();
	int count = 0;


	while (origin->definingPhaseCount() >= _config.minPhaseCount) {
		double maxResidual = 0;
		int    maxIndex = -1;

		for (int i=0; i<arrivalCount; i++) {

			Arrival &arr = origin->arrivals[i];
			if (arr.excluded)
				continue;

			double normalizedResidual = arr.residual/_config.maxResidualUse;
			// add penalty for positive residuals  // TODO: make it configurable?
			if (normalizedResidual > 0) normalizedResidual *= 1.5;

			// if the residual is bad, keep track of the worst residual
			if (fabs(normalizedResidual) > maxResidual) {
				maxIndex = i;
				maxResidual = fabs(normalizedResidual);
			}
		}

		if (maxIndex == -1)
			break;

		if (fabs(maxResidual) < 1)
			break;

		OriginPtr copy = new Origin(*origin);
		Arrival &arr = copy->arrivals[maxIndex];
		arr.excluded = Arrival::LargeResidual;

#ifdef LOG_RELOCATOR_CALLS
		SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
		// NOTE that the behavior of _relocator is configured outside this routine
		OriginPtr relo = _relocator.relocate(copy.get());
		if ( ! relo)
			break;

		origin->updateFrom(relo.get());
		SEISCOMP_DEBUG_S(" TRM " + printOneliner(relo.get()) + " exc " + arr.pick->id);
		count ++;
	}

	// try to get some large-residual picks back into the solution
	while (true) {
		double minResidual = 1000;
		int    minIndex = -1;

		for (int i=0; i<arrivalCount; i++) {

			Arrival &arr = origin->arrivals[i];
			if (arr.excluded != Arrival::LargeResidual)
				continue;

			if (fabs(arr.residual) < minResidual) {
				minIndex = i;
				minResidual = fabs(arr.residual);
			}
		}

		// if the best excluded pick is either not found
		// or has too big a residual, stop here
		if (minIndex == -1 || minResidual > 2*_config.goodRMS)
			break;

		OriginPtr copy = new Origin(*origin);
		Arrival &arr = copy->arrivals[minIndex];
		arr.excluded = Arrival::NotExcluded;

#ifdef LOG_RELOCATOR_CALLS
		SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
		OriginPtr relo = _relocator.relocate(copy.get());
		if ( ! relo)
			break;

		origin->updateFrom(relo.get());
		SEISCOMP_DEBUG_S(" TRM " + printOneliner(relo.get()) + " inc " + arr.pick->id);
		count ++;
	}

	return count > 0;
}


bool Autoloc3::setStation(Station *station) {

	std::string key = station->net + "." + station->code;
	// if the station was configured already, there is nothing to do
	if (_stations.find(key) != _stations.end())
		return false;

	const StationConfig::Entry &e
		= _stationConfig.get(station->net, station->code);
	station->maxNucDist = e.maxNucDist;
	station->maxLocDist = 180;
	station->used = e.usage > 0;
        _stations.insert(StationMap::value_type(key, station));

	// propagate to _nucleator and _relocator
	_relocator.setStation(station);
	_nucleator.setStation(station);

        SEISCOMP_DEBUG("Initialized station %-8s", key.c_str());

	return true;
}


void Autoloc3::setLocatorProfile(const string &profile) {
	_nucleator.setLocatorProfile(profile);
	_relocator.setProfile(profile);
}


void Autoloc3::setConfig(const Config &config) {
	_config = config;
}


bool Autoloc3::setGridFile(const string &gridfile)
{
	if ( ! _nucleator.setGridFile(gridfile))
		return false;
	_nucleator._config.maxRadiusFactor = _config.maxRadiusFactor;
	return true;
}

void Autoloc3::setPickLogFilePrefix(const string &fname)
{
	_pickLogFilePrefix = fname;
}

void Autoloc3::setPickLogFileName(const string &fname)
{
	if (fname == _pickLogFileName && _pickLogFile.is_open() )
		return;

	// log file name has changed and/or file is not open

	if ( _pickLogFile.is_open() )
		_pickLogFile.close();

	_pickLogFileName = fname;
	if (fname == "")
		return;

	_pickLogFile.open(fname.c_str(), ios_base::app);
	if ( ! _pickLogFile.is_open() ) {
		SEISCOMP_ERROR_S("Failed to open pick log file " + fname);
		return;
	}
	SEISCOMP_INFO_S("Logging picks to file " + fname);
}

void Autoloc3::reset()
{
	SEISCOMP_INFO("reset requested");
	_associator.reset();
	_nucleator.reset();
	_outgoing.clear();
	_origins.clear();
	_lastSent.clear();
	_pick.clear();
	_blacklist.clear();
	_newOrigins.clear();
//	cleanup(now());
}


void Autoloc3::shutdown()
{
	SEISCOMP_INFO("shutting down autoloc");

	reset();
	_associator.shutdown();
	_nucleator.shutdown();
}


void Autoloc3::cleanup(Time minTime)
{
	if ( ! minTime) {
		minTime = now() - _config.maxAge - 1800;

		if (now() < _nextCleanup)
			return;
		if (_config.maxAge <= 0)
			return;
	}

	int beforePickCount   = Pick::count();
	int beforeOriginCount = Origin::count();

	for(PickMap::iterator
	    it = _pick.begin(); it != _pick.end(); ) {

		PickCPtr pick = (*it).second;
		if (pick->time < minTime)
			_pick.erase(it++);
		else ++it;
	}

	int nclean = _nucleator.cleanup(minTime);
	SEISCOMP_INFO("CLEANUP: Nucleator: %d items removed", nclean);
	_nextCleanup = now() + _config.cleanupInterval;
	SEISCOMP_INFO("CLEANUP ********** pick count   = %d/%d", beforePickCount,Pick::count());
	SEISCOMP_INFO("CLEANUP ********** origin count = %d/%d", beforeOriginCount,Origin::count());

	OriginVector _originsTmp;
	for(OriginVector::iterator
	    it = _origins.begin(); it != _origins.end(); ++it) {

		OriginPtr origin = *it;

		if (origin->time < minTime)
			continue;
	
		_originsTmp.push_back(origin);
	}
	_origins = _originsTmp;


	vector<OriginID> ids;
	for (map<int, OriginPtr>::iterator
	     it = _lastSent.begin(); it != _lastSent.end(); ++it) {

		const Origin *origin = (*it).second.get();
		if (origin->time < minTime)
			ids.push_back(origin->id);
	}

	for(vector<OriginID>::iterator
	    it = ids.begin(); it != ids.end(); ++it) {

		OriginID id = *it;
		_lastSent.erase(id);
	}
	dumpState();
}

}  // namespace Autoloc




namespace Autoloc {

bool Autoloc3::_depthIsResolvable(Origin *origin)
{
//	if (depthPhaseCount(origin)) {
//		origin->depthType = Origin::DepthPhases;
//		return true;
//	}

	if (origin->depthType == Origin::DepthDefault && origin->dep != _config.defaultDepth)
		origin->depthType = Origin::DepthFree;

	OriginPtr test = new Origin(*origin);
	_relocator.useFixedDepth(false);
	test->depthType = Origin::DepthFree;
#ifdef LOG_RELOCATOR_CALLS
	SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
	OriginPtr relo = _relocator.relocate(test.get());
	if (relo) {
#ifdef EXTRA_DEBUGGING
		SEISCOMP_DEBUG("_depthIsResolvable for origin %ld: dep=%.1f smaj=%.1f sdep=%.1f stim=%.1f",
			       origin->id, relo->dep, relo->error.semiMajorAxis, relo->error.sdepth, relo->error.stime);
#endif
		if (relo->error.sdepth > 0.) {
			if (relo->error.sdepth < 15*relo->error.stime) {
#ifdef EXTRA_DEBUGGING
				SEISCOMP_DEBUG("_depthIsResolvable passed for origin %ld (using new criterion A)", origin->id);
#endif
				return true;
			}
			if (relo->error.sdepth < 0.7*relo->dep) {
#ifdef EXTRA_DEBUGGING
				SEISCOMP_DEBUG("_depthIsResolvable passed for origin %ld (using new criterion B)", origin->id);
#endif
				return true;
			}
		}
	}
#ifdef EXTRA_DEBUGGING
	else {
		SEISCOMP_DEBUG("_depthIsResolvable failed for origin %ld", origin->id);
	}
#endif


#ifdef EXTRA_DEBUGGING
	SEISCOMP_DEBUG("__depthIsResolvable poorly for origin %ld (using new criterion)", origin->id);
	SEISCOMP_DEBUG("__depthIsResolvable using old criterion now");
#endif

	test = new Origin(*origin);
	test->dep = _config.defaultDepth;
	_relocator.useFixedDepth(true);
#ifdef LOG_RELOCATOR_CALLS
	SEISCOMP_DEBUG("RELOCATE autoloc.cpp line %d", __LINE__);
#endif
	relo = _relocator.relocate(test.get());
	if ( ! relo) {
		// if we fail to relocate using a fixed shallow depth, we
		// assume that the original depth is resolved.
#ifdef EXTRA_DEBUGGING
		SEISCOMP_DEBUG("_depthIsResolvable passed for origin %ld (using old criterion A)", origin->id);
#endif
		return true;
	}

	// relo here has the default depth (fixed)
	double score1 = _score(origin), score2 = _score(relo.get());
	if ( score2 < 0.8*score1 ) {
#ifdef EXTRA_DEBUGGING
		SEISCOMP_DEBUG("_depthIsResolvable passed for origin %ld (using old criterion B)", origin->id);
#endif
		return true;
	}

	if (origin->dep != relo->dep)
		SEISCOMP_INFO("Origin %ld: changed depth from %.1f to default of %.1f   score: %.1f -> %.1f", origin->id, origin->dep, relo->dep, score1, score2);
	origin->updateFrom(relo.get());
	origin->depthType = Origin::DepthDefault;
	_updateScore(origin); // why here?

#ifdef EXTRA_DEBUGGING
	SEISCOMP_DEBUG("_depthIsResolvable poorly for origin %ld", origin->id);
#endif
	return false;
}

}  // namespace Autoloc
