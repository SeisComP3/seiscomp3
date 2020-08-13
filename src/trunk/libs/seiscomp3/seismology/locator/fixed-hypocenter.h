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
		virtual bool init(const Config::Config &config) override;

		//! Returns supported parameters to be changed.
		virtual IDList parameters() const override;

		//! Returns the value of a parameter.
		virtual std::string parameter(const std::string &name) const override;

		//! Sets the value of a parameter.
		virtual bool setParameter(const std::string &name,
		                          const std::string &value) override;

		virtual IDList profiles() const override;
		virtual void setProfile(const std::string &name) override;

		virtual int capabilities() const override;

		virtual DataModel::Origin *locate(PickList& pickList) override;
		virtual DataModel::Origin *locate(PickList& pickList,
		                                  double initLat, double initLon, double initDepth,
		                                  const Seiscomp::Core::Time& initTime) override;

		virtual DataModel::Origin *relocate(const DataModel::Origin* origin) override;


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
