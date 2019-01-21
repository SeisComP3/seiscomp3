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


#include <gempa/caps/riff.h>
#include <gempa/caps/endianess.h>

#include <cstdio>
#include <cstring>


namespace Gempa {
namespace CAPS {
namespace RIFF {


bool ChunkHeader::setChunkType(const char *t) {
	int i;

	if ( t != NULL ) {
		for ( i = 0; i < 4; ++i ) {
			if ( t[i] == '\0' ) break;
			chunkType[i] = t[i];
		}

		// Input type must not have more than 4 characters
		if ( i == 3 && t[i] != '\0' && t[i+1] != '\0' ) {
			memset(chunkType, ' ', 4);
			return false;
		}
	}
	else
		i = 0;

	// Pad with whitespaces
	for ( ; i < 4; ++i )
		chunkType[i] = ' ';

	return true;
}


bool ChunkHeader::isChunkType(const char *t) const {
	for ( int i = 0; i < 4; ++i ) {
		if ( t[i] == '\0' && chunkType[i] == ' ' ) break;
		if ( t[i] != chunkType[i] ) return false;
	}
	return true;
}


bool ChunkHeader::get(std::streambuf &input) {
	Endianess::Reader get(input);
	get(chunkType, 4);
	get(chunkSize);
	return get.good;
}


ChunkIterator::ChunkIterator() : _stream(&_own) {}


ChunkIterator::ChunkIterator(const std::string &filename) {
	begin(filename);
}


ChunkIterator::ChunkIterator(std::istream &input) {
	begin(input);
}


void ChunkIterator::begin(const std::string &filename) {
	_stream = &_own;
	_index = 0;
	_own.open(filename.c_str());
	memset(&_header, 0, sizeof(_header));
}


void ChunkIterator::begin(std::istream &input) {
	_stream = &input;
	_index = input.tellg();
	_header.chunkSize = 0;
	memset(&_header, 0, sizeof(_header));
}


bool ChunkIterator::next() {
	while ( _stream->good() ) {
		// Jump to next header
		_stream->seekg(_index + _header.chunkSize);

		if ( !_header.read(*_stream) )
			break;

		//std::cout << " - "; std::cout.write(_header.chunkType, 4); std::cout << " : " << _header.chunkSize << std::endl;

		_index = _stream->tellg();
		return true;
	}

	return false;
}


const ChunkHeader &ChunkIterator::header() const {
	return _header;
}


size_t ChunkIterator::headerPos() const {
	return _index - _header.dataSize();
}


size_t ChunkIterator::contentPos() const {
	return _index;
}


Chunk::~Chunk() {}


bool HeadChunk::get(std::streambuf &input, int size) {
	Endianess::Reader r(input);

	r(data.version);

	char dt;
	r(dt);
	data.packetType = static_cast<PacketType>(dt);

	r(data.unitOfMeasurement.str, 4);

	return r.good;
}


int HeadChunk::chunkSize() const {
	return sizeof(data.version) +
	       sizeof(data.unitOfMeasurement.str) +
	       sizeof(char);
}


bool SIDChunk::get(std::streambuf &input, int size) {
	char tmp;
	int  count = size;
	Endianess::Reader r(input);

	r(&tmp, 1);
	networkCode.clear();
	while ( r.good && count-- ) {
		if ( tmp == '\0' ) break;
		networkCode += tmp;
		r(&tmp, 1);
	}

	if ( !r.good ) return false;

	r(&tmp, 1);
	stationCode.clear();
	while ( r.good && count-- ) {
		if ( tmp == '\0' ) break;
		stationCode += tmp;
		r(&tmp, 1);
	}

	if ( !r.good ) return false;

	r(&tmp, 1);
	locationCode.clear();
	while ( r.good && count-- ) {
		if ( tmp == '\0' ) break;
		locationCode += tmp;
		r(&tmp, 1);
	}

	if ( !r.good ) return false;

	r(&tmp, 1);
	channelCode.clear();
	while ( r.good && count-- ) {
		if ( tmp == '\0' ) break;
		channelCode += tmp;
		r(&tmp, 1);
	}

	return true;
}


int SIDChunk::chunkSize() const {
	return networkCode.size() + 1 +
	       stationCode.size() + 1 +
	       locationCode.size() + 1 +
	       channelCode.size();
}


template <int SIZE_T, bool BigEndian>
CPtrChunk<SIZE_T,BigEndian>::CPtrChunk(const char* d, int len) : data(d), size(len) {}


template <int SIZE_T, bool BigEndian>
int CPtrChunk<SIZE_T,BigEndian>::chunkSize() const {
	return size;
}


template <int SIZE_T, bool BigEndian>
bool CPtrChunk<SIZE_T,BigEndian>::get(std::streambuf &, int) {
	return false;
}


template <typename T, int SIZE_T, bool BigEndian>
struct CPtrWriter {
	static bool Take(std::streambuf &output, const T*, int count);
};


// To little endian
template <typename T, int SIZE_T>
struct CPtrWriter<T,SIZE_T,true> {
	static bool Take(std::streambuf &output, const T *data, int count) {
		Endianess::Writer w(output);
		for ( int i = 0; i < count; ++i ) {
			T tmp = Endianess::Swapper<T,true,SIZE_T>::Take(data[i]);
			w((const char*)&tmp, SIZE_T);
		}
		return w.good;
	}
};


template <typename T, int SIZE_T>
struct CPtrWriter<T,SIZE_T,false> {
	static bool Take(std::streambuf &output, const T *data, int count) {
		return (int)output.sputn((const char*)data, SIZE_T*count) == SIZE_T*count;
	}
};


template <int SIZE_T, bool BigEndian>
VectorChunk<SIZE_T,BigEndian>::VectorChunk(std::vector<char> &d)
: data(d), startOfs(-1), len(-1) {}


template <int SIZE_T, bool BigEndian>
VectorChunk<SIZE_T,BigEndian>::VectorChunk(std::vector<char> &d, int ofs, int count)
: data(d), startOfs(ofs), len(count) {}


template <int SIZE_T, bool BigEndian>
int VectorChunk<SIZE_T,BigEndian>::chunkSize() const {
	return data.size();
}


template <int SIZE_T, bool BigEndian>
bool VectorChunk<SIZE_T,BigEndian>::get(std::streambuf &input, int size) {
	int count = size/SIZE_T;

	if ( len >= 0 ) {
		if ( len > count )
			return false;

		count = len;
	}

	// Skip first samples (bytes = samples*sizeof(T))
	if ( startOfs > 0 )
		input.pubseekoff(startOfs*SIZE_T, std::ios_base::cur);

	Endianess::Reader r(input);

	data.resize(count*SIZE_T);
	r(&data[0], data.size());

	// Convert array to little endian
	Endianess::ByteSwapper<BigEndian,SIZE_T>::Take(&data[0], count);

	// Go the end of chunk
	if ( (int)data.size() < size )
		input.pubseekoff(size-data.size(), std::ios_base::cur);

	return r.good;
}


template struct CPtrChunk<1,false>;
template struct CPtrChunk<1,true>;
template struct CPtrChunk<2,false>;
template struct CPtrChunk<2,true>;
template struct CPtrChunk<4,false>;
template struct CPtrChunk<4,true>;
template struct CPtrChunk<8,false>;
template struct CPtrChunk<8,true>;

template struct VectorChunk<1,false>;
template struct VectorChunk<1,true>;
template struct VectorChunk<2,false>;
template struct VectorChunk<2,true>;
template struct VectorChunk<4,false>;
template struct VectorChunk<4,true>;
template struct VectorChunk<8,false>;
template struct VectorChunk<8,true>;


}
}
}
