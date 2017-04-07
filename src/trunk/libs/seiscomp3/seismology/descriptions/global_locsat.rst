The LocSAT locator interface implements a wrapper for the LocSAT locator
written by Walter Nagy (according to the README file shipped with the
LocSAT distribution).


Travel time tables
==================

SeisComP ships with two predefined travel time tables: tab and iasp91.
LocSAT travel time tables are located under :file:`share/locsat/tables/`.

The default profile is *iasp91*.


Station corrections
===================

LocSAT does not support station corrections natively. At least checking
the code:

.. code-block:: c

   sta_cor[i]  = 0.0;    /* FIX !!!!!!*/


However the SeisComP wrapper adds this feature. It allows to define a
:file:`.stacor` file which defines corrections of observation times
in seconds. A correction is **subtracted** (not added) from
the observation time to be compatible with the NonLinLoc station correction
definitions (http://alomax.free.fr/nlloc/soft3.03/control.html#_NLLoc_locdelay_).

Each LocSAT profile (travel time table) can have an associated station
correction file. To use station corrections for the iasp91 tables, the file
:file:`share/locsat/tables/iasp91.stacor` needs to be created.

A station correction table looks like this:

.. code-block:: sh

   # LOCDELAY code phase numReadings delay
   LOCDELAY GE.MORC P 1 -0.1

The fourth column (numReadings) is ignored and just provided for compatibility
reasons with NonLinLoc.

Format
------

- **code** (*string*) station code (after all alias evaluations)
- **phase** (*string*) phase type (any of the available travel time tables)
- **numReadings** (*integer*) number of residuals used to calculate mean residual/delay (not used by NLLoc, included for compatibility with the format of a summary, phase statistics file)
- **delay** (*float*) delay in seconds, subtracted from observed time
