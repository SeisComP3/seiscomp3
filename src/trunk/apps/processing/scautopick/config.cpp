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



#define SEISCOMP_COMPONENT Autopick

#include <stdio.h>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>

#include "config.h"


namespace Seiscomp {
namespace Applications {


Picker::Config::Config() {
	amplitudeGroup = "AMPLITUDE";
	phaseHint = "P";

	test = false;
	offline = false;

	useAllStreams = true;
	calculateAmplitudes = true;
	interpolateGaps = false;
	maxGapLength = 4.5;

	defaultChannel = "BH";
	defaultFilter = "RMHP(10)>>ITAPER(30)>>BW(4,0.7,2)>>STALTA(2,80)";
	defaultTriggerOnThreshold = 3.0;
	defaultTriggerOffThreshold = 1.5;

	minDuration = -1;
	maxDuration = -1;

	triggerDeadTime = 30.0;
	amplitudeMaxTimeWindow = 10.0;
	amplitudeMinOffset = 3.0;

	defaultTimeCorrection = Core::TimeSpan(-0.8);
	ringBufferSize = Core::TimeSpan(5.*60.);
	leadTime = 60.;
	initTime = 60.;

	pickerType = "";
	killPendingSecondaryProcessors = true;
	sendDetections = false;

	amplitudeList.insert("MLv");
	amplitudeList.insert("mb");
	amplitudeList.insert("mB");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Picker::Config::init(const Client::Application *app) {
	try { amplitudeGroup = app->configGetString("connection.amplitudeGroup"); }
	catch ( ... ) {}

	try { phaseHint = app->configGetString("phaseHint"); }
	catch ( ... ) {}

	try { calculateAmplitudes = app->configGetBool("calculateAmplitudes"); }
	catch (...) {}

	try { defaultFilter = app->configGetString("filter"); }
	catch (...) {}

	try { useAllStreams = app->configGetBool("useAllStreams"); }
	catch (...) {}

	try { defaultTimeCorrection = app->configGetDouble("timeCorrection"); }
	catch (...) {}
	try { ringBufferSize = app->configGetDouble("ringBufferSize"); }
	catch (...) {}
	try { leadTime = app->configGetDouble("leadTime"); }
	catch (...) {}
	try { initTime = app->configGetDouble("initTime"); }
	catch (...) {}
	try { interpolateGaps = app->configGetBool("gapInterpolation"); }
	catch (...) {}

	try { defaultTriggerOnThreshold = app->configGetDouble("thresholds.triggerOn"); }
	catch (...) {}
	try { defaultTriggerOffThreshold = app->configGetDouble("thresholds.triggerOff"); }
	catch (...) {}
	try { maxGapLength = app->configGetDouble("thresholds.maxGapLength"); }
	catch (...) {}
	try { triggerDeadTime = app->configGetDouble("thresholds.deadTime"); }
	catch (...) {}
	/* TODO: Needs support in detector
	try { minDuration = app->configGetDouble("thresholds.minDuration"); }
	catch (...) {}
	try { maxDuration = app->configGetDouble("thresholds.maxDuration"); }
	catch (...) {}
	*/

	try { amplitudeMaxTimeWindow = app->configGetDouble("thresholds.amplMaxTimeWindow"); }
	catch (...) {}
	try { amplitudeMinOffset = app->configGetDouble("thresholds.minAmplOffset"); }
	catch (...) {}

	try {
		std::vector<std::string> amplitudes = app->configGetStrings("amplitudes");
		amplitudeList.clear();
		amplitudeList.insert(amplitudes.begin(), amplitudes.end());
	}
	catch (...) {}

	try {
		std::vector<std::string> amplitudes = app->configGetStrings("amplitudes.enableUpdate");
		amplitudeUpdateList.clear();
		amplitudeUpdateList.insert(amplitudes.begin(), amplitudes.end());
	}
	catch (...) {}

	try { pickerType = app->configGetString("picker"); }
	catch ( ... ) {}

	try { secondaryPickerType = app->configGetString("spicker"); }
	catch ( ... ) {}

	try { killPendingSecondaryProcessors = app->configGetBool("killPendingSPickers"); }
	catch ( ... ) {}

	try { sendDetections = app->configGetBool("sendDetections"); }
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Picker::Config::init(const Client::CommandLine &commandline) {
	test = commandline.hasOption("test");
	offline = commandline.hasOption("offline") || commandline.hasOption("ep");
	dumpRecords = commandline.hasOption("dump-records");
	sendDetections = commandline.hasOption("send-detections") ? true : sendDetections;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Picker::Config::dump() const {
	printf("Configuration:\n");
	printf("amplitude group                  %s\n",     amplitudeGroup.c_str());
	printf("testMode                         %s\n",     test ? "true":"false");
	printf("offline                          %s\n",     offline ? "true":"false");
	printf("useAllStreams                    %s\n",     useAllStreams ? "true":"false");
	printf("calculateAmplitudes              %s\n",     calculateAmplitudes ? "true":"false");
	printf("calculateAmplitudeTypes          ");
	if ( amplitudeList.empty() )
		printf("[]\n");
	else {
		for ( StringSet::const_iterator it = amplitudeList.begin();
		      it != amplitudeList.end(); ++it ) {
			if ( it != amplitudeList.begin() )
				printf(", ");
			printf("%s", it->c_str());
		}
		printf("\n");
	}
	printf("interpolateGaps                  %s\n",     interpolateGaps ? "true":"false");
	printf("maxGapLength                     %.2f s\n", maxGapLength);
	printf("defaultFilter                    %s\n",     defaultFilter.c_str());
	printf("defaultTriggerOnThreshold        %.2f\n",   defaultTriggerOnThreshold);
	printf("defaultTriggerOffThreshold       %.2f\n",   defaultTriggerOffThreshold);
	//printf("minDuration                      %.2f\n",   minDuration);
	//printf("maxDuration                      %.2f\n",   maxDuration);
	printf("triggerDeadTime                  %.2fs\n", triggerDeadTime);
	printf("amplitudeMaxTimeWindow           %.2fs\n", amplitudeMaxTimeWindow);
	printf("amplitudeMinOffset               %.2fs\n", amplitudeMinOffset);
	printf("defaultTimeCorrection            %.2fs\n", defaultTimeCorrection);
	printf("ringBufferSize                   %.0fs\n", ringBufferSize);
	printf("leadTime                         %.0fs\n", leadTime);
	printf("initTime                         %.0fs\n", initTime);
	printf("pickerType                       %s\n",    pickerType.c_str());
	printf("secondaryPickerType              %s\n",    secondaryPickerType.c_str());
	printf("killPendingSPickers              %s\n",    killPendingSecondaryProcessors ? "true" : "false");
	printf("sendDetections                   %s\n",    sendDetections ? "true" : "false");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
