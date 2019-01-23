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
//- Module:        $RCSfile: TauPSiteFunctionals.h,v $
//- Creator:       Jim Hipp
//- Creation Date: April 23, 2007
//- Revision:      $Revision: 1.9 $
//- Last Modified: $Date: 2012/10/25 00:25:13 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

#ifndef TAUPSITEFNCTNLS_H
#define TAUPSITEFNCTNLS_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <map>
#include <set>
#include <cmath>

using namespace std;
// use standard library objects

// **** _LOCAL INCLUDES_ *******************************************************

// **** _BEGIN TAUP NAMESPACE_ *************************************************

namespace taup {

// **** _FORWARD REFERENCES_ ***************************************************

class TauPSite;

// **** _CLASS CONSTANTS_ ******************************************************

// *****************************************************************************
// **** SplitDistance Definition ***********************************************
// *****************************************************************************

// *****************************************************************************
//
//! \brief Function object used by the Brents minF function to find the minimum
//! (or maximum) of a retrograde layer. The function operator() defines a
//! TauPSite::integrateDistance(...) function as the functional for finding
//! the minimum for some ray parameter p. This object is only used in the
//! TauPModel::findLimits(...) function when a retrograde layer is detected.
//
// *****************************************************************************
class SplitDistance
{
  public:

    //! \brief Standard constructor sets the internal TauPModel and a set of
    //! velocity layers.
    SplitDistance(TauPSite* tps) : sdTPS(tps) {};

    //! Copy constructor.
    SplitDistance(const SplitDistance& sd) : sdTPS(sd.sdTPS) {};

    //! Destructor.
    virtual ~SplitDistance() {};

    //! Assignment operator.
    SplitDistance& operator=(const SplitDistance& sd) {return *this;};

    //! \brief Function object operator() definition. This operator
    //! returns the ray distance as a function of the input ray
    //! parameter \em p.
    double operator()(double p);

  private:

    //! \brief A reference to the velocity layer vector used by this
    //! SplitDistance object.
    TauPSite* sdTPS;

}; // SplitDistance End Definition

// *****************************************************************************
// **** TPZeroFunctional Definition ********************************************
// *****************************************************************************

// *****************************************************************************
//
//! \brief The primary layer search functional used by Brents zeroIn(...)
//! function to find layers that contain a turning ray whose distance matches
//! the distance between the source and receiver.
//
//! The objects operator()(double p) function finds the zero-in value at the
//! input ray parameter p from
//!
//!   zero-in functional   = tpzD - 2.0 * tpzRayLegDist +
//!                          tpzRSrcSgn * tpzSrcLegDist +
//!                          tpzRRcvrSgn * tpzRcvrLegDist;
//!
//! for a bottoming ray or
//!
//!   zero-in functional   = tpzd - 
//!                          tpzRSrcSgn * tpzSrcLegDist -
//!                          tpzRRcvrSgn * tpzRcvrLegDist;
//!
//! for an upgoing or downgoing ray.
//!
//! where
//!
//!   tpzD           = The input distance between source and receiver,
//!   tpzRayLegDist  = 1/2 the integrated distance from planet surface to
//!                    planet surface at the input ray parameter,
//!   tpzSrcLegDist  = The integrated distance from planet surface to the
//!                    source depth at the input ray parameter,
//!   tpzRcvrLegDist = The integrated distance from planet surface to the
//!                    receiver depth at the input ray parameter,
//!   tpzRSrcSgn     = The sign (+-1.0) of the source (-1.0 if the source is
//!                    elevated above the planet surface).
//!   tpzRRcvrSgn    = The sign (+-1.0) of the receiver (-1.0 if the receiver
//!                    is elevated above the planet surface).
//!
//! This function is called in TauPSite::calculateTravelTimes(...) to evaluate
//! each layer as a possible solution for the input source to receiver distance
//! tpzD.
//
// *****************************************************************************
class TPZeroFunctional
{
  public:

    // **** _PUBLIC LIFECYCLES_ ************************************************

    //! \brief Default constructor.
    TPZeroFunctional() : tpzTPS(NULL), tpzRSrc(0.0), tpzRSrcSgn(1.0),
                         tpzRRcvr(0.0), tpzRRcvrSgn(1.0), tpzPLast(-1.0),
                         tpzDLast(-1.0), tpzD(0.0), tpzPT(0.0),
                         tpzRayLegDist(0.0), tpzRayLegTime(0.0),
                         tpzSrcLegDist(0.0), tpzSrcLegTime(0.0),
                         tpzRcvrLegDist(0.0), tpzRcvrLegTime(0.0),
                         tpzIsTurningZero(true), tpzIsRayLegValid(false),
                         tpzIsRcvrLegValid(false), tpzIsSrcLegValid(false),
                         tpzRadius(EARTH_RAD) {};

    //! \brief Standard constructor. Assigns the TauPSite for this
    //! TPZeroFunctional.
    TPZeroFunctional(TauPSite* tps) :
                     tpzTPS(tps), tpzRSrc(0.0), tpzRSrcSgn(1.0),
                     tpzRRcvr(0.0), tpzRRcvrSgn(1.0), tpzPLast(-1.0),
                     tpzDLast(-1.0), tpzD(0.0), tpzPT(0.0),
                     tpzRayLegDist(0.0), tpzRayLegTime(0.0),
                     tpzSrcLegDist(0.0), tpzSrcLegTime(0.0),
                     tpzRcvrLegDist(0.0), tpzRcvrLegTime(0.0),
                     tpzIsTurningZero(true), tpzIsRayLegValid(false),
                     tpzIsRcvrLegValid(false), tpzIsSrcLegValid(false),
                     tpzRadius(EARTH_RAD) {};

    //! Copy constructor.
    TPZeroFunctional(const TPZeroFunctional& tpzf) :
                     tpzTPS(tpzf.tpzTPS),
                     tpzRSrc(tpzf.tpzRSrc), tpzRSrcSgn(tpzf.tpzRSrcSgn),
                     tpzRRcvr(tpzf.tpzRRcvr), tpzRRcvrSgn(tpzf.tpzRRcvrSgn),
                     tpzPLast(tpzf.tpzPLast), tpzDLast(tpzf.tpzDLast),
                     tpzD(tpzf.tpzD), tpzPT(tpzf.tpzPT),
                     tpzRayLegDist(tpzf.tpzRayLegDist),
                     tpzRayLegTime(tpzf.tpzRayLegTime),
                     tpzSrcLegDist(tpzf.tpzSrcLegDist),
                     tpzSrcLegTime(tpzf.tpzSrcLegTime),
                     tpzRcvrLegDist(tpzf.tpzRcvrLegDist),
                     tpzRcvrLegTime(tpzf.tpzRcvrLegTime),
                     tpzIsTurningZero(tpzf.tpzIsTurningZero),
                     tpzIsRayLegValid(tpzf.tpzIsRayLegValid),
                     tpzIsRcvrLegValid(tpzf.tpzIsRcvrLegValid),
                     tpzIsSrcLegValid(tpzf.tpzIsSrcLegValid),
                     tpzRadius(tpzf.tpzRadius) {};

    //! Destructor.
    virtual ~TPZeroFunctional() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment operator.
    TPZeroFunctional& operator=(const TPZeroFunctional& tpzf)
    {
      // set all values from tpzf into this TPZeroFunctional

      tpzD              = tpzf.tpzD;
      tpzPT             = tpzf.tpzPT;
      tpzRSrc           = tpzf.tpzRSrc;
      tpzRSrcSgn        = tpzf.tpzRSrcSgn;
      tpzRRcvr          = tpzf.tpzRRcvr;
      tpzRRcvrSgn       = tpzf.tpzRRcvrSgn;
      tpzPLast          = tpzf.tpzPLast;
      tpzDLast          = tpzf.tpzDLast;
      tpzRayLegDist     = tpzf.tpzRayLegDist;
      tpzRayLegTime     = tpzf.tpzRayLegTime;
      tpzSrcLegDist     = tpzf.tpzSrcLegDist;
      tpzSrcLegTime     = tpzf.tpzSrcLegTime;
      tpzRcvrLegDist    = tpzf.tpzRcvrLegDist;
      tpzRcvrLegTime    = tpzf.tpzRcvrLegTime;
      tpzRadius         = tpzf.tpzRadius;
      tpzTPS            = tpzf.tpzTPS;
      tpzIsTurningZero  = tpzf.tpzIsTurningZero;
      tpzIsRayLegValid  = tpzf.tpzIsRayLegValid;
      tpzIsRcvrLegValid = tpzf.tpzIsRcvrLegValid;
      tpzIsSrcLegValid  = tpzf.tpzIsSrcLegValid;
      tpzPLast          = -1.0;

      // exit

      return *this;
    };

    //! \brief The function objects operator() definition which is
    //! used by a Brents::zeroIn(...) function to find the value of
    //! the ray parameter \em p that gives a source to receiver
    //! distance of tpzD.
    double operator()(double p)
    {
      // calculate new zero in condition if p is different than last
      // call

      if (p != tpzPLast)
      {
        // save p and new zero in condition

        tpzPLast = p;
        distance(p);
        if (tpzIsTurningZero)
          tpzDLast = getTurningZero();
        else
          tpzDLast = getUpGoingZero();
      }

      // return zero in condition at p

      return tpzDLast;
    };

    // **** _PUBLIC METHODS_ ***************************************************

    //! \brief Sets the turning zero for the operator() function
    //! (tpzIsTurningZero is set to true).
    void      setTurningZero()
    {
      tpzIsTurningZero = true;
    };

    //! \brief Returns true if the turning zero is set for return by the
    //! operator() function (tpzIsTurningZero is true).
    bool      isTurningZero()
    {
      return tpzIsTurningZero;
    };

    //! \brief Returns the turning leg zero evaluation from the last distance()
    //! function evaluation.
    double    getTurningZero()
    {
      return (tpzD - (2.0 * tpzRayLegDist - tpzRSrcSgn * tpzSrcLegDist -
                      tpzRRcvrSgn * tpzRcvrLegDist));
    };

    //! \brief Returns true if the turning ray, receiver, and source legs are
    //! valid in the last distance() function call evaluation.
    bool      isTurningRayValid()
    {
      return (tpzIsRayLegValid && tpzIsRcvrLegValid && tpzIsSrcLegValid);
    };

    //! \brief Sets the upgoing zero for the operator() function
    //! (tpzIsTurningZero is set to false).
    void      setUpGoingZero()
    {
      tpzIsTurningZero = false;
    };

    //! \brief Returns true if the upgoing zero is set for return by the
    //! operator() function (tpzIsTurningZero is false).
    bool      isUpGoingZero()
    {
      return !tpzIsTurningZero;
    };

    //! \brief Returns the upgoing leg zero evaluation from the last distance()
    //! function evaluation.
    double    getUpGoingZero()
    {
      //return (tpzD - fabs(tpzRSrcSgn * tpzSrcLegDist -
      //                    tpzRRcvrSgn * tpzRcvrLegDist));
      return (tpzD - tpzSrcLegDist);
    };

    //! \brief Returns true if the upgoing receiver and source legs are valid
    //! in the last distance() function call evaluation.
    bool      isUpGoingRayValid()
    {
      return (tpzIsRcvrLegValid && tpzIsSrcLegValid);
    };

    //! \brief Returns the minimum allowable ray parameter for a ray to transfer
    //! between the source and receiver depths.
    double    getMinP();

    //! Sets the TauPSite
    void      setTauPSite(TauPSite* tps)
    {
      tpzTPS = tps;
      tpzPLast = -1.0;
    };

    //! Gets the TauPSite.
    TauPSite& getTauPSite()
    {
      return *tpzTPS;
    };

    //! Sets the planet radius (default to Earth = 6371.0 km)
    void      setPlanetRadius(double pr)
    {
      tpzRadius = pr;
      tpzPLast = -1.0;
    };

    //! Returns the planet radius (default to Earth = 6371.0 km)
    double    getPlanetRadius() const
    {
      return tpzRadius;
    };

    //! Sets the source radius and sign.
    void      setSourceRadius(double r)
    {
      tpzRSrcSgn = setRadius(r);
      tpzRSrc    = r;
      tpzPLast = -1.0;
    };

    //! Returns the source radius.
    double    getSourceRadius() const
    {
      return tpzRSrc;
    }

    //! Sets the source radius and sign from the input depth.
    void      setSourceDepth(double d)
    {
      setSourceRadius(tpzRadius - d);
    };

    //! Returns the source depth.
    double    getSourceDepth() const
    {
      return tpzRadius - tpzRSrc;
    }

    //! Sets the receiver radius and sign.
    void      setReceiverRadius(double r)
    {
      tpzRRcvrSgn = setRadius(r);
      tpzRRcvr    = r;
      tpzPLast = -1.0;
    };

    //! Returns the receiver radius.
    double    getReceiverRadius() const
    {
      return tpzRRcvr;
    }

    //! Sets the receiver radius and sign from the input depth.
    void      setReceiverDepth(double d)
    {
      setReceiverRadius(tpzRadius - d);
    };

    //! Returns the receiver depth.
    double    getReceiverDepth() const
    {
      return tpzRadius - tpzRRcvr;
    }

    //! Sets the search distance between the source and receiver.
    void      setDist(double d)
    {
      tpzD = d;
      tpzPLast = -1.0;
    };

    //! Sets the search distance between the source and receiver.
    double    getDist() const
    {
      return tpzD;
    };

    //! Sets the layer top ray parameter \em p for the current search layer.
    void      setPTop(double p)
    {
      tpzPT = p;
      tpzPLast = -1.0;
    };

    //! Gets the layer top ray parameter \em p for the current search layer.
    double    getPTop() const
    {
      return tpzPT;
    };

    //! \brief The primary function of this object which calculates the
    //! ray travel distance between the source and the receiver positions
    //! as a function of the input ray parameter \em p. The entire
    //! surface-to-surface ray leg is evaluated in addition to the
    //! surface-to-source and surface-to-receiver legs.
    void      distance(double p);

    //! \brief Calculates the travel time between the source and receiver
    //! for the current layer at the zero in ray parameter value \em p.
    //! This function is only called once the zero in functional determines
    //! the critical ray parameter that gives a source to receiver distance
    //! equal to tpzD.
    double    time(double p);

    //! Returns the surface-to-surface ray distance.
    double getRayDistance() const {return tpzRayLegDist;};

    //! Returns the surface-to-surface ray time.
    double getRayTime() const {return tpzRayLegTime;};

    //! Returns the surface-to-source ray distance.
    double getSourceLegDistance() const {return tpzRSrcSgn * tpzSrcLegDist;};

    //! Returns the surface-to-source ray time.
    double getSourceLegTime() const {return tpzRSrcSgn * tpzSrcLegTime;};

    //! Returns the surface-to-receiver ray distance.
    double getReceiverLegDistance() const
    {
      return tpzRRcvrSgn * tpzRcvrLegDist;
    };

    //! Returns the surface-to-receiver ray time.
    double getReceiverLegTime() const {return tpzRRcvrSgn * tpzRcvrLegTime;};

    //! Return the layer containing the source position.
    int    getSourceLayerId() const {return getRadiusLayerId(tpzRSrc);};

    //! Return the layer containing the receiver position.
    int    getReceiverLayerId() const {return getRadiusLayerId(tpzRRcvr);};

    //! Return the layer containing the input radius \em r.
    int    getRadiusLayerId(double r) const;

  private:

    // **** _PRIVATE METHODS_ **************************************************

    //! Private function that determines if the input radius \em r is below
    //! or above the surface of the Earth. 1.0 is returned if \em r is below
    //! the Earths surface and -1.0 is returned if \em r is above the Earths
    //! surface. If r is above the Earths surface it is recast as a depth to
    //! behave properly when using the velocity models.
    double    setRadius(double& r)
    {
      double rsgn = 1.0;

      // if the input radius exceeds the maximum planet radius then 
      // this is an elevated source ... set r to depth and
      // reverse the sign

      if (r > tpzRadius)
      {
        r = 2.0 * tpzRadius - r;
        rsgn = -1.0;
      }
      return rsgn;
    };

    // **** _PRIVATE DATA_ *****************************************************

    //! \brief The site assigned to this functional
    TauPSite* tpzTPS;

   //! The source radius.
    double tpzRSrc;

    //! The source elevation sign (1.0 for below and -1.0 for above the
    //! Earths surface).
    double tpzRSrcSgn;

    //! The receiver radius.
    double tpzRRcvr;

    //! The receiver elevation sign (1.0 for below and -1.0 for above the
    //! Earths surface).
    double tpzRRcvrSgn;

    //! The last ray parameter used to evaluate the zero in distance criteria.
    double tpzPLast;

    //! The last evaluated zero in distance criteria.
    double tpzDLast;

    //! The distance between the source and receiver.
    double tpzD;

    //! \brief The current layer top ray parameter. This value is redefined for
    //! each layer before searching the layer for a match with tpzD.
    double tpzPT;

    //! The surface-to-surface ray leg distance.
    double tpzRayLegDist;

    //! The surface-to-surface ray leg time.
    double tpzRayLegTime;

    //! The surface-to-source ray leg distance.
    double tpzSrcLegDist;

    //! The surface-to-source ray leg time.
    double tpzSrcLegTime;

    //! The surface-to-receiver ray leg distance.
     double tpzRcvrLegDist;

     //! The surface-to-receiver ray leg time.
     double tpzRcvrLegTime;

     //! A flag which causes the functional operator () to return the turning ray
     //! zero if true. Otherwise, the upgoing ray zero is returned.
     bool   tpzIsTurningZero;

    //! A flag that is true if the last calculated surface-to-surface ray leg
    //! was valid.
    bool   tpzIsRayLegValid;

    //! A flag that is true if the last calculated surface-to-receiver ray leg
    //! was valid.
    bool   tpzIsRcvrLegValid;

    //! A flag that is true if the last calculated surface-to-source ray leg
    //! was valid.
    bool   tpzIsSrcLegValid;

     //! The Planet Radius (Defaults to Earth Radius (6371.0 km).
    double tpzRadius;

}; // end TPZeroFunctional Definition

} // end namespace taup

#endif // TAUPSITEFNCTNLS_H
