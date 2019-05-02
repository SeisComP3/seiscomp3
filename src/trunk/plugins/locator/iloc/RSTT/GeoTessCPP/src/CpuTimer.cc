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

#include "CpuTimer.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Default constructor.
 */
CpuTimer::CpuTimer()
{
  initTimer();
}

/**
 * Initializes real (wall clock) and cpu times to the current time.
 */
void CpuTimer::initTimer()
{
  // initialize real and cpu times

  cpuRealTime = getCurrRealTime();
  cpuCPUTime  = getCurrCPUTime();
}

/**
 * Returns the elapsed cpu time (msec) since the last time the cpu
 * time was initialized.
 */
double CpuTimer::cpuTime()
{
  return getCurrCPUTime() - cpuCPUTime;
}

/**
 * Returns the elapsed cpu time (msec) since the last time the cpu time was
 * initialized and reinitializes the cpu time to the current time.
 */
double CpuTimer::cpuTimeInit()
{
  double old_time = cpuCPUTime;
  cpuCPUTime = getCurrCPUTime();

  return (cpuCPUTime - old_time);
}

/**
 * Returns the elapsed real (wall clock) time (msec) since the last time the
 * real time was initialized.
 */
double CpuTimer::realTime()
{
  return getCurrRealTime() - cpuRealTime;
}

/**
 * Returns the elapsed real (wall clock) time (msec) since the last time the
 * realtime was initialized and reinitializes the real time to the current
 * time.
 */
double CpuTimer::realTimeInit()
{
  double old_time = cpuRealTime;
  cpuRealTime = getCurrRealTime();

  return (cpuRealTime - old_time);
}

/**
 * Returns the time string from the input milisecond number. The time string
 * will look like "#:##:##:##:#### days" if the input time is one day or
 * longer; "##:##:##:#### hrs" if the input time is less than a day but
 * greater than or equal to 1 hour; "#:##:#### min" if the input time is less
 * than an hour but greater than or equal to 1 minute; "#:#### sec" if the
 * input time is less than 1 minute but greater than or equal to 1 second; or
 * "# msec" if the input time is less than 1 second.
 *
 * @param tm
 *          The input time in miliseconds.
 * @return The equivalent time string.
 */
string CpuTimer::elapsedTimeString(double tm)
{
  string ts = "";
  int days, hrs, min, sec, msec;
  days = hrs = min = sec = msec = 0;

  // calculate hours, minutes, seconds and milliseconds

  if (tm > 86400000.0)
  {
    days = (int) (tm / 86400000.0);
    tm -= days * 86400000.0;
  }
  if (tm > 3600000.0)
  {
    hrs = (int) (tm / 3600000.0);
    tm -= hrs * 3600000.0;
  }
  if (tm > 60000.0)
  {
    min = (int) (tm / 60000.0);
    tm -= min * 60000.0;
  }
  if (tm > 1000.0)
  {
    sec = (int) (tm / 1000.0);
    tm -= sec * 1000.0;
  }
  msec = (int) tm;

  // find largest unit and form output string

  if (days > 0)
  {
    ts = CPPUtils::itos(days) + ":";
    if (hrs < 10) ts += "0";
    ts += CPPUtils::itos(hrs) + ":";
    if (min < 10) ts += "0";
    ts += CPPUtils::itos(min) + ":";
    if (sec < 10) ts += "0";
    ts += CPPUtils::itos(sec) + ":";
    if (msec < 10)
      ts += "00";
    else if (msec < 100) ts += "0";
    ts += CPPUtils::itos(msec) + " days";
  }
  else if (hrs > 0)
  {
    ts = CPPUtils::itos(hrs) + ":";
    if (min < 10) ts += "0";
    ts += CPPUtils::itos(min) + ":";
    if (sec < 10) ts += "0";
    ts += CPPUtils::itos(sec) + ":";
    if (msec < 10)
      ts += "00";
    else if (msec < 100) ts += "0";
    ts += CPPUtils::itos(msec) + " hrs";
  }
  else if (min > 0)
  {
    ts = CPPUtils::itos(min) + ":";
    if (sec < 10) ts += "0";
    ts += CPPUtils::itos(sec) + ":";
    if (msec < 10)
      ts += "00";
    else if (msec < 100) ts += "0";
    ts += CPPUtils::itos(msec) + " min";
  }
  else if (sec > 0)
  {
    ts = CPPUtils::itos(sec) + ":";
    if (msec < 10)
      ts += "00";
    else if (msec < 100) ts += "0";
    ts += CPPUtils::itos(msec) + " sec";
  }
  else
    ts = CPPUtils::itos(msec) + " msec";

  // return result

  return ts;
}

/**
 * Returns the elapsed time from the input number in miliseconds. If the
 * input number is larger than 1 day of miliseconds then it is output as
 * dt/1000/60/60/24 days, if larger than 1 hour it is output as dt/1000/60/60
 * hours, if larger than 1 minute it is output as dt/1000/60 minutes, if
 * larger than 1 second it is output as dt/1000 seconds, otherwise it is
 * output as dt miliseconds.
 *
 * @param dt The input time in miliseconds to be output as days, hours,
 *           minutes, seconds, miliseconds depending on its magnitude.
 * @return The formated string with units.
 */
string CpuTimer::elapsedTimeStringFraction(double dt)
{
  string units = "miliseconds";
  if (dt > 1000.0)
  {
    dt /= 1000.0;
    units = "seconds";

    if (dt >= 60.0)
    {
      dt /= 60.0;
      units = "minutes";

      if (dt >= 60.0)
      {
        dt /= 60.0;
        units = "hours";

        if (dt >= 24.0)
        {
          dt /= 24.0;
          units = "days";
        }
      }
    }
  }

	return CPPUtils::dtos(dt, "%.2f") + " " + units;
}

/**
 * Returns the elapsed time from the input number in miliseconds. If the
 * input number is larger than 1 day of miliseconds then it is output as
 * dt/1000/60/60/24 days, if larger than 1 hour it is output as dt/1000/60/60
 * hours, if larger than 1 minute it is output as dt/1000/60 minutes, if
 * larger than 1 second it is output as dt/1000 seconds, otherwise it is
 * output as dt miliseconds.
 *
 * @param dt The input time in miliseconds to be output as days, hours,
 *           minutes, seconds, miliseconds depending on its magnitude.
 * @return The formated string with units.
 */
string CpuTimer::elapsedTimeStringFractionAbbrvUnits(double dt)
{
  string units = "msecs";
  if (dt > 1000.0)
  {
    dt /= 1000.0;
    units = "secs";

    if (dt >= 60.0)
    {
      dt /= 60.0;
      units = "mins";

      if (dt >= 60.0)
      {
        dt /= 60.0;
        units = "hrs";

        if (dt >= 24.0)
        {
          dt /= 24.0;
          units = "days";
        }
      }
    }
  }

	return CPPUtils::dtos(dt, "%.2f") + " " + units;
}

/**
 * Returns the current time as a string Www Mmm dd hh:mm:ss yyyy
 * Where Www is the weekday, Mmm is the month in letters, dd is the day of the
 * month, hh:mm:ss is the time, and yyyy is the year.
 */
string CpuTimer::now()
{
  // get current time in time_stamp and decode into tm structure

  time_t time_stamp = time(NULL);
	struct tm * timeinfo;
  timeinfo = localtime(&time_stamp);
  string tmstr = asctime(timeinfo);
  CPPUtils::removeEOL(tmstr);
	return tmstr;
}

} // end namespace geotess
