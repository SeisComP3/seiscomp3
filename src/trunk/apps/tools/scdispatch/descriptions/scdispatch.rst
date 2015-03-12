scdispatch reads an SC3ML file and creates notifier objects for them that
are sent to the corresponding messaging groups (see :confval:`routingtable`).
In contrast to :ref:`scdb` which writes SC3ML files directly into the database
scdispatch uses the messaging bus. If :ref:`scmaster` is configured with
the database plugin messages will end up in the database as well.

scdispatch can work in two modes. The first mode is used when a concrete
operation is specified such as *add*, *update* or *remove*. In that case all
objects in the SC3ML are encapsulated in a notifier with that specific operation
and sent to the messaging. No check is performed if the object is already in
the database or not.

In the second mode scdispatch loads the corresponding objects from the database
and calculates differences. It will then create corresponding notifiers with
operations *add*, *update* or *remove* and sent them to the messaging. That mode
is quite close to a sync operation with the exception that top level objects
(such as origin or event) that are not part of the input SC3ML are left untouched
in the database. It can be used to synchronize event information from one system
with another.

Examples
========

#. Send different objects from a SC3ML file because the default behavior is to merge:

   .. code-block:: sh

      scdispatch -i test.xml

#. Send new objects:

   .. code-block:: sh

      scdispatch -i test.xml -O add

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

      scdispatch -i test.xml -O add

   or the event objects can be left out of the routing table, e.g.

   .. code-block:: sh

      scdispatch -i test.xml -O add \
                 --routingtable Pick:PICK, \
                                Amplitude:AMPLITUDE, \
                                Origin:LOCATION,StationMagnitude:MAGNITUDE, \
                                Magnitude:MAGNITUDE


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
