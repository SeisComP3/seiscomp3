/***************************************************************************
 * Copyright (C) 2011 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#ifndef __GEMPA_INVMGR_INV_H__
#define __GEMPA_INVMGR_INV_H__


#include <seiscomp3/datamodel/inventory_package.h>

#include <map>
#include <set>
#include <string>
#include <iostream>

#include "task.h"


//! Handler class for log entries created during sync, merge, ...
struct LogHandler {
	virtual ~LogHandler() {}

	enum Level {
		Debug,
		Information,
		Conflict,
		Unresolved,
		Warning,
		Error
	};

	/**
	 * Publishes the log entry for a certain operation. Passed are
	 * additionally two objects which are e.g. conflicting.
	 * @param level   The log level for this message
	 * @param message The message text
	 * @param obj1    The first object which can be a NULL pointer
	 * @param obj2    The second object which can be a NULL pointer
	 */
	virtual void publish(Level level, const char *message,
	                     const Seiscomp::DataModel::Object *obj1,
	                     const Seiscomp::DataModel::Inventory *source1,
	                     const Seiscomp::DataModel::Object *obj2,
	                     const Seiscomp::DataModel::Inventory *source2) = 0;
};


class InventoryTask : public Task {
	public:
		typedef Seiscomp::DataModel::Object Object;
		typedef Seiscomp::DataModel::Inventory Inventory;
		typedef std::map<const Object*,Inventory*> SourceMap;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		InventoryTask(Seiscomp::DataModel::Inventory *inv);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setLogHandler(LogHandler *handler);

		const Seiscomp::DataModel::Datalogger * findDatalogger(const std::string &) const;
		const Seiscomp::DataModel::Sensor *findSensor(const std::string &) const;
		const Seiscomp::DataModel::AuxDevice *findAuxDevice(const std::string &) const;
		const Seiscomp::DataModel::ResponseFIR *findFIR(const std::string &) const;
		const Seiscomp::DataModel::ResponsePAZ *findPAZ(const std::string &) const;
		const Seiscomp::DataModel::ResponsePolynomial *findPoly(const std::string &) const;
		const Seiscomp::DataModel::ResponseFAP *findFAP(const std::string &) const;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		Seiscomp::DataModel::Datalogger *dataloggerByName(const std::string &) const;
		Seiscomp::DataModel::Sensor *sensorByName(const std::string &) const;
		Seiscomp::DataModel::AuxDevice *auxDeviceByName(const std::string &) const;
		Seiscomp::DataModel::ResponseFIR *respFIRByName(const std::string &) const;
		Seiscomp::DataModel::ResponsePAZ *respPAZByName(const std::string &) const;
		Seiscomp::DataModel::ResponseFAP *respFAPByName(const std::string &) const;
		Seiscomp::DataModel::ResponsePolynomial *respPolynomialByName(const std::string &) const;

		void log(LogHandler::Level level, const char *message,
		         const Seiscomp::DataModel::Object *obj1,
		         const Seiscomp::DataModel::Object *obj2);

		void prepareSession(const Seiscomp::DataModel::Inventory *inv);
		void cleanUp();

		template <typename T>
		T *create(const std::string &publicIDHint);

		// Updates a response in the target inventory or adds it
		// if required. Returns the updated or added response where
		// the input publicID can differ from the output
		Seiscomp::DataModel::ResponseFIR *
		process(const Seiscomp::DataModel::ResponseFIR *);
		Seiscomp::DataModel::ResponsePAZ *
		process(const Seiscomp::DataModel::ResponsePAZ *);
		Seiscomp::DataModel::ResponsePolynomial *
		process(const Seiscomp::DataModel::ResponsePolynomial *);
		Seiscomp::DataModel::ResponseFAP *
		process(const Seiscomp::DataModel::ResponseFAP *);


	// ------------------------------------------------------------------
	//  Protected types and members
	// ------------------------------------------------------------------
	protected:
		typedef std::map<std::string, const Object*> ObjectLookup;
		typedef std::map<std::string, std::string> IDMap;
		typedef std::set<const Object*> ObjectSet;

		struct Session {
			ObjectLookup dataloggerLookup;
			ObjectLookup sensorLookup;
			ObjectLookup auxDeviceLookup;
			ObjectLookup firLookup;
			ObjectLookup pazLookup;
			ObjectLookup polyLookup;
			ObjectLookup fapLookup;
			ObjectSet    touchedPublics;
		};

		Inventory    *_inv;
		ObjectLookup  _publicObjects;
		ObjectLookup  _dataloggerNames;
		ObjectLookup  _sensorNames;
		ObjectLookup  _auxDeviceNames;
		ObjectLookup  _FIRNames;
		ObjectLookup  _PAZNames;
		ObjectLookup  _PolyNames;
		ObjectLookup  _FAPNames;

		Session       _session;

		LogHandler   *_logHandler;
		SourceMap     _sources;
};



template <typename T>
T *InventoryTask::create(const std::string &publicIDHint) {
	T *obj;
	if ( _publicObjects.find(publicIDHint) == _publicObjects.end() &&
	     (!Seiscomp::DataModel::PublicObject::IsRegistrationEnabled() ||
	      Seiscomp::DataModel::PublicObject::Find(publicIDHint) == NULL) )
		obj = T::Create(publicIDHint);
	else {
		SEISCOMP_DEBUG("Cannot create %s", publicIDHint.c_str());
		obj = T::Create();
	}

	_publicObjects[obj->publicID()] = obj;
	return obj;
}


#endif
