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
//- Module:        $RCSfile: IntegrateFunction.h,v $
//- Creator:       Jim Hipp
//- Creation Date: April 17, 2007
//- Revision:      $Revision: 1.6 $
//- Last Modified: $Date: 2011/10/03 14:43:30 $
//- Last Check-in: $Author: avencar $
//-
//- ****************************************************************************

#ifndef INTEGRATEFUNCTION_H
#define INTEGRATEFUNCTION_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <float.h>
#include <vector>
#include <iostream>
#include <limits>

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
//! \brief Class supports numerical integration of an arbitrary function using
//! a fourth order Simpsons rule for closed intervals. Open intervals at one
//! end-point are handled by using Simpsons rule in a segmented sense to
//! approach the unbounded limit in successive iterations each time dividing
//! the difference to the unbounded limit by 10.0.
//
//! This object must be instantiated with the function to be integrated. A
//! tolerance may be provided at instantiation or reset to another value at
//! a later time. Both the function to be integrated and the tolerance can be
//! accessed with predefined "get" functions.
//!
//! The integration functions are integrateClosed(...) and integrateAOpen(...)
//! defined below. The function integrateClosed(...) is a recursively
//! adaptive function that resolves the interval into one or more sub-intervals
//! until the convergence tolerance is met. Unlike their non-adaptive
//! counterparts these functions resolve each region of the function to whatever
//! resolution is required to meet the accuracy requirement. The open interval
//! version uses the closed interval definition for all internal sub-intervals.
//! The closed interval simpson rule is defined by
//!
//!   (h/3)(f0 + 4*f1 + f2)
//!
//! where h is the interval width, f0 is the function to be integrated defined
//! at the left most range, f2 is defined at the right most range, and f1 is
//! defined at the mid-point of the range.
//!
//! The simpsons adaptive integration evalutes the integral across a sub-
//! interval range using the rule 3 times, once across the sub-interval, once
//! across the 1st half of the sub-interval and once across the second half of
//! the sub-interval. The split halves are summed to form one result while the
//! entire interval evaluation forms the other. The former has an accuracy that
//! is about 4 times greater than the entire interval evaluation. Convergence is
//! attained when the difference between the two is less than the input
//! tolerance. Five function evaluation are required for each call to the closed
//! form Simpson rule.
//!
//! The function integrateAOpen(...) solves an improper integral containing an
//! unbounded integrand at the initial limit a (of a,b). This function uses the
//! closed form Simpsons solution to solve the integral from a+e to b where e is
//! some non-zero small number. Then it solves the integral a+e/10 to a+e in a
//! series of consecutive iterations where e is reduced by 10 at each iteration.
//! The results of each solution are summed with the first solution to obtain
//! the final result. The iterations are continued until convergence occurs or
//! an unbounded result terminates the solution with an accuracy less than that
//! which was requested.
//! 
// *****************************************************************************
template<class F>
class UTIL_EXP IntegrateFunction
{
  public:

    // **** _PUBLIC LIFCYCLES_ *************************************************

    //! \brief Standard Constructor.
    //!
    //! Since IntegrateFunction requires a reference assignment for the function
    //! F& ifF, the default constructor is not supported.
    IntegrateFunction(F& f, double tol);

    //! Copy Constructor.
    IntegrateFunction(const IntegrateFunction<F>& ifctn);

    //! Destructor.
    virtual ~IntegrateFunction();

    // **** _PUBLIC OPERATORS_ *************************************************

    //! Assignment Operator.
    IntegrateFunction<F>& operator=(const IntegrateFunction<F>& ifctn);

    // **** _PUBLIC METHODS_ ***************************************************

    //! \brief Adaptive closed form numerical integration of the input function
    //! over the closed interval \em a to \em b. The function is assumed to be
    //! defined over the entire interval [a, b]. The function uses the 3 point
    //! Simpsons rule to evaluate all sub-intervals.
    double            integrateClosed(double a, double b);

    //! \brief Adaptive closed form numerical integration of the input function
    //! over the open interval \em a to \em b, where the integrand function is
    //! undefined at \em a. The function uses the 3 point Simpsons rule to
    //! evaluate all sub-intervals. and slowly encroaches on the limit \em a
    //! until convergence is attained.
    double            integrateAOpenS(double a, double b);

    //! Returns a reference to the integrated function.
    F&                getF();

    //! Returns a const reference to the integrated function.
    const F&          getF() const;

    //! Sets the integration tolerance value.
    void              setTolerance(double tol);

    //! Returns the integration tolerance value.
    double            getTolerance() const;

    //! Returns the test 5 integral value.
    //static double     getTest(int f, double ftrn, double tol);

  private:

    // **** _PRIVATE METHODS_ **************************************************

    //! \brief Adaptive closed form numerical integration of the input function
    //! over the closed interval \em a to \em b. The function is assumed to be
    //! defined over the entire interval [a, b]. The function uses the 3 point
    //! Simpsons rule to evaluate all sub-intervals.
    //
    //! The array \em fr contains previous function evaluations at ifF(a),
    //! ifF((a + b)/2), and ifF(b). This function is only called by the public
    //! cover function integrateClosed(a, b).
    double            integrateClosedRcrsv(double a, double b, double* fr);

    //! \brief A 3 point 4th order Simpsons integration rule.
    //
    //!
    //!   (h/3)(f0 + 4*f1 + f2)
    //!
    //! where h is the interval width, f0 is the function to be integrated
    //! defined at the left most range, f2 is defined at the right most range,
    //! and f1 is defined at the mid-point of the range.
    //!
    //! The rule evaluates the integral across a sub-interval range using the
    //! rule 3 times, once across the sub-interval, once across the 1st half of
    //! the sub-interval and once across the second half of the sub-interval.
    //! The split halves are summed to form one result while the entire
    //! interval evaluation forms the other. The former has an accuracy that is
    //! about 4 times greater than the entire interval evaluation. Convergence
    //! is attained when the difference between the two is less than the input
    //! tolerance. Five function evaluations are required for the closed form
    //! solution. However, 3 evaluations are passed from the previous call so
    //! only 2 new function evalutions are required per call to this function.
    //! The 3 previous functions evaluations are provided in the array \em fa
    //! at locations fa[0], fa[2], and fa[4]. Evaluations for fa[1] and fa[3]
    //! are performed in this function and saved into the array.
    double            simpson(double a, double b, double* fa, double& s);

    // **** _PRIVATE DATA_ *****************************************************

    //! The integration convergence tolerance.
    double            ifTol;

    //! The function to be integrated numerically.
    F&                ifF;
};

// **** _INLINE FUNCTION IMPLEMENTATIONS_ **************************************

// Returns a reference to the integrated function.
template<class F>
inline F& IntegrateFunction<F>::getF()
{
  return ifF;
}

// Returns a const reference to the integrated function.
template<class F>
inline const F& IntegrateFunction<F>::getF() const
{
  return ifF;
}

// Sets the integration tolerance value.
template<class F>
inline void IntegrateFunction<F>::setTolerance(double tol)
{
  ifTol = tol;
}

// Returns the integration tolerance value.
template<class F>
inline double IntegrateFunction<F>::getTolerance() const
{
  return ifTol;
}

// Adaptive closed form numerical integration of the input function
// over the closed interval a to b. The function is assumed to be
// defined over the entire interval [a, b]. The function uses the 3 point
// Simpsons rule to evaluate all sub-intervals.
template<class F>
inline double IntegrateFunction<F>::integrateClosed(double a, double b)
{
  double fr[3];

  // evaluate the function at a, b, and the midpoint and save into fr.

  fr[0] = ifF(a);
  fr[1] = ifF(0.5 * (a + b));
  fr[2] = ifF(b);

  // integrate and return result

  return integrateClosedRcrsv(a, b, fr);
}

template<class F>
inline double IntegrateFunction<F>::integrateAOpenS(double a, double b)
{
  double fs, fa, ae, be, senew;

  // set the integration result to 0.0 and calculate a small value near a
  // but slightly larger (e). Set the smallest legal value for e (minstep)
  // and account for precision loss in the step size due to significant
  // digits in front of the decimal place (of a).

  double s = 0.0;
  double e = (b - a) * ifTol;
  fa = fabs(a);
  double minstep = 10.0 * DBL_EPSILON;
  if (fa > 1.0) minstep *= fa;

  // calculate integral from just shy of the asymptote to b and
  // loop over successive steps getting closer to a each step
  // (e to e/10 each loop). Loop while  the stepsize is decremented by
  // an order of magnitude at each step. 

  ae = a + e;
  s = integrateClosed(ae, b);
  be = ae; ae = a + 0.1 * e;
  do
  {
    // get contribution from a+e/10 to a+e ... sum contribution to s

    senew = integrateClosed(ae, be);
    s += senew;

    // see if error criteria has been met ... if so return s, otherwise
    // decrement e by 10 and try again

    fs = fabs(s);
    if ((fabs(senew) < ifTol * fs) || (fs < ifTol)) 
      return s;
    else
      e /= 10.0;

    // continue while e is larger than minimum step size

    be = ae; ae = a + 0.1 * e;
  } while ((e > minstep) && (be > ae) && (ae > a));

  // output error tolerance condtion and return best estimate

  if (fabs(senew) > ifTol)
  {
    // should throw here ... need to resume after the fact
    // should output a, b, e, ae, be, s, senew, iftol
    cout << "  Error:: Function Error Tolerance Exceeded ... " << endl
         << "          Tolerance Condition Was Not Met." << endl;
  }
  return s;
}

// Adaptive closed form numerical integration of the input function
// over the closed interval a to b. The function is assumed to be
// defined over the entire interval [a, b]. The function uses the 3 point
// Simpsons rule to evaluate all sub-intervals.
//
// The array fr contains previous function evaluations at ifF(a),
// ifF((a + b)/2), and ifF(b). This private function is only called by the
// public cover function integrateClosed(a, b).
template<class F>
inline double IntegrateFunction<F>::integrateClosedRcrsv(double a, double b,
                                                         double* fr)
{
  // set the integration result to 0.0 and save the input previously
  // calculated function evalutions in fr into fa. fa[1] and fa[3] are
  // calculated in function simpson(...) below.

  double s = 0.0;
  double fa[5];
  fa[0] = fr[0];
  fa[2] = fr[1];
  fa[4] = fr[2];

  // evaluate Simpsons rule and return result if tolerance is met.

  double se = simpson(a, b, fa, s);
  double fs = fabs(s);
  if ((se < ifTol * fs) || (fs < ifTol))
    return s;
  else
  {
    // not accurate enough yet ... calculate interval mid-point and verify
    // machine precison will allow evaluation.

    double m = 0.5 * (a + b);
    if ((m <= a) | (b <= m))
    {
      if (se > ifTol)
      {
        // write out warning
        // should throw error here ... write out a, b, m, s, se, iftol
        cout << "  Error:: Function Error Tolerance Exceeded ... " << endl
             << "          Tolerance Condition Was Not Met." << endl;
      }
      return s;
    }

    // integrate over left and right sub-intervals ... return the summed
    // result

    return integrateClosedRcrsv(a, m, &fa[0]) +
           integrateClosedRcrsv(m, b, &fa[2]);
  }
}

// A 3 point 4th order Simpsons integration rule.
//
//   (h/3)(f0 + 4*f1 + f2)
//
// where h is half the interval width, f0 is the function to be integrated
// defined at the left most range, f2 is defined at the right most range,
// and f1 is defined at the mid-point of the range.
//
// The rule evaluates the integral across a sub-interval range using the
// rule 3 times, once across the sub-interval, once across the 1st half of
// the sub-interval and once across the second half of the sub-interval.
// The split halves are summed to form one result while the entire
// interval evaluation forms the other. The former has an accuracy that is
// about 4 times greater than the entire interval evaluation. Convergence
// is attained when the difference between the two is less than the input
// tolerance.
//
// Five function evaluations are required for the closed form solution.
// However, 3 evaluations are passed from the previous call so only 2 new
// function evalutions are required per call to this function. The 3
// previous functions evaluations are provided in the array fa at
// locations fa[0], fa[2], and fa[4]. Evaltions for fa[1] and fa[3] are
// performed in this function and saved into the array.
template<class F>
inline double IntegrateFunction<F>::simpson(double a, double b,
                                            double* fa, double& s)
{
  // calculate interval width and the two remaining function evalutions

  double h = b - a;
  fa[1] = ifF(a + 0.25 * h);
  fa[3] = ifF(a + 0.75 * h);
  h    *= 0.5;

  // calculate results over entire interval (s1) and the interval split in
  // half (s). Return s and the difference between both evaluations.

  double dh = h / 6.0;
  double s1 = 2.0 * dh * (fa[0] + 4.0 * fa[2] + fa[4]);
  s  = dh * (fa[0] + 4.0 * (fa[1] + fa[3]) + 2.0 * fa[2] + fa[4]);
  return fabs(s - s1);
}

// **** _TEST FUNCTION IMPLEMENTATIONS_ **************************************

//! \brief A test function object used exclusively for testing the
//! integration properties of the IntegrateFunction class.
//
//! This object defines the integrand function 6 which can
//! be integrated from a to b. The analytic result is
//! 2x. The function is used to test the convergence
//! of the closed simpsons rule.

// f     = velocity function
// ftrn  = is fractional radius where rtrn = ftrn * (rtop - rbot) + rbot
// fa    = is fractional radius where ra = fa * (rtop - rtrn) + rtrn above the turn radius
// fb    = is fractional radius where rb = fb * (rtop - rtrn) + rtrn above the turn radius
//         (Note:  fb > fa, fb == 1 gives rb == rtop)
//  tif.setF(f);
//  tif.setRTurn(ftrn);
//  double a = tif.getA(fa);
//  double b = tif.getB(fb);
//class TestIntegrateFunction
//{
//  public:
//
//    //! Default Constructor.
//    TestIntegrateFunction() {f=1;};
//
//    //! Copy Constructor.
//    TestIntegrateFunction(const TestIntegrateFunction& tif) {f=1;};
//
//    //! Destructor.
//   ~TestIntegrateFunction() {f=1;};
//
//    //! Assignment Operator.
//    TestIntegrateFunction& operator=(const TestIntegrateFunction& tif)
//    {return *this;};
//
//    //! \brief Function parentheses operator(). Returns a double given a
//    //! single double argument. This function is the integrand.
//    double    operator()(double r)
//    {
//      ++fccnt;
//      double vr  = v(r);
//      double pv = p * vr;
//
//      //return sqrt((r - pv) * (r + pv)) / r / vr; // Tau Function
//      return 1.0 / sqrt((r - pv) * (r + pv));    // Turning Asymptote
//      //return pv / sqrt((r - pv) * (r + pv)) / r; // Distance
//      //return r / sqrt((r - pv) * (r + pv)) / vr; // Time
//    }
//
//    //! Zeros the function call count.
//    void      resetFCCount() {fccnt = 0;};
//
//    //! Returns the function call count.
//    int       getFCCount() const {return fccnt;};
//
//    //! Set velocity model
//    void      setF(int fin) {f = fin;};
//
//    //! Set turning radius
//    void      setRTurn(double ftrn)
//    {
//      rturn = ftrn * (rbot() - rtop()) + rtop();
//      p = rturn / v(rturn);
//    };
//
//    //! Get a limit.
//    double    getLimit(double fin) const
//    {
//      return fin * (rtop() - rturn) + rturn;
//    };
//
//    // mantle1 p vel    from r = 6336 to 6251
//    double    v1(double r) const
//    {
//      double rn = r / 6371.0;
//      return 8.78541  -  0.74953 * rn;
//    };
//
//    // mantle2 p vel    from r = 6251 to 6161
//    double    v2(double r) const
//    {
//      double rn = r / 6371.0;
//      return 25.41389 - 17.69722 * rn;
//    };
//
//    // mantle3 p vel    from r = 6161 to 5961
//    double    v3(double r) const
//    {
//      double rn = r / 6371.0;
//      return 30.78765 - 23.25415 * rn;
//    };
//
//    // mantle4 p vel    from r = 5961 to 5711
//    double    v4(double r) const
//    {
//      double rn = r / 6371.0;
//      return 29.38896 - 21.40656 * rn;
//    };
//
//    // mantle5 p vel    from r = 5711 to 5611
//    double    v5(double r) const
//    {
//      double rn = r / 6371.0;
//      return 25.96984 - 16.93412 * rn;
//    };
//
//    // mantle p vel     from r = 5611 to 3631
//    double    v6(double r) const
//    {
//      double rn = r / 6371.0;
//      return 25.1486 + rn * (-41.1538 + rn * (51.9932 + rn * -26.6083));
//    };
//
//    // c-m trns p vel   from r = 3631 to 3482
//    double    v7(double r) const
//    {
//      double rn = r / 6371.0;
//      return 14.49470 - 1.47089 * rn;
//    };
//
//    // outer core p vel from r = 3482 to 1217.1
//    double    v8(double r) const
//    {
//      double rn = r / 6371.0;
//      return 10.03904 + rn * (3.75665 + rn * -13.67046);
//    };
//
//    // inner core p vel from r = 1217.1 to 0
//    double    v9(double r) const
//    {
//      double rn = r / 6371.0;
//      return 11.24094 +  rn * rn * -4.09689;
//    };
//
//    double    rtop() const
//    {
//      if (f < 5)
//      {
//        if (f < 3)
//        {
//          if (f == 1)
//            return 6336.0;
//          else
//            return 6251.0;
//        }
//        else if (f == 3)
//          return 6161.0;
//        else
//          return 5961.0;
//      }
//      else if (f < 7)
//      {
//        if (f == 5)
//          return 5711.0;
//        else
//          return 5611.0;
//      }
//      else if (f < 9)
//      {
//        if (f == 7)
//          return 3631.0;
//        else
//          return 3482.0;
//      }
//      else
//        return 1217.1;
//
//      return 0.0;
//    };
//
//    double    rbot() const
//    {
//      if (f < 5)
//      {
//        if (f < 3)
//        {
//          if (f == 1)
//            return 6251.0;
//          else
//            return 6161.0;
//        }
//        else if (f == 3)
//          return 5961.0;
//        else
//          return 5711.0;
//      }
//      else if (f < 7)
//      {
//        if (f == 5)
//          return 5611.0;
//        else
//          return 3631.0;
//      }
//      else if (f < 9)
//      {
//        if (f == 7)
//          return 3482.0;
//        else
//          return 1217.1;
//      }
//      else
//        return 0.0;
//
//      return 0.0;
//    };
//
//    double    v(double r) const
//    {
//      if (f < 5)
//      {
//        if (f < 3)
//        {
//          if (f == 1)
//            return v1(r);
//          else
//            return v2(r);
//        }
//        else if (f == 3)
//          return v3(r);
//        else
//          return v4(r);
//      }
//      else if (f < 7)
//      {
//        if (f == 5)
//          return v5(r);
//        else
//          return v6(r);
//      }
//      else if (f < 9)
//      {
//        if (f == 7)
//          return v7(r);
//        else
//          return v8(r);
//      }
//      else
//        return v9(r);
//
//      return 0.0;
//    };
//
//  private:
//
//    //! \brief The integrand function call count. Used to examine the
//    //! number of times the operator()(double x) function is called.
//    int fccnt;
//
//    int    f;
//    double p;
//
//    double rturn;
//};

} // end namespace util

#endif // INTEGRATEFUNCTION_H
