Amplitude
---------
The MLr amplitude calculation is that of the Mlv amplitude computation

Station Magnitude
-----------------

The MLr plugin calculates the individual station local magnitude using the following formula: 

.. math::

   //based on sed MLh magnitude: mag = \log10(waamp1) + A \times hypdistkm + B
   mag = log10(waampl)-log10(waamplRef)
   log10(waamplRef)= 0.2869 - 1.272*1e-3*(hypdistkm) -(1.493 * log10 (hypdistkm)) + (StationCorrection) 
   // A(station) Station correction - given by  module.trunk.NZ.WEL.MLR.params, A ; 
   // Sation Correction is set to be distance dependent; 
   // Option nomag disable the station magnitude.


waampl is the amplitude produced by the MLr plugin. Hypdistkm is the distance
from the sensor to the hypocenter in kilometers.

Overall Event Magnitude
-----------------------

The GNS/Geonet Mlr local magnitude is using the default Sc3 behaviour for the automatic network magnitudes.
Hard coded range are 0-20 degrees maximum distance, and 800 km max depth.
