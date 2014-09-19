/***************************************************************************
 * Copyright (C) 2013 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#ifndef __GEMPA_INVMGR_CHECK_H__
#define __GEMPA_INVMGR_CHECK_H__


#include <seiscomp3/core/timewindow.h>

#include <list>
#include <map>

#include "inv.h"


//! \brief Check class that checks consistency of an inventory.
class Check : public InventoryTask {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		//! Creates a check object with a target inventory
		//! which holds the final merged inventory
		Check(Seiscomp::DataModel::Inventory *inv);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		typedef std::list<Seiscomp::Core::TimeWindow> TimeWindows;
		typedef std::map<std::string, TimeWindows> EpochMap;

		bool check();


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		template <typename T>
		void checkEpoch(const T *obj);

		template <typename T>
		void checkOverlap(TimeWindows &epochs, const T *obj);

		template <typename T1, typename T2>
		void checkOutside(const T1 *parent, const T2 *obj);

		Seiscomp::DataModel::Sensor *findSensor(const std::string &) const;
		Seiscomp::DataModel::ResponsePAZ *findPAZ(const std::string &) const;
		Seiscomp::DataModel::ResponsePolynomial *findPoly(const std::string &) const;
};


#endif
