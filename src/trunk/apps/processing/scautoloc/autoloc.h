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




#ifndef __SEISCOMP_AUTOLOC_AUTOLOC3__
#define __SEISCOMP_AUTOLOC_AUTOLOC3__

#include <string>
#include <map>
#include <set>

#include <seiscomp3/seismology/locsat.h>

#include "datamodel.h"
#include "nucleator.h"
#include "associator.h"
#include "locator.h"

namespace Autoloc {

typedef enum { GlobalNetwork, RegionalNetwork, LocalNetwork } NetworkType;

class StationConfig {
	public:
		class Entry {
			public:
				float maxNucDist, maxLocDist;
				int usage;

			Entry() {
				maxNucDist = maxLocDist = 0;
				usage = 0;
			}
		};

		StationConfig();
		bool read(const std::string &filename);

		const Entry& get(const std::string &net, const std::string &sta) const;

	private:
		std::map<std::string, Entry> _entry;
};


class Autoloc3 {

	public:
		Autoloc3();
		virtual ~Autoloc3();

	public:
		class Config {
		public:
			Config();

			// During cleanup() all objects older than maxAge
			// (in hours) are removed.
			// If this parameter is <= 0, cleanup() is disabled.
			double maxAge;

			// time span within which we search for picks which may indicate extraordinary activity
			double dynamicPickThresholdInterval;

			// typically good RMS in our network
			double goodRMS;

			// maximum RMS of all used picks
			double maxRMS;

			// maximum residual of any pick to be used for location
			double maxResidualUse;

			// maximum residual of any pick to be kept associated
			double maxResidualKeep;

			// NOTE: maxRMS < maxResidualUse < maxResidualKeep
			// typically:
			//    maxResidualKeep = 3*maxResidualUse
			//    maxResidualUse  = 2*maxRMS


			// Use this depth if there is no depth resolution
			double defaultDepth;           // default 10 km
			double defaultDepthStickiness; // 0...1 default 0.5

			// Try to relocate an origin using the configured default depth.
			// If the resulting origin is "not much worse" than the free-depth
			// origin, fix the depth at the default depth.
			bool tryDefaultDepth;         // default true

			bool adoptManualDepth;

			// Minimum depth in case there is depth resolution
			double minimumDepth;          // default 5 km

			// maximum depth of origin, checked before sending, default is 1000
			double maxDepth;

			// Max. secondary azimuthal gap
			double maxAziGapSecondary;

			// Maximum distance of stations used in computing
			// the azimuthal gap
			double maxAziGapDist;

			// Max. distance of stations to be used
			double maxStaDist;

			// Default max. distance of stations to be used in
			// the nucleator
			// May be overridden by individual values per station.
			double defaultMaxNucDist;

			// Minimum signal-to-noise ratio for a pick to
			// be processed
			double minPickSNR;              // default 3

			// Unless we have a clear definition of "affinity",
			// do not change this!
			double minPickAffinity;         // default 0.05

			// Minimum number of picks for a normal origin
			// to be reported
			int minPhaseCount;           // default  6

			// Minimum score for an origin to be reported;
			// this seems to be a more reliable criterion
			// than number of phases.
			double minScore;

			// Minimum station count for which we ignore PKP phases
			// XXX not yet used
			int minStaCountIgnorePKP;  // default 20

			// if a pick can be associated to an origin with at
			// least this high a score, bypass the nucleator,
			// which will improve speed
			double minScoreBypassNucleator;

			// Maximum allowes "probability" for an origin to be a
			// fake event. Value between 0 and 1
			double maxAllowedFakeProbability;

			// EXPERIMENTAL!!!
			double distSlope;

			// EXPERIMENTAL!!!
			double maxRadiusFactor;

			// EXPERIMENTAL!!!
			NetworkType networkType;

			double cleanupInterval;

			double publicationIntervalTimeSlope;
			double publicationIntervalTimeIntercept;
			int    publicationIntervalPickCount;
			// XXX maybe score interval instead?

			// If true, offline mode is selected. In offline mode,
			// no database is accessed, and station locations are
			// read from a plain text file.
			bool offline;

			// If true, test mode is selected. In test mode, no
			// origins are sent. This is not the same as offline
			// mode. Test mode is like normal online mode except
			// no origins are sent (only logging output is
			// produced).
			bool test;

			// If true, playback mode is selected. In playback mode,
			// origins are sent immediately without delay.
			bool playback;

			// If true then manual picks are being used as automatics
			// picks are
			bool useManualPicks;

			// The pick log file
			std::string pickLogFile;
			int         pickLogDate;

			// locator profile, e.g. "iasp91", "tab" etc.
			std::string locatorProfile;

			// The station configuration file
			std::string staConfFile;

			// misc. experimental options
			bool aggressivePKP;
			bool reportAllPhases;
			bool useManualOrigins;
			bool adoptManualOriginsFixedDepth;
			bool useImportedOrigins;

			// enable the XXL feature
			bool xxlEnabled;

			// minimum absolute amplitude to flag a pick as XXL
			double xxlMinAmplitude;            // default  10000 nm/s

			// minimum SNR to flag a pick as XXL
			double xxlMinSNR;            // default  8

			// ignore all picks within this time window
			// length (in sec) following an XXL pick
			double xxlDeadTime;		// default 60 s

			// Minimum number of picks for an XXL origin
			// to be reported
			unsigned int xxlMinPhaseCount; // default 4 picks
			double xxlMaxStaDist;          // default 10 degrees
			double xxlMaxDepth;            // default 100 km

		public:
			void dump() const;
		};

	public:
		// initialization stuff
		void setConfig(const Config &config);
		const Config &config() const { return _config; }

		bool setGridFile(const std::string &);
		void setPickLogFilePrefix(const std::string &);
		void setPickLogFileName(const std::string &);
		bool setStation(Station *);
		void setLocatorProfile(const std::string &);

		bool init();

	public:
		// Feed a Pick and try to get something out of it.
		//
		// The Pick may be associated to an existing Origin or
		// trigger the creation of a new Origin. If "something"
		// resulted, return true, otherwise false.
		bool feed(const Pick *);

		// Feed a PickGroup and try to get something out of it.
		//
		// This feeds all the picks in the PickGroup atomically.
		bool feed(const PickGroup&); // TODO: NOT IMPLEMENTED YET

		// Feed an external or manual Origin
		bool feed(Origin *);

		// Report all new origins and thereafter empty _newOrigins.
		// This calls _report(Origin*) for each new Origin
		bool report();

		void dumpState() const;
		void dumpConfig() const { _config.dump(); }

	public:
		// Trigger removal of old objects.
		void cleanup(Time minTime=0);

		void reset();
		void shutdown();

	public:
		// Get a Pick from Autoloc's internal buffer by ID.
		// If the pick is not found, return NULL
		const Pick* pick(const std::string &id) const;

		// Current time. In offline mode time of the last pick.
		Time now();

		// Synchronize the internal timing.
		//
		// This is necessary in playback mode were instead of
		// using the hardware clock we either use the pick time
		// or pick creation time.
		bool sync(const Time &t);

	protected:
		// !!!!!!!!!!!!!!!!!!!!!!!
		// This must be reimplemented in a subclass to properly
		// print/send/etc. the origin
		// !!!!!!!!!!!!!!!!!!!!!!!
		virtual bool _report(const Origin *);

	protected:
		// flush any pending (Origin) messages by calling _report()
		void _flush();

	private:
		//
		// tool box
		//

		// Compute the score.
		double _score(const Origin *) const;
		void _updateScore(Origin *);

		// Process the pick. Involves trying to associate with an
		// existing origin and -upon failure- nucleate a new one.
		bool _process(const Pick *);

		// If the pick is an XXL pick, try to see if there are more
		// XXL picks possibly giving rise to a preliminary origin
		OriginPtr _xxlPreliminaryOrigin(const Pick*);

		// Try to enhance the score by removing each pick
		// and relocating.
		//
		// Returns true if the score could be enhanced.
		bool _enhanceScore(Origin *, int maxloops=0);

		// Rename P <-> PKP accoring to distance/traveltime
		// FIXME: This is a hack we would want to avoid.
		void _rename_P_PKP(Origin *);

		// Really big outliers, where the assiciation obviously
		// went wrong and which are excluded from the location
		// anyway, are removed.
		// 
		// Returns number of removed outliers.
		int _removeWorstOutliers(Origin *);

		// Try all that can be done to improve the origin:
		//
		// * exclude picks with unacceptably large residuals
		//
		// * see if there are picks previously excluded due to
		//   large residual, but which now have acceptably small
		//   residual
		//
		// TODO:
		// * see if any picks that previously slipped through
		//   nucleator and associator do match the origin and if
		//   so, try a relocation
		//
		// TODO:
		// * see if there are distant stations which due to a
		//   large number of near stations can be ignored 
		//     -> useful to get rid of PKP
		//
		// TODO:
		// * take care of PP, SKP, possibly pP and the like
		//
		// Returns true on success.
//		bool _improve(Origin *);

		// true, if this pick was marked as bad and must not be used
		void _setBlacklisted(const Pick *, bool=true);

		// true, if this pick was marked as bad and must not be used
		bool _blacklisted(const Pick *) const;
	
		// true if pick comes with valid station info
		bool _addStationInfo(const Pick *);

		// feed pick to nucleator and return a new origin or NULL
		OriginPtr _tryNucleate(const Pick*);

		// feed pick to nucleator and return a new origin or NULL
		OriginPtr _tryAssociate(const Pick*);

		// one last check before the origin is accepted.
		// doesn't modify the origin
		bool _passedFinalCheck(const Origin *origin);
		// called by _passedFilter()
		// TODO: integrate into _passedFilter()?

		// check whether the origin is acceptable
		bool _passedFilter(Origin*);

		// check whether the origin meets the publication criteria
		bool _publishable(const Origin*) const;

		// store a pick in internal buffer
		bool _store(const Pick*);

		// store an origin in internal buffer
		bool _store(Origin*);

		// try to find and add more matching picks to origin
		bool _addMorePicks(Origin*, bool keepDepth=false);

		// try to associate a pick to an origin
		// true if successful
		bool _associate(Origin*, const Pick*, const std::string &phase);

		// try to judge if an origin *may* be a fake, e.g.
		// coincident PP phases if an earlier event
		//
		// returns a "probability" between 0 and 1
		double _testFake(Origin*) const;

		// set the depth of this origin to the default depth
		//
		// if successful, true is returned, otherwise false
		bool _setDefaultDepth(Origin*);

		// Attempt to decide whether the focal depth can be resolved considering
		// the station distribution. Returns true if resolvable, false if not.
		bool _depthIsResolvable(Origin*);

		bool _setTheRightDepth(Origin*);

		// Determine whether the epicenter location requires the use of the default depth.
		// This may be due to the epicenter located
		// * far outside the network or
		// * near to an ocean ridge where on one hand we don't have depth resolution
		//   but on the other hand we can safely assume a certain (shallow) depth.
		//
		// returns true if the default depth should be used, false otherwise
		bool _epicenterRequiresDefaultDepth(const Origin*) const;

		// For convenience. Checks whether a residual is within bounds.
		// TODO: Later this shall accomodate distance dependent limits.
		// The factor allows somewhat more generous residuals e.g.
		// for association.
		bool _residualOK(const Arrival&, double minFactor=1, double maxFactor=1) const;

		// Don't allow Arrivals with residual > maxResidual. This
		// criterion is currently enforced mercilessly, without
		// allowing for asymmetry. What you specify (maxResidual) is
		// what you get.
		//
		// NOTE that the behavior is partly controlled through the configuration of _relocator
		bool _trimResiduals(Origin*);

		bool _log(const Pick*);

		bool _tooLowSNR(const Pick*) const;

		bool _followsBiggerPick(const Pick*) const;

		// checks if the pick could be a Pdiff of a known origin
		bool _perhapsPdiff(const Pick *) const;

		// Suppress picks with low amplitude if station is producing
		// many unassociated picks in a recent time span
		bool _tooManyRecentPicks(const Pick *) const;

		// Merge two origins considered to be the same event
		// The second is merged into the first. A new instance
		// is returned that has the ID of the first
		Origin *merge(const Origin*, const Origin*);

		bool _associated(const Pick *) const;

		bool _rework(Origin *);

		void _ensureConsistentArrivals(Origin *);

		void _ensureAcceptableRMS(Origin *, bool keepDepth=false);

		// Exclude PKP phases if there are enough P phases.
		// This is part of _rework()
		bool _excludePKP(Origin *);

		// If all stations are close to the epicenter, but just a few
		// are very far away, exclude the farthest by setting
		// Arrival::exlcude to Arrival::StationDistance
		bool _excludeDistantStations(Origin *);

		OriginID _newOriginID();

		// try to find an "equivalent" origin
		Origin *_findEquivalent(const Origin*);

		// for a newly fed origin, find an equivalent internal origin
		Origin *_findMatchingOrigin(const Origin*);


	private:
		Associator _associator;
		GridSearch _nucleator;
		Locator    _relocator;

	private:
		// origins that were created/modified during the last
		// feed() call
		OriginVector _newOrigins;

		// origins waiting for a _flush()
		std::map<int, Time>      _nextDue;
		std::map<int, OriginPtr> _lastSent;
		std::map<int, OriginPtr> _outgoing;

		Time     _now;
		Time     _nextCleanup;

	protected:
		typedef std::map<std::string, PickCPtr> PickMap;
		PickMap  _pick;
		std::string   _pickLogFilePrefix;
		std::string   _pickLogFileName;
		std::ofstream _pickLogFile;

	private:
		StationMap _stations;

		// a list of NET.STA strings for missing stations
		std::set<std::string> _missingStations;
		std::set<PickCPtr> _blacklist;

		OriginVector _origins;
		Config   _config;
		StationConfig _stationConfig;
};


}  // namespace Autoloc

#endif
