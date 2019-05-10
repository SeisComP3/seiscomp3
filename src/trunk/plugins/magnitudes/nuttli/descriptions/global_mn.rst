
The amplitude unit in SeisComP3 is **meter/second** (m/s).

Settings
--------

Add the *nuttli* plugin to the list of loaded plugins and set the region-specific
calibration parameters in the global configuration. E.g. in the global module
configuration:

.. code-block:: sh

   plugins = ${plugins},nuttli

Amplitude time window parameters and magnitude corrections are configurable in the
global bindings.

scamp
~~~~~

Add the Nuttli magnitude type to the range of magnitudes for which the amplitudes are
to be calculated, e.g.:

.. code-block:: sh

   amplitudes = ML,MLv,mb,mB,AMN

.. note::

   Provide *AMN* for computing Nuttli-type amplitudes.

scmag
~~~~~

Add the Nuttli magnitude type to the range of magnitudes to be calculated, e.g.:

.. code-block:: sh

   magnitudes = ML,MLv,mb,mB,MN
