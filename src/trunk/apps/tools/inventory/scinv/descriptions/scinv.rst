scinv merges inventory XML files to a single inventory, synchronises an inventory
with another (most common use is with database), creates initial key files and
much more ...

scinv is used by :file:`etc/init/scinv.py` to synchronise the inventory from
:file:`etc/inventory` with the database.

.. code-block:: sh

   seiscomp update-config inventory

Commands
========

scinv works with different commands. The command **must** be given as **1st**
parameter to the application. All others parameters must follow.

.. code-block:: sh

   scinv $command [options] [files]


sync
----

Synchronises an applications inventory with a given source given as file(s).
The applications inventory is either read from the database or given with
*--inventory-db*. As a result all information in the source is written to target
and target does not contain any additional information. The source must hold all
information. This works different to merge. If an output file is specified with
*-o* no notifiers are generated and sent via messaging.

This command is used by :file:`etc/init/scinv.py` as follows:

.. code-block:: sh

   scinv sync --console=1 -H localhost:$p --filebase "$fb" \
              --rc-dir "$rc" --key-dir "$kd"

where

.. code-block:: sh

   $p = configured messaging port
   $fb = $SEISCOMP_ROOT/etc/inventory
   $rc = $SEISCOMP_ROOT/var/lib/rc
   $kd = $SEISCOMP_ROOT/etc/key


merge
-----

Merges two or more inventories into one inventory. This command
is useful to merge existing subtrees into a final inventory before
synchronization.

.. code-block:: sh

   scinv merge net1.xml net2.xml -o inv.xml


apply
-----

Applies stored notifiers created with **sync** and option *--create-notifer*
which is saved in a file (*-o*). Source is the applications inventory read
from the database or given with *--inventory-db*.
If *-o* is passed no messages are sent but the result is stored in a file.
Useful to test/debug or prepare an inventory for offline processing.


.. code-block:: sh

   # Synchronise inventory and save the notifiers locally (no messages
   # are sent)
   scinv sync -d mysql://sysop:sysop@localhost/seiscomp3 \
         --create-notifier -o sync_patch.xml

   # Sent the notifiers to the target system
   scinv apply -H localhost sync_patch.xml

This operation can be useful to save synchronisation diffs for validation or to
debug problems.


keys
----

Synchronise station key files with current inventory pool. This command merges
all XML files in the inventory pool (or the given files) and checks if a
corresponding station key file in :file:`etc/key` exists. If not an empty
station key file is created. If a station key file without a corresponding
station in the merged inventory is found, it is deleted.

ls
--

List contained items up to channel level. This command is useful to inspect
an XML file or the complete inventory pool.

.. code-block:: sh

   $ scinv ls SK.KOLS.xml
     network SK       Slovak National Network of Seismic Stations
       epoch 1980-01-01
       station KOLS   Kolonicke sedlo, Slovakia
         epoch 2004-09-01
         location __
           epoch 2004-09-01
           channel BHE
             epoch 2006-04-25 12:00:00 - 2010-03-24
           channel BHN
             epoch 2006-04-25 12:00:00 - 2010-03-24
           channel BHZ
             epoch 2006-04-25 12:00:00 - 2010-03-24
           channel EHE
             epoch 2004-09-01 - 2006-04-25 10:00:00
           channel EHN
             epoch 2004-09-01 - 2006-04-25 10:00:00
           channel EHZ
             epoch 2004-09-01 - 2006-04-25 10:00:00
           channel HHE
             epoch 2006-04-25 12:00:00 - 2010-03-24
           channel HHE
             epoch 2010-03-25
           channel HHN
             epoch 2006-04-25 12:00:00 - 2010-03-24
           channel HHN
             epoch 2010-03-25
           channel HHZ
             epoch 2006-04-25 12:00:00 - 2010-03-24
           channel HHZ
             epoch 2010-03-25

The default level of information printed is *chan*. Available levels are *net*,
*sta*, *chan* and *resp*.

To check the available networks and stations in the inventory pool, calling

.. code-block:: sh

   scinv ls

is enough.


check
-----

Checks consistency of passed inventory files or a complete filebase. In the
first step the inventory is merged from all files. In the second step several
consistency checks are applied such as:

- overlapping epochs on each level (network, station, ...)
- valid epochs (start < end)
- defined gain in a stream
- set gainUnit
- distance of the sensor location to the station location
- "invalid" location 0/0

In future further checks will be added to make this tool a real help for
correct meta data creation.
