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


#ifndef __SEISCOMP_IO_RECORDS_SACRECORD_H__
#define __SEISCOMP_IO_RECORDS_SACRECORD_H__


#include <seiscomp3/core/record.h>
#include <seiscomp3/core/array.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(SACRecord);


class SC_SYSTEM_CORE_API SACRecord : public Record {
	public:
		//! Initializing Constructor
		SACRecord(const std::string &net = "AB", const std::string &sta = "12345",
		          const std::string &loc = "", const std::string &cha = "XYZ",
		          Core::Time stime = Core::Time(), double fsamp=0., int tqual=-1,
		          Array::DataType dt = Array::DOUBLE, Hint h = DATA_ONLY);

		//! Copy Constructor
		SACRecord(const SACRecord &rec);
		SACRecord(const Record &rec);

		//! Destructor
		virtual ~SACRecord();


	public:
		//! Assignment operator
		SACRecord &operator=(const SACRecord &other);

		//! Returns the data samples if the data is available; otherwise 0
		Array* data();

		//! Returns the data samples if the data is available; otherwise 0
		const Array* data() const;

		const Array* raw() const;

		//! Sets the data sample array. The ownership goes over to the record.
		void setData(Array* data);

		//! Sets the data sample array.
		void setData(int size, const void *data, Array::DataType datatype);

		//! Returns a deep copy of the calling object.
		SACRecord *copy() const;

		void saveSpace() const;

		void read(std::istream &in) throw(Core::StreamException);
		void write(std::ostream &out) throw(Core::StreamException);


	private:
		mutable ArrayPtr _data;

};


}
}

#endif
