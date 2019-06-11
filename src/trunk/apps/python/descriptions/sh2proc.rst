sh2proc converts `Seismic Handler <http://www.seismic-handler.org/>`_ event data to
SeisComP3 XML format. Data is read from input file or `stdin` if no input file is
specified.  The result is available on `stdout`.

Code mapping
============

Since Seismic Handler only specifies station and component codes, a mapping to
SeisComP3 network, location and channel codes is necessary. The script assumes
that the same station code is not used in different networks. In case an
ambiguous id is found a warning is printed and the first network code is used.
The channel and stream code is extracted from the dectecStream and detecLocid
configured in the global bindings. In case no configuration module is available
the first location and stream is used.

Event parameters
================

* Event types given in Seismic Handler files are mapped to SeisComP3 event types:

  .. csv-table::
     :header: "Seismic Handler", "SeisComP3"

     "teleseismic quake","earthquake"
     "regional quake","earthquake"
     "local quake","earthquake"
     "quarry blast","quarry blast"
     "nuclear explosion","nuclear explosion"
     "mining event","mining explosion"

* The EventID given in Seismic Handler files is mapped as a comment to the event.

Magnitudes
==========

* Magnitude types given in Seismic Handler files are mapped to SeisComP3 magnitudes:

  .. csv-table::
     :header: "Seismic Handler", "SeisComP3"

     "m","M"
     "ml","ML"
     "mb","mb"
     "ms","Ms(BB)"
     "mw","Mw"
     "bb","mB"

* ML magnitudes in Seismic Handler files have no corresponding measured amplitudes.
  Therefore the ML station magnitudes are converted without referencing the amplitude.

* Seismic Handler uses the phase name "L" for referring to surface waves without
  further specification. The phase name is kept unchanged.

Distance calculations
=====================

In Seismic Handler files distances can be given in units of km or degree but in
SeisComP3 only degree is used. Both representations are considered for conversion.
In case of double posting preference is given to the Seismic Handler values given in km
due to their higher precision.

Beam parameters
===============

Seismic Handler files provide the phase picks with theoretical, measured and corrected
slowness and (back) azimuth but the pick in SeisComP3 knows only one value.
During conversion highest preference is given to corrected values.
The theoretical values are ignored.

Limitations
===========

The following parameters from Seismic Handler files are not considered:

* Phase Flag
* Location Input Params
* Reference Location Name
* Quality Number
* Ampl&Period Source
* Location Quality
* Reference Latitude
* Reference Longitude
* Amplitude Time

Further processing in SeisComP3
===============================

The created XML files can be used in multiple ways, e.g.:

#. By other modules in an XML-base playback
#. Inject into the messaging system by :ref:`scdispatch`
#. Integrate into the database by :ref:`scdb`

Examples
========

#. Convert the Seismic Handler file `shm.evt` and writes SC3ML into the file
   `sc3.xml`. The database connection to read inventory and configuration
   information is fetched from the default messaging connection.

   .. code-block:: sh

      sh2proc shm.evt > sc3.xml

#. Read Seismic Handler data from `stdin`. Inventory and configuration information
   is provided through files.

   .. code-block:: sh

      cat shm.evt | sh2proc --inventory-db=inventory.xml --config-db=config.xml > sc3.xml

shm.evt file format
===================

The list of parameters supported by sh2proc may be incomplete.
Read the original `format and parameter description <http://www.seismic-handler.org/wiki/ShmDocFileEvt>`_
of the SeismicHandler .evt files for providing correct input files.

Example of a SeismicHandler `shm.evt` file with supported parameters:

.. code-block:: sh

    Event ID               : 1170102002
    Station code           : VITZ
    Onset time             : 2-JAN-2017_12:25:40.415
    Onset type             : emergent
    Phase name             : Pg
    Event Type             : mining event
    Applied filter         : SHM_BP_1HZ_25HZ_3
    Component              : Z
    Quality number         : 2
    Pick Type              : manual
    Weight                 : 4
    Theo. Azimuth (deg)    :   27.29
    Theo. Backazimuth (deg):  207.36
    Distance (deg)         :  0.122
    Distance (km)          : 13.572
    Magnitude ml           : 1.0
    Phase Flags            : L
    --- End of Phase ---


    Event ID               : 1170102002
    Station code           : WESF
    Onset time             : 2-JAN-2017_12:25:53.714
    Onset type             : emergent
    Phase name             : Pg
    Event Type             : mining event
    Applied filter         : SHM_BP_1HZ_25HZ_3
    Component              : Z
    Quality number         : 2
    Pick Type              : manual
    Weight                 : 4
    Theo. Azimuth (deg)    :  106.98
    Theo. Backazimuth (deg):  287.91
    Distance (deg)         :  0.807
    Distance (km)          : 89.708
    Magnitude ml           : 1.8
    Mean Magnitude ml      : 1.1
    Latitude               : +50.779
    Longitude              :  +10.003
    Depth (km)             :   0.0
    Depth type             : (g) estimated
    Origin time            :  2-JAN-2017_12:25:38.273
    Region Table           : GEO_REG
    Region ID              : 5326
    Source region          : Tann, E of Fulda
    Velocity Model         : deu
    Location Input Params  : 20
    Reference Location Name: CENTRE
    --- End of Phase ---
