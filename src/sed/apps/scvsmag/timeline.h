/***************************************************************************
 * Copyright
 * ---------
 * This file is part of the Virtual Seismologist (VS) software package.
 * VS is free software: you can redistribute it and/or modify it under
 * the terms of the "SED Public License for Seiscomp Contributions"
 *
 * VS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the SED Public License for Seiscomp
 * Contributions for more details.
 *
 * You should have received a copy of the SED Public License for Seiscomp
 * Contributions with VS. If not, you can find it at
 * http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt
 *
 * Authors of the Software: Jan Becker, Yannik Behr and Stefan Heimers
 * Copyright (C) 2006-2013 by Swiss Seismological Service
 ***************************************************************************/

#ifndef TIMELINE_H__
#define TIMELINE_H__

#include <stdexcept>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/processing/waveformprocessor.h>
#include <seiscomp3/datamodel/vs/vs_package.h>
#include <seiscomp3/math/geo.h>
#include <set>

#include "circular.h"

namespace Seiscomp {

MAKEENUM(
	SoilClass,
	EVALUES(
	Rock,
	Soil,
	SoilClassQuantity
	),
	ENAMES(
	"rock",
	"soil",
	"soilclassquantity",
	)
);

enum ValueType {
	Acceleration, Velocity, Displacement, ValueTypeQuantity
};

MAKEENUM(
	WaveType,
	EVALUES(
		P_Wave,
		S_Wave,
		UndefinedWaveType
	),
	ENAMES(
		"P-wave",
		"S-wave",
		"Undefined wavetype"
	)
);

enum ReturnCode {
	no_problem, clipped_data, not_enough_data, no_data, undefined_problem,
	index_error, ReturnCodeQuantity
};

enum Component {
	Z, H1, H2, H, ComponentQuantity
};

struct Envelope {
	Envelope() {
		for ( int i = 0; i < ValueTypeQuantity; ++i )
			values[i] = -1;
		clipped = false;
	}

	// Negative values are treated as "unset".
	float values[ValueTypeQuantity];
	bool clipped;
};

struct Cell {
	Cell() {}
	Envelope envelopes[ComponentQuantity];
};

typedef circular_buffer<Cell> Row; // see circular.h by Pete Goodliffe 

struct SensorBuffer {
	SensorBuffer(size_t capacity) :
			buffer(capacity) {
	}

	typedef Processing::WaveformProcessor::SignalUnit SignalUnit;
	Row buffer;
	SignalUnit sensorUnit;
	std::string locationCode;
	std::string streamCode; // Without component code (e.g. HH)
};

typedef boost::shared_ptr<SensorBuffer> SensorBufferPtr;
typedef std::list<SensorBufferPtr> SensorBuffers;
typedef boost::shared_ptr<SensorBuffers> SensorBuffersPtr;

class Timeline {
public:
	//! Pair of network and station code
	typedef std::pair<std::string, std::string> StationID;
	typedef std::map<StationID, SensorBuffersPtr> Stations;
	typedef std::set<StationID> StationList;

	/**
	 Initializes the timeline and sets the number of slots
	 in the past to "past" and the number of slots in the future
	 to "future". past and future are relative to the current
	 reference time.
	 @param past Number of seconds to save for past values
	 @param future Number of seconds to save for future values
	 */
	void init(int past, int future, int timeout);

	/**
	 Sets the current reference time and shift the ringbuffers
	 of all rows accordingly.
	 @param ref New reference time
	 */
	bool setReferenceTime(const Core::Time &ref);

	//! Returns the current reference time.
	const Core::Time &referenceTime() const {
		return _referenceTime;
	}

	/**
	 Same as setReferenceTime(referenceTime() + secs)
	 @param secs Number of seconds to be added to current reference
	 time.
	 */
	bool step(int secs = 1);

	/**
	 Updates the timeline grid.
	 @param env An envelope instance.
	 */
	bool feed(const DataModel::VS::Envelope *env);

	/**
	 Returns the maximum vertical and maximum horizontal envelope
	 of a station id between start and end.
	 @return If false is returned the maximum envelope could not
	 be found due to:
	 1. no envelopes available in the passed time span for
	 either the vertical or the horizontal component
	 2. time span is outside the buffer window
	 3. the passed id is not available
	 */
	ReturnCode maxmimum(const StationID &id, const Core::Time &start,
			const Core::Time &end, const Core::Time &pick, Envelope &vertical,
			Core::Time &timeVertical, Envelope &horizontal,
			Core::Time &timeHorizontal, std::string &locationCode,
			std::string &channelCode) const;

	/**
	 Checks for which stations within a given distance of the epicenter data
	 is available.
	 */
	ReturnCode pollbuffer(double epiclat, double epiclon, double dthresh,
			int &stationcount) const;

	/**
	 Returns the number of envelope streams in the buffer.
	 @return int The number of envelope streams.
	 */
	int StreamCount();
private:
	Core::Time _referenceTime;
	Stations _stations;
	int _headSlots;
	int _backSlots;
	int _clipTimeout;
};
}
#endif
