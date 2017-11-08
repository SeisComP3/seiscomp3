scsohlog connects to the messaging and collects all information sent from connected
clients. It creates an XML file and writes that to disc at a configurable interval.
That output can be read by any consumer and converted to the desired output.

Example
=======

Create an output XML file every 60 seconds and execute a custom script to process
that XML file.

.. code-block:: sh

   #!/bin/sh
   scsohlog -o stat.xml -i 60 --script process-stat.sh

You can also preconfigure these values:

.. code-block:: sh

   monitor.output.file = /path/to/stat.xml
   monitor.output.interval = 60
   monitor.output.script = /path/to/script.sh
