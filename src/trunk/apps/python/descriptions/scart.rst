The archive tool scart creates playback files (multiplexed MiniSEED files) from
:term:`SDS` structured data (e.g. created by slarchive) or from data passed from
another record source such as :ref:`Arclink <arclink>`. It can also playback
records directly out of an SDS structure. Furthermore it can be used to import
multiplexed MiniSEED files into a local SDS structure.
So it is possible to save event based waveform data in combination with
scevtstreams into another archive.

.. _scart-config:

Configuration
=============

scart can make use of :ref:`global_recordstream`
implementations which are provided by additional plugins.
For loading additional plugins, e.g. the *xyz* plugin create and configure *scart.cfg*:

.. code-block:: sh

   plugins = xyz

Examples
========

#. Extract data from a local :term:`SDS` archive into a miniSEED file and sort by endtime:

   .. code-block:: sh

      scart -dsvE -t '<start-time>~<end-time>' <SDS archive> > file.mseed

   .. note::

      Sorting data is computational expensive but required for waveform playbacks.

#. Push miniSEED data into a local :term:`SDS` archive:

   .. code-block:: sh

      scart  -I file://<file.mseed> <SDS archive>

#. Collect data from an arclink server using the :ref:`global_recordstream`
   interface and write to a miniSEED file. The data streams and the time spans are
   defined in a list file, e.g. created by :ref:`scevtstreams`.

   .. code-block:: sh

      scart  -I arclink://<server>:18001 --list list.file --stdout > file.mseed

.. note::

   Repeated pushing of miniSEED data into an archive will duplicate the data.
