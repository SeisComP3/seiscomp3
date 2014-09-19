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



#include <iostream>
#include <math.h>
#include <string.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core/interfacefactory.ipp>

using namespace Seiscomp;

IMPLEMENT_SC_ABSTRACT_CLASS(Record, "CoreRecord");
IMPLEMENT_INTERFACE_FACTORY(Record, SC_SYSTEM_CORE_API);


Record::Record(Array::DataType datatype, Hint h)
 : _net(""), _sta(""), _loc(""), _cha(""), _stime(Core::Time(0,0)),
   _datatype(datatype), _hint(h), _nsamp(0), _fsamp(0), _timequal(-1) {}


Record::Record(Array::DataType datatype, Hint h,
               std::string net, std::string sta, std::string loc, std::string cha,
               Core::Time stime, int nsamp, double fsamp, int tqual)
 : _net(net), _sta(sta), _loc(loc), _cha(cha), _stime(stime),
   _datatype(datatype), _hint(h), _nsamp(nsamp), _fsamp(fsamp), _timequal(tqual) {}


Record::Record(const Record &rec)
 : Core::BaseObject(), 
   _net(rec.networkCode()), _sta(rec.stationCode()), _loc(rec.locationCode()),
   _cha(rec.channelCode()), _stime(rec.startTime()), _datatype(rec.dataType()),
   _hint(rec._hint), _nsamp(rec.sampleCount()),
   _fsamp(rec.samplingFrequency()), _timequal(rec.timingQuality()) {}


Record::~Record () {
}


Record& Record::operator=(const Record &rec) {
	if (&rec != this) {
		_datatype = rec.dataType();
		_hint = rec._hint;
		_net = rec.networkCode();
		_sta = rec.stationCode();
		_loc = rec.locationCode();
		_cha = rec.channelCode();
		_stime = rec.startTime();
		_nsamp = rec.sampleCount();
		_fsamp = rec.samplingFrequency();
                _timequal = rec.timingQuality();
	}

	return (*this);
}


const std::string &Record::networkCode() const {
	return _net;
}


void Record::setNetworkCode(std::string net) {
	_net = net;
}


const std::string &Record::stationCode() const {
	return _sta;
}


void Record::setStationCode(std::string sta) {
	_sta = sta;
}


const std::string &Record::locationCode() const {
	return _loc;
}


void Record::setLocationCode(std::string loc) {
	_loc = loc;
}


const std::string &Record::channelCode() const {
	return _cha;
}


void Record::setChannelCode(std::string cha) {
	_cha = cha;
}


const Core::Time& Record::startTime() const {
	return _stime;
}


void Record::setStartTime(const Core::Time& time) {
	_stime = time;
}


Core::Time Record::endTime() const throw(Seiscomp::Core::ValueException) {
	double span = 0;

	if (_fsamp > 0.)
		span = sampleCount() / _fsamp;
	else
		throw Core::ValueException("Record::endTime(): _fsamp out of range");

	return _stime + Core::Time(span);
}


Core::TimeWindow Record::timeWindow() const {
	return Core::TimeWindow(_stime,endTime());
}


int Record::sampleCount() const {
	return _nsamp;
}


double Record::samplingFrequency() const {
	return _fsamp;
}

int Record::timingQuality() const {
    return _timequal;
}

void Record::setTimingQuality(int tqual) {
    _timequal = tqual;
}

std::string Record::streamID() const {
	return (_net+"."+_sta+"."+_loc+"."+_cha);
}


Array::DataType Record::dataType() const {
	return _datatype;
}


void Record::setDataType(Array::DataType dt) {
	_datatype = dt;
}


void Record::setHint(Hint h) {
	_hint = h;
}


void Record::serialize(Archive& ar) {
	ar & TAGGED_MEMBER(net);
	ar & TAGGED_MEMBER(sta);
	ar & TAGGED_MEMBER(loc);
	ar & TAGGED_MEMBER(cha);
	ar & TAGGED_MEMBER(stime);
	ar & TAGGED_MEMBER(fsamp);
}

std::istream& operator>>(std::istream &is, Record &rec) {
    rec.read(is);
    return is;
}

std::ostream& operator<<(std::ostream &os, Record &rec) {
    rec.write(os);
    return os;
}
