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




#ifndef _SEISCOMP_AUTOLOC_NUCLEATOR_
#define _SEISCOMP_AUTOLOC_NUCLEATOR_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

#include "datamodel.h"
#include "locator.h"
//#include "autoloc.h"

namespace Autoloc {

class Nucleator
{
	public:
		virtual bool init() = 0;
		virtual void setStation(const Station *station);
	public:
		virtual bool feed(const Pick *pick) = 0;
		const OriginVector &newOrigins() {
			return _newOrigins;
		}

		virtual int  cleanup(const Time& minTime) = 0;
		virtual void reset() = 0;
		virtual void shutdown() = 0;

	protected:
		// finalize configuration - to be called e.g. to set up a grid
		virtual void setup() = 0;

	protected:
		StationMap _stations;
//		double _config_maxDistanceXXL;

		std::set<std::string> _configuredStations;

	public:
		OriginVector _newOrigins;
};


class GridPoint;
DEFINE_SMARTPOINTER(GridPoint);
typedef std::vector<GridPointPtr> Grid;


class GridSearch : public Nucleator
{
	public:
		GridSearch();
		virtual bool init();

	public:
		// Configuration parameters controlling the behaviour of the Nucleator
		class Config {
		public:
			// minimum number of stations for new origin
			int nmin;
		
			// maximum distance of stations contributing to new origin
			double dmax;

			// configurable multiplier, 0 means it is ignored
			double maxRadiusFactor;
	
			// minimum cumulative amplitude of all picks
			double amin;
			int aminskip; // skip this many largest amplitudes
		
			std::string amplitudeType;
		
			int verbosity;
		
			Config() {
				nmin = 5;
				dmax = 180;
				maxRadiusFactor = 1;
				amin = 5*nmin;
				aminskip = 1;
				amplitudeType = "snr"; // XXX not yet used
				verbosity = 0;
			}
		};

	public:
		void setStation(const Station *station);
		const Config &config() const { return _config; }
		void setConfig(const Config &config) { _config = config; }
		bool setGridFile(const std::string &gridfile);

		void setLocatorProfile(const std::string &profile);

	public:
		// The Nucleator reads Pick's and Amplitude's. Only picks
		// with associated amplitude can be fed into the Nucleator.
		bool feed(const Pick *pick);
	
		int cleanup(const Time& minTime);
	
		void reset()
		{
			_newOrigins.clear();
		}
		void shutdown()
		{
			_abort = true;
		}

	protected:
		virtual void setup();

		// setup a single station - ideally "on the fly"
		bool _setupStation(const Station *station);

	private:
		bool _readGrid(const std::string &gridfile);

	private:
		Grid    _grid;
		Locator _relocator;

		bool _abort;

	public: // FIXME
		Config  _config;
};

// From a GridPoint point of view, a station has a
// distance, azimuth, traveltime etc. These are stored
// in StationWrapper, together with the corresponding
// StationPtr.
DEFINE_SMARTPOINTER(StationWrapper);
class StationWrapper  : public Seiscomp::Core::BaseObject {
public:
	StationWrapper(const Station *station, const std::string &phase, float distance, float azimuth, float ttime, float hslow)
		:  station(station), distance(distance), azimuth(azimuth), ttime(ttime), hslow(hslow), phase(phase)  {}
	StationWrapper(const StationWrapper &other) {
		station     = other.station;
		phase       = other.phase;
		distance    = other.distance;
		azimuth     = other.azimuth;
		ttime       = other.ttime;
		hslow       = other.hslow;
//			vslow       = other.vslow;
//			backazimuth = other.backazimuth;
	}
	// Since there will be of the order
	// 10^5 ... 10^6 StationWrapper's,
	// we need to use floats
	const Station *station;
	float distance, azimuth; //, backazimuth;
	float ttime, hslow; //, vslow;
	// to further save space, make this a
	// pointer to a static phase list entry:
	std::string phase;
};

// A Pick projected in back time, corresponding
// to the grid point location
//DEFINE_SMARTPOINTER(ProjectedPick);
//class ProjectedPick : public Seiscomp::Core::BaseObject {
class ProjectedPick {
public:
	ProjectedPick(const Time &t);
	ProjectedPick(PickCPtr p, StationWrapperCPtr w);
	ProjectedPick(const ProjectedPick&);
	~ProjectedPick();

	static int count();

	bool operator<(const ProjectedPick &p) const {
		return (this->projectedTime() < p.projectedTime()); }
	bool operator>(const ProjectedPick &p) const {
		return (this->projectedTime() > p.projectedTime()); }


	Time projectedTime() const { return _projectedTime; }

	Time _projectedTime;
	PickCPtr p;
	StationWrapperCPtr wrapper;
};

class GridPoint : public Hypocenter
{
	public:

	public:
		// normal grid point
		GridPoint(double latitude, double longitude, double depth);

		// grid point based on an existing origin (to get the aftershocks)
		GridPoint(const Origin &);
		~GridPoint() {
			_wrappers.clear();
			_picks.clear();
		}

	public:
		// feed a new pick and perhaps get a new origin
		const Origin* feed(const Pick*);

		// remove all picks older than tmin
		int cleanup(const Time& minTime);

	public:
//		void setStations(const StationMap *stations);

		bool setupStation(const Station *station);

	public: // private:
		// config
		double _radius, _dt;
		double maxStaDist;
		int _nmin;

		// min. number of picks for prelim. alert if all picks are XXL
		// XXX NOT YET USED XXX
		int _nminPrelim;

	private:
		std::map<std::string, StationWrapperCPtr> _wrappers;
		std::multiset<ProjectedPick>          _picks;
		OriginPtr _origin;
};

double originScore(const Origin *origin, double maxRMS=3.5, double radius=0.);

}

#endif
