/*****************************************************************************
 * monitor.h
 *
 * Module "SeedlinkMonitor"
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef MONITOR_H
#define MONITOR_H

#include <string>
#include <list>
#include <new> // bad_alloc

#include <libxml/xmlIO.h>
#include <libxml/tree.h>

#include "qtime.h"

#include "buffer.h"
#include "utils.h"

namespace SeedlinkMonitor_private {

using namespace std;
using namespace Utilities;

enum InfoLevel
  { 
    IdInfo,
    CapabilityInfo,
    StationInfo,
    StreamInfo,
    GapInfo,
    ConnectionInfo,
    AllInfo,
    N_InfoLevel
  };

extern const char *const InfoLevelNames[N_InfoLevel];

//*****************************************************************************
// Exceptions
//*****************************************************************************

class MonitorError: public GenericException
  {
  public:
    MonitorError(const string &message):
      GenericException("Monitor", message) {}
  };

class MonitorRegexError: public MonitorError
  {
  public:
    MonitorRegexError(const string &errmsg):
      MonitorError(string() + "regex error: " + errmsg) {}
  };
        
class MonitorConfigError: public MonitorError
  {
  public:
    MonitorConfigError(const string &filename):
      MonitorError(string() + "error loading configuration from '" + 
        filename + "'") {}
  };
        
//*****************************************************************************
// InfoBuffer
//*****************************************************************************

class InfoBuffer: public Buffer
  {
  friend class rc_ptr<InfoBuffer>;
  private:
    void *dataptr;

    ~InfoBuffer()
      {
        free(dataptr);
      }

  public:
    InfoBuffer(int size): Buffer(size)
      {
        if((dataptr = malloc(size)) == NULL) throw bad_alloc();
      }

    void *data() const
      {
        return dataptr;
      }
  };

//*****************************************************************************
// MessageBuffer
//*****************************************************************************

class MessageBuffer
  {
  friend class MessageStore;
  friend class rc_ptr<MessageBuffer>;
  private:
    char *dataptr;
    int used_bytes;

    ~MessageBuffer()
      {
        free(dataptr);
      }

  public:
    const int capacity;

    MessageBuffer(int capacity_init): used_bytes(0), capacity(capacity_init)
      {
        if((dataptr = (char *) malloc(capacity)) == NULL) throw bad_alloc();
      }

    char *data() const
      {
        return dataptr;
      }

    int size() const
      {
        return used_bytes;
      }
  };

//*****************************************************************************
// IPACL
//*****************************************************************************

struct ip_mask
  {
    unsigned int ip;
    unsigned int mask;
    ip_mask(unsigned int ip_init, unsigned int mask_init):
      ip(ip_init), mask(mask_init) {}
  };

class IPACL
  {
  private:
    list<ip_mask> ip_mask_list;

  public:
    bool add(const string &ipstr);
    bool check(unsigned int ip) const;
    
    void clear()
      {
        ip_mask_list.clear();
      }
  };

//*****************************************************************************
// ConnectionMonitor
//*****************************************************************************

class ConnectionMonitor
  {
  public:
    virtual bool add_selector(const string &selstr) =0;
    virtual bool have_selectors() const =0;
    virtual void clear_selectors() =0;
    virtual bool set_begin_time(int year, int month, int day,
      int hour, int minute, int sec, int usec, int seqstart) =0;
    virtual bool set_end_time(int year, int month, int day,
      int hour, int minute, int sec, int usec) =0;
    virtual bool time_valid() =0;
    virtual void set_begin_seq(int seq, bool valid) =0;
    virtual int get_begin_seq() =0;
    virtual void set_realtime(bool value) =0;
    virtual void set_eod(bool value) =0;
    virtual bool end_of_data() =0;
    virtual void reset() =0;
    virtual void check_seq(int seq) =0;
    virtual void count_packet() =0;
    virtual bool match_packet(const void *head, int size) =0;
    virtual void getinfo(xmlNodePtr parent, int info_level) const =0;
    virtual ~ConnectionMonitor() {}
  };

//*****************************************************************************
// StationMonitor
//*****************************************************************************

class StationMonitor
  {
  public:
    virtual void set_begin_seq(int seq) =0;
    virtual void set_end_seq(int seq) =0;
    virtual void configure_stream_check(bool enabled, const string &regex,
      int treshold) =0;
    virtual void add_packet(int seq, const void *head, int size) =0;
    virtual void new_segment() =0;
    virtual void delete_oldest_segment() =0;
    virtual rc_ptr<ConnectionMonitor> add_connection(const string &address,
      int port) =0;
    virtual void getmsg(ostream &mout, unsigned int ipaddr) const =0;
    virtual void getinfo(xmlNodePtr parent, int info_level,
      unsigned int ipaddr) const =0;
    virtual void save_state(const string &filename) const =0;
    virtual void restore_state(const string &filename) =0;
    virtual bool ipaccess(unsigned int ipaddr) const =0;
    virtual void reset() =0;
    virtual ~StationMonitor() {}
  };

//*****************************************************************************
// MasterMonitor
//*****************************************************************************

class MasterMonitor
  {
  public:
    virtual void add_capability(const string &name, bool restricted) =0;
    virtual rc_ptr<StationMonitor> add_station(const string &name,
      const string &network, const string &description,
      const IPACL &ip_access) =0;
    virtual void cat_out(list<rc_ptr<MessageBuffer> > &buflist,
      unsigned int ipaddr) const =0;
    virtual void error_out(list<rc_ptr<InfoBuffer> > &buflist) const =0;
    virtual void info_out(list<rc_ptr<InfoBuffer> > &buflist, int info_level,
      unsigned int ipaddr) const =0;
    virtual bool iptrusted(unsigned int ipaddr) const =0;
    virtual ~MasterMonitor() {}
  };


//*****************************************************************************
// Some utility functions
//*****************************************************************************

int packet_type(const sl_fsdh_s *fsdh, int size);
double packet_sample_rate(const sl_fsdh_s *fsdh);
INT_TIME packet_begin_time(const sl_fsdh_s *fsdh);
INT_TIME packet_end_time(const sl_fsdh_s *fsdh);
int packet_type2int(const char *type);
const char *packet_type2string(int type);


//*****************************************************************************
// Entry Point
//*****************************************************************************

rc_ptr<MasterMonitor> make_master_monitor(int reclen,
  const string &info_streamname, const string &error_streamname,
  const string &software, const string &organization, const IPACL &ip_trusted);

} // namespace SeedlinkMonitor_private

namespace SeedlinkMonitor {

using SeedlinkMonitor_private::MonitorError;
using SeedlinkMonitor_private::MonitorRegexError;
using SeedlinkMonitor_private::MonitorConfigError;
using SeedlinkMonitor_private::InfoLevel;
using SeedlinkMonitor_private::IdInfo;
using SeedlinkMonitor_private::CapabilityInfo;
using SeedlinkMonitor_private::StationInfo;
using SeedlinkMonitor_private::StreamInfo;
using SeedlinkMonitor_private::ConnectionInfo;
using SeedlinkMonitor_private::N_InfoLevel;
using SeedlinkMonitor_private::InfoLevelNames;
using SeedlinkMonitor_private::InfoBuffer;
using SeedlinkMonitor_private::MessageBuffer;
using SeedlinkMonitor_private::IPACL;
using SeedlinkMonitor_private::ConnectionMonitor;
using SeedlinkMonitor_private::StationMonitor;
using SeedlinkMonitor_private::MasterMonitor;
using SeedlinkMonitor_private::packet_type;
using SeedlinkMonitor_private::packet_sample_rate;
using SeedlinkMonitor_private::packet_begin_time;
using SeedlinkMonitor_private::packet_end_time;
using SeedlinkMonitor_private::packet_type2int;
using SeedlinkMonitor_private::packet_type2string;
using SeedlinkMonitor_private::make_master_monitor;

} // namespace SeedlinkMonitor

#endif // MONITOR_H

