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


#ifndef __SEISCOMP_DATAMODEL_AMPLITUDEREFERENCE_H__
#define __SEISCOMP_DATAMODEL_AMPLITUDEREFERENCE_H__


#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(AmplitudeReference);

class Reading;


class SC_SYSTEM_CORE_API AmplitudeReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AmplitudeReferenceIndex();
		AmplitudeReferenceIndex(const std::string& amplitudeID);

		//! Copy constructor
		AmplitudeReferenceIndex(const AmplitudeReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AmplitudeReferenceIndex&) const;
		bool operator!=(const AmplitudeReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string amplitudeID;
};


class SC_SYSTEM_CORE_API AmplitudeReference : public Object {
	DECLARE_SC_CLASS(AmplitudeReference);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AmplitudeReference();

		//! Copy constructor
		AmplitudeReference(const AmplitudeReference& other);

		//! Custom constructor
		AmplitudeReference(const std::string& amplitudeID);

		//! Destructor
		~AmplitudeReference();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AmplitudeReference& operator=(const AmplitudeReference& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const AmplitudeReference& other) const;
		bool operator!=(const AmplitudeReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const AmplitudeReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setAmplitudeID(const std::string& amplitudeID);
		const std::string& amplitudeID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const AmplitudeReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const AmplitudeReference* lhs) const;

	
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
		AmplitudeReferenceIndex _index;
};


}
}


#endif
