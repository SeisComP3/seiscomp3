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


#include <seiscomp3/core/exceptions.h>

namespace Seiscomp {
namespace Core {


GeneralException::GeneralException() {}
GeneralException::GeneralException( const std::string& str) : std::exception() {
	_descr = str;
}

GeneralException::~GeneralException() throw() {}

const char* GeneralException::what( void ) const throw() {
	return _descr.c_str();
}


MemoryException::MemoryException() : GeneralException("memory allocation error") {}
MemoryException::MemoryException(std::string what) : GeneralException(what) {}


StreamException::StreamException() : GeneralException("stream error") {}
StreamException::StreamException(std::string what) : GeneralException(what) {}


EndOfStreamException::EndOfStreamException() : StreamException("end of stream") {}
EndOfStreamException::EndOfStreamException(std::string what) : StreamException(what) {}


TypeConversionException::TypeConversionException() : GeneralException("type conversion error") {}
TypeConversionException::TypeConversionException(const std::string& str ) : GeneralException(str) {}


OverflowException::OverflowException() : GeneralException("overflow") {}
OverflowException::OverflowException(const std::string& str ) : GeneralException(str) {}


UnderflowException::UnderflowException() : GeneralException("underflow") {}
UnderflowException::UnderflowException(const std::string& str ) : GeneralException(str) {}


ValueException::ValueException() : GeneralException("value error") {}
ValueException::ValueException(const std::string& str ) : GeneralException(str) {}


TypeException::TypeException() : GeneralException("type error") {}
TypeException::TypeException(const std::string& str ) : GeneralException(str) {}


ClassNotFound::ClassNotFound() : GeneralException("the requested classname has not been registered") {}
ClassNotFound::ClassNotFound(const std::string& str ) : GeneralException(str) {}


DuplicateClassname::DuplicateClassname() : GeneralException("duplicate classname") {}
DuplicateClassname::DuplicateClassname(const std::string& str ) : GeneralException(str) {}


}
}
