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


#ifndef __SEISCOMP_IO_RECORDSTREAM_H__
#define __SEISCOMP_IO_RECORDSTREAM_H__

#include <iostream>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/io/recordstreamexceptions.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(RecordStream);

class SC_SYSTEM_CORE_API RecordStream : public Seiscomp::Core::InterruptibleObject {
	DECLARE_SC_CLASS(RecordStream);

	protected:
		RecordStream();

	public:
		virtual ~RecordStream() {}

	public:
		//! Sets the type of the record to be generated
		//! Currently the following record types are supported:
		//!  - 'ah'     AH format
		//!  - 'mseed'  MiniSeed
		virtual bool setRecordType(const char*);

		virtual bool setSource(std::string) = 0;
		virtual void close() = 0;

		//! Adds a seismic stream request to the record stream
		virtual bool addStream(std::string net, std::string sta, std::string loc, std::string cha) = 0;

		//! Adds a seismic stream request to the record stream
		virtual bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
		const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime) = 0;

		//! Sets the given start time
		virtual bool setStartTime(const Seiscomp::Core::Time &stime) = 0;

		//! Sets the given end time
		virtual bool setEndTime(const Seiscomp::Core::Time &etime) = 0;

		//! Sets the given time window
		virtual bool setTimeWindow(const Seiscomp::Core::TimeWindow &w) = 0;

		//! Sets timeout
		virtual bool setTimeout(int seconds) = 0;

		virtual std::istream& stream() = 0;

		virtual Record* createRecord(Array::DataType, Record::Hint);

		//! Notifies the stream about a successfully stored record.
		//! The default implementation does nothing.
		virtual void recordStored(Record*);

		//! Returns a record stream for the given service
		//! @return A pointer to the recordstream object
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordStream* Create(const char* service);

		//! Returns a record stream for the given service that creates
		//! records of type recordType
		//! @return A pointer to the recordstream object. If the recordstream
		//!         does not support the requested type, NULL will be returned
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordStream* Create(const char* service, const char* recordType);

		//! Opens a recordstream at source.
		//! @param url A source URL of format [service://]address[#type],
		//!            e.g. file:///data/record.mseed#mseed. service defaults to
		//!            'file' and the default type is 'mseed'
		//! @return A pointer to the recordstream object. If the recordstream
		//!         does not support the requested type, NULL will be returned
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordStream* Open(const char* url);

	private:
		RecordFactory* _factory;
};


DEFINE_INTERFACE_FACTORY(RecordStream);


#define REGISTER_RECORDSTREAM(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::RecordStream, Class> __##Class##InterfaceFactory__(Service)

}
}

#endif
