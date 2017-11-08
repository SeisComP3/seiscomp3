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


#ifndef __SEISCOMP_IO_RECORDSTREAM_ISOARCHIVE_H__
#define __SEISCOMP_IO_RECORDSTREAM_ISOARCHIVE_H__

#include <set>
#include <map>
#include <iostream>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordstream/archive.h>
#include <seiscomp3/io/recordstream/streamidx.h>

#include "isofile.h"


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(IsoArchive);


/**
 * This class allows the file access to an Iso9660 file archive given by the
 * archive root and stream time windows (wildcarding not supported!!!)
 * archive structure: <root>/<year>/<network>/<station>.<network>.<year>.iso
 */
class IsoArchive:  public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(IsoArchive);

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		IsoArchive();
		IsoArchive(const std::string arcroot);
		IsoArchive(const IsoArchive &arc);
		virtual ~IsoArchive();

	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		IsoArchive &operator=(const IsoArchive &arc);


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

		virtual bool setStartTime(const Seiscomp::Core::Time &startTime);
		virtual bool setEndTime(const Seiscomp::Core::Time &endTime);
		virtual void close();
		virtual Record *next();

		std::string archiveRoot() const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		void setFilenames();

		std::string _arcroot;
		Seiscomp::Core::Time _stime;
		Seiscomp::Core::Time _etime;
		std::set<StreamIdx> _streams;
		std::map<std::string, std::set<StreamIdx *> >::iterator _curiter;
		std::map<std::string, std::set<StreamIdx *> > _fnames;

		IsoFilePtr _recstream;
};


}
}


#endif
