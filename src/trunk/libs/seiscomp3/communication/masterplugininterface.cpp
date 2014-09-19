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

#include "masterplugininterface.h"

#include <seiscomp3/core/interfacefactory.ipp>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Communication::MasterPluginInterface, SC_SYSTEM_CLIENT_API);

namespace Seiscomp {
namespace Communication {

IMPLEMENT_SC_ABSTRACT_CLASS(MasterPluginInterface, "MasterPluginInterface");


} // namespace Communication
} // namespace Seiscomp
