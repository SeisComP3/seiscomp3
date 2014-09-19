scm monitors client activity. scm connects to a certain master and periodically
processes the status messages sent by the clients.

Each client status if forwarded to the plugins loaded by scm. By default
the :ref:`mncursesplugin <scm_ncurses>` is loaded which presents an interface
similar to the gnu program top.

Filters
=======

Plugins might support filtering client status information. To configure filters
each plugin supports a configuration value :confval:`$name.filter`. This filter
is a string which can be constructed from available status info tags and logical
and numerical operators.

List of tags:

.. code-block:: sh

   time
   privategroup
   hostname
   clientname
   ips
   programname
   pid
   cpuusage
   totalmemory
   clientmemoryusage
   memoryusage
   sentmessages
   receivedmessages
   messagequeuesize
   summedmessagequeuesize
   averagemessagequeuesize
   summedmessagesize
   averagemessagesize
   objectcount
   uptime
   responsetime

A filter might look like this:

.. code-block:: sh

   memailplugin.filter = "(cpuusage>100 || totalmemory>1000) && hostname==proc-machine"


Numerical operators
-------------------

Numerical operators are applied to a tag name and a constant value.

========  =================
Operator  Description
========  =================
==        equal
!=        not equal
<         less than
>         greater than
<=        less or equal
>=        greater or equal
========  =================


Logical operators
-----------------

Logical operators are applied to a group (might be enclosed in brackets) or
numerical expressions.

========  =================
Operator  Description
========  =================
!         not
&&        and
||        or
========  =================


Multiple instances
==================

To monitor different clients sets with different criteria and different plugins
it is common practice to create aliases of scm and to configure each instance
separately

.. code-block:: sh

   seiscomp alias create scm_level1 scm
   seiscomp alias create scm_level2 scm

where :program:`scm_level1` could monitor all mandatory clients whereas
:program:`scm_level2` monitors all clients which are not crucial for operation.
