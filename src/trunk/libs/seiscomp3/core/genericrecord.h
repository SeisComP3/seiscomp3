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


#ifndef __SC_CORE_GENERICRECORD_H__
#define __SC_CORE_GENERICRECORD_H__

#include <string>
#include <iostream>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/array.h>

namespace Seiscomp {

DEFINE_SMARTPOINTER(GenericRecord);

class SC_SYSTEM_CORE_API GenericRecord : public Record {
	DECLARE_SC_CLASS(GenericRecord);
	DECLARE_SERIALIZATION;


 public:
	//! Default Constructor
	GenericRecord(Array::DataType dt = Array::DOUBLE, Hint h = DATA_ONLY);
	
	//! Initializing Constructor
	GenericRecord(std::string net, std::string sta,
	              std::string loc, std::string cha,
	              Core::Time stime, double fsamp, int tqual = -1,
	              Array::DataType dt = Array::DOUBLE,
	              Hint h = DATA_ONLY);
	
	//! Copy Constructor
	GenericRecord(const GenericRecord& rec);
	
	//! Another Constructor
	GenericRecord(const Record& rec);

	//! Destructor
	virtual ~GenericRecord();
	
	//! Assignment operator
	GenericRecord& operator=(const GenericRecord& rec);
	
	//! Sets the sample frequency
	void setSamplingFrequency(double freq);

	//! Returns the data samples if the data is available; otherwise 0
	Array* data();

	//! Returns the data samples if the data is available; otherwise 0
	const Array* data() const;

	//! Same as data()
	const Array* raw() const;

	//! Sets the data sample array. The ownership goes over to the record.
	void setData(Array* data);

	//! Sets the data sample array.
	void setData(int size, const void *data, Array::DataType datatype);	

	//! Updates internal parameters caused by data updates
	void dataUpdated();

	//! Frees the memory allocated for the data samples.
	void saveSpace() const;

	//! Returns a deep copy of the calling object.
	Record* copy() const;

	void read(std::istream &in) throw(Core::StreamException);
	void write(std::ostream &out) throw(Core::StreamException);
	
 private:
	ArrayPtr _data;
};
 
}

#endif
