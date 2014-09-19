sync_arc is a tool that merges inventory from different arclink sources into
your SeisComP3 database by following a list of nodes (master table) provided by
the webdc.eu portal (or another source as configured on the configuration file).
This tool is today used inside the EIDA (European Integrated Data Archives)
network to keep its nodes synchronized (the inventory and routing parts).

.. warning::

   Running :command:`seiscomp update-config` (or using the GUI
   :ref:`scconfig`) to update the system configuration will erase from your
   database the last one-way synchronized inventory and routing. It will be
   necessary to run the sync_arc again to restore the missing entries of the
   inventory and routing after each update.
