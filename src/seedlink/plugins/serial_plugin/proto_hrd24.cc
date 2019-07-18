/***************************************************************************** 
 * proto_hrd24.cc
 *
 * Nanometrics HRD-24 protocol implementation
 *
 * (c) 2004 Andres Heinloo, GFZ Potsdam
 * (c) 2004 Recai YALGIN, Kandilli Observatory
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdio>
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

const int NCHAN             = 3;
const int BNDLEN            = 17;
const int HEADLEN           = 21;
const int NONVALUE          = 0x7fffffff;
const int MAX_BUNDLES       = 59;
const int MAX_PACKLEN       = HEADLEN + MAX_BUNDLES * BNDLEN + 4;

//*****************************************************************************
// CRC-CCITT Table
//*****************************************************************************

unsigned short crctab[256] =
  {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
  };

//*****************************************************************************
// Data Structures
//*****************************************************************************

struct PacketHeader
  {
    le_u_int32_t oldest_packet;

    // Packet Header Bundle
    
    u_int8_t type;
    le_u_int32_t lseconds;
    le_u_int16_t sseconds;
    le_u_int16_t serial_no;
    le_u_int32_t sequence;
    le_int32_t rate_chn_x0;
  } PACKED;

struct DataBundle
  {
    u_int8_t desc;
    union { int8_t d8[4]; le_int16_t d16[2]; le_int32_t d32; } w[4];
  } PACKED;

struct StatusBundle
  {
    u_int8_t type;
    le_u_int32_t lseconds;
    u_int8_t data[12];
  } PACKED;

struct GPSSatelliteStatusBundle
  {
    u_int8_t type;
    le_u_int32_t lseconds;
    le_u_int16_t status;
    le_u_int16_t sat_chan[5];
  } PACKED;

struct VCXOCalibrationBundle
  {
    u_int8_t type;
    le_u_int32_t lseconds; 
    le_u_int16_t vcxo;
    le_u_int16_t time_diff_at_lock;
    le_u_int16_t time_error;
    le_u_int16_t frequency_error;
    le_u_int16_t crystal_temp;
    u_int8_t pll_status;
    u_int8_t gps_status;
  } PACKED;

struct SlowInternalSOHBundle
  {
    u_int8_t type;
    le_u_int32_t lseconds;
    le_float32_t batt_voltage;
    le_float32_t vcxo_temp;
    le_float32_t radio_snr;
  } PACKED;

struct GPSLocationBundle
  {
    u_int8_t type;
    le_u_int32_t lseconds;
    le_float32_t latitude;
    le_float32_t longitude;
    le_float32_t elevation;
  } PACKED;

//*****************************************************************************
// HRD24Protocol
//*****************************************************************************

class HRD24Protocol: public Proto
  {
  private:
    int fd;
    int xn[NCHAN];
    vector<rc_ptr<OutputChannel> > hrd24_channels;

    void set_time(time_t lsec, int ssec);
    void process_gps_satellite_status_bundle(const GPSSatelliteStatusBundle *b,
      int instrument_id);
    void process_vcxo_calibration_bundle(const VCXOCalibrationBundle *b,
      int instrument_id);
    void process_slow_internal_soh_bundle(const SlowInternalSOHBundle *b,
      int instrument_id);
    void process_gps_location_bundle(const GPSLocationBundle *b,
      int instrument_id);
    void process_status_packet(const StatusBundle *b, int instrument_id);
    void process_data_packet(const DataBundle *b, int chn, int smple);
    void do_start();

    unsigned int updcrc(unsigned char c, unsigned int crc)
      {
        return (crctab[(crc & 0xff) ^ c] ^ (crc >> 8)) & 0xffff;
      }

  public:
    HRD24Protocol(const string &myname): hrd24_channels(NCHAN)
      {
        for(int i = 0; i < NCHAN; ++i)
            xn[i] = NONVALUE;
      }
      
    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision);
    void flush_channels();
    void start();
  };

void HRD24Protocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);
    
    if(*tail || n >= NCHAN)
        throw PluginADInvalid(source_id, channel_name);

    if(hrd24_channels[n] != NULL)
        throw PluginADInUse(source_id, hrd24_channels[n]->channel_name);

    hrd24_channels[n] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void HRD24Protocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(hrd24_channels[n] == NULL) continue;
        hrd24_channels[n]->flush_streams();
      }
  }

void HRD24Protocol::start()
  {
    internal_check(sizeof(PacketHeader) == HEADLEN);
    internal_check(sizeof(DataBundle) == BNDLEN);
    internal_check(sizeof(StatusBundle) == BNDLEN);
    internal_check(sizeof(GPSSatelliteStatusBundle) == BNDLEN);
    internal_check(sizeof(VCXOCalibrationBundle) == BNDLEN);
    internal_check(sizeof(SlowInternalSOHBundle) == BNDLEN);
    internal_check(sizeof(GPSLocationBundle) == BNDLEN);
    internal_check(dconf.nbundles <= MAX_BUNDLES);
    
    fd = open_port(O_RDONLY);

    try
      {
        do_start();
      }
    catch(PluginError &e)
      {
        close(fd);
        throw;
      }

    close(fd);
  }

void HRD24Protocol::set_time(time_t lsec, int ssec)
  {
    struct tm *ptm;
    EXT_TIME et;

    ptm = gmtime(&lsec);
    et.year = ptm->tm_year + 1900;
    et.month = ptm->tm_mon + 1;
    et.day = ptm->tm_mday;
    et.hour = ptm->tm_hour;
    et.minute = ptm->tm_min;
    et.second = ptm->tm_sec;
    et.usec = ssec * 100;
    et.doy = mdy_to_doy(et.month, et.day, et.year);
    digitime.it = ext_to_int(et);
    digitime.valid = true;
  }

void HRD24Protocol::process_gps_satellite_status_bundle(const GPSSatelliteStatusBundle *b,
  int instrument_id)
  {
    internal_check(b->type == 15);

    int ntracked = 0;
    
    logs(LOG_DEBUG) << "GPS SNR ";
    for(int i = 0; i < 5; ++i)
      {
        logs(LOG_DEBUG) << ((b->sat_chan[i] >> 8) & 0x3f);

        if(((b->sat_chan[i] >> 14) & 0x3) == 3)
          {
            ++ntracked;
            logs(LOG_DEBUG) << "* ";
          }
        else
          {
            logs(LOG_DEBUG) << " ";
          }
      }

    logs(LOG_DEBUG) << endl;

    if(dconf.soh_log_dir.length() == 0)
        return;
    
    tm tnow = *gmtime((time_t *)&b->lseconds);
    
    char datetime[32];
    strftime(datetime, 32, "%Y-%m-%d %H:%M:%S", &tnow);

    char tFile1[32];
    sprintf(tFile1, "%s.HRD24-%d.", station_name.c_str(), instrument_id);

    char tFile2[32];
    strftime(tFile2, 32, "%b.%d.%Y.gst", &tnow);

    string filename = dconf.soh_log_dir + "/" + tFile1 + tFile2;
    FILE *fp = fopen(filename.c_str(), "a");

    if(fp == NULL)
        return;

    if(ftell(fp) == 0)
      {
        fprintf(fp, "Gps Channel SOH for Instrument %d\n", instrument_id);
        fprintf(fp, " Time(secs),          Time(date-time),SolnState,FigMerit,NSatForSoln,NSatTracked,Act1,Act2,Act3,Act4,Act5,SNR1,SNR2,SNR3,SNR4,SNR5,PRN1,PRN2,PRN3,PRN4,PRN5\n");
      }
    
    // No information about how to find out "SolnState,FigMerit,NSatForSoln" values
    
    int SolnState = 0;
    int FigureOfMerit = 0;
    int NSatForSoln = 0;
    
    fprintf(fp, "%11u,%25s,%9d,%8d,%11d,%11d,",
      b->lseconds, datetime, SolnState, FigureOfMerit, NSatForSoln, ntracked);

    for(int i = 0; i < 5; ++i)
        fprintf(fp, "%4d,", (b->sat_chan[i] >> 14) & 0x3);

    for(int i = 0; i < 5; ++i)
        fprintf(fp, "%4d,", (b->sat_chan[i] >> 8) & 0x3f);

    for(int i = 0; i < 4; ++i)
        fprintf(fp, "%4d,", b->sat_chan[i] & 0x1f);
    
    fprintf(fp, "%4d\n", b->sat_chan[4] & 0x1f);
    
    fclose(fp);
  }

void HRD24Protocol::process_vcxo_calibration_bundle(const VCXOCalibrationBundle *b,
  int instrument_id)
  {
    internal_check(b->type == 7);

    if(b->pll_status == 1 && b->gps_status < 3)
      {
        digitime.quality = 100;
        digitime.exact = true;
      }
    else if(b->pll_status == 2 && b->gps_status < 1)
      {
        digitime.quality = 90;
        digitime.exact = true;
      }
    else if(b->pll_status == 2 && b->gps_status < 3)
      {
        digitime.quality = 60;
        digitime.exact = true;
      }
    else if(digitime.exact)
      {
        digitime.quality = 20;
      }
    else
      {
        digitime.quality = 0;
      }

    logs(LOG_DEBUG) << "timing quality " << digitime.quality << "%" << endl;

    if(dconf.soh_log_dir.length() == 0)
        return;
    
    tm tnow = *gmtime((time_t *)&b->lseconds);
    
    char datetime[32];
    strftime(datetime, 32, "%Y-%m-%d %H:%M:%S", &tnow);

    char tFile1[32];
    sprintf(tFile1, "%s.HRD24-%d.", station_name.c_str(), instrument_id);

    char tFile2[32];
    strftime(tFile2, 32, "%b.%d.%Y.vcx", &tnow);

    string filename = dconf.soh_log_dir + "/" + tFile1 + tFile2;
    FILE *fp = fopen(filename.c_str(), "a");

    if(fp == NULL)
        return;

    if(ftell(fp) == 0)
      {
        fprintf(fp, "Vcxo SOH for Instrument %d\n", instrument_id);
        fprintf(fp, " Time(secs),          Time(date-time), VcxoValue, TimeDiffAtLock, TimeError, FreqError, CrystalTemp, PLLStatus, GPSStatus\n");
      }
    
    // In documentation and according to example SOH file "VcxoValue,TimeDiffAtLock,TimeError,FreqError"
    // values should be floating point numbers. However in the packet these are just 2 byte values.
    // The program saves these as integer at the moment.
 
    fprintf(fp, "%11u,%25s,%10.2f,%15.2f,%10.2f,%10.2f,%12d,%10d,%10d\n",
      b->lseconds, datetime, (float)b->vcxo, (float)b->time_diff_at_lock,
      (float)b->time_error, (float)b->frequency_error, b->crystal_temp,
      b->pll_status, b->gps_status);

    fclose(fp);
  }

void HRD24Protocol::process_slow_internal_soh_bundle(const SlowInternalSOHBundle *b,
  int instrument_id)
  {
    internal_check(b->type == 34);

    if(dconf.soh_log_dir.length() == 0)
        return;
    
    tm tnow = *gmtime((time_t *)&b->lseconds);
    
    char datetime[32];
    strftime(datetime, 32, "%Y-%m-%d %H:%M:%S", &tnow);

    char tFile1[32];
    sprintf(tFile1, "%s.HRD24-%d.", station_name.c_str(), instrument_id);

    char tFile2[32];
    strftime(tFile2, 32, "%b.%d.%Y.hrd", &tnow);

    string filename = dconf.soh_log_dir + "/" + tFile1 + tFile2;
    FILE *fp = fopen(filename.c_str(), "a");

    if(fp == NULL)
        return;

    if(ftell(fp) == 0)
      {
        fprintf(fp, "HRD Slow Internal SOH for Instrument %d\n", instrument_id);
        fprintf(fp, " Time(secs),          Time(date-time), BattVoltage,    VCXOTemp,    RadioSNR\n");
      }
    
    fprintf(fp, "%11u,%25s,%12.4f,%12.4f,%12.4f\n",
      b->lseconds, datetime, b->batt_voltage, b->vcxo_temp, b->radio_snr);

    fclose(fp);
  }

void HRD24Protocol::process_gps_location_bundle(const GPSLocationBundle *b,
  int instrument_id)
  {
    internal_check(b->type == 13);

    if(dconf.soh_log_dir.length() == 0)
        return;
    
    tm tnow = *gmtime((time_t *)&b->lseconds);
    
    char datetime[32];
    strftime(datetime, 32, "%Y-%m-%d %H:%M:%S", &tnow);

    char tFile1[32];
    sprintf(tFile1, "%s.HRD24-%d.", station_name.c_str(), instrument_id);

    char tFile2[32];
    strftime(tFile2, 32, "%b.%d.%Y.loc", &tnow);

    string filename = dconf.soh_log_dir + "/" + tFile1 + tFile2;
    FILE *fp = fopen(filename.c_str(), "a");

    if(fp == NULL)
        return;

    if(ftell(fp) == 0)
      {
        fprintf(fp, "Gps Location SOH for Instrument %d\n", instrument_id);
        fprintf(fp, " Time(secs),          Time(date-time),    Latitude,  Longtitude, Elevation\n");
      }
    
    fprintf(fp, "%11u,%25s,%12.4f,%12.4f,%10.2f\n",
      b->lseconds, datetime, b->latitude, b->longitude, b->elevation);

    fclose(fp);
  }

void HRD24Protocol::process_status_packet(const StatusBundle *b,
  int instrument_id)
  {
    for(int bn = 0; bn < dconf.nbundles; ++bn)
      {
        switch(b[bn].type)
          {
          case 7:
            logs(LOG_DEBUG) << "found VCXO calibration bundle, t=" << b[bn].lseconds << endl;
            process_vcxo_calibration_bundle((VCXOCalibrationBundle *) &b[bn],
              instrument_id);
            break;
          
          case 9: // null bundle
            return;
          
          case 10:
            logs(LOG_DEBUG) << "found Min-Max1 bundle, t=" << b[bn].lseconds << endl;
            break;

          case 11:
            logs(LOG_DEBUG) << "found Min-Max2 bundle, t=" << b[bn].lseconds << endl;
            break;

          case 12:
            logs(LOG_DEBUG) << "found instrument log bundle, t=" << b[bn].lseconds << endl;
            break;

          case 13:
            logs(LOG_DEBUG) << "found GPS location bundle, t=" << b[bn].lseconds << endl;
            process_gps_location_bundle((GPSLocationBundle *) &b[bn],
              instrument_id);
            break;

          case 15:
            logs(LOG_DEBUG) << "found GPS sattellite status/reference time error bundle, t=" << b[bn].lseconds << endl;
            process_gps_satellite_status_bundle((GPSSatelliteStatusBundle *) &b[bn],
              instrument_id);
            break;

          case 20:
            logs(LOG_DEBUG) << "found D1 (early) treshold trigger bundle, t=" << b[bn].lseconds << endl;
            break;

          case 21:
            logs(LOG_DEBUG) << "found D2 (late) treshold trigger bundle, t=" << b[bn].lseconds << endl;
            break;

          case 22:
            logs(LOG_DEBUG) << "found (early) STA/LTA trigger bundle, t=" << b[bn].lseconds << endl;
            break;

          case 23:
            logs(LOG_DEBUG) << "found (late) STA/LTA trigger bundle, t=" << b[bn].lseconds << endl;
            break;

          case 24:
            logs(LOG_DEBUG) << "found event bundle, t=" << b[bn].lseconds << endl;
            break;

          case 32:
            logs(LOG_DEBUG) << "found fast external SOH bundle, t=" << b[bn].lseconds << endl;
            break;

          case 33:
            logs(LOG_DEBUG) << "found slow external SOH bundle, t=" << b[bn].lseconds << endl;
            break;

          case 34:
            logs(LOG_DEBUG) << "found slow internal SOH bundle, t=" << b[bn].lseconds << endl;
            process_slow_internal_soh_bundle((SlowInternalSOHBundle *) &b[bn],
              instrument_id);
            break;

          case 39:
            logs(LOG_DEBUG) << "found GPS time quality bundle, t=" << b[bn].lseconds << endl;
            break;

          default:
            logs(LOG_DEBUG) << "unsupported status bundle " << (int)b->type << ", t=" << b[bn].lseconds << endl;
          }
      }
  }

void HRD24Protocol::process_data_packet(const DataBundle *b,
  int chn, int smple)
  {
    if(chn >= NCHAN)
      {
        logs(LOG_WARNING) << "invalid channel " << chn << endl;
        return;
      }
    
    if(hrd24_channels[chn] == NULL)
        return;
    
    if(xn[chn] != NONVALUE && xn[chn] != smple)
        logs(LOG_WARNING) << "data integrity error in channel " << chn << endl;
    
    hrd24_channels[chn]->set_timemark(digitime.it, 0,
      digitime.quality);
    
    for(int bn = 0; bn < dconf.nbundles; ++bn)
      {
        if(b[bn].desc == 9) // null bundle
            return;
            
        for(int wn = 0; wn < 4; ++wn)
          {
            switch((b[bn].desc >> (6 - (wn << 1))) & 0x3)
              {
              case 0:
                logs(LOG_WARNING) << "invalid dataset descriptor in channel " << chn << endl;
                xn[chn] = NONVALUE;
                return;

              case 1:
                for(int i = 0; i < 4; ++i)
                  {
                    smple += b[bn].w[wn].d8[i];
                    hrd24_channels[chn]->put_sample(smple);
                  }
                break;

              case 2:
                for(int i = 0; i < 2; ++i)
                  {
                    smple += b[bn].w[wn].d16[i];
                    hrd24_channels[chn]->put_sample(smple);
                  }
                break;

              case 3:
                smple += b[bn].w[wn].d32;
                hrd24_channels[chn]->put_sample(smple);
                break;
              }
          }
      }

    xn[chn] = smple;
  }

void HRD24Protocol::do_start()
  {
    unsigned char buf[MAX_PACKLEN];
    unsigned short sync_value = 0;
    int sync_count = -2;
    
    while(!terminate_proc)
      {
        if(read_port(fd, buf, 1) == 0)
            continue;

        ++sync_count;
        
        if((sync_value = sync_value << 8 | buf[0]) != 0xaabb)
            continue;
        
        if(sync_count != 0)
            logs(LOG_INFO) << "sync " << sync_count << " bytes" << endl;

        sync_count = -2;
        
        if(read_port(fd, buf, HEADLEN + BNDLEN * dconf.nbundles + 2) == 0)
          {
            logs(LOG_WARNING) << "read timeout" << endl;
            continue;
          }

        int crc = 22999;
        for(int i = 0; i < HEADLEN + BNDLEN * dconf.nbundles; ++i)
            crc = updcrc(buf[i], crc);

        if(crc != *((le_u_int16_t *) &buf[HEADLEN + BNDLEN * dconf.nbundles]))
          {
            logs(LOG_WARNING) << "CRC mismatch" << endl;
            continue;
          }
        
        const PacketHeader* h = (PacketHeader *) buf;

        if((h->type & ~0x20) == 9)
          {
            logs(LOG_DEBUG) << "received filler packet, type = " <<
              (int)h->type << endl;
            continue;
          }
        
        int chn = h->rate_chn_x0 & 0x3;
        int smple = h->rate_chn_x0 >> 8;
        set_time(h->lseconds, h->sseconds);
        
        logs(LOG_DEBUG) << "received packet, type = " << (int)h->type <<
          ", channel = " << chn <<
          ", time = " << time_to_str(digitime.it, MONTHS_FMT) << endl;

        if(h->type & 0x20)
          {
            logs(LOG_DEBUG) << "retransmit packet ignored" << endl;
            continue;
          }
        
        switch(h->type)
          {
          case 1:
            process_data_packet((DataBundle *) &buf[HEADLEN], chn, smple);
            break;
            
          case 2:
            process_status_packet((StatusBundle *) &buf[HEADLEN],
              h->serial_no & 0x7ff);
            break;

          default:
            logs(LOG_DEBUG) << "unsupported packet type " << (int)h->type << endl;
          }
      }
  }

RegisterProto<HRD24Protocol> proto("hrd24");

} // unnamed namespace

