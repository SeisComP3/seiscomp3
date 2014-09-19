invextr extracts or removes networks, stations and channels from an inventory
XML file.
It takes basically three important parameters:

1. channel ID list
2. input file
3. output file

whereas the output file defaults to stdout and the input file to
stdin if not given.

A channel ID is a simple string that is matched against the final channel ID
in the inventory. This final channel ID is constructed by joining the codes of
all stages with a dot where the stages are network, station, location and
channel.

Examples
--------

Suppose an inventory with network GE, a station MORC and several channels:

.. code-block:: sh

   network GE
     station MORC
       location __
         channel BHZ    ID: GE.MORC..BHZ
         channel BHN    ID: GE.MORC..BHN
         channel BHE    ID: GE.MORC..BHE
         channel LHZ    ID: GE.MORC..LHZ
         channel LHN    ID: GE.MORC..LHN
         channel LHE    ID: GE.MORC..LHE

The IDs are matched against what is passed with --chans.

.. code-block:: sh

   invextr --chans "GE*" inv.xml

Nothing is filtered because GE* matches all available IDs.

.. code-block:: sh

   invextr --chans "*MORC*" inv.xml

Nothing is filtered again because *MORC* matches all available IDs.

.. code-block:: sh

   invextr --chans "GE.MORC" inv.xml

Everything is filtered because GE.MORC does not match with any ID. To make it
work, an asterisk needs to be appended: GE.MORC* or GE.MORC.*.

To extract all vertical components, use:

.. code-block:: sh

   invextr --chans "*Z" inv.xml

To extract BHN and LHZ, use:

.. code-block:: sh

   invextr --chans "*BHN,*LHZ" inv.xml

To remove all HH and SH channels, use:

.. code-block:: sh

   invextr --rm --chans "*HH?,*SH?" inv.xml
