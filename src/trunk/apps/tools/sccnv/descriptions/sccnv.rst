sccnv reads input given in a supported format, converts the content to another
format and writes the output. Use the option --format-list
for a list of supported formats.

Examples
========

#. Read the list of supported formats:

   .. code-block:: sh

      $ sccnv --format-list

#. Convert an file in arclink XML format to SC3ML and store the content in a file:

   .. code-block:: sh

      $ sccnv -i arclink:Package_inventory.xml -o inventory.sc3.xml
