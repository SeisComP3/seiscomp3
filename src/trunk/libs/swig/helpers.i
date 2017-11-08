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

%module helpers

%{
#include <seiscomp3/core/array.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/core/greensfunction.h>
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
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/io/recordfilter.h>
#include <seiscomp3/io/recordfilter/demux.h>
#include <seiscomp3/io/recordfilter/iirfilter.h>
#include <seiscomp3/io/recordfilter/resample.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/importer.h>
#include <seiscomp3/io/exporter.h>
#include <seiscomp3/io/gfarchive.h>
#include <seiscomp3/io/archive/binarchive.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/records/ahrecord.h>
#include <seiscomp3/io/records/mseedrecord.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/communication/connection.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/messages.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/io/recordstream/file.h>
#include <seiscomp3/io/recordstream/slconnection.h>
#include <seiscomp3/io/recordstream/arclink.h>
#include <seiscomp3/io/recordstream/combined.h>
#include "helpers.h"
%}

%exception {
  try {
    $action
  }
  catch ( const Swig::DirectorException &e) {
    SWIG_exception(SWIG_ValueError, e.what());
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

%include "exception.i"
%import "Communication.i"
%import "DataModel.i"
%include "helpers.h"

%pythoncode %{

from seiscomp3 import Communication, DataModel, Logging, IO

class ConnectionError(Exception):
    pass

class DatabaseError(Exception):
    pass

class SerializationError(Exception):
    pass

class DatabaseQuery(_DatabaseQuery):
    def getEvents(self, limit, offset=0, newestFirst=True, minTime=None, maxTime=None,
        minLatitude=None, maxLatitude=None, minLongitude=None, maxLongitude=None,
        minMagnitude=None, minArrivals=None):

        return _DatabaseQuery.getEvents(self, limit, offset, newestFirst,
            minTime, maxTime, minLatitude, maxLatitude, minLongitude, maxLongitude,
            minMagnitude, minArrivals)

class SimpleConnection(object):
    def __init__(self, config, name, group):
        mediatorAddress = config.getString("connection.server")
        dbDriverName = config.getString("database.type")
        dbAddress = config.getString("database.parameters")

        connection = Communication.Connection.Create(mediatorAddress, name, group)
        if connection is None:
            Logging.error("Cannot connect to Mediator")
            raise ConnectionError, "connection could not be established"
        else:
            Logging.info("Connection has been established")

        dbDriver = IO.DatabaseInterface.Create(dbDriverName)
        if dbDriver is None:
            Logging.error("Cannot find database driver " + dbDriverName)
            raise DatabaseError, "driver not found"
        
        if not dbDriver.connect(dbAddress):
            Logging.error("Cannot connect to database at " + dbAddress)
            raise DatabaseError, "connection could not be established"
        
        self.__connection = connection

        # This reference to dbDriver is essential, since dbQuery becomes
        # invalid when dbDriver is deleted
        self.__dbDriver = dbDriver
        self.dbQuery = DatabaseQuery(dbDriver)

    def subscribe(self, group):
        r = self.__connection.subscribe(group)
        if r != Core.Status.SEISCOMP_SUCCESS:
            raise ConnectionError, "could not subscribe to " + group
        
        return True

    def readMessage(self, waitForNew=True):
        r = self.__connection.readMessage(waitForNew)
        if not r:
            raise ConnectionError, "could not read message"
        
        return r

    def send(self, *args):
        r = self.__connection.send(*args)
        if not r:
            raise ConnectionError, "could not send message"

        return r

    def disconnect(self):
        return self.__connection.disconnect()

def readobj(obj, file):
    if not _readobj(obj, file):
        raise SerializationError, "could not read object from " + file

def writeobj(obj, file):
    if not _writeobj(obj, file):
        raise SerializationError, "could not write object to " + file

%}

