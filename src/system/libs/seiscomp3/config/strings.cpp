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


#include "strings.h"
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <cerrno>


namespace Seiscomp {
namespace Config {
namespace Private {


std::string toString(const std::string &value) {
	return value;
}



std::string toString(bool v) {
	return std::string(v?"true":"false");
}


template <>
bool fromString(char &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 0);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (char)retval;
	return true;
}


template <>
bool fromString(unsigned char &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 0);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (unsigned char)retval;
	return true;
}


template <>
bool fromString(int &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 0);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (int)retval;
	return true;
}


template <>
bool fromString(unsigned int &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 0);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (unsigned int)retval;
	return true;
}


template <>
bool fromString(long &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 0);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (long)retval;
	return true;
}


template <>
bool fromString(unsigned long &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 0);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (unsigned long)retval;
	return true;
}


template <>
bool fromString(float &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	double retval = strtod(str.c_str(), &endptr);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (float)retval;
	return true;
}


template <>
bool fromString(double &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;
	value = strtod(str.c_str(), &endptr);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( value == 0 && str.c_str() == endptr )
			return false;
	}

	return true;
}


template <>
bool fromString(bool &value, const std::string &str) {
	char* endptr = NULL;
	errno = 0;

	if ( compareNoCase(str, "true") == 0 ) {
		value = true;
		return true;
	}

	if ( compareNoCase(str, "false") == 0 ) {
		value = false;
		return true;
	}

	long int retval = strtol(str.c_str(), &endptr, 0);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (bool)retval;
	return true;
}


bool fromString(std::string &value, const std::string &str) {
	value.assign(str);
	return true;
}


int compareNoCase(const std::string &a, const std::string &b) {
	std::string::const_iterator it_a = a.begin(), it_b = b.begin();
	while ( it_a != a.end() && it_b != b.end() ) {
		char upper_a = toupper(*it_a);
		char upper_b = toupper(*it_b);
		if ( upper_a < upper_b )
			return -1;
		else if ( upper_a > upper_b )
			return 1;

		++it_a; ++it_b;
	}

	return it_a == a.end()?(it_b == b.end()?0:-1):(it_b == b.end()?1:0);
}


std::string &trim(std::string &str) {
	std::string::size_type pos;
	pos = str.find_first_not_of(WHITESPACE);
	if (pos != 0) str.erase(0, pos);
	pos = str.find_last_not_of(WHITESPACE);
	if ( pos != std::string::npos) str.erase(pos + 1, std::string::npos);
	return str;
}


bool isWhitespace(const char c) {
	if ( WHITESPACE.find_first_of(c) == std::string::npos )
		return false;
	return true;
}


bool isWhitespace(const std::string &str) {
	std::string::const_iterator cIt = str.begin();
	for ( ; cIt != str.end(); ++cIt ) {
		if ( !isWhitespace(*cIt) )
			return false;
	}
	return true;
}


} // namespace Private
} // namespace Config
} // namespace Seiscomp
