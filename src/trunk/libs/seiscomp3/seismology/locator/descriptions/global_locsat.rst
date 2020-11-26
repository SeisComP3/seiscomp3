The LOCSAT locator interface implements a wrapper for the LocSAT locator
by S.R. Bratt and W. Nagy [#Nagy]_ (according to the README file shipped with the
LocSAT distribution) referred to as **LOCSAT** in |scname|.


Travel-time tables
==================

|scname| ships with two predefined travel time tables: tab and iasp91.
LOCSAT travel time tables are located under :file:`share/locsat/tables/`.

The default profile is *iasp91*.


Travel-time interface
=====================

LOCSAT provides an interface for computing travel times based on coordinates and depth.

Use "LOCSAT" as a value for the travel-time interface when configurable, e.g. by
:ref:`global_fixedhypocenter`.


Configuration in |scname| modules
=================================

Use "LOCSAT" as a value for the locator type or interface when configurable, e.g. by
:ref:`screloc` or :ref:`solv`.


Station corrections
===================

LOCSAT does not support station corrections natively. At least checking
the code:

.. code-block:: c

   sta_cor[i]  = 0.0;    /* FIX !!!!!!*/


However the |scname| wrapper adds this feature. It allows to define a
:file:`.stacor` file which defines corrections of observation times
in seconds. A correction is **subtracted** (not added) from
the observation time to be compatible with the NonLinLoc station correction
definitions [#NLL]_.

Each LOCSAT profile (travel time table) can have an associated station
correction file. To use station corrections for the iasp91 tables, the file
:file:`$SEISCOMP_ROOT/share/locsat/tables/iasp91.stacor` needs to be created.

A station correction table takes the form:

.. code-block:: sh

   # LOCDELAY code phase numReadings delay
   LOCDELAY GE.MORC P 1 -0.1

with

- **code** (*string*) station code (after all alias evaluations)
- **phase** (*string*) phase type (any of the available travel time tables)
- **numReadings** (*integer*) number of residuals used to calculate mean residual/delay
  (not used by NLLoc, included for compatibility with the format of a summary,
  phase statistics file)
- **delay** (*float*) delay in seconds, subtracted from observed time

.. note::

   The fourth column (numReadings) is ignored and just provided for compatibility
   reasons with :ref:`NonLinLoc <global_nonlinloc>`.


References
==========

.. target-notes::

.. [#Nagy] S.R. Bratt and W. Nagy 1991). The LocSAT Program, Science Applications
   International Corporation (SAIC), San Diego.
.. [#NLL] NonLinLoc station corrections: http://alomax.free.fr/nlloc/soft3.03/control.html#_NLLoc_locdelay_
