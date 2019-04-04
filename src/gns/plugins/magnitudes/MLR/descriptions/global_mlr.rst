Amplitude
---------
The MLr amplitude calculation is that of the Mlv amplitude computation.

Station Magnitude
-----------------

Magnitude based on SED MLh magnitude
The MLr plugin calculates the individual station local magnitude using the following formula:

.. math::

   mag = \log10(waampl) - \log10(waamplRef)

.. math::

   \log10(waamplRef)= 0.2869 - 1.272 \times 1^{-3} \times hypdistkm - 1.493 \times \log10(hypdistkm) + StationCorrection

waampl is the amplitude produced by the MLr plugin. Hypdistkm is the distance
from the sensor to the hypocenter in kilometers.
A(station) Station correction is given by  module.trunk.NZ.WEL.MLR.params, A.
Station Correction is set to be distance dependent:
Format: "UpToKilometers A ; UpToNextKilometers A ".
Option "nomag" disable the station magnitude.

* Amplitude unit in SeisComP3: **millimeter** (mm) from :ref:`MLv<global_mlv>`
* Time window: 150 s by :ref:`scautopick` or distance dependent
* Distance range: 0 - 20 deg
* Depth range: 0 - 800 km

Overall Event Magnitude
-----------------------

The GNS/Geonet Mlr local magnitude is using the default Sc3 behaviour for the automatic network magnitudes.
Hard coded range are 0-20 degrees maximum distance and 800 km maximum depth.

Configuration
-------------

Add the mlr plugin to the existing plugins in the global configuration.
Set the calibration parameters in the global bindings. Add MLr to the list of
magnitudes in the configuration of :ref:`scamp` and :ref:`scmag` for computation
and in :ref:`scesv` for visibility.
