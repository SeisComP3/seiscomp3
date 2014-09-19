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



#ifndef __SEISCOMP_IO_GFARCHIVE_HELMBERGER_H__
#define __SEISCOMP_IO_GFARCHIVE_HELMBERGER_H__


#include <seiscomp3/io/gfarchive.h>

#include <string>
#include <map>
#include <list>
#include <set>


namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API HelmbergerArchive : public GFArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		HelmbergerArchive();
		HelmbergerArchive(const std::string &baseDirectory);

		//! D'tor
		~HelmbergerArchive();


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
		bool hasModel(const std::string &) const;
		Core::GreensFunction* read(const std::string &file,
		                           const Core::TimeSpan &ts, double timeOfs);


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
		typedef std::set<double> DoubleList;

		struct ModelConfig {
			double velocity;
			DoubleList distances;
			DoubleList depths;
		};

		typedef std::map<std::string, ModelConfig> ModelMap;

		ModelMap           _models;
		std::string        _baseDirectory;
		Core::TimeSpan     _defaultTimespan;
		RequestList        _requests;
};

}
}

#endif
