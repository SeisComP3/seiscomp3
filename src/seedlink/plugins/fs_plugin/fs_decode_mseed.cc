/***************************************************************************** 
 * fs_decode_mseed.cc
 *
 * Mini-SEED decoder module for fs_plugin
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <string>
#include <map>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "qtime.h"
#include "qutils.h"
#include "sdr_utils.h"
#include "ms_unpack.h"

#include "libslink.h"

#include "fs_plugin.h"
#include "cppstreams.h"
#include "utils.h"
#include "diag.h"

#include "plugin.h"

namespace {

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

const int HEADER_SIZE         = 64;
const int MAX_PACKET_SIZE     = 4096;
const int MAX_SAMPLES         = ((MAX_PACKET_SIZE - HEADER_SIZE) * 2);

//*****************************************************************************
// FS_Decode_MSEED
//*****************************************************************************

class FS_Decode_MSEED: public FS_Decoder
  {
  private:
    map<string, string > channel_map;

    size_t readn(int fd, void *vptr, size_t n);
    void decoding_error(DATA_HDR *hdr);
    int send_mseed_unpack(const string &channel_name, const void *dataptr,
      size_t data_size);

  public:
    const string myname;
    
    FS_Decode_MSEED(const string &myname_init): myname(myname_init) { }
    
    void attach_output_channel(const string &source_id,
      const string &channel_name);
      
    void flush_channels();

    void process_file(const string &file, off_t offset, ssize_t len);
  };

size_t FS_Decode_MSEED::readn(int fd, void *vptr, size_t n)
  {
    ssize_t nread;
    size_t nleft = n;
    char* ptr = static_cast<char*>(vptr);
    
    while(nleft > 0)
      {
        nread = read(fd, ptr, nleft);
        if(nread < 0)
          {
            perror("read error");
            exit(1);
          }

        if(!nread) return 0;

        nleft -= nread;
        ptr += nread;
      }

    return(n);
  }

void FS_Decode_MSEED::decoding_error(DATA_HDR *hdr)
  {
    logs(LOG_ERR) << "error decoding Mini-SEED packet " << hdr->seq_no <<
      ", station " << hdr->station_id << ", channel " << hdr->location_id <<
      hdr->channel_id << endl;
  }

int FS_Decode_MSEED::send_mseed_unpack(const string &channel_name,
  const void *dataptr, size_t data_size)
  {
    DATA_HDR *hdr;
    BLOCKETTE_HDR *bh;
    BS *bs;
    int nsamples;
    int32_t data[MAX_SAMPLES];

    if((hdr = decode_hdr_sdr((SDR_HDR *)dataptr, data_size)) == NULL)
      {
        logs(LOG_ERR) << "invalid Mini-SEED packet" << endl;
        return data_size;
      }
    
    trim(hdr->station_id);
    trim(hdr->channel_id);
    trim(hdr->location_id);
    trim(hdr->network_id);
    
    int timing_quality = default_timing_quality, usec99 = 0;
    bool data_record = false;
    
    for(bs = hdr->pblockettes; bs != (BS *)NULL; bs = bs->next)
      {
        if(bs->wordorder != my_wordorder &&
          swab_blockette(bs->type, bs->pb, bs->len) == 0)
            bs->wordorder = my_wordorder;

        bh = (BLOCKETTE_HDR *) bs->pb;
        
        if(bh->type == 1000 && ((BLOCKETTE_1000 *)bh)->format != 0)
            data_record = true;

        if(bh->type == 1001)
          {
            timing_quality = ((BLOCKETTE_1001 *)bh)->clock_quality;
            usec99 = ((BLOCKETTE_1001 *)bh)->usec99;
          }
      }
    
    if(!data_record)
      {
        free_data_hdr(hdr);
        return data_size;
      }
            
    nsamples = ms_unpack(hdr, MAX_SAMPLES, (char *) dataptr, data);
    
    if(nsamples < 0)
      {
        decoding_error(hdr);
        free_data_hdr(hdr);
        return data_size;
      }
    
    // Already done by Qlib2!
    // INT_TIME it = add_time(hdr->hdrtime, 0, usec99);

    EXT_TIME et = int_to_ext(hdr->hdrtime);
    struct ptime pt;
    pt.year = et.year;
    pt.yday = et.doy;
    pt.hour = et.hour;
    pt.minute = et.minute;
    pt.second = et.second;
    pt.usec = et.usec;

    int r = 0;
    if(station_name.length() != 0)
      {
        r = send_raw3(station_name.c_str(), channel_name.c_str(), &pt,
          hdr->num_ticks_correction, timing_quality, data, nsamples);
      }
    else
      {
        r = send_raw3(hdr->station_id, channel_name.c_str(), &pt,
          hdr->num_ticks_correction, timing_quality, data, nsamples);
      }
    
    DEBUG_MSG("sent " << r << " bytes of data, station \"" << station <<
      "\", channel \"" << channel << "\"" << endl);
    
    free_data_hdr(hdr);

    if(r <= 0) return r;

    return data_size;
  }


void FS_Decode_MSEED::attach_output_channel(const string &source_id,
  const string &channel_name)
  {
    string src;

    switch(source_id.length())
      {
      case 5:
        src = string(source_id);
        break;

      case 3:
// hack for edata-recorder!
      case 2:
      case 1:
// end of hack
        src = string("  ") + string(source_id);
        break;

      default:
        throw PluginADInvalid(source_id, channel_name);
      }

    map<string, string >::iterator p;
    if((p = channel_map.find(src)) != channel_map.end())
        throw PluginADInUse(source_id, p->second);

    channel_map.insert(make_pair(src, channel_name));
  }

void FS_Decode_MSEED::flush_channels()
  {
    if(station_name.length() == 0)
        return;

    map<string, string >::iterator p;
    for(p = channel_map.begin(); p != channel_map.end(); ++p)
        if(send_flush3(station_name.c_str(), p->second.c_str()) < 0)
            throw PluginBrokenLink(strerror(errno));
  }

void FS_Decode_MSEED::process_file(const string &file, off_t offset,
  ssize_t len)
  {
    // offset and len (incremental reading) currently not supported,
    // we read the whole file at once

    char packet[MAX_PACKET_SIZE];
    sl_fsdh_s *fsdh = (sl_fsdh_s *) packet;
    sl_blkt_1000_s *blkt_1000 = (sl_blkt_1000_s *)((char *) fsdh + sizeof(sl_fsdh_s));

    int fd;
    if((fd = open(file.c_str(), O_RDONLY)) < 0)
      {
        logs(LOG_ERR) << "cannot open file '" << file << "'" << endl;
        return;
      }
    
    while(1)
      {
        if(!readn(fd, packet, HEADER_SIZE)) break;
        if((1 << blkt_1000->rec_len) > MAX_PACKET_SIZE)
          {
            logs(LOG_ERR) << "invalid record length, file '" << file << "'" << endl;
            break;
          }

        size_t data_size = (1 << blkt_1000->rec_len) - HEADER_SIZE;

        if(!readn(fd, packet + HEADER_SIZE, data_size))
          {
            logs(LOG_WARNING) << "unexpected end of file '" << file << "'" << endl;
            break;
          }

        string key = string(fsdh->location, 0, 2) + string(fsdh->channel, 0, 3);

        map<string, string>::iterator p;
        if((p = channel_map.find(key)) == channel_map.end()) continue;

        int r = send_mseed_unpack(p->second, packet, data_size);

        if(r < 0)
          {
            close(fd);
            throw PluginBrokenLink(strerror(errno));
          }
        else if(r == 0)
          {
            close(fd);
            throw PluginBrokenLink();
          }
      }

    close(fd);
  }

RegisterDecoder<FS_Decode_MSEED> decode("mseed");

} // unnamed namespace

