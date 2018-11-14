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


#ifndef __SEISCOMP_DATAMODEL_JOURNALENTRY_H__
#define __SEISCOMP_DATAMODEL_JOURNALENTRY_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(JournalEntry);

class Journaling;


class SC_SYSTEM_CORE_API JournalEntry : public Object {
	DECLARE_SC_CLASS(JournalEntry);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		JournalEntry();

		//! Copy constructor
		JournalEntry(const JournalEntry& other);

		//! Destructor
		~JournalEntry();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		JournalEntry& operator=(const JournalEntry& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const JournalEntry& other) const;
		bool operator!=(const JournalEntry& other) const;

		//! Wrapper that calls operator==
		bool equal(const JournalEntry& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setCreated(const OPT(Seiscomp::Core::Time)& created);
		Seiscomp::Core::Time created() const;

		void setObjectID(const std::string& objectID);
		const std::string& objectID() const;

		void setSender(const std::string& sender);
		const std::string& sender() const;

		void setAction(const std::string& action);
		const std::string& action() const;

		void setParameters(const std::string& parameters);
		const std::string& parameters() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Journaling* journaling() const;

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
		// Attributes
		OPT(Seiscomp::Core::Time) _created;
		std::string _objectID;
		std::string _sender;
		std::string _action;
		std::string _parameters;
};


}
}


#endif
