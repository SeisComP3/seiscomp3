/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_STRINGTYPE_H__
#define __SEISCOMP_STATIONXML_STRINGTYPE_H__


#include <stationxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


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
		StringType(const StringType& other);

		//! Custom constructor
		StringType(const std::string& text);

		//! Destructor
		~StringType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator std::string&();
		operator const std::string&() const;

		//! Copies the metadata of other to this
		StringType& operator=(const StringType& other);
		bool operator==(const StringType& other) const;


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
