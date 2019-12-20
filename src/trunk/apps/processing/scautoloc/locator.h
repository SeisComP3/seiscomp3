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




#ifndef SEISCOMP_AUTOLOC_LOCATOR
#define SEISCOMP_AUTOLOC_LOCATOR
#include <string>
#include <map>

#include <seiscomp3/seismology/locsat.h>
#include "datamodel.h"

namespace Autoloc {

DEFINE_SMARTPOINTER(MySensorLocationDelegate);

class MySensorLocationDelegate : public Seiscomp::Seismology::SensorLocationDelegate
{
	public:
		~MySensorLocationDelegate();
	public:
		virtual Seiscomp::DataModel::SensorLocation* getSensorLocation(Seiscomp::DataModel::Pick *pick) const;
		void setStation(const Station *station);
	private:
		typedef std::map<std::string, Seiscomp::DataModel::SensorLocationPtr> SensorLocationList;
		SensorLocationList _sensorLocations;
};

class Locator {
	public:
		Locator();
		~Locator();

		bool init();
		void setStation(const Station *station);
		void setMinimumDepth(double);

		void setFixedDepth(double depth, bool use=true) {
			_sc3locator->setFixedDepth(depth, use);
		}

		void useFixedDepth(bool use=true) {
			_sc3locator->useFixedDepth(use);
		}

		void setProfile(const std::string &name) {
			_sc3locator->setProfile(name);
		}

	public:
		Origin *relocate(const Origin *origin);


	private:
		// this is the SC3-level relocate
		Origin *_sc3relocate(const Origin *origin);

	private:
		Seiscomp::Seismology::LocatorInterfacePtr _sc3locator;

		MySensorLocationDelegatePtr sensorLocationDelegate;

		double _minDepth;

		size_t _locatorCallCounter;
};



bool determineAzimuthalGaps(const Origin*, double *primary, double *secondary);

}  // namespace Autoloc

#endif
