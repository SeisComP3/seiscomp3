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


#ifndef __SEISCOMP_DATAMODEL_DATAAVAILABILITY_H__
#define __SEISCOMP_DATAMODEL_DATAAVAILABILITY_H__


#include <vector>
#include <seiscomp3/datamodel/dataextent.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataAvailability);
DEFINE_SMARTPOINTER(DataExtent);


/**
 * \brief This type can hold data availability related objects
 * \brief (extent and segment).
 */
class SC_SYSTEM_CORE_API DataAvailability : public PublicObject {
	DECLARE_SC_CLASS(DataAvailability);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataAvailability();

		//! Copy constructor
		DataAvailability(const DataAvailability& other);

		//! Destructor
		~DataAvailability();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		DataAvailability& operator=(const DataAvailability& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const DataAvailability& other) const;
		bool operator!=(const DataAvailability& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataAvailability& other) const;

	
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
		bool add(DataExtent* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(DataExtent* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeDataExtent(size_t i);
		bool removeDataExtent(const DataExtentIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t dataExtentCount() const;

		//! Index access
		//! @return The object at index i
		DataExtent* dataExtent(size_t i) const;
		DataExtent* dataExtent(const DataExtentIndex& i) const;

		//! Find an object by its unique attribute(s)
		DataExtent* findDataExtent(const std::string& publicID) const;

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
		std::vector<DataExtentPtr> _dataExtents;

	DECLARE_SC_CLASSFACTORY_FRIEND(DataAvailability);
};


}
}


#endif
