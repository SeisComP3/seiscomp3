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
//- Program:       GreatCircleFactory
//- Module:        $RCSfile: GreatCircleFactory.cc,v $
//- Revision:      $Revision: 1.15 $
//- Last Modified: $Date: 2011/11/30 22:41:27 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "GreatCircleFactory.h"
#include "Grid.h"
#include "SLBMException.h"

//using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

GreatCircleFactory::GreatCircleFactory() 
{
}

GreatCircleFactory::~GreatCircleFactory()
{
}

GreatCircle* GreatCircleFactory::create(
		const int& phase,
		Grid* grid, 
		const double& latSource, 
		const double& lonSource,
		const double& depthSource,
		const double& latReceiver,
		const double& lonReceiver,
		const double& depthReceiver,
		const double& chMax)
{
	if (phase < Pn || phase > Lg)
	{
		ostringstream os;
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(4);
		os << endl << "ERROR in GreatCircleFactory::create" << endl
			<< phase << " is not a recognized phase.  Must be one of Pn, Sn, Pg, Lg." << endl
			<< "source   lat, lon, depth = " 
			<< setw(12) << latSource*RAD_TO_DEG << ", " 
			<< setw(12) << lonSource*RAD_TO_DEG << ", " 
			<< setw(12) << depthSource << endl
			<< "receiver lat, lon, depth = " 
			<< setw(12) << latReceiver*RAD_TO_DEG << ", " 
			<< setw(12) << lonReceiver*RAD_TO_DEG << ", " 
			<< setw(12) << depthReceiver << endl
			<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),100);
	}

	GreatCircle* greatcircle = NULL;

	try
	{
		if (phase == Pn || phase == Sn) 
			greatcircle = new GreatCircle_Xn(
				phase,
				*grid, 
				latSource, 
				lonSource,
				depthSource,
				latReceiver,
				lonReceiver,
				depthReceiver,
				chMax);
		else if (phase == Pg || phase == Lg) 
			greatcircle = new GreatCircle_Xg(
				phase,
				*grid, 
				latSource, 
				lonSource,
				depthSource,
				latReceiver,
				lonReceiver,
				depthReceiver);
		return greatcircle;
	}
	catch( SLBMException ex )
	{
		if (greatcircle)
			delete greatcircle;
		greatcircle = NULL;

		throw SLBMException(ex.emessage, ex.ecode);
	}

}

} // end slbm namespace
