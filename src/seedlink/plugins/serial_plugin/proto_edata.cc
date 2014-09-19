/***************************************************************************** 
 * proto_edata.cc
 *
 * Earth Data protocol implementation
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
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
#include <cstdio>
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

const int MAX_SAMPLE_RATE   = 3000;
const int PRI_DATA_CHANNELS = 3;
const int SEC_DATA_CHANNELS = 3;
const int DATA_CHANNELS     = PRI_DATA_CHANNELS + SEC_DATA_CHANNELS; // 0...5
const int PRI_SOH_CHANNELS  = 8;
const int SEC_SOH_CHANNELS  = 6;
const int SOH_CHANNELS      = PRI_SOH_CHANNELS + SEC_SOH_CHANNELS;   // 6...19
const int PLL_CHANNEL       = DATA_CHANNELS + SOH_CHANNELS;          // 20
const int NCHAN             = PLL_CHANNEL + 1;
const int TIME_WAIT         = 60;

const int NMEA_LENGTH       = 72;
const int TAIP_LENGTH       = 129;

//*****************************************************************************
// Data Structures
//*****************************************************************************

struct SegmentHeader
  {
    char id[4];
    le_u_int32_t size;
  } PACKED;

struct MOD_seg
  {
    /* MOD Segment */
    
    char device_id[12];
    char version[6];
    char space1;
    char serial_no[4];
    char space2;
    char test_pattern[8];
    le_u_int32_t block_count;
    le_u_int16_t ncomps;
    le_u_int16_t sample_rate;
    le_u_int16_t bytesper;
    char filter_type[2];
    le_u_int16_t deci12;
    le_u_int16_t deci10;
    le_int16_t plldata;
    char gain[2];
    le_int32_t cal_gain[PRI_DATA_CHANNELS];
    le_int32_t offset[PRI_DATA_CHANNELS];
    le_int16_t adc[PRI_SOH_CHANNELS];
    char coupling[2];
    le_int32_t peak[PRI_DATA_CHANNELS];
    le_u_int16_t gps_flags;
    le_u_int32_t gps_block;
    char gps_message[NMEA_LENGTH];
  } PACKED;

struct MDE_seg
  {
    /* MDE Segment */

    char serial_no[8];
    le_u_int16_t deci10_5;
    le_u_int16_t deci10_6;
    le_u_int16_t deci10_7;
    le_int32_t cal_gain[SEC_DATA_CHANNELS];
    le_int32_t offset[SEC_DATA_CHANNELS];
    le_int16_t adc[SEC_SOH_CHANNELS];
    char gps_message[TAIP_LENGTH];
  } PACKED;

struct SUM_seg
  {
    /* SUM Segment */

    le_u_int16_t reserved;
    le_u_int16_t checksum;
  } PACKED;

struct GPSData
  {
    int hour;
    int min;
    int sec;
    int day;
    int month;
    int year;
    double latitude;
    double longitude;
    double velocity;
    double heading;
    double magnetic_variation;
    bool valid;

    GPSData(): hour(0), min(0), sec(0), day(0), month(0), year(0),
      latitude(0), longitude(0), velocity(0), heading(0), magnetic_variation(0),
      valid(false) {}
  };

struct DigitizerStatus
  {
    le_int32_t cal_gain1[PRI_DATA_CHANNELS];
    le_int32_t cal_gain2[SEC_DATA_CHANNELS];
    le_int32_t offset1[PRI_DATA_CHANNELS];
    le_int32_t offset2[SEC_DATA_CHANNELS];
    INT_TIME realtime;
    int time_wait;
    int td;
  };

//*****************************************************************************
// EdataProtocol
//*****************************************************************************

class EdataProtocol: public Proto
  {
  private:
    int fd;
    bool startup_messages;
    bool gps_messages;
    bool soh_messages;
    DigitizerStatus status;
    vector<rc_ptr<OutputChannel> > edata_channels;

    SegmentHeader modhead, mdehead, dathead, sumhead;
    MOD_seg *modseg;
    MDE_seg *mdeseg;
    SUM_seg *sumseg;
    char *datseg;
    unsigned int modsize, mdesize, datsize, sumsize, gpssize;

    void do_start();
    void set_time_from_pc();
    void set_time_from_gps(GPSData &gps);
    void update_status();
    void decode_nmea(GPSData &gps, const char *gpsmsg);
    void decode_taip(GPSData &gps, const char *gpsmsg);
  
  public:
    EdataProtocol(const string &myname):
      startup_messages(true), gps_messages(true), soh_messages(true),
      edata_channels(NCHAN), modseg(NULL), mdeseg(NULL), sumseg(NULL),
      datseg(NULL), modsize(0), datsize(0), sumsize(0)
      {
        memset(&modhead, 0, sizeof(SegmentHeader));
        memset(&mdehead, 0, sizeof(SegmentHeader));
        memset(&dathead, 0, sizeof(SegmentHeader));
        memset(&sumhead, 0, sizeof(SegmentHeader));
      }

    ~EdataProtocol()
      {
        if(modseg != NULL) delete modseg;
        if(mdeseg != NULL) delete mdeseg;
        if(sumseg != NULL) delete sumseg;
        if(datseg != NULL) delete datseg;
      }
    
    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision);
    void flush_channels();
    void start();
  };

void EdataProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);
    
    if(*tail || n >= NCHAN)
      throw PluginADInvalid(source_id, channel_name);

    if(edata_channels[n] != NULL)
        throw PluginADInUse(source_id, edata_channels[n]->channel_name);

    edata_channels[n] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void EdataProtocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(edata_channels[n] == NULL) continue;
        edata_channels[n]->flush_streams();
      }
  }

void EdataProtocol::start()
  {
    fd = open_port(O_RDONLY);
    sync_data(fd);

    if(!dconf.use_pctime_if_no_gps)
        logs(LOG_INFO) << "digitizer will be enabled when GPS in-lock message is received" << endl;

    memset(&status, 0, sizeof(DigitizerStatus));

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

void EdataProtocol::set_time_from_gps(GPSData &gps)
  {
    EXT_TIME et;

    et.year = gps.year;
    et.month = gps.month;
    et.day = gps.day;
    et.hour = gps.hour;
    et.minute = gps.min;
    et.second = gps.sec;
    et.usec = 0;
    et.doy = mdy_to_doy(et.month, et.day, et.year);
    digitime.it = add_time(ext_to_int(et), 0, dconf.time_offset);
  }

void EdataProtocol::set_time_from_pc()
  {
    time_t t = time(NULL);
    tm *ptm = gmtime(&t);
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
  }

void EdataProtocol::do_start()
  {
    while(!terminate_proc)
      {
        if(read_port(fd, &modhead, sizeof(SegmentHeader)) == 0) continue;

        if(strncmp(modhead.id, "MOD", 3))
          {
            logs(LOG_WARNING) << "bad MOD segment" << endl;
            seed_log << "bad MOD segment" << endl;
            sync_data(fd);
            continue;
          }
    
        if(modseg == NULL)
          {
            logs(LOG_INFO) << "MOD segment size is " << modhead.size << " bytes" << endl;
            modsize = max(size_t(modhead.size), sizeof(MOD_seg));
            if((modseg = static_cast<MOD_seg *>(malloc(modsize))) == NULL)
                throw bad_alloc();
            memset(modseg, 0, modsize);
          }
    
        if(read_port(fd, modseg, modsize) == 0)
          {
            logs(LOG_WARNING) << "read timeout" << endl;
            continue;
          }
    
#ifdef DEBUG_RAW_PACKETS
        DEBUG_MSG("packet " << modseg->block_count << " received (GPS block " <<
          modseg->gps_block << ")" << endl);
        DEBUG_MSG("GPS message: " << string(modseg->gps_message, NMEA_LENGTH) << endl);
#endif

        if(read_port(fd, &dathead, sizeof(SegmentHeader)) == 0)
          {
            logs(LOG_WARNING) << "read timeout" << endl;
            continue;
          }

        if(!strncmp(dathead.id, "MDE", 3))
          {
            mdehead = dathead;
            
            if(mdeseg == NULL)
              {
                logs(LOG_INFO) << "MDE segment size is " << mdehead.size << " bytes" << endl;
                mdesize = max(size_t(mdehead.size), sizeof(MDE_seg));
                if((mdeseg = static_cast<MDE_seg *>(malloc(mdesize))) == NULL)
                    throw bad_alloc();
                memset(mdeseg, 0, mdesize);
              }
        
            if(read_port(fd, mdeseg, mdesize) == 0)
              {
                logs(LOG_WARNING) << "read timeout" << endl;
                continue;
              }
    
            if(read_port(fd, &dathead, sizeof(SegmentHeader)) == 0)
              {
                logs(LOG_WARNING) << "read timeout" << endl;
                continue;
              }

            DEBUG_MSG("extended GPS message: " << string(mdeseg->gps_message, TAIP_LENGTH) << endl);
          }
    
        if(strncmp(dathead.id, "DAT", 3))
          {
            logs(LOG_WARNING) << "bad DAT segment" << endl;
            seed_log << "bad DAT segment" << endl;
            sync_data(fd);
            continue;
          }
    
        if(datseg == NULL)
          {
            logs(LOG_INFO) << "DAT segment size is " << dathead.size << " bytes" << endl;
            logs(LOG_INFO) << "sample_rate = " << dathead.size / (modseg->bytesper * modseg->ncomps) <<
              ", bytesper = " << modseg->bytesper << ", ncomps = " << modseg->ncomps << endl;
            datsize = dathead.size;
            if((datseg = static_cast<char *>(malloc(datsize))) == NULL)
                throw bad_alloc();
          }

        if(read_port(fd, datseg, datsize) == 0)
          {
            logs(LOG_WARNING) << "read timeout" << endl;
            continue;
          }
    
        if(dconf.checksum_used)
          {
            if(read_port(fd, &sumhead, sizeof(SegmentHeader)) == 0)
              {
                logs(LOG_WARNING) << "read timeout" << endl;
                continue;
              }
        
            if(strncmp(sumhead.id, "SUM", 3))
              {
                logs(LOG_WARNING) << "bad SUM segment" << endl;
                seed_log << "bad SUM segment" << endl;
                sync_data(fd);
                continue;
              }
          
            if(sumseg == NULL)
              {
                logs(LOG_INFO) << "SUM segment size is " << sumhead.size << " bytes" << endl;
                sumsize = max(size_t(sumhead.size), sizeof(SUM_seg));
                if((sumseg = static_cast<SUM_seg *>(malloc(sumsize))) == NULL)
                    throw bad_alloc();
                memset(sumseg, 0, sumsize);
              }
        
            if(read_port(fd, sumseg, sumsize) == 0)
              {
                logs(LOG_WARNING) << "read timeout" << endl;
                continue;
              }
        
            u_int16_t csum = 0;
    
            for(unsigned int i = 0; i < sizeof(SegmentHeader); ++i)
              {
                csum += ((u_int8_t *)&modhead)[i];
                csum += ((u_int8_t *)&mdehead)[i];
                csum += ((u_int8_t *)&dathead)[i];
                csum += ((u_int8_t *)&sumhead)[i];
              }
    
            for(unsigned int i = 0; i < modsize; ++i)
                csum += ((u_int8_t *)modseg)[i];

            if(mdeseg != NULL)
                for(unsigned int i = 0; i < mdesize; ++i)
                    csum += ((u_int8_t *)mdeseg)[i];
            
            for(unsigned int i = 0; i < datsize; ++i)
                csum += ((u_int8_t *)datseg)[i];

            for(unsigned int i = 0; i < sumsize - 2; ++i)
                csum += ((u_int8_t *)sumseg)[i];

            if(csum != sumseg->checksum)
              {
                logs(LOG_WARNING) << "packet " << modseg->block_count << " has bad checksum" << endl;
                seed_log << "packet " << modseg->block_count << " has bad checksum" << endl;
              }

#ifdef DEBUG_RAW_PACKETS
            DEBUG_MSG("checksum: " << sumseg->checksum << " (calculated " << csum << ")" << endl);
#endif
          }
    
        update_status();
    
        if(digitime.valid)
          {
            for(int i = 0; i < NCHAN; ++i)
              {
                if(edata_channels[i] == NULL) continue;
                edata_channels[i]->set_timemark(digitime.it, 0,
                  digitime.quality);
              }

            char* p = datseg;
            for(unsigned int j = 0; j < datsize / (modseg->bytesper * modseg->ncomps); ++j)
              {
                for(int i = 0; i < modseg->ncomps; ++i)
                  {
                    if(edata_channels[i] == NULL) continue;
            
                    int32_t sample_val = 0;
                
                    switch(modseg->bytesper)
                      {
                      case 1:
                        sample_val = *(int8_t *)p;
                        break;
                      case 2:
                        sample_val = *(le_int16_t *)p >> ((dconf.lsb > 16) ? (dconf.lsb - 16): 0);
                        break;
                      case 3:
                        sample_val = (*(le_int32_t *)p << 8) >> (8 + ((dconf.lsb > 8) ? (dconf.lsb - 8): 0));
                        break;
                      case 4:
                        sample_val = *(le_int32_t *)p >> dconf.lsb;
                        break;
                      }

                    p += modseg->bytesper;
                    edata_channels[i]->put_sample(sample_val);
                  }
              }

            for(int i = 0; i < SOH_CHANNELS; ++i)
              {
                if(edata_channels[i + DATA_CHANNELS] == NULL) continue;
                
                if(i < PRI_SOH_CHANNELS)
                    edata_channels[i + DATA_CHANNELS]->put_sample(modseg->adc[i]);
                else if(mdeseg != NULL)
                    edata_channels[i + DATA_CHANNELS]->put_sample(mdeseg->adc[i - PRI_SOH_CHANNELS]);
                else
                    edata_channels[i + DATA_CHANNELS]->put_sample(0);
              }

            if(edata_channels[PLL_CHANNEL] != NULL)
                edata_channels[PLL_CHANNEL]->put_sample(modseg->plldata);
          }
      }
  }

void EdataProtocol::update_status()
  {
    GPSData gps;

    if(modseg->gps_message[0] == '$')
        decode_nmea(gps, modseg->gps_message);
    else if(mdeseg != NULL)
        decode_taip(gps, mdeseg->gps_message);
    else
        gps.valid = false;
    
    if(gps.valid && modseg->block_count == modseg->gps_block)
      {
        set_time_from_gps(gps);
        
        if(!digitime.exact)
          {
            logs(LOG_INFO) << "GPS in lock" << endl;
            digitime.exact = true;
            status.time_wait = TIME_WAIT;
          }

        int td;
        if(digitime.valid)
            td = int(tdiff(digitime.it, status.realtime) / 1000000 + 0.5) - 1;
        else
            td = 0;
        
        if(td != status.td)
          {
            logs(LOG_INFO) << "GPS time shift " << td - status.td << " sec" << endl;

            if(status.time_wait == 0)
                logs(LOG_INFO) << "using internal time" << endl;

            status.td = td;
            status.time_wait = TIME_WAIT;
          }
        else if(status.time_wait > 0 && (--status.time_wait == 0))
          {
            if(!digitime.valid)
              {
                logs(LOG_INFO) << "digitizer enabled" << endl;
                digitime.valid = true;
              }
            
            logs(LOG_INFO) << "using GPS time" << endl;
            status.td = 0;
          }

        digitime.it = add_time(digitime.it, -status.td, 0);
        digitime.quality = 100;
        status.realtime = digitime.it;
      }
    else if(digitime.valid)
      {
        digitime.it = add_time(digitime.it, 1, 0);
        digitime.quality = dconf.unlock_tq;
        
        if(digitime.exact)
          {
            logs(LOG_INFO) << "GPS out of lock" << endl;
            digitime.exact = false;
          }
        
        status.realtime = digitime.it;
      }
    else if(dconf.use_pctime_if_no_gps)
      {
        set_time_from_pc();
        digitime.valid = true;
      }
    else
        return;
                
    EXT_TIME et = int_to_ext(digitime.it);
    
    if(et.hour == 0 && et.minute == 0 && et.second == 1)
      {
        startup_messages = true;
        gps_messages = true;
        soh_messages = true;
      }

    if(dconf.statusinterval && !(digitime.it.second % (dconf.statusinterval * 60)))
        soh_messages = true;
    
    if(startup_messages)
      {
        seed_log << ident_str << endl
                 << "Device ID: " << string(modseg->device_id, 11) << " "
                    "Version: " << string(modseg->version, 6) << " "
                    "Serial No.: ";

        if(mdeseg)
            seed_log << string(mdeseg->serial_no, 8);
        else
            seed_log << string(modseg->serial_no, 4);

        seed_log << endl
                 << "Components: " << modseg->ncomps << " "
                    "Sample rate: " << modseg->sample_rate / modseg->ncomps << " "
                    "Bytes per sample: " << modseg->bytesper << endl
                 << "Filter type: " << string(modseg->filter_type, 2) << " "
                    "Deci12: " << modseg->deci12 << " "
                    "Deci10: " << modseg->deci10 << " ";

        if(mdeseg)
            seed_log << "Deci10_5: " << mdeseg->deci10_5 << " "
                     << "Deci10_6: " << mdeseg->deci10_6 << " "
                     << "Deci10_7: " << mdeseg->deci10_7 << " ";
        
        seed_log << "Gain: " << string(modseg->gain, 2) << endl
                 << "Program setup: "
                    "protocol=" << dconf.proto_name << " "
                    "checksum=" << dconf.checksum_used << " "
                    "pctime=" << dconf.use_pctime_if_no_gps << " "
                    "lsb=" << dconf.lsb << " "
                    "statusinterval=" << dconf.statusinterval << endl;
      }
    
    if(startup_messages ||
      memcmp(modseg->cal_gain, status.cal_gain1, PRI_DATA_CHANNELS * 4) ||
      (mdeseg != NULL && memcmp(mdeseg->cal_gain, status.cal_gain2, SEC_DATA_CHANNELS * 4)))
      {
        memcpy(status.cal_gain1, modseg->cal_gain, PRI_DATA_CHANNELS * 4);
        seed_log << "cal. gain: " << modseg->cal_gain[0] << " "
                                  << modseg->cal_gain[1] << " "
                                  << modseg->cal_gain[2];

        if(mdeseg != NULL)
          {
            memcpy(status.cal_gain2, mdeseg->cal_gain, SEC_DATA_CHANNELS * 4);
            seed_log << " "       << mdeseg->cal_gain[0] << " "
                                  << mdeseg->cal_gain[1] << " "
                                  << mdeseg->cal_gain[2];
          }

        seed_log << endl;
      }
        
    if(startup_messages ||
      memcmp(modseg->offset, status.offset1, PRI_DATA_CHANNELS * 4) ||
      (mdeseg != NULL && memcmp(mdeseg->offset, status.offset2, SEC_DATA_CHANNELS * 4)))
      {
        memcpy(status.offset1, modseg->offset, PRI_DATA_CHANNELS * 4);
        seed_log << "offset: "    << modseg->offset[0] << " "
                                  << modseg->offset[1] << " "
                                  << modseg->offset[2];

        if(mdeseg != NULL)
          {
            memcpy(status.offset2, mdeseg->offset, SEC_DATA_CHANNELS * 4);
            seed_log << " "       << mdeseg->offset[0] << " "
                                  << mdeseg->offset[1] << " "
                                  << mdeseg->offset[2];
          }

        seed_log << endl;
      }
    
    if(startup_messages)
        seed_log << "16 bit phase error in 1 second pll: " << modseg->plldata << endl;
    
    if(gps_messages && digitime.exact)
      {
        gps_messages = false;
        seed_log << fixed << setfill('0')
                 << "GPS: date: "     << setw(2) << gps.day
                                      << setw(2) << gps.month
                                      << setw(2) << gps.year << " "
                 <<      "time: "     << setw(2) << gps.hour
                                      << setw(2) << gps.min
                                      << setw(2) << gps.sec << endl
                 << "GPS: latitude "  << setprecision(4) << gps.latitude << " "
                 <<      "longitude "                    << gps.longitude << " "
                 <<      "heading "   << setprecision(1) << gps.heading << endl
                 << "GPS: velocity "                     << gps.velocity << " "
                 <<      "magnetic variation "           << gps.magnetic_variation << endl;
      }

    if(soh_messages && !(et.hour == 0 && et.minute == 0 && et.second == 0))
      {
        soh_messages = false;
        seed_log << fixed
                 << "State of health: "
                 << setprecision(3) << double(modseg->adc[0]) * 10.9 / 2700.0 + 5.0 << "V "
                 << setprecision(1) << double(modseg->adc[1]) * 22.0 / 170.0 << "mA "
                                    << double(modseg->adc[2]) / 10.0 - 50.0 << "C "
                 << setprecision(3) << double(modseg->adc[3]) * 10.9 / 2700.0 + 5.0 << "V "
                                    << double(modseg->adc[4]) / 1000.0 << "V "
                                    << double(modseg->adc[5]) / 1000.0 << "V "
                                    << double(modseg->adc[6]) / 1000.0 << "V "
                                    << double(modseg->adc[7]) / 1000.0 << "V";
        if(mdeseg)
            seed_log << " "
                     << setprecision(1) << double(mdeseg->adc[0]) / 10.0 - 50.0 << "C "
                                        << double(mdeseg->adc[1]) / 10.0 - 50.0 << "C "
                                        << double(mdeseg->adc[2]) / 10.0 - 50.0 << "C "
                                        << double(mdeseg->adc[3]) / 10.0 - 50.0 << "C "
                                        << double(mdeseg->adc[4]) / 10.0 - 50.0 << "C "
                                        << double(mdeseg->adc[5]) / 10.0 - 50.0 << "C";
         
        seed_log << endl;
      }
    
    startup_messages = false;
  }

void EdataProtocol::decode_nmea(GPSData &gps, const char *gpsmsg)
  {
    int r, hour, min, sec, day, month, year, latd, lond;
    double lats, lons, vel, hd, mv;
    char slat, slon, smv;

    r = sscanf(gpsmsg, "$GPRMC,%2u%2u%2u,%*c,%2u%lf,%c,%3u%lf,%c,%lf,%lf,%2u%2u%2u,%lf,%c*",
        &hour, &min, &sec, &latd, &lats, &slat, &lond, &lons, &slon, &vel, &hd,
        &day, &month, &year, &mv, &smv);

    if(r == EOF || r == 0)
      {
        gps.valid = false;
        return;
      }

    if(r < 16)
      {
        logs(LOG_WARNING) << "error decoding GPS message" << endl;
        gps.valid = false;
        return;
      }

    gps.hour = hour;
    gps.min = min;
    gps.sec = sec;
    gps.day = day;
    gps.month = month;
    gps.year = year + 2000;
    gps.latitude = ((slat == 'N') ? 1: -1) * ((double)latd + lats / 60.0);
    gps.longitude = ((slon == 'E') ? 1: -1) * ((double)lond + lons / 60.0);
    gps.velocity = vel;
    gps.heading = hd;
    gps.magnetic_variation = ((smv == 'E') ? 1: -1) * mv;
    gps.valid = true;
  }

void EdataProtocol::decode_taip(GPSData &gps, const char *gpsmsg)
  {
    char buf[TAIP_LENGTH + 1], *pv_ptr, *tm_ptr;
    int r, lat, lon, vel, hd, hour, min, msec, day, month, year, valid;
    div_t d_sec;
    
    strncpy(buf, gpsmsg, TAIP_LENGTH);
    buf[TAIP_LENGTH] = 0;
    
    if((pv_ptr = strstr(buf, ">RPV")) == NULL)
      {
        gps.valid = false;
        return;
      }

    if((tm_ptr = strstr(buf, ">RTM")) == NULL)
      {
        gps.valid = false;
        return;
      }

    r = sscanf(pv_ptr, ">RPV%*5u%8d%9d%3u%3u%*1u%1u",
      &lat, &lon, &vel, &hd, &valid);

    if(r == EOF || r == 0 || valid == 0)
      {
        gps.valid = false;
        return;
      }

    if(r < 5)
      {
        logs(LOG_WARNING) << "error decoding GPS message" << endl;
        gps.valid = false;
        return;
      }

    r = sscanf(tm_ptr, ">RTM%2u%2u%5u%2u%2u%4u%*2u%*1u%*2u%1u",
      &hour, &min, &msec, &day, &month, &year, &valid);

    if(r == EOF || r == 0 || valid == 0)
      {
        gps.valid = false;
        return;
      }

    if(r < 7)
      {
        logs(LOG_WARNING) << "error decoding GPS message" << endl;
        gps.valid = false;
        return;
      }

    d_sec = div(msec + 500, 1000);

    gps.hour = hour;
    gps.min = min;
    gps.sec = d_sec.quot;
    gps.day = day;
    gps.month = month;
    gps.year = year;
    gps.latitude = double(lat) / 100000;
    gps.longitude = double(lon) / 100000;
    gps.velocity = vel;
    gps.heading = hd;
    gps.magnetic_variation = 0;
    gps.valid = true;

    DEBUG_MSG("hour = " << gps.hour << ", min = " << gps.min << ", sec = " 
      << gps.sec << endl
      << "day = " << gps.day << ", month = " << gps.month << ", year = "
      << gps.year << endl
      << "latitude = " << gps.latitude << ", longitude = " << gps.longitude
      << endl);
  }

RegisterProto<EdataProtocol> proto("edata");

} // unnamed namespace

