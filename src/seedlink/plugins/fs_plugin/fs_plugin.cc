/***************************************************************************** 
 * fs_plugin.cc
 *
 * SeedLink plugin that reads data from filesystem
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
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
#include <sys/types.h>
#include <sys/stat.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#include "confbase.h"
#include "conf_ini.h"
#include "confattr.h"
#include "cppstreams.h"
#include "utils.h"
#include "plugin.h"
#include "fs_plugin.h"
#include "diag.h"

#define MYVERSION "1.0 (2019.199)"

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

const char *const SEED_NEWLINE = "\r\n";
const char *const ident_str    = "SeedLink FS-Plugin v" MYVERSION;

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
    "-m, --list-modules            List available modules\n"
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
    "-m             List available modules\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

string station_name;
string network_name = "XX";
string input_type;
string data_format;
string location;
string file_pattern;
string timestamp_file;
set<string> accepted_stations;
int default_timing_quality = -1;

int polltime = 1;
int file_read_delay = 0;
int verbosity = 0;
int zero_sample_limit = -1;
int scan_level = 2;
bool move_files = false;
bool delete_files = false;
bool use_timestamp = true;
string plugin_name;
ostream seed_log(NULL);
DigitizerTime digitime;
rc_ptr<FS_Input> input;
rc_ptr<FS_Decoder> decoder;
bool daemon_mode = false, daemon_init = false;
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

//*****************************************************************************
// ChannelDef -- definition of element "channel"
//*****************************************************************************

class ChannelDef: public CfgElement
  {
  private:
    string channel_name;
    string source_id;
    set<string> channels_defined;
    
  public:
    ChannelDef(const string &name): CfgElement(name) {}
    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &name);
    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> ChannelDef::start_attributes(ostream &cfglog,
  const string &name)
try
  {
    if(channels_defined.find(name) != channels_defined.end())
      {
        cfglog << "channel " << name << " is already defined" << endl;
        return NULL;
      }

    channel_name = name;
    source_id = "";
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("source_id", source_id));
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
        cfglog << "source ID was not specified for channel " << channel_name << endl;
        return;
      }

    decoder->attach_output_channel(source_id, channel_name);
    channels_defined.insert(channel_name);
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
  }

//*****************************************************************************
// SectionDef -- definition of section element
//*****************************************************************************

class SectionDef: public CfgElement
  {
  private:
    const string want;
    bool &found;
    bool dup;
    bool notmysection;
    list<string> station_list;
    const rc_ptr<CfgAttributeMap> attributes;
    const rc_ptr<CfgElementMap> children;

  public:
    SectionDef(const string &want_init, bool &found_init):
      CfgElement("section"), want(want_init), found(found_init), dup(false),
      notmysection(false) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &name);
    void end_attributes(ostream &cfglog);
    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &name);
  };

rc_ptr<CfgAttributeMap> SectionDef::start_attributes(ostream &cfglog,
  const string &name)
  {
    notmysection = strcasecmp(want.c_str(), name.c_str());
    
    if(notmysection) return NULL;
    
    if(found)
      {
        cfglog << "duplicate section '" << name << "'" << endl;
        dup = true;
        return NULL;
      }

    found = true;

    station_list.clear();

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("station", station_name));
    atts->add_item(StringListAttribute("station_list", station_list, ","));
    atts->add_item(StringAttribute("network", network_name));
    atts->add_item(StringAttribute("input_type", input_type));
    atts->add_item(StringAttribute("data_format", data_format));
    atts->add_item(StringAttribute("location", location));
    atts->add_item(StringAttribute("pattern", file_pattern));
    atts->add_item(StringAttribute("timestamp_file", timestamp_file));
    atts->add_item(IntAttribute("default_timing_quality",
      default_timing_quality, -1, 100));
    atts->add_item(IntAttribute("zero_sample_limit", zero_sample_limit, -1, 5,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("delay", file_read_delay, 0, 1000));
    atts->add_item(IntAttribute("polltime", polltime, 1,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("verbosity", verbosity, 0,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("scan_level", scan_level, 1, 2));
    atts->add_item(BoolAttribute("move_files", move_files, "yes", "no"));
    atts->add_item(BoolAttribute("delete_files", delete_files, "yes", "no"));
    atts->add_item(BoolAttribute("use_timestamp", use_timestamp, "yes", "no"));
    
    return atts;
  }

void SectionDef::end_attributes(ostream &cfglog)
  {
    if(notmysection) return;
    
//    if(station_name.length() == 0)
//      {
//        cfglog << "station name was not specified" << endl;
//        return;
//      }
    
    if(input_type.length() == 0)
      {
        cfglog << "input type was not specified" << endl;
        return;
      }
    
    if(data_format.length() == 0)
      {
        cfglog << "data format was not specified" << endl;
        return;
      }
    
    if(location.length() == 0)
      {
        cfglog << "location was not specified" << endl;
        return;
      }
    
    if((decoder = RegisteredModule<FS_Decoder>::instance(data_format)) == NULL)
      {
        cfglog << "data format '" << data_format << "' is not supported"
          << endl;
        return;
      }

    if((input = RegisteredModule<FS_Input>::instance(input_type)) == NULL)
      {
        cfglog << "input type '" << input_type << "' is not supported"
          << endl;
        return;
      }

    input->init(decoder);

    list<string>::iterator p;
    for(p = station_list.begin(); p != station_list.end(); ++p)
        accepted_stations.insert(*p);
  }

rc_ptr<CfgElementMap> SectionDef::start_children(ostream &cfglog,
  const string &name)
  {
    if(dup || decoder == NULL || input == NULL ||
      strcasecmp(want.c_str(), name.c_str()))
      return NULL;
    
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(ChannelDef("channel"));

    return elms;
  }

//*****************************************************************************
// Loading config file
//*****************************************************************************

void configure_plugin(const string &config_file)
  {
    bool found = false;
    logs(LOG_INFO) << "loading configuration from file '" << config_file
      << "'" << endl;
    read_config_ini(config_file, SectionDef(plugin_name, found));
    
    if(!found) throw CfgCannotFindSection(plugin_name, config_file);
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

//*****************************************************************************
// load_timestamp()
//*****************************************************************************

void load_timestamp()
  {
    if(timestamp_file.length() == 0) return;
    
    FILE *f;
    
    if((f = fopen(timestamp_file.c_str(), "r")) == NULL) return;

    unsigned long t1;
    unsigned long t2;
    if(fscanf(f, "%lu.%lu", &t1, &t2) != 2)
      {
        logs(LOG_WARNING) << "could not read timestamp from file '"
          << timestamp_file << "'" << endl;
        return;
      }

    input->set_timestamp(Timestamp(t1, t2));
    fclose(f);
  }

//*****************************************************************************
// save_timestamp()
//*****************************************************************************

void save_timestamp()
  {
    if(timestamp_file.length() == 0) return;
    
    FILE *f;
    
    if((f = fopen(timestamp_file.c_str(), "w")) == NULL)
      {
        logs(LOG_WARNING) << "could not open file '" << timestamp_file
          << "' for writing " << endl;
        return;
      }

    Timestamp tstmp = input->get_timestamp();
    fprintf(f, "%lu.%06lu", tstmp.sec, tstmp.usec);
    fclose(f);
  }

} // namespace SeedlinkPlugin_private

using namespace SeedlinkPlugin_private;

namespace CPPStreams {

Stream logs = make_stream(SystemLog());

}

namespace PluginModule {

template<>
RegisteredModule<FS_Input>* RegisteredModule<FS_Input>::registered = NULL;

template<>
RegisteredModule<FS_Decoder>* RegisteredModule<FS_Decoder>::registered = NULL;

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
        { "list-modules",   no_argument,       NULL, 'm' },
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    string config_file = CONFIG_FILE;
    
    int c;
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
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
                    list_modules<FS_Input>("Available input modules");
                    list_modules<FS_Decoder>("Available format modules");
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
    
    configure_plugin(config_file);

    if(input == NULL || decoder == NULL)
      {
        logs(LOG_ERR) << "fatal config errors detected" << endl;
        return 1;
      }
    
    load_timestamp();
    while(!terminate_proc)
      {
        input->check_for_new_data(location, file_pattern);
        save_timestamp();
        sleep(polltime);
      }
    
    decoder->flush_channels();
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
 
