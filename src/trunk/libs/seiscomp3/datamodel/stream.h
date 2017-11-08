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
#include <vector>
#include <string>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Stream);
DEFINE_SMARTPOINTER(Comment);

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
class SC_SYSTEM_CORE_API Stream : public PublicObject {
	DECLARE_SC_CLASS(Stream);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Stream();

	public:
		//! Copy constructor
		Stream(const Stream& other);

		//! Constructor with publicID
		Stream(const std::string& publicID);

		//! Destructor
		~Stream();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Stream* Create();
		static Stream* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Stream* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
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
		Seiscomp::Core::Time end() const;

		//! Reference to datalogger/@publicID
		void setDatalogger(const std::string& datalogger);
		const std::string& datalogger() const;

		//! Reference to datalogger/calibration/@serialNumber
		void setDataloggerSerialNumber(const std::string& dataloggerSerialNumber);
		const std::string& dataloggerSerialNumber() const;

		//! Reference to datalogger/calibration/@channel
		void setDataloggerChannel(const OPT(int)& dataloggerChannel);
		int dataloggerChannel() const;

		//! Reference to sensor/@publicID
		void setSensor(const std::string& sensor);
		const std::string& sensor() const;

		//! Reference to sensor/calibration/@serialNumber
		void setSensorSerialNumber(const std::string& sensorSerialNumber);
		const std::string& sensorSerialNumber() const;

		//! Reference to sensor/calibration/@channel
		void setSensorChannel(const OPT(int)& sensorChannel);
		int sensorChannel() const;

		//! Serial no. of clock (GPS). Mostly unused
		void setClockSerialNumber(const std::string& clockSerialNumber);
		const std::string& clockSerialNumber() const;

		//! Sample rate numerator (always >0, eg., not identical to
		//! 52.18)
		void setSampleRateNumerator(const OPT(int)& sampleRateNumerator);
		int sampleRateNumerator() const;

		//! Sample rate denominator (always >0, eg., not identical to
		//! 52.19)
		void setSampleRateDenominator(const OPT(int)& sampleRateDenominator);
		int sampleRateDenominator() const;

		//! Depth (52.13)
		void setDepth(const OPT(double)& depth);
		double depth() const;

		//! Azimuth (52.14)
		void setAzimuth(const OPT(double)& azimuth);
		double azimuth() const;

		//! Dip (52.15)
		void setDip(const OPT(double)& dip);
		double dip() const;

		//! Overall sensitivity (58.04) in counts/gainUnit
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Gain frequency (58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const;

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
		bool restricted() const;

		//! Whether the metadata is synchronized with other datacenters
		void setShared(const OPT(bool)& shared);
		bool shared() const;


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
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(Comment* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;

		//! Find an object by its unique attribute(s)

		SensorLocation* sensorLocation() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

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

		// Aggregations
		std::vector<CommentPtr> _comments;

	DECLARE_SC_CLASSFACTORY_FRIEND(Stream);
};


}
}


#endif
