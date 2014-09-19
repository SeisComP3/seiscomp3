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
#include <seiscomp3/io/recordstream/archive.h>


namespace Seiscomp {
namespace RecordStream {


    DEFINE_SMARTPOINTER(IsoFile);


/* This class allows the access to an ISO9660 image file given by the 
   file name and stream time windows (wildcarding not supported!!!) 
   The image contains compressed files in SeisComP Data Structure (SDS) */
class IsoFile:  public Seiscomp::IO::RecordStream {
    DECLARE_SC_CLASS(IsoFile);

	
public:
    // ----------------------------------------------------------------------
    //  Xstruction
    // ----------------------------------------------------------------------
    IsoFile();
    IsoFile(const std::string name);
    IsoFile(const IsoFile &iso);
    virtual ~IsoFile();
    
    // ----------------------------------------------------------------------
    //  Operators
    // ----------------------------------------------------------------------
    IsoFile& operator=(const IsoFile &iso);

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
    std::istream& stream() throw(ArchiveException);
    void close();
    void clean();
    std::string name() const;
		
private:
    // ----------------------------------------------------------------------
    //  Implementation
    // ----------------------------------------------------------------------
    std::string _name;
    std::string _extrDir;
    bool _readingData;
    ISO9660::IFS _ifs;
    Seiscomp::IO::RecordStreamPtr _recstream;
    std::map<std::string,std::string> _dnames;
    std::set<std::string> _fnames;

    void uncompress(ifstream &isofile, ofstream &outfile, 
                    unsigned long long offset, unsigned long filesize);
    void createSDS(const std::string sdspath);
    void extractFiles();
};

}
}

#endif
