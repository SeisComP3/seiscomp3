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
//- Program:       SNL Base Utility Library (Util)
//- Module:        $RCSfile: Brents.h,v $
//- Creator:       Jim Hipp
//- Creation Date: April 17, 2007
//- Revision:      $Revision: 1.5 $
//- Last Modified: $Date: 2011/10/03 14:43:30 $
//- Last Check-in: $Author: avencar $
//-
//- ****************************************************************************

#ifndef BRENTS_H
#define BRENTS_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>

using namespace std;
// use standard library objects

// **** _LOCAL INCLUDES_ *******************************************************

#include "UtilGlobals.h"

// **** _BEGIN UTIL NAMESPACE_ *************************************************

namespace util {

// **** _CLASS CONSTANTS_ ******************************************************
// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************
//
//! \brief Class supports Brents zeroF and minF functions that finds the root
//! of or minimum (maximum) of a provided function given an input range
//! containing the root (or minimum or maximum) and a convergence tolerance.
//
//! This object must assign the functional before use (at instantiation or with
//! the function setF(f)). A tolerance may be provided at instantiation or reset
//! to another value at a later time. both the function to be zeroed (or
//! minimized) and the tolerance can be accessed with predefined "get"
//! functions.
//!
//! The function that evaluates the zero, zeroF(...), or finds the minimum /
//! maximum, minF(...), are defined in detail below.
//
// *****************************************************************************
template<class F>
class UTIL_EXP Brents
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! Default Constructor.
    Brents();

    //! \brief Standard Constructor.
    //!
    //! Assigns the input function, \em f, and a tolerance \em tol.
    Brents(F& f, double tol);

    //! Copy Constructor.
    Brents(const Brents<F>& zb);

    //! Destructor.
    virtual ~Brents();

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment Operator.
    Brents<F>&        operator=(const Brents<F>& zb);

    // **** _PUBLIC METHODS_ ***************************************************

    //! \brief function zeroF - obtains a function zero within the given range.
    //
    //! Input
    //!   double zeroin(ax, bx)
    //!   double ax;                        Root will be seeked for within
    //!   double bx;                        a range [ax, bx]
    //!   double (*f)(double x);            Name of the function whose zero
    //!                                     will be seeked for
    //!   double tol;                       Acceptable tolerance for the root
    //!                                     value.
    //!                                     May be specified as 0.0 to cause
    //!                                     the program to find the root as
    //!                                     accurate as possible
    //!
    //! Output
    //!   Zeroin returns an estimate for the root with accuracy
    //!   4*EPSILON*abs(x) + tol
    //!
    //! Algorithm
    //!   G.Forsythe, M.Malcolm, C.Moler, Computer methods for mathematical
    //!   computations. M., Mir, 1980, p.180 of the Russian edition
    //!
    //!  The function makes use of the bissection procedure combined with
    //!  the linear or quadric inverse interpolation. At every step the program
    //!  operates on three abscissae - a, b, and c, defined as
    //!    b - the last and the best approximation to the root
    //!    a - the last but one approximation
    //!    c - the last but one or even earlier approximation than a such that
    //!        1) |f(b)| <= |f(c)|
    //!        2) f(b) and f(c) have opposite signs, i.e. b and c confine
    //!           the root
    //!
    //!  At every step Zeroin selects one of the two new approximations, the
    //!  former being obtained by the bissection procedure and the latter
    //!  resulting in the interpolation (if a,b, and c are all different
    //!  the quadric interpolation is utilized, otherwise the linear one).
    //!  If the latter (i.e. obtained by the interpolation) point is 
    //!  reasonable (i.e. lies within the current interval [b,c] not being
    //!  too close to the boundaries) it is accepted. The bissection result
    //!  is used in the other case. Therefore, the range of uncertainty is
    //!  ensured to be reduced at least by the factor 1.6
    double            zeroF(double ax, double bx);

    //! \brief Returns the functional minimum between the input abscissas
    //! \em ax and \em cx at the internall defined function zbF.
    //
    //! The abscissa bx is evaluated between ax and cx such that zbF(bx)
    //! is less than or greater than both zbF(ax) and zbF(cx). This function
    //! isolates the minimum to a fractional precision of about \em tol
    //! using Brent's method. The abscissa of the minimum is returned in xmin,
    //! and the minimum function value is returned as the function value.
    double            minF(double ax, double bx, double cx,
                           double& xmin);

    //! Sets the functional to f.
    void              setF(F& f);

    //! Returns a reference to the functional.
    F&                getF();

    //! Returns a const reference to the functional.
    const F&          getF() const;

    //! Sets the tolerance value.
    void              setTolerance(double tol);

    //! Returns the tolerance value.
    double            getTolerance() const;

    //! Sets the search method to find a functional minimum (the default)
    //! Note: Used by minF only.
    void              setMinimumSearch();

    //! Sets the search method to find a functional maximum.
    //! Note: Used by minF only.
    void              setMaximumSearch();

    //! Returns true if the search method is for a minimum.
    //! Note: Applies to function minF only.
    bool              isMinimumSearch() const;

    //! Output the test 1 zero-in value
    //static void       getTest1(double a, double b, double tol);

    //! Output the test 2 max-search value
    //static void       getTest2(double a, double b, double c, double tol,
    //                          const string& minmax);

private:

    // **** _PRIVATE DATA_ *****************************************************

    //! The zeroin convergence tolerance.
    double            zbTol;

    //! The function to be zeroed, minimized, or maximized using Brents
    //! methods.
    F*                zbF;

    //! \brief A flag used by the minF(...) function to switch between a
    //! minimum and maximum searcher. If the flag is 1.0 the search is for a
    //! minimum. A value of -1.0 searches for a maximum. Note: The Brents
    //! minimum/maximum searcher can only find one or the other and not both
    //! without setting this flag. An end-point and associated value will be
    //! returned if the setting is wrong.
    double            bMinMaxFlg;
};

// **** _INLINE FUNCTION IMPLEMENTATIONS_ **************************************

//! Sets the functional to \em f.
template<class F>
inline void Brents<F>::setF(F& f)
{
  zbF = &f;
}

//! Returns a reference to the functional.
template<class F>
inline F& Brents<F>::getF()
{
  return *zbF;
}

//! Returns a const reference to the functional.
template<class F>
inline const F& Brents<F>::getF() const
{
  return *zbF;
}

//! Sets the Brents tolerance value.
template<class F>
inline void Brents<F>::setTolerance(double tol)
{
  zbTol = tol;
}

//! Returns the Brents tolerance value.
template<class F>
inline double Brents<F>::getTolerance() const
{
  return zbTol;
}

//! Sets the search method to a minimum (the default).
//
//! Note: Used function minF only.
template<class F>
inline void Brents<F>::setMinimumSearch()
{
  bMinMaxFlg = 1.0;
}

//! Sets the search method to a maximum.
//
//! Note: Used function minF only.
template<class F>
inline void Brents<F>::setMaximumSearch()
{
  bMinMaxFlg = -1.0;
}

//! Returns true if the search method is a minimum.
//
//! Note: Applies only to function minF.
template<class F>
inline bool Brents<F>::isMinimumSearch() const
{
  return ((bMinMaxFlg == 1.0) ? true : false);
}

// **** _TEST FUNCTION IMPLEMENTATIONS_ **************************************

//! \brief Test of the zero-in functionality. This object provides a function
//! object whose operator() definition solves for the zero of
//!
//!     47 - 2x^3 + 3x - 2;
//class TestZeroIn1
//{
//  public:
//
//    //! Default Constructor.
//    TestZeroIn1() {};
//
//    //! Copy Constructor.
//    TestZeroIn1(const TestZeroIn1& tzif) {};
//
//    //! Destructor.
//   ~TestZeroIn1() {};
//
//    //! Assignment Operator.
//    TestZeroIn1& operator=(const TestZeroIn1& tzif)
//    {return *this;};
//
//    //! \brief Function parentheses operator(). Returns a double given a
//    //! single double argument. This function is the function to be zeroed.
//    //! and for which a maximum (or minimum) will be found.
//    double    operator()(double x)
//    {
//      ++fccnt;
//      return 47 - (2.0 * x * x * x - 3.0 * x + 2.0);
//    };
//
//    //! Zeros the function call count.
//    void      resetFCCount() {fccnt = 0;};
//
//    //! Returns the function call count.
//    int       getFCCount() const {return fccnt;};
//
//  private:
//
//    //! \brief The integrand function call count. Used to examine the
//    //! number of times the operator()(double x) function is called.
//    int fccnt;
//};

//! \brief Test of the min /max functionality. This object provides a function
//! object whose operator() definition solves the min / max that lies between
//!
//!     2x^3 - 3x + 2;
//class TestMaxF2
//{
//  public:
//
//    //! Default Constructor.
//    TestMaxF2() {};
//
//    //! Copy Constructor.
//    TestMaxF2(const TestMaxF2& tmf) {};
//
//    //! Destructor.
//   ~TestMaxF2() {};
//
//    //! Assignment Operator.
//    TestMaxF2& operator=(const TestMaxF2& tmf)
//    {return *this;};
//
//    //! \brief Function parentheses operator(). Returns a double given a
//    //! single double argument. This function is the function for which
//    //! a maximum (or minimum) will be found. These exist at +-sqrt(2)/2.
//    double    operator()(double x)
//    {
//      ++fccnt;
//      return (2.0 * x * x * x - 3.0 * x + 2.0);
//    };
//
//    //! Zeros the function call count.
//    void      resetFCCount() {fccnt = 0;};
//
//    //! Returns the function call count.
//    int       getFCCount() const {return fccnt;};
//
//  private:
//
//    //! \brief The integrand function call count. Used to examine the
//    //! number of times the operator()(double x) function is called.
//    int fccnt;
//};

} // end namespace util

#endif // BRENTS_H
