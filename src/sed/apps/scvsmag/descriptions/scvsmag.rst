Part of the :ref:`VS` package.

scvsmag is part of a new SeisComP implementation of the
`Virtual Seismologist`_ (VS) Earthquake Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released
under the `SED Public License for SeisComP Contributions`_. For a
given origin it estimates single station magnitudes and a network magnitude
based on  the envelope attenuation relationship and ground motion amplitude
ratio derived  by Cua (2005). The original VS algorithm applies the Bayesian
theorem by defining magnitude as the value that maximizes the product of a
likelihood function and a prior probability density function. In the current
version of scvsmag only the likelihood function is implemented and no prior
information is used at this stage.

Logging
=======

Apart from the standard log messages in :file:`scvsmag.log`, processing log messages are
also written to :file:`scvsmag-processing-info.log` every time the VS Magnitude of an event
is re-evaluated. A typical entry is shown below.

.. code-block:: sh

   1  2013/06/28 10:51:01 [processing/info/VsMagnitude] Start logging for event: sed2012cyqr
   2  2013/06/28 10:51:01 [processing/info/VsMagnitude] update number: 0
   3  2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..BNALP.HH; Wavetype: P-wave; Soil class: rock; Magnitude: 3.47
   4  2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 46.87; station lon: 8.43; epicentral distance: 32.26;
   5  2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 3.57e-03; PGV(Z): 6.91e-05; PGD(Z): 1.62e-06
   6  2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 2.67e-03; PGV(H): 3.44e-05; PGD(H): 1.02e-06
   7  2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..MUO.HH; Wavetype: S-wave; Soil class: rock; Magnitude: 3.83
   8  2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 46.97; station lon: 8.64; epicentral distance: 22.45;
   9  2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 8.19e-03; PGV(Z): 2.12e-04; PGD(Z): 6.91e-06
   10 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 2.18e-02; PGV(H): 5.00e-04; PGD(H): 1.72e-05
   11 2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..WILA.HH; Wavetype: P-wave; Soil class: rock; Magnitude: 3.50
   12 2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 47.41; station lon: 8.91; epicentral distance: 41.16;
   13 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 4.38e-03; PGV(Z): 6.42e-05; PGD(Z): 1.85e-06
   14 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 3.35e-03; PGV(H): 6.40e-05; PGD(H): 1.88e-06
   15 2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..ZUR.HH; Wavetype: S-wave; Soil class: rock; Magnitude: 3.79
   16 2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 47.37; station lon: 8.51; epicentral distance: 23.99;
   17 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 9.17e-02; PGV(Z): 1.03e-03; PGD(Z): 1.64e-05
   18 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 9.63e-02; PGV(H): 2.12e-03; PGD(H): 5.31e-05
   19 2013/06/28 10:51:01 [processing/info/VsMagnitude] VS-mag: 3.69; median single-station-mag: 3.79; lat: 47.15; lon: 8.52; depth : 25.32 km
   20 2013/06/28 10:51:01 [processing/info/VsMagnitude] creation time: 2012-02-11T22:45:40.00Z; origin time: 2012-02-11T22:45:26.27Z; t-diff: 13.73; time since origin arrival: 0.864; time since origin creation: 0.873
   21 2013/06/28 10:51:01 [processing/info/VsMagnitude] # picked stations: 6; # envelope streams: 79
   22 2013/06/28 10:51:01 [processing/info/VsMagnitude] Distance threshold (dt): 44.68 km; # picked stations < dt: 4; # envelope streams < dt: 4
   23 2013/06/28 10:51:01 [processing/info/VsMagnitude] Stations not used for VS-mag: CH.HASLI CH.LLS
   24 2013/06/28 10:51:01 [processing/info/VsMagnitude] Magnitude check: 0.027; Arrivals check: 0.000; Azimuthal gap: 34.992
   25 2013/06/28 10:51:01 [processing/info/VsMagnitude] likelihood: 0.99
   26 2013/06/28 10:51:01 [processing/info/VsMagnitude] End logging for event: sed2012cyqr

Explanation
-----------

The following table comments each line in the above output.

+---------+---------------------------------------------------------------------+
| Line    | Description                                                         |
+=========+=====================================================================+
| 1       | Start of the log message for the event with the given event ID      |
+---------+---------------------------------------------------------------------+
| 2       | Update counter for this event.                                      |
+---------+---------------------------------------------------------------------+
| 3 - 18  | Information about the stations that contribute to a VS magnitude    |
|         | estimate. Each station has four lines with the first line giving    |
|         | the stream name, the wavetype of the contributing amplitude,        |
|         | the soil type at the site and the single station magnitude. The     |
|         | next line shows the location and epicentral distance of the sensor. |
|         | On the two following lines peak-ground-acceleration (PGA) -velocity |
|         | (PGV) and -displacement (PGD) are given in SI units for vertical    |
|         | and the root-mean-square horizontal component.                      |
+---------+---------------------------------------------------------------------+
| 19      | The VS magnitude, the median of the single station magnitudes, the  |
|         | cordinates of the hypocenter                                        |
+---------+---------------------------------------------------------------------+
| 20      | The creation time of the magnitude, the origin time and the         |
|         | difference between the two ('tdiff'); also given are the time since |
|         | origin arrival and time since origin creation which is a measure of |
|         | how long it took to evaluate the first magnitude estimate.          |
+---------+---------------------------------------------------------------------+
| 21      | The number of stations contributing to an origin ('# picked         |
|         | stations') and the number of envelope streams available             |
|         | ('# envelope streams').                                             |
+---------+---------------------------------------------------------------------+
| 22      | Distance threshold from epicenter within which the relative         |
|         | difference between picked stations and envelope streams is          |
|         | evaluated (see line 24). Also shown is the number of picked         |
|         | stations and envelope streams within this distance threshold.       |
+---------+---------------------------------------------------------------------+
| 23      | Stations that were used for picking but not for the magnitude       |
|         | estimation.                                                         |
+---------+---------------------------------------------------------------------+
| 24      | 'Magnitude check' is the relative difference between the VS         |
|         | magnitude and the median of the single station magnitudes.          |
|         | 'Arrivals check' is the relative difference between the number of   |
|         | picked stations and the number of envelope streams available within |
|         | as certain epicentral distance. The 'Azimuthal gap' is the largest  |
|         | difference in azimuth between the picked stations.                  |
+---------+---------------------------------------------------------------------+
| 25      | The 'likelihood' is the product of the quality checks described     |
|         | above. See :ref:`ref_VS_LH` for more details                        |
+---------+---------------------------------------------------------------------+
| 26      | End of the log message for the event with the given event ID.       |
+---------+---------------------------------------------------------------------+

Logging envelope messages
-------------------------
The envelope messages received by scvsmag can optionally be written to the log-file
:file:`envelope-logging-info.log` by setting:

.. code-block:: sh

   vsmag.logenvelopes=true

The format of :file:`envelope-logging-info.log` is self-explanatory, note however
that the timestamp of the envelope value marks the start time of the 1 s waveform
window over which the envelope value was computed. Depending on the size of your
seismic network, :file:`envelope-logging-info.log` might quickly use a lot of disk
space.

.. _ref_VS_LH:

Computing the likelihood value
------------------------------
The likelihood is computed as follows:

.. code-block:: sh

   likelihood = Magnitude check * Arrivals check * Azimuthal Gap Check

If the magnitude check exceeds a magnitude dependent threshold its value is set
to 0.4, otherwise it is 1.0. The thresholds are defined as follows:

+-----------+-----------+
| Magnitude | Threshold |
+===========+===========+
| <1.5      | 0.5       |
+-----------+-----------+
| <2.0      | 0.4       |
+-----------+-----------+
| <2.5      | 0.3       |
+-----------+-----------+
| <3.0      | 0.25      |
+-----------+-----------+
| <4.0      | 0.2       |
+-----------+-----------+
| >4.0      | 0.18      |
+-----------+-----------+

If the arrivals check exceeds a value of 0.5 (i.e. more than half of the real-time
stations within a certain epicentral distance have not contributed picks to the
location) its value is set to 0.3, otherwise it is 1.0. The epicentral distance
threshold is the middle between the maximum and the average epicentral distance
of the stations contributing picks to the location.

The permissible azimuthal gap can be configured (default is 300). If it is
exceeded, 'Azimuthal Gap Check' is set to 0.2, otherwise it is set to 1.0.

A likelihood of 0.024, therefore, indicates, that all three quality checks failed.
If all quality checks succeeded the likelihood is set to 0.99.

scvsmag configuration
---------------------

scvsmag receives the amplitudes from :ref:`scenvelope` via the messaging system.
When the scenvelope is configured to send the amplitudes to the "VS" group
instead of "AMPLITUDE", the configuration must be adjusted.
In this case, replace the "AMPLITUDE" group with the "VS" message group in :confval:`connection.subscriptions`:

.. code:: sh

   connection.subscriptions = VS, EVENT, LOCATION, PICK

Consider also the remaining :ref:`configuration parameters <scvsmag_configuration>`.

scautoloc configuration
=======================

Because :ref:`scautoloc` was not designed with EEW in mind, there are a few
settings necessary to ensure that location estimates are sent to scvsmag as
quickly as possible:

.. code-block:: sh

   # If this string is non-empty, an amplitude obtained from an amplitude object
   # is used by ... . If this string is "mb", a period obtained from the amplitude
   # object is also used; if it has some other value, then 1 is used. If
   # this string is empty, then the amplitude is set to 0.5 * thresholdXXL, and 1
   # is used for the period.
   autoloc.amplTypeAbs = snr

   # This is the parameter "a" in the equation Δt = aN + b for the time interval
   # between origin updates.
   autoloc.publicationIntervalTimeSlope = 0

   # This is the parameter "b" in the above mentioned equation for the update
   # interval Δt.
   autoloc.publicationIntervalTimeIntercept = 0

   # Minimum number of phases.
   autoloc.minPhaseCount = 6

For :ref:`scautoloc` to provide locations with 6 stations, its grid configuration file 
requires to be setup with equal or lower minimum pick count, and with a corresponding 
maximum station distance to avoid false alerts.

For :ref:`scautopick` to provide snr amplitudes quickly requires the following 
setting:

.. code-block:: sh

   # The time window used to compute a maximum (snr) amplitude on the filtered
   # waveforms.
   thresholds.amplMaxTimeWindow = 1

scevent configuration
=====================

For :ref:`scevent` to create an event from an origin with 6 phases requires the
following setting:

.. code-block:: sh

   # Minimum number of Picks for an Origin that is automatic and cannot be
   # associated with an Event to be allowed to form an new Event.
   eventAssociation.minimumDefiningPhases = 6

:ref:`scautoloc` also has a so-called XXL feature that allows you to create a
location estimate with 4 P-wave detections (otherwise 6 is the minimum).
Although this feature is reserved for large magnitude events you can, in
principle, adapt the XXL thresholds to also locate moderate seismicity with the
first four picks. This may, however, lead to a larger number of false alerts
and it is, therefore, recommended to used this feature only as intended.

.. note::
   If scvsmag receives identical picks from different pipelines, the internal
   buffering fails. The missing picks are automatically retrieved from the
   database if necessary and if a connection to the database has been established.
   Alternatively, if picking is done on the same streams in several pipelines, they
   can be distinguished by modifying their respective public IDs.

References
==========

Borcherdt, R. D., 1994: Estimates of Site-Dependent Response Spectra for Design (Methodology and Justification), Earthquake Spectra

.. target-notes::

.. _`Virtual Seismologist` : http://www.seismo.ethz.ch/en/research-and-teaching/products-software/EEW/Virtual-Seismologist/
.. _`SED Public License for SeisComP Contributions` : http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt
