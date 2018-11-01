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


#ifndef __SEISCOMP_CORE_STRING_H__
#define __SEISCOMP_CORE_STRING_H__

#include <string>
#include <ctime>
#include <vector>
#include <complex>
#include <seiscomp3/core/optional.h>
#include <seiscomp3/core/enumeration.h>
#include <seiscomp3/core/datetime.h>


namespace Seiscomp {
namespace Core {


/** Array of whitespace characters */
const std::string WHITESPACE = "\t\n\v\f\r ";


/**
 * Converts a value into a string. Conversions are supported
 * for following types:
 *   char
 *   int, long
 *   float, double
 *   Core::Time, std::complex
 *   and any other type that is supported by std::ostream
 *   std::vector and OPT() of all above types
 *
 * @param value The value
 * @return The value as string
 */
template <typename T>
std::string toString(const T& value);

template <typename T>
std::string toString(const std::complex<T>& value);

SC_SYSTEM_CORE_API std::string toString(const std::string& value);
SC_SYSTEM_CORE_API std::string toString(bool value);
SC_SYSTEM_CORE_API std::string toString(const Time& value);
SC_SYSTEM_CORE_API std::string toString(const Enumeration& value);

template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
std::string toString(const Enum<ENUMTYPE, END, NAMES> &value);

template <typename T>
std::string toString(const std::vector<T> &v);

template <typename T>
std::string toString(const ::boost::optional<T> &v);


/**
 * Converts a string into a value. Conversions are supported
 * for following types:
 *   char
 *   int, long
 *   float, double
 *   std::vector of all above types
 * IMPORTANT: integer types are converted in base 10!
 *
 * @param value The target value
 * @param str The source string
 */
template <typename T>
bool fromString(T &value, const std::string &str);

template <typename T>
bool fromString(std::complex<T> &value, const std::string &str);

SC_SYSTEM_CORE_API bool fromString(Time &value, const std::string &str);
SC_SYSTEM_CORE_API bool fromString(Enumeration &value, const std::string &str);
SC_SYSTEM_CORE_API bool fromString(std::string &value, const std::string &str);

template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
bool fromString(Enum<ENUMTYPE, END, NAMES> &value, const std::string &str);

template <typename T>
bool fromString(std::vector<T> &vec, const std::string &str);


/**
 * @brief Produces output according to a format as used by printf. The output
 *        is written to a string and returned.
 * @param fmt A format description as used by printf
 * @return The string containing the output
 */
SC_SYSTEM_CORE_API std::string stringify(const char *fmt, ...);


SC_SYSTEM_CORE_API
int split(std::vector<std::string> &tokens, const char *source,
          const char *delimiter, bool compressOn = true);

SC_SYSTEM_CORE_API bool isEmpty(const char*);

/**
 * A case-insensitive comparison.
 * @return Result as defined by strcmp
 */
SC_SYSTEM_CORE_API int compareNoCase(const std::string &a, const std::string &b);

/** Removes whitespace at the beginning and end of the string.
 * @param string to be trimmed (in/out parameter)
 * @return returns the trimmed string
 */
SC_SYSTEM_CORE_API std::string &trim(std::string &str);

/** Checks if given character is whitespace */
bool isWhitespace(const char c);

/** Checks if the given string solely contanins whitespace */
bool isWhitespace(const std::string &str);

/** wildcmp() compares a string containing wildcards with another
 * string where '?' represents a single character and '*'
 * represents zero to unlimited characters.
 * wildicmp() performs the same operation, but is case insensitive
 * This code has been written by Alessandro Cantatore
 * http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html
 * @param wild The string containing the wildcards
 * @param str The string checked against the wildcard string
 * @return The match result
 */
SC_SYSTEM_CORE_API bool wildcmp(const char *wild, const char *str);
SC_SYSTEM_CORE_API bool wildcmp(const std::string &wild, const std::string &str);
SC_SYSTEM_CORE_API bool wildicmp(const char *wild, const char *str);
SC_SYSTEM_CORE_API bool wildicmp(const std::string &wild, const std::string &str);


}
}


#include <seiscomp3/core/strings.ipp>

#endif
