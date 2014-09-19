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


#ifndef __SEISCOMP_IO_RECORDOUTPUTSTREAM_H__
#define __SEISCOMP_IO_RECORDOUTPUTSTREAM_H__

#include <iostream>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/io/recordstreamexceptions.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(RecordOutputStream);

class SC_SYSTEM_CORE_API RecordOutputStream : public Seiscomp::Core::InterruptibleObject {
	DECLARE_SC_CLASS(RecordOutputStream);

	protected:
		RecordOutputStream();

	public:
		virtual ~RecordOutputStream() {}

	public:
		virtual bool setTarget(std::string) = 0;
		virtual void close() = 0;

		virtual std::ostream& stream() = 0;

		//! Returns a record stream for the given service
		//! @return A pointer to the recordstream object
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordOutputStream* Create(const char* service);

		//! Returns a record stream for the given service that creates
		//! records of type recordType
		//! @return A pointer to the recordstream object. If the recordstream
		//!         does not support the requested type, NULL will be returned
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordOutputStream* Create(const char* service, const char* recordType);

		//! Opens a recordstream at source.
		//! @param url A source URL of format [service://]address[#type],
		//!            e.g. file:///data/record.mseed#mseed. service defaults to
		//!            'file' and the default type is 'mseed'
		//! @return A pointer to the recordstream object. If the recordstream
		//!         does not support the requested type, NULL will be returned
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordOutputStream* Open(const char* url);
};


DEFINE_INTERFACE_FACTORY(RecordOutputStream);


#define REGISTER_RECORDOUTPUTSTREAM(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::RecordOutputStreamFactory, Class> __##Class##InterfaceFactory__(Service)

}
}

#endif
