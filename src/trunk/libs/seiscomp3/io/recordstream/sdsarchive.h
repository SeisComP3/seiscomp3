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


#ifndef __SEISCOMP_IO_RECORDSTREAM_SDSARCHIVE_H__
#define __SEISCOMP_IO_RECORDSTREAM_SDSARCHIVE_H__

#include <iostream>
#include <sstream>
#include <queue>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordstream/archive.h>
#include <seiscomp3/io/recordstream/streamidx.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(SDSArchive);


/* This class allows the file access to a SDS data archive given by the 
   archive root and stream time windows (wildcarding not supported!!!)
   archive structure: 
   <root>/<year>/<net>/<sta>/<cha>.D/<net>.<sta>.<loc>.<cha>.D.<year>.<doy> */
class SC_SYSTEM_CORE_API SDSArchive:  public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(SDSArchive);

	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		SDSArchive();
		SDSArchive(const std::string arcroot);
		SDSArchive(const SDSArchive &arc);
		virtual ~SDSArchive();


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		SDSArchive& operator=(const SDSArchive &arc);


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string src);
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha);
		bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
		               const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);
		bool removeStream(std::string net, std::string sta, std::string loc, std::string cha);
		bool setStartTime(const Seiscomp::Core::Time &stime);
		bool setEndTime(const Seiscomp::Core::Time &etime);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
		bool setTimeout(int seconds);
		std::istream& stream() throw(ArchiveException);
		void close();
		std::string archiveRoot() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		Seiscomp::Core::Time getStartTime(const std::string &file);
		int getDoy(const Seiscomp::Core::Time &time);
		virtual std::string filename(int doy, int year);
		virtual void setFilenames();
		bool setStart(const std::string &fname);
		bool isEnd();


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		std::string                         _arcroot;
		Seiscomp::Core::Time                _stime;
		Seiscomp::Core::Time                _etime;
		std::set<StreamIdx>                 _streams;
		std::set<StreamIdx>::const_iterator _curiter;
		StreamIdx const                    *_curidx;
		std::queue<std::string>             _fnames;
		Seiscomp::IO::RecordStreamPtr       _recstream;

	friend class IsoFile;
};


}
}

#endif
