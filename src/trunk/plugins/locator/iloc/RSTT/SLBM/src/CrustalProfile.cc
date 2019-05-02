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
//- Program:       CrustalProfile
//- Module:        $RCSfile: CrustalProfile.cc,v $
//- Revision:      $Revision: 1.32 $
//- Last Modified: $Date: 2012/12/03 23:59:06 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "Grid.h"
#include "CrustalProfile.h"
#include "GridProfile.h"

#include <iostream>

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _STATIC INITIALIZATIONS_************************************************

int CrustalProfile::crustalProfileClassCount = 0;

CrustalProfile::CrustalProfile() : InterpolatedProfile(), fudgeFactor(1.0), cpTPS(NULL)
{
	++crustalProfileClassCount;
}

CrustalProfile::~CrustalProfile()
{
	--crustalProfileClassCount;
  if (cpTPS) delete cpTPS;
}

CrustalProfile::CrustalProfile(const CrustalProfile &other) 
: InterpolatedProfile(other), fudgeFactor(other.fudgeFactor), cpTPS(0)
{
	++crustalProfileClassCount;
  if (other.cpTPS) cpTPS = new taup::TauPSite(*other.cpTPS);

	for (int k=0; k<NLAYERS; k++)
	{
		radius[k] = other.radius[k];
		velocity[k] = other.velocity[k];
	}

	inCrust = other.inCrust;
}

CrustalProfile& CrustalProfile::operator=(const CrustalProfile& other)
{
  if (cpTPS) delete cpTPS;
  cpTPS = (taup::TauPSite*) 0;
  if (other.cpTPS) cpTPS = new taup::TauPSite(*other.cpTPS);

	InterpolatedProfile::operator = (other);
	for (int k=0; k<NLAYERS; k++)
	{
		radius[k] = other.radius[k];
		velocity[k] = other.velocity[k];
	}
	fudgeFactor = other.fudgeFactor;
    return *this;
}

bool CrustalProfile::operator==(const CrustalProfile& other)
{
	return 
		InterpolatedProfile::operator == (other)
		&& radius == other.radius
		&& velocity == other.velocity;
}

void CrustalProfile::setup(Grid& grid, const int& _phase, 
		const double& lat, const double& lon, const double& depth) 
{
	location.setLocation(lat, lon, depth);
	double earthRadius = getEarthRadius();

	phase = _phase;

	if (cpTPS) 
	{
		delete cpTPS;
		cpTPS = NULL;
	}

	grid.findProfile(location, nodes, nodeIds, coefficients);

	// get interpolated radius values for all intervals.
	// Note:  it is assumed that MANTLE == NLAYERS-1
	for (int k=0; k<NLAYERS; k++)
	{
		// sb 10/07/2009: changed this line with 
		// the two that follow and made derivatives
		// and second derivatives smooth.
		//interpRadius(k, radius[k]);
		interpDepth(k, radius[k]);
		radius[k] = earthRadius-radius[k];
		interpVelocity(phase%2, k, velocity[k]);
	}

	// number of layers with non-zero thickness.
	// This list may or may not include the WATER
	// layer, depending on how TOP_LAYER is defined.
	// The mantle is guaranteed to be included.
	nlay = 0;

	// save the indeces of the layers that have
	// non-zero thickness.
	for (int k=TOP_LAYER; k<NLAYERS-1; k++)
	{
		if (k==MIDDLE_CRUST_N)
		{
			// if this layer is the middle crust/n, only add layer to 
			// list of non-zero thickness layers if phase is Pn,Sn
			// and layer is finite thickness
			if ((phase == Pn || phase == Sn) && 
				radius[MIDDLE_CRUST_N]-radius[LOWER_CRUST] > 1e-6)
							layid[nlay++] = k;
		}
		else if (k==MIDDLE_CRUST_G)
		{
			// if this layer is the middle crust/g, only add layer to 
			// list of non-zero thickness layers if phase is Pg/Lg
			// and layer is finite thickness
			if ((phase == Pg || phase == Lg) && 
				radius[MIDDLE_CRUST_G]-radius[LOWER_CRUST] > 1e-6)
							layid[nlay++] = k;
		}
		else if (radius[k]-radius[k+1] > 1e-6)
							layid[nlay++] = k;
	}

	// always include the mantle
	layid[nlay++] = MANTLE;

	// the index of the entry in layid whose value is MIDDLE_CRUST.
	nMiddleCrust = -1; 

	// index of entry in layid that contains the location of this profile.
	topLayid = -1;

	inCrust = location.getRadius() > radius[MANTLE];

	// if the location is in the mantle, then layid should remain empty.
	// if it is in the crust then populate it.
	if (inCrust)
	{
		// location is in the crust.
		// iterate over all the non-zero thickness layers
		for (int k=0; k<nlay; k++)
			// skip ones whose top is shallower than location radius.
			if (radius[layid[k]] < location.getRadius())
			{
				// if we have not yet identified the layer that 
				// the location radius is in, do so now.
				if (topLayid == -1)
					topLayid = max(0, k-1);

				// keep track of the index in the layid array 
				// that corresponds to the middle crust, or,
				// if the middle crust is zero thickness, then
				// the next non-zero thickness layer below the
				// middle crust.
				if (nMiddleCrust < 0 && layid[k] >= MIDDLE_CRUST_G)
					nMiddleCrust = k;
			}
	}

	if (phase == Pg || phase == Lg)
	{
		// change the velocity of the lower crust to be equal to the velocity
		// of the middle crust.
		velocity[LOWER_CRUST] = velocity[MIDDLE_CRUST_G];

		// if the velocity of the upper crust is greater than the velocity of the 
		// middle crust minus .1 km/sec, then reduce all velocities above the 
		// middle crust by a fudge factor such that the velocity of the upper crust
		// is equal to the velocity of the middle crust minus 0.1 km/sec.
		fudgeFactor = min(1., (velocity[MIDDLE_CRUST_G] - 0.1)
		  /velocity[UPPER_CRUST]);
		if (fudgeFactor < 1.0)
			for (int i=0; i <= UPPER_CRUST; ++i)
				velocity[i] *= fudgeFactor;
	}
	else
		// fudgeFactor is 1.0 for Pn/Sn
		fudgeFactor = 1.0;


	// check for zero velocities along the ray path
	for (int i=0; i<nlay; i++)
		if (velocity[layid[i]] < 1e-6)
		{
			ostringstream os;
			os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
			  os << endl << "ERROR in CrustalProfile constructor" << endl
				<< "Layer " << layid[i]
				<< " has velocity of 0 km/sec." << endl
				<< "Phase = " << (phase==Pn ? "Pn" : phase==Sn ? "Sn" : phase==Pg ? "Pg" : phase==Lg ? "Lg" : "?")
			    << endl << "Location = " << location.toString() << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),500);
		}

}

void CrustalProfile::xtCrust(
    GreatCircle* greatCircle,
	const double& p,
	double& xTotal,
	double& tTotal)
{
	int j, n;
	if (greatCircle->getHeadWaveInterface() == MANTLE)
		n = nlay-1;
	else if (greatCircle->getHeadWaveInterface() == MIDDLE_CRUST_G)
		n = nMiddleCrust;
	else
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in CrustalProfile::xtCrust()" << endl
			<< "greatCircle->getHeadWaveInterface() returned " << greatCircle->getHeadWaveInterface()
			<< endl << endl
			<< toString()
			<< endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),502);
	}

	// set rayParameter = p, except:
	// if p < 0 or p > critical ray parameter, 
	// set rayParameter = critical ray parameter.
	double rayParameter = (p < 0. ? getPCrit(greatCircle) 
		: min(p, getPCrit(greatCircle)));

	double r, pv;
	bool done = false;

	while (!done)
	{
		done = true;

		// initialize output variables
		xTotal = 0.;  tTotal = 0.;

		for (j=topLayid; j<n; j++)
		{
			// the top of the layer.  
			r = (j==topLayid ? location.getRadius() : radius[layid[j]]);

			// set pv = ray parameter * velocity.  pv also = rBottom * sin(i)
			pv = rayParameter*velocity[layid[j]];

			//// if sin(i) > 1.0, then ray does not reach the moho.
			//// Reduce the rayParameter and start all over.
			//if (pv/radius[layid[j+1]] > 1.0)
			//{
			//	rayParameter = radius[layid[j+1]]/velocity[layid[j]] * (1 - 1e-14);
			//	done = false;
			//	break;
			//}

			// if sin(i) > 1.0, then ray does not reach the moho.
			// this is an error, throw exception.
			if (pv/radius[layid[j+1]] > 1.0)
			{
				ostringstream os;
				os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
				  os << endl << "ERROR in CrustalProfile::xtCrust()" << endl
					<< "A crustal layer has velocity greater than the velocity of the top of the mantle." 
					<< endl << endl
					<< toString()
					<< endl
					<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
				throw SLBMException(os.str(),501);
			}

			// horizontal component of the distance travelled by the ray 
			// in this layer in radians.
			xTotal += acos(pv/r) - acos(pv/radius[layid[j+1]]);

			// travel time of the ray in this layer, in seconds.
			tTotal += (sqrt(r*r-pv*pv) 
				- sqrt(radius[layid[j+1]]*radius[layid[j+1]]-pv*pv))
				/velocity[layid[j]];

		}
	}
}

void CrustalProfile::xtCrust(
	GreatCircle* greatCircle,
	const double& p,
	int lid[],
	double x[],
	double r[],
	double v[],
	double t[],
	int& npoints)
{
	int j, n;

	if (greatCircle->getHeadWaveInterface() == MANTLE)
		n = nlay-1;
	else if (greatCircle->getHeadWaveInterface() == MIDDLE_CRUST_G)
		n = nMiddleCrust;
	else
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		  os << endl << "ERROR in CrustalProfile::xtCrust()" << endl
			<< "greatCircle->getHeadWaveInterface() returned " << greatCircle->getHeadWaveInterface()
			<< endl << endl
			<< toString()
			<< endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),502);
	}


	npoints = n+1;

	// set rayParameter = p, except:
	// if p < 0 or p > critical ray parameter, 
	// set rayParameter = critical ray parameter.
	double rayParameter = (p < 0. ? getPCrit(greatCircle) 
		: min(p, getPCrit(greatCircle)));

	double dx=0., dt=0., pv;
	bool done = false;

	while (!done)
	{
		done = true;

		for (j=0; j<npoints; j++)
		{
			// the top of the layer.  
			x[j] = (j==0 ? 0. : x[j-1]+dx);
			r[j] = (j==0 ? location.getRadius() : radius[layid[j]]);
			v[j] = velocity[layid[j]];
			t[j] = (j==0 ? 0. : t[j-1]+dt);
			lid[j] = layid[j];

			// set pv = ray parameter * velocity.  pv also = rBottom * sin(i)
			pv = rayParameter*velocity[layid[j]];

			//// if sin(i) > 1.0, then ray does not reach the moho.
			//// Reduce the rayParameter and start all over.
			//if (pv/r[j] > 1.0)
			//{
			//	rayParameter = r[j]/velocity[layid[j]] * (1 - 1e-14);
			//	done = false;
			//	break;
			//}

			// if sin(i) > 1.0, then ray does not reach the moho.
			// this is an error, throw exception.
			if (pv/r[j] > 1.0)
			{
				ostringstream os;
				os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
				  os << endl << "ERROR in CrustalProfile::xtCrust()" << endl
					<< "A crustal layer has velocity greater than the velocity of the top of the mantle." 
					<< endl << endl
					<< toString()
					<< endl
					<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
				throw SLBMException(os.str(),502);
			}

			if (j < nlay-1)
			{
				// horizontal component of the distance travelled by the ray 
				// in this layer in radians.
				dx = acos(pv/r[j]) - acos(pv/radius[layid[j+1]]);

				// travel time of the ray in this layer, in seconds.
				dt = (sqrt(r[j]*r[j]-pv*pv) 
					- sqrt(radius[layid[j+1]]*radius[layid[j+1]]-pv*pv))
					/velocity[layid[j]];
			}
		}
	}
}

string CrustalProfile::toString(GreatCircle* greatCircle, double rayParameter)
{
	// number of crustal layers (mantle is not included)
	int n = greatCircle->getHeadWaveInterface(); 

	double earthRadius = getEarthRadius();

	// total travel time and horizontal offset down to head wave interface
	double xTotal, tTotal;

	// create a Location to hold the pierce point
	Location piercePoint;

	if (inCrust)
	{

		// compute x,z and t.
		//xtCrust(greatCircle, rayParameter, xTotal, tTotal);
		xtCrust(greatCircle, rayParameter, xTotal, tTotal);

		// create an array to hole the vector triple product of 
		// (source cross receiver) cross source   OR
		// (receiver cross source) cross receiver
		double vtp[3];

		if (location == greatCircle->getSourceProfile()->getLocation())
			// this CrustalProfile is the source crustal profile.  compute
			// (source cross receiver) cross source 
			greatCircle->getSourceProfile()->getLocation().vectorTripleProduct(
				greatCircle->getReceiverProfile()->getLocation(), vtp);
		else
			// this CrustalProfile is the receiver crustal profile, compute
			// (receiver cross source) cross receiver
			greatCircle->getReceiverProfile()->getLocation().vectorTripleProduct(
				greatCircle->getSourceProfile()->getLocation(), vtp);

		// populate pierce point with the pierce point appropriate for either
		// the source or the receiver as determined above.
		location.move(vtp, xTotal, piercePoint);
	}

	ostringstream os;

	os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);

	int w = 9;

	os  << "             " << setw(w) << "Lat" << setw(w) << "Lon" << setw(w) << "Depth"
			<< setw(w) << "R Earth" << setw(w) << "P" << setw(w) << "P crit"
			<< endl;

	os << "   Location: " 
		<< setprecision(4)
		<< setw(w) << location.getLatDegrees()
		<< setw(w) << location.getLonDegrees()
		<< setprecision(3)
		<< setw(w) << location.getDepth()
		<< setprecision(2)
		<< setw(w) << earthRadius
		<< setw(w) << rayParameter
		<< setw(w) << getPCrit(greatCircle)
		<< setprecision(4)
		<< endl;

	os << "   Pierce Pt:" ;
	if (inCrust)
		os << setw(w) << piercePoint.getLatDegrees()
			<< setw(w) << piercePoint.getLonDegrees()
			<< setprecision(3)
			<< setw(w) << earthRadius-radius[n];
	else
		os << "  Ray did not pierce the head wave interface.";
	os << endl << endl;

	w=9;
	os << setprecision(4);

	string layers[9] = {"water", "sediment1", "sediment2", "sediment3",
			"upper_crust", "middle_crust", "middle_crust", "lower_crust", "mantle"};

	os << "   Model Layers:" << endl << endl;

	os << " " << setw(5) << "#" 
		<< left 
		<< " " << setw(13) << "Layer" 
		<< right
		<< setw(w-1) << "Vel" 
		<< setw(w) << "Top" 
		<< setw(w) << "Thick" 
		<< endl;

	--w;
	for (int i=0; i<getNIntervals(); i++)
	{
		if (i==MIDDLE_CRUST_G && (phase == Pn || phase == Sn)) continue;
		if (i==MIDDLE_CRUST_N && (phase == Pg || phase == Lg)) continue;

		// layer number
		os << " " << setw(5) << i+1;

		// layer name
		os << " " << setw(13) << left << layers[i] << right;

		// velocity
		os << " " << setprecision(4) << setw(w-1) << velocity[i];

		// depth
		os << " " << setprecision(4) << setw(w) << getDepth(i);

		// thickness
		if (i < getNIntervals()-1)
		{
			double thick;
			if (i==MIDDLE_CRUST_N || i==MIDDLE_CRUST_G)
				thick = getInterfaceRadius(i)-getInterfaceRadius(LOWER_CRUST);
			else
				thick = getInterfaceRadius(i)-getInterfaceRadius(i+1);
			os << " " << setprecision(4) << setw(w) << thick;
		}

		os << endl;
	}

	os << endl << endl
		<< "   Applied Layers:" << endl << endl;

	if (nlay == 0)
		os << "     Ray did not traverse the crust." << endl << endl;
	else
	{
		os << " " << setw(5) << "#" 
		<< " " << setw(w-1) << "Vel" 
		<< " " << setw(w) << "Top" 
		<< " " << setw(w) << "Thick" 
		<< " " << setw(w) << "i, deg"
		<< " " << setw(w) << "X, deg" 
		<< " " << setw(w) << "X, km" 
		<< " " << setw(w) << "TT" 
		<< endl;


		double x[NLAYERS], t[NLAYERS], v[NLAYERS], r[NLAYERS];
		int lid[NLAYERS], npoints;

		xtCrust(greatCircle, rayParameter, lid, x, r, v, t, npoints);

		for (int i=0; i<npoints; i++)
		{
			// layer number
			os << " " << setw(5) << lid[i]+1;

			// layer name
			//os << " " << setw(13) << left << layers[layerid[i]] << right;

			// velocity
			os << " " << setprecision(4) << setw(w-1) << v[i];

			// depth
			os << " " << setprecision(4) << setw(w) << earthRadius-r[i];

			// thickness
			if (i == npoints-1)
				os << " " << setw(w) << " ";
			else
				os << " " << setprecision(4) << setw(w) << r[i]-r[i+1];

			//incidence angle at top of layer.
			os << " " << setprecision(4) << setw(w) 
				<< asin(min(1., rayParameter*v[i]/r[i])) * RAD_TO_DEG;

			// x, deg
			os << " " << setprecision(5) << setw(w) << x[i] * RAD_TO_DEG;

			// x, km
			os << " " << setprecision(4) << setw(w) << x[i] * r[i];

			// travel time
			os << " " << setprecision(4) << setw(w) << t[i];

			os << endl;
		}
	}

	return os.str();
}

string CrustalProfile::toString()
{
	double earthRadius = getEarthRadius();

	ostringstream os;

	os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);

	int w = 9;

	os  << "             " << setw(w) << "Lat" << setw(w) << "Lon" << setw(w) << "Depth"
			<< setw(w) << "R Earth" 
			<< endl;

	os << "   Location: " 
		<< setprecision(4)
		<< setw(w) << location.getLatDegrees()
		<< setw(w) << location.getLonDegrees()
		<< setprecision(3)
		<< setw(w) << location.getDepth()
		<< setprecision(2)
		<< setw(w) << earthRadius
		//<< setw(w) << rayParameter
		<< setprecision(4)
		<< endl << endl;

	os << setprecision(6);

	os << "   Fudge factor: " << fudgeFactor << endl << endl;

	string layers[9] = {"water", "sediment1", "sediment2", "sediment3",
			"upper_crust", "middle_crust_n", "middle_crust_g", "lower_crust", "mantle"};

	os << "   Model Layers:" << endl << endl;

	w=9;
	os << setprecision(4);

	os << " " << setw(5) << "#" 
		<< left 
		<< " " << setw(15) << "Layer" 
		<< right
		<< setw(w-1) << "Vel" 
		<< setw(w) << "Top" 
		<< setw(w) << "Thick" 
		<< endl;

	--w;
	for (int i=0; i<getNIntervals(); i++)
	{
		int j = i+1;
		if (i==MIDDLE_CRUST_N || i==MIDDLE_CRUST_G) j = LOWER_CRUST;

		if (i < getNIntervals()-1 && getInterfaceRadius(i)-getInterfaceRadius(j) < 1e-6) 
			continue;

		//if (i==MIDDLE_CRUST_G) continue;
		if (i==MIDDLE_CRUST_G && (phase == Pn || phase == Sn)) continue;
		//if (i==MIDDLE_CRUST_N && (phase == Pg || phase == Lg)) continue;

		// layer number
		os << " " << setw(5) << i+1;

		// layer name
		os << " " << setw(15) << left << layers[i] << right;

		// velocity
		os << " " << setprecision(4) << setw(w-1) << velocity[i];

		// depth
		os << " " << setprecision(4) << setw(w) << getDepth(i);

		// thickness
		if (i < getNIntervals()-1)
		{
			//double thick;
			//if (i==MIDDLE_CRUST_N || i==MIDDLE_CRUST_G)
			//	thick = getInterfaceRadius(i)-getInterfaceRadius(LOWER_CRUST);
			//else
			//	thick = getInterfaceRadius(i)-getInterfaceRadius(i+1);
			os << " " << setprecision(4) << setw(w) << getInterfaceRadius(i)-getInterfaceRadius(j);
		}

		os << endl;
	}

	return os.str();
}

size_t CrustalProfile::memSize()
{
	return InterpolatedProfile::memSize()
		+ sizeof(location)
		+ sizeof(location)
		+ sizeof(radius) + getNIntervals()*sizeof(double)
		+ sizeof(velocity) + getNIntervals()*sizeof(double)
		;

}

void CrustalProfile::setTauPSite(taup::TauPSite* tps)
{
  if (cpTPS) delete cpTPS;
	cpTPS = tps;
}

taup::TauPSite* CrustalProfile::getTauPSite()
{
	return cpTPS;
}

int CrustalProfile::getClassCount()
{
  return crustalProfileClassCount;
}

} // end slbm namespace

