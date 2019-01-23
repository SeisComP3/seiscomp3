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
//- Module:        $RCSfile: IntegrateFunction.cc,v $
//- Creator:       Jim Hipp
//- Creation Date: April 17, 2007
//- Revision:      $Revision: 1.7 $
//- Last Modified: $Date: 2011/10/07 13:14:57 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

// **** _SYSTEM INCLUDES_ ******************************************************

#include <cmath>
#include <iomanip>

// **** _LOCAL INCLUDES_ *******************************************************

#include "IntegrateFunction.h"

// **** _BEGIN UTIL NAMESPACE_ *************************************************

namespace util {

// **** _STATIC INITIALIZATIONS_************************************************

//template class IntegrateFunction<TestIntegrateFunction>;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

// Standard Constructor.
//
// Since IntegrateFunction requires a reference assignment for the function
// ifF, the default constructor is not supported.
template<class F>
IntegrateFunction<F>::IntegrateFunction(F& f, double tol) : ifF(f), ifTol(tol)
{
}

// Copy Constructor.
template<class F>
IntegrateFunction<F>::IntegrateFunction(const IntegrateFunction<F>& ifnctn) :
                      ifF(ifnctn.ifF), ifTol(ifnctn.ifTol)
{
}

// Destructor.
template<class F>
IntegrateFunction<F>::~IntegrateFunction()
{
}

// Assignment Operator.
template<class F>
IntegrateFunction<F>& IntegrateFunction<F>::
                      operator=(const IntegrateFunction<F>& ifnctn)
{
  ifTol = ifnctn.ifTol;

  return *this;
}

//getTest4(int f, double ftrn, double fa, double fb, double tol)
// f     = velocity function
// ftrn  = is fractional radius where rtrn = ftrn * (rtop - rbot) + rbot
// fa    = is fractional radius where ra = fa * (rtop - rtrn) + rtrn above the turn radius
// fb    = is fractional radius where rb = fb * (rtop - rtrn) + rtrn above the turn radius
//         (Note:  fb > fa, fb == 1 gives rb == rtop)
//template<class F>
//double IntegrateFunction<F>::getTest(int f, double ftrn, double tol)
//{
//  double numY;
//
//  // create test function and IntegrateFunction ... reset function
//  // call count to zero
//
//  TestIntegrateFunction tif;
//  IntegrateFunction<TestIntegrateFunction> iftif(tif, tol);
//  tif.resetFCCount();
//
//  // retrieve analytic and numerical solutions
//
//  tif.setF(f);
//
//  bool useTauFunction = false;
//  tif.setRTurn(ftrn);
//  double a = tif.getLimit(0.0);
//  double b = tif.getLimit(1.0);
//  if (useTauFunction)
//  {
//    if ((f == 9) && (a == 0.0))
//      numY = iftif.integrateAOpenS(a, b);
//    else
//      numY = iftif.integrateClosed(a, b);
//  }
//  else
//  {
//    numY = iftif.integrateAOpenS(a, b);
//  }
//
//  // output results
//
//  cout << "count: " << tif.getFCCount() << std::setprecision(14)
//       << ",  numeric: " << numY << endl;
//
//  return numY;
//}

} // end namespace util
