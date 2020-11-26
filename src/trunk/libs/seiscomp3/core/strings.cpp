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


#define SEISCOMP_COMPONENT Utils

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <sstream>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <cerrno>


namespace Seiscomp {
namespace Core {


namespace {

const char *timeFormatPrecise = "%FT%T.%fZ";
const char *timeFormat2 = "%FT%TZ";

}


std::string toString(const std::string& value) {
	return value;
}



std::string toString(bool v) {
	return std::string(v?"true":"false");
}


std::string toString(const Seiscomp::Core::Time& v) {
	return v.toString(timeFormatPrecise);
}


std::string toString(const Enumeration& value) {
	return value.toString();
}


template <>
SC_SYSTEM_CORE_API bool fromString(char& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 10);
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
SC_SYSTEM_CORE_API bool fromString(unsigned char& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 10);
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
SC_SYSTEM_CORE_API bool fromString(int& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 10);
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
SC_SYSTEM_CORE_API bool fromString(unsigned int& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 10);
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
SC_SYSTEM_CORE_API bool fromString(long& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 10);
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
SC_SYSTEM_CORE_API bool fromString(unsigned long int& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long int retval = strtol(str.c_str(), &endptr, 10);
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
SC_SYSTEM_CORE_API bool fromString(long long int& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long long int retval = strtoll(str.c_str(), &endptr, 10);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = retval;
	return true;
}


template <>
SC_SYSTEM_CORE_API bool fromString(unsigned long long int& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;
	long long int retval = strtoll(str.c_str(), &endptr, 10);
	if ( errno != 0 )
		return false;
	if ( endptr ) {
		if ( str.c_str() + str.size() != endptr )
			return false;
		else if ( retval == 0 && str.c_str() == endptr )
			return false;
	}

	value = (unsigned long long)retval;
	return true;
}


template <>
SC_SYSTEM_CORE_API bool fromString(float& value, const std::string& str) {
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
SC_SYSTEM_CORE_API bool fromString(double& value, const std::string& str) {
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
SC_SYSTEM_CORE_API bool fromString(bool& value, const std::string& str) {
	char* endptr = NULL;
	errno = 0;

	if ( str.empty() )
		return false;

	if ( compareNoCase(str, "true") == 0 ) {
		value = true;
		return true;
	}

	if ( compareNoCase(str, "false") == 0 ) {
		value = false;
		return true;
	}

	long int retval = strtol(str.c_str(), &endptr, 10);
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


bool fromString(Time& value, const std::string& str) {
	if ( !value.fromString(str.c_str(), timeFormatPrecise) ) {
		if ( !value.fromString(str.c_str(), timeFormat2) )
			return false;
	}

	return true;
}


bool fromString(Enumeration& value, const std::string& str) {
	return value.fromString(str);
}


bool fromString(std::string& value, const std::string& str) {
	value.assign(str);
	return true;
}


std::string stringify(const char* fmt, ...) {
	// A static buffer that hopefully covers 99% of all use cases
	char staticBuffer[64];

	// The dynamic buffer that will be used if the static buffer is
	// not large enough
	char* dynamicBuffer = NULL;

	// The buffer actually written to
	char *buffer = staticBuffer;
	int size = sizeof(staticBuffer);
	int nsize;
	va_list params;
	int maxIterations = 10;

	va_start(params, fmt);
	nsize = vsnprintf(buffer, size, fmt, params);

	while ( nsize >= size ) { //fail -> create dynamic buffer with more space
		if ( dynamicBuffer != NULL )
			delete [] dynamicBuffer;

		dynamicBuffer = new char[nsize + 1]; //+1 for /0
		size = nsize+1;
		buffer = dynamicBuffer;

		va_end(params);
		va_start(params, fmt);
		nsize = vsnprintf(buffer, size, fmt, params);

		--maxIterations;
		if ( !maxIterations ) {
			SEISCOMP_ERROR("Stringify failed after 10 iterations: buffer still not large enough: %d < %d: aborting",
			               size, nsize);
			*buffer = '\0';
			break;
		}
	}

	std::string ret(buffer);
	va_end(params);

	if ( dynamicBuffer != NULL )
		delete [] dynamicBuffer;

	return ret;
}


int split(std::vector<std::string>& tokens, const char* source,
          const char* delimiter, bool compressOn) {
	boost::split(tokens, source, boost::is_any_of(delimiter),
	             ((compressOn) ? boost::token_compress_on : boost::token_compress_off));
	return static_cast<int>(tokens.size());
}


size_t splitExt(std::vector<std::string> &tokens, const char *source,
                const char *delimiter, bool compressOn, bool unescape,
                bool trim, const char *whitespaces, const char *quotes) {
	tokens.clear();
	size_t lenTok;
	size_t lenSource = strlen(source);
	char delimFound = 0;
	const char *tok = NULL;

	if ( unescape ) {
		std::string tmp(source, lenSource);
		char *sourceCopy = const_cast<char *>(tmp.c_str());
		while ( lenSource > 0 ) {
			tok = tokenizeUnescape(lenTok, lenSource, sourceCopy, delimFound,
			                       delimiter, trim, whitespaces, quotes);
			if ( tok != NULL ) {
				tokens.push_back(std::string(tok, lenTok));
			}
			else if ( tokens.empty() || !compressOn ) {
				tokens.push_back("");
			}
		}
	}
	else {
		while ( lenSource > 0 ) {
			tok = tokenizeExt(lenTok, lenSource, source, delimFound, delimiter,
			                  trim, whitespaces, quotes);
			if ( tok != NULL ) {
				tokens.push_back(std::string(tok, lenTok));
			}
			else if ( tokens.empty() || !compressOn ) {
				tokens.push_back("");
			}
		}
	}

	if ( delimFound )
		tokens.push_back("");

	return tokens.size();
}


const char *tokenizeExt(size_t &lenTok, size_t &lenSource, const char *&source,
                        char &delimFound, const char *delimiter, bool trim,
                        const char *whitespaces, const char *quotes) {
	lenTok = 0;
	delimFound = 0;

	const char *tok = NULL;
	size_t trailing_spaces = 0;
	char quote = 0;

	for ( ; lenSource && *source != 0; --lenSource, ++source ) {
		// check for protected character
		if ( *source == '\\' ) {
			if ( lenTok == 0 ) {
				tok = source;
			}
			else {
				++lenTok;
				trailing_spaces = 0;
			}

			// skip following character
			--lenSource; ++source;
			if ( lenSource && *source != 0 ) {
				++lenTok;
			}

			continue;
		}

		// check for unprotected delimiter outside of quotes
		if ( quote == 0 and strchr(delimiter, *source) != NULL ) {
			delimFound = *source;
			--lenSource; ++source;
			lenTok -= trailing_spaces;
			return tok;
		}

		// check for terminating quote character
		if ( *source == quote ) {
			quote = 0;
		}
		// check for beginning quote
		else if ( quote == 0 and strchr(quotes, *source) != NULL ) {
			quote = *source;
			trailing_spaces = 0;
		}
		// trimming outside unprotected characters outside of quotes
		else if ( trim ) {
			if ( !quote and strchr(whitespaces, *source) != NULL ) {
				// trim leading whitespaces
				if ( lenTok == 0 )
					continue;
				// count trailing spaces
				else
					++trailing_spaces;
			}
			else {
				trailing_spaces = 0;
			}
		}

		// mark beginning of string
		if ( lenTok == 0 )
			tok = source;

		++lenTok;
	}

	lenTok -= trailing_spaces;
	return tok;
}


const char *tokenizeUnescape(size_t &lenTok, size_t &lenSource, char *&source,
                             char &delimFound, const char *delimiter,
                             bool trim, const char *whitespaces,
                             const char *quotes) {
	lenTok = 0;
	delimFound = 0;

	const char *tok = NULL;
	char *tokEnd = NULL;
	size_t trailing_spaces = 0;
	char quote = 0;

	for ( ; lenSource && *source != 0; --lenSource, ++source ) {
		// check for backslash character
		if ( *source == '\\' ) {
			// initialize token if not done so far
			if ( lenTok == 0 )
				tok = tokEnd = source;
			else
				*tokEnd = *source;
			++lenTok;

			// read next char
			--lenSource; ++source;

			// backslash was last char: do not unescape
			if ( !lenSource || *source == 0 ) {
				return tok;
			}

			// backslash outside quotes: unescape backslash, quotes,
			// delimiter and whitespaces
			if ( quote == 0 ) {
				if ( *source != '\\' &&
				     strchr(quotes, *source) == NULL &&
				     strchr(delimiter, *source) == NULL &&
				     strchr(whitespaces, *source) == NULL ) {
					++tokEnd; ++lenTok;
				}
			}
			// backslash inside quotes: unescape backslash and start quote
			// character only
			else {
				if ( *source != '\\' && *source != quote ) {
					++tokEnd; ++lenTok;
				}
			}

			*tokEnd = *source;
			++tokEnd;
			trailing_spaces = 0;
			continue;
		}

		// check for unprotected delimiter outside of quotes
		if ( quote == 0 and strchr(delimiter, *source) != NULL ) {
			delimFound = *source;
			--lenSource; ++source;
			lenTok -= trailing_spaces;
			return tok;
		}
		// check for terminating quote character
		else if ( *source == quote ) {
			quote = 0;
			trailing_spaces = 0;
			continue;
		}
		// check for beginning quote
		else if ( quote == 0 and strchr(quotes, *source) != NULL ) {
			quote = *source;
			trailing_spaces = 0;
			continue;
		}
		// trim outside unprotected characters outside of quotes
		else if ( trim ) {
			if ( !quote and strchr(whitespaces, *source) != NULL ) {
				// trim leading whitespaces
				if ( lenTok == 0 )
					continue;
				// count trailing spaces
				else
					++trailing_spaces;
			}
			else {
				trailing_spaces = 0;
			}
		}

		// mark beginning of string
		if ( lenTok == 0 ) {
			tok = source;
			tokEnd = source + 1;
		}
		// copy source char and advance end pointer of result string
		else {
			*tokEnd = *source;
			++tokEnd;
		}

		++lenTok;
	}

	lenTok -= trailing_spaces;
	return tok;
}


bool isEmpty(const char* str) {
	return str == NULL || *str == '\0';
}


int compareNoCase(const std::string& a, const std::string& b) {
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


std::string& trim(std::string& str) {
	/*
	const char whitespace[] = "\t\n\v\f\r ";

	std::string::size_type pos;
	pos = str.find_first_not_of(whitespace);
	if (pos != 0) str.erase(0, pos);

	pos = str.find_last_not_of(whitespace);
	if (pos != std::string::npos) str.erase(pos + 1, std::string::npos);
	*/
	boost::trim(str);
	return str;
}


bool isWhitespace(const char c) {
	if ( WHITESPACE.find_first_of(c) == std::string::npos )
		return false;
	return true;
}


bool isWhitespace(const std::string& str) {
	std::string::const_iterator cIt = str.begin();
	for ( ; cIt != str.end(); ++cIt ) {
		if ( !isWhitespace(*cIt) )
			return false;
	}
	return true;
}


bool wildcmp(const char *pat, const char *str) {
	const char *s, *p;
	bool star = false;

loopStart:
	for ( s = str, p = pat; *s; ++s, ++p ) {
		switch ( *p ) {
			case '?':
				break;
			case '*':
				star = true;
				str = s, pat = p;
				do { ++pat; } while (*pat == '*');
				if ( !*pat ) return true;
				goto loopStart;
			default:
				if ( *s != *p )
					goto starCheck;
				break;
		} /* endswitch */
	} /* endfor */

	while (*p == '*') ++p;

	return (!*p);

starCheck:
	if ( !star ) return false;
	++str;
	goto loopStart;
}


bool wildcmp(const std::string &wild, const std::string &str) {
	return wildcmp(wild.c_str(), str.c_str());
}


bool wildicmp(const char *pat, const char *str) {
	const char *s, *p;
	bool star = false;

loopStart:
	for ( s = str, p = pat; *s; ++s, ++p ) {
		switch ( *p ) {
			case '?':
				break;
			case '*':
				star = true;
				str = s, pat = p;
				do { ++pat; } while (*pat == '*');
				if ( !*pat ) return true;
				goto loopStart;
			default:
				if ( toupper(*s) != toupper(*p) )
					goto starCheck;
				break;
		} /* endswitch */
	} /* endfor */

	while (*p == '*') ++p;

	return (!*p);

starCheck:
	if ( !star ) return false;
	++str;
	goto loopStart;
}


bool wildicmp(const std::string &wild, const std::string &str) {
	return wildicmp(wild.c_str(), str.c_str());
}



} // namespace Core
} // namespace Seiscomp
