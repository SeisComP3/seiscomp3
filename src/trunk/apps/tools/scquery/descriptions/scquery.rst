Reads database objects and writes them to the command line.

Examples
========

Choose any query profile defined in the :ref:`queries.cfg<scquery_files>`. Provide
the required parameters in the same order as in the database request. The required
parameters are indicated by hashes, e.g. ##latMin##

1. List all available query profiles:

   .. code-block:: sh

      scquery --showqueries

#. Profile **event_filter**: fetch all event IDs and event parameters for events
   with magnitude ranging from 2.5 to 5 in central Germany between 2014 and 2017:

   .. code-block:: sh

      scquery event_filter 50 52  11.5 12.5 2.5 5 2014-01-01 2018-01-01 -d localhost/seiscomp3 > events_vogtland.txt

#. Profile **eventByAuthor**: fetch all event IDs where the preferred origin was
   provided by a specfic author for events 2.5 to 5 with 6 to 20 phases in central
   Germany between 2014 and 2017:

   .. code-block:: sh

      scquery eventByAuthor 50 52  11.5 12.5 6 20 2.5 5 2014-01-01 2018-01-01 scautoloc -d localhost/seiscomp3 > events_vogtland.txt

Parameter files
===============

.. _scquery_files:

1. **scquery.cfg** configuration file containing the database connection, e.g.:

   .. code-block:: sh

      database.type = mysql
      database.parameters = sysop:sysop@localhost/seiscomp3


#. **queries.cfg**: file containing the database queries. Copy the file to
   @LOGDIR@ or @SYSTMCOFIGDIR@

   .. code-block:: sh

      queries = event_filter, eventByAuthor, eventWithStationCount,phaseCountPerAuthor

      query.event_filter.description = "Returns all events (lat, lon, mag, time) that fall into a certain region and a magnitude range"
      query.event_filter = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value,Origin.longitude_value, Origin.depth_value,Magnitude.magnitude_value, Magnitude.type from Origin,PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.eventByAuthor.description = "get events by prefered origin author etc"
      query.eventByAuthor = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value as lat,Origin.longitude_value as lon, Origin.depth_value as dep, Magnitude.magnitude_value as mag, Magnitude.type as mtype, Origin.quality_usedPhaseCount as phases, Event.type as type, Event.typeCertainty as certainty, Origin.creationInfo_author from   Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where  Event._oid=PEvent._oid and Origin._oid=POrigin._oid and  Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Origin.quality_usedPhaseCount >= ##minPhases## and Origin.quality_usedPhaseCount <= ##maxPhases## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##' and Origin.creationInfo_author like '##author##%';"

      query.eventWithStationCount.description = "get events by prefered origin author etc"
      query.eventWithStationCount = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value as lat,Origin.longitude_value as lon, Origin.depth_value as dep, Magnitude.magnitude_value as mag, Magnitude.type as mtype, Origin.quality_usedStationCount as stations, Event.type as type, Event.typeCertainty as certainty, Origin.creationInfo_author from   Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where  Event._oid=PEvent._oid and Origin._oid=POrigin._oid and  Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.phaseCountPerAuthor="Get phase count per author from #EventID#"
      query.phaseCountPerAuthor="select PEvent.publicID,Origin.creationInfo_author, max(Origin.quality_usedPhaseCount) from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, OriginReference where Origin._oid=POrigin._oid and Event._oid=PEvent._oid and OriginReference._parent_oid=Event._oid and OriginReference.originID=POrigin.publicID and PEvent.publicID='##EventID##' group by Origin.creationInfo_author;"
