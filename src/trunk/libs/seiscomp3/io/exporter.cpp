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



#include <seiscomp3/io/exporter.h>
#include <seiscomp3/core/interfacefactory.ipp>

#include <fstream>
#include <iostream>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::IO::Exporter, SC_SYSTEM_CORE_API);


namespace Seiscomp {
namespace IO {

namespace {

template <int N>
struct SinkBuf : std::streambuf {
	SinkBuf(ExportSink *s) : sink(s) {
		setp(out, out + N);
	}

	~SinkBuf() { sync(); }

	virtual int overflow(int c) {
		if ( traits_type::eq_int_type(traits_type::eof(), c))
			return traits_type::eof();

		if ( sync() )
			return traits_type::eof();

		traits_type::assign(*pptr(), traits_type::to_char_type(c));
		pbump(1);

		return traits_type::not_eof(c);
	}

	virtual int sync() {
		if ( pbase() == pptr() ) return 0;

		int bytes = pptr() - pbase();
		int res = sink->write(pbase(), bytes);
		// Reset put pointer
		setp(out, out + N);
		return res == bytes ? 0 : 1;
	}

	ExportSink *sink;
	char        out[N];
};


}


IMPLEMENT_SC_ABSTRACT_CLASS(Exporter, "Exporter");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Exporter::Exporter() {
	_prettyPrint = false;
	_indentation = 2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Exporter::~Exporter() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Exporter *Exporter::Create(const char *type) {
	return ExporterFactory::Create(type);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Exporter::setFormattedOutput(bool enable) {
	_prettyPrint = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Exporter::setIndent(int i) {
	_indentation = i;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(std::streambuf* buf, Core::BaseObject *obj) {
	return put(buf, obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(std::string filename, Core::BaseObject *obj) {
	if ( filename != "-" ) {
		std::ofstream ofs(filename.c_str(), std::ios_base::out);
		if ( !ofs.good() ) return false;
		return put(ofs.rdbuf(), obj);
	}

	return put(std::cout.rdbuf(), obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(ExportSink *sink, Core::BaseObject *obj) {
	SinkBuf<512> buf(sink);
	return write(&buf, obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
