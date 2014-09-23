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


#ifndef __SEISCOMP_DATAMODEL_ROUTING_H__
#define __SEISCOMP_DATAMODEL_ROUTING_H__


#include <vector>
#include <seiscomp3/datamodel/route.h>
#include <seiscomp3/datamodel/access.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Routing);
DEFINE_SMARTPOINTER(Route);
DEFINE_SMARTPOINTER(Access);


class SC_SYSTEM_CORE_API Routing : public PublicObject {
	DECLARE_SC_CLASS(Routing);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Routing();

		//! Copy constructor
		Routing(const Routing& other);

		//! Destructor
		~Routing();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Routing& operator=(const Routing& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Routing& other) const;
		bool operator!=(const Routing& other) const;

		//! Wrapper that calls operator==
		bool equal(const Routing& other) const;

	
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
		bool add(Route* obj);
		bool add(Access* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Route* obj);
		bool remove(Access* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeRoute(size_t i);
		bool removeRoute(const RouteIndex& i);
		bool removeAccess(size_t i);
		bool removeAccess(const AccessIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t routeCount() const;
		size_t accessCount() const;

		//! Index access
		//! @return The object at index i
		Route* route(size_t i) const;
		Route* route(const RouteIndex& i) const;

		Access* access(size_t i) const;
		Access* access(const AccessIndex& i) const;

		//! Find an object by its unique attribute(s)
		Route* findRoute(const std::string& publicID) const;

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
		std::vector<RoutePtr> _routes;
		std::vector<AccessPtr> _accesss;

	DECLARE_SC_CLASSFACTORY_FRIEND(Routing);
};


}
}


#endif
