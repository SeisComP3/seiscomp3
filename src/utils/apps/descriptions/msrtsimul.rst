msrtsimul simulates a real-time data acquisition by injecting miniSEED data from a
file into the seedlink buffer. It can be used for simulating real-time conditions
in playbacks for whole-system demonstrations, user training, etc.

.. note::

   The data a played back as if they were recorded at current time. Therefore creation
   times and the actual data times including pick times, event times etc. will be faked.
   Use the command line option *--mode historic* for keeping the actual data times.

* The historic mode allows to process waveforms in real time with the stream inventory
  valid at the time when the data were created including streams closed
  at current time. Using the historic mode may require :ref:`scautopick`
  to be started with the option *playback*.
* Use :ref:`scmssort` to sort the data by end time for a realistic playback.

Examples
--------

1. Playback miniSEED waveforms in real time with verbose output:

   .. code-block:: sh

      msrtsimul -v miniSEED-file

#. Playback miniSEED waveforms in historic mode. This may require :ref:`scautopick`
   to be started with the option *playback*:

   .. code-block:: sh

      msrtsimul -v -m historic miniSEED-file
