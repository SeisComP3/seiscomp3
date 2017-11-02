scbulletin transforms the parameters of an event or an origin either to autoloc1
or to autoloc3 format.

Input Modes
-----------

Two modes of parameter input are possible:

1. Either one can fetch all necessary information from database directly
#. or one can provide a representation of the origin as XML file, to transform it
   to a standard format.

The first mode is the dump-mode the second is the input-mode. For dumping either
choose eventID or the originID. If the eventID is choosen the preferred origin
will be used.

Output Modes
------------

Different output formats are available:

1. **autoloc1** working with **-1**
#. **autoloc3** working with **-3**
#. **extended autoloc3** working with **-3 -x**
#. **enhanced** working with **-1 -e** or  **-3 -e** for high-precision output.

If called with an event or origin ID a database connection is necessary to
fetch the corresponding object. Otherwise scbulletin will read the input source
(defaults to stdin), grab the first found event or origin and dump it.

Examples
========

#. Create bulletin from event in database

   .. code-block:: sh

      scbulletin -d mysql://sysop:sysop@localhost/seiscomp3 -E gfz2012abcd

#. Convert XML file to bulletin

   .. code-block:: sh

      scbulletin < gfz2012abcd.xml

   .. code-block:: sh

      scbulletin -i gfz2012abcd.xml
