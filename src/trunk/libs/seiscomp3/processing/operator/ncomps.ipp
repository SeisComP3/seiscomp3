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


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/typedarray.h>


namespace Seiscomp {
namespace Processing {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, int N, class PROC, int BSIZE>
WaveformProcessor::Status NCompsOperator<T,N,PROC,BSIZE>::process(int, const Record *rec) {
	Core::Time minStartTime;
	Core::Time maxStartTime;
	Core::Time minEndTime;
	WaveformProcessor::Status status;

	status = WaveformProcessor::WaitingForData;

	for ( int i = 0; i < N; ++i ) {
		// Not all traces available, nothing to do
		if ( _states[i].endTime.valid() ) {
			if ( _states[i].endTime > minStartTime )
				minStartTime = _states[i].endTime;
		}

		if ( _states[i].buffer.empty() ) {
			return status;
		}
	}


	// Find common start time for all three components
	RecordSequence::iterator it[N];
	RecordSequence::iterator it_end[N];
	int maxStartComponent;
	int skips;
	double samplingFrequency, timeTolerance;

	// Initialize iterators for each component
	for ( int i = 0; i < N; ++i )
		it[i] = _states[i].buffer.begin();

	// Store sampling frequency of first record of first component
	// All records must match this sampling frequency
	samplingFrequency = (*it[0])->samplingFrequency();
	timeTolerance = 0.5 / samplingFrequency;

	while ( true ) {
		if ( minStartTime ) {
			for ( int i = 0; i < N; ++i ) {
				while ( it[i] != _states[i].buffer.end() ) {
					if ( (*it[i])->endTime() <= minStartTime ) {
						it[i] = _states[i].buffer.erase(it[i]);
					}
					else
						break;
				}

				// End of stream?
				if ( it[i] == _states[i].buffer.end() )
					return status;
			}
		}

		// Advance all other components to first record matching
		// the first sampling frequency found
		for ( int i = 0; i < N; ++i ) {
			if ( ((*it[i])->samplingFrequency() != samplingFrequency) )
				return WaveformProcessor::InvalidSamplingFreq;
		}

		// Find maximum start time of all three records
		skips = 1;
		while ( skips ) {
			maxStartComponent = -1;

			for ( int i = 0; i < N; ++i ) {
				if ( !i || maxStartTime < (*it[i])->startTime() ) {
					maxStartTime = (*it[i])->startTime();
					maxStartComponent = i;
				}
			}

			skips = 0;

			// Check all other components against maxStartTime
			for ( int i = 0; i < N; ++i ) {
				if ( i == maxStartComponent ) continue;

				while ( (*it[i])->endTime() <= maxStartTime ) {
					if ( (*it[i])->samplingFrequency() != samplingFrequency )
						return WaveformProcessor::InvalidSamplingFreq;

					++it[i];

					// End of sequence? Nothing can be done anymore
					if ( it[i] == _states[i].buffer.end() )
						return status;

					// Increase skip counter
					++skips;
				}
			}
		}

		// Advance all iterators to last non-gappy record
		for ( int i = 0; i < N; ++i ) {
			RecordSequence::iterator tmp = it[i];
			it_end[i] = it[i];
			++it_end[i];
			while ( it_end[i] != _states[i].buffer.end() ) {
				const Record *rec = it_end[i]->get();

				// Skip records with wrong sampling frequency
				if ( rec->samplingFrequency() != samplingFrequency )
					return WaveformProcessor::InvalidSamplingFreq;

				double diff = (double)(rec->startTime()-(*tmp)->endTime());
				if ( fabs(diff) > timeTolerance ) break;

				tmp = it_end[i];
				++it_end[i];
			}

			it_end[i] = tmp;
		}

		// Find minimum end time of all three records
		for ( int i = 0; i < N; ++i ) {
			if ( !i || minEndTime > (*it_end[i])->endTime() )
				minEndTime = (*it_end[i])->endTime();
		}

		GenericRecordPtr comps[N];
		int minLen = 0;

		// Clip maxStartTime to minStartTime
		if ( maxStartTime < minStartTime )
			maxStartTime = minStartTime;

		// Align records
		for ( int i = 0; i < N; ++i ) {
			float tq = 0;
			int tqCount = 0;
			GenericRecordPtr rec = new GenericRecord((*it[i])->networkCode(),
			                                         (*it[i])->stationCode(),
			                                         (*it[i])->locationCode(),
			                                         (*it[i])->channelCode(),
			                                         maxStartTime, samplingFrequency);

			typename Core::SmartPointer< NumericArray<T> >::Impl data = new NumericArray<T>;
			RecordSequence::iterator seq_end = it_end[i];
			++seq_end;

			for ( RecordSequence::iterator rec_it = it[i]; rec_it != seq_end;  ) {
				const Array *rec_data = (*rec_it)->data();
				if ( (*rec_it)->startTime() > minEndTime )
					break;

				++it[i];

				const NumericArray<T> *srcData = NumericArray<T>::ConstCast(rec_data);
				typename Core::SmartPointer< NumericArray<T> >::Impl tmp;
				if ( srcData == NULL ) {
					tmp = (NumericArray<T>*)rec_data->copy(NumericArray<T>::ArrayType);
					srcData = tmp.get();
				}

				int startIndex = 0;
				int endIndex = srcData->size();

				if ( (*rec_it)->startTime() < maxStartTime )
					startIndex += (int)(double(maxStartTime-(*rec_it)->startTime())*(*rec_it)->samplingFrequency());

				if ( (*rec_it)->endTime() > minEndTime )
					endIndex -= (int)(double((*rec_it)->endTime()-minEndTime)*(*rec_it)->samplingFrequency());

				int len = endIndex-startIndex;
				// Skip empty records
				if ( len <= 0 ) {
					++rec_it;
					continue;
				}

				if ( (*rec_it)->timingQuality() >= 0 ) {
					tq += (*rec_it)->timingQuality();
					++tqCount;
				}

				data->append(len, srcData->typedData()+startIndex);
				++rec_it;
			}

			if ( tqCount > 0 )
				rec->setTimingQuality((int)(tq / tqCount));

			minLen = i==0?data->size():std::min(minLen, data->size());

			rec->setData(data.get());

			comps[i] = rec;
		}

		T *data[N];

		for ( int i = 0; i < N; ++i ) {
			NumericArray<T> *ar = static_cast<NumericArray<T>*>(comps[i]->data());
			if ( ar->size() > minLen ) {
				ar->resize(minLen);
				comps[i]->dataUpdated();
			}

			data[i] = static_cast<NumericArray<T>*>(comps[i]->data())->typedData();

			// Set last transformed end time of component
			if ( !_states[i].endTime.valid() ||
			     _states[i].endTime < comps[i]->endTime() ) {
				_states[i].endTime = comps[i]->endTime();
			}
		}

		if ( minLen > 0 ) {
			// Process finally
			_proc(rec, data, minLen, maxStartTime, rec->samplingFrequency());

			for ( int i = 0; i < N; ++i )
				if ( _proc.publish(i) ) store(comps[i].get());
		}

		status = WaveformProcessor::InProgress;

		minStartTime = minEndTime;
	}

	return status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, int N, class PROC, int BSIZE>
WaveformProcessor::Status NCompsOperator<T,N,PROC,BSIZE>::feed(const Record *rec) {
	if ( rec->data() == NULL ) return WaveformProcessor::WaitingForData;

	int i = _proc.compIndex(rec->channelCode());
	if ( i >= 0 ) {
		_states[i].buffer.feed(rec);
		return process(i, rec);
	}

	return WaveformProcessor::WaitingForData;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, int N, class PROC, int BSIZE>
void NCompsOperator<T,N,PROC,BSIZE>::reset() {
	for ( int i = 0; i < N; ++i )
		_states[i] = State();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
