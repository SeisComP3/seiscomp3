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
//- Program:       SNL TauP Location Library (TauPLoc)
//- Module:        $RCSfile: TauPSite.cc,v $
//- Creator:       Jim Hipp
//- Creation Date: April 23, 2007
//- Revision:      $Revision: 1.17 $
//- Last Modified: $Date: 2011/10/07 13:15:00 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

// **** _SYSTEM INCLUDES_ ******************************************************

#include <string>
#include <sstream>
#include <fstream>

// **** _LOCAL INCLUDES_ *******************************************************

#include "TauPSite.h"
#include "TPVelocityModels.h"
#include "TauPLocVersion.h"
#include "TauPException.h"

#include "Brents.cc"

// **** _BEGIN TAUP NAMESPACE_ *************************************************

template class util::Brents<taup::SplitDistance>;
template class util::Brents<taup::TPZeroFunctional>;

namespace taup {

// **** _STATIC INITIALIZATIONS_************************************************

int               TauPSite::tpsClassCount           = 0;
const double      TauPSite::tpsDerivStepSize        = 1.0e-7;
const double      TauPSite::tpsBrentsZeroInTol      = 1.0e-6;

vector<TravelTimeResult*> TauPSite::tpsReuseTTR = vector<TravelTimeResult*>();

// **** _LOCAL DEFINES_ ********************************************************

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Default Constructor.
//
// *****************************************************************************
TauPSite::TauPSite() : tpsIsVelModlOwned(false),
					   tpsRcvrName(""),
					   tpsRcvrPhase(""),
					   tpsModelName(""),
					   tpsRcvrRad(0.0),
					   tpsRcvrDepth(0.0),
					   tpsSrcRad(0.0),
					   tpsSrcDepth(0.0),
                       tpsDist(0.0),
                       tpsLastDist(-1.0),
					   tpsLastDepth(-1.0)
{
  ++tpsClassCount;

  // intialize functiona and zeroin object

  tpsZeroF.setTauPSite(this);
  tpsZeroIn.setF(tpsZeroF);
  tpsZeroIn.setTolerance(tpsBrentsZeroInTol);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Standard Constructor. Input station and major phase name ("P" or "S").
//
// *****************************************************************************
TauPSite::TauPSite(const string& staname, const string& phase) :
                   tpsIsVelModlOwned(false),
				   tpsRcvrName(staname),
				   tpsRcvrPhase(phase),
				   tpsModelName(""),
                   tpsRcvrRad(0.0),
				   tpsRcvrDepth(0.0),
                   tpsSrcRad(0.0),
				   tpsSrcDepth(0.0),
                   tpsDist(0.0),
				   tpsLastDist(-1.0),
				   tpsLastDepth(-1.0)
{
  ++tpsClassCount;

  // intialize functiona and zeroin object

  tpsZeroF.setTauPSite(this);
  tpsZeroIn.setF(tpsZeroF);
  tpsZeroIn.setTolerance(tpsBrentsZeroInTol);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Copy Constructor.
//
// *****************************************************************************
TauPSite::TauPSite(const TauPSite& tps) :
                   tpsIsVelModlOwned(tps.tpsIsVelModlOwned),
				   tpsRcvrName(tps.tpsRcvrName),
				   tpsRcvrPhase(tps.tpsRcvrPhase),
                   tpsModelName(tps.tpsModelName),
                   tpsRcvrRad(tps.tpsRcvrRad),
                   tpsRcvrDepth(tps.tpsRcvrDepth),
				   tpsSrcRad(tps.tpsSrcRad),
				   tpsSrcDepth(tps.tpsSrcDepth),
				   tpsDist(tps.tpsDist),
				   tpsLastDist(-1.0),
				   tpsLastDepth(-1.0)
{
  int i;

  ++tpsClassCount;

  // intialize functional and zeroin object

  tpsZeroF.setTauPSite(this);
  tpsZeroIn.setF(tpsZeroF);
  tpsZeroIn.setTolerance(tpsBrentsZeroInTol);
  tpsZeroF.setPTop(tps.tpsZeroF.getPTop());

  // copy any velocity layers

  tpsVLayer.reserve(tps.tpsVLayer.size());
  for (i = 0; i < (int) tps.tpsVLayer.size(); ++i)
  {
    if (tpsIsVelModlOwned)
    {
      tpsVLayer.push_back(TPVelocityLayer::newModelCopy(tps.tpsVLayer[i]));
    }
    else
    {
      tpsVLayer.push_back(tps.tpsVLayer[i]);
    }
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Destructor.
//
// *****************************************************************************
TauPSite::~TauPSite()
{
  // delete all TravelTimeResult map entries

  map<double, TravelTimeResult*>::iterator it;
  for (it = tpsTTR.begin(); it != tpsTTR.end(); ++it) delete it->second;
  tpsTTR.clear();

  // delete any owned velocity models
  
  clearVelocityModels();

  // delete tpsReuseTTR stack if this is the last TauPSite object

  --tpsClassCount;
  if (tpsClassCount == 0)
  {
    while(tpsReuseTTR.size() > 0)
    {
      delete tpsReuseTTR.back();
      tpsReuseTTR.pop_back();
    }

	//for (int i=0; i<tpsReuseTTR.size(); ++i)
	//	delete tpsReuseTTR[i];
	//tpsReuseTTR.clear();
  }
}
    
// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Assignment operator.
//
// *****************************************************************************
TauPSite& TauPSite::operator=(const TauPSite& tps)
{
  int i;

  // set attributes

  tpsIsVelModlOwned = tps.tpsIsVelModlOwned;
  tpsDist           = tps.tpsDist;
  tpsSrcRad         = tps.tpsSrcRad;
  tpsSrcDepth       = tps.tpsSrcDepth;
  tpsRcvrName       = tps.tpsRcvrName;
  tpsRcvrRad        = tps.tpsRcvrRad;
  tpsRcvrDepth      = tps.tpsRcvrDepth;
  tpsRcvrPhase      = tps.tpsRcvrPhase;
  tpsModelName      = tps.tpsModelName;

  // set functional pTop

  tpsZeroF.setPTop(tps.tpsZeroF.getPTop());

  // copy velocity layers

  tpsVLayer.reserve(tps.tpsVLayer.size());
  for (i = 0; i < (int) tps.tpsVLayer.size(); ++i)
  {
    if (tpsIsVelModlOwned)
    {
      tpsVLayer.push_back(TPVelocityLayer::newModelCopy(tps.tpsVLayer[i]));
    }
    else
    {
      tpsVLayer.push_back(tps.tpsVLayer[i]);
    }
  }

  // exit

  return *this;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Sets 'this' sites depth.
//
// *****************************************************************************
void TauPSite::setSiteDepth(double depth)
{
  // set depth into the zero in functional

  tpsRcvrDepth = depth;
  tpsZeroF.setReceiverDepth(depth);
  tpsRcvrRad  = tpsZeroF.getReceiverRadius();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Sets the velocity models for 'this' TauPSite ... model ownership
//! is set to NOT OWNED (tpsIsVelModlOwned = false).
//
// *****************************************************************************
void TauPSite::setVelocityModels(const vector<TPVelocityLayer*>& vm)
{
  clearVelocityModels();
  tpsIsVelModlOwned = false;
  tpsVLayer = vm;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Appends another velocity model to the profile for 'this' TauPSite.
//! If this is the first then the model pointer is saved and ownership
//! is set to NOT OWNED (tpsIsVelModlOwned = false). If this is the second
//! or greater velocity layer added then if tpsIsVelModlOwned is false
//! the pointer is added else a copy of the object is made and added.
//
// *****************************************************************************
void TauPSite::appendVelocityModel(TPVelocityLayer* vm)
{
  // see if this is the first entry

  if ((int) tpsVLayer.size() > 0)
  {
    // not the first entry ... if owned create a new copy ... else
    // simply add pointer

    if (tpsIsVelModlOwned)
      tpsVLayer.push_back(TPVelocityLayer::newModelCopy(vm));
    else
      tpsVLayer.push_back(vm);
  }
  else
  {
    // first entry ... save vm and set ownership to false

    tpsVLayer.push_back(vm);
    tpsIsVelModlOwned = false;
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Appends a new OWNED constant velocity model to the profile
//! stack. If this is the first entry a new model is created and
//! tpsIsVelModlOwned is set to true. If this is the second or greater
//! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
//! the new model is added to the stack. Note the top radius must equal
//! the bottom radius of the previous layer else an error is thrown.
//
// *****************************************************************************
void TauPSite::appendConstVelocityModel(double c, double rt, double rb,
                                        const string& layrnam)
{
  // see if this is the first entry

  if ((int) tpsVLayer.size() > 0)
  {
    // not the first entry ... if owned make a new entry ... else
    // throw an error

    if (tpsIsVelModlOwned)
    {
      // check top radius with previous bottom radius ... if ok add new
      // velocity model

      if (rt == tpsVLayer.back()->getRb())
        tpsVLayer.push_back(new VelocityConst(c, rt, rb, layrnam));
      else
      {
		    ostringstream os;
        os << endl << "ERROR: Top radius (" << rt << " km) does not equal "
           << "previous layer bottom radius (" << tpsVLayer.back()->getRb()
           << " km) ..." << endl
  			    << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
            << __FILE__ << " line " << __LINE__ << endl << endl;
		    throw TauPException(os.str());
      }
    }
    else
    {
	    ostringstream os;
      os << endl << "ERROR: Attempting to add a new velocity model layer "
         << "to an existing \"Owned\" velocity model ..." << endl
         << "Operation is not allowed ..." << endl
			   << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
         << __FILE__ << " line " << __LINE__ << endl << endl;
	    throw TauPException(os.str());
    }
  }
  else
  {
    // first entry ... create new velocity model and add it
    // to the profile ... set ownership to true

    tpsVLayer.push_back(new VelocityConst(c, rt, rb, layrnam));
    tpsIsVelModlOwned = true;
  }
  tpsVLayer.back()->setPhaseType(tpsRcvrPhase);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Appends a new OWNED linear velocity model to the profile
//! stack. If this is the first entry a new model is created and
//! tpsIsVelModlOwned is set to true. If this is the second or greater
//! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
//! the new model is added to the stack. Note the top radius must equal
//! the bottom radius of the previous layer else an error is thrown.
//
// *****************************************************************************
void TauPSite::appendLinearVelocityModel(double a0, double a1,
                                         double rt, double rb,
                                         const string& layrnam,
                                         double normradius)
{
  // see if this is the first entry

  if ((int) tpsVLayer.size() > 0)
  {
    // not the first entry ... if owned make a new entry ... else
    // throw an error

    if (tpsIsVelModlOwned)
    {
      // check top radius with previous bottom radius ... if ok add new
      // velocity model

      if (rt == tpsVLayer.back()->getRb())
        tpsVLayer.push_back(new VelocityLinear(a0, a1, rt, rb,
                                               layrnam, normradius));
      else
      {
		    ostringstream os;
        os << endl << "ERROR: Top radius (" << rt << " km) does not equal "
           << "previous layer bottom radius (" << tpsVLayer.back()->getRb()
           << " km) ..." << endl
  			    << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
            << __FILE__ << " line " << __LINE__ << endl << endl;
		    throw TauPException(os.str());
      }
    }
    else
    {
	    ostringstream os;
      os << endl << "ERROR: Attempting to add a new velocity model layer "
         << "to an existing \"Owned\" velocity model ..." << endl
         << "Operation is not allowed ..." << endl
			   << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
         << __FILE__ << " line " << __LINE__ << endl << endl;
	    throw TauPException(os.str());
    }
  }
  else
  {
    // first entry ... create new velocity model and add it
    // to the profile ... set ownership to true

    tpsVLayer.push_back(new VelocityLinear(a0, a1, rt, rb,
                                           layrnam, normradius));
    tpsIsVelModlOwned = true;
  }
  tpsVLayer.back()->setPhaseType(tpsRcvrPhase);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Appends a new OWNED quadratic velocity model to the profile
//! stack. If this is the first entry a new model is created and
//! tpsIsVelModlOwned is set to true. If this is the second or greater
//! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
//! the new model is added to the stack. Note the top radius must equal
//! the bottom radius of the previous layer else an error is thrown.
//
// *****************************************************************************
void TauPSite::appendQuadraticVelocityModel(double a0, double a1, double a2,
                                            double rt, double rb,
                                            const string& layrnam,
                                            double normradius)
{
  // see if this is the first entry

  if ((int) tpsVLayer.size() > 0)
  {
    // not the first entry ... if owned make a new entry ... else
    // throw an error

    if (tpsIsVelModlOwned)
    {
      // check top radius with previous bottom radius ... if ok add new
      // velocity model

      if (rt == tpsVLayer.back()->getRb())
        tpsVLayer.push_back(new VelocityQuadratic(a0, a1, a2, rt, rb,
                                                  layrnam, normradius));
      else
      {
		    ostringstream os;
        os << endl << "ERROR: Top radius (" << rt << " km) does not equal "
           << "previous layer bottom radius (" << tpsVLayer.back()->getRb()
           << " km) ..." << endl
  			    << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
            << __FILE__ << " line " << __LINE__ << endl << endl;
		    throw TauPException(os.str());
      }
    }
    else
    {
	    ostringstream os;
      os << endl << "ERROR: Attempting to add a new velocity model layer "
         << "to an existing \"Owned\" velocity model ..." << endl
         << "Operation is not allowed ..." << endl
			   << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
         << __FILE__ << " line " << __LINE__ << endl << endl;
	    throw TauPException(os.str());
    }
  }
  else
  {
    // first entry ... create new velocity model and add it
    // to the profile ... set ownership to true

    tpsVLayer.push_back(new VelocityQuadratic(a0, a1, a2, rt, rb,
                                              layrnam, normradius));
    tpsIsVelModlOwned = true;
  }
  tpsVLayer.back()->setPhaseType(tpsRcvrPhase);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Appends a new OWNED cubic velocity model to the profile
//! stack. If this is the first entry a new model is created and
//! tpsIsVelModlOwned is set to true. If this is the second or greater
//! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
//! the new model is added to the stack. Note the top radius must equal
//! the bottom radius of the previous layer else an error is thrown.
//
// *****************************************************************************
void TauPSite::appendCubicVelocityModel(double a0, double a1,
                                        double a2, double a3,
                                        double rt, double rb,
                                        const string& layrnam,
                                        double normradius)
{
  // see if this is the first entry

  if ((int) tpsVLayer.size() > 0)
  {
    // not the first entry ... if owned make a new entry ... else
    // throw an error

    if (tpsIsVelModlOwned)
    {
      // check top radius with previous bottom radius ... if ok add new
      // velocity model

      if (rt == tpsVLayer.back()->getRb())
        tpsVLayer.push_back(new VelocityCubic(a0, a1, a2, a3, rt, rb,
                                              layrnam, normradius));
      else
      {
		    ostringstream os;
        os << endl << "ERROR: Top radius (" << rt << " km) does not equal "
           << "previous layer bottom radius (" << tpsVLayer.back()->getRb()
           << " km) ..." << endl
  			    << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
            << __FILE__ << " line " << __LINE__ << endl << endl;
		    throw TauPException(os.str());
      }
    }
    else
    {
	    ostringstream os;
      os << endl << "ERROR: Attempting to add a new velocity model layer "
         << "to an existing \"Owned\" velocity model ..." << endl
         << "Operation is not allowed ..." << endl
			   << "TauPLoc Version " << TAUPLOC_VRSN << "  File "
         << __FILE__ << " line " << __LINE__ << endl << endl;
	    throw TauPException(os.str());
    }
  }
  else
  {
    // first entry ... create new velocity model and add it
    // to the profile ... set ownership to true

    tpsVLayer.push_back(new VelocityCubic(a0, a1, a2, a3, rt, rb,
                                          layrnam, normradius));
    tpsIsVelModlOwned = true;
  }
  tpsVLayer.back()->setPhaseType(tpsRcvrPhase);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Clears all velocity models from the current profile. If the
//! models are OWNED they are deleted.
//
// *****************************************************************************
void TauPSite::clearVelocityModels()
{
  int i;

  // delete any owned velocity models

  if (tpsIsVelModlOwned)
  {
    for (i = 0; i < (int) tpsVLayer.size(); ++i) delete tpsVLayer[i];
  }

  // clear the list, reset the flag, and exit

  tpsVLayer.clear();
  tpsIsVelModlOwned = false;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Calculates travel times for all valid phases of this site at the
//! input distance, \em srcdist, and depth, \em srcdepth. If the flag
//! \em evalderivs is true then derivatives are calculated for each input
//! evaluated branch that satisfies the input distance and depth.
//
// *****************************************************************************
void TauPSite::calculateTravelTimes(double srcdist, double srcdepth,
                                    bool evalderivs)
{
  int i;
  double p, pT, pM, pB, pMin, pTeps, tpzeroT, tpzeroB, tpzeroTeps;
  bool zeroTvalid, zeroBvalid, zeroTepsvalid;

  TravelTimeResult* ttr;
  double derivstep = tpsDerivStepSize;

  // exit if no profile is defined

  if ((int) tpsVLayer.size() > 0)
  {
    // return if source distance and depth is the same as the previous call

    if ((srcdist == tpsLastDist) && (srcdepth == tpsLastDepth)) return;

    // save distance and depth ... find layer limits if this is the first call

    tpsLastDist = srcdist;
    tpsLastDepth = srcdepth;
    if (tpsVLayer[0]->getPmin() == -1.0) findLimits();

    // calculate travel times ... clear any entries from the travel time map

    clearTTRMap();

    // get TPZeroFunctional ... set distance and depth, and get velocity profile

    tpsZeroF.setDist(srcdist);
    tpsZeroF.setSourceDepth(srcdepth);

    // get minimum p for which a solution exists (corresponds to the maximum
    // depth of the receiver or source, i.e. which ever is deepest).

    pMin = tpsZeroF.getMinP();
    //dumpLocalSrcRcvrLayers(cout);
    //dumpLayerInfo(cout);

    // calculate direct upgoing component and see if their is a solution ...

    tpsZeroF.setPTop(tpsVLayer[0]->getPt());
    tpsZeroF.setUpGoingZero();

    // get functional at p = pMin and p = 0.0

    tpzeroT = tpsZeroF(pMin);
    zeroTvalid = tpsZeroF.isUpGoingRayValid();

    tpzeroB = tpsZeroF(0.0);
    zeroBvalid = tpsZeroF.isUpGoingRayValid();

    // see if their is a solution

    p = -1.0;
    if (zeroTvalid && zeroBvalid)
    {
      if (tpzeroT == 0.0)
        p = pMin;
      else if (tpzeroB == 0.0)
        p = 0.0;
      else if (tpzeroT * tpzeroB < 0.0)
        p = tpsZeroIn.zeroF(pMin, 0.0);
    }

    // if there was a solution save the result

    if (p != -1.0)
    {
      // found solution save results to travel time results storage
      // evaluate derivatives if requested

      ttr = saveResult(p, -1, false, false, false, false, false);
      tpsTTR[ttr->ttrT] = ttr;
      if (evalderivs)
        evaluateDerivatives(pMin, 0.0, ttr->ttrT, srcdist, srcdepth,
                            derivstep, derivstep, ttr->ttrDerivs);
    }

    // primary travel-time calculator for turning rays in all branches
    // find all branch solutions

    tpsZeroF.setTurningZero();
    for (i = 0; i < (int) tpsVLayer.size(); ++i)
    {
      // get ith layer and type

      TPVelocityLayer& vl = *tpsVLayer[i];
      int lt = vl.getLayerType();

      // find ray parameter limits for layer

      if (lt == 0)
      {
        pT = vl.getPt();
        pB = vl.getPb();
      }
      else if (lt == 1)
      {
        pT = vl.getPmin();
        pB = vl.getPb();
      }
      else if (lt == 2)
      {
        pT = vl.getPt();
        pB = vl.getPmin();
      }

      // only have solution if pB < pMin

      if (pB < pMin)
      {
        // set pT into functional to define upper limit for layer and then
        // adjust pT if it exceeds the minimum turning p (pMin)

        tpsZeroF.setPTop(pT);
        if (pT > pMin) pT = pMin;

        // calculate zero criteria for top layer boundary ... get both
        // turning ray (1) and upgoing (2) zero and validity condition

        tpzeroT = tpsZeroF(pT);
        zeroTvalid = tpsZeroF.isTurningRayValid();

        //if (tst) debugOutZeros(100, pT, vl.getPCrit());

        // see if the layer is split

        if (vl.isSplitLayer())
        {
          // split layer ... calculate bottom boundary at critical point ...
          // get both turning ray (1) and upgoing (2) zero and validity
          // condition ... only calculate if critical p is < pT as pT may
          // have been set to pMin which could be smaller than pM

          pM = vl.getPCrit();
          if (pM < pT)
          {
            tpzeroB = tpsZeroF(pM);
            zeroBvalid = tpsZeroF.isTurningRayValid();

            // if tpZeroT * tpZeroB is <= 0 then a solution was found
            // get solution ray parameter ... check the turning ray first folowed
            // by the upgoing ray for a solution

            p = -1.0;
            if (zeroTvalid && zeroBvalid)
            {
              if (tpzeroT == 0.0)
                p = pT;
              else if (tpzeroB == 0.0)
                p = pM;
              else if (tpzeroT * tpzeroB < 0.0)
                p = tpsZeroIn.zeroF(pT, pM);
              else
              {
                // did not find a sign change. In case this is a region where the
                // turning radius is near the top of a retrograde layer. These can
                // have a very small switch back that will have to be searched for
                // as sometimes it will be the only ray satisfying the requested
                // distance. A search will be performed to find a sign change
                // somewhere between pT and pM. Generally it is near the top so a
                // check of pT + 1.0e-4 will provide the biggest payoff and will
                // be performed first. If that fails a check of (pT + pM) / 2.0
                // will be performed. Finally, if that fails a min/max search will
                // be calculated to find the min/max of a tpsZeroF(pTeps) result
                // along the curve from pTeps = pT to pM. If that fails no point was
                // possible.

                // first try pT + 1.0e-4

                pTeps = pT + 1.0e-4;
                tpzeroTeps = tpsZeroF(pTeps);
                zeroTepsvalid = tpsZeroF.isTurningRayValid();
                if (zeroTepsvalid && (tpzeroT * tpzeroTeps < 0.0))
                {
                  //p = tpsZeroIn.zeroF(pT, pTeps);
                  p = tpsZeroIn.zeroF(pTeps, pM);
                }
                else
                {
                  // pT + 1.0e-4 failed ... try (pT + pM) / 2

                  pTeps = 0.5 * (pT + pM);
                  tpzeroTeps = tpsZeroF(pTeps);
                  zeroTepsvalid = tpsZeroF.isTurningRayValid();
                  if (zeroTepsvalid && (tpzeroT * tpzeroTeps < 0.0))
                  {
                    //p = tpsZeroIn.zeroF(pT, pTeps);
                    p = tpsZeroIn.zeroF(pTeps, pM);
                  }
                  else
                  {
                    // (pT + pM) / 2 failed ... try finding min/max
                    // only try if abs(tpZeroT) < 1.0e-3

                    if (fabs(tpzeroT) < 1.0e-3)
                    {
                      util::Brents<TPZeroFunctional>* mnmxf;
                      mnmxf = new util::Brents<TPZeroFunctional>(tpsZeroF,
                                                                 1.0e-7);
                      if (pT < 0.0)
                        mnmxf->setMaximumSearch();
                      else
                        mnmxf->setMinimumSearch();
                      tpzeroTeps = mnmxf->minF(pT, pTeps, pM, pTeps);
                      zeroTepsvalid = tpsZeroF.isTurningRayValid();
                      if (zeroTepsvalid && (tpzeroT * tpzeroTeps < 0.0))
                      {
                        //p = tpsZeroIn.zeroF(pT, pTeps);
                        p = tpsZeroIn.zeroF(pTeps, pM);
                      }
                    }
                  }
                }
              }
            }

            if (p != -1.0)
            {
              // found solution save results to travel time results storage
              // evaluate derivatives if requested

              ttr = saveResult(p, i, false, false, false, true,
                               tpsZeroF.isTurningZero());
              tpsTTR[ttr->ttrT] = ttr;
              if (evalderivs)
                evaluateDerivatives(pT, pM, ttr->ttrT, srcdist, srcdepth,
                                    derivstep, derivstep, ttr->ttrDerivs);
            }

            // set top boundary result to bottom and continue with bottom
            // half of split layer

            pT = pM;
            tpzeroT = tpzeroB;
            zeroTvalid = zeroBvalid;
          }
        }

        // calculate zero criteria for bottom layer and see if layer
        // brackets a solution ... get both turning ray (1) and upgoing (2)
        // zero and validity condition

        tpzeroB = tpsZeroF(pB);
        zeroBvalid = tpsZeroF.isTurningRayValid();

        // if tpZeroT * tpZeroB is <= 0 then a solution was found
        // get solution ray parameter ... check the turning ray first folowed
        // by the upgoing ray for a solution

        p = -1.0;
        if (zeroTvalid && zeroBvalid)
        {
          if ((tpzeroT == 0.0) && !vl.isSplitLayer())
            p = pT;
          else if (tpzeroB == 0.0)
            p = pB;
          else if (tpzeroT * tpzeroB < 0.0)
            p = tpsZeroIn.zeroF(pT, pB);
        }

        if (p != -1.0)
        {
          // found solution save results to travel time results storage
          // evaluate derivatives if requested

          ttr = saveResult(p, i, false, false, vl.isSplitLayer(), false,
                           tpsZeroF.isTurningZero());
          tpsTTR[ttr->ttrT] = ttr;
          if (evalderivs)
            evaluateDerivatives(pT, pB, ttr->ttrT, srcdist, srcdepth,
                                derivstep, derivstep, ttr->ttrDerivs);
        }

        // test for an interface solution ... these use the upward and
        // downward legs of a ray turning at the bottom of the layer
        // but then traveling between legs along the layer boundary.
        // the distance between source and receiver must be larger than
        // the distance required for a bottom turning ray. Also, must be
        // a valid turning ray for diffraction to work.

        if ((zeroTvalid && zeroBvalid) &&
            (tpzeroB > 0.0) && (i < (int) tpsVLayer.size() - 1))
        {
          if (vl.isPhaseDiffDefined())
          {
            // This interface has a valid time at the input distance and an
            // interface phase has been requested ... tpzeroB contains the
            // diffraction distance ... save results and evaluate derivative
            // if requested

            double pI = vl.getRb() / vl.getVb();

            ttr = saveResultI(pB, pI, tpzeroB, i, true, false);
            tpsTTR[ttr->ttrT] = ttr;
            if (evalderivs)
              evaluateDerivatives(pT, pB, ttr->ttrT, srcdist, srcdepth,
                                  derivstep, derivstep, ttr->ttrDerivs, pI);
          }

          if (vl.isPhaseDiffLowerDefined())
          {
            // This interface has a valid time at the input distance and an
            // interface phase has been requested ... tpzeroB contains the
            // diffraction distance ... save results and evaluate derivative
            // if requested

            double pI = vl.getRb() / tpsVLayer[i+1]->getVt();

            ttr = saveResultI(pB, pI, tpzeroB, i, false, true);
            tpsTTR[ttr->ttrT] = ttr;
            if (evalderivs)
              evaluateDerivatives(pT, pB, ttr->ttrT, srcdist, srcdepth,
                                  derivstep, derivstep, ttr->ttrDerivs, pI);
          }
        }
      } // if (pB < pMin)
    } // next layer i
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Debug function to output source receiver local conditions.
//
// *****************************************************************************
void TauPSite::dumpLocalSrcRcvrLayers(ostream& os)
{
  int i, ia, ib;
  double minR, maxR, Rsrc, Rrcvr;
  string minNm, maxNm;

  // get source and reciever radius

  Rsrc  = tpsZeroF.getSourceRadius();
  Rrcvr = tpsZeroF.getReceiverRadius();

  // find min and max of source and receiver

  if (Rsrc < Rrcvr)
  {
    minR = Rsrc;
    maxR = Rrcvr;
    minNm = "Source  ";
    maxNm = "Receiver";
  }
  else
  {
    minR = Rrcvr;
    maxR = Rsrc;
    minNm = "Receiver";
    maxNm = "Source  ";
  }

  // move down to layer containing maximum radius

  ia = 0;
  while ((ia < (int) tpsVLayer.size()) && (maxR < tpsVLayer[ia]->getRb())) ++ia;

  // move down to layer containing minimum radius

  ib = ia;
  while ((ib < (int) tpsVLayer.size()) && (minR < tpsVLayer[ib]->getRt())) ++ib;
  --ib;

  // loop from ia to ib and output information

  os << endl;
  for (i = ia; i <= ib; ++i)
  {
    // if this is the first entry write out the top radius

    if (i == ia)
      os << string(60, '-') << " R = " << tpsVLayer[i]->getRt() << endl;

    // write out layer top ray parameter

    os << string(50, ' ') << " Pt = " <<  tpsVLayer[i]->getPt() << endl;

    // write out layer and velocity

    os << string(20, ' ') << "Layer " << i << ": V = "
       << tpsVLayer[i]->getVt() << endl << endl;

    // write out maxR sorce or receiver R and P

    if (i == ia)
      os << "    " << maxNm << " R = " << maxR << ",  P = "
         << tpsVLayer[i]->pAtR(maxR) << endl;

    // write out minR sorce or receiver R and P

    if (i == ib)
      os << "    " << minNm << " R = " << minR << ",  P = "
         << tpsVLayer[i]->pAtR(minR) << endl;

    // write out P and R at layer bottom

    os << string(50, ' ') << " Pb = " <<  tpsVLayer[i]->getPb() << endl;
    os << string(60, '-') << " R = " << tpsVLayer[i]->getRb() << endl;
  }
  os << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Debug function to output layer information.
//
// *****************************************************************************
void TauPSite::dumpLayerInfo(ostream& os)
{
  int i;
  string minNm, maxNm;

  // loop from ia to ib and output information

  os << endl;
  for (i = 0; i < (int) tpsVLayer.size(); ++i)
  {
    // if this is the first entry write out the top radius

    if (i == 0)
      os << string(60, '-') << " R = " << tpsVLayer[i]->getRt() << endl;

    // write out layer top ray parameter and velocity

    os << string(50, ' ') << " Pt = " <<  tpsVLayer[i]->getPt() << endl;
    os << string(50, ' ') << " Vt = " <<  tpsVLayer[i]->getVt() << endl;
    os << string(50, ' ') << " Dt = " <<  tpsVLayer[i]->getDistT() << endl;

    // write out layer

    os << string(20, ' ') << "Layer " << i << endl;

    // write out layer bot ray parameter and velocity

    os << string(50, ' ') << " Pb = " <<  tpsVLayer[i]->getPb() << endl;
    os << string(50, ' ') << " Vb = " <<  tpsVLayer[i]->getVb() << endl;
    os << string(50, ' ') << " Db = " <<  tpsVLayer[i]->getDistB() << endl;

    // write R at layer bottom

    os << string(60, '-') << " R = " << tpsVLayer[i]->getRb() << endl;
  }
  os << endl;
}

//******************************************************************************
//**** Begin: Derivative Evaluation Functions **********************************
//******************************************************************************

// Derivative calculation occurs in the function evaluateDerivatives(...).
// The initial call is made by calculateTravelTimes(...). The arguments
// assume that a valid travel time has been evaluated for some layer whose
// top and bottom ray parameter is given by pT and pB, respectively. The
// valid travel time is passed in as T00. The travel time was evaluated at
// dist and depth. The derivatives will be determined numerically
// by calculating travel times at steps away from dist and depth using the
// step sizes edist and edepth. The derivatives are returned in the array
// derivs[] and include derivs[0] = dT/dDist, derivs[1] = dT/dDpth,
// derivs[2] = d^2T/dDist/dDpth, and derivs[3] = d^2T/dDist^2. A Brents
// zero-in function object is also passed in (bz) containing the
// TPZeroFunctional object that is used to find the travel time at an
// arbitrary distance and depth. The TPZeroFunctional contains the receiver
// location and the layered velocity profiles. The travel time is assumed
// to correspond to a turning ray unless the diffracted ray parameter is
// input as a non-negative value (defaults to -1.0).
//
// On entry the function evaluateDerivatives(...) assumes that the
// TPZeroFunctional object has had its top ray parameter set to pT.
// It immediately calls evalDerivsPrimary(...) which calls the function
// layeredTravelTime(...) at the four primary stencil locations. If
// successful it immediately returns with the derivatives stored in derivs[].
//
// If one or more of the stencil locations cannot be evaluated for the input
// layer then one or two alternate stencils are tried, These include B and A,
// B and C, E and F, and E and D. If any are successful the function returns
// with the evaluated derivatives. Each pair is selected based on the point
// that failed during the primary evaluation. If both alternates also fail
// then the function reEvaluateDerivatives(...) is called which reduces the
// step sizes (edist and edepth) by 1/2 and recalls evaluateDerivatives(...)
// recursively. Each recursive call increments a counter which, if it exceeds
// 5, will trip an error condition which causes a return without evaluting
// the derivatives. The derivative function calling sequence is shown in the
// diagram below.
//
//                         initial call
//                              |
//  |--> evaluateDerivatives <--|
//  |    {
//  |      evalDerivsPrimary ------->|
//  |      evalDerivsAlternateA ---->|
//  |      evalDerivsAlternateB ---->|
//  |      evalDerivsAlternateC ---->|----> layeredTravelTime
//  |      evalDerivsAlternateD ---->|
//  |      evalDerivsAlternateE ---->|
//  |      evalDerivsAlternateF ---->|
//  |
//  |<--   reEvaluateDerivatives
//       }
//

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Calculates the travel time derivatives (\em derivs) at the input
//! distance (\em dist) and depth (\em depth). The derivatives are evaluated
//! numerically by calculating travel time at dist + f*edist and
//! depth + f*edepth. The fraction f is either -1, -1/2, 0, 1/2, or 1 and is
//! evaluated automatically for various stencils. The user must set \em edist
//! and \em edepth to some small step size to begin. Seven different stencils
//! will be tried until one succeeds in evaluating the derivatives. If none
//! succeed the step sized are cut in half and the function is recursively
//! called to try again. If after 5 recursion none of the 7 different
//! stencils are successfully evaluated an error is thrown.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The evaluated travel time at \em dist
//! and \em depth is given by \em T00 and is used to numerically evaluate
//! the derivatives. The brents zero-in function containing the travel time
//! zero functional (TPZeroFunctional) is input as \em bz. A diffracted result
//! is returned at ray parameter \em pB if the interface slowness, \em pI,
//! is non-negative. The default value for \em pI is defined as -1.0
//! (a turning ray). The recursion level (\em derivcnt) is initialized to 0
//! and is incremented each time this function is recursively called.
//!
//! The derivative array contains the following four derivatives on output
//!
//!    derivs[0] = dT/dD
//!    derivs[1] = dT/dr
//!    derivs[2] = d^2T/dD/dr
//!    derivs[3] = d^2T/dD^2
//
// *****************************************************************************
void TauPSite::evaluateDerivatives(double pT, double pB, double T00,
                                   double dist, double depth, double edist,
                                   double edepth, double* derivs, double pI,
                                   int derivcnt)
{
  ++derivcnt;
  int bid = evalDerivsPrimary(pT, pB, T00, dist, depth, edist, edepth,
                              derivs, pI);

  // done if bid = 0 ... otherwise try the alternate stencils

  if (bid == 0)
    return;
  else if (bid == 1)
  {
    // try alternate B
    if (!evalDerivsAlternateB(pT, pB, T00, dist, depth,
                              edist, edepth, derivs, pI))
    {
      // didn't work ... try alternate A
      if (!evalDerivsAlternateA(pT, pB, T00, dist, depth,
                                edist, edepth, derivs, pI))
      {
        // didn't work ... reduce edist and edepth by 2 and recursively
        // call evaluateDerivatives

        reEvaluateDerivatives(pT, pB, T00, dist, depth, edist, edepth,
                              derivs, pI, derivcnt);
      }
    }
  }
  else if (bid == 2)
  {
    // try 1st alternate
    if (!evalDerivsAlternateB(pT, pB, T00, dist, depth,
                              edist, edepth, derivs, pI))
    {
      // try 2nd alternate
      if (!evalDerivsAlternateC(pT, pB, T00, dist, depth,
                                edist, edepth, derivs, pI))
      {
        // didn't work ... reduce edist and edepth by 2 and recursively
        // call evaluateDerivatives

        reEvaluateDerivatives(pT, pB, T00, dist, depth, edist, edepth,
                              derivs, pI, derivcnt);
      }
    }
  }
  else if (bid == 3)
  {
    // try 1st alternate
    if (!evalDerivsAlternateE(pT, pB, T00, dist, depth,
                              edist, edepth, derivs, pI))
    {
      // try 2nd alternate
      if (!evalDerivsAlternateF(pT, pB, T00, dist, depth,
                                edist, edepth, derivs, pI))
      {
        // didn't work ... reduce edist and edepth by 2 and recursively
        // call evaluateDerivatives

        reEvaluateDerivatives(pT, pB, T00, dist, depth, edist, edepth,
                              derivs, pI, derivcnt);
      }
    }
  }
  else if (bid == 4)
  {
    // try 1st alternate
    if (!evalDerivsAlternateE(pT, pB, T00, dist, depth,
                              edist, edepth, derivs, pI))
    {
      // try 2nd alternate
      if (!evalDerivsAlternateD(pT, pB, T00, dist, depth,
                                edist, edepth, derivs, pI))
      {
        // didn't work ... reduce edist and edepth by 2 and recursively
        // call evaluateDerivatives

        reEvaluateDerivatives(pT, pB, T00, dist, depth, edist, edepth,
                              derivs, pI, derivcnt);
      }
    }
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Recursively calls evaluateDerivatives(...) using a distance and
//! depth stepsize reduced by 2.0. If the input \em derivcnt flag is 5 then
//! the function prints and error message and returns without calling the
//! function evaluateDerivatives(...).
//
// *****************************************************************************
void TauPSite::reEvaluateDerivatives(double pT, double pB, double T00,
                                     double dist, double depth,
                                     double edist, double edepth,
                                     double* derivs, double pI, int derivcnt)
{
  // didn't work ... reduce edist and edepth by 2 and recursively
  // call evaluateDerivatives ... exit with an error if after 5
  // recursions the derivatives could not be evaluated

  if (derivcnt < 5)
  {
    edist /= 2.0;
    edepth /= 2.0;
    evaluateDerivatives(pT, pB, T00, dist, depth, edist, edepth,
                        derivs, pI, derivcnt);
  }
  else
  {
    // Error:: could not find derivatives
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
//! depth \em depth using a first order stencil surrounding the current the
//! current interpolation point whose travel time is given by \em T00.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The step size surrounding the current
//! interpolation point is \em edist and \em edepth. A diffracted result
//! is returned if the interface slowness, \em pI, is non-negative. In
//! that case the turning ray component travel time is evaluated at ray
//! parameter \em pB. The default value for \em pI is defined as -1.0
//! (a turning ray). This function is only called by the function
//! evaluateDerivatives.
//!
//! The additional four points necessary to evaluate the derivatives are
//! defined about T00 at steps edist and edepth away using the following
//! stencil:
//!
//!     (4)                  (3)
//!   T(-1,1)               T(1,1)
//!      *                     *
//!
//!               T(0,0)
//!                 *
//!     (1)                  (2)
//!   T(-1,-1)              T(1,-1)
//!      *                     *
//
// *****************************************************************************
int TauPSite::evalDerivsPrimary(double pT, double pB, double T00,
                                double dist, double depth,
                                double edist, double edepth,
                                double* derivs, double pI)
{
  double t1, t2, t3, t4;

  // evaluate the four stencil locations ... return if unable to
  // calculate a valid travel time

                                                                  // delD, delR
  if (layeredTravelTime(pT, pB, dist - edist   , depth + edepth,  //  -1,  1
                        t1, pI) == -1.0) return 1;

  if (layeredTravelTime(pT, pB, dist + edist   , depth - edepth,  //   1, -1
                        t2, pI) == -1.0) return 2;

  if (layeredTravelTime(pT, pB, dist + edist   , depth + edepth,  //   1,  1
                        t3, pI) == -1.0) return 3;

  if (layeredTravelTime(pT, pB, dist - edist   , depth + edepth,  //  -1,  1
                        t4, pI) == -1.0) return 4;

  // valid points ... calculate derivatives

  derivs[0] = (t2 - t1 + t3 - t4) / edist / 4.0;                     // dT/dD
  derivs[1] = (t3 - t2 + t4 - t1) / edepth / 4.0;                    // dT/dr
  derivs[2] = (t3 - t4 - t2 + t1) / edist / edepth / 4.0;            // d2T/dDdr
  derivs[3] = (t3 + t2 - 4.0 * T00 + t4 + t1) / edist / edist / 2.0; // d2T/dD2

  // return 0 (ok)

  return 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
//! depth \em depth using a first order stencil located in front of
//! (+distance) and above (+radius) the current interpolation point whose
//! travel time is given by \em T00.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The step size surrounding the current
//! interpolation point is \em edist and \em edepth. A diffracted result
//! is returned if the interface slowness, \em pI, is non-negative. In
//! that case the turning ray component travel time is evaluated at ray
//! parameter \em pB. The default value for \em pI is defined as -1.0
//! (a turning ray). This function is only called by the function
//! evaluateDerivatives.
//!
//! The additional four points necessary to evaluate the derivatives are
//! defined about T00 at steps edist and edepth away using the following
//! stencil:
//!
//!     (4)                   (3)
//!   T(0,1)                T(1,1)
//!      *                     *
//!
//!
//!
//!                (1)        (2)
//!   T(0,0)     T(.5,0)    T(1,0)
//!      *          *          *
//
// *****************************************************************************
int TauPSite::evalDerivsAlternateA(double pT, double pB, double T00,
                                   double dist, double depth,
                                   double edist, double edepth,
                                   double* derivs, double pI)
{
  double t1, t2, t3, t4;

  // evaluate the four stencil locations ... return if unable to
  // calculate a valid travel time

                                                                  // delD, delR
  if (layeredTravelTime(pT, pB, dist + .5*edist, depth         ,  //  .5,  0
                        t1, pI) == -1.0) return 1;

  if (layeredTravelTime(pT, pB, dist + edist   , depth         ,  //   1,  0
                        t2, pI) == -1.0) return 2;

  if (layeredTravelTime(pT, pB, dist + edist   , depth + edepth,  //   1,  1
                        t3, pI) == -1.0) return 3;

  if (layeredTravelTime(pT, pB, dist           , depth + edepth,  //   0,  1
                        t4, pI) == -1.0) return 4;

  // valid points ... calculate derivatives

  derivs[0] = 2.0 * (t1 - T00) / edist;                              // dT/dD
  derivs[1] = (t4 - T00) / edepth;                                   // dT/dr
  derivs[2] = (t3 - t4 - t2 + T00) / edist / edepth;                 // d2T/dDdr
  derivs[3] = 4.0 * (t2 - 2.0 * t1 + T00) / edist / edist;           // d2T/dD2

  // return 0 (ok)

  return 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
//! depth \em depth using a first order stencil located on either side of
//! (distance) and above (+radius) the current interpolation point whose
//! travel time is given by \em T00.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The step size surrounding the current
//! interpolation point is \em edist and \em edepth. A diffracted result
//! is returned if the interface slowness, \em pI, is non-negative. In
//! that case the turning ray component travel time is evaluated at ray
//! parameter \em pB. The default value for \em pI is defined as -1.0
//! (a turning ray). This function is only called by the function
//! evaluateDerivatives.
//!
//! The additional four points necessary to evaluate the derivatives are
//! defined about T00 at steps edist and edepth away using the following
//! stencil:
//!
//!     (4)                  (3)
//!   T(-1,1)               T(1,1)
//!      *                     *
//!
//!
//!
//!     (1)                  (2)
//!   T(-1,0)     T(0,0)    T(1,0)
//!      *          *          *
//
// *****************************************************************************
int TauPSite::evalDerivsAlternateB(double pT, double pB, double T00,
                                   double dist, double depth,
                                   double edist, double edepth,
                                   double* derivs, double pI)
{
  double t1, t2, t3, t4;

  // evaluate the four stencil locations ... return if unable to
  // calculate a valid travel time

                                                                  // delD, delR
  if (layeredTravelTime(pT, pB, dist - edist   , depth         ,  //  -1,  0
                        t1, pI) == -1.0) return 1;

  if (layeredTravelTime(pT, pB, dist + edist   , depth         ,  //   1,  0
                        t2, pI) == -1.0) return 2;

  if (layeredTravelTime(pT, pB, dist + edist   , depth + edepth,  //   1,  1
                        t3, pI) == -1.0) return 3;

  if (layeredTravelTime(pT, pB, dist - edist   , depth + edepth,  //  -1,  1
                        t4, pI) == -1.0) return 4;

  // valid points ... calculate derivatives

  derivs[0] = (t2 - t1) / edist / 2.0;                               // dT/dD
  derivs[1] = (t3 - t2 + t4 - t1) / edepth / 2.0;                    // dT/dr
  derivs[2] = (t3 - t4 - t2 + t1) / edist / edepth / 2.0;            // d2T/dDdr
  derivs[3] = (t2 - 2.0 * T00 + t1) / edist / edist;                 // d2T/dD2

  // return 0 (ok)

  return 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
//! depth \em depth using a first order stencil located behind
//! (-distance) and above (+radius) the current interpolation point whose
//! travel time is given by \em T00.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The step size surrounding the current
//! interpolation point is \em edist and \em edepth. A diffracted result
//! is returned if the interface slowness, \em pI, is non-negative. In
//! that case the turning ray component travel time is evaluated at ray
//! parameter \em pB. The default value for \em pI is defined as -1.0
//! (a turning ray). This function is only called by the function
//! evaluateDerivatives.
//!
//! The additional four points necessary to evaluate the derivatives are
//! defined about T00 at steps edist and edepth away using the following
//! stencil:
//!
//!     (4)                   (3)
//!   T(-1,1)                T(0,1)
//!      *                     *
//!
//!
//!
//!     (1)        (2)
//!   T(-1,0)    T(-.5,0)    T(0,0)
//!      *          *          *
//
// *****************************************************************************
int TauPSite::evalDerivsAlternateC(double pT, double pB, double T00,
                                   double dist, double depth,
                                   double edist, double edepth,
                                   double* derivs, double pI)
{
  double t1, t2, t3, t4;

  // evaluate the four stencil locations ... return if unable to
  // calculate a valid travel time

                                                                  // delD, delR
  if (layeredTravelTime(pT, pB, dist - edist, depth            ,  //  -1,  0
                        t1, pI) == -1.0) return 1;

  if (layeredTravelTime(pT, pB, dist - .5*edist, depth         ,  // -.5,  0
                        t2, pI) == -1.0) return 2;

  if (layeredTravelTime(pT, pB, dist           , depth + edepth,  //   0,  1
                        t3, pI) == -1.0) return 3;

  if (layeredTravelTime(pT, pB, dist - edist   , depth + edepth,  //  -1,  1
                        t4, pI) == -1.0) return 4;

  // valid points ... calculate derivatives

  derivs[0] = 2.0 * (T00 - t2) / edist;                              // dT/dD
  derivs[1] = (t3 - T00) / edepth;                                   // dT/dr
  derivs[2] = (t3 - t4 - T00 + t1) / edist / edepth;                 // d2T/dDdr
  derivs[3] = 4.0 * (T00 - 2.0 * t2 + t1) / edist / edist;           // d2T/dD2

  // return 0 (ok)

  return 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
//! depth \em depth using a first order stencil located in front of
//! (+distance) and below (-radius) the current interpolation point whose
//! travel time is given by \em T00.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The step size surrounding the current
//! interpolation point is \em edist and \em edepth. A diffracted result
//! is returned if the interface slowness, \em pI, is non-negative. In
//! that case the turning ray component travel time is evaluated at ray
//! parameter \em pB. The default value for \em pI is defined as -1.0
//! (a turning ray). This function is only called by the function
//! evaluateDerivatives.
//!
//! The additional four points necessary to evaluate the derivatives are
//! defined about T00 at steps edist and edepth away using the following
//! stencil:
//!
//!                (4)       (3)
//!   T(0,0)     T(.5,0)    T(1,0)
//!      *          *          *
//!
//!
//!
//!     (1)                   (2)
//!   T(0,-1)               T(1,-1)
//!      *                     *
//
// *****************************************************************************
int TauPSite::evalDerivsAlternateD(double pT, double pB, double T00,
                                   double dist, double depth,
                                   double edist, double edepth,
                                   double* derivs, double pI)
{
  double t1, t2, t3, t4;

  // evaluate the four stencil locations ... return if unable to
  // calculate a valid travel time

                                                                  // delD, delR
  if (layeredTravelTime(pT, pB, dist           , depth - edepth,  //   0, -1
                        t1, pI) == -1.0) return 1;

  if (layeredTravelTime(pT, pB, dist + edist   , depth - edepth,  //   1, -1
                        t2, pI) == -1.0) return 2;

  if (layeredTravelTime(pT, pB, dist + edist   , depth         ,  //   1,  0
                        t3, pI) == -1.0) return 3;

  if (layeredTravelTime(pT, pB, dist + .5*edist, depth         ,  //  .5,  0
                        t4, pI) == -1.0) return 4;

  // valid points ... calculate derivatives

  derivs[0] = 2.0 * (t4 - T00) / edist;                              // dT/dD
  derivs[1] = (T00 - t1) / edepth;                                   // dT/dr
  derivs[2] = (t3 - T00 - t2 + t1) / edist / edepth;                 // d2T/dDdr
  derivs[3] = 4.0 * (t3 - 2.0 * t4 + T00) / edist / edist;           // d2T/dD2

  // return 0 (ok)

  return 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
//! depth \em depth using a first order stencil located on either side of
//! (distance) and below (-radius) the current interpolation point whose
//! travel time is given by \em T00.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The step size surrounding the current
//! interpolation point is \em edist and \em edepth. A diffracted result
//! is returned if the interface slowness, \em pI, is non-negative. In
//! that case the turning ray component travel time is evaluated at ray
//! parameter \em pB. The default value for \em pI is defined as -1.0
//! (a turning ray). This function is only called by the function
//! evaluateDerivatives.
//!
//! The additional four points necessary to evaluate the derivatives are
//! defined about T00 at steps edist and edepth away using the following
//! stencil:
//!
//!     (4)                  (3)
//!   T(-1,0)     T(0,0)    T(1,0)
//!      *          *          *
//!
//!
//!
//!     (1)                  (2)
//!   T(-1,-1)              T(1,-1)
//!      *                     *
//
// *****************************************************************************
int TauPSite::evalDerivsAlternateE(double pT, double pB, double T00,
                                   double dist, double depth,
                                   double edist, double edepth,
                                   double* derivs, double pI)
{
  double t1, t2, t3, t4;

  // evaluate the four stencil locations ... return if unable to
  // calculate a valid travel time

                                                                  // delD, delR
  if (layeredTravelTime(pT, pB, dist - edist   , depth - edepth,  //  -1, -1
                        t1, pI) == -1.0) return 1;

  if (layeredTravelTime(pT, pB, dist + edist   , depth - edepth,  //   1, -1
                        t2, pI) == -1.0) return 2;

  if (layeredTravelTime(pT, pB, dist + edist   , depth         ,  //   1,  0
                        t3, pI) == -1.0) return 3;

  if (layeredTravelTime(pT, pB, dist - edist   , depth         ,  //  -1,  0
                        t4, pI) == -1.0) return 4;

  // valid points ... calculate derivatives

  derivs[0] = (t3 - t4) / edist / 2.0;                               // dT/dD
  derivs[1] = (t3 - t2 + t4 - t1) / edepth / 2.0;                    // dT/dr
  derivs[2] = (t3 - t4 - t2 + t1) / edist / edepth / 2.0;            // d2T/dDdr
  derivs[3] = (t3 - 2.0 * T00 + t4) / edist / edist;                 // d2T/dD2

  // return 0 (ok)

  return 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
//! depth \em depth using a first order stencil located behind
//! (-distance) and below (-radius) the current interpolation point whose
//! travel time is given by \em T00.
//
//! The derivatives are only evaluated if a valid solution exists for some
//! ray parameter between \em pT, the layer top ray parameter, and \em pB,
//! the layer bottom ray parameter. The step size surrounding the current
//! interpolation point is \em edist and \em edepth. A diffracted result
//! is returned if the interface slowness, \em pI, is non-negative. In
//! that case the turning ray component travel time is evaluated at ray
//! parameter \em pB. The default value for \em pI is defined as -1.0
//! (a turning ray). This function is only called by the function
//! evaluateDerivatives.
//!
//! The additional four points necessary to evaluate the derivatives are
//! defined about T00 at steps edist and edepth away using the following
//! stencil:
//!
//!     (4)        (3)
//!   T(-1,0)    T(-.5,0)    T(0,0)
//!      *          *          *
//!
//!
//!
//!     (1)                    (2)
//!   T(-1,-1)               T(0,-1)
//!      *                     *
//
// *****************************************************************************
int TauPSite::evalDerivsAlternateF(double pT, double pB, double T00,
                                   double dist, double depth,
                                   double edist, double edepth,
                                   double* derivs, double pI)
{
  double t1, t2, t3, t4;

  // evaluate the four stencil locations ... return if unable to
  // calculate a valid travel time
                                                                  // delD, delR
  if (layeredTravelTime(pT, pB, dist - edist   , depth - edepth,  //  -1, -1
                        t1, pI) == -1.0) return 1;

  if (layeredTravelTime(pT, pB, dist           , depth - edepth,  //   0, -1
                        t2, pI) == -1.0) return 2;

  if (layeredTravelTime(pT, pB, dist - .5*edist, depth         ,  // -.5,  0
                        t3, pI) == -1.0) return 3;

  if (layeredTravelTime(pT, pB, dist - edist   , depth         ,  //  -1,  0
                        t4, pI) == -1.0) return 4;

  // valid points ... calculate derivatives

  derivs[0] = 2.0 * (T00 - t3) / edist;                              // dT/dD
  derivs[1] = (T00 - t2) / edepth;                                   // dT/dr
  derivs[2] = (T00 - t4 - t2 + t1) / edist / edepth;                 // d2T/dDdr
  derivs[3] = 4.0 * (T00 - 2.0 * t3 + t4) / edist / edist;           // d2T/dD2

  // return 0 (ok)

  return 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Calculates the travel time (\em T) at input distance (\em dist)
//! and depth (\em depth). The solution is performed for a layer whose ray
//! parameter at the top and bottom is defined by \em pT and \em pB. If no
//! solution exists for that layer the function returns -1.0. Otherwise, the
//! ray parameter that provides the input distance and depth is returned.
//! The brents zero-in function containing the travel time zero functional
//! (TPZeroFunctional) is input as \em bz. A diffracted result is returned
//! if the interface slowness, \em pI, is non-negative. The default value
//! for \em pI is defined as -1.0 (a turning ray).
//
// *****************************************************************************
double TauPSite::layeredTravelTime(double pT, double pB, double dist,
                                   double depth, double& T, double pI)
{
  // init ray parameter (p) and set distance and depth

  double p = -1.0;

  tpsZeroF.setDist(dist);
  tpsZeroF.setSourceDepth(depth);
  if (pI == -1.0) // turning wave
  {
    // this is a turning ray ... get functional distance at pT and pB

    double tpzeroT = tpsZeroF(pT);
    double tpzeroB = tpsZeroF(pB);

    // if tpzeroT is zero set p = pT; if tpzeroB is zero set p = pB; else if
    // tpzeroT * tpzeroB is < than zero than pT and pB bracket as solution ...
    // find the ray parameter

    if (tpzeroT == 0.0)
      p = pT;
    else if (tpzeroB == 0.0)
      p = pB;
    else if (tpzeroT * tpzeroB < 0.0)
      p = tpsZeroIn.zeroF(pT, pB);

    // if p is not -1.0 then a solution was found ... calculate travel time

    if (p != -1.0) T = tpsZeroF.time(p);
  }
  else // diffracted wave
  {
    // this is a diffracted ray ... calculate diffraction distance and see
    // if it is positive (or zero).

    tpsZeroF.distance(pB);
    double D = tpsZeroF.getTurningZero();
    if (D >= 0.0)
    {
      // valid diffraction event ... set ray parameter to pB and calculate
      // travel time

      p = pB;
      T = D * pI + tpsZeroF.time(p);
      // pI = vl.getRb() / vel;
      // where vel is vl.getVb() or vl[i+1].getVb()
      // D must be in units of radians
    }
  }

  // done ... return ray parameter

  return p;
}

//******************************************************************************
//**** End: Derivative Evaluation Functions ************************************
//******************************************************************************

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Saves the diffracted ray results into a TravelTimeResult object.
//! and returns the object.
//
// *****************************************************************************
TravelTimeResult* TauPSite::saveResultI(double pB, double pI, double dist,
                                        int i, bool isUpper, bool isLower)
{
  // save ray based results

  TravelTimeResult* ttr = saveResult(pB, i, isUpper, isLower,
                                     false, false, true);

  // calculate layer bottom boundary traversal distance and time

  double ti = dist * pI;

  // set time and distance into result and exit

  ttr->ttrDIntrfc = dist;
  ttr->ttrTIntrfc = ti;
  ttr->ttrT      += ti;

  return ttr;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Saves the travel time results for the ray parameter \em p that turned
//! in velocity layer \em i. The TravelTimeResult object containing the result
//! is returned to the caller.
//
// *****************************************************************************
TravelTimeResult* TauPSite::saveResult(double p, int i,
                                       bool isintrfcupper, bool isintrfclower,
                                       bool isspltlower, bool isspltupper,
                                       bool isturningzero)
{
  TravelTimeResult* ttr;

  // calculate travel time components

  double t = tpsZeroF.time(p);

  // create new result container and add to map sorted on travel time

  if (tpsReuseTTR.size() > 0)
  {
    // TTR is available from reuse stack

    ttr = tpsReuseTTR.back();
    tpsReuseTTR.pop_back();
  }
  else
  {
    // create new TTR

    ttr = new TravelTimeResult();
  }

  // see if rays is "Turning" or "upgoing" or "downgoing"

  if (isturningzero)
  {
    // get velocity model

    TPVelocityLayer& vl = *tpsVLayer[i];

    // set ray type

    ttr->ttrRayType = "Turning";

    // output layer index as a string

    ostringstream os;
    os << i;
    string lvl = os.str();

    // set phase information

    if (isspltupper)
    {
      ttr->ttrPhaseEval = vl.getPhaseType() + lvl + "b+";
      ttr->ttrPhaseName = vl.getPhaseNameUpper();
      if (ttr->ttrPhaseName == "") ttr->ttrPhaseName = vl.getPhaseName();
    }
    else if (isspltlower)
    {
      ttr->ttrPhaseEval = vl.getPhaseType() + lvl + "b-";
      ttr->ttrPhaseName = vl.getPhaseNameLower();
      if (ttr->ttrPhaseName == "") ttr->ttrPhaseName = vl.getPhaseName();
    }
    else if (isintrfcupper)
    {
      ttr->ttrPhaseEval = vl.getPhaseType() + lvl + "i+";
      ttr->ttrPhaseName = vl.getPhaseNameDiff();
    }
    else if (isintrfclower)
    {
      ttr->ttrPhaseEval = vl.getPhaseType() + lvl + "i-";
      ttr->ttrPhaseName = vl.getPhaseNameDiffLower();
    }
    else
    {
      ttr->ttrPhaseEval = vl.getPhaseType() + lvl + "b";
      ttr->ttrPhaseName = vl.getPhaseName();
    }

    // set layer, turning radius, and velocity at radius

    ttr->ttrVelLayer  = &vl;
    ttr->ttrR         = vl.rAtP(p);
    ttr->ttrV         = vl(ttr->ttrR);
  }
  else
  {
    // set ray type to upgoing or downgoing

    string updwn = "";
    double r = 0.0;
    if (tpsZeroF.getReceiverDepth() < tpsZeroF.getSourceDepth())
    {
      ttr->ttrRayType = "UpGoing";
      i = tpsZeroF.getSourceLayerId();
      updwn = "up";
      r = tpsZeroF.getSourceRadius();
    }
    else
    {
      ttr->ttrRayType = "DownGoing";
      i = tpsZeroF.getReceiverLayerId();
      updwn = "dn";
      r = tpsZeroF.getReceiverRadius();
    }

    // get originating layer

    TPVelocityLayer& vl = *tpsVLayer[i];

    // output layer index as a string

    ostringstream os;
    os << i;
    string lvl = os.str();

    // set phase names

    ttr->ttrPhaseEval = vl.getPhaseType() + lvl + updwn;
    ttr->ttrPhaseName = vl.getPhaseName();

    // set layer, source/receiver radius, and velocity at radius

    ttr->ttrVelLayer  = &vl;
    ttr->ttrR         = r;
    ttr->ttrV         = vl(r);
  }

  // set remaining data

  ttr->ttrLayerIndex        = i;
  ttr->ttrIsInterfaceUpper  = isintrfcupper;
  ttr->ttrIsInterfaceLower  = isintrfclower;
  ttr->ttrIsSplitLower      = isspltlower;
  ttr->ttrIsSplitUpper      = isspltupper;
  ttr->ttrP                 = p;
  ttr->ttrT                 = t;
  ttr->ttrDRay              = tpsZeroF.getRayDistance();
  ttr->ttrTRay              = tpsZeroF.getRayTime();
  ttr->ttrDSrc              = tpsZeroF.getSourceLegDistance();
  ttr->ttrTSrc              = tpsZeroF.getSourceLegTime();
  ttr->ttrDRcvr             = tpsZeroF.getReceiverLegDistance();
  ttr->ttrTRcvr             = tpsZeroF.getReceiverLegTime();
  ttr->ttrDIntrfc           = 0.0;
  ttr->ttrTIntrfc           = 0.0;

  // initialize derivatives as undefined

  double* td = ttr->ttrDerivs;
  td[0] = td[1] = td[2]= td[3] = NA_VALUE;

  // return ttr and exit

  return ttr;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Clears the TravelTimeResult map of all entries.
//
// *****************************************************************************
void TauPSite::clearTTRMap()
{
  // loop over all entries in ttrm and push their TravelTimeResult
  // entries onto tpmReuseTTR ... clear map and exit

  map<double, TravelTimeResult*>::iterator it;
  for (it = tpsTTR.begin(); it != tpsTTR.end(); ++it)
    tpsReuseTTR.push_back(it->second);

  tpsTTR.clear();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the first travel time result. If no travel time exists a
//! NULL pointer is returned
//
// *****************************************************************************
TravelTimeResult* TauPSite::getFirstTravelTimeResult()
{
  if ((int) tpsTTR.size() > 0)
    return tpsTTR.begin()->second;
  else
    return (TravelTimeResult*) NULL;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the first travel time (smallest) for the last call to
//! calculateTravelTimes(dist, depth). If no travel time exists a
//! -1.0 is returned.
//
// *****************************************************************************
double TauPSite::getFirstTravelTime() const
{
  if ((int) tpsTTR.size() > 0)
    return tpsTTR.begin()->first;
  else
    return -1.0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the first travel time (smallest) for the last call to
//! calculateTravelTimes(dist, depth) that is not a diffracted ray
//! (e.g. not Pdiff). If only a diffracted results exists this first is
//! returned. If no travel time exists a -1.0 is returned.
//
// *****************************************************************************
double TauPSite::getFirstNonDiffractedTravelTime() const
{
  // see if any rays are present

  if ((int) tpsTTR.size() > 0)
  {
    // loop until a non-diffracted entry is found

    map<double, TravelTimeResult*>::const_iterator it;
    for (it = tpsTTR.begin(); it != tpsTTR.end(); ++it)
    {
      // return first non-diffracted ray

      if (!it->second->ttrIsInterfaceUpper &&
          !it->second->ttrIsInterfaceLower) return it->first;
    }

    // all rays are diffracted ... return first ray.

    return tpsTTR.begin()->first;
  }
  else
    // no valid entries ... return -1.0

    return -1.0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the travel time with the input matching \em phase.
//! If the matching phase was not found and \em matchPhase is true a travel
//! time result of -1 is returned. If the matching phase was not found and
//! \em matchPhase is false then the first arrival travel time is returned.
//
// *****************************************************************************
double TauPSite::getTravelTime(const string& phase, bool matchPhase) const
{ 
  double tt = -1.0;
  map<double, TravelTimeResult*>::const_iterator itttr;

  // find the phase in the phase map that corresponds to the input base models
  // phase. If not found exit as it has already been reset to null above.

  for (itttr = tpsTTR.begin(); itttr != tpsTTR.end(); ++itttr)
  {
    // get next result and see if matchPhase is set

    TravelTimeResult& ttr = *(itttr->second);
    if (matchPhase)
    {
      // match phase exactly ... if found return travel time

      if (((ttr.ttrPhaseName.size() > 0) && (phase == ttr.ttrPhaseName)) ||
          (phase == ttr.ttrPhaseEval))
        return itttr->first;
    }
    else
    {
      // match phase partially ... set first result as solution if not yet set

      if (tt == -1.0) tt = itttr->first;

      // return partial match if found

      if (((ttr.ttrPhaseName.size() > 0) && 
           (phase == ttr.ttrPhaseName.substr(0, phase.size()))) ||
          (phase == ttr.ttrPhaseEval.substr(0, phase.size())))
        return itttr->first;
    }
  }

  // no match ... return -1.0 or first discovered travel time

  return tt;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the travel time result with the input matching \em phase.
//! If the matching phase was not found and \em matchPhase is true a NULL
//! pointer is returned. If the matching phase was not found and \em matchPhase
//! is false then the first arrival travel time result is returned.
//
// *****************************************************************************
TravelTimeResult* TauPSite::getTravelTimeResult(const string& phase,
                                                bool matchPhase)
{ 
  TravelTimeResult* ttr = (TravelTimeResult*) NULL;
  map<double, TravelTimeResult*>::const_iterator itttr;

  // find the phase in the phase map that corresponds to the input base models
  // phase. If not found exit as it has already been reset to null above.

  for (itttr = tpsTTR.begin(); itttr != tpsTTR.end(); ++itttr)
  {
    // get next result and see if matchPhase is set

    TravelTimeResult& ttrit = *(itttr->second);
    if (matchPhase)
    {
      // match phase exactly ... if found return travel time result

      if ((phase == ttrit.ttrPhaseName) || (phase == ttrit.ttrPhaseEval))
        return &ttrit;
    }
    else
    {
      // match phase partially ... set first result as solution if not yet set

      if (!ttr) ttr = itttr->second;

      // see if exact entry can be found

      if (((ttrit.ttrPhaseName.size() > 0) && 
           (phase == ttrit.ttrPhaseName.substr(0, phase.size()))) ||
          (phase == ttrit.ttrPhaseEval.substr(0, phase.size())))
        return &ttrit;
    }
  }

  // no match ... return null or first discovered travel time result

  return ttr;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the total integrated distance of a ray with ray parameter
//! \em p in the input parameter \em d. The function returns true if successful
//! and false if the result is invalid. The result is integrated from the
//! surface of the planet to the turning depth, wherever that may be.
//
//! If the ray parameter is equal to the top of a layers ray parameter in a shadow
//! then the boolean flag \em bottom_pass should be set to true to ensure that
//! the integration does not terminate before accumulating the run-up to the top
//! of the next layer.
//
// *****************************************************************************
bool TauPSite::integrateDistance(double p, double& d, bool bottom_pass)
{
  // exit if p is invalid ... return d = PI if p == 0

  if (p < 0.0)
    return false;
  else if (p == 0.0)
  {
    d = PI / 2.0;
    return true;
  }

  // loop over each layer integrating distance ... exit if the
  // ray parameter is larger than the layers top ray parameter
  // or if the ray turns in the layer. If bottom_pass is set to
  // true then continue past layer where turning occurs.
  // when this case occurs turning happens at a previous layer
  // bottom. This value is only set in setLimits(...) when
  // a shadow region needs to be crossed to determine the
  // distance to the top (or middle) of the next valid layer
  // where turning can occur

  d = 0.0;
  int i = 0;
  while (i < (int) tpsVLayer.size())
  {
    // get the ith layers distance contribution and see if the ray is
    // still valid

    d += tpsVLayer[i]->integDistance(p);
    if (tpsVLayer[i]->invalidRay())
    {
      // ray is invalid ... if the p is equal to the top of the layer
      // then return valid process ... otherwise return invalid

      if (tpsVLayer[i]->getPt() == p)
        return true;
      else
        return false;
    }

    // see if the ray is a turning ray ... if it is and the bottom_pass
    // flag is true then set the flag to false and continue. This will
    // force another layer evaluation which will accumulate any distance
    // in a shadow region at the top of the layer.

    if (tpsVLayer[i]->turningRay())
    {
      if (!bottom_pass) return true;
      bottom_pass = false;
    }

    // increment to next layer and continue

    ++i;
  };

  // can only get here if no turning layer was found ... return false

  return false;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the total integrated distance of a ray with ray parameter
//! \em p in the input parameter \em d. The function returns true if successful
//! and false if the result is invalid. The result is integrated from the
//! surface of the planet to the input depth \em r.
//
// *****************************************************************************
bool TauPSite::integrateDistance(double p, double r, double& d)
{
  // exit if p is invalid

  if (p < 0.0) return false;

  // integrate each layer above the layer that brackets r

  d = 0.0;
  int i = 0;
  while ((i < (int) tpsVLayer.size()) && (r < tpsVLayer[i]->getRt()))
  {
    // get the ith layers distance contribution and see if the ray is
    // still valid

    d += tpsVLayer[i]->integDistance(p, r);
    if (tpsVLayer[i]->invalidRay())
    {
      // ray is invalid ... if the p is equal to the top of the layer
      // then return valid process ... otherwise return invalid

      if (tpsVLayer[i]->getPt() == p)
        return true;
      else
        return false;
    }

    // increment to next layer and continue

    ++i;
  };

  // done ... return true

  return true;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the total integrated distance of a ray with ray parameter
//! \em p in the input parameter \em d. The function returns true if successful
//! and false if the result is invalid. The result is integrated from the
//! \em r1 to depth \em r2. An invalid ray is returned if p is greater than the
//! larger ray parameter evaluated at r1 and r2 or if the ray turns before
//! reaching r2 (r1 must be > r2).
//
// *****************************************************************************
bool TauPSite::integrateDistance(double p, double r1, double r2, double& d)
{
  // exit if p is invalid

  if (p < 0.0) return false;

  // find beginning layer

  int i = 0;
  while ((i < (int) tpsVLayer.size()) && (r1 < tpsVLayer[i]->getRb())) ++i;

  // integrate each layer containing r1 through r2

  d = 0.0;
  while ((i < (int) tpsVLayer.size()) && (r2 < tpsVLayer[i]->getRt()))
  {
    // get the ith layers distance contribution and see if the ray is
    // still valid

    d += tpsVLayer[i]->integDistance(p, r1, r2);
    if (tpsVLayer[i]->invalidRay())
    {
      // ray is invalid ... turned in layer before reaching r2, or r1 is
      // smaller than r2, or p is larger than p evaluated at r1 and r2

      return false;
    }

    // increment to next layer and continue

    ++i;
  };

  // done ... return true

  return true;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the total integrated travel time of a ray with ray parameter
//! \em p in the input parameter \em t. The function returns true if successful
//! and false if the result is invalid. The result is integrated from the
//! surface of the planet to the turning depth, wherever that may be.
//
//! If the ray parameter is equal to the top of a layers ray parameter in a shadow
//! then the boolean flag \em bottom_pass should be set to true to ensure that
//! the integration does not terminate before accumulating the run-up to the top
//! of the next layer.
//
// *****************************************************************************
bool TauPSite::integrateTime(double p, double& t, bool bottom_pass)
{
  // exit if p is invalid

  if (p < 0.0) return false;

  // loop over each layer integrating time ... exit if the
  // ray parameter is larger than the layers top ray parameter
  // or if the ray turns in the layer. If bottom_pass is set to
  // true then continue past layer where turning occurs.
  // when this case occurs turning happens at a previous layer
  // bottom. This value is only set in setLimits(...) when
  // a shadow region needs to be crossed to determine the
  // distance to the top (or middle) of the next valid layer
  // where turning can occur

  t = 0.0;
  int i = 0;
  while (i < (int) tpsVLayer.size())
  {
    // get the ith layers travel time contribution and see if the ray is
    // still valid

    t += tpsVLayer[i]->integTime(p);
    if (tpsVLayer[i]->invalidRay()) return false;

    // see if the ray is a turning ray ... if it is and the bottom_pass
    // flag is true then set the flag to false and continue. This will
    // force another layer evaluation which will accumulate any travel time
    // in a shadow region at the top of the layer.

    if (tpsVLayer[i]->turningRay())
    {
      if (!bottom_pass) return true;
      bottom_pass = false;
    }

    // increment to next layer and continue

    ++i;
  };

  // can only get here if no turning layer was found ... return false

  return false;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the total integrated travel time of a ray with ray parameter
//! \em p in the input parameter \em t. The function returns true if successful
//! and false if the result is invalid. The result is integrated from the
//! surface of the planet to the input depth \em r.
//
// *****************************************************************************
bool TauPSite::integrateTime(double p, double r, double& t)
{
  // exit if p is invalid

  if (p < 0.0) return false;

  // integrate each layer above the layer that brackets r

  t = 0.0;
  int i = 0;
  while ((i < (int) tpsVLayer.size()) && (r < tpsVLayer[i]->getRt()))
  {
    // get the ith layers travel time contribution and see if the ray is
    // still valid

    t += tpsVLayer[i]->integTime(p, r);
    if (tpsVLayer[i]->invalidRay()) return false;

    // increment to next layer and continue

    ++i;
  };

  // done ... return true

  return true;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the total integrated travel time of a ray with ray parameter
//! \em p in the input parameter \em d. The function returns true if successful
//! and false if the result is invalid. The result is integrated from the
//! \em r1 to depth \em r2. An invalid ray is returned if p is greater than the
//! larger ray parameter evaluated at r1 and r2 or if the ray turns before
//! reaching r2 (r1 must be > r2).
//
// *****************************************************************************
bool TauPSite::integrateTime(double p, double r1, double r2, double& t)
{
  // exit if p is invalid

  if (p < 0.0) return false;

  // find beginning layer

  int i = 0;
  while ((i < (int) tpsVLayer.size()) && (r1 < tpsVLayer[i]->getRb())) ++i;

  // integrate each layer containing r1 through r2

  t = 0.0;
  while ((i < (int) tpsVLayer.size()) && (r2 < tpsVLayer[i]->getRt()))
  {
    // get the ith layers travel time contribution and see if the ray is
    // still valid

    t += tpsVLayer[i]->integTime(p, r1, r2);
    if (tpsVLayer[i]->invalidRay())
    {
      // ray is invalid ... turned in layer before reaching r2, or r1 is
      // smaller than r2, or p is larger than p evaluated at r1 and r2

      return false;
    }

    // increment to next layer and continue

    ++i;
  };

  // done ... return true

  return true;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Evaluates the limits of each velocity layer in the input vector.
//! The limits include the layer type, pMin, the derivative of travel time
//! with respect to the ray parameter (slowness), The distance at pT and pB,
//! and a retrogrades layer split distance and ray parameter.
//
// *****************************************************************************
void TauPSite::findLimits()
{
  int i;
  double d, dp, dDdP, delP;

  // cycle through each velocity layer in the input vector and
  // find its limits such as pmin, Dtop, Dbot, dD_dP_top, dD_dP_bot
  // pmin of first layer is Pt(0) ... pmin of the ith layer is
  // min(pmin[i-1], Pt[i-1], Pb[i-1])

  // need a map of beginning distance with the layer that gives it
  // multimap<double D, TPVelocityLayer*>
  //
  // need an interfaceLayer to derive off of TPVelocityLayer containing
  // r0 (the radius of the interface), v0 (the fastest velocity [top or
  // bottom] of the interface), D0 (the distance of a turning ray that
  // bottoms on the layer before the interface), T0 (the time of the turning
  // ray that bottoms on the layer before the interface). Then D1 is pi - D0,
  // and T = r0 * (d - D0) / v0 + 2 * T0. Where D is a distance between D0
  // and D1.

  for (i = 0; i < (int) tpsVLayer.size(); ++i)
  {
    // set pmin for this layer ... set layer type to shadow (3)

    delP = .1 * fabs(tpsVLayer[i]->getPt() - tpsVLayer[i]->getPb());
    if (delP > 1.0) delP = 1.0;

    tpsVLayer[i]->setLayerType(3);
    if (i == 0)
      tpsVLayer[0]->setPmin(max(tpsVLayer[0]->getPt(),
                         tpsVLayer[0]->getPb()) + 1.0);
    else
      tpsVLayer[i]->setPmin(min(tpsVLayer[i-1]->getPmin(),
                             min(tpsVLayer[i-1]->getPt(),
                                 tpsVLayer[i-1]->getPb())));


    // calculate distance and dD/dP at top

    if (tpsVLayer[i]->getPt() <= tpsVLayer[i]->getPmin())
    {
      // ray can turn at top ... evaluate pt component here
      // (Note: set bottom_pass to true so integration won't be
      //  truncated by a bottom turning ray in an earlier
      //  layer ... if next layer results in invalid condition
      //  the answer (d) will still be correct)
      // set layer type to turning (0)

      tpsVLayer[i]->setLayerType(0);
      integrateDistance(tpsVLayer[i]->getPt(), d, true);
      tpsVLayer[i]->setDistT(d);
      if (tpsVLayer[i]->getPb() < tpsVLayer[i]->getPt())
      {
        integrateDistance(tpsVLayer[i]->getPt() - delP, dp);
        dDdP = (d - dp) / delP;
      }
      else
      {
        integrateDistance(tpsVLayer[i]->getPt() + delP, dp);
        dDdP = (dp - d) / delP;
      }
      tpsVLayer[i]->setdDistdPT(dDdP);
    }
    else if (tpsVLayer[i]->getPb() <= tpsVLayer[i]->getPmin())
    {
      // ray can turn between pmin and bottom ... evaluate pmin component here
      // set layer type to bottom half turning - top half shadow (1)

      tpsVLayer[i]->setLayerType(1);
      integrateDistance(tpsVLayer[i]->getPmin(), d, true);
      tpsVLayer[i]->setDistT(d);
      if (tpsVLayer[i]->getPb() < tpsVLayer[i]->getPt())
      {
        //cout << tpsVLayer[i]->getPt() * DEG_TO_RAD << "   "
        //     << tpsVLayer[i]->getPmin() * DEG_TO_RAD << "   "
        //     << (tpsVLayer[i]->getPmin() - delP) * DEG_TO_RAD << endl;
        integrateDistance(tpsVLayer[i]->getPmin() - delP, dp);
        dDdP = (d - dp) / delP;
      }
      else
      {
        integrateDistance(tpsVLayer[i]->getPmin() + delP, dp);
        dDdP = (dp - d) / delP;
      }
      tpsVLayer[i]->setdDistdPT(dDdP);
    }

    // calculate distance and dD/dP at bottom

    //if (tpsVLayer[i]->getRb() == 0.0) delP *= 10.0;
    if (tpsVLayer[i]->getPb() <= tpsVLayer[i]->getPmin())
    {
      // ray can turn at bottom ... evaluate pb component here
      // layer type has already been set to turning (0)
      // or bottom half turning (1)
      
      integrateDistance(tpsVLayer[i]->getPb(), d);
      tpsVLayer[i]->setDistB(d);
      if (tpsVLayer[i]->getPb() > tpsVLayer[i]->getPt())
      {
        integrateDistance(tpsVLayer[i]->getPb() - delP, dp);
        dDdP = (d - dp) / delP;
      }
      else
      {
        integrateDistance(tpsVLayer[i]->getPb() + delP, dp);
        dDdP = (dp - d) / delP;
      }
      tpsVLayer[i]->setdDistdPB(dDdP);
    }
    else if (tpsVLayer[i]->getPt() <= tpsVLayer[i]->getPmin())
    {
      // ray can turn between top and pmin ... evaluate pmin component here
      // set layer type to top half turning - bottom half shadow (2)

      tpsVLayer[i]->setLayerType(2);
      integrateDistance(tpsVLayer[i]->getPmin(), d);
      tpsVLayer[i]->setDistB(d);
      if (tpsVLayer[i]->getPb() > tpsVLayer[i]->getPt())
      {
        integrateDistance(tpsVLayer[i]->getPmin() - delP, dp);
        dDdP = (d - dp) / delP;
      }
      else
      {
        integrateDistance(tpsVLayer[i]->getPmin() + delP, dp);
        dDdP = (dp - d) / delP;
      }
      tpsVLayer[i]->setdDistdPB(dDdP);
    }
    //if (tpsVLayer[i]->getPb() == 0.0) tpsVLayer[i]->setdDistdPB(-1.0);

    if (false)
    {
      double pm = min(tpsVLayer[i]->getPt(), tpsVLayer[i]->getPmin());
      if (i == 0) cout << "layer Pt[min](deg) Dt(deg) Pb(deg)  Db(deg)" << endl;
      cout << i << "     "
           << pm * DEG_TO_RAD << "      "
           << 2.0 * tpsVLayer[i]->getDistT() * RAD_TO_DEG << "  "
           << tpsVLayer[i]->getPb() * DEG_TO_RAD << "  "
           << 2.0 * tpsVLayer[i]->getDistB() * RAD_TO_DEG << endl;
    }

    // test for critical point in layer

    double a, b, c;

    a = tpsVLayer[i]->getdDistdPT();
    b = tpsVLayer[i]->getdDistdPB();
    c = a * b;
    if (tpsVLayer[i]->getdDistdPT() * tpsVLayer[i]->getdDistdPB() < 0.0)
    {
      // two values of P can give the same distance in this layer ...
      // find the critical P which yields dDdP = 0.0

      SplitDistance sd(this);
      util::Brents<SplitDistance> zb(sd, tpsBrentsZeroInTol);

      double pml    = min(tpsVLayer[i]->getPt(),
                          tpsVLayer[i]->getPmin()) - delP;
      double pb     = tpsVLayer[i]->getPb() + delP;
      double psplit = 0.5 * (pml + pb);

      if (tpsVLayer[i]->getdDistdPT() > 0.0)
        zb.setMinimumSearch();
      else
        zb.setMaximumSearch();
      double dcrit = zb.minF(pml, pb, psplit, psplit);

      tpsVLayer[i]->setSplitLayer(psplit, dcrit);
    }

    // store layer in map

    //tpmMap(min(d, tpsVLayer[i]->setdDistdPT(dDdP))) = tpsVLayer[i];
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief This function writes out the surface to surface distance for waves
//! turning in layer \em i of the wave velocity structure. The ray parameter
//! (s/deg) is written along with the travel distance (deg) into the input
//! stream \em os. At least /em n points will be written but more may be
//! required if the maximum traversed distance divided by /em n is exceeded in
//! any step. The function automatically decrements the ray-parameter step size
//! for regions where the corresponding distance step size exceeds the
//! aforementioned limit.
//
// *****************************************************************************
void TauPSite::writeLayerData(int i, ostream& os, int n, double f0, double f1)
{
  int m;
  double r, v, p, p0, p1, plast, dlast, d0, d1, dd, dp, ddp, d, t;

  // initialize if necessary

  if (tpsVLayer[0]->getPmin() == -1.0) findLimits();

  // get the ith layer and find its top ray-parameter limit

  TPVelocityLayer* layri = tpsVLayer[i];
  if (layri->getPt() <= layri->getPmin())
    p0 = layri->getPt();
  else
    p0 = layri->getPmin();

  // now get the bottom ray-parameter limit

  if (layri->getPb() <= layri->getPmin())
    p1 = layri->getPb();
  else
    p1 = layri->getPmin();

  // scale limits to p0 = (p1 - p0) * f0 + p0 and p1 = (p1 - p0) * f1 + p0

  p0 = (p1 - p0) * f0 + p0;
  p1 = (p1 - p0) * f1 + p0;

  // get the layers top and bottom distance and find the distance
  // and ray-parameter desired step sizes

  d0 = layri->getDistT();
  d1 = layri->getDistB();
  dd = fabs(d1 - d0) / n;
  dp = (p1 - p0) / n;

  // set p and plast to the top ray-paramter and evaluate distance

  p = plast = p0;
  integrateDistance(p0, d, true);

  // output distance and save to dlast
  // write r, p(r), v(r), d(r), t(r)

  r = layri->rAtP(p);
  v = (*layri)(r);
  integrateTime(p0, t, true);
  os << std::right << i << "    "
     << std::setprecision(8) << std::setw(12)
     << r << "    " << p * DEG_TO_RAD << "   "
     << v << "    " << 2.0 * d * RAD_TO_DEG << "   "
     << 2.0 * t << endl;
  dlast = d;

  // set ray-parameter step size scale to 1 and initial step size to dp
  // loop until p = p1

  m = 1;
  ddp = dp;
  while (p != p1)
  {
    // get next ray-parameter adjust p back to p1 if it is larger

    p = plast + ddp;
    if (p < p1) p = p1;

    // get the distance at p ... see if the distance step exceeds the
    // required step otherwise see if distance step is less than
    // necessary step size dd

    integrateDistance(p, d);
    if (fabs(d - dlast) > dd)
    {
      // step is to large ... decrease ray-paramter step size by halves until
      // the distance step size is <= dd

      while (fabs(d - dlast) > dd)
      {
        // increase scale to decrease step size

        m *= 2;
        ddp = dp / m;

        // evaluate new smaller p and adjust back to p1 if necessary

        p = plast + ddp;
        if (p < p1) p = p1;

        // calculate corresponding distance (d)

        integrateDistance(p, d);
      };
    }
    else if ((fabs(d - dlast) < dd) && (m > 1) && (p != p1))
    {
      // step size may be too small ... increase step size by 2s until the
      // distance step size is just larger than dd, or the maximum ray
      // parameter step size (dp) is attained (occurs when m = 1), or if
      // p is == to the last point p1

      while ((fabs(d - dlast) < dd) && (m > 1) && (p != p1))
      {
        // cut scale to increment step size

        m /= 2;
        ddp = dp / m;

        // evaluate new larger p and adjust back to p1 if necessary

        p = plast + ddp;
        if (p < p1) p = p1;

        // calculate corresponding distance (d)

        integrateDistance(p, d);
      };
    }

    // output point (p,d) and update last to current ... loop for next

    r = layri->rAtP(p);
    v = (*layri)(r);
    integrateTime(p, t);
    os << std::right << i << "    "
       << std::setprecision(8) << std::setw(12)
       << r << "    " << p * DEG_TO_RAD << "   "
       << v << "    " << 2.0 * d * RAD_TO_DEG << "   "
       << 2.0 * t << endl;
    plast = p;
    dlast = d;
  };
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return the amount of heap allocated memory by this object.
//
// *****************************************************************************
int64 TauPSite::get_alloc_memory() const
{
  // get base class allocation

  int64 mem = 0;

  map<double, TravelTimeResult*>::const_iterator it;
  mem += (sizeof(pair<double, TravelTimeResult*>) +
          sizeof(TravelTimeResult)) * tpsTTR.size();
  mem += tpsVLayer.capacity() * sizeof(TPVelocityLayer*);
  for (int i = 0; i < (int) tpsVLayer.size(); ++i)
    mem += tpsVLayer[i]->classSize();

  // return result and exit

  return mem;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to string.
//
// *****************************************************************************
string TauPSite::toString() const
{
  ostringstream os;
  toStream(os, "  ");
  return os.str();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void TauPSite::toStream(ostream& os, string indent) const
{
  int mns;

  string fmt;

  // print class name and increment indent

  mns = indent.size() + class_name().size() + 10;
  os << indent << class_name() << " (" << this << ") "
     << string(79 - mns, '*') << endl << endl;
  indent.append(2, ' ');
  
  // print out run-time data

  os << indent << "Class Count                  = "
     << tpsClassCount << endl;
  os << indent << "Object Size (bytes)          = "
     << sizeof(TauPSite) << endl;
  os << indent << "Memory Size (bytes)          = "
     << TauPSite::get_memory() << endl << endl;

  // print out serialized data

  os << endl;
  writeData(os);
  os << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
string TravelTimeResult::toString(string indent) const
{
  ostringstream os;
  toStream(os, indent);
  return os.str();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void TravelTimeResult::toStream(ostream& os, string indent) const
{
  os << endl;

  if (ttrPhaseName != "")
    os << indent << "Input Assigned Phase Name             = " << ttrPhaseName
       << endl;

  os << indent << "Evaluated Phase Name                  = " << ttrPhaseEval
     << endl;
  os << indent << "Ray Type                              = " << ttrRayType
     << endl;
  os << indent << "Turning Layer Index (0=top)           = " << ttrLayerIndex
     << endl;

  if (ttrIsInterfaceUpper)
    os << indent << "Ray is diffracted along bottom boundary ..." << endl;
  if (ttrIsInterfaceLower)
    os << indent << "Ray is diffracted along next layers upper boundary ... "
       << endl;
  if (ttrIsSplitUpper)
    os << indent << "Ray turned in upper half of a retrograde layer ..."
       << endl;
  if (ttrIsSplitLower)
    os << indent << "Ray turned in lower half of a retrograde layer ..."
       << endl;

  os << indent << "Ray Parameter (sec/deg)               = "
     << ttrP * DEG_TO_RAD << endl;
  os << indent << "Turning Radius (km)                   = " << ttrR << endl;
  os << indent << "Total Travel Time (sec)               = " << ttrT << endl;
  os << indent << "Surf-to-Surf Distance (deg)           = "
     << 2.0 * ttrDRay * RAD_TO_DEG << endl;
  os << indent << "Surf-to-Surf Travel Time (sec)        = "
     << 2.0 * ttrTRay << endl;
  os << indent << "Surf-to_Source Distance (deg)         = "
     << ttrDSrc * RAD_TO_DEG << endl;
  os << indent << "Surf-to-Source Travel TIme (sec)      = "
     << ttrTSrc << endl;
  os << indent << "Surf-to_Recvr Distance (deg)          = "
     << ttrDRcvr * RAD_TO_DEG << endl;
  os << indent << "Surf-to-Recvr Travel TIme (sec)       = "
     << ttrTRcvr << endl;

  if (ttrIsInterfaceUpper || ttrIsInterfaceLower)
  {
    os << indent << "Diffracted dist. along boundary (deg) = "
       << ttrDIntrfc * RAD_TO_DEG << endl;
    os << indent << "Diffracted time along boundary (sec)  = "
       << ttrTIntrfc << endl;
  }

  if (ttrDerivs[0] != NA_VALUE)
  {
    os << indent << "dT/dDistance (sec/deg)             = "
       << ttrDerivs[0] * DEG_TO_RAD << endl;
    os << indent << "dT/dDepth (sec/km)                 = "
       << ttrDerivs[1] << endl;
    os << indent << "d^2T/dDistance/dDepth (sec/deg/km) = "
       << ttrDerivs[2] * DEG_TO_RAD << endl;
    os << indent << "d^2T/dDistance^2 (sec/deg^2)       = "
       << ttrDerivs[3] * DEG_TO_RAD * DEG_TO_RAD << endl;
  }
  os << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief  Writes the TauPSite as a standard ASCII file in CLR format (see
//! TauPModel function readData for a description of the format).
//
// *****************************************************************************
void TauPSite::writeData(const string& filename) const
{
  ofstream os;

  // Open output file

  os.open(filename.c_str(), ios::out);
  writeData(os);
  os.close();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Private function used to output n zero results between p0 and p1.
//
// *****************************************************************************
void TauPSite::debugOutZeros(int n, double p0, double p1)
{
   double del = (p1 - p0) / (n - 1);
   for (int i = 0; i < n; ++i)
   {
     double p = del * i + p0;
     cout << setw(17) << p << "  " << setw(17) << tpsZeroF(p) << endl;
   }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the TauPSite as a standard ASCII file in CLR format (see
//! TauPModel function readData for a description of the format).
//
// *****************************************************************************
void TauPSite::writeData(ostream& os) const
{
  int i;

  // Write our header information (Non-empty strings only.

  os << "  // Site Information" << endl << endl;
  os << "  Name            = \"" << tpsRcvrName << "\"" << endl;
  os << "  Phase           = \"" << tpsRcvrPhase << "\"" << endl;
  os << "  Model           = \"" << tpsModelName << "\"" << endl;
  os << "  Depth(km)       = " << std::fixed << std::showpoint
     << std::setprecision(2) << tpsRcvrDepth << endl;
  os << "  Radius(km)      = " << std::fixed << std::showpoint
     << std::setprecision(2) << tpsRcvrRad << endl;

  // write out CLR layer information.

  string nm;

  // Write CLR format data

  os << "  // CLR format data" << endl << endl;
  if (tpsVLayer.size() > 0)
  {
    // write P velocities only

    for (i = 0; i < (int) tpsVLayer.size(); ++i)
    {
      TPVelocityLayer& tpvl = *tpsVLayer[i];
      os << "  [Layer]" << endl;
      os << "    Name  = \"" << tpvl.getLayerName() << "\""
         << endl;
      tpvl.writeNormRadius(os);
      os << "    Depth = " << std::fixed << std::showpoint
         << std::setprecision(4) << std::setw(12)
         << std::right << tpsZeroF.getPlanetRadius() - tpvl.getRt()
         << std::setw(12) << tpsZeroF.getPlanetRadius() - tpvl.getRb()
         << endl;
      nm = tpvl.getPhaseName();
      if (nm != "")
        os << "    " << tpsRcvrPhase << "PhaseName = " << nm << endl;

      nm = tpvl.getPhaseNameUpper();
      if (nm != "")
        os << "    " << tpsRcvrPhase << "PhaseNameUpper = " << nm << endl;

      nm = tpvl.getPhaseNameLower();
      if (nm != "")
        os << "    " << tpsRcvrPhase << "PhaseNameLower = " << nm << endl;

      nm = tpvl.getPhaseNameDiff();
      if (tpvl.isPhaseDiffDefined())
        os << "    " << tpsRcvrPhase << "PhaseNameDiff = " << nm << endl;

      nm = tpvl.getPhaseNameDiffLower();
      if (tpvl.isPhaseDiffLowerDefined())
        os << "    " << tpsRcvrPhase << "PhaseNameDiffLower = " << nm << endl;
      os << "    Vel" << tpsRcvrPhase << "  = ";
      tpvl.writeVelocity(os);
      os << "  [End:Layer]" << endl << endl;
    }
  }
}

} // end namespace taup
