/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/



#include "filter.h"

namespace Seiscomp
{
namespace Applications
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::map<std::string, Filter*> Filter::_Registry;
std::map<std::string, Filter::Operator> Filter::_Operators;
bool OriginFilter::_wasAccepted = false;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_MESSAGE_FILTER(PickFilter)
IMPLEMENT_MESSAGE_FILTER(AmplitudeFilter)
IMPLEMENT_MESSAGE_FILTER(OriginFilter)
IMPLEMENT_MESSAGE_FILTER(ArrivalFilter)
IMPLEMENT_MESSAGE_FILTER(EventFilter)
IMPLEMENT_MESSAGE_FILTER(StationMagnitudeFilter)
IMPLEMENT_MESSAGE_FILTER(MagnitudeFilter)
IMPLEMENT_MESSAGE_FILTER(WaveformQualityFilter)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Applications
} // namespace Seiscomp

