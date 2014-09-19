Part of the :ref:`VS` package.

scenvelope is part of a new SeisComp3 implementation of the
`Virtual Seismologist <http://www.seismo.ethz.ch/research/vs>`_ (VS) Earthquake
Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released
under the `'SED Public License for SeisComP3 Contributions' 
<http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt>`_. It generates
real-time envelope values for horizontal and vertical acceleration, velocity and
displacement from raw acceleration and velocity waveforms. It was implemented
to handle the waveform pre-processing necessary for the :ref:`scvsmag` module.
It provides in effect continuous real-time streams of PGA, PGV and PGD values which
could also be used independently of :ref:`scvsmag`.

The processing procedure is as follows:
1. gain correction
2. baseline correction
3. high-pass filter with a corner frequency of 3 s period
4. integration or differentiation to velocity, acceleration and displacement
5. computation of the absolute value within 1 s intervals

The resulting envelope values are sent as messages to :ref:`scmaster`. Depending
on the number of streams that are processed this can result in a significant
number of messages (#streams/s).

scenvelope sends messages of type "VS" which requires all modules receiving these
messages to load the plugin "dmvs". This can be most easily configured through 
the configuration parameter in :file:`global.cfg`:

.. code-block:: sh

   plugins = dmvs

By default scenvelope sends messages to the group "VS".