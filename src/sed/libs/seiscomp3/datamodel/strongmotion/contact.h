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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_CONTACT_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_CONTACT_H__


#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(Contact);


class SC_STRONGMOTION_API Contact : public Core::BaseObject {
	DECLARE_SC_CLASS(Contact);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

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
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Contact& other) const;
		bool operator!=(const Contact& other) const;

		//! Wrapper that calls operator==
		bool equal(const Contact& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void setForename(const std::string& forename);
		const std::string& forename() const;

		void setAgency(const std::string& agency);
		const std::string& agency() const;

		void setDepartment(const std::string& department);
		const std::string& department() const;

		void setAddress(const std::string& address);
		const std::string& address() const;

		void setPhone(const std::string& phone);
		const std::string& phone() const;

		void setEmail(const std::string& email);
		const std::string& email() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _name;
		std::string _forename;
		std::string _agency;
		std::string _department;
		std::string _address;
		std::string _phone;
		std::string _email;
};


}
}
}


#endif
