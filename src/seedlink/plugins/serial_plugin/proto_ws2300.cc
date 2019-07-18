/***************************************************************************** 
 * proto_ws2300.cc
 *
 * Lacrosse 2300 Weather Station protocol implementation
 *
 * (c) 2005 Andres Heinloo, GFZ Potsdam
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
#include <cmath>
#include <cstring>
#include <cerrno>

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

extern "C" {
#include "rw2300.h"
#include "linux2300.h"
}

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

const int NCHAN             = 15;
const int SAMPLE_PERIOD     = 10;

//*****************************************************************************
// WS_Channel
//*****************************************************************************

class WS2300Protocol;

class WS_Channel
  {
  private:
    int lastsecond;
    int lastvalue;

  protected:
    virtual int getvalue(WS2300Protocol *obj) =0;

  public:
    WS_Channel(): lastsecond(-1), lastvalue(0) { }
    virtual ~WS_Channel() { }
    
    int operator()(WS2300Protocol *obj)
      {
        if(lastsecond != digitime.it.second)
          {
            lastvalue = getvalue(obj);
            lastsecond = digitime.it.second;
          }

        return lastvalue;
      }
  };

#define decl_channel(name) \
class WS_##name: public WS_Channel \
  { \
  private: \
    int getvalue(WS2300Protocol *obj); \
  }; \
WS_##name ws_##name

//*****************************************************************************
// WS2300Protocol
//*****************************************************************************

class WS2300Protocol: public Proto
  {
  private:
    WEATHERSTATION fd;
    WS_Channel *ws_input[NCHAN];
    bool startup_message;
    bool soh_message;
    bool time_shift_applied;
    int last_hour;
    vector<rc_ptr<OutputChannel> > ws2300_channels;

    int get_time_from_pc();
    int get_time_from_ws();
    void set_ws_hour(int hour);
    void get_time();
    int read_bcd2(int addr);
    int read_bcd4(int addr);
    int read_bcd5(int addr);
    int read_bcd6(int addr);
    int read_bin1(int addr);
    int read_bin3(int addr);
    void do_start();

    decl_channel(IndoorTemperature);
    decl_channel(OutdoorTemperature);
    decl_channel(Windchill);
    decl_channel(Dewpoint);
    decl_channel(IndoorHumidity);
    decl_channel(OutdoorHumidity);
    decl_channel(RainTotal);
    decl_channel(Rain1h);
    decl_channel(Rain24h);
    decl_channel(RawWindSpeedDirection);
    decl_channel(WindSpeed);
    decl_channel(WindDirection);
    decl_channel(AbsoluteAirPressure);
    decl_channel(RelativeAirPressure);
    decl_channel(HFStatus);

  public:
    WS2300Protocol(const string &myname):
      startup_message(true), soh_message(true), time_shift_applied(false),
      last_hour(-1), ws2300_channels(NCHAN)
      {
        ws_input[0] = &ws_IndoorTemperature;
        ws_input[1] = &ws_OutdoorTemperature;
        ws_input[2] = &ws_Windchill;
        ws_input[3] = &ws_Dewpoint;
        ws_input[4] = &ws_IndoorHumidity;
        ws_input[5] = &ws_OutdoorHumidity;
        ws_input[6] = &ws_RainTotal;
        ws_input[7] = &ws_Rain1h;
        ws_input[8] = &ws_Rain24h;
        ws_input[9] = &ws_WindSpeed;
        ws_input[10] = &ws_WindDirection;
        ws_input[11] = &ws_AbsoluteAirPressure;
        ws_input[12] = &ws_RelativeAirPressure;
        ws_input[13] = &ws_HFStatus;
        ws_input[14] = &ws_RawWindSpeedDirection;
        internal_check(NCHAN == 15);
      }

    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision);
    void flush_channels();
    void start();
  };

void WS2300Protocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset,
  const string &realunit, int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);
    
    if(*tail || n >= NCHAN)
        throw PluginADInvalid(source_id, channel_name);

    if(ws2300_channels[n] != NULL)
        throw PluginADInUse(source_id, ws2300_channels[n]->channel_name);

    ws2300_channels[n] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void WS2300Protocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(ws2300_channels[n] == NULL) continue;
        ws2300_channels[n]->flush_streams();
      }
  }

void WS2300Protocol::start()
  {
    char *port_name;
    if((port_name = (char *) malloc(dconf.port_name.length() + 1)) == NULL)
        throw bad_alloc();

    strcpy(port_name, dconf.port_name.c_str());
    fd = open_weatherstation(port_name);
    free(port_name);

    try
      {
        do_start();
      }
    catch(PluginError &e)
      {
        seed_log << "closing device" << endl;
        close_weatherstation(fd);
        throw;
      }

    seed_log << "closing device" << endl;
    close_weatherstation(fd);
  }

int WS2300Protocol::get_time_from_pc()
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
    digitime.valid = true;
    digitime.exact = true;
    return et.hour;
  }

int WS2300Protocol::get_time_from_ws()
  {
    unsigned char data[20];
    unsigned char command[25];

    if(read_safe(fd, 0x200, 3, data, command) != 3)
        throw PluginReadError(dconf.port_name);

    if(read_safe(fd, 0x240, 3, data + 3, command) != 3)
        throw PluginReadError(dconf.port_name);

    EXT_TIME et;
    et.second = (data[0] >> 4) * 10 + (data[0] & 0xf);
    et.minute = (data[1] >> 4) * 10 + (data[1] & 0xf);
    et.hour = (data[2] >> 4) * 10 + (data[2] & 0xf);
    et.day = (data[3] >> 4) * 10 + (data[3] & 0xf);
    et.month = (data[4] >> 4) * 10 + (data[4] & 0xf);
    et.year = (data[5] >> 4) * 10 + (data[5] & 0xf) + 2000;
    et.usec = 0;

    if(et.year < 2000 || et.year > 2099 ||
      et.month < 1 || et.month > 12 ||
      et.day < 1 || et.day > 31 ||
      et.hour < 0 || et.hour > 23 ||
      et.minute < 0 || et.minute > 59 ||
      et.second < 0 || et.second > 61)
      {
        logs(LOG_WARNING) << "invalid WS time: " << et.year << "," <<
          et.month << "," << et.day << "," << et.hour << "," <<
          et.minute << "," << et.second << endl;

        digitime.exact = false;
        return -1;
      }

    et.doy = mdy_to_doy(et.month, et.day, et.year);
    digitime.it = ext_to_int(et);
    digitime.valid = true;
    digitime.exact = true;
    
    logs(LOG_DEBUG) << "WS time: " << time_to_str(digitime.it, MONTHS_FMT) << endl;
    return et.hour;
  }

void WS2300Protocol::set_ws_hour(int hour)
  {
    unsigned char data[2];
    unsigned char command[25];

    data[0] = hour % 10;
    data[1] = hour / 10;
    
    if (write_safe(fd, 0x204, 2, WRITENIB, data, command) != 2)
        throw PluginReadError(dconf.port_name);
  }

void WS2300Protocol::get_time()
  {
    int hour;
    if(dconf.use_pctime_if_no_gps)
      {
        hour = read_bcd2(0x204);
        get_time_from_pc();
      }
    else
      {
        hour = get_time_from_ws();
        if(time_shift_applied && hour != -1)
            digitime.it = add_time(digitime.it, -8 * 3600, 0);
      }

    if(hour == 0)
      {
        if(!time_shift_applied)
          {
            set_ws_hour(8);
            time_shift_applied = true;
          }
      }
    else if(hour == 16)
      {
        if(time_shift_applied)
          {
            set_ws_hour(8);
            time_shift_applied = false;
          }
      }

    if(hour != last_hour)
      {
        logs(LOG_INFO) << "WS hour: " << hour << endl;
        last_hour = hour;
      }
  }

int WS2300Protocol::read_bcd2(int addr)
  {
    unsigned char data[20];
    unsigned char command[25];

    if(read_safe(fd, addr, 1, data, command) != 1)
        throw PluginReadError(dconf.port_name);

    return ((data[0] >> 4) * 10 + (data[0] & 0xf));
  }

int WS2300Protocol::read_bcd4(int addr)
  {
    unsigned char data[20];
    unsigned char command[25];

    if(read_safe(fd, addr, 2, data, command) != 2)
        throw PluginReadError(dconf.port_name);

    return ((data[0] >> 4) * 10 + (data[0] & 0xf) +
      (data[1] >> 4) * 1000 + (data[1] & 0xf) * 100);
  }

int WS2300Protocol::read_bcd5(int addr)
  {
    unsigned char data[20];
    unsigned char command[25];

    if(read_safe(fd, addr, 3, data, command) != 3)
        throw PluginReadError(dconf.port_name);

    return ((data[0] >> 4) * 10 + (data[0] & 0xf) +
      (data[1] >> 4) * 1000 + (data[1] & 0xf) * 100 +
      (data[2] & 0xf) * 10000);
  }

int WS2300Protocol::read_bcd6(int addr)
  {
    unsigned char data[20];
    unsigned char command[25];

    if(read_safe(fd, addr, 3, data, command) != 3)
        throw PluginReadError(dconf.port_name);

    return ((data[0] >> 4) * 10 + (data[0] & 0xf) +
      (data[1] >> 4) * 1000 + (data[1] & 0xf) * 100 +
      (data[2] >> 4) * 100000 + (data[2] & 0xf) * 10000);
  }

int WS2300Protocol::read_bin1(int addr)
  {
    unsigned char data[20];
    unsigned char command[25];

    if(read_safe(fd, addr, 1, data, command) != 1)
        throw PluginReadError(dconf.port_name);

    return data[0];
  }

int WS2300Protocol::read_bin3(int addr)
  {
    unsigned char data[20];
    unsigned char command[25];

    if(read_safe(fd, addr, 3, data, command) != 3)
        throw PluginReadError(dconf.port_name);

    return (data[0] + (data[1] << 8) + (data[2] << 16));
  }

int WS2300Protocol::WS_IndoorTemperature::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd4(0x346) - 3000;
    logs(LOG_DEBUG) << "IndoorTemperature = " << double(value) / 100 << endl;
    return value;
  }

int WS2300Protocol::WS_OutdoorTemperature::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd4(0x373) - 3000;
    logs(LOG_DEBUG) << "OutdoorTemperature = " << double(value) / 100 << endl;
    return value;
  }

int WS2300Protocol::WS_Windchill::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd4(0x3a0) - 3000;
    logs(LOG_DEBUG) << "Windchill = " << double(value) / 100 << endl;
    return value;
  }

int WS2300Protocol::WS_Dewpoint::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd4(0x3ce) - 3000;
    logs(LOG_DEBUG) << "Dewpoint = " << double(value) / 100 << endl;
    return value;
  }

int WS2300Protocol::WS_IndoorHumidity::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd2(0x3fb);
    logs(LOG_DEBUG) << "IndoorHumidity = " << value << endl;
    return value;
  }

int WS2300Protocol::WS_OutdoorHumidity::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd2(0x419);
    logs(LOG_DEBUG) << "OutdoorHumidity = " << value << endl;
    return value;
  }

int WS2300Protocol::WS_RainTotal::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd6(0x4d2);
    logs(LOG_DEBUG) << "RainTotal = " << double(value) / 100 << endl;
    return value;
  }

int WS2300Protocol::WS_Rain1h::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd6(0x4b4);
    logs(LOG_DEBUG) << "Rain1h = " << double(value) / 100 << endl;
    return value;
  }

int WS2300Protocol::WS_Rain24h::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd6(0x497);
    logs(LOG_DEBUG) << "Rain24h = " << double(value) / 100 << endl;
    return value;
  }

int WS2300Protocol::WS_RawWindSpeedDirection::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bin3(0x527);
    logs(LOG_DEBUG) << "RawWindSpeedDirection = 0x" <<
      hex << setfill('0') << setw(6) << value << dec << endl;

    return value;
  }

int WS2300Protocol::WS_WindSpeed::getvalue(WS2300Protocol *obj)
  {
    int value = -1;
    int wsd = obj->ws_RawWindSpeedDirection(obj);
    
    if((wsd & 0xff) == 0x00)
        value = (wsd >> 8) & 0xfff;
        
    logs(LOG_DEBUG) << "WindSpeed = ";
    
    if(value == -1)
        logs(LOG_DEBUG) << "N/A" << endl;
    else
        logs(LOG_DEBUG) << double(value) / 10 << endl;

    return value;
  }

int WS2300Protocol::WS_WindDirection::getvalue(WS2300Protocol *obj)
  {
    int value = -1;
    int wsd = obj->ws_RawWindSpeedDirection(obj);
    
    if((wsd & 0xff) == 0x00)
        value = (wsd >> 20) & 0xf;
        
    logs(LOG_DEBUG) << "WindDirection = ";
    
    if(value == -1)
        logs(LOG_DEBUG) << "N/A" << endl;
    else
        logs(LOG_DEBUG) << double(value) * 22.5 << endl;

    return value;
  }

int WS2300Protocol::WS_AbsoluteAirPressure::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd5(0x5d8);
    logs(LOG_DEBUG) << "AbsoluteAirPressure = " << double(value) / 10 << endl;
    return value;
  }

int WS2300Protocol::WS_RelativeAirPressure::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bcd5(0x5e2);
    logs(LOG_DEBUG) << "RelativeAirPressure = " << double(value) / 10 << endl;
    return value;
  }

int WS2300Protocol::WS_HFStatus::getvalue(WS2300Protocol *obj)
  {
    int value = obj->read_bin1(0x54d) & 0xf;
    logs(LOG_DEBUG) << "HFStatus = 0x" << hex << value << dec << endl;
    return value;
  }

void WS2300Protocol::do_start()
  {
    get_time();
    INT_TIME sample_time = digitime.it;
    int last_hf_status = -1;
    int last_soh = -1;

    while(!terminate_proc)
      {
        get_time();
        int td = int(tdiff(digitime.it, sample_time) / 1000000.0 + 0.5);
        if(td >= -SAMPLE_PERIOD && td < 0)
            continue;

        if(td < -SAMPLE_PERIOD || td > 2 * SAMPLE_PERIOD)
            sample_time = digitime.it;

        EXT_TIME et = int_to_ext(sample_time);
        if(et.hour == 0 && et.minute == 0 && et.second < SAMPLE_PERIOD)
          {
            startup_message = true;
            soh_message = true;
          }
        
        if(dconf.statusinterval &&
          digitime.it.second / (dconf.statusinterval * 60) != last_soh)
          {
            soh_message = true;
            last_soh = digitime.it.second / (dconf.statusinterval * 60);
          }

        if(startup_message && !(et.hour == 0 && et.minute == 0 && et.second == 0))
          {
            startup_message = false;
            seed_log << ident_str << endl
                     << "Lacrosse WS2300 Weather Station" << endl
                     << "Program setup: "
                        "protocol=" << dconf.proto_name << " "
                        "pctime=" << dconf.use_pctime_if_no_gps << " "
                        "statusinterval=" << dconf.statusinterval << endl;
          }

        if(soh_message && !(et.hour == 0 && et.minute == 0 && et.second == 0))
          {
            soh_message = false;
            seed_log << dec << fixed
                     << "Weather data: "
                     << setprecision(2)
                     << double(ws_IndoorTemperature(this)) / 100 << "C "
                     << double(ws_OutdoorTemperature(this)) / 100 << "C "
                     << double(ws_Windchill(this)) / 100 << "C "
                     << double(ws_Dewpoint(this)) / 100 << "C "
                     << ws_IndoorHumidity(this) << "% "
                     << ws_OutdoorHumidity(this) << "% "
                     << double(ws_RainTotal(this)) / 100 << "mm "
                     << double(ws_Rain1h(this)) / 100 << "mm "
                     << double(ws_Rain24h(this)) / 100 << "mm "
                     << setprecision(1);
            
            int wind_speed = ws_WindSpeed(this);
            if(wind_speed == -1)
                seed_log << "N/A ";
            else
                seed_log << double(wind_speed) / 10 << "m/s ";

            int wind_direction = ws_WindDirection(this);
            if(wind_direction == -1)
                seed_log << "N/A ";
            else
                seed_log << double(wind_direction) * 22.5 << "deg ";
                
            seed_log << double(ws_AbsoluteAirPressure(this)) / 10 << "hPa "
                     << "0x" << hex << ws_HFStatus(this) << dec << endl;
          }
            
        int hf_status = ws_HFStatus(this);
        if(hf_status != last_hf_status)
          {
            switch(hf_status)
              {
              case 0x0:
                logs(LOG_INFO) << "HF: cable" << endl;
                break;

              case 0x3:
                logs(LOG_INFO) << "HF: no signal" << endl;
                break;

              case 0xf:
                logs(LOG_INFO) << "HF: wireless" << endl;
                break;

              default:
                logs(LOG_INFO) << "HF: unknown status 0x" <<
                  hex << hf_status << dec << endl;
              }
            
            last_hf_status = hf_status;
          }
        
        for(int i = 0; i < NCHAN; ++i)
          {
            if(ws2300_channels[i] == NULL)
                continue;

            ws2300_channels[i]->set_timemark(sample_time, 0, digitime.quality);
            ws2300_channels[i]->put_sample((*this->ws_input[i])(this));
            ws2300_channels[i]->flush();
          }

        sample_time = add_time(sample_time, SAMPLE_PERIOD, 0);
      }
  }
    
RegisterProto<WS2300Protocol> proto("ws2300");

} // unnamed namespace

