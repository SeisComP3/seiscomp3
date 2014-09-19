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



#ifndef __SEISCOMP_IO_GFARCHIVE_ARCLINK_H__
#define __SEISCOMP_IO_GFARCHIVE_ARCLINK_H__


#include <seiscomp3/io/gfarchive.h>
#include <seiscomp3/io/socket.h>
#include <seiscomp3/io/archive/binarchive.h>

#include <string>
#include <list>


namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API ArclinkArchive : public GFArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		ArclinkArchive();
		ArclinkArchive(const std::string &url);

		//! D'tor
		~ArclinkArchive();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string);
		void close();

		bool setTimeSpan(const Core::TimeSpan &span);

		//! Adds a request for a greensfunction.
		bool addRequest(const std::string &id,
		                const std::string &model, double distance,
		                double depth);

		bool addRequest(const std::string &id,
		                const std::string &model, double distance,
		                double depth,
		                const Core::TimeSpan &span);

		Core::GreensFunction* get();


	private:
		int handshake();


	// ----------------------------------------------------------------------
	//  Private member
	// ----------------------------------------------------------------------
	private:
		struct Request {
			Core::TimeSpan timeSpan;
			std::string    id;
			std::string    model;
			double         distance;
			double         depth;
		};

		typedef std::list<Request> RequestList;

		std::string        _url;
		Core::TimeSpan     _defaultTimespan;
		RequestList        _requests;
		Socket             _sock;
		BinaryArchive      _archive;
		std::stringbuf     _buffer;
		std::string        _requestID;
};

}
}

#endif
