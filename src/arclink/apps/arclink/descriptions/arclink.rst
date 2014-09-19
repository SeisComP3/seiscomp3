SeedLink was designed for real-time data transfer. A SeedLink client can only access data that is in a
relatively small real-time ringbuffer. Moreover, SeedLink has neither the functionality to query the
station database nor deal with the instrument responses and thus does not support full SEED. ArcLink
complements SeedLink by providing the above functionality. The ArcLink protocol is similar to SeedLink: it is
based on TCP and uses simple commands in ASCII coding. One conceptual difference is that the client does not
"subscribe" to real-time streams, but requests data based on time windows. Unlike SeedLink, the data may not
be sent immediately, but possibly minutes or even hours later, when the request is processed.
The reply to an successful ArcLink request is a request identifier that is then used by the client
to get the status, to download the data
and to delete the request.

The ArcLink server implementation in SeisComP3 does not access the data archive directly, but delegates this
job to a "request handler". Thus, it is possible to use ArcLink for accessing different data archives by using
different request handlers. This is similar to SeedLink support for different plug-ins for different channel
sources. As for SeedLink, the request handler processes are started by the ArcLink server as needed. In the
ArcLink configuration file the user can define a minimum and maximum number of request handlers to be started
and to be kept running. 
The ArcLink server dynamically creates and destroys request handlers as needed.
There is also a possibility to set the maximum number of request handlers per request
type that are allowed to run in parallel. Each request handler can handle only one request at one time.

Furthermore, the ArcLink protocol itself does not impose a limit on the type of data that being provided, but
the implementation provided in the SeisComP3 implements today five different request types:

  **Waveform**
    Used to request seismological waveform data in mini-SEED or full SEED formats

  **Response**
    Used to request station metadata information in dataless SEED format

  **Inventory**
    Used to request station metadata information in ArcLink XML format

  **Routing**
    Used to request routing information in ArcLink Routing XML format

  **Qc**
    Used to request quality control information in XML format

When communicating with an ArcLink server, a client should implement the ArcLink client protocol as described at the 
:ref:`ArcLink protocol <arclink_protocol>` documentation.

Routing and Access control
--------------------------

  The ArcLink server and request handler provided on the default SeisComP3 installation access the station
  information (inventory) from the database by the messaging system, just like any other SeisComP3
  program.
  For fulfilling routing requests more information than the station inventory is needed, as the
  ArcLink server needs
  to have a list binding network/station/location/channel codes to Internet server addresses running
  other instances of the ArcLink server. Also, to implement access control to the restricted networks the
  ArcLink server needs to have a list of e-mail address (or identifiers) allowed to access each network.

  Routing
    The Routing information is a list of addresses (given as *host:port*, e.g., "webdc.eu:18002") of
    different ArcLink servers that can provide data for each stream listed in the SeisComp3
    inventory. The routing information is configured in the ArcLink module bindings.

  Access
    Access information is a list of e-mail addresses (or generic ids) allowed to access the waveform data for a
    certain stream. Those e-mail addresses should be configured on the arclink-access modules bindings
    (:ref:`arclink-access bindings information <arclink-access>`). To set a stream (or network, station) as
    restricted you have to modify the inventory that is loaded into your SeisComp3 inventory database.



See Also
--------

  1. :ref:`ArcLink protocol <arclink_protocol>`
  2. :ref:`ArcLink request handler protocol <request_handler_protocol>`

