/*****************************************************************************
 * reftek_plugin.cc
 *
 * SeedLink plugin for RTPD
 *
 * (c) 2004 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <set>
#include <cstring>
#include <cstdio>
#include <csignal>
#include <cerrno>
#include <map>

#include <unistd.h>
#include <syslog.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

//#define TEST

extern "C" {
#include "reftek.h"
}

#include "confbase.h"
#include "conf_ini.h"
#include "confattr.h"
#include "cppstreams.h"
#include "utils.h"
#include "plugin.h"
#include "plugin_exceptions.h"
#include "diag.h"

#define MYVERSION "1.3 (2013.226)"

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

const int         POLL_USEC       = 1000000;
const int         SHMSGOFF        = 24;
const int         SHMSGLEN        = 1000;
const int         DEFAULT_TQ      = 50;
const int         UNLOCK_TQ       = 10;
const int         TMO_DEF         = 60;
const int         TMO_MIN         = 5;
const int         MAX_PHASE_ERROR = 5;
const char *const LOCK_MSG        = "CLOCK IS LOCKED";
const char *const UNLOCK_MSG      = "CLOCK IS UNLOCKED";
const char *const PHASE_ERROR_MSG = "INTERNAL CLOCK PHASE ERROR OF "; /*K.S.*/
const char *const SEED_NEWLINE    = "\r\n";
const char *const ident_str       = "SeedLink RTPD Plugin v" MYVERSION;

typedef map<int, string> Unit2StationMap;

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
    "-m, --map-file=FILE           Alternative station map file\n"
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

string plugin_id;
int verbosity = 0;
bool daemon_mode = false, daemon_init = false;
volatile sig_atomic_t terminate_proc = 0;

void int_handler(int sig)
  {
    terminate_proc = 1;
  }


void read_map_file(Unit2StationMap &map, const string &filename)
  {
    map.clear();
    ifstream ifs(filename.data());
    if ( !ifs.is_open() ) return;

    // read line by line
    string line;
    while ( getline(ifs, line) )
      {
        if ( line.empty() ) continue;
        if ( line[0] == '#' ) continue;

        stringstream ss(line);
        int unit_id;
        string name;
        ss >> hex >> unit_id >> name;
        logs(LOG_INFO) << "adding mapping from unit " << hex << uppercase
                       << unit_id << " to station " << name << endl;
        map[unit_id] = name;
      }
  }


//*****************************************************************************
// Exceptions
//*****************************************************************************

class PluginRTPDError: public PluginError
  {
  public:
    PluginRTPDError(const string &msg): PluginError(msg) {}
  };

class PluginRTPDBrokenConnection: public PluginRTPDError
  {
  public:
    const int severity;
    PluginRTPDBrokenConnection(const string &msg, int severity_init):
      PluginRTPDError(msg), severity(severity_init) {}
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
            string msgout = string(p, strlen(p) - 1) + " - " + plugin_id + ": " + msg;
            write(STDOUT_FILENO, msgout.c_str(), msgout.length());
          }

        return msg.length();
      }
  };

//*****************************************************************************
// ReftekUnit
//*****************************************************************************

class ReftekUnit
  {
  private:
    string name;
    bool mapped;
    const int default_tq;
    const int unlock_tq;
    const bool log_soh;
    bool gps_msg_received;
    bool gps_in_lock;
    int gps_phase_error;  /*K.S. 14-Jan-2008*/
    time_t last_recv;

    ptime depoch2pt(REAL64 depoch);
    void proc_DT(const struct reftek_dt &dt);
    void proc_SH(const struct reftek_sh &sh, const char *msg);

  public:
    ReftekUnit(const string &name_init, bool mapped_init, int default_tq_init,
      int unlock_tq_init, bool log_soh_init):
      name(name_init), mapped(mapped_init), default_tq(default_tq_init),
      unlock_tq(unlock_tq_init), log_soh(log_soh_init), gps_msg_received(false),
      gps_in_lock(false), gps_phase_error(0), last_recv(0) {}
    
    void proc_pkt(UINT8 *pkt);
    
    time_t last_packet_received()
      {
        return last_recv;
      }
  };

ptime ReftekUnit::depoch2pt(REAL64 depoch)
  {
    time_t t = (time_t)depoch;
    tm* ptm = gmtime(&t);
    ptime pt;
    pt.year = ptm->tm_year + 1900;
    pt.yday = ptm->tm_yday + 1;
    pt.hour = ptm->tm_hour;
    pt.minute = ptm->tm_min;
    pt.second = ptm->tm_sec;
    pt.usec = int((depoch - double(t)) * 1000000.0);
    return pt;
  }

void ReftekUnit::proc_DT(const struct reftek_dt &dt)
  {
    ptime pt = depoch2pt(dt.tstamp);

    char stream_id[4];
    snprintf(stream_id, 4, "%d.%d", dt.stream, dt.chan);

    int tq = 100;
    int kstmp;
    if(!gps_msg_received)
        tq = default_tq;
    else if(!gps_in_lock)
        tq = unlock_tq;
    else  {  /*K.S. 14-Jan-2008*/
        kstmp = abs(gps_phase_error);
        if (kstmp > MAX_PHASE_ERROR) {
            if (kstmp > 100000)
                tq = 10;
            else if (kstmp > 10000)
                tq = 30;
            else if (kstmp > 1000)
                tq = 50;
            else if (kstmp > 100)
                tq = 65;
            else if (kstmp > 10)
                tq = 80;
            else
                tq = 90;
        }
    }

#ifndef TEST
    int r = send_raw3(name.data(), stream_id, &pt, 0, tq, (int32_t *)dt.data,
      dt.nsamp);
#else
    int r = dt.nsamp;
#endif

    if(r < 0) throw PluginBrokenLink(strerror(errno));
    else if(r == 0 && dt.nsamp != 0) throw PluginBrokenLink();
  }

void ReftekUnit::proc_SH(const struct reftek_sh &sh, const char *msg)
  {
    ptime pt = depoch2pt(sh.tstamp);

    char tmpbuf[SHMSGLEN + 1];
    strncpy(tmpbuf, msg, SHMSGLEN);
    tmpbuf[SHMSGLEN] = 0;
    char *msgptr; /* K.S. 14-Jan-8, pointer to phase error message */

    if(strstr(tmpbuf, LOCK_MSG))
      {
        gps_msg_received = true;
        gps_in_lock = true;
      }
    else if(strstr(tmpbuf, UNLOCK_MSG))
      {
        gps_msg_received = true;
        gps_in_lock = false;
      }
    else if(strstr(tmpbuf, PHASE_ERROR_MSG))  /* K.S. 14-Jan-8 */
      {
        gps_msg_received = true;
        gps_in_lock = true;
        msgptr = strstr( tmpbuf, PHASE_ERROR_MSG ) + strlen(PHASE_ERROR_MSG);
        if  (sscanf(msgptr,"%d",&gps_phase_error) != 1)  {
            gps_phase_error = 1000;  /* don't know anything better */
        } /*endif*/
      }
    
    if(log_soh)
      {
        string msgout;
        char* p = tmpbuf;
        while(*p)
          {
            int msglen = strcspn(p, SEED_NEWLINE);
            int seplen = strspn(p + msglen, SEED_NEWLINE);
            string msgline(p, msglen);
            if(msgout.length() + plugin_id.length() + msgline.length() + 5 > PLUGIN_MAX_MSG_SIZE)
              {
#ifndef TEST
                int r = send_log3(name.data(), &pt, "%s", msgout.c_str());
#else
                int r = 1;
#endif
                if(r < 0) throw PluginBrokenLink(strerror(errno));
                else if(r == 0) throw PluginBrokenLink();

                msgout = string();
              }

            msgout += (plugin_id + ": " + msgline + SEED_NEWLINE);
            p += (msglen + seplen);
          }

        if(msgout.length() > 0)
          {
#ifndef TEST
            int r = send_log3(name.data(), &pt, "%s", msgout.c_str());
#else
            int r = 1;
#endif
            if(r < 0) throw PluginBrokenLink(strerror(errno));
            else if(r == 0) throw PluginBrokenLink();
          }
      }
  }

void ReftekUnit::proc_pkt(UINT8 *pkt)
  {
    last_recv = time(NULL);

    if(reftek_type(pkt) == REFTEK_DT)
      {
        struct reftek_dt dt;
        if(!reftek_dt(&dt, pkt, TRUE))
          {
            logs(LOG_ERR) << "can't decode DT packet" << endl;
            return;
          }

        if(dt.dcerr)
            logs(LOG_WARNING) << "decompression error" << endl;

        if(mapped) proc_DT(dt);
      }
    else if(reftek_type(pkt) == REFTEK_SH)
      {
        struct reftek_sh sh;
        if(!reftek_sh(&sh, pkt))
          {
            logs(LOG_ERR) << "can't decode SH packet" << endl;
            return;
          }

        if(mapped) proc_SH(sh, (char *)(pkt + SHMSGOFF));
      }
    else
      {
        logs(LOG_WARNING) << "received unexpected packet (type "
          << string((char *)pkt, 2) << ")" << endl;
      }
  }
        
//*****************************************************************************
// RTPDConnection
//*****************************************************************************

class RTPDConnection
  {
  private:
    const string host;
    const int port;
    const int default_tq;
    const int unlock_tq;
    const bool log_soh;
    RTP* rtp;
    time_t last_poll;
    const Unit2StationMap *unit_map;
    map<int, rc_ptr<ReftekUnit> > reftek_units;

    void rtpd_connect(int retry);
    void rtpd_poll(int tmo);
    void rtpd_disconnect();

  public:
    RTPDConnection(const string &host_init, int port_init, int retry, int tmo,
      int default_tq_init, int unlock_tq_init, bool log_soh_init,
      const Unit2StationMap *unit_map_init = NULL);

    ~RTPDConnection();
  };

void RTPDConnection::rtpd_connect(int retry)
  {
    rtp_attr rtpattr = RTP_DEFAULT_ATTR;

    rtpattr.at_block = false;
    rtpattr.at_pmask = RTP_PMASK_DT | RTP_PMASK_SH;

    if((rtp = rtp_open((char *)host.c_str(), port, &rtpattr, retry)) == NULL)
        throw PluginRTPDError("could not connect RTPD");

    logs(LOG_NOTICE) << "[" << host << ":" << port << "] connected" << endl;

    if(!rtp_getattr(rtp, &rtpattr))
        throw PluginRTPDError("could not get connection attributes");

    logs(LOG_DEBUG) << hex << setfill('0')
      << "DAS mask               (at_dasid)  = " << setw(4) << rtpattr.at_dasid << endl
      << "Packet mask            (at_pmask)  = " << setw(4) << rtpattr.at_pmask << endl
      << "Stream mask            (at_smask)  = " << setw(4) << rtpattr.at_smask << endl
      << dec
      << "Socket I/O timeout     (at_timeo)  = " << rtpattr.at_timeo << endl
      << "TCP/IP transmit buffer (at_sndbuf) = " << rtpattr.at_sndbuf << endl
      << "TCP/IP receive  buffer (at_rcvbuf) = " << rtpattr.at_rcvbuf << endl
      << "blocking I/O flag      (at_block)  = " << (rtpattr.at_block? "true": "false") << endl;
  }

void RTPDConnection::rtpd_disconnect()
  {
    if(rtp != NULL)
      {
        logs(LOG_NOTICE) << "[" << host << ":" << port << "] closing connection"
          << endl;

        rtp_close(rtp);
        rtp = NULL;
      }
  }

void RTPDConnection::rtpd_poll(int tmo)
  {
    UINT8 buf[RTP_MAXMSGLEN];
    INT32 nbytes;
    
    if(!rtp_daspkt(rtp, buf, &nbytes))
        throw PluginRTPDBrokenConnection("error receiving data from RTPD",
          rtp_errno(rtp));

    if(nbytes > 0)
      {
        UINT16 exp, unit, seqno;
        DOUBLE tstamp;
        
        reftek_com(buf, &exp, &unit, &seqno, &tstamp);
        
        map<int, rc_ptr<ReftekUnit> >::iterator p = reftek_units.find(unit);
        if(p == reftek_units.end())
          {
            // The passed station name for Seedlink. Default is a string
            // representation of the unit id
            string name;
            char unit_id[5];
            bool mapped = false;
            snprintf(unit_id, 5, "%04X", unit);
            name = unit_id;

            // Resolve mapping from unit to station name. If not given, use
            // the unit-id as name
            if ( unit_map != NULL )
              {
                Unit2StationMap::const_iterator mit = unit_map->find(unit);
                if ( mit != unit_map->end() )
                  {
                    mapped = true;
                    name = mit->second;
                  }
              }

            logs(LOG_NOTICE) << "unit " << hex << uppercase << setfill('0')
              << setw(4) << unit << " detected" << dec;
            if(mapped)
              logs(LOG_NOTICE) << " mapped to " << name;
            else
              logs(LOG_NOTICE) << " (unmapped)";
            logs(LOG_NOTICE) << endl;

            p = reftek_units.insert(make_pair(unit,
              new ReftekUnit(name, mapped, default_tq, unlock_tq, log_soh))).first;
          }

        p->second->proc_pkt(buf);
      }

    int time_now = time(NULL);
    if(time_now != last_poll)
      {
        map<int, rc_ptr<ReftekUnit> >::iterator p = reftek_units.begin();
        while(p != reftek_units.end())
          {
            if(p->second->last_packet_received() < time_now - tmo)
              {
                logs(LOG_NOTICE) << "unit " << hex << uppercase << setfill('0')
                  << setw(4) << p->first << " removed" << dec << endl;

                reftek_units.erase(p++);
                continue;
              }

            ++p;
          }
      }
  }

RTPDConnection::RTPDConnection(const string &host_init, int port_init,
  int retry, int tmo, int default_tq_init, int unlock_tq_init,
  bool log_soh_init, const Unit2StationMap *unit_map_init):
  host(host_init), port(port_init),  default_tq(default_tq_init),
  unlock_tq(unlock_tq_init), log_soh(log_soh_init), rtp(NULL),
  last_poll(0), unit_map(unit_map_init)
  {
    logs(LOG_INFO) << "[" << host << ":" << port << "] "
      "attempting connection" << endl;

    rtpd_connect(retry);

    while(!terminate_proc)
      {
        try
          {
            rtpd_poll(tmo);
          }
        catch(PluginRTPDBrokenConnection &e)
          {
            if(e.severity > retry)
                throw;

            logs(LOG_WARNING) << e.what() << endl;
            rtpd_disconnect();
            
            logs(LOG_INFO) << "[" << host << ":" << port << "] "
              "attempting to reconnect" << endl;

            rtpd_connect(retry);
          }
      }
  }

RTPDConnection::~RTPDConnection()
  {
    logs(LOG_NOTICE) << "shutdown" << endl;

    if(rtp != NULL)
      {
        rtp_break(rtp);
        rtpd_disconnect();
      }
  }

//*****************************************************************************
// get_progname()
//*****************************************************************************

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
// RetryAttribute
//*****************************************************************************

class RetryAttribute: public CfgAttribute
  {
  private:
    int &valref;

  public:
    RetryAttribute(const string &name, int &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value);
  };

bool RetryAttribute::assign(ostream &cfglog, const string &value)
  {
    if(!strcasecmp(value.c_str(), "never"))
      {
        valref = RTP_ERR_NONE;
        return true;
      }
    
    if(!strcasecmp(value.c_str(), "transient"))
      {
        valref = RTP_ERR_TRANSIENT;
        return true;
      }
    
    if(!strcasecmp(value.c_str(), "nonfatal"))
      {
        valref = RTP_ERR_NONFATAL;
        return true;
      }

    cfglog << "[" << item_name << "] value must be 'never', 'transient' or "
      "'nonfatal'" << endl;

    return false;
  }

//*****************************************************************************
// Main
//*****************************************************************************

int main(int argc, char **argv)
try
  {
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    struct option ops[] = 
      {
        { "verbosity",      required_argument, NULL, 'X' },
        { "daemon",         no_argument,       NULL, 'D' },
        { "config-file",    required_argument, NULL, 'f' },
        { "map-file",       required_argument, NULL, 'm' },
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    string config_file = CONFIG_FILE;
    string map_file;

    int c;
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    while((c = getopt_long(argc, argv, "vDf:m:Vh", ops, NULL)) != EOF)
#else
    while((c = getopt(argc, argv, "vDf:m:Vh")) != EOF)
#endif
      {
        switch(c)
          {
          case 'v': ++verbosity; break;
          case 'X': verbosity = atoi(optarg); break;
          case 'D': daemon_mode = true; break;
          case 'f': config_file = optarg; break;
          case 'm': map_file = optarg; break;
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

    plugin_id = string(argv[optind]);
    
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGINT, &sa, NULL));
    N(sigaction(SIGTERM, &sa, NULL));
    
    sa.sa_handler = SIG_IGN;
    N(sigaction(SIGHUP, &sa, NULL));
    N(sigaction(SIGPIPE, &sa, NULL));
    
    if(daemon_mode)
      {
        logs(LOG_INFO) << ident_str << " started" << endl;
        logs(LOG_INFO) << "take a look into syslog files for more messages" << endl;

        openlog(plugin_id.c_str(), 0, SYSLOG_FACILITY);
        daemon_init = true;
      }

    redirect_ostream(cout, LogFunc(), LOG_INFO);
    redirect_ostream(cerr, LogFunc(), LOG_ERR);
    redirect_ostream(clog, LogFunc(), LOG_ERR);

    logs(LOG_NOTICE) << ident_str << " started" << endl;
    
    string host = "localhost";
    int port = RTP_DEFAULT_PORT;
    int retry = RTP_ERR_NONFATAL;
    int tmo = TMO_DEF;
    int default_tq = DEFAULT_TQ;
    int unlock_tq = UNLOCK_TQ;
    bool log_soh = true;

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("host", host));
    atts->add_item(IntAttribute("port", port, 1, 65535));
    atts->add_item(RetryAttribute("retry", retry));
    atts->add_item(IntAttribute("timeout", tmo, TMO_MIN,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("default_tq", default_tq, 0, 100));
    atts->add_item(IntAttribute("unlock_tq", unlock_tq, -1, 100));
    atts->add_item(BoolAttribute("log_soh", log_soh, "true", "false"));
    
    logs(LOG_INFO) << "loading configuration from file '" << config_file << "'"
      << endl;

    read_config_ini(config_file, plugin_id, atts, new CfgElementMap);

    Unit2StationMap unit_map;
    if ( !map_file.empty() )
      {
        logs(LOG_INFO) << "loading map from file '" << map_file << "'"
          << endl;

        read_map_file(unit_map, map_file);
      }

    RTPDConnection conn(host, port, retry, tmo, default_tq, unlock_tq,
      log_soh, unit_map.empty()?NULL:&unit_map);
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
 
