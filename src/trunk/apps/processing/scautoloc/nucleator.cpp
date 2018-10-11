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
#include <seiscomp3/core/strings.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <math.h>
using namespace std;

#include "util.h"
#include "sc3adapters.h"
#include "locator.h"
#include "nucleator.h"


namespace Autoloc {

static std::string station_key (const Station *station)
{
	return station->net + "." + station->code;
}


typedef std::set<PickCPtr> PickSet;

void Nucleator::setStation(const Station *station)
{
	std::string key = station->net + "." + station->code;
	if (_stations.find(key) != _stations.end())
		return; // nothing to insert
	_stations.insert(StationMap::value_type(key, station));
}

void GridSearch::setStation(const Station *station)
{
	Nucleator::setStation(station);
	_relocator.setStation(station);
}

GridSearch::GridSearch()
{
//	_stations = 0;
	_abort = false;
}


bool GridSearch::setGridFile(const std::string &gridfile)
{
	return _readGrid(gridfile);
}


void GridSearch::setLocatorProfile(const std::string &profile) {
	_relocator.setProfile(profile);
}


int GridSearch::cleanup(const Time& minTime)
{
	int count = 0;

	for (Grid::iterator it=_grid.begin(); it!=_grid.end(); ++it) {

		GridPoint *gridpoint = it->get();
		count += gridpoint->cleanup(minTime);
	}

	return count;
}

static int _projectedPickCount=0;

ProjectedPick::ProjectedPick(const Time &t)
	: _projectedTime(t)
{
	_projectedPickCount++;
}

ProjectedPick::ProjectedPick(PickCPtr p, StationWrapperCPtr w)
	: _projectedTime(p->time - w->ttime), p(p), wrapper(w)
{
	_projectedPickCount++;
}

ProjectedPick::ProjectedPick(const ProjectedPick &other)
	: _projectedTime(other._projectedTime), p(other.p), wrapper(other.wrapper)
{
	_projectedPickCount++;
}

ProjectedPick::~ProjectedPick()
{
	_projectedPickCount--;
}

int ProjectedPick::count()
{
	return _projectedPickCount;
}


GridPoint::GridPoint(double latitude, double longitude, double depth)
	: Hypocenter(latitude,longitude,depth), _radius(4), _dt(50), maxStaDist(180), _nmin(6), _nminPrelim(4), _origin(new Origin(latitude,longitude,depth,0))
{
}

GridPoint::GridPoint(const Origin &origin)
	: Hypocenter(origin.lat,origin.lon,origin.dep), _radius(4), _dt(50), maxStaDist(180), _nmin(6), _nminPrelim(4), _origin(new Origin(origin))
{
}


const Origin*
GridPoint::feed(const Pick* pick)
{
	// find the station corresponding to the pick
	const std::string key = station_key(pick->station());

	std::map<std::string, StationWrapperCPtr>::const_iterator
		xit = _wrappers.find(key);
	if (xit==_wrappers.end())
		// this grid cell may be out of range for that station
		return NULL;
	StationWrapperCPtr wrapper = (*xit).second;
	if ( ! wrapper->station ) {
		// TODO test in Nucleator::feed() and use logging
		// TODO at this point probably an exception should be thrown
		SEISCOMP_ERROR("Nucleator: station '%s' not found", key.c_str());
		return NULL;
		
	}

	// At this point we hold a "wrapper" which wraps a station and adds a
	// fews grid-point specific attributes such as the distance from this
	// gridpoint to the station etc.

	// If the station distance exceeds the maximum station distance
	// configured for the grid point...
	if ( wrapper->distance > maxStaDist ) return NULL;

	// If the station distance exceeds the maximum nucleation distance
	// configured for the station...
	if ( wrapper->distance > wrapper->station->maxNucDist )
		return NULL;

	// back-project pick to hypothetical origin time
	ProjectedPick pp(pick, wrapper);

	// store newly inserted pick
	std::multiset<ProjectedPick>::iterator latest = _picks.insert(pp);

	// roughly test if there is a cluster around the new pick
	std::multiset<ProjectedPick>::iterator it,
		lower  = _picks.lower_bound(pp.projectedTime() - _dt),
		upper  = _picks.upper_bound(pp.projectedTime() + _dt);
	std::vector<ProjectedPick> pps;
	for (it=lower; it!=upper; ++it)
		pps.push_back(*it);

	int npick=pps.size();

	// if the number of picks around the new pick is too low...
	if (npick < _nmin)
		return NULL;

	// now take a closer look at how tightly clustered the picks are
	double dt0 = 4; // XXX
	std::vector<int> _cnt(npick);
	std::vector<int> _flg(npick);
	for (int i=0; i<npick; i++) {
		_cnt[i] = _flg[i] = 0;
	}
	for (int i=0; i<npick; i++) {
		
		ProjectedPick &ppi = pps[i];
		double t_i   = ppi.projectedTime();
		double azi_i = ppi.wrapper->azimuth;
		double slo_i = ppi.wrapper->hslow;

		for (int k=i; k<npick; k++) {
			
			ProjectedPick &ppk = pps[k];
			double t_k   = ppk.projectedTime();
			double azi_k = ppk.wrapper->azimuth;
			double slo_k = ppk.wrapper->hslow;

			double azi_diff = fabs(fmod(((azi_k-azi_i)+180.), 360.)-180.);
			double dtmax = _radius*(slo_i+slo_k) * azi_diff/90. + dt0;

			if (fabs(t_i-t_k) < dtmax) {
				_cnt[i]++;
				_cnt[k]++;

				if(ppi.p == pp.p || ppk.p == pp.p)
					_flg[k] = _flg[i] = 1;
			}
		}
	}
	
	int sum=0;
	for (int i=0; i<npick; i++)
		sum += _flg[i];
	if (sum < _nmin)
		return NULL;

	std::vector<ProjectedPick> group;
	int cntmax = 0;
	Time otime;
	for (int i=0; i<npick; i++) {
		if ( ! _flg[i])
			continue;
		group.push_back(pps[i]);
		if (_cnt[i] > cntmax) {
			cntmax = _cnt[i];
			otime = pps[i].projectedTime();
		}
	}


// vvvvvvvvvvvvv Iteration

	std::vector<double> ptime(npick);
	for (int i=0; i<npick; i++) {
		ptime[i] = pps[i].projectedTime();
	}

	// estimate origin time from the median projected times
	// because the projected times are in order
//	double mtime = pps[npick/2].projectedTime(); // median time

//	mtime = 0.;
//	for (int i=0; i<npick; i++) {
//		mtime += ptime[i] / npick;
//	}

// X	double l1 = 0;
// X
// X	i=0;
// X	std::vector<double> dtx(npick), dty(npick);
// X	for (it=lower; it!=upper; ++it) {
// X		string code = (*it).p->station->code;
// X
// X		StationWrapper &sw = _wrappers[code];
// X		if ( ! sw.station ) {
// X			SEISCOMP_WARNING("No StationWrapper for " + code);
// X			continue;
// X		}
// X
// X		double az =  sw.azimuth*M_PI/180;
// X		double px = -sw.hslow * sin(az);
// X		double py = -sw.hslow * cos(az);
// X
// X		double dx=0.01,dy=0.01;
// X
// X		dtx[i] = px*dx;
// X		dty[i] = py*dy;
// X
// X		l1 += fabs( (*it).projectedTime() - mtime );
// X	}
// X
// X	double mtx = 0, mty = 0;
// X	for (i=0;i<npick;i++) {
// X		mtx += dtx[i];
// X		mty += dty[i];
// X	}
// X	mtx = mtx/(npick*0.01);
// X	mty = mty/(npick*0.01);
// X
// X	SEISCOMP_DEBUG("dmtx = %f  dmty = %f", mtx, mty);

//	mtx += mtime;
//	mty += mtime;

// X	double l1x = 0, l1y = 0;
// X
// X	for (it=lower,i=0; it!=upper; ++it) {
// X		string code = (*it).p->station->code;
// X		StationWrapper &sw = _wrappers[code];
// X		if ( ! sw.station ) {
// X			continue;
// X		}
// X		l1x += fabs( (*it).projectedTime()+dtx[i]/0.01 - (mtime+mtx) );
// X		l1y += fabs( (*it).projectedTime()+dty[i]/0.01 - (mtime+mty) );
// X	}
// X	SEISCOMP_DEBUG("dl1x = %f  dl2y = %f", (l1x-l1)/npick, (l1y-l1)/npick);
// X
// X	for (i=0;i<npick;i++) {
// X		l1x += dtx[i]/0.01;
// X		l1y += dty[i]/0.01;
// X	}

//	double meandev = l1/npick;
	

//	Origin* origin = new Origin(lat, lon, dep, otime);
	_origin->arrivals.clear();
	// add Picks/Arrivals to that newly created Origin
	set<string> stations;
	for (unsigned int i=0; i<group.size(); i++) {
		const ProjectedPick &pp = group[i];

		PickCPtr pick = pp.p;
		const std::string key = station_key(pick->station());
		// avoid duplicate stations XXX ugly without amplitudes
		if( stations.count(key))
			continue;
		stations.insert(key);

		StationWrapperCPtr sw( _wrappers[key]);

		Arrival arr(pick.get());
		arr.residual = pp.projectedTime() - otime;
		arr.distance = sw->distance;
		arr.azimuth  = sw->azimuth;
		arr.excluded = Arrival::NotExcluded;
		arr.phase = (pick->time - otime < 960.) ? "P" : "PKP";
//		arr.weight   = 1;
		_origin->arrivals.push_back(arr);
	}

	if (_origin->arrivals.size() < (size_t)_nmin)
		return NULL;

	return _origin.get();
}

int GridPoint::cleanup(const Time& minTime)
{
	int count = 0;

	// this is for counting only
	std::multiset<ProjectedPick>::iterator it, upper=_picks.upper_bound(minTime);
	for (it=_picks.begin(); it!=upper; ++it)
		count++;

	_picks.erase(_picks.begin(), upper);

	return count;
}

/*
void GridPoint::setStations(const StationMap *stations)
{
	_wrappers.clear();

	if ( stations == NULL ) return;

	for (StationMap::const_iterator
	     it = stations->begin(); it != stations->end(); ++it) {

		const Station *station = (*it).second.get();
		setupStation(station);
	}
}
*/

bool GridPoint::setupStation(const Station *station)
{
	double delta=0, az=0, baz=0;
	delazi(this, station, delta, az, baz);

	// Don't setup the grid point for a station if it is out of
	// range for that station - this reduces the memory used by
	// the grid
	if ( delta > station->maxNucDist )
		return false;

	TravelTime tt;
	if ( ! travelTimeP(lat, lon, dep, station->lat, station->lon, 0, delta, tt))
		return false;

	StationWrapperCPtr sw = new StationWrapper(station, tt.phase, delta, az, tt.time, tt.dtdd);
	std::string key = station_key (sw->station);
	_wrappers[key] = sw;

	return true;
}


/*
static double avgfn2(double x)
{
	if (x<-1 || x>1)
		return 0;

	x *= M_PI*0.5;
	x = cos(x);
	return x*x;
}

static double avgfn2(double x)
{
	if (x<-0.75 || x>0.75)
		return 0;
	if (x>-0.25 || x<0.25)
		return 1;

	x = 2*(x + (x>0 ? -0.25 : 0.25));
	x *= M_PI*0.5;
	x = cos(x);
	return x*x;
}
*/

static double avgfn2(double x)
{
	const double w = 0.2; // plateau width

	if (x < -1 || x > 1)
		return 0;
	if (x > -w && x < w)
		return 1;

	x = (x + (x>0 ? -w : w))/(1-w);
//	x = cos(x*M_PI*0.5);
	x = 0.5*(cos(x*M_PI)+1);
	return x*x;
}


static double depthFactor(double depth)
{
	// straight line, easy (but also risky!) to be made configurable
	return 1+0.0005*(200-depth);
}

static PickSet originPickSet(const Origin *origin)
{
	PickSet picks;

	int arrivalCount = origin->arrivals.size();
	for(int i=0; i<arrivalCount; i++) {
		Arrival &arr = ((Origin*)origin)->arrivals[i];
		if (arr.excluded) continue;
		picks.insert(arr.pick);
	}
	return picks;
}

double originScore(const Origin *origin, double maxRMS, double networkSizeKm)
{
//	networkSizeKm = 200.;
	((Origin*)origin)->arrivals.sort();

	double score = 0, amplScoreMax=0;
	int arrivalCount = origin->arrivals.size();
	//int n = origin->definingPhaseCount();
	for(int i=0; i<arrivalCount; i++) {
		double phaseScore = 1; // 1 for P / 0.3 for PKP
		Arrival &arr = ((Origin*)origin)->arrivals[i];
		PickCPtr pick = arr.pick;
		if ( ! pick->station()) {
// XXX			SEISCOMP_WARNING("originScore: missing station info for pick '%s'", pick->id.c_str());
			continue;
		}
		arr.score = 0;
		arr.ascore = arr.dscore = arr.tscore = 0;

		// higher score for picks with higher SNR
		double snr = pick->snr > 3 ? pick->snr : 3;
		if ( snr > 1.E07 )
			continue;
		if ( snr > 100 )
			snr = 100;

		// FIXME: This is HIGHLY experimental:
		// For a manual pick without SNR, as produced by
		// scolv, we assume a default value.
		if (manual(pick.get()) && pick->snr <= 0)
			snr = 10; // make this configureable

		double normamp = pick->normamp;
		if (manual(pick.get()) && normamp <= 0)
			normamp = 1; // make this configureable

		double snrScore = log10(snr);

		double d = arr.distance;
		// FIXME: This is HIGHLY experimental:
		// For a small, dense network the distance score must decay quickly with distance
		// whereas for teleseismic usages it must be a much broader function
		double r = networkSizeKm <= 0 ? pick->station()->maxNucDist : (0.5*networkSizeKm/111.195);
		double distScore = 1.5*exp(-d*d/(r*r));

		// Any amplitude > 1 percent of the XXL threshold
		// will have an increased amplitude score
		double q = 0.8;
		if (normamp <= 0) {
			SEISCOMP_WARNING("THIS SHOULD NEVER HAPPEN: pick %s with  normamp %g  amp %g (not critical)",
				       pick->id.c_str(), normamp, pick->amp);
			continue;
		}

		double amplScore = 1+q*(1+0.5*log10(normamp));
		// The score must *not* be reduced based on low amplitude
		if (amplScore < 1) amplScore = 1;

		// Amplitudes usually decrease with distance.
		// This hack takes this fact into account for computing the score.
		// A sudden big amplitude at large distance cannot increase the score too badly
		if(amplScoreMax==0)
			amplScoreMax = amplScore;
		else {
			if (i>2 && amplScore > amplScoreMax+0.4)
				amplScore = amplScoreMax+0.4;
			if (amplScore > amplScoreMax)
				amplScoreMax = amplScore;
		}

		amplScore *= snrScore;
//		weight += amplScore;

		// "scaled" residual
		// This accounts for the fact that at the beginning, origins
		// are more susceptible to outliers, so we allow a somewhat
		// higher residual. XXX Note that this may increase the score
		// for origins with less arrivals, which might be harmful.
		//double q = 20;
		//double x = q*q/(n*n+q*q);

		double timeScore = avgfn2(arr.residual/(2*maxRMS));

		arr.dscore = distScore;
		arr.ascore = amplScore;
		arr.tscore = timeScore;

		if (arr.excluded) {
			if (arr.excluded != Arrival::UnusedPhase)
				continue;
			if (arr.phase.substr(0,3) != "PKP")
				continue;
			phaseScore = 0.3;
		}

//		arr.score = phaseScore*weight*timeScore;
		arr.score = phaseScore*timeScore*distScore*amplScore;
		score += arr.score;

		// higher score for picks not excluded from location
//		score += arr.excluded ? 0 : 0.3;
	}

	// the closer the better
//	score -= 0.1*(origin->medianStationDistance() - 30);

	// slight preference to shallower origins
	score *= depthFactor(origin->dep);

/*
	double rms = origin->rms();

	// This is a function that penalizes rms > q*maxRMS
	// At rms == maxRMS this function is 1.
	double  q = 0.33, rmspenalty = 0;
	if (rms > maxRMS) {
		double x = ((rms-maxRMS*q)/(maxRMS-maxRMS*q));
		rmspenalty = x*x;
	}
*/

	return score;
}




static Origin* bestOrigin(OriginVector &origins)
{
	double maxScore = 0;
	Origin* best = 0;

	for (OriginVector::iterator it=origins.begin();
	     it != origins.end(); ++it) {

		Origin* origin = (*it).get();
		double score = originScore(origin);
		if (score > maxScore) {
			maxScore = score;
			best = origin;
		}
	}

	return best;
}




bool GridSearch::feed(const Pick *pick)
{
	_newOrigins.clear();

	// see if pick is a duplicate
//	if (_pa.count( pick->id ) )
//		// TODO: Check if pick needs to be updated, e.g.
//		// if the pick was blacklisted, it must be removed
//		return false;
//
//	_pa[ pick->id ] = pa;

	if (_stations.size() == 0) {
		SEISCOMP_ERROR("\nGridSearch::feed() NO STATIONS SET\n");
		exit(1);
	}

	std::string net_sta = pick->net + "." + pick->sta;

	// link pick to station through pointer

	if (pick->station() == 0) {
		StationMap::const_iterator it = _stations.find(net_sta);
		if (it == _stations.end()) {
			SEISCOMP_ERROR_S("\nGridSearch::feed() NO STATION " + net_sta + "\n");
			return false;
		}
		SEISCOMP_ERROR("GridSearch::feed()  THIS SHOULD NEVER HAPPEN");
		pick->setStation((*it).second.get());
	}


	// Has the station been configured already? If not, do it now.

	bool stationSetupNeeded = false;
	if (_configuredStations.find(net_sta) == _configuredStations.end()) {
		_configuredStations.insert(net_sta);
		stationSetupNeeded = true;
		SEISCOMP_DEBUG_S("GridSearch: setting up station " + net_sta);
	}

	std::map<PickSet, OriginPtr> pickSetOriginMap;

	// Main loop
	//
	// Feed the new pick into the individual grid points
	// and save all "candidate" origins in originVector

	double maxScore = 0;
	for (Grid::iterator it=_grid.begin(); it!=_grid.end(); ++it) {

		GridPoint *gp = it->get();

		if (stationSetupNeeded) {
			gp->setupStation(pick->station());
		}

		const Origin *origin = gp->feed(pick);
		if ( ! origin)
			continue;

		// look at the origin, check whether
		//  * it fulfils certain minimum criteria
		//  * we have already seen a similar but better origin 

		// test minimum number of picks
		if (origin->arrivals.size() < 6) // TODO: make this limit configurable
			continue;
		// is the new pick part of the returned origin?
		if (origin->findArrival(pick) == -1)
			// this is actually an unexpected condition!
			continue;

		const PickSet pickSet = originPickSet(origin);
		// test if we already have an origin with this particular pick set
		if (pickSetOriginMap.find(pickSet) != pickSetOriginMap.end()) {
			double score1 = originScore(pickSetOriginMap[pickSet].get());
			double score2 = originScore(origin);
			if (score2<=score1)
				continue;
		}

		double score = originScore(origin);
		if (score < 0.6*maxScore)
			continue;

		if (score > maxScore)
			maxScore = score;

/*
		_relocator.useFixedDepth(true);
SEISCOMP_DEBUG("RELOCATE nucleator.cpp 692");
		OriginPtr relo = _relocator.relocate(origin);
		if ( ! relo)
			continue;

		double delta, az, baz;
		delazi(origin->lat, origin->lon, gp->lat, gp->lon, delta, az, baz);
		if (_config.maxRadiusFactor > 0 &&
		    delta > _config.maxRadiusFactor*gp->_radius) // XXX private
			continue;
*/

		OriginPtr newOrigin = new Origin(*origin);

		pickSetOriginMap[pickSet] = newOrigin;
	}

	OriginVector tempOrigins;
	for (std::map<PickSet, OriginPtr>::iterator
	     it = pickSetOriginMap.begin(); it != pickSetOriginMap.end(); ++it) {

		Origin *origin = (*it).second.get();
		if (originScore(origin) < 0.6*maxScore)
			continue;

// XXX XXX XXX XXX XXX
// Hier nur jene Origins aus Gridsearch zulassen, die nicht mehrheitlich aus assoziierten Picks bestehen.
// XXX XXX XXX XXX XXX

		_relocator.useFixedDepth(true);
//SEISCOMP_DEBUG("RELOCATE nucleator.cpp 724");
		OriginPtr relo = _relocator.relocate(origin);
		if ( ! relo)
			continue;

		// see if the new pick is within the maximum allowed nucleation distance
		int index = relo->findArrival(pick);
		if (index==-1) {
			SEISCOMP_ERROR("pick unexpectedly not found in GridSearch::feed()");
			continue;
		}
		if (relo->arrivals[index].distance > pick->station()->maxNucDist)
			continue;

		tempOrigins.push_back(relo);
	}

	// Now for all "candidate" origins in tempOrigins try to find the
	// "best" one. This is a bit problematic as we don't go back and retry
	// using the second-best but give up here. Certainly scope for
	// improvement.

	OriginPtr best = bestOrigin(tempOrigins);
	if (best) {
		_relocator.useFixedDepth(false);
//SEISCOMP_DEBUG("RELOCATE nucleator.cpp 762");
		OriginPtr relo = _relocator.relocate(best.get());
		if (relo)
			_newOrigins.push_back(relo);
	}

	return _newOrigins.size() > 0;
}


bool GridSearch::_readGrid(const std::string &gridfile)
{
	ifstream ifile(gridfile.c_str());

	if ( ifile.good() )
		SEISCOMP_DEBUG_S("Reading gridfile " + gridfile);
	else {
		SEISCOMP_ERROR_S("Failed to read gridfile " + gridfile);
		return false;
	}

	_grid.clear();
	double lat, lon, dep, rad, dmax; int nmin;
	while ( ! ifile.eof() ) {
		std::string line;
		std::getline(ifile, line);

		Seiscomp::Core::trim(line);

		// Skip empty lines
		if ( line.empty() ) continue;

		// Skip comments
		if ( line[0] == '#' ) continue;

		std::istringstream iss(line, std::istringstream::in);

		if (iss >> lat >> lon >> dep >> rad >> dmax >> nmin) {
			GridPoint *gp = new GridPoint(lat, lon, dep);
			gp->_nmin = nmin;
			gp->_radius = rad;
			gp->maxStaDist = dmax;
			_grid.push_back(gp);
		}
	}
	SEISCOMP_DEBUG("read %d grid lines",int(_grid.size()));
	return true;
}


void GridSearch::setup()
{
//	_relocator.setStations(_stations);
}

}






















































/*
		TravelTimeList *ttlist = ttt.compute(
			_latitude, _longitude, _depth, slat, slon, salt);

		TravelTime tt;
		for (TravelTimeList::iterator it = ttlist->begin();
		     it != ttlist->end(); ++it) {
			tt = *it;
			if (delta < 114)
			// for  distances < 114, allways take 1st arrival
				break;
			if (tt.phase == "Pdiff")
			// for  distances >= 114, skip Pdiff, take next
				continue;
			break;
		}
		delete ttlist;
*/
