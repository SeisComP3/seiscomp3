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


#ifndef __SEISCOMP_DATAMODEL_PICKREFERENCE_H__
#define __SEISCOMP_DATAMODEL_PICKREFERENCE_H__


#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(PickReference);

class Reading;


class SC_SYSTEM_CORE_API PickReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PickReferenceIndex();
		PickReferenceIndex(const std::string& pickID);

		//! Copy constructor
		PickReferenceIndex(const PickReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const PickReferenceIndex&) const;
		bool operator!=(const PickReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string pickID;
};


class SC_SYSTEM_CORE_API PickReference : public Object {
	DECLARE_SC_CLASS(PickReference);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PickReference();

		//! Copy constructor
		PickReference(const PickReference& other);

		//! Custom constructor
		PickReference(const std::string& pickID);

		//! Destructor
		~PickReference();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PickReference& operator=(const PickReference& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const PickReference& other) const;
		bool operator!=(const PickReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const PickReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setPickID(const std::string& pickID);
		const std::string& pickID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const PickReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const PickReference* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Reading* reading() const;

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
		PickReferenceIndex _index;
};


}
}


#endif
