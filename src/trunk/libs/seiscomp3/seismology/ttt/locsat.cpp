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



#include <math.h>
#include <iostream>
#include <stdexcept>
#include <string.h>

#include <seiscomp3/system/environment.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/seismology/ttt/locsat.h>


namespace Seiscomp {
namespace TTT {


extern "C" {

void distaz2_(double *lat1, double *lon1, double *lat2, double *lon2, double *delta, double *azi1, double *azi2);
int setup_tttables_dir (const char *new_dir);
double compute_ttime(double distance, double depth, char *phase, int extrapolate);
int num_phases();
char **phase_types();

}


/*
namespace {


bool get_vel(std::vector<Locsat::Velocity> &lay, float depth, float *vp, float *vs) {
	bool ldep = false;
	size_t i = 0;

	if ( lay.empty() ) return false;

	if ( depth < 0 ) depth = 0;

	while ( !ldep && i < lay.size() ) {
		if ( lay[i].depth <= depth ) {
			if ( lay[i].depth == depth) {
				*vp = lay[i].vp;
				*vs = lay[i].vs;
				return true;
			}

			++i;
		}
		else
			ldep = true;
	}

	if ( ldep ) {
		*vp = lay[i-1].vp+(lay[i].vp-lay[i-1].vp)*(depth-lay[i-1].depth)/(lay[i].depth-lay[i-1].depth);
		*vs = lay[i-1].vs+(lay[i].vs-lay[i-1].vs)*(depth-lay[i-1].depth)/(lay[i].depth-lay[i-1].depth);
	}
	else {
		*vp = lay.back().vp;
		*vs = lay.back().vs;
	}

	return true;
}


bool read_tvel(std::vector<Locsat::Velocity> &velocities, const char *path) {
	FILE *fp;
	char *line = NULL;
	size_t len;
	std::string filename = path;

	filename += ".tvel";

	fp = fopen(filename.c_str(), "r");
	if ( fp == NULL )
		return false;

	// Skip the first two lines
	getline(&line, &len, fp);
	getline(&line, &len, fp);

	while ( getline(&line, &len, fp) > 0 ) {
		float z,p,s;
		if ( sscanf(line, "%f %f %f", &z, &p, &s) == 3 ) {
			velocities.push_back(Locsat::Velocity(z,p,s));
		}
	}

	fclose(fp);
	if ( line != NULL ) free(line);

	return true;
}


}
*/


std::string Locsat::_model;
int Locsat::_tabinCount = 0;
//std::vector<Locsat::Velocity> Locsat::_velocities;


Locsat::Locsat() : _initialized(false), _Pindex(-1) {}


Locsat::Locsat(const Locsat &other) {
	*this = other;
}


Locsat &Locsat::operator=(const Locsat &other) {
	_initialized = other._initialized;
	_Pindex = other._Pindex;
	if ( _initialized )
		++_tabinCount;

	return *this;
}


Locsat::~Locsat() {
	if ( _initialized ) {
		--_tabinCount;

		if ( !_tabinCount )
			_model.clear();
	}
}


bool Locsat::setModel(const std::string &model) {
	if ( _model != model ) {
		if ( _initialized ) {
			--_tabinCount;

			if ( !_tabinCount )
				_model.clear();

			_initialized = false;
		}
	}

	if ( _tabinCount && _model != model ) return false;

	if ( !_tabinCount )
		InitPath(model);

	if ( !_initialized ) {
		++_tabinCount;
		_initialized = true;
	}

	return _Pindex != -1;
}


const std::string &Locsat::model() const {
	return _model;
}


void Locsat::InitPath(const std::string &model)
{
	if ( _tabinCount ) {
		if ( model != _model )
			throw MultipleModelsError(_model);
		return;
	}

	//_velocities.clear();

	setup_tttables_dir((Environment::Instance()->shareDir() + "/locsat/tables/" + model).c_str());
	//read_tvel(_velocities, (Environment::Instance()->shareDir() + "/locsat/tables/" + model).c_str());

	_model = model;

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
}


TravelTimeList *Locsat::compute(double delta, double depth) {
	int nphases = num_phases();
	char **phases = phase_types();
	//float vp, vs;
	float takeoff;

	TravelTimeList *ttlist = new TravelTimeList;
	ttlist->delta = delta;
	ttlist->depth = depth;

	//bool has_vel = get_vel(depth, &vp, &vs);

	for ( int i = 0; i < nphases; ++i ) {
		char *phase = phases[i];
		double ttime = compute_ttime(delta, depth, phase, 0);
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


TravelTimeList *Locsat::compute(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	if ( !_tabinCount ) setModel("iasp91");

	double delta, azi1, azi2;

//	Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);
	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);


	/* TODO apply ellipticity correction */
	TravelTimeList *ttlist = compute(delta, dep1);
	ttlist->delta = delta;
	ttlist->depth = dep1;
	TravelTimeList::iterator it;
        for (it = ttlist->begin(); it != ttlist->end(); ++it) {

		double ecorr = 0.;
		if (ellipcorr((*it).phase, lat1, lon1, lat2, lon2, dep1, ecorr)) {
//			fprintf(stderr, " %7.3f %5.1f  TT = %.3f ecorr = %.3f\n", delta, dep1, (*it).time, ecorr);
			(*it).time += ecorr;
		}
	}
	return ttlist;
}


TravelTime Locsat::computeFirst(double delta, double depth) {
	if ( _Pindex < 0 ) throw NoPhaseError();
	char **phases = phase_types();
	char *phase = phases[_Pindex];
	double ttime = compute_ttime(delta, depth, phase, 1);
	if ( ttime < 0 ) throw NoPhaseError();

	return TravelTime(phase, ttime, 0, 0, 0, 0);
}


TravelTime Locsat::computeFirst(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	if ( !_tabinCount ) setModel("iasp91");

	double delta, azi1, azi2;

	Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);

	/* TODO apply ellipticity correction */
	return computeFirst(delta, dep1);
}


REGISTER_TRAVELTIMETABLE(Locsat, "LOCSAT");


}
}
