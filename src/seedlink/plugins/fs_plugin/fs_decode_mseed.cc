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
#include <cstdio>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>

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
const int STATLEN             = 5;
const int NETLEN              = 2;
const int LOCLEN              = 2;
const int CHLEN               = 3;

//*****************************************************************************
// FS_Decode_MSEED
//*****************************************************************************

class FS_Decode_MSEED: public FS_Decoder
  {
  private:
    map<string, string > channel_map;

    size_t readn(int fd, void *vptr, size_t n);
    void get_id(const sl_fsdh_s *fsdh, string &net, string &sta,
      string &loc, string &chn);
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

void FS_Decode_MSEED::get_id(const sl_fsdh_s *fsdh, string &net, string &sta,
  string &loc, string &chn)
  {
    for(int n = NETLEN; n > 0; --n)
        if(fsdh->network[n - 1] != ' ')
          {
            net = string(fsdh->network, n);
            break;
          }

    for(int n = STATLEN; n > 0; --n)
        if(fsdh->station[n - 1] != ' ')
          {
            sta = string(fsdh->station, n);
            break;
          }

    for(int n = LOCLEN; n > 0; --n)
        if(fsdh->location[n - 1] != ' ')
          {
            loc = string(fsdh->location, n);
            break;
          }

    for(int n = CHLEN; n > 0; --n)
        if(fsdh->channel[n - 1] != ' ')
          {
            chn = string(fsdh->channel, n);
            break;
          }
  }

int FS_Decode_MSEED::send_mseed_unpack(const string &channel_name,
  const void *dataptr, size_t data_size)
  {
    const sl_fsdh_s* fsdh = reinterpret_cast<const sl_fsdh_s *>(dataptr);
    SLMSrecord* msr = sl_msr_new();
    sl_msr_parse(NULL, reinterpret_cast<const char *>(dataptr), &msr, 1, 1);

    string net, sta, loc, chn;
    get_id(fsdh, net, sta, loc, chn);

    if(msr == NULL)
      {
        logs(LOG_ERR) << "error decoding Mini-SEED packet " <<
          string(fsdh->sequence_number, 6) <<
          ", station " << net << "_" << sta <<
          ", channel " << loc << "." << chn << endl;

        return data_size;
      }

    if(msr->numsamples < 0 || msr->numsamples > MAX_SAMPLES)
      {
        logs(LOG_ERR) << "error decoding Mini-SEED packet " <<
          string(fsdh->sequence_number, 6) <<
          ", station " << net << "_" << sta <<
          ", channel " << loc << "." << chn << endl;

        sl_msr_free(&msr);
        return data_size;
      }

    if(msr->numsamples == 0)
      {
        // not a data record
        sl_msr_free(&msr);
        return data_size;
      }

    int timing_quality = default_timing_quality, usec99 = 0;

    if(msr->Blkt1001 != NULL)
      {
        timing_quality = msr->Blkt1001->timing_qual;
        usec99 = msr->Blkt1001->usec;
      }

    struct ptime pt;
    pt.year = ntohs(fsdh->start_time.year);
    pt.yday = ntohs(fsdh->start_time.day);
    pt.hour = fsdh->start_time.hour;
    pt.minute = fsdh->start_time.min;
    pt.second = fsdh->start_time.sec;
    pt.usec = ntohs(fsdh->start_time.fract) * 100 + usec99;

    int r = 0;
    if(station_name.length() != 0)
      {
        r = send_raw3(station_name.c_str(), channel_name.c_str(), &pt,
          ntohs(fsdh->time_correct), timing_quality, msr->datasamples,
          msr->numsamples);
      }
    else
      {
        r = send_raw3((net + "." + sta).c_str(), channel_name.c_str(), &pt,
          ntohs(fsdh->time_correct), timing_quality, msr->datasamples,
          msr->numsamples);
      }
    
    DEBUG_MSG("sent " << r << " bytes of data, station \"" << station <<
      "\", channel \"" << channel << "\"" << endl);
    
    sl_msr_free(&msr);

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

