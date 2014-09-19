screloc is an automatic relocator that receives origins from realtime
locators such as scautoloc and relocates them with a configurable locator.
To run screloc along with all processing modules add it to the list of
clients in the seiscomp configuration frontend.

screloc does not yet listen to event information and does not skip origins for
that a more recent one exists. Instead any incoming automatic origin is
processed.

The following example configuration show how to setup screloc for
:ref:`NonLinLoc <global_nonlinloc>`:

.. code-block:: sh

   plugins = ${plugins}, locnll

   # Define the locator algorithm to use
   reloc.locator = NonLinLoc

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

