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


#ifndef __SC_IO_XMLRECORD_H__
#define __SC_IO_XMLRECORD_H__

#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(XMLRecord);


class SC_SYSTEM_CORE_API XMLRecord : public GenericRecord {
	public:
		//! Default Constructor
		XMLRecord();

	public:
		void read(std::istream &in) throw(Core::StreamException);
		void write(std::ostream &out) throw(Core::StreamException);
};

}
}

#endif
