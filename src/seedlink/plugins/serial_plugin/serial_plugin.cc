/*****************************************************************************
 * serial_plugin.cc
 *
 * SeedLink plugin for serial digitizers
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
#include <string>
#include <vector>
#include <set>
#include <cstring>
#include <cstdio>
#include <csignal>
#include <cerrno>

#include <unistd.h>
#include <syslog.h>
#include <termios.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__) || defined(__APPLE__)
#include <getopt.h>
#endif


#include "qutils.h"

#include "confbase.h"
#include "conf_ini.h"
#include "confattr.h"
#include "cppstreams.h"
#include "utils.h"
#include "plugin.h"
#include "serial_plugin.h"
#include "diag.h"

#define MYVERSION "2.2 (2010.256)"

#ifndef CONFIG_FILE
#define CONFIG_FILE "/home/sysop/config/plugins.ini"
#endif

#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_LOCAL0
#endif

namespace SeedlinkPlugin_private {

using namespace std;
using namespace CfgParser;
using namespace CPPStreams;
using namespace Utilities;

const char *const ident_str = "SeedLink Serial Digitizer Plugin v" MYVERSION;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__) || defined(__APPLE__)
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
    "-m, --list-modules            List supported digitizers/protocols\n"
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
    "-m             List supported digitizers/protocols\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

string station_name;
enum PortType { Serial_Port, FIFO_Port, TCP_Port };
PortType port_type = Serial_Port;

int verbosity = 0;
bool daemon_mode = false, daemon_init = false;
string plugin_name;
ostream seed_log(NULL);
rc_ptr<Proto> proto;
DigitizerTime digitime;
DigitizerConfig dconf;
struct sockaddr_in inet_addr;
bool tcp_connected = false;
volatile sig_atomic_t terminate_proc = 0;

void int_handler(int sig)
  {
    terminate_proc = 1;
  }

//*****************************************************************************
// SystemLog
//*****************************************************************************

class SystemLog
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
            string msgout = string(p, strlen(p) - 1) + " - " + plugin_name + ": " + msg;
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
        if(!digitime.valid) return msg.length();

        std::string msgout;
        int n = 0, msglen, seplen;
        while(msglen = strcspn(msg.c_str() + n, SEED_NEWLINE),
          seplen = strspn(msg.c_str() + n + msglen, SEED_NEWLINE))
          {
            std::string msgline(msg, n, msglen);
            if(msgout.length() + plugin_name.length() + msgline.length() + 5 > PLUGIN_MAX_MSG_SIZE)
                break;

            msgout += (plugin_name + ": " + msgline + SEED_NEWLINE);
            n += (msglen + seplen);
          }

        if(n == 0)
          {
            msgout = plugin_name + ": " + msg;
            n = msg.length();
          }

        EXT_TIME et = int_to_ext(digitime.it);

        struct ptime pt;
        pt.year = et.year;
        pt.yday = et.doy;
        pt.hour = et.hour;
        pt.minute = et.minute;
        pt.second = et.second;
        pt.usec = et.usec;

        int r = send_log3(station_name.c_str(), &pt, "%s", msgout.c_str());

        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();

        return n;
      }
  };

#ifndef HAVE_CFMAKERAW
//*****************************************************************************
// cfmakeraw()
//*****************************************************************************

void cfmakeraw(struct termios *termios_p)
  {
    termios_p->c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
                    /* echo off, canonical mode off, extended input
                       processing off, signal chars off */

    termios_p->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
                    /* no SIGINT on BREAK, CR-to-NL off, input parity
                       check off, don't strip 8th bit on input,
                       output flow control off */

    termios_p->c_cflag &= ~(CSIZE | PARENB);
                    /* clear size bits, parity checking off */
    termios_p->c_cflag |= CS8;
                    /* set 8 bits/char */

    termios_p->c_oflag &= ~(OPOST);
                    /* output processing off */

    termios_p->c_cc[VMIN] = 1; /* Case B: 1 byte at a time, no timer */
    termios_p->c_cc[VTIME] = 0;
  }
#endif

//*****************************************************************************
// open_port()
//*****************************************************************************

int open_dev(mode_t mode)
  {
    static const long bpsvalue[] = { 2400, 9600, 19200, 38400, 57600, 115200, -1 };
    static const tcflag_t bpsflag[] = { B2400, B9600, B19200, B38400, B57600, B115200 };
    int fd, bpsidx;
    struct termios tio;

    if((fd = open(dconf.port_name.c_str(), mode | O_NONBLOCK)) < 0)
        throw PluginCannotOpenPort(dconf.port_name);

    if(tcgetattr(fd, &tio) < 0)
      {
        port_type = FIFO_Port;   // probably
        return fd;
      }

    port_type = Serial_Port;

    cfmakeraw(&tio);
    tio.c_cflag |= CLOCAL | CREAD | HUPCL;
    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;

    for(bpsidx = 0; bpsvalue[bpsidx] != -1; ++bpsidx)
        if(dconf.port_bps == bpsvalue[bpsidx]) break;

    if(bpsvalue[bpsidx] < 0)
      {
        close(fd);
        throw PluginInvalidBaudrate(dconf.port_name, dconf.port_bps);
      }

    cfsetospeed(&tio, bpsflag[bpsidx]);
    cfsetispeed(&tio, bpsflag[bpsidx]);
    N(tcsetattr(fd, TCSANOW, &tio));
    return fd;
  }

int open_tcp(int fd)
  {
    int sock;
    N(sock = socket(PF_INET, SOCK_STREAM, 0));
    fcntl(sock, F_SETFL, O_NONBLOCK);

    if(connect(sock, (struct sockaddr *) &inet_addr, sizeof(inet_addr)) < 0 &&
      errno != EINPROGRESS && errno != EAGAIN)
      {
        logs(LOG_WARNING) << dconf.port_name << ": " << strerror(errno) << endl;
      }

    if(fd != -1)
      {
        if(fd != sock)
          {
            N(dup2(sock, fd));
            close(sock);
          }

        return fd;
      }

    return sock;
  }

int open_port(mode_t mode)
  {
    if(port_type == TCP_Port)
        return open_tcp(-1);

    return open_dev(mode);
  }

//*****************************************************************************
// sync_data()
//*****************************************************************************

// read and throw away all pending data

void sync_data(int fd)
  {
    int r;
    char buf[1024];
    fd_set fds;
    struct timeval tv;

    while(1)
      {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        if(port_type != Serial_Port)
          {
            tv.tv_sec = 1;
            tv.tv_usec = 0;
          }
        else
          {
            tv.tv_sec = 0;
            tv.tv_usec = 20000000 / dconf.port_bps;
          }

        if((r = select(fd + 1, &fds, NULL, NULL, &tv)) < 0)
          {
            if(errno == EINTR) continue;
            throw PluginLibraryError("select error");
          }

        if(r == 0) return;

        if(read(fd, buf, 1024) < 0)
            throw PluginLibraryError("read error");
      }
  }

//*****************************************************************************
// read_port()
//*****************************************************************************

ssize_t read_port(int fd, void *vptr, size_t n)
  {
    ssize_t nread;
    size_t nleft = n;
    char *ptr = static_cast<char *>(vptr);
    int r;
    fd_set fds;
    struct timeval tv;

    while(nleft > 0)
      {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        tv.tv_sec = 10;
        tv.tv_usec = 0;

        if((r = select(fd + 1, &fds, NULL, NULL, &tv)) < 0)
          {
            if(errno == EINTR) continue;
            throw PluginLibraryError("select error");
          }

#if 0
        if(r == 0)
          {
            if(port_type == TCP_Port)
              {
                if(tcp_connected)
                  {
                    logs(LOG_NOTICE) << dconf.port_name << ": " <<
                      "Connection closed" << endl;
                  }

                close(fd);
                open_tcp(fd);
                tcp_connected = false;
              }

            return 0;
          }
#endif

        if(!FD_ISSET(fd, &fds))
            continue;

        nread = read(fd, ptr, nleft);

        if(port_type == TCP_Port)
          {
            if(nread < 0)
              {
                logs(LOG_WARNING) << dconf.port_name << ": " <<
                  strerror(errno) << endl;

                if(tcp_connected)
                  {
                    logs(LOG_NOTICE) << dconf.port_name << ": " <<
                      "Connection closed" << endl;
                  }

                close(fd);
                sleep(10);
                open_tcp(fd);
                tcp_connected = false;
                return 0;
              }
            else if(nread == 0)
              {
                if(tcp_connected)
                  {
                    logs(LOG_NOTICE) << dconf.port_name << ": " <<
                      "Connection closed" << endl;
                  }

                close(fd);
                sleep(10);
                open_tcp(fd);
                tcp_connected = false;
                return 0;
              }
            else if(!tcp_connected)
              {
                logs(LOG_NOTICE) << dconf.port_name << " " << "connected" << endl;
                tcp_connected = true;
              }
          }
        else
          {
            if(nread < 0) throw PluginReadError(dconf.port_name, strerror(errno));
            else if(nread == 0) throw PluginReadError(dconf.port_name);
          }

        nleft -= nread;
        ptr += nread;
      }

    return(n);
  }

//*****************************************************************************
// ChannelDef -- definition of element "channel"
//*****************************************************************************

class ChannelDef: public CfgElement
  {
  private:
    string statid;
    string chanid;
    bool first;
    string source_id;
    double scale;
    double realscale;
    double realoffset;
    string realunit;
    int precision;
    set<string> channels_defined;

  public:
    ChannelDef(const string &name): CfgElement(name), first(true), scale(-1.0),
      realscale(-1.0), precision(2) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog, const string &name);
    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> ChannelDef::start_attributes(ostream &cfglog, const string &name)
try
  {
    if(first)
      {
        first = false;

        if(station_name.length() == 0)
          {
            cfglog << "station name was not specified" << endl;
            return NULL;
          }

        if(dconf.proto_name.length() == 0)
          {
            cfglog << "protocol was not specified" << endl;
            return NULL;
          }

        if(dconf.port_name.length() == 0)
          {
            cfglog << "port name was not specified" << endl;
            return NULL;
          }

        proto = RegisteredModule<Proto>::instance(dconf.proto_name);
        if(proto == NULL)
          {
            cfglog << "protocol " << dconf.proto_name << " is not supported"
              << endl;
            return NULL;
          }
      }

    if(proto == NULL)
      {
        return NULL;
      }

    if(channels_defined.find(name) != channels_defined.end())
      {
        cfglog << "channel " << name << " is already defined" << endl;
        return NULL;
      }

    statid = station_name;
    chanid = name;
    source_id = name;
    scale = -1.0;
    realscale = -1.0;
    realoffset = 0;
    realunit = "";
    precision = 2;

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("station", statid));
    atts->add_item(StringAttribute("source_id", source_id));
    atts->add_item(FloatAttribute("scale", scale));
    atts->add_item(FloatAttribute("realscale", realscale));
    atts->add_item(FloatAttribute("realoffset", realoffset));
    atts->add_item(StringAttribute("realunit", realunit));
    atts->add_item(IntAttribute("precision", precision, 0, 10));
    return atts;
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
    return NULL;
  }

void ChannelDef::end_attributes(ostream &cfglog)
try
  {
    if(source_id.length() == 0)
      {
        cfglog << "source ID was not specified for channel " << chanid << endl;
        return;
      }

    proto->attach_output_channel(source_id, chanid, statid, scale,
      realscale, realoffset, realunit, precision);

    channels_defined.insert(chanid);
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
  }

//*****************************************************************************
// Loading config file
//*****************************************************************************

void configure_plugin(const string &config_file)
  {
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("station", station_name));
    atts->add_item(StringAttribute("port", dconf.port_name));
    atts->add_item(IntAttribute("bps", dconf.port_bps, 0, IntAttribute::lower_bound));
    atts->add_item(StringAttribute("protocol", dconf.proto_name));
    atts->add_item(IntAttribute("checksum", dconf.checksum_used, 0, 1));
    atts->add_item(IntAttribute("pctime", dconf.use_pctime_if_no_gps, 0, 1));
    atts->add_item(IntAttribute("lsb", dconf.lsb, 0, 24));
    atts->add_item(IntAttribute("statusinterval", dconf.statusinterval, 0, 24*60));
    atts->add_item(IntAttribute("time_offset", dconf.time_offset));
    atts->add_item(IntAttribute("zero_sample_limit", dconf.zero_sample_limit,
      -1, 5, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("default_tq", dconf.default_tq, -1, 100));
    atts->add_item(IntAttribute("unlock_tq", dconf.unlock_tq, -1, 100));
    atts->add_item(IntAttribute("bundles", dconf.nbundles, 1, 59));
    atts->add_item(StringAttribute("soh_log_dir", dconf.soh_log_dir));
    atts->add_item(IntAttribute("baseaddr", dconf.baseaddr, 0, 65535));

    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(ChannelDef("channel"));

    logs(LOG_INFO) << "loading configuration from file '" << config_file << "'" << endl;
    read_config_ini(config_file, plugin_name, atts, elms);
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

} // namespace SeedlinkPlugin_private

using namespace SeedlinkPlugin_private;

namespace CPPStreams {

Stream logs = make_stream(SystemLog());

}


#if defined(__APPLE__)
template<>
RegisteredModule<Proto>* RegisteredModule<Proto>::registered = NULL;
#else
namespace PluginModule {

template<>
RegisteredModule<Proto>* RegisteredModule<Proto>::registered = NULL;

}
#endif

//*****************************************************************************
// Main
//*****************************************************************************

int main(int argc, char **argv)
try
  {
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__) || defined(__APPLE__)
    struct option ops[] =
      {
        { "verbosity",      required_argument, NULL, 'X' },
        { "daemon",         no_argument,       NULL, 'D' },
        { "config-file",    required_argument, NULL, 'f' },
        { "list-modules",   no_argument,       NULL, 'm' },
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    string config_file = CONFIG_FILE;

    int c;
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__) || defined(__APPLE__)
    while((c = getopt_long(argc, argv, "vDf:mVh", ops, NULL)) != EOF)
#else
    while((c = getopt(argc, argv, "vDf:mVh")) != EOF)
#endif
      {
        switch(c)
          {
          case 'v': ++verbosity; break;
          case 'X': verbosity = atoi(optarg); break;
          case 'D': daemon_mode = true; break;
          case 'f': config_file = optarg; break;
          case 'm': cout << ident_str << endl;
                    list_modules<Proto>("Supported protocols");
                    exit(0);
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

    plugin_name = string(argv[optind]);

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
        openlog(plugin_name.c_str(), 0, SYSLOG_FACILITY);
        daemon_init = true;
      }

    redirect_ostream(cout, SystemLog(), LOG_INFO);
    redirect_ostream(cerr, SystemLog(), LOG_ERR);
    redirect_ostream(clog, SystemLog(), LOG_ERR);
    redirect_ostream(seed_log, SEEDLog(), 0);

    logs(LOG_NOTICE) << ident_str << " started" << endl;

    init_qlib2(0);

    configure_plugin(config_file);

    if(proto == NULL)
      {
        logs(LOG_ERR) << "fatal config errors detected" << endl;
        return 1;
      }

    if(!strncasecmp(dconf.port_name.c_str(), "tcp://", 6))
      {
        port_type = TCP_Port;

        string::size_type port_sep;
        if((port_sep = dconf.port_name.find(':', 6)) == string::npos)
            throw PluginInvalidPort(dconf.port_name);

        string host_name(dconf.port_name, 6, port_sep - 6);
        string tcp_port_str(dconf.port_name, port_sep + 1, string::npos);

        char *tail;
        unsigned short int tcp_port;
        tcp_port = strtoul(tcp_port_str.c_str(), &tail, 10);
        if(*tail) throw PluginInvalidPort(dconf.port_name);

        struct hostent *hostinfo = NULL;
        if((hostinfo = gethostbyname(host_name.c_str())) == NULL)
          {
            logs(LOG_ERR) << "unknown host: " << host_name << endl;
            return 1;
          }
        else
          {
            memset(&inet_addr, 0, sizeof(inet_addr));
            inet_addr.sin_family = AF_INET;
            inet_addr.sin_port = htons(tcp_port);
            inet_addr.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];
          }
      }

    digitime.quality = dconf.default_tq;
    digitime.valid = false;
    digitime.exact = false;

    proto->start();
    proto->flush_channels();
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

