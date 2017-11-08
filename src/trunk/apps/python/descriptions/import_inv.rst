import_inv is a wrapper for inventory converters. Inventory converters convert
an input format such as

+-------------------+-------------------------------------------------------------------+
| Format            | Description                                                       |
+===================+===================================================================+
| sc3               | `SeisComP3 inventory XML <http://geofon.gfz-potsdam.de/schema/>`_ |
+-------------------+-------------------------------------------------------------------+
| key               | SeisComP station keys files (key version 2.5)                     |
+-------------------+-------------------------------------------------------------------+
| arclink           | Arclink inventory XML                                             |
+-------------------+-------------------------------------------------------------------+
| nettab            | GEOFON nettabs                                                    |
+-------------------+-------------------------------------------------------------------+
| dlsv              | `dataless SEED <http://www.iris.edu/data/dataless.htm>`_          |
+-------------------+-------------------------------------------------------------------+
| fdsnxml           | `FDSN StationXML <http://www.fdsn.org/xml/station/>`_             |
+-------------------+-------------------------------------------------------------------+

to SeisComP3 inventory XML which is the read by the trunk config module to
synchronize the local inventory file pool with the central inventory database.

If another format needs to be converted it is very easy to provide a new
converter.


Converter interface
-------------------

To make a new converter make with import_inv it must implement an interface
on shell level. Furthermore the converter program must be named :file:`{format}2inv`
and must live in :file:`SEISCOMP_ROOT/bin`.

The converter program must take the input location (file, directory, URL, ...)
as first parameter and the output file (SeisComP3 XML) as second parameter. The
output file must be optional and default to stdout.

To add a new converter for a new format, e.g. Excel, place the new converter
program at :file:`$SEISCOMP_ROOT/bin/excel2inv`. When
:program:`import_inv help formats` is called it globs for
:file:`$SEISCOMP_ROOT/bin/*2inv` and prints all available formats.
