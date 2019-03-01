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


#include <gempa/caps/mseedpacket.h>
#include <gempa/caps/log.h>
#include <gempa/caps/utils.h>

#include <cstdio>
#include <streambuf>
#include <iostream>
#include <libmseed.h>

namespace {

#define LOG_SID(FUNC, format,...)\
do {\
	char n[6];\
	char s[6];\
	char c[6];\
	char l[6];\
	ms_strncpclean(n, head.network, 2);\
	ms_strncpclean(s, head.station, 5);\
	ms_strncpclean(l, head.location, 2);\
	ms_strncpclean(c, head.channel, 3);\
	FUNC("[%s.%s.%s.%s] " format, n, s, l, c, ##__VA_ARGS__);\
} while(0)

}

namespace Gempa {
namespace CAPS {


MSEEDDataRecord::MSEEDDataRecord() {}

const char *MSEEDDataRecord::formatName() const {
	return "MSEED";
}


void MSEEDDataRecord::readMetaData(std::streambuf &buf, int size,
                                   Header &header,
                                   Time &startTime,
                                   Time &endTime) {
#if 1 // Set this to 1 to enable no-malloc fast MSeed meta parser
	fsdh_s head;

	if ( size <= 0 ) {
		CAPS_WARNING("read metadata: invalid size of record: %d", size);
		return;
	}

	if ( size < MINRECLEN || size > MAXRECLEN ) {
		CAPS_WARNING("read metadata: invalid MSEED record size: %d", size);
		return;
	}

	// Read first 32 byte
	size_t read = buf.sgetn((char*)&head, sizeof(head));
	if ( read < sizeof(head) ) {
		CAPS_WARNING("read metadata: input buffer underflow: only %d/%d bytes read",
		             (int)read, (int)sizeof(head));
		return;
	}

	if ( !MS_ISVALIDHEADER(((char*)&head)) ) {
		CAPS_WARNING("read metadata: invalid MSEED header");
		return;
	}

	bool headerswapflag = false;
	if ( !MS_ISVALIDYEARDAY(head.start_time.year, head.start_time.day) )
		headerswapflag = true;

	/* Swap byte order? */
	if ( headerswapflag ) {
		MS_SWAPBTIME(&head.start_time);
		ms_gswap2a(&head.numsamples);
		ms_gswap2a(&head.samprate_fact);
		ms_gswap2a(&head.samprate_mult);
		ms_gswap4a(&head.time_correct);
		ms_gswap2a(&head.data_offset);
		ms_gswap2a(&head.blockette_offset);
	}

	header.dataType = DT_Unknown;

	hptime_t hptime = ms_btime2hptime(&head.start_time);
	if ( hptime == HPTERROR ) {
		LOG_SID(CAPS_DEBUG, "read metadata: invalid start time");
		return;
	}

	if ( head.time_correct != 0 && !(head.act_flags & 0x02) )
		hptime += (hptime_t)head.time_correct * (HPTMODULUS / 10000);

	// Parse blockettes
	uint32_t blkt_offset = head.blockette_offset;
	uint32_t blkt_length;
	uint16_t blkt_type;
	uint16_t next_blkt;

	if ( blkt_offset < sizeof(head) ) {
		LOG_SID(CAPS_DEBUG, "read metadata: blockette "
		        "offset points into header");
		return;
	}

	uint32_t coffs = 0;

	while ( (blkt_offset != 0) && ((int)blkt_offset < size) &&
	        (blkt_offset < MAXRECLEN) ) {
		char bhead[6];
		buf.pubseekoff(blkt_offset-sizeof(head)-coffs, std::ios_base::cur,
		               std::ios_base::in);
		if ( buf.sgetn(bhead, 6) != 6 ) {
			LOG_SID(CAPS_DEBUG, "read metadata: "
			        "failed to read blockette header");
			break;
		}

		coffs = 6;

		memcpy(&blkt_type, bhead, 2);
		memcpy(&next_blkt, bhead+2, 2);

		if ( headerswapflag ) {
			ms_gswap2(&blkt_type);
			ms_gswap2(&next_blkt);
		}

		blkt_length = ms_blktlen(blkt_type, bhead, headerswapflag);

		if ( blkt_length == 0 ) {
			LOG_SID(CAPS_DEBUG, "read metadata: "
			        "unknown blockette length for type %d",
			        blkt_type);
			break;
		}

		/* Make sure blockette is contained within the msrecord buffer */
		if ( (int)(blkt_offset - 4 + blkt_length) > size ) {
			LOG_SID(CAPS_DEBUG, "read metadata: blockette "
			        "%d extends beyond record size, truncated?",
			        blkt_type);
			break;
		}

		if ( blkt_type == 1000 ) {
			switch ( (int)bhead[4] ) {
				case DE_ASCII:
					header.dataType = DT_INT8;
					break;
				case DE_INT16:
					header.dataType = DT_INT16;
					break;
				case DE_INT32:
				case DE_STEIM1:
				case DE_STEIM2:
				case DE_CDSN:
				case DE_DWWSSN:
				case DE_SRO:
					header.dataType = DT_INT32;
					break;
				case DE_FLOAT32:
					header.dataType = DT_FLOAT;
					break;
				case DE_FLOAT64:
					header.dataType = DT_DOUBLE;
					break;
				case DE_GEOSCOPE24:
				case DE_GEOSCOPE163:
				case DE_GEOSCOPE164:
					header.dataType = DT_FLOAT;
					break;
				default:
					break;
			}
		}
		else if ( blkt_type == 1001 ) {
			// Add usec correction
			hptime += ((hptime_t)bhead[5]) * (HPTMODULUS / 1000000);
		}

		/* Check that the next blockette offset is beyond the current blockette */
		if ( next_blkt && next_blkt < (blkt_offset + blkt_length - 4) ) {
			LOG_SID(CAPS_DEBUG, "read metadata: offset to "
			        "next blockette (%d) is within current blockette "
			        "ending at byte %d",
			        blkt_type, (blkt_offset + blkt_length - 4));
			break;
		}
		/* Check that the offset is within record length */
		else if ( next_blkt && next_blkt > size ) {
			LOG_SID(CAPS_DEBUG, "read metadata: offset to "
			        "next blockette (%d) from type %d is beyond record "
			        "length", next_blkt, blkt_type);
			break;
		}
		else
			blkt_offset = next_blkt;
	}

	startTime = Time((hptime_t)hptime/HPTMODULUS,(hptime_t)hptime%HPTMODULUS);
	endTime = startTime;

	if ( head.samprate_fact > 0 ) {
		header.samplingFrequencyNumerator = head.samprate_fact;
		header.samplingFrequencyDenominator = 1;
	}
	else {
		header.samplingFrequencyNumerator = 1;
		header.samplingFrequencyDenominator = -head.samprate_fact;
	}

	if ( head.samprate_mult > 0 )
		header.samplingFrequencyNumerator *= head.samprate_mult;
	else
		header.samplingFrequencyDenominator *= -head.samprate_mult;

	if ( header.samplingFrequencyNumerator > 0.0 && head.numsamples > 0 ) {
		hptime = (hptime_t)head.numsamples * HPTMODULUS * header.samplingFrequencyDenominator / header.samplingFrequencyNumerator;
		endTime += TimeSpan((hptime_t)hptime/HPTMODULUS,(hptime_t)hptime%HPTMODULUS);
	}

	timeToTimestamp(_header.samplingTime, startTime);
#else
	std::vector<char> data(size);
	size_t read = buf.sgetn(&data[0], data.size());
	if ( read != data.size() ) {
		CAPS_WARNING("read metadata: input buffer underflow: only %d/%d bytes read",
		                 (int)read, (int)data.size());
		return;
	}

	unpackHeader(&data[0], data.size());

	header = _header;
	startTime = _startTime;
	endTime = _endTime;
#endif
}


const DataRecord::Header *MSEEDDataRecord::header() const {
	return &_header;
}


Time MSEEDDataRecord::startTime() const {
	return _startTime;
}


Time MSEEDDataRecord::endTime() const {
	return _endTime;
}


size_t MSEEDDataRecord::dataSize(bool /*withHeader*/) const {
	return _data.size();
}


DataRecord::ReadStatus MSEEDDataRecord::get(std::streambuf &buf, int size,
                                            const Time &start,
                                            const Time &end,
                                            int) {
	//MSRecord *ms_rec = NULL;

	if ( size <= 0 ) {
		CAPS_WARNING("get: invalid size of record: %d", size);
		return RS_Error;
	}

	_data.resize(size);
	size_t read = buf.sgetn(&_data[0], _data.size());
	if ( read != _data.size() ) {
		CAPS_WARNING("get: input buffer underflow: only %d/%d bytes read",
		             (int)read, (int)_data.size());
		return RS_Error;
	}

	arraybuf tmp(&_data[0], size);
	readMetaData(tmp, size, _header, _startTime, _endTime);

	if ( start.valid() || end.valid() ) {
		// Out of scope?
		if ( end.valid() && (end <= _startTime) )
			return RS_AfterTimeWindow;

		if ( start.valid() && (start >= _endTime) )
			return RS_BeforeTimeWindow;
	}

	return RS_Complete;

	/*
	// Only unpack the header structure
	int state = msr_unpack(&_data[0], _data.size(), &ms_rec, 0, 0);
	if ( state != MS_NOERROR ) {
		switch ( state ) {
			case MS_GENERROR:
				CAPS_WARNING("get: generic libmseed error");
				break;
			case MS_NOTSEED:
				CAPS_WARNING("get: input data is not seed");
				break;
			case MS_WRONGLENGTH:
				CAPS_WARNING("get: length of data read was not correct");
				break;
			case MS_OUTOFRANGE:
				CAPS_WARNING("get: SEED record length out of range");
				break;
			case MS_UNKNOWNFORMAT:
				CAPS_WARNING("get: unknown data encoding format");
				break;
			case MS_STBADCOMPFLAG:
				CAPS_WARNING("get: invalid Steim compression flag(s)");
				break;
		}
		if ( ms_rec != NULL )
			msr_free(&ms_rec);
		return RS_Error;
	}

	hptime_t hptime = msr_starttime(ms_rec);
	_startTime = Time((hptime_t)hptime/HPTMODULUS,(hptime_t)hptime%HPTMODULUS);
	_endTime = _startTime;

	if ( ms_rec->samprate > 0.0 && ms_rec->samplecnt > 0 ) {
		hptime = (hptime_t)(((double)(ms_rec->samplecnt) / ms_rec->samprate * HPTMODULUS) + 0.5);
		_endTime += TimeSpan((hptime_t)hptime/HPTMODULUS,(hptime_t)hptime%HPTMODULUS);
	}

	_header.dataType = DT_Unknown;
	timeToTimestamp(_header.samplingTime, _startTime);

	if ( ms_rec->fsdh->samprate_fact > 0 ) {
		_header.samplingFrequencyNumerator = ms_rec->fsdh->samprate_fact;
		_header.samplingFrequencyDenominator = 1;
	}
	else {
		_header.samplingFrequencyNumerator = 1;
		_header.samplingFrequencyDenominator = -ms_rec->fsdh->samprate_fact;
	}

	if ( ms_rec->fsdh->samprate_mult > 0 )
		_header.samplingFrequencyNumerator *= ms_rec->fsdh->samprate_mult;
	else
		_header.samplingFrequencyDenominator *= -ms_rec->fsdh->samprate_mult;

	switch ( ms_rec->sampletype ) {
		case 'a':
			_header.dataType = DT_INT8;
			break;
		case 'i':
			_header.dataType = DT_INT32;
			break;
		case 'f':
			_header.dataType = DT_FLOAT;
			break;
		case 'd':
			_header.dataType = DT_DOUBLE;
			break;
		default:
			_header.dataType = DT_Unknown;
			break;
	}

	msr_free(&ms_rec);

	if ( start.valid() || end.valid() ) {
		// Out of scope?
		if ( end.valid() && (end <= _startTime) )
			return RS_AfterTimeWindow;

		if ( start.valid() && (start >= _endTime) )
			return RS_BeforeTimeWindow;
	}

	return RS_Complete;
	*/
}


void MSEEDDataRecord::setData(const void *data, size_t size) {
	_data.resize(size);
	memcpy(_data.data(), data, size);
	unpackHeader();
}

void MSEEDDataRecord::unpackHeader(char *data, size_t size) {
	// Only unpack the header structure
	MSRecord *ms_rec = NULL;
	int state = msr_unpack(data, size, &ms_rec, 0, 0);
	if ( state != MS_NOERROR ) {
		CAPS_WARNING("read metadata: read error: %d", state);
		if ( ms_rec != NULL )
			msr_free(&ms_rec);
		return;
	}

	hptime_t hptime = msr_starttime(ms_rec);
	_startTime = Time((hptime_t)hptime/HPTMODULUS,(hptime_t)hptime%HPTMODULUS);
	_endTime = _startTime;

	if ( ms_rec->samprate > 0.0 && ms_rec->samplecnt > 0 ) {
		hptime = (hptime_t)(((double)(ms_rec->samplecnt) / ms_rec->samprate * HPTMODULUS) + 0.5);
		_endTime += TimeSpan((hptime_t)hptime/HPTMODULUS,(hptime_t)hptime%HPTMODULUS);
	}

	_header.dataType = DT_Unknown;
	timeToTimestamp(_header.samplingTime, _startTime);

	if ( ms_rec->fsdh->samprate_fact > 0 ) {
		_header.samplingFrequencyNumerator = ms_rec->fsdh->samprate_fact;
		_header.samplingFrequencyDenominator = 1;
	}
	else {
		_header.samplingFrequencyNumerator = 1;
		_header.samplingFrequencyDenominator = -ms_rec->fsdh->samprate_fact;
	}

	if ( ms_rec->fsdh->samprate_mult > 0 )
		_header.samplingFrequencyNumerator *= ms_rec->fsdh->samprate_mult;
	else
		_header.samplingFrequencyDenominator *= -ms_rec->fsdh->samprate_mult;

	switch ( ms_rec->sampletype ) {
		case 'a':
			_header.dataType = DT_INT8;
			break;
		case 'i':
			_header.dataType = DT_INT32;
			break;
		case 'f':
			_header.dataType = DT_FLOAT;
			break;
		case 'd':
			_header.dataType = DT_DOUBLE;
			break;
		default:
			_header.dataType = DT_Unknown;
			break;
	}

	msr_free(&ms_rec);
}


}
}
