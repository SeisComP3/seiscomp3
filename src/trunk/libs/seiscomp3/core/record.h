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


#ifndef __SEISCOMP_CORE_RECORD_H__
#define __SEISCOMP_CORE_RECORD_H__


#include <string>
#include <time.h>
#include <iostream>

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/core/array.h>
#include <seiscomp3/core/exceptions.h>



namespace Seiscomp {


class BitSet;


DEFINE_SMARTPOINTER(Record);

class SC_SYSTEM_CORE_API Record : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(Record);
	DECLARE_SERIALIZATION;

	// ----------------------------------------------------------------------
	//  Public enumeration
	// ----------------------------------------------------------------------
	public:
		//! Specifies the memory storage flags.
		enum Hint {
			META_ONLY,
			DATA_ONLY,
			SAVE_RAW,
			H_QUANTITY
		};
	

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! Default Constructor
		Record(Array::DataType datatype, Hint);

		//! Initializing Constructor
		Record(Array::DataType, Hint,
		       std::string net, std::string sta, std::string loc, std::string cha,
		       Seiscomp::Core::Time stime, int nsamp, double fsamp, int tqual);

		//! Copy Constructor
		Record(const Record &rec);

		//! Destructor
		virtual ~Record();

	
	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		//! Assignment operator
		Record &operator=(const Record &rec);
	

	// ----------------------------------------------------------------------
	//  Public attribute access
	// ----------------------------------------------------------------------
	public:
		//! Returns the network code
		const std::string &networkCode() const;

		//! Sets the network code
		virtual void setNetworkCode(std::string net);

		//! Returns the station code
		const std::string &stationCode() const;

		//! Sets the station code
		virtual void setStationCode(std::string sta);

		//! Returns the location code
		const std::string &locationCode() const;

		//! Sets the location code
		virtual void setLocationCode(std::string loc);

		//! Returns the channel code
		const std::string &channelCode() const;

		//! Sets the channel code
		virtual void setChannelCode(std::string cha);

		//! Returns the start time
		const Core::Time& startTime() const;

		//! Sets the start time
		virtual void setStartTime(const Core::Time& time);

		//! Returns the end time
		Core::Time endTime() const;

		//! Returns the time window between start and end time of a record
		Core::TimeWindow timeWindow() const;
	
		//! Returns the sample number
		int sampleCount() const;

		//! Returns the sample frequency
		double samplingFrequency() const;

		//! Returns the timing quality
		int timingQuality() const;

		//! Sets the timing quality
		void setTimingQuality(int tqual);

		//! Returns the so called stream ID: <net>.<sta>.<loc>.<cha>
		std::string streamID() const;

		//! Returns the data type specified for the data sample requests
		Array::DataType dataType() const;

		//! Sets the data type for the data requests which can differ from the real type of the data samples
		void setDataType(Array::DataType dt);

		//! Sets the hint used for data operations
		void setHint(Hint h);
	

	// ----------------------------------------------------------------------
	//  Public data access
	// ----------------------------------------------------------------------
	public:
		//! Returns a nonmutable pointer to the data samples if the data is available; otherwise 0
		//! (the data type is independent from the original one and was given by the DataType flag in the constructor)
		virtual const Array* data() const = 0;

		//! Returns the raw data of the record if existing
		virtual const Array* raw() const = 0;

		//! Returns a deep copy of the calling object.
		virtual Record* copy() const = 0;

		//! Return the clip mask for the data in the record. The clip mask
		//! holds a bit for each sample and sets that bit to 1 if the sample
		//! is clipped. The default implementation always returns NULL.
		//! Support has to be provided in derived implementations.
		virtual const BitSet *clipMask() const;


	// ----------------------------------------------------------------------
	//  Public methods
	// ----------------------------------------------------------------------
	public:
		//! Frees the memory allocated for the data samples.
		virtual void saveSpace() const = 0;

		virtual void read(std::istream &in) = 0;
		virtual void write(std::ostream &out) = 0;
	

	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		std::string     _net;
		std::string     _sta;
		std::string     _loc;
		std::string     _cha;
		Core::Time      _stime;
		Array::DataType _datatype;
		Hint            _hint;
		int             _nsamp;
		double          _fsamp;
		int             _timequal;
};


DEFINE_INTERFACE_FACTORY(Record);

#define REGISTER_RECORD(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Record, Class> __##Class##InterfaceFactory__(Service)


}


SC_SYSTEM_CORE_API std::istream& operator>>(std::istream &is, Seiscomp::Record &rec);
SC_SYSTEM_CORE_API std::ostream& operator<<(std::ostream &os, Seiscomp::Record &rec);


#endif
