scevtstreams reads all picks of an event and determines the time window between
the first pick and the last pick. In addition a time margin is added to this
time window. It writes the streams that are picked including the determined
time window for the event to stdout. This tool gives appropriate input
information for :ref:`scart` to archive event based data.


Examples
========

Create a playback of an event with a time window of 5 minutes data and sort
the records by end time:

.. code-block:: sh

   scevtstreams -E gfz2012abcd -d mysql://sysop:sysop@localhost/seiscomp3 -L 0 -m 300 |
   scart -dsvE --list - ~/seiscomp3/acquisition/archive > gfz2012abcd-sorted.mseed

Download waveforms from Arclink and import into local archive:

.. code-block:: sh

   scevtstreams -E gfz2012abcd -d mysql://sysop:sysop@localhost/seiscomp3 -L 0 -m 300 -R |
   scart --list - ./my-archive
