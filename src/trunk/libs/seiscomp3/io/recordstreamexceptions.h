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


#ifndef __SEISCOMP_IO_RECORDSTREAMEXCEPTIONS_H__
#define __SEISCOMP_IO_RECORDSTREAMEXCEPTIONS_H__

#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API RecordStreamException: public Seiscomp::Core::StreamException {
	public:
		RecordStreamException(): Seiscomp::Core::StreamException("RecordStream exception") {}
		RecordStreamException(const std::string& what): StreamException(what) {}
};


class SC_SYSTEM_CORE_API RecordStreamTimeout: public RecordStreamException {
	public:
		RecordStreamTimeout(): RecordStreamException("timeout") {}
		RecordStreamTimeout(const std::string& what): RecordStreamException(what) {}
};


}
}

#endif
