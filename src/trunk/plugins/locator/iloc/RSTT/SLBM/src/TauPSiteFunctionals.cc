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
//- Module:        $RCSfile: TauPSiteFunctionals.cc,v $
//- Creator:       Jim Hipp
//- Creation Date: April 23, 2007
//- Revision:      $Revision: 1.10 $
//- Last Modified: $Date: 2011/10/07 13:15:00 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

// **** _SYSTEM INCLUDES_ ******************************************************

#include <string>
#include <sstream>
#include <fstream>

// **** _LOCAL INCLUDES_ *******************************************************

#include "TPVelocityModels.h"
#include "TauPSiteFunctionals.h"
#include "TauPSite.h"

// **** _BEGIN TAUP NAMESPACE_ *************************************************

namespace taup {

// **** _STATIC INITIALIZATIONS_************************************************
// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Function object operator() definition. This operator
//! returns the ray distance as a function of the input ray
//! parameter \em p.
//
// *****************************************************************************
double SplitDistance::operator()(double p)
{
  double d = 0.0;
  sdTPS->integrateDistance(p, d);
  return d;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief The primary function of TPZeroFunctional which calculates the
//! ray travel distance between the source and the receiver positions
//! as a function of the input ray parameter \em p. The entire
//! surface-to-surface ray leg is evaluated in addition to the
//! surface-to-source and surface-to-receiver legs.
//
// *****************************************************************************
void TPZeroFunctional::distance(double p)
{
  // zero all legs and get first layer velocity model

  tpzIsRcvrLegValid = tpzIsSrcLegValid = tpzIsRayLegValid= true;
  tpzRayLegDist = tpzSrcLegDist = tpzRcvrLegDist = 0.0;
  TauPSite& tps = *tpzTPS;
  TPVelocityLayer& vl0 = *(tps.getVelocityModels()[0]);

  // see if p is at the top of the current search layer (tpzPT)
  // and find the surface-to-surface distance

  if (tpzIsTurningZero)
  {
    // find the turning ray

    if (p == tpzPT)
      tpzIsRayLegValid = tps.integrateDistance(p, tpzRayLegDist, true);
    else
      tpzIsRayLegValid = tps.integrateDistance(p, tpzRayLegDist);

    // now find the source and receiver legs which are only
    // evaluated to the source and receiver depths, tpzRSrc and
    // tpzRRcvr, respectively. If the source or receiver lies above 0
    // depth then use the first layer (index 0) as if it is infinite
    // thickness

    if (tpzRRcvr != vl0.getRt())
    {
      // if receiver is elevated then use top layer as an inverted layer

      if (tpzRRcvrSgn == -1.0)
        tpzRcvrLegDist = vl0.integrateDistance(p, tpzRRcvr);
      else
        tpzIsRcvrLegValid = tps.integrateDistance(p, tpzRRcvr, tpzRcvrLegDist);
    }

    if (tpzRSrc!= vl0.getRt())
    {
      // if receiver is elevated then use top layer as an inverted layer

      if (tpzRSrcSgn == -1.0)
        tpzSrcLegDist = vl0.integrateDistance(p, tpzRSrc);
      else
        tpzIsSrcLegValid = tps.integrateDistance(p, tpzRSrc, tpzSrcLegDist);
    }
  }
  else
  {
    // find the up/down going ray

    double r1 = tpzRRcvr;
    double r2 = tpzRSrc;
    if (r1 < r2)
    {
      r1 = tpzRSrc;
      r2 = tpzRRcvr;
    }

    tpzRayLegDist = NA_VALUE;
    tpzRcvrLegDist = NA_VALUE;
    tpzIsSrcLegValid = tps.integrateDistance(p, r1, r2, tpzSrcLegDist);
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Calculates the travel time between the source and receiver
//! for the current layer at the zero in ray parameter value \em p.
//! This function is only called once the zero in functional determines
//! the critical ray parameter that gives a source to receiver distance
//! equal to tpzD.
//
// *****************************************************************************
double TPZeroFunctional::time(double p)
{
  // zero all legs

  tpzRayLegTime = tpzSrcLegTime = tpzRcvrLegTime = 0.0;
  TauPSite& tps = *tpzTPS;
  TPVelocityLayer& vl0 = *(tps.getVelocityModels()[0]);

  // see if p is at the top of the current search layer (tpzPT)
  // and find the surface-to-surface time

  if (tpzIsTurningZero)
  {
    // find the turning ray

    if (p == tpzPT)
      tps.integrateTime(p, tpzRayLegTime, true);
    else
      tps.integrateTime(p, tpzRayLegTime);

    // now find the source and receiver legs which are only
    // evaluated to the source and receiver depths, tpzRSrc and
    // tpzRRcvr, respectively

    if (tpzRRcvrSgn == -1.0)
      tpzRcvrLegTime = vl0.integrateTime(p, tpzRRcvr);
    else
      tps.integrateTime(p, tpzRRcvr, tpzRcvrLegTime);

    if (tpzRSrcSgn == -1.0)
      tpzSrcLegTime = vl0.integrateTime(p, tpzRSrc);
    else
      tps.integrateTime(p, tpzRSrc, tpzSrcLegTime);

    return (2.0 * tpzRayLegTime - tpzRSrcSgn * tpzSrcLegTime -
            tpzRRcvrSgn * tpzRcvrLegTime);
  }
  else
  {
    // find the up/down going ray

    double r1 = tpzRRcvr;
    double r2 = tpzRSrc;
    if (r1 < r2)
    {
      r1 = tpzRSrc;
      r2 = tpzRRcvr;
    }

    tpzRayLegTime = NA_VALUE;
    tpzRcvrLegTime = NA_VALUE;
    tpzIsSrcLegValid = tps.integrateTime(p, r1, r2, tpzSrcLegTime);

    //return fabs(tpzRSrcSgn * tpzSrcLegTime -
    //            tpzRRcvrSgn * tpzRcvrLegTime);
    return tpzSrcLegTime;
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the minimum allowable ray parameter for a ray to transfer
//! between the source and receiver depths.
//
// *****************************************************************************
double    TPZeroFunctional::getMinP()
{
  int i;
  double minR, maxR, pmin, ptt, ptb;

  // get velocity models for this functional

  TauPSite& tps = *tpzTPS;
  const vector<TPVelocityLayer*>& v = tps.getVelocityModels();

  // find deepest radius of source or receiver

  minR = (tpzRSrc < tpzRRcvr) ? tpzRSrc : tpzRRcvr;
  maxR = (tpzRSrc > tpzRRcvr) ? tpzRSrc : tpzRRcvr;

  // move down to layer containing minimum radius

  i = 0;
  pmin = DBL_MAX;
  while ((i < (int) v.size()) && (maxR < v[i]->getRb())) ++i;

  // loop over intermediate layers checking top and bottom ray parameters

  while ((i < (int) v.size()) && (minR < v[i]->getRt()))
  {
    if (maxR > v[i]->getRt())
      ptt = v[i]->getPt();
    else
      ptt = v[i]->pAtR(maxR);

    if (minR > v[i]->getRb())
      ptb = v[i]->pAtR(minR);
    else
      ptb = v[i]->getPb();

    if (ptt < pmin) pmin = ptt;
    if (ptb < pmin) pmin = ptb;

    ++i;
  }

  // done ... return pmin less some small delta

  //return pmin - 1.0e-10;
  return pmin;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the layer containing the input radius.
//
// *****************************************************************************
int TPZeroFunctional::getRadiusLayerId(double r) const
{
  // get velocity models for this functional

  TauPSite& tps = *tpzTPS;
  const vector<TPVelocityLayer*>& v = tps.getVelocityModels();

  int i = 0;
  while (r < v[i]->getRb()) ++i;
  return i;
}

} // end namespace taup
