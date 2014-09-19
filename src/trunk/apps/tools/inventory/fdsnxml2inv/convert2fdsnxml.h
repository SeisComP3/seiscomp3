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


#ifndef __SYNC_STAXML_CONVERT2STAXML_H__
#define __SYNC_STAXML_CONVERT2STAXML_H__


#include "converter.h"

#include <list>
#include <map>
#include <string>


namespace Seiscomp {

namespace FDSNXML {

class FDSNStationXML;
class Network;
class Station;
class Channel;
class Response;

}


namespace DataModel {

class Object;
class Inventory;
class Station;
class SensorLocation;
class Stream;
class Sensor;
class Datalogger;
class ResponseFIR;
class ResponsePAZ;
class ResponsePolynomial;

}


//! \brief Converter class for SC3 -> StationXML  that works on a
//! \brief StationXML message and merges all pushed inventories.
class Convert2FDSNStaXML : public Converter {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Convert2FDSNStaXML(FDSNXML::FDSNStationXML *msg);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		bool push(const DataModel::Inventory *inv);


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		bool process(FDSNXML::Network *, const DataModel::Station *);
		bool process(FDSNXML::Station *,
		             const DataModel::SensorLocation *,
		             const DataModel::Stream *);
		bool process(FDSNXML::Channel *,
		             const DataModel::Stream *,
		             const DataModel::Datalogger *);
		bool process(FDSNXML::Channel *,
		             const DataModel::Stream *,
		             const DataModel::Sensor *);

		const DataModel::Datalogger *findDatalogger(const std::string &publicID);
		const DataModel::Sensor *findSensor(const std::string &publicID);
		const DataModel::ResponseFIR *findFIR(const std::string &publicID);
		const DataModel::ResponsePAZ *findPAZ(const std::string &publicID);
		const DataModel::ResponsePolynomial *findPoly(const std::string &publicID);


	// ------------------------------------------------------------------
	//  Members
	// ------------------------------------------------------------------
	private:
		typedef std::map<std::string, const DataModel::Object*> ObjectLookup;

		ObjectLookup                _dataloggerLookup;
		ObjectLookup                _sensorLookup;
		ObjectLookup                _firLookup;
		ObjectLookup                _pazLookup;
		ObjectLookup                _polyLookup;

		FDSNXML::FDSNStationXML    *_msg;
		const DataModel::Inventory *_inv;
};


}


#endif
