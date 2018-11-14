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


#ifndef __SEISCOMP_DATAMODEL_ARCLINKLOG_H__
#define __SEISCOMP_DATAMODEL_ARCLINKLOG_H__


#include <vector>
#include <seiscomp3/datamodel/arclinkrequest.h>
#include <seiscomp3/datamodel/arclinkuser.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkLog);
DEFINE_SMARTPOINTER(ArclinkRequest);
DEFINE_SMARTPOINTER(ArclinkUser);


class SC_SYSTEM_CORE_API ArclinkLog : public PublicObject {
	DECLARE_SC_CLASS(ArclinkLog);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkLog();

		//! Copy constructor
		ArclinkLog(const ArclinkLog& other);

		//! Destructor
		~ArclinkLog();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ArclinkLog& operator=(const ArclinkLog& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ArclinkLog& other) const;
		bool operator!=(const ArclinkLog& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkLog& other) const;

	
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
		bool add(ArclinkRequest* obj);
		bool add(ArclinkUser* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(ArclinkRequest* obj);
		bool remove(ArclinkUser* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeArclinkRequest(size_t i);
		bool removeArclinkRequest(const ArclinkRequestIndex& i);
		bool removeArclinkUser(size_t i);
		bool removeArclinkUser(const ArclinkUserIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t arclinkRequestCount() const;
		size_t arclinkUserCount() const;

		//! Index access
		//! @return The object at index i
		ArclinkRequest* arclinkRequest(size_t i) const;
		ArclinkRequest* arclinkRequest(const ArclinkRequestIndex& i) const;

		ArclinkUser* arclinkUser(size_t i) const;
		ArclinkUser* arclinkUser(const ArclinkUserIndex& i) const;

		//! Find an object by its unique attribute(s)
		ArclinkRequest* findArclinkRequest(const std::string& publicID) const;
		ArclinkUser* findArclinkUser(const std::string& publicID) const;

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
		std::vector<ArclinkRequestPtr> _arclinkRequests;
		std::vector<ArclinkUserPtr> _arclinkUsers;

	DECLARE_SC_CLASSFACTORY_FRIEND(ArclinkLog);
};


}
}


#endif
