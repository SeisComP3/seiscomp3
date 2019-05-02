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


#ifndef __GEMPA_CAPS_MSEEDPACKET_H__
#define __GEMPA_CAPS_MSEEDPACKET_H__


#include <gempa/caps/packet.h>
#include <vector>


namespace Gempa {
namespace CAPS {


class MSEEDDataRecord : public DataRecord {
	public:
		MSEEDDataRecord();

		virtual const char *formatName() const;

		virtual void readMetaData(std::streambuf &buf, int size,
		                          Header &header,
		                          Time &startTime,
		                          Time &endTime);

		virtual const Header *header() const;
		virtual Time startTime() const;
		virtual Time endTime() const;

		virtual size_t dataSize(bool withHeader) const;

		virtual ReadStatus get(std::streambuf &buf, int size,
		                       const Time &start = Time(),
		                       const Time &end = Time(),
		                       int maxSize = -1);

		/**
		 * @brief Returns the packet type
		 * @return The packet type
		 */
		PacketType packetType() const { return MSEEDPacket; }

		/**
		 * @brief Initializes the internal data vector from the given buffer
		 * @param The buffer to read the data from
		 * @param The buffer size
		 */
		virtual void setData(const void *data, size_t size);

		void unpackHeader() { unpackHeader(_data.data(), _data.size()); }


	protected:
		Header _header;

		Time   _startTime;
		Time   _endTime;

		int    _dataType;


	private:
		void unpackHeader(char *data, size_t size);
};


}
}


#endif
