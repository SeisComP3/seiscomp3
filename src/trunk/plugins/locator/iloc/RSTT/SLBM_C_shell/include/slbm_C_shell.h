//- ****************************************************************************
//-
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
//- certain rights in this software.
//-
//- BSD Open Source License.
//- All rights reserved.
//-
//- Redistribution and use in source and binary forms, with or without
//- modification, are permitted provided that the following conditions are met:
//-
//-    * Redistributions of source code must retain the above copyright notice,
//-      this list of conditions and the following disclaimer.
//-    * Redistributions in binary form must reproduce the above copyright
//-      notice, this list of conditions and the following disclaimer in the
//-      documentation and/or other materials provided with the distribution.
//-    * Neither the name of Sandia National Laboratories nor the names of its
//-      contributors may be used to endorse or promote products derived from
//-      this software without specific prior written permission.
//-
//- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//- POSSIBILITY OF SUCH DAMAGE.
//-

//! \file
//!
//! FILE slbm_C_shell.h
//!
//! DESCRIPTION<br>
//! Proto types for the SLBM C Shell Interface
//!
//!
//! A "C" Interface to the SLBM C++ library, providing access to all supported functionality.
//!
//! The slbm_C_shell Interface (C++) manages a connection to the C++ SLBM library and
//! provides C compatible
//! functions. To make this work, 4 conditions must be met:
//! <br>1) The C++ shared objects libslbm.so and libslbmCshell.so must both be accessible
//! via the LD_LIBRARY_PATH.
//! <br>2) The C application must be linked to libslbm.so and libslbmCshell.so
//! <br>3) The C application must execute the command: slbm_shell_create();
//! <br>4) The C application must execute the command: slbm_shell_loadVelocityModelBinary();
//!
//!If all of these conditions are successfully met, then the the C application can use
//! the slbm_C_shell interface through the shared object library libslbmCshell.so to
//! access all the methods described in this document.
//!
//!Almost all functions in the C interface return an integer error code that will be 0 if the function
//!completed successfully or a positive error code if the function generated some sort of error.
//!Applications should check the value of the returned error code after each function call.
//!If the returned error code is  > 0 then function slbm_shell_getErrorMessage()
//!can be called to retrieve an errorMessage that provides information about the error.  Some of the most
//!important error codes are listed in the followng table.
//!
//! <table border="2" cellspacing = "0" cellpadding="5">
//! <tr><td><b>Code</b></td><td><b>Function</b></td><td><b>Message</b></td></tr>
//! <tr><td>106</td><td>Grid::reaDataBuffererFromFile</td><td>Could not open velocity model file.</td></tr>
//! <tr><td>200</td><td>GreatCircle_Xg constructor</td><td>Pg/Lg not valid because source or receiver is below the Moho.</td></tr>
//! <tr><td>201</td><td>GreatCircle_Xn::computeTravelTime</td><td>Source-receiver separation exceeds maximum value.</td></tr>
//! <tr><td>202</td><td>GreatCircle_Xn::computeTravelTime</td><td>Source depth exceeds maximum value.</td></tr>
//! <tr><td>203</td><td>GreatCircle_Xn::computeTravelTimeCrust</td><td>Horizontal offset below the source plus
//! horizontal offset below the receiver is greater than the source-receiver separation</td></tr>
//! <tr><td>300</td><td>GreatCircle_Xn::computeTravelTimeCrust</td><td>nIterations == 10000</td></tr>
//! <tr><td>301</td><td>GreatCircle_Xn::computeTravelTimeCrust</td><td>c*H is greater than ch_max.</td></tr>
//! <tr><td>302</td><td>GreatCircle_Xn::computeTravelTimeMantle</td><td>Could not converge on stable ray parameter.</td></tr>
//! <tr><td>303</td><td>GreatCircle_Xn::computeTravelTimeMantle</td><td>search for minimum H failed.</td></tr>
//! <tr><td>304</td><td>GreatCircle_Xn::computeTravelTimeMantle</td><td>c*H > ch_max.</td></tr>
//! <tr><td>305</td><td>GreatCircle_Xn::brent</td><td>Too many iterations.</td></tr>
//! <tr><td>400</td><td>GreatCircle_Xg::computeTravelTime</td><td>computeTravelTimeTaup() and computeTravelTimeHeadwave() both returned NA_VALUE.</td></tr>
//! <tr><td>401</td><td>GreatCircle_Xn::computeTravelTime</td><td>Receiver depth below Moho is illegal.</td></tr>
//! <tr><td>402</td><td>GreatCircle_Xn::computeTravelTimeCrust</td><td>Source is too close to the receiver.</td></tr>
//! </table>
//!
//!The libslbmCshell.so maintains a C++ Grid object, which manages interaction with
//! the Earth model. The Earth model is loaded by calling the loadVelocityModel(String)
//! method, described below. This Grid object remains in memory until deleted with the
//! command slbm_shell_delete(), or a different Earth model is loaded with another
//! call to loadVelocityModel(String).
//!
//!libslbmCshell.so also maintains a single instance of a C++ GreatCircle object which is
//! instantiated with a call to createGreatCircle(). Once instantiated, many slbm_C_shell
//! methods can retrieve information from this GreatCircle object, such as getTravelTime(),
//! getWeights(), toString(int), and more. Once instantiated, the GreatCircle can be
//! interrogated until it is replaced with another GreatCircle by a subsequent call
//! to createGreatCircle(), or is deleted by clear().
//!
//! The Grid object stores a vector of map objects which associates the phase and Location of
//! a CrustalProfile object with a pointer to the instance of the CrustalProfile. When
//! createGreatCircle() is called with a latitude, longitude and depth which has been
//! used before, the Grid object will return a pointer to the existing CrustalProfile
//! object, thereby enhancing performance. This vector of maps is cleared when clear()
//! is called. The implications of all this is that applications that loop over many
//! calls to createGreatCircle() will see a performance improvement if clear() is
//! not called within the loop. However, for problems where the number of sources
//! and/or receivers is so large that memory becomes an issue, applications could
//! call clear() within the loop to save memory.
//!
//! All calculations assume the Earth is defined by a GRS80 ellipsoid.
//! For a description of how points along a great circle are calculated see
//! <A href="../../../doc/geovectors.pdf">geovectors.pdf</A>
//!
//!Company: Sandia National Laboratories
//!
//!Author: Sandy Ballard


#ifndef SLBM_C_SHELL_H
#define SLBM_C_SHELL_H

#if defined(_WIN32) || defined(WIN32)

	#ifndef SLBM_C_EXPORT
	#define SLBM_LIB __declspec(dllimport)
	#else
	#define SLBM_LIB __declspec(dllexport)
	#endif

#else

	#define SLBM_LIB
	
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//=================================================================================================
//                                   ****NOTICE****
//
// The "C" shell implementation of the SLBM package has changed due to a package reorganization.
//	The file "SLBMGlobals.h" has now been wrapped into a namespace, it is no longer possible to #include
// "SLBMGlobals.h" and compile in "C". The solution, so as not to require any changes to code already
// developed, is to hard-code the constants from "SLBMGlobals.h" into this header file. While it does
// satisfy that no changes are required to existing code, it is a liability if any of those constants
// do change. In the event (unlikely) the constants do change, the change will need to be made in two
// places.
//

//static const char* SlbmVersion = "3.0.0";

//! \brief Convenience constant for P wave index.
//!
//! Convenience constant for P wave index.218
static const int PWAVE = 0;

//! \brief Convenience constant for S wave index
//!
//! Convenience constant for S wave index
static const int SWAVE = 1;

//! \brief Convenience constant for Pn phase
//!
//! Convenience constant for Pn phase
static const int Pn = 0;

//! \brief Convenience constant for Sn phase
//!
//! Convenience constant for Sn phase
static const int Sn = 1;

//! \brief Convenience constant for Pg phase
//!
//! Convenience constant for Pg phase
static const int Pg = 2;

//! \brief Convenience constant for Lg phase
//!
//! Convenience constant for Lg phase
static const int Lg = 3;

#define WATER          0
#define SEDIMENT1      1
#define SEDIMENT2      2
#define SEDIMENT3      3
#define UPPER_CRUST    4
#define MIDDLE_CRUST_N 5
#define MIDDLE_CRUST_G 6
#define LOWER_CRUST    7
#define MANTLE         8
#define NLAYERS        9
// MANTLE has to be last layer and must == NLAYERS-1

// Parameter TOP_LAYER controls behavior when sources/receivers
// are in layer WATER.
// If TOP_LAYER = 0, and phase = Pn/Pg then rays that travel
// through the WATER layer travel at the velocity of water.
// If TOP_LAYER = 0, and phase = Sn/Lg then rays that travel
// through the WATER layer cause an exception to be thrown.
// If TOP_LAYER = 1 and the depth of a source or receiver is
// less than the depth of the top of layer SEDIMENT1, then
// the code will search downward through the layers, starting
// at layer SEDIMENT1, until if finds a layer with non-zero
// thickness.  It will then extend the top of that layer up
// to the depth of the source/receiver.  This essentially
// assumes that all sources/receivers in the ocean are actually
// located on islands that are too small to have been explicitly
// defined in the model.
//
// Bottom line is: to include water set TOP_LAYER = 0.
// To ignore water, set TOP_LAYER = 1;
static const int TOP_LAYER = 1;

//! \brief Retrieve the SLBM Version number.
//!
//! Retrieve the SLBM Version number.
//!	@param str a char pointer to contain the version number. NOTE: user should
//! allocate storage for approximately 10 chars.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getVersion ( char* str );

	//! \brief Retrieve last error message.
//!
//! Retrieves last error message. This method should be called
//! following any function call that returns a "1" indicating an error
//! has occurred. The string is populated with the text obtained from the
//! SLBMException::emessage attribute.
//!	@param str a char pointer to contain the err message. NOTE: user should
//! allocate storage for approximately 500 chars.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getErrorMessage ( char* str );

//! \brief Instantiate a SLBM Interface object.
//!
//! Instantiate a SLBM Interface object.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_create ();

//! \brief Instantiate a SLBM Interface object with fixed earth radius.
//!
//! Instantiate a SLBM Interface object fixing the earth radius to the double value
//! passed in argument list.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_create_fixedEarthRadius ( double* radius );

//! \brief Deletes the SlbmInterface object instantiated with the call
//! slbm_shell_create().
//!
//! Deletes the SlbmInterface object instaniated with the call
//! slbm_shell_create().
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_delete ();

//! \brief Load the velocity model into memory from
//! the specified file or directory.  This method
//! automatically determines the format of the model.
//!
//! Load the velocity model into memory from the specified
//! file or directory.  This method
//! automatically determines the format of the model and
//! hence is able to load all model formats.
//!
//! <p>This method is deprecated in SLBM versions 3 and higher
//! and is provided only for backward compatibility
//! with previous versions of SLBM.  It simply calls
//! loadVelocityModel(modelPath).
//!
//! @param modelDirectory the path to the file or directory
//! that contains the model.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_loadVelocityModelBinary ( const char* modelDirectory );

//! \brief Deprecated.  Use saveVelocityModel() instead.
//! Specify the directory into which the model that is
//! currently in memory should be written the next time
//! that saveVelocityModelBinary() is called.
//!
//! This method is deprecated and is provided only for backward
//! compatibility with previous versions of SLBM.  Use method
//! saveVelocityModel() instead.
//! <p>Specify the directory into which the model that is
//! currently in memory should be written the next time
//! that saveVelocityModelBinary() is called.  The model
//! will be written in format 3, which is the SLBM version 3
//! binary directory format (GeoTess).
//! @param directoryName the name of the directory where model
//! files are to be written.  The directory will be
//! created if it does not exist.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_specifyOutputDirectory ( const char* directoryName );

//! \brief Deprecated.  Use saveVelocityModel() instead.
//! Write the model in format 3 to directory
//! previously specified with a call to specifyOutputDirectory().
//!
//! This method is deprecated and is provided only for backward
//! compatibility with previous versions of SLBM.  Use method
//! saveVelocityModel() instead.
//! <p>The model is written in format 3 to the directory previously
//! specified with a call to specifyOutputDirectory().
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_saveVelocityModelBinary ();

//! \brief Load the velocity model into memory from
//! the specified file or directory.  This method
//! automatically determines the format of the model.
//!
//! Load the velocity model into memory from the specified
//! file or directory.  This method
//! automatically determines the format of the model and
//! hence is able to load all model formats.
//!
//! @param modelPath the path to the file or directory
//! that contains the model.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_loadVelocityModel ( const char* modelPath );

//! \brief Save the velocity model currently in memory to
//! the specified file or directory.
//!
//! Save the velocity model currently in memory to
//! the specified file or directory.
//!
//! <p>The following formats are supported:
//! <ol>
//! <li>SLBM version 1 ascii file.  All model information is
//! output to a single file in ascii format.
//! This format was available in SLBM version 2, but never used.
//!
//! <li>SLBM version 2 directory format. Model information is
//! output to a number of different files and directories, mostly
//! in binary format.  This format was used almost exclusively in
//! SLBM version 2.
//!
//! <li>SLBM version 3 directory format. Model information is
//! output to a number of different files and directories, mostly
//! in binary format.  This format is very similar to format 2
//! with the difference being that the model tessellation and values
//! are stored in GeoTess format instead of the custom SLBM format.
//!
//! <li>SLBM version 3 single-file format.  This is the default+preferred
//! format.  All model information is written to a single file.
//! If the modelFileName extension is '.ascii' the file is written
//! in ascii format, otherwise it is written in binary format.
//! </ol>
//!
//! See SLBM_Design.pdf in the main documentation directory
//! for detailed information about model output formats.
//!
//! <p>Models stored in SLBM version 1 and 2 formats (formats 1 and 2)
//! only support linear interpolation.  Models stored in SLBM
//! version 3 formats (formats 3 and 4) support both linear
//! and natural neighbor interpolation.
//!
//! @param modelFileName the full or relative path to the
//! file or directory to which the earth model is to
//! be written.  For formats 2 and 3, the directory will be
//! created if it does not exist.
//! @param format the desired format of the output.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_saveVelocityModelFormat ( const char* modelFileName, int format );

//! \brief Save the velocity model currently in memory to the specified file.
//!
//! Save the velocity model currently in memory to
//! the specified file.  Attempting to save the model
//! to the same file from which the model was originally
//! read will result in an exception.  If the file
//! already exists, it will be overwritten without
//! warning or backup.
//! @param modelFileName the full or relative path plus
//! file name of the file to which the earth model is to
//! be written.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_saveVelocityModel ( const char* modelFileName );

//! \brief Instantiate a new GreatCircle object between
//! two locations.
//!
//! Instantiate a new GreatCircle object between
//! two locations.
//! @param phase the phase that this GreatCircle is to
//! support.  Recognized phases are Pn, Sn, Pg and Lg.
//! @param sourceLat the geographic latitude of the source
//! in radians.
//! @param sourceLon the longitude of source in radians.
//! @param sourceDepth the depth of the source in km.
//! @param receiverLat the geographic latitude of the receiver
//! in radians.
//! @param receiverLon the longitude of the receiver in radians.
//! @param receiverDepth the depth of the receiver in km.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_createGreatCircle ( char* phase, double* sourceLat, double* sourceLon, double* sourceDepth,
									double* receiverLat, double* receiverLon, double* receiverDepth );
//! \brief Returns true if the current GreatCirlce object has
//! been instantiated and is ready to be interrogated.
//!
//! Returns true if the current GreatCirlce object has
//! been instantiated and is ready to be interrogated.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_isValid ();

//! \brief Delete the current GreatCircle object from memory and
//! clear the pool of stored CrustalProfile objects.
//! The model Grid is not deleted and remains accessible.
//!
//! Delete the current GreatCircle object from memory and
//! clear the pool of stored CrustalProfile objects.
//! The model Grid is not deleted and remains accessible.
//!
//! The Grid object owned by SlbmInterface stores a vector of map objects which associates
//! the phase and Location of a CrustalProfile object with a pointer to the instance
//! of the CrustalProfile.  When createGreatCircle() is called with a latitude,
//! longitude and depth which has been used before, the Grid object will return a
//! pointer to the existing CrustalProfile object, thereby enhancing performance.
//! This vector of maps is cleared when SlbmInterface::clear() is called.  The
//! implications of all this is that applications that loop over many calls to
//! createGreatCircle() will see a performance improvement if clear() is not called
//! within the loop.  However, for problems with a huge number of sources and or receivers,
//! if memory becomes an issue, applications could call clear() within the loop
//! to save memory.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_clear ();

//! \brief Retrieve the source-receiver separation, in radians.
//!
//! Retrieve the source-receiver separation, in radians.
//! @param dist the source-receiver separation is returned
//! in distance.  If the GreatCircle is invalid, distance
//! will equal BaseObject::NA_VALUE.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getDistance ( double* dist );

//! \brief Retrieve horizontal offset below the source, in radians.
//!
//! Retrieve horizontal offset below the source, in radians.
//! This is the angular distance between the location of the
//! source and the source pierce point where the ray impinged
//! on the headwave interface.
//! @param dist the horizontal offset below the source, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getSourceDistance ( double* dist );

//! \brief Retrieve horizontal offset below the receiver, in radians.
//!
//! Retrieve horizontal offset below the receiver, in radians.
//! This is the angular distance between the location of the
//! receiver and the receiver pierce point where the ray impinged
//! on the headwave interface.
//! @param dist the horizontal offset below the receiver, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getReceiverDistance ( double* dist );

//! \brief Retrieve angular distance traveled by the ray
//! below the headwave interface, in radians.
//!
//! Retrieve the angular distance traveled by the ray
//! below the headwave interface, in radians.
//! This is the total distance minus the horizontal offsets
//! below the source and receiver.  getSourceDistance() +
//! getReceiverDistance() + getHeadwaveDistance() =
//! getDistance().
//! @param dist the angular distance traveled by the ray
//! below the headwave interface, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getHeadwaveDistance ( double* dist );

//! \brief Retrieve horizontal distance traveled by the ray
//! below the headwave interface, in radians.
//!
//! Retrieve horizontal distance traveled by the ray
//! below the headwave interface, in km.
//! This is the sum of path_increment(i) * R(i) where path_increment(i) is the
//! angular distance traveled by the ray in each angular
//! distance increment along the head wave interface, and R(i)
//! is the radius of the head wave interface in that same
//! horizontal increment.
//! @param dist the horizontal distance traveled by the ray
//! below the headwave interface, in km.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getHeadwaveDistanceKm ( double* dist );

//! \brief Retrieve the total travel time for the GreatCircle,
//! in seconds.
//!
//! Retrieve the total travel time for the GreatCircle,
//! in seconds.
//! @param travelTime the total travel time in seconds is returned
//! in travelTime.  If the GreatCircle is invalid, travelTime
//! will equal BaseObject::NA_VALUE.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getTravelTime ( double* travelTime );

//! \brief Retrieve the total travel time and the 4 components that
//! contribute to it for the current GreatCircle.
//!
//! Retrieve the total travel time and the 4 components that
//! contribute to it for the current GreatCircle.
//! If the greatCircle is invalid, tTotal and all the
//! components will equal BaseObject::NA_VALUE.
//!
//! @param tTotal the total travel time, in seconds.
//! @param tSource the crustal travel time below the source, in seconds.
//! @param tReceiver the crustal travel time below the receiver, in seconds.
//! @param tHeadwave the head wave travel time, in seconds.
//! @param tGradient the Zhao gradient correction term, in seconds.
//! For GreatCircle objects that support Pg and Lg, this is always 0.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getTravelTimeComponents ( double* tTotal, double* tSource, double* tReceiver, double* tHeadwave, double* tGradient );

//! \brief Retrieve the weight assigned to each grid node that
//! was touched by the GreatCircle.
//!
//! Retrieve the weight assigned to each grid node that
//! was touched by the GreatCircle.
//!
//! A map which associates an instance of a GridProfile object with a
//! double <I>weight</I> is initialized.  Then every LayerProfile on the head
//! wave interface between the source and
//! receiver is visited and the angular distance, <I>d</I>, that the ray
//! traveled in the horizontal segment is retrieved.  If <I>d</I> > 0,
//! then the neighboring GridProfile objects that contributed to
//! the interpolated value of the LayerProfile are visited.
//! The product of <I>d * R * C</I>  is added to the weight associated
//! with that GridProfile object, where <I>R</I> is the radius of the
//! head wave interface for the LayerProfile object being evaluated,
//! and <I>C</I> is the interpolation coefficient for the
//! GridProfile - LayerProfile pair under consideration.
//! Then, all the GridProfile objects in the map are visited, the
//! grid node IDs extracted into int array <I>nodeId</I>, and the
//! <I>weight</I> extracted into double array <I>weight</I>.
//!
//! Note: Only grid nodes touched by this GreatCircle are included in the
//! output.  Each grid node is included only once, even though more than
//! one LayerProfile object may have contributed some weight to it.
//! The sum of all the weights will equal the horizontal distance
//! traveled by the ray along the head wave interface, from the source
//! pierce point to the receiver pierce point, in km.
//! @param nodeId the node IDs of all the grid nodes touched by the
//! current GreatCircle.
//! @param weight the weights of all the grid nodes touched by the
//! current GreatCircle.  Calling application must dimension this
//! array large enough to handle any possible size.
//! @param nWeights the number of elements in nodeId and weight.
//! Calling application must dimension this
//! array large enough to handle any possible size.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getWeights ( int nodeId[], double weight[], int* nWeights );

//! \brief Retrieve the node IDs and the interpolation
//! coefficients for the source CrustalProfile.
//!
//! Retrieve the node IDs and the interpolation
//! coefficients for the source CrustalProfile.  There will be
//! as many as 5 of each.  The sum of the weights will equal 1.
//! @param nodeids[] the nodeIds that influenced the interpolated values at the source.
//! @param weights[] the interpolation coefficients applied to the grid
//! nodes that influenced the interpolated values at the source.
//! @param nWeights the size of nodeids and weights (max value is 5).
//! @return "0" if this call was successful, positive error code if this call generated an error.
// TODO: test this
SLBM_LIB int slbm_shell_getWeightsSource ( int nodeids[], double weights[], int* nWeights );

//! \brief Retrieve the node IDs and the interpolation
//! coefficients for the receiver CrustalProfile.
//!
//! Retrieve the node IDs and the interpolation
//! coefficients for the receiver CrustalProfile.  The sum of the weights
//! will equal 1.
//! @param nodeids[] the nodeIds that influenced the interpolated values
//! at the receiver.
//! @param weights[] the interpolation coefficients applied to the grid
//! nodes that influenced the interpolated values at the receiver.
//! @param nWeights the size of nodeids and weights (max value is 5).
//! @return "0" if this call was successful, positive error code if this call generated an error.
// TODO: test this
SLBM_LIB int slbm_shell_getWeightsReceiver ( int nodeids[], double weights[], int* nWeights );

//! \brief Returns a human-readable string representation
//! of the GreatCircle object.
//!
//! Returns a human-readable string representation
//! of the GreatCircle object.
//! @param str a char pointer. NOTE: User should allocate memory to accommodate
//! 10000 chars.
//! @param verbosity specifies the amount of information
//! that is to be included in the return string.  Each
//! verbosity level includes all information in preceding
//! verbosity levels.
//! - 0 : nothing.  An empty string is returned.
//! - 1 : total distance and travel time summary
//! - 2 : gradient correction information for Pn/Sn.
//!       Nothing for Pg/Lg
//! - 3 : Source and receiver CrustalProfile information.
//! - 4 : Grid node weights.
//! - 5 : Head wave interface profiles
//! - 6 : Interpolation coefficients for great circle nodes on
//!       the head wave interface.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_toString ( char* str, int verbosity );

//! \brief Retrieve the number of Grid nodes in the Earth model.
//!
//! Retrieve the number of Grid nodes in the Earth model.
//! @param numGridNodes the number of elements in the grid
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getNGridNodes ( int* numGridNodes );

//! \brief Retrieve the number of LayerProfile objects positioned along
//! the head wave interface.
//!
//! Retrieve the number of LayerProfile objects positioned along
//! the head wave interface.  It is useful to call this method
//! before calling getGreatCircleData() since the value returned
//! by this method will be the number of elements that will be
//! populated in parameters headWaveVelocity[], neighbors[] and
//! coefficients[].
//! @param npoints number of LayerProfle objects.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getNHeadWavePoints ( int* npoints );

//! \brief Retrieve the lat (radians), lon (radians),
//! interface depths (km), P and S wave interval velocities (km/sec)
//! and P and S mantle gradient (1/sec) information
//! associated with a specified node in the velocity grid.
//!
//! Retrieve the interface depth, velocity and gradient
//! information associated with a specified node in the
//! velocity grid.
//! @param nodeId the node ID of the grid point in the model
//! (zero based index).
//! @param latitude the latitude of the grid node in radians.
//! @param longitude the longitude of the grid node in radians.
//! @param depth the depths of all the model interfaces, in km.
//! @param pvelocity an array containing the P velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param svelocity an array containing the S velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param gradient a 2-element array containing the P and S
//! velocity gradients in the mantle, in 1/sec.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getGridData ( int* nodeId, double* latitude, double* longitude, double* depth,
						   double* pvelocity, double* svelocity, double* gradient );

//! \brief Modify the velocity and gradient information
//! associated with a specified node in the Grid.
//!
//! Modify the velocity and gradient information
//! associated with a specified node in the Grid.
//! @param nodeId the node number of the grid point in the model.
//! (zero based index).
//! @param depth an array containing the depths of the tops of all the interfaces.
//! @param pvelocity an array containing the P velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param svelocity an array containing the S velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param gradient a 2-element array containing the P and S
//! velocity gradients in the mantle, in 1/sec.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setGridData ( int* nodeId, double* depth, double* pvelocity, double* svelocity, double* gradient );

//! \brief Retrieve the data required for input to the travel time
//! calculation, for the current GreatCircle object.
//!
//! Retrieve the data required for input to the travel time
//! calculation, for the current GreatCircle object.
//! @param phase the phase supported by the current GreatCircle.  Will be one of
//! Pn, Sn, Pg, Lg.
//! @param path_increment the actual horizontal separation of the LayerProfile
//! objects along the head wave interface, in radians.  The actual
//! separation will be reduced from the value requested in the call to
//! createGreatCircle() in order that some number of equal sized increments
//! will exactly fit between the source and receiver.
//! @param sourceDepth the depths of all the model interfaces below the source, in km.
//! @param sourceVelocity the P or S velocity of each interval below the source, in km/sec.
//! @param receiverDepth the depths of all the model interfaces below the receiver, in km.
//! @param receiverVelocity the P or S velocity of each interval below the
//! receiver, in km/sec.
//! @param npoints the number of horizontal increments sampled along the
//! head wave interface.  To discover this number before calling this method
//! call getNHeadWavePoints().
//! @param headWaveVelocity the P or S velocity at the center of each
//! horizontal segment between the source and the receiver, in km/sec.
//! The first horizontal segment starts at the source, the last horizontal
//! segment ends at the receiver, and each one is of size path_increment.  The head
//! wave velocities are interpolated at the center of each of these horizontal
//! segments, just below the head wave interface.
//! @param gradient the P or S velocity gradient in the mantle at the center
//! of each horizontal segment of the head wave, in 1/sec.  For Pg and Lg,
//! the values will be BaseObject::NA_VALUE.
//! @return "0" if this call was successful, positive error code if this call generated an error.
// TODO: test this
SLBM_LIB int slbm_shell_getGreatCircleData ( char* phase, double* path_increment, double sourceDepth[], double sourceVelocity[],
	double receiverDepth[],	double receiverVelocity[], int* npoints, double headWaveVelocity[], double gradient[]);

//! \brief Retrieve information about the interpolated points along the headwave
//! path, including the number of points, the indexes of the grid nodes that contributed
//! to interpolation of values at the points, and the interpolation coefficients used to
//! calculate values at the points.
//!
//! Retrieve information about the interpolated points along the headwave
//! path, including the number of points, the indexes of the grid nodes that contributed
//! to interpolation of values at the points, and the interpolation coefficients used to
//! calculate values at the points.
//!
//! The caller must supply all of the array required by this method and retains
//! ownership of those arrays.  This method assumes the arrays have been allocated
//! with sufficient memory to hold the requested information and simply populates
//! the supplied arrays.
//!
//! @param neighbors a ragged 2D array of ints with dimensions npoints x nnodes
//! containing the nodeIds of the neighboring grid nodes used to derive the interpolated
//! data at each head wave profile.
//! @param coefficients a ragged 2D array of doubles with dimensions npoints x nnodes containing
//! the interpolation coefficients applied to each element of neighbors.
//! @param maxpoints the maximum size of the first dimension of arrays neighbors and coefficients.
//! If npoints exceeds this value, an exception is thrown.  200 is a good estimate.
//! @param maxnodes the maximum size of the second dimension of arrays neighbors and coefficients.
//! If any value of nnodes exceeds this value, an exception is thrown. 5 is a good estimate.
//! @param npoints the number of horizontal increments sampled along the head wave interface.
//! @param nnodes an int array of length npoints containing the number of nodes that contributed
//! to the interpolation of information at the center of each horizontal segment of the ray path.
//! @return "0" if this call was successful, positive error code if this call generated an error.
// TODO: test this
SLBM_LIB int slbm_shell_getGreatCircleNodeInfo( int** neighbors, double** coefficients, const int* maxpoints,
	const int* maxnodes, int* npoints, int* nnodes );

//! \brief Retrieve interpolated data from the earth model at a single
//! specified latitude, longitude.
//!
//! Retrieve interpolated data from the earth model at a single
//! specified latitude, longitude.
//! @param lat the latitude where information is to be interpolated,
//! in radians.
//! @param lon the longitude where information is to be interpolated,
//! in radians.
//! @param nodeIds the nodeIds of the grid nodes that were involved
//! in the interpolation.
//! @param coefficients the interpolation coefficients that were applied to
//! the information from the neighboring grid nodes.
//! @param nnodes an integer value specifying number of grid nodes that
//! contributed to interpolation of values at this point.
//! @param depth the depths of the tops of the interfaces in the Earth model,
//! in km.  There will be one of these for each layer of the model.
//! @param pvelocity the P velocities of each layer of the model, in km/sec.
//! @param svelocity the S velocities of each layer of the model, in km/sec.
//! @param pgradient the mantle P velocity gradient, in 1/sec.
//! @param sgradient the mantle S velocity gradient, in 1/sec.
//! @return true if successful.  If not successful, nodeIds are all -1 and
//! all other returned arrays are populated with BaseObject::NA_VALUE.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getInterpolatedPoint ( double* lat, double* lon,
		int* nodeIds, double* coefficients, int* nnodes, double* depth, double* pvelocity,
		double* svelocity, double* pgradient, double* sgradient );

//! \brief Retrieve interpolated data from the earth model along a
//! transect defined by equal sized, 1 dimensional arrays of latitude and longitude.
//!
//! Retrieve interpolated data from the earth model along a
//! transect defined by equal sized, 1 dimensional arrays of latitude and longitude.
//! @param lat the latitudes along the transect, in radians.
//! @param lon the longitudes along the transect, in radians.
//! @param nLatLon the number of interpolated points along the transect.
//! @param nodeId a 2D array of ints with at least nLatLon x 5 elements that will be populated
//! with the nodeIds of the grid nodes that were involved in the interpolations.
//! @param coefficients a 2D array of doubles with at least nLatLon x 5 elements that will be populated
//! with the  interpolation coefficients that were applied to
//! the information from the neighboring grid nodes.
//! @param nNodes a 1D array of ints with at least nLatLon elements that will
//! be populated with the number of nodes that contributes to each nodeId,
//! coefficients.
//! @param depth the depths of the tops of the interfaces in the Earth model,
//! in km.
//! @param pvelocity the P velocities of each layer of the model, in km/sec.
//! @param svelocity the S velocities of each layer of the model, in km/sec.
//! @param pgradient the mantle P velocity gradient, in 1/sec.
//! @param sgradient the mantle S velocity gradient, in 1/sec.
//! @param nInvalid the number of points that were out of model range.
//! For any points outside of the model range, nodeIds are all -1 and
//! all other returned arrays are populated with BaseObject::NA_VALUE.
//! @return "0" if all points were in model range (nInvalid == 0).
// TODO: test this
SLBM_LIB int slbm_shell_getInterpolatedTransect ( double lat[], double lon[], int* nLatLon, int** nodeId,
	double** coefficients, int* nNodes, double depth[][NLAYERS], double pvelocity[][NLAYERS],
	double svelocity[][NLAYERS], double pgradient[NLAYERS], double sgradient[NLAYERS], int* nInvalid );

//! \brief Specify the latitude and longitude range in radians for active nodes.
//!
//! Specify the latitude and longitude range in radians for active nodes.
//! Active nodes are defined as follows:  for each triangle in the
//! tessellation, if any of the 3 nodes that define the triangle is
//! within the latitude longitude range specified by this method, then
//! all 3 nodes are defined to be active nodes.
//! Lats and lons must be specified in radians.
//! @param latmin minimum latitude in radians
//! @param lonmin minimum longitude in radians
//! @param latmax maximum latitude in radians
//! @param lonmax maximum longitude in radians
SLBM_LIB int slbm_shell_initializeActiveNodes ( double* latmin, double* lonmin, double* latmax, double* lonmax );

//! \brief Specify the name of a file that contains a list of points that define a
//! polygon that enclose the set of grid nodes that are to be considered active nodes.
//!
//! Specify the name of a file that contains a list of points that define a
//! polygon that enclose the set of grid nodes that are to be considered active nodes.
//! <p>
//! Active nodes are defined as follows:  for each triangle in the
//! tessellation, if any of the 3 nodes that define the triangle is
//! within the polygon specified by this method, then
//! all 3 nodes are defined to be active nodes.
//! <p>If the last point and first point are not coincident, then the polygon is 'closed'
//! by connecting the first and last point by an edge.
//!
//! @param polygonFileName the name of a file that contains a list of points that define a
//! polygon that enclose the set of grid nodes that are to be considered active nodes.
//!
SLBM_LIB int slbm_shell_initActiveNodesFile(char* polygonFileName);

//! \brief Specify a list of points that define a polygon that enclose the set of grid nodes
//! that are to be considered active nodes.
//!
//! Specify a list of points that define a polygon that enclose the set of grid nodes
//! that are to be considered active nodes.
//! Active nodes are defined as follows:  for each triangle in the
//! tessellation, if any of the 3 nodes that define the triangle is
//! within the polygon specified by this method, then
//! all 3 nodes are defined to be active nodes.
//! <p>If the last point and first point are not coincident, then the polygon is 'closed'
//! by connecting the first and last point by an edge.
//!
//! @param lat a 1D array of doubles specifying the latitudes of the points that define
//! the polygon. Whether units are degrees or radians depends on parameter inDegrees.
//! @param lon a 1D array of doubles specifying the longitudes of the points that define
//! the polygon. Whether units are degrees or radians depends on parameter inDegrees.
//! @param npoints a single integer value specifying the number of latitude and longitude
//! points defined.
//! @param inDegrees if true, latitudes and longitudes are assumed to be in degrees,
//! if false, they are assumed to be in radians.
//!
SLBM_LIB int slbm_shell_initActiveNodesPoints(double* lat, double* lon, int* npoints, int* inDegrees);

//! \brief Retrieve the number of active nodes in the Grid.
//!
//! Retrieve the number of active nodes in the Grid.
SLBM_LIB int slbm_shell_getNActiveNodes ( int* nNodes );

//! \brief Clear all active nodes.
//! Clear all active nodes.
SLBM_LIB int slbm_shell_clearActiveNodes();

//! \brief Retrieve the grid node ID that corresponds to a specified
//! active node ID.
//!
//! Retrieve the grid node ID that corresponds to a specified
//! active node ID.
SLBM_LIB int slbm_shell_getGridNodeId ( int activeNodeId, int* gridNodeId );

//! \brief Retrieve the active node ID that corresponds to a specified
//! grid node ID.
//!
//! Retrieve the active node ID that corresponds to a specified
//! grid node ID.
SLBM_LIB int slbm_shell_getActiveNodeId ( int gridNodeId );

//! \brief Retrieve the number of times that node
//! has been 'touched' by a GreatCircle object.
//!
//! Retrieve the number of times that node
//! has been 'touched' by a GreatCircle object.
SLBM_LIB int slbm_shell_getNodeHitCount ( int* nodeId, int* hitCount );

//! \brief Retrieve the node IDs of the nodes that surround the
//! specified node.
//!
//! Retrieve the node IDs of the nodes that surround the specified node.
//! <p>The caller must supply int array neighbors which is dimensioned large
//! enough to hold the maximum number of neighbors that a node can have,
//! which is 8.  The actual number of neighbors is returned in nNeighbors.
SLBM_LIB int slbm_shell_getNodeNeighbors ( int* nid, int neighbors[], int* nNeighbors );

//! \brief Retrieve the node IDs of the nodes that surround the
//! specified node.
//!
//! Retrieve the node IDs of the nodes that surround the specified node.
//! <p>The caller must supply int array neighbors which is dimensioned large
//! enough to hold the maximum number of neighbors that a node can have,
//! which is 8.  The actual number of neighbors is returned in nNeighbors.
SLBM_LIB int slbm_shell_getNodeNeighborInfo ( int* nid, int neighbors[], double distance[], double azimuth[], int* nNeighbors );

//! \brief Retrieve the angular separation of two grid nodes, in radians.
//!
//! Retrieve the angular separation of two grid nodes, in radians.
SLBM_LIB int slbm_shell_getNodeSeparation ( int* node1, int* node2, double* distance );

//! \brief Retrieve the azimuth from grid node1 to grid node2, radians.
//!
//! Retrieve the azimuth from grid node1 to grid node2, radians.
SLBM_LIB int slbm_shell_getNodeAzimuth ( int* node1, int* node2, double* azimuth );

//! \brief Retrieve the travel time uncertainty in sec
//! for specified phase, distance.
//! 
//! Retrieve the travel time uncertainty in sec
//! for specified phase, distance.
//! @param phase Pn, Sn, Pg or Lg
//! @param distance source-receiver separation in radians.
//! @param uncertainty returns the uncertainty in sec
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getTravelTimeUncertainty ( int* phase, double* distance, double* uncertainty );

//! \brief Retrieve travel time uncertainty in sec 
//! using the phase and distance specified in last call to getGreatCircle().
//!
//! Retrieve travel time uncertainty in sec 
//! using the phase and distance specified in last call to getGreatCircle().
//! @param uncertainty uncertainty of the travel time in seconds.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getTTUncertainty ( double* uncertainty );

//! \brief Retrieve the slowness uncertainty in sec/radian
//! for specified phase, distance.
//!
//! Retrieve the slowness uncertainty in sec/radian
//! for specified phase, distance.
//! @param phase Pn, Sn, Pg or Lg
//! @param distance source-receiver separation in radians.
//! @param uncert returns the uncertainty in sec/radian
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getSlownessUncertainty( int* phase, double* distance, double* uncert );

//! \brief Retrieve uncertainty of the horizontal slowness, in seconds/radian 
//! using the phase and distance specified in last call to getGreatCircle().
//!
//! Retrieve uncertainty of the horizontal slowness, in seconds/radian, 
//! using the phase and distance specified in last call to getGreatCircle().
//! @param slownessUncertainty uncertainty of the horizontal slowness, in seconds/radian.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getSHUncertainty(double* slownessUncertainty);

//! \brief Retrieve some of the parameters that contribute to the calculation of
//! of total travel time using the Zhao algorithm.
//!
//! Retrieve some of the parameters that contribute to the calculation of
//! of total travel time using the Zhao algorithm.  This method only returns
//! meaningful results for phases Pn and Sn.  For Pg and Lg, all the parameters
//! of type double are returned with values BaseObject::NA_VALUE and udSign is
//! returned with value of -999.
//! @param Vm the velocity at the top of the mantle averaged along the Moho
//! between the source and receiver pierce points.
//! @param Gm the velocity gradient at the top of the mantle averaged along the Moho
//! between the source and receiver pierce points.
//! @param H the turning depth of the ray relative to the Moho
//! @param C a constant whose product with V0 gives the mantle velocity gradient
//! for a flat Earth. V0 is the velocity of the top of the mantle averaged over
//! the whole model.
//! @param Cm a constant whose product with Vm gives the mantle velocity gradient
//! for a flat Earth.
//! @param udSign a value of 0 indicates the source is in the crust.
//! +1 indicates the ray leaves a mantle source in the downgoing
//! direction.  -1 indicates the ray leaves a mantle source in an upgoing direction.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getZhaoParameters ( double* Vm, double* Gm, double* H, double* C, double* Cm, int* udSign );

//! \brief Retrieve the weight assigned to each active node that
//! was touched by the GreatCircle.
//!
//! \brief Retrieve the weight assigned to each active node that
//! Retrieve the weight assigned to each  node that
//! was touched by the GreatCircle.
//!
//! <p>A map which associates an instance of a GridProfile object with a
//! double <I>weight</I> is initialized.  Then every LayerProfile on the head
//! wave interface between the source and
//! receiver is visited and the angular distance, <I>d</I>, that the ray
//! traveled in the horizontal segment is retrieved.  If <I>d</I> > 0,
//! then the neighboring GridProfile objects that contributed to
//! the interpolated value of the LayerProfile are visited.
//! The product of <I>d * R * C</I>  is added to the weight associated
//! with that GridProfile object, where <I>R</I> is the radius of the
//! head wave interface for the LayerProfile object being evaluated,
//! and <I>C</I> is the interpolation coefficient for the
//! GridProfile - LayerProfile pair under consideration.
//! Then, all the GridProfile objects in the map are visited, the
//! grid node IDs extracted into int array <I>nodeId</I>, and the
//! <I>weight</I> extracted into double array <I>weight</I>.
//!
//! <p>Note: Only grid nodes touched by this GreatCircle are included in the
//! output.  Each grid node is included only once, even though more than
//! one LayerProfile object may have contributed some weight to it.
//! The sum of all the weights will equal the horizontal distance
//! traveled by the ray along the head wave interface, from the source
//! pierce point to the receiver pierce point, in km.
//! @param nodeId the active node IDs of all the grid nodes touched by the
//! current GreatCircle.
//!
//! @param weight the weights of all the grid nodes touched by the
//! current GreatCircle.  Calling application must dimension this
//! array large enough to handle any possible size.
//! @param nWeights the number of elements in nodeId and weight.
//! Calling application must dimension this
//! array large enough to handle any possible size.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getActiveNodeWeights ( int nodeId[], double weight[], int* nWeights );

//! \brief Retrieve the active node IDs and the interpolation
//! coefficients for the source CrustalProfile.
//!
//! Retrieve the active node IDs and the interpolation
//! coefficients for the source CrustalProfile.  The sum of the weights
//! will equal 1.
//! @return "0" if this call was successful, positive error code if this call generated an error.
// TODO: test this
SLBM_LIB int slbm_shell_getActiveNodeWeightsSource ( int nodeids[], double weights[], int* nWeights );

//! \brief Retrieve the active node IDs and the interpolation
//! coefficients for the receiver CrustalProfile.
//!
//! Retrieve the active node IDs and the interpolation
//! coefficients for the receiver CrustalProfile.  The sum of the weights
//! will equal 1.
//! @return "0" if this call was successful, positive error code if this call generated an error.
// TODO: test this
SLBM_LIB int slbm_shell_getActiveNodeWeightsReceiver ( int nodeids[], double weights[], int* nWeights );


//! \brief Retrieve the node IDs of the nodes that surround the
//! specified node.
//!
//! Retrieve the node IDs of the nodes that surround the specified node.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getActiveNodeNeighbors ( int* nid, int neighbors[], int* nNeighbors );

//! \brief Retrieve the node IDs of the nodes that surround the
//! specified node.
//!
//! Retrieve the node IDs of the nodes that surround the specified node.
//! <p>The caller must supply int array neighbors which is dimensioned large
//! enough to hold the maximum number of neighbors that a node can have,
//! which is 8.  The actual number of neighbors is returned in nNeighbors.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getActiveNodeNeighborInfo ( int* nid, int neighbors[], double distance[],
	double azimuth[], int* nNeighbors );

//! \brief Retrieve the lat (radians), lon (radians),
//! interface depths (km), P and S wave interval velocities (km/sec)
//! and P and S mantle gradient (1/sec) information
//! associated with a specified active node in the velocity grid.
//!
//! Retrieve the interface depth, velocity and gradient
//! information associated with a specified active node in the
//! velocity grid.
//! @param nodeId the active node ID of the grid point in the model
//! (zero based index).
//! @param latitude the latitude of the grid node in radians.
//! @param longitude the longitude of the grid node in radians.
//! @param depth the depths of all the model interfaces, in km.
//! @param pvelocity an array containing the P velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param svelocity an array containing the S velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param gradient a 2-element array containing the P and S
//! velocity gradients in the mantle, in 1/sec.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getActiveNodeData (	int* nodeId,
									double* latitude,
									double* longitude,
									double depth[NLAYERS],
									double pvelocity[NLAYERS],
									double svelocity[NLAYERS],
									double gradient[2] );

//! \brief Modify the velocity and gradient information
//! associated with a specified active node in the Grid.
//!
//! Modify the velocity and gradient information
//! associated with a specified active node in the Grid.
//! @param nodeId the node number of the grid point in the model.
//! (zero based index).
//! @param depth an array containing the depths of the tops of the
//! layers, in km
//! @param pvelocity an array containing the P velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param svelocity an array containing the S velocities of all the
//! intervals at the specified grid node, in km/sec.
//! @param gradient a 2-element array containing the P and S
//! velocity gradients in the mantle, in 1/sec.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setActiveNodeData (	int* nodeId,
									double depth[NLAYERS],
									double pvelocity[NLAYERS],
									double svelocity[NLAYERS],
									double gradient[2] );

//! \brief Set the value of chMax.  c is the zhao c parameter and h is
//! the turning depth of the ray below the moho.  Zhao method only valid
//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
//!
//! Set the value of chMax.  c is the zhao c parameter and h is
//! the turning depth of the ray below the moho.  Zhao method only valid
//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
//! This call modifies global parameter BaseObject::ch_max
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setCHMax ( double* chMax );

//! \brief Retrieve the current value of chMax.  c is the zhao c parameter
//! and h is the turning depth of the ray below the moho.  Zhao method only valid
//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
//!
//! Retrieve the current value of chMax.  c is the zhao c parameter and h is
//! the turning depth of the ray below the moho.  Zhao method only valid
//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
//! This call retrieves global parameter BaseObject::ch_max
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getCHMax ( double* chMax );

//! \brief Retrieve the average P or S wave mantle velocity that is specified
//! in the model input file, in km/sec.
//!
//! Retrieve the average P or S wave mantle velocity that is specified
//! in the model input file.  This value is used in the calculation of
//! the Zhao c parameter.
//! @param type specify either BaseObject::PWAVE or BaseObject::SWAVE.
//! @param velocity the P or S wave velocity is returned in this parameter,
//! in km/sec.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getAverageMantleVelocity ( int* type, double* velocity );

//! \brief Set the average P or S wave mantle velocity that is recorded
//! in the model input file, in km/sec.
//!
//! Set the average P or S wave mantle velocity that is specified
//! in the model input file.  This value is used in the calculation of
//! the Zhao c parameter.
//! @param type specify either BaseObject::PWAVE or BaseObject::SWAVE.
//! @param velocity the P or S wave velocity that is to be set,
//! in km/sec.  This value will be stored in the model file, if the
//! model file is written to file by a call to saveVelocityModel()
//! subsequent to a call to this method.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setAverageMantleVelocity ( int* type, double* velocity );

//! \brief Retrieve the tessellation ID of the model currently in memory.
//!
//! Retrieve the tessellation ID of the model currently in memory.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getTessId ( char* tessId );

//! \brief Retrieve the fraction of the path length of the current
//! GreatCircle object that is within the currently defined active region.
//!
//! Retrieve the fraction of the path length of the current
//! GreatCircle object that is within the currently defined active region.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getFractionActive ( double* fractionActive );

//! \brief Set the maximum source-receiver separation for Pn/Sn phase,
//! in radians.
//!
//! Set the maximum source-receiver separation for Pn/Sn phase,
//! in radians.  Source-receiver separations greater than the specified
//! value will result in an exception being thrown in createGreatCircle().
//! Default value is PI radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setMaxDistance ( const double* maxDistance );

//! \brief Retrieve the current value for the maximum source-receiver
//! separation, in radians.
//!
//! Retrieve the current value for the maximum source-receiver
//! separation, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getMaxDistance ( double* maxDistance );

//! \brief Set the maximum source depth for Pn/Sn phase,
//! in km.
//!
//! Set the maximum source depth for Pn/Sn phase,
//! in km.  Source depths greater than the specified
//! value will result in an exception being thrown in createGreatCircle().
//! Default value is 9999 km.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setMaxDepth ( const double* maxDepth );

//! \brief Retrieve the current value for the maximum source depth, in km.
//!
//! Retrieve the current value for the maximum source depth, in km.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getMaxDepth ( double* maxDepth );

//! \brief Retrieve information about Pg/Lg travel time calculations.
//!
//! Retrieve information about Pg/Lg travel time calculations.  This method
//! only returns useful information when the phase is Pg or Lg.  For Pn and
//! Sn, all information is returned as SLBMGlobals::NA_VALUE.
//! @param tTotal is the total travel time in seconds.  It will be exactly
//! equal to the lesser of tTaup or tHeadwave, except that if tTaup is equal
//! to SLBMGlobals::NA_VALUE, then tTotal will equal tHeadwave.
//! @param tTaup is the taup travel time in seconds.  If this value is equal
//! to SLBMGlobals::NA_VALUE, it means that the taup calculation failed for
//! some reason (shadow zones, etc.).
//! @param tHeadwave is the headwave travel time in secods
//! @param pTaup TauP ray parameter.
//! @param pHeadwave headwave ray parameter.
//! @param trTaup is the radius at which the taup ray turned, in km.
//! @param trHeadwave is the radius at which the headwave ray turned, in km.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getPgLgComponents(double* tTotal, double* tTaup,
			double* tHeadwave, double* pTaup, double* pHeadwave, double* trTaup, double* trHeadwave );

//! \brief Retrieve the horizontal slowness, i.e., the derivative of travel time
//! wrt to receiver-source distance, in seconds/radian.
//!
//! Retrieve the horizontal slowness, in seconds/radian.
//! @param slowness the derivative of travel time wrt to source latitude.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getSlowness(double* slowness);

// new gtb 16Jan2009
//-------------------------------------------------------------
//! \brief Retrieve the derivative of travel time wrt to source latitude,
//! in seconds/radian.
//!
//! Retrieve the derivative of travel time wrt to source latitude,
//! in seconds/radian.
//! @param dtt_dlat the derivative of travel time wrt to source latitude.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_get_dtt_dlat(double* dtt_dlat);

//! \brief Retrieve the derivative of travel time wrt to source longitude,
//! in seconds/radian.
//!
//! Retrieve the derivative of travel time wrt to source longitude,
//! in seconds/radian.
//! @param dtt_dlon the derivative of travel time wrt to source longitude.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_get_dtt_dlon(double* dtt_dlon);

//! \brief Retrieve the derivative of travel time wrt to source depth,
//! in seconds/km.
//!
//! Retrieve the derivative of travel time wrt to source depth,
//! in seconds/km.
//! @param dtt_ddepth the derivative of travel time wrt to source depth.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_get_dtt_ddepth(double* dtt_ddepth);

////! \brief Retrieve the derivative of horizontal slowness wrt to source-receiver
////! separation, in seconds/radian.
////!
////! Retrieve the derivative of horizontal slowness wrt to source-receiver
////! separation, in seconds/radian.
////! @param dsh_ddist the derivative of horizontal slowness wrt to source-receiver
////! separation, in seconds/radian.
////! @return "0" if this call was successful, positive error code if this call generated an error.
//int slbm_shell_get_dsh_ddist(double* dsh_ddist);
//
////! \brief Retrieve the derivative of horizontal slowness wrt to source latitude,
////! in seconds/radian.
////!
////! Retrieve the derivative of horizontal slowness wrt to source latitude,
////! in seconds/radian.
////! @param dsh_dlat the derivative of horizontal slowness wrt to source latitude.
////! @return "0" if this call was successful, positive error code if this call generated an error.
//int slbm_shell_get_dsh_dlat(double* dsh_dlat);
//
////! \brief Retrieve the derivative of horizontal slowness wrt to source longitude,
////! in seconds/radian.
////!
////! Retrieve the derivative of horizontal slowness wrt to source longitude,
////! in seconds/radian.
////! @param dsh_dlon the derivative of horizontal slowness wrt to source longitude.
////! @return "0" if this call was successful, positive error code if this call generated an error.
//int slbm_shell_get_dsh_dlon(double* dsh_dlon);
//
////! \brief Retrieve the derivative of horizontal slowness wrt to source depth,
////! in seconds/km.
////!
////! Retrieve the derivative of horizontal slowness wrt to source depth,
////! in seconds/km.
////! @param dsh_ddepth the derivative of horizontal slowness wrt to source depth.
////! @return "0" if this call was successful, positive error code if this call generated an error.
//int slbm_shell_get_dsh_ddepth(double* dsh_ddepth);

// new sb 2011/07/27  version 2.7.0
//-------------------------------------------------------------------------

//! \brief compute distance and azimuth between two points, A and B
//! (all quantities are in radians).
//!
//! compute distance and azimuth between two points, A and B
//! (all quantities are in radians). Computed distance will range
//! between 0 and PI and azimuth will range from -PI to PI.
//! If distance is zero, or if A is located at north or south
//! pole, azimuth will be set to naValue.
//! @param aLat the latitude of the first specified point, in radians.
//! @param aLon the longitude of the first specified point, in radians.
//! @param bLat the latitude of the second specified point, in radians.
//! @param bLon the longitude of the second specified point, in radians.
//! @param distance from point A to point B, in radians.
//! @param azimuth from point A to point B, in radians.
//! @param naValue value to return if result is invalid, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getDistAz(double aLat, double aLon, double bLat, double bLon,
	double* distance, double* azimuth, double naValue);

//! \brief Find point B that is the specified distance and azimuth
//! from point A.  All quantities are in radians.
//!
//!Find point B that is the specified distance and azimuth
//! from point A.
//! @param aLat the latitude of the first specified point, in radians.
//! @param aLon the longitude of the first specified point, in radians.
//! @param distance from point A to point B, in radians.
//! @param azimuth from point A to point B, in radians.
//! @param bLat the latitude of the second specified point, in radians.
//! @param bLon the longitude of the second specified point, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_movePoint(double aLat, double aLon, double distance, double azimuth,
	double* bLat, double* bLon);

//! \brief Retrieve the latitudes, longitudes and depths of all the profile positions
//! along the moho.
//!
//! Retrieve the latitudes, longitudes and depths of all the profile positions
//! along the moho.  Profile positions are located at the center of each segment
//! of the head wave interface between the source and receiver.  The first position
//! is located path_increment/2 radians from the source, the last profile position is located
//! path_increment/2 radians from the receiver, and the others are spaced path_increment radians apart.
//! @param latitude the latitude at the center of each headwave segment, in radians.
//! @param longitude the longitude at the center of each headwave segment, in radians.
//! @param depth the depth below surface of ellipsoid of the headwave interface at the center of each headwave segment, in km.
//! @param npoints the number of horizontal increments sampled along the head wave interface.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getGreatCircleLocations ( double latitude[], double longitude[] ,
										double depth[], int* npoints);

//! \brief Retrieve an array of lat, lon points along a great circle
//! path between two specified points, a and b.
//!
//! Retrieve an array of lat, lon points along a great circle path
//! between two specified points. The great circle path between a and b is
//! divided into npoints-1 equal size cells and the computed points are
//! located at the boundaries of those cells.  First point will
//! coincide with point a and last point with point b.
//! @param aLat the latitude of the first specified point, in radians.
//! @param aLon the longitude of the first specified point, in radians.
//! @param bLat the latitude of the second specified point, in radians.
//! @param bLon the longitude of the second specified point, in radians.
//! @param npoints the desired number of points along the great circle,
//! in radians.
//! @param latitude the latitudes of the points along the great circle, in radians.
//! @param longitude the longitudes of the points along the great circle, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getGreatCirclePoints(double aLat, double aLon, double bLat, double bLon,
	int npoints, double latitude[], double longitude[]);

//! \brief Retrieve an array of lat, lon points along a great circle
//! path between two specified points, a and b.
//!
//! Retrieve an array of lat, lon points along a great circle path
//! between two specified points. The great circle path between a and b is
//! divided into npoints equal size cells and the computed points are
//! located at the centers of those cells.
//! @param aLat the latitude of the first specified point, in radians.
//! @param aLon the longitude of the first specified point, in radians.
//! @param bLat the latitude of the second specified point, in radians.
//! @param bLon the longitude of the second specified point, in radians.
//! @param npoints the desired number of points along the great circle
//! @param latitude the latitudes of the points along the great circle, in radians.
//! @param longitude the longitudes of the points along the great circle, in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getGreatCirclePointsOnCenters(double aLat, double aLon, double bLat, double bLon,
	int npoints, double latitude[], double longitude[]);

//! \brief Retrieve the latitude  and longitude of the moho pierce point below the source,
//! in radians.
//!
//! Retrieve the latitude  and longitude of the moho pierce point below the source,
//! in radians.  For Pg, Lg and sources in the mantle an exception is thrown.
//! @param lat the latitude of the source pierce point, in radians.
//! @param lon the longitude of the source pierce point, in radians.
//! @param depth moho depth in km below sea level
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getPiercePointSource(double* lat, double* lon, double* depth);

//! \brief Retrieve the latitude  and longitude of the moho pierce point below the receiver,
//! in radians.
//!
//! Retrieve the latitude  and longitude of the moho pierce point below the receiver,
//! in radians.  For Pg, Lg  an exception is thrown.
//! @param lat the latitude of the receiver pierce point, in radians.
//! @param lon the longitude of the receiver pierce point, in radians.
//! @param depth moho depth in km below sea level
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getPiercePointReceiver(double* lat, double* lon, double* depth);

//! \brief Retrieve del_distance in radians.
//!
//! Retrieve current value of del_distance, in radians.  This is the horizontal separation
//! between two points used to compute horizontal slowness and the derivative of travel time
//! with respect to lat and lon.
//! @param delDistance horizontal separation in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getDelDistance(double* delDistance);

//! \brief Set the value of del_distance, radians.
//!
//! Set the value of del_distance, in radians.  This is the horizontal separation
//! between two points used to compute horizontal slowness and the derivative of travel time
//! with respect to lat and lon.
//! @param delDistance horizontal separation in radians.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setDelDistance( double delDistance);

//! \brief Retrieve del_depth in km.
//!
//! Retrieve current value of del_depth, in km.  This is the vertical separation
//! between two points used to compute the derivative of travel time with respect to depth.
//! @param delDepth vertical separation in km.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_getDelDepth(double* delDepth);

//! \brief Set the value of del_depth, in km.
//!
//! Set the value of del_depth, in km.  This is the vertical separation
//! between two points used to compute the derivative of travel time with respect to depth.
//! @param delDepth vertical separation in km.
//! @return "0" if this call was successful, positive error code if this call generated an error.
SLBM_LIB int slbm_shell_setDelDepth( double delDepth);

//! \brief Set the desired spacing of great circle nodes 
//! along the head wave interface, in radians.  
//!
//! Set the desired spacing of great circle nodes 
//! along the head wave interface, in radians.  
//! The actual spacing will be
//! reduced from the requested value in order that an integral 
//! number of equally spaced LayerProfile objects will exactly
//! span the source-receiver separation.  Defaults to 
//! 0.1 degrees if not specified.  
//!
//! @param pathIncrement the desired spacing of great circle nodes
//! along the head wave interface, in radians.  
SLBM_LIB int slbm_shell_setPathIncrement(double pathIncrement);

//! \brief Retrieve the current value of the spacing of great circle nodes 
//! along the head wave interface, in radians.  
//!
//! Retrieve the current value of the spacing of great circle nodes 
//! along the head wave interface, in radians.
//! The actual spacing will be
//! reduced from the requested value in order that an integral 
//! number of equally spaced LayerProfile objects will exactly
//! span the source-receiver separation.  The default value is 0.1 degrees.
//!
//! @param pathIncrement the current value of the spacing of great circle nodes
//! along the head wave interface, in radians.  
SLBM_LIB int slbm_shell_getPathIncrement(double* pathIncrement);

//! \brief Specify the interpolation type to use, either
//! 'linear' or 'natural_neighbor'.
//!
//! Specify the interpolation type to use, either
//! "LINEAR" or "NATUTAL_NEIGHBOR"..  When using the old
//! SLBMGrid objects, LINEAR is the only option allowed.
//!
SLBM_LIB int slbm_shell_setInterpolatorType(char* interpolatorType);

//! \brief Retrieve the type of interpolator currently
//! in use; either "LINEAR" or "NATUTAL_NEIGHBOR".
//!
//! @return the type of interpolator currently
//! in use; either "LINEAR" or "NATUTAL_NEIGHBOR".
SLBM_LIB int slbm_shell_getInterpolatorType(char* interpolatorType);

//! \brief Retrieve a string describing the contents of the model.
//! Retrieve a string describing the contents of the model.
//! @param modelString char* big enough to hold answer.
//! @param allocatedSize size of modelString.  If not big enough
//! to hold the answer, error is returned.
SLBM_LIB int slbm_shell_getModelString(char* modelString, int* allocatedSize);

//! \brief Retrieve a conveniently formated table of the uncertainty
//! values for the specified phaseIndex and attributeIndex.
//!
//! Retrieve a conveniently formated table of the uncertainty
//! values for the specified phaseIndex and attributeIndex.
//! @param phaseIndex 0:Pn, 1:Sn, 2:Pg, 3:Lg
//! @param attributeIndex 0:TT, 1:AZ, 2:SH
//! @param uncertaintyTable a char* array big enough to hold the results.
//! 1000 characters should do it.
//! @param allocatedSize the size of the supplied char*.  If the required
//! size is bigger than the allocatedSize, a zero length string is returned,
//! return value is -1, and errorMessage will indicate how much space was
//! required.
SLBM_LIB int slbm_shell_getUncertaintyTable(int* phaseIndex, int* attributeIndex, char* uncertaintyTable,
		int* allocatedSize);

//! \brief Retrieve an inconveniently formated table of the uncertainty
//! values for the specified phaseIndex and attributeIndex.
//!
//! Retrieve a inconveniently formated table of the uncertainty
//! values for the specified phaseIndex and attributeIndex.
//! @param phaseIndex 0:Pn, 1:Sn, 2:Pg, 3:Lg
//! @param attributeIndex 0:TT, 1:AZ, 2:SH
//! @param uncertaintyTable a char* array big enough to hold the results.
//! 1000 characters should do it.
//! @param allocatedSize the size of the supplied char*.  If the required
//! size is bigger than the allocatedSize, a zero length string is returned,
//! return value is -1, and errorMessage will indicate how much space was
//! required.
SLBM_LIB int slbm_shell_getUncertaintyTableFileFormat(int* phaseIndex, int* attributeIndex, char* uncertaintyTable,
		int* allocatedSize);

//-------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _SLBM_C_SHELL_H */
