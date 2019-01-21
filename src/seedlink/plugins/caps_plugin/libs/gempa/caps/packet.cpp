/***************************************************************************
 * libcapsclient
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#include <gempa/caps/packet.h>

#include <cstring>


namespace Gempa {
namespace CAPS {


bool PacketDataHeader::setUOM(const char *type) {
	int i;

	if ( type != NULL ) {
		for ( i = 0; i < 4; ++i ) {
			if ( type[i] == '\0' ) break;
			unitOfMeasurement.str[i] = type[i];
		}

		// Input type must not have more than 4 characters
		if ( i == 3 && type[i] != '\0' && type[i+1] != '\0' ) {
			memset(unitOfMeasurement.str, '\0', 4);
			return false;
		}
	}
	else
		i = 0;

	// Pad with null bytes
	for ( ; i < 4; ++i )
		unitOfMeasurement.str[i] = '\0';

	return true;
}


std::string PacketDataHeader::uom(char fill) const {
	std::string s;
	for ( int i = 0; i < 4; ++i ) {
		if ( unitOfMeasurement.str[i] == '\0' ) break;
		s += unitOfMeasurement.str[i];
	}

	if ( s.size() < 4 && fill != '\0' ) {
		for ( int i = s.size(); i < 4; ++i )
			s += fill;
	}

	return s;
}


bool PacketDataHeader::operator!=(const PacketDataHeader &other) const {
	return version != other.version ||
	       packetType != other.packetType ||
	       unitOfMeasurement.ID != other.unitOfMeasurement.ID;
}

bool DataRecord::Header::put(std::streambuf &buf) const {
	Endianess::Writer put(buf);
	char dt = (char)dataType;
	put(dt);

	put(samplingTime.year);
	put(samplingTime.yday);
	put(samplingTime.hour);
	put(samplingTime.minute);
	put(samplingTime.second);
	put(samplingTime.usec);
	put(samplingFrequencyNumerator);
	put(samplingFrequencyDenominator);

	return put.good;
}


void DataRecord::Header::setSamplingTime(const Time &ts) {
	int year, yday, hour, min, sec, usec;
	ts.get2(&year, &yday, &hour, &min, &sec, &usec);
	samplingTime.year = year;
	samplingTime.yday = yday;
	samplingTime.hour = hour;
	samplingTime.minute = min;
	samplingTime.second = sec;
	samplingTime.usec = usec;
}


bool DataRecord::Header::compatible(const Header &other) const {
	return dataType == other.dataType &&
	       samplingFrequencyNumerator == other.samplingFrequencyNumerator &&
	       samplingFrequencyDenominator == other.samplingFrequencyDenominator;
}


bool DataRecord::Header::operator!=(const Header &other) const {
	return dataType != other.dataType ||
	       samplingFrequencyNumerator != other.samplingFrequencyNumerator ||
	       samplingFrequencyDenominator != other.samplingFrequencyDenominator;
}

DataRecord::~DataRecord() {}

}
}
