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


#ifndef __SEISCOMP_DATAMODEL_FOCALMECHANISMREFERENCE_H__
#define __SEISCOMP_DATAMODEL_FOCALMECHANISMREFERENCE_H__


#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(FocalMechanismReference);

class Event;


class SC_SYSTEM_CORE_API FocalMechanismReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FocalMechanismReferenceIndex();
		FocalMechanismReferenceIndex(const std::string& focalMechanismID);

		//! Copy constructor
		FocalMechanismReferenceIndex(const FocalMechanismReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const FocalMechanismReferenceIndex&) const;
		bool operator!=(const FocalMechanismReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string focalMechanismID;
};


class SC_SYSTEM_CORE_API FocalMechanismReference : public Object {
	DECLARE_SC_CLASS(FocalMechanismReference);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FocalMechanismReference();

		//! Copy constructor
		FocalMechanismReference(const FocalMechanismReference& other);

		//! Custom constructor
		FocalMechanismReference(const std::string& focalMechanismID);

		//! Destructor
		~FocalMechanismReference();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		FocalMechanismReference& operator=(const FocalMechanismReference& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const FocalMechanismReference& other) const;
		bool operator!=(const FocalMechanismReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const FocalMechanismReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setFocalMechanismID(const std::string& focalMechanismID);
		const std::string& focalMechanismID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const FocalMechanismReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const FocalMechanismReference* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Event* event() const;

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
		FocalMechanismReferenceIndex _index;
};


}
}


#endif
