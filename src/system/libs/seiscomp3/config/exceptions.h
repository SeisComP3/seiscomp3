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


#ifndef __SEISCOMP_CONFIG_EXCEPTIONS_H__
#define __SEISCOMP_CONFIG_EXCEPTIONS_H__


#include <exception>
#include <string>
#include <seiscomp3/config/api.h>


namespace Seiscomp {
namespace Config {


class SC_CONFIG_API Exception : public std::exception {
	public:
		Exception() : _what("Configuration exception") {}
		Exception(const std::string &str) : _what(str) {}
		Exception(const char *str) : _what(str) {}
		virtual ~Exception() throw() {}

		const char *what() const throw() { return _what.c_str(); }

	private:
		std::string _what;
};


class SC_CONFIG_API OptionNotFoundException : public Exception {
	public:
		OptionNotFoundException() : Exception("Option not found") { }
		OptionNotFoundException(const std::string& str) : Exception("Option not found for: " + str) { }
};


class SC_CONFIG_API TypeConversionException : public Exception {
	public:
		TypeConversionException() : Exception("Type conversion error") { }
		TypeConversionException(const std::string& str) : Exception("Type conversion error: " + str) { }
};


class SC_CONFIG_API SyntaxException : public Exception {
	public:
		SyntaxException() : Exception("Syntax error") { }
		SyntaxException(const std::string& str) : Exception("Syntax error: " + str) { }
};


class SC_CONFIG_API CaseSensitivityException : public Exception {
	public:
		CaseSensitivityException() : Exception("Case-insensitiv names are ambigous") { }
		CaseSensitivityException(const std::string &str) : Exception("Case-insensitiv names are ambigous: " + str) { }
};


} // namespace Config
} // namespace Seiscomp


#endif
