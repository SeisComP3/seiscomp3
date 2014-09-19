scimport is responsible to forward messages from one system to another. The
difference to :ref:`scimex` is that scimport does not handle the messages
event based. scimport supports two different modes. The relay mode does a
simple mapping from GROUP:SYSTEM_A to GROUP:SYSTEM_B. This mode is default.

In case GROUP is not defined in the second system the message is forwarded to
IMPORT_GROUP. The import mode supports custom mapping and filter functionality.
It is possible to forward GROUP1:SYSTEM_A to GROUP2:SYSTEM_B. In addition the
forwarded objects can be filtered by:

Pick
 - Status
 - Mode
 - Phase
 - AgencyID

Amplitude
 - Amplitude

Origin
 - Location
 - Depth
 - AgencyID
 - Status
 - Mode

Event
 - Type

StationMagnitude
 - Type

Magnitude
 - Type


Examples
========

Example scimport.cfg

.. code-block:: sh

   # The address of the importing system
   sink = sinkAddress

   # This option has to be set if the application runs in import mode.
   # The routing table has to be defined in the form of source_group:sink_group
   routingtable = PICK:PICK

   # List of sink groups to subscribe to. If this option is not set the message
   # groups will be determined automatically. If this option is set but not
   # needed for a setup it can be ignored with the option --ignore-groups
   msggroups = GROUP_ONE, GROUP_TWO

   # Available filter options
   filter.pick.mode     = manual
   filter.pick.status   = confirmed
   filter.pick.phase    = P
   filter.pick.agencyID = GFZ

   # Values: eq (==), lt (<=) ,gt (>=), *
   filter.amplitude.operator = gt
   filter.amplitude.amplitude = 100

   # Values: lat0:lat1 (range)
   filter.origin.latitude = -90:90

   # Values: lon0:lon1 (range)
   filter.origin.longitude = -180:180
   filter.origin.depth     = 0:100
   filter.origin.agencyID  = GFZ

   # Values: automatic, manual
   filter.origin.mode      = manual
   filter.origin.status    = confirmed

   # Values: earthquake, explosion, quarry blast, chemical explosion,
   # nuclear explosion, landslide, debris avalanche, rockslide,
   # mine collapse, volcanic eruption, meteor impact, plane crash,
   # building collapse, sonic boom, other
   filter.event.type = earthquake

   # Values: Whatever your magnitudes are named
   filter.stationMagnitude.type = MLv

   # Values: Whatever your magnitudes are named
   filter.magnitude.type = MLv

   # Values: latency, delay, timing quality, gaps interval, gaps length,
   # spikes interval, spikes amplitude, offset, rms
   filter.qc.type = latency
