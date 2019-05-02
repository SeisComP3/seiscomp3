sccnv reads input given in a supported format, converts the content to another
format and writes the output. Use the option --format-list
for a list of supported formats.

Formats
=======

Different formats are supported for input and output files.

+------------+--------------------------------------------------------------------------------------------+---------+---------+
| Name       | Description                                                                                | Input   | Output  |
+============+============================================================================================+=========+=========+
| arclink    | Arclink `XML format <https://www.seiscomp3.org/doc/applications/arclink-status-xml.html>`_ |    X    |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| bson       |                                                                                            |    X    |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| bson-json  |                                                                                            |         |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| csv        | comma-separated values                                                                     |         |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| hyp71sum2k | Hypo71 format                                                                              |         |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| ims10      |                                                                                            |         |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| json       | `JSON <https://www.json.org/>`_ format                                                     |    X    |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| qml1.2     | `QuakeML <https://quake.ethz.ch/quakeml/>`_ format                                         |         |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| qml1.2rt   | `QuakeML <https://quake.ethz.ch/quakeml/>`_ real time (RT) format                          |         |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| scdm0.51   |                                                                                            |    X    |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+
| trunk      | SeisComP3 XML - :ref:`SC3ML <api-datamodel-python>`                                        |    X    |    X    |
+------------+--------------------------------------------------------------------------------------------+---------+---------+

Examples
========

#. Print the list of supported formats:

   .. code-block:: sh

      $ sccnv --format-list

#. Convert an inventory file in arclink XML format to SC3ML and store the content in a file:

   .. code-block:: sh

      $ sccnv -i arclink:Package_inventory.xml -o inventory.sc3.xml

#. Convert an event parameter file in SC3ML format to ims1.0 and store the content in a file:

   .. code-block:: sh

      $ sccnv -i trunk:event.xml -o ims10:event.ims
