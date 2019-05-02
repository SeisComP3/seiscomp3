scamp measures several different kinds of amplitudes from waveform data.
It listens for origins and measures amplitudes in time windows determined
from the origin. The resulting amplitude objects are sent to the "AMPLITUDE"
messaging group. scamp is the counterpart of scmag. Usually, all
amplitudes are computed at once by scamp and then published.
Only very rarely an amplitude needs to be recomputed if the location of an
origin changes significantly. The amplitude can be reused by scmag, making
magnitude computation and update efficient. Currently, the automatic picker
in SeisComP3, scautopick, also measures a small set of amplitudes
(namely "snr" and "mb", the signal-to-noise ratio and the amplitude used in
mb magnitude computation, respectively) for each automatic pick in fixed
time windows. If there already exists an amplitude, e.g. a previously determined
one by scautopick, scamp will not measure it again for the respective stream.

Amplitudes are also needed, however, for manual picks. scamp does this as well.
Arrivals with weight smaller than 0.5 (default) in the corresponding Origin are
discarded. This minimum weight can be configured with
:confval:`amptool.minimumPickWeight`.

Amplitudes for the following magnitudes are currently computed:


:term:`MLh <magnitude, local (ML)>`
   Local magnitude calculated on the vertical component using a correction term
   to fit with the standard ML

:term:`MLv <magnitude, local vertical (MLv)>`
   Local magnitude calculated on the vertical component using a correction term
   to fit with the standard ML

:term:`MLh <magnitude, local horizontal (MLh)>`
   Local magnitude calculated on the horizontal components to SED specifications.

:term:`MLr <magnitude, local GNS/GEONET (MLr)>`
   Local magnitude calculated from MLv amplitudes based on GNS/GEONET specifications
   for New Zealand.

:term:`MN <magnitude, Nuttli (MN)>`
   Canadian Nuttli magnitude.

:term:`mb <magnitude, body-wave (mb)>`
   Narrow band body wave magnitude measured on a WWSSN-SP filtered trace

:term:`mB <magnitude, broadband body-wave (mB)>`
   Broad band body wave magnitude

:term:`Mwp <magnitude, broadband P-wave moment (Mwp)>`
   The body wave magnitude of Tsuboi et al. (1995)

:term:`Mjma <magnitude, JMA (M_JMA)>`
   Mjma is computed on displacement data using body waves of period < 30s

:term:`Ms(BB) <magnitude, surface wave (Ms)>`
   Broad band surface-wave magnitude

:term:`Md <magnitude, duration (Md)>`
   Duration magnitude as described in https://earthquake.usgs.gov/research/software/#HYPOINVERSE

Note that in order to be used by scmag, the input amplitude names for the
various magnitude types must match exactly.

Re-processing
=============

*scamp* can be used to reprocess and to update amplitudes, e.g. when inventory paramters
had to be changed retrospectively. Updating ampitudes requires waveform access.
The update can be performed in

1. offline processing based on xml files (:confval:`--ep<`). :confval:`--reprocess<reprocess>`
   will replace exisiting amplitudes. Updated values can be dispatched to the messing by
   :ref:`scdispatch` making them available for further processing, e.g. by :ref:`scmag`.
#. with messaging by setting :confval:`start-time` or :confval:`end-time`. All parameters
   are read from the database. :confval:`--commit<commit>` will send the
   updated parameters to the messing system making them available for further processing,
   e.g. by :ref:`scmag`. Otherwise, XML output is generated.

Offline amplitude update
------------------------

**Example:**

.. code-block:: sh

   seiscomp exec scamp --ep evtID.xml --inventory-db inventory.xml --config-db config.xml \
                       --reprocess --debug > evtID_update.xml
   scdispatch -O merge -H host -i evtID_update.xml

Amplitude update with messaging
-------------------------------

**Example:**

.. code-block:: sh

   scamp -u testuser --inventory-db inventory.xml --config-db config.xml -H host \
         --start-time '2016-10-15 00:00:00' --end-time '2016-10-16 19:20:00' \
         --commit
