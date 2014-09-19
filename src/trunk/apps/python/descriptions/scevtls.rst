scevtls lists all available event ids within a given time range to stdout.

Example
=======

Print all event ids for the complete year 2012.

.. code-block:: sh

   scevtls -d mysql://sysop:sysop@localhost/seiscomp3 \
           --begin "2012-01-01 00:00:00" \
           --end "2013-01-01 00:00:00"
