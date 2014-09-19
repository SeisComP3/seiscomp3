/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_STRINGTYPE_H__
#define __SEISCOMP_FDSNXML_STRINGTYPE_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(StringType);


class StringType : public Core::BaseObject {
	DECLARE_CASTS(StringType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StringType();

		//! Copy constructor
		StringType(const StringType &other);

		//! Custom constructor
		StringType(const std::string& text);

		//! Destructor
		~StringType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		StringType& operator=(const StringType &other);
		bool operator==(const StringType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: text
		void setText(const std::string& text);
		const std::string& text() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _text;
};


}
}


#endif
