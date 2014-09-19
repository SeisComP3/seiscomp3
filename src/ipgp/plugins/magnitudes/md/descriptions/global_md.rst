The duration magnitude is based on coda duration measurement.
It's usually valid for small earthquakes up to magnitude 4 to 5.

First used in 1972 by Lee et al., Duration magnitude (Md) or Coda duration
magnitude plugin estimates Richter magnitude of local earthquakes by using
signal duration on vertical components of seismographs.

Estimations are quite stable for local earthquakes ranging from magnitude
Md 0.0 to 5.0.


Amplitude processing
--------------------

Duration magnitude is usually computed on short period seismometers by searching
the time at which the amplitude of the signal is close to pre-earthquake amplitude.

Since it's mainly used for small earthquake whose signal is at rather high frequency,
it's usefull to highpass filter broadband seismomters (select sismo type 6 and a 
Butterworth filter "3,1.5").
Or a better solution is to deconvolve the signals and reconvolve with a widely used
short-period instrument : the 1Hz eigen-frequency L4C (select sismo type 9).
If you have the full responses in your inventory and have activated them 
(amplitudes.enableResponses set to true), you will be able to use also accelerometers.

The plugin then searches for the maximum amplitude of the signal, which should be
the S-wave and then computed mean amplitude of one-second time windows.
As soon as a one-second time window mean amplitude vs pre-earthquake amplitude
reaches the configured SNR ratio, the process is stopped.
The middle of the one-second time window is assumed to be the end of the Coda and
the time difference between Coda time and P arrival time is stored as Coda duration.



Magnitude processing
--------------------

Once amplitudes calculated by the AmplitudeProcessor and a Coda has been found,
the generic formula is applied and the duration magnitude is computed 
for a given station, if it fits the criteria (max depth, max distance).

.. math::

   mag = FMA + FMB \times \log10(period) + (FMF \times period) + (FMD \times epidistkm) + (FMZ \times depth) + STACOR 


Plugin
======

The Coda duration magnitude plugin (Md) is installed under :file:`share/plugins/md.so`.
It provides a new implementations of AmplitudeProcessor and MagnitudeProcessor.

To add the plugin to a module add it to the modules configuration, either
:file:`modulename.cfg` or :file:`global.cfg`:

.. code-block:: sh

   plugins = ${plugins}, md

Basically it can be used by modules: :ref:`scamp`, :ref:`scmag`, :ref:`scolv`.


More information
----------------

Description of the formula can be found in Hypo2000 manual from USGS website.
`<http://earthquake.usgs.gov/research/software/#HYPOINVERSE>`_
