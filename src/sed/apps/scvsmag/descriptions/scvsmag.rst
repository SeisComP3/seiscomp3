Part of the :ref:`VS` package.

scvsmag is part of a new SeisComp3 implementation of the
`Virtual Seismologist <http://www.seismo.ethz.ch/research/vs>`_
(VS) Earthquake Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released
under the `'SED Public License for SeisComP3 Contributions'
<http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt>`_. For a
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
   24 2013/06/28 10:51:01 [processing/info/VsMagnitude] Magnitude check: 0.027; Arrivals check: 0.000
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
|         | the the stream name, the wavetype of the contributing amplitude,    |
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
|         | If it exceeds a certain threshold the magnitude quality value is    |
|         | set to 0.4 otherwise to 1.0. 'Arrivals check' is the relative       |
|         | difference betweeen the number of picked stations and the number of |
|         | envelope streams contributing to the VS magnitude. If it exceeds a  |
|         | certain threshold the arrivals quality criteria is set to 0.3       |
|         | otherwise to 1.0. The full decision tree for computing the          |
|         | likelihood and the related thresholds is shown                      |
|         | :ref:`here <fig-VS-likelihood>`.                                    |
+---------+---------------------------------------------------------------------+
| 25      | The 'likelihood' is the product of the magnitude and the arrivals   |
|         | quality criteria. If both are 1.0 than the likelihoodis set to      |
|         | 0.99.                                                               |
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

References
==========

Borcherdt, R. D., 1994: Estimates of Site-Dependent Response Spectra for Design (Methodology and Justification), Earthquake Spectra

.. note::
   If scvsmag receives identical picks from different pipelines, the internal 
   buffering fails. The missing picks are automatically retrieved from the 
   database if necessary and if a connection to the database has been established. 
   Alternatively, if picking is done on the same streams in several pipelines they
   can be distinguished by modifying their respective public IDs.
