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


#ifndef __SEISCOMP_IO_RECORDFILTER_SPECTRALIZER__
#define __SEISCOMP_IO_RECORDFILTER_SPECTRALIZER__


#include <map>
#include <deque>

#include <seiscomp3/math/filter.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/genericrecord.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(Spectrum);
class SC_SYSTEM_CORE_API Spectrum : public Core::BaseObject {
	public:
		Spectrum(const Core::Time &stime,
		         const Core::Time &etime,
		         const Core::TimeSpan &dt,
		         double freq, int sampleCount)
		: _startTime(stime), _endTime(etime), _dt(dt), _sampleCount(sampleCount)
		, _frequency(freq) {}

		void setData(ComplexDoubleArray *data) {
			_data = data;
		}

		bool isValid() const { return _data && _data->size() > 0; }

		ComplexDoubleArray *data() { return _data.get(); }

		const Core::Time &startTime() const { return _startTime; }
		const Core::Time &endTime() const { return _endTime; }
		const Core::TimeSpan &dt() const { return _dt; }

		Core::TimeSpan length() const { return _endTime - _startTime; }
		Core::Time center() const { return _startTime + Core::TimeSpan(double(length())*0.5); }

		double minimumFrequency() const { return 0; }
		double maximumFrequency() const { return _frequency; }


	private:
		Core::Time            _startTime;
		Core::Time            _endTime;
		Core::TimeSpan        _dt;
		int                   _sampleCount;
		double                _frequency;
		ComplexDoubleArrayPtr _data;
};



DEFINE_SMARTPOINTER(Spectralizer);
class SC_SYSTEM_CORE_API Spectralizer : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief The Options struct holds the parameters used to
		 * compute the spectrum.
		 */
		struct Options {
			Options();

			//! The window length in seconds used to compute the spectrum.
			double      windowLength;
			//! The window overlap for subsequent spectra. This value is
			//! defined as a fraction of windowLength and must be in [0,1).
			//! The effective time step is windowLength*windowOverlap.
			double      windowOverlap;
			//! The output spectrum samples. A value of -1 returns the full
			//! spectrum whereas values > 0 reduce the spectrum to the given
			//! number of samples using the maximum value of each bin with
			//! respect to magnitude of each spectrum sample.
			int         specSamples;
			//! An optional filter applied in advance to compute a spectrum.
			std::string filter;
			//! Disables aligning the processed time window with the given
			//! time step.
			bool        noalign;
			//! The taper width applied to either side of the processed time
			//! window given as fraction of windowLength, e.g. 0.05 for 5%.
			double      taperWidth;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Spectralizer();
		virtual ~Spectralizer();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setOptions(const Options &opts);

		//! Push an record and compute spectra. Returns true if a records
		//! can be popped. The record id (net,sta,loc,cha) is not checked
		//! against the last record pushed. This filtering is left to
		//! the caller.
		bool push(const Record *rec);

		//! Pops an spectrum if available. Returns NULL if more data are
		//! required.
		Spectrum *pop();

		//! Returns whether records are available
		bool canPop() const { return !_nextSpectra.empty(); }

		//! Clean up all associated resources
		void cleanup();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		typedef Core::SmartPointer< Math::Filtering::InPlaceFilter<double> >::Impl FilterPtr;
		struct SpecBuffer {
			SpecBuffer() {}
			~SpecBuffer() {}

			FilterPtr filter;

			// Fixed source sample rate derived from the first record
			// received.
			double sampleRate;
			double dt;

			// The ring buffer that holds the last samples for fft.
			std::vector<double> buffer;
			DoubleArray tmp;
			int tmpOffset;

			size_t samplesToSkip;

			// The number of samples still missing in the buffer before
			// fft can be done
			size_t missingSamples;

			// The front index of the ring buffer
			size_t front;

			// Time of front of ring buffer
			Core::Time startTime;

			// End time of last record
			Core::Time lastEndTime;

			void reset(FilterPtr refFilter) {
				missingSamples = buffer.size();
				front = 0;
				samplesToSkip = 0;
				startTime = Core::Time();
				lastEndTime = Core::Time();

				if ( refFilter ) {
					filter = refFilter->clone();
					filter->setSamplingFrequency(sampleRate);
				}
				else
					filter = NULL;
			}
		};

		void init(const Record *rec);
		Record *fft(const Record *rec);

		double                        _windowLength;
		double                        _timeStep;
		bool                          _noalign;
		int                           _specSamples;
		double                        _taperWidth;
		FilterPtr                     _filter;
		SpecBuffer                   *_buffer;
		std::deque<Spectrum*>         _nextSpectra;
};


}
}


#endif
