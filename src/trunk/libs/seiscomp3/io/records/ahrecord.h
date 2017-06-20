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


#ifndef __SEISCOMP_IO_RECORDS_AHRECORD_H__
#define __SEISCOMP_IO_RECORDS_AHRECORD_H__

#include <string>
#include <vector>
#include <complex>
#include <ostream>

#include <seiscomp3/core/record.h>
#include <seiscomp3/core/array.h>
#include <seiscomp3/core.h>

// AH format specifics - DO NOT CHANGE
#define NEXTRAS 21
#define AH_DATATYPE_UNDEFINED 0
#define AH_DATATYPE_FLOAT     1
#define AH_DATATYPE_COMPLEX   2 // not (yet) supported
#define AH_DATATYPE_VECTOR    3 // not supported
#define AH_DATATYPE_TENSOR    4 // not supported
#define AH_DATATYPE_DOUBLE    6 // not (yet) supported

namespace Seiscomp {

namespace IO {


DEFINE_SMARTPOINTER(AHRecord);

class SC_SYSTEM_CORE_API AHRecord : public Record {
	DECLARE_SC_CLASS(AHRecord);

    public:
	//! Default Constructor
//	AHRecord();

	//! Initializing Constructor
	AHRecord(std::string net="AB", std::string sta="ABC",
                 std::string loc="", std::string cha="XYZ",
                 Core::Time stime=Core::Time(), double fsamp=0., int tqual=-1,
                 Array::DataType dt = Array::DOUBLE, Hint h = DATA_ONLY);

	//! Copy Constructor
	AHRecord(const AHRecord& rec);
	AHRecord(const Record& rec);

	//! Destructor
	virtual ~AHRecord();

	//! Assignment operator
	AHRecord& operator=(const AHRecord& rec);

	//! Returns the sample frequency
//	double samplingFrequency() const;

	//! Sets the sample frequency
	void setSamplingFrequency(double freq);

	//! Returns the data samples if the data is available; otherwise 0
	Array* data();

	//! Returns the data samples if the data is available; otherwise 0
	const Array* data() const;

	const Array* raw() const;

	//! Sets the data sample array. The ownership goes over to the record.
	void setData(Array* data);

	//! Sets the data sample array.
	void setData(int size, const void *data, Array::DataType datatype);

	//! Frees the memory allocated for the data samples.
	void saveSpace() const { /* no effect for AH records*/ }

	//! Returns a deep copy of the calling object.
	AHRecord* copy() const;

	void read(std::istream &in);

	void write(std::ostream &out);


	//! get/set gain
	float gain() const;
	void setGain(float value);

	//! get/set 'extra' fields
	float extra(int i) const;
	void setExtra(int i, float value);

	// FIXME
	// The following public members are here only temporarily. Not clear,
	// what to do with them.
	float elat, elon, edep;
	Core::Time etim;
	std::string com, ecom, log;
//	char xcom[80], ecom[80], log[202];
	float slat, slon, salt, sgain,snorm;
	float rmin, maxamp, delta;
	std::vector<float> cal; // pass-through only
	float _delta; // sampling interval instead of sampling frequency

    protected:
	void _reset();

    private:
	ArrayPtr _data;

	float _extra[NEXTRAS];
	float _gain;
};

class SC_SYSTEM_CORE_API AHOutput {
	public:
		AHOutput() : _ofstream(0) {}
		AHOutput(const std::string &filename);
		~AHOutput();
		
		bool put(AHRecord *rec);

	private:
		std::string _filename;
		std::ofstream *_ofstream;
};

SC_SYSTEM_CORE_API AHRecord* read_one(std::istream &is);
SC_SYSTEM_CORE_API bool write_one(AHRecord *rec, std::ostream &os);

}

}

#endif
