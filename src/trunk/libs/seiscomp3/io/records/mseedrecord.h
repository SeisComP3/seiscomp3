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


#ifndef __SEISCOMP_IO_RECORDS_MSEEDRECORD_H__
#define __SEISCOMP_IO_RECORDS_MSEEDRECORD_H__

#include <string>
#include <boost/thread/mutex.hpp>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core.h>


typedef struct MSRecord_s MSRecord;

namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(MSeedRecord);


class SC_SYSTEM_CORE_API LibmseedException : public Core::StreamException {
 public:
	LibmseedException() : Core::StreamException("libmseed error") {}
	LibmseedException(std::string what) : Core::StreamException(what) {}
};


/**
 * Uses seiscomp error logging as component MSEEDRECORD.
 **/
class SC_SYSTEM_CORE_API MSeedRecord: public Record {
	DECLARE_SC_CLASS(MSeedRecord);

public:
	//! Initializing Constructor
	MSeedRecord(Array::DataType dt = Array::DOUBLE, Hint h = SAVE_RAW);

	//! Initializing Constructor
	MSeedRecord(MSRecord *msrec, Array::DataType dt = Array::DOUBLE, Hint h = SAVE_RAW);

	//! Copy Constructor
	MSeedRecord(const MSeedRecord &ms);

	//! Copy-from-Record  Constructor
	MSeedRecord(const Record &rec, int reclen=512);

	//! Destructor
	virtual ~MSeedRecord();

	//! Assignment Operator
	MSeedRecord& operator=(const MSeedRecord &ms);

	virtual void setNetworkCode(std::string net);

	//! Sets the station code
	virtual void setStationCode(std::string sta);

	//! Sets the location code
	virtual void setLocationCode(std::string loc);

	//! Sets the channel code
	virtual void setChannelCode(std::string cha);

	//! Sets the start time
	virtual void setStartTime(const Core::Time& time);

	//! Returns the sequence number
	int sequenceNumber() const;

	//! Sets the sequence number
	void setSequenceNumber(int seqno);

	//! Returns the data quality
	char dataQuality() const;

	//! Sets the data quality
	void setDataQuality(char qual);

	//! Returns the sample rate factor
	int sampleRateFactor() const;

	//! Sets the sample rate factor
	void setSampleRateFactor(int srfact);

	//! Returns the sample rate multiplier
	int sampleRateMultiplier() const;

	//! Sets the sample rate multiplier
	void setSampleRateMultiplier(int srmult);

	//! Returns the byteorder
	unsigned short byteOrder() const;

	//! Returns the encoding code
	unsigned short encoding() const;

	//! Returns the sample rate numerator
	int sampleRateNumerator() const;

	//! Returns the sample rate denominator
	int sampleRateDenominator() const;

	//! Returns the number of data frames
	int frameNumber() const;

	//! Returns the end time of data samples  
	const Seiscomp::Core::Time& endTime() const;

	//! Returns the length of a Mini SEED record
	int recordLength() const;

	//! Returns the leap seconds
	int leapSeconds() const;

	//! Returns a nonmutable pointer to the data samples if the data is available; otherwise 0
	//! (the data type is independent from the original one and was given by the DataType flag in the constructor)
	const Array* data() const throw(LibmseedException);

	const Array* raw() const;

	//! Frees the memory occupied by the decoded data samples.
	//! ! Use it with the hint SAVE_RAW only otherwise the data samples cannot be redecoded!
	void saveSpace() const;

	//! Returns a deep copy of the calling object.
	Record* copy() const;

	//! Sets flag specifying the encoding type of the write routine.
	//! true(default) -> use the encoding of the original record; false -> use the type of the data
	void useEncoding(bool flag);

	//! Sets the record length used for the output
	void setOutputRecordLength(int reclen);

	//! Extract the packed MSeedRecord attributes from the given stream
	void read(std::istream &in) throw(Core::StreamException);

	//! Encode the record into the given stream
	void write(std::ostream& out) throw(Core::StreamException);

private:
	CharArray _raw;
	mutable ArrayPtr _data;
	int _seqno;
	char _rectype;
	int _srfact;
	int _srmult;
	unsigned short _byteorder;
	unsigned short _encoding;
	int _srnum;
	int _srdenom;
	int _reclen;
	int _nframes;
	int _leap;
	Seiscomp::Core::Time _etime;
        bool _encodingFlag;

	void _setDataAttributes(int reclen, char *data) const
	     throw(Seiscomp::IO::LibmseedException);

	/* callback function for libmseed-function msr_pack(...) */
	static void _Record_Handler(char *record, int reclen, void *packed) {
	    /* to make the data available to the overloaded operator<< */
            reinterpret_cast<CharArray *>(packed)->append(reclen, record);
	}
	
};

} // namespace IO
} // namespace Seiscomp

#endif
