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
//- Program:       SlbmInterfaceToJNI
//- Module:        $RCSfile: SlbmInterfaceToJNI.h,v $
//- Revision:      $Revision: 1.21 $
//- Last Modified: $Date: 2013/02/25 15:48:00 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef SlbmInterfaceToJNI_H
#define SlbmInterfaceToJNI_H

// **** _SYSTEM INCLUDES_ ******************************************************
#include <cmath>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "SlbmInterface.h"
#include "GridProfile.h"
#include "SLBMException.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

//! \brief Adds functionality to SlbmInterface to support the Java Native 
//! Interface (JNI).  The JNI allows libSLBM to be accessed using programs
//! written in the Java programing language.
//!
//! Adds functionality to SlbmInterface to support the Java Native 
//! Interface (JNI).  The JNI allows libSLBM to be accessed using programs
//! written in the Java programing language.  All of the methods defined
//! in this class should only be accessed by SLBM_JNI/SlbmInterface.cc.
//! It is not envisioned that other applications will access this class directly.
class SLBM_EXP_IMP SlbmInterfaceToJNI : public SlbmInterface
{

public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	SlbmInterfaceToJNI();

	SlbmInterfaceToJNI(const double& earthRadius);

	//! \brief Destructor. 
	//!
	//! Destructor.
	~SlbmInterfaceToJNI();

	string getPhase();
	
	void accessGridProfile(const int& nodeId);

	void deleteGridProfile();

	double getGridLat();

	double getGridLon();

	void getGridDepth(vector<double>& depths);

	void getGridVelocity(const int& waveType, double* velocity);

	void getGridGradient(double* gradient);

	//void setGridDepths(const vector<double>& depth);

	//void setGridVelocity(const int& waveType, const vector<double>& velocity);

	// setGridGradient(const vector<double>& gradient);

	void createQueryProfile(const double& lat, const double& lon);

	void deleteQueryProfile();

	int getQueryNCoefficients();

	vector<int>& getQueryNodeId();

	vector<double>& getQueryCoefficient();

	double* getQueryDepth();

	double* getQueryVelocity(const int& waveType);

	double* getQueryGradient();

	void computeWeights();

	void computeWeightsSource();

	void computeWeightsReceiver(); 

	void deleteWeights() { nodeIds.clear(); weights.clear();  };

	const vector<int>& getWeightNodes() { return nodeIds; };

	const vector<double>& getWeights() { return weights; };

	string getGreatCirclePhase();

	double getActualPathIncrement();

	void getGreatCircleNeighbors(const int& i, int* nodeIds, int& size);

	void  getGreatCircleCoefficients(const int& i, double* coeff, int& size);

	void getGreatCircleSourceDepth(double* depths, int& n);

	void getGreatCircleReceiverDepth(double* depths, int& n);

	double* getGreatCircleSourceVelocity(int& n);

	double* getGreatCircleReceiverVelocity(int& n);

	void getGreatCircleHeadwaveVelocity(vector<double>& velocities);

	void getGreatCircleHeadwaveGradient(vector<double>& gradients);

	void computeGreatCirclePoints(
		const double& aLat,
		const double& aLon,
		const double& bLat,
		const double& bLon,
		const int& npoints,
		const bool& onCenters);

	void computeGreatCircleLocations();

	void deleteGreatCirclePoints() 
	 { greatCirclePointsLat.clear(); greatCirclePointsLon.clear(); greatCirclePointsDepth.clear();  };

	const vector<double>& getGreatCirclePointsLat() { return greatCirclePointsLat; };

	const vector<double>& getGreatCirclePointsLon() { return greatCirclePointsLon; };

	const vector<double>& getGreatCirclePointsDepth() { return greatCirclePointsDepth; };

	int getNGridNodes();

	int getNHeadWavePoints();

	void accessGridNodeNeighbors(int nid );


private:

	QueryProfile* queryProfile;

	GridProfile* gridProfile;

	vector<int> nodeIds;

	vector<double> weights;

	vector<double> greatCirclePointsLat, greatCirclePointsLon, greatCirclePointsDepth;

};

inline void SlbmInterfaceToJNI::computeGreatCirclePoints(
		const double& aLat,
		const double& aLon,
		const double& bLat,
		const double& bLon,
		const int& npoints,
		const bool& onCenters)
{
	deleteGreatCirclePoints();

	Location a(aLat, aLon, 0.);
	Location b(bLat, bLon, 0.);
	double delta, delta0;

	if (onCenters)
	{
		delta = a.distance(b)/npoints;
		delta0 = 0.5*delta;
	}
	else
	{
		delta = a.distance(b)/(npoints-1);
		delta0 = 0;
	}

	double moveDirection[3];
	a.vectorTripleProduct(b, moveDirection);
	for (int i=0; i<npoints; i++)
	{
		a.move(moveDirection, delta0+i*delta, b);
		greatCirclePointsLat.push_back(b.getLat());
		greatCirclePointsLon.push_back(b.getLon());
	}
}

inline void SlbmInterfaceToJNI::computeGreatCircleLocations()
{
	deleteGreatCirclePoints();

	Location loc;
	for (int i=0; i<greatCircle->getNProfiles(); i++)
	{
		// get the location of this layer profile
		greatCircle->getLayerProfileLocation(i, loc);
		greatCirclePointsLat.push_back(loc.getLat());
		greatCirclePointsLon.push_back(loc.getLon());
		greatCirclePointsDepth.push_back(loc.getDepth());
	}
}

inline string SlbmInterfaceToJNI::getPhase()
{
	if (greatCircle->getPhase() == Pn)
		return "Pn";
	if (greatCircle->getPhase() == Sn)
		return "Sn";
	if (greatCircle->getPhase() == Pg)
		return "Pg";
	if (greatCircle->getPhase() == Lg)
		return "Lg";
	return "unknown";
}

inline void SlbmInterfaceToJNI::accessGridProfile(const int& nodeId)
{
	gridProfile = grid->getProfile(nodeId);
}

inline double SlbmInterfaceToJNI::getGridLat()
{
	return gridProfile->getLat();
}

inline double SlbmInterfaceToJNI::getGridLon()
{
	return gridProfile->getLon();
}


inline void SlbmInterfaceToJNI::getGridDepth(vector<double>& depths)
{
	gridProfile->getInterfaceDepths(depths);
}

inline void SlbmInterfaceToJNI::getGridVelocity(const int& waveType, double* velocity)
{
	gridProfile->getVelocity(waveType, velocity);
}

inline void SlbmInterfaceToJNI::getGridGradient(double* gradient)
{
	gridProfile->getMantleGradient(gradient);
}

//inline void SlbmInterfaceToJNI::setGridDepths(const vector<double>& depth)
//{
//	gridProfile->setDepths(depth);
//}
//
//inline void SlbmInterfaceToJNI::setGridVelocity(const int& waveType,
//											  const vector<double>& velocity)
//{
//	gridProfile->setVelocity(waveType, velocity);
//}
//
//inline void SlbmInterfaceToJNI::setGridGradient(const vector<double>& gradient)
//{
//	gridProfile->setGradient(gradient);
//}

inline void SlbmInterfaceToJNI::createQueryProfile(const double &lat, const double &lon)
{
	deleteQueryProfile();
	Location loc(lat, lon, 0.);
	queryProfile = grid->getQueryProfile(loc);
}

inline void SlbmInterfaceToJNI::deleteQueryProfile()
{
	if (queryProfile) delete queryProfile; 
	queryProfile = NULL; 
}

inline int SlbmInterfaceToJNI::getQueryNCoefficients()
{
	return queryProfile->getNCoefficients();
}

inline vector<int>& SlbmInterfaceToJNI::getQueryNodeId()
{
	return queryProfile->getNodeIds();
}

inline vector<double>& SlbmInterfaceToJNI::getQueryCoefficient()
{
	return queryProfile->getCoefficients();
}

inline double* SlbmInterfaceToJNI::getQueryDepth()
{
	return queryProfile->getDepth();
}

inline double* SlbmInterfaceToJNI::getQueryVelocity(const int& waveType)
{
	return queryProfile->getVelocity(waveType);
}

inline double* SlbmInterfaceToJNI::getQueryGradient()
{
	return queryProfile->getMantleGradient();
}

inline void SlbmInterfaceToJNI::computeWeights() 
{ 
	SlbmInterface::getWeights(nodeIds, weights);
}


inline string SlbmInterfaceToJNI::getGreatCirclePhase()
{
	return greatCircle->getPhaseString();
}

inline double SlbmInterfaceToJNI::getActualPathIncrement()
{
	return greatCircle->getActualPathIncrement(); 
}

inline void SlbmInterfaceToJNI::getGreatCircleNeighbors(const int& i, int* nodeids, int& size)
{
	greatCircle->getProfile(i)->getNodeIds(nodeids, size);
}

inline void SlbmInterfaceToJNI::getGreatCircleCoefficients(const int& i, double* coeff, int& size)
{
	greatCircle->getProfile(i)->getCoefficients(coeff, size);
}

inline void SlbmInterfaceToJNI::getGreatCircleSourceDepth(double* depths, int& n)
{
	n = greatCircle->getSourceProfile()->getNIntervals();
	greatCircle->getSourceProfile()->getDepths(depths);
}

inline void SlbmInterfaceToJNI::getGreatCircleReceiverDepth(double* depths, int& n)
{
	n = greatCircle->getReceiverProfile()->getNIntervals();
	greatCircle->getReceiverProfile()->getDepths(depths);
}

inline double* SlbmInterfaceToJNI::getGreatCircleSourceVelocity(int& n)
{
	n = greatCircle->getSourceProfile()->getNIntervals();
	return greatCircle->getSourceProfile()->getVelocities();
}

inline double* SlbmInterfaceToJNI::getGreatCircleReceiverVelocity(int& n)
{
	n = greatCircle->getReceiverProfile()->getNIntervals();
	return greatCircle->getReceiverProfile()->getVelocities();
}

inline void SlbmInterfaceToJNI::getGreatCircleHeadwaveVelocity(vector<double>& v)
{
	v.clear();
	for (int i=0; i<greatCircle->getNProfiles(); i++)
		v.push_back(greatCircle->getProfile(i)->getVelocity());
}

inline void SlbmInterfaceToJNI::getGreatCircleHeadwaveGradient(vector<double>& g)
{
	g.clear();
	for (int i=0; i<greatCircle->getNProfiles(); i++)
		g.push_back(greatCircle->getProfile(i)->getGradient());
}

inline int SlbmInterfaceToJNI::getNGridNodes()
{
	int n = -1;
	n = grid->getNNodes();
	return n;
}

inline int SlbmInterfaceToJNI::getNHeadWavePoints()
{
    int nHeadWavePoints = -1;
	nHeadWavePoints = greatCircle->getNProfiles(); 
	return nHeadWavePoints;
}

inline void SlbmInterfaceToJNI::computeWeightsSource() 
{ 
	nodeIds = greatCircle->getSourceProfile()->getNodeIds();
	weights = greatCircle->getSourceProfile()->getCoefficients();
}
inline void SlbmInterfaceToJNI::computeWeightsReceiver() 
{ 
	nodeIds = greatCircle->getReceiverProfile()->getNodeIds();
	weights = greatCircle->getReceiverProfile()->getCoefficients();
}

} // end slbm namespace

#endif
