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
//- Module:        $RCSfile: GreatCircle.cc,v $
//- Revision:      $Revision: 1.41 $
//- Last Modified: $Date: 2013/02/18 12:49:29 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "GreatCircle.h"
#include "Grid.h"
#include "Location.h"
#include "CrustalProfile.h"
#include "LayerProfile.h"
#include "SLBMException.h"
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int GreatCircle::greatCircleClassCount = 0;

double GreatCircle::MAX_DISTANCE = 15.*DEG_TO_RAD;
double GreatCircle::MAX_DEPTH = 200.;

double GreatCircle::DEL_DISTANCE = 1e-3;
double GreatCircle::DEL_DEPTH = 1e-1;

double GreatCircle::PATH_INCREMENT = 0.1*DEG_TO_RAD;

GreatCircle::GreatCircle(
		const int& _phase,
		Grid& _grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver)
		:   grid(_grid),
			phase(_phase),
			headWaveInterface((_phase/2==0 ? MANTLE : MIDDLE_CRUST_G)),
			source(NULL),
			receiver(NULL),
			tTotal(NA_VALUE),
			actual_path_increment(NA_VALUE),
			distance(NA_VALUE),
			esaz(NA_VALUE),
			rayParameter(NA_VALUE),
			xSource(NA_VALUE),
			tSource(NA_VALUE),
			xReceiver(NA_VALUE),
			tReceiver(NA_VALUE),
			xHorizontal(NA_VALUE),
			tHorizontal(NA_VALUE),
			tGamma(NA_VALUE),
			sourceRayParameter(NA_VALUE),
			receiverRayParameter(NA_VALUE),
			turningRadius(NA_VALUE),
			sourceIndex(-1),
			receiverIndex(-1),
			ttHminus(NA_VALUE),
			ttHplus(NA_VALUE),
			//ttZplus(NA_VALUE),
			ttHZplus(NA_VALUE),
			ttEast(NA_VALUE),
			ttWest(NA_VALUE),
			ttEastZ(NA_VALUE),
			ttNorth(NA_VALUE),
			ttSouth(NA_VALUE),
			ttNorthZ(NA_VALUE)

			{

	++greatCircleClassCount;

	setNAValues();

	// create the CrustalProfile objects at each end of the great circle.
	receiver = grid.getReceiverProfile(phase, latReceiver, lonReceiver, depthReceiver);

	source = grid.getSourceProfile(phase, latSource, lonSource, depthSource);

	// find the distance from source to receiver in radians
	distance = source->getLocation().distance(receiver->getLocation());

	// find the azimuth from source to receiver in radians
	// esaz = source->getLocation().azimuth(receiver->getLocation());

	// Find the normalized vector triple product of the source and receiver
	// Locations.  This is used to compute other Location objects on the 
	// great circle path defined by the source and receiver Locations.
	// This will fail if the two locations are coincident or 180 degrees apart.
	int n;
	if (source->getLocation().vectorTripleProduct(receiver->getLocation(), vtp))
	{
		// find number of increments of size actual_path_increment that will fit in that
		// distance (minimum of one).
		n = (int)ceil(distance/PATH_INCREMENT);
		if (n <= 0)	n=1;
		
		// calculate a new increment size such that n of them will exactly
		// fit between source and receiver pierce points.
		actual_path_increment = distance/n;
		// profiles is the vector of LayerProfile objects along the head wave
		// interface.  Resize it to hold n LayerProfile pointers, but do not
		// initialize them.  They will be initialized as needed, in method
		// getProfile(const int&).
	}
	else
	{
		actual_path_increment = distance;
		n = 1;
	}

	profiles.resize(n, NULL);

	//cout << "GreatCircle::GreatCircle() PATH_INCREMENT, actual_path_increment =  " 
	//	<< PATH_INCREMENT << ",  " << actual_path_increment << "  " << n << endl;
}

GreatCircle::~GreatCircle()
{
	--greatCircleClassCount;
	
	// Note:  GreatCircle does not delete the two CrustalProfile objects
	// to which it has references.  Those objects are managed by the 
	// CrustalProfileStore objects owned by Grid.

	// delete all the LayerProfile objects.
	for (int i=0; i<(int)profiles.size(); i++)
		if (profiles[i]) delete profiles[i];
}

//! \brief Copy constructor.
//! 
//! Copy constructor.
GreatCircle::GreatCircle(const GreatCircle &other)
: grid(other.grid),
	phase(other.phase),
	headWaveInterface(other.headWaveInterface),
	source(other.source),
	receiver(other.receiver),
	profiles(other.profiles),
	tTotal(other.tTotal),
	actual_path_increment(other.actual_path_increment),
	distance(other.distance),
	esaz(other.esaz),
	rayParameter(other.rayParameter),
	xSource(other.xSource),
	tSource(other.tSource),
	xReceiver(other.xReceiver),
	tReceiver(other.tReceiver),
	xHorizontal(other.xHorizontal),
	tHorizontal(other.tHorizontal),
	tGamma(other.tGamma),
	sourceRayParameter(other.sourceRayParameter),
	receiverRayParameter(other.receiverRayParameter),
	turningRadius(other.turningRadius),
	sourceIndex(other.sourceIndex),
	receiverIndex(other.receiverIndex),
	ttHminus(other.ttHminus),
	ttHplus(other.ttHplus),
	//ttZplus(other.ttZplus),
	ttHZplus(other.ttHZplus),
	ttEast(other.ttEast),
	ttWest(other.ttWest),
	ttEastZ(other.ttEastZ),
	ttNorth(other.ttNorth),
	ttSouth(other.ttSouth),
	ttNorthZ(other.ttNorthZ)

 {
	++greatCircleClassCount;	
}

//! \brief Equal operator.
//! 
//! Equal operator.
GreatCircle& GreatCircle::operator=(const GreatCircle& other)
{
	grid     = other.grid;
	source   = other.source;
	receiver = other.receiver;
	phase    = other.phase;
	profiles = other.profiles;
	actual_path_increment    = other.actual_path_increment;
	distance = other.distance;
	esaz = other.esaz;
	ttHminus = other.ttHminus;
	ttHplus = other.ttHplus;
	ttHZplus = other.ttHZplus;

	ttNorth = other.ttNorth;
	ttSouth = other.ttSouth;
	ttNorthZ = other.ttNorthZ;

	ttEast = other.ttEast;
	ttWest = other.ttWest;
	ttEastZ = other.ttEastZ;

	//ttZplus = other.ttZplus;

	return *this;
}

LayerProfile* GreatCircle::getProfile(const int& i)
{
	if (!profiles[i])
	{
		source->getLocation().move(vtp, ((double)i+0.5)*actual_path_increment, location);
		profiles[i] = grid.getLayerProfile(this, location);
		if (profiles[i] == NULL)
		{
			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);
			  os << endl << "ERROR in GreatCircle::getProfile" << endl
				<< "Unable to interpolate a LayerProfile along head wave interface." << endl
				<< "lat, lon (deg) = " << location.getLatDegrees() << ", " << location.getLonDegrees() 
				<< "is outside of model range."	<< endl
				  << "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),600);
		}
	}
	return profiles[i];
}

void GreatCircle::getData(
		int& p,
		double& d,
		double sourceDepth[NLAYERS],
		double sourceVelocity[NLAYERS],
		double receiverDepth[NLAYERS],
		double receiverVelocity[NLAYERS],
		int& npoints,
		double headWaveVelocity[],
		double gradient[]
	)
{
	p = phase;
	d = actual_path_increment;

	for (int i=0; i<NLAYERS; i++)
	{
		sourceDepth[i]      = source->getDepth(i);
		sourceVelocity[i]   = source->getVelocity(i);
		receiverDepth[i]    = receiver->getDepth(i);
		receiverVelocity[i] = receiver->getVelocity(i);
	}

	npoints = (int)profiles.size();

	for (int i=0; i<npoints; i++)
	{
		getProfile(i); // ensure that profiles[i] has been instantiated.
		headWaveVelocity[i] = profiles[i]->getVelocity();

		if (phase==Pn || phase==Sn) 
			gradient[i] = profiles[i]->getGradient();
		else 
			gradient[i] = NA_VALUE;
	}
}

void GreatCircle::getData(
		int& p,
		double& d,
		vector<double>& sourceDepth,
		vector<double>& sourceVelocity,
		vector<double>& receiverDepth,
		vector<double>& receiverVelocity,
		vector<double>& headWaveVelocity,
		vector<double>& gradient
	)
{
	p = phase;
	d = actual_path_increment;

	sourceDepth.clear();          
	sourceVelocity.clear();       
	receiverDepth.clear();        
	receiverVelocity.clear();     

	for (int i=0; i<NLAYERS; i++)
	{
		sourceDepth.push_back(source->getDepth(i));
		sourceVelocity.push_back(source->getVelocity(i));
		receiverDepth.push_back(receiver->getDepth(i));
		receiverVelocity.push_back(receiver->getVelocity(i));
	}

	headWaveVelocity.clear();     
	gradient.clear();             

	for (int i=0; i<(int)profiles.size(); i++)
	{
		getProfile(i); // ensure that profiles[i] has been instantiated.
		headWaveVelocity.push_back(profiles[i]->getVelocity());
		if (phase==Pn || phase==Sn) 
			gradient.push_back(profiles[i]->getGradient());
		else 
			gradient.push_back(NA_VALUE);
	}
}

void GreatCircle::getNodeInfo(
	int** neighbors,
	double** coefficients,
	const int& maxpoints,
	const int& maxnodes,
	int& npoints,
	int* nnodes
	)
{
	npoints = (int)profiles.size();

	if (npoints > maxpoints)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);
		os << endl << "ERROR in GreatCircle::getNodeInfo" << endl
				<< "npoints = " << npoints << " is greater than maxpoints = " << maxpoints	<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),600);
	}

	LayerProfile* p;
	int requiredSize = 0;
	for (int i=0; i<npoints; i++)
	{
		p = getProfile(i);

		nnodes[i] = (int)p->getNodes().size();

		if (nnodes[i] > requiredSize)
			requiredSize = nnodes[i];

		if (nnodes[i] <= maxnodes)
			for (int j=0; j<nnodes[i]; j++)
			{
				neighbors[i][j] = p->getNodes()[j]->getNodeId();
				coefficients[i][j] = p->getCoefficients()[j];
			}
	}

	if (requiredSize > maxnodes)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);
		os << endl << "ERROR in GreatCircle::getNodeInfo" << endl
				<< "nnodes = " << requiredSize << " is greater than maxnodes = " << maxnodes	<< endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),600);
	}

}

void GreatCircle::getNodeInfo(
	vector<vector<int> >& neighbors,
	vector<vector<double> >& coefficients
	)
{
	int nnodes, npoints = (int)profiles.size();
	neighbors.resize(npoints);
	coefficients.resize(npoints);

	LayerProfile* p;
	for (int i=0; i<npoints; i++)
	{
		p = getProfile(i);
		nnodes = p->getNCoefficients();
		neighbors[i].resize(nnodes);
		coefficients[i].resize(nnodes);

		for (int j=0; j<nnodes; j++)
		{
			neighbors[i][j] = p->getNodes()[j]->getNodeId();
			coefficients[i][j] = p->getCoefficients()[j];
		}
	}
}

void GreatCircle::getWeights(vector<int>& nodeids, vector<double>& weights)
{
	int i;
	double d, r;
	GridProfile* gp;

	weights.clear();
	nodeids.clear();
	for (int i=0; i<(int)profiles.size(); i++)
	{
		d=getActualPathIncrement(i);
		if (d > 0.)
		{
			r = getProfile(i)->getRadius();
			for (int j=0; j<getProfile(i)->getNCoefficients(); j++)
			{
				gp = profiles[i]->getNode(j);
				if (gp->getWeight() == NA_VALUE)
				{
					nodeids.push_back(gp->getNodeId());
					gp->setWeight(0.);
				}
				// increment the GridProfile weight
				gp->addWeight(
					d * r * profiles[i]->getCoefficient(j));
			}
		}
	}

	// fill weights and node ids
	for (i=0; i<(int)nodeids.size(); i++)
	{
		gp = grid.getProfile(nodeids[i]);
		weights.push_back(gp->getWeight());
		gp->setWeight(NA_VALUE);
		// record fact that this gridProfile was touched by a ray.
		gp->incrementHitCount();
	}
}

void GreatCircle::getWeights(int nodeids[], double weights[], int& nweights)
{
	int i;
	double d, r;
	GridProfile* gp;

	nweights = 0;
	for (int i=0; i<(int)profiles.size(); i++)
	{
		d=getActualPathIncrement(i);
		if (d > 0.)
		{
			r = getProfile(i)->getRadius();
			for (int j=0; j<getProfile(i)->getNCoefficients(); j++)
			{
				gp = profiles[i]->getNodes()[j];
				if (gp->getWeight() == NA_VALUE)
				{
					nodeids[nweights++] = gp->getNodeId();
					gp->setWeight(0.);
				}
				// increment the GridProfile weight
				gp->addWeight(
					d * r * profiles[i]->getCoefficients()[j]);
			}
		}
	}

	// fill weights and node ids
	for (i=0; i<nweights; i++)
	{
		gp = grid.getProfile(nodeids[i]);
		weights[i] = gp->getWeight();
		gp->setWeight(NA_VALUE);
		// record fact that this gridProfile was touched by a ray.
		gp->incrementHitCount();
	}
}

void GreatCircle::getLayerProfileLocation(const int& i, Location& loc)
{ 
	source->getLocation().move(vtp, ((double)i+0.5)*actual_path_increment, loc);	
	loc.setRadius(getProfile(i)->getRadius());
}

void GreatCircle::getGreatCircleLocation(const double& dist, Location& loc)
{ 
	source->getLocation().move(vtp, dist, loc);
	loc.setRadius(NA_VALUE);
}

int GreatCircle::getClassCount()
{
  return greatCircleClassCount;
}

void GreatCircle::setNAValues()
{ 
	rayParameter = NA_VALUE;
	receiverRayParameter = NA_VALUE;
	sourceRayParameter = NA_VALUE;
	tGamma = NA_VALUE;
	tHorizontal = NA_VALUE;
	tReceiver = NA_VALUE;
	tSource = NA_VALUE;
	ttEast = NA_VALUE;
	ttEastZ = NA_VALUE;
	ttHZplus = NA_VALUE;
	ttHminus = NA_VALUE;
	ttHplus = NA_VALUE;
	ttNorth = NA_VALUE;
	ttNorthZ = NA_VALUE;
	ttSouth = NA_VALUE;
	ttWest = NA_VALUE;
	turningRadius = NA_VALUE;
	xHorizontal = NA_VALUE;
	xReceiver = NA_VALUE;
	xSource = NA_VALUE;
}

//! \brief Retrieve source-receiver azimuth, in radians.
//! 
//! Retrieve source-receiver azimuth, in radians.
//! @return the source-receiver azimuth, in radians.
double GreatCircle::getEsaz()
{
	if (esaz == NA_VALUE)
		esaz = source->getLocation().azimuth(receiver->getLocation());
	return esaz;
}


double GreatCircle::get_ttZplus()
{
	GreatCircle* gc = GreatCircleFactory::create(
		getPhase(), &grid, 
		getSourceProfile()->getLocation().getLat(), 
		getSourceProfile()->getLocation().getLon(), 
		(getSourceProfile()->getLocation().getDepth()+DEL_DEPTH),
		getReceiverProfile()->getLocation().getLat(),
		getReceiverProfile()->getLocation().getLon(),
		getReceiverProfile()->getLocation().getDepth(),
		1e30);

	double ttZplus = gc->getTravelTime();
	delete gc;

	return ttZplus;
}

double GreatCircle::get_ttZminus()
{
	GreatCircle* gc = GreatCircleFactory::create(
		getPhase(), &grid, 
		getSourceProfile()->getLocation().getLat(), 
		getSourceProfile()->getLocation().getLon(), 
		(getSourceProfile()->getLocation().getDepth()-DEL_DEPTH),
		getReceiverProfile()->getLocation().getLat(),
		getReceiverProfile()->getLocation().getLon(),
		getReceiverProfile()->getLocation().getDepth(),
		1e30);

	double ttZminus = gc->getTravelTime();
	delete gc;

	return ttZminus;
}

double GreatCircle::get_ttHminus()
{
	if (ttHminus == NA_VALUE)
	{
		Location position;
		//getSourceProfile()->getLocation().move(getEsaz(), DEL_DISTANCE, position);
        getGreatCircleLocation(DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth(),
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttHminus = gc->getTravelTime();

		delete gc;
    }
	return ttHminus;
}

double GreatCircle::get_ttHplus()
{
	if (ttHplus == NA_VALUE)
	{
		Location position;
        getGreatCircleLocation(-DEL_DISTANCE, position);
		
		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth(),
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttHplus = gc->getTravelTime();

		delete gc;
    }
	return ttHplus;
}

double GreatCircle::get_ttHZplus()
{
	if (ttHZplus == NA_VALUE)
	{

		Location position;
		getSourceProfile()->getLocation().move(getEsaz()+PI, DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth()+DEL_DEPTH,
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttHZplus = gc->getTravelTime();
		delete gc;
    }
	return ttHZplus;
}

double GreatCircle::get_ttNorth()
{
	if (ttNorth == NA_VALUE)
	{
		Location position;
		getSourceProfile()->getLocation().move_north(DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth(),
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttNorth = gc->getTravelTime();
		delete gc;
    }
	return ttNorth;
}

double GreatCircle::get_ttSouth()
{
	if (ttSouth == NA_VALUE)
	{
		Location position;
		getSourceProfile()->getLocation().move_north(-DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth(),
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttSouth = gc->getTravelTime();
		delete gc;
    }
	return ttSouth;
}

double GreatCircle::get_ttNorthZ()
{
	if (ttNorthZ == NA_VALUE)
	{

		Location position;
		getSourceProfile()->getLocation().move_north(DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth()+DEL_DEPTH,
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttNorthZ = gc->getTravelTime();
		delete gc;
    }
	return ttNorthZ;
}

double GreatCircle::get_ttEast()
{
	if (ttEast == NA_VALUE)
	{
		Location position;
		getSourceProfile()->getLocation().move(PI/2., DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth(),
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttEast = gc->getTravelTime();
		delete gc;
    }
	return ttEast;
}

double GreatCircle::get_ttWest()
{
	if (ttWest == NA_VALUE)
	{
		Location position;
		getSourceProfile()->getLocation().move(
			-PI/2, DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth(),
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttWest = gc->getTravelTime();
		delete gc;
    }
	return ttWest;
}

double GreatCircle::get_ttEastZ()
{
	if (ttEastZ == NA_VALUE)
	{

		Location position;
		getSourceProfile()->getLocation().move(PI/2, DEL_DISTANCE, position);

		GreatCircle* gc = GreatCircleFactory::create(
			getPhase(), &grid, 
			position.getLat(), position.getLon(), 
			getSourceProfile()->getLocation().getDepth()+DEL_DEPTH,
			getReceiverProfile()->getLocation().getLat(),
			getReceiverProfile()->getLocation().getLon(),
			getReceiverProfile()->getLocation().getDepth(),
			1e30);

		ttEastZ = gc->getTravelTime();
		delete gc;
    }
	return ttEastZ;
}

void GreatCircle::get_dtt_ddepth(double& dtt_ddepth)
{
	if (getTravelTime() > -1.)
	{
		// if del_depth will span the moho, then make the step up
		// by using -DEL_DISTANCE
		if (getSourceProfile()->isInCrust() 
			&& getSourceProfile()->getLocation().getDepth()+DEL_DEPTH 
			>= getSourceProfile()->getDepth(MANTLE))
				dtt_ddepth = (getTravelTime() - get_ttZminus())/DEL_DEPTH;
		else
			dtt_ddepth = (get_ttZplus() - getTravelTime())/DEL_DEPTH;
	}
	else
		dtt_ddepth = NA_VALUE;
}

//void GreatCircle::get_dsh_dlat(double& dsh_dlat)
//{
//	double azim, tt0, ttplus, slow, slowPlus;
//
//	// find position small distance north of current point.
//	Location pointNorth;
//	getSourceProfile()->getLocation().move_north(DEL_DISTANCE, pointNorth);
//
//	GreatCircle* gc = GreatCircleFactory::create(
//			getPhase(), &grid, 
//			pointNorth.getLat(), pointNorth.getLon(), 
//			getSourceProfile()->getLocation().getDepth(),
//			getReceiverProfile()->getLocation().getLat(),
//			getReceiverProfile()->getLocation().getLon(),
//			getReceiverProfile()->getLocation().getDepth(),
//			1e30);
//
//	gc->get_dtt_ddist(slowPlus);
//
//	get_dtt_ddist(slow);
//
//	dsh_dlat = (slowPlus - slow)/DEL_DISTANCE;
//
//	delete gc;
//
//	//get_dsh_ddist(dsh_dlat);
//	//dsh_dlat *= cos(getEsaz()+PI);
//
//}
//
//void GreatCircle::get_dsh_dlon(double& dsh_dlon)
//{
//	double azim, tt0, ttplus, slow, slowPlus;
//
//	// find position small distance east of current point.
//	Location pointEast;
//	getSourceProfile()->getLocation().move(PI/2, DEL_DISTANCE, pointEast);
//
//	GreatCircle* gc = GreatCircleFactory::create(
//			getPhase(), &grid, 
//			pointEast.getLat(), pointEast.getLon(), 
//			getSourceProfile()->getLocation().getDepth(),
//			getReceiverProfile()->getLocation().getLat(),
//			getReceiverProfile()->getLocation().getLon(),
//			getReceiverProfile()->getLocation().getDepth(),
//			1e30);
//
//	gc->get_dtt_ddist(slowPlus);
//
//	get_dtt_ddist(slow);
//
//	dsh_dlon = (slowPlus - slow)/DEL_DISTANCE;
//
//	delete gc;
//
//	//get_dsh_ddist(dsh_dlon);
//	//dsh_dlon *= sin(getEsaz()+PI);
//}
//
//void GreatCircle::get_dsh_ddepth(double& dsh_ddepth)
//{
//	double slow, slowPlus;
//
//	GreatCircle* gc = GreatCircleFactory::create(
//			getPhase(), &grid, 
//			getSourceProfile()->getLocation().getLat(), 
//			getSourceProfile()->getLocation().getLon(), 
//			(getSourceProfile()->getLocation().getDepth()+DEL_DEPTH),
//			getReceiverProfile()->getLocation().getLat(),
//			getReceiverProfile()->getLocation().getLon(),
//			getReceiverProfile()->getLocation().getDepth(),
//			1e30);
//
//	gc->get_dtt_ddist(slowPlus);
//
//	get_dtt_ddist(slow);
//
//	dsh_ddepth = (slowPlus - slow)/DEL_DEPTH;
//
//	delete gc;
//}

} // end slbm namespace
