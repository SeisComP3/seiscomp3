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


//! HACK
//! INCOMPLETE IMPLEMENTATION OF SEISMIC HANDLER FORMAT ( SH )
//! HACK


#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <ios>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

#include <seiscomp3/io/records/shrecord.h>
#include <seiscomp3/core/arrayfactory.h>
#include <seiscomp3/core/typedarray.h>

#include <boost/lexical_cast.hpp>

#define SH_SUCCESS     0
#define SH_ERROR       1


using namespace std;

namespace Seiscomp {

namespace IO {


static int sh_put(ostream& os, const SHRecord& rec);


IMPLEMENT_SC_CLASS_DERIVED(SHRecord, Record, "SHRecord");
REGISTER_RECORD(SHRecord, "sh");



SHRecord::SHRecord(string net, string sta, string loc, string cha,
                   Core::Time stime, double fsamp, int tqual,
                   Array::DataType dt, Hint h)
    : Record(dt,h,net,sta,loc,cha,stime,0,fsamp,tqual), _data(0)
{
}



SHRecord::SHRecord(const SHRecord& rec)
	: Record(rec)
{
	_data = rec._data ? rec._data->clone() : NULL;
}




SHRecord::SHRecord(const Record& rec)
	: Record(rec), _data(0)
{
	_data = rec.data() ? rec.data()->clone() : NULL;
}




SHRecord::~SHRecord() {}




SHRecord& SHRecord::operator=(const SHRecord& rec)
{
	std::cerr << "incomplete SHRecord::operator= called" << std::endl;

	if (this != &rec) {
		if (_data) {
			_data = NULL;
		}
		_data = rec._data ? rec._data->clone() : NULL;
// FIXME incomplete
	}

	return *this;
}




Array* SHRecord::data()
{
	return _data.get();
}




const Array* SHRecord::raw() const
{
	return NULL;
}




const Array* SHRecord::data() const
{
	return _data.get();
}



void SHRecord::setData(Array* data)
	throw (Core::TypeException)
{
	switch (data->dataType()) {
	case Array::INT: // XXX will be serialized as float
	case Array::FLOAT:
	case Array::DOUBLE:
	// TODO: support for complex<float>, complex<double> needed
		break;
	default:
		throw Core::TypeException("illegal SH data type");
	}
	_data = data;
	_nsamp = _data->size();
	_datatype = _data->dataType();
}




void SHRecord::setData(int size, const void *data,
			Array::DataType dataType)
	throw (Core::TypeException)
{
	switch (dataType) {
	case Array::INT: // XXX will be serialized as float
	case Array::FLOAT:
	case Array::DOUBLE:
	// TODO: support for complex<float>, complex<double> needed
		break;
	default:
		throw Core::TypeException("illegal SH data type");
	}
	_data = ArrayFactory::Create(_datatype, dataType, size, data);
	_nsamp = _data->size();
}




SHRecord* SHRecord::copy() const
{
	return new SHRecord(*this);
}




void SHRecord::read(istream &in) throw(Core::StreamException)
{
}




void SHRecord::write(ostream &out) throw(Core::StreamException)
{
	if (sh_put(out, *this) != SH_SUCCESS)
		throw Core::StreamException();
}



static int
sh_put(ostream& os, const SHRecord& rec)
{
	//! write header+data to an ostream

	// date formatting: 05-AUG-2008_13:59:27.970
	std::string month = rec.startTime().toString("%b");
	std::transform(month.begin(), month.end(), month.begin(), ::toupper);
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(3) << boost::lexical_cast<double>(rec.startTime().toString("%S.%f"));
	std::string start = rec.startTime().toString("%d-") + month + rec.startTime().toString("-%Y_%H:%M:") + oss.str();

	os << "STATION: " << rec.stationCode() << std::endl;
	//os << "START: " << rec.startTime().toString("%d-%b-%Y_%H:%M:%S.%f") << std::endl;
 	os << "START: " << start << std::endl;
	os << "DELTA: " << 1.0/rec.samplingFrequency() << std::endl;
	os << "LENGTH: " << rec.data()->size() << std::endl;
	os << "COMP: " << rec.channelCode().substr(2,1) << std::endl;

	FloatArrayCPtr data = FloatArray::ConstCast(rec.data());
	for (int i = 0; i < rec.data()->size(); ++i)
		os <<  data->get(i) << std::endl;

	return SH_SUCCESS;
}


SHOutput::SHOutput(const std::string &filename)
: _ofstream(0)
{
	_filename = filename;
	
	if (_filename != "") {
		_ofstream = new std::ofstream(filename.c_str());
	}
}


SHOutput::SHOutput(const Record *rec)
: _ofstream(0)
{
	_filename = rec->startTime().toString("%Y_%j_")+rec->stationCode()+"_"+rec->channelCode()+".ASC";
	std::cerr << "SHOutput: " << _filename << std::endl;	
	_ofstream = new std::ofstream(_filename.c_str());
	sh_put(*_ofstream, SHRecord(*rec));
}


SHOutput::~SHOutput()
{
	if (_ofstream) {
		_ofstream->close();
		delete _ofstream;
	}
}

bool SHOutput::put(const SHRecord *rec) {
	if ( !rec ) return false;

	try {
		if ( _ofstream )
			sh_put(*_ofstream, *((SHRecord*)rec));
		else
			sh_put(std::cout, *((SHRecord*)rec));
	}
	catch(...) {
		return false;
	}
	
	return true;
}


}

}
