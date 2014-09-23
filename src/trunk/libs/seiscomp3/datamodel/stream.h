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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_STREAM_H__
#define __SEISCOMP_DATAMODEL_STREAM_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Stream);

class SensorLocation;


class SC_SYSTEM_CORE_API StreamIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StreamIndex();
		StreamIndex(const std::string& code,
		            Seiscomp::Core::Time start);

		//! Copy constructor
		StreamIndex(const StreamIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const StreamIndex&) const;
		bool operator!=(const StreamIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a stream (channel) with defined
 * \brief frequency response
 */
class SC_SYSTEM_CORE_API Stream : public Object {
	DECLARE_SC_CLASS(Stream);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Stream();

		//! Copy constructor
		Stream(const Stream& other);

		//! Destructor
		~Stream();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Stream& operator=(const Stream& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Stream& other) const;
		bool operator!=(const Stream& other) const;

		//! Wrapper that calls operator==
		bool equal(const Stream& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Stream code (52.04)
		void setCode(const std::string& code);
		const std::string& code() const;

		//! Start of epoch in ISO datetime format (52.22)
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of epoch (52.23)
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const throw(Seiscomp::Core::ValueException);

		//! Reference to datalogger/@publicID
		void setDatalogger(const std::string& datalogger);
		const std::string& datalogger() const;

		//! Reference to datalogger/calibration/@serialNumber
		void setDataloggerSerialNumber(const std::string& dataloggerSerialNumber);
		const std::string& dataloggerSerialNumber() const;

		//! Reference to datalogger/calibration/@channel
		void setDataloggerChannel(const OPT(int)& dataloggerChannel);
		int dataloggerChannel() const throw(Seiscomp::Core::ValueException);

		//! Reference to sensor/@publicID
		void setSensor(const std::string& sensor);
		const std::string& sensor() const;

		//! Reference to sensor/calibration/@serialNumber
		void setSensorSerialNumber(const std::string& sensorSerialNumber);
		const std::string& sensorSerialNumber() const;

		//! Reference to sensor/calibration/@channel
		void setSensorChannel(const OPT(int)& sensorChannel);
		int sensorChannel() const throw(Seiscomp::Core::ValueException);

		//! Serial no. of clock (GPS). Mostly unused
		void setClockSerialNumber(const std::string& clockSerialNumber);
		const std::string& clockSerialNumber() const;

		//! Sample rate numerator (always >0, eg., not identical to
		//! 52.18)
		void setSampleRateNumerator(const OPT(int)& sampleRateNumerator);
		int sampleRateNumerator() const throw(Seiscomp::Core::ValueException);

		//! Sample rate denominator (always >0, eg., not identical to
		//! 52.19)
		void setSampleRateDenominator(const OPT(int)& sampleRateDenominator);
		int sampleRateDenominator() const throw(Seiscomp::Core::ValueException);

		//! Depth (52.13)
		void setDepth(const OPT(double)& depth);
		double depth() const throw(Seiscomp::Core::ValueException);

		//! Azimuth (52.14)
		void setAzimuth(const OPT(double)& azimuth);
		double azimuth() const throw(Seiscomp::Core::ValueException);

		//! Dip (52.15)
		void setDip(const OPT(double)& dip);
		double dip() const throw(Seiscomp::Core::ValueException);

		//! Overall sensitivity (58.04) in counts/gainUnit
		void setGain(const OPT(double)& gain);
		double gain() const throw(Seiscomp::Core::ValueException);

		//! Gain frequency (58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const throw(Seiscomp::Core::ValueException);

		//! Sensor's unit of measurement (eg., M/S, M/S**2)
		void setGainUnit(const std::string& gainUnit);
		const std::string& gainUnit() const;

		//! Data format, eg.: "steim1", "steim2", "mseedN" (N =
		//! encoding format in blockette 1000)
		void setFormat(const std::string& format);
		const std::string& format() const;

		//! Channel flags (52.21)
		void setFlags(const std::string& flags);
		const std::string& flags() const;

		//! Whether the stream is "restricted"
		void setRestricted(const OPT(bool)& restricted);
		bool restricted() const throw(Seiscomp::Core::ValueException);

		//! Whether the metadata is synchronized with other datacenters
		void setShared(const OPT(bool)& shared);
		bool shared() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const StreamIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Stream* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		SensorLocation* sensorLocation() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		StreamIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		std::string _datalogger;
		std::string _dataloggerSerialNumber;
		OPT(int) _dataloggerChannel;
		std::string _sensor;
		std::string _sensorSerialNumber;
		OPT(int) _sensorChannel;
		std::string _clockSerialNumber;
		OPT(int) _sampleRateNumerator;
		OPT(int) _sampleRateDenominator;
		OPT(double) _depth;
		OPT(double) _azimuth;
		OPT(double) _dip;
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		std::string _gainUnit;
		std::string _format;
		std::string _flags;
		OPT(bool) _restricted;
		OPT(bool) _shared;
};


}
}


#endif
