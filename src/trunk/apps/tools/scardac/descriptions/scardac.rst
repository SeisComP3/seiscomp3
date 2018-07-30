scardac is a tool that scans an :ref:`SDS waveform archive <_slarchive>` for
available `miniSEED <http://www.iris.edu/data/miniseed.htm>`_ data. It is
intended to be run periodically, e.g. as a cronjob, and will collect information
about

* data extents - the absolute earliest and latest times data is available of a
  particular channel
* data segments - continuous data segments sharing the same quality and sampling
  rate attributes

The information is stored in the SeisComP3 database under the root element
:ref:`DataAvailability <api-datamodel-python>`. Access to the data is provided
by the :ref:`fdsnws` module via the services:

* :ref:`/fdsnws/station <sec-station>` (extent information only, see
  ``matchtimeseries`` and ``includeavailability`` request parameters).
* :ref:`/fdsnws/ext/availability <sec-avail>` (extent and segment information
  provided in different formats)

Workflow
--------

1. Read existing ``Extents`` from database
2. Scan the SDS archive for new channel IDs and create new ``Extents``
3. Subsequently process the ``Extents`` using ``threads`` number of parallel
   threads. For each ``Extent``:

   1. Find all available daily data files
   2. Sort the file list according date
   3. For each data file

     * Remove ``DataSegments`` that do longer exists
     * Update or create ``DataSegments`` that changed or are new
     * A segment is split if

       * the ``jitter`` (difference between previous records end time and
         current records start time) is exceeded
       * the quality or sampling rate changed

     * merge segment information into ``DataAttributeExtents`` (``Extents``
       sharing the same quality and sample rate information)
     * merge segment start and end time into overall ``Extent``


