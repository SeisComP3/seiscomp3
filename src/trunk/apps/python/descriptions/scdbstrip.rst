SeisComP3s processing is continuously writing to the database. This causes
the database to grow and to occupy much space on the harddisc. scdbstrip taggles
this problem and removed processed objects from the database older than a
configurable time span.

This clean-up procedure is based on events. scdbstrip will remove all events
with an origin time older than specified. It will also remove all associated
objects such as picks, origins, arrivals, amplitudes and so on.

scdbstrip does not run as a daemon. To remove old objects continuously scdbstrip
should be added to the list of cronjobs running every e.g. 30 minutes. The more
often it runs the less objects it has to remove and the faster it will unlock
the database again.


Known issues
============

When running scdbstrip for the first time on a large database it can happen
that it aborts in case of MYSQL with the following error message:


   .. code-block:: sh

      [  3%] Delete origin references of old events...08:48:22 [error]
      execute("delete Object from Object, OriginReference, old_events where
      Object._oid=OriginReference._oid and
      OriginReference._parent_oid=old_events._oid") = 1206 (The total number
      of locks exceeds the lock table size)

      Exception: ERROR: command 'delete Object from Object, OriginReference,
      old_events where Object._oid=OriginReference._oid and
      OriginReference._parent_oid=old_events._oid' failed

That means your MYSQL server cannot hold enough data required for deletion.
There are two solutions to this:

1. Increase the memory pool used by MYSQL by changing the configuration to:

   .. code-block:: sh

      innodb_buffer_pool_size = 64M

   The size of the new buffer depends on the size of the database that should
   be cleaned up.


2. Run scdbstrip on smaller batches for the first time:

   .. code-block:: sh

      $ scdbstrip -d seis:mypass@localhost/seiscomp3 --days 1000
      $ scdbstrip -d seis:mypass@localhost/seiscomp3 --days 900
      ...
      $ scdbstrip -d seis:mypass@localhost/seiscomp3 --days 100


Examples
========

#. Keep the events of the last 30 days

   .. code-block:: sh

      scdbstrip --days 30 -d mysql://sysop:sysop@localhost/seiscomp3
