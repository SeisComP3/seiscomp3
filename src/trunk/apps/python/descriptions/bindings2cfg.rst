bindings2cfg dumps the bindings configuration from a specific key directory
to the given database or a configuration XML. In this way, the bindings parameters
can be configured in a directory different from $SEISCOMP_ROOT/etc/. From this
non-standard directory the configuration XML can be created without
prior writing the bindings to a database and reading from there using e.g.
:ref:`scxmldump`.

This utility is useful for repeating parameter tuning.

Examples
========

#. Write the bindings configuration from some key directory to a configuration
   XML file:

   .. code-block:: sh

     bindings2cfg --key-dir ./etc/key -o config.xml


#. Write the bindings configuration from some key directory to the seiscomp3
   database on localhost

   .. code-block:: sh

      bindings2cfg --key-dir ./etc/key -d mysql://sysop:sysop@localhost/seiscomp3
