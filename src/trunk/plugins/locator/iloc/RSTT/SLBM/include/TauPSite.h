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
//- Module:        $RCSfile: TauPSite.h,v $
//- Creator:       Jim Hipp
//- Creation Date: April 23, 2007
//- Revision:      $Revision: 1.11 $
//- Last Modified: $Date: 2011/09/30 18:08:40 $
//- Last Check-in: $Author: avencar $
//-
//- ****************************************************************************

#ifndef TAUPSITE_H
#define TAUPSITE_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <map>
#include <set>

using namespace std;
// use standard library objects

// **** _LOCAL INCLUDES_ *******************************************************

#include "TauPGlobals.h"

#include "Brents.h"
#include "TauPSiteFunctionals.h"

// **** _BEGIN TAUP NAMESPACE_ *************************************************

namespace taup {

// **** _FORWARD REFERENCES_ ***************************************************

class TPVelocityLayer;
class TravelTimeResult;

// **** _CLASS CONSTANTS_ ******************************************************


// *****************************************************************************
//
//! The TauPSite object is used to represent a specific receiver location for
//! the TauP 1D spherical travel time calculation. The objects primary function
//! is calculateTravelTimes(double srcdist, double srcdepth) which determines
//! all viable ray path solutions at the input source distance / depth position
//! to the 1D spherical travel time equations as defined in:
//!
//!   Lay, Thorne; Wallace, T. C.; "Modern Global Seismology", Volume 58,
//!       International Geophysics Series, Academic Press, 1995, pp 91-93.
//!
//
//! The object contains functionality to assign or create a series of layered
//! velocity models that define the planetary velocity structure to be modeled.
//! Functionality is also provided to set the receiver depth. Both the receiver
//! and source depths can be negative (elevated) which uses the top layer
//! velocity structure as an infinite propagation medium using an inverted
//! radius definition.
//!
//! All valid branches are evaluated for a single input distance / depth. These
//! are stored in a map that associates the travel time for the branch with an
//! internal descriptive object (a TravelTimeResult) that describes the path in
//! specific detail. The entries in the map are stored in ascending order
//! (fastest to slowest). The Model also solves for all requested diffracted
//! phases (e.g. Pdiff) if a particular velocity layers upper or lower
//! diffracted phase interface flag is set.
//!
//! Individual layers can be modeled as one of the following variations:
//!
//!    Constant
//!    Power Law
//!    Linear
//!    Quadratic
//!    Cubic
//!
//! Each layer can be given a standard phase name or can default to the internal
//! evaluated phase name of the form Tn[b,i][+,-] where T can be "P" or "S"; n
//! is the turning layer index between 0 and N-1 where N is the number of
//! layers; either b or i is appended for a body wave or diffracted interface;
//! and either + or - can be appended whose meaning depends on if b or i was
//! used in the naming. If b (a body wave) is defined and the turning layer is
//! a retro-grade layer (i.e. contains a minimum in path distance wrt. the ray
//! parameter such as the outer core for example) then a "+" will be appended
//! if the ray turned in the upper half of the retro-grade layer above the
//! minimum, or a "-" will be appended if the ray turned in the lower half of
//! the layer below the minimum. If "i" (a diffracted ray) is defined and the
//! ray uses the upper layer velocity to propagate then a "+" will be appended,
//! else if the ray uses the lower layer velocity to propagate a "-" will be
//! appended.
//
// *****************************************************************************
class TAUP_EXP TauPSite
{
  public:

    // **** _PUBLIC LIFECYCLES_ ************************************************

    //! Default Constructor.
    TauPSite();

    //! \brief Standard Constructor. Sets the station name and phase
    //! represented by this TauPSite.
    TauPSite(const string& staname, const string& phase);

    //! Copy Constructor.
    TauPSite(const TauPSite& tps);

    //! Destructor.
    virtual ~TauPSite();

    // **** _PUBLIC OPERATORS_ *************************************************
    
    //! Assignment operator.
    TauPSite&         operator=(const TauPSite& tps);

    // **** _PUBLIC METHODS_ ***************************************************

    //! \brief Sets 'this' sites depth.
    void              setSiteDepth(double depth);

    //! \brief Sets the velocity models for 'this' TauPSite ... model ownership
    //! is set to NOT OWNED (tpsIsVelModlOwned = false).
    void              setVelocityModels(const vector<TPVelocityLayer*>& vm);

    //! \brief Appends another velocity model to the profile for 'this'
    //! TauPSite. If this is the first then model pointer is saved and ownership
    //! is set to NOT OWNED (tpsIsVelModlOwned = false). If this is the second
    //! or greater velocity layer added then if tpsIsVelModlOwned is false
    //! the pointer is added else a copy of the object is made and added.
    void              appendVelocityModel(TPVelocityLayer* vm);

    //! \brief Appends a new OWNED constant velocity model to the profile
    //! stack. If this is the first entry a new model is created and
    //! tpsIsVelModlOwned is set to true. If this is the second or greater
    //! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
    //! the new model is added to the stack. Note the top radius must equal
    //! the bottom radius of the previous layer else an error is thrown.
    void              appendConstVelocityModel(double c, double rt, double rb,
                                               const string& layrnam = "");

    //! \brief Appends a new OWNED linear velocity model to the profile
    //! stack. If this is the first entry a new model is created and
    //! tpsIsVelModlOwned is set to true. If this is the second or greater
    //! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
    //! the new model is added to the stack. Note the top radius must equal
    //! the bottom radius of the previous layer else an error is thrown.
    void              appendLinearVelocityModel(double a0, double a1,
                                                double rt, double rb,
                                                const string& layrnam = "",
                                                double normradius = 1.0);

    //! \brief Appends a new OWNED quadratic velocity model to the profile
    //! stack. If this is the first entry a new model is created and
    //! tpsIsVelModlOwned is set to true. If this is the second or greater
    //! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
    //! the new model is added to the stack. Note the top radius must equal
    //! the bottom radius of the previous layer else an error is thrown.
    void              appendQuadraticVelocityModel(double a0, double a1,
                                                   double a2,
                                                   double rt, double rb,
                                                   const string& layrnam = "",
                                                   double normradius = 1.0);

    //! \brief Appends a new OWNED cubic velocity model to the profile
    //! stack. If this is the first entry a new model is created and
    //! tpsIsVelModlOwned is set to true. If this is the second or greater
    //! model and tpsIsVelModlOwned is false an error is thrown. Otherwise,
    //! the new model is added to the stack. Note the top radius must equal
    //! the bottom radius of the previous layer else an error is thrown.
    void              appendCubicVelocityModel(double a0, double a1,
                                               double a2, double a3,
                                               double rt, double rb,
                                               const string& layrnam = "",
                                               double normradius = 1.0);

    //! \brief Clears all velocity models from the current profile. If the
    //! models are OWNED they are deleted.
    void              clearVelocityModels();

    //! \brief Calculates travel times for all valid phases at the input
    //! distance and depth. This is the primary work function of the
    //! TauPSite object.
    void              calculateTravelTimes(double srcdist, double srcdepth,
                                           bool evalderivs = true);

    //! \brief Returns the first travel time result. If no travel time exists
    //! a NULL pointer is returned
    TravelTimeResult* getFirstTravelTimeResult();

    //! \brief Returns the first travel time (smallest) for the last call to
    //! calculateTravelTimes(dist, depth).
    double            getFirstTravelTime() const;

    //! \brief Returns the first travel time (smallest) for the last call to
    //! calculateTravelTimes(dist, depth) that is not a diffracted ray
    //! (e.g. not Pdiff). If only a diffracted results exists this first is
    //! returned. If no travel time exists a -1.0 is returned.
    double            getFirstNonDiffractedTravelTime() const;

    TravelTimeResult* getTravelTimeResult(const string& phase,
                                          bool matchPhase);
    //- Returns the travel time result with the input matching phase.
    //- If the matching phase was not found and matchPhase is true a NULL
    //- pointer is returned. If the matching phase was not found and matchPhase
    //- is false then the first arrival travel time result is returned.

    double            getTravelTime(const string& phase,
                                    bool matchPhase) const;
    //- Returns the travel time with the input matching phase.
    //- If the matching phase was not found and matchPhase is true a travel
    //- time result of -1 is returned. If the matching phase was not found and
    //- matchPhase is false then the first arrival travel time is returned.

    //! \brief Returns the map of all discovered travel time results sorted from
    //! fastest to slowest.
    const map<double, TravelTimeResult*>& getAllTravelTimes() const;

    //! \brief Returns the total integrated distance of a ray with ray
    //! parameter \em p in the input parameter \em d. The function returns
    //! true if successful and false if the result is invalid. The result is
    //! integrated from the surface of the planet to the turning depth,
    //! wherever that may be.
    //
    //! If the ray parameter is equal to the top of a layers ray parameter in a shadow
    //! then the boolean flag \em bottom_pass should be set to true to ensure that
    //! the integration does not terminate before accumulating the run-up to the top
    //! of the next layer.
    bool              integrateDistance(double p, double& d,
                                        bool bottom_pass = false);

    //! \brief Returns the total integrated distance of a ray with ray
    //! parameter \em p in the input parameter \em d. The function returns
    //! true if successful and false if the result is invalid. The result is
    //! integrated from the surface of the planet to the input depth \em r.
    bool              integrateDistance(double p, double r, double& d);

    //! \brief Returns the total integrated distance of a ray with ray parameter
    //! \em p in the input parameter \em d. The function returns true if successful
    //! and false if the result is invalid. The result is integrated from the
    //! \em r1 to depth \em r2.
    bool              integrateDistance(double p, double r1, double r2, double& d);

    //! \brief Returns the total integrated travel time of a ray with ray
    //! parameter \em p in the input parameter \em t. The function returns
    //! true if successful and false if the result is invalid. The result is
    //! integrated from the surface of the planet to the turning depth,
    //! wherever that may be.
    //
    //! If the ray parameter is equal to the top of a layers ray parameter in
    //! a shadow then the boolean flag \em bottom_pass should be set to true to
    //! ensure that the integration does not terminate before accumulating the
    //! run-up to the top of the next layer.
    bool              integrateTime(double p, double& d,
                                    bool bottom_pass = false);

    //! \brief Returns the total integrated travel time of a ray with ray
    //! parameter \em p in the input parameter \em t. The function returns
    //! true if successful and false if the result is invalid. The result is
    //! integrated from the surface of the planet to the input depth \em r.
    bool              integrateTime(double p, double r, double& d);

    //! \brief Returns the total integrated travel time of a ray with ray parameter
    //! \em p in the input parameter \em d. The function returns true if successful
    //! and false if the result is invalid. The result is integrated from the
    //! \em r1 to depth \em r2. An invalid ray is returned if p is greater than the
    //! larger ray parameter evaluated at r1 and r2 or if the ray turns before
    //! reaching r2 (r1 must be > r2).
    bool              integrateTime(double p, double r1, double r2, double& t);

    //! \brief  Writes the TauPSite as a standard ASCII file in CLR format
    //! (see TauPModel function readData for a description of the format).
    virtual void      writeData(const string& filename) const;

    //! \brief This function writes out the surface to surface distance for
    //! waves turning in layer \em i of the input wave velocity layer vector
    //! \em vellyr.
    //
    //! The ray parameter (s/deg) is written along with the travel distance
    //! (deg) into the input stream \em os. At least \em n points will be
    //! written but more may be required if the maximum traversed distance
    //! divided by \em n is exceeded in any step. The function automatically
    //! decrements the ray-parameter step size for areas where the
    //! corresponding distance step size exceeds the aforementioned limit.
    void              writeLayerData(int i, ostream& os, int n,
                                     double f0 = 0.0, double f1 = 1.0);

    //! \brief Debug function to output source receiver local conditions.
    void              dumpLocalSrcRcvrLayers(ostream& os);

    //! \brief Debug function to output layer information.
    void              dumpLayerInfo(ostream& os);

    //! Print object data to string.
    string            toString() const;

    //! Print object data to input stream \em os.
    virtual void      toStream(ostream& os, string indent) const;

    // **** _PUBLIC PROPERTIES_ ************************************************

    //! Static function that returns the class name.
    static  string    class_name();

    //! Virtual function that returns the class name.
    virtual string    get_class_name() const;

    //! Static function that returns the class size.
    virtual int       class_size() const;

    //! Static function that returns the class common name.
    static  string    commonName();

    //! virtual function that returns the class common name.
    virtual string    getCommonName() const;

    //! Returns the assigned site name.
    const string&     getName() const;

    //! Returns the assigned site phase.
    const string&     getPhase() const;

    //! Sets the assigned model name string.
    void              setModelName(const string& modname);

    //! Returns the assigned model name string.
    const string&     getModelName() const;

    //! Set the Earth radius for this site. Overrides the default.
    void              setEarthRadius(double r);

    //! Return the Earth radius.
    double            getEarthRadius() const;

    //! \brief Sets the Brents zero-in tolerance.
    void              setBrentsTolerance(double tol);

    //! \brief Returns the Brents zero-in tolerance.
    double            getBrentsTolerance() const;

    //! Returns the vector of layered velocity models assigned to this site.
    const vector<TPVelocityLayer*>& getVelocityModels() const;

    //! Returns the total memory resource for this TauPSite.
    virtual int64     get_memory() const;

    //! Returns the total heap allocated memory resource for this TauPSite.
    virtual int64     get_alloc_memory() const;

    // **** _PUBLIC INQUIRIES_ *************************************************

  private:

    // **** _PRIVATE METHODS_ **************************************************

    //! \brief Writes the TauPSite as a standard ASCII file in CLR format (see
    //! TauPModel function readData for a description of the format).
    virtual void      writeData(ostream& os) const;

    //! \brief Clears the input ttrm map and stores the TravelTimeResult
    //! entries for future use.
    void              clearTTRMap();

    //! \brief saves the diffracted ray results into the TravelTimeResult object
    //! returned by the call to saveResult().
    TravelTimeResult* saveResultI(double pB, double pI, double dist,
                                  int i, bool isUpper, bool isLower);

    //! \brief Saves the travel time results for the ray parameter p that turned
    //! in velocity layer i. The TravelTimeResult object containing the result
    //! is returned to the caller.
    TravelTimeResult* saveResult(double p, int i,
                                 bool isintrfcupper, bool isintrfclower,
                                 bool isspltlower, bool isspltupper,
                                 bool isturningzero);

    //! \brief Evaluates the limits of each velocity layer in the input vector.
    //! The limits include the layer type, pMin, the derivative of travel time
    //! with respect to the ray parameter (slowness), The distance at pT and pB,
    //! and a retrogrades layer split distance and ray parameter.
    void              findLimits();

    //! \brief Private function used to output n zero results between p0 and p1.
    void              debugOutZeros(int n, double p0, double p1);

    //**************************************************************************
    //**** Begin: Derivative Evaluation Functions ******************************
    //**************************************************************************

    // Derivative calculation occurs in the function evaluateDerivatives(...).
    // The initial call is made by calculateTravelTimes(...). The arguments
    // assume that a valid travel time has been evaluated for some layer whose
    // top and bottom ray parameter is given by pT and pB, respectively. The
    // valid travel time is passed in as T00. The travel time was evaluated at
    // dist and depth. The derivatives will be determined numerically
    // by calculating travel times at steps away from dist and depth using the
    // step sizes edist and edepth. The derivatives are returned in the array
    // derivs[] and include derivs[0] = dT/dDist, derivs[1] = dT/dDpth,
    // derivs[2] = d^2T/dDist/dDpth, and derivs[3] = d^2T/dDist^2. The travel
    // time is assumed to correspond to a turning ray unless the diffracted
    // ray parameter (pI) is input as a non-negative value (defaults to -1.0).
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

    //! \brief Calculates the travel time derivatives (\em derivs) at the
    //! input distance (\em dist) and depth (\em depth). The derivatives are
    //! evaluated numerically by calculating travel time at dist + f*edist and
    //! depth + f*edepth. The fraction f is either -1, -1/2, 0, 1/2, or 1 and
    //! is evaluated automatically for various stencils. The user must set
    //! \em edist and \em edepth to some small step size to begin. Seven
    //! different stencils will be tried until one succeeds in evaluating the
    //! derivatives. If none succeed the step sizes are cut in half and the
    //! function is recursively called to try again. If after 5 recursions none
    //! of the 7 different stencils are successfully evaluated an error is
    //! thrown.
    //
    //! The derivatives are only evaluated if a valid solution exists for some
    //! ray parameter between \em pT, the layer top ray parameter, and \em pB,
    //! the layer bottom ray parameter. The evaluated travel time at \em dist
    //! and \em depth is given by \em T00 and is used to numerically evaluate
    //! the derivatives. A diffracted result is returned at ray parameter
    //! \em pB if the interface slowness, \em pI, is non-negative. The default
    //! value for \em pI is defined as -1.0 (a turning ray). The recursion
    //! level (\em derivcnt) is initialized to 0 and is incremented each time
    //! this function is recursively called.
    //!
    //! The returned derivative array contains the following four derivatives
    //! on outputa:
    //!
    //!    derivs[0] = dT/dD
    //!    derivs[1] = dT/dr
    //!    derivs[2] = d^2T/dD/dr
    //!    derivs[3] = d^2T/dD^2
    //!
    void              evaluateDerivatives(double pT, double pB, double T00,
                                          double dist, double depth,
                                          double edist, double edepth,
                                          double* derivs, double pI = -1.0,
                                          int derivcnt = 0);

    //! \brief Recursively calls evaluateDerivatives(...) using a distance and
    //! depth stepsize reduced by 2.0. If the input \em derivcnt flag is 5
    //! then the function prints and error message and returns without calling
    //! the function evaluateDerivatives(...).
    void              reEvaluateDerivatives(double pT, double pB, double T00,
                                            double dist, double depth,
                                            double edist, double edepth,
                                            double* derivs, double pI,
                                            int derivcnt);

    //! \brief Evaluates the derivatives (\em derivs) at distance \em dist and
    //! depth \em depth using a first order stencil surrounding the current
    //! interpolation point whose travel time is given by \em T00.
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
    int               evalDerivsPrimary(double pT, double pB, double T00,
                                        double dist, double depth,
                                        double edist, double edepth,
                                        double* derivs, double pI);

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
    int               evalDerivsAlternateA(double pT, double pB, double T00,
                                           double dist, double depth,
                                           double edist, double edepth,
                                           double* derivs, double pI);

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
    int               evalDerivsAlternateB(double pT, double pB, double T00,
                                           double dist, double depth,
                                           double edist, double edepth,
                                           double* derivs, double pI);

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
    int               evalDerivsAlternateC(double pT, double pB, double T00,
                                           double dist, double depth,
                                           double edist, double edepth,
                                           double* derivs, double pI);

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
    int               evalDerivsAlternateD(double pT, double pB, double T00,
                                           double dist, double depth,
                                           double edist, double edepth,
                                           double* derivs, double pI);

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
    int               evalDerivsAlternateE(double pT, double pB, double T00,
                                           double dist, double depth,
                                           double edist, double edepth,
                                           double* derivs, double pI);

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
    int               evalDerivsAlternateF(double pT, double pB, double T00,
                                           double dist, double depth,
                                           double edist, double edepth,
                                           double* derivs, double pI);

    //! \brief Calculates the travel time (\em T) at input distance (\em dist)
    //! and depth (\em depth). The solution is performed for a layer whose ray
    //! parameter at the top and bottom is defined by \em pT and \em pB. If no
    //! solution exists for that layer the function returns -1.0. Otherwise, the
    //! ray parameter that provides the input distance and depth is returned.
    //! The brents zero-in function containing the travel time zero functional
    //! (TPZeroFunctional) is input as \em bz. A diffracted result is returned
    //! if the interface slowness, \em pI, is non-negative. The default value
    //! for \em pI is defined as -1.0 (a turning ray).
    double            layeredTravelTime(double pT, double pB, double dist,
                                        double depth, double& T, double pI);

    //**************************************************************************
    //**** End: Derivative Evaluation Functions ********************************
    //**************************************************************************

    // **** _PRIVATE DATA_ *****************************************************

    //! \brief Static instantiation count used to maintain a count of
    //! all objects of this type that have been instantiated.
    static int        tpsClassCount;

    //! \brief Static default numerical derivative distance / depth stepsize.
    static const double tpsDerivStepSize;

    //! \brief Static default Brents zero-in Tolerance.
    static const double tpsBrentsZeroInTol;

    //! \brief A temporary static stack used to contain unused TravelTimeResult
    //! objects to avoid reallocation costs.
    static vector<TravelTimeResult*> tpsReuseTTR;

    //! \brief The velocity structure of the various layers used by 'this'
    //! TauPSite. If tpsIsVelModlOwned is true then the velocity structure
    //! is owned by 'this' TauPSite. Otherwise, it was assigned by some
    //! TauPModel object.
    vector<TPVelocityLayer*> tpsVLayer;

    //! \brief Internal flag that is true if the velocity models used by this
    //! TauPSite were instantiated by it. If they are owned by some other
    //! object this flag is false.
    bool              tpsIsVelModlOwned;

    //! The receiver name.
    string            tpsRcvrName;

    //! The receiver phase type (P or S).
    string            tpsRcvrPhase;

    //! The velocity model name.
    string            tpsModelName;

    //! The receiver radius.
    double            tpsRcvrRad;

    //! The receiver depth.
    double            tpsRcvrDepth;

    //! The source radius.
    double            tpsSrcRad;

    //! The source depth.
    double            tpsSrcDepth;

    //! The distance between the source and receiver.
    double            tpsDist;

    //! The last evaluated source-receiver distance.
    double            tpsLastDist;

    //! The last evaluated source depth.
    double            tpsLastDepth;

    //! \brief The TauP zero-in functional used to zero-in on a ray parameter
    //! that gives the source distance / depth.
    TPZeroFunctional  tpsZeroF;

    //! \brief The Brents zero-in and function maximum utility object used to
    //! find the zero in tpsZeroF and the distance versus ray parameter minimum
    //! in retro-grade layers.
    util::Brents<TPZeroFunctional> tpsZeroIn;

    //! \brief The map of all evaluated branch phases that satisfy the latest
    //! input source distance/depth. The results are sorted on travel time from
    //! minimum to maximum. If the map is empty on solution was found for the
    //! input source distance / depth. The ray characteristics are contained in
    //! the associated TravelTimeResult object for each discovered ray.
    map<double, TravelTimeResult*> tpsTTR;

}; // TauPSite End Definition

// *****************************************************************************
// **** TravelTimeResult Definition ********************************************
// *****************************************************************************

// *****************************************************************************
//
//! \brief A public container (struct) that holds the result of a specific
//! ray that satisfied the source distance / depth! requirement. This object
//! maintains all pertinent information related to the ray for subsequent
//! retrieval and evaluation.
//
// *****************************************************************************
class TAUP_EXP TravelTimeResult
{
  public:

    //! Default constructor.
    TravelTimeResult() {};

    //! Dump TTR to string.
    string           toString(string indent = "") const;

    //! Dump TTR to stream.
    void             toStream(ostream& os, string indent = "") const;

    //! The velocity layer within which the ray turned.
    TPVelocityLayer* ttrVelLayer;

    //! \brief The velocity layers input provided phase name if given. Else the
    //! string is empty.
    string           ttrPhaseName;

    //! \brief The velocity layers evaluated default name. This string is never
    //! empty. This will be Xnb, Xnb+, Xnb-, Xni+, or Xni- where n is the layer
    //! index and X is "P" or "S".
    string           ttrPhaseEval;

    //! \brief A string that contains "Turning", "UpGoing", or "DownGoing"
    //! indicating the type of ray stored by this result.
    string           ttrRayType;

    //! The index of the velocity layer within which the ray turned.
    int              ttrLayerIndex;             // Xn

    //! /brief A boolean, which if true, defines this information as a ray
    //! that diffracts along the lower boundary of the layer using the
    //! layers velocity at the lower boundary.
    bool             ttrIsInterfaceUpper;       // Xni+

    //! /brief A boolean, which if true, defines this information as a ray
    //! that diffracts along the lower boundary of the layer using the
    //! next layers velocity at its upper boundary.
    bool             ttrIsInterfaceLower;       // Xni-

    //! \brief A boolean, which if true, defines the ray as turning in the
    //! upper half of a retrograde layer.
    bool             ttrIsSplitUpper;           // Xnb+

    //! \brief A boolean, which if true, defines the ray as turning in the
    //! lower half of a retrograde layer.
    bool             ttrIsSplitLower;           // Xnb- ... else Xnb

    //! The ray parameter (slowness) of the ray.
    double           ttrP;

    //! The ray turning radius.
    double           ttrR;

    //! The velocity at the turning point.
    double           ttrV;

    //! The total travel time of the ray from source to receiver.
    double           ttrT;

    //! The surface-to-surface ray distance.
    double           ttrDRay;

    //! The surface-to-surface ray time.
    double           ttrTRay;

    //! The surface-to-source ray distance.
    double           ttrDSrc;

    //! The surface-to-source ray time.
    double           ttrTSrc;

    //! The surface-to-receiver ray distance.
    double           ttrDRcvr;

    //! The surface-to-receiver ray time.
    double           ttrTRcvr;

    //! The distance along the layer boundary if ttrIsInterface is true.
    double           ttrDIntrfc;

    //! The time along the layer boundary if ttrIsInterface is true.
    double           ttrTIntrfc;

    //! The derivative array containing the following entries
    //!    ttrDerivs[0] = dT/dDist;
    //!    ttrDerivs[1] = dT/dDpth;
    //!    ttrDerivs[2] = d^2T/dDist/dDpth;
    //!    ttrDerivs[3] = d^2T/dDist^2;
    double           ttrDerivs[4];

  private:
};

// *****************************************************************************
// **** _INLINE FUNCTION IMPLEMENTATIONS_ **************************************
// *****************************************************************************

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Static function that returns the class name.
//
// *****************************************************************************
inline string TauPSite::class_name()
{
  return "TauPSite";
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the class name.
//
// *****************************************************************************
inline string TauPSite::get_class_name() const
{
  return class_name();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the class size (in bytes).
//
// *****************************************************************************
inline int TauPSite::class_size() const
{
  return (int) sizeof(TauPSite);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Static function that returns the common name of the object.
//
// *****************************************************************************
inline string TauPSite::commonName()
{
  return ("1-D Radial TauP Travel Time Site Model");
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the common name.
//
// *****************************************************************************
inline string TauPSite::getCommonName() const
{
  return commonName();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the total memory allocated by 'this' TTBMTauP object.
//
// *****************************************************************************
inline int64 TauPSite::get_memory() const
{
  return (sizeof(TauPSite) + TauPSite::get_alloc_memory());
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the site name.
//
// *****************************************************************************
inline const string& TauPSite::getName() const
{
  return tpsRcvrName;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the site phase.
//
// *****************************************************************************
inline const string& TauPSite::getPhase() const
{
  return tpsRcvrPhase;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the model name.
//
// *****************************************************************************
inline void TauPSite::setModelName(const string& modname)
{
  tpsModelName = modname;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the model name.
//
// *****************************************************************************
inline const string& TauPSite::getModelName() const
{
  return tpsModelName;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the vector of layered velocity models.
//
// *****************************************************************************
inline const vector<TPVelocityLayer*>& TauPSite::getVelocityModels() const
{
  return tpsVLayer;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Set the Earth radius.
//
// *****************************************************************************
inline void TauPSite::setEarthRadius(double r)
{
  tpsZeroF.setPlanetRadius(r);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return the Earth radius.
//
// *****************************************************************************
inline double TauPSite::getEarthRadius() const
{
  return tpsZeroF.getPlanetRadius();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Sets the Brents zero-in tolerance.
//
// *****************************************************************************
inline void TauPSite::setBrentsTolerance(double tol)
{
  tpsZeroIn.setTolerance(tol);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the Brents zero-in tolerance.
//
// *****************************************************************************
inline double TauPSite::getBrentsTolerance() const
{
  return tpsZeroIn.getTolerance();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the first travel time result. If no travel time exists
//! a NULL pointer is returned
// *****************************************************************************
inline const map<double, TravelTimeResult*>& TauPSite::getAllTravelTimes() const
{
  return tpsTTR;
}

} // end namespace taup

#endif // TAUPSITE_H
