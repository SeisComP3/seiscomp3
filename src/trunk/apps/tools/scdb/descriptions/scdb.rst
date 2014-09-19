A major component of the SeisComP3 system is the database. Almost all
applications have only read access to the database, but all the processing
results and objects have to be written into the database. This was the task of
scdb. In very first versions of SeisComP3 scdb was the only component that had
write access to the database. Its task is to connect to :ref:`scmaster` and populate
the database with all received notifier messages. Although it worked it
introduced race conditions caused by the latency of the database backend since
all other clients received the message at the same time. Accessing the database
immediately at this point in time did not guarantuee that the object was
written already.

In consequence, the scmaster itself gained write access to the database and
forwards messages to all clients after they are written to database.

:ref:`scdb` by definition does not check existing objects in the database. It only
generates INSERT/UPDATE/DELETE statements based on the data used and sends
these statements to the database. E.g. if :ref:`scdb` receives a message to
insert a new object into the database and this object exists already, the
database will raise an error because :ref:`scdb` hasn't checked it.


Online mode
-----------

Now scdb can be used to maintain a backup or archive the database that is not
part of the realtime processing. When running scdb as database write daemon it
can inform a client about the database connection to use. A client sends a
DatabaseRequest message and scdb sends back a DatabaseResponse message containing
the database connection parameters.

For that it connects to a messaging server and writes all received messages to a
configured database, e.g. a backup database.


.. code-block:: sh

   scdb -H server -o mysql://sysop:sysop@db-server/seiscomp3

In the above example :ref:`scdb` connects to "server" and writes all messages to the
output database. The database connection it received from the messaging server
during the handshake is reported to client requesting a database address. To
overwrite the read-only database, just override the applications database
address:

.. code-block:: sh

   scdb -H server -d mysql://sysop:sysop@db-server/seiscomp3 \
                  -o mysql://writer:12345@db-server/seiscomp3


Offline mode
------------

Another important task of :ref:`scdb` is to populate the database with any SeisComP3
datamodel content. In combination with :ref:`scxmldump` it can be used to copy events
from one database to another.

For that it does not connect to a messaging server but reads data from XML
files and writes it to the database.


.. warning::

   When reading XML files the output database address is not passed
   with -o but -d. The applications database address is used.

.. code-block:: sh

   scdb -i data.xml -d mysql://sysop:sysop@db-server/seiscomp3
