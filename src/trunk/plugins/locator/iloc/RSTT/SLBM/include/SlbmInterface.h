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


//- ****************************************************************************
//-
//- Program:       SlbmInterface
//- Module:        $RCSfile: SlbmInterface.h,v $
//- Revision:      $Revision: 1.109 $
//- Last Modified: $Date: 2013/09/08 22:44:43 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef SlbmInterface_H
#define SlbmInterface_H

// **** _SYSTEM INCLUDES_ ******************************************************
#include <map>
#include <cmath>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GreatCircle.h"
#include "GreatCircle.h"
#include "GreatCircleFactory.h"
#include "GridGeoTess.h"
#include "Uncertainty.h"
#include "SLBMException.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

//! \brief The primary interface to the SLBM library, providing
//! access to all supported functionality.
//!
//! The primary interface to the SLBM library, providing
//! access to all supported functionality.  
//!
//! SlbmInterface maintains a Grid object which is loaded into memory 
//! with the loadVelocityModel() method.  This Grid object remains in 
//! memory until the SlbmInterface destructor is called. 
//!
//! SlbmInterface also maintains a single instance of a GreatCircle object which
//! is instantiated with a call to createGreatCircle().  Once instantiated, many
//! SlbmInterface methods can retrieve information from it, such as getTravelTime(),
//! getTravelTimeComponents(), getWeights(), and more.  Once instantiated, the
//! GreatCircle can be interrogated until it is replaced with another GreatCircle
//! by a subsequent call to createGreatCircle(), or is deleted by clear().
class SLBM_EXP SlbmInterface
{

public:

	//! \brief Default constructor.  Instantiates an SlbmInterface object
	//! based on an ellipsoidal earth.
	//! 
	//! Default constructor.  Instantiates an SlbmInterface object 
	//! based on an ellipsoidal earth.  Geographic latitudes are converted
	//! to geocentric latitudes, and the radius of the Earth varies as a
	//! function of latitude.
	SlbmInterface();

	//! \brief Parameterized constructor.  Instantiates an SlbmInterface object 
	//! that is only partly based on an ellipsoidal earth.
	//!
	//! Parameterized constructor.  Instantiates an SlbmInterface object 
	//! that is only partly based on an ellipsoidal earth.  
	//! Geographic latitudes are converted to geocentric latitudes, 
	//! but the radius of the Earth is considered a constant independent
	//! of latitude. 
	//! @param earthRadius the constant radius of the earth in km.
	SlbmInterface(const double& earthRadius);

	//! \brief Destructor. 
	//!
	//! Destructor.  Deletes the GreatCircle object 
	//! and the Grid object.
	virtual ~SlbmInterface();

	//! \brief Retrieve the SLBM Version number.
	//!
	//! Retrieve the SLBM Version number.  
	//! @return the current version number
	string getVersion() { return SlbmVersion; };

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
	void loadVelocityModel(const string& modelPath);

	//! \brief Save the velocity model currently in memory to
	//! the specified file.
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
	//! <li>SLBM version 4 single-file format.  This is the default+preferred
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
	//! If omitted, defaults to 4: all model information written to a single
	//! file.
	void saveVelocityModel(const string& modelFileName, const int& format=4);

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	//! \brief Deprecated. Load the velocity model into memory from
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
	//! @param modelPath the path to the file or directory
	//! that contains the model.
	void loadVelocityModelBinary(const string& modelPath)
	{ loadVelocityModel(modelPath); };

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
	void specifyOutputDirectory(const string& directoryName);

	//! \brief Deprecated.  Use saveVelocityModel() instead.
	//! Write the model in format 3 to directory
	//! previously specified with a call to specifyOutputDirectory().
	//!
	//! This method is deprecated and is provided only for backward
	//! compatibility with previous versions of SLBM.  Use method
	//! saveVelocityModel() instead.
	//! <p>The model is written in format 3 to the directory previously
	//! specified with a call to specifyOutputDirectory().
	void saveVelocityModelBinary();

	//! \brief Load the velocity model into memory from an FDB
	//! database configured DataBuffer.
	//!
	//! Load the velocity model into memory from
	//! an FDB database configured DataBuffer.
	//! Slbm uses the same DataBuffer read statements as used
	//! by the file based function of the same name.
	//! See loadVelocityModelBinary(const string& modelDirectory);
	void loadVelocityModelBinary(util::DataBuffer& buffer);

	//! \brief Write the model currently in memory out to the
	//! input DataBuffer.
	//!
	//! Write the model currently in memory out to the input
	//! DataBuffer.
	void saveVelocityModelBinary(util::DataBuffer& buffer);

	///@endcond

	//! \brief Returns the size of a DataBuffer object required
	//! to store this SLBMInterface objects model data.
	//!
	//! Returns the size of a DataBuffer object required
	//! to store this SLBMInterface objects model data.
	int  getBufferSize() const;

	//! \brief Specify the interpolation type to use, either
	//! 'linear' or 'natural_neighbor'.
	//!
	//! Specify the interpolation type to use, either
	//! "LINEAR" or "NATUTAL_NEIGHBOR". With models loaded in the old
	//! SLBM version 2 formats (formats 1 and 2), LINEAR is the only option allowed.
	//!
	void setInterpolatorType(const string& interpolatorType);

	//! \brief Retrieve the type of interpolator currently
	//! in use; either "LINEAR" or "NATUTAL_NEIGHBOR".
	//!
	//! @return the type of interpolator currently
	//! in use; either "LINEAR" or "NATUTAL_NEIGHBOR".
	string getInterpolatorType();

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
	void createGreatCircle(const string& phase,
					const double& sourceLat,
					const double& sourceLon,
					const double& sourceDepth,
					const double& receiverLat,
					const double& receiverLon,
					const double& receiverDepth);

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
	void createGreatCircle(const int& phase,
					const double& sourceLat,
					const double& sourceLon,
					const double& sourceDepth,
					const double& receiverLat,
					const double& receiverLon,
					const double& receiverDepth);

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
	void clear();

	//! \brief Returns true if the current GreatCirlce object has 
	//! been instantiated and is ready to be interrogated.
	//!
	//! Returns true if the current GreatCirlce object has 
	//! been instantiated and is ready to be interrogated.
	bool isValid() { return valid; };

	//! \brief Retrieve the phase specified in last call to createGreatCircle().
	//! 
	//! Retrieve the phase specified in last call to createGreatCircle().
	string getPhase() { return greatCircle->getPhaseString(); };
	
	//! \brief Retrieve the source-receiver separation, in radians.
	//! 
	//! Retrieve the source-receiver separation, in radians.
	//! @param distance the source-receiver separation is returned 
	//! in distance.
	void getDistance(double& distance) { distance = getDistance(); };

	//! \brief Retrieve the source-receiver separation, in radians.
	//!
	//! Retrieve the source-receiver separation, in radians.
	//! @return the source-receiver separation, in radians.
	double getDistance();
	
	//! \brief Retrieve horizontal offset below the source, in radians.
	//! 
	//! Retrieve horizontal offset below the source, in radians.
	//! This is the angular distance between the location of the 
	//! source and the source pierce point where the ray impinged
	//! on the headwave interface.
	//! @param dist the horizontal offset below the source, in radians.
	void getSourceDistance(double& dist);

	//! \brief Retrieve horizontal offset below the receiver, in radians.
	//! 
	//! Retrieve horizontal offset below the receiver, in radians.
	//! This is the angular distance between the location of the 
	//! receiver and the receiver pierce point where the ray impinged
	//! on the headwave interface.
	//! @param dist the horizontal offset below the receiver, in radians.
	void getReceiverDistance(double& dist);

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
	void getHeadwaveDistance(double& dist);

	//! \brief Retrieve horizontal distance traveled by the ray
	//! below the headwave interface, in radians.
	//! 
	//! Retrieve horizontal distance traveled by the ray
	//! below the headwave interface, in km.
	//! This is the sum of actual_path_increment(i) * R(i) where actual_path_increment(i) is the
	//! angular distance traveled by the ray in each angular
	//! distance increment along the head wave interface, and R(i)
	//! is the radius of the head wave interface in that same
	//! horizontal increment.
	//! @param dist the horizontal distance traveled by the ray
	//! below the headwave interface, in km.
	void getHeadwaveDistanceKm(double& dist);

	//! \brief Retrieve the total travel time for the GreatCircle,
	//! in seconds.  
	//!
	//! Retrieve the total travel time for the GreatCircle,
	//! in seconds.   
	//! @param travelTime the total travel time in seconds is returned 
	//! in travelTime.  If the GreatCircle is invalid, travelTime 
	//! will equal SLBMGlobals::NA_VALUE.
	void getTravelTime(double& travelTime);

	//! \brief Retrieve the total travel time and the 4 components that
	//! contribute to it for the current GreatCircle.
	//!
	//! Retrieve the total travel time and the 4 components that
	//! contribute to it for the current GreatCircle.  
	//! If the greatCircle is invalid, tTotal and all the 
	//! components will equal SLBMGlobals::NA_VALUE.
	//!
	//! @param tTotal the total travel time, in seconds.
	//! @param tSource the crustal travel time below the source, in seconds.
	//! @param tReceiver the crustal travel time below the receiver, in seconds.
	//! @param tHeadwave the head wave travel time, in seconds.
	//! @param tGradient the Zhao gradient correction term, in seconds.
	//! For GreatCircle objects that support Pg and Lg, this is always 0.
	void getTravelTimeComponents(double& tTotal, double& tSource, double& tReceiver, 
			double& tHeadwave, double& tGradient);

	//! \brief Retrieve the horizontal slowness, i.e., the derivative of travel time 
	//! wrt to receiver-source distance, in seconds/radian.  
	//!
	//! Retrieve the horizontal slowness, in seconds/radian. 
	//! @param slowness the derivative of travel time wrt to source latitude.
	void getSlowness(double& slowness);

	//! \brief Retrieve the horizontal slowness, i.e., the derivative of travel time 
	//! wrt to receiver-source distance, in seconds/radian.  
	//!
	//! Retrieve the horizontal slowness, in seconds/radian. 
	//! @param slowness the derivative of travel time wrt to source latitude.
	void get_dtt_ddist(double& slowness) { getSlowness(slowness); };
	double get_dtt_ddist() { double x; getSlowness(x); return x; };

	//! \brief Retrieve the derivative of travel time wrt to source latitude,
	//! in seconds/radian.  
	//!
	//! Retrieve the derivative of travel time wrt to source latitude,
	//! in seconds/radian. 
	//! @param dtt_dlat the derivative of travel time wrt to source latitude.
	void get_dtt_dlat(double& dtt_dlat);
	double get_dtt_dlat() { double x; get_dtt_dlat(x); return x; };

	//! \brief Retrieve the derivative of travel time wrt to source longitude,
	//! in seconds/radian.  
	//!
	//! Retrieve the derivative of travel time wrt to source longitude,
	//! in seconds/radian. 
	//! @param dtt_dlon the derivative of travel time wrt to source longitude.
	void get_dtt_dlon(double& dtt_dlon);
	double get_dtt_dlon() { double x; get_dtt_dlon(x); return x; };

	//! \brief Retrieve the derivative of travel time wrt to source depth,
	//! in seconds/km.  
	//!
	//! Retrieve the derivative of travel time wrt to source depth,
	//! in seconds/km. 
	//! @param dtt_ddepth the derivative of travel time wrt to source depth.
	void get_dtt_ddepth(double& dtt_ddepth);
	double get_dtt_ddepth() { double x; get_dtt_ddepth(x); return x; };

	////! \brief Retrieve the derivative of travel time wrt to source latitude,
	////! in seconds/radian.  
	////!
	////! Retrieve the derivative of travel time wrt to source latitude,
	////! in seconds/radian. 
	////! @param dtt_dlat the derivative of travel time wrt to source latitude.
	//void get_dtt_dlat_fast(double& dtt_dlat);

	////! \brief Retrieve the derivative of travel time wrt to source longitude,
	////! in seconds/radian.  
	////!
	////! Retrieve the derivative of travel time wrt to source longitude,
	////! in seconds/radian. 
	////! @param dtt_dlon the derivative of travel time wrt to source longitude.
	//void get_dtt_dlon_fast(double& dtt_dlon);

//	//! \brief Retrieve the derivative of horizontal slowness wrt to source-receiver distance,
//	//! in seconds/radian^2.
//	//!
//	//! Retrieve the derivative of horizontal slowness wrt to source-receiver distance,
//	//! in seconds/radian^2.
//	void get_dsh_ddist(double& dsh_ddist);
//	double get_dsh_ddist() { double x; get_dsh_ddist(x); return x; };
//
//	//! \brief Retrieve the derivative of horizontal slowness wrt to source latitude,
//	//! in seconds/radian^2.
//	//!
//	//! Retrieve the derivative of horizontal slowness wrt to source latitude,
//	//! in seconds/radian^2.
//	//! @param dsh_dlat the derivative of horizontal slowness wrt to source latitude.
//	void get_dsh_dlat(double& dsh_dlat);
//
//	//! \brief Retrieve the derivative of horizontal slowness wrt to source longitude,
//	//! in seconds/radian^2.
//	//!
//	//! Retrieve the derivative of horizontal slowness wrt to source longitude,
//	//! in seconds/radian^2.
//	//! @param dsh_dlon the derivative of horizontal slowness wrt to source longitude.
//	void get_dsh_dlon(double& dsh_dlon);
//
//	//! \brief Retrieve the derivative of horizontal slowness wrt to source depth,
//	//! in seconds/radian-km.
//	//!
//	//! Retrieve the derivative of horizontal slowness wrt to source depth,
//	//! in seconds/radian-km.
//	//! @param dsh_ddepth the derivative of horizontal slowness wrt to source depth.
//	void get_dsh_ddepth(double& dsh_ddepth);
//	double get_dsh_ddepth() { double x; get_dsh_ddepth(x); return x; }

	//! \brief Retrieve the weight assigned to each grid node that 
	//! was touched by the GreatCircle.
	//!
	//! Retrieve the weight assigned to each grid node that 
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
	//! @param nodeId the node IDs of all the grid nodes touched by the 
	//! current GreatCircle.
	//! 
	//! @param weight the weights of all the grid nodes touched by the 
	//! current GreatCircle.  Calling application must dimension this
	//! array large enough to handle any possible size.
	//! @param nWeights the number of elements in nodeId and weight.
	//! Calling application must dimension this
	//! array large enough to handle any possible size.
	void getWeights(int nodeId[], double weight[], int& nWeights);
	
	//! \brief Retrieve the weight assigned to each active node that 
	//! was touched by the GreatCircle.
	//!
	//! Retrieve the weight assigned to each active node that 
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
	//!
	//! @param nodeId the active node IDs of all the grid nodes touched by the 
	//! current GreatCircle.  These are active node ids, not grid node ids.  If 
	//! a grid node has weight but is not an active node, the nodeId will be -1.
	//! @param weight the weights of all the grid nodes touched by the 
	//! current GreatCircle.  Calling application must dimension this
	//! array large enough to handle any possible size.
	//! @param nWeights the number of elements in nodeId and weight.
	//! Calling application must dimension this
	//! array large enough to handle any possible size.
	void getActiveNodeWeights(int nodeId[], double weight[], int& nWeights);
	
	//! \brief Retrieve the weight assigned to each grid node that 
	//! was touched by the GreatCircle.
	//!
	//! Retrieve the weight assigned to each grid node that 
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
	//! current GreatCircle.  These are active node ids, not grid node ids.  If 
	//! a grid node has weight but is not an active node, the nodeId will be -1.
	//! @param weight the weights of all the grid nodes touched by the 
	//! current GreatCircle.  
	void getWeights(vector<int>& nodeId, vector<double>& weight);

	//! \brief Retrieve the weight assigned to each active node that 
	//! was touched by the GreatCircle.
	//!
	//! Retrieve the weight assigned to each active node that 
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
	//! @param nodeId the active node IDs
	//! of all the grid nodes touched by the current GreatCircle.
	//! @param weight the weights
	//! of all the grid nodes touched by the current GreatCircle.
	void getActiveNodeWeights(vector<int>& nodeId, vector<double>& weight);

	//! \brief Retrieve the node IDs and the interpolation
	//! coefficients for the source CrustalProfile.
	//!
	//! Retrieve the node IDs and the interpolation
	//! coefficients for the source CrustalProfile.
	//! For linear interpolation, nWeights will equal 3 but for
	//! natural neighbor interpolation nWeights will be variable
	//! number less than or equal to 5.
	//! The sum of the weights will equal 1.
	//! @param nodeids the node indexes of the grid nodes
	//! involved in interpolation.
	//! @param weights the weights associated with each nodeid
	//! @param nWeights the number of nodeids and weights returned.
	void getWeightsSource(int nodeids[], double weights[], int& nWeights);

	//! \brief Retrieve the active node IDs and the interpolation
	//! coefficients for the source CrustalProfile.
	//!
	//! Retrieve the active node IDs and the interpolation
	//! coefficients for the source CrustalProfile.
	//! For linear interpolation, nWeights will equal 3 but for
	//! natural neighbor interpolation nWeights will be variable
	//! but less than 10.
	//! The sum of the weights will equal 1.
	//! @param nodeids the node indexes of the grid nodes
	//! involved in interpolation.
	//! @param weights the weights associated with each nodeid
	//! @param nWeights the number of nodeids and weights returned.
	void getActiveNodeWeightsSource(int nodeids[], double weights[], int& nWeights);

	//! \brief Retrieve the node IDs and the interpolation
	//! coefficients for the receiver CrustalProfile.
	//! For linear interpolation, nWeights will equal 3 but for
	//! natural neighbor interpolation nWeights will be variable
	//! but less than 10.
	//! The sum of the weights will equal 1.
	//! @param nodeids the node indexes of the grid nodes
	//! involved in interpolation.
	//! @param weights the weights associated with each nodeid
	//! @param nWeights the number of nodeids and weights returned.
	void getWeightsReceiver(int nodeids[], double weights[], int& nWeights);

	//! \brief Retrieve the active node IDs and the interpolation
	//! coefficients for the receiver CrustalProfile.
	//! For linear interpolation, nWeights will equal 3 but for
	//! natural neighbor interpolation nWeights will be variable
	//! but less than 10.
	//! The sum of the weights will equal 1.
	//! @param nodeids the node indexes of the grid nodes
	//! involved in interpolation.
	//! @param weights the weights associated with each nodeid
	//! @param nWeights the number of nodeids and weights returned.
	void getActiveNodeWeightsReceiver(int nodeids[], double weights[], int& nWeights);

	//! \brief Returns a human-readable string representation
	//! of the GreatCircle object.
	//!
	//! Returns a human-readable string representation
	//! of the GreatCircle object.
	//! @param verbosity specifies the amount of information
	//! that is to be included in the return string.  Each 
	//! verbosity level includes all information in preceeding
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
	//! - 7 : Node hit count and node neighbors for every node
	//!       touched by any GreatCircle instantiated by this instance
	//!       of SlbmInterface.
	string toString(const int& verbosity);

	//! \brief Retrieve the number of Grid nodes in the Earth model.
	//! 
	//! Retrieve the number of Grid nodes in the Earth model.
	void getNGridNodes(int& n); 

	//! \brief Retrieve the number of Grid nodes in the Earth model.
	//!
	//! Retrieve the number of Grid nodes in the Earth model.
	int getNGridNodes();

	//! \brief Retrieve the number of LayerProfile objects positioned along
	//! the head wave interface.
	//!
	//! Retrieve the number of LayerProfile objects positioned along
	//! the head wave interface.  It is useful to call this method 
	//! before calling getGreatCircleData() since the value returned
	//! by this method will be the number of elements that will be 
	//! populated in parameters headWaveVelocity[], neighbors[] and
	//! coefficients[].
	void getNHeadWavePoints(int& nHeadWavePoints);

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
	void getGridData(
		const int& nodeId,
		double& latitude,
		double& longitude,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double gradient[2]);

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
	void getActiveNodeData(
		const int& nodeId,
		double& latitude,
		double& longitude,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double gradient[2]);

	//! \brief Modify the velocity and gradient information
	//! associated with a specified node in the Grid.
	//!
	//! Modify the velocity and gradient information
	//! associated with a specified node in the Grid.
	//! @param nodeId the node number of the grid point in the model.
	//! (zero based index).
	//! @param depths an array containing the depths of the tops of the
	//! layers, in km
	//! @param pvelocity an array containing the P velocities of all the 
	//! intervals at the specified grid node, in km/sec.
	//! @param svelocity an array containing the S velocities of all the 
	//! intervals at the specified grid node, in km/sec.
	//! @param gradient a 2-element array containing the P and S 
	//! velocity gradients in the mantle, in 1/sec.
	void setGridData(
		const int& nodeId,
		double depths[NLAYERS], 
		double pvelocity[NLAYERS], 
		double svelocity[NLAYERS],
		double gradient[2]);

	//! \brief Modify the depth, velocity and gradient information
	//! associated with a specified active node in the Grid.
	//!
	//! Modify the depth, velocity and gradient information
	//! associated with a specified active node in the Grid.
	//! @param nodeId the node number of the grid point in the model.
	//! (zero based index).
	//! @param depths an array containing the depths of the tops of the
	//! layers, in km
	//! @param pvelocity an array containing the P velocities of all the 
	//! intervals at the specified grid node, in km/sec.
	//! @param svelocity an array containing the S velocities of all the 
	//! intervals at the specified grid node, in km/sec.
	//! @param gradient a 2-element array containing the P and S 
	//! velocity gradients in the mantle, in 1/sec.
	void setActiveNodeData(
		const int& nodeId,
		double depths[NLAYERS], 
		double pvelocity[NLAYERS], 
		double svelocity[NLAYERS],
		double gradient[2]);

	//! \brief Retrieve information about the great circle path including the
	//! interface depths at source and receiver, the velocity profiles at the source
	//! and receiver, and mantle velocity and velocity gradient at points along the
	//! great circle path from source pierce point to receiver pierce point.
	//!
	//! Retrieve information about the great circle path including the
	//! interface depths at source and receiver, the velocity profiles at the source
	//! and receiver, and mantle velocity and velocity gradient at points along the
	//! great circle path from source pierce point to receiver pierce point.
	//! The caller must supply all of the arrays required by this method and retains
	//! ownership of those arrays.  This method assumes the arrays have been allocated
	//! with sufficient memory to hold the requested information and simply populates
	//! the supplied arrays.
	//! @param phase the phase supported by the current GreatCircle.  Will be one of 
	//! Pn, Sn, Pg, Lg.
	//! @param actual_path_increment the actual horizontal separation of the LayerProfile
	//! objects along the head wave interface, in radians.  
	//! @param sourceDepth a double array of length NLAYERS containing the depths of all the model
	//! interfaces below the source, in km.
	//! @param sourceVelocity a double array of length NLAYERS containing the P or S velocity
	//! of each interval below the source, in km/sec.
	//! @param receiverDepth a double array of length NLAYERS containing the depths of
	//! all the model interfaces below the receiver, in km.
	//! @param receiverVelocity a double array of length NLAYERS containing the P or S velocity
	//! of each interval below the receiver, in km/sec.
	//! @param npoints the number of points along the headwave path where velocity and gradient
	//! values are interpolated.
	//! @param headWaveVelocity a double array of length npoints containing the P or S
	//! velocity at the center of each horizontal segment between the source and the receiver, in km/sec.
	//! The first horizontal segment starts at the source, the last horizontal
	//! segment ends at the receiver, and each one is of size actual_path_increment.  The head
	//! wave velocities are interpolated at the center of each of these horizontal
	//! segments, just below the head wave interface.
	//! @param gradient a double array of length npoints containing the P or S velocity
	//! gradient in the mantle at the center of each horizontal segment of the head wave, in 1/sec.
	//! For Pg and Lg, the values will be SLBMGlobals::NA_VALUE.
	void getGreatCircleData(
		string& phase,
		double& actual_path_increment,
		double sourceDepth[NLAYERS],
		double sourceVelocity[NLAYERS],
		double receiverDepth[NLAYERS],
		double receiverVelocity[NLAYERS],
		int& npoints,
		double headWaveVelocity[],
		double gradient[]
		);

	//! \brief Retrieve the latitudes, longitudes and depths of all the profile positions
	//! along the headwave interface.
	//!
	//! Retrieve the latitudes, longitudes and depths of all the profile positions
	//! along the headwave interface.  Profile positions are located at the center of each segment
	//! of the head wave interface between the source and receiver.  The first position
	//! is located actual_path_increment/2 radians from the source, the last profile position is located
	//! actual_path_increment/2 radians from the receiver, and the others are spaced actual_path_increment radians apart.
	//! @param lat the latitude at the center of each headwave segment, in radians.
	//! @param lon the longitude at the center of each headwave segment, in radians.
	//! @param depth the depth of the headwave interface at the center of each headwave segment, in km.
	//! @param npoints the number of horizontal increments sampled along the
	//! head wave interface.
	void getGreatCircleLocations(double lat[], double lon[], double depth[], int& npoints);

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
	void getGreatCircleNodeInfo(
		int** neighbors,
		double** coefficients,
		const int& maxpoints,
		const int& maxnodes,
		int& npoints,
		int* nnodes
		);

	//! \brief Retrieve interpolated data from the earth model at a single
	//! specified latitude, longitude.
	//!
	//! Retrieve interpolated data from the earth model at a single
	//! specified latitude, longitude.
	//! @param lat the latitude where information is to be interpolated, 
	//! in radians.
	//! @param lon the longitude where information is to be interpolated, 
	//! in radians.
	//! @param nodeId the nodeIds of the grid nodes that were involved
	//! in the interpolation.  
	//! @param coefficients the interpolation coefficients that were applied to 
	//! the information from the neighboring grid nodes.
	//! @param nnodes the number of grid nodes involved in the interpolation.
	//! @param depth the depths of the tops of the interfaces in the Earth model, 
	//! in km.  There will be one of these for each layer of the model.
	//! @param pvelocity the P velocities of each layer of the model, in km/sec.  
	//! @param svelocity the S velocities of each layer of the model, in km/sec.  
	//! @param pgradient the mantle P velocity gradient, in 1/sec.
	//! @param sgradient the mantle S velocity gradient, in 1/sec.
	//! @return true if successful.  If not successful, nodeIds are all -1 and
	//! all other returned arrays are populated with SLBMGlobals::NA_VALUE.
	//! 
	void getInterpolatedPoint(
		const double& lat, 
		const double& lon,
		int* nodeId,
		double* coefficients,
		int& nnodes,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double& pgradient,
		double& sgradient
		);

	//! \brief Retrieve interpolated data from the earth model along a 
	//! transect defined by equal sized, 1 dimensional arrays of latitude and longitude.
	//!
	//! Retrieve interpolated data from the earth model along a 
	//! transect defined by equal sized, 1 dimensional arrays of latitude and longitude.
	//! @param lat the latitudes along the transect, in radians. 
	//! @param lon the longitudes along the transect, in radians. 
	//! @param nLatLon the number of interpolated points along the transect.
	//! @param neighbors the nodeIds of the grid nodes that were involved
	//! in the interpolations.  Caller should supply a 2D array with at least nLatLon x 5
	//! elements that will be populated with values.
	//! @param coefficients the interpolation coefficients that were applied to 
	//! the information from the neighboring grid nodes.  Caller should supply a
	//! 2D array with at least nLatLon x 5 elements that will be populated with values.
	//! @param nNeighbors a 1D array of ints with at least nLatLon elements that will be
	//! populated with the number of nodes and coefficients associated with each lat, lon.
	//! @param depth the depths of the tops of the interfaces in the Earth model, in km.
	//! @param pvelocity the P velocities of each layer of the model, in km/sec.  
	//! @param svelocity the S velocities of each layer of the model, in km/sec.  
	//! @param pgradient the mantle P velocity gradient, in 1/sec. 
	//! @param sgradient the mantle S velocity gradient, in 1/sec. 
	//! @param nInvalid the number of points that were out of model range.  
	//! For any points outside of the model range, nodeIds are all -1 and
	//! all other returned arrays are populated with SLBMGlobals::NA_VALUE.
	//! @return true if all points were in model range (nInvalid == 0).
	void getInterpolatedTransect(
		double lat[], 
		double lon[],
		const int& nLatLon,
		int** neighbors,
		double** coefficients,
		int* nNeighbors,
		double depth[][NLAYERS],
		double pvelocity[][NLAYERS],
		double svelocity[][NLAYERS],
		double pgradient[NLAYERS],
		double sgradient[NLAYERS],
		int& nInvalid
		);

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
	void initializeActiveNodes(const double& latmin, const double& lonmin, 
		const double& latmax, const double& lonmax);

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
	void initializeActiveNodes(const string& polygonFileName);

	//! \brief Specify a polygon that enclose the set of grid nodes that are to be considered active nodes.
	//!
	//! Specify a polygon that enclose the set of grid nodes that are to be considered active nodes.
	//! <p>
	//! Active nodes are defined as follows:  for each triangle in the
	//! tessellation, if any of the 3 nodes that define the triangle is
	//! within the polygon specified by this method, then
	//! all 3 nodes are defined to be active nodes.
	//!
	//! @param polygon a pointer to a polygon object that encloses the set of grid nodes
	//! that are to be considered active nodes.
	//!
	void initializeActiveNodes(GeoTessPolygon* polygon);

	//! \brief Specify a list of points that define a polygon that encloses the set of grid nodes
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
	void initializeActiveNodes(double* lat, double* lon, const int& npoints, const bool& inDegrees=true);

	//! \brief Specify a list of points that define a polygon that encloses the set of grid nodes
	//! that are to be considered active nodes.
	//!
	//! Specify a list of points that define a polygon that encloses the set of grid nodes
	//! that are to be considered active nodes.
	//! Active nodes are defined as follows:  for each triangle in the
	//! tessellation, if any of the 3 nodes that define the triangle is
	//! within the polygon specified by this method, then
	//! all 3 nodes are defined to be active nodes.
	//! <p>If the last point and first point are not coincident, then the polygon is 'closed'
	//! by connecting the first and last point by an edge.
	//!
	//! @param unitVectors a 2D array of doubles specifying the list of unit vectors
	//! that define the polygon.
	//!
	void initializeActiveNodes(vector<double*>& unitVectors);

	//! \brief Retrieve the number of active nodes in the Grid.
	//!
	//! Retrieve the number of active nodes in the Grid.  
	int getNActiveNodes();

	//! \brief Clear all active nodes.
	//! Clear all active nodes.
	void clearActiveNodes();

	//! \brief Retrieve the grid node ID that corresponds to a specified
	//! active node ID.
	//!
	//! Retrieve the grid node ID that corresponds to a specified
	//! active node ID.
	int getGridNodeId(int activeNodeId);

	//! \brief Retrieve the active node ID that corresponds to a specified
	//! grid node ID.
	//!
	//! Retrieve the active node ID that corresponds to a specified
	//! grid node ID.
	int getActiveNodeId(int gridNodeId);

	//! \brief Retrieve the number of times that the specified node has been 
	//! 'touched' by a GreatCircle object.
	//!
	//! Retrieve the number of times that the specified node has been 'touched' 
	//! by a GreatCircle object.  The hit count of each node is initialized in the 
	//! loadVelocityModel() method.  Every time the getWeights() method is called 
	//! for a particular GreatCircle object, all the nodeIds that contribute any 
	//! weight to that GreatCircle object have their hit count incremented by one.
	void getNodeHitCount(const int& nodeId, int& hitCount);

	//! \brief Clear the node hit count by setting the hit count of every
	//! node to zero.
	//!
	//! Clear the node hit count by setting the hit count of every
	//! node to zero.
	void clearNodeHitCount();

	//! \brief Retrieve the node IDs of the nodes that surround the 
	//! specified node.
	//!
	//! Retrieve the node IDs of the nodes that surround the specified node.
	//! <p>The caller must supply int array neighbors which is dimensioned large
	//! enough to hold the maximum number of neighbors that a node can have, 
	//! which is 8.  The actual number of neighbors is returned in nNeighbors.
	void getNodeNeighbors(const int& nid, int neighbors[], int& nNeighbors);

	//! \brief Retrieve the active node IDs of the nodes that surround the 
	//! specified active node.
	//!
	//! Retrieve the active node IDs of the active nodes that surround the specified active node.
	//! <p>The caller must supply int array neighbors which is dimensioned large
	//! enough to hold the maximum number of neighbors that a node can have, 
	//! which is 8.  The actual number of neighbors is returned in nNeighbors.
	void getActiveNodeNeighbors(const int& nid, int neighbors[], int& nNeighbors);

	//! \brief Retrieve the node IDs of the nodes that surround the 
	//! specified node.
	//!
	//! Retrieve the node IDs of the nodes that surround the specified node.
	void getNodeNeighbors(const int& nid, vector<int>& neighbors);

	//! \brief Retrieve active the node IDs of the active nodes that surround the 
	//! specified active node.
	//!
	//! Retrieve the active node IDs of the active nodes that surround the specified active node.
	void getActiveNodeNeighbors(const int& nid, vector<int>& neighbors);

	//! \brief Retrieve the node IDs of the nodes that surround the 
	//! specified node.
	//!
	//! Retrieve the node IDs of the nodes that surround the specified node.
	//! <p>The caller must supply int array neighbors which is dimensioned large
	//! enough to hold the maximum number of neighbors that a node can have, 
	//! which is 8.  The actual number of neighbors is returned in nNeighbors.
	void getNodeNeighborInfo(const int& nid, int neighbors[], double distance[],
		double azimuth[], int& nNeighbors);

	//! \brief Retrieve the active node IDs of the nodes that surround the 
	//! specified node.
	//!
	//! Retrieve the active node IDs of the nodes that surround the specified node.
	//! <p>The caller must supply int array neighbors which is dimensioned large
	//! enough to hold the maximum number of neighbors that a node can have, 
	//! which is 8.  The actual number of neighbors is returned in nNeighbors.
	void getActiveNodeNeighborInfo(const int& nid, int neighbors[], double distance[],
		double azimuth[], int& nNeighbors);

	//! \brief Retrieve the active node IDs of the nodes that surround the 
	//! specified node.
	//!
	//! Retrieve the active node IDs of the nodes that surround the specified node.
	//! <p>The caller must supply int array neighbors which is dimensioned large
	//! enough to hold the maximum number of neighbors that a node can have, 
	//! which is 8.  The actual number of neighbors is returned in nNeighbors.
	void getActiveNodeNeighborInfo(const int& nid, 
			vector<int>& neighbors, vector<double>& distance, vector<double>& azimuth);

	//! \brief Retrieve the angular separation of two grid nodes, in radians.
	//! 
	//! Retrieve the angular separation of two grid nodes, in radians.
	void getNodeSeparation(const int& node1, const int& node2, double& distance);

	//! \brief Retrieve the azimuth from grid node1 to grid node2, radians.
	//! 
	//! Retrieve the azimuth from grid node1 to grid node2, radians.
	void getNodeAzimuth(const int& node1, const int& node2, double& azimuth);

	//! \brief Retrieve a pointer to the GreatCircle object.
	//! 
	//! Retrieve a pointer to the GreatCircle object.  This is
	//! not the recommended method for interacting with GreatCircle objects
	//! but will allow inquisitive applications direct access to all the public 
	//! methods of GreatCircle and all the various objects accessible through
	//! it (CrustalProfile, LayerProfile, etc.).  This method will not be 
	//! available through any interfaces other than the c++ interface.
	GreatCircle* getGreatCircleObject();

	//! \brief Retrieve a pointer to the Grid object.
	//! 
	//! Retrieve a pointer to the Grid object. This method will not be 
	//! available through any interfaces other than the c++ interface.
	Grid* getGridObject() { return grid; };

	//! \brief Retrieve the travel time uncertainty in sec
	//! for specified phase, distance (in radians).
	//! 
	//! Retrieve the travel time uncertainty in sec
	//! for specified phase, distance.
	//! @param phase 0:Pn, 1:Sn, 2:Pg or 3:Lg
	//! @param distance source-receiver separation in radians.
	//! @param uncert returns the uncertainty in sec
	void getTravelTimeUncertainty( const int& phase, const double& distance, double& uncert );

	//! \brief Retrieve travel time uncertainty in sec 
	//! using the phase and distance specified in last call to getGreatCircle().
	//!
	//! Retrieve travel time uncertainty in sec 
	//! using the phase and distance specified in last call to getGreatCircle().
	//! @param travelTimeUncertainty uncertainty of the travel time in seconds.
	void getTravelTimeUncertainty(double& travelTimeUncertainty);

	//! \brief Retrieve the slowness uncertainty in sec/radian
	//! for specified phase, distance (in radians).
	//!
	//! Retrieve the slowness uncertainty in sec/radian
	//! for specified phase, distance.
	//! @param phase 0:Pn, 1:Sn, 2:Pg or 3:Lg
	//! @param distance source-receiver separation in radians.
	//! @param uncert returns the uncertainty in sec/radian
	void getSlownessUncertainty( const int& phase, const double& distance, double& uncert );

	//! \brief Retrieve uncertainty of the horizontal slowness, in seconds/radian 
	//! using the phase and distance specified in last call to getGreatCircle().
	//!
	//! Retrieve uncertainty of the horizontal slowness, in seconds/radian, 
	//! using the phase and distance specified in last call to getGreatCircle().
	//! @param slownessUncertainty uncertainty of the horizontal slowness, in seconds/radian.
	void getSlownessUncertainty(double& slownessUncertainty);

	string getUncertaintyTable(const int& attribute, const int& phase);

	string getUncertaintyFileFormat(const int& attribute, const int& phase);

	//! \brief Retrieve some of the parameters that contribute to the calculation of 
	//! of total travel time using the Zhao algorithm.
	//!
	//! Retrieve some of the parameters that contribute to the calculation of 
	//! of total travel time using the Zhao algorithm.  This method only returns
	//! meaningful results for phases Pn and Sn.  For Pg and Lg, all the parameters
	//! of type double are returned with values SLBMGlobals::NA_VALUE and udSign is
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
	void getZhaoParameters(double& Vm, double& Gm, double& H, double& C, double& Cm, int& udSign);

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
	void getPgLgComponents(double& tTotal, 
											  double& tTaup, double& tHeadwave, 
											  double& pTaup, double& pHeadwave, 
											  double& trTaup, double& trHeadwave);

	//! \brief Set the value of chMax.  c is the zhao c parameter and h is 
	//! the turning depth of the ray below the moho.  Zhao method only valid 
	//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
	//!
	//! Set the value of chMax.  c is the zhao c parameter and h is 
	//! the turning depth of the ray below the moho.  Zhao method only valid 
	//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
	//! This call modifies global parameter SLBMGlobals::CH_MAX
	void static setCHMax(const double& chMax);

	//! \brief Retrieve the current value of chMax.  c is the zhao c parameter 
	//! and h is the turning depth of the ray below the moho.  Zhao method only valid 
	//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
	//!
	//! Retrieve the current value of chMax.  c is the zhao c parameter and h is 
	//! the turning depth of the ray below the moho.  Zhao method only valid 
	//! for c*h << 1. When c*h > chMax, then slbm will throw an exception.
	//! This call retrieves global parameter SLBMGlobals::CH_MAX
	void static getCHMax(double& chMax);

	//! \brief Retrieve the average P or S wave mantle velocity that is specified
	//! in the model input file, in km/sec.  
	//! 
	//! Retrieve the average P or S wave mantle velocity that is specified
	//! in the model input file.  This value is used in the calculation of
	//! the Zhao c parameter.  
	//! @param type specify either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity the P or S wave velocity is returned in this parameter,
	//! in km/sec.
	void getAverageMantleVelocity(const int& type, double& velocity);

	//! \brief Set the average P or S wave mantle velocity that is recorded
	//! in the model input file, in km/sec.  
	//! 
	//! Set the average P or S wave mantle velocity that is specified
	//! in the model input file.  This value is used in the calculation of
	//! the Zhao c parameter.  
	//! @param type specify either SLBMGlobals::PWAVE or SLBMGlobals::SWAVE.
	//! @param velocity the P or S wave velocity that is to be set,
	//! in km/sec.  This value will be stored in the model file, if the 
	//! model file is written to file by a call to saveVelocityModel()
	//! subsequent to a call to this method.
	void setAverageMantleVelocity(const int& type, const double& velocity);

	//! \brief Retrieve the tessellation ID of the model currently in memory.
	//! 
	//! Retrieve the tessellation ID of the model currently in memory.
	void getTessId(string& tessId);

	//! \brief Retrieve the fraction of the path length of the current 
	//! GreatCircle object that is within the currently defined active region.
	//!
	//! Retrieve the fraction of the path length of the current 
	//! GreatCircle object that is within the currently defined active region.
	void getFractionActive(double& fractionActive);

	//! \brief Set the maximum source-receiver separation for Pn/Sn phase, 
	//! in radians.
	//!
	//! Set the maximum source-receiver separation for Pn/Sn phase, 
	//! in radians.  Source-receiver separations greater than the specified
	//! value will result in an exception being thrown in createGreatCircle().
	//! Default value is PI radians.
	void static setMaxDistance(const double& maxDistance);

	//! \brief Retrieve the current value for the maximum source-receiver 
	//! separation, in radians.
	//!
	//! Retrieve the current value for the maximum source-receiver 
	//! separation, in radians.
	void static getMaxDistance(double& maxDistance);

	//! \brief Set the maximum source depth for Pn/Sn phase, 
	//! in km.
	//!
	//! Set the maximum source depth for Pn/Sn phase, 
	//! in km.  Source depths greater than the specified
	//! value will result in an exception being thrown in createGreatCircle().
	//! Default value is 9999 km.
	void static setMaxDepth(const double& maxDepth);

	//! \brief Retrieve the current value for the maximum source depth, in km.
	//!
	//! Retrieve the current value for the maximum source depth, in km.
	void static getMaxDepth(double& maxDepth);

	//! \brief Retrieve a table that lists the number of instances of various
	//! SLBM classes that are currently instantiated.
	//!
	//! Retrieve a table that lists the number of instances of various
	//! SLBM classes that are currently instantiated.  Very useful for 
	//! debugging memory leaks.
	string getClassCount();

	//! \brief A string containing the path to the SLBM model.
	//! 
	//! A string containing the path to the SLBM model used for locating
	//! phase-specific model error data files.
	const string& getModelPath() const;

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
	//! @param naValue value to return if result is invalid.
	void getDistAz(const double& aLat, const double& aLon, 
		const double& bLat, const double& bLon, 
		double& distance, double& azimuth, const double& naValue);

	//! \brief Find point B that is the specified distance and azimuth 
	//! from point A, in radians.
	//!
	//! Find point B that is the specified distance and azimuth
	//! from point A.  All quantities are in radians.
	//!
	//! @param aLat latitude of point A, in radians
	//! @param aLon longitude of point A, in radians
	//! @param distance angular distance from point A to point B, in radians
	//! @param azimuth azimuth from pointA to point B, clockwise from north, in radians.
	//! @param bLat (output) latitude of point B in radians
	//! @param bLon (output) latitude of point B in radians
	void movePoint(const double& aLat, const double& aLon, 
		const double& distance, const double& azimuth,
		double& bLat, double& bLon);
	
	//! \brief Retrieve the latitude  and longitude of the moho pierce point below the source,
	//! in radians.
	//!
	//! Retrieve the latitude  and longitude of the moho pierce point below the source,
	//! in radians.  For Pg, Lg and sources in the mantle an exception is thrown.
	//! @param lat the latitude of the source pierce point, in radians.
	//! @param lon the longitude of the source pierce point, in radians.
	//! @param depth moho depth in km below sea level
	void getPiercePointSource(double& lat, double& lon, double& depth);

	//! \brief Retrieve the latitude  and longitude of the moho pierce point below the receiver,
	//! in radians.
	//!
	//! Retrieve the latitude  and longitude of the moho pierce point below the receiver,
	//! in radians.  For Pg, Lg  an exception is thrown.
	//! @param lat the latitude of the receiver pierce point, in radians.
	//! @param lon the longitude of the receiver pierce point, in radians.
	//! @param depth moho depth in km below sea level
	void getPiercePointReceiver(double& lat, double& lon, double& depth);

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
	void getGreatCirclePoints(					
		const double& aLat,
		const double& aLon,
		const double& bLat,
		const double& bLon,
		const int& npoints,
		double latitude[],
		double longitude[]);

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
	void getGreatCirclePointsOnCenters(					
		const double& aLat,
		const double& aLon,
		const double& bLat,
		const double& bLon,
		const int& npoints,
		double latitude[],
		double longitude[]);

	//! \brief Change the value of step change in distance used to compute
	//! horizontal derivatives(in radians).
	//!
	//! Change the value of step change in distance used to compute
	//! horizontal derivatives (radians)
	void setDelDistance(const double& del_distance);

	//! \brief Retrieve the value of step change in distance used to compute
	//! horizontal derivatives (radians)
	//!
	//! Retrieve the value of step change in distance used to compute
	//! horizontal derivatives (radians)
	void getDelDistance(double& del_distance);
	
	//! \brief Change the value of step change in depth used to compute
	//! depth derivatives (km)
	//!
	//! Change the value of step change in depth used to compute
	//! depth derivatives (km)
	void setDelDepth(const double& del_depth);

	//! \brief Retrieve the value of step change in depth used to compute
	//! depth derivatives (km)
	//!
	//! Retrieve the value of step change in depth used to compute
	void getDelDepth(double& del_depth);

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
	void setPathIncrement(const double& pathIncrement);

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
	void getPathIncrement(double& pathIncrement);

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
	//! @return the current value of the spacing of great circle nodes
	//! along the head wave interface, in radians.
	double getPathIncrement();

	string getModelString() { return grid->toString(); }

protected:

	//! \brief The Grid object that stores the velocity model.
	//! 
	//! The Grid object that stores the velocity model.
	Grid* grid;

	//! \brief The most recently requested GreatCircle object.
	//! 
	//! The most recently requested GreatCircle object.
	GreatCircle* greatCircle;
	
	//! c is the zhao c parameter and h is the turning depth of the 
	//! ray below the moho.  Zhao method only valid for c*h << 1.
	//! When c*h > ch_max, then slbm will throw an exception.
	static double CH_MAX;

	//! \brief true if the current GreatCirlce object has 
	//! been instantiated and is ready to be interrogated.
	//!
	//! true if the current GreatCirlce object has 
	//! been instantiated and is ready to be interrogated.
	bool valid;

	//! deletes current greatCircle object and sets ttHminus, ttHplus,
	//! ttZplus and ttHZplus equal to NA_VALUE.
	void clearGreatCircles();

private:

	// copies of the source and receiver lats and lons that were
	// passed to createGreatCircle the last time it was called.
	// Used to compute distance in instances when createGreateCircle
	// fails. Units are radians.
	double srcLat, srcLon, rcvLat, rcvLon;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline void SlbmInterface::createGreatCircle(
					const string& p,
					const double& sourceLat,
					const double& sourceLon,
					const double& sourceDepth,
					const double& receiverLat,
					const double& receiverLon,
					const double& receiverDepth)
{
	int iphase = (p=="Pn" ? Pn : (p=="Sn" ? Sn : (p=="Pg" ? Pg : (p=="Lg" ? Lg : -1))));
	if (iphase == -1)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		 os << endl << "ERROR in SlbmInterface::createGreatCircle" << endl
			<< p << " is not a recognized phase.  Must be one of Pn, Sn, Pg, Lg" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),112);
	}

	createGreatCircle(
	iphase,
	sourceLat,
	sourceLon,
	sourceDepth,
	receiverLat,
	receiverLon,
	receiverDepth
	);
}

inline GreatCircle* SlbmInterface::getGreatCircleObject()
{
	return greatCircle;
}

inline void SlbmInterface::get_dtt_dlat(double& value)
{
	if (!isValid())
	{
		value = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::get_dtt_dlat" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->get_dtt_dlat(value);
}

inline void SlbmInterface::get_dtt_dlon(double& value)
{
	if (!isValid())
	{
		value = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::get_dtt_dlon" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->get_dtt_dlon(value);
}

inline void SlbmInterface::get_dtt_ddepth(double& value)
{
	if (!isValid())
	{
		value = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::get_dtt_ddepth" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->get_dtt_ddepth(value);
}

//inline void SlbmInterface::get_dtt_dlat_fast(double& value)
//{
//	if (!isValid())
//	{
//		value = NA_VALUE;
//		ostringstream os;
//		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
//		  os << endl << "ERROR in SlbmInterface::get_dtt_dlat" << endl
//			<< "GreatCircle is invalid." << endl
//			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
//		throw SLBMException(os.str(),113);
//	}
//	greatCircle->get_dtt_dlat_fast(value);
//}
//
//inline void SlbmInterface::get_dtt_dlon_fast(double& value)
//{
//	if (!isValid())
//	{
//		value = NA_VALUE;
//		ostringstream os;
//		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
//		  os << endl << "ERROR in SlbmInterface::get_dtt_dlon" << endl
//			<< "GreatCircle is invalid." << endl
//			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
//		throw SLBMException(os.str(),113);
//	}
//	greatCircle->get_dtt_dlon_fast(value);
//}
//
//inline void SlbmInterface::get_dsh_ddist(double& value)
//{
//	if (!isValid())
//	{
//		value = NA_VALUE;
//		ostringstream os;
//		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
//		  os << endl << "ERROR in SlbmInterface::get_dsh_ddist" << endl
//			<< "GreatCircle is invalid." << endl
//			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
//		throw SLBMException(os.str(),113);
//	}
//	greatCircle->get_dsh_ddist(value);
//}
//
//inline void SlbmInterface::get_dsh_dlat(double& value)
//{
//	if (!isValid())
//	{
//		value = NA_VALUE;
//		ostringstream os;
//		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
//		  os << endl << "ERROR in SlbmInterface::get_dsh_dlat" << endl
//			<< "GreatCircle is invalid." << endl
//			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
//		throw SLBMException(os.str(),113);
//	}
//	greatCircle->get_dsh_dlat(value);
//}
//
//inline void SlbmInterface::get_dsh_dlon(double& value)
//{
//	if (!isValid())
//	{
//		value = NA_VALUE;
//		ostringstream os;
//		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
//		  os << endl << "ERROR in SlbmInterface::get_dsh_dlon" << endl
//			<< "GreatCircle is invalid." << endl
//			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
//		throw SLBMException(os.str(),113);
//	}
//	greatCircle->get_dsh_dlon(value);
//}
//
//inline void SlbmInterface::get_dsh_ddepth(double& value)
//{
//	if (!isValid())
//	{
//		value = NA_VALUE;
//		ostringstream os;
//		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
//		  os << endl << "ERROR in SlbmInterface::get_dsh_ddepth" << endl
//			<< "GreatCircle is invalid." << endl
//			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
//		throw SLBMException(os.str(),113);
//	}
//	greatCircle->get_dsh_ddepth(value);
//}

inline void SlbmInterface::getSlowness(double& value)
{
	if (!isValid())
	{
		value = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getSlowness" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->get_dtt_ddist(value);
}

inline double SlbmInterface::getDistance()
{
	if (greatCircle == NULL)
	{
		Location s(srcLat, srcLon, 0.);
		Location r(rcvLat, rcvLon, 0.);
		return s.distance(r);
	}
	else
		return greatCircle->getDistance();
}

inline void SlbmInterface::getSourceDistance(double& distance)
{
	if (!isValid())
	{
		distance = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getSourceDistance" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	distance = greatCircle->getSourceDistance();
}

inline void SlbmInterface::getReceiverDistance(double& distance)
{
	if (!isValid())
	{
		distance = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getReceiverDistance" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	distance = greatCircle->getReceiverDistance();
}

inline void SlbmInterface::getHeadwaveDistance(double& distance)
{
	if (!isValid())
	{
		distance = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getHeadwaveDistance" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	distance = greatCircle->getHeadwaveDistance();
}

inline void SlbmInterface::getHeadwaveDistanceKm(double& distance)
{
	if (!isValid())
	{
		distance = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getHeadwaveDistanceKm" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	distance = greatCircle->getHeadwaveDistanceKm();
}

inline void SlbmInterface::getTravelTime(double& tTotal)
{
	if (!isValid())
	{
		tTotal = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getTravelTime" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	tTotal=greatCircle->getTravelTime();
}

inline void SlbmInterface::getTravelTimeComponents(
			double& tTotal, double& tSource, double& tReceiver, 
			double& tHeadwave, double& tGradient)
{
	if (!isValid())
	{
		tTotal = NA_VALUE;
		tSource = NA_VALUE;
		tReceiver = NA_VALUE;
		tHeadwave = NA_VALUE;
		tGradient = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getTravelTimeComponents" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->getTravelTime(tTotal, tSource, tReceiver, 
		tHeadwave, tGradient);
}

inline void SlbmInterface::getWeights(int nodeId[], double weight[], int& nWeights)
{
	if (!isValid())
	{
		nWeights=-1;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getWeights" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->getWeights(nodeId, weight, nWeights);
}

inline void SlbmInterface::getActiveNodeWeights(int nodeId[], double weight[], int& nWeights)
{
	getWeights(nodeId, weight, nWeights);
	for (int i=0; i<nWeights; ++i)
		nodeId[i] = grid->getActiveNodeId(nodeId[i]);
}

inline void SlbmInterface::getWeights(
			vector<int>& nodeId, vector<double>& weight)
{
	if (!isValid())
	{
		nodeId.clear();
		weight.clear();
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getWeights" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->getWeights(nodeId, weight);
}

inline void SlbmInterface::getActiveNodeWeights(
			vector<int>& nodeId, vector<double>& weight)
{
	getWeights(nodeId, weight);
	for (int i=0; i<(int)nodeId.size(); ++i)
		nodeId[i] = grid->getActiveNodeId(nodeId[i]);
}

inline void SlbmInterface::getWeightsSource(int nodeids[], double weights[], int& nWeights)
{
	if (!isValid())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getWeightsSource" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->getSourceProfile()->getWeights(nodeids, weights, nWeights);
}

inline void SlbmInterface::getActiveNodeWeightsSource(int nodeids[], double weights[], int& nWeights)
{
	getWeightsSource(nodeids, weights, nWeights);
	for (int i=0; i<nWeights; ++i)
		nodeids[i] = grid->getActiveNodeId(nodeids[i]);
}

inline void SlbmInterface::getWeightsReceiver(int nodeids[], double weights[], int& nWeights)
{
	if (!isValid())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getWeightsReceiver" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	greatCircle->getReceiverProfile()->getWeights(nodeids, weights, nWeights);
}

inline void SlbmInterface::getActiveNodeWeightsReceiver(int nodeids[], double weights[], int& nWeights)
{
	getWeightsReceiver(nodeids, weights, nWeights);
	for (int i=0; i<nWeights; ++i)
		nodeids[i] = grid->getActiveNodeId(nodeids[i]);
}

inline string SlbmInterface::toString(const int& verbosity) 
{ 
	if (!isValid())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::toString" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	ostringstream os;
	if (verbosity > 0)
	{
		os << endl
			<< "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl
			<< "Great Circle " << endl << endl
			<< greatCircle->toString(verbosity); 
	
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) 
			<< setprecision(4);
		double slowness;
		getSlowness(slowness);
		os << endl << "Horizontal slowness = "
			<< slowness << " sec/radian"
			<< endl << endl;
	}

	return os.str();
}

inline void SlbmInterface::getGreatCircleData(
		string& phase,
		double& actual_path_increment,
		double sourceDepth[NLAYERS],
		double sourceVelocity[NLAYERS],
		double receiverDepth[NLAYERS],
		double receiverVelocity[NLAYERS],
		int& npoints,
		double headWaveVelocity[],
		double gradient[]
		)
{
	if (!isValid())
	{
		phase = "";
		actual_path_increment = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getGreatCircleData" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	int p;
	greatCircle->getData(p, actual_path_increment, 
		sourceDepth, sourceVelocity, receiverDepth,	receiverVelocity, 
		npoints, headWaveVelocity, gradient);
	phase = (p==Pn ? "Pn" : (p==Sn ? "Sn" : (p==Pg ? "Pg" : (p==Lg ? "Lg" 
		: "unknown phase"))));
}

inline void SlbmInterface::getGreatCircleNodeInfo(
	int** neighbors,
	double** coefficients,
	const int& maxpoints,
	const int& maxnodes,
	int& npoints,
	int* nnodes
	)
{
	if (!isValid())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getGreatCircleNodeInfo" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	greatCircle->getNodeInfo(neighbors, coefficients, maxpoints, maxnodes, npoints, nnodes);
}

inline void SlbmInterface::getGreatCircleLocations(
		double lat[],
		double lon[],
		double depth[],
		int& npoints
		)
{
	if (!isValid())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getGreatCircleData" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	npoints = greatCircle->getNProfiles();
	Location loc;
	for (int i=0; i<greatCircle->getNProfiles(); i++)
	{
		// get the location of this layer profile
		greatCircle->getLayerProfileLocation(i, loc);
		lat[i] = loc.getLat();
		lon[i] = loc.getLon();
		depth[i] = loc.getDepth();
	}
}


inline void SlbmInterface::getInterpolatedPoint(
		const double& lat, 
		const double& lon,
		int* nodeId,
		double* coefficients,
		int& nNeighbors,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double& pgradient,
		double& sgradient
		)
{
	Location location(lat, lon, 0.);
	QueryProfile* profile = grid->getQueryProfile(location);
	profile->getData(nodeId, coefficients, nNeighbors, depth, pvelocity, svelocity, pgradient, sgradient);
	delete profile;
}

inline void SlbmInterface::getInterpolatedTransect(
		double lat[], 
		double lon[],
		const int& nLatLon,
		int** nodeId,
		double** coefficients,
		int* nNeighbors,
		double depth[][NLAYERS],
		double pvelocity[][NLAYERS],
		double svelocity[][NLAYERS],
		double pgradient[NLAYERS],
		double sgradient[NLAYERS],
		int& nInvalid
		)
{
	nInvalid = 0;
	for (int i=0; i<nLatLon; i++)
	{
		try
		{
			getInterpolatedPoint(lat[i], lon[i], nodeId[i],
				coefficients[i], nNeighbors[i], depth[i], pvelocity[i], svelocity[i],
				pgradient[i], sgradient[i]);
		}
		catch (SLBMException ex)
		{
			for (int j=0; j<nNeighbors[i]; j++)
			{
				nodeId[i][j] = -1;
				coefficients[i][j] = NA_VALUE;
			}

			for (int j=0; j<NLAYERS; j++)
			{
				depth[i][j] = NA_VALUE;
				pvelocity[i][j] = NA_VALUE;
				svelocity[i][j] = NA_VALUE;
			}

			pgradient[i] = NA_VALUE;
			sgradient[i] = NA_VALUE;

			++nInvalid;
		}
	}
}

inline void SlbmInterface::getNGridNodes(int& n) 
{ 
	if (!grid)	
	{
		n = -1;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNGridNodes" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
    n = grid->getNNodes();
}

inline int SlbmInterface::getNGridNodes()
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNGridNodes" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
    return grid->getNNodes();
}

inline void SlbmInterface::getNHeadWavePoints(int& nHeadWavePoints) 
{ 
	if (!greatCircle)	
	{
		nHeadWavePoints = -1;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNHeadWavePoints" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
    nHeadWavePoints = greatCircle->getNProfiles(); 
}

//inline void SlbmInterface::getMaxInterpotionNodes(int& maxNodes)
//{
//	if (!greatCircle)
//	{
//		nHeadWavePoints = -1;
//		ostringstream os;
//		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
//		  os << endl << "ERROR in SlbmInterface::getMaxInterpotionNodes" << endl
//			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
//			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
//		throw SLBMException(os.str(),113);
//	}
//
//	maxNodes = 1;
//	for (int i=0; i<greatCircle->getNProfiles(); i++)
//		if (greatCircle->profiles[i]->getNodes().size() > maxNodes)
//			maxNodes = greatCircle->profiles[i]->getNodes().size();
//}

inline void SlbmInterface::getTravelTimeUncertainty( const int& phase, const double& distance, double& uncert ) 
{ 
	if (grid->getUncertainty()[phase][TT] == NULL)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		os << endl << "ERROR in SlbmInterface::getTravelTimeUncertainty" << endl
			<< "Uncertainty object is invalid." << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),602);
	}
	else
	{
		uncert =  grid->getUncertainty()[phase][TT]->getUncertainty( distance );
	}
}

inline void SlbmInterface::getTravelTimeUncertainty(double& travelTimeUncertainty)
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getTravelTimeUncertainty" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}

	getTravelTimeUncertainty(greatCircle->getPhase(), 
		greatCircle->getDistance(), travelTimeUncertainty);
}

inline void SlbmInterface::getSlownessUncertainty( const int& phase, const double& distance, double& uncert ) 
{ 
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getSlownessUncertainty" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}

	if (grid->getUncertainty()[phase][SH] == NULL)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		os << endl << "ERROR in SlbmInterface::getSlownessUncertainty" << endl
			<< "Uncertainty object is invalid.." << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),602);
	}
	else
	{
		// retrieve slowness uncertainty in sec/radian
		uncert =  grid->getUncertainty()[phase][SH]->getUncertainty( distance );
	}
}

inline void SlbmInterface::getSlownessUncertainty(double& slownessUncertainty)
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getSlownessUncertainty" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}

	getSlownessUncertainty(greatCircle->getPhase(), 
		greatCircle->getDistance(), slownessUncertainty);
}

inline 	string SlbmInterface::getUncertaintyTable(const int& phase, const int& attribute)
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getUncertaintyTable" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}

	if (grid->getUncertainty()[phase][attribute] == NULL)
	{
		ostringstream os;
		os << "No uncertainty information is available for phase " <<
				Uncertainty::getPhase(phase) << " attribute " <<
				Uncertainty::getAttribute(attribute) << endl;
		return os.str();
	}
	return grid->getUncertainty()[phase][attribute]->toStringTable();
}

inline 	string SlbmInterface::getUncertaintyFileFormat(const int& phase, const int& attribute)
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getUncertaintyFileFormat" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}

	if (grid->getUncertainty()[phase][attribute] == NULL)
	{
		ostringstream os;
		os << "No uncertainty information is available for phase " << phase << " attribute " << attribute << endl;
		return os.str();
	}
	return grid->getUncertainty()[phase][attribute]->toStringFile();
}

inline 	void SlbmInterface::getZhaoParameters(double& Vm, double& Gm, double& H, double& C, double& Cm, int& udSign)
{
	if (!greatCircle)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getZhaoParameters" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
    greatCircle->getZhaoParameters(Vm, Gm, H, C, Cm, udSign); 
}

inline 	void SlbmInterface::getPgLgComponents(double& tTotal, 
											  double& tTaup, double& tHeadwave, 
											  double& pTaup, double& pHeadwave, 
											  double& trTaup, double& trHeadwave)
{
	if (!greatCircle)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getPgLgComponents" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
    greatCircle->getPgLgComponents(tTotal, tTaup, tHeadwave, 
		pTaup, pHeadwave, trTaup, trHeadwave); 
}


inline 	void SlbmInterface::getNodeNeighbors(const int& nid, int neighbors[], int& nNeighbors)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNodeNeighbors" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getNodeNeighbors(nid, neighbors, nNeighbors);
}

inline 	void SlbmInterface::getActiveNodeNeighbors(const int& nid, int neighbors[], int& nNeighbors)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getActiveNodeNeighbors" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getActiveNodeNeighbors(nid, neighbors, nNeighbors);
}

inline 	void SlbmInterface::getNodeNeighbors(const int& nid, vector<int>& neighbors)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNodeNeighbors" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getNodeNeighbors(nid, neighbors);
}

inline 	void SlbmInterface::getActiveNodeNeighbors(const int& nid, vector<int>& neighbors)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getActiveNodeNeighbors" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getActiveNodeNeighbors(nid, neighbors);
}

inline 	void SlbmInterface::getNodeNeighborInfo(const int& nid, int neighbors[], 
			double distance[], double azimuth[], int& nNeighbors)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNodeNeighborInfo" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getNodeNeighborInfo(nid, neighbors, distance, azimuth, nNeighbors);
}

inline 	void SlbmInterface::getActiveNodeNeighborInfo(const int& nid, int neighbors[], 
			double distance[], double azimuth[], int& nNeighbors)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getActiveNodeNeighborInfo" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getActiveNodeNeighborInfo(nid, neighbors, distance, azimuth, nNeighbors);
}

inline 	void SlbmInterface::getActiveNodeNeighborInfo(const int& nid, 
			vector<int>& neighbors, vector<double>& distance, vector<double>& azimuth)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getActiveNodeNeighborInfo" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getActiveNodeNeighborInfo(nid, neighbors, distance, azimuth);
}

inline 	void SlbmInterface::getNodeSeparation(const int& node1, const int& node2, double& distance)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNodeSeparation" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getNodeSeparation(node1, node2, distance);
}

inline 	void SlbmInterface::getNodeAzimuth(const int& node1, const int& node2, double& azimuth)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNodeAzimuth" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getNodeAzimuth(node1, node2, azimuth);
}

inline void SlbmInterface::initializeActiveNodes(const double& latmin, const double& lonmin, 
		const double& latmax, const double& lonmax)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::initializeActiveNodes" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->initializeActiveNodes(latmin, lonmin, latmax, lonmax);
}

inline void SlbmInterface::initializeActiveNodes(double* lat, double* lon, const int& npoints, const bool& degrees)
{
	vector<double*> v;
	v.reserve(npoints);
	if (degrees)
		for (int i=0; i<npoints; ++i)
			v.push_back(GeoTessUtils::getVectorDegrees(lat[i], lon[i]));
	else
		for (int i=0; i<npoints; ++i)
			v.push_back(GeoTessUtils::getVector(lat[i], lon[i]));
	initializeActiveNodes(v);
}

inline void SlbmInterface::initializeActiveNodes(vector<double*>& unitVectors)
{
	initializeActiveNodes(new GeoTessPolygon(unitVectors));
}

inline void SlbmInterface::initializeActiveNodes(GeoTessPolygon* polygon)
{
	grid->initializeActiveNodes(polygon);
}

inline void SlbmInterface::initializeActiveNodes(const string& fileName)
{
	grid->initializeActiveNodes(new GeoTessPolygon(fileName));
}

inline void SlbmInterface::clearActiveNodes()
{
	grid->clearActiveNodes();
}


inline int SlbmInterface::getNActiveNodes()
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::nextActiveNode" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	return grid->getNActiveNodes();
}

inline int SlbmInterface::getGridNodeId(int activeNodeId)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getGridNodeId" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	return grid->getGridNodeId(activeNodeId);
}

inline int SlbmInterface::getActiveNodeId(int gridNodeId)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getActiveNodeId" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	return grid->getActiveNodeId(gridNodeId);
}

inline void SlbmInterface::getNodeHitCount(const int& nodeId, int& hitCount)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getNodeHitCount" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->getNodeHitCount(nodeId, hitCount);
}

inline void SlbmInterface::clearNodeHitCount()
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::clearNodeHitCount" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->clearNodeHitCount();
}

inline void SlbmInterface::getAverageMantleVelocity(const int& type, double& velocity)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::setAverageMantleVelocity" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	velocity = grid->getAverageMantleVelocity(type);
}

inline void SlbmInterface::setAverageMantleVelocity(const int& type, 
													const double& velocity)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::setAverageMantleVelocity" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->setAverageMantleVelocity(type, velocity);
}

inline void SlbmInterface::getTessId(string& tessId)
{
	if (!grid)	
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getTessId" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	tessId = grid->getTessId();
}

inline void SlbmInterface::getFractionActive(double& fractionActive)
{
	if (!isValid())
	{
		fractionActive = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getFractionActive" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}
	fractionActive = greatCircle->getFractionActive();
}

inline string SlbmInterface::getClassCount()
{
	ostringstream os;
	os << "Class counts:" << endl;
	os << "GreatCircle         = " << GreatCircle::getClassCount()	<< endl;
	os << "GridProfile         = " << GridProfile::getClassCount()	<< endl;
	os << "GeoStack            = " << GeoStack::getClassCount()	<< endl;
	os << "InterpolatedProfile = " << InterpolatedProfile::getClassCount()	<< endl;
	os << "CrustalProfile      = " << CrustalProfile::getClassCount()	<< endl;
	os << "LayerProfile        = " << LayerProfile::getClassCount()	<< endl;
	os << "QueryProfile        = " << QueryProfile::getClassCount()	<< endl;
	os << "Location            = " << Location::getClassCount()	<< endl;
	return os.str();
}

inline const string& SlbmInterface::getModelPath() const { return grid->getModelPath(); }

inline void SlbmInterface::getPiercePointSource(double& lat, double& lon, double& depth)
{ 
	if (!isValid())
	{
		lat = lon = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getPiercePointSource" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	
	if (greatCircle->getPhase() == Pg || greatCircle->getPhase() == Lg)
	{
		lat = lon = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getPiercePointSource" << endl
			<< "Cannot compute moho pierce points for " << getPhase() << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	if (!greatCircle->getSourceProfile()->isInCrust())
	{
		lat = lon = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getPiercePointSource" << endl
			<< "Cannot compute moho pierce point for source in the mantle." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	Location loc;
	greatCircle->getGreatCircleLocation(greatCircle->getSourceDistance(), loc);

	lat = loc.getLat();
	lon = loc.getLon();
	
	QueryProfile* profile = NULL;
	profile = grid->getQueryProfile(loc);
	depth = profile->getDepth()[MANTLE];
	delete profile;
}

inline void SlbmInterface::getPiercePointReceiver(double& lat, double& lon, double& depth)
{ 
	if (!isValid())
	{
		lat = lon = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getPiercePointReceiver" << endl
			<< "GreatCircle is invalid." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	
	if (greatCircle->getPhase() == Pg || greatCircle->getPhase() == Lg)
	{
		lat = lon = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getPiercePointReceiver" << endl
			<< "Cannot compute moho pierce points for " << getPhase() << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	if (!greatCircle->getReceiverProfile()->isInCrust())
	{
		lat = lon = NA_VALUE;
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getPiercePointReceiver" << endl
			<< "Cannot compute moho pierce point for receiver in the mantle." << endl
			  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),113);
	}

	Location loc;
	greatCircle->getGreatCircleLocation(
		greatCircle->getDistance()-greatCircle->getReceiverDistance(), loc);

	lat = loc.getLat();
	lon = loc.getLon();

	QueryProfile* profile = NULL;
	profile = grid->getQueryProfile(loc);
	depth = profile->getDepth()[MANTLE];
	delete profile;
}

inline void SlbmInterface::getGreatCirclePoints(					
		const double& aLat,
		const double& aLon,
		const double& bLat,
		const double& bLon,
		const int& npoints,
		double latitude[],
		double longitude[])
{
	Location a(aLat, aLon, 0.);
	Location b(bLat, bLon, 0.);
	double dx = a.distance(b)/(npoints-1);

	double moveDirection[3];
	a.vectorTripleProduct(b, moveDirection);
	for (int i=0; i<npoints; i++)
	{
		a.move(moveDirection, i*dx, b);
		latitude[i] = b.getLat();
		longitude[i] = b.getLon();
	}
}

inline void SlbmInterface::getGreatCirclePointsOnCenters(
		const double& aLat,
		const double& aLon,
		const double& bLat,
		const double& bLon,
		const int& npoints,
		double latitude[],
		double longitude[])
{
	Location a(aLat, aLon, 0.);
	Location b(bLat, bLon, 0.);
	double dx = a.distance(b)/npoints;

	double moveDirection[3];
	a.vectorTripleProduct(b, moveDirection);
	for (int i=0; i<npoints; i++)
	{
		a.move(moveDirection, (i+0.5)*dx, b);
		latitude[i] = b.getLat();
		longitude[i] = b.getLon();
	}
}

inline void SlbmInterface::getDistAz(const double& aLat, const double& aLon, 
	const double& bLat, const double& bLon, 
	double& distance, double& azimuth, const double& naValue)
{
	Location ptA(aLat, aLon);
	Location ptB(bLat, bLon);
	distance = ptA.distance(ptB);
	azimuth = ptA.azimuth(ptB, naValue);
}

inline void SlbmInterface::movePoint(const double& aLat, const double& aLon, 
		const double& distance, const double& azimuth,
		double& bLat, double& bLon)
{
	Location ptA(aLat, aLon);
	Location ptB;
	ptA.move(azimuth, distance, ptB);
	bLat = ptB.getLat();
	bLon = ptB.getLon();
}

inline void SlbmInterface::setInterpolatorType(const string& interpolatorType)
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::setInterpolatorType" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	grid->setInterpolatorType(interpolatorType);
}

inline string SlbmInterface::getInterpolatorType()
{
	if (!grid)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(9);
		  os << endl << "ERROR in SlbmInterface::getInterpolatorType" << endl
			<< "Grid is invalid.  Has the earth model been loaded with call to loadVelocityModel()?" << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),114);
	}
	return grid->getInterpolatorType();
}



} // end slbm namespace

#endif // SlbmInterface_H
