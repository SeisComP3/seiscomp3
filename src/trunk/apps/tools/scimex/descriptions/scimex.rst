scimex manages the SC3 object exchange between two or more different SeisComP3 systems in
real time. scimex may import or export the data to one or several systems. In
contrary to :ref:`scimport` the exchange of the SC3 objects is event based.
This means no messages will be exchanged until the exporting system has produced
an event.

By default all objects (picks, amplitudes, origins, arrivals, station
magnitudes, magnitudes, magnitude references) are transferred to the other
system. The user can define filters at both the sender and the receiver, to
limit the events for which objects are transferred. Possible filter parameters
are the event location, magnitude, arrival count and agency. scimex supports
two modi: *import* and *export*. In export mode scimex collects all objects
relevant for an event (e.g. picks, amplitudes, origins, magnitudes) from
scmaster's message groups at the source and checks if the filter criteria
match. Once the criteria are fulfilled, the whole package of objects is send
to the scmaster IMPORT group of the receiving system.

At the receiving SC3 system an instance of scimex runs in import mode. It
fetches the whole event information from its own IMPORT group, checks the local
filter criteria of the system and sends the collected objects to the different
message groups, e.g. Pick, Amplitude, Magnitude, Location. In export mode
several recipients can be defined and for each recipient individual filters
can be set. To run several instances of scimex on one system, aliases have to
be defined, e.g. for import:

.. code-block:: sh

   seiscomp alias create scimex_import scimex

and for export:

.. code-block:: sh

   seiscomp alias create scimex_export scimex

Then the configuration can be split into scimex_import.cfg and
scimex_export.cfg.


Examples
========

For a push-type configuration, in which the exporting server must be able to
connect to the Spread server on the receiving host. On the receiving host:

scimex_import.cfg

.. code-block:: sh

   connection.username = scimexIm
   connection.server = localhost

   mode = IMPORT

   cleanupinterval = 86400

   importHosts = import1

   criteria.world.longitude = -180:180
   criteria.world.latitude = -90:90
   criteria.world.magnitude = 1:9
   criteria.world.agencyID = ""
   criteria.world.arrivalcount = 15

   hosts.import1.address = localhost
   # The criterion "world" has been defined above
   hosts.import1.criteria = world
   # optional and true per default
   hosts.import1.filter = false
   # optional and true per default
   hosts.import1.useDefinedRoutingTable = true
   hosts.import1.routingtable = Pick:IMPORT,StationAmplitude:IMPORT,
                                  Origin:LOCATION,Arrival:LOCATION,
                                  StationMagnitude:MAGNITUDE,
                                  Magnitude:MAGNITUDE,
                                  StationMagnitudeContribution:MAGNITUDE,
                                  OriginReference:EVENT,Event:EVENT


In this example, Pick and StationAmplitude objects are sent to the
receiving system's IMPORT group to avoid interfering with the receiving system's
picking.

On the sending system, only those events with a high enough magnitude
and enough arrivals, and with the AgencyID "GFZ" are exported:
scimex_export.cfg

.. code-block:: sh

   connection.username="scimexEx"
   connection.server = localhost

   mode = EXPORT

   cleanupinterval = 7200
   exportHosts = exp1, exp2

   # Match everything with magnitude above or equal 5
   # and with more than 25 phases which comes from
   # agency GFZ.
   criteria.globalM5.latitude     = -90:90
   criteria.globalM5.longitude    = -180:180
   criteria.globalM5.magnitude    = 5:10
   criteria.globalM5.arrivalcount = 25
   criteria.globalM5.agencyID     = GFZ

   # Export to a system which still runs a very old version. The
   # messages need to be converted.
   hosts.exp1.address = 192.168.0.3
   hosts.exp1.criteria = globalM5
   hosts.exp1.conversion = imexscdm0.51

   hosts.exp2.address = 192.168.0.4
   hosts.exp2.criteria = globalM5
