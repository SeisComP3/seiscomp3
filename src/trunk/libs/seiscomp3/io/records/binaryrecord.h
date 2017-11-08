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


#ifndef __SC_IO_BINARYRECORD_H__
#define __SC_IO_BINARYRECORD_H__

#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(BinaryRecord);


class SC_SYSTEM_CORE_API BinaryRecord : public GenericRecord {
	public:
		//! Default Constructor
		BinaryRecord();

	public:
		void read(std::istream &in);
		void write(std::ostream &out);
};

}
}

#endif
