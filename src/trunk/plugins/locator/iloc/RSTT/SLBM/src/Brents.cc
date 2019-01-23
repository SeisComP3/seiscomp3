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
//- Module:        $RCSfile: Brents.cc,v $
//- Creator:       Jim Hipp
//- Creation Date: April 17, 2007
//- Revision:      $Revision: 1.11 $
//- Last Modified: $Date: 2012/06/01 14:48:28 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

// **** _SYSTEM INCLUDES_ ******************************************************

#include <float.h>
#include <limits>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstdio>

// **** _LOCAL INCLUDES_ *******************************************************

#include "Brents.h"

// **** _BEGIN UTIL NAMESPACE_ *************************************************

namespace util {

// **** _STATIC INITIALIZATIONS_************************************************

//template class Brents<TestZeroIn1>;
//template class Brents<TestMaxF2>;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

//! Default Constructor.
//
template<class F>
Brents<F>::Brents() : zbTol(1.0e-6), zbF(NULL), bMinMaxFlg(1.0)
{
}

//! Standard Constructor.
//
//! Assigns the functional, \em f, and the tolerance \em tol.
template<class F>
Brents<F>::Brents(F& f, double tol) : zbTol(tol),zbF(&f),  bMinMaxFlg(1.0)
{
}

//! Copy constructor.
template<class F>
Brents<F>::Brents(const Brents<F>& zb) : zbTol(zb.zbTol),
                  zbF(zb.zbF), bMinMaxFlg(zb.bMinMaxFlg)
{
}

//! Destructor.
template<class F>
Brents<F>::~Brents()
{
}

//! Assignment operator.
template<class F>
Brents<F>& Brents<F>::operator=(const Brents<F>& zb)
{
  zbF        = zb.zbF;
  zbTol      = zb.zbTol;
  bMinMaxFlg = zb.bMinMaxFlg;

  return *this;
}

// *****************************************************************************
//
//! function zeroF - obtains a function zero within the given range.
//!
//! Input
//!   double zeroF(ax, bx)
//!   double ax;                        Root will be seeked for within
//!   double bx;                        a range [ax,bx]
//!   double (*f)(double x);            Name of the function whose zero
//!                                     will be seeked for
//!   double tol;                       Acceptable tolerance for the root
//!                                     value.
//!                                     May be specified as 0.0 to cause
//!                                     the program to find the root as
//!                                     accurate as possible
//!
//! Output
//!   zeroF returns an estimate for the root with accuracy
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
//!  At every step zeroF selects one of the two new approximations, the
//!  former being obtained by the bissection procedure and the latter
//!  resulting in the interpolation (if a,b, and c are all different
//!  the quadric interpolation is utilized, otherwise the linear one).
//!  If the latter (i.e. obtained by the interpolation) point is 
//!  reasonable (i.e. lies within the current interval [b,c] not being
//!  too close to the boundaries) it is accepted. The bissection result
//!  is used in the other case. Therefore, the range of uncertainty is
//!  ensured to be reduced at least by the factor 1.6
//
// *****************************************************************************
template<class F>
double Brents<F>::zeroF(double ax, double bx)
{
  double a, b, c, fa, fb, fc, p, q, new_step, prev_step, tol_act;

  // initialize limits

  F& f = *zbF;
  a = ax; b = bx;
  fa = f(a);
  fb = f(b);
  c = a; fc = fa;

  // enter the primary loop

  while(1)
  {
    // set previous step size and test interval for data swap

    prev_step = b - a;
    if (fabs(fc) < fabs(fb))
    {
      // set b as the best guess so far

      a  = b;  b  = c;  c  = a;
      fa = fb;  fb = fc;  fc = fa;
    }

    // calculate current error and create a new step size

    tol_act = 2.0 * DBL_EPSILON * fabs(b) + 0.5 * zbTol;
    new_step = 0.5 * (c - b);

    // if tolerance is acceptable then return b

    if ((fabs(new_step) <= tol_act) || (fb == 0.0)) return b;

    // Perform the quadric interpolation if the previous step was both
    // large enough and in the true direction

    if ((fabs(prev_step) >= tol_act) && (fabs(fa) > fabs(fb)))
    {
      register double t1,cb,t2;

      // apply linear interpolation if only two distinct points ...
      // otherwise perform quadric inverse interpolation

      cb = c - b;
      if (a == c)
      {
        t1 = fb / fa;
        p  = cb * t1;
        q  = 1.0 - t1;
      }
      else
      {
        q = fa / fc;  t1 = fb / fc;  t2 = fb / fa;
        p = t2 * (cb * q * (q - t1) - (b - a) * (t1 - 1.0));
        q = (q - 1.0) * (t1 - 1.0) * (t2 - 1.0);
      }

      // p was calculated with the opposite sign ... if negative make p
      // positive ... if positive change q sign

      if (p > 0.0)
        q = -q;
      else
        p = -p;

      // if b + p / q falls in range [b, c] and is not too large then
      // accept it ... if p / q is too large then the bissection procedure
      // can reduce the range [b, c]

      if ((p < (0.75 * cb * q - fabs(tol_act * q) / 2.0)) &&
         (p < fabs(prev_step * q / 2.0)))
        new_step = p / q;
    }

    // adjust the step size to be not less than the tolerance

    if (fabs(new_step) < tol_act)
    {
      if (new_step > 0.0)
	      new_step = tol_act;
      else
	      new_step = -tol_act;
    }

    // save the previous approximation and calculate a new function evaluation
    // at the new b

    a  = b;
    fa = fb;
    b += new_step;
    fb = f(b);

    // adjust c to have opposite sign of b if fb and fc are of the same sign

    if (fb * fc > 0.0)
      //if ( (fb > 0 && fc > 0) || (fb < 0 && fc < 0) )
    {
      c  = a;
      fc = fa;
    }
  }
}


// *****************************************************************************
//
//! \brief Returns the functional minimum between the input abscissas
//! \em ax and \em cx at the interval defined function zbF.
//
//! The abscissa bx is evaluated between ax and cx such that zbF(bx)
//! is less than or greater than both zbF(ax) and zbF(cx). This function
//! isolates the minimum to a fractional precision of about zbTol
//! using Brent's method. The abscissa of the minimum is returned in xmin,
//! and the minimum function value is returned as the function value.
//
// *****************************************************************************
template<class F>
double Brents<F>::minF(double ax, double bx, double cx, double& xmin)
{
  const double CGOLD = 0.3819660112501051;
  const double ZEPS  = 1.0e-10;
  const int    ITMAX = 100;

  double a,b,d,e,fu,fv,fx,fw,u,v,w,x;
  double etemp,p,q,r,tol1,tol2,xm;
  int    iter;

  // intialize

  F& f = *zbF;
  a = b = ax;
  if (cx < ax)
    a = cx;
  else
    b = cx;
  v = bx; w = v; x = v; e = d = 0.0;
  fx = bMinMaxFlg * f(x); fv = fx; fw = fx;

  // inter main loop

  for (iter = 0; iter < ITMAX; ++iter)
  {
    // evaluate mid point and convergence tolerances

    xm   = 0.5 * (a + b);
    tol1 = zbTol * fabs(x) + ZEPS;
    tol2 = 2.0 * tol1;

    // see if convergence has been met

    if (fabs(x - xm) <= tol2 - 0.5 * (b - a)) break;

    // see if a trial parabolic fit or a golden section fit should be performed

    if (fabs(e) > tol1)
    {
      //Construct a trial parabolic fit

      r = (x - w) * (fx - fv);
      q = (x - v) * (fx - fw);
      p = (x - v) * q - (x - w) * r;
      q = 2.0 * (q - r);
      if (q > 0)  p = -p;
      q = fabs(q);
      etemp = e;
      e = d;

      // see if golden section or parabolic fit is required

      if ((fabs(p) >= fabs(0.5 * q * etemp)) ||
          (p <= q * (a - x)) || (p >= q * (b-x)))
      {
        // do golden section fit

        if (x >= xm)
          e = a - x;
        else
          e = b - x;

        d = CGOLD * e;
      }
      else
      {
        // do parabolic fit

        d = p / q;
        u = x + d;
        if (u -a < tol2 || b - u < tol2)
        {
          d = fabs(tol1);
          if (xm - x < 0.0) d = -d;
        }
      }
    }
    else
    {
      // do golden section fit

      if (x >= xm)
        e = a - x;
      else
        e = b - x;

      d = CGOLD * e;
    }

    // calculate new function evaluation point from d

    if (fabs(d) >= tol1)
      u = x + d;
    else
    {
      if (d < 0)
        u = x - fabs(tol1);
      else
        u = x + fabs(tol1);
    }
    // evaluate function and update values dependent on if fu is larger or
    // smaller then fx

    fu = bMinMaxFlg * f(u);
    if (fu <= fx)
    {
      if (u >= x)
        a = x;
      else
        b = x;

      v = w; fv = fw;
      w = x; fw = fx;
      x = u; fx = fu;
    }
    else
    {
      if (u < x)
        a = u;
      else
        b = u;

      if (fu <= fw || w == x)
      {
        v = w; fv = fw;
        w = u; fw = fu;
      }
      else if (fu <= fv || v == x || v == w)
      {
        v = u; fv = fu;
      }
    }
  }

  if (iter == ITMAX) printf(" Brent exceed maximum iterations.\n");
  xmin = x;
  return bMinMaxFlg * fx;
}

//! A test function object used exclusively for testing the
//! Zero-In properties of the Brents class.
//
//! This object defines the zero-in function 1 which has a zero at
//! 3.0
//template<class F>
//void Brents<F>::getTest1(double a, double b, double tol)
//{
//  double numY;
//
//  // create test function and IntegrateFunction ... reset function
//  // call count to zero
//
//  TestZeroIn1 tzif;
//  Brents<TestZeroIn1> btzif(tzif, tol);
//  tzif.resetFCCount();
//
//  // retrieve zero root
//
//  numY = btzif.zeroF(a, b);
//
//  // output results
//
//  cout << "count: " << tzif.getFCCount() << std::setprecision(14)
//       << ",  root: " << numY << endl;
//}

//! A test function object used exclusively for testing the
//! MaxF properties of the Brents class.
//
//! This object defines the maxF function 2 which has a maximum
//! at -sqrt(2)/2 and a minimum at sqrt(2)/2
//template<class F>
//void Brents<F>::getTest2(double a, double b, double c, double tol,
//                         const string& minmax)
//{
//  double numY;
//
//  // create test function and IntegrateFunction ... reset function
//  // call count to zero
//
//  TestMaxF2 tmf;
//  Brents<TestMaxF2> btzif(tmf, tol);
//  tmf.resetFCCount();
//  if (minmax == "max")
//    btzif.setMaximumSearch();
//  else if (minmax == "min")
//    btzif.setMinimumSearch();
//
//  // retrieve min or max root
//
//  double xroot = 0.0;
//  numY = btzif.minF(a, b, c, xroot);
//
//  // output results
//
//  cout << "count: " << tmf.getFCCount() << std::setprecision(14)
//       << ",  root: " << xroot << ",  value: " << numY << endl;
//}

} // end namespace util
