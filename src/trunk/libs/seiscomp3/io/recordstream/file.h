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


#ifndef __SEISCOMP_SERVICES_RECORDSTREAM_RECORDFILE_H__
#define __SEISCOMP_SERVICES_RECORDSTREAM_RECORDFILE_H__

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace RecordStream {

DEFINE_SMARTPOINTER(File);

class SC_SYSTEM_CORE_API File : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(File);

	public:
		enum SeekDir {
			Begin = std::ios_base::beg,
			Current = std::ios_base::cur,
			End = std::ios_base::end
		};

	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		File();
		File(std::string name);
		File(const File &f);
		virtual ~File();


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		File& operator=(const File &f);


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string);

		//! The following five methods are not implemented yet
		//! and return always 'false'.
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
			const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);
		bool setStartTime(const Seiscomp::Core::Time &stime);
		bool setEndTime(const Seiscomp::Core::Time &etime);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
		bool setTimeout(int seconds);

		void close();

		std::string name() const;
		std::istream& stream();


	public:
		size_t tell();

		File &seek(size_t pos);
		File &seek(int off, SeekDir dir);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		std::string _name;
		std::fstream _fstream;
		std::istream* _current;
};
 
}
}

#endif
