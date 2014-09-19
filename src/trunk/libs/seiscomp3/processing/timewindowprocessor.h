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


#ifndef __SEISCOMP_PROCESSING_TIMEWINDOWPROCESSOR__
#define __SEISCOMP_PROCESSING_TIMEWINDOWPROCESSOR__

#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/processing/waveformprocessor.h>


namespace Seiscomp {

namespace Processing {


DEFINE_SMARTPOINTER(TimeWindowProcessor);

class SC_SYSTEM_CLIENT_API TimeWindowProcessor : public WaveformProcessor {
	DECLARE_SC_CLASS(TimeWindowProcessor);

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		TimeWindowProcessor();

		//! D'tor
		~TimeWindowProcessor();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		virtual void reset();

		//! Sets the time window for the data to be fed
		void setTimeWindow(const Core::TimeWindow &tw);
		const Core::TimeWindow &timeWindow() const;

		//! Returns the time window including the safety
		//! margin.
		const Core::TimeWindow &safetyTimeWindow() const;

		//! Sets the timewindow margin added to the timewindow
		//! when feeding the data into the processor. The default
		//! margin are 60 seconds.
		void setMargin(const Core::TimeSpan&);
		const Core::TimeSpan& margin() const;

		//! Derived classes should implement this method to
		//! compute their needed timewindow
		virtual void computeTimeWindow() {}

		//! Returns the continuous data for the requested timewindow
		const DoubleArray &continuousData() const;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		virtual void fill(size_t n, double *samples);
		virtual bool store(const Record *rec);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	protected:
		DoubleArray _data;

	private:
		Core::TimeWindow _timeWindow;
		Core::TimeWindow _safetyTimeWindow;
		Core::TimeSpan _safetyMargin;
};


}

}

#endif
