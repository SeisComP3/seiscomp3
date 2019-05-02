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
//- Program:       GreatCircle_Xg
//- Module:        $RCSfile: GreatCircle_Xg.cc,v $
//- Revision:      $Revision: 1.56 $
//- Last Modified: $Date: 2012/12/03 15:21:51 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "GreatCircle_Xg.h"
#include "Grid.h"
#include "Location.h"
#include "CrustalProfile.h"
#include "LayerProfile.h"
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

GreatCircle_Xg::GreatCircle_Xg(
		const int& _phase,
		Grid& _grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver)
	: GreatCircle(_phase, _grid, 
		latSource, lonSource, depthSource, 
		latReceiver, lonReceiver, depthReceiver),
		taupModelRadius(6471.),
		taupResult(NULL)

{
	if (getDistance() > MAX_DISTANCE)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in GreatCircle_Xg::GreatCircle_Xg()" << endl
			<< "Source-receiver separation exceeds maximum value." << endl
			<< "Source-receiver separation (degrees) : " << getDistance()*RAD_TO_DEG << endl
			<< "Maximum allowed separation (degrees) : " << MAX_DISTANCE*RAD_TO_DEG << endl
			<< "Source   location : " << source->getLocation().toString() << endl
			<< "Receiver location : " << receiver->getLocation().toString() << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),201);
	}

	// if either source or receiver is below the Moho, throw error.  Pg/Lg is
	// a crustal phase.
	if (!source->isInCrust() || !receiver->isInCrust())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in GreatCircle_Xg constructor" << endl
			<< "Pg/Lg not valid because source or receiver is below the Moho." << endl
			<< "Source-receiver separation (deg) = " 
			<< source->getLocation().distance(receiver->getLocation())*RAD_TO_DEG << endl
			<< "Receiver lat, lon, depth, radius, moho depth : " 
			<< receiver->getLocation().toString() << "  "
			<< receiver->getLocation().getRadius() << "  "
			<< receiver->getDepth(MANTLE) << endl
			<< "Source   lat, lon, depth, radius, moho depth : " 
			<< source->getLocation().toString() << "  "
			<< source->getLocation().getRadius() << "  "
			<< source->getDepth(MANTLE) << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),200);
	}

  // see if a TauP site needs to be constructed for this receiver
  if (receiver->getTauPSite() == 0)
  {
    // This is a new receiver ... build a new TauPSite using the crustal velocity
    // stack assigned to the receiver. First create the TauP site and set
    // the earth radius and the receiver depth.

    string phtype = "P";
    if (phase == 1)
      phtype = "S";
    else if (phase == 3)
      phtype = "L";

	// create the taup model
    taup::TauPSite* tps = new taup::TauPSite("", phtype);
    tps->setEarthRadius(taupModelRadius);
    tps->setSiteDepth(taupModelRadius - receiver->getLocation().getRadius());

    // append each constant velocity model to its velocity stack ... first get
    // number of non zero-thickness layers and their indirect indicess

    int nlay = receiver->getNLayid();
    const int* layid = receiver->getLayid();

    // loop over each constant layer and create the TauPSite velocity model.
	// Ignore layers whose bottom is higher than the model radius.
    double v, rtop, rbot;

    for (int i = 0; i < nlay-1; ++i) 
    {
		// find the bottom of this layer
		rbot = receiver->getInterfaceRadius(layid[i+1]);
	  
		// skip this layer if its bottom is above the model radius
		if (rbot >= taupModelRadius) continue;

		if (tps->getVelocityModels().size() == 0)
			rtop = taupModelRadius;
		else
			rtop = receiver->getInterfaceRadius(layid[i]);

		v = receiver->getVelocity(layid[i]);

		tps->appendConstVelocityModel(v, rtop, rbot);
    }

    // finally add the mantle with constant velocity.  
	tps->appendConstVelocityModel(v, rbot, 0.);

 
	// finished ... assign into the receiver

    receiver->setTauPSite(tps);

  }

  computeTravelTime();
}

GreatCircle_Xg::~GreatCircle_Xg()
{
}

GreatCircle_Xg::GreatCircle_Xg(const GreatCircle_Xg &other)
: GreatCircle(other)
{
}

GreatCircle_Xg& GreatCircle_Xg::operator=(const GreatCircle_Xg& other)
{
	GreatCircle::operator = (other);
	return *this;
}

void GreatCircle_Xg::computeTravelTime()
{
	xSource=xReceiver=xHorizontal=tSource=tReceiver=tHorizontal=tTotal=0;

	tHeadwave = pHeadwave = trHeadwave = pSource = NA_VALUE;

	solutionMethod = "neither";

	// attempt to compute the travel time using the TauP method
	computeTravelTimeTaup();

	// Added SCM 1/23/08
	if (taupResult && taupResult->ttrRayType =="DownGoing")
	  pTaup = taupResult->ttrP / taupResult->ttrR;
	else
	  pTaup = -1; 

	computeTravelTimeHeadwave();

	// if both methods produced valid results
	if (taupResult && tHeadwave != NA_VALUE)
	{
		// SCM 1/24/08. Is the source in the upper crust?
		if (source->getLocation().getRadius() >= source->getInterfaceRadius(MIDDLE_CRUST_G))
		{
			// specify that taup is preferred solution under following conditions:
			// 1) headwave ray parameter is less than taup ray parameter, or
			// 2) taup says the ray leaves source in upgoing direction, or
			// 3) taup travel time is less than headwave travel time

			// SCM 1/24/08.  changed test from ray parameter to bottoming depth 
			// if taup ray bottoms above the mid crustal layer (measured under the station, 
			// or if the ray is upgoing, then use taup.
			if ((int)profiles.size() == 0 
				|| taupResult->ttrR >  getProfile( (int)profiles.size()-1)->getRadius()
				|| taupResult->ttrRayType == "UpGoing")
					solutionMethod = "GreatCircle_Xg::computeTravelTimeTaup()";
			else
				solutionMethod = "GreatCircle_Xg::computeTravelTimeHeadwave()";
		}
		else
		{
			// SCM 1/24/08.  If source is in the lower crust,then ...
			// if ray is upgoing and taup_time>headeave_time then use taup.
			// the constraint on upgoing is straightforward. 
			// the constraint on arriving after the headwave insures that the local
			// calculation  will transition to the regional (headwave) calculation
			// and the transition will be piecewise continuous.
			if (taupResult->ttrRayType == "UpGoing" && taupResult->ttrT > tHeadwave)
				solutionMethod = "GreatCircle_Xg::computeTravelTimeTaup()";
			else
				solutionMethod = "GreatCircle_Xg::computeTravelTimeHeadwave()";
		}

	}
	// if only taup produced valid result
	else if (taupResult)
		solutionMethod = "GreatCircle_Xg::computeTravelTimeTaup()";
	// if only headwave produced valid result
	else if (tHeadwave != NA_VALUE)
		solutionMethod = "GreatCircle_Xg::computeTravelTimeHeadwave()";
	
	if (solutionMethod == "GreatCircle_Xg::computeTravelTimeTaup()")
	{
		tTotal = taupResult->ttrT;
		rayParameter = taupResult->ttrP;
		turningRadius = taupResult->ttrR;
	}
	else if (solutionMethod == "GreatCircle_Xg::computeTravelTimeHeadwave()")
	{
		tTotal = tHeadwave;
		rayParameter = pHeadwave;
		turningRadius = trHeadwave;
	}
	else
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in GreatCircle_Xg::computeTravelTime" << endl
			<< "computeTravelTimeTaup() and computeTravelTimeHeadwave() both returned NA_VALUE." << endl
			<< "Source-receiver separation (deg) = " 
			<< source->getLocation().distance(receiver->getLocation())*RAD_TO_DEG << endl
			<< "Receiver lat, lon, depth, radius, moho depth : " 
			<< receiver->getLocation().toString() << "  "
			<< receiver->getLocation().getRadius() << "  "
			<< receiver->getDepth(MANTLE) << endl
			<< "Source   lat, lon, depth, radius, moho depth : " 
			<< source->getLocation().toString() << "  "
			<< source->getLocation().getRadius() << "  "
			<< source->getDepth(MANTLE) << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),400);
	}

}

void GreatCircle_Xg::computeTravelTimeTaup()
{
	// get the receiver's TauPSite object 
	taup::TauPSite* tps = receiver->getTauPSite();

	// evaluate the TauP travel times for the receiver stack at the source
	// distance and depth.  Source depth is the taup model radius minus
	// the source radius.
	// The last arguement==false means don't compute derivatives.
	tps->calculateTravelTimes(getDistance(), 
		taupModelRadius - source->getLocation().getRadius(), false);

	// retrieve the TravelTimeResult object for the first arrival
	taupResult = tps->getFirstTravelTimeResult();
}


void GreatCircle_Xg::computeTravelTimeHeadwave()
{
	double dkm;

	sourceIndex = 0;
	receiverIndex = max(0, (int)profiles.size()-1);

	// profiles is vector of LayerProfile objects positioned along the top of
	// the middle crust (Pg/Lg version).  They are positioned along the great
	// circle path from source to receiver.
	for (int i=0; i<(int)profiles.size(); i++)
	{
		// actual_path_increment is the horizontal angular increment in 
		// radians (defaults to 0.1 degree).  Multiply by radius of top 
		// of MIDDLE_CRUST_G to convert to km.
		dkm = actual_path_increment * getProfile(i)->getRadius();

		// sum horizontal distance along top of middle crust.
		xHorizontal +=  dkm;  

		// travel time is horizontal distance divided by velocity
		tHorizontal += dkm / profiles[i]->getVelocity();
	}

	// the ray parameter is = slowness = time / horizontal distance.
	// A flat earth is assumed, so the units here are sec/km
	if (pHeadwave == NA_VALUE)
	{
		if (xHorizontal > 0.)
			pHeadwave = tHorizontal / xHorizontal;
		else
			pHeadwave = 0.;
	}

	// getLayid returns the indeces of the crustal layers that have non-zero
	// thickness.  
	const int* rlayid = receiver->getLayid();

	int j, k=receiver->getTopLayid();
	double h;

	// loop over layers starting with layer that contains the receiver and
	// ending with upper crust.
	while (rlayid[k] < MIDDLE_CRUST_G)
	{
		j = rlayid[k];

		if (k == receiver->getTopLayid())
			// set h = the radius of the receiver minus radius of top of next 
			// interface
			h = receiver->getLocation().getRadius()-receiver->getInterfaceRadius(j+1);
		else 
			// set h = the radius of the top of layer[j] minus top of layer[j+1]
			h = receiver->getInterfaceRadius(j)-receiver->getInterfaceRadius(j+1);

		tReceiver += h/receiver->getVelocity(j) 
			* sqrt(max( 0., 1.-sqr(pHeadwave*receiver->getVelocity(j))));

		++k;
	}

	if (source->getLocation().getRadius() >= source->getInterfaceRadius(MIDDLE_CRUST_G))
	{
		// source is above the top of the middle crust.  
		
		// Same calculation as for receiver.

		pSource = pHeadwave;

		// turning radius is top of middle crust
		trHeadwave = source->getInterfaceRadius(MIDDLE_CRUST_G);

		k = source->getTopLayid();
		const int* slayid = source->getLayid();
		while (slayid[k] < MIDDLE_CRUST_G)
		{
			j = slayid[k];
			
			// the top of the layer. 
			if (k == source->getTopLayid())
				// set r = the radius of the source.
				h = source->getLocation().getRadius()-source->getInterfaceRadius(j+1);
			else 
				// set r = the radius of the top of layer[j]
				h = source->getInterfaceRadius(j)-source->getInterfaceRadius(j+1);

			tSource += h/source->getVelocity(j) 
				* sqrt(max(0., 1.-sqr(pHeadwave*source->getVelocity(j))));

			++k;
		}
	}
	else 
	{
		// Source is in either the middle crust or the lower crust.
		// It is assumed here that the lower crust has the same velocity
		// as the middle crust.

		// turning radius is source depth.
		trHeadwave = source->getLocation().getRadius();

		h = source->getInterfaceRadius(MIDDLE_CRUST_G) 
			- source->getLocation().getRadius();

		// set ray parameter using velocity of middle crust

		pSource = 0.997/source->getVelocity(MIDDLE_CRUST_G);

		// add travel time through middle and lower crust, using velocity
		// of middle crust for the entire interval.
		// SCM 1/24/08, it didn't look like the p*v was being squared
		// tSource += h/source->getVelocity(MIDDLE_CRUST_G) * .0697565 ; // sqrt(1.-sqr(pSource*v)));
		// tSource += h/source->getVelocity(MIDDLE_CRUST_G) * .003 ; // sqrt(1.-sqr(pSource*pSource*v*v)));

		// SCM this isn't really the ray parameter. Ray parameter would be radius_source/radius_midCrust/velocity_midCrust
		// However, we would multiply ray parameter by velocity_midCrust in the slowness calculation (3 lines down) 
		// leaving radius_source/radious_midCrust
		pTaup = source->getLocation().getRadius() / source->getInterfaceRadius(MIDDLE_CRUST_G);
			tSource += h/source->getVelocity(MIDDLE_CRUST_G) * sqrt(1.-pTaup*pTaup); // sqrt(1.-sqr(pSource*pSource*v*v)));			
	}

	// total equals the sum of the parts.
    tHeadwave = tSource + tReceiver + tHorizontal;

}


string GreatCircle_Xg::toString(const int& verbosity)
{
	if (verbosity < 1) return "";

	ostringstream os;

	os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);

	os << "   SLBM Version " << SlbmVersion << endl << endl;

	if (Location::EARTH_RADIUS > 0.)
		os << "   Earth radius = constant " << Location::EARTH_RADIUS 
			<< " km." << endl << endl;
	else
		os << "   Earth radius varies as a function of latitude. "
			<< endl << endl;

	os << "   Phase = " << getPhaseString() << endl << endl;

	os << "   Source-Receiver separation = "
		<< getDistance()*RAD_TO_DEG << " degrees" << endl << endl;

	os << "   Headwave travel time       = "
		 << tHeadwave << endl << endl;

	os << "   Taup travel time           = "
		 << (taupResult ? taupResult->ttrT : NA_VALUE) << endl << endl;

	os << "   Solution method = " << solutionMethod << endl << endl;

	if (solutionMethod.find("Taup") != string::npos)
		toStringTaup(os, verbosity);
	else if (solutionMethod.find("Headwave") != string::npos)
		toStringHeadwave(os, verbosity);

	return os.str();
}

void GreatCircle_Xg::toStringTaup(ostringstream& os, const int& verbosity)
{
	os << "   Travel time                = " << tTotal << endl << endl;

	taup::TravelTimeResult* ttr = receiver->getTauPSite()->getFirstTravelTimeResult();

	os << "   Ray parameter              = " << ttr->ttrP << endl << endl;

	os << "   Turning Depth              = " << 
		receiver->getLocation().getEarthRadius()-ttr->ttrR << endl << endl;

	if (verbosity >= 3)
	{
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);

		os << "   Source Profile:" << endl << endl
			<< source->toString() << endl;

		os << endl << "   Receiver Profile:" << endl << endl
			<< receiver->toString() << endl;

	}

	os << receiver->getTauPSite()->toString() << endl << endl;

	os << taupResult->toString();
}

void GreatCircle_Xg::toStringHeadwave(ostringstream& os, const int& verbosity)
{
	int n = getHeadWaveInterface();

	int w = 10;

	os << "               " 
		<< setw(w) << "X, deg"
		<< setw(w) << "X, km"
		<< setw(w) << "T, sec"
		<< setw(w) << "    P, sec/km"
		<< endl;

	os << "   Source    = " 
		<< setw(w) << xSource * RAD_TO_DEG
		<< setw(w) << xSource * source->getInterfaceRadius(n)
		<< setw(w) << tSource 
		<< setw(w) << pSource 
		<< endl;
	os << "   Receiver  = " 
		<< setw(w) << xReceiver * RAD_TO_DEG
		<< setw(w) << xReceiver * receiver->getInterfaceRadius(n)
		<< setw(w) << tReceiver 
		<< setw(w) << rayParameter 
		<< endl;
	os << "   Head wave = " 
		<< setw(w) << (getDistance() - xSource - xReceiver) * RAD_TO_DEG
		<< setw(w) << xHorizontal 
		<< setw(w) << tHorizontal 
		<< endl;
	os << "   Total     = " 
		<< setw(w) << source->getLocation().distanceDegrees(receiver->getLocation())
		<< setw(w) << xSource* source->getInterfaceRadius(n)
				+xReceiver* receiver->getInterfaceRadius(n)
				+xHorizontal
		<< setw(w) << tSource+tReceiver+tHorizontal
		<< endl << endl;

	os << "   Fraction Active          = " << setw(w) << getFractionActive() 
		<< endl << endl;

	// verbosity == 2 does not apply to Pg, Lg.  See GreatCircle_Xn.cc

	if (verbosity >= 3)
	{
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);

		os << "   Source Profile:" << endl << endl
			<< source->toString() << endl;

		os << endl << "   Receiver Profile:" << endl << endl
			<< receiver->toString() << endl;

		if (verbosity >= 4)
		{
			os << endl << "   Grid Node Weights:" << endl << endl;
			os << setw(13)   << "grid_id" 
				<< setw(12) << "active_id" 
				<< setw(15) << "weight" << endl;
			os << setprecision(8);
			vector<int> nodeids;
			vector<double> weights;
			getWeights(nodeids, weights);
			double sum = 0.;
			for (int i=0; i<(int)nodeids.size(); i++)
			{
				os << "      "
					<< " " << setw(6) << nodeids[i]
					<< " " << setw(11) << grid.getActiveNodeId(nodeids[i])
					<< " " << setw(14) << weights[i]
					<< endl;
				sum += weights[i];
			}
			os << endl << "      Sum of weights = " << sum << " km" << endl;
		
			if (verbosity >= 5)
			{
				os << endl << "   Head wave Profiles:" << endl << endl;

				int w = 8;
				os << setw(6) << "#"
					<< setw(w)   << "Lat" << setw(w+1) << "Lon" << setw(w) << "Head"
					<< setw(w+1)   << "R Earth" << setw(w)   << "Vel";

				os << setw(w+1) << "dx" 
					<< setw(w+1) << "Active" 
					<< endl;
				
				for (int i=0; i<(int)profiles.size(); i++)
				{
					getProfile(i); // ensure profiles[i] instantiated
					// find location
					source->getLocation().move(vtp, ((double)i+0.5)*actual_path_increment, location);

					os << setw(6) << i;

					os  << setprecision(3)
						<< setw(w) << location.getLatDegrees()
						<< setw(w+1) << location.getLonDegrees()
						<< setprecision(3)
						<< setw(w) << location.getEarthRadius()-profiles[i]->getRadius()
						<< setprecision(2)
						<< setw(w+1) << location.getEarthRadius()
						<< setprecision(4)
						<< setw(w) << profiles[i]->getVelocity()
						<< setw(w+1) << setprecision(4)
						<< getActualPathIncrement(i)*RAD_TO_DEG;

						if (getActualPathIncrement(i) < 1e-9)
							os << "      -";
						else if (profiles[i]->isActiveProfile())
							os << "     y";
						else 
							os << "    n";

						os << endl;

				}

				os << endl;

				if (verbosity >= 6)
				{
					
					os << endl << "   Head wave Profile Interpolation Coefficients:" << endl << endl;
					os << setw(6) << "#";
					for (int i=0; i<profiles[i]->getNCoefficients(); i++)
						os << setw(7)   << "id" << setw(10) << "coeff";
					os << endl;
					for (int i=0; i<(int)profiles.size(); i++)
					{
						os << setw(6) << i << setprecision(6);
						for (int j=0; j<profiles[i]->getNCoefficients(); j++)
							os << " " << setw(6) << profiles[i]->getNodes()[j]->getNodeId()
								<< " " << setw(9) << profiles[i]->getCoefficients()[j];
						os << endl;
					}
						os << endl;
				}
			}
		}
	}
}

} // end slbm namespace
