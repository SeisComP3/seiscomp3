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
 *   Author: Jan Becker, gempa GmbH                                        *
 ***************************************************************************/


#define SEISCOMP_COMPONENT REGIONCHECK

#include <seiscomp3/core/plugin.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/plugins/events/eventprocessor.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/geo/geofeatureset.h>


ADD_SC_PLUGIN("Region check for events that sets the event type to \"outside of network interest\" "
              "if the location is inside preconfigured regions",
              "Jan Becker, gempa GmbH <jabe@gempa.de>", 0, 1, 0)


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Geo;


class RegionCheckProcessor : public Seiscomp::Client::EventProcessor {
	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		RegionCheckProcessor() {}


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
		bool setup(const Config::Config &config) {
			const GeoFeatureSet &featureSet = GeoFeatureSetSingleton::getInstance();
			const vector<GeoFeature*> &features = featureSet.features();

			_hasPositiveRegions = false;
			_hasNegativeRegions = false;

			try {
				vector<string> regions = config.getStrings("rc.regions");
				if ( regions.empty() )
					throw Core::ValueException();

				for ( size_t i = 0; i < regions.size(); ++i ) {
					string regionName = regions[i];
					Core::trim(regionName);

					if ( regionName.empty() ) {
						SEISCOMP_ERROR("RC: empty region names are not allowed");
						return false;
					}

					if ( regionName == "accept" ) {
						_regions.push_back(RegionCheck(NULL, true));
						_hasPositiveRegions = true;
						SEISCOMP_DEBUG("RC: add world as accept region");
					}
					else if ( regionName == "reject" ) {
						_regions.push_back(RegionCheck(NULL, false));
						_hasNegativeRegions = true;
						SEISCOMP_DEBUG("RC: add world as reject region");
					}
					else {
						bool foundRegion = false;
						bool positiveRegion = true;

						if ( regionName[0] == '!' ) {
							positiveRegion = false;
							regionName.erase(0,1);
							Core::trim(regionName);
						}

						// Check for available BNA regions
						for ( vector<GeoFeature*>::const_iterator it = features.begin();
						      it != features.end(); ++it ) {
							string featureName = (*it)->name();
							Core::trim(featureName);

							if ( regionName != featureName ) continue;

							if ( foundRegion ) {
								SEISCOMP_ERROR("RC: multiple definition of region '%s' in .bna file. "
								               "Check the parameter regions or the .bna files in the BNA directories!",
								               featureName.c_str());
								return false;
							}

							if ( !(*it)->closedPolygon() ) {
								SEISCOMP_ERROR("RC: region '%s' unused. Polygon is not closed. "
								               "Check the .bna files in the BNA directories!",
								               featureName.c_str());
								return false;
							}

							SEISCOMP_INFO("RC: add %s region '%s'",
							              positiveRegion ? "include":"exclude",
							              featureName.c_str());
							_regions.push_back(RegionCheck(*it, positiveRegion));

							if ( positiveRegion )
								_hasPositiveRegions = true;
							else
								_hasNegativeRegions = true;

							foundRegion = true;
						}

						if ( !foundRegion ) {
							SEISCOMP_ERROR("RC: region '%s' has not been found",
							               regionName.c_str());
							return false;
						}
					}
				}
			}
			catch ( ... ) {
				SEISCOMP_ERROR("RC: No regions configured, region check is useless");
				return false;
			}

			return true;
		}


		/**
		 * @brief Processes and modifies an event. It checks the current
		 *        location against the configured regions and sets the
		 *        type to OUTSIDE_OF_NETWORK_INTEREST if the location is not
		 *        inside in any of the regions.
		 * @param event The event to be processed
		 * @return Update flag: true, if the event has been updated, false
		 *         otherwise.
		 */
		bool process(Event *event) {
			Origin *org = Origin::Find(event->preferredOriginID());

			if ( !org ) {
				SEISCOMP_WARNING("%s: RC: no origin information",
				                 event->publicID().c_str());
				return false;
			}

			Vertex location;

			try {
				location.set(org->latitude().value(), org->longitude().value());
			}
			catch ( ... ) {
				SEISCOMP_WARNING("%s: RC: no lat/lon information available",
				                 event->publicID().c_str());
				return false;
			}

			SEISCOMP_DEBUG("%s: RC: checking regions for location %f / %f",
			               event->publicID().c_str(), location.lat, location.lon);

			OPT(EventType) currentType;

			try {
				currentType = event->type();
			}
			catch ( ... ) {}

			bool isInside = false;

			for ( size_t i = 0; i < _regions.size(); ++i ) {
				bool isInsideRegion;

				if ( _regions[i].first )
					isInsideRegion = _regions[i].first->contains(location);
				else
					isInsideRegion = true;

				if ( _regions[i].second ) {
					// Positive region
					if ( isInsideRegion ) {
						// Inside of network interest
						isInside = true;
						SEISCOMP_DEBUG("%s: RC: region '%s' matches",
						               event->publicID().c_str(),
						               (_regions[i].first ? _regions[i].first->name().c_str() : "world"));

						// If there are possible negative regions following this
						// match then continue.
						if ( !_hasNegativeRegions )
							break;
					}
				}
				else {
					// Negative region
					if ( isInsideRegion ) {
						// Outside of network interest
						isInside = false;
						SEISCOMP_DEBUG("%s: RC: region '%s' matches and it is a negative region",
						               event->publicID().c_str(),
						               (_regions[i].first ? _regions[i].first->name().c_str() : "world"));

						// Continue with checking as there might follow a positive
						// region that overrides again that result unless there
						// are only negative regions configured
						if ( !_hasPositiveRegions )
							break;
					}
				}
			}

			if ( !isInside ) {
				if ( !currentType || (currentType && *currentType != OUTSIDE_OF_NETWORK_INTEREST) ) {
					SEISCOMP_DEBUG("%s: RC: event is out of network interest",
					               event->publicID().c_str());
					event->setType(EventType(OUTSIDE_OF_NETWORK_INTEREST));
					return true;
				}
			}
			else {
				if ( currentType && *currentType == OUTSIDE_OF_NETWORK_INTEREST ) {
					SEISCOMP_DEBUG("%s: RC: event is inside of network interest: removing type",
					               event->publicID().c_str());
					event->setType(Core::None);
					return true;
				}
			}

			return false;
		}


	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		typedef pair<GeoFeature*,bool> RegionCheck;
		vector<RegionCheck> _regions;
		bool                _hasPositiveRegions;
		bool                _hasNegativeRegions;
};


REGISTER_EVENTPROCESSOR(RegionCheckProcessor, "RegionCheck");
