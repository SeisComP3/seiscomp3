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

%module(package="seiscomp3", directors="1") Client
%{
#include "seiscomp3/core/typedarray.h"
#include "seiscomp3/core/genericrecord.h"
#include "seiscomp3/core/exceptions.h"
#include "seiscomp3/core/datamessage.h"
#include "seiscomp3/core/greensfunction.h"
#include "seiscomp3/io/recordstream.h"
#include "seiscomp3/io/recordinput.h"
#include "seiscomp3/io/recordfilter.h"
#include "seiscomp3/io/recordfilter/demux.h"
#include "seiscomp3/io/recordfilter/iirfilter.h"
#include "seiscomp3/io/recordfilter/resample.h"
#include "seiscomp3/io/importer.h"
#include "seiscomp3/io/exporter.h"
#include "seiscomp3/io/gfarchive.h"
#include "seiscomp3/io/archive/binarchive.h"
#include "seiscomp3/io/archive/xmlarchive.h"
#include "seiscomp3/io/records/mseedrecord.h"
#include "seiscomp3/communication/servicemessage.h"
#include "seiscomp3/communication/systemconnection.h"
#include "seiscomp3/communication/networkinterface.h"
#include "seiscomp3/communication/systemmessages.h"
#include "seiscomp3/client/application.h"
#include "seiscomp3/client/streamapplication.h"
#include "seiscomp3/client/commandline.h"
#include "seiscomp3/client/pluginregistry.h"
#include "seiscomp3/client/inventory.h"
#include "seiscomp3/client/configdb.h"
#include "seiscomp3/datamodel/publicobjectcache.h"
#include "seiscomp3/datamodel/messages.h"
#include "seiscomp3/io/recordstream/file.h"
#include "seiscomp3/io/recordstream/slconnection.h"
#include "seiscomp3/io/recordstream/arclink.h"
#include "seiscomp3/io/recordstream/combined.h"


#include "seiscomp3/math/coord.h"
#include "seiscomp3/math/geo.h"
#include "seiscomp3/math/filter.h"
#include "seiscomp3/math/filter/average.h"
#include "seiscomp3/math/filter/stalta.h"
#include "seiscomp3/math/filter/chainfilter.h"
#include "seiscomp3/math/filter/biquad.h"
#include "seiscomp3/math/filter/butterworth.h"
#include "seiscomp3/math/filter/taper.h"
#include "seiscomp3/math/filter/rmhp.h"
#include "seiscomp3/math/restitution/transferfunction.h"

%}

%feature("director") Seiscomp::Client::Application;
%feature("director") Seiscomp::Client::StreamApplication;

%feature("director:except") {
  if ($error != NULL) {
    throw Swig::DirectorMethodException();
  }
}
%exception {
  try { $action }
  catch (Swig::DirectorException &e) { SWIG_fail; }
}

%feature("pythonprepend") Seiscomp::Client::Application::Application(int argc, char** argv) %{
    argv = [ bytes(a.encode()) for a in argv ]
%}

%feature("pythonprepend") Seiscomp::Client::StreamApplication::StreamApplication(int argc, char** argv) %{
    argv = [ bytes(a.encode()) for a in argv ]
%}

%feature("pythonprepend") Seiscomp::Client::CommandLine::parse(int argc, char** argv) %{
    argv = [ bytes(a.encode()) for a in argv ]
%}

%include "stl.i"
%include "std_vector.i"
%include "std_string.i"
%include "std_set.i"
%import "Logging.i"
%import "../../../system/libs/swig/Config.i"
%import "Communication.i"
%import "DataModel.i"
%import "Math.i"
%import "Utils.i"

// This tells SWIG to treat char ** as a special case
%typemap(in) char ** {
	/* Check if is a list */
	if (PyList_Check($input)) {
		int size = PyList_Size($input);
		int i;
		$1 = (char **) malloc((size+1)*sizeof(char *));
		for (i = 0; i < size; i++) {
			PyObject *o = PyList_GetItem($input,i);
			if (PyString_Check(o))
				$1[i] = PyString_AsString(PyList_GetItem($input,i));
			else {
				PyErr_SetString(PyExc_TypeError,"list must contain strings");
				free($1);
				return NULL;
			}
		}
		$1[i] = 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError,"not a list");
		return NULL;
	}
}

// This cleans up the char ** array we did malloc before the function call
%typemap(freearg) char ** {
	free($1);
}

%rename(execute) Seiscomp::Client::Application::exec;

%ignore Seiscomp::Client::PluginRegistry::begin;
%ignore Seiscomp::Client::PluginRegistry::end;
%ignore Seiscomp::Client::Application::handleMonitorLog;
%ignore Seiscomp::Client::Application::handleInterrupt;
%feature("nodirector") Seiscomp::Client::Application::exit;
%ignore Seiscomp::Client::StreamApplication::storeRecord;
%ignore Seiscomp::Client::StreamApplication::acquisitionFinished;

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
%include "seiscomp3/client/commandline.h"
%include "seiscomp3/client/monitor.h"
%include "seiscomp3/client/application.h"
%include "seiscomp3/client/streamapplication.h"
%include "seiscomp3/client/inventory.h"
%include "seiscomp3/client/configdb.h"
%include "seiscomp3/client/pluginregistry.h"

%template(optionInt) Seiscomp::Client::CommandLine::option<int>;
%template(optionBool) Seiscomp::Client::CommandLine::option<bool>;
%template(optionDouble) Seiscomp::Client::CommandLine::option<double>;
%template(optionString) Seiscomp::Client::CommandLine::option<std::string>;

%extend Seiscomp::Client::CommandLine {
	void addIntOption(const char* group, const char* option,
	                  const char* description) {
		self->addOption(group, option, description, (int*)NULL);
	}

	void addIntOption(const char* group, const char* option,
	                  const char* description, int defaultValue) {
		self->addOption(group, option, description, (int*)NULL, defaultValue);
	}

	void addDoubleOption(const char* group, const char* option,
	                     const char* description) {
		self->addOption(group, option, description, (double*)NULL);
	}

	void addDoubleOption(const char* group, const char* option,
	                     const char* description, double defaultValue) {
		self->addOption(group, option, description, (double*)NULL, defaultValue);
	}

	void addBoolOption(const char* group, const char* option,
	                   const char* description) {
		self->addOption(group, option, description, (bool*)NULL);
	}

	void addBoolOption(const char* group, const char* option,
	                   const char* description, bool defaultValue) {
		self->addOption(group, option, description, (bool*)NULL, defaultValue);
	}

	void addStringOption(const char* group, const char* option, const char* description) {
		self->addOption(group, option, description, (std::string*)NULL);
	}

	void addStringOption(const char* group, const char* option,
	                     const char* description, const std::string& defaultValue) {
		self->addOption(group, option, description, (std::string*)NULL, defaultValue);
	}
};
