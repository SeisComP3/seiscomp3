SeisComP applications access waveform data through the RecordStream interface.
The following tables lists available implementations:

.. csv-table::
   :header: "Name", "Service Prefix", "Description"

   ":ref:`rs-slink`", "``slink``", "Connects to :ref:`SeedLink server <seedlink>`"
   ":ref:`rs-arclink`", "``arclink``", "Connects to :ref:`ArcLink server <arclink>`"
   ":ref:`rs-fdsnws`", "``fdsnws``", "Connects to :ref:`FDSN Web service <fdsnws>`"
   ":ref:`rs-file`", "``file``", "Reads records from file"
   ":ref:`rs-sdsarchive`", "``sdsarchive``", "Reads records from SeisComP archive (SDS)"
   ":ref:`rs-odcarchive`", "``odcarchive``", "Reads records from Orpheus archive (ODC)"
   ":ref:`rs-memory`", "``memory``", "Reads records from memory"
   ":ref:`rs-combined`", "``combined``", "Combines archive and real-time stream"
   ":ref:`rs-balanced`", "``balanced``", "Distributes requests to multiple proxy streams"
   ":ref:`rs-dec`", "``dec``", "Decimates (resamples) a proxy stream"
   ":ref:`rs-resample`", "``resample``", "Resamples (up or down) a proxy stream to a given sampling rate"

The RecordStream used by an application is either specified on the
commandline (`-I URI`) or configured through using the parameter
:confval:`recordstream` and :confval:`recordstream`.

.. _rs-slink:

SeedLink
--------

This RecordStream fetches data from a SeedLink server. The source is read as an
URL and supports URL encoded parameters. The default host is set to
`localhost`, the default port to `18000`. Optional parameters are:

- `timeout` - connection timeout in seconds, default: 300
- `retries` - number of connection retry attempts, default: 0
- `no-batch` - disables BATCH mode to request data, does not take a value

Examples
^^^^^^^^

- ``slink://``
- ``slink://geofon.gfz-potsdam.de?timeout=60&retries=5``
- ``slink://localhost:18042``

.. _rs-arclink:

ArcLink
-------

This RecordStream fetches data from a ArcLink server. The source is read as an
URL and supports URL encoded parameters. The default host is set to
`localhost`, the default port to `18001`. Optional parameters are:

- `user` - user name required on some servers
- `pwd` - password required on some servers
- `dump` - optional output file for all records being received

Examples
^^^^^^^^

- ``arclink://``
- ``arclink://geofon.gfz-potsdam.de?user=foo&pwd=secret``
- ``arclink://localhost:18042``
- ``arclink://localhost?dump=test.mseed``

.. _rs-fdsnws:

FDSNWS
------

This RecordStream fetches data from a FDSN Web service. The source is read as an
URL.

Examples
^^^^^^^^

- ``fdsnws://service.iris.edu:80/fdsnws/dataselect/1/query``

.. _rs-file:

File
----

This RecordStream reads data from a file. The source is read as an file path. If
the source is set to `'-'` the data is read from `stdin`. By default the record
type is set to `mseed`. If a file name extension is available the record type is
set as follows:

========= ===========
Extension Record Type
========= ===========
`*.xml`   `xml`
`*.bin`   `binary`
`*.mseed` `mseed`
========= ===========

Examples
^^^^^^^^

- ``file://-``
- ``file:///tmp/input.mseed``

.. _rs-sdsarchive:

SDSArchive
----------

This RecordStream reads data from an SeisComP (SDS) archive using the
:ref:`rs-file` RecordStream. The source is interpreted as a directory path.

Example
^^^^^^^

- ``sdsarchive:///home/sysop/seiscomp3/var/lib/archive``

.. _rs-odcarchive:

ODCArchive
----------

This RecordStream reads data from an ODC archive using the :ref:`rs-file`
RecordStream. The source is interpreted as a directory path.

Example
^^^^^^^

- ``odcarchive:///path/to/record/archive``

.. _rs-memory:

Memory
------

This RecordStream reads data from memory and is only useful for developing
applications. For instance a record sequence stored in an internal buffer could
be passed to an instance of this RecordStream for reading.

.. _rs-combined:

Combined
--------

This RecordStream combines one archive and one real-time RecordStream, e.g.
:ref:`rs-arclink` and :ref:`rs-slink`. First the archive stream is read up to
the size of the real-time buffer. Then the acquisition is switched to the
real-time stream. The syntax for the source is similar to an URL:

``combined://real-time-stream;archive-stream??parameters``

By default the real-time stream is set to :ref:`rs-slink` and the
archive-stream is set to :ref:`rs-arclink`. Any other streams may be configured.
The parameters of the combined stream are separated by 2 question marks (`??`)
in order to distinguish them from the parameters used in the proxy streams:

- `slinkMax|rtMax|1stMax` - Buffer size in seconds of the first stream
  (typically the real-time stream), default: 3600

  Time spans can be configured with an additional and optional suffix:

  ======  =============
  Suffix  Multiplicator
  ======  =============
  s       1
  m       60
  h       3600
  d       86400
  w       86400*7
  ======  =============

- `splitTime` - The absolute time of the separation of both sources. The argument
  is an ISO time string, e.g. 2018-05-10T12:00:00Z or a year, e.g. 2018, which is
  the same as 2018-01-01T00:00:00.000Z.
  `splitTime` can be used if the waveform archives are spread over several directories
  or harddisks. See also the :ref:`examples<rs_splitTime>`.

The combined record stream may be nested allowing the configuration of a
(theoretically) infinite number of archive streams. The syntax for a nested
configuration uses parenthesis:

``combined://real-time-stream;combined/(archive-stream1;archive-stream2??parameters)??parameters``

.. _rs_splitTime:

Examples
^^^^^^^^

.. csv-table::
   :header: "URL", "Description"

   "``combined://localhost:18000;localhost:18001``", "Seedlink on localhost:18000 combined with Arclink on localhost 18001"
   "``combined://slink/localhost:18000;arclink/localhost:18001``", "Same as above"
   "``combined://;``", "Same as above"
   "``combined://:18042;?user=foo&pwd=secret??rtMax=1800``", "Seedlink on localhost:18042 combined with Arclink on localhost 18001, real-time (SeedLink) buffer size set to 30min"
   "``combined://slink/localhost:18000;sdsarchive//home/sysop/seiscomp3/var/lib/archive``", Seedlink combined with SDS archive
   "``combined://slink/localhost:18000;combined/(arclink/localhost:18001;arclink/localhost:18002??1stMax=30d)??1stMax=1h``", Seedlink combined with a combined record stream using two Arclink sources
   "``combined://slink/localhost:18000;combined/(sdsarchive//home/sysop/seiscomp3/var/lib/archive;combined/(sdsarchive//home/sysop/seiscomp3/var/lib/archive2017;sdsarchive//home/sysop/seiscomp3/var/lib/archive2016??splitTime=2017)??splitTime=2018)``", "Seedlink combined with a combined recordStream providing access to 3 different SDS archives separated by time. The first SDS archive contains the most recent archived data. The other two contain the data from 2016 and 2017."
   "``combined://slink/localhost:18000;combined/(sdsarchive//home/sysop/seiscomp3/var/lib/archive;combined/(sdsarchive//home/sysop/seiscomp3/var/lib/archive2017;sdsarchive//home/sysop/seiscomp3/var/lib/archive2016??splitTime=2017-06-01T00:00:00Z)??splitTime=2018-06-01T00:00:00Z)``", "Seedlink combined with a combined recordStream providing access to 3 different SDS archives separated by time. The first SDS archive contains the most recent archived data. The other two are separated in mid of 2016."

.. _rs-balanced:

Balanced
--------

This RecordStream distributes requests quasi-equally (but deterministically) to
multiple proxy streams. It can be used for load balancing and to improve failure
tolerance. The algorithm to choose a proxy stream (counting from 0) is based on
station code and can be expressed in Python as follows:

.. code-block:: python

   stationCode = "WLF"
   nproxies = 2

   x = 0
   for c in stationCode:
       x += ord(c)

   print("choosing proxy stream", x % nproxies)

Examples
^^^^^^^^

.. csv-table::
   :header: "URL", "Description"

   "``balanced://slink/server1:18000;slink/server2:18000``", "Distribute requests to 2 :ref:`rs-slink` RecordStreams"
   "``balanced://combined/(server1:18000;server1:18001);combined/(server2:18000;server2:18001)``", "Distribute requests to 2 :ref:`rs-combined` RecordStreams"

.. _rs-dec:

Decimation
----------

This RecordStream decimates (resamples) a proxy stream, e.g. :ref:`rs-slink`.
The syntax for the source is similar to an URL:

``dec://proxy-stream?parameters/address``

Optional parameters are:

- `rate` - target sampling rate in Hz, default: 1
- `fp` - default: 0.7
- `fs` - default: 0.9
- `cs` - coefficient scale, default: 10

Examples
^^^^^^^^

- ``dec://slink/localhost:18000``
- ``dec://file?rate=2/-``
- ``dec://combined/;``

.. _rs-resample:

Resample
--------

This RecordStream resamples (up or down) a proxy stream, e.g. :ref:`rs-slink`,
to a given sampling rate. The syntax for the source is similar to an URL:

``resample://proxy-stream?parameters/address``

Optional parameters are:

- `rate` - target sampling rate in Hz, default: 1
- `fp` - default: 0.7
- `fs` - default: 0.9
- `cs` - coefficient scale, default: 10
- `lw` - lanczos kernel width, default: 3
- `debug` - enables debug output, default: false


Examples
^^^^^^^^

- ``resample://slink/localhost:18000``
- ``resample://file?rate=2/-``
- ``resample://combined/;``
