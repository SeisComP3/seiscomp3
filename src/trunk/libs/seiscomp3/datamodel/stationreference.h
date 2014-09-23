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


#ifndef __SEISCOMP_DATAMODEL_STATIONREFERENCE_H__
#define __SEISCOMP_DATAMODEL_STATIONREFERENCE_H__


#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(StationReference);

class StationGroup;


class SC_SYSTEM_CORE_API StationReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationReferenceIndex();
		StationReferenceIndex(const std::string& stationID);

		//! Copy constructor
		StationReferenceIndex(const StationReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const StationReferenceIndex&) const;
		bool operator!=(const StationReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string stationID;
};


/**
 * \brief This type describes a station reference within a station
 * \brief group
 */
class SC_SYSTEM_CORE_API StationReference : public Object {
	DECLARE_SC_CLASS(StationReference);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationReference();

		//! Copy constructor
		StationReference(const StationReference& other);

		//! Custom constructor
		StationReference(const std::string& stationID);

		//! Destructor
		~StationReference();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		StationReference& operator=(const StationReference& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const StationReference& other) const;
		bool operator!=(const StationReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const StationReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Reference to network/station/@publicID
		void setStationID(const std::string& stationID);
		const std::string& stationID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const StationReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const StationReference* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		StationGroup* stationGroup() const;

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
		StationReferenceIndex _index;
};


}
}


#endif
