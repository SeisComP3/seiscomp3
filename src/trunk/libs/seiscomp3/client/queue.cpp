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


#define SEISCOMP_COMPONENT Queue

#include <seiscomp3/client/queue.h>
#include <seiscomp3/client/queue.ipp>
#include <seiscomp3/client.h>


namespace Seiscomp {
namespace Client {

template class SC_SYSTEM_CLIENT_API ThreadedQueue<Core::BaseObject*>;

}
}
