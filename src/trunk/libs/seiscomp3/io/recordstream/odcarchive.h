/* *************************************************************************
 *   Copyright (C) 2006 by GFZ Potsdam
 *
 *   $Author:  $
 *   $Email:  $
 *   $Date:  $
 *   $Revision:  $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * *************************************************************************/

#ifndef __SEISCOMP_IO_RECORDSTREAM_ODCARCHIVE_H__
#define __SEISCOMP_IO_RECORDSTREAM_ODCARCHIVE_H__

#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordstream/archive.h>
#include <seiscomp3/io/recordstream/streamidx.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(ODCArchive);


/* This class allows the file access to a ODC data archive given by the 
   archive root and stream time windows (wildcarding not supported!!!)
   archive structure: 
   <root>/<year>/<doy>/<sta>.<cha>[_<loc>].<net>.<year>.<doy> */
class SC_SYSTEM_CORE_API ODCArchive:  public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(ODCArchive);

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		ODCArchive();
		ODCArchive(const std::string arcroot);
		ODCArchive(const ODCArchive &arc);
		virtual ~ODCArchive();


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		ODCArchive &operator=(const ODCArchive &arc);


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
		                       const Seiscomp::Core::Time &stime,
		                       const Seiscomp::Core::Time &etime);

		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);
		virtual void close();

		virtual Record *next();

		std::string archiveRoot() const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		bool stepStream();

		int getDoy(const Seiscomp::Core::Time &time);
		std::string ODCfilename(int doy, int year);
		void setFilenames();
		bool setStart(const std::string &fname);
		bool isEnd();


	private:
		std::string _arcroot;
		Seiscomp::Core::Time _stime;
		Seiscomp::Core::Time _etime;
		std::set<StreamIdx> _streams;
		std::set<StreamIdx>::const_iterator _curiter;
		StreamIdx const *_curidx;
		std::queue<std::string> _fnames;
		std::fstream _recstream;
};

}
}

#endif
