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


#define SEISCOMP_COMPONENT StreamIdx

#include <string>
#include "streamidx.h"

namespace Seiscomp {
namespace RecordStream {

using namespace std;
using namespace Seiscomp::Core;


StreamIdx::StreamIdx() {}

StreamIdx::StreamIdx(const string& net, const string& sta, const string& loc,
		     const string& cha)
    : _net(net), _sta(sta), _loc(loc), _cha(cha) {}

StreamIdx::StreamIdx(const string& net, const string& sta, const string& loc,
		     const string& cha, const Time& stime, const Time& etime)
    : _net(net), _sta(sta), _loc(loc), _cha(cha),
      _stime(stime), _etime(etime) {}

StreamIdx& StreamIdx::operator=(const StreamIdx &other) {
    if (this != &other) {
	this->~StreamIdx();
	new(this) StreamIdx(other);
    }
    
    return *this;
}

bool StreamIdx::operator<(const StreamIdx &other) const {
    if (_net < other._net) return true;
    else if(_net == other._net) {
	if(_sta < other._sta) return true;
	else if(_sta == other._sta) {
	    if(_loc < other._loc) return true;
	    else if(_loc == other._loc) {
		if(_cha < other._cha) return true;
                else if(_cha == other._cha) {
                     if(_stime < other._stime) return true;
                     else if(_stime == other._stime) {
                         if(_etime < other._etime) return true;
                     }
                 } 
	    }
	}
    }

    return false;
}

bool StreamIdx::operator!=(const StreamIdx &other) const {
    return (_net != other._net || _sta != other._sta ||
	    _loc != other._loc || _cha != other._cha);
}

bool StreamIdx::operator==(const StreamIdx &other) const {
    return !(*this != other);
}

bool StreamIdx::operator>=(const StreamIdx &other) const {
    return !(*this < other);
}

bool StreamIdx::operator>(const StreamIdx &other) const {
    return (*this >= other && *this != other);
}
    
bool StreamIdx::operator<=(const StreamIdx &other) const {
    return (*this < other || *this == other);
}

const string &StreamIdx::network() const {
    return _net;
}

const string &StreamIdx::station() const {
    return _sta;
}

const string &StreamIdx::channel() const {
    return _cha;
}
   
const string &StreamIdx::location() const {
	return _loc;
}
 
string StreamIdx::selector() const {
    string loc = _loc;
    string cha = _cha;
    string::size_type pos = loc.find('*',0);

    if (loc.length() > 0) {
    	if (pos != string::npos) loc.replace(pos,1,1,'?');
    	if (loc.length() < 2) loc.append(2-loc.length(),'?');
    }
    
    pos = cha.find('*',0);
    if (pos != string::npos) cha.replace(pos,1,1,'?');
    if (cha.length() < 3) cha.append(3-cha.length(),'?');
    
    string selector = loc + cha + ".D";
    return selector;
}

Time StreamIdx::startTime() const {
    return _stime;
}

Time StreamIdx::endTime() const {
    return _etime;
}

string StreamIdx::str(const Time& stime, const Time& etime) const {
	Time bt;
	if (_stime == Time())
		bt = stime;
	else
		bt = _stime;

	Time et;
	if (_etime == Time())
		et = etime;
	else
		et = _etime;

	if ( et.microseconds() > 0 )
		et += Time(1,0);

	return bt.toString("%Y,%m,%d,%H,%M,%S") + " " +
	       et.toString("%Y,%m,%d,%H,%M,%S") + " " +
	       _net + " " + _sta + " " + _cha + " " + _loc;
}

Time StreamIdx::timestamp() const {
    return _timestamp;
}

void StreamIdx::setTimestamp(Time &rectime) const {
    if (_timestamp < rectime)
        _timestamp = rectime;
}

}
}
