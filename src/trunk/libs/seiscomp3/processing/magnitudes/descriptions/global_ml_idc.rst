Amplitude
---------

The SNSNR amplitudes are calculated on the vertical component seismograms.

Station Magnitude
-----------------

.. math::

   mag = \log10(A) + B(\Delta)

with

A: amplitude of type SBSNR

B: attenuation correction function of epicentral distance in km


The default corrections are read from a file installed at
:file:`@DATADIR@/magnitudes/IDC/global.ml`. If that file is not present no magnitude
will be calculated.

Station corrections
-------------------

Station magnitudes can be computed with a station specific correction table
which is configured in the global bindings. The parameter :confval:`magnitudes.ML(IDC).A`
takes a path and allows to use placeholders for network code (:code:`{net}`),
station code (:code:`{sta}`) and location code (:code:`{loc}`).

Example:

.. code::

   magnitudes.ML(IDC).A = @DATADIR@/magnitudes/IDC/{net}.{sta}.ml


* Amplitude unit in SeisComP3: **nanometer** (nm)
* Time window: 4.5 s
* Default distance range: 0 - 20 deg
* Depth range: 0 - 40 km
