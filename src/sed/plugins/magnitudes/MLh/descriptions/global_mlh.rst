Amplitude
---------

The MLh amplitude calculation is very similar to the original ML. The two differences are: 

- It uses the maximum of the two horizontal components (average can be configured if necessary) 
- It uses zero-to-peak in stead of peak-to-peak values 

Zero-to-peak is calculated by just dividing the peak-to-peak amplitude by two.
This is not exact for unsymmetrical signals, but that doesn't matter because the
code actually generates zero-to-peak amplitudes internally and multiplies them
with two. So in the end we get real zero-to-peak values.

Station Magnitude
-----------------

The MLh plugin calculates the individual station magnitude using the following formula: 

.. math::

   mag = \log10(waamp1) + A \times epdistkm + B

waampl is the amplitude produced by the MLh plugin. Epidistkm is the distance from the sensor
to the epicenter in kilometers. A and B are parameters that can be configured in a config file.
Several pairs of A and B can be configured for different ranges of epicenter distance.

Overall Event Magnitude
-----------------------

The SED standard is to use the median value of all contributing station magnitudes, no trimming.
