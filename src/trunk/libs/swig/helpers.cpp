/***************************************************************************
 * (C) 2006 - GFZ-Potsdam
 *
 * Author: Andres Heinloo
 * Email: geofon_devel@gfz-potsdam.de
 * $Date: 2007-02-04 03:10:47 +0100 (Sun, 04 Feb 2007) $
 * $Revision: 856 $
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

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/optional.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/event.h>

#include "helpers.h"

namespace Seiscomp {
namespace Sc3py {
namespace _private {

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace Seiscomp::DataModel;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool _writeobj(BaseObject* obj, const char* file) {
	XMLArchive ar;

	if(!ar.create(file))
		return false;

	ar << obj;
	ar.close();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool _readobj(BaseObject* obj, const char* file) {
	XMLArchive ar;

	if(!ar.open(file))
		return false;

	ar >> obj;
	ar.close();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

_DatabaseQuery::_DatabaseQuery(DatabaseInterface* dbDriver)
	: DatabaseQuery(dbDriver) {
}

_DatabaseQuery::~_DatabaseQuery() {
}

DatabaseIterator _DatabaseQuery::getEvents(int limit, int offset, bool newestFirst,
	const OPT(Time)& minTime, const OPT(Time)& maxTime,
	const OPT(double)& minLatitude, const OPT(double)& maxLatitude,
	const OPT(double)& minLongitude, const OPT(double)& maxLongitude,
	const OPT(double)& minMagnitude, const OPT(int)& minArrivals) {

	ostringstream query;
	query << "SELECT PublicObject.publicID,Event.* FROM Event LEFT JOIN PublicObject USING (_oid) ";
	query << "LEFT JOIN PublicObject AS POrigin ON POrigin.publicID=Event.preferredOriginID ";
	query << "LEFT JOIN Origin ON Origin._oid=POrigin._oid ";

	if (minMagnitude) {
		query << "LEFT JOIN PublicObject AS PMagnitude ON PMagnitude.publicID=Event.preferredMagnitudeID ";
		query << "LEFT JOIN Magnitude ON Magnitude._oid=PMagnitude._oid ";
	}

	query << "WHERE TRUE ";
	if (minTime)
		query << "AND Origin.time_value>=" << toString(*minTime) << " ";
	if (maxTime)
		query << "AND Origin.time_value<=" << toString(*maxTime) << " ";
	if (minLatitude)
		query << "AND Origin.latitude>=" << (*minLatitude) << " ";
	if (maxLatitude)
		query << "AND Origin.latitude<=" << (*maxLatitude) << " ";
	if (minLongitude)
		query << "AND Origin.longitude>=" << (*minLongitude) << " ";
	if (maxLongitude)
		query << "AND Origin.longitude<=" << (*maxLongitude) << " ";
	if (minMagnitude)
		query << "AND Magnitude.magnitude_value>=" << (*minMagnitude) << " ";
	if (minArrivals)
		query << "AND (SELECT COUNT(_parent_oid) FROM Arrival WHERE Arrival._parent_oid=Origin._oid)>=" << (*minArrivals) << " ";
	
	query << "ORDER BY ";
	if (newestFirst)
		query << "Origin.time_value DESC, Origin.time_value_ms DESC ";
	else
		query << "Origin.time_value ASC, Origin.time_value_ms ASC ";
	
	query << "LIMIT " << offset << ", " << limit;

	return getObjectIterator(query.str(), Event::TypeInfo());
}

} // namespace _private
} // namespace Sc3py
} // namespace Seiscomp

