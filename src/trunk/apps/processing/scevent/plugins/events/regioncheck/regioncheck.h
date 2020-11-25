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
 *                                                                         *
 *   Author: Jan Becker, Dirk Roessler, gempa GmbH                         *
 ***************************************************************************/

#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/geo/featureset.h>
#include <seiscomp3/plugins/events/eventprocessor.h>
#include <seiscomp3/seismology/regions.h>

namespace Seiscomp {
namespace DataModel {

class Event;
class Origin;

}

class RegionCheckProcessor : public Seiscomp::Client::EventProcessor {
	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		RegionCheckProcessor();


	// ----------------------------------------------------------------------
	// Public EventProcessor interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Setup the processor and read settings from the passed
		 *        configuration.
		 * @param config The config object to read settings from
		 * @return Success flag
		 */
		bool setup(const Config::Config &config);


		/**
		 * @brief Processes and modifies an event. It checks the current
		 *        location against the configured regions and sets the
		 *        type to OUTSIDE_OF_NETWORK_INTEREST if the location is not
		 *        inside in any of the regions.
		 * @param event The event to be processed
		 * @return Update flag: true, if the event has been updated, false
		 *         otherwise.
		 */
		bool process(DataModel::Event *event, const Journal &journal);


	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		typedef std::pair<Seiscomp::Geo::GeoFeature*,bool> RegionCheck;
		std::vector<RegionCheck>  _regions;
		OPT(DataModel::EventType) _eventTypePositive;
		OPT(DataModel::EventType) _eventTypeNegative;
		bool                      _hasPositiveRegions;
		bool                      _hasNegativeRegions;
		bool                      _setType;
		bool                      _readEventTypeFromBNA;
};

}
