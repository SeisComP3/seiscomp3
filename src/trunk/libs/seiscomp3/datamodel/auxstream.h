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


#ifndef __SEISCOMP_DATAMODEL_AUXSTREAM_H__
#define __SEISCOMP_DATAMODEL_AUXSTREAM_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(AuxStream);

class SensorLocation;


class SC_SYSTEM_CORE_API AuxStreamIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AuxStreamIndex();
		AuxStreamIndex(const std::string& code,
		               Seiscomp::Core::Time start);

		//! Copy constructor
		AuxStreamIndex(const AuxStreamIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AuxStreamIndex&) const;
		bool operator!=(const AuxStreamIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a stream (channel) without defined
 * \brief frequency response
 */
class SC_SYSTEM_CORE_API AuxStream : public Object {
	DECLARE_SC_CLASS(AuxStream);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AuxStream();

		//! Copy constructor
		AuxStream(const AuxStream& other);

		//! Destructor
		~AuxStream();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AuxStream& operator=(const AuxStream& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const AuxStream& other) const;
		bool operator!=(const AuxStream& other) const;

		//! Wrapper that calls operator==
		bool equal(const AuxStream& other) const;


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

		//! Reference to auxDevice/@publicID
		void setDevice(const std::string& device);
		const std::string& device() const;

		//! Serial number of device
		void setDeviceSerialNumber(const std::string& deviceSerialNumber);
		const std::string& deviceSerialNumber() const;

		//! Reference to auxSource/@name
		void setSource(const std::string& source);
		const std::string& source() const;

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
		const AuxStreamIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const AuxStream* lhs) const;

	
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
		AuxStreamIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		std::string _device;
		std::string _deviceSerialNumber;
		std::string _source;
		std::string _format;
		std::string _flags;
		OPT(bool) _restricted;
		OPT(bool) _shared;
};


}
}


#endif
