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





#ifndef __APPS_PICKER_CONFIG_H__
#define __APPS_PICKER_CONFIG_H__

#include <string>


namespace Seiscomp {

namespace Client {

	class Application;
	class CommandLine;

}

namespace Applications {
namespace Picker {


typedef std::set<std::string> StringSet;


class Config {
	public:
		Config();

		void init(const Client::Application *app);
		void init(const Client::CommandLine &config);

		std::string amplitudeGroup;
		std::string phaseHint;

		// Sets the test mode. When enabled no picks and
		// amplitudes are send via the messaging.
		bool        test;

		// Sets the offline mode. When enabled no database
		// is used to read available streams.
		bool        offline;
		bool        dumpRecords;

		// Create a picker for every stream data is received
		// for. This flag is set when the database is not used
		// (offline mode) and records are read from files unless
		// explicitly disabled.
		bool        useAllStreams;

		// Enables/disables amplitude calculation
		bool        calculateAmplitudes;

		// Enables/disables gap interpolation
		// When disabled the picker is going to be resetted
		// after a gap has been detected otherwise the gap
		// is going to be interpolated when is length is
		// under a certain length (see next parameter).
		bool        interpolateGaps;

		// The maximum gap length in seconds to handle
		// when interpolateGaps is set to true.
		// If the gap length is larget than this value the
		// picker will be resetted.
		double      maxGapLength;

		// The default channel to pick on for a station
		// without component code (BH, SH, ...).
		std::string defaultChannel;

		// The default filter when no per station configuration
		// has been read.
		std::string defaultFilter;

		// The default threshold to trigger a pick.
		double      defaultTriggerOnThreshold;
		// The default threshold to enable triggering
		// again.
		double      defaultTriggerOffThreshold;

		// The dead time in seconds after a pick has been
		// triggered.
		double      triggerDeadTime;

		// The minimum duration of a trigger, negative value = disabled
		double      minDuration;

		// The maximum duration of a trigger, negative value = disabled
		double      maxDuration;

		// The timewindow in seconds to calculate a SNR amplitude.
		double      amplitudeMaxTimeWindow;

		// The minimum amplitude offset used in the formula to
		// calculate the minimum amplitude to reach when a
		// previous pick on a stream exists.
		// minAmplitude = amplitudeMinOffset + lastAmplitude * exp(-(trigger - lastTrigger)^2)
		double      amplitudeMinOffset;

		// The default time correction in seconds to apply
		// when a pick is going to be emitted.
		double      defaultTimeCorrection;

		// The global record ringbuffer size in seconds.
		double      ringBufferSize;

		// The timespan in seconds that will be substracted from
		// NOW to acquire records.
		double      leadTime;

		// The initialization time per stream in seconds after
		// the first records has been received. Within that time
		// the picker is disabled.
		double      initTime;

		// The amplitude types to calculate
		StringSet   amplitudeList;

		// The amplitude types that can be updated
		StringSet   amplitudeUpdateList;

		// The picker type to use
		std::string pickerType;

		// The secondary picker type to use
		std::string secondaryPickerType;

		// Whether kill previously started secondary pickers when a new
		// primary pick has been declared
		bool        killPendingSecondaryProcessors;

		// Send detections as well if a picker is configured?
		bool        sendDetections;

		// Accept historic data in real-time playbacks?
		bool        playback;

	public:
		void dump() const;
};

}
}
}


#endif
