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
   @CONFIGDIR@ or to @SYSTEMCONFIGDIR@.

   **General event queries**

   .. code-block:: sh

      queries = eventFilter, eventUncertainty, eventByAuthor, eventWithStationCount, phaseCountPerAuthor, eventType

      query.eventFilter.description = "Returns all events (lat, lon, mag, time) that fall into a certain region and a magnitude range"
      query.eventFilter = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value,Origin.longitude_value, Origin.depth_value,Magnitude.magnitude_value, Magnitude.type from Origin,PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.eventUncertainty.description = "Returns all events (eventsIDs, time, lat, lat error, lon, lon error, depth, depth error, magnitude, region name) in the form of an event catalog"
      query.eventUncertainty = "select PEvent.publicID, Origin.time_value as OT, ROUND(Origin.latitude_value,3), ROUND(Origin.latitude_uncertainty,3), ROUND(Origin.longitude_value,3), ROUND(Origin.longitude_uncertainty,3), ROUND(Origin.depth_value,3), ROUND(Origin.depth_uncertainty,3), ROUND(Magnitude.magnitude_value,1), EventDescription.text from Event, PublicObject as PEvent, EventDescription, Origin, PublicObject as POrigin, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and Event.preferredOriginID=POrigin.publicID and Event.preferredMagnitudeID=PMagnitude.publicID and Event._oid=EventDescription._parent_oid and EventDescription.type='region name' and Event.type = '##type##' and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.eventByAuthor.description = "Get events by prefered origin author etc"
      query.eventByAuthor = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value as lat,Origin.longitude_value as lon, Origin.depth_value as dep, Magnitude.magnitude_value as mag, Magnitude.type as mtype, Origin.quality_usedPhaseCount as phases, Event.type as type, Event.typeCertainty as certainty, Origin.creationInfo_author from   Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where  Event._oid=PEvent._oid and Origin._oid=POrigin._oid and  Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Origin.quality_usedPhaseCount >= ##minPhases## and Origin.quality_usedPhaseCount <= ##maxPhases## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##' and Origin.creationInfo_author like '##author##';"

      query.eventWithStationCount.description = "Get events by prefered origin author etc"
      query.eventWithStationCount = "select PEvent.publicID, Origin.time_value as OT, Origin.latitude_value as lat,Origin.longitude_value as lon, Origin.depth_value as dep, Magnitude.magnitude_value as mag, Magnitude.type as mtype, Origin.quality_usedStationCount as stations, Event.type as type, Event.typeCertainty as certainty, Origin.creationInfo_author from   Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where  Event._oid=PEvent._oid and Origin._oid=POrigin._oid and  Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

      query.phaseCountPerAuthor.description="Get phase count per author from #EventID#"
      query.phaseCountPerAuthor="select PEvent.publicID,Origin.creationInfo_author, max(Origin.quality_usedPhaseCount) from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, OriginReference where Origin._oid=POrigin._oid and Event._oid=PEvent._oid and OriginReference._parent_oid=Event._oid and OriginReference.originID=POrigin.publicID and PEvent.publicID='##EventID##' group by Origin.creationInfo_author;"

      query.eventType.description = "Returns all eventIDs from event where the type is flagged as 'event type'"
      query.eventType = "select pe.publicID, o.time_value as OT from PublicObject pe, PublicObject po, Event e, Origin o where pe._oid = e._oid and po._oid = o._oid and e.preferredOriginID = po.publicID and e.type = '##type##' and o.time_value >= '##startTime##' and o.time_value <= '##endTime##'";


   **More examples and some statistics**

   .. code-block:: sh

        queries = time, mag_time, space_time, all, space_mag_time, event, fm_space_time, picks, assoc_picks, pref_assoc_picks, sta_net_mag, sta_net_mag_type, delta_sta_net_mag, delta_sta_net_mag_type

        query.time.description = "Events in time range"
        query.time = "select PEvent.publicID, Origin.time_value, round(Origin.latitude_value,4), round(Origin.longitude_value,4), round(Origin.depth_value, 1), round(Magnitude.magnitude_value, 1), Magnitude.type, Origin.quality_usedPhaseCount, Origin.quality_usedStationCount, Event.typeCertainty, Event.type, Origin.creationInfo_author from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

        query.mag_time.description = "Events in magnitude-time range"
        query.mag_time = "select PEvent.publicID, Origin.time_value, round(Origin.latitude_value,4), round(Origin.longitude_value,4), round(Origin.depth_value,1), round(Magnitude.magnitude_value,1), Magnitude.type, Origin.quality_usedPhaseCount, Origin.quality_usedStationCount, Event.typeCertainty, Event.type, Origin.creationInfo_author from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

        query.space_time.description = "Events in space-time range"
        query.space_time = "select PEvent.publicID, Origin.time_value, round(Origin.latitude_value,4), round(Origin.longitude_value,4), round(Origin.depth_value,1), round(Magnitude.magnitude_value,1), Magnitude.type, Origin.quality_usedPhaseCount, Origin.quality_usedStationCount, Event.typeCertainty, Event.type, Origin.creationInfo_author from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

        query.all.description = "Events in space-magnitude-time-quality range by author"
        query.all = "select PEvent.publicID, Origin.time_value, round(Origin.latitude_value,4), round(Origin.longitude_value,4), round(Origin.depth_value, 1), round(Magnitude.magnitude_value, 1), Magnitude.type, Origin.quality_usedPhaseCount, Origin.quality_usedStationCount, Event.typeCertainty, Event.type, Origin.creationInfo_author from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Origin.quality_usedPhaseCount >= ##minPhases## and Origin.quality_usedPhaseCount <= ##maxPhases## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##' and Origin.creationInfo_author like '##author##%';"

        query.space_mag_time.description = "Events in space-magnitude-time range"
        query.space_mag_time = "select PEvent.publicID, Origin.time_value, round(Origin.latitude_value,4), round(Origin.longitude_value,4), round(Origin.depth_value,1), round(Magnitude.magnitude_value,1), Magnitude.type, Origin.quality_usedPhaseCount, Origin.quality_usedStationCount, Event.typeCertainty, Event.type, Origin.creationInfo_author from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Magnitude.magnitude_value >= ##minMag## and Magnitude.magnitude_value <= ##maxMag## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

        query.fm_space_time.description = "Events with focal mechanisms in space-time range"
        query.fm_space_time = "select PEvent.publicID, Origin.time_value, round(Origin.latitude_value,4), round(Origin.longitude_value,4), round(Origin.depth_value,1), round(Magnitude.magnitude_value,1), Magnitude.type, MomentTensor.doubleCouple, MomentTensor.variance, Event.typeCertainty, Event.type, Origin.creationInfo_author from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, Magnitude, PublicObject as PMagnitude, FocalMechanism, PublicObject as PFocalMechanism, MomentTensor where Event._oid=PEvent._oid and Origin._oid=POrigin._oid and Magnitude._oid=PMagnitude._oid and PMagnitude.publicID=Event.preferredMagnitudeID and FocalMechanism._oid=PFocalMechanism._oid and PFocalMechanism.publicID=Event.preferredFocalMechanismID and MomentTensor._parent_oid = FocalMechanism._oid and POrigin.publicID=Event.preferredOriginID and Origin.latitude_value >= ##latMin## and Origin.latitude_value <= ##latMax## and Origin.longitude_value >= ##lonMin## and Origin.longitude_value <= ##lonMax## and Origin.time_value >= '##startTime##' and Origin.time_value <= '##endTime##';"

        query.event.description ="List authors and number of origins for event"
        query.event="select PEvent.publicID,Origin.creationInfo_author, max(Origin.quality_usedPhaseCount) from Origin, PublicObject as POrigin, Event, PublicObject as PEvent, OriginReference where Origin._oid=POrigin._oid and Event._oid=PEvent._oid and OriginReference._parent_oid=Event._oid and OriginReference.originID=POrigin.publicID and PEvent.publicID='##EventID##' group by Origin.creationInfo_author;"

        query.picks.description = "List number of picks per station in a certain timespan"
        query.picks = "SELECT waveformID_networkCode AS Network, waveformID_stationCode AS Station, COUNT(_oid) AS Picks, MIN(time_value) AS Start, MAX(time_value) AS End FROM Pick WHERE time_value >= '##startTime##' AND time_value <= '##endTime##' GROUP BY waveformID_networkCode, waveformID_stationCode;"

        query.assoc_picks.description = "list number of associated picks per station in a certain time span"
        query.assoc_picks = "SELECT Pick.waveformID_networkCode AS Network, Pick.waveformID_stationCode AS Station, COUNT(DISTINCT(Pick._oid)) AS Picks, MIN(Pick.time_value) AS Start, MAX(Pick.time_value) AS End FROM Pick, PublicObject PPick, Arrival WHERE Pick._oid = PPick._oid AND PPick.publicID = Arrival.pickID AND Pick.time_value >= '##startTime##' AND Pick.time_value <= '##endTime##' GROUP BY Pick.waveformID_networkCode, Pick.waveformID_stationCode;"

        query.pref_assoc_picks.description = "list number of associated picks of preferred origins per station for certain time span"
        query.pref_assoc_picks = "SELECT Pick.waveformID_networkCode AS Network, Pick.waveformID_stationCode AS Station, COUNT(DISTINCT(Pick._oid)) AS Picks, MIN(Pick.time_value) AS Start, MAX(Pick.time_value) AS End FROM Pick, PublicObject PPick, Arrival, Origin, PublicObject POrigin, Event WHERE Event.preferredOriginID = POrigin.publicID AND Origin._oid = POrigin._oid AND Origin._oid = Arrival._parent_oid AND Pick._oid = PPick._oid AND PPick.publicID = Arrival.pickID AND Pick.time_value >= '##startTime##' AND Pick.time_value <= '##endTime##' GROUP BY Pick.waveformID_networkCode, Pick.waveformID_stationCode;"

        query.sta_net_mag.description = "compares station magnitudes of a particular station with the network magnitude in a certain time span"
        query.sta_net_mag = "SELECT StationMagnitude.waveformID_networkCode AS Network, StationMagnitude.waveformID_stationCode AS Station, StationMagnitude.magnitude_value AS StaMag, Magnitude.magnitude_value AS NetMag, Magnitude.type AS NetMagType, StationMagnitude.creationInfo_creationTime AS CreationTime FROM StationMagnitude, PublicObject PStationMagnitude, StationMagnitudeContribution, Magnitude WHERE StationMagnitude._oid = PStationMagnitude._oid AND StationMagnitudeContribution.stationMagnitudeID = PStationMagnitude.publicID AND StationMagnitudeContribution._parent_oid = Magnitude._oid AND StationMagnitude.waveformID_networkCode = '##netCode##' AND StationMagnitude.waveformID_stationCode = '##staCode##' AND StationMagnitude.creationInfo_creationTime >= '##startTime##' AND StationMagnitude.creationInfo_creationTime <= '##endTime##' ORDER BY StationMagnitude.creationInfo_creationTime;"

        query.sta_net_mag_type.description = "compares station magnitudes of a particular station with the network magnitude of specific type in a certain time span"
        query.sta_net_mag_type = "SELECT StationMagnitude.waveformID_networkCode AS Network, StationMagnitude.waveformID_stationCode AS Station, StationMagnitude.magnitude_value AS StaMag, Magnitude.magnitude_value AS NetMag, Magnitude.type AS NetMagType, StationMagnitude.creationInfo_creationTime AS CreationTime FROM StationMagnitude, PublicObject PStationMagnitude, StationMagnitudeContribution, Magnitude WHERE StationMagnitude._oid = PStationMagnitude._oid AND StationMagnitudeContribution.stationMagnitudeID = PStationMagnitude.publicID AND StationMagnitudeContribution._parent_oid = Magnitude._oid AND StationMagnitude.waveformID_networkCode = '##netCode##' AND StationMagnitude.waveformID_stationCode = '##staCode##' AND StationMagnitude.creationInfo_creationTime >= '##startTime##' AND StationMagnitude.creationInfo_creationTime <= '##endTime##' AND Magnitude.type = '##magType##' ORDER BY StationMagnitude.creationInfo_creationTime;"

        query.delta_sta_net_mag.description = "calculates delta values of station and network magnitudes for all stations in a certain time span"
        query.delta_sta_net_mag = "SELECT StationMagnitude.waveformID_networkCode AS Network, StationMagnitude.waveformID_stationCode AS Station, AVG(StationMagnitude.magnitude_value - Magnitude.magnitude_value) AS DeltaAvg, MIN(StationMagnitude.magnitude_value - Magnitude.magnitude_value) AS DeltaMin, MAX(StationMagnitude.magnitude_value - Magnitude.magnitude_value) AS DeltaMax, MIN(StationMagnitude.creationInfo_creationTime) AS Start, MAX(StationMagnitude.creationInfo_creationTime) AS End FROM StationMagnitude, PublicObject PStationMagnitude, StationMagnitudeContribution, Magnitude WHERE StationMagnitude._oid = PStationMagnitude._oid AND StationMagnitudeContribution.stationMagnitudeID = PStationMagnitude.publicID AND StationMagnitudeContribution._parent_oid = Magnitude._oid AND StationMagnitude.creationInfo_creationTime >= '##startTime##' AND StationMagnitude.creationInfo_creationTime <= '##endTime##' GROUP BY StationMagnitude.waveformID_networkCode, StationMagnitude.waveformID_stationCode;"

        query.delta_sta_net_mag_type.description = "calculates delta values of station and network magnitudes for all stations and all magnitude types in a certain time span"
        query.delta_sta_net_mag_type = "SELECT StationMagnitude.waveformID_networkCode AS Network, StationMagnitude.waveformID_stationCode AS Station, AVG(StationMagnitude.magnitude_value - Magnitude.magnitude_value) AS DeltaAvg, MIN(StationMagnitude.magnitude_value - Magnitude.magnitude_value) AS DeltaMin, MAX(StationMagnitude.magnitude_value - Magnitude.magnitude_value) AS DeltaMax, Magnitude.type AS NetMagType, MIN(StationMagnitude.creationInfo_creationTime) AS Start, MAX(StationMagnitude.creationInfo_creationTime) AS End FROM StationMagnitude, PublicObject PStationMagnitude, StationMagnitudeContribution, Magnitude WHERE StationMagnitude._oid = PStationMagnitude._oid AND StationMagnitudeContribution.stationMagnitudeID = PStationMagnitude.publicID AND StationMagnitudeContribution._parent_oid = Magnitude._oid AND StationMagnitude.creationInfo_creationTime >= '##startTime##' AND StationMagnitude.creationInfo_creationTime <= '##endTime##' GROUP BY StationMagnitude.waveformID_networkCode, StationMagnitude.waveformID_stationCode, Magnitude.type;"
