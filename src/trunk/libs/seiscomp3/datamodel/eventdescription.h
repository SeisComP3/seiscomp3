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


#ifndef __SEISCOMP_DATAMODEL_EVENTDESCRIPTION_H__
#define __SEISCOMP_DATAMODEL_EVENTDESCRIPTION_H__


#include <string>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(EventDescription);

class Event;


class SC_SYSTEM_CORE_API EventDescriptionIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		EventDescriptionIndex();
		EventDescriptionIndex(EventDescriptionType type);

		//! Copy constructor
		EventDescriptionIndex(const EventDescriptionIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const EventDescriptionIndex&) const;
		bool operator!=(const EventDescriptionIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		EventDescriptionType type;
};


/**
 * \brief Free-form string with additional event description. This
 * \brief can be a
 * \brief well-known name, like 1906 San Francisco Earthquake. A
 * \brief number of
 * \brief categories can be given in type.
 */
class SC_SYSTEM_CORE_API EventDescription : public Object {
	DECLARE_SC_CLASS(EventDescription);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		EventDescription();

		//! Copy constructor
		EventDescription(const EventDescription& other);

		//! Custom constructor
		EventDescription(const std::string& text);
		EventDescription(const std::string& text,
		                 EventDescriptionType type);

		//! Destructor
		~EventDescription();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		EventDescription& operator=(const EventDescription& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const EventDescription& other) const;
		bool operator!=(const EventDescription& other) const;

		//! Wrapper that calls operator==
		bool equal(const EventDescription& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Free-form text with earthquake description.
		void setText(const std::string& text);
		const std::string& text() const;

		//! Category of earthquake description.
		void setType(EventDescriptionType type);
		EventDescriptionType type() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const EventDescriptionIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const EventDescription* lhs) const;

	
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
		EventDescriptionIndex _index;

		// Attributes
		std::string _text;
};


}
}


#endif
