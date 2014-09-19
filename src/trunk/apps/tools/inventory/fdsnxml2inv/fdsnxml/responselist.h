/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_RESPONSELIST_H__
#define __SEISCOMP_FDSNXML_RESPONSELIST_H__


#include <fdsnxml/metadata.h>
#include <vector>
#include <fdsnxml/basefilter.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(ResponseListElement);



DEFINE_SMARTPOINTER(ResponseList);


/**
 * \brief Response: list of frequency, amplitude and phase values.
 * \brief Corresponds to SEED blockette 55.
 */
class ResponseList : public BaseFilter {
	DECLARE_CASTS(ResponseList);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ResponseList();

		//! Copy constructor
		ResponseList(const ResponseList &other);

		//! Destructor
		~ResponseList();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ResponseList& operator=(const ResponseList &other);
		bool operator==(const ResponseList &other) const;

	
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
		bool addElement(ResponseListElement *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeElement(ResponseListElement *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeElement(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t elementCount() const;

		//! Index access
		//! @return The object at index i
		ResponseListElement* element(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Aggregations
		std::vector<ResponseListElementPtr> _elements;
};


}
}


#endif
