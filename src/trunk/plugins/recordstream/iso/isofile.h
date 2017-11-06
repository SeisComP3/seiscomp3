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


#ifndef __SEISCOMP_IO_RECORDSTREAM_ISOFILE_H__
#define __SEISCOMP_IO_RECORDSTREAM_ISOFILE_H__

#include <set>
#include <map>
#include <cdio++/iso9660.hpp>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordstream/sdsarchive.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(IsoFile);


/* This class allows the access to an ISO9660 image file given by the 
   file name and stream time windows (wildcarding not supported!!!) 
   The image contains compressed files in SeisComP Data Structure (SDS) */
class IsoFile:  public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(IsoFile);


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		IsoFile();
		IsoFile(const std::string name);
		IsoFile(const IsoFile &iso);
		virtual ~IsoFile();


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		IsoFile& operator=(const IsoFile &iso);


	// ----------------------------------------------------------------------
	//  RecordStream Interface
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
		virtual bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
		virtual void close();
		virtual Record *next();

		void clean();
		std::string name() const;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		void uncompress(std::ifstream &isofile, std::ofstream &outfile,
		                unsigned long long offset, unsigned long filesize);
		void createSDS(const std::string &sdspath);
		void extractFiles();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		std::string _name;
		std::string _extrDir;
		bool _readingData;
		ISO9660::IFS _ifs;
		SDSArchivePtr _recstream;
		std::map<std::string,std::string> _dnames;
		std::set<std::string> _fnames;
};


}
}


#endif
