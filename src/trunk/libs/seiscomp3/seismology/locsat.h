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



#ifndef __SEISCOMP3_SEISMOLOGY_LOCSAT_H__
#define __SEISCOMP3_SEISMOLOGY_LOCSAT_H__


#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/seismology/locatorinterface.h>
#include <seiscomp3/core.h>


#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <map>


namespace Seiscomp{


namespace Internal {

struct Locator_params;
class LocSAT;
class Loc;

}


enum LocSATParams {
	LP_NUM_DEG_FREEDOM,     /* 9999    - number of degrees of freedom    */
	LP_EST_STD_ERROR,       /* 1.0     - estimate of data std error      */
	LP_CONF_LEVEL,          /* 0.9     - confidence level    		     */
	LP_DAMPING,             /* -1.0    - damping (-1.0 means no damping) */
	LP_MAX_ITERATIONS,      /* 20      - limit iterations to convergence */
	LP_FIX_DEPTH,           /* true    - use fixed depth ?               */
	LP_FIXING_DEPTH,        /* 0.0     - fixing depth value              */
	LP_LAT_INIT,            /* modifiable - initial latitude             */
	LP_LONG_INIT,           /* modifiable - initial longitude            */
	LP_DEPTH_INIT,          /* modifiable - initial depth                */
	LP_USE_LOCATION,        /* true    - use current origin data ?       */
	LP_VERBOSE,             /* true    - verbose output of data ?        */
	LP_COR_LEVEL,           /* 0       - correction table level          */
	LP_OUT_FILENAME,        /* NULL    - name of file to print data      */
	LP_PREFIX,              /* NULL    - dir name & prefix of tt tables  */
	LP_MIN_ARRIVAL_WEIGHT,  /*  0.5    - if arr-weight = less than this, locsat will ignore this arrival */
	LP_RMS_AS_TIME_ERROR    /* true    - locate in two passes and use the arrival RMS
	                                     as time error in the second pass */
};


struct SC_SYSTEM_CORE_API LocSATErrorEllipsoid {
	LocSATErrorEllipsoid() {
		sxx=syy=szz=stt=sxy=sxz=syz=stx=sty=stz=sdobs=smajax=sminax=strike=sdepth=stime=conf=0.;
	}

	float   sxx;
	float   syy;
	float   szz;
	float   stt;
	float   sxy;
	float   sxz;
	float   syz;
	float   stx;
	float   sty;
	float   stz;
	float   sdobs;
	float   smajax;
	float   sminax;
	float   strike;
	float   sdepth;
	float   stime;
	float   conf;
};


class SC_SYSTEM_CORE_API LocSAT : public Seismology::LocatorInterface {
	public:
		LocSAT();
		virtual ~LocSAT();

		virtual bool init(const Config::Config &config);

		//! Returns supported parameters to be changed.
		virtual IDList parameters() const;

		//! Returns the value of a parameter.
		virtual std::string parameter(const std::string &name) const;

		//! Sets the value of a parameter.
		virtual bool setParameter(const std::string &name,
		                          const std::string &value);

		virtual IDList profiles() const;
		virtual void setProfile(const std::string &name);

		static void setDefaultProfile(const std::string &name);
		static std::string currentDefaultProfile();

		void setNewOriginID(const std::string& newOriginID);

		int capabilities() const;

		DataModel::Origin* locate(PickList& pickList);
		DataModel::Origin* locate(PickList& pickList,
		                          double initLat, double initLon, double initDepth,
		                          const Seiscomp::Core::Time& initTime);

		DataModel::Origin* relocate(const DataModel::Origin* origin);

		const LocSATErrorEllipsoid &errorEllipsoid() const {
			return _errorEllipsoid;
		}

	private:
		void setLocatorParams(int param, const char* value);
		std::string getLocatorParams(int param) const;
		void setDefaultLocatorParams();

		DataModel::Origin* relocate(const DataModel::Origin* origin, double timeError);
		bool loadArrivals(const DataModel::Origin* origin, double timeError);
		DataModel::Origin* fromPicks(PickList& pickList);
		DataModel::Origin* loc2Origin(Internal::Loc* loc);

		double stationCorrection(const std::string &staid, const std::string &stacode,
		                         const std::string &phase) const;


	private:
		typedef std::map<std::string, double> PhaseCorrectionMap;
		typedef std::map<std::string, PhaseCorrectionMap> StationCorrectionMap;

		static std::string        _defaultTablePrefix;
		static IDList             _allowedParameters;

		StationCorrectionMap      _stationCorrection;
		std::string               _newOriginID;
		std::string               _tablePrefix;
		bool                      _computeConfidenceEllipsoid;
		Internal::LocSAT         *_locateEvent;
		Internal::Locator_params *_locator_params;
		double                    _minArrivalWeight;
		bool                      _useArrivalRMSAsTimeError;

		IDList                    _profiles;

		LocSATErrorEllipsoid      _errorEllipsoid;
};


}// of namespace Seiscomp

#endif
