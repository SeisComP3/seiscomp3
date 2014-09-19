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



#include "criterion.h"

#include <algorithm>




namespace Seiscomp {
namespace Applications {




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Criterion::LatitudeRange& Criterion::latitudeRange() const
{
	return _latitudeRange;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Criterion::setLatitudeRange(double lat0, double lat1)
{
	_latitudeRange = std::make_pair(lat0, lat1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Criterion::LongitudeRange& Criterion::longitudeRange() const
{
	return _longitudeRange;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Criterion::setLongitudeRange(double lon0, double lon1)
{
	_longitudeRange = std::make_pair(lon0, lon1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Criterion::MagnitudeRange& Criterion::magnitudeRange() const
{
	return _magnitudeRange;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Criterion::setMagnitudeRange(double mag0, double mag1)
{
	_magnitudeRange = std::make_pair(mag0, mag1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Criterion::arrivalCount() const
{
	return _arrivalCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Criterion::setArrivalCount(size_t count)
{
	_arrivalCount = count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Criterion::AgencyIDs& Criterion::agencyIDs() const
{
	return _agencyIDs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Criterion::setAgencyIDs(const AgencyIDs& ids)
{
	_agencyIDs = ids;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Criterion::isInLatLonRange(double lat, double lon)
{
	bool result = true;
	if (lat < _latitudeRange.first || lat > _latitudeRange.second)
	{
		result = false;
		std::ostringstream ss;
		ss << "Latitude " << lat << " not in [" << _latitudeRange.first << ":" << _latitudeRange.second << "]";
		append(ss.str());
	}

	if (lon < _longitudeRange.first || lon > _longitudeRange.second)
	{
		result = false;
		std::ostringstream ss;
		ss << "Longitude " << lon << " not in [" << _longitudeRange.first << ":" << _longitudeRange.second << "]";
		append(ss.str());
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Criterion::isInMagnitudeRange(double mag)
{
	bool result = false;
	if (mag >= _magnitudeRange.first && mag <= _magnitudeRange.second)
		result = !result;
	else
	{
		std::ostringstream ss;
		ss << "Magnitude " << mag << " not in [" << _magnitudeRange.first << ":" << _magnitudeRange.second << "]";
		append(ss.str());
	}
	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Criterion::checkArrivalCount(size_t count)
{
	bool result = false;
	if (count >= _arrivalCount)
		result = !result;
	else
	{
		std::ostringstream ss;
		ss << "Arrival count " << count << " < " << _arrivalCount;
		append(ss.str());
	}
	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Criterion::checkAgencyID(const std::string& id)
{
	// Every agency is eligible
	if (_agencyIDs.empty())
		return true;

	bool result = false;
	AgencyIDs::const_iterator it = std::find(_agencyIDs.begin(), _agencyIDs.end(), id);
	if (it != _agencyIDs.end())
		result = !result;
	else
	{
		std::ostringstream ss;
		ss << "AgencyId " << id << " not in ";
		for (size_t i = 0; i < _agencyIDs.size(); ++i)
			ss << _agencyIDs[i] << " ";
		append(ss.str());
	}
	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





} // namespace Applications
} // namespace Seiscomp
