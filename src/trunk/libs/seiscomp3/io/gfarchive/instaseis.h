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
		                const std::string &model, double distance,
		                double depth);

		bool addRequest(const std::string &id,
		                const std::string &model, double distance,
		                double depth,
		                const Core::TimeSpan &span);

		Core::GreensFunction* get();


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

		Socket             _socket;
		std::string        _model;

		std::string        _host;
		Core::TimeSpan     _defaultTimespan;
		RequestList        _requests;

		double             _dt;

		int                _maxLength;
		double             _srcShift;

		double             _minDepth;
		double             _maxDepth;

		double             _minDist;
		double             _maxDist;

		int                _timeout;
};


}
}


#endif
