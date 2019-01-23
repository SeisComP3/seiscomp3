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
//- Program:       GreatCircle
//- Module:        $RCSfile: GreatCircle.h,v $
//- Revision:      $Revision: 1.41 $
//- Last Modified: $Date: 2013/09/08 22:44:43 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef GreatCircle_H
#define GreatCircle_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <map>
#include <string>
#include <iostream>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "Location.h"
#include "LayerProfile.h"
//#include "CrustalProfile.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class Grid;
class CrustalProfile;

// **** _CLASS DEFINITION_ *****************************************************
//
//! \brief The GreatCircle class manages information related to a great circle path
//! between two Locations on the Earth, including the ability to compute the total
//! travel time from a seismic source to a receiver.
//!
//! The GreatCircle class manages information related to a great circle path
//! between two locations on the Earth, including the ability to compute the total
//! travel time from a seismic source to a receiver.  The GreatCircle class 
//! has derived classes GreatCircle_Xn, which computes
//! travel times for Pn and Sn phases, and GreatCircle_Xg, which computes
//! travel times for Pg and Lg phases.  Calling applications should use
//! GreatCircleFactory to retrieve the appropriate type of GreatCircle
//! object.
//!
//! A GreatCircle maintains two 
//! CrustalProfile objects, one for the source and one for the receiver.  These
//! CrustalProfile objects contain information about the velocity as a function
//! of depth from the surface down to the Moho.
//! The Location objects owned by the CrustalProfile objects reflect the 
//! latitude, longitude and depths of the source and receiver.
//! 
//! A GreatCircle object will generate a number of closely spaced LayerProfile 
//! objects which lie along the great circle path between the source and 
//! and receiver, at the depth of the interface along which the
//! head wave travels (the Moho for Pn and Sn, the top of the middle crust for
//! Pg and Lg).  
//!
//! The desired spacing of the LayerProfile objects is specified in the 
//! GreatCircle constructor, but the 
//! actual spacing may be reduced slightly so that an integral number
//! of horizontal increments will exactly span the distance between the
//! source and receiver piercepoints.  Actual spacing will be as large as 
//! possible but less than or equal to the specified value.  The actual
//! spacing can be determined by calling getActualPathIncrement().
//!
//! The LayerProfile objects used by a GreatCircle are owned by the 
//! GreatCircle in that they are created by it on an as-needed basis, they
//! are deleted by it in the GreatCircle destructor, and LayerProfile objects
//! are not shared among different GreatCircle objects.  
//! CrustalProfile objects, on the other hand, can be shared among multiple
//! GreatCircle objects.  Grid maintains a map that associates waveType/location
//! combinations with pointers to CrustalProfile objects.  Upon creation,
//! GreatCircle objects request CrustalProfile objects from Grid.  If Grid
//! already has a CrustalProfile object for the specified wave type / location,
//! it provides GreatCircle with the pointer.  GreatCircle does not delete
//! CrustalProfile objects in its desctructor.  CrustalProfiles are deleted
//! by calling Grid::clearCrustalProfiles() or in the Grid destructor.
//!
class SLBM_EXP_IMP GreatCircle
{

public:

	//! \brief Parameterized constructor.  
	//! 
	//! Parameterized constructor.  
	//! @param _phase the phase that this GreatCircle object is to support.
	//! Must be one of SLBMGlobals::Pn, Sn, Pg, Lg.
	//! @param _grid The Grid from which LayerProfile objects
	//! will be extracted.
	//! @param latSource the geographic latitude of the source, in radians
	//! @param lonSource the geographic longitude of the source, in radians
	//! @param depthSource the depth of the source below sea level, in km
	//! @param latReceiver the geographic latitude of the receiver, in radians
	//! @param lonReceiver the geographic longitude of the receiver, in radians
	//! @param depthReceiver the depth of the receiver below sea level, in km
	GreatCircle(
		const int& _phase,
		Grid& _grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver);

	//! \brief Destructor.  Deletes LayerProfile objects 
	//! associated with this GreatCircle object.
	//!
	//! Destructor.  Deletes LayerProfile objects 
	//! associated with this GreatCircle object.
	virtual ~GreatCircle();

	//! \brief Copy constructor.
	//! 
	//! Copy constructor.
	GreatCircle(const GreatCircle &other);

	//! \brief Equal operator.
	//! 
	//! Equal operator.
	GreatCircle& operator=(const GreatCircle& other);

	//! \brief Retrieve a reference to the Grid object that this
	//! GreatCircle is associated with.
	//!
	//! Retrieve a reference to the Grid object that this
	//! GreatCircle is associated with.
	Grid& getGrid() { return grid; };

	//! \brief Retrieve the phase that this GreatCircle supports.  
	//! Will be one of SLBMGlobals::Pn, SLBMGlobals::Sn, SLBMGlobals::Pg, 
	//! SLBMGlobals::Lg.
	//!
	//! Retrieve the phase that this GreatCircle supports.  
	//! Will be one of SLBMGlobals::Pn, SLBMGlobals::Sn, SLBMGlobals::Pg, 
	//! SLBMGlobals::Lg.
	int getPhase() { return phase; };

	//! \brief Retrieve the index of the head wave interface 
	//! (SLBMGlobals::MANTLE for Pn, Sn; SLBMGlobals::MIDDLE_CRUST for Pg, Lg).
	//!
	//! Retrieve the index of the head wave interface 
	//! (SLBMGlobals::MANTLE for Pn, Sn; SLBMGlobals::MIDDLE_CRUST for Pg, Lg).
	const int& getHeadWaveInterface()  { return headWaveInterface; };

	//! \brief Retrieve a string representation fo the the phase that this 
	//! GreatCircle supports.  Will be one of 'Pn', 'Sn', 'Pg', 'Lg'.
	//!
	//! Retrieve a string representation fo the the phase that this 
	//! GreatCircle supports.  Will be one of 'Pn', 'Sn', 'Pg', 'Lg'.
	string getPhaseString();

	//! \brief Retrieve a pointer to the source CrustalProfile.
	//! 
	//! Retrieve a pointer to the source CrustalProfile.  This object can
	//! be queried for velocity vs depth, travel time, Location, etc.
	CrustalProfile* getSourceProfile() { return source; };

	//! \brief Retrieve a pointer to the receiver CrustalProfile.
	//! 
	//! Retrieve a pointer to the receiver CrustalProfile.  This object can
	//! be queried for velocity vs depth, travel time, Location, etc.
	CrustalProfile* getReceiverProfile() { return receiver; };

	double getSourceRayParameter() { return sourceRayParameter; };

	double getReceiverRayParameter() { return receiverRayParameter; };

	//! \brief Retrieve a pointer to one of the LayerProfile objects
	//! that comprise the head wave portion of the ray path.
	//! 
	//! Retrieve a pointer to one of the LayerProfile objects in profiles
	//! that comprise the head wave portion of the ray path.  This object can
	//! be queried for depth, radius, velocity and gradient information.
	//!
	//! If the requested LayerProfile object has not yet been 
	//! instantiated (is NULL), it is instantiated by this method.
	//!
	//! The profiles are located at the centers of horizontal
	//! increments of length actual_path_increment (radians).  The first 
	//! horizontal increment starts at the point on the head
	//! wave propagation interface with latitude and longitude
	//! equal to that of the source.
	//! The last horizontal increment ends at the point on the head
	//! wave propagation interface with latitude and longitude
	//! equal to the that of the receiver.
	//! The spacing is nominally equal to
	//! the value path_increment specified in the constructor, but is generally 
	//! reduced somewhat so that an integral number of horizontal
	//! increments that are all the same size will fit into the distance
	//! from source to receiver.  The actual spacing can be retrieved
	//! with a call to getActualPathIncrement().
	LayerProfile* getProfile(const int& i);

	//! \brief Retrieve the number of LayerProfile object positioned along
	//! the head wave interface.
	//! 
	//! Retrieve the number of LayerProfile object positioned along
	//! the head wave interface.
	int getNProfiles() { return (int)profiles.size(); };

	//! \brief Retrieve the Location of a LayerProfile.
	//! 
	//! Retrieve the Location of a LayerProfile.  Method Location::getRadius()
	//! will return the radius of the head wave interface at the position
	//! of the requested LayerProfile.
	void getLayerProfileLocation(const int& i, Location& loc);

	//! \brief Retrieve source-receiver separation, in radians.
	//! 
	//! Retrieve source-receiver separation, in radians.
	//! getDistance() = getSourceDistance() +
	//! getReceiverDistance() + getHeadwaveDistance().
	//! @return the source-receiver separation, in radians.
	double getDistance() { return distance; };

	//! \brief Retrieve source-receiver azimuth, in radians.
	//! 
	//! Retrieve source-receiver azimuth, in radians.
	//! @return the source-receiver azimuth, in radians.
	double getEsaz();

	//! \brief Retrieve horizontal offset below the source, in radians.
	//! 
	//! Retrieve horizontal offset below the source, in radians.
	//! This is the angular distance between the location of the 
	//! source and the source pierce point where the ray impinged
	//! on the headwave interface.
	//! @return the horizontal offset below the source, in radians.
	double getSourceDistance() { return xSource; };

	//! \brief Retrieve horizontal offset below the receiver, in radians.
	//! 
	//! Retrieve horizontal offset below the receiver, in radians.
	//! This is the angular distance between the location of the 
	//! receiver and the receiver pierce point where the ray impinged
	//! on the headwave interface.
	//! @return the horizontal offset below the receiver, in radians.
	double getReceiverDistance() { return xReceiver; };

	//! \brief Retrieve angular distance traveled by the ray
	//! below the headwave interface, in radians.
	//! 
	//! Retrieve the angular distance traveled by the ray
	//! below the headwave interface, in radians.
	//! This is the total distance minus the horizontal offsets
	//! below the source and receiver.  getSourceDistance() +
	//! getReceiverDistance() + getHeadwaveDistance() = 
	//! getDistance().
	//! @return the angular distance traveled by the ray
	//! below the headwave interface, in radians.
	double getHeadwaveDistance() { return distance-xSource-xReceiver; };

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
	//! @return the horizontal distance traveled by the ray
	//! below the headwave interface, in km.
	double getHeadwaveDistanceKm() { return xHorizontal; };

	//! \brief Retrieve horizontal separation of LayerProfiles, in radians.
	//! 
	//! Retrieve horizontal separation of LayerProfiles, in radians.
	//! @return the horizontal separation of the LayerProfiles, in radians.
	double getActualPathIncrement() { return actual_path_increment; };

	//! \brief Retrieve the travel time from the source to the receiver.
	//!
	//! Retrieve the travel time from the source to the receiver.  
	//! The travelTime was actually
	//! computed when the GreatCircle constructor was called.  This method
	//! returns the previously computed value.
	//! @return travel time in seconds.  Returns SLBMGlobals::NA_VALUE if
	//! result is invalid.
	double getTravelTime();

	//! \brief Retrieve the total travel time and the 4 components that
	//! contribute to it.  
	//!
	//! Retrieve the total travel time and the 4 components that
	//! contribute to it.  The values were actually
	//! computed when the GreatCircle constructor was called.  This method
	//! returns the previously computed values.  The values will equal
	//! SLBMGlobals::NA_VALUE if they are invalid.
	//! @param tTotal the total travel time, in seconds.
	//! @param tSource the crustal travel time below the source, in seconds.
	//! @param tReceiver the crustal travel time below the receiver, in seconds.
	//! @param tMantle the head wave travel time in the mantle, in seconds.
	//! @param tGradient the Zhao gradient correction term, in seconds.
	void getTravelTime(double& tTotal, double& tSource, double& tReceiver, 
			double& tMantle, double& tGradient);

	//! \brief Retrieve a formatted string providing a detailed description
	//! of the information managed by this GreatCircle object.  
	//!
	//! Retrieve a formatted string providing a detailed description
	//! of the information managed by this GreatCircle object.  
	//! @param verbosity specifies the amount of information
	//! that is to be included in the return string.  Each 
	//! verbosity level includes all information in preceeding
	//! verbosity levels.
	//! - 0 : nothing.  An empty string is returned.
	//! - 1 : total distance and travel time summary
	//! - 2 : gradient correction information for Pn/Sn.
	//!       Nothing for Pg/Lg
	//! - 3 : Source and receiver profiles
	//! - 4 : Grid node weights.
	//! - 5 : Head wave interface profiles
	//! - 6 : Interpolation coefficients for great circle nodes on
	//!       the head wave interface.
	virtual string toString(const int& verbosity) = 0;

	//! \brief Retrieve information about the great circle path including the
	//! interface depths at source and receiver, the velocity profiles at the source
	//! and receiver, and mantle velocity and velocity gradient at points along the
	//! great circle path from source pierce point to receiver pierce point.
	//!
	//! Retrieve information about the great circle path including the
	//! interface depths at source and receiver, the velocity profiles at the source
	//! and receiver, and mantle velocity and velocity gradient at points along the
	//! great circle path from source pierce point to receiver pierce point.
	//! <p>
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
	void getData(
		int& phase,
		double& actual_path_increment,
		double sourceDepth[NLAYERS],
		double sourceVelocity[NLAYERS],
		double receiverDepth[NLAYERS],
		double receiverVelocity[NLAYERS],
		int& npoints,
		double headWaveVelocity[],
		double gradient[]
		);

	//! \brief Retrieve information about the great circle path including the
	//! interface depths at source and receiver, the velocity profiles at the source
	//! and receiver, and mantle velocity and velocity gradient at points along the
	//! great circle path from source pierce point to receiver pierce point.
	//!
	//! Retrieve information about the great circle path including the
	//! interface depths at source and receiver, the velocity profiles at the source
	//! and receiver, and mantle velocity and velocity gradient at points along the
	//! great circle path from source pierce point to receiver pierce point.
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
	//! @param headWaveVelocity a double array of length npoints containing the P or S
	//! velocity at the center of each horizontal segment between the source and the receiver, in km/sec.
	//! The first horizontal segment starts at the source, the last horizontal
	//! segment ends at the receiver, and each one is of size actual_path_increment.  The head
	//! wave velocities are interpolated at the center of each of these horizontal
	//! segments, just below the head wave interface.
	//! @param gradient a double array of length npoints containing the P or S velocity
	//! gradient in the mantle at the center of each horizontal segment of the head wave, in 1/sec.
	//! For Pg and Lg, the values will be SLBMGlobals::NA_VALUE.
	void getData(
		int& phase,
		double& actual_path_increment,
		vector<double>& sourceDepth,
		vector<double>& sourceVelocity,
		vector<double>& receiverDepth,
		vector<double>& receiverVelocity,
		vector<double>& headWaveVelocity,
		vector<double>& gradient
		);

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
	//! If npoints exceeds this value, an exception is thrown.
	//! @param maxnodes the maximum size of the second dimension of arrays neighbors and coefficients.
	//! If any value of nnodes exceeds this value, an exception is thrown.
	//! @param npoints the number of horizontal increments sampled along the head wave interface.
	//! @param nnodes an int array of length npoints containing the number of nodes that contributed
	//! to the interpolation of information at the center of each horizontal segment of the ray path.
	void getNodeInfo(
		int** neighbors,
		double** coefficients,
		const int& maxpoints,
		const int& maxnodes,
		int& npoints,
		int* nnodes
		);

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
	//! @param neighbors a ragged 2D array of ints with dimensions npoints x nnodes
	//! containing the nodeIds of the neighboring grid nodes used to derive the interpolated
	//! data at each head wave profile.
	//! @param coefficients a ragged 2D array of doubles with dimensions npoints x nnodes containing
	//! the interpolation coefficients applied to each element of neighbors.
	void getNodeInfo( vector<vector<int> >& neighbors, vector<vector<double> >& coefficients );

	//! \brief Retrieve a location object that is located on the great circle
	//! at specified distance from source.
	//!
	//! Retrieve a location object that is located on the great circle
	//! at specified distance from source.
	//! @param distance angular distance from source in radians.
	//! @param loc the Location object to be populated with new position information.
	void getGreatCircleLocation(const double& distance, Location& loc);

	//! \brief Retrieve the weight assigned to each grid node that 
	//! was touched by the GreatCircle.
	//!
	//! Retrieve the weight assigned to each grid node that 
	//! was touched by the GreatCircle.
	//!
	//! The <I>weight</I> vector is initialized to 0 for each visited
	//! GridProfile object. Then every LayerProfile on the head
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
	//! 
	//! Note: Only grid nodes touched by this GreatCircle are included in the 
	//! output.  Each grid node is included only once, even though more than
	//! one LayerProfile object may have contributed some weight to it.  
	//! The sum of all the weights will equal the horizontal distance
	//! traveled by the ray along the head wave interface, from the source
	//! pierce point to the receiver pierce point, in km.
	//! @param nodeids the ID numbers of all the GridProfile objects
	//! touched by this GreatCircle.
	//! @param weights the weight associated with each GridProfile object
	//! touched by this GreatCircle.
	void getWeights(vector<int>& nodeids, vector<double>& weights);

	//! \brief Retrieve the weight assigned to each grid node that 
	//! was touched by the GreatCircle.
	//!
	//! Retrieve the weight assigned to each grid node that 
	//! was touched by the GreatCircle.
	//!
	//! The <I>weight</I> array is initialized to 0 for each visited
	//! GridProfile object. Then every LayerProfile on the head
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
	//! 
	//! Note: Only grid nodes touched by this GreatCircle are included in the 
	//! output.  Each grid node is included only once, even though more than
	//! one LayerProfile object may have contributed some weight to it.  
	//! The sum of all the weights will equal the horizontal distance
	//! traveled by the ray along the head wave interface, from the source
	//! pierce point to the receiver pierce point, in km.
	//! @param nodeids the ID numbers of all the GridProfile objects
	//! touched by this GreatCircle.
	//! @param weights the weight associated with each GridProfile object
	//! touched by this GreatCircle.
	//! @param nweights the number of elements in nodeId and weight.
	//! Calling application must dimension this
	//! array large enough to handle any possible size.
	void getWeights(int nodeids[], double weights[], int& nweights);

	double getFractionActive();

	double getRayParameter() { return rayParameter; };

	double getTurningRadius() { return turningRadius; };

	virtual void getZhaoParameters(double& Vm, double& Gm, double& H, double& C, double& Cm, int& udSign) = 0;

	virtual void getPgLgComponents(double& tT, 
								double& tTaup, double& tHeadwave, 
								double& pTaup, double& pHeadwave, 
								double& trTaup, double& trHeadwave)
	{ tT = tTaup = tHeadwave = pTaup = pHeadwave = trTaup = trHeadwave = NA_VALUE; };

	virtual size_t memSize();

	static int getClassCount();


	static void setDelDistance(const double& del_distance) { DEL_DISTANCE = del_distance; }

	static double getDelDistance() { return DEL_DISTANCE; }
	
	static void setDelDepth(const double& del_depth) { DEL_DEPTH = del_depth; }

	static double getDelDepth() { return DEL_DEPTH; }
	
	static void setPathIncrement(const double& path_increment) { PATH_INCREMENT = path_increment; }

	static double getPathIncrement() { return PATH_INCREMENT; }
	
	//! \brief Retrieve the derivative of travel time wrt to source-receiver separation,
	//! in seconds/radian.  
	//!
	//! Retrieve the derivative of travel time wrt to source-receiver separation,
	//! in seconds/radian. 
	//! @return the derivative of travel time wrt to source-receiver separation,
	//! in seconds/radian.  
	void get_dtt_ddist(double& dtt_ddist);
	
	//! \brief Retrieve the derivative of travel time wrt to source latitude,
	//! in seconds/radian.  
	//!
	//! Retrieve the derivative of travel time wrt to source latitude,
	//! in seconds/radian. 
	//! @param dtt_dlat the derivative of travel time wrt to source latitude.
	void get_dtt_dlat(double& dtt_dlat);

	//! \brief Retrieve the derivative of travel time wrt to source longitude,
	//! in seconds/radian.  
	//!
	//! Retrieve the derivative of travel time wrt to source longitude,
	//! in seconds/radian. 
	//! @param dtt_dlon the derivative of travel time wrt to source longitude.
	void get_dtt_dlon(double& dtt_dlon);

	//! \brief Retrieve the derivative of travel time wrt to source depth,
	//! in seconds/km.  
	//!
	//! Retrieve the derivative of travel time wrt to source depth,
	//! in seconds/km. 
	//! @param dtt_ddepth the derivative of travel time wrt to source depth.
	void get_dtt_ddepth(double& dtt_ddepth);

	//! \brief Retrieve the derivative of travel time wrt to source latitude,
	//! in seconds/radian.  
	//!
	//! Retrieve the derivative of travel time wrt to source latitude,
	//! in seconds/radian. 
	//! @param dtt_dlat the derivative of travel time wrt to source latitude.
	//void get_dtt_dlat_fast(double& dtt_dlat);

	//! \brief Retrieve the derivative of travel time wrt to source longitude,
	//! in seconds/radian.  
	//!
	//! Retrieve the derivative of travel time wrt to source longitude,
	//! in seconds/radian. 
	//! @param dtt_dlon the derivative of travel time wrt to source longitude.
	//void get_dtt_dlon_fast(double& dtt_dlon);

	//! \brief Retrieve the derivative of horizontal slowness wrt to source-receiver distance,
	//! in seconds/radian^2.  
	//!
	//! Retrieve the derivative of horizontal slowness wrt to source-receiver distance,
	//! in seconds/radian^2. 
	//void get_dsh_ddist(double& dsh_ddist);

	//! \brief Retrieve the derivative of horizontal slowness wrt to source latitude,
	//! in seconds/radian^2.  
	//!
	//! Retrieve the derivative of horizontal slowness wrt to source latitude,
	//! in seconds/radian^2. 
	//! @param dsh_dlat the derivative of horizontal slowness wrt to source latitude.
	//void get_dsh_dlat(double& dsh_dlat);

	//! \brief Retrieve the derivative of horizontal slowness wrt to source longitude,
	//! in seconds/radian^2.  
	//!
	//! Retrieve the derivative of horizontal slowness wrt to source longitude,
	//! in seconds/radian^2. 
	//! @param dsh_dlon the derivative of horizontal slowness wrt to source longitude.
	//void get_dsh_dlon(double& dsh_dlon);

	//! \brief Retrieve the derivative of horizontal slowness wrt to source depth,
	//! in seconds/radian-km.  
	//!
	//! Retrieve the derivative of horizontal slowness wrt to source depth,
	//! in seconds/radian-km. 
	//! @param dsh_ddepth the derivative of horizontal slowness wrt to source depth.
	//void get_dsh_ddepth(double& dsh_ddepth);

	static double MAX_DISTANCE;
	static double MAX_DEPTH;

	void setNAValues();

protected:

	static int greatCircleClassCount;

	//! \brief A reference to the Grid from which the source and receiver 
	//! CrustalProfile objects and the LayerProfile objects are interpolated. 
	//!
	//! A reference to the Grid from which the source and receiver 
	//! CrustalProfile objects and the LayerProfile objects are interpolated. 	
	Grid& grid;

	//! \brief The phase that this GreatCircle object is set up for
	//! (Pn, Sn, Pg or Lg).
	//!
	//! The phase that this GreatCircle object is set up for.  
	//! Will be one of Pn, Sn, Pg or Lg, which are int constants 
	//! defined in SLBMGlobals.h.
	int phase;

	//! \brief The index of the earth model interface along which 
	//! head waves propagate.  
	//! If phase is Pn or Sn, headWaveInterface will equal SLBMGlobals::MANTLE.
	//! If phase is Pg or Lg, headWaveInterface will equal SLBMGlobals::MIDDLE_CRUST.
	//!
	//! The index of the earth model interface along which head waves propagate.  
	//! If phase is Pn or Sn, headWaveInterface will equal SLBMGlobals::MANTLE.
	//! If phase is Pg or Lg, headWaveInterface will equal SLBMGlobals::MIDDLE_CRUST.
	int headWaveInterface;

	//! \brief The source CrustalProfile object.
	//!
	//! The source CrustalProfile object.  This object can
	//! be queried for velocity vs depth, travel time, Location, etc.
	CrustalProfile* source;

	//! \brief The receiver CrustalProfile object.
	//!
	//! The receiver CrustalProfile object.  This object can
	//! be queried for velocity vs depth, travel time, Location, etc.
	CrustalProfile* receiver;

	//! \brief A Location object used in a variety of places to manipulate
	//! Location information.
	Location location;

	//! \brief The vector triple product of the source and receiver
	//! Locations.   Used to compute Locations along the great circle
	//! path from source to receiver.
	//!
	//! \brief The vector triple product of the source and receiver
	//! Locations.   (source cross receiver) cross source.
	//! Used to compute Locations along the great circle
	//! path from source to receiver.
	double vtp[3];

	//! \brief The LayerProfile objects which are positioned on
	//! the interface along which the head wave propagates.
	//!
	//! The LayerProfile objects which are positioned on
	//! the interface along which the head wave propagates.
	//! The profiles are located at the centers of horizontal
	//! increments of length actual_path_increment (radians).  The first 
	//! horizontal increment starts at the point on the head
	//! wave propagation interface with latitude and longitude
	//! equal to the latitude and longitude of the source.
	//! The last horizontal increment ends at the point on the head
	//! wave propagation interface with latitude and longitude
	//! equal to the latitude and longitude of the receiver.
	//! The spacing is nominally equal to
	//! the value path_increment specified in the constructor, but may be 
	//! reduced somewhat so that an integral number of horizontal
	//! increments that are all the same size will fit into the distance
	//! from source to receiver.  The actual spacing can be retrieved
	//! with a call to getActualPathIncrement().
	//!
	//! actual_path_increment is determined in the GreatCircle() constructor
	//! and profiles is resized at that point to hold the correct
	//! number of LayerProfile objects.  The profiles are not instantiated
	//! in the constructor since the ones close to the source
	//! and receiver will never be needed.  Profiles are instantiated
	//! as needed in protected method getProfile(const int& i).
	vector<LayerProfile*> profiles;

	//! \brief The method used to calculate travel times.
	//!
	//! The method used to calculate travel times.  Will be a string
	//! that indicates the name of the c++ method that was used to
	//! calculate the travel times.
	string solutionMethod;

	//! \brief The total travel time from source to receiver, in seconds.
	//!
	//! The total travel time from source to receiver, in seconds.
	//! Includes both crustal legs, head wave component, and gradient
	//! correction, if appropriate.  Computed in computeTravelTime(),
	//! which is called from the GreatCircle constructor.
	double tTotal;

	//! \brief The actual spacing between the elements of profiles, 
	//! in radians.
	//!
	//! The actual spacing between the elements of profiles, 
	//! in radians.
	double actual_path_increment;

	//! \brief The horizontal source-receiver separation, in radians.
	//! 
	//! The horizontal source-receiver separation, in radians.
	double distance;

	//! \brief The source-receiver azimuth, in radians.
	//! 
	//! The source-receiver azimuth, in radians.
	double esaz;

	// \brief The ray parameter, in sec/radian or sec/km.
	//!
	//! The ray parameter, in sec/radian or sec/km.
	double rayParameter;

	//! \brief The horizontal distance from the source Location to
	//! the source pierce point, in radians.
	//!
	//! The horizontal distance from the source Location to
	//! the source pierce point, in radians.  
	//! This value is computed in CrustalProfile::xtCrust().
	double xSource;

	//! \brief The time required for the ray to travel from the source
	//! Location to piercePointSource, in seconds.
	//! 
	//! The time required for the ray to travel from the source
	//! Location to source pierce point, in seconds. 
	double tSource;

	//! \brief The horizontal distance from the receiver Location to
	//! the receiver pierce point, in radians.
	//!
	//! The horizontal distance from the receiver Location to
	//! the receiver pierce point, in radians.  
	//! This value is computed in CrustalProfile::xtCrust().
	double xReceiver;

	//! \brief The time required for the ray to travel from the receiver
	//! pierce point to the receiver Location, in seconds.
	//! 
	//! The time required for the ray to travel from the receiver
	//! pierce point to the receiver Location, in seconds.
	double tReceiver;

	//! \brief The horizontal distance traveled by the ray as a head wave,
	//! in km.
	//!
	//! The horizontal distance traveled by the ray as a head wave,
	//! in km.
	double xHorizontal;

	//! \brief The time spent by the ray traveling horizontally as a head wave, 
	//! in seconds.
	//!
	//! The time spent by the ray traveling horizontally as a head wave, 
	//! in seconds.
	double tHorizontal;

	//! \brief The gradient correction term of the total travel time, in sec.
	//!
	//! The gradient correction term of the total travel time, in sec.
	//! Zero of Pg, Lg.
	double tGamma;

	double sourceRayParameter;
	double receiverRayParameter;

	double turningRadius;

	//! \brief The index of the element in the profiles array corresponding
	//! to the horizontal increment in which the source pierce point is 
	//! located.  
	//!
	//! The index of the element in the profiles array corresponding
	//! to the horizontal increment in which the source pierce point is 
	//! located.
	int sourceIndex;

	//! \brief The index of the element in the profiles array corresponding
	//! to the horizontal increment in which the receiver pierce point is 
	//! located.  
	//!
	//! The index of the element in the profiles array corresponding
	//! to the horizontal increment in which the receiver pierce point is 
	//! located.
	int receiverIndex;

	//! \brief Retrieve the component of the horizontal separation between elements
	//! of profiles that actually contribute to the head wave, in radians.
	//!
	//! Retrieve the component of the horizontal separation between elements
	//! of profiles that actually contribute to the head wave, in radians.
	//! For horizontal increments of the source-receiver separation that are
	//! entirely between the source and source pierce point, or between the 
	//! receiver pierce point and the receiver, this will return 0.  
	//! For horizontal increments that lie entirely between the source and
	//! receiver pierce points, this will return actual_path_increment.  
	//! For the horizontal increments that include the source pierce point
	//! and/or the receiver pierce point, this 
	//! will return some fraction of actual_path_increment, reflecting the part of the 
	//! horizontal increment that was actually traversed by the head wave.
	double getActualPathIncrement(const int& i);

	//! \brief Method to compute the travel time from source to receiver, in 
	//! seconds.
	//!
	//! Method to compute the travel time from source to receiver, in 
	//! seconds.
	virtual void computeTravelTime() = 0;

	double sqr(const double& x) { return x*x; };

	//! the travel time at a source position which is very slightly 
	//! closer to the receiver than current source. Used in 
	//! calculation of derivatives wrt source position.
	double ttHminus;

	//! the travel time at a source position which is very slightly 
	//! further from the receiver than current source. Used in 
	//! calculation of derivatives wrt source position.
	double ttHplus;

	//! the travel time at a source position which is very slightly 
	//! greater depth than current source. Used in 
	//! calculation of derivatives wrt source position.
	//double ttZplus;

	//! the travel time at a source position which is very slightly 
	//! further from the receiver, and at greater depth than current 
	//! source. Used in calculation of derivatives wrt source position.
	double ttHZplus;
	
	//! the travel time at a source position which is very slightly 
	//! to the east of the source measured along a great circle path
	//! through the source.
	// Used in calculation of derivatives wrt source position.
	double ttEast;

	//! the travel time at a source position which is very slightly 
	//! to the west of the source measured along a great circle path
	//! through the source.
	// Used in calculation of derivatives wrt source position.
	double ttWest;

	//! the travel time at a source position which is very slightly 
	//! to the east of the source measured along a great circle path
	//! through the source, and at a slightly deeper depth.
	// Used in calculation of derivatives wrt source position.
	double ttEastZ;
	
	//! the travel time at a source position which is very slightly 
	//! to the north of the source measured along a great circle path
	//! through the source.
	// Used in calculation of derivatives wrt source position.
	double ttNorth;

	//! the travel time at a source position which is very slightly 
	//! to the South of the source measured along a great circle path
	//! through the source.
	// Used in calculation of derivatives wrt source position.
	double ttSouth;

	//! the travel time at a source position which is very slightly 
	//! to the north of the source measured along a great circle path
	//! through the source, and at a slightly deeper depth.
	// Used in calculation of derivatives wrt source position.
	double ttNorthZ;
	
	//! retrieve value of ttHminus.  Compute the value if it is NA_VALUE.
	double get_ttHminus();
	
	//! retrieve value of ttHplus.  Compute the value if it is NA_VALUE.
	double get_ttHplus();
	
	//! retrieve value of ttZplus.  Compute the value if it is NA_VALUE.
	double get_ttZplus();
	
	//! retrieve value of ttZplus.  Compute the value if it is NA_VALUE.
	double get_ttZminus();
	
	//! retrieve value of ttHZplus.  Compute the value if it is NA_VALUE.
	double get_ttHZplus();

	//! retrieve value of ttHminus.  Compute the value if it is NA_VALUE.
	double get_ttEast();
	
	//! retrieve value of ttHplus.  Compute the value if it is NA_VALUE.
	double get_ttWest();
	
	//! retrieve value of ttHZplus.  Compute the value if it is NA_VALUE.
	double get_ttEastZ();

	//! retrieve value of ttHminus.  Compute the value if it is NA_VALUE.
	double get_ttNorth();
	
	//! retrieve value of ttHplus.  Compute the value if it is NA_VALUE.
	double get_ttSouth();
	
	//! retrieve value of ttHZplus.  Compute the value if it is NA_VALUE.
	double get_ttNorthZ();

	// when computing horizontal slowness, the source is moved distance
	// del_distance (radians) away from the receiver and a new travel time
	// is computed.  The difference in travel time divided by del_distance
	// is the horizontal slowness.  Units are radians.
	static double DEL_DISTANCE;

	// when computing derivative of travel time wrt depth, the depth of the 
	// source is increased by del_depth and a new travel time computed.
	// The difference in travel time divided by del_depth is the 
	// derivative of travel time wrt depth.  Units of DEL_DEPTH is km.
	static double DEL_DEPTH;

	//! \brief the desired spacing of great circle nodes 
	//! along the head wave interface, in radians.
    //!
	//! the desired spacing of great circle nodes 
	//! along the head wave interface, in radians.  Defaults to 
	//! 0.1 degrees if not specified.  The actual spacing will be
	//! reduced from the requested value in order that an integral 
	//! number of equally spaced LayerProfile objects will exactly
	//! span the source-receiver separation.
	static double PATH_INCREMENT;

};

inline double GreatCircle::getTravelTime()
{
	return tTotal;
}

inline void GreatCircle::getTravelTime(double& tT, double& tS, double& tR, double& tM, double& tG)
{
	tT = tTotal;
	tS = tSource;
	tR = tReceiver;
	tM = tHorizontal;
	tG = tGamma;
}

inline size_t GreatCircle::memSize()
{
	//size_t n = 
	//4 // sizeof(grid)
	//+ sizeof(phase)
	//+ sizeof(source)
	//+ sizeof(receiver)
	//+ sizeof(location)
	//+ sizeof(vtp)+3*sizeof(double)
	//+ sizeof(tTotal)
	//+ sizeof(actual_path_increment)
	//+ sizeof(distance)
	//+ sizeof(esaz)
	//+ sizeof(xSource)
	//+ sizeof(tSource)
	//+ sizeof(xReceiver)
	//+ sizeof(tReceiver)
	//+ sizeof(xHorizontal)
	//+ sizeof(tHorizontal)
	//+ sizeof(sourceIndex)
	//+ sizeof(receiverIndex);
	//for (int i=0; i<(int)profiles.size(); i++)
	//	if (profiles[i]) n += profiles[i]->memSize();

	size_t n = 
	sizeof(&grid) // Grid&
	+ sizeof(phase)
	+ sizeof(headWaveInterface)
	+ sizeof(source)  // CrustalProfile*
	+ sizeof(receiver) // CrustalProfile*
	+ sizeof(location) // Location
	+ sizeof(vtp) + 3 * sizeof(double) // vtp[3]
	+ sizeof(profiles) // vector<LayerProfile*> profiles)
	+ sizeof(solutionMethod) // string
	+ sizeof(tTotal)
	+ sizeof(actual_path_increment)
	+ sizeof(distance)
	+ sizeof(esaz)
	+ sizeof(rayParameter)
	+ sizeof(xSource)
	+ sizeof(tSource)
	+ sizeof(xReceiver)
	+ sizeof(tReceiver)
	+ sizeof(xHorizontal)
	+ sizeof(tHorizontal)
	+ sizeof(tGamma)
	+ sizeof(sourceRayParameter)
	+ sizeof(receiverRayParameter)
	+ sizeof(turningRadius)
	+ sizeof(sourceIndex)
	+ sizeof(receiverIndex)
	+ sizeof(ttHminus)
	+ sizeof(ttHplus)
	//+ sizeof(ttZplus)
	+ sizeof(ttHZplus)
	+ sizeof(ttEast)
	+ sizeof(ttWest)
	+ sizeof(ttEastZ)
	+ sizeof(ttNorth)
	+ sizeof(ttSouth)
	+ sizeof(ttNorthZ)
	;

	return n;
}

inline double GreatCircle::getActualPathIncrement(const int& i)
{
	// this horizontal segment lies completely in between the 
	// source and receiver.  return actual_path_increment.
	if (i > sourceIndex && i < receiverIndex)
		return actual_path_increment;

	// if the source and receiver have the same index, it means the ray
	// hit the headwave interface in some horizontal segment, and left
	// the headwave interface before exiting the horizontal segment.
	// In this case, return the source-receiver separation minus the
	// horizontal offsets below the source and the receiver.
	if (i == sourceIndex && i == receiverIndex)
		return distance - xSource - xReceiver;

	// the ray hit the interface in this segment.  return only the 
	// part of the horizontal segment that starts at the source pierce point
	// and ends at the profile just to the right.
	if (i == sourceIndex)
		return (sourceIndex+1)*actual_path_increment - xSource;

	// this is the horizontal segment such that the ray left the 
	// interface in this segment.  return only the part of the 
	// horizontal segment that starts at the profile that is the
	// left side of the horizontal segment and ends at the 
	// receiver pierce point.
	if (i == receiverIndex)
		return distance - xReceiver - receiverIndex*actual_path_increment;

	return 0.;
}

inline string GreatCircle::getPhaseString() 
{ 
	return (phase==Pn ? "Pn" : (phase==Sn ? "Sn" : (phase==Pg ? "Pg" : (phase==Lg ? "Lg" 
			: "unknown phase"))));
}

inline double GreatCircle::getFractionActive()
{
	int nactive = 0;
	for (int i=0; i<(int)profiles.size(); ++i)
		if (getProfile(i)->isActiveProfile())
			++nactive;
	return ((double)nactive)/((double)profiles.size());
}

inline void GreatCircle::get_dtt_ddist(double& dtt_ddist)
{
	if (getTravelTime() > -1.)
	  dtt_ddist = (get_ttHplus() - getTravelTime())/DEL_DISTANCE;
	else
		dtt_ddist = NA_VALUE;
}

inline void GreatCircle::get_dtt_dlat(double& dtt_dlat)
{
	if (getTravelTime() > -1.)
		dtt_dlat = (get_ttNorth() - getTravelTime())/DEL_DISTANCE;
	else
		dtt_dlat = NA_VALUE;
}

inline void GreatCircle::get_dtt_dlon(double& dtt_dlon)
{
	if (getTravelTime() > -1.)
		dtt_dlon = (get_ttEast() - getTravelTime())/DEL_DISTANCE;
	else
		dtt_dlon = NA_VALUE;
}

//inline void GreatCircle::get_dtt_dlat_fast(double& dtt_dlat)
//{
//	dtt_dlat = (get_ttHplus() - getTravelTime())/DEL_DISTANCE*cos(getEsaz()+PI);
//}
//
//inline void GreatCircle::get_dtt_dlon_fast(double& dtt_dlon)
//{
//	dtt_dlon = (get_ttHplus() - getTravelTime())/DEL_DISTANCE*sin(getEsaz()+PI);
//}
//

//inline void GreatCircle::get_dsh_ddist(double& dsh_ddist)
//{
//	dsh_ddist = (get_ttHminus() - 2*getTravelTime() + get_ttHplus())
//			/DEL_DISTANCE/DEL_DISTANCE;
//}
//
//inline void GreatCircle::get_dsh_dlat(double& dsh_dlat)
//{
//	dsh_dlat = (get_ttNorth() - 2*getTravelTime() + get_ttSouth())
//			/DEL_DISTANCE/DEL_DISTANCE;
//}
//
//inline void GreatCircle::get_dsh_dlon(double& dsh_dlon)
//{
//	dsh_dlon = (get_ttEast() - 2*getTravelTime() + get_ttWest())
//			/DEL_DISTANCE/DEL_DISTANCE;
//}
//
//inline void GreatCircle::get_dsh_ddepth(double& dsh_ddepth)
//{
//	dsh_ddepth = (get_ttHZplus() - get_ttZplus()- get_ttHplus()
//		+ getTravelTime()) /DEL_DEPTH/DEL_DISTANCE;
//}

} // end slbm namespace

#endif // GreatCircle.h
