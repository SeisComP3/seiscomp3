scbulletin transforms the parameters of an origin either to autoloc1 or autoloc3
format. Two modes are possible. Either one can fetch all necessary information
from database directly or one can provide a representation of the origin as
file, to transform it to a standard format. The first mode is the dump-mode the
second is the input-mode. For dumping either choose eventID or the originID. If
the eventID is choosen the preferred origin will be used. Three output formats
are available autoloc1, autoloc3 or extended autoloc3.

If called with an event or origin ID a database connection is necessary to
fetch the corresponding object otherwise scbulletin will read the input source
(defaults to stdin) and grab the first found event that is dumped.

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
