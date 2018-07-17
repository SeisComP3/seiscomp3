/***************************************************************************** 
 * chain_plugin.cc
 *
 * SeedLink chain plugin
 *
 * (c) 2003 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <cstring>
#include <cstdio>
#include <csignal>
#include <cerrno>

#include <unistd.h>
#include <syslog.h>
#include <termios.h>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#include "qdefines.h"
#include "qutils.h"
#include "qtime.h"

#include "libslink.h"

#include "confbase.h"
#include "conf_xml.h"
#include "confattr.h"
#include "cppstreams.h"
#include "utils.h"
#include "plugin.h"
#include "plugin_exceptions.h"
#include "descriptor.h"
#include "diag.h"

#include "schedule.h"

#define MYVERSION "2.0 (2018.198)"

#ifndef CONFIG_FILE
#define CONFIG_FILE "/home/sysop/config/chain.xml"
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

const int POLL_USEC           = 1000000;
const int MAX_SAMPLES         = ((SLRECSIZE - 64) * 2);
const int SHUTDOWN_WAIT       = 10;
const int WRITE_TIMEOUT       = 600;
const int REGEX_ERRLEN        = 100;
const int REQLEN              = 100;
const int EXT_DATA_FD         = PLUGIN_FD - 1;
const int EXT_CMD_FD          = PLUGIN_FD - 2;
const int LIBSLINK_LOGPRIO    = LOG_INFO;
const int STATLEN             = 5;
const int NETLEN              = 2;
const int LOCLEN              = 2;
const int CHLEN               = 3;
const char *const SHELL       = "/bin/bash";

const char *const ident_str = "SeedLink Chain Plugin v" MYVERSION;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
const char *const opterr_message = "Try `%s --help' for more information\n";
const char *const help_message = 
    "Usage: %s [options] plugin_name\n"
    "\n"
    "'plugin_name' is used to identify the plugin in log messages\n"
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
    "'plugin_name' is used to identify the plugin in log messages\n"
    "\n"
    "-v             Increase verbosity level\n"
    "-D             Daemon mode\n"
    "-f FILE        Alternative configuration file\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

string plugin_name;
int verbosity = 0;
bool daemon_mode = false, daemon_init = false;
volatile sig_atomic_t terminate_proc = 0;

void int_handler(int sig)
  {
    terminate_proc = 1;
  }

//*****************************************************************************
// Some utility functions
//*****************************************************************************

void get_id(const sl_fsdh_s *fsdh, string &net, string &sta, string &loc,
  string &chn)
  {
    for(int n = NETLEN; n > 0; --n)
        if(fsdh->network[n - 1] != ' ')
          {
            net = string(fsdh->network, n);
            break;
          }
    
    for(int n = STATLEN; n > 0; --n)
        if(fsdh->station[n - 1] != ' ')
          {
            sta = string(fsdh->station, n);
            break;
          }
        
    for(int n = LOCLEN; n > 0; --n)
        if(fsdh->location[n - 1] != ' ')
          {
            loc = string(fsdh->location, n);
            break;
          }
    
    for(int n = CHLEN; n > 0; --n)
        if(fsdh->channel[n - 1] != ' ')
          {
            chn = string(fsdh->channel, n);
            break;
          }
  }

int packet_type2int(const char *type)
  {
    if(type == NULL) return SLNUM;
    else if(strlen(type) != 1) return -1;
    
    switch(toupper(*type))
      {
        case 'D': return SLDATA;
        case 'E': return SLDET;
        case 'T': return SLTIM;
        case 'C': return SLCAL;
        case 'L': return SLMSG;
        case 'O': return SLBLK;
      }

    return -1;
  }

const char *packet_type2string(int type)
  {
    switch(type)
      {
        case SLDATA: return "D";
        case SLDET: return "E";
        case SLTIM: return "T";
        case SLCAL: return "C";
        case SLMSG: return "L";
        case SLBLK: return "O";
      }

    return NULL;
  }

//*****************************************************************************
// Exceptions
//*****************************************************************************

class PluginCannotReadChild: public PluginError
  {
  public:
    PluginCannotReadChild():
      PluginError(string() + "cannot read data from child process") {}
  };

class PluginInvalidSchedule: public PluginError
  {
  public:
    PluginInvalidSchedule(const string &message):
      PluginError(message) {}
  };

class PluginInvalidRenameElement: public PluginError
  {
  public:
    PluginInvalidRenameElement():
      PluginError("invalid rename element") {}
  };

class PluginUnpackStreamAlreadyUsed: public PluginError
  {
  public:
    PluginUnpackStreamAlreadyUsed(const string &stream):
      PluginError(string() + "source " + stream + " is already used in another "
        "unpack element") {}
  };

class PluginUnpackInvalidSource: public PluginError
  {
  public:
    PluginUnpackInvalidSource(const string &stream):
      PluginError(string() + "unpack source " + stream + "is not valid") {}
  };

class PluginTriggerStreamAlreadyUsed: public PluginError
  {
  public:
    PluginTriggerStreamAlreadyUsed(const string &stream):
      PluginError(string() + "source " + stream + " is already used in another "
        "trigger element") {}
  };

class PluginTriggerInvalidSource: public PluginError
  {
  public:
    PluginTriggerInvalidSource(const string &stream):
      PluginError(string() + "trigger source " + stream + "is not valid") {}
  };

class PluginStreamAlreadyInTable: public PluginError
  {
  public:
    PluginStreamAlreadyInTable(const StreamDescriptor &strd):
      PluginError(string() + "multiple timetable entries for stream " +
        strd.to_string()) {}
  };

class PluginStreamInvalidTime: public PluginError
  {
  public:
    PluginStreamInvalidTime(const StreamDescriptor &strd):
      PluginError(string() + "timetable entry for stream " + strd.to_string() +
        " has invalid time") {}
  };

class PluginStationAlreadyDefined: public PluginError
  {
  public:
    PluginStationAlreadyDefined(const StationDescriptor &stad):
      PluginError(string() + "station " + stad.to_string() +
        " is already defined") {}
  };

class PluginStationIdAlreadyUsed: public PluginError
  {
  public:
    PluginStationIdAlreadyUsed(const string &id):
      PluginError(string() + "station ID " + id + " is already in use") {}
  };

class PluginRegexError: public PluginError
  {
  public:
    PluginRegexError(const string &errmsg):
      PluginError(string() + "regex error: " + errmsg) {}
  };

//*****************************************************************************
// Log stuff
//*****************************************************************************

// Everything goes through sl_log(), so our own messages will be in sync
// with internal messages of libslink

class LogFunc
  {
  private:
    static int prio;
    
  public:
    enum { msglen = 200 };

    static void helper(const char *msg);

    int operator()(int priority, const string &msg)
      {
        int verb = 2, level = 0;
        
        switch(priority)
          {
          case LOG_EMERG:
          case LOG_ALERT:
          case LOG_CRIT:
          case LOG_ERR:
            verb = -1; level = 1; break;

          case LOG_WARNING:
          case LOG_NOTICE:
            verb = 0; level = 0; break;

          case LOG_INFO:
            verb = 1; level = 0; break;

          case LOG_DEBUG:
            verb = 2; level = 0;
          }
        
        prio = priority;
        sl_log(level, verb, "%s", msg.c_str());
        prio = LIBSLINK_LOGPRIO;
        return msg.length();
      }
  };

int LogFunc::prio = LIBSLINK_LOGPRIO;

void LogFunc::helper(const char *msg)
  {
    if(daemon_init)
      {
        syslog(LogFunc::prio, "%s", msg);
      }
    else
      {
        time_t t = time(NULL);
        char *p = asctime(localtime(&t));
        string msgout = string(p, strlen(p) - 1) + " - " + plugin_name +
          ": " + msg;

        write(STDOUT_FILENO, msgout.c_str(), msgout.length());
      }
  }

void log_setup(int verb)
  {
    sl_loginit(verb, LogFunc::helper, NULL, LogFunc::helper, NULL);
  }

//*****************************************************************************
// StreamRenamer
//*****************************************************************************

class StreamRenamer
  {
  private:
    char from_loc[LOCLEN];
    char from_chn[CHLEN];
    char to_loc[LOCLEN];
    char to_chn[CHLEN];

    void init(const string &pattern, char *loc, char *chn);
    bool string_match(const char *tmpl, const char *str, int len) const;

  public:
    StreamRenamer(const string &from, const string &to)
      {
        init(from, from_loc, from_chn);
        init(to, to_loc, to_chn);
      }
    
    bool hit(sl_fsdh_s *fsdh);
  };
    
void StreamRenamer::init(const string &pattern, char *loc, char *chn)
  {
    const char* p = pattern.c_str();
    int len = pattern.length();

    if(len == LOCLEN + CHLEN)
      {
        strncpy(loc, p, LOCLEN);
        strncpy(chn, p + LOCLEN, CHLEN);
      }
    else if(len == CHLEN)
      {
        strncpy(loc, "??", LOCLEN);
        strncpy(chn, p, CHLEN);
      }
    else
      {
        throw PluginInvalidRenameElement();
      }
  }

bool StreamRenamer::string_match(const char *tmpl, const char *str,
  int len) const
  {
    int i;
    const char *pt, *ps;
    for(i = 0, pt = tmpl, ps = str; i < len && *pt; ++i, ++pt, ((*ps)? ++ps: 0))
        if(*pt != '?' && ((*ps)? *ps: ' ') != *pt) return false;

    return true;
  }

bool StreamRenamer::hit(sl_fsdh_s *fsdh)
  {
    if(string_match(from_loc, fsdh->location, LOCLEN) &&
      string_match(from_chn, fsdh->channel, CHLEN))
      {
        if(strncmp(to_loc, "??", 2))
          {
            for(int i = 0; i < LOCLEN; ++i)
                if(to_loc[i] != '?')
                    fsdh->location[i] = to_loc[i];
          }
        else if(strncmp(from_loc, "??", LOCLEN))
            memset(fsdh->location, 32, LOCLEN);

        if(strncmp(to_chn, "???", 3))
          {
            for(int i = 0; i < CHLEN; ++i)
                if(to_chn[i] != '?')
                    fsdh->channel[i] = to_chn[i];
          }
        else if(strncmp(from_chn, "???", CHLEN))
            memset(fsdh->channel, 32, CHLEN);

        return true;
      }

    return false;
  }

//*****************************************************************************
// TriggerBuffer
//*****************************************************************************

class StreamPacket
  {
  public:
    INT_TIME it;
    char data[SLRECSIZE];
  };

class TriggerBuffer
  {
  private:
    const string station_id;
    const int buffer_length;
    const int pre_seconds;
    const int post_seconds;
    bool trigger_on;
    bool trigger_off_requested;
    INT_TIME trigger_off_time;
    list<rc_ptr<StreamPacket> > packets;

  public:
    const string station_name;
    
    TriggerBuffer(const string station_id_init, int buffer_length_init,
      int pre_seconds_init, int post_seconds_init):
      station_id(station_id_init), buffer_length(buffer_length_init),
      pre_seconds(pre_seconds_init), post_seconds(post_seconds_init),
      trigger_on(false), trigger_off_requested(false) {}
      
    void set_trigger_on(int year, int month, int day,
      int hour, int minute, int second);

    void set_trigger_off(int year, int month, int day,
      int hour, int minute, int second);

    void push_packet(INT_TIME begin_time, void *pseed);
  };

void TriggerBuffer::set_trigger_on(int year, int month, int day,
  int hour, int minute, int second)
  {
    if(year   < 1900 || year   >   2099 ||
       month  <    1 || month  >     12 ||
       day    <    1 || day    >     31 ||
       hour   <    0 || hour   >     23 ||
       minute <    0 || minute >     59 ||
       second <    0 || second >     61)
       {
         logs(LOG_WARNING) << "trigger_on: invalid time" << endl;
         return;
       }

    EXT_TIME et;
    et.year = year;
    et.month = month;
    et.day = day;
    et.hour = hour;
    et.minute = minute;
    et.second = second;
    et.usec = 0;
    et.doy = mdy_to_doy(et.month, et.day, et.year);

    INT_TIME it = ext_to_int(et);

    for(; !packets.empty(); packets.pop_front())
      {
        if(int(tdiff(it, packets.front()->it) / 1000000.0) > pre_seconds)
            continue;

        int r = send_mseed(station_id.c_str(), packets.front()->data,
          SLRECSIZE);
            
        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();
      }

    trigger_on = true;
    trigger_off_requested = false;
  }

void TriggerBuffer::set_trigger_off(int year, int month, int day,
  int hour, int minute, int second)
  {
    if(year   < 1900 || year   >   2099 ||
       month  <    1 || month  >     12 ||
       day    <    1 || day    >     31 ||
       hour   <    0 || hour   >     23 ||
       minute <    0 || minute >     59 ||
       second <    0 || second >     61)
       {
         logs(LOG_WARNING) << "trigger_off: invalid time" << endl;
         return;
       }

    EXT_TIME et;
    et.year = year;
    et.month = month;
    et.day = day;
    et.hour = hour;
    et.minute = minute;
    et.second = second;
    et.usec = 0;
    et.doy = mdy_to_doy(et.month, et.day, et.year);

    trigger_off_time = ext_to_int(et);
    trigger_off_requested = true;
  }

void TriggerBuffer::push_packet(INT_TIME begin_time, void *pseed)
  {
    while(!packets.empty() &&
      int(tdiff(begin_time, packets.front()->it) / 1000000.0) > buffer_length)
         packets.pop_front();

    if(trigger_on)
      {
        int r = send_mseed(station_id.c_str(), pseed, SLRECSIZE);
            
        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();
      }
    else
      {
        StreamPacket* pack = new StreamPacket;
        pack->it = begin_time;
        memcpy(pack->data, pseed, SLRECSIZE);
        packets.push_back(pack);
      }

    if(trigger_off_requested &&
      int(tdiff(begin_time, trigger_off_time) / 1000000.0) > post_seconds)
      {
        trigger_on = false;
        trigger_off_requested = false;
      }
  }

//*****************************************************************************
// Station
//*****************************************************************************

struct stream_start
  {
    int recno;
    INT_TIME it;

    stream_start(int recno_init, const INT_TIME &it_init):
      recno(recno_init), it(it_init) {}
  };

struct unpack_options
  {
    string dest_channel;
    bool double_rate;

    unpack_options(const string &dest_channel_init, bool double_rate_init):
      dest_channel(dest_channel_init), double_rate(double_rate_init) {}
  };

class Station
  {
  private:
    string myid;
    string out_name;
    string out_network;
    int default_timing_quality;
    int overlap_removal;

    list<rc_ptr<StreamRenamer> > rename_list;
    map<string, rc_ptr<TriggerBuffer> > trigger_map;
    map<string, unpack_options> unpack_map;
    map<StreamDescriptor, stream_start> stream_start_table;

    void rename_streams(sl_fsdh_s *fsdh);
    bool check_overlap(const StreamDescriptor &strd, int recno,
      const INT_TIME &it);

  public:
    enum OverlapRemoval
      {
        OverlapRemoval_Initial,
        OverlapRemoval_Full,
        OverlapRemoval_None
      };

    Station(const string &myid_init, const string &out_name_init,
      const string &out_network_init, int default_timing_quality_init,
      int overlap_removal_init);

    Station() {}
    
    void add_renamer(const string &from, const string &to);
    void add_unpack(const string &src, const string &dest, bool double_rate);
    void add_trigger(const string &src, int buffer_length, int pre_seconds,
      int post_seconds);

    void set_start_time(const StreamDescriptor strd, int recno,
      int year, int doy, int hour, int minute, int second, int usec);
      
    void clear_timetable();
    
    void set_trigger_on(int year, int month, int day, int hour, int minute,
      int second);

    void set_trigger_off(int year, int month, int day, int hour, int minute,
      int second);

    void process_mseed(char *pseed, int packtype, int seq, int size);
  };

Station::Station(const string &myid_init, const string &out_name_init,
  const string &out_network_init, int default_timing_quality_init,
  int overlap_removal_init):
  myid(myid_init), out_name(out_name_init), out_network(out_network_init), 
  default_timing_quality(default_timing_quality_init),
  overlap_removal(overlap_removal_init) {}
    
void Station::add_renamer(const string &from, const string &to)
  {
    rename_list.push_back(new StreamRenamer(from, to));
  }

void Station::add_unpack(const string &src, const string &dest,
  bool double_rate)
  {
    if(unpack_map.find(src) != unpack_map.end())
        throw PluginUnpackStreamAlreadyUsed(src);

    string srcname;
    switch(src.length())
      {
      case 5:
        srcname = src;
        break;

      case 3:
        srcname = "  " + src;
        break;

      default:
        throw PluginUnpackInvalidSource(src);
      }

    unpack_map.insert(make_pair(srcname, unpack_options(dest, double_rate)));
  }

void Station::add_trigger(const string &src, int buffer_length, int pre_seconds,
  int post_seconds)
  {
    if(trigger_map.find(src) != trigger_map.end())
        throw PluginTriggerStreamAlreadyUsed(src);

    string srcname;
    switch(src.length())
      {
      case 5:
        srcname = src;
        break;

      case 3:
        srcname = "  " + src;
        break;

      default:
        throw PluginTriggerInvalidSource(src);
      }

    trigger_map.insert(make_pair(srcname,
      new TriggerBuffer(myid, buffer_length, pre_seconds, post_seconds)));
  }

void Station::set_start_time(const StreamDescriptor strd, int recno,
  int year, int doy, int hour, int minute, int second, int usec)
  {
    if(overlap_removal == OverlapRemoval_None)
        return;
    
    if(stream_start_table.find(strd) != stream_start_table.end())
        throw PluginStreamAlreadyInTable(strd);
   
    if(year   < 1900 || year   >   2099 ||
       doy    <    1 || doy    >    366 ||
       hour   <    0 || hour   >     23 ||
       minute <    0 || minute >     59 ||
       second <    0 || second >     61 ||
       usec   <    0 || usec   > 999999)
        throw PluginStreamInvalidTime(strd);

    EXT_TIME et;
    et.year = year;
    et.doy = doy;
    et.hour = hour;
    et.minute = minute;
    et.second = second;
    et.usec = usec;
    dy_to_mdy(et.doy, et.year, &et.month, &et.day);

    INT_TIME it = ext_to_int(et);
    stream_start_table.insert(make_pair(strd, stream_start(recno, it)));
  }

void Station::clear_timetable()
  {
    stream_start_table.clear();
  }

void Station::rename_streams(sl_fsdh_s *fsdh)
  {
    list<rc_ptr<StreamRenamer> >::iterator p;
    for(p = rename_list.begin(); p != rename_list.end(); ++p)
        if((*p)->hit(fsdh)) break;
  }
    
bool Station::check_overlap(const StreamDescriptor &strd, int recno,
  const INT_TIME &it)
  {
    bool is_overlap = false;
    map<StreamDescriptor, stream_start>::iterator p;
    if((p = stream_start_table.find(strd)) != stream_start_table.end())
      {
        INT_TIME table_it = p->second.it;
        int table_recno = p->second.recno;
    
        double dt = tdiff(it, table_it);
        int ds = recno - table_recno;

        if(dt > -100 || (dt > -1000000 && ds == 0))
            stream_start_table.erase(p);
        else
            is_overlap = true;
      }

    if(overlap_removal == OverlapRemoval_Full && is_overlap == false)
        stream_start_table.insert(make_pair(strd,
          stream_start((recno + 1) % 1000000, add_dtime(it, 1))));

    return is_overlap;
  }

void Station::set_trigger_on(int year, int month, int day, int hour,
  int minute, int second)
  {
    map<string, rc_ptr<TriggerBuffer> >::iterator p;
    for(p = trigger_map.begin(); p != trigger_map.end(); ++p)
        p->second->set_trigger_on(year, month, day, hour, minute, second);
  }

void Station::set_trigger_off(int year, int month, int day, int hour,
  int minute, int second)
  {
    map<string, rc_ptr<TriggerBuffer> >::iterator p;
    for(p = trigger_map.begin(); p != trigger_map.end(); ++p)
        p->second->set_trigger_off(year, month, day, hour, minute, second);
  }

void Station::process_mseed(char *pseed, int packtype, int seq, int size)
  {
    if(out_name.length() == 0)
        return;
    
    sl_fsdh_s* fsdh = reinterpret_cast<sl_fsdh_s *>(pseed);
    int n;

    strncpy(fsdh->station, out_name.c_str(), STATLEN);
    if((n = out_name.length()) < STATLEN)
        memset(fsdh->station + n, 32, STATLEN - n);

    strncpy(fsdh->network, out_network.c_str(), NETLEN);
    if((n = out_network.length()) < NETLEN)
        memset(fsdh->network + n, 32, NETLEN - n);

    rename_streams(fsdh);

    string net, sta, loc, chn;
    get_id(fsdh, net, sta, loc, chn);
    StreamDescriptor strd(loc, chn, packtype);
    
    EXT_TIME et;
    et.year = htons(fsdh->start_time.year);
    et.doy = htons(fsdh->start_time.day);
    et.hour = fsdh->start_time.hour;
    et.minute = fsdh->start_time.min;
    et.second = fsdh->start_time.sec;
    et.usec = htons(fsdh->start_time.fract) * 100;
    dy_to_mdy(et.doy, et.year, &et.month, &et.day);

    INT_TIME hdrtime = ext_to_int(et);

    int recno;
    sscanf(fsdh->sequence_number, "%6d", &recno);

    if(check_overlap(strd, recno, hdrtime))
        return;
    
    if(packtype != SLDATA)
      {
        int r = send_mseed2(myid.c_str(), (loc + "_" + chn + "_" + packet_type2string(packtype)).c_str(), seq, pseed, SLRECSIZE);
            
        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();

        return;
      }
            
    char srcname[6];
    sprintf(srcname, "%2.2s%3.3s", loc.c_str(), chn.c_str());
      
    map<string, rc_ptr<TriggerBuffer> >::iterator p;
    if((p = trigger_map.find(srcname)) != trigger_map.end())
      {
        p->second->push_packet(hdrtime, pseed);
      }
    else
      {
        int r = send_mseed2(myid.c_str(), (loc + "_" + chn + "_" + packet_type2string(packtype)).c_str(), seq, pseed, SLRECSIZE);
        
        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();
      }

    map<string, unpack_options>::iterator q;
    if((q = unpack_map.find(srcname)) != unpack_map.end())
      {
        SLMSrecord* msr = sl_msr_new();
        sl_msr_parse(NULL, pseed, &msr, 1, 1);
        
        if(msr == NULL) return;

        int timing_quality = default_timing_quality;
        if(msr->Blkt1001 != NULL)
            timing_quality = msr->Blkt1001->timing_qual;

        if(msr->numsamples < 0 || msr->numsamples > MAX_SAMPLES)
          {
            logs(LOG_ERR) << "error decoding Mini-SEED packet " <<
              string(fsdh->sequence_number, 6) << ", "
              "station " << net << "_" << sta << " (" << myid << "), "
              "stream " << loc << "." << chn << ".D" << endl;

            sl_msr_free(&msr);
            return;
          }

        struct ptime pt;
        pt.year = ntohs(fsdh->start_time.year);
        pt.yday = ntohs(fsdh->start_time.day);
        pt.hour = fsdh->start_time.hour;
        pt.minute = fsdh->start_time.min;
        pt.second = fsdh->start_time.sec;
        pt.usec = ntohs(fsdh->start_time.fract) * 100;

        int r = 0;
        if(q->second.double_rate)
          {
            int32_t datasamples[2 * MAX_SAMPLES];

            for(int i = 0; i < msr->numsamples; ++i)
              {
                datasamples[i << 1] = msr->datasamples[i] << 1;
                datasamples[(i << 1) + 1] = 0;
              }

            r = send_raw3(myid.c_str(), q->second.dest_channel.c_str(), &pt,
              ntohs(fsdh->time_correct), timing_quality, datasamples,
              2 * msr->numsamples);
          }
        else
          {
            r = send_raw3(myid.c_str(), q->second.dest_channel.c_str(), &pt,
              ntohs(fsdh->time_correct), timing_quality, msr->datasamples,
              msr->numsamples);
          }
    
        sl_msr_free(&msr);
        
        DEBUG_MSG("sent " << r << " bytes of data, station \"" << myid <<
          "\", channel \"" << q->second << "\"" << endl);
    
        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();
      }
  }
    
//*****************************************************************************
// StationGroup
//*****************************************************************************

class StationGroup
  {
  private:
    pid_t pid;
    int parent_fd;
    bool sigterm_sent;
    bool sigkill_sent;
    bool shutdown_requested;
    Timer uptime_timer;
    Timer standby_timer;
    Timer shutdown_timer;
    
    Timer seqsave_timer;
    string seqfile;
    string ifup_cmd;
    string ifdown_cmd;
    bool multi;
    bool batch;
    bool have_schedule;
    schedule_t schedule;
    time_t last_schedule_check;
    string lockfile;
    SLCD *slcd;

    static StationGroup *obj;
    static void term_handler(int sig);
    
    void open_connection();
    void close_connection();
    void kill_connection();
    void runcmd(const string &cmd);
    void ifup();
    void ifdown();
    void lock_wait();

  public:
    StationGroup(const string &address, bool multi_init, bool batch_init,
      int netto, int netdly, int keepalive, int uptime, int standby, int shutdown,
      int seqsave, const string &seqfile_init, const string &ifup_init,
      const string &ifdown_init, const string &lockfile_init);

    ~StationGroup();

    void set_schedule(const string &schedule_str);
    void add_station(const string &net, const string &sta, const string &sel);
    void shutdown();
    void check();
    bool confirm_shutdown();

    int filedes()
      {
        return parent_fd;
      }
  };

StationGroup::StationGroup(const string &address, bool multi_init, bool batch_init,
  int netto, int netdly, int keepalive, int uptime, int standby, int shutdown,
  int seqsave, const string &seqfile_init, const string &ifup_init,
  const string &ifdown_init, const string &lockfile_init):
  pid(-1), parent_fd(-1), sigterm_sent(false), sigkill_sent(false),
  shutdown_requested(false), uptime_timer(uptime, 0), standby_timer(standby, 0),
  shutdown_timer(shutdown, 0), seqsave_timer(seqsave, 0), seqfile(seqfile_init),
  ifup_cmd(ifup_init), ifdown_cmd(ifdown_init), multi(multi_init), batch(batch_init),
  have_schedule(false), last_schedule_check(0), lockfile(lockfile_init)
  {
    if((slcd = sl_newslcd()) == NULL)
      throw bad_alloc();

    if((slcd->sladdr = (char *) malloc(address.length() + 1)) == NULL)
      throw bad_alloc();

    strcpy(slcd->sladdr, address.c_str());
    slcd->multistation = multi;
    slcd->dialup = (uptime != 0);
    slcd->batchmode = batch;
    slcd->keepalive = keepalive;
    slcd->netto = netto;
    slcd->netdly = netdly;
  }

StationGroup::~StationGroup()
  {
    sl_freeslcd(slcd);
  }

StationGroup *StationGroup::obj = NULL;

void StationGroup::term_handler(int sig)
  {
    sl_terminate(obj->slcd);
  }

void StationGroup::set_schedule(const string &schedule_str)
  {
    switch(init_schedule(&schedule, schedule_str.c_str()))
      {
      case SCHED_MINUTE_ERR:
        throw PluginInvalidSchedule("invalid minute in schedule " +
          schedule_str);
          
      case SCHED_HOUR_ERR:
        throw PluginInvalidSchedule("invalid hour in schedule " +
          schedule_str);

      case SCHED_DOM_ERR:
        throw PluginInvalidSchedule("invalid day of month in schedule " +
          schedule_str);

      case SCHED_MONTH_ERR:
        throw PluginInvalidSchedule("invalid month in schedule " +
          schedule_str);

      case SCHED_DOW_ERR:
        throw PluginInvalidSchedule("invalid day of week in schedule " +
          schedule_str);

      case SCHED_EOI_ERR:
        throw PluginInvalidSchedule("extra characters at the end of "
          "schedule " + schedule_str);
      }

    have_schedule = true;
    time_t sec = time(NULL);
    struct tm* tm = localtime(&sec);
    last_schedule_check = sec - tm->tm_sec;
  }

void StationGroup::add_station(const string &sta, const string &net,
  const string &sel)
  {
    const char* csel = ((sel.length() == 0)? NULL: sel.c_str());

    if(multi)
        sl_addstream(slcd, net.c_str(), sta.c_str(), csel, -1, NULL);
    else
        sl_setuniparams(slcd, csel, -1, NULL);
  }

void StationGroup::runcmd(const string &cmd)
  {
    if(cmd.length() == 0) return;

    int shellpid;
    if((shellpid = fork()) < 0)
        throw PluginLibraryError("cannot fork");

    if(shellpid == 0 && execl(SHELL, SHELL, "-c", cmd.c_str(), NULL) < 0)
        throw PluginLibraryError(string("cannot exec ") + SHELL);

    int status;
    if(waitpid(shellpid, &status, 0) < 0)
        throw PluginLibraryError("waitpid error");

    if(WIFSIGNALED(status))
      {
        logs(LOG_WARNING) << "shell command \"" << cmd << "\" was terminated "
          "by signal " << WTERMSIG(status) << endl;
      }
    else if(WIFEXITED(status) && WEXITSTATUS(status) != 0)
      {
        logs(LOG_WARNING) << "shell command \"" << cmd << "\" exited "
          "with error status " << WEXITSTATUS(status) << endl;
      }
  }

void StationGroup::ifup()
  {
    runcmd(ifup_cmd);
  }

void StationGroup::ifdown()
  {
    runcmd(ifdown_cmd);
  }

void StationGroup::lock_wait()
  {
    if(lockfile.length() == 0) return;
    
    int lock_fd;
    if((lock_fd = open(lockfile.c_str(), O_WRONLY | O_CREAT,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
      {
        logs(LOG_WARNING) << "cannot open '" << lockfile << "'" << endl;
        return;
      }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
  
    if(fcntl(lock_fd, F_SETLKW, &lock) < 0)
      {
        logs(LOG_WARNING) << "cannot lock '" << lockfile << "' (" <<
          strerror(errno) << ")" << endl;

        return;
      }

    char buf[10];
    sprintf(buf, "%d\n", getpid());

    N(ftruncate(lock_fd, 0));
    if(write(lock_fd, buf, strlen(buf)) != (int) strlen(buf))
        logs(LOG_WARNING) << "cannot write pid to '" << lockfile << "'" << endl;
  }

void StationGroup::open_connection()
  {
    internal_check(parent_fd == -1);
    internal_check(pid == -1);
    
    int pipe_fd[2];
    N(pipe(pipe_fd));

    sigset_t newmask, oldmask;
    N(sigemptyset(&newmask));
    N(sigaddset(&newmask, SIGTERM));
    N(sigaddset(&newmask, SIGINT));
    N(sigprocmask(SIG_BLOCK, &newmask, &oldmask));
    
    N(pid = fork());

    if(pid)
      {
        close(pipe_fd[1]);
        parent_fd = pipe_fd[0];
        fcntl(parent_fd, F_SETFD, FD_CLOEXEC);

        N(sigprocmask(SIG_SETMASK, &oldmask, NULL));
        uptime_timer.reset();
        sigterm_sent = false;
        sigkill_sent = false;
        shutdown_requested = false;
        return;
      }

    try
      {
        obj = this;
        close(pipe_fd[0]);

        int child_fd = pipe_fd[1];
        N(fcntl(child_fd, F_SETFL, O_NONBLOCK));
    
        struct sigaction sa;
        sa.sa_handler = term_handler;
        sa.sa_flags = 0;                     // SA_RESTART?
        N(sigemptyset(&sa.sa_mask));
        N(sigaction(SIGTERM, &sa, NULL));

        sa.sa_handler = SIG_IGN;
        N(sigaction(SIGINT, &sa, NULL));

        lock_wait();

        ifup();
    
        if(seqfile.length() > 0)
          {
            if(sl_recoverstate(slcd, seqfile.c_str()) < 0)
                logs(LOG_WARNING) << "could not read connection state from "
                  "file '" << seqfile << "'" << endl;
          }
    
        N(sigprocmask(SIG_UNBLOCK, &newmask, NULL));
      
        seqsave_timer.reset();

        int r;
        SLpacket* slpack;
        while((r = sl_collect(slcd, &slpack)) == SLPACKET)
          {
            if(writen_tmo(child_fd, (void *)slpack, SLHEADSIZE + SLRECSIZE,
              WRITE_TIMEOUT) <= 0)
              {
                logs(LOG_ERR) << "error sending data to the main process" <<
                  endl;

                // Ugly hack to decrease sequence number,
                // hopefully it helps to avoid a data gap...
                sl_fsdh_s* fsdh = reinterpret_cast<sl_fsdh_s *>(slpack->msrecord);
                string net, sta, loc, chn;
                get_id(fsdh, net, sta, loc, chn);

                slstream_s *stream;
                for(stream = slcd->streams; stream; stream = stream->next)
                  {
                    if(!strcmp(stream->net, net.c_str()) &&
                      !strcmp(stream->sta, sta.c_str()))
                      {
                        stream->seqnum = (stream->seqnum - 1) & 0xffffff;
                        break;
                      }
                  }

                if(stream == NULL)
                    logs(LOG_ERR) << "station " << net << "_" << sta
                      << " not found in stream list" << endl;
                    
                break;
              }

            if(seqsave_timer.expired() && seqfile.length() != 0)
              {
                if(sl_savestate(slcd, seqfile.c_str()) < 0)
                    logs(LOG_WARNING) << "could not save connection state "
                      "to file '" << seqfile << "'" << endl;

                seqsave_timer.reset();
              }
          }

        N(sigprocmask(SIG_BLOCK, &newmask, NULL));

        close(child_fd);
        child_fd = -1;
    
        if(slcd->link != -1)
            sl_disconnect(slcd);

        if(seqfile.length() != 0 && r == 0)
          {
            if(sl_savestate(slcd, seqfile.c_str()) < 0)
                logs(LOG_WARNING) << "could not save connection state to "
                  "file '" << seqfile << "'" << endl;
          }

        ifdown();
      }
    catch(exception &e)
      {
        logs(LOG_ERR) << e.what() << endl;
        exit(1);
      }
    catch(...)
      {
        logs(LOG_ERR) << "unknown exception" << endl;
        exit(1);
      }

    exit(0);
  }

void StationGroup::close_connection()
  {
    internal_check(pid > 0);
    internal_check(parent_fd != -1);
    
    kill(pid, SIGTERM);
    sigterm_sent = true;
    shutdown_timer.reset();
  }

void StationGroup::kill_connection()
  {
    internal_check(pid > 0);
    internal_check(parent_fd != -1);
    
    kill(pid, SIGKILL);
    sigkill_sent = true;
  }

void StationGroup::shutdown()
  {
    if(!sigterm_sent && pid > 0)
        close_connection();

    shutdown_requested = true;
  }

void StationGroup::check()
  {
    internal_check(pid != 0);
    
    if(pid < 0)
      {
        if(shutdown_requested || parent_fd >= 0)
            return;
        
        time_t curtime = time(NULL);
        if(have_schedule)
          {
            if(curtime - last_schedule_check >= 60)
              {
                if(check_schedule(&schedule, (time_t) curtime))
                    open_connection();
              
                last_schedule_check += 60;
              }
          }
        else if(standby_timer.expired())
          {
            open_connection();
          }
        
        return;
      }

    if(!sigterm_sent && uptime_timer.expired())
      {
        close_connection();
        return;
      }

    int status, completed;
    if((completed = waitpid(pid, &status, WNOHANG)) < 0)
      {
        logs(LOG_ERR) << "waitpid: " << strerror(errno) << endl;
        close(parent_fd);
        parent_fd = -1;
        pid = -1;
        return;
      }

    if(!completed)
      {
        if(sigterm_sent && !sigkill_sent && shutdown_timer.expired())
            kill_connection();
            
        return;
      }

    pid = -1;

    if(WIFSIGNALED(status))
      {
        logs(LOG_WARNING) << "SeedLink connection terminated on signal " <<
          WTERMSIG(status) << endl;
      }
    else if(WIFEXITED(status) && WEXITSTATUS(status) != 0)
      {
        logs(LOG_WARNING) << "SeedLink connection terminated with status " <<
          WEXITSTATUS(status) << endl;
      }

    standby_timer.reset();
  }

bool StationGroup::confirm_shutdown()
  {
    if(pid < 0)
      {
        if(parent_fd >= 0)
            close(parent_fd);

        parent_fd = -1;

        return shutdown_requested;
      }
    
    return false;
  }
    
//*****************************************************************************
// Extension
//*****************************************************************************

class Extension_Partner
  {
  public:
    virtual void extension_request(const string &cmd) = 0;
    virtual ~Extension_Partner() {};
  };

class Extension
  {
  private:
    Extension_Partner &partner;
    const string cmdline;
    pid_t pid;
    int data_input_fd;
    int data_output_fd;
    int command_fd;
    int send_timeout;
    bool output_active;
    bool sigterm_sent;
    bool sigkill_sent;
    bool shutdown_requested;
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
    char reqbuf[REQLEN + 1];
    int reqwp;
    regex_t stream_filter_rx;
    bool have_stream_filter;

    void check_proc();
    bool read_data();
    void forward_packet();
    void check_data();
    void check_cmds();
    void kill_proc();
    void term_proc();

  public:
    const string name;
    
    Extension(Extension_Partner &partner_init, const string &name_init,
      const string &filter_init, const string &cmdline_init, int recv_timeout,
      int send_timeout_init, int start_retry, int shutdown_wait);

    ~Extension();
        
    void start();
    void feed(char *pseed, int packtype, int size);
    bool check();
    void shutdown();
    
    pair<int, int> filedes()
      {
        return make_pair(data_input_fd, command_fd);
      }
  };

Extension::Extension(Extension_Partner &partner_init, const string &name_init,
  const string &filter, const string &cmdline_init, int recv_timeout,
  int send_timeout_init, int start_retry, int shutdown_wait):
  partner(partner_init), cmdline(cmdline_init), pid(-1), data_input_fd(-1),
  data_output_fd(-1), command_fd(-1), send_timeout(send_timeout_init),
  output_active(false), sigterm_sent(false), sigkill_sent(false),
  shutdown_requested(false), read_timer(recv_timeout, 0),
  start_retry_timer(start_retry, 0), shutdown_timer(shutdown_wait, 0),
  read_state(ReadInit), data_bytes(0), nread(0), nleft(0), ptr(NULL),
  reqwp(0), have_stream_filter(false), name(name_init)
  {
    if(filter.length() > 0)
      {
        int err = regcomp(&stream_filter_rx, filter.c_str(),
          REG_EXTENDED | REG_ICASE | REG_NOSUB);

        if(err != 0)
          {
            char errbuf[REGEX_ERRLEN];
            regerror(err, &stream_filter_rx, errbuf, REGEX_ERRLEN);
            throw PluginRegexError(errbuf);
          }

        have_stream_filter = true;
      }
  }

Extension::~Extension()
  {
    if(data_output_fd >= 0)
        close(data_output_fd);

    if(data_input_fd >= 0)
        close(data_input_fd);

    if(command_fd >= 0)
        close(command_fd);

    if(have_stream_filter)
        regfree(&stream_filter_rx);
  }

void Extension::check_proc()
  {
    internal_check(pid != 0);
    
    int status, completed;

    if(pid < 0) return;

    if((completed = waitpid(pid, &status, WNOHANG)) < 0)
      {
        logs(LOG_ERR) << "waitpid: " << strerror(errno) << endl;
        pid = -1;
        if(data_output_fd >= 0)
          {
            close(data_output_fd);
            data_output_fd = -1;
            output_active = false;
          }

        return;
      }

    if(!completed) return;

    pid = -1;
    if(data_output_fd >= 0)
      {
        close(data_output_fd);
        data_output_fd = -1;
        output_active = false;
      }

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

    start_retry_timer.reset();
  }
    
bool Extension::read_data()
  {
    if(read_state == ReadInit)
      {
        nleft = sizeof(PluginPacketHeader);
        ptr = reinterpret_cast<char *>(&header_buf);
        read_state = ReadHeader;
      }
    
    int nread;
    if((nread = read(data_input_fd, ptr, nleft)) == 0)
      {
        if(!sigterm_sent && !sigkill_sent)
            logs(LOG_WARNING) << "[" << name << "] unexpected eof data" << endl;

        close(data_input_fd);
        data_input_fd = -1;
        return false;
      }

    if(nread < 0)
      {
        if(errno != EAGAIN)
          {
            logs(LOG_WARNING) << "[" << name << "] data read error (" <<
              strerror(errno) << ")" << endl;

            close(data_input_fd);
            data_input_fd = -1;
          }

        return false;
      }

    read_timer.reset();
    
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

            close(data_input_fd);
            data_input_fd = -1;
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

    return true;
  }

void Extension::forward_packet()
  {
    int r;
    
    r = writen(PLUGIN_FD, &header_buf, sizeof(struct PluginPacketHeader));
    
    if(r < 0) throw PluginBrokenLink(strerror(errno));
    else if(r == 0) throw PluginBrokenLink();

    if(data_bytes != 0)
      {
        r = writen(PLUGIN_FD, data_buf, data_bytes);

        if(r < 0) throw PluginBrokenLink(strerror(errno));
        else if(r == 0) throw PluginBrokenLink();
      }
  }

void Extension::check_data()
  {
    while(read_data())
      {
        if(nleft == 0)
            forward_packet();
      }
  }

void Extension::check_cmds()
  {
    int nread;
    if((nread = read(command_fd, &reqbuf[reqwp], REQLEN - reqwp)) == 0)
      {
        if(!sigterm_sent && !sigkill_sent)
            logs(LOG_WARNING) << "[" << name << "] unexpected eof cmd" << endl;

        close(command_fd);
        command_fd = -1;
        return;
      }

    if(nread < 0)
      {
        if(errno != EAGAIN)
          {
            logs(LOG_WARNING) << "[" << name << "] cmd read error (" <<
              strerror(errno) << ")" << endl;

            close(command_fd);
            command_fd = -1;
          }

        return;
      }

    read_timer.reset();
    
    reqwp += nread;
    reqbuf[reqwp] = 0;

    int reqrp = 0, reqlen, seplen;
    while(reqlen = strcspn(reqbuf + reqrp, "\r\n"),
      seplen = strspn(reqbuf + reqrp + reqlen, "\r\n"))
      {
        reqbuf[reqrp + reqlen] = 0;
        partner.extension_request(reqbuf + reqrp);
        reqrp += (reqlen + seplen);
      }
    
    if(reqlen >= REQLEN)
      {
        logs(LOG_WARNING) << "[" << name << "] command buffer overflow" << endl;
        reqwp = reqrp = 0;

        close(command_fd);
        command_fd = -1;
        return;
      }
        
    memmove(reqbuf, reqbuf + reqrp, reqlen);
    reqwp -= reqrp;
    reqrp = 0;
  }

void Extension::kill_proc()
  {
    internal_check(pid > 0);
    
    kill(pid, SIGKILL);
    sigkill_sent = true;
  }

void Extension::term_proc()
  {
    internal_check(pid > 0);
    
    kill(pid, SIGTERM);
    sigterm_sent = true;
    output_active = false;
    shutdown_timer.reset();
  }

void Extension::start()
  {
    internal_check(data_input_fd < 0);
    internal_check(data_output_fd < 0);
    internal_check(command_fd < 0);
    
    int data_input_pipe[2];
    int data_output_pipe[2];
    int command_pipe[2];

    N(pipe(data_input_pipe));
    N(pipe(data_output_pipe));
    N(pipe(command_pipe));
    
    N(pid = fork());

    if(pid) 
      {
        close(data_input_pipe[1]);
        close(data_output_pipe[0]);
        close(command_pipe[1]);
        
        data_input_fd = data_input_pipe[0];
        data_output_fd = data_output_pipe[1];
        command_fd = command_pipe[0];
        
        N(fcntl(data_input_fd, F_SETFD, FD_CLOEXEC));
        N(fcntl(data_output_fd, F_SETFD, FD_CLOEXEC));
        N(fcntl(command_fd, F_SETFD, FD_CLOEXEC));

        N(fcntl(data_input_fd, F_SETFL, O_NONBLOCK));
        N(fcntl(data_output_fd, F_SETFL, O_NONBLOCK));
        N(fcntl(command_fd, F_SETFL, O_NONBLOCK));
        
        read_timer.reset();
        read_state = ReadInit;
        reqwp = 0;
        sigterm_sent = false;
        sigkill_sent = false;
        shutdown_requested = false;
        output_active = true;
        return;
      }

    close(data_input_pipe[0]);
    close(data_output_pipe[1]);
    close(command_pipe[0]);
    
    if(data_input_pipe[1] != PLUGIN_FD)
      {
        N(dup2(data_input_pipe[1], PLUGIN_FD));
        close(data_input_pipe[1]);
      }

    if(data_output_pipe[0] != EXT_DATA_FD)
      {
        N(dup2(data_output_pipe[0], EXT_DATA_FD));
        close(data_output_pipe[0]);
      }

    if(command_pipe[1] != EXT_CMD_FD)
      {
        N(dup2(command_pipe[1], EXT_CMD_FD));
        close(command_pipe[1]);
      }

    logs(LOG_INFO) << "[" << name << "] starting shell" << endl;
    
    execl(SHELL, SHELL, "-c", (cmdline + " " + name).c_str(), NULL);

    logs(LOG_ERR) << string() + "cannot execute shell '" + SHELL + "' "
      "(" + strerror(errno) + ")" << endl;
    exit(0);
  }

void Extension::feed(char *pseed, int packtype, int size)
  {
    if(!output_active)
        return;
    
    internal_check(data_output_fd >= 0);
    
    if(have_stream_filter)
      {
        sl_fsdh_s* fsdh = reinterpret_cast<sl_fsdh_s *>(pseed);
        string net, sta, loc, chn;
        get_id(fsdh, net, sta, loc, chn);

        const char *stype;
        if((stype = packet_type2string(packtype)) == NULL)
            stype = "X";

        char stream_id[NETLEN + STATLEN + LOCLEN + CHLEN + 10];
        sprintf(stream_id, "%s_%s_%s_%s_%s", net.c_str(), sta.c_str(),
          loc.c_str(), chn.c_str(), stype);

        if(regexec(&stream_filter_rx, stream_id, 0, NULL, 0) == REG_NOMATCH)
            return;
      }

    int r;
    if((r = writen_tmo(data_output_fd, pseed, size, send_timeout)) < 0)
      {
        logs(LOG_WARNING) << "[" << name << "] write error (" <<
          strerror(errno) << ")" << endl;

        term_proc();
      }
    else if(r == 0)
      {
        logs(LOG_WARNING) << "[" << name << "] write timeout" << endl;
        term_proc();
      }
  }

bool Extension::check()
  {
    internal_check(pid != 0);

    if(data_input_fd >= 0)
        check_data();

    if(command_fd >= 0)
        check_cmds();
    
    check_proc();

    if(pid < 0)
      {
        if(shutdown_requested)
          {
            if(sigkill_sent)
              {
                if(data_input_fd >= 0)
                  {
                    close(data_input_fd);
                    data_input_fd = -1;
                  }

                if(command_fd >= 0)
                  {
                    close(command_fd);
                    command_fd = -1;
                  }
              }

            if(data_input_fd < 0 && command_fd < 0)
                return true;
          }

        if(!shutdown_requested && start_retry_timer.expired())
          {
            if(data_input_fd >= 0)
              {
                close(data_input_fd);
                data_input_fd = -1;
              }

            if(command_fd >= 0)
              {
                close(command_fd);
                command_fd = -1;
              }

            start();
          }

        return false;
      }

    if(!sigterm_sent)
      {
        if(data_input_fd < 0 || command_fd < 0)
          {
            term_proc();
          }
        else if(read_timer.expired())
          {
            logs(LOG_WARNING) << "[" << name << "] timeout" << endl;
            term_proc();
          }

        return false;
      }
      
    if(!sigkill_sent && shutdown_timer.expired())
      {
        logs(LOG_WARNING) << "[" << name << "] shutdown time expired" << endl;
        kill_proc();
      }

    return false;
  }

void Extension::shutdown()
  {
    if(!sigterm_sent && pid > 0)
        term_proc();
    
    shutdown_requested = true;
  }

//*****************************************************************************
// Chain
//*****************************************************************************

class Chain: private Extension_Partner
  {
  private:
    map<StationDescriptor, rc_ptr<Station> > stations;
    map<string, rc_ptr<Station> > station_id_map;
    list<rc_ptr<StationGroup> > groups;
    list<rc_ptr<Extension> > extensions;
    time_t last_ext_check;

    void extension_request(const string &cmd);
    int process_slpacket(int fd);
    void setup_timetable(const string &timetable_loader);

  public:
    Chain(): last_ext_check(0) {}

    void setup(const string &timetable_loader);

    void new_extension(const string &name, const string &filter,
      const string &cmdline, int recv_timeout, int send_timeout,
      int start_retry, int shutdown_wait);
    
    rc_ptr<StationGroup> new_group(const string &address, bool multi, bool batch,
      int netto, int netdly, int keepalive, int uptime, int standby,
      int shutdown, int seqsave, const string &seqfile, const string &ifup,
      const string &ifdown, const string &lockfile);
    
    rc_ptr<Station> new_station(rc_ptr<StationGroup> group, const string &id,
      const string &in_name, const string &in_network, const string &out_name,
      const string &out_network, const string &selectors,
      int default_timing_quality, int overlap_removal);

    void check();
    void shutdown();
  };

void Chain::extension_request(const string &cmd)
  {
    if(strncasecmp(cmd.c_str(), "TRIGGER ", 8))
      {
        logs(LOG_WARNING) << "invalid command: " << cmd << endl;
        return;
      }
    
    int year, month, day, hour, min, sec;
    char trigger_state[4], station_id[11], c;
    if(sscanf(cmd.c_str(), "%*s %3s %10s %d %d %d %d %d %d%c", trigger_state,
      station_id, &year, &month, &day, &hour, &min, &sec, &c) != 8)
      {
        logs(LOG_WARNING) << "wrong syntax: " << cmd << endl;
        return;
      }

    bool trigger_on = false;;
    if(!strcasecmp(trigger_state, "ON"))
      {
        trigger_on = true;
      }
    else if(!strcasecmp(trigger_state, "OFF"))
      {
        trigger_on = false;
      }
    else
      {
        logs(LOG_WARNING) << "wrong syntax: " << cmd << endl;
        return;
      }
    
    logs(LOG_NOTICE) << cmd << endl;

    map<string, rc_ptr<Station> >::iterator p;
    if((p = station_id_map.find(station_id)) == station_id_map.end())
      {
        logs(LOG_WARNING) << "station " << station_id << " not found" << endl;
        return;
      }

    if(trigger_on)
        p->second->set_trigger_on(year, month, day, hour, min, sec);
    else
        p->second->set_trigger_off(year, month, day, hour, min, sec);
  }

void Chain::setup_timetable(const string &timetable_loader)
  {
    if(timetable_loader.length() == 0)
        return;
    
    logs(LOG_INFO) << "loading timetable" << endl;
    
    FILE *fp;
    if((fp = popen(timetable_loader.c_str(), "r")) == NULL)
      {
        logs(LOG_WARNING) << "could not execute '" <<
          timetable_loader << "'" << endl;
        return;
      }

    map<StationDescriptor, rc_ptr<Station> >::iterator p = stations.begin();

    if(p == stations.end())
      {
        pclose(fp);
        return;
      }
    
    StationDescriptor cur_stad = p->first;

    char net[3], sta[6], stream[9], *loc, *chn, *stype;
    int type, recno, year, doy, hour, min, sec, usec;
    
    int r;
    while((r = fscanf(fp, "%2s %5s %8s %d %d %d %d %d %d %d\n", net, sta,
      stream, &recno, &year, &doy, &hour, &min, &sec, &usec)) == 10)
      {
        loc = strdup(stream);

        if((chn = strchr(loc, '.')) == NULL)
          {
            logs(LOG_WARNING) << "invalid stream: " << stream << endl;
            free(loc);
            continue;
          }

        *(chn++) = 0;

        if((stype = strchr(chn, '.')) == NULL)
          {
            logs(LOG_WARNING) << "invalid stream: " << stream << endl;
            free(loc);
            continue;
          }

        *(stype++) = 0;

        if((type = packet_type2int(stype)) == -1)
          {
            logs(LOG_WARNING) << "invalid stream: " << stream << endl;
            free(loc);
            continue;
          }
            
        StationDescriptor stad(net, sta);
        StreamDescriptor strd(loc, chn, type);

        free(loc);

        if(stad != cur_stad)
          {
            p = stations.find(stad);
            cur_stad = stad;
          }

        if(p == stations.end())
            continue;

        try
          {
            p->second->set_start_time(strd, recno, year, doy, hour,
              min, sec, usec);
              
            logs(LOG_INFO) << stad.to_string() << " " <<
              strd.to_string() << " " << recno << " " << year << " " <<
              doy << " " << hour << " " << min << " " << sec << " " <<
              usec << endl;
          }
        catch(PluginError &e)
          {
            logs(LOG_WARNING) << e.message << endl;
          }
      }

    pclose(fp);
        
    if(r == 0 || r == EOF)
      {
        logs(LOG_INFO) << "timetable loaded" << endl;
        return;
      }
      
    logs(LOG_INFO) << "loading timetable failed" << endl;

    for(p = stations.begin(); p != stations.end(); ++p)
        p->second->clear_timetable();
  }

int Chain::process_slpacket(int fd)
  {
    char buf[SLHEADSIZE + SLRECSIZE];
    SLpacket* slpack = reinterpret_cast<SLpacket *>(buf);
    sl_fsdh_s* fsdh = reinterpret_cast<sl_fsdh_s *>(slpack->msrecord);

    int bytes_read;
    if((bytes_read = readn(fd, buf, SLHEADSIZE + SLRECSIZE)) <= 0)
        return bytes_read;

    internal_check(bytes_read == SLHEADSIZE + SLRECSIZE);

    int packtype = sl_packettype(slpack);

    if(packtype == SLNUM)
        logs(LOG_ERR) << "could not determine packet type" << endl;
      
    if(packtype >= SLNUM)
        return bytes_read;
    
    string net, sta, loc, chn;
    get_id(fsdh, net, sta, loc, chn);
    StationDescriptor stad(net, sta);

    map<StationDescriptor, rc_ptr<Station> >::iterator sp;
    if((sp = stations.find(stad)) == stations.end())
      {
        logs(LOG_WARNING) << "station " << stad.to_string() <<
          " received, but not requested" << endl;

        stations.insert(make_pair(stad, new Station()));
        return bytes_read;
      }

    sp->second->process_mseed(slpack->msrecord, packtype, sl_sequence(slpack), SLRECSIZE);

    list<rc_ptr<Extension> >::iterator ep;
    for(ep = extensions.begin(); ep != extensions.end(); ++ep)
        (*ep)->feed(slpack->msrecord, packtype, SLRECSIZE);

    return bytes_read;
  }

void Chain::setup(const string &timetable_loader)
  {
    setup_timetable(timetable_loader);

    list<rc_ptr<Extension> >::iterator p;
    for(p = extensions.begin(); p != extensions.end(); ++p)
        (*p)->start();
  }

void Chain::check()
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    int fd_max = -1;

    list<rc_ptr<StationGroup> >::iterator gp;
    for(gp = groups.begin(); gp != groups.end(); ++gp)
      {
        (*gp)->check();
        
        int fd = (*gp)->filedes();
        
        if(fd != -1)
            FD_SET(fd, &read_set);

        if(fd > fd_max)
            fd_max = fd;
      }

    list<rc_ptr<Extension> >::iterator ep;
    for(ep = extensions.begin(); ep != extensions.end(); ++ep)
      {
        pair<int, int> fd = (*ep)->filedes();
        
        if(fd.first  != -1)
            FD_SET(fd.first, &read_set);

        if(fd.first > fd_max)
            fd_max = fd.first;
        
        if(fd.second != -1)
            FD_SET(fd.second, &read_set);

        if(fd.second > fd_max)
            fd_max = fd.second;
      }
        
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if(select(fd_max + 1, &read_set, NULL, NULL, &tv) < 0)
      {
        if(errno == EINTR) return;
        throw PluginLibraryError("select error");
      }

    gp = groups.begin();
    while(gp != groups.end())
      {
        int r = 0;
        int fd = (*gp)->filedes();
        if(fd != -1 && FD_ISSET(fd, &read_set))
          {
            if((r = process_slpacket(fd)) < 0)
                throw PluginCannotReadChild();
          }

        if(r == 0 && (*gp)->confirm_shutdown())
          {
            groups.erase(gp++);
            continue;
          }

        ++gp;
      }

    time_t curtime = time(NULL);
    
    ep = extensions.begin();
    while(ep != extensions.end())
      {
        pair<int, int> fd = (*ep)->filedes();

        if((fd.first == -1 || !FD_ISSET(fd.first, &read_set)) &&
          (fd.second == -1 || !FD_ISSET(fd.second, &read_set)) &&
          last_ext_check == curtime)
          {
            ++ep;
            continue;
          }

        if((*ep)->check())
          {
            extensions.erase(ep++);
            continue;
          }

        ++ep;
      }

    last_ext_check = curtime;
  }
        
void Chain::shutdown()
  {
    logs(LOG_INFO) << "shutting down" << endl;
    
    list<rc_ptr<StationGroup> >::iterator gp;
    for(gp = groups.begin(); gp != groups.end(); ++gp)
        (*gp)->shutdown();
        
    while(!groups.empty())
        check();

    list<rc_ptr<Extension> >::iterator ep;
    for(ep = extensions.begin(); ep != extensions.end(); ++ep)
        (*ep)->shutdown();
        
    while(!extensions.empty())
        check();

    map<StationDescriptor, rc_ptr<Station> >::iterator q;
    for(q = stations.begin(); q != stations.end(); ++q)
        q->second->clear_timetable();
  }

void Chain::new_extension(const string &name, const string &filter,
  const string &cmdline, int recv_timeout, int send_timeout,
  int start_retry, int shutdown_wait)
  {
    rc_ptr<Extension> extension = new Extension(*this, name, filter, cmdline,
      recv_timeout, send_timeout, start_retry, shutdown_wait);

    extensions.push_back(extension);
  }
    
rc_ptr<StationGroup> Chain::new_group(const string &address,
  bool multi, bool batch, int netto, int netdly, int keepalive, int uptime,
  int standby, int shutdown, int seqsave, const string &seqfile,
  const string &ifup, const string &ifdown, const string &lockfile)
  {
    rc_ptr<StationGroup> group = new StationGroup(address, multi, batch,
      netto, netdly, keepalive, uptime, standby, shutdown, seqsave, seqfile,
      ifup, ifdown, lockfile);
    
    groups.push_back(group);
    return group;
  }

rc_ptr<Station> Chain::new_station(rc_ptr<StationGroup> group,
  const string &id, const string &in_name, const string &in_network,
  const string &out_name, const string &out_network, const string &selectors,
  int default_timing_quality, int overlap_removal)
  {
    StationDescriptor stad(in_network, in_name);

    if(stations.find(stad) != stations.end())
        throw PluginStationAlreadyDefined(stad);

    if(station_id_map.find(id) != station_id_map.end())
        throw PluginStationIdAlreadyUsed(id);
    
    group->add_station(in_name, in_network, selectors);
    
    rc_ptr<Station> station = new Station(id, out_name, out_network,
      default_timing_quality, overlap_removal);
      
    stations.insert(make_pair(stad, station));
    station_id_map.insert(make_pair(id, station));

    return station;
  }

Chain chain;

//*****************************************************************************
// ExtensionElement
//*****************************************************************************

class ExtensionElement: public CfgElement
  {
  private:
    string name;
    string cmd;
    string filter;
    int recv_timeout;
    int send_timeout;
    int start_retry;
    int shutdown_wait;
    set<string> extensions_defined;

  public:
    ExtensionElement(): CfgElement("extension") {}
    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog, const string &);
    void end_attributes(ostream &cfglog);
  };
        
rc_ptr<CfgAttributeMap> ExtensionElement::start_attributes(ostream &cfglog,
  const string &)
  {
    name = "";
    cmd = "";
    filter = "";
    recv_timeout = 0;
    send_timeout = 60;
    start_retry = 60;
    shutdown_wait = 10;

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("name", name));
    atts->add_item(StringAttribute("filter", filter));
    atts->add_item(StringAttribute("cmd", cmd));
    atts->add_item(IntAttribute("recv_timeout", recv_timeout, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("send_timeout", send_timeout, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("start_retry", start_retry, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("shutdown_wait", shutdown_wait, 0, IntAttribute::lower_bound));
    return atts;
  }
 
void ExtensionElement::end_attributes(ostream &cfglog)
try
  {
    if(name.length() == 0)
      {
        cfglog << "extension name is not specified" << endl;
        return;
      }

    if(cmd.length() == 0)
      {
        cfglog << "extension command is not specified" << endl;
        return;
      }
        
    if(extensions_defined.find(name) != extensions_defined.end())
      {
        cfglog << "extension " << name << " is already defined" << endl;
        return;
      }

    chain.new_extension(name, filter, cmd, recv_timeout, send_timeout,
      start_retry, shutdown_wait);
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
  }

//*****************************************************************************
// RenameElement
//*****************************************************************************

class RenameElement: public CfgElement
  {
  private:
    string from;
    string to;
    rc_ptr<Station> station;

  public:
    RenameElement(rc_ptr<Station> station_init): CfgElement("rename"),
      station(station_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog, const string &);

    void end_attributes(ostream &cfglog);
  };
        
rc_ptr<CfgAttributeMap> RenameElement::start_attributes(ostream &cfglog,
  const string &)
  {
    from = "";
    to = "";
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("from", from));
    atts->add_item(StringAttribute("to", to));
    return atts;
  }

void RenameElement::end_attributes(ostream &cfglog)
try
  {
    if(from.length() == 0)
      {
        cfglog << "from-pattern of rename element is not specified" << endl;
        return;
      }

    if(to.length() == 0)
      {
        cfglog << "to-pattern of rename element is not specified" <<
          endl;
        return;
      }

    station->add_renamer(from, to);
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
  }

//*****************************************************************************
// UnpackElement
//*****************************************************************************

class UnpackElement: public CfgElement
  {
  private:
    string src;
    string dest;
    bool double_rate;
    rc_ptr<Station> station;

  public:
    UnpackElement(rc_ptr<Station> station_init): CfgElement("unpack"),
      station(station_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog, const string &);

    void end_attributes(ostream &cfglog);
  };
        
rc_ptr<CfgAttributeMap> UnpackElement::start_attributes(ostream &cfglog,
  const string &)
  {
    src = "";
    dest = "";
    double_rate = false;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("src", src));
    atts->add_item(StringAttribute("dest", dest));
    atts->add_item(BoolAttribute("double_rate", double_rate, "yes", "no"));
    return atts;
  }

void UnpackElement::end_attributes(ostream &cfglog)
try
  {
    if(src.length() == 0)
      {
        cfglog << "source stream of unpack element is not specified" << endl;
        return;
      }

    if(dest.length() == 0)
      {
        cfglog << "destination stream of unpack element is not specified" <<
          endl;
        return;
      }

    station->add_unpack(src, dest, double_rate);
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
  }

//*****************************************************************************
// TriggerElement
//*****************************************************************************

class TriggerElement: public CfgElement
  {
  private:
    string src;
    int buffer_length;
    int pre_seconds;
    int post_seconds;
    rc_ptr<Station> station;

  public:
    TriggerElement(rc_ptr<Station> station_init): CfgElement("trigger"),
      station(station_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog, const string &);

    void end_attributes(ostream &cfglog);
  };
        
rc_ptr<CfgAttributeMap> TriggerElement::start_attributes(ostream &cfglog,
  const string &)
  {
    src = "";
    buffer_length = 60;
    pre_seconds = 20;
    post_seconds = 20;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("src", src));
    atts->add_item(IntAttribute("buffer_length", buffer_length, 10, 600));
    atts->add_item(IntAttribute("pre_seconds", pre_seconds, 10, 600));
    atts->add_item(IntAttribute("post_seconds", post_seconds, 10, 600));
    return atts;
  }

void TriggerElement::end_attributes(ostream &cfglog)
try
  {
    if(src.length() == 0)
      {
        cfglog << "source stream of trigger is not specified" << endl;
        return;
      }

    station->add_trigger(src, buffer_length, pre_seconds, post_seconds);
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
  }

//*****************************************************************************
// StationElement
//*****************************************************************************

class StationElement: public CfgElement
  {
  private:
    const rc_ptr<StationGroup> group;
    const bool multi;
    const string def_overlap_removal;
    string id;
    string in_name;
    string in_network;
    string out_name;
    string out_network;
    string selectors;
    string overlap_removal;
    int default_timing_quality;
    bool first;

  public:
    StationElement(rc_ptr<StationGroup> group_init, bool multi_init,
      const string &overlap_removal):
      CfgElement("station"), group(group_init), multi(multi_init),
      def_overlap_removal(overlap_removal), first(true) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &);
  };
        
rc_ptr<CfgAttributeMap> StationElement::start_attributes(ostream &cfglog,
  const string &)
  {
    if(!first && !multi)
      {
        cfglog << "only one station in group can be defined in uni-station "
          "mode" << endl;
        return NULL;
      }
        
    first = false;

    id = "";
    in_name = "";
    in_network = "";
    out_name = "";
    out_network = "";
    selectors = "";
    overlap_removal = def_overlap_removal;
    default_timing_quality = -1;

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("id", id));
    atts->add_item(StringAttribute("name", in_name));
    atts->add_item(StringAttribute("network", in_network));
    atts->add_item(StringAttribute("out_name", out_name));
    atts->add_item(StringAttribute("out_network", out_network));
    atts->add_item(StringAttribute("selectors", selectors));
    atts->add_item(StringAttribute("overlap_removal", overlap_removal));
    atts->add_item(IntAttribute("default_timing_quality",
      default_timing_quality, -1, 100));
    return atts;
  }

rc_ptr<CfgElementMap> StationElement::start_children(ostream &cfglog,
  const string &)
try
  {
    if(in_name.length() == 0)
      {
        cfglog << "station code is not specified" << endl;
        return NULL;
      }

    if(in_network.length() == 0)
      {
        cfglog << "network code is not specified" << endl;
        return NULL;
      }
    
    if(out_name.length() == 0)
        out_name = in_name;

    if(out_network.length() == 0)
        out_network = in_network;
    
    if(id.length() == 0)
        id = out_network + "_" + out_name;

    int ovrl = Station::OverlapRemoval_None;
    if(!strcasecmp(overlap_removal.c_str(), "full"))
        ovrl = Station::OverlapRemoval_Full;
    else if(!strcasecmp(overlap_removal.c_str(), "initial"))
        ovrl = Station::OverlapRemoval_Initial;
    else if(strcasecmp(overlap_removal.c_str(), "none"))
        cfglog << "value of overlap_removal should be \"full\", "
          "\"initial\" or \"none\". Using \"none\"." << endl;
    
    rc_ptr<Station> station = chain.new_station(group, id, in_name,
      in_network, out_name, out_network, selectors, default_timing_quality,
      ovrl);
    
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(RenameElement(station));
    elms->add_item(UnpackElement(station));
    elms->add_item(TriggerElement(station));
    return elms;
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
    return NULL;
  }
        
//*****************************************************************************
// GroupElement
//*****************************************************************************

class GroupElement: public CfgElement
  {
  private:
    const bool def_multi;
    const bool def_batch;
    const int def_netto;
    const int def_netdly;
    const int def_keepalive;
    const int def_standby;
    const int def_seqsave;
    const string def_overlap_removal;
    string address;
    bool multi;
    bool batch;
    int netto;
    int netdly;
    int keepalive;
    int uptime;
    int standby;
    int seqsave;
    string overlap_removal;
    string seqfile;
    string ifup;
    string ifdown;
    string schedule_str;
    string lockfile;

  public:
    GroupElement(bool multi, bool batch, int netto, int netdly, int keepalive,
      int standby, int seqsave, const string &overlap_removal):
      CfgElement("group"), def_multi(multi), def_batch(batch), def_netto(netto),
      def_netdly(netdly), def_keepalive(keepalive), def_standby(standby),
      def_seqsave(seqsave), def_overlap_removal(overlap_removal) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &);
  };
        
rc_ptr<CfgAttributeMap> GroupElement::start_attributes(ostream &cfglog,
  const string &)
  {
    address = "";
    multi = def_multi;
    batch = def_batch;
    netto = def_netto;
    netdly = def_netdly;
    keepalive = def_keepalive;
    uptime = 0;
    standby = def_standby;
    seqsave = def_seqsave;
    overlap_removal = def_overlap_removal;
    seqfile = "";
    ifup = "";
    ifdown = "";
    schedule_str = "";
    lockfile = "";
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("address", address));
    atts->add_item(BoolAttribute("multistation", multi, "yes", "no"));
    atts->add_item(BoolAttribute("batchmode", batch, "yes", "no"));
    atts->add_item(IntAttribute("netto", netto, 0, 10,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("netdly", netdly, 0,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("keepalive", keepalive, 0, 10,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("uptime", uptime, 0, 10,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("standby", standby, 0,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("seqsave", seqsave, 10,
      IntAttribute::lower_bound));
    atts->add_item(StringAttribute("overlap_removal", overlap_removal));
    atts->add_item(StringAttribute("seqfile", seqfile));
    atts->add_item(StringAttribute("ifup", ifup));
    atts->add_item(StringAttribute("ifdown", ifdown));
    atts->add_item(StringAttribute("schedule", schedule_str));
    atts->add_item(StringAttribute("lockfile", lockfile));
    return atts;
  }

rc_ptr<CfgElementMap> GroupElement::start_children(ostream &cfglog,
  const string &)
try
  {
    if(address.length() == 0)
      {
        cfglog << "address of SeedLink server is not specified" << endl;
        return NULL;
      }

    if(strcasecmp(overlap_removal.c_str(), "full") &&
      strcasecmp(overlap_removal.c_str(), "initial") &&
      strcasecmp(overlap_removal.c_str(), "none"))
      {
        cfglog << "value of overlap_removal should be \"full\", "
          "\"initial\" or \"none\". Using \"none\"." << endl;
        overlap_removal = "none";
      }
        
    rc_ptr<StationGroup> group = chain.new_group(address, multi, batch, netto,
      netdly, keepalive, uptime, standby, SHUTDOWN_WAIT, seqsave, seqfile,
      ifup, ifdown, lockfile);

    if(schedule_str.length() != 0)
        group->set_schedule(schedule_str);
    
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(StationElement(group, multi, overlap_removal));
    return elms;
  }
catch(PluginError &e)
  {
    cfglog << e.message << endl;
    return NULL;
  }
        
//*****************************************************************************
// ChainElement
//*****************************************************************************

class ChainElement: public CfgElement
  {
  private:
    string &timetable_loader;
    string overlap_removal;
    int verbosity;
    bool multi;
    bool batch;
    int netto;
    int netdly;
    int keepalive;
    int standby;
    int seqsave;

  public:
    ChainElement(string &timetable_loader_init):
      CfgElement("chain"), timetable_loader(timetable_loader_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &);
  };

rc_ptr<CfgAttributeMap> ChainElement::start_attributes(ostream &cfglog,
  const string &)
  {
    overlap_removal = "none";
    verbosity = ::verbosity;
    multi = true;
    batch = true;
    netto = 0;
    netdly = 0;
    keepalive = 0;
    standby = 0;
    seqsave = 0;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("timetable_loader", timetable_loader));
    atts->add_item(StringAttribute("overlap_removal", overlap_removal));
    atts->add_item(IntAttribute("verbosity", verbosity, 0, 5));

    atts->add_item(BoolAttribute("multistation", multi, "yes", "no"));
    atts->add_item(BoolAttribute("batchmode", batch, "yes", "no"));
    atts->add_item(IntAttribute("netto", netto, 0, 10,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("netdly", netdly, 0,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("keepalive", keepalive, 0, 10,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("standby", standby, 0,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("seqsave", seqsave, 10,
      IntAttribute::lower_bound));
    
    return atts;
  }
    
void ChainElement::end_attributes(ostream &cfglog)
  {
    log_setup(verbosity);
  }

rc_ptr<CfgElementMap> ChainElement::start_children(ostream &cfglog,
  const string &)
  {
    if(strcasecmp(overlap_removal.c_str(), "full") &&
      strcasecmp(overlap_removal.c_str(), "initial") &&
      strcasecmp(overlap_removal.c_str(), "none"))
      {
        cfglog << "value of overlap_removal should be \"full\", "
          "\"initial\" or \"none\". Using \"none\"." << endl;
        overlap_removal = "none";
      }
        
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(ExtensionElement());
    elms->add_item(GroupElement(multi, batch, netto, netdly, keepalive, standby,
      seqsave, overlap_removal));
    return elms;
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
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    log_setup(2); // default verbosity before reading configuration
    
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
          case 'V': cout << ident_str << endl;
                    exit(0);
          case 'H': fprintf(stdout, help_message, get_progname(argv[0]).c_str());
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

    redirect_ostream(cout, LogFunc(), LOG_INFO);
    redirect_ostream(cerr, LogFunc(), LOG_ERR);
    redirect_ostream(clog, LogFunc(), LOG_ERR);

    logs(LOG_NOTICE) << ident_str << " started" << endl;
    
    init_qlib2(0);
    
    string timetable_loader;
    read_config_xml(config_file, ChainElement(timetable_loader));

    try
      {
        chain.setup(timetable_loader);
        while(!terminate_proc)
            chain.check();
      }
    catch(exception &e)
      {
        chain.shutdown();
        throw;
      }

    chain.shutdown();
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
 
