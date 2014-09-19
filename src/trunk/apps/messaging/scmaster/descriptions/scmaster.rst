A SeisComP3 system consists of a set of independent applications each performing a discrete task.
The communication between the applications is realized by a TCP/IP based messaging system.
This messaging system is based on the open source toolkit `Spread <http://www.spread.org>`_ that provides a high performance
messaging service across local and wide area networks. At the top of Spread a mediator,
scmaster, handles additional requirements of SeisComP3 that are not natively provided by Spread.
The messaging system is used for the exchange of meta data (e.g. picks) and configurations to some extent.

.. figure:: media/scmaster/system.*
   :alt: sketch of a distributed system
   :align: center

   Schematic view on a distributed SeisComP3 system.

----

scmaster was designed as a kind of microkernel which delegates client requests. Therefore it
is the key application responsible for the orchestration of the distributed system. In order to participate
in the distributed system a client needs to send a connect request to the scmaster. In turn the master
returns an acknowledgment message which either informs the client of its admission or rejection. If the
connect request was successful the acknowledgment message will provide the client with the available message
groups it can subscribe to. Moreover, all currently connected clients will be notified about the newly joined
member. In case the master is configured with a database the client will also receive a direct follow up
message which holds the address of this database if the dbplugin is enabled. The address can be used to
retrieve archived data later on.

After a connection has been established every message will pass through the master first where it is
processed accordingly and then relayed to the target groups. Once a client is done with processing a disconnect
message will be sent to the master who in turn notifies all remaining clients about the leaving.
scmaster can be configured with a database (dbplugin) to ensure the integrity of the system. Before a message
is distributed by scmaster the message is written to the specified database. This way each message
is stored before it enters the system. In case of a crash all necessary information can be recovered
from the database. Currently, driver exist for MySQL, PostgreSQL and sqlite. Note that the scmaster can
run without a database but loses data integrity in doing so.

A common :file:`scmaster.cfg` looks like this:

.. code-block:: sh

   plugins = dbplugin
   plugins.dbPlugin.dbDriver = mysql
   plugins.dbPlugin.readConnection = "sysop:sysop@host/seiscomp3"
   plugins.dbPlugin.writeConnection = "sysop:sysop@localhost/seiscomp3"

