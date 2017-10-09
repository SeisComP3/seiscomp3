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


#define SEISCOMP_COMPONENT ISOFILE

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <zlib.h>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/recordstream/sdsarchive.h>

#include "isofile.h"


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::RecordStream;


IMPLEMENT_SC_CLASS_DERIVED(IsoFile,
                           Seiscomp::IO::RecordStream,
                           "IsoFile");

REGISTER_RECORDSTREAM(IsoFile, "isofile");

IsoFile::IsoFile() 
  : RecordStream(), _extrDir("/tmp/seiscomp3"), _readingData(false) {
    _recstream = RecordStream::Create("sdsarchive");
}

IsoFile::IsoFile(string name) 
  : RecordStream(), _extrDir("/tmp/seiscomp3"), _readingData(false) {
    setSource(name);
    _recstream = RecordStream::Create("sdsarchive");
}

IsoFile::IsoFile(const IsoFile &iso) 
  : RecordStream(), _extrDir("/tmp/seiscomp3"), _readingData(false) {
    setSource(iso.name());
    _recstream = RecordStream::Create("sdsarchive");
}

IsoFile::~IsoFile() {
    close();
}

IsoFile& IsoFile::operator=(const IsoFile &iso) {
    if (this != &iso) {
	_ifs.close();
	setSource(iso.name());
    }
  
    return *this;
}

bool IsoFile::setSource(string src) {
    _name = src;
    if (!_ifs.open(_name.c_str(),ISO_EXTENSION_ALL)) {
	SEISCOMP_ERROR("Opening ISO image %s failed",_name.c_str());
	return false;
    }

    return true;
}

bool IsoFile::addStream(string net, string sta, string loc, string cha) {
    return _recstream->addStream(net,sta,loc,cha);
}

bool IsoFile::addStream(string net, string sta, string loc, string cha, const Time &stime, const Time &etime) {
    return _recstream->addStream(net,sta,loc,cha,stime,etime);
}

bool IsoFile::removeStream(string net, string sta, string loc, string cha) {
    return SDSArchive::Cast(_recstream)->removeStream(net,sta,loc,cha);
}

bool IsoFile::setStartTime(const Time &stime) {
    return _recstream->setStartTime(stime);
}

bool IsoFile::setEndTime(const Time &etime) {
    return _recstream->setEndTime(etime);
}

bool IsoFile::setTimeWindow(const TimeWindow &w) {
    return _recstream->setTimeWindow(w);
}

bool IsoFile::setTimeout(int seconds) {
    return false;
}

void IsoFile::close() {
    _ifs.close();
    clean();
}

void IsoFile::clean() {
    /* remove the temporary files */
    for (set<string>::iterator it = _fnames.begin(); it != _fnames.end(); ++it)
        remove(it->c_str());
    _fnames.clear();
    /* remove the temporary directories */
    int count = _dnames.size();
    while (count) {
        for (map<string,string>::iterator it = _dnames.begin(); it != _dnames.end(); ++it) {
            string path = _extrDir + it->second;
            rmdir(path.c_str());
            size_t pos = it->second.find_last_of('/');
            if (pos != string::npos && pos > 0)
                it->second.erase(pos);
            else
                --count;
        }
    }
    _dnames.clear();
}

string IsoFile::name() const {
    return _name;
}

void IsoFile::uncompress(ifstream &isofile, ofstream &outfile, 
                         unsigned long long offset, unsigned long filesize) {
    const unsigned char zisofs_magic[8] = {
        0x37, 0xE4, 0x53, 0x96, 0xC9, 0xDB, 0xD6, 0x07
    };
    struct mkzftree_file_header {
        char magic[8];
        char uncompressed_len[4];
        unsigned char header_size;
        unsigned char block_size;
        char reserved[2];
    };
    struct mkzftree_file_header hdr;

    isofile.seekg(offset);
    if (isofile.good()) {
        isofile.read((char *)&hdr,sizeof(hdr));
        if (memcmp(&hdr.magic,zisofs_magic,sizeof(zisofs_magic))) {
            SEISCOMP_INFO("Maybe file is not compressed (with mkzftree); simply it will be copied!");
            isofile.seekg(offset);
            char c;
            while (filesize) {
                isofile.get(c);
                outfile.put(c);
                --filesize;
            }
        } else { 
            const short PTRSIZE = 4;
            unsigned long bsize = 1 << hdr.block_size;
            unsigned long csize, startptr, endptr;
            unsigned long long ptrpos = isofile.tellg();
            char *inbuf, *outbuf;
            
            try {
                outbuf = new char[bsize];
            } catch (std::bad_alloc &) {
                throw ArchiveException("Memory allocation failed");
            }
            
            do {
                startptr = isofile.get() + (isofile.get() << 8) + (isofile.get() << 16) + (isofile.get() << 24);
                endptr = isofile.get() + (isofile.get() << 8) + (isofile.get() << 16) + (isofile.get() << 24);
                csize = endptr - startptr;
                
                try {
                    inbuf = new char[csize];
                } catch (std::bad_alloc &) {
                    delete[] outbuf;
                    throw ArchiveException("Memory allocation failed");
                }
                
                ptrpos += PTRSIZE;         
                isofile.seekg(startptr+offset);
                isofile.read(inbuf,csize);
                
                if (::uncompress((Bytef *)outbuf,(uLongf *)&bsize,(Bytef *)inbuf,csize) != Z_OK) {
                    delete[] inbuf;
                    delete[] outbuf;
                    throw ArchiveException("Uncompressing file using zlib's uncompress() failed");
                }
                
                if (bsize != (unsigned long)(1 << hdr.block_size) && endptr != filesize) {
                    delete[] inbuf;
                    delete[] outbuf;
                    throw ArchiveException("Incorrect number of uncompressed bytes");
                }
                
                outfile.write(outbuf,bsize);
                if (!outfile.good()) {
                    delete[] inbuf;
                    delete[] outbuf;
                    throw ArchiveException("Writing uncompressed file failed");          
                }
                
                delete[] inbuf;
                isofile.seekg(ptrpos);
            }  while (filesize != endptr);
            delete[] outbuf;
        }
    } else 
        SEISCOMP_ERROR("Reading isofile failed!");
}

void IsoFile::createSDS(const string sdspath) {
    string tmppath = _extrDir + sdspath;
    size_t pos = tmppath.find('/');

    while (pos != string::npos) {
        if (pos > 0)
            if (mkdir(tmppath.substr(0,pos).c_str(),S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST)
                SEISCOMP_ERROR("Creating directory %s failed",tmppath.substr(0,pos).c_str());
        pos = tmppath.find('/',pos+1);
    }
    if (mkdir(tmppath.c_str(),S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST)
        SEISCOMP_ERROR("Creating directory %s failed",tmppath.c_str());
}

void IsoFile::extractFiles() {
    SDSArchivePtr recstream = SDSArchive::Cast(_recstream);
    for (recstream->_curiter = recstream->_streams.begin();
	 recstream->_curiter != recstream->_streams.end(); 
	 recstream->_curiter++) {

	recstream->_curidx = &*recstream->_curiter;
	recstream->setFilenames();
	while (!recstream->_fnames.empty()) {   
	    string sdsname = recstream->_fnames.front();
	    recstream->_fnames.pop();
            string sdspath = sdsname.substr(0,sdsname.find_last_of("/"));
	    SEISCOMP_DEBUG("get iso file information for %s",sdsname.c_str());
	    ISO9660::Stat *pstat = _ifs.stat(sdsname.c_str(),true);

	    if (!pstat || !pstat->p_stat)
		SEISCOMP_ERROR("Getting file information failed for %s",sdsname.c_str());
	    else {
                ISO9660::PVD *pvd;
                int blocksize = ISO_BLOCKSIZE;
                unsigned long lsn, filesize;
                unsigned long long offset;
                string fname = _extrDir + sdsname;
                _fnames.insert(fname);
                _dnames[sdspath] = sdspath;
                createSDS(sdspath);
                ifstream isofile(_name.c_str(),ios::binary);
                ofstream outfile(fname.c_str(),ios::binary);

                pvd = _ifs.read_pvd();
                if (!pvd) 
                    SEISCOMP_ERROR("Couldn't get the primary volume descriptor; Setting blocksize to %d",ISO_BLOCKSIZE);
                else 
                    blocksize = pvd->get_pvd_block_size();

                lsn = pstat->p_stat->lsn;
                offset = (unsigned long long)lsn * blocksize;
                filesize = pstat->p_stat->size;
                try {
                    uncompress(isofile,outfile,offset,filesize);
                } catch (ArchiveException ex) {
                    SEISCOMP_ERROR("Exception during uncompress: %s",ex.what());
                }

                outfile.close();		
                isofile.close();
	    }
	}
    }
    _recstream->setSource(_extrDir);
    _readingData = true;
}

istream& IsoFile::stream() {
    if (!_readingData)
	extractFiles();
    
    return _recstream->stream();
}
