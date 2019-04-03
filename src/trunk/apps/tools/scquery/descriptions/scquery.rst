*scquery* reads and writes objects from the SeisComP3 database configured in
:ref:`scquery.cfg<scquery_config>`. It takes into account the query profiles
defined in :ref:`queries.cfg<scquery_queries>`.

.. _scquery_examples:

Examples
========

Choose any query profile defined in the :ref:`queries.cfg<scquery_queries>`. Provide
the required parameters in the same order as in the database request. The required
parameters are indicated by hashes, e.g. ##latMin##

1. List all available query profiles using the commandline option :confval:`showqueries`:

   .. code-block:: sh

      scquery -d localhost/seiscomp3 --showqueries

#. Profile **event_filter**: fetch all event IDs and event parameters for events
   with magnitude ranging from 2.5 to 5 in central Germany between 2014 and 2017:

   .. code-block:: sh

      scquery -d localhost/seiscomp3 event_filter 50 52  11.5 12.5 2.5 5 2014-01-01 2018-01-01 > events_vogtland.txt

#. Profile **eventByAuthor**: fetch all event IDs where the preferred origin was
   provided by a specfic author for events 2.5 to 5 with 6 to 20 phases in central
   Germany between 2014 and 2017:

   .. code-block:: sh

      scquery -d localhost/seiscomp3 eventByAuthor 50 52  11.5 12.5 6 20 2.5 5 2014-01-01 2018-01-01 scautoloc > events_vogtland.txt

#. Profile **eventType**: fetch all event IDs and event times from events
   with the given event type and within the provided time interval:

   .. code-block:: sh

      scquery -d localhost/seiscomp3 eventType explosion '2017-11-01 00:00:00' '2018-11-01 00:00:00'

Parameter and query profile files
=================================

.. _scquery_config:

1. The configuration file **scquery.cfg** contains the database connection paraemters.
   Copy the file to @CONFIGDIR@ or to @SYSTEMCONFIGDIR@:

   .. code-block:: sh

      database.type = mysql
      database.parameters = sysop:sysop@localhost/seiscomp3

   If the database connection is configured, the database option :confval:`-d <database>` in the
   :ref:`Examples<scquery_examples>` can be omitted or used to overwrite the configuration.

   .. _scquery_queries:

#. Create the profile file **queries.cfg** containing the database queries. Copy the file to
   @CONFIGDIR@ or to @SYSTEMCONFIGDIR@:

   .. code-block:: sh

      queries = eventFilter, eventUncertainty, eventByAuthor, eventWithStationCount, phaseCountPerAuthor, eventType

      query.eventFilter.description = "Returns all events (lat, lon, mag, time) that fall into a certain region and a magnitude range"
      query.eventFilter = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value,Origin.longitude_value, Origin.depth_value,Magnitude.magnitude_value, Magnitude.type from Origin,PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.eventUncertainty.description = "Returns all events (eventsIDs, time, lat, lat error, lon, lon error, depth, depth error, magnitude, region name) in the form of an event catalog"
      query.eventUncertainty = "select PEvent.publicID, Origin.time_value as OT, ROUND(Origin.latitude_value,3), ROUND(Origin.latitude_uncertainty,3), ROUND(Origin.longitude_value,3), ROUND(Origin.longitude_uncertainty,3), ROUND(Origin.depth_value,3), ROUND(Origin.depth_uncertainty,3), ROUND(Magnitude.magnitude_value,1), EventDescription.text from Event, PublicObject as PEvent, EventDescription, Origin, PublicObject as POrigin, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and Event.preferredOriginID=POrigin.publicID and Event.preferredMagnitudeID=PMagnitude.publicID and Event._oid=EventDescription._parent_oid and EventDescription.type='region name' and Event.type = '##type##' and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.eventByAuthor.description = "get events by prefered origin author etc"
      query.eventByAuthor = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value as lat,Origin.longitude_value as lon, Origin.depth_value as dep, Magnitude.magnitude_value as mag, Magnitude.type as mtype, Origin.quality_usedPhaseCount as phases, Event.type as type, Event.typeCertainty as certainty, Origin.creationInfo_author from   Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where  Event._oid=PEvent._oid and Origin._oid=POrigin._oid and  Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Origin.quality_usedPhaseCount >= ##minPhases## and Origin.quality_usedPhaseCount <= ##maxPhases## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##' and Origin.creationInfo_author like '##author##';"

      query.eventWithStationCount.description = "get events by prefered origin author etc"
      query.eventWithStationCount = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value as lat,Origin.longitude_value as lon, Origin.depth_value as dep, Magnitude.magnitude_value as mag, Magnitude.type as mtype, Origin.quality_usedStationCount as stations, Event.type as type, Event.typeCertainty as certainty, Origin.creationInfo_author from   Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where  Event._oid=PEvent._oid and Origin._oid=POrigin._oid and  Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.phaseCountPerAuthor.description="Get phase count per author from #EventID#"
      query.phaseCountPerAuthor="select PEvent.publicID,Origin.creationInfo_author, max(Origin.quality_usedPhaseCount) from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, OriginReference where Origin._oid=POrigin._oid and Event._oid=PEvent._oid and OriginReference._parent_oid=Event._oid and OriginReference.originID=POrigin.publicID and PEvent.publicID='##EventID##' group by Origin.creationInfo_author;"

      query.eventType.description = "Returns all eventIDs from event where the type is flagged as 'event type'"
      query.eventType = "select pe.publicID, o.time_value as OT from PublicObject pe, PublicObject po, Event e, Origin o where pe._oid = e._oid and po._oid = o._oid and e.preferredOriginID = po.publicID and e.type = '##type##' and o.time_value >= '##startTime##' and o.time_value <= '##endTime##'";
