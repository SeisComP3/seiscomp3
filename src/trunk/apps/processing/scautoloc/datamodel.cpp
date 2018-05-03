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

#include <assert.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <set>
#include <seiscomp3/math/mean.h>
#include "datamodel.h"
#include "util.h"
#include "sc3adapters.h"

namespace Autoloc {

static int _pickCount=0;

Pick::Pick()
{
	amp = snr = per = normamp = 0;
	xxl = false;
	status = Automatic;
	_pickCount++;
	id = "";
//	orid1 = orid2 = 0;
	_station = NULL;
	_origin = NULL;
}

Pick::Pick(const Pick &other)
	: id(other.id), net(other.net), sta(other.sta), loc(other.loc), cha(other.cha), time(other.time), amp(other.amp), per(other.per), snr(other.snr), normamp(other.normamp), status(other.status), xxl(other.xxl)
{
	setStation(other.station());
}

Pick::Pick(const std::string &id, const std::string &net, const std::string &sta, const Time &time)
	: id(id), net(net), sta(sta), time(time)
{
	amp = snr = per = 0;
	xxl = false;
	_station = NULL;
	_origin = NULL;
	status = Automatic;
	_pickCount++;
}

Pick::~Pick()
{
	_pickCount--;
}

int Pick::count()
{
	return _pickCount;
}

void Pick::setStation(const Station *sta) const
{
	_station = (Station*)sta;
}

void Pick::setOrigin(const Origin *org) const
{
	_origin = (Origin*)org;
}


Arrival::Arrival(const Pick *pick, const std::string &phase, double residual)
	: origin(NULL), pick(pick), phase(phase), residual(residual)
{
	excluded = NotExcluded;
	score = 0;
	ascore = dscore = tscore = 0;
}

Arrival::Arrival(const Arrival &other)
{
	*this = other;
	this->pick = other.pick;
	this->origin = other.origin;
}

Arrival::Arrival(const Origin *origin, const Pick *pick, const std::string &phase, double residual, double affinity)
	: origin(origin), pick(pick), phase(phase), residual(residual), affinity(affinity)
{
	score = 0;
	ascore = dscore = tscore = 0;
}

bool operator<(const Arrival& a, const Arrival& b) {

	if (a.distance < b.distance)
		return true;
	if (a.distance > b.distance)
		return false;

	if (a.pick->time < b.pick->time)
		return true;
	if (a.pick->time > b.pick->time)
		return false;

	return false;
}

bool ArrivalVector::sort()
{
	std::sort(begin(),end());
	return false;
}

// static unsigned long _i = 1;

static int _originCount = 0;

Origin::Origin(double lat, double lon, double dep, const Time &time)
	: Hypocenter(lat,lon,dep), time(time), timeerr(0)
{
//	id = _i++;
	id = 0;
	_originCount++;
	imported = preliminary = false;
	processingStatus = New;
	locationStatus = Automatic;
	depthType = DepthFree;
	timestamp = 0.;
	score = 0;
}

Origin::Origin(const Origin &other)
	: Hypocenter(other.lat,other.lon,other.dep), time(other.time)
{
	updateFrom(&other);
	id = other.id;
	_originCount++;
}

Origin::~Origin()
{
	arrivals.clear();
	_originCount--;
}

int Origin::count()
{
	return _originCount;
}

int Origin::findArrival(const Pick *pick) const
{
	int arrivalCount = arrivals.size();
	for (int i=0; i<arrivalCount; i++) {
		if (arrivals[i].pick == pick)
			return i;
	}

	return -1;
}

void Origin::updateFrom(const Origin *other)
{
	unsigned long _id = id;
	*this = *other;
	arrivals = other->arrivals;
	id = _id;
}


bool Origin::add(const Arrival &arr)
{
	if ( findArrival(arr.pick.get()) != -1 ) {
		SEISCOMP_WARNING_S("Pick already present -> not added.  id = " + arr.pick->id);
		return false;
	}

	arr.pick->setOrigin(this);
	arrivals.push_back(arr);
	return true;
}

int Origin::phaseCount(double dmin, double dmax) const
{
	int count = 0;

	for (ArrivalVector::const_iterator
		it = arrivals.begin(); it != arrivals.end(); ++it) {
		const Arrival &arr = *it;

		if (dmin==0. && dmax==180.) {
			double delta, az, baz;
			delazi(this, arr.pick->station(), delta, az, baz);
			if (delta < dmin || delta > dmax)
				continue;
		}

		if (arr.excluded && arr.phase != "PKP")
			continue;

		count++;

	}

	return count;
}

int Origin::definingPhaseCount(double dmin, double dmax) const
{
	int count = 0;

	for (ArrivalVector::const_iterator
		it = arrivals.begin(); it != arrivals.end(); ++it) {
		const Arrival &arr = *it;

		if (dmin!=0. || dmax!=180.) {
			double delta, az, baz;
			delazi(this, arr.pick->station(), delta, az, baz);
			if (delta < dmin || delta > dmax)
				continue;
		}

		if (arr.excluded)
			continue;

		count++;

	}

	return count;
}


int Origin::associatedStationCount() const {
	std::set<std::string> stations;

	for (ArrivalVector::const_iterator
		it = arrivals.begin(); it != arrivals.end(); ++it) {
		const Arrival &arr = *it;

		if ( !arr.pick ) continue;

		stations.insert(arr.pick->net + "." + arr.pick->sta);
	}

	return stations.size();
}


int Origin::definingStationCount() const {
	std::set<std::string> stations;

	for (ArrivalVector::const_iterator
		it = arrivals.begin(); it != arrivals.end(); ++it) {
		const Arrival &arr = *it;

		if (arr.excluded) continue;

		if ( !arr.pick ) continue;

		stations.insert(arr.pick->net + "." + arr.pick->sta);
	}

	return stations.size();
}


double Origin::rms() const
{
	// This essentially implies that for an imported origin RMS has
	// no meaning, no matter if the origin has arrivals or not.
	if (imported) return 0;

	int arrivalCount = arrivals.size();
	std::vector<double> res;
	for(int i=0; i<arrivalCount; i++) {
		if ( ! arrivals[i].excluded)
			res.push_back(arrivals[i].residual);
	}

	return Seiscomp::Math::Statistics::rms(res);
}

double Origin::medianStationDistance() const
{
	int arrivalCount = arrivals.size();
	std::vector<double> distance;

	for(int i=0; i<arrivalCount; i++) {
		if ( ! arrivals[i].excluded)
			distance.push_back(arrivals[i].distance);
	}

	if (distance.size() == 0)
		return -1;

	return Seiscomp::Math::Statistics::median(distance);
}

void Origin::geoProperties(double &min, double &max, double &gap) const
{
	min = 180.;
	max = 0.;
	gap = -1;

	int arrivalCount = arrivals.size();

	std::vector<double> azi;

	for(int i=0; i<arrivalCount; i++) {
		if ( ! arrivals[i].excluded ) {
			if ( arrivals[i].distance < min )
				min = arrivals[i].distance;
			else if ( arrivals[i].distance > max )
				max = arrivals[i].distance;
			
			azi.push_back(arrivals[i].azimuth);
		}
	}

	std::sort(azi.begin(), azi.end());
	azi.push_back(azi.front()+360.0);

	gap = 0.0;

	for ( size_t i = 0; i < azi.size()-1; ++i ) {
		double azGap = azi[i+1]-azi[i];
		if ( azGap > gap )
			gap = azGap;
	}
}

int OriginDB::mergeEquivalentOrigins(const Origin *start)
{
	return 0;
}

bool OriginDB::find(const Origin *origin) const
{
	for (const_iterator it=begin(); it!=end(); ++it) {
		if (origin == (*it).get())
			return true;
	}
	return false;
}

Origin* OriginDB::find(const OriginID &id)
{
	for (iterator it=begin(); it!=end(); ++it) {
		if (id == (*it)->id)
			return (*it).get();
	}
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


const Origin *OriginDB::bestEquivalentOrigin(const Origin *origin) const
{
	const Origin *best = 0;
	int maxCommonPickCount = 0;

	for (const_iterator it=begin(); it != end(); ++it) {

		const double max_dt = 1500;
		const Origin* this_origin = (*it).get();

		if (fabs(this_origin->time - origin->time) > max_dt)
			continue;

		int commonPickCount = countCommonPicks(origin, this_origin);
		if (commonPickCount<3)
			continue; // FIXME: hackish

		if (commonPickCount > maxCommonPickCount) {
			maxCommonPickCount = commonPickCount;
			best = this_origin;
		}
	}

	return best;
}

}  // namespace Autoloc
