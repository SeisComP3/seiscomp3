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


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(IsoArchive);


/* This class allows the file access to an Iso9660 file archive given by the 
   archive root and stream time windows (wildcarding not supported!!!) 
   archive structure: <root>/<year>/<network>/<station>.<network>.<year>.iso */
class IsoArchive:  public Seiscomp::IO::RecordStream {
    DECLARE_SC_CLASS(IsoArchive);

public:
    // ----------------------------------------------------------------------
    //  Xstruction
    // ----------------------------------------------------------------------
    IsoArchive();
    IsoArchive(const std::string arcroot);
    IsoArchive(const IsoArchive &arc);
    virtual ~IsoArchive();
    
    // ----------------------------------------------------------------------
    //  Operators
    // ----------------------------------------------------------------------
    IsoArchive& operator=(const IsoArchive &arc);

    // ----------------------------------------------------------------------
    //  Public Interface
    // ----------------------------------------------------------------------
    bool setSource(std::string src);
    bool addStream(std::string net, std::string sta, std::string loc, std::string cha);
    bool addStream(std::string net, std::string sta, std::string loc, std::string cha,
		   const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);
    bool removeStream(std::string net, std::string sta, std::string loc, std::string cha);
    bool setStartTime(const Seiscomp::Core::Time &stime);
    bool setEndTime(const Seiscomp::Core::Time &etime);
    bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);
    bool setTimeout(int seconds);
    std::istream& stream();
    void close();
    std::string archiveRoot() const;
		
private:
    // ----------------------------------------------------------------------
    //  Implementation
    // ----------------------------------------------------------------------
    std::string _arcroot;
    Seiscomp::Core::Time _stime;
    Seiscomp::Core::Time _etime;
    std::set<StreamIdx> _streams;
    std::map<std::string, std::set<StreamIdx *> >::iterator _curiter;
    std::map<std::string, std::set<StreamIdx *> > _fnames;    
    Seiscomp::IO::RecordStreamPtr _recstream;

    void setFilenames();
};

}
}

#endif
