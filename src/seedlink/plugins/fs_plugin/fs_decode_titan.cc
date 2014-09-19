/***************************************************************************** 
 * fs_decode_titan.cc
 *
 * Titan decoder module
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

extern "C" {
#include "read_titan.h"
}

#include "qtime.h"

#include "fs_plugin.h"
#include "plugin_channel.h"
#include "cppstreams.h"
#include "utils.h"

//*****************************************************************************
// Global variables for Titan routines
//*****************************************************************************

struct option opt;
FILE   *Fp_log;
int    byteswap;
Event  evn;
Paths  paths;
char   Station[8];
char   Network[3];
int    datalist;

namespace {

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

class FS_Decode_Titan *instance_ptr;

class FS_Decode_Titan: public FS_Decoder
  {
  private:
    rc_ptr<OutputChannel> output[NCHAN][NCOMP];

    void init_channel(int chan);

  public:
    const string myname;
    
    FS_Decode_Titan(const string &myname_init);
    
    void attach_output_channel(const string &source_id,
      const string &channel_name);
      
    void flush_channels();

    void process_file(const string &file, off_t offset, ssize_t len);
    void output_data(int last, int time_jump);
  };

FS_Decode_Titan::FS_Decode_Titan(const string &myname_init):
  myname(myname_init)
  {
    evn.evn_time       = NULL;
    evn.evn_duration   = NULL;
    evn.cur_event      = 0;
    evn.Fp_sism_evntbl = NULL;
    opt.chan            = -1;
    opt.comp            = -1;
    opt.do_sac          = FALSE;
    opt.sacsun          = FALSE;
    opt.do_ah           = FALSE;
    opt.do_mseed        = FALSE;
    opt.do_seed         = FALSE;
    opt.do_segy         = FALSE;
    opt.do_bindata      = FALSE;
    opt.do_sismalp      = FALSE;
    opt.do_asc          = FALSE;
    opt.titseg          = FALSE;
    opt.use_database    = TRUE;
    opt.do_offset       = 0;
    opt.timespan        = 0.0;
    opt.event_list[0]   = '\0';
    opt.evn_duration    = 180;
    opt.num_traces      = 0;
    opt.daydir          = FALSE;
    opt.beg_offset      = 0;
    opt.end_offset      = -1;
    opt.do_time         = FALSE;
    opt.do_coord        = FALSE;
    opt.output_delta_t  = TRUE;
    opt.tcorr_mode      = CORRECT_DFT;
    opt.dtfile          = SMOOTHED;
    opt.noinfo          = FALSE;
    opt.gain_range      = TRUE;
    opt.discard_short_blk = TRUE;
    opt.info.on         = FALSE;
    opt.srate           = 0.0;
    opt.verb = ((verbosity > 0) ? (verbosity - 1): 0);
    
    Station[0] = 0;
    Network[0] = 0;
    Fp_log = stdout;
    strncpy(opt.station, station_name.c_str(), 7);
    opt.station[7] = 0;
    strncpy(netname, network_name.c_str(), PATHLEN - 1);
    netname[PATHLEN - 1] = 0;
    
    char temp[40];
    find_wordorder(temp);
    if(!strncmp(temp, "3210", 4)) byteswap = TRUE;
    else byteswap = FALSE;

    paths_init();
  }

void FS_Decode_Titan::attach_output_channel(const string &source_id,
  const string &channel_name)
  {
    int chan, comp;
    char c;

    if(sscanf(source_id.c_str(), "%u:%u%c", &chan, &comp, &c) != 2 ||
      chan >= NCHAN || comp >= NCOMP)
      throw PluginADInvalid(source_id, channel_name);
    
    if(output[chan][comp] != NULL)
        throw PluginADInUse(source_id, output[chan][comp]->channel_name);
    
    output[chan][comp] = new OutputChannel(channel_name, station_name,
      zero_sample_limit, -1); // FIXME: scale
  }

void FS_Decode_Titan::flush_channels()
  {
    for(int chan = 0; chan < NCHAN; ++chan)
      {
        for (int comp = 0; comp < NCOMP; ++comp)
          {
            if(output[chan][comp] == NULL) continue;
            output[chan][comp]->flush_streams();
          }
      }
  }

void FS_Decode_Titan::process_file(const string &file, off_t offset,
  ssize_t len)
  {
    char *filename;
    if((filename = (char *) malloc(file.length() + 1)) == NULL)
        throw bad_alloc();

    strcpy(filename, file.c_str());
    instance_ptr = this;
    process_titan(filename);
    instance_ptr = NULL;
    free(filename);
  }
        
void FS_Decode_Titan::init_channel(int chan)
  {
    saveOutputParms(chan);

    double dbltime = OutData[chan].uncorrected_time - 
      OutData[chan].adcdelay - OutData[chan].filtdelay;

    time_t t = (time_t) dbltime;
    struct tm* ptm = gmtime(&t);
    EXT_TIME et;

    et.year = ptm->tm_year + 1900;
    et.month = ptm->tm_mon + 1;
    et.day = ptm->tm_mday;
    et.hour = ptm->tm_hour;
    et.minute = ptm->tm_min;
    et.second = ptm->tm_sec;
    et.usec = long((dbltime - double(t)) * 1000000.0);
    et.doy = mdy_to_doy(et.month, et.day, et.year);

    digitime.it = ext_to_int(et);
    digitime.valid = true;

    for(int comp = 0; comp < Channel[chan].numcomp; comp++)
      {
        if(output[chan][comp] == NULL) continue;
        output[chan][comp]->set_timemark(digitime.it,
          long(-OutData[chan].observ_dt * 1000000.0), default_timing_quality);
      }
  }

void FS_Decode_Titan::output_data(int last, int time_jump)
try
  {
    for(int chan = 0; chan < NCHAN; chan++)
      {
        if(Channel[chan].numcomp == 0) continue;
        
        if(Channel[chan].isnew == 1 && Channel[chan].ninp > 0)
          {
            init_channel(chan);
            Channel[chan].isnew = 0;
          }

        if(time_jump)
          {
            init_channel(chan);
            OutData[chan].nsamples = Channel[chan].nout;
            totOutSamples += OutData[chan].nsamples;
            Channel[chan].nout = 0;
          }
    
        for (int comp = 0; comp < Channel[chan].numcomp; comp++)
          {
            if(output[chan][comp] == NULL) continue;

            for (int i = 0; i < Channel[chan].ninp; i++)
                output[chan][comp]->put_sample(inp_data[chan][comp][i]);
          }

        Channel[chan].nout += Channel[chan].ninp;
        Channel[chan].ninp = 0;
  
        if(last)
          {
            Channel[chan].isnew = 1;
            OutData[chan].nsamples = Channel[chan].nout;
            totOutSamples += OutData[chan].nsamples;
          }
      }
  }
catch(exception &e)
  {
    logs(LOG_ERR) << e.what() << endl;
    exit(1);
  }
catch(...)
  {
    logs(LOG_ERR) << "unknown exception" << endl;
    exit(1);
  }

RegisterDecoder<FS_Decode_Titan> decode("titan");

} // unnamed namespace

/*==================================================================*
    output_data
    input args:
        last:        flag telling that end of data is detected.
        time_jump:   flag telling a discontinuity occured in the data.

    This module is called by processDataFrame() and by readTitanLoop().
    If last is not set, the module is called upon reading of a time
    frame and whe should have got 128 primary mux data triplets and
    optionally 128 /decim secondary mux data triplets in the
    inp_data[][][] array.

    The process is dispatched back to our object
 *==================================================================*/

void output_data(int last, int time_jump)
  {
    instance_ptr->output_data(last, time_jump);
  }

