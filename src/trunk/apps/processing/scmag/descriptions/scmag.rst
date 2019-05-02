The purpose of scmag is to compute magnitudes from pre-computed amplitudes.
Instead it takes amplitudes and origins as input and produces StationMagnitudes
and (network) Magnitudes as output. It does not access waveforms.
The resulting magnitudes are sent to the "MAGNITUDE" group. scmag doesn’t access
any waveforms. It only uses amplitudes previously calculated.

The purpose of scmag is the decoupling of magnitude computation from amplitude
measurements. This allows several modules to generate amplitudes concurrently,
like :ref:`scautopick` or :ref:`scamp`. As soon as an origin comes in, the amplitudes related
to the picks are taken either from the memory buffer or the database to compute
the magnitudes.

Primary magnitudes
------------------

Currently the following magnitude types are implemented:

:term:`MLh <magnitude, local (ML)>`
   Local magnitude calculated on the vertical component using a correction term
   to fit with the standard ML

:term:`MLv <magnitude, local vertical (MLv)>`
   Local magnitude calculated on the vertical component using a correction term
   to fit with the standard ML

:term:`MLh <magnitude, local horizontal (MLh)>`
   Local magnitude calculated on the horizontal components to SED specifications.

:term:`MLr <magnitude, local GNS/GEONET (MLr)>`
   Local magnitude calculated from MLv amplitudes based on GNS/GEONET specifications
   for New Zealand.

:term:`MN <magnitude, Nuttli (MN)>`
   Canadian Nuttli magnitude.

:term:`mb <magnitude, body-wave (mb)>`
   Narrow band body wave magnitude measured on a WWSSN-SP filtered trace

:term:`mB <magnitude, broadband body-wave (mB)>`
   Broad band body wave magnitude

:term:`Mwp <magnitude, broadband P-wave moment (Mwp)>`
   The body wave magnitude of Tsuboi et al. (1995)

:term:`Mjma <magnitude, JMA (M_JMA)>`
   Mjma is computed on displacement data using body waves of period < 30s

:term:`Ms(BB) <magnitude, surface wave (Ms)>`
   Broad band surface-wave magnitude

:term:`Md <magnitude, duration (Md)>`
   Duration magnitude as described in https://earthquake.usgs.gov/research/software/#HYPOINVERSE

Derived magnitudes
------------------

Additionally, scmag derives the following magnitudes from primary magnitudes:

:term:`Mw(mB) <magnitude, derived mB (Mw(mB))>`
   Estimation of the moment magnitude Mw based on mB using the Mw vs. mB
   regression of Bormann and Saul (2008)

:term:`Mw(Mwp) <magnitude, derived Mwp (Mw(Mwp))>`
   Estimation of the moment magnitude Mw based on Mwp using the Mw vs. Mwp
   regression of Whitmore et al. (2002)

:term:`M(summary)`
   Summary magnitude, which consists of a weighted average of the individual
   magnitudes and attempts to be a best possible compromise between all magnitudes.
   See below for configuration and also scevent for how to add the summary magnitude
   to the list of possible preferred magnitudes or how to make it always preferred.

:term:`Mw(avg)`
   Estimation of the moment magnitude Mw based on a weighted average of other
   magnitudes, currently MLv, mb and Mw(mB), in future possibly other magnitudes as
   well, especially those suitable for very large events. The purpose of Mw(avg) is
   to have, at any stage during the processing, a “best possible” estimation of the
   magnitude by combining all available magnitudes into a single, weighted average.
   Initially the average will consist of only MLv and/or mb measurements, but as soon
   as Mw(mB) measurements become available, these (and in future other large-event
   magnitudes) become progressively more weight in the average.

If an amplitude is updated, the corresponding magnitude is updated as well.
This allows the computation of preliminary, real-time magnitudes even before
the full length of the P coda is available.


Relationship between amplitudes and origins
===========================================

scmag makes use of the fact that origins sent by scautoloc and scolv include
the complete set of arrivals, which reference picks used for origin computation.
The picks in turn are referenced by a number of amplitudes, some of which are
relevant for magnitude computation.


Summary magnitude
=================

scmag can compute a summary magnitude which is a weighted sum of all available
magnitudes. This magnitude is called **M** and is computed as follows:

.. math::

   M = \frac{\sum w_{i} M_{i}}{\sum w_i}

   w_{i} = a_i stacount(M_{i}) + b_i

The coefficients a and b can be configured per magnitude type. Furthermore each
magnitude type can be included or excluded from the summary magnitude calculation.
