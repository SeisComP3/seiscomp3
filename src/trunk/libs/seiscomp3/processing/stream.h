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


#ifndef __SEISCOMP_PROCESSING_STREAM_H__
#define __SEISCOMP_PROCESSING_STREAM_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/processing/sensor.h>


namespace Seiscomp {

namespace DataModel {

class Stream;

}

namespace Processing  {


DEFINE_SMARTPOINTER(Stream);

class SC_SYSTEM_CLIENT_API Stream : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Stream();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Initialized the stream meta data from the inventory
		//! if available
		void init(const std::string &networkCode,
		          const std::string &stationCode,
		          const std::string &locationCode,
		          const std::string &channelCode,
		          const Core::Time &time);

		void init(const DataModel::Stream *stream);

		void setCode(const std::string &code);
		const std::string &code() const;

		Sensor *sensor() const;
		void setSensor(Sensor *sensor);

		//! Applies the gain the data.
		void applyGain(int n, double *inout);
		void applyGain(DoubleArray &inout);

		//! Removes the gain the data.
		void removeGain(int n, double *inout);
		void removeGain(DoubleArray &inout);


	public:
		Core::TimeWindow epoch;

		double      gain;
		OPT(double) gainFrequency;
		std::string gainUnit;
		double      azimuth;
		double      dip;


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		// Sensor information
		SensorPtr   _sensor;

		// Stream code
		std::string _code;
};


}
}

#endif
