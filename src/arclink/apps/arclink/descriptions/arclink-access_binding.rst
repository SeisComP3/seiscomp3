For each network, station, stream marked as restricted in the inventory you
may define a set of e-mail addresses of users that should have access to the
data. This information is stored inside the SeisComp3 databases and it is later
used by the request-handler (part of ArcLink) during the request
authorization.

The process of expanding the access rules and generation of entries in the database
is similar to the routing.

Generating Access
-----------------

To define a list of users (pedro and joao) that have access to a certain
restricted network, for example 9U, just create a file called profile_9u at the
etc/key/arclink-access/ with the following line:

.. code-block:: sh

   access.users = pedro, joao

or on a more sophisticated case:

.. code-block:: sh

   access.users = pedro, joao
   access.streams = BH*

Then link this profile from each station binding file in the etc/key
folder. To link this profile just add a line like:

.. code-block:: sh 

   arclink-access:9u

to the station key file (file named like station_[Network Code]_[Station Code]).

Tuning of the access can be done by using disabledStationCode (advised),
streams, start and end parameters like (similar syntax and behavior) done with
the :ref:`routing binding <define-arclink-routing>` configuration on the ArcLink
module.

Dumping Access
--------------

To dump access you should use the *dump_db* command, that is also used to dump
the routing.  Just add the option *with-access* like in:

.. code-block:: sh

   % dump_db --routing --with-access routing.xml

the resulting file will then contain a list of access entries found in the database.
Example:

.. code-block:: xml

   <?xml version="1.0" encoding="utf-8"?>
   <ns0:routing xmlns:ns0="http://geofon.gfz-potsdam.de/ns/Routing/1.0/">
     <ns0:route locationCode="" networkCode="GE"
                publicID="Route#20120830110313.459068.338"
                stationCode="" streamCode="">
       <ns0:arclink address="localhost:18001" end="" priority="1"
                    start="1980-01-01T00:00:00.0000Z" />
       <ns0:seedlink address="localhost:18000" priority="1" />
     </ns0:route>
     <ns0:access end="" locationCode="" networkCode="GE"
                 start="1980-01-01T00:00:00.0000Z" stationCode=""
                 streamCode="" user="pedro" />
     <ns0:access end="" locationCode="" networkCode="GE"
                 start="1980-01-01T00:00:00.0000Z" stationCode=""
                 streamCode="" user="joao" />
   </ns0:routing>
