/***************************************************************************
 * Copyright
 * ---------
 * This file is part of the Virtual Seismologist (VS) software package.
 * VS is free software: you can redistribute it and/or modify it under
 * the terms of the "SED Public License for Seiscomp Contributions"
 *
 * VS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the SED Public License for Seiscomp
 * Contributions for more details.
 *
 * You should have received a copy of the SED Public License for Seiscomp
 * Contributions with VS. If not, you can find it at
 * http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt
 *
 * Authors of the Software: Michael Fischer and Yannik Behr
 * Copyright (C) 2006-2013 by Swiss Seismological Service
 ***************************************************************************/

#define SEISCOMP_COMPONENT VsMagnitude

#include "Vs30Mapping.h"

namespace ch {
namespace sed {

Vs30Mapping::Tuple::Tuple(float lat, float lon, float vsx) {
	_lat = lat;
	_lon = lon;
	_vsx = vsx;
}

std::string Vs30Mapping::Tuple::toString() {
	return "lat:" + Seiscomp::Core::toString(_lat) + ", lon:"
			+ Seiscomp::Core::toString(_lon) + ", vsx:"
			+ Seiscomp::Core::toString(_vsx);
}

bool Vs30Mapping::TupleHandler::open(std::string filename) {
	_isP = NULL; //&std::cin;
	_fbP = NULL;
	_vsdefault = -1;

	if ( Seiscomp::Core::toString("-") != filename ) {
		_fbP = new std::filebuf(); // NEW_MEM
		if ( !_fbP->open(filename.c_str(), std::ios::in) ) {
			SEISCOMP_ERROR("couldn't open %s", filename.c_str());
			return false;
		}
		_isP = new std::istream(_fbP); // NEW_MEM
	}

	return true;
}

void Vs30Mapping::TupleHandler::close() {
	delete _isP;
	_fbP->close();
	delete _fbP;
}

bool Vs30Mapping::TupleHandler::load(std::string filename) {
	if ( !open(filename) ) {
		SEISCOMP_ERROR( "could not open file %s", filename.c_str());
		return false;
	} else {
		if ( !read() ) {
			SEISCOMP_ERROR(
					"errors while reading or interpreting file %s", filename.c_str());
			return false;
		}
		close();
	}
	return true; // if successful
}

float Vs30Mapping::TupleHandler::getVsDefault() {
	return _vsdefault;
}

void Vs30Mapping::TupleHandler::setVsDefault(float val) {
	_vsdefault = val;
}

float Vs30Mapping::TupleHandler::getVs(double lat, double lon) {

	SEISCOMP_DEBUG("lat: %.2f, lon: %.2f", lat, lon);
	// coord is not used in the default implementation
	return _vsdefault;
}

bool Vs30Mapping::TupleHandlerGrid::read() {
	if ( NULL == _isP )
		return false;

	float lat;
	float lon;

	float oldlat = std::numeric_limits<float>::min();
	float oldlon = std::numeric_limits<float>::min();
	float oldlon2 = std::numeric_limits<float>::min();

	float vsx;

	for ( int linecnt = 0; !(*_isP).eof(); linecnt++ ) {

		char buf[4096];
		(*_isP).getline(buf, 4096);
		int charcnt = (*_isP).gcount();
		if ( 3 > charcnt ) {
			continue; // empty line found, skip to next
		}
		if ( !(4096 > charcnt) )
			return false; // line too long

		int ret = sscanf(buf, "%f,%f,%f", &lon, &lat, &vsx);
		if ( !(3 == ret) ) // ret is nr. of items found by scanf
			return false;

		_tuplelist.push_back(Tuple(lat, lon, vsx));

		// sanity checks
		if ( !(-180 <= lon) ) {
			SEISCOMP_ERROR(
					"Longitude must be equal or greater than -180 (lon=%.2f)", lon);
			return false;
		}
		if ( !(lon <= +180) ) {
			SEISCOMP_ERROR(
					"Longitude must be equal or smaller than +180 (lon=%.2f)", lon);
			return false;
		}
		if ( !((fabs(lon) > fabs(oldlon)) || (fabs(oldlon) > fabs(oldlon2))) ) {
			SEISCOMP_ERROR("Longitudes must increase continuously");
			return false;
		}
		if ( !(-90 <= lat) ) {
			SEISCOMP_ERROR(
					"Latitude must be equal or greater than -90 (lat=%.2f)", lat);
			return false;
		}
		if ( !(lat <= +90) ) {
			SEISCOMP_ERROR(
					"Latitude must be equal or smaller than +90 (lat=%.2f)", lat);
			return false;
		}
		if ( !(fabs(lat) >= fabs(oldlat)) ) {
			SEISCOMP_ERROR("Latitudes must increase continuously");
			return false;
		}
		if ( !(150 <= vsx) ) {
			SEISCOMP_ERROR(
					"Vs30 values must be equal or greater than 150 m/s. (Vs30 = %.2f m/s)", vsx);
			return false;
		}
		if ( !(vsx <= 2000) ) {
			SEISCOMP_ERROR(
					"Vs30 values must be equal or smaller than 2000 m/s. (Vs30 = %.2f m/s)", vsx);
			return false;
		}

		if ( true ) {
			oldlon2 = oldlon;
			oldlon = lon;
		}
		if ( oldlat != lat ) {
			_rowidx.push_back(_tuplelist.size() - 1); // remember begin of new row
			oldlat = lat;
			oldlon = std::numeric_limits<float>::min();
		}
	}
	return true; // successfully read
}

float Vs30Mapping::TupleHandlerGrid::getRow(double lat, double lon, size_t l,
		size_t r) {
	if ( !(l < _rowidx.size()) ) {
		SEISCOMP_ERROR(
				"Line index exceeds the number of rows. (line index=%d, number of rows=%d)", (int)l, (int) _rowidx.size());
		return -1;
	}
	if ( !(r < _rowidx.size() && (l <= r)) ) {
		SEISCOMP_ERROR(
				"Row index exceeds the number of rows or line index is greater than row index. (line index=%d, row index=%d, number of rows=%d)", (int)l, (int)r, (int)_rowidx.size());
		return -1;
	}

	if ( 0 == (r - l) ) {
		return l;
	}

	if ( 1 == (r - l) ) {
		float vl = fabs(_tuplelist.at(_rowidx.at(l))._lat - lat);
		float vr = fabs(_tuplelist.at(_rowidx.at(r))._lat - lat);
		if ( vl < vr ) {
			return l;
		} else {
			return r;
		}
	}

	int k = l + (r - l) / 2;

	Vs30Mapping::Tuple * p = &(_tuplelist.at(_rowidx.at(k)));
	if ( lat < p->_lat ) {
		return getRow(lat, lon, l, k);
	} else {
		return getRow(lat, lon, k, r);
	}
}

float Vs30Mapping::TupleHandlerGrid::getCol(double lat, double lon, size_t l,
		size_t r) {
	if ( !(l < _tuplelist.size()) ) {
		SEISCOMP_ERROR(
				"Line index exceeds the number of lines. (line index=%d, number of lines=%d)", (int)l, (int)_tuplelist.size());
		return -1;
	}
	if ( !(r < _tuplelist.size() && (l <= r)) ) {
		SEISCOMP_ERROR(
				"Row index exceeds the number of lines or line index is greater than row index. (line index=%d, row index=%d, number of lines=%d)", (int)l, (int)r, (int)_tuplelist.size());
		return -1;
	}

	/*
	 SEISCOMP_DEBUG("line=%d, row=%d",(int)l,(int)r);
	 SEISCOMP_DEBUG("%s",_tuplelist.at(l).toString().c_str());
	 SEISCOMP_DEBUG("%s",_tuplelist.at(r).toString().c_str());
	 */

	if ( 0 == (r - l) ) {
		return l;
	}

	if ( 1 == (r - l) ) {
		float vl = fabs(_tuplelist.at(l)._lon - lon);
		float vr = fabs(_tuplelist.at(r)._lon - lon);
		if ( vl < vr ) {
			return l;
		} else {
			return r;
		}
	}

	int k = l + (r - l) / 2;
	Tuple * p = &(_tuplelist.at(k));
	if ( lon < p->_lon ) {
		return getCol(lat, lon, l, k);
	} else {
		return getCol(lat, lon, k, r);
	}
}

float Vs30Mapping::TupleHandlerGrid::getVs(double lat, double lon) {
	int row = getRow(lat, lon, 0, _rowidx.size() - 1);
	int col = getCol(lat, lon, _rowidx.at(row), _rowidx.at(row + 1) - 1)
			- _rowidx.at(row);

	Vs30Mapping::Tuple * p = &(_tuplelist.at(_rowidx.at(row) + col));

	if ( true ) {
		// station should be close to a grid point now
		if ( (fabs(lat - p->_lat) < 0.1) && (fabs(lon - p->_lon) < 0.1) ) {
			return p->_vsx;
		}
	}

	// not found; return some default value
	return TupleHandler::getVs(lat, lon);
}

Vs30Mapping::TupleHandler * Vs30Mapping::_tuplehandlerP = NULL;

Vs30Mapping * Vs30Mapping::_vsxmappingP = NULL;

int Vs30Mapping::TYPE_VS30 = 30;

float Vs30Mapping::getVsDefault(int vstype) {
	if ( !(TYPE_VS30 == vstype) ) {
		SEISCOMP_ERROR("Only Vs30 correction supported.");
		return -1;
	}

	float vsdefault = _vsxmappingP->_tuplehandlerP->getVsDefault();

	return vsdefault;
}

void Vs30Mapping::setVsDefault(int vstype, float val) {
	if ( !(TYPE_VS30 == vstype) ) {
		SEISCOMP_ERROR("Only Vs30 correction supported.");
		return;
	}

	_vsxmappingP->_tuplehandlerP->setVsDefault(val);
}

float Vs30Mapping::getVs(int vstype, double lat, double lon) {
	if ( !(TYPE_VS30 == vstype) ) {
		SEISCOMP_ERROR("Only Vs30 correction supported.");
		return -1;
	}

	// a negative value means that vsx mapping is turned off
	float vsdefault = getVsDefault(vstype);
	if ( 0 > vsdefault ) {
		return vsdefault;
	}

	return _vsxmappingP->_tuplehandlerP->getVs(lat, lon);
}

Vs30Mapping * Vs30Mapping::createInstance(std::string filename) {
	if ( !(NULL == _vsxmappingP) ) {
		SEISCOMP_ERROR("Instance has already been created.");
		return NULL;
	}

	_vsxmappingP = new Vs30Mapping(); // NEW_MEM

	_vsxmappingP->_tuplehandlerP = new TupleHandlerGrid(); // NEW_MEM
	if ( _vsxmappingP->_tuplehandlerP->load(filename) ) {
		return _vsxmappingP;
	} else {
		return NULL;
	}
}

Vs30Mapping * Vs30Mapping::sharedInstance() {
	if ( (NULL == _vsxmappingP) ) {
		SEISCOMP_ERROR("Instance has not been created.");
		return NULL;
	}
	return _vsxmappingP;
}

Vs30Mapping * Vs30Mapping::destroyInstance(Vs30Mapping * vsxmappingP) {
	if ( !(_vsxmappingP == vsxmappingP) ) {
		SEISCOMP_ERROR("Argument is not of type Vs30Mapping.");
		return NULL;
	}
	if ( NULL != _vsxmappingP ) {
		delete _vsxmappingP->_tuplehandlerP;
		delete _vsxmappingP; // DEL_MEM
		_vsxmappingP = NULL;
	}
	return NULL;
}

}
}

// eof
