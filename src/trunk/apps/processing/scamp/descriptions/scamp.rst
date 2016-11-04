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
Picks with weight smaller than 0.5 in the corresponding Origin are discarded.

Amplitudes for the following magnitudes are currently computed:

MLv
   Local magnitude calculated on the vertical component using a correction term to fit with the standard ML.

MLh
   Local amplitude calculated on the horizontals.

mb
   Narrow band body wave magnitude using a third order Butterworth filter with corner frequencies of 0.7 and 2.0 Hz. Note that this amplitude is also computed by scautopick for all automatic picks.

mB
   Broad band body wave magnitude.

Mw(mB)
   Estimation of the moment magnitude Mw based on mB.

Note that in order to be used by scmag, the input amplitude names for the
various magnitude types must match exactly.

Re-processing
============

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
