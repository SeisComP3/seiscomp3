/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_PHONE_H__
#define __SEISCOMP_FDSNXML_PHONE_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Phone);


class Phone : public Core::BaseObject {
	DECLARE_CASTS(Phone);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Phone();

		//! Copy constructor
		Phone(const Phone &other);

		//! Destructor
		~Phone();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Phone& operator=(const Phone &other);
		bool operator==(const Phone &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: CountryCode
		void setCountryCode(const OPT(int)& countryCode);
		int countryCode() const throw(Seiscomp::Core::ValueException);

		//! XML tag: AreaCode
		void setAreaCode(int areaCode);
		int areaCode() const;

		//! XML tag: PhoneNumber
		void setPhoneNumber(const std::string& phoneNumber);
		const std::string& phoneNumber() const;

		//! XML tag: description
		void setDescription(const std::string& description);
		const std::string& description() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(int) _countryCode;
		int _areaCode;
		std::string _phoneNumber;
		std::string _description;
};


}
}


#endif
