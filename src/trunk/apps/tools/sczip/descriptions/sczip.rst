sczip can compress and decompress data using the ZIP algorithm (PKZIP). It
is not meant to supersede any available packagers but a little helper to
decompress SC3 zipped XML formats. Like GZip, sczip can only handle one file
and does not support archives. It compresses a byte stream and outputs a byte
stream.

Examples
========

Decompress a file

.. code-block:: sh

   sczip -d file.xml.zip -o file.xml

.. code-block:: sh

   sczip -d file.xml.zip -o file.xml

Compress a file

.. code-block:: sh

   sczip file.xml -o file.xml.zip

.. code-block:: sh

   sczip < file.xml > file.xml.zip
