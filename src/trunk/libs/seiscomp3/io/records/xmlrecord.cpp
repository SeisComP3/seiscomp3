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


#include <seiscomp3/io/records/xmlrecord.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <stdio.h>


namespace Seiscomp {

namespace IO {


REGISTER_RECORD(XMLRecord, "xml");


XMLRecord::XMLRecord() {}

void XMLRecord::read(std::istream &in) throw(Core::StreamException) {
	XMLArchive ar(in.rdbuf(), true);
	ar & NAMED_OBJECT("GenericRecord", *this);
	// Setting the eof bit causes the input to abort the reading
	if ( in.rdbuf()->sgetc() == EOF )
		in.setstate(std::ios_base::eofbit);
}

void XMLRecord::write(std::ostream &out) throw(Core::StreamException) {
	XMLArchive ar(out.rdbuf(), false);
	ar & NAMED_OBJECT("GenericRecord", *this);
}

}
}
