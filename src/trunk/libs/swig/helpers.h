/***************************************************************************
 * (C) 2006 - GFZ-Potsdam
 *
 * Author: Andres Heinloo
 * Email: geofon_devel@gfz-potsdam.de
 * $Date: 2007-02-03 19:31:08 +0100 (Sat, 03 Feb 2007) $
 * $Revision: 852 $
 * $LastChangedBy: andres $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ***************************************************************************/

#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <cstdio>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/datamodel/databasequery.h>

namespace Seiscomp {
namespace Sc3py {
namespace _private {

bool _readobj(Seiscomp::Core::BaseObject* obj, const char* file);
bool _writeobj(Seiscomp::Core::BaseObject* obj, const char* file);

class _DatabaseQuery: public Seiscomp::DataModel::DatabaseQuery {
  public:
	//! Constructor
	_DatabaseQuery(Seiscomp::IO::DatabaseInterface* dbDriver);

	//! Destructor
	~_DatabaseQuery();

	Seiscomp::DataModel::DatabaseIterator getEvents(int limit, int offset, bool newestFirst,
		const OPT(Seiscomp::Core::Time)& minTime, const OPT(Seiscomp::Core::Time)& maxTime,
		const OPT(double)& minLatitude, const OPT(double)& maxLatitude,
		const OPT(double)& minLongitude, const OPT(double)& maxLongitude,
		const OPT(double)& minMagnitude, const OPT(int)& minArrivals);
};

} // namespace _private

using _private::_readobj;
using _private::_writeobj;
using _private::_DatabaseQuery;

} // namespace Sc3py
} // namespace Seiscomp

#endif

