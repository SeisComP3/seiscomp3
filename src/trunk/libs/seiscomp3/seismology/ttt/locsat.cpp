/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <math.h>
#include <iostream>
#include <stdexcept>
#include <string.h>

#include <seiscomp3/system/environment.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/seismology/ttt/locsat.h>


#define EXTRAPOLATE 0

namespace Seiscomp {
namespace TTT {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
extern "C" {

void distaz2_(double *lat1, double *lon1, double *lat2, double *lon2, double *delta, double *azi1, double *azi2);
int setup_tttables_dir(const char *new_dir);
double compute_ttime(double distance, double depth, char *phase, int extrapolate);
int num_phases();
char **phase_types();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::Locsat() : _Pindex(-1) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::Locsat(const Locsat &other) {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat &Locsat::operator=(const Locsat &other) {
	_Pindex = other._Pindex;
	_tablePrefix = other._tablePrefix;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::~Locsat() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Locsat::setModel(const std::string &model) {
	_model = model;

	if ( _model.empty() ) {
		_tablePrefix.clear();
		return true;
	}

	std::string tablePrefix = Environment::Instance()->shareDir() + "/locsat/tables/" + model;
	if ( _tablePrefix == tablePrefix ) return true;

	_tablePrefix = tablePrefix;
	return initTables();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Locsat::model() const {
	return _model;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Locsat::initTables() {
	if ( _tablePrefix.empty()
	  || (setup_tttables_dir(_tablePrefix.c_str()) != 0) )
		return false;

	int nphases = num_phases();
	char **phases = phase_types();

	_Pindex = -1;
	if ( phases != NULL ) {
		for ( int i = 0; i < nphases; ++i ) {
			if ( !strcmp(phases[i], "P") ) {
				_Pindex = i;
				break;
			}
		}
	}

	return _Pindex != -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTimeList *Locsat::compute(double delta, double depth) {
	int nphases = num_phases();
	char **phases = phase_types();
	//float vp, vs;
	double takeoff;

	TravelTimeList *ttlist = new TravelTimeList;
	ttlist->delta = delta;
	ttlist->depth = depth;

	//bool has_vel = get_vel(depth, &vp, &vs);

	for ( int i = 0; i < nphases; ++i ) {
		char *phase = phases[i];
		double ttime = compute_ttime(delta, depth, phase, EXTRAPOLATE);
		// This comparison is there to also skip NaN values
		if ( !(ttime > 0) ) continue;

		/*
		if ( has_vel ) {
			float v = (phase[i][0]=='s' || phase[i][0]=='S') ? vs : vp;
			takeoff = takeoff_angle(dtdd[i], depth, v);
			if ( dtdh[i] > 0. )
				takeoff = 180.-takeoff;
		}
		else*/
			takeoff = 0;

		ttlist->push_back(TravelTime(phase, ttime, 0, 0, 0, takeoff));
	}

	ttlist->sortByTime();

	return ttlist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::compute(const char *phase, double delta, double depth) {
	double ttime = compute_ttime(delta, depth, const_cast<char*>(phase), 0);
	if ( !(ttime > 0) ) throw NoPhaseError();

	return TravelTime(phase, ttime, 0, 0, 0, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTimeList *Locsat::compute(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	if ( !initTables() ) return NULL;

	double delta, azi1, azi2;

	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);

	/* TODO apply ellipticity correction */
	TravelTimeList *ttlist = compute(delta, dep1);
	ttlist->delta = delta;
	ttlist->depth = dep1;

	if ( ellc ) {
		TravelTimeList::iterator it;
		for ( it = ttlist->begin(); it != ttlist->end(); ++it ) {
			double ecorr = 0.;
			if ( ellipcorr((*it).phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
				(*it).time += ecorr;
		}
	}

	return ttlist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::compute(const char *phase,
                           double lat1, double lon1, double dep1,
                           double lat2, double lon2, double alt2,
                           int ellc) {
	if ( !initTables() ) throw NoPhaseError();

	double delta, azi1, azi2;
	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);

	TravelTime tt = compute(phase, delta, dep1);

	if ( ellc ) {
		double ecorr = 0.;
		if ( ellipcorr(tt.phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
			tt.time += ecorr;
	}

	return tt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::computeFirst(double delta, double depth) {
	char **phases = phase_types();
	char *phase = phases[_Pindex];
	double ttime = compute_ttime(delta, depth, phase, EXTRAPOLATE);
	if ( ttime < 0 ) throw NoPhaseError();

	return TravelTime(phase, ttime, 0, 0, 0, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::computeFirst(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	if ( !initTables() ) throw NoPhaseError();

	double delta, azi1, azi2;
	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);

	TravelTime tt = computeFirst(delta, dep1);
	if ( ellc ) {
		double ecorr = 0.;
		if ( ellipcorr(tt.phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
			tt.time += ecorr;
	}

	return tt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_TRAVELTIMETABLE(Locsat, "LOCSAT");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
