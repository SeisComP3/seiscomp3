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


#ifndef __SEISCOMP_DATAMODEL_SENSORLOCATION_H__
#define __SEISCOMP_DATAMODEL_SENSORLOCATION_H__


#include <seiscomp3/core/datetime.h>
#include <vector>
#include <string>
#include <seiscomp3/datamodel/auxstream.h>
#include <seiscomp3/datamodel/stream.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(SensorLocation);
DEFINE_SMARTPOINTER(AuxStream);
DEFINE_SMARTPOINTER(Stream);

class Station;


class SC_SYSTEM_CORE_API SensorLocationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SensorLocationIndex();
		SensorLocationIndex(const std::string& code,
		                    Seiscomp::Core::Time start);

		//! Copy constructor
		SensorLocationIndex(const SensorLocationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const SensorLocationIndex&) const;
		bool operator!=(const SensorLocationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a sensor location
 */
class SC_SYSTEM_CORE_API SensorLocation : public PublicObject {
	DECLARE_SC_CLASS(SensorLocation);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		SensorLocation();

	public:
		//! Copy constructor
		SensorLocation(const SensorLocation& other);

		//! Constructor with publicID
		SensorLocation(const std::string& publicID);

		//! Destructor
		~SensorLocation();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static SensorLocation* Create();
		static SensorLocation* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static SensorLocation* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		SensorLocation& operator=(const SensorLocation& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const SensorLocation& other) const;
		bool operator!=(const SensorLocation& other) const;

		//! Wrapper that calls operator==
		bool equal(const SensorLocation& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Station code (52.03)
		void setCode(const std::string& code);
		const std::string& code() const;

		//! Start of epoch in ISO datetime format
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of epoch
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Sensor latitude (52.10)
		void setLatitude(const OPT(double)& latitude);
		double latitude() const;

		//! Sensor longitude (52.11)
		void setLongitude(const OPT(double)& longitude);
		double longitude() const;

		//! Sensor elevation (52.12)
		void setElevation(const OPT(double)& elevation);
		double elevation() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const SensorLocationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const SensorLocation* lhs) const;

	
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
		bool add(AuxStream* obj);
		bool add(Stream* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(AuxStream* obj);
		bool remove(Stream* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeAuxStream(size_t i);
		bool removeAuxStream(const AuxStreamIndex& i);
		bool removeStream(size_t i);
		bool removeStream(const StreamIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t auxStreamCount() const;
		size_t streamCount() const;

		//! Index access
		//! @return The object at index i
		AuxStream* auxStream(size_t i) const;
		AuxStream* auxStream(const AuxStreamIndex& i) const;

		Stream* stream(size_t i) const;
		Stream* stream(const StreamIndex& i) const;

		//! Find an object by its unique attribute(s)

		Station* station() const;

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
		SensorLocationIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		OPT(double) _latitude;
		OPT(double) _longitude;
		OPT(double) _elevation;

		// Aggregations
		std::vector<AuxStreamPtr> _auxStreams;
		std::vector<StreamPtr> _streams;

	DECLARE_SC_CLASSFACTORY_FRIEND(SensorLocation);
};


}
}


#endif
