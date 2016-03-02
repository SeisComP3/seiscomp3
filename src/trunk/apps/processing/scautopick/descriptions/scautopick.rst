scautopick searches for waveform anomalies in form of changes in amplitude.
It basically applies a robust sta/lta algorithm to the waveform streams which
have been filtered before with a Butterworth filter of third order with corner
frequencies of 0.7 and 2 Hz. Once the sta/lta ratio reached a specific value
(by default 3) for a particular stream, a pick is set to the time when this
threshold is exceeded. Once the ratio reaches a factor of 3, a pick is created
and the picker is set inactive. The picker is reactivated for this stream once
the sta/lta ratio falls to the value of 1.5.

The second task of scautopick is to calculate amplitudes for given magnitude
types where the time window starts at the pick time. For example mb is calculated
for a fixed time window of 30s after the pick, mB for time window of 60s, for
MLv a time window of 150s is estimated to make sure that S-arrivals are inside
this time window. The precalculated amplitudes are sent out and received by
the magnitude tool.

scautopick usually runs in the background connected to a real-time data source
such as :ref:`Seedlink <seedlink>`. This is referred to as online mode. Another
option to run scautopick is on offline mode with files.

Online mode
===========

In online mode the workflow draws like this:

* scautopicks reads all of its bindings and subscribes to stations
  where :confval:`detecEnable` is set to ``true``
* the data time window requested from the data source is [system-:confval:`leadTime`, NULL]
  meaning an open end time that causes Seedlink to stream real-time data if no
  more data are in the buffers
* each incoming record is filtered according to :confval:`detecFilter`
* the samples are checked for exceedance of :confval:`trigOn` and in the positive
  case either a post picker (:confval:`picker`) is launched or a Pick object
  will be sent
* if :confval:`sendDetections` is set to ``true`` trigger will be sent in any
  case for e.g. debugging
* after the primary stage has finished (detector only or picker) a secondary
  picker will be launched if configured with :confval:`spicker`

These steps repeat for any incoming record.

Offline mode
============

.. note::

   Due to code changes in the file data source the command line option **--playback**
   is essential otherwise a real-time time window is set and all records are
   most likely filtered out.

To tune scautopick or to do playbacks it is helpful to run scautopick not with
a real-time data source but on a data set, e.g. a multiplexed sorted MiniSEED
volume. scautopick will apply the same workflow as in online mode but the
acquisition of data records has to change. If the input data (file) has been
read scautopick will exit and furthermore it must not ask for a particular
time window, especially not for a real-time time window. To accomplish that
the command line parameter :option:`--playback` has to be used.

.. code-block:: sh

   $ scautopick --playback -I data.mseed

This call will process all records in :file:`data.mseed` for which bindings
exist and send the results to the messaging. If all data records are processed
scautpick will exit. The processing steps are similar to the online mode.
