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


#ifndef __SEISCOMP_IO_RECORDSTREAM_ARCHIVE_H__
#define __SEISCOMP_IO_RECORDSTREAM_ARCHIVE_H__

#include <seiscomp3/io/recordstream.h>


namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API ArchiveException: public Seiscomp::IO::RecordStreamException {
	public:
		ArchiveException(): RecordStreamException("Archive exception") {}
		ArchiveException(const std::string& what): RecordStreamException(what) {}
};


}
}

#endif
