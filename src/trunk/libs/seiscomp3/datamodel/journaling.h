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


#ifndef __SEISCOMP_DATAMODEL_JOURNALING_H__
#define __SEISCOMP_DATAMODEL_JOURNALING_H__


#include <vector>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Journaling);
DEFINE_SMARTPOINTER(JournalEntry);


class SC_SYSTEM_CORE_API Journaling : public PublicObject {
	DECLARE_SC_CLASS(Journaling);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Journaling();

		//! Copy constructor
		Journaling(const Journaling& other);

		//! Destructor
		~Journaling();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Journaling& operator=(const Journaling& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Journaling& other) const;
		bool operator!=(const Journaling& other) const;

		//! Wrapper that calls operator==
		bool equal(const Journaling& other) const;

	
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
		bool add(JournalEntry* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(JournalEntry* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeJournalEntry(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t journalEntryCount() const;

		//! Index access
		//! @return The object at index i
		JournalEntry* journalEntry(size_t i) const;

		//! Find an object by its unique attribute(s)
		JournalEntry* findJournalEntry(JournalEntry* journalEntry) const;

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
		// Aggregations
		std::vector<JournalEntryPtr> _journalEntrys;

	DECLARE_SC_CLASSFACTORY_FRIEND(Journaling);
};


}
}


#endif
