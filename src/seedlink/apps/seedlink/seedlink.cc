/*****************************************************************************
 * seedlink.cc
 *
 * SeedLink daemon
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <csignal>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <syslog.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#include "libslink.h"
#include "qtime.h"

#include "conf_ini.h"
#include "conf_xml.h"
#include "confattr.h"
#include "iosystem.h"
#include "monitor.h"
#include "cppstreams.h"
#include "utils.h"
#include "sproc.h"
#include "format.h"
#include "mseed.h"
#include "steim1.h"
#include "steim2.h"
#include "filterimpl.h"
#include "plugin.h"
#include "diag.h"

#define MYVERSION "3.3 (2020.122)"

#ifndef CONFIG_FILE
#define CONFIG_FILE "/home/sysop/config/seedlink.ini"
#endif

#ifndef SEEDLINK_DIR
#define SEEDLINK_DIR "/home/sysop/seedlink"
#endif

#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_LOCAL0
#endif

#ifdef TCPWRAP
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;
#endif

using namespace std;
using namespace CfgParser;
using namespace IOSystem;
using namespace SeedlinkMonitor;
using namespace CPPStreams;
using namespace Utilities;
using namespace SProc;

namespace {

const int         MAX_FILTER_LENGTH = 10000;
const char *const LOG_STREAM        = "LOG";
const char *const INFO_STREAM       = "INF";
const char *const ERROR_STREAM      = "ERR";
const char *const SHELL             = "/bin/bash";

const char *const ident_str = "SeedLink v" MYVERSION;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
const char *const opterr_message = "Try `%s --help' for more information\n";
const char *const help_message = 
    "Usage: %s [options]\n"
    "\n"
    "-v                            Increase verbosity level\n"
    "    --verbosity=LEVEL         Set verbosity level\n"
    "-D, --daemon                  Daemon mode\n"
    "-f, --config-file=FILE        Alternative configuration file\n"
    "-V, --version                 Show version information\n"
    "-h, --help                    Show this help message\n";
#else
const char *const opterr_message = "Try `%s -h' for more information\n";
const char *const help_message =
    "Usage: %s [options]\n"
    "\n"
    "-v             Increase verbosity level\n"
    "-D             Daemon mode\n"
    "-f FILE        Alternative configuration file\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

string network_id;
string organization;
string seed_encoding = "steim1";
int no_of_buffers = 100;
int no_of_blanks = 10;
int no_of_segments = 2;
int segsize = 100;
string seedlink_dir = SEEDLINK_DIR;
int max_connections = 0;
int max_connections_per_ip = 0;
int seq_gap_limit = 0;
int trusted_info_level = 1;
int untrusted_info_level = 1;
int plugin_timeout = 0;
int plugin_start_retry = 0;
int plugin_shutdown_wait = 0;
double backfill_capacity = 0;
int proc_gap_warn_default = 2;
int proc_gap_flush_default = 0;
int proc_gap_reset_default = 0;

int bytespersec = 0;
int tcp_port = 18000;
bool request_log = true;
bool stream_check = false;
string gap_check_pattern;
int gap_treshold = 10000;
bool trusted_window_extraction = false;
bool untrusted_window_extraction = false;
bool trusted_websocket = false;
bool untrusted_websocket = false;

string daemon_name;
int verbosity = 0;
bool daemon_mode = false, daemon_init = false;
rc_ptr<ConnectionManager> connectionManager;

//*****************************************************************************
// Exceptions
//*****************************************************************************

class SeedlinkError: public GenericException
  {
  public:
    SeedlinkError(const string &message):
      GenericException("Seedlink", message) {}
  };

class SeedlinkLibraryError: public SeedlinkError
  {
  public:
    SeedlinkLibraryError(const string &message):
      SeedlinkError(message + " (" + strerror(errno) + ")") {}
  };

class SeedlinkCannotCreateFile: public SeedlinkLibraryError
  {
  public:
    SeedlinkCannotCreateFile(const string &name):
      SeedlinkLibraryError("cannot create file '" + name + "'") {}
  };

class SeedlinkCannotOpenFile: public SeedlinkLibraryError
  {
  public:
    SeedlinkCannotOpenFile(const string &name):
      SeedlinkLibraryError("cannot open file '" + name + "'") {}
  };

class SeedlinkBadFileFormat: public SeedlinkError
  {
  public:
    SeedlinkBadFileFormat(const string &name):
      SeedlinkError("file '" + name + "' has wrong format") {}
  };

class SeedlinkErrorLoadingFilter: public SeedlinkError
  {
  public:
    SeedlinkErrorLoadingFilter(const string &file, const string &filter):
      SeedlinkError("error loading filter " + filter + " from file '" + file + "'") {}
  };

class CfgCannotFindSection: public CfgError
  {
  public:
    CfgCannotFindSection(const string &name, const string &file):
      CfgError("section '" + name + "' doesn't exist in file '" + file + "'") {}
  };

//*****************************************************************************
// LogFunc
//*****************************************************************************

class LogFunc
  {
  public:
    enum { msglen = 200 };
    
    int operator()(int priority, const string &msg)
      {
        if(daemon_init)
          {
            syslog(priority, "%s", msg.c_str());
          }
        else
          {
            int verb = 2;
            
            switch(priority)
              {
              case LOG_EMERG:
              case LOG_ALERT:
              case LOG_CRIT:
              case LOG_ERR:
                verb = -1; break;

              case LOG_WARNING:
              case LOG_NOTICE:
                verb = 0; break;

              case LOG_INFO:
                verb = 1; break;

              case LOG_DEBUG:
                verb = 2;
              }

            if(verbosity < verb)
                return msg.length();

            time_t t = time(NULL);
            char *p = asctime(localtime(&t));
            string::size_type n = msg.length();
            while(n > 0 && (msg[n - 1] == '\n' || msg[n - 1] == '\r')) --n;
            string msgout = string(p, strlen(p) - 1) + " - " + daemon_name +
              ": " + string(msg, 0, n) + '\n';
            write(STDOUT_FILENO, msgout.c_str(), msgout.length());
          }

        return msg.length();
      }
  };

//*****************************************************************************
// Plugin
//*****************************************************************************

class PluginPartner
  {
  public:
    virtual rc_ptr<StreamProcessor> get_plugin_sproc(const string &plugin_name,
      rc_ptr<StreamProcessorSpec> spspec) =0;
    virtual rc_ptr<StreamProcessor> get_station_sproc() =0;
    virtual ~PluginPartner() {};
  };

class Plugin
  {
  private:
    const string cmdline;
    pid_t pid;
    int parent_fd;
    bool child_active;
    bool sigterm_sent;
    bool sigkill_sent;
    bool data_available;
    bool shutdown_requested;
    bool restart_requested;
    Timer read_timer;
    Timer start_retry_timer;
    Timer shutdown_timer;
    enum { ReadInit, ReadHeader, ReadData } read_state;
    PluginPacketHeader header_buf;
    char data_buf[PLUGIN_MAX_DATA_BYTES];
    int data_bytes;
    ssize_t nread;
    size_t nleft;
    char *ptr;
    bool raw_ignored;
    set<string> missing_inputs;
    rc_ptr<StreamProcessorSpec> spspec;
     
    bool check_child();
    bool read_helper();

    void kill_plugin()
      {
        if(pid > 0)
          {
            kill(pid, SIGKILL);
            sigkill_sent = true;
          }
      }

  public:
    const string name;
  
    Plugin(const string name_init, const string &cmdline_init, int read_timeout,
      int start_retry, int shutdown_wait, rc_ptr<StreamProcessorSpec> spspec_init):
      cmdline(cmdline_init), pid(0), parent_fd(-1), child_active(true),
      sigterm_sent(false), sigkill_sent(false), data_available(false),
      shutdown_requested(false), restart_requested(false), read_timer(read_timeout, 0),
      start_retry_timer(start_retry, 0), shutdown_timer(shutdown_wait, 0),
      read_state(ReadInit), data_bytes(0), nread(0), nleft(0), ptr(NULL),
      raw_ignored(false), spspec(spspec_init), name(name_init) {}

    ~Plugin()
      {
        if(parent_fd >= 0) close(parent_fd);
      }
        
    void start();
    bool read(PluginPacketHeader &head, void *data);
    rc_ptr<Input> get_input(const string &input_name, PluginPartner &station);
    
    void shutdown(bool restart = false)
      {
        if(pid > 0)
          {
            kill(pid, SIGTERM);
            sigterm_sent = true;
          }

        shutdown_timer.reset();
        shutdown_requested = true;
        restart_requested = restart;
      }

    bool running()
      {
        return (pid > 0 || child_active);
      }

    bool active()
      {
        return child_active;
      }

    int fd()
      {
        return parent_fd;
      }
  };

void Plugin::start()
  {
    int pipe_fd[2];

    if(parent_fd >= 0) close(parent_fd);
    N(pipe(pipe_fd));

    if(pipe_fd[0] >= FD_SETSIZE)
      {
        logs(LOG_ERR) << "cannot start plugin " + name + 
          ": FD_SETSIZE exceeded (" << pipe_fd[0] << ")" << endl;

        close(pipe_fd[0]);
        close(pipe_fd[1]);
      }

    N(pid = fork());

    if(pid) 
      {
        close(pipe_fd[1]);
        parent_fd = pipe_fd[0];
        N(fcntl(parent_fd, F_SETFD, FD_CLOEXEC));
        N(fcntl(parent_fd, F_SETFL, O_NONBLOCK));
        
        read_timer.reset();
        read_state = ReadInit;
        sigterm_sent = false;
        sigkill_sent = false;
        data_available = false;
        shutdown_requested = false;
        return;
      }

    close(pipe_fd[0]);
    
    if(pipe_fd[1] != PLUGIN_FD)
      {
        N(dup2(pipe_fd[1], PLUGIN_FD));
        close(pipe_fd[1]);
      }

    logs(LOG_INFO) << "[" << name << "] starting shell" << endl;
    
    execl(SHELL, SHELL, "-c", (cmdline + " " + name).c_str(), NULL);

    logs(LOG_ERR) << string() + "cannot execute shell '" + SHELL + "' "
      "(" + strerror(errno) + ")" << endl;
    exit(0);
  }

bool Plugin::check_child()
  {
    int status, completed;

    if(pid <= 0) return false;

    if((completed = waitpid(pid, &status, WNOHANG)) < 0)
      {
        logs(LOG_ERR) << "waitpid: " << strerror(errno) << endl;
        pid = 0;
        return false;
      }

    if(!completed) return true;

    if(WIFSIGNALED(status))
      {
        logs(LOG_WARNING) << "[" << name << "] terminated on signal " <<
          WTERMSIG(status) << endl;
      }
    else if(WIFEXITED(status) && WEXITSTATUS(status) != 0)
      {
        logs(LOG_WARNING) << "[" << name << "] terminated with error status " <<
          WEXITSTATUS(status) << endl;
      }
    else
      {
        logs(LOG_NOTICE) << "[" << name << "] terminated" << endl;
      }

    pid = 0;
    return false;
  }
    
bool Plugin::read_helper()
  {
    int nread;
    
    child_active = false;
    
    if(read_state == ReadInit)
      {
        nleft = sizeof(PluginPacketHeader);
        ptr = reinterpret_cast<char *>(&header_buf);
        read_state = ReadHeader;
      }
    
    nread = ::read(parent_fd, ptr, nleft);

    if(nread == 0)
      {
        if(data_available && !shutdown_requested && !sigkill_sent)
            logs(LOG_WARNING) << "[" << name << "] unexpected eof" << endl;

        return false;
      }

    if(nread < 0)
      {
        if(errno == EAGAIN)
          {
            if(!sigkill_sent)
                child_active = true;
          }
        else
          {
            logs(LOG_WARNING) << "[" << name << "] read error (" <<
              strerror(errno) << ")" << endl;
          }

        return false;
      }

    if(!data_available)
      {
        logs(LOG_INFO) << "[" << name << "] data is available" << endl;
        data_available = true;
      }

    nleft -= nread;
    ptr += nread;

    if(read_state == ReadHeader && nleft == 0)
      {
        if(header_buf.packtype == PluginRawDataTimePacket ||
          header_buf.packtype == PluginRawDataPacket)
            data_bytes = header_buf.data_size << 2;
        else if(header_buf.packtype == PluginLogPacket ||
          header_buf.packtype == PluginMSEEDPacket)
            data_bytes = header_buf.data_size;
        else
            data_bytes = 0;

        if(data_bytes > PLUGIN_MAX_DATA_BYTES || data_bytes < 0)
          {
            logs(LOG_ERR) << "[" << name << "] invalid data size (" <<
              header_buf.data_size << ")" << endl;
            return false;
          }

        nleft = data_bytes;
        ptr = data_buf;
        read_state = ReadData;
      }

    if(read_state == ReadData && nleft == 0)
      {
        read_state = ReadInit;
      }

    child_active = true;
    return true;
  }

bool Plugin::read(PluginPacketHeader &head, void *data)
  {
    if(!child_active)
      {
        if(pid <= 0)
          {
            if(restart_requested || start_retry_timer.expired())
              {
                restart_requested = false;
                start();
              }
          }
        else if(!check_child())
          {
            start_retry_timer.reset();
          }

        if(pid <= 0) return false;
      }
    
    while(read_helper())
      {
        read_timer.reset();
    
        if(nleft == 0)
          {
            head = header_buf;
            memcpy(data, data_buf, data_bytes);
            return true;
          }
      }
    
    if(shutdown_requested)
      {
        if(!sigkill_sent && pid > 0 && shutdown_timer.expired())
          {
            logs(LOG_WARNING) << "[" << name << "] shutdown time expired" << endl;
            kill_plugin();
          }

        return false;
      }

    if(!child_active)
      {
        shutdown();
      }
    else if(read_timer.expired())
      {
        logs(LOG_WARNING) << "[" << name << "] timeout" << endl;
        shutdown();
      }
    
    return false;
  }

rc_ptr<Input> Plugin::get_input(const string &input_name, PluginPartner &partner)
  {
    if(raw_ignored || missing_inputs.find(input_name) != missing_inputs.end())
        return NULL;

    rc_ptr<Input> input;

    if(spspec != NULL)
      {
        rc_ptr<StreamProcessor> sproc = partner.get_plugin_sproc(name, spspec);
        input = sproc->get_input(input_name);
      }

    if(input == NULL)
      {
        rc_ptr<StreamProcessor> sproc = partner.get_station_sproc();

        if(sproc == NULL)
          {
            if(spspec == NULL && !raw_ignored)
              {
                logs(LOG_WARNING) << name << " raw data ignored" << endl;
                raw_ignored = true;
              }

            return NULL;
          }

        input = sproc->get_input(input_name);
      }

    if(input == NULL)
      {
        logs(LOG_WARNING) << name << " channel " << input_name <<
          " ignored" << endl;
        missing_inputs.insert(input_name);
        return NULL;
      }

    return input;
  }

list<rc_ptr<Plugin> > plugins;


//*****************************************************************************
// Backfilling MiniSEED
//*****************************************************************************

struct MSEEDPacket
  {
  MSEEDPacket() {}
  void set_data(const void *rec, const INT_TIME &stime_init, const INT_TIME &etime_init)
    {
      memcpy(data, rec, (1 << MSEED_RECLEN));
      stime = stime_init;
      etime = etime_init;
    }

  INT_TIME stime;
  INT_TIME etime;
  char data[1 << MSEED_RECLEN];
  };


//*****************************************************************************
// Station
//*****************************************************************************

bool lt(const INT_TIME &it1, const INT_TIME &it2)
  {
    if(it1.year < it2.year) return true;
    if(it1.year > it2.year) return false;
    if(it1.second < it2.second) return true;
    if(it1.second > it2.second) return false;
    if(it1.usec < it2.usec) return true;
    if(it1.usec > it2.usec) return false;
    return false;
  }

class Station: public StreamProcessor::InputVisitor, private PluginPartner
  {
  private:
    typedef SProc::Backfilling<MSEEDPacket> MSEEDBackfilling;

    rc_ptr<BufferStore> bufs;
    rc_ptr<Format> log_format;
    rc_ptr<StreamProcessorSpec> spspec;
    rc_ptr<StreamProcessor> sproc;
    string station_name;
    string network_id;
    string seed_encoding;
    int proc_gap_warn;
    int proc_gap_flush;
    int proc_gap_reset;
    int minbufs;
    double backfill_capacity;
    map<string, rc_ptr<StreamProcessor> > plugin_sproc;
    map<string, rc_ptr<MSEEDBackfilling> > mseed_backfilling;

    INT_TIME pt_to_it(const ptime &pt);
    rc_ptr<StreamProcessor> get_plugin_sproc(const string &plugin_name,
      rc_ptr<StreamProcessorSpec> spspec);
    rc_ptr<StreamProcessor> get_station_sproc();
    void visit(const string &channel_name, rc_ptr<Input> input, void *data);

    rc_ptr<MSEEDBackfilling> get_mseed_backfilling(const char *id)
      {
        // Insert new backfilling structure on demand
        pair< map<string, rc_ptr<MSEEDBackfilling> >::iterator, bool> itp;
        itp = mseed_backfilling.insert(make_pair(id, rc_ptr<MSEEDBackfilling>()));
        if(itp.first->second == NULL)
          {
            itp.first->second = new MSEEDBackfilling(backfill_capacity);
          }
        return itp.first->second;
      }

  public:
    const string name;

    Station(const string &name_init, rc_ptr<BufferStore> bufs_init,
      rc_ptr<Format> log_format_init, rc_ptr<StreamProcessorSpec> spspec_init,
      const string &station_name_init, const string &network_id_init,
      const string &seed_encoding_init, int proc_gap_warn_init,
      int proc_gap_flush_init, int proc_gap_reset_init, double backfill_capacity_init):
      bufs(bufs_init), log_format(log_format_init), spspec(spspec_init),
      station_name(station_name_init), network_id(network_id_init),
      seed_encoding(seed_encoding_init), proc_gap_warn(proc_gap_warn_init),
      proc_gap_flush(proc_gap_flush_init), proc_gap_reset(proc_gap_reset_init), minbufs(0),
      backfill_capacity(backfill_capacity_init), name(name_init) {}

    Station(const string &name_init):
      bufs(NULL), log_format(NULL), proc_gap_warn(0), proc_gap_flush(0),
      proc_gap_reset(0), minbufs(0), backfill_capacity(-1.0), name(name_init) {}

    virtual ~Station() {}

    void send_mseed(const void *rec);
    void send_log(const ptime &pt, const char *msg, int msglen);
    void send_raw_with_time(rc_ptr<Plugin> plugin, const string &input_name,
      const ptime &pt, int usec_correction, int timing_quality,
      const int32_t *data, int number_of_samples);
    void send_raw(rc_ptr<Plugin> plugin, const string &input_name,
      const int32_t *data, int number_of_samples);
    void send_gap(rc_ptr<Plugin> plugin, const string &input_name,
      int usec_correction, int timing_quality, int number_of_samples);
    void send_flush(rc_ptr<Plugin> plugin, const string &input_name);

    void commit_mseed(const void *rec);
    void commit_data(const string &input_name, rc_ptr<Input> input,
                     const INT_TIME &it, int usec_correction, int timing_quality,
                     const int32_t *data, int number_of_samples);
    void commit_packet(const string &input_name, rc_ptr<Input> input,
                       const InputBackfilling::PacketPtr &pkt);

    void sync_buffer(const string &input_name, rc_ptr<Input> input);
    void sync_buffer(const char *name, rc_ptr<MSEEDBackfilling> bf);

    void flush()
      {
        // Visit all inputs and flush the backfilling buffer
        map<string, rc_ptr<StreamProcessor> >::iterator it1;
        for(it1 = plugin_sproc.begin(); it1 != plugin_sproc.end(); ++it1)
          {
            rc_ptr<StreamProcessor> sproc = it1->second;
            sproc->visit_inputs(*this, NULL);
            sproc->flush();
          }

        // flush all MiniSEED buffers
        map<string, rc_ptr<MSEEDBackfilling> >::iterator it;
        for(it = mseed_backfilling.begin(); it != mseed_backfilling.end(); ++it)
          {
            rc_ptr<MSEEDBackfilling> bf = it->second;
            MSEEDBackfilling::PacketList::iterator pit;
            for(pit = bf->buffer.begin(); pit != bf->buffer.end(); ++pit)
              {
                commit_mseed((*pit)->data);
              }
          }
      }
  };

INT_TIME Station::pt_to_it(const ptime &pt)
  {
    EXT_TIME et;
    et.year = pt.year;
    et.doy = pt.yday;
    et.hour = pt.hour;
    et.minute = pt.minute;
    et.second = pt.second;
    et.usec = pt.usec;
    dy_to_mdy(et.doy, et.year, &et.month, &et.day);
    return ext_to_int(et);
  }

rc_ptr<StreamProcessor> Station::get_plugin_sproc(const string &plugin_name,
  rc_ptr<StreamProcessorSpec> spspec)
  {
    map<string, rc_ptr<StreamProcessor> >::iterator it = plugin_sproc.find(plugin_name);
    if(it != plugin_sproc.end())
        return it->second;

    minbufs += 3 * spspec->number_of_streams();
    if(bufs->size() < minbufs)
      {
        bufs->enlarge(minbufs);
        logs(LOG_INFO) << "increased the number of buffers for station " <<
          network_id << "." << station_name << " to " << minbufs << endl;
      }

    rc_ptr<StreamProcessor> sproc;
    if(!strcasecmp(seed_encoding.c_str(), "steim1"))
      {
        sproc = spspec->instance(make_encoder_spec<Steim1Encoder, MSEEDFormat>(bufs,
          MSEED_RECLEN, Steim1Packet, station_name, network_id));
      }
    else
      {
        sproc = spspec->instance(make_encoder_spec<Steim2Encoder, MSEEDFormat>(bufs,
          MSEED_RECLEN, Steim2Packet, station_name, network_id));
      }

    plugin_sproc.insert(make_pair(plugin_name, sproc));

    // initialize inputs regarding backfilling
    sproc->visit_inputs(*this, (void*)0x01);

    return sproc;
  }

rc_ptr<StreamProcessor> Station::get_station_sproc()
  {
    if(sproc != NULL)
        return sproc;

    if(spspec == NULL)
        return NULL;

    minbufs += 3 * spspec->number_of_streams();
    if(bufs->size() < minbufs)
      {
        bufs->enlarge(minbufs);
        logs(LOG_INFO) << "increased the number of buffers for station " <<
          network_id << "." << station_name << " to " << minbufs << endl;
      }

    if(!strcasecmp(seed_encoding.c_str(), "steim1"))
      {
        sproc = spspec->instance(make_encoder_spec<Steim1Encoder, MSEEDFormat>(bufs,
          MSEED_RECLEN, Steim1Packet, station_name, network_id));
      }
    else
      {
        sproc = spspec->instance(make_encoder_spec<Steim2Encoder, MSEEDFormat>(bufs,
          MSEED_RECLEN, Steim2Packet, station_name, network_id));
      }

    // initialize inputs regarding backfilling
    sproc->visit_inputs(*this, (void*)0x01);

    return sproc;
  }

void Station::visit(const string &channel_name, rc_ptr<Input> input, void *data)
  {
    // No data -> flush
    if(data == NULL)
      {
        // Send all pending packets even if there are gaps. This is only
        // called during Station::flush which is called after a shutdown
        // request.
        InputBackfilling::PacketList::iterator it;
        for(it = input->backfilling.buffer.begin();
            it != input->backfilling.buffer.end(); ++it)
          {
            commit_packet(channel_name, input, *it);
          }
      }
    // Init inputs
    else if (data == (void*)0x01)
      {
        // Set backfill capacity
        input->backfilling.capacity = backfill_capacity;
      }
  }

void Station::send_mseed(const void *rec)
  {
    if(bufs == NULL) return;

    if(backfill_capacity > 0)
      {
        const sl_fsdh_s* fsdh = static_cast<const sl_fsdh_s *>(rec);

	if(packet_type(fsdh, MAX_HEADER_LEN) != SLDATA)
          {
            commit_mseed(rec);
            return;
          }

        INT_TIME stime = packet_begin_time(fsdh);
        INT_TIME etime = packet_end_time(fsdh);
        char id[6];
        id[5] = '\0';
        memcpy(id, fsdh->location, 2);
        memcpy(id+2, fsdh->channel, 3);
        rc_ptr<MSEEDBackfilling> backfilling = get_mseed_backfilling(id);

        if(backfilling->comitted)
          {
            double gap = tdiff(stime, backfilling->last_commit)*1E-6;
            // A gap larger than one sample?
            if(gap*packet_sample_rate(fsdh) >= 1)
              {
                MSEEDBackfilling::PacketPtr pkt = new MSEEDBackfilling::Packet;
                pkt->set_data(rec, stime, etime);

                // Insert packet into the buffer (sorted by start time)
                backfilling->insert(pkt);
                logs(LOG_DEBUG) << name << " channel " << id
                                << " data queued (currently " << backfilling->buffer.size()
                                << " pkts) due to a gap of " << gap << "s"
                                << endl;
                sync_buffer(id, backfilling);
                return;
              }
            else
              {
                // Record before the last commit? Just flush it, its too old
                if ( lt(etime, backfilling->last_commit) )
                  {
                    logs(LOG_DEBUG) << name << " channel " << id
                                    << " sent out-of-order record, buffer too small?"
                                    << endl;
                    commit_mseed(rec);
                    return;
                  }

                backfilling->last_commit = etime;
                backfilling->comitted = true;
                commit_mseed(rec);

                sync_buffer(id, backfilling);
              }
          }
        else
          {
            // Instead of charging the backfill buffer we are sending the
            // first packet. Both options would work with all their advantages/
            // disadvantages. This could be made configurable later.
            /*
            MSEEDBackfilling::PacketPtr pkt = new MSEEDBackfilling::Packet;
            pkt->set_data(rec, stime, etime);

            // Insert packet into the buffer (sorted by start time)
            backfilling->insert(pkt);
            logs(LOG_DEBUG) << name << " channel " << id
                                    << " charging backfilling buffer (currently "
                                    << backfilling->buffer.size() << " pkts)"
                                    << endl;
            sync_buffer(id, backfilling);
            */

            // Release the packet
            backfilling->last_commit = etime;
            backfilling->comitted = true;
            commit_mseed(rec);
          }
        return;
      }

    commit_mseed(rec);
  }

void Station::commit_mseed(const void *rec)
  {
    Buffer* buf = bufs->get_buffer();
    memcpy(buf->data(), rec, (1 << MSEED_RECLEN));
    bufs->queue_buffer(buf);
  }

void Station::send_log(const ptime &pt, const char *msg, int msglen)
  {
    if(log_format == NULL) return;
    
    Packet<char> pckt = log_format->get_packet<char>(pt_to_it(pt), 0, -1);
    int msglen1 = min(msglen, PLUGIN_MAX_MSG_SIZE);
    
    memset(pckt.data, 0, pckt.datalen);
    memcpy(pckt.data, msg, msglen1);
    log_format->queue_packet(pckt, msglen1, 0);
  }

void Station::send_raw_with_time(rc_ptr<Plugin> plugin, const string &input_name,
  const ptime &pt, int usec_correction, int timing_quality, const int32_t *data,
  int number_of_samples)
  {
    rc_ptr<Input> input;
    if((input = plugin->get_input(input_name, *this)) == NULL)
        return;

    INT_TIME stime = pt_to_it(pt);

    if(input->backfilling.is_enabled())
      {
        if(input->backfilling.comitted)
          {
            double gap = tdiff(stime, input->backfilling.last_commit)*1E-6;
            // A gap larger than one sample?
            if(gap*input->clk.freqn >= input->clk.freqd)
              {
                input->backfilling.current = new InputBackfilling::Packet;
                input->backfilling.current->set_data(data, number_of_samples, stime,
                                                     usec_correction, timing_quality,
                                                     input->clk.freqn, input->clk.freqd);

                // Insert packet into the buffer (sorted by start time)
                input->backfilling.insert(input->backfilling.current);
                logs(LOG_DEBUG) << name << " channel " << input_name
                                << " data queued (currently " << input->backfilling.buffer.size()
                                << " pkts) due to a gap of " << gap << "s"
                                << endl;
                sync_buffer(input_name, input);
                // And done for now
                return;
              }
            else
              {
                // Otherwise we continue as without backfilling
                input->backfilling.current = NULL;

                INT_TIME etime;
                etime = add_dtime(stime, 1000000 * (double(number_of_samples) * double(input->clk.freqd) / double(input->clk.freqn)));

                // Record before the last commit? Just flush it, its too old
                if ( lt(etime, input->backfilling.last_commit) )
                  {
                    logs(LOG_DEBUG) << name << " channel " << input_name
                                    << " sent out-of-order record, buffer too small?"
                                    << endl;
                    commit_data(input_name, input, stime, usec_correction,
                                timing_quality, data, number_of_samples);
                    return;
                  }

                input->backfilling.last_commit = etime;
                input->backfilling.comitted = true;
                commit_data(input_name, input, stime, usec_correction,
                            timing_quality, data, number_of_samples);

                sync_buffer(input_name, input);
              }
          }
        else
          {
            // Instead of charging the backfill buffer we are sending the
            // first packet. Both options would work with all their advantages/
            // disadvantages.
            /*
            input->backfilling.current = new InputBackfilling::Packet;
            input->backfilling.current->set_data(data, number_of_samples, stime,
                                                 usec_correction, timing_quality,
                                                 input->clk.freqn, input->clk.freqd);

            // Insert packet into the buffer (sorted by start time)
            input->backfilling.insert(input->backfilling.current);
            logs(LOG_DEBUG) << name << " channel " << input_name
                                    << " charging backfilling buffer (currently "
                                    << input->backfilling.buffer.size() << " pkts)"
                                    << endl;
            sync_buffer(input_name, input);
            */

            // Release the packet
            INT_TIME etime;
            etime = add_dtime(stime, 1000000 * (double(number_of_samples) * double(input->clk.freqd) / double(input->clk.freqn)));

            input->backfilling.last_commit = etime;
            input->backfilling.comitted = true;
            commit_data(input_name, input, stime, usec_correction,
                        timing_quality, data, number_of_samples);
          }
        return;
      }

    commit_data(input_name, input, stime, usec_correction,
                timing_quality, data, number_of_samples);
  }

void Station::send_raw(rc_ptr<Plugin> plugin, const string &input_name, const int32_t *data,
  int number_of_samples)
  {
    rc_ptr<Input> input;
    if((input = plugin->get_input(input_name, *this)) == NULL)
        return;

    if(input->backfilling.current != NULL)
      {
        input->backfilling.current->append_data(data, number_of_samples,
                                                input->clk.freqn, input->clk.freqd);
        sync_buffer(input_name, input);
        return;
      }

    input->send_data(data, number_of_samples);

    // Update last commit time
    if(input->backfilling.is_enabled())
        input->backfilling.last_commit =
            add_dtime(input->backfilling.last_commit, 1000000 * (double(number_of_samples) * double(input->clk.freqd) / double(input->clk.freqn)));
  }

void Station::send_gap(rc_ptr<Plugin> plugin, const string &input_name, int usec_correction,
  int timing_quality, int number_of_samples)
  {
    rc_ptr<Input> input;
    if((input = plugin->get_input(input_name, *this)) == NULL)
        return;

    input->add_ticks(number_of_samples, usec_correction, timing_quality);

    if(proc_gap_warn != 0 &&
      (input->time_gap() <= -proc_gap_warn || input->time_gap() >= proc_gap_warn))
        logs(LOG_WARNING) << name << " : " << input_name << " time gap " <<
          setprecision(6) << input->time_gap() / 1000000.0 << " seconds " <<
          "(explicit)" << endl;

    if(proc_gap_reset != 0 &&
      (input->time_gap() <= -proc_gap_reset || input->time_gap() >= proc_gap_reset))
        input->reset();

    else if(proc_gap_flush != 0 &&
      (input->time_gap() <= -proc_gap_flush || input->time_gap() >= proc_gap_flush))
        input->flush();
  }

void Station::send_flush(rc_ptr<Plugin> plugin, const string &input_name)
  {
    rc_ptr<Input> input;
    if((input = plugin->get_input(input_name, *this)) == NULL)
        return;

    if(input->backfilling.current!=NULL)
        input->backfilling.current->flush = true;
    else
        input->flush();
  }


void Station::commit_data(const string &input_name, rc_ptr<Input> input,
                          const INT_TIME &it, int usec_correction,
                          int timing_quality,
                          const int32_t *data, int number_of_samples)
  {
    input->set_time(it, usec_correction, timing_quality);

    if(proc_gap_warn != 0 &&
      (input->time_gap() <= -proc_gap_warn || input->time_gap() >= proc_gap_warn))
        logs(LOG_WARNING) << name << " : " << input_name << " time gap " <<
          setprecision(6) << input->time_gap() / 1000000.0 << " seconds " <<
          "(detected)" << endl;

    if(proc_gap_reset != 0 &&
      (input->time_gap() <= -proc_gap_reset || input->time_gap() >= proc_gap_reset))
        input->reset();

    else if(proc_gap_flush != 0 &&
      (input->time_gap() <= -proc_gap_flush || input->time_gap() >= proc_gap_flush))
        input->flush();

    input->send_data(data, number_of_samples);
  }

void Station::commit_packet(const string &input_name, rc_ptr<Input> input,
                            const InputBackfilling::PacketPtr &pkt)
  {
    input->backfilling.last_commit = pkt->etime;
	input->backfilling.comitted = true;
    commit_data(input_name, input, pkt->stime, pkt->usec_correction,
                pkt->timing_quality, pkt->data.data(), pkt->data.size());

    if(pkt->flush)
        input->flush();
  }


void Station::sync_buffer(const string &input_name, rc_ptr<Input> input)
  {
    // Cut the buffer to its capacity
    while(!input->backfilling.buffer.empty() &&
          tdiff(input->backfilling.buffer.back()->etime, input->backfilling.buffer.front()->stime)*1E-6 >= input->backfilling.capacity)
      {
        // Commit first packet
        InputBackfilling::PacketPtr &pkt = input->backfilling.buffer.front();
        commit_packet(input_name, input, pkt);
        input->backfilling.buffer.pop_front();

        logs(LOG_DEBUG) << name << " channel " << input_name
                        << " trimming buffer to its capacity (still "
                        << input->backfilling.buffer.size() << " pkts)"
                        << endl;
      }

    if(!input->backfilling.comitted) return;

    // Check if this packet filled a gap to the buffer
    while(!input->backfilling.buffer.empty())
      {
        InputBackfilling::PacketPtr &bpkt = input->backfilling.buffer.front();
        double gap = tdiff(bpkt->stime, input->backfilling.last_commit)*1E-6;
        // A gap larger than one sample -> break flushing
        if(gap*input->clk.freqn >= input->clk.freqd) break;

        // commit data and check the next packet
        commit_packet(input_name, input, bpkt);
        input->backfilling.buffer.pop_front();

        logs(LOG_DEBUG) << name << " channel " << input_name
                        << " flushed continuous records from buffer (still "
                        << input->backfilling.buffer.size() << " pkts)"
                        << endl;
      }
  }

void Station::sync_buffer(const char* cha, rc_ptr<MSEEDBackfilling> bf)
  {
    double used_capacity;

    // Cut the buffer to its capacity
    while(!bf->buffer.empty() &&
          (used_capacity = tdiff(bf->buffer.back()->etime, bf->buffer.front()->stime)*1E-6) >= bf->capacity)
      {
        // Commit first packet
        MSEEDBackfilling::PacketPtr &pkt = bf->buffer.front();
        commit_mseed(pkt->data);
        bf->last_commit = pkt->etime;
        bf->comitted = true;
        bf->buffer.pop_front();

        logs(LOG_DEBUG) << name << " channel " << cha
                        << " trimming buffer to its capacity ("
                        << used_capacity << " > " << bf->capacity << ", still "
                        << bf->buffer.size() << " pkts)"
                        << endl;
      }

    if(!bf->comitted) return;

    // Check if this packet filled a gap to the buffer
    while(!bf->buffer.empty())
      {
        MSEEDBackfilling::PacketPtr &bpkt = bf->buffer.front();
        const sl_fsdh_s* fsdh = reinterpret_cast<const sl_fsdh_s *>(bpkt->data);
        double gap = tdiff(bpkt->stime, bf->last_commit)*1E-6;
        // A gap larger than one sample -> break flushing
        if(gap*packet_sample_rate(fsdh) >= 1) break;

        // commit data and check the next packet
        commit_mseed(bpkt->data);
        bf->last_commit = bpkt->etime;
        bf->buffer.pop_front();

        logs(LOG_DEBUG) << name << " channel " << cha
                        << " flushed continuous records from buffer (still "
                        << bf->buffer.size() << " pkts)"
                        << endl;
      }
  }


map<string, rc_ptr<Station> > stations;

//*****************************************************************************
// PluginDef -- definition of element "plugin"
//*****************************************************************************

class PluginDef: public CfgElement
  {
  private:
    const map<string, rc_ptr<StreamProcessorSpec> > sproc_defs;
    string plugin_name;
    string cmd;
    int timeout;
    int start_retry;
    int shutdown_wait;
    string sproc_name;
    set<string> plugins_defined;
    
  public:
    PluginDef(const string &name, const map<string, rc_ptr<StreamProcessorSpec> > &sproc_defs_init):
      CfgElement(name), sproc_defs(sproc_defs_init) {}
    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog, const string &name);
    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> PluginDef::start_attributes(ostream &cfglog,
  const string &name)
  {
    if(plugins_defined.find(name) != plugins_defined.end())
      {
        cfglog << "plugin " << name << " is already defined" << endl;
        return NULL;
      }

    plugin_name = name;
    timeout = plugin_timeout;
    start_retry = plugin_start_retry;
    shutdown_wait = plugin_shutdown_wait;
    sproc_name = "";
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("cmd", cmd));
    atts->add_item(IntAttribute("timeout", timeout, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("start_retry", start_retry, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("shutdown_wait", shutdown_wait, 0, IntAttribute::lower_bound));
    atts->add_item(StringAttribute("proc", sproc_name));
    return atts;
  }

void PluginDef::end_attributes(ostream &cfglog)
  {
    if(cmd.length() == 0)
      {
        cfglog << "Command was not specified for plugin " << plugin_name << endl;
        return;
      }
    
    rc_ptr<StreamProcessorSpec> spspec;
    if(sproc_name.length() != 0 &&
      (spspec = get_object(sproc_defs, sproc_name)) == NULL)
      {
        cfglog << "stream processor '" << sproc_name << "' is not defined" << endl;
        return;
      }

    plugins.push_back(new Plugin(plugin_name, cmd, timeout, start_retry, shutdown_wait, spspec));
    plugins_defined.insert(plugin_name);
  }

//*****************************************************************************
// StationDef -- definition of element "station"
//*****************************************************************************

class StationDef: public CfgElement
  {
  private:
    const map<string, rc_ptr<StreamProcessorSpec> > sproc_defs;
    const IPACL ip_trusted;
    const IPACL default_ip_access;
    string station_key;
    string station_name;
    string network_id;
    string description;
    string sproc_name;
    string seed_encoding;
    string gap_check_pattern;
    int gap_treshold;
    int no_of_buffers;
    int no_of_blanks;
    int no_of_segments;
    int segsize;
    int proc_gap_warn;
    int proc_gap_flush;
    int proc_gap_reset;
    double backfill_capacity;
    bool request_log;
    bool stream_check;
    list<string> ip_access_str;
    
  public:
    StationDef(const string &name, const map<string, rc_ptr<StreamProcessorSpec> > &sproc_defs_init,
      const IPACL &ip_trusted_init, const IPACL &ip_access_init):
      CfgElement(name), sproc_defs(sproc_defs_init),
      ip_trusted(ip_trusted_init), default_ip_access(ip_access_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog, const string &name);
    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> StationDef::start_attributes(ostream &cfglog,
  const string &name)
  {
    station_key = name;
    station_name = name;
    description = "";
    sproc_name = "";
    seed_encoding = ::seed_encoding;
    gap_check_pattern = ::gap_check_pattern;
    gap_treshold = ::gap_treshold;
    network_id = ::network_id;
    no_of_buffers = ::no_of_buffers;
    no_of_blanks = ::no_of_blanks;
    no_of_segments = ::no_of_segments;
    segsize = ::segsize;
    request_log = ::request_log;
    proc_gap_warn = ::proc_gap_warn_default;
    proc_gap_flush = ::proc_gap_flush_default;
    proc_gap_reset = ::proc_gap_reset_default;
    backfill_capacity = ::backfill_capacity;
    stream_check = ::stream_check;
    ip_access_str.clear();
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("name", station_name));
    atts->add_item(StringAttribute("network", network_id));
    atts->add_item(StringAttribute("description", description));
    atts->add_item(StringAttribute("proc", sproc_name));
    atts->add_item(StringAttribute("encoding", seed_encoding));
    atts->add_item(StringAttribute("gap_check_pattern", gap_check_pattern));
    atts->add_item(IntAttribute("gap_treshold", gap_treshold, 100, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("buffers", no_of_buffers, 10, 100000));
    atts->add_item(IntAttribute("blanks", no_of_blanks, 0, 100));
    atts->add_item(IntAttribute("segments", no_of_segments, 2, 1000));
    atts->add_item(IntAttribute("segsize", segsize, 10, 100000));
    atts->add_item(IntAttribute("proc_gap_warn", proc_gap_warn, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("proc_gap_flush", proc_gap_flush, 0, 100, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("proc_gap_reset", proc_gap_reset, 0, 100, IntAttribute::lower_bound));
    atts->add_item(FloatAttribute("backfill_buffer", backfill_capacity,  0, 86400, FloatAttribute::upper_bound));
    atts->add_item(BoolAttribute("request_log", request_log, "enabled", "disabled"));
    atts->add_item(BoolAttribute("stream_check", stream_check, "enabled", "disabled"));
    atts->add_item(StringListAttribute("access", ip_access_str, ", "));
    return atts;
  }

void StationDef::end_attributes(ostream &cfglog)
try
  {
    if(has_object(stations, station_key))
      {
        cfglog << "station " << station_key << " is already defined" << endl;
        return;
      }

    if(seed_encoding != "steim1" and seed_encoding != "steim2")
      {
        cfglog << "unsupported encoding '" << seed_encoding << "'" << endl;
        return;
      }

    const string station_dir = seedlink_dir + "/" + station_key;
    const string segment_dir = station_dir + "/segments";
    const string test_file1 = station_dir + "/test";
    const string test_file2 = segment_dir + "/test";

    mkdir(seedlink_dir.c_str(), 0755);
    mkdir(station_dir.c_str(), 0755);
    mkdir(segment_dir.c_str(), 0755);
    
    int fd;
    if((fd = creat(test_file1.c_str(), 0644)) < 0)
        throw SeedlinkCannotCreateFile(test_file1);
    
    close(fd);
    unlink(test_file1.c_str());
    if((fd = creat(test_file2.c_str(), 0644)) < 0)
        throw SeedlinkCannotCreateFile(test_file2);
    
    close(fd);
    unlink(test_file2.c_str());
    
    IPACL ip_access;
    if(ip_access_str.empty())
      {
        ip_access = default_ip_access;
      }
    else
      {
        list<string>::iterator p;
        for(p = ip_access_str.begin(); p != ip_access_str.end(); ++p)
          {
            if(!ip_access.add(*p))
              {
                cfglog << "invalid IP address/mask: " << *p << endl;
                return;
              }
          }
      }
                
    rc_ptr<StreamProcessorSpec> spspec;
    if(sproc_name.length() != 0 &&
      (spspec = get_object(sproc_defs, sproc_name)) == NULL)
      {
        cfglog << "stream processor '" << sproc_name << "' is not defined" << endl;
        return;
      }

    if(connectionManager == NULL)
      {
        rc_ptr<MasterMonitor> monitor = make_master_monitor(MSEED_RECLEN,
          INFO_STREAM, ERROR_STREAM, ident_str, organization, ip_trusted);
        connectionManager = make_conn_manager(daemon_name, ident_str,
          ::network_id, monitor, ::request_log, max_connections,
          max_connections_per_ip, trusted_info_level, untrusted_info_level,
          trusted_window_extraction, untrusted_window_extraction,
          trusted_websocket, untrusted_websocket, bytespersec, 0, 100000);
      }
    
    rc_ptr<BufferStore> bufs = connectionManager->register_station(station_key,
      station_name, network_id, description, request_log,
      seedlink_dir + "/" + station_key, no_of_buffers, no_of_blanks, segsize,
      no_of_segments, seq_gap_limit, stream_check, gap_check_pattern,
      gap_treshold, ip_access);

    if(bufs == NULL)
      {
        cfglog << "failed to register " << station_key << "(" << network_id <<
          "_" << station_name << " is already in use)" << endl;

        return;
      }

    rc_ptr<Format> log_format = new MSEEDFormat(bufs, MSEED_RECLEN, LogPacket,
      station_name, network_id, "", LOG_STREAM, 0, 0);

    insert_object(stations, new Station(station_key, bufs, log_format, spspec,
      station_name, network_id, seed_encoding, proc_gap_warn, proc_gap_flush,
      proc_gap_reset, backfill_capacity));
  }
catch(MonitorRegexError &e)
  {
    cfglog << e.message << endl;
  }

//*****************************************************************************
// Loading a filters file
//*****************************************************************************

template<class T>
inline void read_first_in_line(const string &file, ifstream &fs, T &val)
  {
    const int maxlen = 100;
    if(!(fs >> val).ignore(maxlen, '\n').good() || fs.gcount() == maxlen)
        throw SeedlinkBadFileFormat(file);
  }

void load_filters(const string &filter_file, map<string, rc_ptr<Filter> > &filters)
  {
    logs(LOG_INFO) << "loading FIR filters from file '" << filter_file << "'" << endl;
    
    ifstream fs(filter_file.c_str());
    if(fs.fail()) throw SeedlinkCannotOpenFile(filter_file);

    int n_filters;
    read_first_in_line(filter_file, fs, n_filters);
    
    logs(LOG_INFO) << n_filters << " filters are defined in file '" << filter_file << "'" << endl;
    
    for(int i = 0; i < n_filters; ++i)
      {
        string filter_name;
        int filter_length;
        double gain;
        int deci;
        double *points;

        read_first_in_line(filter_file, fs, filter_name);
        read_first_in_line(filter_file, fs, filter_length);
        read_first_in_line(filter_file, fs, gain);
        read_first_in_line(filter_file, fs, deci);

        if(filter_length > MAX_FILTER_LENGTH || filter_length < deci)
            throw SeedlinkErrorLoadingFilter(filter_file, filter_name);
        
        FilterImpl::FilterType filter_type;
        int n_points;
        
        if(filter_name.substr(filter_name.length() - 1) == "M")
          {
            filter_type = FilterImpl::MinimumPhase;
            n_points = filter_length;
          }
        else
          {
            filter_type = FilterImpl::ZeroPhase;
            // handles filters with odd number of coefficients as well
            n_points = filter_length / 2 + filter_length % 2;
          }
        
        points = new double[n_points];
        
        for(int j = 0; j < n_points; ++j)
            if(!(fs >> points[j]).good())
                throw SeedlinkErrorLoadingFilter(filter_file, filter_name);

        if(has_object(filters, filter_name))
          {
            logs(LOG_WARNING) << "filter " << filter_name << " has been already defined" << endl;
            delete [] points;
            continue;
          }

        insert_object<Filter>(filters, new FilterImpl(filter_name, filter_type, filter_length,
          deci, gain, points));
        
        logs(LOG_INFO) << "filter " << filter_name << ", length " << filter_length <<
          ", decimation factor " << deci << ", digital gain " << gain << endl;
      }
  }

//*****************************************************************************
// Loading a streams file
//*****************************************************************************

void load_streams(const string &stream_file, const map<string, rc_ptr<Filter> > &filters,
  map<string, rc_ptr<StreamProcessorSpec> > &sproc_defs)
  {
    logs(LOG_INFO) << "loading stream definitions from file '" << stream_file << "'" << endl;
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(make_stream_proc_cfg(filters, sproc_defs));
    read_config_xml(stream_file, "streams", new CfgAttributeMap, elms);
  }

//*****************************************************************************
// Locking the lockfile
//*****************************************************************************

bool run_check(const string &lockfile)
  {
    int fd, val;
    char buf[10];
    struct flock lock;
  
    if((fd = open(lockfile.c_str(), O_WRONLY | O_CREAT,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
        throw SeedlinkCannotOpenFile(lockfile);

    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
  
    if(fcntl(fd, F_SETLK, &lock) < 0)
      {
        if(errno == EACCES || errno == EAGAIN) return 1;
        else throw SeedlinkLibraryError("cannot lock file '" + lockfile + "'");
      }
  
    N(ftruncate(fd, 0));
    sprintf(buf, "%d\n", getpid());
    
    errno = 0;
    if(write(fd, buf, strlen(buf)) != (int) strlen(buf))
        throw SeedlinkLibraryError("cannot write pid to '" + lockfile + "'");

    N((val = fcntl(fd,F_GETFD,0)));

    val |= FD_CLOEXEC;
    N(fcntl(fd, F_SETFD, val));

    return 0;
  }

//*****************************************************************************
// InfoLevelAttribute -- definition of attribute InfoLevel
//*****************************************************************************

class InfoLevelAttribute: public CfgAttribute
  {
  private:
    int &valref;
    
  public:
    InfoLevelAttribute(const string &name, int &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value);
  };

bool InfoLevelAttribute::assign(ostream &cfglog, const string &value)
  {
    int level;
    for(level = 0; level < N_InfoLevel; ++level)
        if(!strcasecmp(value.c_str(), InfoLevelNames[level])) break;

    if(level == N_InfoLevel)
      {
        cfglog << "[" << item_name << "] '" << value << "' is not a valid "
          "info level name" << endl;
        return false;
      }

    valref = level;
    return true;
  }

//*****************************************************************************
// SeedlinkDef -- definition of main config element
//*****************************************************************************

class SeedlinkDef: public CfgElement
  {
  private:
    bool &found;
    bool dup;
    string lockfile;
    list<string> filter_files;
    list<string> stream_files;
    list<string> ip_trusted_str;
    list<string> ip_access_str;
    const rc_ptr<CfgAttributeMap> attributes;
    const rc_ptr<CfgElementMap> children;

  public:
    SeedlinkDef(bool &found_init):
      CfgElement("section"), found(found_init), dup(false) {}
    
    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &name);
    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &name);
  };

rc_ptr<CfgAttributeMap> SeedlinkDef::start_attributes(ostream &cfglog,
  const string &name)
  {
    if(strcasecmp(daemon_name.c_str(), name.c_str())) return NULL;
    
    if(found)
      {
        cfglog << "duplicate section '" << name << "'" << endl;
        dup = true;
        return NULL;
      }

    found = true;

    lockfile = "";
    filter_files.clear();
    stream_files.clear();

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringListAttribute("filters", filter_files, ", "));
    atts->add_item(StringListAttribute("streams", stream_files, ", "));
    atts->add_item(StringListAttribute("trusted", ip_trusted_str, ", "));
    atts->add_item(StringListAttribute("access", ip_access_str, ", "));
    atts->add_item(StringAttribute("lockfile", lockfile));
    atts->add_item(IntAttribute("buffers", no_of_buffers, 10, 100000));
    atts->add_item(IntAttribute("blanks", no_of_blanks, 0, 100));
    atts->add_item(IntAttribute("segments", no_of_segments, 2, 1000));
    atts->add_item(IntAttribute("segsize", segsize, 10, 100000));
    atts->add_item(StringAttribute("filebase", seedlink_dir));
    atts->add_item(IntAttribute("connections", max_connections, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("connections_per_ip", max_connections_per_ip, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("seq_gap_limit", seq_gap_limit, 0, (1 << 24)));
    atts->add_item(IntAttribute("plugin_timeout", plugin_timeout, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("plugin_start_retry", plugin_start_retry, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("plugin_shutdown_wait", plugin_shutdown_wait, 0, IntAttribute::lower_bound));
    atts->add_item(StringAttribute("network", network_id));
    atts->add_item(StringAttribute("organization", organization));
    atts->add_item(StringAttribute("encoding", seed_encoding));
    atts->add_item(StringAttribute("gap_check_pattern", gap_check_pattern));
    atts->add_item(IntAttribute("gap_treshold", gap_treshold, 100, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("bytespersec", bytespersec, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("proc_gap_warn", proc_gap_warn_default, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("proc_gap_flush", proc_gap_flush_default, 0, 100, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("proc_gap_reset", proc_gap_reset_default, 0, 100, IntAttribute::lower_bound));
    atts->add_item(FloatAttribute("backfill_buffer", backfill_capacity,  86400, 0, FloatAttribute::upper_bound));
    atts->add_item(IntAttribute("port", tcp_port, 1, 65535));
    atts->add_item(BoolAttribute("request_log", request_log, "enabled", "disabled"));
    atts->add_item(BoolAttribute("stream_check", stream_check, "enabled", "disabled"));
    atts->add_item(InfoLevelAttribute("info", untrusted_info_level));
    atts->add_item(InfoLevelAttribute("info_trusted", trusted_info_level));
    atts->add_item(BoolAttribute("window_extraction", untrusted_window_extraction, "enabled", "disabled"));
    atts->add_item(BoolAttribute("window_extraction_trusted", trusted_window_extraction, "enabled", "disabled"));
    atts->add_item(BoolAttribute("websocket", untrusted_websocket, "enabled", "disabled"));
    atts->add_item(BoolAttribute("websocket_trusted", trusted_websocket, "enabled", "disabled"));

    return atts;
  }

rc_ptr<CfgElementMap> SeedlinkDef::start_children(ostream &cfglog,
  const string &name)
  {
    if(dup || strcasecmp(daemon_name.c_str(), name.c_str())) return NULL;
    
    if(lockfile.length() != 0 && run_check(lockfile))
      {
        logs(LOG_ERR) << "already running" << endl;
        exit(1);
      }
    
    list<string>::iterator p;
    
    IPACL ip_trusted;
    for(p = ip_trusted_str.begin(); p != ip_trusted_str.end(); ++p)
      {
        if(!ip_trusted.add(*p))
          {
            cfglog << "invalid IP address/mask: " << *p << endl;
            return NULL;
          }
      }
        
    IPACL ip_access;
    for(p = ip_access_str.begin(); p != ip_access_str.end(); ++p)
      {
        if(!ip_access.add(*p))
          {
            cfglog << "invalid IP address/mask: " << *p << endl;
            return NULL;
          }
      }
    
    if(strcasecmp(seed_encoding.c_str(), "steim1") &&
      strcasecmp(seed_encoding.c_str(), "steim2"))
      {
        cfglog << "unsupported encoding '" << seed_encoding << "'" << endl;
        return NULL;
      }
    
    map<string, rc_ptr<StreamProcessorSpec> > sproc_defs;

    if(!stream_files.empty())
      {
        map<string, rc_ptr<Filter> > filters;

        if(!filter_files.empty())
          {
            for(p = filter_files.begin(); p != filter_files.end(); ++p)
                load_filters(*p, filters);
          }
    
        list<string>::const_iterator p;
        for(p = stream_files.begin(); p != stream_files.end(); ++p)
            load_streams(*p, filters, sproc_defs);
      }
    
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(PluginDef("plugin", sproc_defs));
    elms->add_item(StationDef("station", sproc_defs, ip_trusted, ip_access));

    return elms;
  }
    
//*****************************************************************************
// Loading config files
//*****************************************************************************

void configure_seedlink(const string &config_file)
  {
    bool found = false;
    logs(LOG_INFO) << "loading " << daemon_name << " configuration from file '" << config_file << "'" << endl;
    read_config_ini(config_file, SeedlinkDef(found));

    if(!found) throw CfgCannotFindSection(daemon_name, config_file);
  }

//*****************************************************************************
// IOHandler
//*****************************************************************************

volatile sig_atomic_t terminate_proc = 0;
volatile sig_atomic_t restart_proc = 0;

void int_handler(int sig)
  {
    terminate_proc = 1;
  }

void restart_handler(int sig)
  {
    restart_proc = 1;
  }


class IOHandler
  {
  private:
    bool terminating;
    time_t last_plugin_check;
  
  public:
    IOHandler(): terminating(false), last_plugin_check(0) {}
    bool operator()(Fdset &fds);
  };

bool IOHandler::operator()(Fdset &fds)
  {
    PluginPacketHeader head;
    char data_buf[PLUGIN_MAX_DATA_BYTES];
    list<list<rc_ptr<Plugin> >::iterator> shutdown_list;
    list<rc_ptr<Plugin> >::iterator p;
    rc_ptr<Station> st;

    if((terminate_proc || restart_proc) && !terminating)
      {
        if ( restart_proc )
            logs(LOG_NOTICE) << "restarting plugins" << endl;
        if ( terminate_proc )
            logs(LOG_NOTICE) << "terminating program" << endl;

        for(p = plugins.begin(); p != plugins.end(); ++p)
            (*p)->shutdown(restart_proc != 0);

        if ( terminate_proc )
            terminating = true;

        // Set back restart_proc to avoid restarting in the next call
        if ( restart_proc ) restart_proc = 0;
      }
    
    time_t curtime = time(NULL);
    
    for(p = plugins.begin(); p != plugins.end(); ++p)
      {
        if(!fds.isactive_read((*p)->fd()) && last_plugin_check == curtime)
            continue;

        while((*p)->read(head, data_buf))
          {
            string station_key(head.station, 0, PLUGIN_SIDLEN);
            
            if((st = get_object(stations, station_key)) == NULL)
              {
                logs(LOG_ERR) << "[" << (*p)->name << "]"
                  " station " << station_key << " is not configured" << endl;

                st = new Station(string(head.station, 0, PLUGIN_SIDLEN));
                insert_object(stations, st);
              }
            
            switch(head.packtype)
              {
              case PluginMSEEDPacket:
                DEBUG_MSG("PluginMSEEDPacket (" << station_key << ")" << endl);
                if(head.data_size != (1 << MSEED_RECLEN))
                  {
                    logs(LOG_ERR) << "[" << (*p)->name << "]"
                      " unsupported Mini-SEED packet size (" << head.data_size <<
                      " instead of " << (1 << MSEED_RECLEN) << ")" << endl;

                    (*p)->shutdown();
                    break;
                  }

                st->send_mseed(data_buf);
                break;

              case PluginLogPacket:
                DEBUG_MSG("PluginLogPacket (" << station_key << ")" << endl);
                st->send_log(head.pt, data_buf, head.data_size);
                break;

              case PluginRawDataTimePacket:
                DEBUG_MSG("PluginRawDataTimePacket (" << station_key << ", " <<
                  string(head.channel, 0, PLUGIN_CIDLEN) << ")" << endl);
                st->send_raw_with_time(*p, string(head.channel, 0, PLUGIN_CIDLEN),
                  head.pt, head.usec_correction, head.timing_quality,
                  reinterpret_cast<int32_t *>(data_buf), head.data_size);
                break;

              case PluginRawDataPacket:
                DEBUG_MSG("PluginRawDataPacket (" << station_key << ", " <<
                  string(head.channel, 0, PLUGIN_CIDLEN) << ")" << endl);
                st->send_raw(*p, string(head.channel, 0, PLUGIN_CIDLEN),
                  reinterpret_cast<int32_t *>(data_buf), head.data_size);
                break;

              case PluginRawDataGapPacket:
                DEBUG_MSG("PluginRawDataGapPacket (" << station_key << ", " <<
                  string(head.channel, 0, PLUGIN_CIDLEN) << ")" << endl);
                st->send_gap(*p, string(head.channel, 0, PLUGIN_CIDLEN),
                  head.usec_correction, head.timing_quality, head.data_size);
                break;

              case PluginRawDataFlushPacket:
                DEBUG_MSG("PluginRawDataFlushPacket (" << station_key << ", " <<
                  string(head.channel, 0, PLUGIN_CIDLEN) << ")" << endl);
                st->send_flush(*p, string(head.channel, 0, PLUGIN_CIDLEN));
                break;

              default:
                logs(LOG_ERR) << "[" << (*p)->name << "] invalid data type" << endl;
                (*p)->shutdown();
              }
          }

        if((*p)->active()) fds.set_read((*p)->fd());
        else fds.clear_read((*p)->fd());
        
        if(terminating && !(*p)->running()) shutdown_list.push_back(p);
      }

    last_plugin_check = curtime;
        
    while(!shutdown_list.empty())
      {
        plugins.erase(shutdown_list.front());
        shutdown_list.pop_front();
      }
    
    if(plugins.size() == 0)
      {
        map<string, rc_ptr<Station> >::iterator s;
        for(s = stations.begin(); s != stations.end(); ++s)
            s->second->flush();
        
        return false;
      }

    return true;
  }

//*****************************************************************************
// More helper functions
//*****************************************************************************

void start_plugins()
  {
    list<rc_ptr<Plugin> >::iterator p;
    
    for(p = plugins.begin(); p != plugins.end(); ++p)
        (*p)->start();
  }
    
string get_progname(char *argv0)
  {
    string::size_type pos;
    string s = argv0;
    if((pos = s.rfind('/')) != string::npos)
        s = string(argv0, pos + 1, string::npos);

    return s;
  }

} // unnamed namespace

namespace CPPStreams {

Stream logs = make_stream(LogFunc());

}

//*****************************************************************************
// Main
//*****************************************************************************

int main(int argc, char **argv)
try
  {
    setpriority(PRIO_PROCESS, 0, -20);
    seteuid(getuid());
    setegid(getgid());

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    struct option ops[] = 
      {
        { "verbosity",      required_argument, NULL, 'X' },
        { "daemon",         no_argument,       NULL, 'D' },
        { "config-file",    required_argument, NULL, 'f' },
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    daemon_name = get_progname(argv[0]);
    
    string config_file = CONFIG_FILE;
    
    int c;
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    while((c = getopt_long(argc, argv, "vDf:Vh", ops, NULL)) != EOF)
#else
    while((c = getopt(argc, argv, "vDf:Vh")) != EOF)
#endif
      {
        switch(c)
          {
          case 'v': ++verbosity; break;
          case 'X': verbosity = atoi(optarg); break;
          case 'D': daemon_mode = true; break;
          case 'f': config_file = optarg; break;
          case 'V': cout << ident_str << endl; exit(0);
          case 'h': fprintf(stdout, help_message, daemon_name.c_str()); exit(0);
          case '?': fprintf(stderr, opterr_message, daemon_name.c_str()); exit(0);
          }
      }
    
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGINT, &sa, NULL));
    N(sigaction(SIGTERM, &sa, NULL));

    sa.sa_handler = restart_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGHUP, &sa, NULL));

    sa.sa_handler = SIG_IGN;
    N(sigaction(SIGPIPE, &sa, NULL));
    
    if(daemon_mode)
      {
        int pid;
        logs(LOG_INFO) << ident_str << " started" << endl;
        logs(LOG_INFO) << "take a look into syslog files for more messages" << endl;
        N(pid = fork());
        if(pid) exit(0);
        setsid();
        openlog(daemon_name.c_str(), 0, SYSLOG_FACILITY);
        daemon_init = true;
      }

    redirect_ostream(cout, LogFunc(), LOG_INFO);
    redirect_ostream(cerr, LogFunc(), LOG_ERR);
    redirect_ostream(clog, LogFunc(), LOG_ERR);

    logs(LOG_NOTICE) << ident_str << " started" << endl;
    
    configure_seedlink(config_file);
    if(stations.size() == 0)
      {
        logs(LOG_ERR) << "no seedlink stations defined" << endl;
        exit(1);
      }
    
    if(plugins.size() == 0)
      {
        logs(LOG_ERR) << "no plugins defined" << endl;
        exit(1);
      }
 
    if(tcp_port == 0)
      {
        logs(LOG_ERR) << "TCP port not specified" << endl;
        exit(1);
      }

    start_plugins();
    
    connectionManager->set_handler(IOHandler());
    connectionManager->restore_state();
    connectionManager->start(tcp_port);
    connectionManager->save_state();

    return 0;
  }
catch(exception &e)
  {
    logs(LOG_ERR) << e.what() << endl;
    return 1;
  }
catch(...)
  {
    logs(LOG_ERR) << "unknown exception" << endl;
    return 1;
  }
 
