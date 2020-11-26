Amplitude
---------

The Ms_20 amplitudes are calculated on vertical-component displacement seismograms
corrected for the instrument response of a :term:`WWSSN_LP` seismograph.

Station Magnitude
-----------------

Ms_20 is the surface-wave magnitude at 20 s period based on the recommendations
by the IASPEI magnitude working group issued on 27 March, 2013.

.. math::

   M_s = \log \left(\frac{A}{T}\right) + 1.66 \log(\Delta) + 0.3

with

A: :term:`WWSSN_LP` corrected ground displacement in nm measured on the vertical-component
seismogram as the maximum absolute trace amplitude of a surface wave at periods between
18 s and 22 s,

T: period of the surface wave in seconds.

The term *Ms_20* is chosen in accordance with the IASPEI standard as of 2013.
Alternatively, the term :math:`M_s (BB)` may be used.

* Amplitude unit in SeisComP3: **nanometer** (nm)
* Time window: 0 s - distance (km) / 3.5 km/s + 30 s
* Period range: 18 s - 22 s, configurable: :confval:`Ms_20.lowerPeriod`, :confval:`Ms_20.upperPeriod`
* Default distance range: 2 - 160 deg, configurable: :confval:`Ms_20.minimumDistance`, :confval:`Ms_20.maximumDistance`
* Depth range: <= 100 km, configurable: :confval:`Ms_20.maximumDepth`

Network magnitude
-----------------

By default, the trimmed mean is calculated from the station magnitudes to form
the network magnitude. Outliers below the 25% and above the 75% percentiles are
removed before the calculation.

Configuration
-------------

Adjust the configurable parameters in global bindings in the Ms_20 section.
