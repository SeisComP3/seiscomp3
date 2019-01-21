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


#ifndef __GEMPA_CAPS_PACKET_H__
#define __GEMPA_CAPS_PACKET_H__


#include <gempa/caps/endianess.h>
#include <gempa/caps/datetime.h>


#include <boost/shared_ptr.hpp>

#include <stdint.h>
#include <string>
#include <vector>



namespace Gempa {
namespace CAPS {


enum PacketType {
	UnknownPacket = 0,
	RawDataPacket,
	MSEEDPacket,
	ANYPacket,
	RTCM2Packet,
	MetaDataPacket,
	FixedRawDataPacket,
	PacketTypeCount
};


enum DataType {
	DT_Unknown = 0,
	DT_DOUBLE  = 1,
	DT_FLOAT   = 2,
	DT_INT64   = 100,
	DT_INT32   = 101,
	DT_INT16   = 102,
	DT_INT8    = 103
};


union UOM {
	char    str[4];
	int32_t ID;
};


struct TimeStamp {
	int16_t  year;   /* year, eg. 2003                   */
	uint16_t yday;   /* day of year (1-366)              */
	uint8_t  hour;   /* hour (0-23)                      */
	uint8_t  minute; /* minute (0-59)                    */
	uint8_t  second; /* second (0-59), 60 if leap second */
	uint8_t  unused; /* unused byte */
	int32_t  usec;   /* microsecond (0-999999)           */
};


struct PacketDataHeader {
	PacketDataHeader() : version(1), packetType(UnknownPacket) {
		unitOfMeasurement.ID = 0;
	}

	uint16_t   version;
	PacketType packetType;
	UOM        unitOfMeasurement;

	bool setUOM(const char *type);
	std::string uom(char fill = '\0') const;

	bool operator!=(const PacketDataHeader &other) const;

	int dataSize() const {
		return sizeof(version) + sizeof((char)packetType) +
		       sizeof(unitOfMeasurement.ID);
	}

	bool put(std::streambuf &buf) {
		Endianess::Writer put(buf);

		char type = (char)packetType;

		put(version);
		put(type);
		put(unitOfMeasurement.ID);

		return put.good;
	}
};

enum StreamIDComponent {
	NetworkCode  = 0,
	StationCode  = 1,
	LocationCode = 2,
	ChannelCode  = 3,
	StreamIDComponentSize
};


struct PacketHeaderV1 {
	uint8_t  SIDSize[4]; /* number of bytes of stream ID components */
	uint16_t size;   /* number of data bytes */

	bool put(std::streambuf &buf) {
		Endianess::Writer put(buf);
		for ( int i = 0; i < 4; ++i )
			put(SIDSize[i]);

		put(size);

		return put.good;
	}

	size_t dataSize() const {
		return sizeof(uint8_t) * 4  + sizeof(size);
	}
};

struct PacketHeaderV2 {
	uint8_t  SIDSize[4]; /* number of bytes of stream ID components */
	uint32_t size;   /* number of data bytes */

	bool put(std::streambuf &buf) {
		Endianess::Writer put(buf);
		for ( int i = 0; i < 4; ++i )
			put(SIDSize[i]);

		put(size);

		return put.good;
	}
	size_t dataSize() const {
		return sizeof(uint8_t) * 4  + sizeof(size);
	}
};


struct ResponseHeader {
	uint16_t id;
	int32_t  size;

	bool get(std::streambuf &buf) {
		Endianess::Reader get(buf);

		get(id);
		get(size);

		return get.good;
	}
};


class DataRecord {
	public:
		typedef std::vector<char> Buffer;

		struct Header {
			DataType   dataType;
			TimeStamp  samplingTime;
			uint16_t   samplingFrequencyNumerator;
			uint16_t   samplingFrequencyDenominator;

			bool get(std::streambuf &buf) {
				Endianess::Reader get(buf);
				char dt;
				get(dt);
				dataType = (DataType)dt;

				get(samplingTime.year);
				get(samplingTime.yday);
				get(samplingTime.hour);
				get(samplingTime.minute);
				get(samplingTime.second);
				get(samplingTime.usec);
				get(samplingFrequencyNumerator);
				get(samplingFrequencyDenominator);

				return get.good;
			}

			bool put(std::streambuf &buf) const;

			int dataSize() const {
				return sizeof(samplingTime.year) +
				       sizeof(samplingTime.yday) +
				       sizeof(samplingTime.hour) +
				       sizeof(samplingTime.minute) +
				       sizeof(samplingTime.second) +
				       sizeof(samplingTime.usec) +
				       sizeof(samplingFrequencyNumerator) +
				       sizeof(samplingFrequencyDenominator) +
				       sizeof(char);
			}

			bool compatible(const Header &other) const;
			bool operator!=(const Header &other) const;

			void setSamplingTime(const Time &timestamp);
		};

		enum ReadStatusCode {
			RS_Error = 0,
			RS_Complete = 1,
			RS_Partial = 2,
			RS_BeforeTimeWindow = 3,
			RS_AfterTimeWindow = 4,
			RS_Max
		};

		class ReadStatus {
			public:
				ReadStatus() {}
				ReadStatus(ReadStatusCode code) : _code(code) {}
				ReadStatus(const ReadStatus &other) : _code(other._code) {}

			public:
				ReadStatus &operator=(const ReadStatus &other) { _code = other._code; return *this; }
				operator ReadStatusCode() const { return _code; }

			private:
				operator bool() { return _code != RS_Error; }

			private:
				ReadStatusCode _code;
		};

	public:
		virtual ~DataRecord();

		virtual const char *formatName() const = 0;

		virtual void readMetaData(std::streambuf &buf, int size,
		                          Header &header,
		                          Time &startTime, Time &endTime) = 0;

		//! Returns the data size in bytes if the current state would
		//! be written to a stream. Trimming has also to be taken into
		//! account while calculating the size.
		virtual size_t dataSize(bool withHeader = true) const = 0;

		//! Reads the packet from a streambuf and trims the data
		//! if possible to start and end. If maxBytes is greater
		//! than 0 then the record should not use more than this
		//! size of memory (in bytes). If not the complete record
		//! has been read, RS_Partial must be returned, RS_Complete
		//! otherwise.
		virtual ReadStatus get(std::streambuf &buf, int size,
		                       const Time &start = Time(), const Time &end = Time(),
		                       int maxBytes = -1) = 0;

		//! Returns the meta information of the data if supported
		virtual const Header *header() const = 0;

		//! Returns the start time of the record
		virtual Time startTime() const = 0;

		//! Returns the end time of the record
		virtual Time endTime() const = 0;

		//! Returns the packet type of the record
		virtual PacketType packetType() const = 0;

		/**
		 * @brief Returns the data vector to be filled by the caller
		 * @return The pointer to the internal buffer
		 */
		virtual Buffer *buffer() { return &_data; }

		/**
		 * @brief Returns pointer to raw data. If the record
		 * is compressed it will be uncompressed
		 * @return The pointer to the raw data
		 */
		virtual Buffer *data() { return &_data; }

	protected:
		Buffer                     _data;
};

typedef boost::shared_ptr<DataRecord> DataRecordPtr;


struct RawPacket {
	PacketDataHeader  header;
	std::string       SID[4];
	std::vector<char> data;
	DataRecord       *record;
};

struct MetaPacket {
	std::string        SID[4];
	PacketDataHeader   packetDataHeader;
	DataRecord        *record;
	DataRecord::Header recordHeader;
	Time               startTime;
	Time               endTime;
};

}
}


#endif
