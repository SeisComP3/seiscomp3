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



#include <seiscomp3/io/gfarchive.h>
#include <map>

#include "db.h"


class KIWIArchive : public Seiscomp::IO::GFArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		KIWIArchive();
		KIWIArchive(const std::string &url);

		//! D'tor
		~KIWIArchive();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string);
		void close();

		std::list<std::string> availableModels() const;
		std::list<double> availableDepths(const std::string &model) const;

		bool setTimeSpan(const Seiscomp::Core::TimeSpan &span);

		//! Adds a request for a greensfunction.
		bool addRequest(const std::string &id,
		                const std::string &model, double distance,
		                double depth);

		bool addRequest(const std::string &id,
		                const std::string &model, double distance,
		                double depth,
		                const Seiscomp::Core::TimeSpan &span);

		Seiscomp::Core::GreensFunction* get();


	// ----------------------------------------------------------------------
	//  Private member
	// ----------------------------------------------------------------------
	private:
		struct Request {
			Seiscomp::Core::TimeSpan timeSpan;
			std::string              id;
			std::string              model;
			double                   distance;
			double                   depth;
		};

		typedef std::list<Request> RequestList;

		std::string                _baseDirectory;
		Seiscomp::Core::TimeSpan   _defaultTimespan;
		RequestList                _requests;

		std::map<std::string, DB*> _models;
};

