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


#define SEISCOMP_COMPONENT MSEEDRECORD
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/records/mseedrecord.h>
#include <seiscomp3/core/arrayfactory.h>

#include <libmseed.h>
#include <cctype>

using namespace Seiscomp;
using namespace Seiscomp::IO;


struct MSEEDLogger {
	MSEEDLogger() {
		ms_loginit(MSEEDLogger::print, "[libmseed] ", MSEEDLogger::diag, "[libmseed] ERR: ");
	}

	static void print(char *msg) {
		//SEISCOMP_DEBUG("%s", msg);
	}
	static void diag(char *msg) {
		//SEISCOMP_DEBUG("%s", msg);
	}
};


static MSEEDLogger __logger__;


IMPLEMENT_SC_CLASS_DERIVED(MSeedRecord, Record, "MSeedRecord");
REGISTER_RECORD(MSeedRecord, "mseed");

MSeedRecord::MSeedRecord(Array::DataType dt, Hint h)
 : Record(dt, h),
   _raw(CharArray()),
   _data(0),
   _seqno(0),
   _rectype('D'),
   _srfact(0),
   _srmult(0),
   _byteorder(0),
   _encoding(0),
   _srnum(0),
   _srdenom(0),
   _reclen(0),
   _nframes(0),
   _leap(0),
   _etime(Seiscomp::Core::Time(0,0)),
   _encodingFlag(true)
{}

MSeedRecord::MSeedRecord(MSRecord *rec, Array::DataType dt, Hint h)
 : Record(dt, h, rec->network, rec->station, rec->location, rec->channel,
          Seiscomp::Core::Time((hptime_t)rec->starttime/HPTMODULUS,(hptime_t)rec->starttime%HPTMODULUS),
          rec->samplecnt, rec->samprate, rec->Blkt1001 ? rec->Blkt1001->timing_qual : -1),
   _data(0),
   _seqno(rec->sequence_number),
   _rectype(rec->dataquality),
   _srfact(rec->fsdh->samprate_fact),
   _srmult(rec->fsdh->samprate_mult),
   _byteorder(rec->byteorder),
   _encoding(rec->encoding),
   _reclen(rec->reclen),
   _encodingFlag(true)
{
 	if (_hint == SAVE_RAW)
 		_raw.setData(rec->reclen,rec->record);
	else
		if (_hint == DATA_ONLY)
			try {
				_setDataAttributes(rec->reclen,rec->record);
			} catch (LibmseedException e) {
				_nsamp = 0;
				_fsamp = 0;
				_data = NULL;
				SEISCOMP_ERROR("LibmseedException in MSeedRecord constructor %s", e.what());
			}
	_srnum = 0;
	_srdenom = 1;
	if (_srfact > 0 && _srmult > 0) {
		_srnum = _srfact*_srmult;
		_srdenom = 1;
	}
	if (_srfact > 0 && _srmult < 0) {
		_srnum = _srfact;
		_srdenom = -_srmult;
	}
	if (_srfact < 0 && _srmult > 0) {
		_srnum = _srmult;
		_srdenom = -_srfact;
	}
	if (_srfact < 0 && _srmult < 0) {
		_srnum = 1;
		_srdenom = _srfact*_srmult;
	}
	_nframes = 0;
	if (rec->Blkt1001)
		_nframes = rec->Blkt1001->framecnt;
	_leap = 0;
	if (rec->fsdh->start_time.sec > 59)
		_leap = rec->fsdh->start_time.sec-59;
	hptime_t hptime = msr_endtime(rec);
	_etime = Seiscomp::Core::Time((hptime_t)hptime/HPTMODULUS,(hptime_t)hptime%HPTMODULUS);
}

MSeedRecord::MSeedRecord(const MSeedRecord &msrec)
 : Record(msrec),
   _seqno(msrec.sequenceNumber()),
   _rectype(msrec.dataQuality()),
   _srfact(msrec.sampleRateFactor()),
   _srmult(msrec.sampleRateMultiplier()),
   _byteorder(msrec.byteOrder()),
   _encoding(msrec.encoding()),
   _srnum(msrec.sampleRateNumerator()),
   _srdenom(msrec.sampleRateDenominator()),
   _nframes(msrec.frameNumber()),
   _leap(msrec.leapSeconds()),
   _etime(msrec.endTime()),
   _encodingFlag(true)
{
	_reclen = msrec._reclen;
	_raw = msrec._raw;
	_data = msrec._data?msrec._data->clone():NULL;
}

MSeedRecord::MSeedRecord(const Record &rec, int reclen)
 : Record(rec),
   _seqno(0),
   _rectype('D'),
   _srfact(0),
   _srmult(0),
   _byteorder(0),
   _encoding(0),
   _srnum(0),
   _srdenom(0),
   _nframes(0),
   _leap(0),
   _etime(Seiscomp::Core::Time()),
   _encodingFlag(false)
{
    _reclen = reclen;
    _data = rec.data()?rec.data()->clone():NULL;
}

MSeedRecord::~MSeedRecord() {}

void MSeedRecord::setNetworkCode(std::string net) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[2];
		strncpy(tmp, net.c_str(), 2);
		memcpy(header->network, tmp, 2);
	}

	Record::setNetworkCode(net);
}

void MSeedRecord::setStationCode(std::string sta) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[5];
		strncpy(tmp, sta.c_str(), 5);
		memcpy(header->station, tmp, 5);
	}

	Record::setStationCode(sta);
}

void MSeedRecord::setLocationCode(std::string loc) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[2];
		strncpy(tmp, loc.c_str(), 2);
		memcpy(header->location, tmp, 2);
	}

	Record::setLocationCode(loc);
}

void MSeedRecord::setChannelCode(std::string cha) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[3];
		strncpy(tmp, cha.c_str(), 3);
		memcpy(header->channel, tmp, 3);
	}

	Record::setChannelCode(cha);
}

void MSeedRecord::setStartTime(const Core::Time& time) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		hptime_t hptime = (hptime_t)time.seconds() * HPTMODULUS + (hptime_t)time.microseconds();
		ms_hptime2btime(hptime, &header->start_time);
		MS_SWAPBTIME(&header->start_time);
	}

	Record::setStartTime(time);
}


MSeedRecord& MSeedRecord::operator=(const MSeedRecord &msrec) {
  if (&msrec != this) {
		Record::operator=(msrec);
		_raw = msrec._raw;
		_data = msrec._data?msrec._data->clone():NULL;
		_seqno = msrec.sequenceNumber();
		_rectype = msrec.dataQuality();
		_srfact = msrec.sampleRateFactor();
		_srmult = msrec.sampleRateMultiplier();
		_byteorder = msrec.byteOrder();
		_encoding = msrec.encoding();
		_srnum = msrec.sampleRateNumerator();
		_srdenom = msrec.sampleRateDenominator();
		_reclen = msrec._reclen;
		_nframes = msrec.frameNumber();
		_leap = msrec.leapSeconds();
		_etime = msrec.endTime();
	}

	return (*this);
}

int MSeedRecord::sequenceNumber() const {
  return _seqno;
}

void MSeedRecord::setSequenceNumber(int seqno) {
  _seqno = seqno;
}

char MSeedRecord::dataQuality() const {
  return _rectype;
}

void MSeedRecord::setDataQuality(char qual) {
	_rectype = qual;
}

int MSeedRecord::sampleRateFactor() const {
	return _srfact;
}

void MSeedRecord::setSampleRateFactor(int srfact) {
	_srfact = srfact;
}

int MSeedRecord::sampleRateMultiplier() const {
	return _srmult;
}

void MSeedRecord::setSampleRateMultiplier(int srmult) {
	_srmult = srmult;
}

unsigned short MSeedRecord::byteOrder() const {
	return _byteorder;
}

unsigned short MSeedRecord::encoding() const {
	return _encoding;
}

int MSeedRecord::sampleRateNumerator() const {
	return _srnum;
}

int MSeedRecord::sampleRateDenominator() const {
	return _srdenom;
}

int MSeedRecord::frameNumber() const {
	return _nframes;
}

const Core::Time& MSeedRecord::endTime() const {
	return _etime;
}

int MSeedRecord::recordLength() const {
	return _reclen;
}

int MSeedRecord::leapSeconds() const {
	return _leap;
}

const Array* MSeedRecord::raw() const {
	return &_raw;
}

const Array* MSeedRecord::data() const {
    if (_raw.data() && (!_data || _datatype != _data->dataType())) {
        _setDataAttributes(_reclen,(char *)_raw.data());
    }

    return _data.get();
}

void MSeedRecord::_setDataAttributes(int reclen, char *data) const {
	MSRecord *pmsr = NULL;

	if (data) {
		if (msr_unpack(data,reclen,&pmsr,1,0) == MS_NOERROR) {
			if (pmsr->numsamples == _nsamp) {
				switch (pmsr->sampletype) {
				case 'i':
					_data = ArrayFactory::Create(_datatype,Array::INT,_nsamp,pmsr->datasamples);
					break;
				case 'f':
					_data = ArrayFactory::Create(_datatype,Array::FLOAT,_nsamp,pmsr->datasamples);
					break;
				case 'd':
					_data = ArrayFactory::Create(_datatype,Array::DOUBLE,_nsamp,pmsr->datasamples);
					break;
				case 'a':
					_data = ArrayFactory::Create(_datatype,Array::CHAR,_nsamp,pmsr->datasamples);
					break;
				}
			} else {
				msr_free(&pmsr);
				throw LibmseedException("The number of the unpacked data samples differs from the sample number in fixed data header.");
			}
			msr_free(&pmsr);
		} else
			throw LibmseedException("Unpacking of Mini SEED record failed.");
	}
}

void MSeedRecord::saveSpace() const {
  if (_hint == SAVE_RAW && _data) {
    _data = 0;
  }
}

Record* MSeedRecord::copy() const {
  return new MSeedRecord(*this);
}

void MSeedRecord::useEncoding(bool flag) {
    _encodingFlag = flag;
}

void MSeedRecord::setOutputRecordLength(int reclen) {
	_reclen = reclen;
}

bool _isHeader(const char *header) {
  return (std::isdigit((unsigned char) *(header)) &&
	  std::isdigit((unsigned char) *(header+1)) &&
	  std::isdigit((unsigned char) *(header+2)) &&
	  std::isdigit((unsigned char) *(header+3)) &&
	  std::isdigit((unsigned char) *(header+4)) &&
	  std::isdigit((unsigned char) *(header+5)) &&
	  std::isalpha((unsigned char) *(header+6)) &&
	  (*(header+7) == ' ' || *(header+7) == '\0'));
}

void MSeedRecord::read(std::istream &is) {
	int reclen = -1;
	int pos = is.tellg();
	MSRecord *prec = NULL;
	const int LEN = 64;
	char header[LEN];
	bool myeof = false;

	is.read(header,LEN);
	while (is.good()) {
		if (MS_ISVALIDHEADER(header)) {
			reclen = ms_detect(header,LEN);
			break;
		}
		else  /* ignore nondata records and scan to the next valid header */ {
			is.read(header,LEN);
		}
	}

	if (reclen <= 0 && is.good()) {  /* scan to the next header to retrieve the record length */
		pos = is.tellg();
		is.read(header,LEN);
		while (is.good()) {
			if (MS_ISVALIDHEADER(header) || MS_ISVALIDBLANK(header) || _isHeader(header)) {
				reclen = static_cast<int>(is.tellg())-pos;
				is.seekg(-(reclen+LEN),std::ios::cur);
				is.read(header,LEN);
				break;
			}
			else
				is.read(header,LEN);
		}
	}

	if (is.eof()) { /* retrieve the record length of the last record */
		is.clear();
		is.seekg(0,std::ios::end);
		reclen = static_cast<int>(is.tellg())-pos+LEN;
		is.seekg(-reclen,std::ios::cur);
		is.read(header,LEN);
		myeof = true;
	}
	else {
		if (is.bad())
			throw Core::StreamException("Fatal error occured during reading from stream.");
	}

	if ( reclen >= LEN ) {
		if ( MS_ISVALIDHEADER(header) && reclen <= (1 << 20) ) {
			std::vector<char> rawrec(reclen);
			memmove(&rawrec[0],header,LEN);
			is.read(&rawrec[LEN],reclen-LEN);
			if ( is.good() ) {
				if ( msr_unpack(&rawrec[0],reclen,&prec,0,0) == MS_NOERROR ) {
					*this = MSeedRecord(prec,this->_datatype,this->_hint);
					msr_free(&prec);
					if ( _fsamp <= 0 )
						throw LibmseedException("Unpacking of Mini SEED record failed.");
				}
				else
					throw LibmseedException("Unpacking of Mini SEED record failed.");
			}
			else if ( is.bad() || !is.eof() )
				throw Core::StreamException("Fatal error occured during reading from stream.");
			else if ( is.eof() )
				throw Core::EndOfStreamException();
		}
		else {
			if ( !myeof )
				return read(is);
			else
				throw Core::EndOfStreamException("Invalid miniSEED header");
		}
	}
	else {
		if ( !myeof )
			throw LibmseedException("Retrieving the record length failed.");
		else
			throw Core::EndOfStreamException("Invalid miniSEED record, too small");
	}
}


void MSeedRecord::write(std::ostream& out) {
	if (!_data) {
		if (!_raw.data())
			throw Core::StreamException("No writable data found");
		else
			data();
	}

	MSRecord *pmsr;
	pmsr = msr_init(NULL);
	if (!pmsr)
		throw Core::StreamException("msr_init failed");

	/* Set MSRecord header values */
	pmsr->reclen = _reclen;
	pmsr->sequence_number = _seqno;
	strcpy(pmsr->network,_net.c_str());
	strcpy(pmsr->station,_sta.c_str());
	strcpy(pmsr->location,_loc.c_str());
	strcpy(pmsr->channel,_cha.c_str());
	pmsr->dataquality = _rectype;
	pmsr->starttime = ms_timestr2hptime(const_cast<char *>(_stime.iso().c_str()));
	pmsr->samprate = _fsamp;
	pmsr->byteorder = 1;
	pmsr->numsamples = _data->size();
	ArrayPtr data = 0;

	if (_encodingFlag) {
		switch (_encoding) {
		case DE_ASCII: {
			pmsr->encoding = DE_ASCII;
			pmsr->sampletype = 'a';
			data = ArrayFactory::Create(Array::CHAR,_data.get());
			pmsr->datasamples = (char *)data->data();
			break;
		}
		case DE_FLOAT32: {
			pmsr->encoding = DE_FLOAT32;
			pmsr->sampletype = 'f';
			data = ArrayFactory::Create(Array::FLOAT,_data.get());
			pmsr->datasamples = (float *)data->data();
			break;
		}
		case DE_FLOAT64: {
			pmsr->encoding = DE_FLOAT64;
			pmsr->sampletype = 'd';
			data = ArrayFactory::Create(Array::DOUBLE,_data.get());
			pmsr->datasamples = (double *)data->data();
			break;
		}
		case DE_INT16:
		case DE_INT32:
		case DE_STEIM1:
		case DE_STEIM2: {
			pmsr->encoding = _encoding;
			pmsr->sampletype = 'i';
			data = ArrayFactory::Create(Array::INT,_data.get());
			pmsr->datasamples = (int *)data->data();
			break;
		}
		default: {
			SEISCOMP_WARNING("Unknown encoding type found %s(%c)! Switch to Integer-Steim2 encoding.", ms_encodingstr(_encoding), _encoding);
			pmsr->encoding = DE_STEIM2;
			pmsr->sampletype = 'i';
			data = ArrayFactory::Create(Array::INT,_data.get());
			pmsr->datasamples = (int *)data->data();
		}
		}
	}
	else {
		switch (_data->dataType()) {
		case Array::CHAR:
			pmsr->encoding = DE_ASCII;
			pmsr->sampletype = 'a';
			pmsr->datasamples = (char *)_data->data();
			break;
		case Array::INT:
			pmsr->encoding = DE_STEIM2;
			pmsr->sampletype = 'i';
			pmsr->datasamples = (int *)_data->data();
			break;
		case Array::FLOAT:
			pmsr->encoding = DE_FLOAT32;
			pmsr->sampletype = 'f';
			pmsr->datasamples = (float *)_data->data();
			break;
		case Array::DOUBLE:
			pmsr->encoding = DE_FLOAT64;
			pmsr->sampletype = 'd';
			pmsr->datasamples = (double *)_data->data();
			break;
		default: {
			SEISCOMP_WARNING("Unknown data type %c! Switch to Integer-Steim2 encoding.", _data->dataType());
			pmsr->encoding = DE_STEIM2;
			pmsr->sampletype = 'i';
			data = ArrayFactory::Create(Array::INT,_data.get());
			pmsr->datasamples = (int *)data->data();
		}
		}
	}

	/* Pack the record(s) */
	CharArray packed;
	int64_t psamples;
	msr_pack(pmsr, &_Record_Handler, &packed, &psamples, 1, 0);
	pmsr->datasamples = 0;
	msr_free(&pmsr);

	out.write(packed.typedData(), packed.size());
}

