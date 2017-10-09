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


#ifndef __SEISCOMP_IO_RECORDS_SHRECORD_H__
#define __SEISCOMP_IO_RECORDS_SHRECORD_H__

#include <string>
#include <vector>
#include <ostream>

#include <seiscomp3/core/record.h>
#include <seiscomp3/core/array.h>
#include <seiscomp3/core.h>


namespace Seiscomp {

namespace IO {


DEFINE_SMARTPOINTER(SHRecord);

class SC_SYSTEM_CORE_API SHRecord : public Record {
	DECLARE_SC_CLASS(SHRecord);

    public:
	//! Default Constructor
//	SHRecord();

	//! Initializing Constructor
	SHRecord(std::string net="AB", std::string sta="ABC",
                 std::string loc="", std::string cha="XYZ",
                 Core::Time stime=Core::Time(), double fsamp=0., int tqual=-1,
                 Array::DataType dt = Array::DOUBLE, Hint h = DATA_ONLY);

	//! Copy Constructor
	SHRecord(const SHRecord& rec);
	SHRecord(const Record& rec);

	//! Destructor
	virtual ~SHRecord();

	//! Assignment operator
	SHRecord& operator=(const SHRecord& rec);

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
	void saveSpace() const {}

	//! Returns a deep copy of the calling object.
	SHRecord* copy() const;

	void read(std::istream &in);
	void write(std::ostream &out);


    private:
	ArrayPtr _data;

};

class SC_SYSTEM_CORE_API SHOutput {
	public:
		SHOutput() : _ofstream(0) {}
		SHOutput(const std::string& filename);
		SHOutput(const Record *rec);

		~SHOutput();
		
		bool put(const SHRecord *rec);

	private:
		std::string _filename;
		std::ofstream *_ofstream;
};

}

}

#endif
