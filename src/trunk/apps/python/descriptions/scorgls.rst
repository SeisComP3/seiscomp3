scorgls lists all available origin ids within a given time range to stdout.
Use :ref:`scevtls` for listing all event ids.

Example
=======

Print all origin ids for the complete year 2012.

.. code-block:: sh

   scorgls -d mysql://sysop:sysop@localhost/seiscomp3 \
           --begin "2012-01-01 00:00:00" \
           --end "2013-01-01 00:00:00"
