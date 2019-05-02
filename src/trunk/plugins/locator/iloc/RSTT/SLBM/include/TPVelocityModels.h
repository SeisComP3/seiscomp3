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
//- Module:        $RCSfile: TPVelocityModels.h,v $
//- Creator:       Jim Hipp
//- Creation Date: April 17, 2007
//- Revision:      $Revision: 1.10 $
//- Last Modified: $Date: 2013/07/24 18:24:04 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

#ifndef TPVELOCITYMODELS_H
#define TPVELOCITYMODELS_H

// **** _SYSTEM INCLUDES_ ******************************************************
#include <float.h>
#include <vector>
#include <cmath>
#include <iostream>

using namespace std;
// use standard library objects

// **** _LOCAL INCLUDES_ *******************************************************

#include "TauPGlobals.h"
#include "IntegrateFunction.h"
#include "DataBuffer.h"
#include "CPPUtils.h"

using util::DataBuffer;

using namespace geotess;

//template<class F> class util::IntegrateFunction;

// **** _BEGIN TAUP NAMESPACE_ *************************************************

namespace taup {

// **** _CLASS CONSTANTS_ ******************************************************
// **** _FORWARD REFERENCES_ ***************************************************

class TPVelocityLayer;
class VelocityConst;
class VelocityPower;
class VelocityLinear;
class VelocityQuadratic;
class VelocityCubic;

template<class V> class TPdDistdr;
template<class V> class TPdTaudr;

// *****************************************************************************
//
//! Distance Integrand. Integrating this function over radius yields distance.
//
//! This is the 1D radially symetric layered model distance integrand given by
//!
//!                          p v(r)
//!     dDist/dr = --------------------------
//!                r sqrt(r^2 - (p v(r))^2)
//!
//! This object requires the velocity model as a template parameter (V) from
//! which the velocity at a radial position (v(r)) is found. The ray
//! parameter (p) is set with the function setP(). The velocity model is set
//! at construction and cannot be changed once defined.
//!
//! The equation above is defined in the overloaded function operator() which
//! is called by the numerical integration facility to integrate the function.
//
// *****************************************************************************
template<class V>
class TPdDistdr
{
  public:

    //! Standard constructor.
    //
    //! This is the only means of setting the velocity model into the
    //! integrand. The ray parameter is initialized to 0.
    TPdDistdr(V& v) : diV(v), diP(0.0) {};

    //! Copy constructor.
    TPdDistdr(const TPdDistdr<V>& di) : diV(di.diV), diP(di.diP) {};

    //! Destructor.
    virtual ~TPdDistdr() {};

    //! Assignment Operator.
    //
    //! Since the velocity model is set by reference it cannot be changed.
    //! The assignment operator simply sets the ray parameter as defined in
    //! the input TPdDistdr
    TPdDistdr&     operator=(const TPdDistdr<V>& di)
    {
      diP = di.diP;
      return *this;
    };

    //! \brief Function operator that returns the result of the distance
    //! integrand evaluated at the input radius \em r.
    double         operator()(double r)
    {
      double pv = diP * diV(r);
      double d  = fabs(r - pv) * (r + pv);
      if (d == 0.0)
        return 1.0 / sqrt(DBL_EPSILON * (r + pv));
      else
        return pv / r / sqrt(d);
    };

    //! Sets the ray parameter to the input value \em p.
    void           setP(double p) {diP = p;};

  private:

    //! The ray parameter as set through the function setP(p).
    double         diP;

    //! The velocity model as set through the standard constructor.
    V&             diV;
};

// *****************************************************************************
//
//! /brief Tau Integrand. Integrating this function over radius yields the Tau
//! function result which when added to p * distance gives the travel time.
//
//! This is the 1D radially symetric layered model time integrand given by
//!
//!                 sqrt(r^2 - (p v(r))^2)
//!     dTau/dr = --------------------------
//!                        r v(r)
//!
//! This object requires the velocity model as a template parameter (V) from
//! which the velocity at a radial position (v(r)) is found. The ray
//! parameter (p) is set with the function setP(). The velocity model is set
//! at construction and cannot be changed once defined.
//!
//! The equation above is defined in the overloaded function operator() which
//! is called by the numerical integration facility to integrate the function.
//
// *****************************************************************************
template<class V>
class TPdTaudr
{
  public:

    //! Standard constructor.
    //
    //! This is the only means of setting the velocity model into the
    //! integrand. The ray parameter is initialized to 0.
    TPdTaudr(V& v) : tiV(v), tiP(0.0) {};

    //! Copy constructor.
    TPdTaudr(const TPdTaudr& ti) : tiV(ti.tiV), tiP(ti.tiP) {};

    //! Destructor.
    virtual ~TPdTaudr() {};

    //! Assignment Operator.
    //
    //! Since the velocity model is set by reference it cannot be changed.
    //! The assignment operator simply sets the ray parameter as defined in
    //! the input TPdTaudr
    TPdTaudr&     operator=(const TPdTaudr& ti)
    {
      tiP = ti.tiP;
      return *this;
    };

    //! \brief Function operator that returns the result of the tau
    //! integrand evaluated at the input radius \em r.
    double         operator()(double r)
    {
      double vr = tiV(r);
      double pv = tiP * vr;

      return sqrt(fabs(r - pv) * (r + pv)) / r / vr;
    };

    //! Sets the ray parameter to the input value \em p.
    void           setP(double p) {tiP = p;};

  private:

    //! The ray parameter as set through the function setP(p).
    double         tiP;

    //! The velocity model as set through the standard constructor.
    V&             tiV;
};

// *****************************************************************************
//
//! \brief Abstract base class velocity layer model inherited by all concrete
//! velocity layer classes. The inheritance hierarchy for a typical velocity
//! layer object, VL, is:
//!
//!     VL::VelocityIntegrate<VL>::TPVelocityLayer.
//
//!
//! VL contains the definition of the velocity function for the layer
//! which is evaluated using the overloaded operator ().
//!
//!     virtual double  operator()(double r);
//!
//! VL also contains the definition for retrieving the radius in the layer
//! given an input ray parameter.
//!
//!     virtual double  rAtP(double p);
//!
//! If the distance and or time functions can be evaluated analytically then
//! they are also defined in VL which will override the default numerical
//! versions of at run time.
//!
//!     virtual double integrateDistance(double p, double r,
//!                                      bool r_open = false);
//!     virtual double integrateTime(double p, double r);
//!
//! The VelocityIntegrate<VL> template object is used to exclusively numerically
//! integrate the distance and time functions if analytic versions were
//! not provided in the VL defintion. A template is used to avoid virtual
//! pointer traversal during the numerical integration process which can
//! evaluate the integrand many hundreds of times per integration.
//!
//! This object, the base class TPVelocityLayer, contains the bulk of the
//! contents of the commonality shared between all velocity layers including a
//! descriptive name, a phase name breakdown, the radius, ray parameter,
//! velocity, distance to, and distance/ray parameter derivatives of the layer
//! defined at the top and bottom, layer type designations, retrograde
//! definitions, and current processing defintions for the layer including the
//! last ray parameter used to integrate distance and time and the results of
//! the integration.
//
// *****************************************************************************
class TAUP_EXP TPVelocityLayer
{
  public: 

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default constructor. Sets top and bottom layer radius to zero.
    TPVelocityLayer() : vlRt(0.0), vlRb(0.0), vlVt(-1.0), vlVb(-1.0),
                        vlPt(-1.0), vlPb(-1.0), vlRLast(-1.0),
                        vlDistT(0.0), vlDistB(0.0), vlIRt(0.0), vlIRb(0.0),
                        vlPmin(-1.0), vlLayerType(-1),
                        vlPCrit(-1.0), vlRTurn(-1.0), vlDistCrit(-1.0),
                        vlSplitLayer(false), vlInvalidRay(false),
                        vlPassingRay(false), vlTurningRay(false),
                        vldDistdP_T(0.0), vldDistdP_B(0.0), vlVTurn(-1.0),
                        vlPhaseUpperIDef(false), vlPhaseLowerIDef(false),
                        vlLayerName("")
    {
      vlPhase = vlPhaseUpper = vlPhaseLower = vlPhaseType= "";
      vlPhaseIUpper = vlPhaseILower = "";
    };

    //! Standard constructor. Sets top and bottom layer radius to \em rt and
    //! \em rb.
    TPVelocityLayer(double rt, double rb, const string& layrnam) :
                    vlRt(rt), vlRb(rb), vlVt(-1.0), vlVb(-1.0),
                    vlPt(-1.0), vlPb(-1.0), vlRLast(-1.0),
                    vlDistT(0.0), vlDistB(0.0), vlIRt(0.0), vlIRb(0.0),
                    vldDistdP_T(0.0), vldDistdP_B(0.0), vlRTurn(-1.0),
                    vlPmin(-1.0), vlLayerType(-1), vlPCrit(-1.0),
                    vlDistCrit(-1.0), vlSplitLayer(false), vlVTurn(-1.0),
                    vlInvalidRay(false), vlPassingRay(false),
                    vlPhaseUpperIDef(false), vlPhaseLowerIDef(false),
                    vlTurningRay(false), vlLayerName(layrnam)
    {
      vlPhase = vlPhaseUpper = vlPhaseLower = vlPhaseType = "";
      vlPhaseIUpper = vlPhaseILower = "";
    };

    //! Copy constructor.
    TPVelocityLayer(const TPVelocityLayer& vl) : vlRt(vl.vlRt), vlRb(vl.vlRb),
                    vlVt(vl.vlVt), vlVb(vl.vlVb), vlPt(vl.vlPt), vlPb(vl.vlPb),
                    vlRLast(vl.vlRLast), vlIRt(vl.vlIRt), vlIRb(vl.vlIRb),
                    vlDistT(vl.vlDistT), vlDistB(vl.vlDistB),
                    vldDistdP_T(vl.vldDistdP_T), vldDistdP_B(vl.vldDistdP_B),
                    vlInvalidRay(vl.vlInvalidRay), vlRTurn(vl.vlRTurn),
                    vlPmin(vl.vlPmin), vlLayerType(vl.vlLayerType),
                    vlPCrit(vl.vlPCrit), vlDistCrit(vl.vlDistCrit),
                    vlSplitLayer(vl.vlSplitLayer), vlVTurn(vl.vlVTurn),
                    vlPassingRay(vl.vlPassingRay),
                    vlTurningRay(vl.vlTurningRay), vlLayerName(vl.vlLayerName),
                    vlPhase(vl.vlPhase), vlPhaseUpper(vl.vlPhaseUpper), 
                    vlPhaseLower(vl.vlPhaseLower), vlPhaseType(vl.vlPhaseType),
                    vlPhaseIUpper(vl.vlPhaseIUpper),
                    vlPhaseUpperIDef(vl.vlPhaseUpperIDef),
                    vlPhaseLowerIDef(vl.vlPhaseLowerIDef),
                    vlPhaseILower(vl.vlPhaseILower)
    {};

    //! Destructor.
    virtual ~TPVelocityLayer() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment Operator.
    TPVelocityLayer&       operator=(const TPVelocityLayer& vl)
    {
      vlRt          = vl.vlRt;
      vlRb          = vl.vlRb;
      vlVt          = vl.vlVt;
      vlVb          = vl.vlVb;
      vlPt          = vl.vlPt;
      vlPb          = vl.vlPb;
      vlRTurn       = vl.vlRTurn;
      vlVTurn       = vl.vlVTurn;

      vlLayerName      = vl.vlLayerName;
      vlPhaseType      = vl.vlPhaseType;
      vlPhase          = vl.vlPhase;
      vlPhaseUpper     = vl.vlPhaseUpper;
      vlPhaseLower     = vl.vlPhaseLower;
      vlPhaseIUpper    = vl.vlPhaseIUpper;
      vlPhaseILower    = vl.vlPhaseILower;
      vlPhaseUpperIDef = vl.vlPhaseUpperIDef;
      vlPhaseLowerIDef = vl.vlPhaseLowerIDef;

      vlIRt         = vl.vlIRt;
      vlIRb         = vl.vlIRb;
      vlRLast       = vl.vlRLast;
      vlDistT       = vl.vlDistT;
      vlDistB       = vl.vlDistB;
      vldDistdP_T   = vl.vldDistdP_T;
      vldDistdP_B   = vl.vldDistdP_B;

      vlPmin        = vl.vlPmin;
      vlLayerType   = vl.vlLayerType;

      vlPCrit       = vl.vlPCrit;
      vlDistCrit    = vl.vlDistCrit;
      vlSplitLayer  = vl.vlSplitLayer;

      vlPassingRay  = vl.vlPassingRay;
      vlTurningRay  = vl.vlTurningRay;
      vlInvalidRay  = vl.vlInvalidRay;

      return *this;
    };

    //! Standard operator() overload which returns the velocity at radius
    //! \em r. The radius is defined from the center of the Earth.
    virtual double         operator()(double r) = ABSTRACT;

    // **** _PUBLIC METHODS_ ***************************************************

    //! Initializes the velocity layer evaluating vlVt, vlVb, vlPt, and vlPb;
    void                   init()
    {
      vlVt = operator()(vlRt);
      vlVb = operator()(vlRb);
      vlPt = pAtR(vlRt);
      vlPb = pAtR(vlRb);
    };

    //! \brief TPVelocityLayer factory method that returns a new
    //! TPVelocityLayer object copied from the input object \em tpvl
    static TPVelocityLayer* newModelCopy(TPVelocityLayer* tpvl);

    //! \brief TPVelocityLayer factory method that returns a new
    //! TPVelocityLayer object of derived type name, /em cnam. The
    //! data used to define the layer is given in the DataBuffer
    //! \em buffer.
    static TPVelocityLayer* newModelCopy(const string& cnam,
                                         DataBuffer& buffer);

    //! \brief Used to write the normalized radius parameter (if
    //! defined) to a CLR output stream \em os.
    virtual void           writeNormRadius(ostream& os) const {};

    //! \brief Used to write the CLR formatted velocity profile to the
    //! input output stream \em os.
    virtual void           writeVelocity(ostream& os) const = ABSTRACT;

    //! Standard function that returns the radius as function of the turning
    //! ray parameter \em p. The turning ray parameter is defined as
    //! \em p = r / v(r) where v(r) is the radial velocity
    virtual double         rAtP(double p) = ABSTRACT;

    //! Standard function that returns the turning ray parameter as a function
    //! of radius \em r. The turning ray parameter is defined as
    //! \em p = r / v(r) where v(r) is the radial velocity.
    double                 pAtR(double r) {return r / operator()(r);};

    //! \brief The distance integration function which gives the distance of
    //! the ray travel from the surface of the Earth to the ray turning depth
    //! as a function of the ray parameter \em p (slowness). The integration
    //! terminates before the turning depth if the radius \em r is positive.
    double                 integDistance(double p, double r = -1.0)
    {
      double dist = 0.0;

      // set ray and integrate if valid

      setRay(p);
      if (!vlInvalidRay)
      {
        // use the turning depth (or bottom of the layer) vlRTurn if
        // r is not defined (-1.0) or is smaller than vlRTurn. Otherwise,
        // integrate from r to the top of the layer.

        double rb = vlRTurn;
        bool   tflg = vlTurningRay;
        if ((r != -1.0) && (r > vlRTurn))
        {
          rb = r;
          tflg = false;
        }

        // assign integration parameters and perform integration

        vlRLast = rb;
        dist = integrateDistance(p, rb, tflg);
      }

      // return distance

      return dist;
    };

    //! \brief The distance integration function which gives the distance of
    //! the ray travel from radius \em r1 to radius \em r2
    //! as a function of the ray parameter \em p (slowness). The integration
    //! terminates at the turning depth if that occurs before reaching \em r2.
    double                 integDistance(double p, double r1, double r2)
    {
      double dist = 0.0;

      // set updown ray flags and integrate if valid or turning

      setUpDownRay(p, r1, r2);
      if (!vlInvalidRay)
        dist = integrateDistance(p, vlIRb, vlIRt, vlTurningRay);
      else if (vlTurningRay)
        dist = integrateDistance(p, vlRTurn, vlIRt, true);

      // return distance

      return dist;
    };

    //! \brief The time integration function which gives the time of
    //! the ray travel from the surface of the Earth to the ray turning depth
    //! as a function of the ray parameter \em p (slowness). The integration
    //! terminates before the turning depth if the radius \em r is positive.
    double                 integTime(double p, double r = -1.0)
    {
      double tim = 0.0;

      // get distance component and integrate Tau if valid

      double d = integDistance(p, r);
      if (!vlInvalidRay)
      {
        // ray is valid calculate time from tau or straight-away

        tim = integrateTime(p, vlRLast);
        if (isTimeIntegralTau()) tim += p * d;
      }

      return tim;
    };

    //! \brief The time integration function which gives the time of
    //! the ray travel from radius \em r1 to radius \em r2
    //! as a function of the ray parameter \em p (slowness). The integration
    //! terminates at the turning depth if that occurs before reaching \em r2.
    double                 integTime(double p, double r1, double r2)
    {
      double tim = 0.0;

      // set updown ray flags and integrate if valid or turning

      setUpDownRay(p, r1, r2);
      double r;
      if (!vlInvalidRay)
        r = vlIRb;
      else if (vlTurningRay)
        r = vlRTurn;
      else
        return tim;

      tim = integrateTime(p, r, vlIRt);
      if (isTimeIntegralTau()) tim += p * integrateDistance(p, r, vlIRt);

      return tim;
    };

    //! \brief The distance integration function that calculates the ray
    //! travel distance from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function is
    //! defined by super classes of this object.
    //
    //! If the flag \em r_open is true and a numerical integration is to be
    //! performed then the open limit integration function
    //! IntegrateFunction::integrateAOpenS(...) is called. Otherwise the closed
    //! form function, IntegrateFunction::integrateClosed(...), is used. If the
    //! velocity model is defined analytically then the flag is ignored.
    virtual double         integrateDistance(double p, double ra,
                                             bool r_open = false) = ABSTRACT;

    //! \brief The distance integration function that calculates the ray
    //! travel distance from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function is
    //! defined by super classes of this object.
    //
    //! If the flag \em r_open is true and a numerical integration is to be
    //! performed then the open limit integration function
    //! IntegrateFunction::integrateAOpenS(...) is called. Otherwise the closed
    //! form function, IntegrateFunction::integrateClosed(...), is used. If the
    //! velocity model is defined analytically then the flag is ignored.
    virtual double         integrateDistance(double p, double ra, double rb,
                                             bool r_open = false) = ABSTRACT;

    //! \brief The time (or tau) integration function that calculates the ray
    //! travel time from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function is
    //! defined by super classes of this object.
    virtual double         integrateTime(double p, double ra) = ABSTRACT;

    //! \brief The time (or tau) integration function that calculates the ray
    //! travel time from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function is
    //! defined by super classes of this object.
    virtual double         integrateTime(double p, double ra,
                                         double rb) = ABSTRACT;

    //! \brief The Default time integral is really Tau function which must be
    //! added to p * distance. Override this function to return false
    //! if the time integral returns time and not tau.
    virtual bool           isTimeIntegralTau() const {return true;};

    //! Returns the top of the layer radius.
    double                 getRt() const {return vlRt;};

    //! Returns the bottom of the layer radius.
    double                 getRb() const {return vlRb;};

    //! Returns the top of the layer velocity.
    double                 getVt() const {return vlVt;};

    //! Returns the bottom of the layer velocity.
    double                 getVb() const {return vlVb;};

    //! Returns the top of the layer ray parameter (slowness).
    double                 getPt() const {return vlPt;};

    //! Returns the bottom of the layer ray parameter (slowness).
    double                 getPb() const {return vlPb;};

    //! /brief Returns the last evaluated turning radius for this layer.
    //! If the last evaluation was a passing ray then the bottom radius
    //! is returned. One must inspect the boolean function isTurning() to
    //! determine if the ray really turned in the layer.
    double                 getTurningRadius() const {return vlRTurn;};

    //! /brief Returns the last evaluated turning velocity for this layer.
    //! If the last evaluation was a passing ray then the bottom velocity
    //! is returned. One must inspect the boolean function isTurning() to
    //! determine if the ray really turned in the layer.
    double                 getTurningVelocity() const {return vlVTurn;};

    //! \brief Sets the minimum ray parameter for the layer. If less than
    //! getPt() then the top half of the layer is shadowed.
    void                   setPmin(double pmin) {vlPmin = pmin;};

    //! \brief Returns the minimum passing ray parameter for the layer.
    //! if less than getPt() then the top half of the layer is shadowed.
    double                 getPmin() const {return vlPmin;};

    //! \brief Sets the accumulated distance through all layers upto the
    //! top of this layer given a ray parameter of vlPt() or vlPmin(),
    //! whichever is smaller.
    void                   setDistT(double d) {vlDistT = d;};

    //! \brief Returns the accumulated distance through all layers upto the
    //! top of this layer given a ray parameter of vlPt() or vlPmin(),
    //! whichever is smaller.
    double                 getDistT() const {return vlDistT;};

    //! \brief Sets the accumulated distance through all layers including
    //! this one given a ray parameter of vlPb() or vlPmin(),
    //! whichever is smaller.
    void                   setDistB(double d) {vlDistB = d;};

    //! \brief Returns the accumulated distance through all layers including
    //! this one given a ray parameter of vlPb() or vlPmin(),
    //! whichever is smaller.
    double                 getDistB() const {return vlDistB;};

    //! \brief Sets the derivative of the distance wrt. the ray parameter at
    //! the top of the layer.
    void                   setdDistdPT(double dDdP) {vldDistdP_T = dDdP;};

    //! \brief Returns the derivative of the distance wrt. the ray parameter at
    //! the top of the layer.
    double                 getdDistdPT() const {return vldDistdP_T;};

    //! \brief Sets the derivative of the distance wrt. the ray parameter at
    //! the bottom of the layer.
    void                   setdDistdPB(double dDdP) {vldDistdP_B = dDdP;};

    //! \brief Returns the derivative of the distance wrt. the ray parameter at
    //! the bottom of the layer.
    double                 getdDistdPB() const {return vldDistdP_B;};

    //! Sets the layer type:
    //!   0 = "Turning Layer"
    //!   1 = "Top Shadow, Bottom Turning"
    //!   2 = "Top Turning, Bottom Shadow"
    //!   3 = "Shadow Layer"
    void                   setLayerType(int lt) {vlLayerType = lt;};

    //! Returns the layer type.
    //!   0 = "Turning Layer"
    //!   1 = "Top Shadow, Bottom Turning"
    //!   2 = "Top Turning, Bottom Shadow"
    //!   3 = "Shadow Layer"
    int                    getLayerType() const {return vlLayerType;};

    //! Returns the descriptive layer name.
    const string&          getLayerName() const {return vlLayerName;};

    //! Sets the layer phase type to "P".
    void                   setPhaseTypeP() {vlPhaseType = "P";};

    //! Sets the layer phase type to "S".
    void                   setPhaseTypeS() {vlPhaseType = "S";};

    //! Sets the layer phase type to \em phtype.
    void                   setPhaseType(const string& phtype)
    {vlPhaseType = phtype;};

    //! Returns the layer phase type string.
    const string&          getPhaseType() const {return vlPhaseType;};

    //! Sets the layer default phase name.
    void                   setPhaseName(const string& name) {vlPhase = name;};

    //! Returns the layer default phase name.
    const string&          getPhaseName() const {return vlPhase;};

    //! Sets the upper phase name if the layer is a retrograde layer.
    void                   setPhaseNameUpper(const string& name)
    {vlPhaseUpper = name;};

    //! Returns the upper phase name if the layer is a retrograde layer.
    const string&          getPhaseNameUpper() const {return vlPhaseUpper;};

    //! Sets the lower phase name if the layer is a retrograde layer.
    void                   setPhaseNameLower(const string& name)
    {vlPhaseLower = name;};

    //! Returns the lower phase name if the layer is a retrograde layer.
    const string&          getPhaseNameLower() const {return vlPhaseLower;};

    //! Sets the upper interface phase name.
    void                   setPhaseNameDiff(const string& name)
    {vlPhaseIUpper = name;};

    //! Returns the upper interface phase name if assigned.
    const string&          getPhaseNameDiff() const {return vlPhaseIUpper;};

    //! Sets the lower interface phase name.
    void                   setPhaseNameDiffLower(const string& name)
    {vlPhaseILower = name;};

    //! Returns the lower interface phase name if assigned.
    const string&          getPhaseNameDiffLower() const {return vlPhaseILower;};

    //! \brief Sets the upper diffracted wave phase for this layer to true
    //! even if it has no formal phase name definition. This will default to
    //! use the default evaluated names of X#i+ where # is the layer # and
    //! X is "P" or "S".
    void                   setPhaseDiffDef(bool def)
    {vlPhaseUpperIDef = def;};

    //! \brief Sets the lower diffracted wave phase for this layer to true
    //! even if it has no formal phase name definition. This will default to
    //! use the default evaluated names of X#i- where # is the layer # and
    //! X is "P" or "S".
    void                   setPhaseDiffLowerDef(bool def)
    {vlPhaseLowerIDef = def;};

    //! Returns true if the upper diffracted phase is defined for this layer.
    bool                   isPhaseDiffDefined() const
    {return vlPhaseUpperIDef;};

    //! Returns true if the lower diffracted phase is defined for this layer.
    bool                   isPhaseDiffLowerDefined() const
    {return vlPhaseLowerIDef;};

    //! Returns the ray parameter that gives the retrograde minimum
    //! distance if the layer is a retrograde layer. Otherwise -1.0 is
    //! returned.
    double                 getPCrit() const {return vlPCrit;};

    //! Returns the distance to the retrograde minimum if the layer is a
    //! retrograde layer. Otherwise -1.0 is returned.
    double                 getDistCrit() const {return vlDistCrit;};

    //! Returns true if the layer is a retrograde layer.
    bool                   isSplitLayer() const {return vlSplitLayer;};

    //! \brief Sets the layers retrograde parameters if the layer was found
    //! to contain a retrograde minimum.
    void                   setSplitLayer(double pcrit, double dcrit)
    {
      vlPCrit    = pcrit;
      vlDistCrit = dcrit;
      vlSplitLayer = true;
    };

    //! \brief Returns true if the last evaluated ray parameter was unable to
    //! pass through this layer.
    bool                   invalidRay() const {return vlInvalidRay;};

    //! \brief Returns true if the last evaluated ray parameter passed through
    //! this layer
    bool                   passingRay() const {return vlPassingRay;};

    //! \brief Returns true if the last evaluated ray parameter produced a
    //! turning ray in this layer.
    bool                   turningRay() const {return vlTurningRay;};

    //! Print object data to string.
    string                 toString() const;

    //! Print object data to input stream \em os.
    virtual void           toStream(ostream& os, string indent) const;

    //! Static function that returns the class name.
    static  string         class_name() {return "TPVelocityLayer";};

    //! Virtual function that returns the class name.
    virtual string         get_class_name() const {return class_name();};

    //! Returns the class size.
    virtual int            classSize() const
    {return (int) sizeof(TPVelocityLayer);};

    //! Virtual function that returns False for each model type. These
    //! functions are defined in each model type to return true.
    virtual bool           isVelocityConstant() const {return false;};
    virtual bool           isVelocityPowerLaw() const {return false;};
    virtual bool           isVelocityLinear() const {return false;};
    virtual bool           isVelocityQuadratic() const {return false;};
    virtual bool           isVelocityCubic() const {return false;};

    //! Used to calculate the size of a DataBuffer required to contain
    //! all of the data defined for a TPVelocityLayer object.
    virtual int            bufferSize() const
    {
      int bs = (int) vlLayerName.size() + (int) vlPhaseType.size() +
               (int) vlPhase.size() + (int) vlPhaseUpper.size() +
               (int) vlPhaseLower.size() + (int) vlPhaseIUpper.size() +
               (int) vlPhaseILower.size() + 14 * sizeof(int);
      bs += 2 * sizeof(int) + 3 * sizeof(char) + 16 * sizeof(double);

      return bs;
    };

    //! Used to write the contents of a TPVelocityLayer object into the
    //! input DataBuffer object \em buffer.
    virtual void           serialize(DataBuffer& buffer)
    {
      buffer.writeString(vlLayerName);
      buffer.writeString(vlPhaseType);
      buffer.writeString(vlPhase);
      buffer.writeString(vlPhaseUpper);
      buffer.writeString(vlPhaseLower);
      buffer.writeString(vlPhaseIUpper);
      buffer.writeString(vlPhaseILower);

      buffer.writeInt32(vlLayerType);

      buffer.writeByte(vlSplitLayer);
      buffer.writeByte(vlPhaseUpperIDef);
      buffer.writeByte(vlPhaseLowerIDef);

      buffer.writeDouble(vlRt);
      buffer.writeRawDouble(vlRb);
      buffer.writeRawDouble(vlVt);
      buffer.writeRawDouble(vlVb);
      buffer.writeRawDouble(vlPt);
      buffer.writeRawDouble(vlPb);
      buffer.writeRawDouble(vlRTurn);
      buffer.writeRawDouble(vlVTurn);
      buffer.writeRawDouble(vlDistT);
      buffer.writeRawDouble(vlDistB);
      buffer.writeRawDouble(vldDistdP_T);
      buffer.writeRawDouble(vldDistdP_B);
      buffer.writeRawDouble(vlPmin);
      buffer.writeRawDouble(vlPCrit);
      buffer.writeRawDouble(vlDistCrit);
    };

    //! Used to read the contents of a TPVelocityLayer object from the
    //! input DataBuffer object \em buffer.
    virtual void           deserialize(DataBuffer& buffer)
    {
      vlLayerName   = buffer.readString();
      vlPhaseType   = buffer.readString();
      vlPhase       = buffer.readString();
      vlPhaseUpper  = buffer.readString();
      vlPhaseLower  = buffer.readString();
      vlPhaseIUpper = buffer.readString();
      vlPhaseILower = buffer.readString();

      vlLayerType   = buffer.readInt32();

      vlSplitLayer     = buffer.readByte();
      vlPhaseUpperIDef = buffer.readByte();
      vlPhaseLowerIDef = buffer.readByte();

      vlRt        = buffer.readDouble();
      vlRb        = buffer.readRawDouble();
      vlVt        = buffer.readRawDouble();
      vlVb        = buffer.readRawDouble();
      vlPt        = buffer.readRawDouble();
      vlPb        = buffer.readRawDouble();
      vlRTurn     = buffer.readRawDouble();
      vlVTurn     = buffer.readRawDouble();
      vlDistT     = buffer.readRawDouble();
      vlDistB     = buffer.readRawDouble();
      vldDistdP_T = buffer.readRawDouble();
      vldDistdP_B = buffer.readRawDouble();
      vlPmin      = buffer.readRawDouble();
      vlPCrit     = buffer.readRawDouble();
      vlDistCrit  = buffer.readRawDouble();
    };

  protected:

    //! \brief Sets the ray as invalid, passing, or turning given an input
    //! ray parameter \em p. This function also sets the turning radius and
    //! velocity (vlRTurn and vlVTurn).
    void                   setRay(double p)
    {
      vlInvalidRay = vlPassingRay = vlTurningRay = false;

      // determine limits of integration

      if (p < vlPt)
      {
        // valid integral determine bottom limit

        if (p < vlPb)
        {
          // ray passes through layer

          vlRTurn = vlRb;
          vlVTurn = vlVb;
          vlPassingRay = true;
        }
        else if (p == vlPb)
        {
          // ray turns in layer at bottom

          vlRTurn = vlRb;
          vlVTurn = vlVb;
          vlTurningRay = true;
        }
        else
        {
          // ray turns in layer

          vlRTurn = rAtP(p);
          vlVTurn = operator()(vlRTurn);
          vlTurningRay = true;
        }
      }
      else
      {
        // invalid ray

        vlInvalidRay = true;
      }
    };

    //! \brief Sets the upgoing / downgoing flags for an integration between
    //! \em r1 and \em r2 for the input ray parameter \em p. If the ray turns
    //! between r1 and r2 in the layer, or the ray parameter is larger than
    //! the largest ray parameter evaluated at r1 and r2, or the r2 > r1 then
    //! the invalid ray flag is set.
    void                 setUpDownRay(double p, double r1, double r2)
    {
      vlPassingRay = vlTurningRay = false;
      vlInvalidRay = true;

      // ensure validity of radial bounds r1 and r2

      if ((r2 < vlRt) && (r1 > vlRb))
      {
        // set top radius to the smaller of vlRt and r1

        double rtt = vlRt;
        if (r1 < vlRt) rtt = r1;

        // set bottom radius to the larger of vlRb and r2

        double rbb = vlRb;
        if (r2 > vlRb) rbb = r2;

        // find minimum and maximum ray paramameter at rtt and rbb

        double ptt = pAtR(rtt);
        double pbb = pAtR(rbb);
        double pmin = pbb;
        double pmax = ptt;
        if (ptt < pbb)
        {
          pmin = ptt;
          pmax = pbb;
        }

        // if p > pmax ray is invalid so just return 0 ... otherwise

        if (p < pmin)
        {
          //valid up/down

          vlRLast = rbb;
          vlIRt   = rtt;
          vlIRb   = rbb;
          vlPassingRay = true;
          vlInvalidRay = false;
        }
        else if (p < pmax)
        {
          //valid turning ray

          vlRTurn = rAtP(p);
          vlVTurn = operator()(vlRTurn);
          vlRLast = vlRTurn;
          vlIRt   = rtt;
          vlIRb   = vlRTurn;
          vlTurningRay = true;
          if (p == pmin) vlInvalidRay = false;
        }
      }
    };

    //! \brief The descriptive name of the velocity layer.
    string                 vlLayerName;

    //! \brief The phase type name ... either "P" or "S".
    string                 vlPhaseType;

    //! \brief The phase name of a ray that turns in the layer. This name
    //! is optional and used to define the phase unless overidden by
    //! vlPhaseUpper or vlPhaseLower for retrograde layers.
    string                 vlPhase;

    //! \brief The name of the phase for a ray that turns in the top
    //! half of a retro-grade layer. This name is optional and only used
    //! if assigned to a retrograde layer.
    string                 vlPhaseUpper;

    //! \brief The name of the phase for a ray that turns in the bottom
    //! half of a retro-grade layer. This name is optional and only used
    //! if assigned to a retrograde layer.
    string                 vlPhaseLower;

    //! \brief The name of the diffracted phase for a ray that bottoms in
    //! the layer and uses the layers bottom velocity to travel along the
    //! layer boundary. This name is optional and only required if the phase
    //! is to be supported.
    string                 vlPhaseIUpper;

    //! \brief The name of the diffracted phase of a ray that bottoms in
    //! the layer and uses the next layers top velocity to travel along the
    //! layer boundary. This name is optional and only required if the phase
    //! is to be supported.
    string                 vlPhaseILower;

    //! \brief An integer value that describes the layer type.
    //
    //! Valid types include:
    //!
    //!     0: A Turning Layer ... A ray can turn anywhere within the layer
    //!        between ray parameter values of vlPt to vlPb.
    //!     1: A Bottom Turning Layer ... A ray can turn in the bottom half of
    //!        the layer between vlPmin and vlPb but is shadowed (excluded) in
    //!        the upper part of the layer.
    //!     2: A Top Turning Layer ... A ray can turn in the top half of the
    //!        layer between vlPt and vlPmin but is shadowed (excluded) in the
    //!        lower part of the layer. And,
    //!     3: A Shadow Layer ... No valid ray turns anywhere within the layer.
    int                    vlLayerType;

    //! \brief Boolean flag that is true if the velocity layer is a retrograde
    //! layer.
    bool                   vlSplitLayer;

    //! \brief Boolean flag that is true if the last distance / time evaluation
    //! for this velocity layer was invalid.
    bool                   vlInvalidRay;

    //! \brief Boolean flag that is true if the last distance / time evaluation
    //! for this velocity layer was a passing ray.
    bool                   vlPassingRay;

    //! \brief Boolean flag that is true if the last distance / time evaluation
    //! for this velocity layer was a turning ray.
    bool                   vlTurningRay;

    //! \brief Set to true if the upper diffracted phase for this layer is
    //! to be evaluated.
    bool                   vlPhaseUpperIDef;

    //! \brief Set to true if the lower diffracted phase for this layer is
    //! to be evaluated.
    bool                   vlPhaseLowerIDef;

    //! The layer top radius.
    double                 vlRt;

    //! The layer bottom radius.
    double                 vlRb;

    //! The last integration top radius for this layer.
    double                 vlIRt;

    //! The last integration bottom radius for this layer.
    double                 vlIRb;

    //! \brief The layer top velocity.
    double                 vlVt;

    //! \brief The layer bottom velocity.
    double                 vlVb;

    //! \brief The layer top ray parameter (vlRt / vlVt).
    double                 vlPt;

    //! \brief The layer bottom ray parameter (vlRb / vlVb).
    double                 vlPb;

    // Turning radius or bottom of layer if this is a passing ray
    double                 vlRTurn;

    // Turning velocity or layer bottom velocity if this is a passing ray
    double                 vlVTurn;

    //! \brief Contains the last evaluated turning depth (or vlRb).
    double                 vlRLast;

    //! \brief The integrated distance to the top of the layer using the
    //! top ray parameter (vlPt or vlPmin if the layer is partially
    //! shadowed).
    double                 vlDistT;

    //! \brief The integrated distance to the bottom of the layer using the
    //! bottom ray parameter (vlPb or vlPmin if the layer is partially
    //! shadowed).
    double                 vlDistB;

    //! \brief The derivative of distance wrt. the ray parameter at the top
    //! of the layer.
    double                 vldDistdP_T;

    //! \brief The derivative of distance wrt. the ray parameter at the bottom
    //! of the layer.
    double                 vldDistdP_B;

    //! \brief The minimum allowed valid ray parameter that marks a passing
    //! limit in the layer. If vlPmin >= vlPt then all rays can penentrate
    //! the layer. If vlPMin < vlPb then the layer is a shadow layer and
    //! only passing rays (p < vlPb) can pass. If in between then vlPmin
    //! marks the boundary where a valid turning ray is allowed and a
    //! bounding shadow region exists above or below vlPmin.
    double                 vlPmin;

    //! \brief The ray parameter that gives the distance minimum of a
    //! retrograde layer. If the layer is not retrograde this value is
    //! ignored.
    double                 vlPCrit;

    //! \brief The distance to the minimum of the retrograde layer if the layer
    //! is retrograde. Otherwise ignored.
    double                 vlDistCrit;

  private:
};

// *****************************************************************************
//
//! The VelocityIntegrate<VL> template object is used to exclusively numerically
//! integrate the distance and time functions if analytic versions were
//! not provided in the superclass defintion (VL).
//
//! A template is used to avoid virtual pointer traversal during the numerical
//! integration process which can evaluate the integrand many hundreds of times
//! per integration. This object creates the numerical integrand templates
//!
//!     TPdDistdr<V>* vmDist;
//!     TPdTaudr<V>*  vmTau;
//!
//! which are used by the integration objects to integrate the spherically
//! defined distance as a function of the layer velocity and input ray
//! parameter. This object also creates the integration objects
//!
//!     IntegrateFunction<TPdDistdr<V> >* vmDistNI;
//!     IntegrateFunction<TPdTaudr<V> >*  vmTauNI;
//!
//! which are used to perform the numericaly integration. This object also
//! defines the numerical versions of the abstract functions
//!
//!     virtual double integrateDistance(double p, double r,
//!                                      bool r_open = false);
//!     virtual double integrateTime(double p, double r);
//!
//! Which can be overridden by any super class definitions if an analytic
//! solution exists.
//
// *****************************************************************************
template<class V>
class TAUP_EXP VelocityIntegrate : public TPVelocityLayer
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default constructor. Sets top radius to zero.
    VelocityIntegrate() : vmDist(NULL), vmTau(NULL),
                          vmDistNI(NULL), vmTauNI(NULL), TPVelocityLayer() {};

    //! Standard constructor. Sets top radius to rt.
    VelocityIntegrate(double rt, double rb, const string& layrnam) :
                      vmDist(NULL), vmTau(NULL),
                      vmDistNI(NULL), vmTauNI(NULL),
                      TPVelocityLayer(rt, rb, layrnam) {};

    //! Copy constructor.
    VelocityIntegrate(const VelocityIntegrate& vi) :
                      vmDist(NULL), vmTau(NULL),
                      vmDistNI(NULL), vmTauNI(NULL), TPVelocityLayer(vi) {};

    //! Destructor.
    virtual ~VelocityIntegrate()
    {
      // Delete integrand and integration function objects if they were
      // defined.

      if (vmDist)
      {
        delete vmDist;
        delete vmTau;
        delete vmDistNI;
        delete vmTauNI;
      }
    };

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment Operator.
    VelocityIntegrate&     operator=(const VelocityIntegrate& vi)
    {
      // delete old definitions and create new ones if they were defined

      if (vmDist)
      {
        delete vmDist;
        vmDist = (TPdDistdr<V>*) NULL;
        delete vmTau;
        vmTau  = (TPdTaudr<V>*) NULL;
        delete vmDistNI;
        vmDistNI = (util::IntegrateFunction<TPdDistdr<V> >*) NULL;
        delete vmTauNI;
        vmTauNI  = (util::IntegrateFunction<TPdTaudr<V> >*) NULL;
      }

      TPVelocityLayer::operator=(vi);

      return *this;
    };

    // **** _PUBLIC METHODS_ ***************************************************

    //! \brief The distance integration function that calculates the ray
    //! travel distance from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function
    //! performs the integration numerically using the integration object
    //! vmDistNI and the distance integrand object vmDist.
    //
    //! If the flag \em r_open is true then the open limit integration function
    //! IntegrateFunction::integrateAOpenS(...) is called. Otherwise the closed
    //! form function, IntegrateFunction::integrateClosed(...), is used.
    virtual double         integrateDistance(double p, double ra, double rb,
                                             bool r_open = false)
    {
      // create the numerical integration objects if they do not yet exist

      if (!vmDist) createNumericObjects(*((V*) this));

      // set p and integrate distance

      vmDist->setP(p);
      if (r_open)
        return vmDistNI->integrateAOpenS(ra, rb);
      else
        return vmDistNI->integrateClosed(ra, rb);
    };

    //! \brief The distance integration function that calculates the ray
    //! travel distance from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function
    //! performs the integration numerically using the integration object
    //! vmDistNI and the distance integrand object vmDist.
    //
    //! If the flag \em r_open is true then the open limit integration function
    //! IntegrateFunction::integrateAOpenS(...) is called. Otherwise the closed
    //! form function, IntegrateFunction::integrateClosed(...), is used.
    virtual double         integrateDistance(double p, double ra,
                                             bool r_open = false)
    {
      return integrateDistance(p, ra, vlRt, r_open);
    };

    //! \brief The time (or tau) integration function that calculates the ray
    //! travel time from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function
    //! performs the integration numerically using the integration object
    //! vmTauNI and the tau integrand object vmTau.
    virtual double         integrateTime(double p, double ra, double rb)
    {
      // create the numerical integration objects if they do not yet exist

      if (!vmTau) createNumericObjects(*((V*) this));

      // set p and integrate time

      vmTau->setP(p);
      if (ra == 0.0)
        return vmTauNI->integrateAOpenS(ra, rb);
      else
        return vmTauNI->integrateClosed(ra, rb);
    };

    //! \brief The time (or tau) integration function that calculates the ray
    //! travel time from the start of the layer boundary (vlRt) to
    //! depth \em r given the input ray parameter \em p. This function
    //! performs the integration numerically using the integration object
    //! vmTauNI and the tau integrand object vmTau.
    virtual double         integrateTime(double p, double ra)
    {
      return integrateTime(p, ra, vlRt);
    };

    //! \brief Sets the integration tolerance limit for this velocity layer.
    //! If the integration objects are not yet instantiated this function
    //! will create them before setting their tolerance
    void                   setIntegTolerance(double tol)
    {
      // create the numerical integration objects if they do not yet exist

      if (!vmDist) createNumericObjects(*((V*) this));

      // set tolerance

      vmDistNI->setTolerance(tol);
      vmTauNI->setTolerance(tol);
    };

  protected:

    //! \brief Used to create the distance and tau integrand functions vmDist,
    //! and vmTau, and their associated integration objects vmDistNI and
    //! vmTauNI.
    void                   createNumericObjects(V& v);

    //! \brief The default integration tolerance
    static const double    vmIntegTol;

    //! \brief The distance integrand object templated on the velocity.
    TPdDistdr<V>*          vmDist;

    //! \brief The tau integrand object templated on the velocity.
    TPdTaudr<V>*           vmTau;

    //! \brief The numerical integration object templated on vmDist.
    util::IntegrateFunction<TPdDistdr<V> >* vmDistNI;

    //! \brief The numerical integration object templated on vmTau.
    util::IntegrateFunction<TPdTaudr<V> >*  vmTauNI;
};

// *****************************************************************************
//
//! \brief Constant velocity model returns a single value for velocity
//! irregardless of the input radius value. This object overrides the
//! numerical distance and time integration with analytic definitions.
//
// *****************************************************************************
class TAUP_EXP VelocityConst : public VelocityIntegrate<VelocityConst>
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default constructor. Sets velocity to zero.
    VelocityConst() : vc(0.0), VelocityIntegrate<VelocityConst>() {};

    //! Standard constructor. Sets velocity to \em c.
    VelocityConst(double c, double rt, double rb, const string& layrnam = "") :
                  vc(c), VelocityIntegrate<VelocityConst>(rt, rb, layrnam) {init();};

    //! Copy constructor.
    VelocityConst(const VelocityConst& vcst) : vc(vcst.vc),
                  VelocityIntegrate<VelocityConst>(vcst) {};

    //! DataBuffer constructor.
    VelocityConst(DataBuffer& buffer) {deserialize(buffer);};

    //! Destructor.
    virtual ~VelocityConst() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment Operator.
    VelocityConst&         operator=(const VelocityConst& vcst)
    {
      VelocityIntegrate<VelocityConst>::operator=(vcst);

      vc = vcst.vc;
      return *this;
    };

    //! Standard operator() overload which returns the velocity at radius
    //! \em r. The radius is defined from the center of the Earth. For a
    //! constant velocity the same value is returned regardless of \em r.
    virtual double         operator()(double r) {return vc;};

    // **** _PUBLIC METHODS_ ***************************************************

    //! Standard function that returns the radius as function of the turning
    //! ray parameter \em p. The turning ray parameter is defined as
    //! \em p = r / v(r) where v(r) is the radial velocity
    virtual double         rAtP(double p) {return p * vc;};

    //! \brief The analytic constant velocity distance integration function
    //! that calculates the ray travel distance from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version and ignores the input
    //! parameter r_open.
    virtual double         integrateDistance(double p, double ra,
                                             bool r_open = false)
    {
      return integrateDistance(p, ra, vlRt, r_open);
    };

    //! \brief The analytic constant velocity distance integration function
    //! that calculates the ray travel distance from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version and ignores the input
    //! parameter r_open.
    virtual double         integrateDistance(double p, double ra, double rb,
                                             bool r_open = false)
    {
      if (vc == 0.0) return 0.0;
      double pv = p * vc;

      double prt = pv / rb;
      double prb = pv / ra;

      return asin(min(prb, 1.0)) - asin(min(prt, 1.0));
    };

    //! \brief The analytic constant velocity time integration function
    //! that calculates the ray travel time from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version.
    virtual double         integrateTime(double p, double ra)
    {
      return integrateTime(p, ra, vlRt);
    };

    //! \brief The analytic constant velocity time integration function
    //! that calculates the ray travel time from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version.
    virtual double         integrateTime(double p, double ra, double rb)
    {
      if (vc == 0.0) return 0.0;
      double pt = rb / vc;
      double pr = ra / vc;
      return sqrt(fabs(pt - p) * (pt + p)) - sqrt(fabs(pr - p) * (pr + p));
    };

    //! The constant velocity time integral returns time and not tau.
    virtual bool           isTimeIntegralTau() const {return false;};

    //! \brief Writes the velocity definition to the input stream \em os.
    virtual void           writeVelocity(ostream& os) const;

    //! Print object data to input stream \em os.
    virtual void           toStream(ostream& os, string indent) const;

    //! Static function that returns the class name.
    static  string         class_name() {return "VelocityConst";};

    //! Virtual function that returns the class name.
    virtual string         get_class_name() const {return class_name();};

    //! Returns the class size.
    virtual int            classSize() const
    {return (int) sizeof(VelocityConst);};

    //! Virtual function that returns true for a Constant model.
    virtual bool           isVelocityConstant() const {return true;};

    //! \brief Returns the size of a DataBuffer required to contain all
    //! serialized information for a VelocityConst object.
    virtual int            bufferSize() const
    {
      return 2 * sizeof(double) + TPVelocityLayer::bufferSize();
    };

    //! \brief Writes the content of the VelocityConst object into the
    //! input DataBuffer \em buffer.
    virtual void           serialize(DataBuffer& buffer)
    {
      buffer.writeDouble(vc);
      TPVelocityLayer::serialize(buffer);
    };

    //! \brief Reads the content of the VelocityConst object from the
    //! input DataBuffer \em buffer.
    virtual void           deserialize(DataBuffer& buffer)
    {
      vc = buffer.readDouble();
      TPVelocityLayer::deserialize(buffer);
    };

  private:

    // **** _PRIVATE DATA_ *****************************************************

    //! The constant velocity value.
    double                 vc;
};

// *****************************************************************************
//
//! \brief Power Law velocity model returns a power law modeled velocity
//! between the top and bottom of the layer as a function of the radius.
//! This object overrides the numerical distance and time integration with
//! analytic definitions.
//
// *****************************************************************************
class TAUP_EXP VelocityPower : public VelocityIntegrate<VelocityPower>
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default constructor. Sets velocity to zero.
    VelocityPower() : vp0(0.0), vp1(0.0), vpB(0.0),
                      VelocityIntegrate<VelocityPower>() {};

    //! Standard constructor.
    VelocityPower(double vt, double vb, double rt, double rb,
                  const string& layrnam = "") :
                  vp0(vt), vp1(vb),
                  VelocityIntegrate<VelocityPower>(rt, rb, layrnam)
    {
      vpB = log(vb / vt) / log(vlRb / vlRt);
      vp1_B = 1.0 - vpB;
      init();
    };

    //! Copy constructor.
    VelocityPower(const VelocityPower& vp) :
                  vp0(vp.vp0), vp1(vp.vp1), vp1_B(vp.vp1_B),
                  vpB(vp.vpB), VelocityIntegrate<VelocityPower>(vp) {};

    //! DataBuffer constructor.
    VelocityPower(DataBuffer& buffer) {deserialize(buffer);};

    //! Destructor.
    virtual ~VelocityPower() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment operator.
    VelocityPower& operator=(const VelocityPower& vp)
    {
      VelocityIntegrate<VelocityPower>::operator=(vp);

      vp0   = vp.vp0;
      vp1   = vp.vp1;
      vpB   = vp.vpB;
      vp1_B = vp.vp1_B;

      return *this;
    };

    //! Standard operator() overload which returns the velocity at radius
    //! \em r. The radius is defined from the center of the Earth.
    virtual double         operator()(double r)
    {
      return vp0 * pow(r / vlRt, vpB);
    };

    // **** _PUBLIC METHODS_ ***************************************************

    //! Standard function that returns the radius as function of the turning
    //! ray parameter \em p. The turning ray parameter is defined as
    //! \em p = r / v(r) where v(r) is the radial velocity
    virtual double         rAtP(double p)
    {
      return pow(p * vp0 * pow(1.0 / vlRt, vpB), 1.0 / vp1_B);
    };

    //! Returns the power law exponent
    double                 B() const {return vpB;};

    //! \brief The analytic power law velocity distance integration function
    //! that calculates the ray travel distance from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version and ignores the input
    //! parameter r_open.
    virtual double         integrateDistance(double p, double ra,
                                             bool r_open = false)
    {
      double pva = p * operator()(ra);
      double pvb = p * vlVt;
      return (asin(min(pva / ra, 1.0)) - asin(min(pvb / vlRt, 1.0))) / vp1_B;
    };

    //! \brief The analytic power law velocity distance integration function
    //! that calculates the ray travel distance from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version and ignores the input
    //! parameter r_open.
    virtual double         integrateDistance(double p, double ra, double rb,
                                             bool r_open = false)
    {
      double pva = p * operator()(ra);
      double pvb = p * operator()(rb);
      return (asin(min(pva / ra, 1.0)) - asin(min(pvb / rb, 1.0))) / vp1_B;
    };

    //! \brief The analytic power law velocity time integration function
    //! that calculates the ray travel time from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version.
    virtual double         integrateTime(double p, double ra)
    {
      double prb = vlRt / vlVt;
      double pra = ra / operator()(ra);
      return (sqrt(fabs(prb - p) * (prb + p)) -
              sqrt(fabs(pra - p) * (pra + p))) / vp1_B;
    };

    //! \brief The analytic power law velocity time integration function
    //! that calculates the ray travel time from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version.
    virtual double         integrateTime(double p, double ra, double rb)
    {
      double pra = ra / operator()(ra);
      double prb = rb / operator()(rb);
      //double p2 = p * p;
      return (sqrt(fabs(pra - p) * (pra + p)) -
              sqrt(fabs(prb - p) * (prb + p))) / vp1_B;
    };

    //! The power law velocity time integral returns time and not tau.
    virtual bool           isTimeIntegralTau() const {return false;};

    //! \brief Writes the velocity definition to the input stream \em os.
    virtual void           writeVelocity(ostream& os) const;

    //! Print object data to input stream \em os.
    virtual void           toStream(ostream& os, string indent) const;

    //! Static function that returns the class name.
    static  string         class_name() {return "VelocityPower";};

    //! Virtual function that returns the class name.
    virtual string         get_class_name() const {return class_name();};

    //! Returns the class size.
    virtual int            classSize() const
    {return (int) sizeof(VelocityPower);};

    //! Virtual function that returns true for a Power Law model.
    virtual bool           isVelocityPowerLaw() const {return true;};

    //! \brief Returns the size of a DataBuffer required to contain all
    //! serialized information for a VelocityPower object.
    virtual int            bufferSize() const
    {
      return 5 * sizeof(double) + TPVelocityLayer::bufferSize();
    };

    //! \brief Writes the content of the VelocityPower object into the
    //! input DataBuffer \em buffer.
    virtual void           serialize(DataBuffer& buffer)
    {
      buffer.writeDouble(vp0);
      buffer.writeRawDouble(vp1);
      buffer.writeRawDouble(vpB);
      buffer.writeRawDouble(vp1_B);
      TPVelocityLayer::serialize(buffer);
    };

    //! \brief Reads the content of the VelocityPower object from the
    //! input DataBuffer \em buffer.
    virtual void           deserialize(DataBuffer& buffer)
    {
      vp0 = buffer.readDouble();
      vp1 = buffer.readRawDouble();
      vpB = buffer.readRawDouble();
      vp1_B = buffer.readRawDouble();
      TPVelocityLayer::deserialize(buffer);
    };

  private:

    // **** _PRIVATE DATA_ *****************************************************

    //! \brief The velocity value at the top of the layer for which this
    //! velocity model was defined
    double                 vp0;

    //! \brief The velocity value at the bottom of the layer for which this
    //! velocity model was defined
    double                 vp1;

    //! \brief The power law exponent for this velocity model.
    double                 vpB;

    //! \brief One minus the velocity model power law exponent.
    double                 vp1_B;
};

// *****************************************************************************
//
//! \brief Linear velocity model returns a linearly modeled velocity
//! between the top and bottom of the layer as a function of the radius.
//! This object overrides the numerical distance integration with an
//! analytic definition.
//
// *****************************************************************************
class TAUP_EXP VelocityLinear : public VelocityIntegrate<VelocityLinear>
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default constructor. Sets velocity to zero.
    VelocityLinear() : va0(0.0), va1(0.0), VelocityIntegrate<VelocityLinear>(){};

    //! Standard constructor.
    VelocityLinear(double a0, double a1, double rt, double rb,
                   const string& layrnam = "", double normradius = 1.0) :
                   va0(a0), va1(a1), vNormRadius(normradius),
                   VelocityIntegrate<VelocityLinear>(rt, rb, layrnam) {init();};

    //! Copy constructor.
    VelocityLinear(const VelocityLinear& vl) : va0(vl.va0), va1(vl.va1),
                   vNormRadius(vl.vNormRadius), VelocityIntegrate<VelocityLinear>(vl) {};

    //! DataBuffer constructor.
    VelocityLinear(DataBuffer& buffer) {deserialize(buffer);};

    //! Destructor.
    virtual ~VelocityLinear() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment operator.
    VelocityLinear&        operator=(const VelocityLinear& vl)
    {
      VelocityIntegrate<VelocityLinear>::operator=(vl);

      va0 = vl.va0;
      va1 = vl.va1;
      vNormRadius = vl.vNormRadius;

      return *this;
    };

    //! Standard operator() overload which returns the velocity at radius
    //! \em r. The radius is defined from the center of the Earth.
    virtual double         operator()(double r)
    {
      double rn = r / vNormRadius;
      return va1 * rn + va0;
    };

    // **** _PUBLIC METHODS_ ***************************************************

    //! Standard function that returns the radius as function of the turning
    //! ray parameter \em p. The turning ray parameter is defined as
    //! \em p = r / v(r) where v(r) is the radial velocity
    virtual double         rAtP(double p)
    {
      return (p * va0) / (1 - p * va1 / vNormRadius);
    };

    //! \brief The analytic linear velocity distance integration function
    //! that calculates the ray travel distance from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version and ignores the input
    //! parameter r_open.
    virtual double         integrateDistance(double p, double r,
                                             bool r_open = false)
    {
      double v1, pv, pv1, pv0, rslt, a, b, c;

      // first set result to analytic solution from the constant coefficient
      // component

      v1 = va1 / vNormRadius;
      pv = p * (va0 + v1 * r) / r;
      if (pv >= 1.0)
        rslt = PI / 2.0;
      else
        rslt = asin(pv);
      pv = p * vlVt / vlRt;
      rslt -= asin(pv);

      // next add to the result the analytic solution from the gradient
      // component ... note: that the additive component depends on the
      // value of c

      pv0 = p * va0;
      pv1 = p * v1;
      pv = pv1 * pv1;
      c = 1.0 - pv;
      if (c < 0.0)
      {
        b = - pv0 * pv1;
        double arg = (c * r + b) / pv0;
        if (arg >= 1.0)
          rslt += pv1 * (PI / 2.0  - asin((c * vlRt + b) / pv0)) / sqrt(-c);
        else
          rslt += pv1 * (asin(arg) - asin((c * vlRt + b) / pv0)) / sqrt(-c);
      }
      else if (c == 0.0)
      {
        rslt += sqrt(-2.0 * v1 * r / va0 - 1.0) -
                sqrt(-2.0 * v1 * vlRt / va0 - 1.0);
      }
      else // (c > 0.0)
      {
        b = - 2.0 * pv0 * pv1;
        a = - pv0 * pv0;

        double sc  = sqrt(c);
        double Rrt = a + vlRt * (b + c * vlRt);
        double Rr  = a + r * (b + c * r);
        rslt += pv1 * (log(2.0 * sc * sqrt(fabs(Rrt)) + 2.0 * c * vlRt + b) -
                       log(2.0 * sc * sqrt(fabs(Rr))  + 2.0 * c * r + b)) / sc;
      }

      // return the result

      return rslt;
    };

    //! \brief The analytic linear velocity distance integration function
    //! that calculates the ray travel distance from the start of the layer
    //! boundary (vlRt) to depth \em r given the input ray parameter \em p.
    //
    //! This function overrides the numerical version and ignores the input
    //! parameter r_open.
    virtual double         integrateDistance(double p, double ra, double rb,
                                             bool r_open = false)
    {
      double v1, pv, pv1, pv0, rslt, a, b, c;

      // first set result to analytic solution from the constant coefficient
      // component

      v1 = va1 / vNormRadius;
      pv = p * (va0 + v1 * ra) / ra;
      if (pv >= 1.0)
        rslt = PI / 2.0;
      else
        rslt = asin(pv);

      pv = p * (va0 + v1 * rb) / rb;
      if (pv >= 1.0)
        rslt -= PI / 2.0;
      else
        rslt -= asin(pv);

      // next add to the result the analytic solution from the gradient
      // component ... note: that the additive component depends on the
      // value of c

      pv0 = p * va0;
      pv1 = p * v1;
      pv = pv1 * pv1;
      c = 1.0 - pv;
      if (c < 0.0)
      {
        b = - pv0 * pv1;
        double arg = (c * ra + b) / pv0;
        if (arg >= 1.0)
          rslt += pv1 * (PI / 2.0  - asin((c * rb + b) / pv0)) / sqrt(-c);
        else
          rslt += pv1 * (asin(arg) - asin((c * rb + b) / pv0)) / sqrt(-c);
      }
      else if (c == 0.0)
      {
        rslt += sqrt(-2.0 * v1 * ra / va0 - 1.0) -
                sqrt(-2.0 * v1 * rb / va0 - 1.0);
      }
      else // (c > 0.0)
      {
        b = - 2.0 * pv0 * pv1;
        a = - pv0 * pv0;

        double sc  = sqrt(c);
        double Rrb = a + rb * (b + c * rb);
        double Rra = a + ra * (b + c * ra);
        rslt += pv1 * (log(2.0 * sc * sqrt(fabs(Rrb)) + 2.0 * c * rb + b) -
                       log(2.0 * sc * sqrt(fabs(Rra)) + 2.0 * c * ra + b)) / sc;
      }

      // return the result

      return rslt;
    };

    //! \brief Writes the normalized radius definition to input stream \em os.
    virtual void           writeNormRadius(ostream& os) const;

    //! \brief Writes the velocity definition to the input stream \em os.
    virtual void           writeVelocity(ostream& os) const;

    //! Print object data to input stream \em os.
    virtual void           toStream(ostream& os, string indent) const;

    //! Static function that returns the class name.
    static  string         class_name() {return "VelocityLinear";};

    //! Virtual function that returns the class name.
    virtual string         get_class_name() const {return class_name();};

    //! Returns the class size.
    virtual int            classSize() const
    {return (int) sizeof(VelocityLinear);};

    //! Virtual function that returns true for a Linear model.
    virtual bool           isVelocityLinear() const {return true;};

    //! \brief Returns the size of a DataBuffer required to contain all
    //! serialized information for a VelocityLinear object.
    virtual int            bufferSize() const
    {
      return 4 * sizeof(double) + TPVelocityLayer::bufferSize();
    };

    //! \brief Writes the content of the VelocityLinear object into the
    //! input DataBuffer \em buffer.
    virtual void           serialize(DataBuffer& buffer)
    {
      buffer.writeDouble(va0);
      buffer.writeRawDouble(va1);
      buffer.writeRawDouble(vNormRadius);
      TPVelocityLayer::serialize(buffer);
    };

    //! \brief Reads the content of the VelocityLinear object from the
    //! input DataBuffer \em buffer.
    virtual void           deserialize(DataBuffer& buffer)
    {
      va0 = buffer.readDouble();
      va1 = buffer.readRawDouble();
      vNormRadius = buffer.readRawDouble();
      TPVelocityLayer::deserialize(buffer);
    };
  private:

    // **** _PRIVATE DATA_ *****************************************************

    //! \brief The normalized radius parameter used to scale the polynomial
    //! coefficients (Defaults to 1.0).
    double                 vNormRadius;

    //! \brief The velocity at the top of the layer.
    double                 va0;

    //! \brief The linear velocity coefficient (the gradient) for a linear
    //! velocity model.
    double                 va1;
};

// *****************************************************************************
//
//! \brief Quadratic velocity model returns a quadraticly modeled velocity
//! between the top and bottom of the layer as a function of the radius.
//
// *****************************************************************************
class TAUP_EXP VelocityQuadratic : public VelocityIntegrate<VelocityQuadratic>
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default constructor. Sets velocity to zero.
    VelocityQuadratic() : va0(0.0), va1(0.0), va2(0.0),
                          VelocityIntegrate<VelocityQuadratic>() {};

    //! Standard constructor.
    VelocityQuadratic(double a0, double a1, double a2, double rt, double rb,
                      const string& layrnam = "", double normradius = 1.0) :
                      va0(a0), va1(a1), va2(a2), vNormRadius(normradius),
                      VelocityIntegrate<VelocityQuadratic>(rt, rb, layrnam) {init();};

    //! Copy constructor.
    VelocityQuadratic(const VelocityQuadratic& vq) :
                      va0(vq.va0), va1(vq.va1), va2(vq.va2),
                      vNormRadius(vq.vNormRadius), VelocityIntegrate<VelocityQuadratic>(vq) {};

    //! DataBuffer constructor.
    VelocityQuadratic(DataBuffer& buffer) {deserialize(buffer);};

    //! Destructor.
    virtual ~VelocityQuadratic() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment operator.
    VelocityQuadratic&     operator=(const VelocityQuadratic& vq)
    {
      VelocityIntegrate<VelocityQuadratic>::operator=(vq);

      va0 = vq.va0;
      va1 = vq.va1;
      va2 = vq.va2;
      vNormRadius = vq.vNormRadius;

      return *this;
    };

    //! Standard operator() overload which returns the velocity at radius
    //! \em r. The radius is defined from the center of the Earth.
    virtual double         operator()(double r)
    {
      double rn = r / vNormRadius;
      return va0 + rn * (va1 + rn * va2);
    };

    // **** _PUBLIC METHODS_ ***************************************************

    //! Standard function that returns the radius as function of the turning
    //! ray parameter \em p. The turning ray parameter is defined as
    //! \em p = r / v(r) where v(r) is the radial velocity
    virtual double         rAtP(double p)
    {
      // return 0 if p = 0

      if (p == 0.0) return 0.0;

      // calculate quadratic coefficient (a +- sqrt(a*a - c)) / b
      // calculate quadratic solution from ar^2 + br + c = 0 where

      double a = p * va2 / vNormRadius / vNormRadius;
      double b = p * va1 / vNormRadius - 1.0;
      double c = p * va0;

      // solution is r = (-b +- sqrt(b^2 - 4ac)) / 2a
      // first check for error

      double sqarg = b * b - 4.0 * a * c;
      if (sqarg < 0.0)
      {
        // error: p is not a valid ray parameter to calculate r
        return 0.0;
      }
      else if (sqarg == 0.0)
      {
        // one root ... return result

        return -b / 2.0 / a;
      }
      else
      {
        // two roots ... get both parts

        double cc = 2.0 * a;
        double aa = -b / cc;
        double sq = sqrt(sqarg) / cc;
        double r1 = aa - sq;
        if ((r1 <= vlRt) && (r1 >= vlRb))
          return r1;
        else
          return aa + sq;
      }
    };

    //! \brief Writes the normalized radius definition to input stream \em os.
    virtual void           writeNormRadius(ostream& os) const;

    //! \brief Writes the velocity definition to the input stream \em os.
    virtual void           writeVelocity(ostream& os) const;

    //! Print object data to input stream \em os.
    virtual void           toStream(ostream& os, string indent) const;

    //! Static function that returns the class name.
    static  string         class_name() {return "VelocityQuadratic";};

    //! Virtual function that returns the class name.
    virtual string         get_class_name() const {return class_name();};

    //! Returns the class size.
    virtual int            classSize() const
    {return (int) sizeof(VelocityQuadratic);};

    //! Virtual function that returns true for a Quadratic model.
    virtual bool           isVelocityQuadratic() const {return true;};

    //! \brief Returns the size of a DataBuffer required to contain all
    //! serialized information for a VelocityQuadratic object.
    virtual int            bufferSize() const
    {
      return 5 * sizeof(double) + TPVelocityLayer::bufferSize();
    };

    //! \brief Writes the content of the VelocityQuadratic object into the
    //! input DataBuffer \em buffer.
    virtual void           serialize(DataBuffer& buffer)
    {
      buffer.writeDouble(va0);
      buffer.writeRawDouble(va1);
      buffer.writeRawDouble(va2);
      buffer.writeRawDouble(vNormRadius);
      TPVelocityLayer::serialize(buffer);
    };

    //! \brief Reads the content of the VelocityQuadratic object from the
    //! input DataBuffer \em buffer.
    virtual void           deserialize(DataBuffer& buffer)
    {
      va0 = buffer.readDouble();
      va1 = buffer.readRawDouble();
      va2 = buffer.readRawDouble();
      vNormRadius = buffer.readRawDouble();
      TPVelocityLayer::deserialize(buffer);
    };

  private:

    // **** _PRIVATE DATA_ *****************************************************

    //! \brief The normalized radius parameter used to scale the polynomial
    //! coefficients (Defaults to 1.0).
    double                 vNormRadius;

    //! \brief The velocity at the top of the layer.
    double                 va0;

    //! \brief The linear velocity coefficient for a quadratic
    //! velocity model.
    double                 va1;

    //! \brief The quadratic velocity coefficient for a quadratic
    //! velocity model.
    double                 va2;
};

// *****************************************************************************
//
//! \brief Cubic velocity model returns a cubicly modeled velocity
//! between the top and bottom of the layer as a function of the radius.
//
// *****************************************************************************
class TAUP_EXP VelocityCubic : public VelocityIntegrate<VelocityCubic>
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default constructor. Sets velocity to zero.
    VelocityCubic() : va0(0.0), va1(0.0), va2(0.0), va3(0.0), vNormRadius(1.0),
                      VelocityIntegrate<VelocityCubic>() {};

    //! Standard constructor.
    VelocityCubic(double a0, double a1, double a2, double a3,
                  double rt, double rb, const string& layrnam = "",
                  double normradius = 1.0) :
                  va0(a0), va1(a1), va2(a2), va3(a3), vNormRadius(normradius),
                  VelocityIntegrate<VelocityCubic>(rt, rb, layrnam) {init();};

    //! Copy constructor.
    VelocityCubic(const VelocityCubic& vc) :
                  va0(vc.va0), va1(vc.va1), va2(vc.va2), va3(vc.va3),
                  vNormRadius(vc.vNormRadius), VelocityIntegrate<VelocityCubic>(vc) {};

    //! DataBuffer constructor.
    VelocityCubic(DataBuffer& buffer) {deserialize(buffer);};

    //! Destructor.
    virtual ~VelocityCubic() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment operator.
    VelocityCubic&         operator=(const VelocityCubic& vc)
    {
      VelocityIntegrate<VelocityCubic>::operator=(vc);

      va0 = vc.va0;
      va1 = vc.va1;
      va2 = vc.va2;
      va3 = vc.va3;
      vNormRadius = vc.vNormRadius;

      return *this;
    };

    //! Standard operator() overload which returns the velocity at radius
    //! \em r. The radius is defined from the center of the Earth.
    virtual double         operator()(double r)
    {
      double rn = r / vNormRadius;
      return va0 + rn * (va1 + rn * (va2 + rn * va3));
    };

    // **** _PUBLIC METHODS_ ***************************************************

    //! Standard function that returns the radius as function of the turning
    //! ray parameter \em p. The turning ray parameter is defined as
    //! \em p = r / v(r) where v(r) is the radial velocity
    virtual double         rAtP(double p);

    //! \brief Writes the normalized radius definition to input stream \em os.
    virtual void           writeNormRadius(ostream& os) const;

    //! \brief Writes the velocity definition to the input stream \em os.
    virtual void           writeVelocity(ostream& os) const;

    //! Print object data to input stream \em os.
    virtual void           toStream(ostream& os, string indent) const;

    //! Static function that returns the class name.
    static  string         class_name() {return "VelocityCubic";};

    //! Virtual function that returns the class name.
    virtual string         get_class_name() const {return class_name();};

    //! Returns the class size.
    virtual int            classSize() const
    {return (int) sizeof(VelocityCubic);};

    //! Virtual function that returns true for a Cubic model.
    virtual bool           isVelocityCubic() const {return true;};

    //! \brief Returns the size of a DataBuffer required to contain all
    //! serialized information for a VelocityCubic object.
    virtual int            bufferSize() const
    {
      return 6 * sizeof(double) + TPVelocityLayer::bufferSize();
    };

    //! \brief Writes the content of the VelocityCubic object into the
    //! input DataBuffer \em buffer.
    virtual void           serialize(DataBuffer& buffer)
    {
      buffer.writeDouble(va0);
      buffer.writeRawDouble(va1);
      buffer.writeRawDouble(va2);
      buffer.writeRawDouble(va3);
      buffer.writeRawDouble(vNormRadius);
      TPVelocityLayer::serialize(buffer);
    };

    //! \brief Reads the content of the VelocityCubic object from the
    //! input DataBuffer \em buffer.
    virtual void           deserialize(DataBuffer& buffer)
    {
      va0 = buffer.readDouble();
      va1 = buffer.readRawDouble();
      va2 = buffer.readRawDouble();
      va3 = buffer.readRawDouble();
      vNormRadius = buffer.readRawDouble();
      TPVelocityLayer::deserialize(buffer);
    };

  private:

    // **** _PRIVATE DATA_ *****************************************************

    //! \brief The normalized radius parameter used to scale the polynomial
    //! coefficients (Defaults to 1.0).
    double                 vNormRadius;

    //! \brief The velocity at the top of the layer.
    double                 va0;

    //! \brief The linear velocity coefficient for a cubic
    //! velocity model.
    double                 va1;

    //! \brief The quadratic velocity coefficient for a cubic
    //! velocity model.
    double                 va2;

    //! \brief The cubic velocity coefficient for a cubic
    //! velocity model.
    double                 va3;
};

// *****************************************************************************
//
//! \brief A function object used to zero-in on the velocity at r that gives
//! the input ray parameter (vzP). The operator() returns r - vzP * vzV(r)
//! where r is input and vzP is set and vzV(r) is the velocity function. This
//! function object is used when r cannot be solved for explicitly.
//
// *****************************************************************************
template<class V>
class VZero
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Standard constructor.
    //
    //! Sets the velocity model, \em v, and the ray parameter, \em p, that will
    //! be used by a Brents function to zero-in on r.
    VZero(double p, V& v) : vzP(p), vzV(v) {};

    //! Copy constructor.
    VZero(const VZero& vz) : vzP(vz.vzP), vzV(vz.vzV) {};

    //! Destructor.
    virtual ~VZero() {};

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment operator.
    VZero&                 operator=(const VZero& vz)
    {
      vzP = vz.vzP;
      return *this;
    };

    //! Standard operator() overload which returns the zero of r - p * vel(r).
    //! This function is used to zero-in on a value for the radius (r) given
    //! a ray parameter and a velocity model.
    double                 operator()(double r) {return r - vzP * vzV(r);};

  private:

    //! \brief A ray parameter that will be used to zero-in on r in the
    //! operator() function.
    double                 vzP;

    //! \brief The velocity model used to zeroin on r.
    V&                     vzV;
};

} // end namespace taup

#endif // TPVelocityModels.h
