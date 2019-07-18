/*****************************************************************************
 * proto_prema.cc
 *
 * Prema DMM 5017 Multimeter protocol implementation
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
#include <cmath>
#include <cstring>
#include <cerrno>
#include <cstdio>

#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef POSIX_PRIORITY_SCHEDULING
#define POSIX_PRIORITY_SCHEDULING
#endif

#ifdef POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif

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

const int DATA_CHANNELS     = 2;
const int NCHAN             = 2 * DATA_CHANNELS;
const int PERIOD_SEC        = 5;
const int PERIOD_USEC       = 0;
const int RECVBUFSIZE       = 256;
const int READ_TIMEOUT      = 10;
const int MAX_TIME_ERROR    = 50000;
const int STARTUP_WAIT      = 6;
const double SCALE          = 30;

const char *const device_setup = "MR VD P01 A0 R3 F2 T6";
const char *const device_setup_check = "MRVDP01A0R3F2T6";
const char *chselect[DATA_CHANNELS] = { "M01", "M02" };

//*****************************************************************************
// PremaProtocol
//*****************************************************************************

void alarm_handler(int sig);

class PremaProtocol: public Proto
  {
  private:
    int fd;
    char recvbuf[RECVBUFSIZE + 1];
    vector<rc_ptr<OutputChannel> > prema_channels;
    int mux, rdstate, wp, rp;
    int startup_wait;
    double time_diff;
    string device_id;
    bool id_request;
    bool startup_message;

    static PremaProtocol *obj;
    static void alarm_handler(int sig);

    ssize_t writen(int fd, const void *vptr, size_t n);
    void init();
    void decode_sample(const char *msg);
    void handle_response(const char *msg);
    void measure_request();
    void do_start();

  public:
    PremaProtocol(const string &myname):
      prema_channels(NCHAN), mux(0), rdstate(0), wp(0), rp(0), startup_wait(0),
      time_diff(0), id_request(false), startup_message(false)
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

PremaProtocol *PremaProtocol::obj;

void PremaProtocol::alarm_handler(int sig)
  {
    obj->measure_request();
  }

void PremaProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);

    if(*tail || n >= NCHAN)
        throw PluginADInvalid(source_id, channel_name);

    if(prema_channels[n] != NULL)
        throw PluginADInUse(source_id, prema_channels[n]->channel_name);

    prema_channels[n] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void PremaProtocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(prema_channels[n] == NULL) continue;
        prema_channels[n]->flush_streams();
      }
  }

void PremaProtocol::start()
  {
// #ifdef POSIX_PRIORITY_SCHEDULING
// Necessary due to a change in the posix standard. 200112L means
// supported, -1 and 0 unsupported
#if (_POSIX_PRIORITY_SCHEDULING - 0) >=  200112L
    struct sched_param schp;
    memset(&schp, 0, sizeof(struct sched_param));

    schp.sched_priority = 10;
    if(sched_setscheduler(0, SCHED_FIFO, &schp) < 0)
      {
        logs(LOG_NOTICE) << string("cannot use real-time scheduler: ") +
          strerror(errno) << endl;
      }
#endif

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

ssize_t PremaProtocol::writen(int fd, const void *vptr, size_t n)
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

void PremaProtocol::init()
  {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    N(sigprocmask(SIG_BLOCK, &mask, NULL));

    logs(LOG_INFO) << "sending *RST" << endl;
    writen(fd, "*RST\n", 5); usleep(500000);

    logs(LOG_INFO) << "sending *IDN?" << endl;
    writen(fd, "*IDN?\n", 6); usleep(500000);

    logs(LOG_INFO) << "sending MP2" << endl;
    writen(fd, "MP2\n", 4); usleep(500000);

    for(int i = 0; i < DATA_CHANNELS; ++i)
      {
        logs(LOG_INFO) << "sending " << chselect[i] << endl;
        writen(fd, chselect[i], strlen(chselect[i]));
        writen(fd, "\n", 1);
        usleep(500000);

        const char* p = device_setup;
        int cmdlen = 0;
        while(p += cmdlen, p += strspn(p, " "), cmdlen = strcspn(p, " "))
          {
            logs(LOG_INFO) << "sending " << string(p, cmdlen) << endl;
            writen(fd, p, cmdlen);
            writen(fd, "\n", 1);
            usleep(500000);
          }
      }

    logs(LOG_INFO) << "init done" << endl;
    writen(fd, "D1\"SeisComP\"\n", 13); usleep(500000);

    digitime.valid = false;
    digitime.exact = false;
    id_request = true;
    mux = DATA_CHANNELS - 1;
    startup_wait = STARTUP_WAIT;
    rdstate = 0;

    N(sigprocmask(SIG_UNBLOCK, &mask, NULL));
  }

void PremaProtocol::decode_sample(const char *msg)
  {
    if(strncmp(&msg[13], device_setup_check, strlen(device_setup_check)))
        throw PluginError(string("incorrect device setup: ") + &msg[13]);

    if(!strncmp(msg, "ERROR ", 6))
      {
        logs(LOG_WARNING) << "error " << string(msg, 6, 2) << endl;
        return;
      }

    char buf[14];
    strncpy(buf, msg, 13);
    buf[13] = 0;

    double fval;
    char c;
    if(sscanf(buf, "%lf%c", &fval, &c) != 1)
      {
        logs(LOG_WARNING) << "could not decode sample: " << buf << endl;
        return;
      }

    int ival = int(round(fval / SCALE * double((1 << (31 - dconf.lsb)) - 1)));
    if(prema_channels[mux] != NULL)
      {
        prema_channels[mux]->set_timemark(digitime.it, 0, 100);
        prema_channels[mux]->put_sample(ival);
      }

    if(prema_channels[DATA_CHANNELS + mux] != NULL)
      {
        prema_channels[DATA_CHANNELS + mux]->set_timemark(digitime.it, 0, 100);
        prema_channels[DATA_CHANNELS + mux]->put_sample(int(time_diff));
      }

    DEBUG_MSG(time_to_str(digitime.it, MONTHS_FMT) << " " << msg << " " <<
      mux << " " << ival << endl);
  }

void PremaProtocol::handle_response(const char *msg)
  {
    if(id_request)
      {
        device_id = msg;
        id_request = false;
        startup_message = true;
        return;
      }

    if(rdstate == 1)
      {
        writen(fd, "RD?\n", 4);
        rdstate = 2;
        return;
      }

    if(rdstate != 2)
      {
        logs(LOG_WARNING) << "unexpected response: " << msg << endl;
        return;
      }

    rdstate = 0;

    struct timeval tv;
    N(gettimeofday(&tv, NULL));
    time_t t = tv.tv_sec;
    tm* ptm = gmtime(&t);

    if(startup_wait > 0)
      {
        --startup_wait;
        return;
      }

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
        time_diff = 0;
      }
    else
      {
        digitime.it = add_time(digitime.it, PERIOD_SEC, PERIOD_USEC);
        time_diff = tdiff(it, digitime.it);

        DEBUG_MSG("time_diff = " << time_diff << endl);

        if(time_diff < -MAX_TIME_ERROR || time_diff > MAX_TIME_ERROR)
          {
            logs(LOG_WARNING) << "time diff. " << time_diff / 1000000.0 << " sec" << endl;
            digitime.it = it;
            time_diff = 0;
          }
      }

    if(et.hour == 0 && et.minute == 0 && et.second == 1)
        startup_message = true;

    if(startup_message)
      {
        startup_message = false;
        seed_log << ident_str << endl
                 << "Device ID: " << device_id << endl
                 << "Device setup: " << device_setup << endl
                 << "Channels: " << DATA_CHANNELS << endl
                 << "Sample rate: " << 1 / ((double(PERIOD_SEC) +
                      double(PERIOD_USEC) / 1000000.0) * double(DATA_CHANNELS)) << endl
                 << "Program setup: "
                    "protocol=" << dconf.proto_name << " "
                    "lsb=" << dconf.lsb << " " << endl;
      }

    decode_sample(msg);
  }

void PremaProtocol::measure_request()
  {
    if(rdstate != 0)
      {
        rdstate = -1;
        return;
      }

    mux = (mux + 1) % DATA_CHANNELS;
    writen(fd, chselect[mux], 3);
    writen(fd, "\n", 1);
    usleep(100000);

    writen(fd, "RD?\n", 4);
    rdstate = 1;
  }

void PremaProtocol::do_start()
try
  {
    init();

    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGALRM, &sa, NULL));

    struct itimerval itv;
    itv.it_interval.tv_sec = PERIOD_SEC;
    itv.it_interval.tv_usec = PERIOD_USEC;
    itv.it_value.tv_sec = PERIOD_SEC;
    itv.it_value.tv_usec = PERIOD_USEC;

    N(setitimer(ITIMER_REAL, &itv, NULL));

    while(!terminate_proc)
      {
        if(rdstate == -1)
            throw PluginError("no response to RD? command");

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
                throw PluginLibraryError("read error");

            if(bytes_read == 0)
                continue;

            wp += bytes_read;
            recvbuf[wp] = 0;

            int rp = 0, msglen, seplen;
            while(msglen = strcspn(recvbuf + rp, "\r\n"),
              seplen = strspn(recvbuf + rp + msglen, "\r\n"))
              {
                recvbuf[rp + msglen] = 0;
                handle_response(recvbuf + rp);
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

    writen(fd, "D0\n", 3);
  }
catch(...)
  {
    writen(fd, "D0\n", 3);
    throw;
  }

RegisterProto<PremaProtocol> proto("prema");

} // unnamed namespace

