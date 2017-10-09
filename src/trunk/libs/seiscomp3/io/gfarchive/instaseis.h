/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   -------------------------------------------------------------------   *
 *   Author: Jan Becker <jabe@gempa.de>                                    *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_IO_GFARCHIVE_INSTASEIS_H__
#define __SEISCOMP_IO_GFARCHIVE_INSTASEIS_H__


#include <seiscomp3/io/gfarchive.h>
#include <seiscomp3/io/socket.h>

#include <string>
#include <vector>
#include <list>


namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API Instaseis : public GFArchive {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Instaseis();
		Instaseis(const std::string &url);

		//! D'tor
		~Instaseis();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string);
		void close();

		std::list<std::string> availableModels() const;
		std::list<double> availableDepths(const std::string &model) const;

		bool setTimeSpan(const Core::TimeSpan &span);

		//! Adds a request for a greensfunction.
		bool addRequest(const std::string &id,
		                const std::string &model,
		                const GFSource &source,
		                const GFReceiver &receiver);

		bool addRequest(const std::string &id,
		                const std::string &model,
		                const GFSource &source,
		                const GFReceiver &receiver,
		                const Core::TimeSpan &span);

		Core::GreensFunction* get();

		OPT(double) getTravelTime(const std::string &phase,
		                          const std::string &model,
		                          const GFSource &source,
		                          const GFReceiver &receiver);


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		bool getInfo() const;


	// ----------------------------------------------------------------------
	//  Private member
	// ----------------------------------------------------------------------
	private:
		struct Request {
			Core::TimeSpan timeSpan;
			std::string    id;
			double         distance;
			double         depth;
		};

		typedef std::list<Request> RequestList;

		std::string          _host;
		std::string          _path;
		int                  _timeout;
		Core::TimeSpan       _defaultTimespan;
		RequestList          _requests;

		mutable Socket       _socket;
		mutable std::string  _model;

		mutable double       _dt;

		mutable int          _maxLength;
		mutable double       _srcShift;

		mutable double       _minDepth;
		mutable double       _maxDepth;

		mutable double       _minDist;
		mutable double       _maxDist;

		mutable bool         _hasInfo;
};


}
}


#endif
