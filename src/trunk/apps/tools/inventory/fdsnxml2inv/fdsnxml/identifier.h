/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_IDENTIFIER_H__
#define __SEISCOMP_FDSNXML_IDENTIFIER_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Identifier);


/**
 * \brief A type to document persistent identifiers. Identifier values should
 * \brief be specified without a URI scheme (prefix), instead the identifer
 * \brief type is documented as an attribute.
 */
class Identifier : public Core::BaseObject {
	DECLARE_CASTS(Identifier);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Identifier();

		//! Copy constructor
		Identifier(const Identifier &other);

		//! Custom constructor
		Identifier(const std::string& value);
		Identifier(const std::string& type,
		           const std::string& value);

		//! Destructor
		~Identifier();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Identifier& operator=(const Identifier &other);
		bool operator==(const Identifier &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: type
		void setType(const std::string& type);
		const std::string& type() const;

		//! XML tag: value
		void setValue(const std::string& value);
		const std::string& value() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _type;
		std::string _value;
};


}
}


#endif
