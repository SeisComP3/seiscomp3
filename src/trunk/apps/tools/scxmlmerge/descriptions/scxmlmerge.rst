scxmlmerge reads all SeisComP3 elements from one or more XML files in SC3ML format.
It merges the content and prints the result to standard output. The input can contain
and :ref:`SeisComP3 element<api-datamodel-python>` and the content can
be filtered to print only some elements such as EventParameters.
The output can be redirected into one single file and used by other applications.

The supported :ref:`SeisComP3 elements<api-datamodel-python>` are:

* EventParameters
* Inventory
* Config
* Routing
* QualityControl
* DataAvailability

By default all supported elements will be parsed and merged. Duplicates are removed.
Use options to restrict the element types.

.. note::

    Use also :ref:`scinv` for merging inventory XML files and for extracting
    inventory information.

Examples
========

#. Merge the all SeisComP3 elements from 2 XML files into a single XML file:

   .. code-block:: sh

      scxmlmerge file1.xml file2.xml > file.xml

#. Merge the all EventParameters and all Config elements from 2 XML files into a
   single XML file. Other element types will be ignored:

   .. code-block:: sh

      scxmlmerge -E -C file1.xml file2.xml > file.xml
