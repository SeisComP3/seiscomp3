/***************************************************************************
 * Copyright (C) 2011 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#ifndef __GEMPA_INVMGR_SYNC_H__
#define __GEMPA_INVMGR_SYNC_H__


#include <list>

#include "inv.h"


//! \brief Sync class that merges inventories into an existing
//! \brief inventory.
class Sync : public InventoryTask {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		//! Creates a merge objects with a target inventory
		//! which holds the final merged inventory
		Sync(Seiscomp::DataModel::Inventory *inv);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Push an inventory object to be synched
		bool push(const Seiscomp::DataModel::Inventory *inv);

		//! Cleans up unused responses and epochs
		void cleanUp();


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		bool process(const Seiscomp::DataModel::StationGroup *);
		bool process(Seiscomp::DataModel::StationGroup *,
		             const Seiscomp::DataModel::StationReference *);
		bool process(const Seiscomp::DataModel::Network *);
		bool process(Seiscomp::DataModel::Network *,
		             const Seiscomp::DataModel::Station *);
		bool process(Seiscomp::DataModel::Station *,
		             const Seiscomp::DataModel::SensorLocation *);
		bool process(Seiscomp::DataModel::SensorLocation *,
		             const Seiscomp::DataModel::Stream *);
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


	// ------------------------------------------------------------------
	//  Private member
	// ------------------------------------------------------------------
	private:
		typedef std::set<Seiscomp::DataModel::Object*> ObjectSet;

		// Tracks all touched objects. Untouched objects will be removed
		ObjectSet _touchedObjects;
		IDMap     _stationIDMap;
};


#endif
