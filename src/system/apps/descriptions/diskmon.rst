Diskmon is a SeisComP3 init script that checks on each call to :program:`seiscomp check`
the filesystem by running the following command:

.. code-block:: sh

   df | awk -v max="%d" \'{ if ( $5 > max ) print $0 }\'


where "%d" is replaced by the configured threshold. If there are lines in the
output (which means some filesystem exceed the usage threshold) it sends
the output along with a description line to all configured receipients using
the :program:`mail` command.

To make diskmon work it is important that :program:`mail` is working on the shell.
