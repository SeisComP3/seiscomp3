/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_PERSON_H__
#define __SEISCOMP_FDSNXML_PERSON_H__


#include <fdsnxml/metadata.h>
#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(Name);
DEFINE_SMARTPOINTER(Agency);
DEFINE_SMARTPOINTER(Email);
DEFINE_SMARTPOINTER(Phone);



DEFINE_SMARTPOINTER(Person);


/**
 * \brief Representation of a person's contact information. A person can
 * \brief belong to multiple agencies and have multiple email addresses and
 * \brief phone numbers.
 */
class Person : public Core::BaseObject {
	DECLARE_CASTS(Person);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Person();

		//! Copy constructor
		Person(const Person &other);

		//! Destructor
		~Person();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Person& operator=(const Person &other);
		bool operator==(const Person &other) const;

	
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
		bool addName(Name *obj);
		bool addAgency(Agency *obj);
		bool addEmail(Email *obj);
		bool addPhone(Phone *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeName(Name *obj);
		bool removeAgency(Agency *obj);
		bool removeEmail(Email *obj);
		bool removePhone(Phone *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeName(size_t i);
		bool removeAgency(size_t i);
		bool removeEmail(size_t i);
		bool removePhone(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t nameCount() const;
		size_t agencyCount() const;
		size_t emailCount() const;
		size_t phoneCount() const;

		//! Index access
		//! @return The object at index i
		Name* name(size_t i) const;
		Agency* agency(size_t i) const;
		Email* email(size_t i) const;
		Phone* phone(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Aggregations
		std::vector<NamePtr> _names;
		std::vector<AgencyPtr> _agencys;
		std::vector<EmailPtr> _emails;
		std::vector<PhonePtr> _phones;
};


}
}


#endif
