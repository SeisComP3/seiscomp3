/* *************************************************************************
 *   Copyright (C) 2006 by GFZ Potsdam
 *
 *   $Author:  $
 *   $Email:  $
 *   $Date:  $
 *   $Revision:  $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * *************************************************************************/

#define SEISCOMP_COMPONENT ODCARCHIVE

#include <sys/stat.h>
#include <errno.h>
#include <iomanip>
#include <seiscomp3/io/records/mseedrecord.h>
#include <seiscomp3/io/recordstream/odcarchive.h>
#include <seiscomp3/logging/log.h>
#include <libmseed.h>

using namespace Seiscomp::RecordStream;
using namespace Seiscomp::IO;
using namespace Seiscomp::Core;
using namespace std;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_CLASS_DERIVED(ODCArchive,
                           Seiscomp::IO::RecordStream,
                           "odcarchive");

REGISTER_RECORDSTREAM(ODCArchive, "odcarchive");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ODCArchive::ODCArchive() : RecordStream() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ODCArchive::ODCArchive(const string arcroot)
: RecordStream(), _arcroot(arcroot) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ODCArchive::ODCArchive(const ODCArchive &mem) : RecordStream() {
	setSource(mem.archiveRoot());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ODCArchive::~ODCArchive() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ODCArchive& ODCArchive::operator=(const ODCArchive &mem) {
	if ( this != &mem )
		_arcroot = mem.archiveRoot();

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::setSource(const string &src) {
	_arcroot = src;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net,sta,loc,cha));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha,
                           const Time &stime, const Time &etime) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net,sta,loc,cha,stime,etime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::setStartTime(const Time &stime) {
	_stime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::setEndTime(const Time &etime) {
	_etime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ODCArchive::close() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string ODCArchive::archiveRoot() const {
	return _arcroot;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ODCArchive::getDoy(const Time &time) {
	int year;

	time.get(&year);
	if ((year%4==0 && year%100!=0) || year%400==0)
	return (366-((int)(Time(year,12,31,23,59,59)-time)/86400));
	return (365-((int)(Time(year,12,31,23,59,59)-time)/86400));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string ODCArchive::ODCfilename(int doy, int year) {
	string net = _curidx->network();
	string sta = _curidx->station();
	string cha = _curidx->channel();
	string loc = _curidx->location();
	stringstream year_stream, doy_stream;

	year_stream << year;
	doy_stream << setfill('0') << setw(3) << doy;
	string path = _arcroot + "/" + year_stream.str() + "/" + doy_stream.str() + "/";
	path += sta + "." + cha;
	if(!loc.empty())
	path += "_" + loc;
	path += "." + net + "." + year_stream.str() + "." + doy_stream.str();

	return path;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ODCArchive::setFilenames() {
	Time stime = (_curidx->startTime() == Time())?_stime:_curidx->startTime();
	Time etime = (_curidx->endTime() == Time())?_etime:_curidx->endTime();

	int sdoy = getDoy(stime);
	int edoy = getDoy(etime);
	int syear, eyear, tmpdoy;

	stime.get(&syear);
	etime.get(&eyear);
	for (int year = syear; year <= eyear; ++year) {
		tmpdoy = (year == eyear)?edoy:getDoy(Time(year,12,31,23,59,59));
		for (int doy = sdoy; doy <= tmpdoy; ++doy)
			_fnames.push(ODCfilename(doy,year));
		sdoy = 1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::setStart(const string &fname) {
	MSRecord *prec = NULL;
	MSFileParam *pfp = NULL;
	double samprate = 0.0;
	Time recstime, recetime;
	Time stime = (_curidx->startTime() == Time())?_stime:_curidx->startTime();
	int retcode;
	long int offset = 0;
	long int size;
	bool result = true;

	_recstream.seekg(0, ios::end);
	size = _recstream.tellg();

	while ((retcode = ms_readmsr_r(&pfp,&prec,const_cast<char *>(fname.c_str()),-1,NULL,NULL,1,0,0)) == MS_NOERROR) {
		samprate = prec->samprate;
		recstime = Time((hptime_t)prec->starttime/HPTMODULUS,(hptime_t)prec->starttime%HPTMODULUS);

		if (recstime > stime )
			break;
		else {
			if (samprate > 0.) {
				recetime = recstime + Time(prec->samplecnt / samprate);
				if (recetime > stime)
					break;
				else
					offset += prec->reclen;
			}
			else
				offset += prec->reclen;
		}
	}

	if ( retcode != MS_ENDOFFILE && retcode != MS_NOERROR ) {
		SEISCOMP_ERROR("Error reading input file %s: %s", fname.c_str(),ms_errorstr(retcode));
		result = false;
	}

	/* Cleanup memory and close file */
	ms_readmsr_r(&pfp,&prec,NULL,0,NULL,NULL,0,0,0);

	_recstream.seekg(offset,ios::beg);
	if (offset == size)
		_recstream.clear(ios::eofbit);

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::isEnd() {
	if (!_fnames.empty())
		return false;

	istream &istr = _recstream;
	streampos strpos = istr.tellg();
	char buffer[sizeof(struct fsdh_s)];
	struct fsdh_s *fsdh = (struct fsdh_s *)buffer;
	Time rectime;
	int year, month, day;
	Time etime = (_curidx->endTime() == Time())?_etime:_curidx->endTime();

	istr.read(buffer,sizeof(struct fsdh_s));
	istr.seekg(strpos);

	/* Check to see if byte swapping is needed (bogus year makes good test) */
	if ((fsdh->start_time.year < 1900) || (fsdh->start_time.year > 2050)) {
		ms_gswap2(&fsdh->start_time.year);
		ms_gswap2(&fsdh->start_time.day);
		ms_gswap2(&fsdh->start_time.fract);
	}

	rectime = Time::FromYearDay(fsdh->start_time.year,fsdh->start_time.day);
	rectime.get(&year,&month,&day);
	rectime.set(year,month,day,(int)fsdh->start_time.hour,(int)fsdh->start_time.min,
	            (int)fsdh->start_time.sec,(int)fsdh->start_time.fract);

	if (rectime > etime) {
		istr.clear(ios::eofbit);
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ODCArchive::stepStream() {
	if ( _recstream.is_open() ) {
		/* eof check: try to read from stream */
		_recstream.peek();
		/* go on at the file's stream */
		if ( _recstream.good() && !isEnd() )
			return true;
	}
	else
		_curiter = _streams.begin();

	bool first = false;
	while ( !_fnames.empty() || _curiter != _streams.end() ) {
		while ( _fnames.empty() && _curiter != _streams.end() ) {
			SEISCOMP_DEBUG("SDS request: %s", _curiter->str(_stime, _etime).c_str());
			if ( _etime == Time() ) _etime = Time::GMT();
			if ( (_curiter->startTime() == Time() && _stime == Time()) ) {
				SEISCOMP_WARNING("... has invalid time window -> ignore this request above");
				++_curiter;
			}
			else {
				_curidx = &*_curiter;
				++_curiter;
				setFilenames();
				first = true;
				break;
			}
		}

		if ( !_fnames.empty() ) {
			while ( !_fnames.empty() ) {
				string fname = _fnames.front();
				_fnames.pop();
				_recstream.open(fname.c_str(), ios_base::in | ios_base::binary);
				if ( !_recstream.is_open() ) {
					SEISCOMP_DEBUG("file %s not found", fname.c_str());
					_recstream.clear();
				}
				else {
					if ( first ) {
						if ( !setStart(fname) )
							SEISCOMP_WARNING("Error reading file %s; start of time window maybe incorrect",fname.c_str());
					}

					if ( !_recstream.eof() && !isEnd() )
						return true;
				}

				first = false;
			}
		}
	}

	if ( !_recstream ) {
		SEISCOMP_DEBUG("no data found in ODC archive");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Record *ODCArchive::next() {
	while ( true ) {
		if ( !stepStream() )
			return NULL;

		MSeedRecord *rec = new MSeedRecord();
		if ( rec == NULL )
			return NULL;

		setupRecord(rec);

		try {
			rec->read(_recstream);
		}
		catch ( Core::EndOfStreamException & ) {
			_recstream.close();
			delete rec;
			continue;
		}
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("file read exception: %s", e.what());
			delete rec;
			continue;
		}

		return rec;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
