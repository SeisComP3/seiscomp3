scevtstreams reads all picks of an event and determines the time window between
the first pick and the last pick. In addition a time margin is added to this
time window. It writes the streams that are picked including the determined
time window for the event to stdout. This tool gives appropriate input
information for :ref:`scart` and
`caps <https://docs.gempa.de/caps/current/apps/capstool.html>`_
(Common Acquisition Protocol Server by gempa GmbH) to dump waveforms from archives
based on event data.


Examples
========

#. Get the time windows for an event in the database:

   .. code-block:: sh

      scevtstreams -E gfz2012abcd -d mysql://sysop:sysop@localhost/seiscomp3

#. Get the time windows for an event in an XML file:

   .. code-block:: sh

      scevtstreams -i event.xml

#. Create a playback of an event with a time window of 5 minutes data and sort the records by end time:

   .. code-block:: sh

      scevtstreams -E gfz2012abcd -d mysql://sysop:sysop@localhost/seiscomp3 -L 0 -m 300 |\
      scart -dsvE --list - ~/seiscomp3/acquisition/archive > gfz2012abcd-sorted.mseed

#. Download waveforms from Arclink and import into local archive. Include all stations from the contributing networks:

   .. code-block:: sh

      scevtstreams -E gfz2012abcd -d mysql://sysop:sysop@localhost/seiscomp3 -L 0 -m 300 -R --all-stations |\
      scart --list - ./my-archive
