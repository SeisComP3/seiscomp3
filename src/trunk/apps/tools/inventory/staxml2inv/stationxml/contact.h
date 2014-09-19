/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_CONTACT_H__
#define __SEISCOMP_STATIONXML_CONTACT_H__


#include <stationxml/metadata.h>
#include <vector>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(AgencyType);
DEFINE_SMARTPOINTER(EmailType);
DEFINE_SMARTPOINTER(PhoneType);



DEFINE_SMARTPOINTER(Contact);


/**
 * \brief Representation of a person's contact information. A person can
 * \brief belong to multiple agencies and have multiple email addresses and
 * \brief phone numbers.
 */
class Contact : public Core::BaseObject {
	DECLARE_CASTS(Contact);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Contact();

		//! Copy constructor
		Contact(const Contact& other);

		//! Destructor
		~Contact();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Contact& operator=(const Contact& other);
		bool operator==(const Contact& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Name
		void setName(const std::string& name);
		const std::string& name() const;

	
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
		bool addEmail(EmailType* obj);
		bool addPhone(PhoneType* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeAgency(AgencyType* obj);
		bool removeEmail(EmailType* obj);
		bool removePhone(PhoneType* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeAgency(size_t i);
		bool removeEmail(size_t i);
		bool removePhone(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t agencyCount() const;
		size_t emailCount() const;
		size_t phoneCount() const;

		//! Index access
		//! @return The object at index i
		AgencyType* agency(size_t i) const;
		EmailType* email(size_t i) const;
		PhoneType* phone(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _name;

		// Aggregations
		std::vector<AgencyTypePtr> _agencys;
		std::vector<EmailTypePtr> _emails;
		std::vector<PhoneTypePtr> _phones;
};


}
}


#endif
