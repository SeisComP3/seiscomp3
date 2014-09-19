/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_OPERATOR_H__
#define __SEISCOMP_STATIONXML_OPERATOR_H__


#include <stationxml/metadata.h>
#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(AgencyType);
DEFINE_SMARTPOINTER(Contact);



DEFINE_SMARTPOINTER(Operator);


/**
 * \brief An operating agency and associated contact persons. If there
 * \brief multiple operators, each one should be encapsulated within an
 * \brief Operator tag. Since the Contact element is a generic type that
 * \brief represents any contact person, it also has its own optional Agency
 * \brief element.
 */
class Operator : public Core::BaseObject {
	DECLARE_CASTS(Operator);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Operator();

		//! Copy constructor
		Operator(const Operator& other);

		//! Destructor
		~Operator();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Operator& operator=(const Operator& other);
		bool operator==(const Operator& other) const;

	
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
		bool addAgency(AgencyType* obj);
		bool addContact(Contact* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeAgency(AgencyType* obj);
		bool removeContact(Contact* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeAgency(size_t i);
		bool removeContact(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t agencyCount() const;
		size_t contactCount() const;

		//! Index access
		//! @return The object at index i
		AgencyType* agency(size_t i) const;
		Contact* contact(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Aggregations
		std::vector<AgencyTypePtr> _agencys;
		std::vector<ContactPtr> _contacts;
};


}
}


#endif
