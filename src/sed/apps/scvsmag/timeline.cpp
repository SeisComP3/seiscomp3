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

#define SEISCOMP_COMPONENT VsMagnitude

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/inventory.h>

#include "timeline.h"
#include "util.h"

/*!
 \brief The timeline contains the ringbuffer that caches incoming envelope data.

 It has a time resolution of one second.


 */

using namespace std;

namespace Seiscomp {

void Timeline::init(int past, int future, int timeout) {
	_headSlots = future;
	_backSlots = past;
	_clipTimeout = timeout;
}

bool Timeline::setReferenceTime(const Core::Time &ref) {
	if ( !_referenceTime.valid() ) {
		_referenceTime = ref;
		return true;
	}

	Core::TimeSpan diff = ref - _referenceTime;
	if ( (double) diff < 0 )
		return false;

	int steps = diff.seconds();
	return step(steps);
}

int Timeline::StreamCount(){
	return _stations.size();
}

/*!
 \brief move <secs> seconds forward on the timeline

 \param secs Number of seconds to move the timeline

 */
bool Timeline::step(int secs) {
	_referenceTime += Core::TimeSpan(secs, 0);

	Stations::iterator it;
	for ( it = _stations.begin(); it != _stations.end(); ++it ) {
		SensorBuffers *buf = it->second.get();
		SensorBuffers::iterator sit;
		for ( sit = buf->begin(); sit != buf->end(); ++sit ) {
			// Shift buffer
			for ( int i = 0; i < secs; ++i )
				(*sit)->buffer.push_back(Cell());
		}
	}

	return true;
}

/*!
 \brief Add an incoming envelope message to the timeline

 \param env VS::Envelope object to be added to the timeline
 */
bool Timeline::feed(const DataModel::VS::Envelope *env) {
	StationID id(env->network(), env->station());
	SensorBuffersPtr station;
	Stations::iterator it;

	it = _stations.find(id);
	if ( it == _stations.end() ) {
		station = SensorBuffersPtr(new SensorBuffers);
		_stations[id] = station;
		SEISCOMP_DEBUG(
				"create new station entry for %s.%s", id.first.c_str(), id.second.c_str());
	} else
		station = it->second;

	int cnt = 0;

	int bufferSize = _headSlots + _backSlots;

	for ( size_t i = 0; i < env->envelopeChannelCount(); ++i ) {
		DataModel::VS::EnvelopeChannel *cha = env->envelopeChannel(i);

		int idx = (int) (env->timestamp() - _referenceTime).seconds()
				+ _backSlots;
		if ( idx < 0 ) {
			SEISCOMP_DEBUG(
					"ignoring received envelope (too old, current time = %s)", _referenceTime.iso().c_str());
			continue;
		}
		if ( idx >= bufferSize ) {
			SEISCOMP_DEBUG(
					"ignoring received envelope (too far in the future, current time = %s, idx = %d, bufferSize = %d)", _referenceTime.iso().c_str(), idx, bufferSize);
			continue;
		}

		SensorBuffers::iterator sit;
		SensorBufferPtr sensor;
		for ( sit = station->begin(); sit != station->end(); ++sit ) {
			if ( cha->waveformID().locationCode() != (*sit)->locationCode )
				continue;
			if ( cha->waveformID().channelCode().compare(0, 2, (*sit)->streamCode) != 0 )
				continue;
			sensor = *sit;
			break;
		}

		if ( !sensor ) {
			Client::Inventory *inv = Client::Inventory::Instance();
			DataModel::Stream *stream = inv->getStream(id.first, id.second,
					cha->waveformID().locationCode(),
					cha->waveformID().channelCode(), env->timestamp());
			Processing::WaveformProcessor::SignalUnit signalUnit;
			if ( stream ) {
				bool unitOK = false;
				try {
					unitOK = signalUnit.fromString(stream->gainUnit());
				} catch ( ... ) {
				}
				if ( !unitOK ) {
					SEISCOMP_ERROR(
							"%s: unable to retrieve gain unit", Private::toStreamID(cha->waveformID()).c_str());
					continue;
				}
			} else {
				SEISCOMP_ERROR(
						"%s: unable to retrieve stream from inventory", Private::toStreamID(cha->waveformID()).c_str());
				continue;
			}

			sensor = SensorBufferPtr(new SensorBuffer(bufferSize));
			sensor->locationCode = cha->waveformID().locationCode();
			sensor->streamCode = cha->waveformID().channelCode().substr(0, 2);
			sensor->sensorUnit = signalUnit;

			// Fill buffer with empty values
			for ( int i = 0; i < bufferSize; ++i )
				sensor->buffer.push_back(Cell());

			SEISCOMP_DEBUG(
					"create new buffer for %s.%s.%s.%s with size %d/%d", id.first.c_str(), id.second.c_str(), sensor->locationCode.c_str(), sensor->streamCode.c_str(), (int)sensor->buffer.size(), bufferSize);

			station->push_back(sensor);
		}

		int component;
		if ( cha->name() == "Z" || cha->name() == "V"  )
			component = Z;
		else if ( cha->name() == "H" )
			component = H;
		else if ( cha->name() == "H1" )
			component = H1;
		else if ( cha->name() == "H2" )
			component = H2;
		else {
			SEISCOMP_WARNING(
					"ignoring unknown envelope channel (%s)", cha->name().c_str());
			continue;
		}

		for ( size_t j = 0; j < cha->envelopeValueCount(); ++j ) {
			DataModel::VS::EnvelopeValue *value = cha->envelopeValue(j);
			Envelope &e = sensor->buffer[idx].envelopes[component];

			if ( value->type() == "acc" ) {
				e.values[Acceleration] = value->value();
				++cnt;
			} else if ( value->type() == "vel" ) {
				e.values[Velocity] = value->value();
				++cnt;
			} else if ( value->type() == "disp" ) {
				e.values[Displacement] = value->value();
				++cnt;
			} else
				SEISCOMP_WARNING(
						"ignoring unknown envelope value type (%s)", value->type().c_str());

			try {
				if ( value->quality() == DataModel::VS::clipped ) {
					sensor->buffer[idx].envelopes[component].clipped = true;
				}
			} catch ( ... ) {
			}
		}

		// Update H if possible
		Envelope &EH = sensor->buffer[idx].envelopes[H];
		Envelope &EH1 = sensor->buffer[idx].envelopes[H1];
		Envelope &EH2 = sensor->buffer[idx].envelopes[H2];

		for ( int j = 0; j < ValueTypeQuantity; ++j ) {
			if ( EH1.values[j] >= 0 && EH2.values[j] >= 0 )
				EH.values[j] = (float) sqrt(
						EH1.values[j] * EH1.values[j]
								+ EH2.values[j] * EH2.values[j]);
		}
		if ( EH1.clipped || EH2.clipped )
			EH.clipped = true;
	}

	return cnt > 0;
}

ReturnCode Timeline::maxmimum(const StationID &id, const Core::Time &start,
		const Core::Time &end, const Core::Time &pick, Envelope &max_Z,
		Core::Time &timeVertical, Envelope &max_H, Core::Time &timeHorizontal,
		std::string &locationCode, std::string &channelCode) const {
		int start_idx = (int) (start - _referenceTime).seconds() + _backSlots;
		int end_idx = (int) (end - _referenceTime).seconds() + _backSlots;
		int pick_idx = (int) (pick - _referenceTime).seconds() + _backSlots;
		int bufferSize = _headSlots + _backSlots;

	// Check if time window is completely outside the buffer
	if ( start_idx >= bufferSize )
		return index_error;
	if ( end_idx < 0 )
		return index_error;

	// Clip start and end indices
	if ( start_idx < 0 )
		start_idx = 0;
	if ( end_idx >= bufferSize )
		end_idx = bufferSize - 1;

	if ( start_idx > end_idx )
		return index_error;

	// check that the pick index is at a reasonable spot
	if ( pick_idx >= end_idx)
		return index_error;

	// Look up station
	Stations::const_iterator it;
	SensorBuffersPtr station;

	it = _stations.find(id);
	if ( it == _stations.end() )
		return no_data;

	station = it->second;

	// Find sensors (either SM or VEL or both)
	SensorBuffers::iterator sit;
	SensorBufferPtr sensorACC, sensorVEL;
	int max_Zidx[] = {-1, -1, -1};
	int max_Hidx[] = {-1, -1, -1};
	int max_idx = -1;
	int min_idx = end_idx;
	int latest_entry = 0;

	// Search for acceleration and velocity sensors. Currently the last of
	// each is taken.
	// TODO: Maybe a better decision is to take the highest values of each
	//       type.
	for ( sit = station->begin(); sit != station->end(); ++sit ) {
		SensorBufferPtr sensor = *sit;
		if ( sensor->sensorUnit
				== Processing::WaveformProcessor::MeterPerSecondSquared )
			sensorACC = sensor;
		else if ( sensor->sensorUnit
				== Processing::WaveformProcessor::MeterPerSecond )
			sensorVEL = sensor;
	}

	if ( sensorVEL ) {
		for ( int i = start_idx; i <= end_idx; ++i ) {
			const Cell &cell = sensorVEL->buffer[i];
			// Ignore clipped values
			if ( cell.envelopes[Z].clipped || cell.envelopes[H].clipped )
				continue;

			// Check vertical values
			for( int ci = 0; ci< ValueTypeQuantity; ++ci ){
				 if ( cell.envelopes[Z].values[ci] >= 0){
					 latest_entry = i;
					 if ( cell.envelopes[Z].values[ci] > max_Z.values[ci] ) {
						 max_Z.values[ci] = cell.envelopes[Z].values[ci];
						 max_Zidx[ci] = i;
						 if ( i > max_idx )
							 max_idx = i;
						 if ( i < min_idx )
							 min_idx = i;
					 }
				 }
			}

			// Check horizontal values
			for( int ci = 0; ci< ValueTypeQuantity; ++ci ){
				if ( cell.envelopes[H].values[ci] >= 0 && cell.envelopes[H].values[ci] > max_H.values[ci] ) {
					max_H.values[ci] = cell.envelopes[H].values[ci];
					max_Hidx[ci] = i;
					if ( i > max_idx )
						max_idx = i;
					if ( i < min_idx )
						min_idx = i;
				}
			}
		}
		// check that the time between the pick and the latest envelope entry
		// in the buffer is at least 1 s
		if ( latest_entry - pick_idx < 1 )
			return not_enough_data;

		SEISCOMP_DEBUG("max_idx: %d; min_idx: %d", max_idx, min_idx);
		if ( max_idx < 0 ) {
			SEISCOMP_DEBUG(
					"No maximum found for %s.%s.%s.", id.first.c_str(), id.second.c_str(), sensorVEL->streamCode.c_str());
		}

		// If clipped data is found in the last '_clipTimeout' seconds
		// check whether there is a co-located strong motion sensor.
		// If not, do not use this sensor.
		int clipcheck_idx = max(min_idx - _clipTimeout, 0);
		for ( int i = clipcheck_idx; i <= max_idx; ++i ) {
			const Cell &cell = sensorVEL->buffer[i];
			if ( cell.envelopes[Z].clipped || cell.envelopes[H].clipped ) {
				SEISCOMP_DEBUG(
						"Record %s.%s.%s has been clipped!", id.first.c_str(), id.second.c_str(), sensorVEL->streamCode.c_str());
				if ( sensorACC ) {
					SEISCOMP_DEBUG(
							"Using %s.%s.%s instead.", id.first.c_str(), id.second.c_str(), sensorACC->streamCode.c_str());
					// Reset index and go to strong motion sensor
					min_idx = end_idx;
					max_idx = -1;
					for ( int i = 0; i < ValueTypeQuantity; ++i ) {
						max_Z.values[i] = -1;
						max_H.values[i] = -1;
					}
					break;
				} else {
					return clipped_data;
				}
			}
		}
		SEISCOMP_DEBUG("Number of maxima not found on Z component: %d", (int)std::count(max_Zidx,max_Zidx+ValueTypeQuantity,-1));
		SEISCOMP_DEBUG("Number of maxima not found on H component: %d", (int)std::count(max_Hidx,max_Hidx+ValueTypeQuantity,-1));
		if ( std::count(max_Zidx,max_Zidx+ValueTypeQuantity,-1) == 0 &&
			std::count(max_Hidx,max_Hidx+ValueTypeQuantity,-1) == 0 ) {
			locationCode = sensorVEL->locationCode;
			channelCode = sensorVEL->streamCode;
			timeVertical = _referenceTime
					+ Core::TimeSpan(max_idx - _backSlots, 0);
			timeHorizontal = _referenceTime
					+ Core::TimeSpan(max_idx - _backSlots, 0);
			return no_problem;
		}
	}

	if ( sensorACC ) {
		for ( int i = start_idx; i <= end_idx; ++i ) {
			const Cell &cell = sensorACC->buffer[i];
			// Ignore clipped values
			if ( cell.envelopes[Z].clipped || cell.envelopes[H].clipped ) {
				SEISCOMP_DEBUG(
						"Record %s.%s.%s has been clipped!", id.first.c_str(), id.second.c_str(), sensorACC->streamCode.c_str());
				continue;
			}

			// Check vertical values
			for(int ci = 0; ci< ValueTypeQuantity; ++ci){
				if ( cell.envelopes[Z].values[ci] >= 0 && cell.envelopes[Z].values[ci] > max_Z.values[ci] ) {
					max_Z.values[ci] = cell.envelopes[Z].values[ci];
					max_Zidx[ci] = i;
					if ( i > max_idx )
						max_idx = i;
					if ( i < min_idx )
						min_idx = i;
				}
			}

			// Check horizontal values
			for(int ci = 0; ci< ValueTypeQuantity; ++ci){
				if ( cell.envelopes[H].values[ci] >= 0 && cell.envelopes[H].values[ci] > max_H.values[ci] ) {
					max_H.values[ci] = cell.envelopes[H].values[ci];
					max_Hidx[ci] = i;
					if ( i > max_idx )
						max_idx = i;
					if ( i < min_idx )
						min_idx = i;
				}
			}
		}

		if ( max_idx < 0 ) {
			SEISCOMP_DEBUG(
					"No maximum found for %s.%s.%s.", id.first.c_str(), id.second.c_str(), sensorACC->streamCode.c_str());
			return no_data;
		}

		// If clipped data is found in the last '_clipTimeout' seconds
		// do not use this sensor.
		int clipcheck_idx = max(min_idx - _clipTimeout, 0);
		for ( int i = clipcheck_idx; i <= max_idx; ++i ) {
			const Cell &cell = sensorACC->buffer[i];
			if ( cell.envelopes[Z].clipped || cell.envelopes[H].clipped ) {
				SEISCOMP_DEBUG(
						"Record %s.%s.%s has been clipped!", id.first.c_str(), id.second.c_str(), sensorACC->streamCode.c_str());
				return clipped_data;
			}
		}

		if ( std::count(max_Zidx,max_Zidx+ValueTypeQuantity,-1) == 0 &&
			std::count(max_Hidx,max_Hidx+ValueTypeQuantity,-1) == 0 ) {
			locationCode = sensorACC->locationCode;
			channelCode = sensorACC->streamCode;
			timeVertical = _referenceTime
					+ Core::TimeSpan(max_idx - _backSlots, 0);
			timeHorizontal = _referenceTime
					+ Core::TimeSpan(max_idx - _backSlots, 0);
			return no_problem;
		}
	}

	return undefined_problem;
}

ReturnCode Timeline::pollbuffer(double epiclat, double epiclon, double dthresh,
		int &stationcount) const {
	int start_idx, end_idx;
	int cnt = 0;
	bool found;
	string locationCode;
	double distdg, azi1, azi2;
	StationList stationsCounted;

	// Look up station
	Client::Inventory *inv = Client::Inventory::Instance();

	// check whether data has arrived within the last 30 s
	start_idx = _backSlots - 30;
	end_idx = _backSlots;
	// Clip start and end indices
	if ( start_idx < 0 )
		start_idx = 0;

	Stations::const_iterator it;
	SensorBuffersPtr station;
	SensorBuffers::const_iterator sit;
	for ( it = _stations.begin(); it != _stations.end(); ++it ) {
		station = it->second;
		found = false;

		// Check whether waveform data has arrived on the vertical
		// component of either the velocity or the acceleration sensor
		SensorBufferPtr sensorACC, sensorVEL;
		for ( sit = station->begin(); sit != station->end(); ++sit ) {
			SensorBufferPtr sensor = *sit;
			if ( sensor->sensorUnit
					== Processing::WaveformProcessor::MeterPerSecondSquared )
				sensorACC = sensor;
			else if ( sensor->sensorUnit
					== Processing::WaveformProcessor::MeterPerSecond )
				sensorVEL = sensor;
		}

		if ( sensorVEL ) {
			for ( int i = start_idx; i <= end_idx; ++i ) {
				const Cell &cell = sensorVEL->buffer[i];
				if ( cell.envelopes[Z].values[Velocity] >= 0){
					found = true;
					locationCode = sensorVEL->locationCode;
					break;
				}
			}
		}

		if ( sensorACC && !found ) {
			for ( int i = start_idx; i <= end_idx; ++i ) {
				const Cell &cell = sensorACC->buffer[i];
				if ( cell.envelopes[Z].values[Velocity] >= 0){
					found = true;
					locationCode = sensorACC->locationCode;
					break;
				}
			}
		}

		if(!found)
			continue;

		// check whether sensor is within the distance threshold
		DataModel::SensorLocation *loc;
		loc = inv->getSensorLocation(it->first.first, it->first.second, locationCode, _referenceTime);
		if ( loc == NULL ) {
			SEISCOMP_WARNING(
					"%s.%s.%s: sensor location not in inventory: ignoring", it->first.first.c_str(), it->first.second.c_str(), locationCode.c_str());
			continue;
		}
		Math::Geo::delazi(epiclat, epiclon, loc->latitude(), loc->longitude(),
						&distdg, &azi1, &azi2);
		if ( distdg < dthresh ){
			cnt++;
			stationsCounted.insert(it->first);
		}
	}
	string resultstr;
	ostringstream out;
	out << "Stations within dt: ";
	for (StationList::iterator it=stationsCounted.begin(); it!=stationsCounted.end(); ++it){
			out << (*it).first << '.' << (*it).second << ' ';
		}
	resultstr = out.str();
	SEISCOMP_DEBUG("%s", resultstr.c_str());

	stationcount = cnt;
	return no_problem;
}
}// end of namespace
