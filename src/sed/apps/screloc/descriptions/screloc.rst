screloc is an automatic relocator that receives origins from realtime
locators such as scautoloc and relocates them with a configurable locator.
screloc can be conveniently used to test different locators and velocity models
or to relocate events with updated velocity models. Check the
:ref:`Example applications <screloc-example>` for screloc.

screloc processes any incoming automatic origin but does not yet listen to event
information nor does it skip origins for that a more recent one exists.

To run screloc along with all processing modules add it to the list of
clients in the seiscomp configuration frontend.


.. code-block:: sh

   seiscomp enable screloc
   seiscomp start screloc

Descriptions of parameters for screloc:

.. code-block:: sh

   seiscomp exec screloc -h

Test the performance of screloc and learn from debug output:

.. code-block:: sh

   seiscomp exec screloc --debug

Setup
=====

The following example configuration shows a setup of screloc for
:ref:`NonLinLoc <global_nonlinloc>`:

.. code-block:: sh

   plugins = ${plugins}, locnll

   # Define the locator algorithm to use
   reloc.locator = NonLinLoc

   # Define a suffix appended to the publicID of the origin to be relocated
   # to form the new publicID.
   # This helps to identify pairs of origins before and after relocation.
   # However, new publicIDs are unrelated to the time of creation.
   # If not defined, a new publicID will be generated automatically.
   reloc.originIDSuffix = "#relocated"

   ########################################################
   ################ NonLinLoc configuration################
   ########################################################
   NLLROOT = ${HOME}/nll/data

   NonLinLoc.outputPath = ${NLLROOT}/output/

   # Define the default control file if no profile specific
   # control file is defined.
   NonLinLoc.controlFile = ${NLLROOT}/NLL.default.conf

   # Set the default pick error in seconds passed to NonLinLoc
   # if no SC3 pick uncertainty is available.
   NonLinLoc.defaultPickError = 0.1

   # Define the available NonLinLoc location profiles. The order
   # implicitly defines the priority for overlapping regions
   #NonLinLoc.profiles = swiss_3d, swiss_1d, global
   NonLinLoc.profiles = swiss_3d, global

   # The earthModelID is copied to earthModelID attribute of the
   # resulting origin
   NonLinLoc.profile.swiss_1d.earthModelID = "swiss regional 1D"

   # Specify the velocity model table path as used by NonLinLoc
   NonLinLoc.profile.swiss_1d.tablePath = ${NLLROOT}/time_1d_regio/regio

   # Specify the region valid for this profile
   NonLinLoc.profile.swiss_1d.region = 41.2, 3.8, 50.1, 16.8

   # The NonLinLoc default control file to use for this profile
   NonLinLoc.profile.swiss_1d.controlFile = ${NLLROOT}/NLL.swiss_1d.conf

   # Configure the swiss_3d profile
   NonLinLoc.profile.swiss_3d.earthModelID = "swiss regional 3D"
   NonLinLoc.profile.swiss_3d.tablePath = ${NLLROOT}/time_3d/ch
   NonLinLoc.profile.swiss_3d.region = 45.15, 5.7, 48.3, 11.0
   NonLinLoc.profile.swiss_3d.controlFile = ${NLLROOT}/NLL.swiss_3d.conf

   # And the global profile
   NonLinLoc.profile.global.earthModelID = iaspei91
   NonLinLoc.profile.global.tablePath = ${NLLROOT}/iasp91/iasp91
   NonLinLoc.profile.global.controlFile = ${NLLROOT}/NLL.global.conf


.. _screloc-example:

Examples
========

* Run screloc to with a specific velocity model given in a profile by :ref:`NonLinLoc <global_nonlinloc>`.
  Use a specific userID and authorID for uniquely recognizing the relocation.
  Changing the priority in :ref:`scevent` before running the example, e.g. to
  TIME_AUTOMATIC, sets the latest origin (which will be created by screloc) to preferred.

  .. code-block:: sh

    # set specific velocity profile defined for NonLinLoc
    profile=<your_profile>
    # set userID
    userID="<your_user>"
    # set authorID
    authorID="<screloc>"

    for i in `scevtls -d mysql://sysop:sysop@localhost/seiscomp3 --begin '2015-01-01 00:00:00' --end '2015-02-01 00:00:00'`; do

        orgID=`echo "select preferredOriginID from Event,PublicObject where Event._oid=PublicObject._oid and PublicObject.publicID='$i'" |\
        mysql -u sysop -p sysop -D seiscomp3 -h localhost -N`

        screloc -O $orgID -d localhost --locator NonLinLoc --profile $profile -u $userID --debug --author=$authorID

    done
