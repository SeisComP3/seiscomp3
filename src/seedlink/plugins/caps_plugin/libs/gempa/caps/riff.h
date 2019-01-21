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


#ifndef __CAPS_IO_RIFF_H__
#define __CAPS_IO_RIFF_H__


#include <iostream>
#include <vector>
#include <fstream>
#include <stdint.h>

#include <gempa/caps/packet.h>


namespace Gempa {
namespace CAPS {
namespace RIFF {


struct ChunkHeader {
	char    chunkType[4]; /* Four charachter chunk type */
	int     chunkSize;    /* Chunk size in bytes */

	bool setChunkType(const char *type);
	bool isChunkType(const char *type) const;

	int  dataSize() const { return 8; }

	bool read(std::istream &input) { return get(*input.rdbuf()); }

	bool get(std::streambuf &input);
};

const int ChunkHeaderSize = 8;


class ChunkIterator {
	public:
		ChunkIterator();
		ChunkIterator(const std::string &filename);
		ChunkIterator(std::istream &input);

		//! Starts iterating over a file or stream
		void begin(const std::string &filename);
		void begin(std::istream &input);

		//! Jumps to the next chunk. Returns false,
		//! if no chunk is available
		bool next();

		//! Returns the current chunk header
		const ChunkHeader &header() const;

		//! Returns the file position pointing
		//! to the current chunk header
		size_t headerPos() const;

		//! Returns the file position pointing
		//! to the current chunk content
		size_t contentPos() const;

		//! Returns the current input stream
		std::istream &istream() const { return *_stream; }


	private:
		ChunkHeader    _header;
		std::istream  *_stream;
		size_t         _index;
		std::ifstream  _own;
};


struct Chunk {
	virtual ~Chunk();

	bool read(std::istream &input, int size) { return get(*input.rdbuf(), size); }

	virtual bool get(std::streambuf &input, int size) = 0;

	virtual int chunkSize() const = 0;
};


struct HeadChunk : Chunk {
	PacketDataHeader data;

	int chunkSize() const;

	bool get(std::streambuf &input, int size);
};


struct SIDChunk : Chunk {
	std::string networkCode;
	std::string stationCode;
	std::string locationCode;
	std::string channelCode;

	int chunkSize() const;

	bool get(std::streambuf &input, int size);
};


template <int SIZE_T, bool BigEndian>
struct CPtrChunk : Chunk {
	const char *data;
	int size;

	CPtrChunk(const char* d, int len);
	virtual ~CPtrChunk() {}

	int chunkSize() const;

	bool get(std::streambuf &input, int size);
};


template <int SIZE_T, bool BigEndian>
struct VectorChunk : Chunk {
	std::vector<char> &data;

	VectorChunk(std::vector<char> &d);

	// sampleOfs and sampleCount are not byte offsets but elements of
	// type T
	VectorChunk(std::vector<char> &d, int sampleOfs, int sampleCount);
	virtual ~VectorChunk() {}

	int chunkSize() const;

	bool get(std::streambuf &input, int size);

	int startOfs;
	int len;
};


}
}
}


#endif
