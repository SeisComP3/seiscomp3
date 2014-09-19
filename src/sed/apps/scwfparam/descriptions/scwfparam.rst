scwfparam is a SeisComP3 module that computes

- peak ground acceleration (PGA)
- peak ground velocity (PGV)
- relative displacement elastic response spectrum (DRS)
- pseudo absolute acceleration elastic response spectrum (PSA)

in real-time or offline. It includes a process scheduler and handles
reprocessing of data in a smart way. It supports ShakeMap XML output as
documented in the
`ShakeMap manual <http://pubs.usgs.gov/tm/2005/12A01/pdf/508TM12-A1.pdf>`_ each
time a new set of data is available.

Scheduling
==========

When the module is not started in offline mode, the processing of events is
scheduled following the configured rules. Parameters that influence the
scheduling are:

- :confval:`wfparam.cron.wakeupInterval`
- :confval:`wfparam.cron.updateDelay`
- :confval:`wfparam.cron.delayTimes`

The wake-up interval specifies when the scheduler is called to check if a
process is about to be started or stopped. The default is 10 seconds.

The scheduler checks then all scheduled jobs, adds a job to the processing queue
if the next run time is not in the future and removes all scheduled jobs with
timestamps in the past. The process queue contains all jobs that are about to
be executed. Because waveform acquisition is a time- and memory-costly operation
only one process can run at a time. Once a process finished, the next process in
the queue is executed (if any). When a process is started, it fetches the latest
event parameters (origin time, magnitude, location).

To add processes to the scheduler, the module distinguishes two cases:

1. Process creation (new event or updated event seen the first time)
2. Process update (event updates after an process has been created)


Process creation
----------------

When a new event or an event update is received which does not have an
associated process yet, a new process is created. The event
time (Origin[Event.preferredOriginID].time) is used to build the default
schedule according to :confval:`wfparam.cron.delayTimes`.

.. code-block:: py

   for each time in wfparam.cron.delayTimes:
     add_cron_job(process, Origin[Event.preferredOriginID].time + time)


Process update
--------------

If a process for an event already exists, the next run time is the current time
plus wfparam.cron.updateDelay. Before adding this job to the scheduler the
application checks if the next scheduled runtime is at least
:confval:`wfparam.cron.updateDelay` seconds after the new run time. If not, a
new job is not addded to the scheduler. Pseudo code to illustrate the strategy
is given below.

.. code-block:: py

   event_updated(event):
     p = process_for_event(event)
     # The schedule for process p could be {T1,T2,T3,T4}
     now = get_current_time()
     next_run = now + wfparam.cron.updateDelay
     # Process currently suspended?
     if isEmpty(p.schedule):
       p.schedule.add(next_run)
     elif (p.schedule[0] - next_run) > wfparam.cron.updateDelay:
       p.schedule.prepend(next_run)
     else:
       # Do nothing, ignore the event update
       pass


Processing
==========

The processing can be divided into the following steps:

- Collect all stations within the configured maximum distance
  (:confval:`wfparam.maximumEpicentralDistance` or
  :confval:`wfparam.magnitudeDistanceTable`)
- Remove already processed channels
- Find the velocity and acceleration stream with the highest sampling frequency

  - The sensor unit is used to distinguish between velocity and acceleration
    streams (M/S, M/S**2)

- Use all allowed components (:confval:`wfparam.streams.whitelist`,
  :confval:`wfparam.streams.blacklist`) of each stream
- Compute expected P arrival time if no pick is available
- Start waveform acquisition
- If the configured time window for one stream is complete, do (optional steps
  are written italic)

  - Check saturation depending on :confval:`wfparam.saturationThreshold`
  - Search maximum raw value (in counts)
  - Apply gain
  - Check STA/LTA threshold 5 seconds around P
  - *If velocity, differentiate data to acceleration*
  - *Compute pre-event cut-off if enabled*
  - Compute offset of pre-event time window
  - *Compute signal duration and check for aftershocks*
  - *Deconvolution using spectral division of FFT spectrum and transfer function*
  - *Apply optional sensitivity correction filter (lo-, hi- or bandpass)*
  - *Apply optional lo-pass, hi-pass or band-pass filter*
  - Compute PGA/PGV
  - Calculate response spectra

- If acquisition finished

  - Collect all values (also recently processed values)

    - Results from velocity streams are always preferred over acceleration
      streams if both are available (eg. co-located stations)

  - Generate ShakeMap event and station XML

    - Unless :confval:`wfparam.output.shakeMap.maximumOfHorizontals` is set
      to true all processed streams are included in XML

  - Call ShakeMap script and pass eventID and event ID path

The channel is considered to be processed if the last step succeeded.


Waveform archival
=================

If :confval:`wfparam.output.waveforms.enable` is set to true all processed
waveforms are stored in the configured output directory
:confval:`wfparam.output.waveforms.path`. The naming convention of a channel
MiniSEED file is:

[EventDateTime]_[net]_[sta]_[loc][cha]_[filter][order]_[freqs].mseed

If :confval:`wfparam.output.waveforms.withEventDirectory` is set to true, an
event directory with the eventID is created additionally where the channel
files are stored under.

Either:

.. code-block:: sh

   /path/to/waveforms/file1.mseed
   /path/to/waveforms/file2.mseed
   ...

or

.. code-block:: sh

   /path/to/waveforms/eventid/file1.mseed
   /path/to/waveforms/eventid/file2.mseed
   ...

The MiniSEED file contains uncompressed float 4096 byte records.

Example:

================== ==============================================
Event time         2011-11-21 08:30:00 Network: CH
Station            SNIB
Location           _ _
Channel            HGZ
Filter             hi-pass
Order              2
Corner frequencies 0.025
Filename           **20111121083000_CH_SNIB_HGZ_HP2_0.025.mseed**
================== ==============================================


Database
========

scwfparam can make use of the database schema extension for strong motion
parameters.

In order to prepare the database the extension schema must be applied. The
database schema is installed in :file:`share/db/wfparam/*.sql`. Login into the
database backend and source the .sql file corresponding to the used database
backend.

In order to enable :ref:`scmaster` to handle messages containing objects for
strong motion parameters load the dmsm (data model strong motion) plugin as
follows in scmaster.cfg:

.. code-block:: sh

   plugins = ${plugins}, dmsm


:ref:`scmaster` must be restarted to activate the plugin.

To activate scwfparam to send messages with strong motion objects, set

.. code-block:: sh

   wfparam.output.messaging = true

in scwfparam.cfg.


ShakeMaps
=========

The ShakeMap XML is generated according the documentation of version 3.5 if
:confval:`wfparam.output.shakeMap.enable` is set to true.

Below an example is given of an event XML and a station XML. The data was
generated from a playback and does **not** describe a **real event**.


Event XML
---------

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
   <!DOCTYPE earthquake SYSTEM "earthquake.dtd">
   <earthquake id="gfz2011oasp" lat="38.916" lon="40.0711"
               depth="10.3249" mag="5.80361" year="2011"
               month="7" day="19" hour="14" minute="54"
               second="21" timezone="GMT"
               locstring="tst2011oasp / 38.916 / 40.0711"
   />


Station XML
-----------

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
   <!DOCTYPE earthquake SYSTEM "stationlist.dtd">
   <stationlist created="" xmlns="ch.ethz.sed.shakemap.usgs.xml">
     <station code="JMB" name="JMB" lat="42.467" lon="26.583">
       <comp name="BHZ">
         <acc value="0.0175823522" flag="0"/>
         <vel value="0.0265134476" flag="0"/>
         <psa03 value="0.0177551343" flag="0"/>
         <psa10 value="0.0179450342" flag="0"/>
         <psa30 value="0.0507100318" flag="0"/>
       </comp>
     </station>
     <station code="BUD" name="BUD" insttype="STS-2/N"
              lat="47.4836" lon="19.0239">
       <comp name="BHZ">
         <acc value="0.0018418704" flag="0"/>
         <vel value="0.0012123935" flag="0"/>
         <psa03 value="0.0019287320" flag="0"/>
         <psa10 value="0.0033152716" flag="0"/>
         <psa30 value="0.0027636448" flag="0"/>
       </comp>
     </station>
     <station code="ANTO" name="ANTO" lat="39.868" lon="32.7934">
       <comp name="BHZ">
         <acc value="0.0322238962" flag="0"/>
         <vel value="0.0250842840" flag="0"/>
         <psa03 value="0.0326696355" flag="0"/>
         <psa10 value="0.0621788884" flag="0"/>
         <psa30 value="0.0903777107" flag="0"/>
       </comp>
     </station>
     <station code="GNI" name="GNI" lat="40.148" lon="44.741">
       <comp name="BHZ">
         <acc value="0.0760558909" flag="0"/>
         <vel value="0.0273735691" flag="0"/>
         <psa03 value="0.0818660133" flag="0"/>
         <psa10 value="0.1230812588" flag="0"/>
         <psa30 value="0.1682284546" flag="0"/>
       </comp>
     </station>
   </stationlist>


Examples
========

#. Running scwfparam offline with a multiplexed miniseed volume, an event xml
   and an inventory xml file. A hi-pass filter of 0.1hz (10secs) is used.
   Processing starts immediately and the application finishes when processing
   is done. The scheduler is disabled in offline mode.

   .. code-block:: sh

      scwfparam --offline -I vallorcine.mseed \
                --inventory-db vallorcine_inv.xml \
                --ep vallorcine.xml -E "Vallorcine.2005.09.08" \
                --lo-filter 0.1 --hi-filter 0

#. Running for a given event with scheduling enabled. Only the given event will
   be processed.

   .. code-block:: sh

      scwfparam -I arclink://localhost:18001 -E gfz2011oeej \
                -d mysql://sysop:sysop@localhost/seiscomp3

#. For running in real-time it is enough to add the module to the client list
   of the trunk package in seiscomp config.

#. Running with remote Arclink server

   To use a remote Arclink server it is
   enough to configure the record stream with -I:

   .. code-block:: sh

      scwfparam --offline -I vallorcine.mseed \
                --inventory-db vallorcine_inv.xml \
                --ep vallorcine.xml -E "Vallorcine.2005.09.08" \
                -I "arclink://arclink.ethz.ch:18002"

   Note that the default acquisition timeout of 30 seconds might not be enough
   to get all the requested data. If necessary, increase the value with
   parameter :confval:`wfparam.acquisition.initialTimeout`. This can also be
   reached on command line:

   .. code-block:: sh

      scwfparam --offline -I vallorcine.mseed \
                --inventory-db vallorcine_inv.xml \
                --ep vallorcine.xml -E "Vallorcine.2005.09.08" \
                -I "arclink://arclink.ethz.ch:18002" \
                --wfparam.acquisition.initialTimeout=300

#. Running with remote Seedlink server

   To use a remote Seedlink server it is enough to configure the record stream with -I:

   .. code-block:: sh

      scwfparam --offline -I vallorcine.mseed \
                --inventory-db vallorcine_inv.xml \
                --ep vallorcine.xml -E "Vallorcine.2005.09.08" \
                -I "slink://geofon.gfz-potsdam.de:18000"
