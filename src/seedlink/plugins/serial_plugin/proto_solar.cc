/***************************************************************************** 
 * proto_solar.cc
 *
 * GITEWS Solar Controller protocol implementation
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
#include <cmath>
#include <cstring>
#include <cerrno>
#include <cstdio>

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

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

const int NCHAN             = 26;  // A..Z
const int SAMPLE_PERIOD     = 10;
const int RECVBUFSIZE       = 256;
const int READ_TIMEOUT      = 15;
const int MAX_TIME_ERROR    = 1000000;

//*****************************************************************************
// SolarProtocol
//*****************************************************************************

void alarm_handler(int sig);

class SolarProtocol: public Proto
  {
  private:
    int fd;
    char recvbuf[RECVBUFSIZE + 1];
    vector<rc_ptr<OutputChannel> > solar_channels;
    int wp, rp;
    bool startup_message;
    bool soh_message;
    int last_day;
    int last_soh;

    static SolarProtocol *obj;
    static void alarm_handler(int sig);
    
    ssize_t writen(int fd, const void *vptr, size_t n);
    void decode_message(const char *msg);
    void handle_response(const char *msg);
    void measure_request();
    void do_start();

  public:
    SolarProtocol(const string &myname): solar_channels(NCHAN),
      wp(0), rp(0), startup_message(true), soh_message(true),
      last_day(-1), last_soh(-1)
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

SolarProtocol *SolarProtocol::obj;

void SolarProtocol::alarm_handler(int sig)
  {
    obj->measure_request();
  }

void SolarProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);
    
    if(*tail || n >= NCHAN)
        throw PluginADInvalid(source_id, channel_name);

    if(solar_channels[n] != NULL)
        throw PluginADInUse(source_id, solar_channels[n]->channel_name);

    solar_channels[n] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void SolarProtocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(solar_channels[n] != NULL)
            solar_channels[n]->flush_streams();
      }
  }

void SolarProtocol::start()
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

ssize_t SolarProtocol::writen(int fd, const void *vptr, size_t n)
  {
    ssize_t nwritten;
    size_t nleft = n;
    const char *ptr = (const char *) vptr;   

    while (nleft > 0)
      {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
            return(nwritten);

        nleft -= nwritten;
        ptr += nwritten;
      }
    
    return(n);
  }

void SolarProtocol::decode_message(const char *msg)
  {
    DEBUG_MSG(msg << endl);

    int n = 0, rp = 0, toklen, seplen;
    while(toklen = strcspn(msg + rp, ";"),
      seplen = strspn(msg + rp + toklen, ";"))
      {
        double val;
        if(sscanf(msg + rp, "%lf;", &val) != 1)
          {
            logs(LOG_WARNING) << "error parsing '" << msg << "' at: " << (msg + rp) << endl;
            return;
          }
        
        if(solar_channels[n] != NULL)
          {
            solar_channels[n]->set_timemark(digitime.it, 0, digitime.quality);
            solar_channels[n]->put_sample(val);
          }
        
        if(++n == NCHAN)
            break;
        
        rp += (toklen + seplen);
      }

    if(toklen)
        logs(LOG_WARNING) << "unused data at the end of message: " << (msg + rp) << endl;
  }

void SolarProtocol::handle_response(const char *msg)
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
            strftime(tbuf, 50, "%Y-%m-%d;%H:%M:%S;", ptm);
            seed_log << "status: " << tbuf << msg << endl;
          }
      }

    decode_message(msg);
  }

void SolarProtocol::measure_request()
  {
    DEBUG_MSG("request" << endl);

    if(writen(fd, "Ma\r", 3) < 0)
        throw PluginLibraryError("error writing to " + dconf.port_name);
  }

void SolarProtocol::do_start()
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
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);
        
        struct timeval tv;
        tv.tv_sec = READ_TIMEOUT;
        tv.tv_usec = 0;

        int r;
        if((r = select(fd + 1, &read_set, NULL, NULL, &tv)) < 0)
          {
            if(errno == EINTR) continue;
            throw PluginLibraryError("select error");
          }

        if(r == 0)
            throw PluginError("timeout");
        
        if(FD_ISSET(fd, &read_set))
          {
            int bytes_read;
            if((bytes_read = read(fd, &recvbuf[wp], RECVBUFSIZE - wp)) < 0)
                throw PluginLibraryError("error reading from " + dconf.port_name);

            if(bytes_read == 0)
                throw PluginError("EOF reading " + dconf.port_name);
 
            wp += bytes_read;
            recvbuf[wp] = 0;
        
            int rp = 0, msglen, seplen;
            while(msglen = strcspn(recvbuf + rp, "\r\n"),
              seplen = strspn(recvbuf + rp + msglen, "\r\n"))
              {
                if(msglen > 0)
                  {
                    recvbuf[rp + msglen] = 0;
                    handle_response(recvbuf + rp);
                  }

                rp += (msglen + seplen);
              }
            
            if(msglen >= RECVBUFSIZE)
              {
                logs(LOG_WARNING) << "receive buffer overflow" << endl;
                wp = rp = 0;
                continue;
              }
        
            memmove(recvbuf, recvbuf + rp, msglen);
            wp -= rp;
            rp = 0;
          }
      }
  }

RegisterProto<SolarProtocol> proto("solar");

} // unnamed namespace

