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


#ifndef __SYNC_STAXML_CONVERT2SC3_H__
#define __SYNC_STAXML_CONVERT2SC3_H__


#include "converter.h"

#include <list>
#include <set>


namespace Seiscomp {

namespace StationXML {

class StaMessage;
class Station;
class StationEpoch;
class Channel;
class ChannelEpoch;
class Response;

}


namespace DataModel {

class Object;
class Inventory;
class Network;
class Station;
class SensorLocation;
class Stream;
class Sensor;
class SensorCalibration;
class Datalogger;
class DataloggerCalibration;
class Decimation;

}


//! \brief Converter class for StationXML -> SC3 that works on an
//! \brief inventory and merges all changes of pushed StationXML
//! \brief messages into it.
class Convert2SC3 : public Converter {
	// ------------------------------------------------------------------
	//  Public datatypes
	// ------------------------------------------------------------------
	public:
		typedef std::pair<std::string, std::string> StringTuple;
		typedef std::set<StringTuple> TupleSet;

		typedef std::pair<std::string, Core::Time> EpochIndex;

		typedef EpochIndex NetworkIndex;
		typedef std::set<NetworkIndex> NetworkSet;
		typedef std::pair<NetworkIndex, EpochIndex> StationIndex;
		typedef std::set<StationIndex> StationSet;
		typedef std::pair<StationIndex, EpochIndex> LocationIndex;
		typedef std::set<LocationIndex> LocationSet;
		typedef std::pair<LocationIndex, EpochIndex> StreamIndex;
		typedef std::set<StreamIndex> StreamSet;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Convert2SC3(DataModel::Inventory *inv);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Enables logging of changed stations. The caller must
		//! provide an instance to a station set which is filled
		//! by the converter.
		bool push(const StationXML::StaMessage *msg);

		//! Cleans up unused responses and epochs
		void cleanUp();

		const TupleSet &visitedStations() const;

		const NetworkSet &touchedNetworks() const;
		const StationSet &touchedStations() const;


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		bool process(DataModel::Network *,
		             const StationXML::Station *, const StationXML::StationEpoch *);

		bool process(DataModel::SensorLocation *,
		             const StationXML::Channel *, const StationXML::ChannelEpoch *,
		             bool isRestricted);

		bool process(DataModel::Datalogger *, DataModel::Stream *,
		             const StationXML::ChannelEpoch *);

		bool process(DataModel::Sensor *, DataModel::Stream *,
		             const StationXML::ChannelEpoch *,
		             const StationXML::Response *);


		DataModel::Datalogger *
		updateDatalogger(const std::string &name, const StationXML::ChannelEpoch *);

		DataModel::Decimation *
		updateDecimation(DataModel::Datalogger *, DataModel::Stream *,
		                 const StationXML::ChannelEpoch *);

		DataModel::DataloggerCalibration *
		updateDataloggerCalibration(DataModel::Datalogger *, DataModel::Stream *,
		                            const StationXML::ChannelEpoch *);

		void
		updateDataloggerDigital(DataModel::Datalogger *, DataModel::Decimation *,
		                        const StationXML::ChannelEpoch *);

		void
		updateDataloggerAnalogue(DataModel::Datalogger *, DataModel::Decimation *,
		                         const StationXML::ChannelEpoch *);

		DataModel::Sensor *
		updateSensor(const std::string &name,
		             const StationXML::ChannelEpoch *,
		             const StationXML::Response *resp);

		DataModel::SensorCalibration *
		updateSensorCalibration(DataModel::Sensor *, DataModel::Stream *,
		                        const StationXML::ChannelEpoch *,
		                        const StationXML::Response *resp);


		const StationXML::Response *findDataloggerResponse() const;

		DataModel::Datalogger *pushDatalogger(DataModel::Datalogger *);
		DataModel::Sensor *pushSensor(DataModel::Sensor *);

	// ------------------------------------------------------------------
	//  Members
	// ------------------------------------------------------------------
	private:
		typedef std::map<std::string, const DataModel::Object*> ObjectLookup;
		typedef std::list<const StationXML::Response*> Responses;
		typedef std::set<std::string> StringSet;

		DataModel::Inventory *_inv;

		// Tracks all network/station codes of visited stations.
		// An empty station code referes to a network.
		TupleSet              _visitedStations;

		// Tracks all modified networks, station, sensor locations and channels
		NetworkSet            _touchedNetworks;
		StationSet            _touchedStations;
		LocationSet           _touchedSensorLocations;
		StreamSet             _touchedStreams;
		StreamSet             _touchedAuxStreams;

		ObjectLookup          _dataloggerLookup;
		ObjectLookup          _sensorLookup;

		// List of responses of a certain channel epoch
		Responses             _responses;
};


}


#endif
