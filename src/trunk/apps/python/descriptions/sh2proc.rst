sh2proc converts SeismicHandler (http://www.seismic-handler.org/) event data to
SeisComP3. Data is read from input file or `stdin` if no input file is
specified.  The result is available on `stdout`.

Since SeismicHandler only specifies station and component codes, a mapping to
SeisComP3 network, location and channel codes is necessary. The script assumes
that the same station code is not used in different networks. In case an
ambiguous id is found a warning is printed and the first network code is used.
The channel and stream code is extracted from the dectecStream and detecLocid
configured in the global binding. In case no configuration module is available
the first location and stream is used.

Example
=======

Converts the SeismicHandler file `shm.evt` and writes SC3ML into the file
`sc3.xml`. The database connection to read inventory and configuration
information is fetched from the default messaging connection.

.. code-block:: sh

   sh2proc shm.evt > sc3.xml

Reads SeismicHandler data from `stdin`. Inventory and configuration information
is provided through files.

.. code-block:: sh

   cat shm.evt | sh2proc --inventory-db=inv.xml --config-db=config.xml > sc3.xml
