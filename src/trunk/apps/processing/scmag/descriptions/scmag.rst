The purpose of scmag is to compute magnitudes. It takes amplitudes and origins
as input and produces StationMagnitudes and Magnitudes as output.
The resulting magnitudes are sent to the "MAGNITUDE" group. scmag doesn’t access
any waveforms. It only uses amplitudes previously calculated, e.g. by :ref:`scamp`.
The purpose of scmag is the decoupling of magnitude computation from amplitude
measurements. This allows several modules to generate amplitudes concurrently,
like scautopick and scamp. As soon as an origin comes in, the amplitudes related
to the picks are taken either from the memory buffer or the database to compute
the magnitudes. Currently the following magnitude types are implemented:

MLv
   Local magnitude calculated on the vertical component using a correction term
   to fit with the standard ML

MLh
   Local magnitude calculated on the horizontal components to SED specifications.

mb
   Narrow band body wave magnitude using a third order Butterworth filter with
   corner frequencies of 0.7 and 2.0 Hz.

mB
   Broad band body wave magnitude.

Mwp
   The body wave magnitude of Tsuboi et al. (1995)

Additionally, scmag computes the following derived magnitudes: 

Mw(mB)
   Estimation of the moment magnitude Mw based on mB using the Mw vs. mB
   regression of Bormann and Saul (2008)

Mw(Mwp)
   Estimation of the moment magnitude Mw based on Mwp using the Mw vs. Mwp
   regression of Whitmore et al. (2002).

M(summary)
   Summary magnitude, which consists of a weighted average of the individual
   magnitudes and attempts to be a best possible compromise between all magnitudes.
   See below for configuration and also scevent for how to add the summary magnitude
   to the list of possible preferred magnitudes or how to make it always preferred.

Mw(avg)
   Estimation of the moment magnitude Mw based on a weighted average of other
   magnitudes, currently MLv, mb and Mw(mB), in future possibly other magnitudes as
   well, especially those suitable for very large events. The purpose of Mw(avg) is
   to have, at any stage during the processing, a “best possible” estimation of the
   magnitude by combining all available magnitudes into a single, weighted average.
   Initially the average will consist of only MLv and/or mb measurements, but as soon
   as Mw(mB) measurements become available, these (and in future other large-event
   magnitudes) become progressively more weight in the average.


Summary magnitude
=================

scmag can compute a summary magnitude which is a weighted sum of all available
magnitudes. This magnitude is called **M** and is computed as follows:

.. math::

   M = \frac{\sum w_{i} M_{i}}{\sum w_i}

   w_{i} = a_i stacount(M_{i}) + b_i

The coefficients a and b can be configured per magnitude type. Furthermore each
magnitude type can be included or excluded from the summary magnitude calculation.
