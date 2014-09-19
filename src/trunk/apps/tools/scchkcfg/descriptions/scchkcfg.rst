scchkcfg checks for case-sensitivity issues of parameter names of a module
configuration. It reads all defined stages (see :ref:`global_modules`) and
checks for each parameter read if it exists again with a different spelling.

Examples
========

.. code-block:: sh

   $ scchkcfg scautopick
   Read configuration files OK
   No possible conflict detected

scchkcfg checks only for possible conflicts since it does not know what parameters
a module will read eventually.

.. code-block:: sh

   $ scchkcfg scautopick
   Read configuration files OK
   Conflict #1
    connection.server    /home/sysop/seiscomp3/etc/global.cfg:8
    connection.Server    /home/sysop/.seiscomp3/scautopick.cfg:1
   1 conflict detected

In this case everything is ok and the conflict needs to be fixed.
:confval:`connection.Server` is not a valid parameter name
(but :confval:`connection.server` is) in
:file:`/home/sysop/.seiscomp3/scautopick.cfg` and thus will not be used.


.. code-block:: sh

   $ scchkcfg scautopick
   Read configuration files OK
   Conflict #1
    module.trunk.global.amplitudes.mb.signalEnd    \
         /home/sysop/.seiscomp3/scautopick.cfg:1
    module.trunk.global.amplitudes.mB.signalEnd    \
         /home/sysop/.seiscomp3/scautopick.cfg:2
   1 conflict detected

In this case the configuration is OK and this is an example why the case-sensitivity
has changed from previous versions: mb != mB. scchkcfg detects a possible
conflicts but does not know that this case is well defined. But it helps the user to
decide whether it needs a fix or not.
