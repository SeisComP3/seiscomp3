fdsnws is a server that provides
`FDSN Web Services <http://www.fdsn.org/webservices>`_ from a SeisComP3 database
and :ref:`global_recordstream` source. Also it may be configured to serve data
availability information similar to the `IRIS DMC IRISWS availability Web
Service <https://service.iris.edu/irisws/availability/1/>`_

.. caution::
   If you expose the FDSN Web Service as a public service, make sure that
   the database connection is read-only. fdsnws will never attempt to write
   into the database.

Service Overview
----------------

The following services are available:

.. csv-table::
   :header: "Service", "Provides", "Provided format"

   ":ref:`fdsnws-dataselect <sec-dataSelect>`", "time series data", "`miniSEED <http://www.iris.edu/data/miniseed.htm>`_"
   ":ref:`fdsnws-station <sec-station>`", "network, station, channel, response metadata", "`FDSN Station XML <http://www.fdsn.org/xml/station/>`_, `StationXML <http://www.data.scec.org/station/xml.html>`_, `SC3ML <http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/>`_"
   ":ref:`fdsnws-event <sec-event>`", "earthquake origin and magnitude estimates", "`QuakeML <https://quake.ethz.ch/quakeml>`_, `SC3ML <http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/>`_"
   ":ref:`ext-availability <sec-avail>`", "waveform data availability information", "text, geocsv, json, sync, request (`fdsnws-dataselect <https://service.iris.edu/fdsnws/dataselect/1>`_)"


The available services can be reached from the fdsnws start page.  The services
also provide an interactive URL builder constructing the request URL based on
user inputs. The FDSN specifications can be found on
`FDSN Web Services <http://www.fdsn.org/webservices>`_.

URL
^^^

* http://localhost:8080/fdsnws

If ``fdsnws`` is started, it accepts connections by default on port 8080 which
can be changed in the configuration. Also please read :ref:`sec-port` for
running the services on a privileged port, e.g. port 80 as requested by the
FDSNWS specification.

.. note::

   If you decide to run the service on a different URL than ``localhost:8080``
   you have to change the URL string in the ``*.wadl`` documents located under
   ``$DATADIR/fdsnws``.

.. _sec-dataSelect:

DataSelect
----------

* provides time series data in miniSEED format
* request type: HTTP-GET, HTTP-POST

URL
^^^

* http://localhost:8080/fdsnws/dataselect/1/builder
* http://localhost:8080/fdsnws/dataselect/1/query
* http://localhost:8080/fdsnws/dataselect/1/queryauth
* http://localhost:8080/fdsnws/dataselect/1/version
* http://localhost:8080/fdsnws/dataselect/1/application.wadl

Example
^^^^^^^

* Request URL for querying waveform data from the GE station BKNI, all BH channels
  on 11 April 2013 between 00:00:00 and 12:00:00:

  ``http://localhost:8080/fdsnws/dataselect/1/query?net=GE&sta=BKNI&cha=BH?&start=2013-04-11T00:00:00&end=2013-04-11T12:00:00``

To submit HTTP-POST requests the command line tool ``curl`` may be used:

.. code-block:: sh

   sysop@host:~$ curl -X POST --data-binary @request.txt "http://localhost:8080/fdsnws/dataselect/1/query"

where *request.txt* contains the POST message body. For details read the
FDSN specifications.

Feature Notes
^^^^^^^^^^^^^

* ``quality`` parameter not implemented (information not available in SeisComP)
* ``minimumlength`` parameter is not implemented
* ``longestonly`` parameter is not implemented
* access to restricted networks and stations is only granted through the
  ``queryauth`` method

The data channels exposed by this service may be restrict by defining an
inventory filter, see section :ref:`sec-inv-filter`.

Service Configuration
^^^^^^^^^^^^^^^^^^^^^

* activate :confval:`serveDataSelect` in the module configuration
* configure the :ref:`global_recordstream` in the module's global configuration.
  If the data is stored in a local waveform archive the
  :ref:`rs-sdsarchive` provides fast access to the data. For archives on remote hosts
  use :ref:`rs-arclink` or :ref:`rs-fdsnws` instead.

.. warning::

   Requesting future or delayed data may block the :ref:`sec-dataSelect` service.
   Therefore, real-time :ref:`global_recordstream` requests such as :ref:`rs-slink`
   should be avoided.
   If :ref:`rs-slink` is inevitable make use of the ``timeout`` and
   ``retries`` parameters. E.g. set the :confval:`recordstream.source` to
   ``localhost:18000?timeout=1&retries=0`` or in case of the :ref:`rs-combined`
   service to
   ``slink/localhost:18000?timeout=1&retries=0;sdsarchive//home/sysop/seiscomp3/var/lib/archive``.

.. _sec-station:

Station
-------

* provides network, station, channel, response metadata
* request type: HTTP-GET, HTTP-POST
* stations may be filtered e.g. by geographic region and time, also the
  information depth level is selectable

URL
^^^

* http://localhost:8080/fdsnws/station/1/builder
* http://localhost:8080/fdsnws/station/1/query
* http://localhost:8080/fdsnws/station/1/version
* http://localhost:8080/fdsnws/station/1/application.wadl

Example
^^^^^^^

* Request URL for querying the information for the GE network on response level:

  http://localhost:8080/fdsnws/station/1/query?net=GE&cha=BH%3F&level=response&nodata=404

Feature Notes
^^^^^^^^^^^^^

* to enable FDSNXML or StationXML support the plugins ``fdsnxml`` resp.
  ``staxml`` have to be loaded
* ``updatedafter`` request parameter not implemented: The last modification time
  in SeisComP is tracked on the object level. If a child of an object is updated
  the update time is not propagated to all parents. In order to check if a
  station was updated all children must be evaluated recursively. This operation
  would be much too expensive.
* ``formatted``: boolean, default: ``false``
* additional values of request parameters:

  * format:

    * standard: ``[xml, text]``
    * additional: ``[fdsnxml (=xml), stationxml, sc3ml]``
    * default: ``xml``

The inventory exposed by this service may be restricted, see section
:ref:`sec-inv-filter`.

.. _sec-event:

Event
-----

* provides earthquake origin and magnitude estimates
* request type: HTTP-GET
* events may be filtered e.g. by hypocenter, time and magnitude

URL
^^^

* http://localhost:8080/fdsnws/event/1/builder
* http://localhost:8080/fdsnws/event/1/query
* http://localhost:8080/fdsnws/event/1/catalogs
* http://localhost:8080/fdsnws/event/1/contributors
* http://localhost:8080/fdsnws/event/1/version
* http://localhost:8080/fdsnws/event/1/application.wadl

Example
^^^^^^^

* Request URL for fetching the event parameters within 10 degrees around 50°N/11°E
  starting on 18 April 2013:

  http://localhost:8080/fdsnws/event/1/query?start=2018-06-01&lat=50&lon=11&maxradius=10&nodata=404

Feature Notes
^^^^^^^^^^^^^

* SeisComP does not distinguish between catalogs and contributors, but
  supports agencyIDs. Hence, if specified, the value of the ``contributor``
  parameter is mapped to the agencyID. The file
  ``@DATADIR@/share/fdsn/contributors.xml`` has to be filled manually with all
  available agency ids
* origin and magnitude filter parameters are always applied to preferred origin
  resp. preferred magnitude
* ``updatedafter`` request parameter not implemented: The last modification time
  in SeisComP is tracked on the object level. If a child of an object is updated
  the update time is not propagated to all parents. In order to check if a
  station was updated all children must be evaluated recursively. This operation
  would be much too expensive.
* additional request parameters:

  * ``includepicks``: boolean, default: ``false``, works only in combination
    with ``includearrivals`` set to ``true``
  * ``includecomments``: boolean, default: ``true``
  * ``formatted``: boolean, default: ``false``

* additional values of request parameters:

  * format:

    * standard: ``[xml, text]``
    * additional: ``[qml (=xml), qml-rt, sc3ml, csv]``
    * default: ``xml``

.. _sec-avail:

Data Availability
-----------------

The data availability web service returns detailed time span information of
what time series data is available at the DMC archive. The availability information
can be created by :ref:`scardac` in the SeisComP3 database from where it is
fetched by fdsnws.

The availability service is no official standard yet. This implementation aims
to be compatible with the `IRIS DMC IRISWS availability Web Service
<https://service.iris.edu/irisws/availability/1/>`_ implementation.

* request type: HTTP-GET, HTTP-POST
* results may be filtered e.g. by channel code, time and quality

URL
^^^

* http://localhost:8080/ext/availability/1/extent - Produces list of available
  time extents (earliest to latest) for selected channels (network, station,
  location and quality) and time ranges.
* http://localhost:8080/ext/availability/1/builder-extent - URL builder helping
  you to form your data extent requests
* http://localhost:8080/ext/availability/1/query - Produces list of contiguous
  time spans for selected channels (network, station, location, channel and
  quality) and time ranges.
* http://localhost:8080/ext/availability/1/builder - URL builder helping you to
  form your data time span requests
* http://localhost:8080/ext/availability/1/version

Examples
^^^^^^^^

* Request URL for data extents of seismic network ``IU``:

  http://localhost:8080/fdsnws/ext/availability/1/extent?net=IU

* Further limit the extents to those providing data for August 1st 2018:

  http://localhost:8080/fdsnws/ext/availability/1/extent?net=IU&start=2018-08-01

* Request URL for continues time spans of station ``ANMO`` in July 2018:

  http://localhost:8080/fdsnws/ext/availability/1/query?sta=ANMO&start=2018-07-01&end=2018-08-01

.. note::

   Use :ref:`scardac` for creating the availability information.

Feature Notes
^^^^^^^^^^^^^

* The IRISWS availability implementation truncates the time spans of the returned
  data extents and segments to the requested start and end times (if any). This
  implementation truncates the start and end time only for the formats: ``sync``
  and ``request``. The ``text``, ``geocsv`` and ``json`` format will return the
  exact time windows extracted from the waveform archive.

  The reasons for this derivation are:

  * Performance: With the ``/extent`` query the ``text``, ``geocsv`` and
    ``json`` offer the display of the number of included time spans
    (``show=timespancount``). The data model offers no efficient way to
    recalculate the number of time spans represented by an extent if the extents
    time window is altered by the requested start and end times. The ``sync``
    and ``request`` formats do not provided this counter and it is convenient to
    use their outputs for subsequent data requests.
  * By truncating the time windows information is lost. There would be no
    efficient way for a client to retrieve the exact time windows falling into a
    specific time span.
  * Network and station epochs returned by the :ref:`sec-station` service are also
    not truncated to the requested start and end times.
  * Truncation can easily be done on client side. No additional network traffic is
    generated.


.. _sec-inv-filter:

Filtering the inventory
-----------------------

The channels served by the :ref:`sec-station` and :ref:`sec-dataSelect` service
may be filtered by specified an INI file in the ``stationFilter`` and
``dataSelectFilter`` configuration parameter. You may use the same file for both
services or define a separate configuration set. **Note:** If distinct file
names are specified and both services are activated, the inventory is loaded
twice which will increase the memory consumption of this module.

.. code-block:: ini

   [Chile]
   code = CX.*.*.*

   [!Exclude station APE]
   code = GE.APE.*.*

   [German (not restricted)]
   code = GE.*.*.*
   restricted = false
   shared = true
   archive = GFZ

The listing above shows a configuration example which includes all Chile
stations. Also all not restricted German stations, with exception of the station
GE.APE, are included.

The configuration is divided into several rules. The rule name is given in
square brackets. A name starting with an exclamation mark defines an exclude
rule, else the rule is an include. The rule name is not evaluated by the
application but is plotted when debugging the rule set, see configuration
parameter ``debugFilter``.

Each rule consists of a set of attributes. The first and mandatory attribute is
``code`` which defines a regular expression for the channel code (network,
station, location, channel). In addition the following optional attributes
exist:

.. csv-table::
   :header: "Attribute", "Type", "Network", "Station", "Location", "Channel"

   "**restricted**", "Boolean", "X", "X", "", "X"
   "**shared**", "Boolean", "X", "X", "", "X"
   "**netClass**", "String", "X", "", "", ""
   "**archive**", "String", "X", "X", "", ""

A rule matches if all of its attributes match. The optional attributes are
evaluated bottom-up where ever they are applicable. E.g. if a rule defines
``restricted = false`` but the restricted flag is not present on channel level
then it is searched on station and then on network level. If no ``restricted``
attribute is found in the hierarchy, the rule will not match even if the value
was set to ``false``.

The individual rules are evaluated in order of their definition. The processing
stops once a matching rule is found and the channel is included or excluded
immediately. So the order of the rules is important.

One may decided to specify a pure whitelist, a pure blacklist, or to mix include
and exclude rules. If neither a matching include nor exclude rule is found, then
channel is only added if no other include rule exists in the entire rule set.


.. _sec-port:

Changing the service port
-------------------------

The FDSN Web service specification defines that the Service SHOULD be available
under port 80. Typically SeisComP3 runs under a user without root permissions
and therefore is not allowed to bind to privileged ports (<1024).
To serve on port 80 you may for instance

* run SeisComP3 with root privileged (not recommended)
* use a proxy Webserver, e.g. Apache with
  `mod-proxy <http://httpd.apache.org/docs/2.2/mod/mod_proxy.html>`_ module
* configure and use :ref:`sec-authbind`
* setup :ref:`sec-firewall` redirect rules


.. _sec-authbind:

Authbind
^^^^^^^^

``authbind`` allows a program which does not or should not run as root to bind
to low-numbered ports in a controlled way. Please refer to ``man authbind`` for
program descriptions. The following lines show how to install and setup authbind
for the user ``sysop`` under the Ubuntu OS.

.. code-block:: sh

   sysop@host:~$ sudo apt-get install authbind
   sysop@host:~$ sudo touch /etc/authbind/byport/80
   sysop@host:~$ sudo chown sysop /etc/authbind/byport/80
   sysop@host:~$ sudo chmod 500 /etc/authbind/byport/80

Once ``authbind`` is configured correctly the FDSN Web services may be started
as follows:

.. code-block:: sh

   sysop@host:~$ authbind --deep seiscomp exec fdsnws

In order use ``authbind`` when starting ``fdsnws`` as SeisComP service the last
line in the ``~/seiscomp3/etc/init/fdsnws.py`` have to be commented in.


.. _sec-firewall:

Firewall
^^^^^^^^

All major Linux distributions ship with their own firewall implementations which
are front-ends for the ``iptables`` kernel functions. The following line
temporary adds a firewall rule which redirects all incoming traffic on port 8080
to port 80.

.. code-block:: sh

   sysop@host:~$ sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to 8080

Please refer to the documentation of your particular firewall solution on how to
set up this rule permanently.

Authentication extension
------------------------

The FDSNWS standard requires HTTP digest authentication as the
authentication mechanism. The "htpasswd" configuration option is used to
define the location of the file storing usernames and passwords of users
who are allowed to get restricted data. Any user with valid credentials
would have access to all restricted data.

An extension to the FDSNWS protocol has been developed in order to use
email-pattern-based access control lists, which is an established
authorization mechanism in SC3 (used by Arclink). It works as follows:

* The user contacts an authentication service (based on eduGAIN AAI,
  e-mail, etc.) and receives a list of attributes (a token), signed by the
  authentication service. The validity of the token is typically 30 days.

* The user presents the token to /auth method (HTTPS) of the dataselect
  service. This method is the only extension to standard FDSNWS that is
  required.

* If the digital signature is valid, a temporary account for /queryauth
  is created. The /auth method returns username and password of this
  account, separated by ':'. The account is typically valid for 24 hours.

* The username and password are to be used with /queryauth as usual.

* Authorization is based on user's e-mail address in the token and
  arclink-access bindings.

Configuration
^^^^^^^^^^^^^

The authentication extension is enabled by setting the "auth.enable"
configuration option to "true" and pointing "auth.gnupgHome" to a directory
where GPG stores its files. Let's use the directory
~/seiscomp3/var/lib/gpg, which is the default.

* First create the direcory and your own signing key:

.. code-block:: sh

  sysop@host:~$ mkdir -m 700 ~/seiscomp3/var/lib/gpg
  sysop@host:~$ gpg --homedir ~/seiscomp3/var/lib/gpg --gen-key

* Now import GPG keys of all authentication services you trust:

.. code-block:: sh

  sysop@host:~$ gpg --homedir ~/seiscomp3/var/lib/gpg --import <keys.asc

* Finally sign all imported keys with your own key (XXXXXXXX is the ID of
  an imported key):

.. code-block:: sh

  sysop@host:~$ gpg --homedir ~/seiscomp3/var/lib/gpg --edit-key XXXXXXXX sign save

* ...and set auth.enable, either using the "scconfig" tool or:

.. code-block:: sh

  sysop@host:~$ echo "auth.enable = true" >>~/seiscomp3/etc/fdsnws.cfg

Usage example
^^^^^^^^^^^^^

A client like fdsnws_fetch is recommended, but also tools like wget and
curl can be used. As an example, let's request data from the restricted
station AAI (assuming that we are authorized to get data of this station).

* The first step is to obtain the token from an authentication service.
  Assuming that the token is saved in "token.asc", credentials of the
  temporary account can be requsted using one of the following commands:

.. code-block:: sh

  sysop@host:~$ wget --post-file token.asc https://geofon.gfz-potsdam.de/fdsnws/dataselect/1/auth -O cred.txt
  sysop@host:~$ curl --data-binary @token.asc https://geofon.gfz-potsdam.de/fdsnws/dataselect/1/auth -o cred.txt

* The resulting file "cred.txt" contains username and password separated by
  a colon, so one can conveniently use a shell expansion:

.. code-block:: sh

  sysop@host:~$ wget "http://`cat cred.txt`@geofon.gfz-potsdam.de/fdsnws/dataselect/1/queryauth?starttime=2015-12-15T16:00:00Z&endtime=2015-12-15T16:10:00Z&network=IA&station=AAI" -O data.mseed
  sysop@host:~$ curl --digest "http://`cat cred.txt`@geofon.gfz-potsdam.de/fdsnws/dataselect/1/queryauth?starttime=2015-12-15T16:00:00Z&endtime=2015-12-15T16:10:00Z&network=IA&station=AAI" -o data.mseed

* Using the fdsnws_fetch utility, the two steps above can be combined into
  one:

.. code-block:: sh

  sysop@host:~$ fdsnws_fetch -a token.asc -s 2015-12-15T16:00:00Z -e 2015-12-15T16:10:00Z -N IA -S AAI -o data.mseed

Logging
-------
In addition to normal SC3 logs, fdsnws can create a simple HTTP access log
and/or a detailed request log. The locations of log files are specified by
"accessLog" and "requestLog" in fdsnws.cfg.

Both logs are text-based and line-oriented. Each line of *access* log
contains the following fields, separated by '|' (some fields can be empty):

* service name;
* hostname of service;
* access time;
* hostname of user;
* IP address of user (proxy);
* length of data in bytes;
* processing time in milliseconds;
* error message;
* agent string;
* HTTP response code;
* username (if authenticated);
* network code of GET request;
* station code of GET request;
* location code of GET request;
* channel code of GET request;

Each line of *request* log contains a JSON object, which has the following
attributes::

service
  service name

userID
  anonymized (numeric) user ID for statistic purposes

clientID
  agent string

userEmail
  e-mail address of authenticated user if using restricted data.

userLocation
  JSON object containing rough user location (eg., country) for statistic
  purposes

created
  time of request creation

status
  "OK", "NODATA", "ERROR" or "DENIED"

bytes
  length of data in bytes

finished
  time of request completion

trace
  request content after wildcard expansion (array of JSON objects)

Each trace object has the following attributes::

net
  network code

sta
  station code

loc
  location code

cha
  channel code

start
  start time

end
  end time

restricted
  True if the data requires authorization

status
  "OK", "NODATA", "ERROR" or "DENIED"

bytes
  length of trace in bytes

Both logs are rotated daily. In case of access log, one week of data is
kept. Request logs are compressed using bzip2 and not removed.

If trackdb.enable=true in fdsnws.cfg, then requests are additionally logged
into SC3 database using the ArcLink request log schema. Be aware that the
number of requests in a production system can be rather large. For example,
the GEOFON datacentre is currently serving between 0.5..1 million FDSNWS
requests per day.
