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




#ifndef _SEISCOMP_AUTOLOC_DATAMODEL_
#define _SEISCOMP_AUTOLOC_DATAMODEL_
#include <string>
#include <map>
#include <list>
#include <vector>

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/datamodel/origin.h>


//#include <seiscomp3/seismology/locsat.h>

// #include "nucleator.h"

namespace Autoloc {

typedef unsigned long OriginID;
typedef double Time;

DEFINE_SMARTPOINTER(Station);

class Station : public Seiscomp::Core::BaseObject {

  public:

	Station(const std::string &code, const std::string &net, double lat, double lon, double alt)
		: code(code), net(net), lat(lat), lon(lon), alt(alt)
	{
		used = true;
		maxNucDist = maxLocDist = 180;
	}

	std::string code, net;
	double lat, lon, alt;
	double maxNucDist, maxLocDist;
	bool used;
};

typedef std::map<std::string, StationCPtr> StationDB;


DEFINE_SMARTPOINTER(Origin);
class Origin;


DEFINE_SMARTPOINTER(Pick);
class Pick;

class Pick : public Seiscomp::Core::BaseObject {

  public:
	typedef enum { Automatic, Manual, Confirmed, IgnoredAutomatic } Status;

  public:

	Pick();
	Pick(const Pick &other);
	Pick(const std::string &id, const std::string &net, const std::string &sta, const Time &time);
	~Pick();

	static int count();

	/* const */ std::string id, net, sta, loc, cha;
	const Station *station() const { return _station.get(); }
	void setStation(const Station *sta) const;

	// get and set the origin this pick is associated with
	const Origin *origin() const { return _origin.get(); }
	void setOrigin(const Origin *org) const;

	Time time;		// pick time
	float amp;		// linear amplitude
	float per;		// period in seconds
	float snr;		// signal-to-noise ratio
	float normamp;		// normalized amplitude

	Status status;
	bool   xxl;		// Does it look like a very big event?

	Seiscomp::Core::BaseObjectCPtr attachement;

  private:
	mutable OriginPtr  _origin; // The (one and only) origin this pick is associated to
	mutable StationPtr _station;	// Station information
};


class Hypocenter : public Seiscomp::Core::BaseObject {

  public:

	Hypocenter(double lat, double lon, double dep)
		: lat(lat), lon(lon), dep(dep), laterr(0), lonerr(0), deperr(0) { }

	double lat, lon, dep;
	double laterr, lonerr, deperr;
};


class Arrival {

  public:

	Arrival(const Pick *pick, const std::string &phase="P", double residual=0);
	Arrival(const Origin *origin, const Pick *pick, const std::string &phase="P", double residual=0, double affinity=1);
	Arrival(const Arrival &);

	OriginCPtr origin;
	PickCPtr pick;
	std::string phase;
	float residual;
//	float weight;
	float distance;
	float azimuth;
	float affinity;
	float score;

	// XXX temporary:
	float dscore, ascore, tscore;

	// Reasons for why a pick may be excluded from the computation of an
	// origin. This allows exclusion of suspicious picks independent of
	// their weight, which remains unmodified.
	enum ExcludeReason {
		NotExcluded      = 0,

		// if a maximum residual is exceeded
		LargeResidual    = 1,

		// e.g. PKP if many stations in 0-100 deg distance range
		StationDistance  = 2,

		// Such a pick must remain excluded from the computation
		ManuallyExcluded = 4,

		// A pick that deteriorates the solution,
		// e.g. reduces score or badly increases RMS
		DeterioratesSolution = 8,

		// PP, SKP, SKS etc.
		UnusedPhase = 16,

		// pick temporarily excluded from the computation
		// XXX experimental XXX
		TemporarilyExcluded = 32
	};
	ExcludeReason excluded;
};


class ArrivalVector : public std::vector<Arrival> {
  public:
	bool sort();
};


class OriginQuality
{
  public:
	OriginQuality() : aziGapPrimary(360), aziGapSecondary(360) {}

	double aziGapPrimary;
	double aziGapSecondary;
};



class OriginErrorEllipsoid
{
  public:
	OriginErrorEllipsoid() : semiMajorAxis(0), semiMinorAxis(0), strike(0), sdepth(0), stime(0), sdobs(0), conf(0) {}

	double semiMajorAxis, semiMinorAxis, strike, sdepth, stime, sdobs, conf;
};



class Origin : public Hypocenter {

  public:
	enum ProcessingStatus { New, Updated, Deleted };

	enum LocationStatus {
		Automatic,              // purely automatic
		ConfirmedAutomatic,     // automatic, but confirmed
		SemiManual,             // manual with automatic additions
		Manual                  // purely manual
	};

	enum DepthType {
		DepthFree,              // depth is sufficiently well constrained by the data
		DepthPhases,            // depth is constrained by depth phases
		DepthMinimum,           // locator wants to put origin at zero depth -> use reasonable default
		DepthDefault,           // no depth resolution -> use reasonable default
		DepthManuallyFixed      // must not be overridden by autoloc
	};

  public:

	Origin(double lat, double lon, double dep, const Time &time);
	Origin(const Origin&);
	~Origin();

	static int count();

	void updateFrom(const Origin*);

	bool add(const Arrival &arr);

	// return index of arrival referencing pick, -1 if not found
	int findArrival(const Pick *pick) const;

	// count the defining phases, optionally within a distance range
	int phaseCount(double dmin=0., double dmax=180.) const;
	int definingPhaseCount(double dmin=0., double dmax=180.) const;

	// count the association stations
	int definingStationCount() const;
	int associatedStationCount() const;

	double rms() const;
	double medianStationDistance() const;

	void geoProperties(double &min, double &max, double &gap) const;

  public:
	OriginID id;

	bool imported;
	bool preliminary;
	std::string publicID;

	std::string methodID;
	std::string earthModelID;

	ProcessingStatus processingStatus;
	LocationStatus locationStatus;

	double score;

	// lat,lon,dep are inherited from Hypocenter
	Time time, timestamp;
	double timeerr;
	DepthType depthType;
	ArrivalVector arrivals;
	OriginQuality quality;
	OriginErrorEllipsoid errorEllipsoid;

  public:
	Seiscomp::DataModel::OriginCPtr sc3manual;
};


class OriginDB : public std::vector<OriginPtr> {

  public:
	bool find(const Origin *) const;
	Origin *find(const OriginID &id);

	// Try to find the best Origin which possibly belongs to the same event
	const Origin *bestEquivalentOrigin(const Origin *start) const;

	// try to find Origins which possibly belong to the same event and try
	// to merge the picks
	int mergeEquivalentOrigins(const Origin *start=0);
};


DEFINE_SMARTPOINTER(Event);

class Event : public Seiscomp::Core::BaseObject {

  public:

    OriginDB origin;
};


typedef std::vector<PickPtr> PickDB;
typedef std::vector<Pick*>   PickGroup;

}  // namespace Autoloc

#endif
