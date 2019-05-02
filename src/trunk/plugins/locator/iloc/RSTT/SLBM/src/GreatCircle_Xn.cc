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
//- Program:       GreatCircle_Xn
//- Module:        $RCSfile: GreatCircle_Xn.cc,v $
//- Revision:      $Revision: 1.45 $
//- Last Modified: $Date: 2012/12/19 17:10:03 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include <limits>

#include "GreatCircle_Xn.h"
#include "Grid.h"
#include "Location.h"
#include "CrustalProfile.h"
#include "LayerProfile.h"
#include "SLBMGlobals.h"
#include <iostream>

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

GreatCircle_Xn::GreatCircle_Xn(
		const int& _phase,
		Grid& _grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver, 
		const double& chMax)
	: GreatCircle(_phase, _grid, 
		latSource, lonSource, depthSource, 
		latReceiver, lonReceiver, depthReceiver),
		ch_max(chMax),
		udSign(-999),
		cmin(1e-6)
{
	computeTravelTime();
}

GreatCircle_Xn::~GreatCircle_Xn()
{
}

//! \brief Copy constructor.
//! 
//! Copy constructor.
GreatCircle_Xn::GreatCircle_Xn(const GreatCircle_Xn &other)
: GreatCircle(other), ch_max(other.ch_max), cmin(other.cmin)
{
}

//! \brief Equal operator.
//! 
//! Equal operator.
GreatCircle_Xn& GreatCircle_Xn::operator=(const GreatCircle_Xn& other)
{
	GreatCircle::operator = (other);
	ch_max = other.ch_max;
	return *this;
}

void GreatCircle_Xn::computeTravelTime()
{
	// set all components of the travel time to zero.
	tTotal=tSource=tReceiver=tHorizontal=tGamma = 0;

	if (getDistance() > MAX_DISTANCE)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in GreatCircle_Xn::computeTravelTime" << endl
			<< "Source-receiver separation exceeds maximum value." << endl
			<< "Source-receiver separation (degrees) : " << getDistance()*RAD_TO_DEG << endl
			<< "Maximum allowed separation (degrees) : " << MAX_DISTANCE*RAD_TO_DEG << endl
			<< "Source   location : " << source->getLocation().toString() << endl
			<< "Receiver location : " << receiver->getLocation().toString() << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),201);
	}

	if (source->getLocation().getDepth() > MAX_DEPTH)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in GreatCircle_Xn::computeTravelTime" << endl
			<< "Source depth exceeds maximum value." << endl
			<< "Source depth (km)          : " << source->getLocation().getDepth() << endl
			<< "Maximum allowed depth (km) : " << MAX_DEPTH << endl
			<< "Source   location : " << source->getLocation().toString() << endl
			<< "Receiver location : " << receiver->getLocation().toString() << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),202);
	}

	// receiver must be above the Moho
	if (!receiver->isInCrust())
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in GreatCircle_Xn::computeTravelTime" << endl
			<< "Receiver depth below Moho is illegal." << endl
			<< "Source   location : " << source->getLocation().toString() << endl
			<< "Receiver location : " << receiver->getLocation().toString() << endl
			<< "Receiver Moho depth : " << receiver->getDepth(MANTLE) << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),401);
	}

	if (source->isInCrust())
		computeTravelTimeCrust();
	else 
		computeTravelTimeMantle();
}

void GreatCircle_Xn::computeTravelTimeCrust()
{
	solutionMethod = "GreatCircle_Xn::computeTravelTimeCrust()";

	// udSign = 0 indicates the source is in the crust.
	udSign = 0;

	int i;

	// find the average moho radius at source and receiver.
	rMoho = (source->getInterfaceRadius(MANTLE) + receiver->getInterfaceRadius(MANTLE))/2;

	bool done = false;
	double dkm;

	// get the critical ray parameters at the source and receiver and
	// set the rayParameter to the smaller of the two.
	rayParameter = min(source->getPCrit(this), receiver->getPCrit(this));

	int nIterations = 0;

	// iterate a few times:  compute ray turning radius, compute ray parameter from
	// the turning radius, update pierce points.  Stop when ray parameter doesn't 
	// change much.
	while (!done)
	{
		if (++nIterations == 10000)
		{
			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
			  os << endl << "ERROR in GreatCircle_Xn::computeTravelTimeCrust" << endl
				<< "nIterations == " << nIterations << endl
				<< "Source   location : " << source->getLocation().toString() << endl
				<< "Receiver location : " << receiver->getLocation().toString() << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			setNAValues();
			throw SLBMException(os.str(),300);
		}

		// compute horizontal offset, in radians, and travel time below the source
		// and receiver.
		source->xtCrust(this, rayParameter, xSource, tSource);
		receiver->xtCrust(this, rayParameter, xReceiver, tReceiver);
	
		if (xSource + xReceiver > getDistance())
		{
			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
			  os << endl << "ERROR in GreatCircle_Xn::computeTravelTimeCrust" << endl
				<< "Horizontal offset below the source (" << xSource*RAD_TO_DEG << " deg) plus" << endl
				<< "horizontal offset below the receiver (" << xReceiver*RAD_TO_DEG << " deg)" << endl
				<< "is greater than the source-receiver separation (" << getDistance()*RAD_TO_DEG 
				<< " deg)" << endl
				<< "Source   location : " << source->getLocation().toString() << endl
				<< "Receiver location : " << receiver->getLocation().toString() << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			setNAValues();
			throw SLBMException(os.str(),203);
		}

		// find the first profile that is within the head wave portion of the ray path.
		// minimum of 0.  SourceIndex is the index of the horizontal interval on the
		// Moho such that the source pierce point lies within the interval.
		sourceIndex = max(0, (int)floor(xSource/actual_path_increment));

		// find the last profile that is within the head wave portion of the ray path.
		// but less than the very last profile.  receiverIndex is the index of the 
		// horizontal interval on the Moho such that the receiver pierce point lies 
		// within the interval.
		receiverIndex = min((int)profiles.size()-1, (int)floor((getDistance()-xReceiver)/actual_path_increment));

		// horizontal distance traveled along moho, in km.  This is horinzontal
		// distance between the source and receiver pierce points.
		xHorizontal = 0;

		// the path averaged mantle velocity, in km/sec.
		Vm = 0;

		// path averaged mantle gradient, in 1/sec.
		Gm = 0;

		// in following code, getActualPathIncrement(i) is a function that returns the horizontal
		// size of each horizontal interval on the Moho, in radians.  getProfile(i)
		// and profiles[i] are synonyms for the horizontal intervals along the Moho.
		// Each profile knows its radius, velocity, gradient, etc.
		for (i=sourceIndex; i <= receiverIndex; i++)
		{
			dkm = getActualPathIncrement(i) * getProfile(i)->getRadius();
			xHorizontal += dkm;
			Vm += profiles[i]->getVelocity() * dkm;
			Gm += profiles[i]->getGradient() * dkm;
		}

		Vm /= xHorizontal;
		Gm /= xHorizontal;

		// turning depth of the ray below the Moho.
		cm = max(cmin, Gm/Vm + 1.0/rMoho);

		H = (1/cm)*(sqrt(sqr(xHorizontal*cm/2) + 1) - 1);

		// radius of turning point
		turningRadius = rMoho - H;

		// ray parameter is radius / velocity at turning radius
		double p = turningRadius/(Vm + Gm * H);

		// if ray parameter has not changed much, then we are done.
		done = abs(p - rayParameter) < 1e-6;

		if (!done) rayParameter = p;
	}

	// Apply constraint on the range of validity of the Zhoa approximation.
	if (H*cm > ch_max)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(6);
		  os << endl << "ERROR in GreatCircle_Xn::computeTravelTimeCrust" << endl
			<< "c*H is greater than ch_max." << endl
			<< "Source   location : " << source->getLocation().toString() << endl
			<< "Receiver location : " << receiver->getLocation().toString() << endl
			<< "H      : " << H << endl
			<< "cm     : " << cm << endl
			<< "H*cm   : " << H*cm << endl
			<< "ch_max : " << ch_max << endl
			<< "Gm     : " << Gm << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),301);
	}

	// appropriate rayParameter is now set.  Iterate over all horizontal increments and 
	// calculate the travel time.
	for (i=sourceIndex; i <= receiverIndex; i++)
		tHorizontal +=  getActualPathIncrement(i) * profiles[i]->getRadius() / profiles[i]->getVelocity();

	// retrieve the average velocity of the mantle, as recorded in the
	// model input file.
	V0 = grid.getAverageMantleVelocity(phase%2);

	// Zhao c parameter.  Quantity which, when multiplied by V0, yields 
	// path averaged mantle gradient, corrected for sphericity of the Earth.
	c = max(cmin, Gm/V0 + 1.0/rMoho);

	// Zhao gamma parameter.  The correction to the head wave travel
	// time to account for gradient in the earth.
	tGamma = -xHorizontal*xHorizontal*xHorizontal*c*c/(V0 * 24);

	tTotal = tSource + tReceiver + tHorizontal + tGamma;

	receiverRayParameter = rayParameter;
	sourceRayParameter = rayParameter;
}

void GreatCircle_Xn::computeTravelTimeMantle()
{
	solutionMethod = "GreatCircle_Xn::computeTravelTimeMantle()";

	// since the source is in the mantle, the values for the
	// horizontal offset in the crust and the travel time in
	// the crust will both be zero and won't change in this
	// method.
	xSource = 0;
	tSource = 0;

	// sourceIndex is the index of the horizontal interval in
	// which the pierce point below the source hits the moho.
	// since the source is in the mantle, this is set to zero
	// and will not change in this method.
	sourceIndex = 0;

	// tReceiver is the travel time in  the crust below the 
	// receiver.  Initialize to zero.
	tReceiver = 0;

	// tHorizontal is the travel time for the head wave along
	// the Moho.
	tHorizontal = 0;

	// tGamma is the Zhao gradient correction term.
	tGamma    = 0;

	// dkm is a temporary variable used to store the horizontal
	// distance, in km, traveled by the ray in any given horizontal
	// increment along the moho.  It will be zero for intervals
	// between the source and source pierce point.  Zero for intervals
	// between the receiver and receiver pierce point.  It will equal
	// actual_path_increment for horizontal increments that lie wholly between the
	// source and receiver pierce points, and will be some value 
	// >= 0 and <= actual_path_increment for the horizontal increments that contain
	// the source or receiver pierce points.
	double dkm;

	// retrieve the average velocity of the mantle, as recorded in the
	// model input file.
	V0 = grid.getAverageMantleVelocity(phase%2);

	// lots of temporary variables defined at point of use.
	double xm, xz, p;

	// rMoho is the average of the moho radius below source and receiver
	rMoho = (source->getInterfaceRadius(MANTLE) + receiver->getInterfaceRadius(MANTLE))/2;

	// depth of event below the moho
	zm = source->getInterfaceRadius(MANTLE) - source->getLocation().getRadius();
	
	// rZm is the radius at the event depth
	rZm = source->getLocation().getRadius();

	// radius ratio of event
	zhao_r = source->getInterfaceRadius(MANTLE) / (source->getInterfaceRadius(MANTLE) - zm); 

	// get the critical spherical ray parameter at the receiver. 
	// Equals (moho radius below receiver / mantle velocity below receiver).
	rayParameter = receiver->getPCrit(this);

	double h1, h2, h3, f1, f2, f3, residual;

	// iterate a few times:  compute ray turning radius, compute ray parameter from
	// the turning radius, update pierce points.  Stop when ray parameter doesn't 
	// change much.
	int max_count = 0;
	do
	{
		if (++max_count > 200)
		{
			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
			  os << endl << "ERROR in GreatCircle_Xn::computeTravelTimeMantle" << endl
				<< "Could not converge on stable ray parameter." << endl
				<< "Source   location : " << source->getLocation().toString() << endl
				<< "Receiver location : " << receiver->getLocation().toString() << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			setNAValues();
			throw SLBMException(os.str(),302);
		}

		// compute horizontal offset, in radians, and travel time below the receiver.
		receiver->xtCrust(this, rayParameter, xReceiver, tReceiver);

		if (xReceiver > getDistance())
		{
			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
			  os << endl << "ERROR in GreatCircle_Xn::computeTravelTimeMantle" << endl
				<< "Source is too close to the receiver." << endl
				<< "Source-receiver separation = " << getDistance()*RAD_TO_DEG 
				<< " deg" << endl
				<< "Source   location : " << source->getLocation().toString() << endl
				<< "Receiver location : " << receiver->getLocation().toString() << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			setNAValues();
			throw SLBMException(os.str(),402);
		}


		// find the last profile that is within the head wave portion of the ray path.
		// but less than the very last profile.
		receiverIndex = min((int)profiles.size()-1, 
			(int)floor((getDistance()-xReceiver)/actual_path_increment));
		
		// compute the path average velocity and gradient.
		xHorizontal=Vm=Gm=0.;

		// constributions from all the whole horizontal increments.
		// Note that sourceIndex is fixed at zero since source is in mantle.
		for (int i=sourceIndex; i <= receiverIndex; i++)
		{
			// dkm is the horizontal distance traveled by the ray in
			// this horizontal increment of the head wave, times the
			// radius of the moho at the center of the horizontal increment.
			dkm = getActualPathIncrement(i) * getProfile(i)->getRadius();
			xHorizontal += dkm;
			Vm += profiles[i]->getVelocity() * dkm;
			Gm += profiles[i]->getGradient() * dkm;
		}

		// divide by total horizontal distance traveled by the ray along
		// the moho in order to get path averaged values.
		Gm /= xHorizontal;
		Vm /= xHorizontal;

		// compute the Zhao c parameter.
		// Note that max(cmin, x) is equivalent to 
		// if (x < cmin) x = cmin.  
		// cmin is defined in the GreatCircle_Xn constructor to be = 1e-8
		cm = max(cmin, Gm/Vm + 1.0/rMoho);
		c  = max(cmin, Gm/V0 + 1.0/rMoho);
		
		// rZm is the radius of the source.
		cmz = max(cmin, Gm/Vm + 1.0/rZm);
		cz  = max(cmin, Gm/V0 + 1.0/rZm);

		// We need to find the value of H, the depth of the turning point of the ray below
		// the Moho.  There are two situations:  if the ray leaves the source in a
		// downgoing direction then the turning point resides between the source and
		// receiver.  If the ray leaves the source in the upgoing direction, then the
		// "turning point" doesn't really exist but if we extrapolate a bit, it resides
		// somewhere further away from the receiver than the source.  We have to search for
		// it in both situations. 
		// 
		// To use Brent's method to find H, we first have to bracket the solution by finding
		// three estimates of H: h1, h2, and h3, where the values of the function to be
		// minimized are f1, f2 and f3, respectively, and where h1 < h2 and h2 < h3 and 
		// f1 > f2 and f2 < f3.  Function mnbrak performs this bracketing operation.  We
		// specify h1 = zm, the depth of the source below the Moho and hold that fixed. 
		// Then we specify a very large value for h2, such that we are certain that the
		// value of H at the minimum will be located between h1 and h2.  For the case of a
		// downgoing ray, H is highly unlikely to exceed 3000 km.  For the case of an
		// upgoing ray, H could be quite large, but certainly less than 1e4.
		// 
		// The ray will be either downgoing or upgoing, so if we check the downgoing case
		// and find a solution, we do not need to check the upgoing case.  If neither case
		// finds a solution, then it may be that the event resides at exactly the turning
		// point.  This is highly unlikely.

		// udSign is positive one for ray that leaves the source
		// in downgoing direction, negative one for ray that leaves
		// the soure in an upgoing direction.
		udSign = +1; // downgoing ray.
		h1 = zm;
		h2 = 3000.;

		// given two estimates of H, h1 an h2, find values of h3, f1, f2 and f3 
		// such that H is between h1 and h3, and f2 < f1 and f2 < f3. 
		mnbrak(h1, h2, h3, f1, f2, f3); 

		// the downgoing ray is invalid if h3 = h1.  Basically, mnbrak
		// could not find a minimum that is deeper than zm.  In other words,
		// func(z) is monitonically increasing for all depths greater
		// than zm.  This happens for deep sources close to the receiver
		// where only upgoing ray has minimum deeper than zm.
		if (h3 != h1)
			// call brent to find H such that func is minimized.
			residual = brent(h1, h2, h3, 1e-8, H);
		else
		{
			// try the upgoing ray
			udSign = -1;
			h1 = zm;
			h2 = 1e4;

			// bracket the solution
			mnbrak(h1, h2, h3, f1, f2, f3); 
			if (h3 != h1)
				// call brent to find H such that func is minimized.
				residual = brent(h1, h2, h3, 1e-8, H);
			else
			{
				// Could not find solution for either upgoing or downgoing case.
				// Check to see if ray leaves the source in exactly horizontal direction.
				H = zm;
				residual = fabs(func(H));
			}
		}

		if (residual >= 1e-6)
		{
			// We failed to find a valid turning depth for the ray.  This has been found to
			// happen in parts of the model where the velocity gradient is substantially 
			// negative.
			
			// debug info:  print out func(H) for upgoing and downgoing rays as function of H.
			//cout << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(12);
			//h3 = 300.;
			//int nh = 100;
			//double dh = (h3-zm)/nh;
			//for (int i=0; i<=nh; ++i)
			//{
			//	h2 = zm + dh*i;
			//	udSign = 1;
			//	cout << h2 << "  " << func(h2);
			//	udSign = -1;
			//	cout << " " << func(h2) << endl;
			//}

			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
			  os << endl << "ERROR in GreatCircle_Xn::computeTravelTimeMantle" << endl
				<< "search for minimum H failed." << endl
				<< "Source   location : " << source->getLocation().toString() << endl
				<< "Receiver location : " << receiver->getLocation().toString() << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			setNAValues();
			throw SLBMException(os.str(),303);
		}

		// turning radius
		turningRadius = rMoho-H;

		// ray parameter is turning radius / velocity at turning radius
		p = rayParameter;
		rayParameter = turningRadius/(Vm + Gm * H);
	}
	while (fabs(1.-p/rayParameter) > 1e-6);

	// Apply constraint on the range of validity of the Zhoa approximation.
	if (cm*H > ch_max)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(6)
			<< endl << "ERROR in GreatCircle_Xn::computeTravelTimeMantle" << endl
			<< "c*H > ch_max." << endl
			<< "Source   location : " << source->getLocation().toString() << endl
			<< "Receiver location : " << receiver->getLocation().toString() << endl
			<< "H      : " << H << endl
			<< "cm     : " << cm << endl
			<< "H*cm   : " << H*cm << endl
			<< "ch_max : " << ch_max << endl
			<< "Gm     : " << Gm << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		setNAValues();
		throw SLBMException(os.str(),304);
	}

	xm = (2/cm)*sqrt(sqr(1 + cm*H) - 1);
	xz = (2/cmz)*sqrt(sqr(1 + cmz*(H-zm)) - 1);

	tHorizontal = (xm - xHorizontal)/Vm + udSign * xz/(Vm + Gm*zm);
	for (int i=sourceIndex; i<=receiverIndex; i++)
		tHorizontal +=  getActualPathIncrement(i)*profiles[i]->getRadius() 
			/ profiles[i]->getVelocity();

	tHorizontal /= 2.;

	// Gradient part of time
	tGamma = -(c*c*xm*xm*xm/V0 + udSign*cz*cz*xz*xz*xz/(V0 + Gm*zm))/48.;
	
	tTotal = tReceiver + tHorizontal + tGamma;
}

double GreatCircle_Xn::func(const double& h)
{
	return sqr( sqrt(sqr(1+cm*h)-1) + 
		udSign*zhao_r*(cm/cmz)*sqrt(sqr(1+cmz*(h-zm))-1) - xHorizontal*cm );
}

// Bracket a minimum of function func.
// Given two initial x values: ax, and bx, search for values 
// of ax, bx and cx such that func(bx) 
void GreatCircle_Xn::mnbrak(double &ax, double &bx, double &cx, 
							double &fa, double &fb, double &fc)
{
	fa = func(ax);
	fb = func(bx);

	cx = bx;
	fc = fb;
	bx = ax+0.5*(cx-ax);
	fb = func(bx);

	while (fb > fa || fb > fc)
	{
		if (cx-ax < 1e-8)
		{
			cx = bx = ax;
			fc = fb = fa;
			return;
		}

		cx = bx;
		fc = fb;
		bx = ax+0.5*(cx-ax);
		fb = func(bx);
	}
}

double GreatCircle_Xn::brent(const double ax, const double bx, const double cx, 
	const double tol, double &xmin)
{
	const int ITMAX=100;
	const double CGOLD=0.3819660;
	const double ZEPS=numeric_limits<double>::epsilon()*1.0e-3;
	int iter;
	double a,b,d=0.0,etemp,fu,fv,fw,fx;
	double p,q,r,tol1,tol2,u,v,w,x,xm;
	double e=0.0;

	a=(ax < cx ? ax : cx);
	b=(ax > cx ? ax : cx);
	x=w=v=bx;
	fw=fv=fx=func(x);
	for (iter=0;iter<ITMAX;iter++) {
		xm=0.5*(a+b);
		tol2=2.0*(tol1=tol*fabs(x)+ZEPS);
		if (fabs(x-xm) <= (tol2-0.5*(b-a))) {
			xmin=x;
			return fx;
		}
		if (fabs(e) > tol1) {
			r=(x-w)*(fx-fv);
			q=(x-v)*(fx-fw);
			p=(x-v)*q-(x-w)*r;
			q=2.0*(q-r);
			if (q > 0.0) p = -p;
			q=fabs(q);
			etemp=e;
			e=d;
			if (fabs(p) >= fabs(0.5*q*etemp) || p <= q*(a-x) || p >= q*(b-x))
				d=CGOLD*(e=(x >= xm ? a-x : b-x));
			else {
				d=p/q;
				u=x+d;
				if (u-a < tol2 || b-u < tol2)
					d=SIGN(tol1,xm-x);
			}
		} else {
			d=CGOLD*(e=(x >= xm ? a-x : b-x));
		}
		u=(fabs(d) >= tol1 ? x+d : x+SIGN(tol1,d));
		fu=func(u);
		if (fu <= fx) {
			if (u >= x) a=x; else b=x;
			shft3(v,w,x,u);
			shft3(fv,fw,fx,fu);
		} else {
			if (u < x) a=u; else b=u;
			if (fu <= fw || w == x) {
				v=w;
				w=u;
				fv=fw;
				fw=fu;
			} else if (fu <= fv || v == x || v == w) {
				v=u;
				fv=fu;
			}
		}
	}
	ostringstream os;
	os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) 
		<< setprecision(2);
	os << endl << "ERROR in GreatCircle_Xn::brent" << endl
		<< "Too many iterations." << endl
		<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
	setNAValues();
	throw SLBMException(os.str(),305);
}

string GreatCircle_Xn::toString(const int& verbosity)
{
	ostringstream os;
	if (verbosity > 0)
	{
		int n = getHeadWaveInterface();

		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) 
			<< setprecision(4);

		os << "   SLBM Version " << SlbmVersion << endl << endl;

		os << "   Solution method = " << solutionMethod << endl << endl;

		if (Location::EARTH_RADIUS > 0.)
			os << "   Earth radius = constant " << Location::EARTH_RADIUS 
				<< " km." << endl << endl;
		else
			os << "   Earth radius varies as a function of latitude. "
				<< endl << endl;

		os << "   Maximum allowed source-receiver separation = "
			<< MAX_DISTANCE*RAD_TO_DEG << " degrees."
			<< endl;

		os << "   Maximum allowed source depth = "
			<< MAX_DEPTH << " km."
			<< endl << endl;

		os << "   Phase = " << getPhaseString() << endl;

		int w = 12;

		os << "              " 
			<< setw(w) << "X, deg"
			<< setw(w) << "X,moho, km"
			<< setw(w) << "T, sec"
			<< endl;

		os << "   Source   = " 
			<< setw(w) << xSource * RAD_TO_DEG
			<< setw(w) << xSource * source->getInterfaceRadius(n)
			<< setw(w) << tSource 
			<< endl;
		os << "   Receiver = " 
			<< setw(w) << xReceiver * RAD_TO_DEG
			<< setw(w) << xReceiver * receiver->getInterfaceRadius(n)
			<< setw(w) << tReceiver 
			<< endl;
		if (inCrust)
		{
			os << "   Moho     = " 
				<< setw(w) << (getDistance() - xSource - xReceiver) * RAD_TO_DEG 
				<< setw(w) << xHorizontal 
				<< setw(w) << tHorizontal 
				<< endl;
			os << "   Gamma    = " 
				<< setw(w) << " "
				<< setw(w) << " "
				<< setw(w) << tGamma 
				<< endl;
		}
		else
		{
			os << "   Mantle   = " 
				<< setw(w) << (getDistance() - xSource - xReceiver) * RAD_TO_DEG 
				<< setw(w) << xHorizontal 
				<< setw(w) << tHorizontal 
				<< endl;
		}
		os << "   Total    = " 
			<< setw(w) << getDistance() * RAD_TO_DEG
			<< setw(w) << xSource* source->getInterfaceRadius(n)
					+xReceiver* receiver->getInterfaceRadius(n)
					+xHorizontal
			<< setw(w) << tTotal 
			<< endl << endl;

		if (verbosity > 1) 
		{
			os << setprecision(4);
			os << "   Average mantle velocity  = " << setw(w) 
				<< grid.getAverageMantleVelocity(phase%2) << endl;
			os << "   Path average velocity    = " << setw(w) << Vm << endl;
			os << setprecision(8);
			os << "   Path average gradient    = " << setw(w) << Gm << endl;
			os << "   C                        = " << setw(w) << c << endl;
			os << setprecision(4);
			os << "   H                        = " << setw(w) << H << endl;
			os << "   C*H                      = " << setw(w) << c*H << endl;
			os << "   chMax                    = " << setw(w) << ch_max << endl;
			os << "   Average Moho depth       = " << setw(w) << 
				(source->getDepth(MANTLE) + receiver->getDepth(MANTLE))/2<< endl;
			os << "   Turning depth            = " << setw(w) << 
				//(source->getDepth(MANTLE) + receiver->getDepth(MANTLE))/2 + H
				turningRadius
				<< endl;
			os << "   Ray parameter            = " << setw(w) << rayParameter << endl;
			os << "   Fraction Active          = " << setw(w) << getFractionActive() << endl;
			os << endl << endl;

			if (verbosity >= 3)
			{
				os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);

				os << "   Source Profile:" << endl << endl
					<< source->toString(this, rayParameter) << endl;

				os << endl << "   Receiver Profile:" << endl << endl
					<< receiver->toString(this, rayParameter) << endl;

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
							<< setw(w)   << "Lat" << setw(w+1) << "Lon" << setw(w) << "Moho"
							<< setw(w+1)   << "R Earth" << setw(w)   << "Vel"
							<< setw(w+2) << "Grad" 
							<< setw(w+1) << "dx" 
							<< setw(w+1) << "Active" 
							<< endl;

						for (int i=0; i<(int)profiles.size(); i++)
						{
							getProfile(i); // ensure profiles[i] instantiated
							// find location
							source->getLocation().move(vtp, ((double)i+0.5)*actual_path_increment, location);

							os << setw(6) << i;

							os << std::setprecision(3)
								<< setw(w) << location.getLatDegrees()
								<< setw(w+1) << location.getLonDegrees()
								<< setprecision(3)
								<< setw(w) << location.getEarthRadius()-profiles[i]->getRadius()
								<< setprecision(2)
								<< setw(w+1) << location.getEarthRadius()
								<< setprecision(4)
								<< setw(w) << profiles[i]->getVelocity()
								<< setprecision(6)
								<< setw(w+2) << profiles[i]->getGradient()
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
	}
	return os.str();
}

} // end slbm namespace
