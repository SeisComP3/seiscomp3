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


#include <stdint.h>
#include <cstring>
#include <cstdio>

#include <seiscomp3/io/records/sacrecord.h>
#include <seiscomp3/io/records/sac.h>
#include <seiscomp3/core/arrayfactory.h>
#include <seiscomp3/core/typedarray.h>


namespace Seiscomp {
namespace IO {

namespace {

/* a SAC structure containing all null values */
struct sac sac_null = {
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
{ '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }
};


void copy_buf(std::string &str, int w, const char *buf) {
	str = std::string();
	while ( w-- ) {
		if ( isspace(*buf) ) break;
		str += *buf++;
	}
}


template <typename T>
struct Swapper4 {
	static void Take(T &v) {
		uint32_t *tmp = (uint32_t*)&v;
		*tmp = ((*tmp << 24) & 0xFF000000) |
		       ((*tmp << 8)  & 0x00FF0000) |
		       ((*tmp >> 8)  & 0x0000FF00) |
		       ((*tmp >> 24) & 0x000000FF);
	}

	static void Take(T *ar, int len) {
		for ( int i = 0; i < len; ++i )
			Take(ar[i]);
	}
};


void swap(struct sac &header) {
	int32_t *data = (int32_t*)&header;
	Swapper4<int32_t>::Take(data, 110);
}


}


REGISTER_RECORD(SACRecord, "sac");

SACRecord::SACRecord(const std::string &net, const std::string &sta,
                     const std::string &loc, const std::string &cha,
                     Core::Time stime, double fsamp, int tqual,
                     Array::DataType dt, Hint h)
: Record(dt, h, net, sta, loc, cha, stime, 0, fsamp, tqual) {}


SACRecord::SACRecord(const SACRecord &other) : Record(other) {
	_data = other._data;
	_nsamp = _data?_data->size():0;
	_datatype = _data?_data->dataType():Array::DT_QUANTITY;
}


SACRecord::SACRecord(const Record &rec) : Record(rec) {
	_data = NULL;
	const Array *ar = rec.data();
	if ( ar )
		_data = ar->clone();

	_nsamp = _data?_data->size():0;
	_datatype = _data?_data->dataType():Array::DT_QUANTITY;
}


SACRecord::~SACRecord() {}


SACRecord &SACRecord::operator=(const SACRecord &other) {
	Record::operator=(other);

	_data = other._data;
	_nsamp = _data?_data->size():0;
	_datatype = _data?_data->dataType():Array::DT_QUANTITY;

	return *this;
}


Array* SACRecord::data() {
	return _data.get();
}


const Array* SACRecord::data() const {
	return _data.get();
}


const Array* SACRecord::raw() const {
	return NULL;
}


void SACRecord::setData(Array* data) {
	_data = data;
	_nsamp = _data?_data->size():0;
	_datatype = _data?_data->dataType():Array::DT_QUANTITY;
}


void SACRecord::setData(int size, const void *data, Array::DataType datatype) {
	_data = ArrayFactory::Create(datatype, datatype, size, data);
	_nsamp = _data?_data->size():0;
	_datatype = _data?_data->dataType():Array::DT_QUANTITY;
}


SACRecord *SACRecord::copy() const {
	return new SACRecord((const Record &)*this);
}


void SACRecord::saveSpace() const {
	_data = NULL;
}


void SACRecord::read(std::istream &in) {
	std::streamsize header_size = sizeof(struct sac);
	struct sac header = sac_null;
	// 632 byte

	if ( !in.read((char*)&header, header_size) )
		throw Core::StreamException("stream underflow while reading SAC header");

	bool swapped = false;

	if ( header.iftype != SAC_ITIME ) {
		int numberOfComponents = 0;
		switch ( header.iftype ) {
			case SAC_IRLIM:
			case SAC_IAMPH:
			case SAC_IXY:
				numberOfComponents = 2;
				break;
			case SAC_IXYZ:
				numberOfComponents = 3;
				break;
		}

		if ( numberOfComponents == 0 ) {
			swap(header);

			if ( header.iftype != SAC_ITIME ) {
				switch ( header.iftype ) {
					case SAC_IRLIM:
					case SAC_IAMPH:
					case SAC_IXY:
						numberOfComponents = 2;
						break;
					case SAC_IXYZ:
						numberOfComponents = 3;
						break;
				}

				// Skip the rest of the record and jump to the beginning of the
				// next one.
				std::streamsize bytesToSkip = 4*numberOfComponents*header.npts;
				in.seekg(bytesToSkip, std::ios_base::cur);

				throw Core::TypeException("SAC record is not a time series");
			}

			swapped = true;
		}
		else {
			std::streamsize bytesToSkip = 4*numberOfComponents*header.npts;
			in.seekg(bytesToSkip, std::ios_base::cur);

			throw Core::TypeException("SAC record is not a time series");
		}
	}

	// reference time
	//
	// This time can be anything. Starting time of the trace, origin time
	// of the event, whatever. All relative times in the SAC header refer
	// to this time.
	Core::Time reftime;

	reftime = Core::Time(header.nzhour*3600+header.nzmin*60+header.nzsec, header.nzmsec*1000);
	reftime += Core::Time::FromYearDay(header.nzyear, header.nzjday);

	setStartTime(reftime + Core::TimeSpan(header.b));

	copy_buf(_net, 8, header.knetwk);
	copy_buf(_sta, 8, header.kstnm);
	copy_buf(_loc, 8, header.khole);
	copy_buf(_cha, 8, header.kcmpnm);

	_fsamp = 1.0 / header.delta;

	FloatArrayPtr ar = new FloatArray(header.npts);

	std::streamsize n = ar->size()*ar->elementSize();

	if ( !in.read((char*)ar->typedData(), n) )
		throw Core::StreamException("stream underflow while reading SAC time series");

	if ( swapped )
		Swapper4<float>::Take(ar->typedData(), ar->size());

	setData(ar.get());
}


void SACRecord::write(std::ostream &out) {
	if ( _data == NULL ) return;

	FloatArrayPtr ar = FloatArray::Cast(_data);
	if ( ar == NULL ) {
		ar = (FloatArray*)_data->copy(Array::FLOAT);
		if ( ar == NULL )
			throw Core::TypeException("SAC record float conversion error");
	}

	const size_t header_size = sizeof(struct sac);
	struct sac header = sac_null;

	// File type
	header.iftype = SAC_ITIME;

	// Start time
	header.b = 0;

	int year, yday, hour, min, sec, usec;
	_stime.get2(&year, &yday, &hour, &min, &sec, &usec);

	header.nzyear = year;
	header.nzjday = yday;
	header.nzhour = hour;
	header.nzmin = min;
	header.nzsec = sec;
	header.nzmsec = usec / 1000;

	header.delta = (float)(1.0 / _fsamp);

	strncpy(header.knetwk, _net.c_str(), 8);
	strncpy(header.kstnm, _sta.c_str(), 8);
	strncpy(header.khole, _loc.c_str(), 8);
	strncpy(header.kcmpnm, _cha.c_str(), 8);

	header.npts = ar->size();

	if ( !out.write((char*)&header, header_size) )
		throw Core::StreamException("stream error while writing SAC header");

	if ( !out.write((char*)ar->typedData(), ar->elementSize()*ar->size()) )
		throw Core::StreamException("stream error while writing SAC time series");
}


}
}
