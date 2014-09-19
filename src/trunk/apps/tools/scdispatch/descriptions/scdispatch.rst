Examples
========

#. Send all objects from a SC3ML file. The default behavior is to add:

   .. code-block:: sh

      scdispatch -i test.xml

#. Send an update:

   .. code-block:: sh

      scdispatch -i test.xml -O update


#. Remove the objects:

   .. code-block:: sh

      scdispatch -i test.xml -O remove


#. Subsets of SC3ML Objects

   It can be useful to import a subset of QuakeML objects, e.g. Origins from other
   agencies and then allow :ref:`scevent` to associate them to existing
   events (and possibly prefer them based on the rules in scevent) or create new
   events for the origins. If the event objects from a SC3ML file are not required
   to be sent to the messaging then either they should be removed (e.g. using XSLT)
   and all the remaining objects in the file added:

   .. code-block:: sh

      scdispatch -i test.xml

   or the event objects can be left out of the routing table, e.g.

   .. code-block:: sh

      scdispatch -i test.xml \
                 --routingtable Pick:PICK, \
                                Arrival:ARRIVAL,Amplitude:AMPLITUDE, \
                                Origin:LOCATION,StationMagnitude:MAGNITUDE, \
                                StationMagnitudeContribution:MAGNITUDE, \
                                Magnitude:MAGNITUDE

   .. note:: The routing table approach is slower, scdispatch still has to try
      to send to the invalid group for each object. It seems that it should be
      possible to route to Event:NULL or some other non used group but this seems
      to always result in another Event (the one in the source SC3ML) being sent
      and loaded. Is this a bug?


#. Testing

   For testing it is useful to watch the results of dispatch with :ref:`scolv` or
   :ref:`scxmldump`. It is also useful to clean the database and logs to remove
   objects from persistent storage to allow repeated reloading of a file.

   .. note:: The following will clear all events from the database and any other
      other object persistence. Modify the mysql command to suit your db setup.

   .. code-block:: sh

      mysql -u root --password='my$q1' -e "DROP DATABASE IF EXISTS seiscomp3; \
        CREATE DATABASE seiscomp3 CHARACTER SET utf8 COLLATE utf8_bin; \
        GRANT ALL ON seiscomp3.* TO 'sysop'@'localhost' IDENTIFIED BY 'sysop'; \
        USE seiscomp3;source seiscomp3/trunk/share/db/mysql.sql;"

      seiscomp start
