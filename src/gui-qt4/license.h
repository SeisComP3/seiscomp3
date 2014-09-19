/***************************************************************************
* Copyright (C) 2009 BY GFZ Potsdam,                                       *
* EMail: seiscomp-devel@gfz-potsdam.de                                     * 
* The term "Non-Commercial Entity" is limited to the following:            *
* - Research institutes                                                    *
* - Public institutes dealing with natural hazards warnings                * 
*                                                                          *
* Provided that you qualify for a Non-Commercial Version License as        *
* specified above, and subject to the terms and conditions contained       *
* herein,GFZ Potsdam hereby grants the licensee, acting as an end user,    *
* an institute wide, non-transferable, non-sublicensable license, valid    *
* for an unlimited period of time, to install and use the Software, free   *
* of charge, for non-commercial purposes only. The licensee is allowed     *
* to develop own software modules based on the SeisComP3 libraries         *
* licensed under GPL and connect them with software modules under this     *
* license. The number of copies is for the licensee not limited within     *
* the user community of the licensee.                                      *
*                                                                          *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
* IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR    * 
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, * 
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        *
* SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.                 *
*                                                                          * 
* GFZ Potsdam reserve all rights not expressly granted to the licensee     *
* herein. Find more details of the license at geofon.gfz-potsdam.de        *
***************************************************************************/


#ifndef __SEISCOMP3_SEISMOLOGY_LICENSE_H__
#define __SEISCOMP3_SEISMOLOGY_LICENSE_H__


#include <iostream>


namespace Seiscomp{
namespace License {


bool isValid();
const char *text();

void printWarning(std::ostream &os);


}
}


#endif

