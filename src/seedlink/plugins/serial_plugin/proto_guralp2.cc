/***************************************************************************** 
 * proto_guralp2.cc
 *
 * New Guralp protocol implementation, based on libgcf2
 *
 * (c) 2007 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstddef>

#include <unistd.h>
#include <time.h>
#include <sys/types.h>

#include "gcf2.h"
#include "qtime.h"

#include "utils.h"
#include "cppstreams.h"
#include "serial_plugin.h"
#include "plugin_channel.h"
#include "diag.h"

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

//*****************************************************************************
// G2Protocol
//*****************************************************************************

class G2Protocol: public Proto
  {
  private:
    map<string, rc_ptr<OutputChannel> > source_map;
    int last_year;
    int last_doy;
    int last_seq;

    static void block_callback(G2Serial* ser, G2SerBlock* block);

  public:
    G2Protocol(const string &myname): last_year(0), last_doy(0),
      last_seq(-1) {}

    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision);

    void flush_channels();
    void start();
  };

void G2Protocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    map<string, rc_ptr<OutputChannel> >::iterator p;

    if((p = source_map.find(source_id)) != source_map.end())
        throw PluginADInUse(source_id, p->second->channel_name);
    
    source_map[source_id] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void G2Protocol::flush_channels()
  {
    map<string, rc_ptr<OutputChannel> >::iterator p;
    for(p = source_map.begin(); p != source_map.end(); ++p)
        if(p->second != NULL) p->second->flush_streams();
  }

void G2Protocol::block_callback(G2Serial* ser, G2SerBlock* block)
  {
    G2Protocol* obj = reinterpret_cast<G2Protocol*>(ser->client_data);
    
    G2SerAck(ser, block);

    if(block->seq == obj->last_seq)
        return;

    obj->last_seq = block->seq;
    
    G2PBlock pb;
    if(G2ParseBlock(reinterpret_cast<G2Block*>(block), &pb))
      {
        logs(LOG_ERR) << "failed to parse block" << endl;
        return;
      }

    tm tm = G2UTC2tm(G2GTime2UTC(pb.start));

    EXT_TIME et;
    et.year = tm.tm_year + 1900;
    et.month = tm.tm_mon + 1;
    et.day = tm.tm_mday;
    et.hour = tm.tm_hour;
    et.minute = tm.tm_min;
    et.second = tm.tm_sec;
    et.usec = 0;
    et.doy = mdy_to_doy(et.month, et.day, et.year);
    digitime.it = ext_to_int(et);
    digitime.valid = true;
    digitime.exact = true;

    if((et.year > obj->last_year ||
        (et.year == obj->last_year && et.doy > obj->last_doy)) &&
      (et.hour != 0 || et.minute != 0 || et.second != 0))
      {
        seed_log << ident_str << SEED_NEWLINE
                 << "protocol: " << dconf.proto_name << endl;

        obj->last_year = et.year;
        obj->last_doy = et.doy;
      }
    
    if(pb.type == G2BLOCKTYPECDSTATUS)
      {
        digitime.quality = ((pb.d.cdstatus.gps_control == 0xff)?
          100: dconf.unlock_tq);
      }
    else if(pb.type == G2BLOCKTYPESTATUS)
      {
        int n = pb.records * 4;
        for(; n > 0 && pb.d.status[n-1] == ' '; --n);
        seed_log << string(pb.d.status, n) << flush;
      }
    else if(pb.type == G2BLOCKTYPEDATA)
      {
        map<string, rc_ptr<OutputChannel> >::iterator p;
        if((p = obj->source_map.find(pb.strid)) != obj->source_map.end())
          {
            p->second->set_timemark(digitime.it, 0, digitime.quality);
            
            for(int i = 0; i < pb.samples; ++i)
                p->second->put_sample(pb.d.data[i]);
          }
      }
  }      

void G2Protocol::start()
  {
    char* port_name = strdup(dconf.port_name.c_str());
    if(!port_name)
        throw bad_alloc();
    
    G2Serial* ser = G2SerOpen(port_name, dconf.port_bps, 0);
    free(port_name);
    
    if(ser == NULL)
        throw PluginCannotOpenPort(dconf.port_name);

    ser->client_data = reinterpret_cast<void*>(this);

    G2SerialP* parser = G2CreateSerParser(ser, block_callback, NULL, NULL, NULL, NULL);
    if(parser == NULL)
        throw PluginError("failed to create serial parser");

    int fd = G2SerFd(ser);
    G2ClrBlocking(fd);
  
    fd_set rfds;
    timeval tv;

    try
      {
        while(!terminate_proc)
          {
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);

            tv.tv_sec = 10;
            tv.tv_usec = 0;

            int r = select(fd + 1, &rfds, NULL, NULL, &tv);
            if(r < 0)
              {
                if(errno == EINTR) continue;
                throw PluginLibraryError("select error");
              }
            else if(r > 0)
              {
                G2SerDispatch (parser);
              }
          }
      }
    catch(PluginError &e)
      {
        seed_log << "closing device" << endl;
        G2DestroySerParser(parser);
        G2SerClose(ser);
        throw;
      }

    seed_log << "closing device" << endl;
    G2DestroySerParser(parser);
    G2SerClose(ser);
  }

RegisterProto<G2Protocol> proto("guralp2");

} // unnamed namespace

