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
 * Authors of the Software: Jan Becker, Yannik Behr and Stefan Heimers
 * Copyright (C) 2006-2013 by Swiss Seismological Service
 ***************************************************************************/

#define SEISCOMP_COMPONENT VsMagnitude

#include <seiscomp3/logging/filerotator.h>
#include <seiscomp3/logging/channel.h>
#include <seiscomp3/logging/fd.h>
#include <seiscomp3/logging/log.h>

#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/timewindow.h>

#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/datamodel/vs/vs_package.h>
#include <seiscomp3/datamodel/magnitude.h>

#include <seiscomp3/client/inventory.h>
#include <seiscomp3/client/application.h>

#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordinput.h>

#include <seiscomp3/processing/application.h>

#include <seiscomp3/math/geo.h>

#include <algorithm>
#include <sstream>

#include "util.h"
#include "timeline.h"
#include "Vs30Mapping.h"
#include "VsSiteCondition.h"

namespace Seiscomp {

class VsTimeWindow: public Core::TimeWindow {
private:
	Core::Time _pickTime;
public:
	Core::Time pickTime() const {
		return _pickTime;
	}
	void setPickTime(const Core::Time &t)
	{
		_pickTime = t;
	}
};

class VsMagnitude: public Client::Application {
public:
	VsMagnitude(int argc, char **argv);
	~VsMagnitude();

protected:
	typedef std::map<Timeline::StationID, VsTimeWindow> VsWindows;
	typedef std::set<Timeline::StationID> StationList;

	DEFINE_SMARTPOINTER(VsEvent);
	struct VsEvent: Core::BaseObject {
		double lat;
		double lon;
		double dep;
		Core::Time time;
		Core::Time expirationTime;
		VsWindows stations;
		OPT(double) vsMagnitude;
		Core::Time originArrivalTime;
		Core::Time originCreationTime;
		int vsStationCount;
		StationList pickedStations; // all stations contributing picks to an origin
		int pickedStationsCount;
		int pickedThresholdStationsCount;
		int allThresholdStationsCount;
		bool isValid; // set to true or false by quality control in VsMagnitude::process(VsEvent *evt)
		double dthresh;
		int update;
		double likelihood;
	};

	typedef std::map<std::string, VsEventPtr> VsEvents;
	typedef std::map<std::string, Core::Time> EventIDBuffer;
	typedef DataModel::PublicObjectTimeSpanBuffer Cache;

	void createCommandLineDescription();
	bool initConfiguration();
	bool init();
	bool validateParameters();

	void handleMessage(Core::Message* msg);
	void handleTimeout();

	void updateObject(const std::string &parentID, DataModel::Object *obj);
	void addObject(const std::string &parentID, DataModel::Object *obj);
	void removeObject(const std::string &parentID, DataModel::Object *obj);

	void handleEvent(DataModel::Event *event);
	void handlePick(DataModel::Pick *pk);
	void handleOrigin(DataModel::Origin *og);

	void processEvents();
	void process(VsEvent *event, std::string eventID);
	void updateVSMagnitude(DataModel::Event *event, VsEvent *vsevt);
	template<typename T>
	bool setComments(DataModel::Magnitude *mag, const std::string id,
			const T value);

	// implemented in qualitycontrol.cpp
	bool isEventValid(double stmag, VsEvent *evt, double &likelihood,
			double &deltamag, double &deltapick);
	double deltaMag(double vsmag, double stmag); // for Quality Control
	double deltaPick(VsEvent *evt); // ratio between the stations that reported a pick to the overall number of stations within a given radius.

	/**
	 Returns the site correction scale.
	 */
	ch::sed::VsSiteCondition::SafList _saflist;
	float siteEffect(double lat, double lon, double MA, ValueType valueType,
			SoilClass &soilClass);

private:
	Cache _cache;
	EventIDBuffer _publishedEvents;
	Timeline _timeline;
	VsEvents _events;
	Core::Time _currentTime;
	Core::Time _lastProcessingTime;
	DataModel::CreationInfo _creationInfo;

	Logging::Channel* _processingInfoChannel;
	Logging::Output* _processingInfoFile;
	Logging::FdOutput* _processingInfoOutput;
	Logging::Channel* _envelopeInfoChannel;
	Logging::Output* _envelopeInfoFile;

	// Configuration
	std::string _vs30filename;
	std::string _proclogfile;
	int _backSlots;
	int _headSlots;
	bool _realtime;
	int _twstarttime;
	int _twendtime;
	int _eventExpirationTime; // number seconds after origin time the calculation of vsmagnitudes is stopped
	std::string _expirationTimeReference;
	double _vs30default;
	int _clipTimeout;
	int _timeout;
	bool _siteEffect; // turn site effects on or off
	double _maxepicdist;
	bool _logenvelopes;
};

}
