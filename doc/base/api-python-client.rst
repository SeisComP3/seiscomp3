.. _api-client-python:

seiscomp3.Client
================

.. py:module:: seiscomp3.Client

Modules are meant to be standalone programs doing a particular job. The
seiscomp3.Client package focuses on three main aspects:

* Communicate with other modules
* Access station- and event metadata through a database
* Fetch waveform data

Therefore a client package has been developed combining these concepts in an
easy way with only a couple of API calls. Since SeisComP3 has been developed in
C++ and uses the object oriented paradigm forcefully, modules build on the
Application (C++: :class:`Seiscomp::Client::Application`, :class:`Python: seiscomp3.Client.Application`)
class. It manages the messaging connection and waveform sources in a transparent
way.

The class :class:`Seiscomp::Client::Application` is the base class for all SC3
applications. It manages messaging and database connections, provides access to
command line options and configuration parameters and also handles and
interprets notifier messages.

Blocking network operations like reading messages are moved into threads that
are synchronized in a single blocking message queue. This queue allows pushing
elements from different threads and unblocks when a new element is ready to be
popped. If the queue is full (currently 10 elements are allowed) the pushing
threads also block until an element can be pushed again.

This way applications do not have to poll and thus do not burn CPU cycles.

The application class is event driven. It runs the event loop which pops the
message queue and dispatches events with their handlers. Handler methods are
prefixed with *handle*, e.g. :func:`handleMessage`.

.. note::

   When overriding handlers it is always good practise to call the base
   handlers before running custom code.


Application class
-----------------

The application class is part of the seiscomp3.Client package. It needs to
be imported first.

.. code-block:: python

   import seiscomp3.Client

A common strategy to write a module with that class is to derive from it and
run it in a Python main method.

.. code-block:: python

   import seiscomp3.Client, sys

   # Class definition
   class MyApp(seiscomp3.Client.Application):
       def __init__(self, argc, argv):
           seiscomp3.Client.Application.__init__(self, argc, argv)

   # Main method to call the app
   def main(argc, argv):
       app = MyApp(argc, argv)
       return app()

   # Call the main method if run as script
   if __name__ == "__main__":
       sys.exit(main(len(sys.argv), sys.argv))


An application can be called with the parenthesis operator () which returns
the applications result code and serves as input to :func:`sys.exit`. Operator()
is a wrapper for :func:`Application.exec`.


The workflow of :func:`Application.exec` looks as follows:

.. code-block:: python

   def exec(self):
       self.returnCode = 1

       if self.init():
           self.returnCode = 0

           if !self.run() and self.returnCode == 0:
               self.returnCode = 1

           self.done()
       }
       else
          self.done()

       return self.returnCode

:func:`init`, :func:`run` and :func:`done` are explained in more detail in
the next sections.


Constructor
^^^^^^^^^^^

To create an application, derive from the seiscomp3.Client.Application class
and configure it in the constructor.

.. code-block:: python
   :linenos:
   :emphasize-lines: 7,9,11

   class MyApp(seiscomp3.Client.Application):
       # MyApp constructor
       def __init__(self, argc, argv):
           # IMPORTANT: call the base class constructor
           seiscomp3.Client.Application.__init__(self, argc, argv)
           # Default is TRUE
           self.setMessagingEnabled(False)
           # Default is TRUE, TRUE
           self.setDatabaseEnabled(False, False)
           # Default is TRUE
           self.setDaemonEnabled(False)

As marked in line 4, the call of the constructor of the base class is very
important. It takes the command line parameters and sets up internal
application variables. Without this call the application will either not run
at all or show undefined/unexpected behaviour.

The constructor takes also the initial parameters of the application such as
enabling a messaging connection and enabling database access.

Messaging, database and daemon mode is enabled by default. The daemon mode is
important if the application should be started as service and therefore should
support the :option:`-D, --daemon` option. Utilities and non daemon applications
should disable that mode.

Example calls to this options are shown in the highlighted lines of the above
code block.

If messaging is enabled, the messaging username is derived from the binary
called (*not the class name*). If the script is called test.py then the username
selected is **test**. The username can be overridden either in the configuration
file (:ref:`global`) or using the API.

.. code-block:: python

   self.setMessagingUsername("test")

Setting the username to an empty string results in a random username selected
by the messaging server.

All application methods are defined in the C++ header file
:file:`src/trunk/libs/seiscomp3/client/application.h`.


Init
^^^^

The workflow of the init function looks like this:

.. code-block:: python

   init (virtual)
       initConfiguration (virtual)
       initCommandLine (virtual)
           createCommandLineDescription (virtual)
       parseCommandLine (virtual)
           printUsage (virtual)
       validateParameters (virtual)
       loadPlugins
       forkDaemon
       initMessaging
       initDatabase
       loadInventory or loadStations
       loadDBConfigModule
       loadCities

Methods marked with virtual can be overriden. :func:`init` itself calls
a lot of handlers that can be customized. Typical handlers are
:func:`initConfiguration`, :func:`createCommandLineDescription`
and :func:`validateParameters`.

:func:`initConfiguration` is used to read parameters of the configuration files
and to populate the internal state. If something fails or if configured values
are out of bounds, False can be returned which causes to :func:`init` to return
False and to exit the application with a non-zero result code.

An example is show below:

.. code-block:: python

   def initConfiguration(self):
       if not seiscomp3.Client.Application.initConfiguration(self):
           return False

       try: self._directory = self.configGetString("directory")
       except: pass

       return True

This method reads the directory parameter from the configuration file(s) and
sets it internally. If the directory is not given in any of the modules
configuration files, it logs an error and aborts the application by returning
False.

:func:`createCommandLineDescription` is used to add custom command line options.
This is a void function and does not return any value. It is also not necessary
to call the baseclass method although it does not hurt.

.. code-block:: python

   def createCommandLineDescription(self):
       self.commandline().addGroup("Storage")
       self.commandline().addStringOption("Storage", "directory,o", "Specify the storage directory")

A new command line option group is added with :func:`addGroup` and then a new
option is added to this group which is a string option. 4 types can be added
as options: string, int, double and bool: :func:`addStringOption`, :func:`addIntOption`,
:func:`addDoubleOption` and :func:`addBoolOption`.

:func:`validateParameters` can be used to fetch the values of previously added
command line options and to validate each parameter. If False is returned, the
application is aborted with a non-zero result code.

.. code-block:: python

   def validateParameters(self):
       try: self._directory = self.commandline().optionString("directory")
       except: pass

       # The directory validity is checked to avoid duplicate checks in
       # initConfiguration.
       if not self._directory:
           seiscomp3.Logging.error("directory not set")
           return False

       if not exists(self._directory):
           seiscomp3.Logging.error("directory %s does not exist" %\
                                   self._directory)
           return False

       return True


Custom initialization code after checking all parameters can be placed in the
overridden method :func:`init`.

.. code-block: python

   def init(self):
       if seiscomp3.Client.Application.init(self) == False:
           return False

       # Custom initialization code runs here.
       setupCustomConnections()
       readMyDataFiles()

       return True

But be aware that the process forked already if started as daemon. To run before
the fork, it needs to be put into :func:`validateParameters`.


Run
^^^

The workflow of the run method looks like this:

.. code-block:: python

   run (virtual)
       startMessageThread
       messageLoop
           readMessage
           dispatchMessage (virtual)
               handleMessage (virtual)
                   addObject (virtual)
                   updateObject (virtual)
                   removeObject (virtual)
               handleReconnect (virtual)
               handleDisconnect (virtual)
               handleTimeout (virtual)
               handleAutoShutdown (virtual)

The run method starts the event loop and wait for new events in the queue.
In case of messaging a thread is started that sits and waits for messages
and feeds them to the queue and to the event loop in :func:`run`. Without
messaging the run loop would do nothing but waiting for SIGTERM or
a timer event enabled with :func:`enableTimer`. If the event loop is not needed
because no timer and messages are needed, it should be overridden and the
code should be placed there. This will disable the event loop.

:func:`run` is expected to return True on success and False otherwise. If False
is returned the application exists with a non-zero return code. Custom return
codes can always be set with :func:`Application.exit`.

If the scmaster sends a message to the client it is received in the applications
message thread and pushed to the queue. The event loop pops the message from
the queue and calls :func:`handleMessage`. The default implementation uses two
settings when handling a messages that can be controlled with
:func:`enableInterpretNotifier` and :func:`enableAutoApplyNotifier`.

:func:`enableInterpretNotifier` controls whether the Application queries the
message type and extracts notifier objects. For each notifier it parses the
operation and dispatches the parentID and the object either to
:func:`addObject`, :func:`updateObject` or :func:`removeObject` handler. This
behaviour is enabled by default. If disabled, a clients needs to parse the
messages by itself and implement this method.

:func:`enableAutoApplyNotifier` controls whether incoming notifer objects are
applied automatically to objects in local memory. If the client has already
an object in memory and an update notifier for this object is received, the object
in the notifier is copied to the local object. This behaviour is enabled by default.


Done
^^^^

The workflow of the done method looks like this:

.. code-block:: python

   done (virtual)
       closeTimer
       closeMessaging
       closeDatabase

:func:`done` is usually not overridden. If custom code and clean up procedures
need to be placed in :func:`done`, the base class **must** be called. :func:`done` is a
void function.

.. code-block:: python

   def done(self):
       seiscomp3.Client.Application.done()

       # Custom clean ups
       closeMyDataFiles();
       closeCustomConnections();



StreamApplication class
-----------------------

The application class has another occurence: :class:`seiscomp3.Client.StreamApplication`.

The class :class:`StreamApplication` extends the :class:`Application`
in terms of record acquisition. It spawns another thread that reads the records
from a configurable source and adds a new handler method
:func:`StreamApplication.handleRecord` to handle these records.


Its workflow looks like this:

.. code-block:: python

   init (virtual)
       +initRecordStream
   run (virtual)
       +startAcquisitionThread
           +storeRecord
       Application.messageLoop
           dispatchMessage (virtual)
               +handleRecord (virtual)
    done (virtual)
        +closeRecordStream


Received records can be handled with :func:`handleRecord`.

.. code-block:: python

   def handleRecord(self, rec):
       print rec.streamID()

The stream subscription should be done in :func:`init`. :func:`recordStream`
returns the RecordStream instance which can be used to add stream requests.

.. code-block:: python

   def init(self):
       if seiscomp3.Client.StreamApplication.init(self) == False:
           return False

       # Subscribe to some streams
       self.recordStream().addStream("GE", "MORC", "", "BHZ")
       return True


The record stream service is configured either with configuration files
(:confval:`recordstream.service`, :confval:`recordstream.source`) or
via command line (:option:`-I, --record-url`).

The application finishes if the record stream read EOF. Running a :class:`StreamApplication`
with :ref:`Seedlink<seedlink>` would probably never terminate since it is a
real time connection and handles reconnects automatically.
