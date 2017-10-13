/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/

#ifndef __SEISCOMP_FDSNXML_DATE_H__
#define __SEISCOMP_FDSNXML_DATE_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/baseobject.h>
#include <cstdlib>


namespace Seiscomp {
namespace FDSNXML {


class DateTime : public Core::Time {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DateTime();

		//! Copy constructor
		DateTime(const Core::Time& other);
		DateTime(const DateTime& other);

		void serialize(Core::BaseObject::Archive &ar);
};


inline bool fromString(DateTime& date, const std::string& str) {
	int year = strtol(str.c_str(), NULL, 10);
	if ( year < 1902 )
		//return Core::Time(1902,1,1);
		return false;
	else if ( year > 2037 )
		//return Core::Time(2037,12,31);
		return false;

	if ( date.fromString(str.c_str(), "%FT%T.%f") )
		return true;

	return date.fromString(str.c_str(), "%FT%T");
}


inline std::string toString(const DateTime& date) {
	if ( date.microseconds() == 0 )
		return date.toString("%FT%T");
	else
		return date.toString("%FT%T.%f");
}


}
}


#endif
