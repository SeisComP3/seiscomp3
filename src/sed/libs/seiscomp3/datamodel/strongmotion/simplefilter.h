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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_SIMPLEFILTER_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_SIMPLEFILTER_H__


#include <vector>
#include <string>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(SimpleFilter);
DEFINE_SMARTPOINTER(FilterParameter);

class StrongMotionParameters;


class SC_STRONGMOTION_API SimpleFilter : public PublicObject {
	DECLARE_SC_CLASS(SimpleFilter);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		SimpleFilter();

	public:
		//! Copy constructor
		SimpleFilter(const SimpleFilter& other);

		//! Constructor with publicID
		SimpleFilter(const std::string& publicID);

		//! Destructor
		~SimpleFilter();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static SimpleFilter* Create();
		static SimpleFilter* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static SimpleFilter* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		SimpleFilter& operator=(const SimpleFilter& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const SimpleFilter& other) const;
		bool operator!=(const SimpleFilter& other) const;

		//! Wrapper that calls operator==
		bool equal(const SimpleFilter& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setType(const std::string& type);
		const std::string& type() const;

	
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
		bool add(FilterParameter* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(FilterParameter* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeFilterParameter(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t filterParameterCount() const;

		//! Index access
		//! @return The object at index i
		FilterParameter* filterParameter(size_t i) const;

		//! Find an object by its unique attribute(s)
		FilterParameter* findFilterParameter(FilterParameter* filterParameter) const;

		StrongMotionParameters* strongMotionParameters() const;

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
		// Attributes
		std::string _type;

		// Aggregations
		std::vector<FilterParameterPtr> _filterParameters;

	DECLARE_SC_CLASSFACTORY_FRIEND(SimpleFilter);
};


}
}
}


#endif
