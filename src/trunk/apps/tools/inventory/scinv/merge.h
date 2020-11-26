/***************************************************************************
 * Copyright (C) 2011 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#ifndef __GEMPA_INVMGR_MERGE_H__
#define __GEMPA_INVMGR_MERGE_H__


#include <list>
#include "inv.h"


//! \brief Merge class that merges multiple inventories into one.
class Merge : public InventoryTask {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		//! Creates a merge objects with a target inventory
		//! which holds the final merged inventory
		Merge(Seiscomp::DataModel::Inventory *inv);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Push an inventory object to be merged
		//! All childs of the passed inventory will be moved into an
		//! internal inventory. After the call inv should be empty.
		//! The ownership of all childs goes to this instance.
		bool push(Seiscomp::DataModel::Inventory *inv, int id);
		bool merge(bool stripUnreferenced);


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		bool process(const Seiscomp::DataModel::StationGroup *);
		bool process(Seiscomp::DataModel::StationGroup *,
		             const Seiscomp::DataModel::StationReference *);
		bool process(const Seiscomp::DataModel::Network *);
		bool process(Seiscomp::DataModel::Network *,
		             const Seiscomp::DataModel::Comment *);
		bool process(Seiscomp::DataModel::Network *,
		             const Seiscomp::DataModel::Station *);
		bool process(Seiscomp::DataModel::Station *,
		             const Seiscomp::DataModel::Comment *);
		bool process(Seiscomp::DataModel::Station *,
		             const Seiscomp::DataModel::SensorLocation *);
		bool process(Seiscomp::DataModel::SensorLocation *,
		             const Seiscomp::DataModel::Comment *);
		bool process(Seiscomp::DataModel::SensorLocation *,
		             const Seiscomp::DataModel::Stream *);
		bool process(Seiscomp::DataModel::Stream *,
		             const Seiscomp::DataModel::Comment *);
		bool process(Seiscomp::DataModel::Stream *,
		             const Seiscomp::DataModel::Datalogger *);
		bool process(Seiscomp::DataModel::Datalogger *,
	                 const Seiscomp::DataModel::Decimation *);
		bool process(Seiscomp::DataModel::Datalogger *,
	                 const Seiscomp::DataModel::DataloggerCalibration *);
		bool process(Seiscomp::DataModel::Stream *,
		             const Seiscomp::DataModel::Sensor *);
		bool process(Seiscomp::DataModel::Sensor *,
		             const Seiscomp::DataModel::SensorCalibration *);
		bool process(Seiscomp::DataModel::SensorLocation *,
		             const Seiscomp::DataModel::AuxStream *);
		bool process(Seiscomp::DataModel::AuxStream *,
		             const Seiscomp::DataModel::AuxDevice *);
		bool process(Seiscomp::DataModel::AuxDevice *,
		             const Seiscomp::DataModel::AuxSource *);

		Seiscomp::DataModel::ResponseFIR *
		process(const Seiscomp::DataModel::ResponseFIR *);
		Seiscomp::DataModel::ResponsePAZ *
		process(const Seiscomp::DataModel::ResponsePAZ *);
		Seiscomp::DataModel::ResponsePolynomial *
		process(const Seiscomp::DataModel::ResponsePolynomial *);
		Seiscomp::DataModel::ResponseFAP *
		process(const Seiscomp::DataModel::ResponseFAP *);
		Seiscomp::DataModel::ResponseIIR *
		process(const Seiscomp::DataModel::ResponseIIR *);

		bool compareDatalogger(std::string &finalID,
		                       const std::string &id1,
		                       const std::string &id2);

		bool compareSensor(std::string &finalID,
		                   const std::string &id1,
		                   const std::string &id2);

	// ------------------------------------------------------------------
	//  Private member
	// ------------------------------------------------------------------
	private:
		Seiscomp::DataModel::InventoryPtr _tmpInv;
		IDMap                             _stationIDMap;
		bool                              _stripUnreferenced;
};


#endif
