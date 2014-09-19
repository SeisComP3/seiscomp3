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


#define SEISCOMP_COMPONENT SDSARCHIVE

#include <sys/stat.h>
#include <errno.h>
#include <iomanip>
#include <seiscomp3/io/recordstream/sdsarchive.h>
#include <seiscomp3/logging/log.h>
#include <libmseed.h>

using namespace Seiscomp::RecordStream;
using namespace Seiscomp::IO;
using namespace Seiscomp::Core;
using namespace std;


namespace {

Time getStartTime(const string &file) {
	MSRecord *prec = NULL;
	MSFileParam *pfp = NULL;

	int retcode = ms_readmsr_r(&pfp,&prec,const_cast<char*>(file.c_str()),0,NULL,NULL,1,0,0);
	if ( retcode == MS_NOERROR ) {
		Time start((hptime_t)prec->starttime/HPTMODULUS,(hptime_t)prec->starttime%HPTMODULUS);
		ms_readmsr_r(&pfp,&prec,NULL,-1,NULL,NULL,0,0,0);
		return start;
	}

	ms_readmsr_r(&pfp,&prec,NULL,-1,NULL,NULL,0,0,0);
	return Time();
}

}

IMPLEMENT_SC_CLASS_DERIVED(SDSArchive,
                           Seiscomp::IO::RecordStream,
                           "sdsarchive");

REGISTER_RECORDSTREAM(SDSArchive, "sdsarchive");


SDSArchive::SDSArchive() : RecordStream() {}

SDSArchive::SDSArchive(const string arcroot) 
: RecordStream(), _arcroot(arcroot) {}

SDSArchive::SDSArchive(const SDSArchive &mem) : RecordStream() {
	setSource(mem.archiveRoot());
}

SDSArchive::~SDSArchive() {}

SDSArchive& SDSArchive::operator=(const SDSArchive &mem) {
	if (this != &mem)
		_arcroot = mem.archiveRoot();

	return *this;
}

bool SDSArchive::setSource(string src) {
	_arcroot = src;
	return true;
}

bool SDSArchive::addStream(string net, string sta, string loc, string cha) {
	pair<set<StreamIdx>::iterator, bool> result;
	try {
		if (cha.at(2) == '?' || cha.at(2) == '*') {
			result = _streams.insert(StreamIdx(net,sta,loc,cha.substr(0,2)+"Z"));
			result = _streams.insert(StreamIdx(net,sta,loc,cha.substr(0,2)+"N"));
			result = _streams.insert(StreamIdx(net,sta,loc,cha.substr(0,2)+"E"));
		}
		else
			result = _streams.insert(StreamIdx(net,sta,loc,cha));
	}
	catch(...) {
		return false;
	}
	return result.second;
}

bool SDSArchive::addStream(string net, string sta, string loc, string cha, const Time &stime, const Time &etime) {
	pair<set<StreamIdx>::iterator, bool> result;

	try {
		if (cha.at(2) == '?' || cha.at(2) == '*') {
			result = _streams.insert(StreamIdx(net,sta,loc,cha.substr(0,2)+"Z",stime,etime));
			result = _streams.insert(StreamIdx(net,sta,loc,cha.substr(0,2)+"N",stime,etime));
			result = _streams.insert(StreamIdx(net,sta,loc,cha.substr(0,2)+"E",stime,etime));
		}
		else
			result = _streams.insert(StreamIdx(net,sta,loc,cha,stime,etime));
	}
	catch(...) {
		return false;
	}

	return result.second;
}

bool SDSArchive::removeStream(std::string net, std::string sta, std::string loc, std::string cha) {
	bool deletedSomething = false;
	std::set<StreamIdx>::iterator it = _streams.begin();

	for ( ; it != _streams.end(); ) {
		if ( it->network()  == net &&
		     it->station()  == sta &&
		     it->location() == loc &&
		     it->channel()  == cha ) {
			_streams.erase(it++);
			deletedSomething = true;
		}
		else
			++it;
	}

	return deletedSomething;
}

bool SDSArchive::setStartTime(const Time &stime) {
	_stime = stime;
	return true;
}

bool SDSArchive::setEndTime(const Time &etime) {
	_etime = etime;
	return true;
}

bool SDSArchive::setTimeWindow(const TimeWindow &w) {
	return setStartTime(w.startTime()) && setEndTime(w.endTime());
}

bool SDSArchive::setTimeout(int seconds) {
	return false;
}

void SDSArchive::close() {}

string SDSArchive::archiveRoot() const {
	return _arcroot;
}

int SDSArchive::getDoy(const Time &time) {
	int year;

	time.get(&year);
	if ((year%4==0 && year%100!=0) || year%400==0)
		return (366-((int)(Time(year,12,31,23,59,59)-time)/86400));
	return (365-((int)(Time(year,12,31,23,59,59)-time)/86400));
}

string SDSArchive::SDSfilename(int doy, int year) {
	string net = _curidx->network();
	string sta = _curidx->station();
	string cha = _curidx->channel();
	string loc = _curidx->location();
	stringstream ss;

	ss << year;
	string path = _arcroot + "/" + ss.str() + "/" + net + "/" + sta + "/" + cha + ".D/" +
	net + "." + sta + "." + loc + "." + cha + ".D." + ss.str() + ".";
	ss.str("");
	ss << setfill ('0') << setw(3) << doy;
	path += ss.str();

	return path;
}

void SDSArchive::setFilenames() {
	Time stime = (_curidx->startTime() == Time())?_stime:_curidx->startTime();
	Time etime = (_curidx->endTime() == Time())?_etime:_curidx->endTime();
	int sdoy = getDoy(stime);
	int edoy = getDoy(etime);
	int syear, eyear, tmpdoy, tmpyear;

	bool first = true;

	stime.get(&syear);
	etime.get(&eyear);
	for ( int year = syear; year <= eyear; ++year ) {
		tmpdoy = (year == eyear)?edoy:getDoy(Time(year,12,31,23,59,59));
		for ( int doy = sdoy; doy <= tmpdoy; ++doy ) {
			string file = SDSfilename(doy,year);
			if ( first ) {
				if ( getStartTime(file) > stime ) {
					Time tmptime = stime - TimeSpan(86400,0);
					int tmpdoy2;
					tmptime.get2(&tmpyear, &tmpdoy2);
					_fnames.push(SDSfilename(tmpdoy2+1, tmpyear));
				}
			}

			_fnames.push(file);
			first = false;
		}
		sdoy = 1;
	}
}

bool SDSArchive::setStart(const string &fname) {
	MSRecord *prec = NULL;
	MSFileParam *pfp = NULL;
	double samprate = 0.0;
	Time recstime, recetime;
	Time stime = (_curidx->startTime() == Time())?_stime:_curidx->startTime();
	off_t fpos;
	int retcode;
	long int offset = 0;
	long int size = _recstream->stream().rdbuf()->in_avail();
	bool result = true;

#define SDSARCHIVE_BINSEARCH
#ifdef SDSARCHIVE_BINSEARCH
	//! binary search
	retcode = ms_readmsr_r(&pfp,&prec,const_cast<char *>(fname.c_str()),0,NULL,NULL,1,0,0);
	if (retcode == MS_NOERROR) {
		recstime = Time((hptime_t)prec->starttime/HPTMODULUS,(hptime_t)prec->starttime%HPTMODULUS);
		long start = 0;
		long half = 0;
		long end = 0;
		int reclen = prec->reclen;

		if ( recstime < stime )
			end = (long)(size/reclen);

		while ( (end - start) > 1 ) {
			half = start + (end - start)/2;
			fpos = -half*reclen;
			//lmp_fseeko(pfp->fp, half*reclen, 0);
			if ( (retcode = ms_readmsr_r(&pfp,&prec,const_cast<char *>(fname.c_str()),0,&fpos,NULL,1,0,0)) == MS_NOERROR ) {
				samprate = prec->samprate;
				recstime = Time((hptime_t)prec->starttime/HPTMODULUS,(hptime_t)prec->starttime%HPTMODULUS);
				if ( samprate > 0. )
					recetime = recstime + TimeSpan((double)(prec->samplecnt / samprate));
				else {
					SEISCOMP_WARNING("sdsarchive: [%s@%ld] Wrong sampling frequency %.2f!", fname.c_str(), half*reclen, samprate);
					recetime = recstime + TimeSpan(1, 0);
					result = false;
				}
			}
			else {
				SEISCOMP_WARNING("sdsarchive: [%s@%ld] Couldn't read mseed header!", fname.c_str(), half*reclen);
				break;
			}

			if ( recetime < stime ) {
				start = half;
				if ((end - start) == 1)
					++half;
			}
			else if ( recstime > stime )
				end = half;
			else if ( recstime <= stime && stime <= recetime ) {
				if (stime == recetime)
					++half;
				break;
			}
		}

		if ( (half == 1) && (recstime > stime) )
			half = 0;
		offset = half*reclen;
	}
#else
	while((retcode = ms_readmsr_r(&pfp,&prec,const_cast<char *>(fname.c_str()),0,NULL,NULL,1,0,0)) == MS_NOERROR) {
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
			} else {
				SEISCOMP_WARNING("sdsarchive: [%s@%ld] Wrong sampling frequency %.2f!", fname.c_str(), offset, samprate);
				offset += prec->reclen;
				result = false;
			}
		}
	}
#endif
	if (retcode != MS_ENDOFFILE && retcode != MS_NOERROR) {
		SEISCOMP_ERROR("sdsarchive: Error reading input file %s: %s", fname.c_str(),ms_errorstr(retcode));
		result = false;
	}

	/* Cleanup memory and close file */
	ms_readmsr_r(&pfp,&prec,NULL,-1,NULL,NULL,0,0,0);

	_recstream->stream().seekg(offset,ios::beg);
	if ( offset >= size )
		_recstream->stream().clear(ios::eofbit);

	return result;
}

bool SDSArchive::isEnd() {
	if (!_fnames.empty())
		return false;

	istream &istr = _recstream->stream();
	streampos strpos = istr.tellg();
	char buffer[sizeof(struct fsdh_s)];
	struct fsdh_s *fsdh = (struct fsdh_s *)buffer;
	Time rectime;
	int year, month, day;
	Time etime = (_curidx->endTime() == Time())?_etime:_curidx->endTime();

	if ( !istr.read(buffer, sizeof(struct fsdh_s)) ) {
		istr.clear(ios::eofbit);
		return true;
	}

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

istream& SDSArchive::stream() throw(ArchiveException) {  
	if ( _recstream ) {
		/* eof check: try to read from stream */
		istream &tmpstream = _recstream->stream();
		tmpstream.peek();
		/* go on at the file's stream */
		if (tmpstream.good() && !isEnd())
			return tmpstream;
	}
	else
		_curiter = _streams.begin();

	bool first = false;
	while (!_fnames.empty() || _curiter != _streams.end()) {
		while (_fnames.empty() && _curiter != _streams.end()) {
			SEISCOMP_DEBUG("SDS request: %s", _curiter->str(_stime, _etime).c_str());
			if (_etime == Time())
				_etime = Time::GMT();
			if ((_curiter->startTime() == Time() && _stime == Time())) {
				SEISCOMP_WARNING("... has invalid time window -> ignore this request above");
				++_curiter;
			} else {
				_curidx = &*_curiter;
				++_curiter;
				setFilenames();
				first = true;
				break;
			}
		}

		if (!_fnames.empty()) {
			_recstream = RecordStream::Create("file");
			if (!_recstream) {
				SEISCOMP_ERROR("Could not create file stream");
				throw ArchiveException("Could not create file stream");
			}

			while ( !_fnames.empty() ) {
				string fname = _fnames.front();
				_fnames.pop();
				if ( !_recstream->setSource(fname.c_str()) ) {
					SEISCOMP_DEBUG("file %s not found", fname.c_str());
					_recstream->stream().clear();
				}
				else {
					if ( first ) {
						if ( !setStart(fname) )
							SEISCOMP_WARNING("Error reading file %s; start of time window maybe incorrect",fname.c_str());
					}

					if ( !_recstream->stream().eof() && !isEnd() )
						return _recstream->stream();
				}

				first = false;
			}
		}
	}

	if (!_recstream) {
		SEISCOMP_DEBUG("no data found in SDS archive");
		throw ArchiveException("no data found in SDS archive");
	}

	return _recstream->stream();
}
