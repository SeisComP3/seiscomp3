/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   Author: Jan Becker, gempa GmbH                                        *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_RECORDSTREAM_DECIMATION_H__
#define __SEISCOMP_RECORDSTREAM_DECIMATION_H__

#include <sstream>
#include <map>

#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace RecordStream {

DEFINE_SMARTPOINTER(Decimation);

class SC_SYSTEM_CORE_API Decimation : public Seiscomp::IO::RecordStream {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		Decimation();
		virtual ~Decimation();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setSource(const std::string &source);
		virtual bool setRecordType(const char *type);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &stime,
		                       const Seiscomp::Core::Time &etime);

		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);
		virtual bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);

		virtual bool setTimeout(int seconds);

		virtual void close();

		virtual Record *next();


	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		void cleanup();

		int checkSR(Record *rec) const;

		bool push(Record *rec);
		GenericRecord *convert(Record *rec);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		typedef std::vector<double> Coefficients;

		struct ResampleStage {
			ResampleStage() : nextStage(NULL) {}
			~ResampleStage() { if ( nextStage ) delete nextStage; }

			double targetRate;

			// Fixed source sample rate derived from the first record
			// received.
			double sampleRate;
			double dt;

			// Flag that indicates that a streams is passed through
			// without resampling.
			bool passThrough;

			int N;
			int N2;
			size_t samplesToSkip;

			Coefficients *coefficients;

			// The ring buffer that holds the last samples for downsampling.
			std::vector<double> buffer;

			// The number of samples still missing in the buffer before
			// filtering can be done
			size_t missingSamples;

			// The front index of the ring buffer
			size_t front;

			// Time of front of ring buffer
			Core::Time startTime;

			// End time of last record
			Core::Time lastEndTime;

			ResampleStage *nextStage;

			void reset() {
				missingSamples = buffer.size();
				front = 0;
				samplesToSkip = 0;
				startTime = Core::Time();
				lastEndTime = Core::Time();

				if ( nextStage ) nextStage->reset();
			}
		};

		typedef std::map<int, Coefficients*> CoefficientMap;
		typedef std::map<std::string, ResampleStage*> StreamMap;

		void init(ResampleStage *stage, Record *rec);
		void initCoefficients(ResampleStage *stage);
		GenericRecord *resample(ResampleStage *stage, Record *rec);

		IO::RecordStreamPtr _source;
		double              _targetRate;
		double              _fp;
		double              _fs;
		int                 _maxN;
		int                 _coeffScale;
		StreamMap           _streams;
		CoefficientMap      _coefficients;
		GenericRecord      *_nextRecord;
};

}
}

#endif
