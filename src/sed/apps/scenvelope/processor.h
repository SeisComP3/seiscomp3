/***************************************************************************
 *   Copyright (C) by ETHZ/SED                                             *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#ifndef __SEISCOMP_APPLICATIONS_SCENVELOPE_PROCESSOR_H__
#define __SEISCOMP_APPLICATIONS_SCENVELOPE_PROCESSOR_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/processing/waveformprocessor.h>
#include <seiscomp3/math/filter/iirintegrate.h>
#include <seiscomp3/math/filter/iirdifferentiate.h>

#include <boost/function.hpp>
#include <vector>
#include <stdexcept>

#include "filter.h"


namespace Seiscomp {

class Record;


class AverageBuffer {
	public:
		AverageBuffer(int size);


	public:
		void clear() { _empty = true; }
		void push(double val);
		double sum() const;
		double average() const;


	private:
		void fill(double val);


	private:
		mutable
		std::vector<double> _bins;
		double              _sum;
		size_t              _front;
		bool                _empty;
};


struct Pool {
	Pool() : samples(NULL), _ofs(0), _size(0) {}

	void reset(size_t i) {
		if ( _size == i ) return;
		delete[] samples;
		samples = new double[i];
		_size = i;
		_ofs = 0;
	}

	size_t capacity() const { return _size; }
	size_t size() const { return _ofs; }
	void clear() { _ofs = 0; }

	void push(double val) {
		if ( _ofs < _size ) samples[_ofs++] = val;
		else throw std::overflow_error("pool overflow");
	}

	double *samples;
	size_t  _ofs;
	size_t  _size;
};



DEFINE_SMARTPOINTER(Processor);
class Processor : public Processing::WaveformProcessor {
	public:
		struct Config {
			Config();
			double  saturationThreshold;
			int     interval; // Envelope interval in ms
			bool    useVSFilterImplementation;
		};

		typedef boost::function<void (const Processor*,
		                              double acc, double vel, double disp,
		                              const Core::Time &timestamp,
		                              bool clipped)> PublishFunc;

		typedef Math::Filtering::InPlaceFilter<double> Filter;
		typedef Core::SmartPointer<Filter>::Impl FilterPtr;

		Processor(size_t baselineCorrectionBufferLength)
		: _baselineCorrection0(baselineCorrectionBufferLength),
		  _baselineCorrection1(baselineCorrectionBufferLength),
		  _baselineCorrection2(baselineCorrectionBufferLength) {}

		void setSaturationThreshold(double t) { _config.saturationThreshold = t; }

		//! Sets the name of the channel, like Z, H1 or H2
		void setName(const char *name) {
			_name = name;
		}

		const std::string &name() const { return _name; }

		//! Sets the waveform ID of the channel
		void setWaveformID(const DataModel::WaveformStreamID &wfid) {
			_waveformID = wfid;
		}

		const DataModel::WaveformStreamID &waveformID() const { return _waveformID; }

		//! Sets the envelope interval in milliseconds
		void setInterval(int ms) { _config.interval = ms; }

		//! Sets using the VS butterworth highpass implementation (true) or the
		//! SC3 internal filters (false).
		void useVSFilterImplementation(bool f);

		void setPublishFunction(const PublishFunc& func);

	protected:
		void reset();
		bool init(const Record *record);
		void process(const Record *record, const DoubleArray &data);
		bool handleGap(Filter *filter, const Core::TimeSpan&,
		               double lastSample, double nextSample,
		               size_t missingSamples);

		//! Setup the current time window according to a reference time
		void setupTimeWindow(const Core::Time &ref);

		//! Flushes a sample packet to compute envelopes for
		void flush();

		double getValue(size_t n, double *samples, Filter *conversion,
		                AverageBuffer &buffer, Filter *filter);

		double getAcceleration(size_t n, double *samples);
		double getVelocity(size_t n, double *samples);
		double getDisplacement(size_t n, double *samples);


	private:
		typedef Math::Filtering::IIRIntegrate<double> Integration;
		typedef Math::Filtering::IIRDifferentiate<double> Differentiation;
		Config                            _config;
		AverageBuffer                     _baselineCorrection0;
		AverageBuffer                     _baselineCorrection1;
		AverageBuffer                     _baselineCorrection2;
		FilterPtr                         _filter0;
		FilterPtr                         _filter1;
		Integration                       _toVelocity;
		Differentiation                   _toAcceleration;
		Integration                       _toDisplacement;
		Pool                              _samplePool;
		Core::TimeSpan                    _dt;
		DataModel::WaveformStreamID       _waveformID;
		std::string                       _name;
		Core::Time                        _currentStartTime;
		Core::Time                        _currentEndTime;
		PublishFunc                       _func;
};


}

#endif
