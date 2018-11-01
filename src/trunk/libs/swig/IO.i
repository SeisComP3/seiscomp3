/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

%module(package="seiscomp3", directors="1") IO
%{
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/greensfunction.h>
#include <seiscomp3/core/datamessage.h>
#include "seiscomp3/math/geo.h"
#include "seiscomp3/math/coord.h"
#include "seiscomp3/math/math.h"
#include "seiscomp3/math/filter.h"
#include "seiscomp3/math/filter/rmhp.h"
#include "seiscomp3/math/filter/taper.h"
#include "seiscomp3/math/filter/average.h"
#include "seiscomp3/math/filter/stalta.h"
#include "seiscomp3/math/filter/chainfilter.h"
#include "seiscomp3/math/filter/biquad.h"
#include "seiscomp3/math/filter/butterworth.h"
#include "seiscomp3/math/filter/taper.h"
#include "seiscomp3/math/filter/seismometers.h"
#include "seiscomp3/math/restitution/transferfunction.h"
#include <seiscomp3/io/database.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordfilter.h>
#include <seiscomp3/io/recordfilter/demux.h>
#include <seiscomp3/io/recordfilter/iirfilter.h>
#include <seiscomp3/io/recordfilter/resample.h>
#include <seiscomp3/io/recordstreamexceptions.h>
#include <seiscomp3/io/importer.h>
#include <seiscomp3/io/exporter.h>
#include <seiscomp3/io/gfarchive.h>
#include <seiscomp3/io/archive/binarchive.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/records/mseedrecord.h>
#include <seiscomp3/io/recordstream/file.h>
#include <seiscomp3/io/recordstream/slconnection.h>
#include <seiscomp3/io/recordstream/arclink.h>
#include <seiscomp3/io/recordstream/combined.h>
%}

%feature("director") Seiscomp::IO::ExportSink;

%newobject Seiscomp::IO::DatabaseInterface::Create;
%newobject Seiscomp::IO::DatabaseInterface::Open;
%newobject Seiscomp::IO::RecordStream::Create;
%newobject Seiscomp::IO::RecordStream::Open;
%newobject Seiscomp::IO::RecordStream::next;
%newobject Seiscomp::IO::RecordInput::next;
%newobject Seiscomp::IO::RecordIterator::current;
%newobject Seiscomp::IO::Importer::read;
%newobject Seiscomp::IO::GFArchive::get;
%newobject Seiscomp::IO::Exporter::Create;

%include std_ios.i
%include std_char_traits.i

%rename Seiscomp::RecordStream::File FileRecordStream;

%newobject Seiscomp::IO::RecordFilterInterface::feed;
%newobject Seiscomp::IO::RecordFilterInterface::flush;

%exception {
  try {
    $action
  }
  catch ( const Seiscomp::Core::ValueException &e) {
    SWIG_exception(SWIG_ValueError, e.what());
  }
  catch ( const std::exception &e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
  catch ( ... ) {
    SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
  }
}

%import "Math.i"
%include "seiscomp3/core.h"
%include "seiscomp3/io/database.h"
%include "seiscomp3/io/gfarchive.h"
%include "seiscomp3/io/recordstream.h"
%include "seiscomp3/io/recordinput.h"
%include "seiscomp3/io/recordfilter.h"
%include "seiscomp3/io/recordfilter/demux.h"
%include "seiscomp3/io/recordfilter/iirfilter.h"
%include "seiscomp3/io/recordfilter/resample.h"
%include "seiscomp3/io/recordstreamexceptions.h"
%include "seiscomp3/io/importer.h"
%include "seiscomp3/io/exporter.h"
%include "seiscomp3/io/archive/xmlarchive.h"
%include "seiscomp3/io/archive/binarchive.h"
//#ifdef HAVE_MSEED
%include "seiscomp3/io/records/mseedrecord.h"
%include "seiscomp3/io/recordstream/file.h"
%include "seiscomp3/io/recordstream/slconnection.h"
%include "seiscomp3/io/recordstream/arclink.h"
%include "seiscomp3/io/recordstream/combined.h"
//#endif


%extend Seiscomp::IO::RecordInput {
	%pythoncode %{
		def __iter__(self):
		    while 1:
		        rec = self.next()
		        if not rec:
		            raise StopIteration

		        yield rec
	%}
};


%template(RecordIIRFilterF) Seiscomp::IO::RecordIIRFilter<float>;
%template(RecordIIRFilterD) Seiscomp::IO::RecordIIRFilter<double>;

%template(RecordResamplerF) Seiscomp::IO::RecordResampler<float>;
%template(RecordResamplerD) Seiscomp::IO::RecordResampler<double>;
%template(RecordResamplerI) Seiscomp::IO::RecordResampler<int>;
