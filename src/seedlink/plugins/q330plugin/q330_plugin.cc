/***************************************************************************** 
 * q330_plugin.cc
 *
 * New plugin for Q330, based on lib330
 *
 * (c) 2007 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <set>
#include <cstring>
#include <cstdio>
#include <csignal>
#include <cerrno>

#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/resource.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#include "confbase.h"
#include "conf_ini.h"
#include "confattr.h"
#include "cppstreams.h"
#include "utils.h"
#include "plugin.h"
#include "plugin_exceptions.h"
#include "diag.h"

extern "C" {
#define pascal_h
#include "libtypes.h"
#include "libclient.h"
#include "libmsgs.h"
#include "libsupport.h"
}

#define MYVERSION "2.0 (2010.256)"

#ifndef CONFIG_FILE
#define CONFIG_FILE "/home/sysop/config/plugins.ini"
#endif

#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_LOCAL0
#endif

using namespace std;
using namespace SeedlinkPlugin;
using namespace CfgParser;
using namespace CPPStreams;
using namespace Utilities;

namespace {

const double EPOCH_DELTA       = 946684800.0;
const char *const SEED_NEWLINE = "\r\n";
const char *const ident_str    = "SeedLink Q330 Plugin v" MYVERSION;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
const char *const opterr_message = "Try `%s --help' for more information\n";
const char *const help_message = 
    "Usage: %s [options] plugin_name\n"
    "\n"
    "'plugin_name' is the section name in config file; it is also used\n"
    "as a signature in log messages\n"
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
    "Usage: %s [options] plugin_name\n"
    "\n"
    "'plugin_name' is the section name in config file; it is also used\n"
    "as a signature in log messages\n"
    "\n"
    "-v             Increase verbosity level\n"
    "-D             Daemon mode\n"
    "-f FILE        Alternative configuration file\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

std::string plugin_id, station_id;
int verbosity = 0, statusinterval = 0, last_day = -1, last_soh = -1;
bool daemon_mode = false, daemon_init = false, lib330_init = false;
bool ident_needed = true, soh_update_needed = true, gps_update_needed = true;
volatile sig_atomic_t term_proc = 0;
volatile int term_err = 0, term_sig = 0;
time_t timestamp = 0;
set<std::string> raw_streams;
ostream seed_log(NULL);

// lib330 stuff
tcontext stationContext;
tpar_register registrationInfo;
tpar_create creationInfo;
volatile tlibstate currentLibState = LIBSTATE_IDLE;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

#define MSG(prio, expr) do { \
    pthread_mutex_lock(&mut); \
    logs(prio) << expr; \
    pthread_mutex_unlock(&mut); \
  } while(0)

//*****************************************************************************
// Signalling
//*****************************************************************************

int sigpipe[2];
enum { READ = 0, WRITE = 1 };

void sigpipe_init()
  {
    N(pipe(sigpipe));
    N(fcntl(sigpipe[READ], F_SETFL, O_NONBLOCK));
    N(fcntl(sigpipe[WRITE], F_SETFL, O_NONBLOCK));
  }

void sigpipe_notify()
  {
    ssize_t r;

    do
      {
        r = write(sigpipe[WRITE], "", 1);
      }
    while(r < 0 && errno == EINTR);

    if(r < 0 && errno != EAGAIN)
        throw PluginLibraryError("error writing signal pipe");
  }

void sigpipe_clear()
  {
    ssize_t r;

    do
      {
        char c;
        r = read(sigpipe[READ], &c, 1);
      }
    while(r > 0 || (r < 0 && errno == EINTR));

    if(r < 0 && errno != EAGAIN)
        throw PluginLibraryError("error reading signal pipe");
    else if(r == 0)
        throw PluginError("unexpected EOF while reading signal pipe");
  }

void int_handler(int sig)
  {
    term_sig = sig;
    term_proc = 1;
    sigpipe_notify();
  }

void chksl(int r)
  {
    if(r < 0)
        MSG(LOG_ERR, "error sending data to seedlink: " << strerror(errno) << endl);
    else if(r == 0)
        MSG(LOG_ERR, "error sending data to seedlink" << endl);

    if(r <= 0)
      {
        term_err = 1;
        term_proc = 1;
        sigpipe_notify();
      }
  }

//*****************************************************************************
// Status messages
//*****************************************************************************

bool gps_update(const topstat &ops, bool initial)
  {
    if(ops.gps_fix == GPF_NB)
      {
        seed_log << "no GPS board installed";
        return true;
      }

    if(ops.gps_stat == GPS_COLDSTART)
      {
        if(initial)
            seed_log << "GPS cold-start";

        return false;
      }

    bool on = false, fix = false, bad = false;
    ostringstream ss;
    
    ss << "GPS ";
    switch(ops.gps_stat)
      {
      case GPS_OFF:       on = false; ss << "off"; break;
      case GPS_OFF_LOCK:  on = false; ss << "off due to GPS lock"; break;
      case GPS_OFF_PLL:   on = false; ss << "off due to PLL lock"; break;
      case GPS_OFF_LIMIT: on = false; ss << "off due to time limit"; break;
      case GPS_OFF_CMD:   on = false; ss << "off by command"; break;
      case GPS_ON:        on = true;  ss << "on"; break;
      case GPS_ON_AUTO:   on = true;  ss << "on automatically"; break;
      case GPS_ON_CMD:    on = true;  ss << "on by command"; break;
      default: bad = true;
      }

    ss << ", ";
    switch(ops.gps_fix)
      {
      case GPF_LF:  bad |= on;  fix = false; ss << "never locked"; break;
      case GPF_OFF: bad |= on;  fix = false; ss << "unknown lock"; break;
      case GPF_1DF: bad |= on;  fix = true;  ss << "last fix 1D"; break;
      case GPF_2DF: bad |= on;  fix = true;  ss << "last fix 2D"; break;
      case GPF_3DF: bad |= on;  fix = true;  ss << "last fix 3D"; break;
      case GPF_NL:  bad |= !on; fix = false; ss << "never locked"; break;
      case GPF_ON:  bad |= !on; fix = false; ss << "unknown lock"; break;
      case GPF_1D:  bad |= !on; fix = true;  ss << "1D fix"; break;
      case GPF_2D:  bad |= !on; fix = true;  ss << "2D fix"; break;
      case GPF_3D:  bad |= !on; fix = true;  ss << "3D fix"; break;
      default: bad = true;
      }

    ss << ", PLL ";
    switch(ops.pll_stat)
      {
      case PLS_LOCK:  ss << "locked"; break;
      case PLS_TRACK: ss << "tracking"; break;
      case PLS_HOLD:  ss << "hold"; break;
      case PLS_OFF:   ss << "off"; break;
      default: bad = true;
      }

    if(bad)
      {
        if(initial)
          {
            seed_log << "invalid GPS status: "
                     << "gps_stat=" << ops.gps_stat << " "
                     << "gps_fix=" << ops.gps_fix << " "
                     << "pll_stat=" << ops.pll_stat << endl;
          }
            
        return false;
      }

    if(!fix || ops.gps_age < 0)
      {
        if(initial)
            seed_log << ss.str() << endl;

        return false;
      }

    seed_log << ss.str() << SEED_NEWLINE
             << "last GPS update " << ops.gps_age << " seconds ago"
             << SEED_NEWLINE
             << fixed << setprecision(4)
             << "latitude " << ops.gps_lat << ", "
             << "longitude " << ops.gps_long << ", "
             << setprecision(1)
             << "elevation " << ops.gps_elev << endl;

    return true;
  }

void soh_update(const topstat &ops)
  {
    seed_log << "clock quality " << ops.clock_qual << "%, "
                     << "drift " << ops.clock_drift << "us"
             << SEED_NEWLINE
             << "mass positions " << ops.mass_pos[0] << " "
                                  << ops.mass_pos[1] << " "
                                  << ops.mass_pos[2] << " "
                                  << ops.mass_pos[3] << " "
                                  << ops.mass_pos[4] << " "
                                  << ops.mass_pos[5]
             << SEED_NEWLINE
             << fixed << setprecision(2)
             << "voltage " << ops.pwr_volt << "V, "
             << setprecision(1)
             << "current " << 1000 * ops.pwr_cur << "mA, "
             << "temperature " << ops.sys_temp << "C" << endl;
  }

void check_status()
  {
    if(timestamp / (24 * 60 * 60) != last_day)
      {
        ident_needed = true;
        soh_update_needed = true;
        gps_update_needed = true;
        last_day = timestamp / (24 * 60 * 60);
      }
    
    if(statusinterval &&
      timestamp / (statusinterval * 60) != last_soh)
      {
        soh_update_needed = true;
        gps_update_needed = true;
        last_soh = timestamp / (statusinterval * 60);
      }

    if(ident_needed || soh_update_needed || gps_update_needed)
      {
        tm tm;
        gmtime_r(&timestamp, &tm);
        
        if(tm.tm_hour != 0 || tm.tm_min != 0 || tm.tm_sec != 0)
          {
            tliberr errcode;
            topstat ops;

            memset(&ops, 0, sizeof(ops));
            lib_get_state(stationContext, &errcode, &ops);

            if(ident_needed)
              {
                seed_log << ident_str << endl;
                ident_needed = false;
              }
            
            if(soh_update_needed)
              {
                soh_update(ops);
                soh_update_needed = false;
                gps_update_needed = !gps_update(ops, true);
              }
            else if(gps_update_needed)
              {
                gps_update_needed = !gps_update(ops, false);
              }
          }
      }
  }

//*****************************************************************************
// lib330 callbacks
//*****************************************************************************

void state_callback(pointer p)
  {
    tstate_call* state = reinterpret_cast<tstate_call*>(p);
    if(state->state_type != ST_STATE)
        return;

    currentLibState = static_cast<tlibstate>(state->info);

    string63 state_name;
    lib_get_statestr(currentLibState, &state_name);
    
    MSG(LOG_INFO, "state change to '" << state_name << "'" << endl);
    
    if(currentLibState == LIBSTATE_RUNWAIT)
        lib_change_state(stationContext, LIBSTATE_RUN, LIBERR_NOERR);

    sigpipe_notify();
  }

void miniseed_callback(pointer p)
  {
    tminiseed_call* data = reinterpret_cast<tminiseed_call*>(p);
    std::string channel_id = std::string(data->location) + std::string(data->channel);

    if(raw_streams.find(channel_id) != raw_streams.end())
        return;

    chksl(send_mseed(station_id.c_str(), data->data_address, data->data_size));
  }

void secdata_callback(pointer p)
  {
    tonesec_call* data = reinterpret_cast<tonesec_call*>(p);
    std::string channel_id = std::string(data->location) + std::string(data->channel);

    if(time_t(data->timestamp + EPOCH_DELTA) > timestamp)
      {
        timestamp = time_t(data->timestamp + EPOCH_DELTA);
        check_status();
      }
    
    if(raw_streams.find(channel_id) == raw_streams.end())
        return;

    chksl(send_raw_depoch(station_id.c_str(), channel_id.c_str(),
      data->timestamp + EPOCH_DELTA, 0, data->qual_perc, data->samples,
      ((data->rate > 0)? data->rate: 1)));
  }

//*****************************************************************************
// lib330 error handling
//*****************************************************************************

void handle_error(tliberr errcode)
  {
    string63 errmsg;
    lib_get_errstr(errcode, &errmsg);
    MSG(LOG_ERR, "{ERR" << setfill('0') << setw(3) << errcode << "} "
        << errmsg << endl);
  }

//*****************************************************************************
// Loading config file
//*****************************************************************************

#define STRCPY(dest, src) do { \
    strncpy(dest, src, sizeof(dest)-1); \
    dest[sizeof(dest)-1] = 0; \
  } while(0)

bool configure_plugin(const std::string &config_file)
  {
    std::string udpaddr;
    int baseport = 0;
    int dataport = 0;
    int hostport = 0;
    std::string serialnumber;
    std::string authcode;
    std::string statefile;
    list<std::string> messages;
    list<std::string> raw_list;
    char *tail;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("station", station_id));
    atts->add_item(StringAttribute("udpaddr", udpaddr));
    atts->add_item(IntAttribute("baseport", baseport, 1, 65535));
    atts->add_item(IntAttribute("dataport", dataport, 1, 4));
    atts->add_item(IntAttribute("hostport", hostport, 1, 65535));
    atts->add_item(StringAttribute("serialnumber", serialnumber));
    atts->add_item(StringAttribute("authcode", authcode));
    atts->add_item(StringAttribute("statefile", statefile));
    atts->add_item(StringListAttribute("messages", messages, ", "));
    atts->add_item(StringListAttribute("raw_streams", raw_list, ", "));
    atts->add_item(IntAttribute("statusinterval", statusinterval, 0, 24*60));

    MSG(LOG_INFO, "loading configuration from file '" << config_file << "'" << endl);
    read_config_ini(config_file, plugin_id, atts, new CfgElementMap);

    if(serialnumber.length() == 0)
      {
        MSG(LOG_ERR, "Q330 serial number is not set" << endl);
        return false;
      }
    
    uint64_t llserial = strtoull(serialnumber.c_str(), &tail, 16);
    memcpy(creationInfo.q330id_serial, &llserial, 8);

    if(*tail)
      {
        MSG(LOG_ERR, "invalid Q330 serial number: " << serialnumber << endl);
        return false;
      }

    switch(dataport)
      {
      case 1:
        creationInfo.q330id_dataport = LP_TEL1;
        break;

      case 2:
        creationInfo.q330id_dataport = LP_TEL2;
        break;

      case 3:
        creationInfo.q330id_dataport = LP_TEL3;
        break;

      case 4:
        creationInfo.q330id_dataport = LP_TEL4;
        break;

      default:
        MSG(LOG_ERR, "Q330 dataport is not set" << endl);
        return false;
      }

    if(station_id.length() == 0)
      {
        MSG(LOG_ERR, "station ID is not set" << endl);
        return false;
      }
    
    STRCPY(creationInfo.q330id_station, station_id.c_str());
    creationInfo.host_timezone = 0;
    STRCPY(creationInfo.host_software, ident_str);
    STRCPY(creationInfo.opt_contfile, statefile.c_str());

    creationInfo.opt_verbose = 0;
    for(list<std::string>::iterator it = messages.begin();
      it != messages.end(); ++it)
      {
        if(!strcasecmp(it->c_str(), "SDUMP"))
            creationInfo.opt_verbose |= VERB_SDUMP;
        else if(!strcasecmp(it->c_str(), "RETRY"))
            creationInfo.opt_verbose |= VERB_RETRY;
        else if(!strcasecmp(it->c_str(), "REGMSG"))
            creationInfo.opt_verbose |= VERB_REGMSG;
        else if(!strcasecmp(it->c_str(), "LOGEXTRA"))
            creationInfo.opt_verbose |= VERB_LOGEXTRA;
        else if(!strcasecmp(it->c_str(), "AUXMSG"))
            creationInfo.opt_verbose |= VERB_AUXMSG;
        else if(!strcasecmp(it->c_str(), "PACKET"))
            creationInfo.opt_verbose |= VERB_PACKET;
        else if(!strcasecmp(it->c_str(), "ALL"))
            creationInfo.opt_verbose |= ~0;
        else
          {
            MSG(LOG_ERR, "invalid message type: " << (*it) << endl);
            return false;
          }
      }
 
    creationInfo.opt_zoneadjust = 0;
    creationInfo.opt_secfilter = OSF_1HZ;
    creationInfo.opt_minifilter = OMF_ALL;
    creationInfo.opt_aminifilter = 0;
    creationInfo.amini_exponent = 0;
    creationInfo.amini_512highest = 0;
    creationInfo.mini_embed = 0;
    creationInfo.mini_separate = 1;
    creationInfo.mini_firchain = NULL;
    creationInfo.call_minidata = miniseed_callback;
    creationInfo.call_aminidata = NULL;
    creationInfo.resp_err = LIBERR_NOERR;
    creationInfo.call_state = state_callback;
    creationInfo.call_messages = NULL;
    creationInfo.call_secdata = secdata_callback;
    creationInfo.call_lowlatency = NULL;

    if(authcode.length() == 0)
      {
        MSG(LOG_ERR, "Q330 authentication code is not set" << endl);
        return false;
      }
    
    uint64_t llauthcode = strtoull(authcode.c_str(), &tail, 16);
    memcpy(registrationInfo.q330id_auth, &llauthcode, 8);

    if(*tail)
      {
        MSG(LOG_ERR, "invalid Q330 authentication code: " << authcode << endl);
        return false;
      }

    if(udpaddr.length() == 0)
      {
        MSG(LOG_ERR, "Q330 address is not set" << endl);
        return false;
      }
    
    STRCPY(registrationInfo.q330id_address, udpaddr.c_str());

    if(baseport == 0)
      {
        MSG(LOG_ERR, "Q330 base UDP port is not set" << endl);
        return false;
      }
    
    registrationInfo.q330id_baseport = baseport;
    registrationInfo.host_mode = HOST_ETH;
    STRCPY(registrationInfo.host_interface, "");
    registrationInfo.host_mincmdretry = 5;
    registrationInfo.host_maxcmdretry = 40;

    if(hostport != 0)
      {
        registrationInfo.host_ctrlport = hostport;
        registrationInfo.host_dataport = hostport + 1;
      }
    else
      {
        registrationInfo.host_ctrlport = 0;
        registrationInfo.host_ctrlport = 0;
      }

    registrationInfo.opt_latencytarget = 0;
    registrationInfo.opt_closedloop = 0;
    registrationInfo.opt_dynamic_ip = 0;
    registrationInfo.opt_hibertime = 0;
    registrationInfo.opt_conntime = 0;
    registrationInfo.opt_connwait = 0;
    registrationInfo.opt_regattempts = 0;
    registrationInfo.opt_ipexpire = 0;
    registrationInfo.opt_buflevel = 0;

    if(raw_list.size() != 0)
      {
        list<std::string>::iterator it;
        for(it = raw_list.begin(); it != raw_list.end(); ++it)
            raw_streams.insert(*it);

        creationInfo.opt_secfilter = OSF_ALL;
      }
    
    return true;
  }

//*****************************************************************************
// SystemLog
//*****************************************************************************

class SystemLog
  {
  public:
    enum { msglen = 200 };
    
    int operator()(int priority, const std::string &msg)
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

            tm tm;
            char buf[100];
            time_t t = time(NULL);
            asctime_r(localtime_r(&t, &tm), buf);
            std::string msgout = std::string(buf, strlen(buf) - 1) + " - " + plugin_id + ": " + msg;
            write(STDOUT_FILENO, msgout.c_str(), msgout.length());
          }

        return msg.length();
      }
  };

//*****************************************************************************
// SEEDLog
//*****************************************************************************

class SEEDLog
  {
  public:
    enum { msglen = PLUGIN_MAX_MSG_SIZE };

    int operator()(int priority, const std::string &msg)
      {
        std::string msgout;
        int n = 0, msglen, seplen;
        while(msglen = strcspn(msg.c_str() + n, SEED_NEWLINE),
          seplen = strspn(msg.c_str() + n + msglen, SEED_NEWLINE))
          {
            std::string msgline(msg, n, msglen);
            if(msgout.length() + plugin_id.length() + msgline.length() + 5 > PLUGIN_MAX_MSG_SIZE)
                break;
            
            msgout += (plugin_id + ": " + msgline + SEED_NEWLINE);
            n += (msglen + seplen);
          }
 
        if(n == 0)
          {
            msgout = plugin_id + ": " + msg;
            n = msg.length();
          }

        tm tm;
        ptime pt;
        gmtime_r(&timestamp, &tm);
        pt.year = tm.tm_year + 1900;
        pt.yday = tm.tm_yday + 1;
        pt.hour = tm.tm_hour;
        pt.minute = tm.tm_min;
        pt.second = tm.tm_sec;
        pt.usec = 0;
        chksl(send_log3(station_id.c_str(), &pt, "%s", msgout.c_str()));
        return n;
      }
  };

//*****************************************************************************
// get_progname()
//*****************************************************************************

std::string get_progname(char *argv0)
  {
    std::string::size_type pos;
    std::string s = argv0;
    if((pos = s.rfind('/')) != std::string::npos)
        s = std::string(argv0, pos + 1, std::string::npos);

    return s;
  }

//*****************************************************************************
// init_plugin()
//*****************************************************************************

void init_plugin(int argc, char **argv)
  {
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

    std::string config_file = CONFIG_FILE;
    
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
          case 'V': cout << ident_str << endl;
                    exit(0);
          case 'h': fprintf(stdout, help_message, get_progname(argv[0]).c_str());
                    exit(0);
          case '?': fprintf(stderr, opterr_message, get_progname(argv[0]).c_str());
                    exit(1);
          }
      }

    if(optind != argc - 1)
      {
        fprintf(stderr, help_message, get_progname(argv[0]).c_str());
        exit(1);
      }

    plugin_id = std::string(argv[optind]);

    // lib330 seems to have problems with too high NOFILE
    struct rlimit rl;
    N(getrlimit(RLIMIT_NOFILE, &rl));
    rl.rlim_cur = 256;
    N(setrlimit(RLIMIT_NOFILE, &rl));
    
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGINT, &sa, NULL));
    N(sigaction(SIGTERM, &sa, NULL));
    
    sa.sa_handler = SIG_IGN;
    N(sigaction(SIGHUP, &sa, NULL));
    N(sigaction(SIGPIPE, &sa, NULL));
    
    sigpipe_init();

    if(daemon_mode)
      {
        MSG(LOG_INFO, ident_str << " started" << endl);
        MSG(LOG_INFO, "take a look into syslog files for more messages" << endl);

        openlog(plugin_id.c_str(), 0, SYSLOG_FACILITY);
        daemon_init = true;
      }

    redirect_ostream(cout, SystemLog(), LOG_INFO);
    redirect_ostream(cerr, SystemLog(), LOG_ERR);
    redirect_ostream(clog, SystemLog(), LOG_ERR);
    redirect_ostream(seed_log, SEEDLog(), 0);

    MSG(LOG_NOTICE, ident_str << " started" << endl);
    
    if(!configure_plugin(config_file))
      {
        MSG(LOG_NOTICE, "fatal config errors detected" << endl);
        exit(1);
      }
      
    pmodules mods = lib_get_modules();
    MSG(LOG_INFO, "lib330 modules:");
    for(int i = 0; i < MAX_MODULES; ++i)
      {
        tmodule* mod = &(*mods)[i];
        if(!mod->name[0])
            continue;

        MSG(LOG_INFO, " " << mod->name << ":" << mod->ver);
        if ((i % 5) == 4 && i < MAX_MODULES-1)
            MSG(LOG_INFO, endl);
      }

    MSG(LOG_INFO, endl);
  }

//*****************************************************************************
// wait_for_state()
//*****************************************************************************

void wait_for_state(tlibstate state)
  {
    while(currentLibState != state)
      {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sigpipe[READ], &read_fds);

        int r = select(sigpipe[READ] + 1, &read_fds, NULL, NULL, NULL);
        if(r < 0)
          {
            if (errno != EINTR)
                throw PluginLibraryError("select error");
          }
        else if(r > 0)
          {
            sigpipe_clear();
          }
      }
  }
 
//*****************************************************************************
// run_plugin()
//*****************************************************************************

void run_plugin()
  {
    tliberr errcode;
    
    lib_create_context(&stationContext, &creationInfo);
    if(creationInfo.resp_err != LIBERR_NOERR)
      {
        handle_error(creationInfo.resp_err);
        return;
      }
    
    lib330_init = true;

    if((errcode = lib_register(stationContext, &registrationInfo)) != LIBERR_NOERR)
      {
        handle_error(errcode);
        return;
      }

    while(!term_proc)
      {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sigpipe[READ], &read_fds);

        int r = select(sigpipe[READ] + 1, &read_fds, NULL, NULL, NULL);
        if(r < 0)
          {
            if (errno != EINTR)
                throw PluginLibraryError("select error");
          }
        else if(r > 0)
          {
            sigpipe_clear();
          }
      }
        
    if(term_err != 0)
        MSG(LOG_INFO, "terminating due to error" << endl);
    else if(term_sig != 0)
        MSG(LOG_INFO, "terminating due to signal " << term_sig << endl);
  }

//*****************************************************************************
// cleanup_plugin()
//*****************************************************************************

void cleanup_plugin()
  {
    if(!lib330_init)
        return;
    
    tliberr errcode;
    
    lib_change_state(stationContext, LIBSTATE_IDLE, LIBERR_NOERR);
    wait_for_state(LIBSTATE_IDLE);
    
    lib_change_state(stationContext, LIBSTATE_TERM, LIBERR_CLOSED);
    wait_for_state(LIBSTATE_TERM);

    if((errcode = lib_destroy_context(&stationContext)) != LIBERR_NOERR)
        handle_error(errcode);
        
    lib330_init = false;
  }

} // unnamed namespace

namespace CPPStreams {

Stream logs = make_stream(SystemLog());

}

//*****************************************************************************
// Main
//*****************************************************************************

int main(int argc, char **argv)
try
  {
    init_plugin(argc, argv);
    run_plugin();
    cleanup_plugin();
    return term_err;
  }
catch(exception &e)
  {
    MSG(LOG_ERR, e.what() << endl);
    cleanup_plugin();
    return 1;
  }
catch(...)
  {
    MSG(LOG_ERR, "unknown exception" << endl);
    cleanup_plugin();
    return 1;
  }
 
