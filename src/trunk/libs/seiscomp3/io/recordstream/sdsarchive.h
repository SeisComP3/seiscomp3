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

#include <fstream>
#include <queue>
#include <list>
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
		virtual bool setSource(const std::string &src);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &startTime,
		                       const Seiscomp::Core::Time &endTime);

		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);

		virtual void close();

		virtual Record *next();

		std::string archiveRoot() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		bool stepStream();
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
		std::string                          _arcroot;
		Seiscomp::Core::Time                 _stime;
		Seiscomp::Core::Time                 _etime;
		std::set<StreamIdx>                  _streamset;
		std::list<StreamIdx>                 _ordered;
		std::list<StreamIdx>::const_iterator _curiter;
		const StreamIdx                     *_curidx;
		std::queue<std::string>              _fnames;
		std::fstream                         _recstream;
		std::string                          _currentFilename;

	friend class IsoFile;
};


}
}

#endif
