/***************************************************************************** 
 * fs_decode_seisan.cc
 *
 * SEISAN decoder module
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * modified:
 *   2003.9.3: updated for a new SeisAn library (7.2?), Chad Trabant
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <string>
#include <map>

#include "qtime.h"

#include "fs_plugin.h"
#include "plugin_channel.h"
#include "cppstreams.h"
#include "utils.h"

typedef long               __g77_integer;
typedef unsigned long      __g77_uinteger;
typedef long long          __g77_longint;
typedef unsigned long long __g77_ulongint;

#include <g2c.h>

//*****************************************************************************
// Redefinitions of FORTRAN symbols
//*****************************************************************************

#define wav1 waveform1_
#define wav2 waveform2_
#define wav4 waveform4_
#define wav5 waveform5_
#define signalx signalx_

//*****************************************************************************
// Array dimentions of SEISAN data structures
//*****************************************************************************

namespace {
const int NFILES  = 200;
const int NCHANS  = 1000;
const int NPOINTS = 600000;
const int LINELEN = 80;
const int STATLEN = 5;
const int COMPLEN = 4;
}

//*****************************************************************************
// Global data for SEISAN routines
//*****************************************************************************

struct
  {
    integer nfiles;
    char filename[NFILES][LINELEN];
    char file_format[NFILES][10];
    integer year[NCHANS];
    integer month[NCHANS];
    integer day[NCHANS];
    integer hour[NCHANS];
    integer min[NCHANS];
    real sec[NCHANS];
    char stat[NCHANS][STATLEN];
    char out_stat[NCHANS][STATLEN];
    char sav_stat[NCHANS][STATLEN];
    char comp[NCHANS][COMPLEN];
    char out_comp[NCHANS][COMPLEN];
    integer nsamp[NCHANS];
    real rate[NCHANS];
    integer nsamp_req[NCHANS];
    char sav_comp[NCHANS][COMPLEN];
    integer file_nr_chan[NCHANS];
    integer nchan;
    real duration[NCHANS];
    integer chan_nr_file[NCHANS];
    char cbyte[NCHANS];
    integer first;
    integer last;
    real total_time;
    real out_total_time;
    real delay[NCHANS];
    real rot_delay[NCHANS];
    real sav_total_time;
    char error_message[LINELEN];
    char header_text[NFILES][LINELEN];
    integer current_chan[3];
    integer out_nchan;
    integer out_chan[NCHANS];
    real out_start[NCHANS];
    real out_duration[NCHANS];
    integer out_year[NCHANS];
    integer out_month[NCHANS];
    integer out_day[NCHANS];
    integer out_hour[NCHANS];
    integer out_min[NCHANS];
    real out_sec[NCHANS];
    integer out_status[NCHANS];
    integer out_first_sample[NCHANS];
    integer out_nsamp[NCHANS];
    real out_rate[NCHANS];
    char out_header_text[NFILES][LINELEN];
    integer sav_nchan;
    integer sav_chan[NCHANS];
    real sav_start[NCHANS];
    real sav_duration[NCHANS];
    integer sav_year[NCHANS];
    integer sav_month[NCHANS];
    integer sav_day[NCHANS];
    integer sav_hour[NCHANS];
    integer sav_min[NCHANS];
    real sav_sec[NCHANS];
    integer sav_status[NCHANS];
    integer sav_first_sample[NCHANS];
    integer sav_nsamp[NCHANS];
    real sav_rate[NCHANS];
    char sav_header_text[NFILES][LINELEN];
    real y1[NPOINTS];
    real y2[NPOINTS];
    real y3[NPOINTS];
    real y3comp[NPOINTS][3];
    char time_error[NCHANS][LINELEN];
    char resp_comp[COMPLEN];
    char resp_file[LINELEN];
    char resp_filename[LINELEN];
    char resp_status[LINELEN];
    char resp_action[LINELEN];
    char resp_type[LINELEN];
    integer resp_year;
    integer resp_month;
    integer resp_day;
    char resp_seisan_chead[1040];
    integer out_first;
    integer sav_first;
    integer out_last;
    real out_delay[NCHANS];
    integer sav_last;
    real sav_delay[NCHANS];
    logical interactive;
  } waveform2_;

struct
  {
    doublereal abs_time[NCHANS];
  } waveform4_;

struct
  {
    char resp_stat[STATLEN];
  } waveform5_;

struct
  {
    char rot_comp[NCHANS];
    char out_cbyte[NCHANS];
    char sav_cbyte[NCHANS];
  } waveform1_;

struct
  {
    real signal1[NPOINTS];
  } signalx_;

//*****************************************************************************
// Prototypes of SEISAN routines
//*****************************************************************************

extern "C" {
void wav_read_channel__(integer *ichan);
void read_wav_header__(integer *ifile);
}

namespace {

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

//*****************************************************************************
// SEISAN wrappers 
//*****************************************************************************

void seisan_read_header()
  {
    integer k = 1;
    read_wav_header__(&k);
  }

void seisan_read_channel(int n)
  {
    integer k = n + 1;
    wav_read_channel__(&k);
  }
    
//*****************************************************************************
// OutputComponent
//*****************************************************************************

class OutputComponent
  {
  private:
    map<string, rc_ptr<OutputChannel> > station_map;
    map<string, rc_ptr<OutputChannel> >::iterator current_station;

  public:
    const string channel_name;
    
    OutputComponent(const string &channel_name_init):
      channel_name(channel_name_init) { }
  
    void select_station(const string &station_name);
    void flush_streams();

    void set_timemark(const INT_TIME &it_mark, int usec_correction)
      {
        current_station->second->set_timemark(it_mark, usec_correction,
          default_timing_quality);
      }

    void put_sample(int32_t sample_val)
      {
        current_station->second->put_sample(sample_val);
      }
  };

void OutputComponent::select_station(const string &station_name)
  {
    if((current_station = station_map.find(station_name)) == station_map.end())
      {
        current_station = station_map.insert(make_pair(station_name,
          new OutputChannel(channel_name, station_name, zero_sample_limit, -1))).first; // FIXME: scale
      }
  }
  
void OutputComponent::flush_streams()
  {
    map<string, rc_ptr<OutputChannel> >::iterator p;
    for(p = station_map.begin(); p != station_map.end(); ++p)
        p->second->flush_streams();
  }

//*****************************************************************************
// FS_Decode_Seisan
//*****************************************************************************

class FS_Decode_Seisan: public FS_Decoder
  {
  private:
    map<string, rc_ptr<OutputComponent> > component_map;

  public:
    const string myname;
    
    FS_Decode_Seisan(const string &myname_init): myname(myname_init) { }
    
    void attach_output_channel(const string &source_id,
      const string &channel_name);
      
    void flush_channels();

    void process_file(const string &file, off_t offset, ssize_t len);
  };

void FS_Decode_Seisan::attach_output_channel(const string &source_id,
  const string &channel_name)
  {
    map<string, rc_ptr<OutputComponent> >::iterator p;
    if((p = component_map.find(source_id)) != component_map.end())
        throw PluginADInUse(source_id, p->second->channel_name);
    
    component_map[source_id] = new OutputComponent(channel_name);
  }

void FS_Decode_Seisan::flush_channels()
  {
    map<string, rc_ptr<OutputComponent> >::iterator p;

    for(p = component_map.begin(); p != component_map.end(); ++p)
        p->second->flush_streams();
  }

void FS_Decode_Seisan::process_file(const string &file, off_t offset,
  ssize_t len)
  {
    int n;

    strncpy(wav2.filename[0], file.c_str(), LINELEN);
    if((n = file.length()) < LINELEN) memset(wav2.filename[0] + n, ' ', LINELEN - n);

    wav2.nfiles = 1;
    wav2.nchan = 0;

    seisan_read_header();
    
    for(n = LINELEN; n > 0; --n)
        if(wav2.error_message[n - 1] != ' ')
          {
            logs(LOG_WARNING) <<
              "SEISAN error: " << string(wav2.error_message, n) << endl;
            break;
          }

    if(wav2.nchan == 0)
      {
        logs(LOG_WARNING) << "no data available" << endl;
        return;
      }

    for(int ch = 0; ch < wav2.nchan; ++ch)
      {
        for(n = STATLEN; n > 0; --n)
            if((wav2.stat[ch][n - 1] >= 'A' && wav2.stat[ch][n - 1] <= 'Z') ||
              (wav2.stat[ch][n - 1] >= '0' && wav2.stat[ch][n - 1] <= '9'))
                break;

        if(n == 0) continue;

        string current_station(wav2.stat[ch], n);
        if(!accepted_stations.empty() &&
          accepted_stations.find(current_station) == accepted_stations.end())
            continue;
        
        map<string, rc_ptr<OutputComponent> >::iterator p = component_map.end();
        for(n = COMPLEN; n > 0; --n)
            if(wav2.comp[ch][n - 1] != ' ')
              {
                p = component_map.find(string(wav2.comp[ch], n));
                break;
              }

        if(p == component_map.end()) continue;
        
        p->second->select_station(current_station);
        
        seisan_read_channel(ch);

        EXT_TIME et;
        et.year = wav2.year[ch];
        et.month = wav2.month[ch];
        et.day = wav2.day[ch];
        et.hour = wav2.hour[ch];
        et.minute = wav2.min[ch];
        et.second = long(wav2.sec[ch]);
        et.usec = long((wav2.sec[ch] - double(et.second)) * 1000000.0);
        et.doy = mdy_to_doy(et.month, et.day, et.year);
        
        digitime.it = ext_to_int(et);
        digitime.valid = true;

        p->second->set_timemark(digitime.it, long(wav2.delay[ch] * 1000000.0));

        for(int i = 0; i < wav2.nsamp[ch]; ++i)
            p->second->put_sample(long(signalx.signal1[i]));
      }
  }

RegisterDecoder<FS_Decode_Seisan> decode("seisan");

} // unnamed namespace

