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


#ifndef __SEISCOMP_CORE_EXCEPTIONS_H__
#define __SEISCOMP_CORE_EXCEPTIONS_H__

#include <seiscomp3/core.h>

#include <string>
#include <exception>
#include <typeinfo>

namespace Seiscomp {
namespace Core {


class SC_SYSTEM_CORE_API GeneralException : public std::exception {
	public:
		GeneralException();
		GeneralException( const std::string& str);

		virtual ~GeneralException() throw();
		
		virtual const char* what( void ) const throw();

	private:
		std::string _descr;
};


class SC_SYSTEM_CORE_API MemoryException : public GeneralException {
	public:
		MemoryException();
		MemoryException(std::string what);
};


class SC_SYSTEM_CORE_API StreamException : public GeneralException {
	public:
		StreamException();
		StreamException(std::string what);
};


class SC_SYSTEM_CORE_API EndOfStreamException : public StreamException {
	public:
		EndOfStreamException();
		EndOfStreamException(std::string what);
};


class SC_SYSTEM_CORE_API TypeConversionException : public GeneralException {
	public:
		TypeConversionException();
		TypeConversionException(const std::string& str);
};


class SC_SYSTEM_CORE_API OverflowException : public GeneralException {
	public:
		OverflowException();
		OverflowException(const std::string& str);
};


class SC_SYSTEM_CORE_API UnderflowException : public GeneralException {
	public:
		UnderflowException();
		UnderflowException(const std::string& str);
};


class SC_SYSTEM_CORE_API ValueException : public GeneralException {
	public:
		ValueException();
		ValueException(const std::string& str);
};


class SC_SYSTEM_CORE_API TypeException : public GeneralException {
	public:
		TypeException();
		TypeException(const std::string& str);
};


class SC_SYSTEM_CORE_API ClassNotFound : public GeneralException {
	public:
		ClassNotFound();
		ClassNotFound(const std::string& str);
};


class SC_SYSTEM_CORE_API DuplicateClassname : public GeneralException {
	public:
		DuplicateClassname();
		DuplicateClassname(const std::string& str);
};


}
}

#endif
