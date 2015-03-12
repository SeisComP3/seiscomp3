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



#ifndef __SEISCOMP3_INTERNAL_SEISMOLOGY_LOCSAT_H__
#define __SEISCOMP3_INTERNAL_SEISMOLOGY_LOCSAT_H__


#include <seiscomp3/core/exceptions.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>


namespace Seiscomp{
namespace Internal{


#include "locsat_internal_types.h"


class LocSAT {

public:
	LocSAT();
	~LocSAT();

	void reset();
	Loc* doLocation();

	void addSite(const char* station, float lat, float lon, float elev);

	void addArrival(long arrival_id, const char* station, const char* phase,
	                double time, float deltime, int defining);
	void setArrivalAzimuth(float azimuth, float delaz, int defining);
	void setArrivalSlowness(float slow, float delslo, int defining);

	void setOrigin(float lat_init, float lon_init, float depth_init);
	void setOriginTime(int year4, int month, int day, int hour, int minute, int second, int usec);
	void setOriginTime(double epoch);

	void setLocatorParams(Locator_params* params);

	void setLocatorErrors();
	void setOriginErr();

	Loc* getNewLocation();

	void printLocatorParams();

private:
	Arrival* _arrival;
	Assoc* _assoc;
	Origerr* _origerr;
	Origin* _origin;
	Site* _sites;
	Locator_params* _locator_params;
	Locator_errors* _locator_errors;
	struct date_time* _dt;
	int _num_obs;
	char* _newnet;
	int _num_sta;

	int _siteCount;
	int _arrivalCount;
	int _assocCount;

};

} // of namespace Internal
} // of namespace Seiscomp

#endif
