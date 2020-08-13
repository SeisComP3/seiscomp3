/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef SEISCOMP_SEISMOLOGY_FIXED_HYPOCENTER_H
#define SEISCOMP_SEISMOLOGY_FIXED_HYPOCENTER_H


#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/seismology/locatorinterface.h>
#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/core.h>

#include <vector>


namespace Seiscomp{
namespace Seismology {


class SC_SYSTEM_CORE_API FixedHypocenter : public LocatorInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		FixedHypocenter();


	// ----------------------------------------------------------------------
	//  Locator interface
	// ----------------------------------------------------------------------
	public:
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

		virtual int capabilities() const;

		virtual DataModel::Origin *locate(PickList& pickList);
		virtual DataModel::Origin *locate(PickList& pickList,
		                                  double initLat, double initLon, double initDepth,
		                                  const Seiscomp::Core::Time& initTime);

		virtual DataModel::Origin *relocate(const DataModel::Origin* origin);


	private:
		// Configuration
		IDList      _profiles;
		int         _degreesOfFreedom;
		double      _confidenceLevel;
		double      _defaultTimeError;
		bool        _usePickUncertainties;
		bool        _verbose;
		std::string _lastError;

		// Runtime
		TravelTimeTableInterfacePtr _ttt;
};


}
}


#endif
