/***************************************************************************** 
 * proto_dr24.cc
 *
 * Geotech DR-24 HLCP protocol implementation
 *
 * (c) 2003 Andres Heinloo, GFZ Potsdam
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "qtime.h"

#include "utils.h"
#include "cppstreams.h"
#include "serial_plugin.h"
#include "plugin_channel.h"
#include "diag.h"

#include "little-endian.h"

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

const int DATA_CHANNELS     = 16;                               // 0...15
const int SOH_CHANNELS      = 16;                               // 16...31
const int NCHAN             = DATA_CHANNELS + SOH_CHANNELS;     // 32
const int SOH_SAMPLE_TIME   = 10;
const int RECVBUFSIZE       = 5120;
const int SENDBUFSIZE       = 256;
const int READ_TIMEOUT      = 10;

//*****************************************************************************
// Data Structures
//*****************************************************************************

struct DataMsgHeader
  {
    char type;
    le_u_int16_t serial;
    unsigned char clk_chn;
    unsigned char sample_rate;
    le_u_int32_t time_tag;
    le_u_int16_t soh;
    char reseved;
  } PACKED;

//*****************************************************************************
// DR24Protocol
//*****************************************************************************

class DR24Protocol: public Proto
  {
  private:
    int fd;
    char recvbuf[RECVBUFSIZE], sendbuf[SENDBUFSIZE];
    unsigned short crctab[256];
    vector<rc_ptr<OutputChannel> > dr24_channels;
    time_t timing_quality_update;
    time_t last_soh_time;
    time_t last_data_time;
    time_t last_channel_time[DATA_CHANNELS];
    int soh[SOH_CHANNELS];
    int soh_start_index;
    int sendsize, rp;
    int nak_count;
    bool soh_initialized;
    bool soh_message;
    bool startup_message;
    
    void decode_data(const char *buf, int len);
    void decode_command_response(const char *buf, int len);
    void decode_unsolicited_response(const char *buf, int len);
    void recv_ack();
    void recv_nack();
    void recv_can();
    void send_frame(const char *buf, int len);
    void send_msg(const char *buf, int len);
    void send_ack();
    void send_nack();
    void send_can();
    void start_acquisition();
    void accept_frame();
    void reject_frame();
    void decode_frame(const char *buf, int len);
    void do_start();

    // Derived from C code on page A-50 of DR-24 manual
    void crcinit()
      {
        int i, j;
        unsigned int crc;

        for(i = 0; i < 256; ++i)
          {
            crc = (i << 8);
            for(j = 0; j < 8; ++j)
                crc = (crc << 1) ^ ((crc & 0x8000)? 0x1021 : 0);

            crctab[i] = crc;
          }
      }

    // Derived from assembler code on page A-51 of DR-24 manual
    unsigned int updcrc(unsigned char c, unsigned int crc)
      {
        return (crctab[((crc >> 8) ^ c) & 0xff] ^ (crc << 8)) & 0xffff;
      }

  public:
    DR24Protocol(const string &myname):
      dr24_channels(NCHAN), timing_quality_update(0), last_soh_time(0),
      last_data_time(0), soh_start_index(-1), sendsize(0), rp(0),
      nak_count(0), soh_initialized(false), soh_message(true),
      startup_message(true)
      {
        memset(soh, 0, sizeof(soh));
        crcinit();
      }

    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision);
    void flush_channels();
    void start();
  };

void DR24Protocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);
    
    if(*tail || n >= NCHAN)
        throw PluginADInvalid(source_id, channel_name);

    if(dr24_channels[n] != NULL)
        throw PluginADInUse(source_id, dr24_channels[n]->channel_name);

    dr24_channels[n] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void DR24Protocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(dr24_channels[n] == NULL) continue;
        dr24_channels[n]->flush_streams();
      }
  }

void DR24Protocol::start()
  {
    fd = open_port(O_RDWR);

    try
      {
        do_start();
      }
    catch(PluginError &e)
      {
        seed_log << "closing device" << endl;
        close(fd);
        throw;
      }

    seed_log << "closing device" << endl;
    close(fd);
  }

void DR24Protocol::decode_data(const char *buf, int len)
  {
    if(len < (int)sizeof(DataMsgHeader))
      {
        logs(LOG_WARNING) << "data frame is too short" << endl;
        return;
      }

    DataMsgHeader* dmh = (DataMsgHeader*) buf;

    DEBUG_MSG("serial = " << dmh->serial << endl <<
              "clk_chn = " << hex << int(dmh->clk_chn) << "h" << dec << endl <<
              "sample_rate = " << int(dmh->sample_rate) << endl <<
              "time_tag = " << dmh->time_tag << endl <<
              "soh [" << (dmh->soh >> 12) << "] = " <<
                (dmh->soh & 0xfff) << endl);

    int sample_rate = 0;
    switch(dmh->sample_rate)
      {
      case 20: sample_rate = 10; break;
      case 21: sample_rate = 20; break;
      case 22: sample_rate = 40; break;
      case 23: sample_rate = 50; break;
      case 24: sample_rate = 60; break;
      case 25: sample_rate = 80; break;

      case 30: sample_rate = 100; break;
      case 31: sample_rate = 120; break;
      case 32: sample_rate = 125; break;
      case 33: sample_rate = 200; break;
      case 34: sample_rate = 250; break;
      case 35: sample_rate = 500; break;

      case 40: sample_rate = 1000; break;

      default:
        logs(LOG_WARNING) << "unknown sample rate (code " <<
          dmh->sample_rate << ")" << endl;
      }
    
    int bytes_per_sample = 0;
    int bits_per_sample = 0;
    switch(dmh->type)
      {
      case 0x00: bytes_per_sample = 2; bits_per_sample = 16; break;
      case 0x04: bytes_per_sample = 3; bits_per_sample = 20; break;
      case 0x08: bytes_per_sample = 3; bits_per_sample = 24; break;
      case 0x0c: bytes_per_sample = 4; bits_per_sample = 28; break;
      case 0x10: bytes_per_sample = 4; bits_per_sample = 32; break;

      default:
        logs(LOG_WARNING) << "unsupported data frame type (" <<
          hex << dmh->type << "h)" << dec << endl;
      }
    
    int soh_index = dmh->soh >> 12;
    if(soh_start_index == -1) soh_start_index = soh_index;
    else if(soh_index == soh_start_index) soh_initialized = true;

    soh[soh_index] = (dmh->soh & 0xfff);
    
    int data_index = (dmh->clk_chn & 0xf);

    time_t time_tag = dmh->time_tag;
    tm *ptm = gmtime(&time_tag);
    EXT_TIME et;

    et.year = ptm->tm_year + 1900;
    et.month = ptm->tm_mon + 1;
    et.day = ptm->tm_mday;
    et.hour = ptm->tm_hour;
    et.minute = ptm->tm_min;
    et.second = ptm->tm_sec;
    et.usec = 0;
    et.doy = mdy_to_doy(et.month, et.day, et.year);
    digitime.it = ext_to_int(et);
    digitime.valid = true;
    
    if(dmh->clk_chn & 0x10)
      {
        digitime.exact = true;
        digitime.quality = 100;
        timing_quality_update = time_tag;
      }
    else if(digitime.quality > dconf.unlock_tq &&
      time_tag >= timing_quality_update + 10)
      {
        --digitime.quality;
        timing_quality_update = time_tag;
      }

    if(time_tag > last_data_time)
      {
        if(et.hour == 0 && et.minute == 0 && et.second == 1)
          {
            startup_message = true;
            soh_message = true;
          }

        if(dconf.statusinterval && !(time_tag % (dconf.statusinterval * 60)))
            soh_message = true;
      }
    
    if(startup_message)
      {
        startup_message = false;
        seed_log << ident_str << endl
                 << "Digitizer serial no.: " << dmh->serial << endl
                 << "Sample rate: " << sample_rate << endl
                 << "Bits per sample: " << bits_per_sample << endl
                 << "Program setup: "
                    "protocol=" << dconf.proto_name << " "
                    "lsb=" << dconf.lsb << " "
                    "statusinterval=" << dconf.statusinterval << endl;
      }

    if(soh_message && soh_initialized &&
      !(et.hour == 0 && et.minute == 0 && et.second == 0))
      {
        soh_message = false;
        seed_log << fixed
                 << "State of health: "
                 << setprecision(3) << double(soh[0]) * (-0.004884) + 10.0 << "V "
                 << setprecision(2) << double(soh[1]) * 0.030525 - 50.0 << "C "
                 << setprecision(3) << double(soh[2]) * 0.006166 << "V "
                                    << double(soh[3]) * (-0.006105) << "V "
                                    << double(soh[4]) * 0.006166 << "V "
                                    << double(soh[5]) * (-0.001832) << "V "
                                    << double(soh[6]) * 0.001832 << "V "
                                    << double(soh[7]) * 0.006166 << "V "
                                    << (soh[8]? "closed": "open") << endl;
      }
    
    if(soh_initialized && time_tag >= last_soh_time + SOH_SAMPLE_TIME)
      {
        for(int i = 0; i < SOH_CHANNELS; ++i)
          {
            if(dr24_channels[DATA_CHANNELS + i] == NULL) continue;

            dr24_channels[DATA_CHANNELS + i]->set_timemark(digitime.it, 0,
              digitime.quality);
          
            dr24_channels[DATA_CHANNELS + i]->put_sample(soh[i]);
          }

        last_soh_time = time_tag;
      }
    
    if(dr24_channels[data_index] != NULL &&
      time_tag != last_channel_time[data_index])
      {
        dr24_channels[data_index]->set_timemark(digitime.it, 0,
          digitime.quality);

        for(int i = sizeof(DataMsgHeader); i < len; i += bytes_per_sample)
          {
            int sample_val = 0;

            switch(bytes_per_sample)
              {
              case 1:
                sample_val = *(int8_t *)(buf + i);
                break;

              case 2:
                sample_val = *(le_int16_t *)(buf + i);
                break;
                
              case 3:
                sample_val = *(le_int32_t *)(buf + i);
                break;

              case 4:
                sample_val = *(le_int32_t *)(buf + i);
              }

            if(bits_per_sample < 32)
              {
                sample_val &= (1 << bits_per_sample) - 1;

                if(sample_val >= 1 << (bits_per_sample - 1))
                    sample_val -= 1 << bits_per_sample;
              }
            
            if(bits_per_sample > 32 - dconf.lsb)
                sample_val >>= dconf.lsb + bits_per_sample - 32;
            
            dr24_channels[data_index]->put_sample(sample_val);
          }

        last_channel_time[data_index] = time_tag;
      }

    last_data_time = time_tag;
  }
  
void DR24Protocol::decode_command_response(const char *buf, int len)
  {
    int type = (unsigned char)buf[0];
    int parm1 = ((len > 3)? ((unsigned char)buf[3]): -1);
    logs(LOG_NOTICE) << "received command response, type = " << hex << type <<
      "h, parm1 = " << hex << parm1 << "h, length = " << dec << len << endl;
  }
  
void DR24Protocol::decode_unsolicited_response(const char *buf, int len)
  {
    int type = (unsigned char)buf[0];
    int parm1 = ((len > 3)? ((unsigned char)buf[3]): -1);
    logs(LOG_NOTICE) << "received unsolicited response, type = " << hex << type <<
      "h, parm1 = " << hex << parm1 << "h, length = " << dec << len << endl;
  }

void DR24Protocol::recv_ack()
  {
    logs(LOG_NOTICE) << "received ACK" << endl;
  }
  
void DR24Protocol::recv_nack()
  {
    logs(LOG_NOTICE) << "received NACK" << endl;
  }
  
void DR24Protocol::recv_can()
  {
    logs(LOG_NOTICE) << "received CAN" << endl;
  }
    
void DR24Protocol::send_frame(const char *buf, int len)
  {
    if(sendsize + len > SENDBUFSIZE - 2)
      {
        logs(LOG_WARNING) << "send buffer full" << endl;
        return;
      }

    sendbuf[sendsize++] = 0xe3;
    memcpy(sendbuf + sendsize, buf, len);
    sendsize += len;
    sendbuf[sendsize++] = 0xe5;
  }
  
void DR24Protocol::send_msg(const char *buf, int len)
  {
    char obuf[SENDBUFSIZE - 2];
    
    int crc = 0;
    for(int i = 0; i < len; ++i)
        crc = updcrc(buf[i], crc);

    int i = 0, j = 0;
    while(i < len)
      {
        if(j > SENDBUFSIZE - 6)
          {
            logs(LOG_WARNING) << "message is too large to send" << endl;
            return;
          }
        
        if((unsigned char)buf[i] >= 0xe0 && (unsigned char)buf[i] <= 0xe5)
          {
            obuf[j++] = 0xe0;
            obuf[j++] = buf[i++] ^ 0xa0;
            continue;
          }
          
        obuf[j++] = buf[i++];
      }

    obuf[j++] = (crc >> 8) & 0xff;
    obuf[j++] = crc & 0xff;

    send_frame(obuf, j);
  }
  
void DR24Protocol::send_ack()
  {
    send_frame("\xe1", 1);
  }
  
void DR24Protocol::send_nack()
  {
    send_frame("\xe2", 1);
  }
  
void DR24Protocol::send_can()
  {
    send_frame("\xe4", 1);
  }
    
void DR24Protocol::start_acquisition()
  {
    send_msg("\x84\x00\x00", 3);
  }

void DR24Protocol::accept_frame()
  {
    send_ack();
    nak_count = 0;
  }

void DR24Protocol::reject_frame()
  {
    if(nak_count < 10)
      {
        send_nack();
        ++nak_count;
      }
    else
      {
        logs(LOG_WARNING) << "frame cancelled after 10 retries" << endl;
        send_can();
        nak_count = 0;
      }
  }

void DR24Protocol::decode_frame(const char *buf, int len)
  {
    char obuf[RECVBUFSIZE - 2];

    if(len == 0)
      {
        logs(LOG_WARNING) << "received zero length frame" << endl;
        reject_frame();
        return;
      }
    
    int type = (unsigned char)buf[0];

    DEBUG_MSG("frame received, type = " << hex << type << "h, length = " <<
      dec << len << endl);

    if(len == 1)
      {
        switch(type)
          {
          case 0xe1: recv_ack(); break;
          case 0xe2: recv_nack(); break;
          case 0xe4: recv_can(); break;

          default:
            logs(LOG_WARNING) << "unknown control frame type (" << hex <<
              type << "h)" << dec << endl;
            reject_frame();
            break;
          }

        return;
      }
    
    if(len < 5)
      {
        logs(LOG_WARNING) << "frame is too short (type = " << hex << type <<
          "h, length = " << dec << len << ")" << endl;
        reject_frame();
        return;
      }
    
    if(type > 0x7f)
      {
        logs(LOG_WARNING) << "unknown frame type (" << hex <<
          type << "h)" << dec << endl;
        reject_frame();
        return;
      }
    
    int i = 0, j = 0;
    while(i < len)
      {
        if((unsigned char)buf[i] >= 0xe1 && (unsigned char)buf[i] <= 0xe5)
          {
            logs(LOG_WARNING) << "invalid character in frame (" << hex <<
              int((unsigned char)(buf[i])) << "h)" << dec << endl;
            reject_frame();
            return;
          }

        if((unsigned char)buf[i] == 0xe0)
          {
            ++i;
            obuf[j++] = buf[i++] ^ 0xa0;
            continue;
          }

        obuf[j++] = buf[i++];
      }

    DEBUG_MSG("frame length after decoding escapes: " << j << " bytes" << endl);
    
    j -= 2;
    
    int crc = 0;
    for(i = 0; i < j; ++i)
        crc = updcrc(obuf[i], crc);

    if(crc != (((obuf[j] << 8) & 0xff00) | (obuf[j + 1] & 0xff)))
      {
        logs(LOG_WARNING) << "CRC mismatch" << endl;
        reject_frame();
        return;
      }

    accept_frame();

    if(type <= 0x1f) decode_data(obuf, j);
    else if(type <= 0x5f) decode_command_response(obuf, j);
    else decode_unsolicited_response(obuf, j);
  }

void DR24Protocol::do_start()
  {
    start_acquisition();
    
    while(!terminate_proc)
      {
        fd_set read_set, write_set;
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_SET(fd, &read_set);
        
        if(sendsize != 0) FD_SET(fd, &write_set);
        
        struct timeval tv;
        tv.tv_sec = READ_TIMEOUT;
        tv.tv_usec = 0;

        int r;
        if((r = select(fd + 1, &read_set, &write_set, NULL, &tv)) < 0)
          {
            if(errno == EINTR) continue;
            throw PluginLibraryError("select error");
          }

        if(r == 0)
          {
            logs(LOG_WARNING) << "read timeout" << endl;

            if(sendsize == 0) send_nack(); // just in case

            continue;
          }
        
        if(FD_ISSET(fd, &write_set) && sendsize > 0)
          {
            int nwritten;
            if((nwritten = write(fd, sendbuf, sendsize)) < 0)
                throw PluginLibraryError("write error");

            sendsize -= nwritten;
            memmove(sendbuf, sendbuf + nwritten, sendsize);
          }

        if(FD_ISSET(fd, &read_set))
          {
            int nread;
            if((nread = read(fd, recvbuf + rp, RECVBUFSIZE - rp)) < 0)
                throw PluginLibraryError("read error");

            rp += nread;
            
            int bof = -1, eof = -1;
            while(1)
              {
                // search for the beginning of frame
                bof = eof + 1;
                while(bof < rp && (unsigned char)recvbuf[bof] != 0xe3)
                    ++bof;

                if(bof == rp)
                  {
                    rp = 0;
                    break;
                  }

                // search for the end of frame
                eof = bof + 1;
                while(eof < rp && (unsigned char)recvbuf[eof] != 0xe5)
                    ++eof;

                if(eof == rp)
                  {
                    memmove(recvbuf, recvbuf + bof, rp - bof);
                    rp -= bof;
                    break;
                  }

                decode_frame(recvbuf + bof + 1, eof - bof - 1);
              }

            if(rp == RECVBUFSIZE)
              {
                logs(LOG_WARNING) << "end of frame not found" << endl;
                rp = 0;
              }
          }
      }
  }

RegisterProto<DR24Protocol> proto("dr24");

} // unnamed namespace

