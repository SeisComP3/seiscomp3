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
#include <string.h>
#include <iostream>
#include <stdexcept>

#include <seiscomp3/system/environment.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/seismology/ttt/libtau.h>


namespace {


double takeoff_angle(double p, double zs, double vzs) {
	// Compute the "takeoff angle" of a wave at the source.
	//
	// p  is the *angular* slowness of the wave, i.e. sec/deg
	// zs is the source depth
	// vz is the velocity at the source
	double pv;

	p  = p*180./M_PI;       // make p slowness in sec/rad
	pv = p*vzs/(6371.-zs);
	if (pv>1.) pv = 1.;

	return 180.*asin(pv)/M_PI;
}


}


extern "C" {

void distaz2_(double *lat1, double *lon1, double *lat2, double *lon2, double *delta, double *azi1, double *azi2);

}


namespace Seiscomp {
namespace TTT {


LibTau::LibTau() : _initialized(false) {
	_depth = -1;
}


LibTau::LibTau(const LibTau &other) {
	*this = other;
}


LibTau &LibTau::operator=(const LibTau &other) {
	_handle = other._handle;
	_depth = -1;
	_initialized = false;
	return *this;
}


LibTau::~LibTau() {
	if ( !_initialized ) return;
	tabout(&_handle);
}


bool LibTau::setModel(const std::string &model) {
	initPath(model);
	return true;
}


const std::string &LibTau::model() const {
	return _model;
}


void LibTau::initPath(const std::string &model) {
	if ( _model != model ) {
		if ( _initialized ) {
			tabout(&_handle);
			_initialized = false;
		}
	}
	else if ( _initialized ) return;

	if ( !model.empty() ) {
		std::string tablePath = Environment::Instance()->shareDir() +
		                        "/ttt/" + model;

		// Fill the handle structure with zeros
		memset(&_handle, 0, sizeof(_handle));
		int err = tabin(&_handle, tablePath.c_str());
		if ( err ) {
			std::ostringstream errmsg;
			errmsg  << tablePath << ".hed and " << tablePath << ".tbl";
			throw FileNotFoundError(errmsg.str());
		}

		_initialized = true;
		brnset(&_handle, "all");
	}

	_model = model;
}


void LibTau::setDepth(double depth) {
	if ( depth <= 0. ) depth = 0.01; // XXX Hack!!!

	if ( depth <= 0. || depth > 800 ) {
		std::ostringstream errmsg;
		errmsg.precision(8);
		errmsg  << "Source depth of " << depth
			<< " km is out of range of 0 < z <= 800";
		throw std::out_of_range(errmsg.str());
	}

	if ( depth != _depth ) {
		_depth = depth;
		depset(&_handle, _depth);
	}
}


TravelTimeList *LibTau::compute(double delta, double depth) {
	int n;
	char ph[1000], *phase[100];
	float time[100], p[100], dtdd[100], dtdh[100], dddp[100], vp, vs;
	TravelTimeList *ttlist = new TravelTimeList;
	ttlist->delta = delta;
	ttlist->depth = depth;

	setDepth(depth);

	for(int i=0; i<100; i++)
		phase[i] = &ph[10*i];

	trtm(&_handle, delta, &n, time, p, dtdd, dtdh, dddp, phase);
	bool has_vel = emdlv(6371-depth, &vp, &vs) == 0;

	for(int i=0; i<n; i++) {
		float takeoff;
		if(has_vel) {
			float v = (phase[i][0]=='s' || phase[i][0]=='S') ? vs : vp;
			takeoff = takeoff_angle(dtdd[i], depth, v);
			if (dtdh[i] > 0.)
				takeoff = 180.-takeoff;
		}
		else
			takeoff = 0;

		ttlist->push_back(
			TravelTime(phase[i], time[i], dtdd[i], dtdh[i], dddp[i], takeoff)
		);
	}

	ttlist->sortByTime();

	return ttlist;
}


TravelTimeList *LibTau::compute(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	if ( !_initialized ) setModel("iasp91");

	double delta, azi1, azi2;

//	Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);
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


TravelTime LibTau::computeFirst(double delta, double depth) {
	int n;
	char ph[1000], *phase[100];
	float time[100], p[100], dtdd[100], dtdh[100], dddp[100], vp, vs;

	setDepth(depth);

	for ( int i = 0; i < 100; ++i )
		phase[i] = &ph[10*i];

	trtm(&_handle, delta, &n, time, p, dtdd, dtdh, dddp, phase);
	emdlv(6371-depth, &vp, &vs);

	if ( n ) {
		float v = (phase[0][0]=='s' || phase[0][0]=='S') ? vs : vp;
		float takeoff = takeoff_angle(dtdd[0], depth, v);
		if ( dtdh[0] > 0. )
			takeoff = 180.-takeoff;

		return TravelTime(phase[0], time[0], dtdd[0], dtdh[0], dddp[0], takeoff);
	}

	throw NoPhaseError();
}


TravelTime LibTau::computeFirst(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	if ( !_initialized ) setModel("iasp91");

	double delta, azi1, azi2;

	Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);

	TravelTime tt = computeFirst(delta, dep1);

	if ( ellc ) {
		double ecorr = 0.;
		if ( ellipcorr(tt.phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
			tt.time += ecorr;
	}

	return tt;
}


REGISTER_TRAVELTIMETABLE(LibTau, "libtau");


}
}
