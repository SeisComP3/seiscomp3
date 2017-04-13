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


#define SEISCOMP_COMPONENT MMI

#include <seiscomp3/core/plugin.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/plugins/events/eventprocessor.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <cmath>


ADD_SC_PLUGIN("Maxmimum Mercalli Intensity stored in an event comment ported "
              "from code developed by GNS Science/New Zealand.",
              "Jan Becker, gempa GmbH <jabe@gempa.de>", 0, 2, 0)


using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;


class MMIProcessor : public Seiscomp::Client::EventProcessor {
	public:
		MMIProcessor() {
			maxMM = 10;
			minMM = 3;

			a1[0] = 4.40;    // constant
			a2[0] = 1.26;    // Mw
			a3[0] = -3.67;   // logD
			a4[0] = 0.012;   // depth
			a5[0] = 0.409;   // interface/crustal
			ad[0] = 11.78;   // dummy

			a1[1] = 3.76;    // constant
			a2[1] = 1.48;    // Mw
			a3[1] = -3.50;   // logD
			a4[1] = 0.0031;  // depth
			a5[1] = 0.0;     // interface/crustal
			ad[1] = 0.0;     // dummy

			depth0 = 70.0;
		}

	public:
		bool setup(const Seiscomp::Config::Config &config) {
			config.getInt(minMM, "MMI.lowerBound");
			config.getInt(maxMM, "MMI.upperBound");

			config.getDouble(a1[0], "MMI.const0");
			config.getDouble(a2[0], "MMI.Mw0");
			config.getDouble(a3[0], "MMI.logD0");
			config.getDouble(a4[0], "MMI.depth0");
			config.getDouble(a5[0], "MMI.crustal0");
			config.getDouble(ad[0], "MMI.dummy0");

			config.getDouble(a1[1], "MMI.const1");
			config.getDouble(a2[1], "MMI.Mw1");
			config.getDouble(a3[1], "MMI.logD1");
			config.getDouble(a4[1], "MMI.depth1");
			config.getDouble(a5[1], "MMI.crustal1");
			config.getDouble(ad[1], "MMI.dummy1");

			config.getDouble(depth0, "MMI.maxShallowDepth");

			return true;
		}


		bool process(Event *event) {
			Origin *org = Origin::Find(event->preferredOriginID());
			Magnitude *mag = Magnitude::Find(event->preferredMagnitudeID());

			if ( !org || !mag ) {
				SEISCOMP_WARNING("%s: MMI: no origin or magnitude information",
				                 event->publicID().c_str());
				setMMI(event, -1.0);
				return false;
			}

			double depth, magnitude;

			try { depth = org->depth().value(); }
			catch ( ... ) {
				SEISCOMP_WARNING("%s: MMI: no depth information available",
				                 event->publicID().c_str());
				setMMI(event, -1.0);
				return false;
			}

			try { magnitude = mag->magnitude().value(); }
			catch ( ... ) {
				SEISCOMP_WARNING("%s: MMI: no magnitude information available",
				                 event->publicID().c_str());
				setMMI(event, -1.0);
				return false;
			}

			setMMI(event, maxMMIatSource(magnitude, depth));

			return false;
		}

	private:
		/**
		 * Adds/updates a comment with id "MMI" of an event.
		 * @param event The event
		 * @param mmi The value to be written into the comment text
		 */
		void setMMI(Event *event, double mmi) {
			std::string value;
			if ( mmi >= 0 )
				value = toString(int(mmi*10)*0.1);

			CommentPtr mmiComment = NULL;

			for ( size_t i = 0; i < event->commentCount(); ++i ) {
				Comment *comment = event->comment(i);
				if ( comment->id() == "MMI" ) {
					mmiComment = comment;
					break;
				}
			}

			if ( !mmiComment ) {
				mmiComment = new Comment();
				mmiComment->setId("MMI");
				event->add(mmiComment.get());
			}
			// Check if something changed
			else if ( mmiComment->text() == value )
				return;
			else
				mmiComment->update();

			// Set the modification to current time
			try {
				mmiComment->creationInfo().setModificationTime(Time::GMT());
			}
			catch ( ... ) {
				CreationInfo ci;
				ci.setModificationTime(Time::GMT());
				mmiComment->setCreationInfo(ci);
			}

			mmiComment->setText(value);
			SEISCOMP_DEBUG("%s: MMI: update value to %s",
			               event->publicID().c_str(),
			               value.c_str());
		}


		/**
		 * Compute the depth to the top of the fault rupture, based on the
		 * earthquake magnitude and centroid depth.
		 * <p/>
		 * Compute HT: use scaling relation M = 5.39 + log(L) where L is fault length.
		 * This gives L = 10 ** (M - 5.39). Use fault width W = L / 2, with a
		 * maximum W of 30 km. HT is then given by HT = depth - 0.5 * W * sin b, where
		 * b is the fault dip. Use a value of 0.85, the average of sin 45 and sin 90.
		 *
		 * @param magnitude magnitude of the earthquake.
		 * @param depth depth of the earthquake in positive km.
		 * @return depth to the top of the rupture in km.
		 */
		double topRupture(double magnitude, double depth) const {
			double topRupture = depth;

			// Note - this is 100.0 depth here is not the same as for defining
			// a deep event.
			if (fabs(depth) < 100.0) {
				double w = 0.5 * pow(10, (magnitude - 5.39));
				w = std::min(w, 30.0);
				topRupture = std::max(fabs(depth) - 0.5 * w * 0.85, 0.0);
			}

			return topRupture;
		}


		/**
		 * The max Modified Mercalli Intensity above the source.
		 *
		 * @param magnitude the earthquake magntitude.
		 * @param depth     the earthquake depth in positive km.
		 * @return the MMI above the source.
		 */
		double maxMMIatSource(double magnitude, double depth) const {
			double depthToTopOfRupture = topRupture(magnitude, depth);

			int n = depth < depth0 ? 0 : 1;

			double maxMMIatSource =
				a1[n] + a2[n] * magnitude +
				a3[n] * log10(depthToTopOfRupture * depthToTopOfRupture * depthToTopOfRupture +
				              ad[n] * ad[n] * ad[n]) / 3.0 +
				a4[n] * depth + a5[n];

			if ( maxMMIatSource < minMM || maxMMIatSource > maxMM ) {
				SEISCOMP_WARNING("MMI: value out of bounds: %.1f not in [%d;%d]",
				                 maxMMIatSource, minMM, maxMM);
				maxMMIatSource = -1.0;
			}

			return maxMMIatSource;
		}


		/**
		 * The max Modified Mercalli Intensity above the source.
		 *
		 * @param magnitude the earthquake magntitude.
		 * @param depth     the earthquake depth in positive km.
		 * @return the MMI above the source.
		 */
		int maxMMI(float depth, float magnitude) {
			return (int) floor(maxMMIatSource(depth, magnitude));
		}


	private:
		int    maxMM;
		int    minMM;
		double a1[2]; // constant
		double a2[2]; // Mw                !
		double a3[2]; // logD              !
		double a4[2]; // depth             !
		double a5[2]; // interface/crustal !
		double ad[2];
		double depth0;
};


REGISTER_EVENTPROCESSOR(MMIProcessor, "MMI");
