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


#include "locsat.h"
#include "locsat_internal.h"


using namespace Seiscomp::Seismology;


namespace Seiscomp {
namespace Internal {


LocSAT::LocSAT() {
	_origerr = (Origerr*)malloc(sizeof(Origerr));
	_origin = (Origin*)malloc(sizeof(Origin));

	_locator_params = (Locator_params*)malloc(sizeof(Locator_params));
	_locator_params->outfile_name = (char*)malloc(1024 * sizeof(char));
	_locator_params->prefix = (char*)malloc(1024 * sizeof(char));

	_dt = (struct date_time*)malloc(sizeof(struct date_time));

	_sites = NULL;
	_arrival = NULL;
	_assoc = NULL;
	_newnet = NULL;
	_locator_errors = NULL;

	reset();
	setOriginErr();
}


LocSAT::~LocSAT() {
	reset();

	if ( _origerr ) free(_origerr);
	if ( _origin ) free(_origin);

	if ( _locator_params ) {
		if ( _locator_params->outfile_name ) free(_locator_params->outfile_name);
		if ( _locator_params->prefix ) free(_locator_params->prefix);
		free(_locator_params);
	}

	if ( _dt ) free(_dt);
}


void LocSAT::reset() {
	if (_sites) free(_sites);
	_siteCount = 0;
	_num_sta = 0;

	if (_arrival) { free(_arrival); _arrival = NULL; }
	_arrivalCount = 0;
	_num_obs = 0;

	if (_assoc) { free(_assoc); _assoc = NULL; }
	_assocCount = 0;

	if (_locator_errors) { free(_locator_errors); _locator_errors = NULL; }

	_newnet = (char *)NULL;
}


Loc* LocSAT::doLocation() {
	if( (_num_sta <= 0) || (_num_obs <= 0))
		throw LocatorException("error: Too few usable data");

	//! XXX 9999 should be enough!
	//! changed original FORTRAN/C files: maxdata 300 --> 9999
	//! -------------------------------------------------------
	// librdwrt/rdcor.h:14
	// libloc/hypinv0.f:217
	// libloc/locsat0.f:215
	// libloc/hypcut0.f:149
	// libloc/rdcor.h:14
	// libloc/solve_via_svd_.c:122
	//! -------------------------------------------------------
	if( (_num_sta > 9999) || (_num_obs > 9999))
		throw LocatorException("error: Too many picks/stations [9999] - Please raise limits within pre-f2c locsat code!");


	int ierr = locate_event (NULL, _sites, _num_sta, _arrival, _assoc,
		 _origin, _origerr, _locator_params, _locator_errors,
		 _num_obs);

	//std::cerr << "ierr = locate_event: " <<  ierr << std::endl;

	switch (ierr) {
		case 0:
			return getNewLocation();
		case 6:
			throw LocatorException("error from locator: SVD routine can't decompose matrix");
		case 5:
			throw LocatorException("error from locator: Insufficient data for a solution");
		case 4:
			throw LocatorException("error from locator: Too few data to constrain O.T.");
		case 3:
			throw LocatorException("error from locator: Too few usable data");
		case 2:
			throw LocatorException("error from locator: Solution did not converge");
		case 1:
			throw LocatorException("error from locator: Exceeded maximum iterations");
		case 13:
			throw LocatorException("error from locator: Opening travel time tables");
		case 14:
			throw LocatorException("error from locator: Error reading travel time tables, unexpected EOF");
		case 15:
			throw LocatorException("error from locator: Unknown error reading travel time tables");
		default:
		{
			std::stringstream ss;
			ss << "error from locator: code " << ierr;
			throw LocatorException(ss.str());
		}
	}
}


void LocSAT::addSite(const char* station, float lat, float lon, float elev) {
	if (_sites)
		for (int i = 0; i < _siteCount; i++)
			if (strcmp(station, _sites[i].sta) == 0){
				return;
			}

	if (!_sites){
		_siteCount = 1;
		_sites = (Site*)malloc(_siteCount * sizeof(Site));
	}
	else{
		_siteCount++;
		_sites = (Site*)realloc(_sites, _siteCount * sizeof(Site));
	}
	strcpy(_sites[_siteCount-1].sta, station);
	_sites[_siteCount-1].lat = lat;
	_sites[_siteCount-1].lon = lon;
	_sites[_siteCount-1].elev = elev*0.001;

	_num_sta = _siteCount;
}


void LocSAT::addArrival(long arrival_id, const char* station, const char* phase,
                        double time, float deltim, int defining) {
	if (!_arrival){
		_arrivalCount = 1;
		_arrival = (Arrival*)malloc(_arrivalCount * sizeof(Arrival));
		_assoc = (Assoc*)malloc(_arrivalCount * sizeof(Assoc));
		_locator_errors = (Locator_errors*)malloc(sizeof(Locator_errors));
	}
	else{
		_arrivalCount++;
		_arrival = (Arrival*)realloc(_arrival, _arrivalCount * sizeof(Arrival));
		_assoc = (Assoc*)realloc(_assoc, _arrivalCount * sizeof(Assoc));
		_locator_errors = (Locator_errors*)realloc(_locator_errors, _arrivalCount * sizeof(Locator_errors));
	}

	_arrival[_arrivalCount-1].time = time;
	_arrival[_arrivalCount-1].deltim = deltim;
	
	if (defining > 0)
		strcpy(_assoc[_arrivalCount-1].timedef, "d");
	else
		strcpy(_assoc[_arrivalCount-1].timedef, "n");


	_arrival[_arrivalCount-1].azimuth = -1.0;
	_arrival[_arrivalCount-1].slow 	 = -1.0;
	strcpy(_assoc[_arrivalCount-1].azdef, "n");
	strcpy(_assoc[_arrivalCount-1].slodef, "n");

	_arrival[_arrivalCount-1].arid = arrival_id;
	_assoc[_arrivalCount-1].arid   = arrival_id;
	strcpy(_arrival[_arrivalCount-1].sta, station);
	strcpy(_assoc[_arrivalCount-1].phase, phase);

	_locator_errors[_arrivalCount-1].arid = 0;
	_locator_errors[_arrivalCount-1].time = 0;
	_locator_errors[_arrivalCount-1].slow = 0;
	_locator_errors[_arrivalCount-1].az = 0;

	_num_obs = _arrivalCount;
	_assocCount = _arrivalCount;
}


void LocSAT::setArrivalAzimuth(float azimuth, float delaz, int defining) {
	_arrival[_arrivalCount-1].azimuth = azimuth;
	_arrival[_arrivalCount-1].delaz = delaz;

	if (defining > 0)
		strcpy(_assoc[_arrivalCount-1].azdef, "d");
	else
		strcpy(_assoc[_arrivalCount-1].azdef, "n");
}


void LocSAT::setArrivalSlowness(float slow, float delslo, int defining) {
	_arrival[_arrivalCount-1].slow = slow;
	_arrival[_arrivalCount-1].delslo = delslo;

	if (defining > 0)
		strcpy(_assoc[_arrivalCount-1].slodef, "d");
	else
		strcpy(_assoc[_arrivalCount-1].slodef, "n");
}


void LocSAT::setOriginErr() {
	_origerr->sdobs		= -1.0;
	_origerr->smajax	= -1.0;
	_origerr->sminax	= -1.0;
	_origerr->strike	= -1.0;
	_origerr->stime		= -1.0;
	_origerr->sdepth	= -1.0;
	_origerr->sxx		= -1.0;
	_origerr->sxz		= -1.0;
	_origerr->syz		= -1.0;
	_origerr->syy		= -1.0;
	_origerr->szz		= -1.0;
	_origerr->sxy		= -1.0;
	_origerr->stt		= -1.0;
	_origerr->stx		= -1.0;
	_origerr->sty		= -1.0;
	_origerr->stz		= -1.0;

	_origin->lat	= -999.9;
	_origin->lon	= -999.9;
	_origin->depth	= -999.9;
}


void LocSAT::setOrigin(float lat_init, float lon_init, float depth_init) {
	_origin->lat                = lat_init;
	_origin->lon                = lon_init;
	_origin->depth              = depth_init;

	_locator_params->lat_init   = lat_init;
	_locator_params->lon_init   = lon_init;
	_locator_params->depth_init = depth_init;
}


void LocSAT::printLocatorParams(){
	std::cerr << "_locator_params->outfile_name   = " <<_locator_params->outfile_name  << std::endl;
	std::cerr << "_locator_params->prefix         = " << _locator_params->prefix << std::endl;

	std::cerr << "_locator_params->cor_level      = " << _locator_params->cor_level << std::endl;

	std::cerr << "_locator_params->use_location   = " << _locator_params->use_location << std::endl;
	std::cerr << "_locator_params->fix_depth      = " << _locator_params->fix_depth << std::endl;
	std::cerr << "_locator_params->fixing_depth   = " << _locator_params->fixing_depth << std::endl;
	std::cerr << "_locator_params->verbose        = " << _locator_params->verbose << std::endl;

	std::cerr << "_locator_params->conf_level     = " << _locator_params->conf_level << std::endl;
	std::cerr << "_locator_params->damp           = " << _locator_params->damp << std::endl;
	std::cerr << "_locator_params->est_std_error  = " << _locator_params->est_std_error << std::endl;
	std::cerr << "_locator_params->num_dof        = " << _locator_params->num_dof << std::endl;
	std::cerr << "_locator_params->max_iterations = " << _locator_params->max_iterations << std::endl;
}


void LocSAT::setLocatorParams(Locator_params* params){

	char *outfile_name = _locator_params->outfile_name;
	char *prefix = _locator_params->prefix;

	memmove(_locator_params, params, sizeof(Locator_params));

	_locator_params->prefix = prefix;
	_locator_params->outfile_name = outfile_name;

	strcpy(_locator_params->outfile_name, params->outfile_name);
	strcpy(_locator_params->prefix, params->prefix);
}


void LocSAT::setLocatorErrors(){

}


void LocSAT::setOriginTime(int year4, int month, int day,
                           int hour, int minute, int second, int usec) {
	_dt->year = year4;
	_dt->month = month;
	_dt->day = day;
	_dt->hour = hour;
	_dt->minute = minute;
	_dt->second = second;

	mdtodate(_dt); /* Convert month/day to Julian date */
	htoe(_dt);     /* Convert human to epoch time */

	_dt->epoch += usec;

	_origin->time = _dt->epoch;
}


void LocSAT::setOriginTime(double epoch) {
	_origin->time = epoch;
}


Loc* LocSAT::getNewLocation() {
	Loc *newLoc = (Loc*)malloc(sizeof(Loc));
	
	newLoc->newnet = _newnet;

	newLoc->siteCount = _siteCount;
	newLoc->sites = _sites;

	newLoc->arrivalCount = _arrivalCount;
	newLoc->arrival = _arrival;

	newLoc->assocCount = _assocCount;
	newLoc->assoc = _assoc;

	newLoc->origerr =_origerr;
	newLoc->origin = _origin;

	newLoc->locator_errors = _locator_errors;
	newLoc->locator_params = _locator_params;

	return newLoc;
}


} // of namespace Internal
} // of namespace Seiscomp
