//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
//- retains certain rights in this software.
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

#ifndef CPU_TIMER_H
#define CPU_TIMER_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <ctime>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Wall clock and cpu timing information.
 *
 * Manages real (wall clock) and cpu timing information and elapsed timing
 * information.
 */
class GEOTESS_EXP_IMP CpuTimer
{
  private:

    // Last real time set when init_timer was called or 'this' CpuTimer was
    // constructed (msec)
    double						cpuRealTime;

    // Last cpu time set when init_timer was called or 'this' CpuTimer was
    // constructed (msec)
    double						cpuCPUTime;

  public:

    // Default constructor initialies timer
											CpuTimer();

    // Initializes timer
    void							initTimer();

    /**
     * Returns the current CPU time in msec since process start.
     */
    static double			getCurrCPUTime()
    									{ return (double) 1000.0 * clock() / CLOCKS_PER_SEC; }

    /**
     * Returns the current real time (wall clock) in msec since 1970.
     */
    static double			getCurrRealTime()
    									{ return (double) 1000.0 * time(NULL); }

    // Returns cpu time (ms) since last init_timer call. The second
    // function also reinitializes cpuCPUTime.
    double						cpuTime();
    double						cpuTimeInit();

    // Returns real time (ms) since last init_timer call. The second
    // function also reinitializes cpuRealTime.
    double						realTime();
    double						realTimeInit();

    // Static elapsed time string output functions given an input elapsed
    // time in msec.
    static string			elapsedTimeString(double tm);
    static string			elapsedTimeStringFraction(double tm);
    static string			elapsedTimeStringFractionAbbrvUnits(double tm);

    // Returns the current time string
    static string			now();
};

} // end namespace geotess

#endif  // CPU_TIMER_H
