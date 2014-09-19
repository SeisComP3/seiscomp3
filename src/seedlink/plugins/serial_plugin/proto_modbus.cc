/***************************************************************************** 
 * proto_modbus.cc
 *
 * MODBUS protocol implementation
 *
 * (c) 2008 Andres Heinloo, GFZ Potsdam
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

#include "qdefines.h"
#include "qtime.h"

#include "utils.h"
#include "cppstreams.h"
#include "serial_plugin.h"
#include "plugin_channel.h"
#include "diag.h"

#include "big-endian.h"

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

const int NCHAN             = 256;
const int SAMPLE_PERIOD     = 10;
const int MAX_TIME_ERROR    = 1000000;

//*****************************************************************************
// Data Structures
//*****************************************************************************

struct ModbusRequest
  {
    be_u_int16_t transaction_id;
    be_u_int16_t protocol_id;
    be_u_int16_t length;
    u_int8_t unit_id;
    u_int8_t function;
    be_u_int16_t reference;
    be_u_int16_t word_count;
  } PACKED;

struct ModbusResponse
  {
    be_u_int16_t transaction_id;
    be_u_int16_t protocol_id;
    be_u_int16_t length;
    u_int8_t unit_id;
    u_int8_t function;
    u_int8_t bc_ex;
    be_int16_t data[NCHAN];
  } PACKED;

//*****************************************************************************
// ModbusProtocol
//*****************************************************************************

void alarm_handler(int sig);

class OutputChannelEx: public OutputChannel
  {
  public:
    const double realscale;
    const double realoffset;
    const string realunit;
    const int precision;
    
    OutputChannelEx(const string &channel_name, const string &station_name,
      int max_zeros, double scale, double realscale_init,
      double realoffset_init, const string &realunit_init, int precision_init):
      OutputChannel(channel_name, station_name, max_zeros, scale),
      realscale(realscale_init), realoffset(realoffset_init),
      realunit(realunit_init), precision(precision_init) {}
  };

class ModbusProtocol: public Proto
  {
  private:
    int fd;
    vector<rc_ptr<OutputChannelEx> > modbus_channels;
    int minchan, maxchan;
    int tran_count;
    bool startup_message;
    bool soh_message;
    int last_day;
    int last_soh;
    ModbusResponse resp;

    static ModbusProtocol *obj;
    static void alarm_handler(int sig);
    
    ssize_t writen(int fd, const void *vptr, size_t n);
    void handle_response();
    void send_request();
    void do_start();

  public:
    ModbusProtocol(const string &myname): modbus_channels(NCHAN),
      minchan(NCHAN), maxchan(0), tran_count(0), startup_message(true),
      soh_message(true), last_day(-1), last_soh(-1)
      {
        obj = this;
      }

    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision);
    void flush_channels();
    void start();
  };

ModbusProtocol *ModbusProtocol::obj;

void ModbusProtocol::alarm_handler(int sig)
  {
    obj->send_request();
  }

void ModbusProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);
    
    if(*tail || n > NCHAN)
        throw PluginADInvalid(source_id, channel_name);

    if(n == NCHAN)
        return;
    
    if(modbus_channels[n] != NULL)
        throw PluginADInUse(source_id, modbus_channels[n]->channel_name);

    modbus_channels[n] = new OutputChannelEx(channel_name, station_name,
      dconf.zero_sample_limit, scale, realscale, realoffset, realunit, precision);

    if(n < minchan) minchan = n;

    if(n > maxchan) maxchan = n;
  }

void ModbusProtocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(modbus_channels[n] != NULL)
            modbus_channels[n]->flush_streams();
      }
  }

void ModbusProtocol::start()
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

ssize_t ModbusProtocol::writen(int fd, const void *vptr, size_t n)
  {
    ssize_t nwritten;
    size_t nleft = n;
    const char *ptr = (const char *) vptr;   

    while (nleft > 0)
      {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
          {
            if(nwritten < 0)
              {
                if(errno == EAGAIN)
                  {
                    if(!terminate_proc)
                      {
                        usleep(100000);
                        continue;
                      }
                  }
                else if(errno == EBADF)  // fail silently if no TCP connection
                  {
                    return 0;
                  }
              }

            return(nwritten);
          }

        nleft -= nwritten;
        ptr += nwritten;
      }
    
    return(n);
  }

void ModbusProtocol::handle_response()
  {
    struct timeval tv;
    N(gettimeofday(&tv, NULL));
    time_t t = tv.tv_sec;
    tm* ptm = gmtime(&t);
    
    EXT_TIME et;
    et.year = ptm->tm_year + 1900;
    et.month = ptm->tm_mon + 1;
    et.day = ptm->tm_mday;
    et.hour = ptm->tm_hour;
    et.minute = ptm->tm_min;
    et.second = ptm->tm_sec;
    et.usec = tv.tv_usec;
    et.doy = mdy_to_doy(et.month, et.day, et.year);

    INT_TIME it = ext_to_int(et);
    
    if(!digitime.valid)
      {
        digitime.it = it;
        digitime.valid = true;
        digitime.exact = true;
      }
    else
      {
        digitime.it = add_time(digitime.it, SAMPLE_PERIOD, 0);
        double time_diff = tdiff(it, digitime.it);

        DEBUG_MSG("time_diff = " << time_diff << endl);
        
        if(time_diff < -MAX_TIME_ERROR || time_diff > MAX_TIME_ERROR)
          {
            logs(LOG_WARNING) << "time diff. " << time_diff / 1000000.0 << " sec" << endl;
            digitime.it = it;
          }
      }

    if(digitime.it.second / (24 * 60 * 60) != last_day)
      {
        startup_message = true;
        soh_message = true;
        last_day = digitime.it.second / (24 * 60 * 60);
      }
    
    if(dconf.statusinterval &&
      digitime.it.second / (dconf.statusinterval * 60) != last_soh)
      {
        soh_message = true;
        last_soh = digitime.it.second / (dconf.statusinterval * 60);
      }

    if(et.hour != 0 || et.minute != 0 || et.second != 0)
      {
        if(startup_message)
          {
            startup_message = false;
            seed_log << ident_str << SEED_NEWLINE
                     << "protocol: " << dconf.proto_name << SEED_NEWLINE
                     << "sample rate: " << (1 / (double(SAMPLE_PERIOD)))
                     << " Hz" << endl;
          }

        if(soh_message)
          {
            soh_message = false;
            char tbuf[50];
            strftime(tbuf, 50, "%Y-%m-%d;%H:%M:%S", ptm);
            seed_log << fixed << "status: " << tbuf;

            for(int n = minchan; n <= maxchan; ++n)
              {
                if(modbus_channels[n] == NULL) continue;

                seed_log << ";" << setprecision(modbus_channels[n]->precision);

                if(modbus_channels[n]->realscale < 0)
                    seed_log << (resp.data[n - minchan] - modbus_channels[n]->realoffset);
                else
                    seed_log << (resp.data[n - minchan] * modbus_channels[n]->realscale - modbus_channels[n]->realoffset);
                seed_log << modbus_channels[n]->realunit;
              }

            seed_log << endl;
          }
      }

    for(int n = minchan; n <= maxchan; ++n)
      {
        if(modbus_channels[n] == NULL) continue;

        modbus_channels[n]->set_timemark(digitime.it, 0, digitime.quality);
        modbus_channels[n]->put_sample(resp.data[n - minchan]);
      }
  }

void ModbusProtocol::send_request()
  {
    DEBUG_MSG("request" << endl);

    ModbusRequest req;
    req.transaction_id = tran_count++;
    req.protocol_id = 0;
    req.length = 6;
    req.unit_id = 1;
    req.function = 4;
    req.reference = dconf.baseaddr + minchan;
    req.word_count = maxchan - minchan + 1;

    if(writen(fd, &req, sizeof(ModbusRequest)) < 0)
        throw PluginLibraryError("error writing to " + dconf.port_name);
  }

void ModbusProtocol::do_start()
  {
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGALRM, &sa, NULL));

    struct itimerval itv;
    itv.it_interval.tv_sec = SAMPLE_PERIOD;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    N(setitimer(ITIMER_REAL, &itv, NULL));
    
    while(!terminate_proc)
      {
        if(read_port(fd, &resp, 11 + (maxchan - minchan) * 2) == 0)
            continue;

        if(resp.function == 0x04)
          {
            if(resp.bc_ex != (maxchan - minchan + 1) * 2)
              {
                logs(LOG_ERR) << "unexpected byte count " << resp.bc_ex << endl;
                continue;
              }

            handle_response();
          }
        else if(resp.function == 0x84)
          {
            logs(LOG_ERR) << "Modbus exception " << resp.bc_ex << endl;
          }
        else
          {
            logs(LOG_ERR) << "unexpected function code " << resp.function << endl;
          }
      }
  }

RegisterProto<ModbusProtocol> proto("modbus");

} // unnamed namespace

