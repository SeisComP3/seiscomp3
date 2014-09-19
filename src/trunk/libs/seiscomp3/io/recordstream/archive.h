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


#ifndef __SEISCOMP_IO_RECORDSTREAM_ARCHIVE_H__
#define __SEISCOMP_IO_RECORDSTREAM_ARCHIVE_H__

#include <iostream>
#include <sstream>
#include <stack>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API ArchiveException: public Seiscomp::IO::RecordStreamException {
	public:
		ArchiveException(): RecordStreamException("Archive exception") {}
		ArchiveException(const std::string& what): RecordStreamException(what) {}
};


DEFINE_SMARTPOINTER(Archive);


/* This class allows the file access to a (certain tree of a) data archive 
   given by the archive root (or subtree) */
class SC_SYSTEM_CORE_API Archive:  public Seiscomp::IO::RecordStream {
    DECLARE_SC_CLASS(Archive);
		
public:
    // ----------------------------------------------------------------------
    //  Xstruction
    // ----------------------------------------------------------------------
    Archive();
    Archive(const std::string arcroot);
    Archive(const Archive &arc);
    virtual ~Archive();
    
    // ----------------------------------------------------------------------
    //  Operators
    // ----------------------------------------------------------------------
    Archive& operator=(const Archive &arc);

    // ----------------------------------------------------------------------
    //  Public Interface
    // ----------------------------------------------------------------------
    bool setSource(std::string src);
    bool addStream(std::string net, std::string sta, std::string loc, std::string cha);
    bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
		   const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);
    bool setStartTime(const Seiscomp::Core::Time &stime);
    bool setEndTime(const Seiscomp::Core::Time &etime);
    bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
    bool setTimeout(int seconds);
    std::istream& stream() throw (ArchiveException);
    void close();
    std::string archiveRoot() const;
		
private:
    // ----------------------------------------------------------------------
    //  Implementation
    // ----------------------------------------------------------------------
    std::string _arcroot;
    std::stack<std::string> _dirstack;
    Seiscomp::IO::RecordStreamPtr _recstream;
};

}
}

#endif
