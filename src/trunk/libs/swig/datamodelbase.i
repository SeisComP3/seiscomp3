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

%{
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/greensfunction.h>
#include <seiscomp3/core/genericrecord.h>
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
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/io/recordfilter.h>
#include <seiscomp3/io/recordfilter/demux.h>
#include <seiscomp3/io/recordfilter/iirfilter.h>
#include <seiscomp3/io/recordfilter/resample.h>
#include <seiscomp3/io/importer.h>
#include <seiscomp3/io/exporter.h>
#include <seiscomp3/io/gfarchive.h>
#include <seiscomp3/io/archive/binarchive.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/records/mseedrecord.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/diff.h>
#include <seiscomp3/io/recordstream/file.h>
#include <seiscomp3/io/recordstream/slconnection.h>
#include <seiscomp3/io/recordstream/arclink.h>
#include <seiscomp3/io/recordstream/combined.h>
%}

%feature("director") Seiscomp::DataModel::CachePopCallback;

%include stl.i
%include std_complex.i
%include std_vector.i
%import "IO.i"

%newobject *::Create;
%newobject *::find;
%newobject *::Find;

%newobject Seiscomp::DataModel::copy;
%newobject Seiscomp::DataModel::DatabaseIterator::get;
%newobject Seiscomp::DataModel::DatabaseArchive::getObject;
%newobject Seiscomp::DataModel::NotifierMessage::get;
%newobject Seiscomp::DataModel::DiffMerge::diff2Message;
%newobject Seiscomp::DataModel::Diff::diff2Message;
%newobject Seiscomp::DataModel::Notifier::GetMessage;
%newobject Seiscomp::DataModel::Notifier::Create;
%ignore Seiscomp::DataModel::Diff::diff;
%ignore Seiscomp::DataModel::Diff2::diff;
%ignore Seiscomp::DataModel::DatabaseIterator::next;

optional(Seiscomp::DataModel::Operation);
enum(Seiscomp::DataModel::Operation);

namespace std {
   %template(vectord) vector<double>;
   %template(vectorc) vector< std::complex<double> >;
};

%ignore Seiscomp::DataModel::PublicObjectCache::const_iterator;
%ignore Seiscomp::DataModel::PublicObjectCache::begin;
%ignore Seiscomp::DataModel::PublicObjectCache::end;

%template(NotifierMessageBase) Seiscomp::Core::GenericMessage<Seiscomp::DataModel::Notifier>;

%exception {
  try {
    $action
  }
  catch ( const Swig::DirectorException &e ) {
    SWIG_fail;
  }
  catch ( const Seiscomp::Core::ValueException &e ) {
    SWIG_exception(SWIG_ValueError, e.what());
  }
  catch ( const std::exception &e ) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
  catch ( ... ) {
    SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
  }
}

%include "seiscomp3/client.h"
%include "seiscomp3/datamodel/object.h"
%include "seiscomp3/datamodel/publicobject.h"
%include "seiscomp3/datamodel/databasearchive.h"
%include "seiscomp3/datamodel/publicobjectcache.h"
%include "seiscomp3/datamodel/notifier.h"
%include "seiscomp3/datamodel/utils.h"
%include "seiscomp3/datamodel/diff.h"

%extend Seiscomp::DataModel::DatabaseIterator {
	void step() {
		++(*self);
	}

	%pythoncode %{
		def __iter__(self):
		    return self

		def __next__(self):
		    o = self.get()
		    if not o:
		        raise StopIteration

		    self.step()
		    return o

		# for Python 2 compatibility
		def next(self):
		    return self.__next__()
	%}
};

%extend Seiscomp::DataModel::PublicObjectCache {
	%pythoncode %{
		def get(self, klass, publicID):
		    o = self.find(klass.TypeInfo(), publicID)
		    return klass.Cast(o)
	%}
};
