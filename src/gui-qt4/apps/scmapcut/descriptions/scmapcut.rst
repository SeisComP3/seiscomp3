*scmapcut* is a commandline tool to create image files containing maps of specific
regions and for selected events. When plotting events given by their eventID, the
event parameters must be provided in a SeisComP3 event XML file. The XML file can
be retrieved from the database using :ref:`scxmldump`.

Examples
========

1. Draw a map for the event with event ID <eventID>. Plot a region of at least
   3 degrees around the epicentre. The created image has 800x400 px.

   .. code-block:: sh

      scmapcut -E <eventID> --ep <eventID>.xml -m 3 -d 800x400 -o <eventID>.png

   .. _fig-workflow:

   .. figure:: media/gempa2017xxxx.png
      :align: center
      :width: 10cm

      Image example.

#. Draw a map for a generic event with magnitude 4. The size of the event shown
   on the map scales with magnitude. Plot a region of at least 3 degrees around
   the epicentre. The created image has 800x400 px.


   .. code-block:: sh

      scmapcut --lat 44 --lon 12 --depth 10 --mag 4 -m 0.5 -d 800x400 -o generic.png

   .. _fig-workflow:

   .. figure:: media/generic.png
      :align: center
      :width: 10cm

      Generic example.
