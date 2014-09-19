Switching from Zurich release and before to new releases means also to lose
the old :command:`seiscomp config` which allowed to edit configuration
parameters (superseded by :ref:`scconfig`) and station metadata.

To make the transition phase easier for users that still manage their inventory
with key files, this important part has been made available in this version
as well.

.. code-block:: sh

   $ seiscomp exec stationconf
   SeisComP version 3.0 (2012.229)

   A) Add/Edit network
   R) Remove network
   W) Write configuration and quit
   Q) Quit without writing configuration
   Command? [A]:

When the configuration is written with :kbd:`w`, an inventory XML file
is created in :file:`etc/inventory` named :file:`stations-key.xml`. From this
point the same procedures needs to be applied as if a new inventory file has
been added to :file:`etc/inventory`, namely
:command:`seiscomp update-config inventory`.


Porting stations from older versions
====================================

To port station key files from older version to this version, the only thing
that needs to be done is to copy 3 directories:

- :file:`old/key` to :file:`new/etc/key-2.5/key`
- :file:`old/config` to :file:`new/etc/key-2.5/config`
- :file:`old/resp` to :file:`new/etc/key-2.5/resp`
