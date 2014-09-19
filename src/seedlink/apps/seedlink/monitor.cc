/*****************************************************************************
 * monitor.cc
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

#include <iostream>
#include <iomanip>
#include <string>
#include <list>
#include <map>
#include <set>

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cerrno>

#include <time.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <libxml/xmlIO.h>
#include <libxml/tree.h>

#include "qdefines.h"
#include "qtime.h"

#include "libslink.h"

#include "monitor.h"
#include "cppstreams.h"
#include "utils.h"
#include "descriptor.h"
#include "buffer.h"
#include "mseed.h"
#include "conf_xml.h"
#include "confattr.h"
#include "diag.h"

namespace SeedlinkMonitor_private {

using namespace std;
using namespace SProc;
using namespace CPPStreams;
using namespace CfgParser;
using namespace Utilities;

const int  ERR_SIZE     = 100;  // max size of error message
const int  CHLEN        = 3;    // length of "seedname" part of a stream id
const int  LOCLEN       = 2;    // length of "location" part of a stream id
const int  MAXSEL       = 100;  // maximum number of selectors per connection
const int  SEQ_MASK     = 0xffffff;

const char *const InfoLevelNames[N_InfoLevel] =
  { "id", "capabilities", "stations", "streams", "gaps", "connections", "all" };

//*****************************************************************************
// get_real_time()
//*****************************************************************************

INT_TIME get_real_time()
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

    return ext_to_int(et);
  }
    
//*****************************************************************************
// InfoStore
//*****************************************************************************

class InfoStore: public BufferStore
  {
  private:
    list<rc_ptr<InfoBuffer> > &buffer_list;
    const int bufsize;

  public:
    InfoStore(list<rc_ptr<InfoBuffer> > &buffer_list_init, 
      int bufsize_init):
      buffer_list(buffer_list_init), bufsize(bufsize_init) {}

    Buffer *get_buffer();
    void queue_buffer(Buffer *buf);
  };

Buffer *InfoStore::get_buffer()
  {
    return new InfoBuffer(bufsize);
  }

void InfoStore::queue_buffer(Buffer *buf)
  {
    rc_ptr<InfoBuffer> xbuf = dynamic_cast<InfoBuffer *>(buf);
    internal_check(xbuf != NULL);
    buffer_list.push_back(xbuf);
  }

//*****************************************************************************
// InfoGenerator
//*****************************************************************************

class InfoGenerator
  {
  private:
    rc_ptr<MSEEDFormat> msf;
    Packet<char> pckt;
    INT_TIME it;
    int pos;

    void iowrite(const char *buf, int len);
    void ioclose();

    static int iowrite_proxy(void *ctx, const char *buf, int len)
      {
        static_cast<InfoGenerator *>(ctx)->iowrite(buf, len);
        return len;
      }

    static int ioclose_proxy(void *ctx)
      {
        static_cast<InfoGenerator *>(ctx)->ioclose();
        return 0;
      }

  public:
    InfoGenerator(int reclen, const string &streamname,
      list<rc_ptr<InfoBuffer> > &buffer_list);
    ~InfoGenerator();
     void write_doc(xmlDocPtr doc);
  };

void InfoGenerator::iowrite(const char *buf, int len)
  {
    int nwritten = 0;

    while(nwritten < len)
      {
        if(!pckt.valid())
          {
            pckt = msf->get_packet<char>(it, 0, -1);
            pos = 0;
          }

        int size = min(pckt.datalen - pos, len - nwritten);
        memcpy(pckt.data + pos, buf + nwritten, size);
        nwritten += size;
        pos += size;

        internal_check(pos <= pckt.datalen);
        if(pos == pckt.datalen) msf->queue_packet(pckt, pos, 0);
      }
  }

void InfoGenerator::ioclose()
  {
    if(pckt.valid()) msf->queue_packet(pckt, pos, 0);
  }

InfoGenerator::InfoGenerator(int reclen, const string &streamname,
  list<rc_ptr<InfoBuffer> > &buffer_list):
  pos(0)
  {
    msf = new MSEEDFormat(new InfoStore(buffer_list, (1 << reclen)),
      reclen, LogPacket, string(), string(), string(), streamname, 0, 0);
  }

InfoGenerator::~InfoGenerator()
  {
    if(pckt.valid()) msf->queue_packet(pckt, pos, 0);
  }
    
void InfoGenerator::write_doc(xmlDocPtr doc)
  {
    it = get_real_time();
    
    xmlOutputBufferPtr obuf;
    if((obuf = xmlOutputBufferCreateIO(iowrite_proxy, ioclose_proxy, this, NULL)) == NULL)
        throw bad_alloc();

    xmlSaveFileTo(obuf, doc, NULL);
  }

//*****************************************************************************
// MessageStore
//*****************************************************************************

class MessageStore
  {
  private:
    list<rc_ptr<MessageBuffer> > &buffer_list;
    const int bufsize;

  public:
    MessageStore(list<rc_ptr<MessageBuffer> > &buffer_list_init,
      int bufsize_init):
      buffer_list(buffer_list_init), bufsize(bufsize_init) {}

    MessageBuffer *get_buffer();
    void queue_buffer(MessageBuffer *buf, int size);
  };

MessageBuffer *MessageStore::get_buffer()
  {
    return new MessageBuffer(bufsize);
  }

void MessageStore::queue_buffer(MessageBuffer *buf, int size)
  {
    buf->used_bytes = size;
    buffer_list.push_back(buf);
  }

//*****************************************************************************
// MessageOutput
//*****************************************************************************

class MessageOutput
  {
  private:
    rc_ptr<MessageStore> msgs;
    MessageBuffer *buf;
    int pos;

  public:
    enum { msglen = 100 };

    MessageOutput(list<rc_ptr<MessageBuffer> > &buffer_list, int bufsize):
      msgs(new MessageStore(buffer_list, bufsize)), buf(NULL), pos(0) {}

    ~MessageOutput();
    
    int operator()(int priority, const string &msg);
  };

MessageOutput::~MessageOutput()
  {
    if(buf != NULL) msgs->queue_buffer(buf, pos);
  }
    
int MessageOutput::operator()(int priority, const string &msg)
  {
    const char* p = msg.c_str();
    int len = msg.length();
    int nwritten = 0;

    while(nwritten < len)
      {
        if(buf == NULL)
          {
            buf = msgs->get_buffer();
            pos = 0;
          }

        int size = min(buf->capacity - pos, len - nwritten);
        memcpy(buf->data() + pos, p + nwritten, size);
        nwritten += size;
        pos += size;

        internal_check(pos <= buf->capacity);
        
        if(pos == buf->capacity)
          {
            msgs->queue_buffer(buf, pos);
            buf = NULL;
          }
      }

    return nwritten;
  }

//*****************************************************************************
// IPACL
//*****************************************************************************

bool IPACL::add(const string &ipstr)
  {
    char c;
    unsigned int ipa, ipb, ipc, ipd, maska, maskb, maskc, maskd, maskbits;
    if(sscanf(ipstr.c_str(), "%u.%u.%u.%u/%u.%u.%u.%u%c", &ipa, &ipb, &ipc,
      &ipd, &maska, &maskb, &maskc, &maskd, &c) == 8)
      {
        if(ipa > 255 || ipb > 255 || ipc > 255 || ipd > 255 ||
          maska > 255 || maskb > 255 || maskc > 255 || maskd > 255)
            return false;

        unsigned int ip = (ipa << 24) | (ipb << 16) | (ipc << 8) | ipd;
        unsigned int mask = (maska << 24) | (maskb << 16) | (maskc << 8) | maskd;
        ip_mask_list.push_back(ip_mask(ip, mask));
      }
    else if(sscanf(ipstr.c_str(), "%u.%u.%u.%u/%u%c", &ipa, &ipb, &ipc, &ipd,
      &maskbits, &c) == 5)
      {
        if(ipa > 255 || ipb > 255 || ipc > 255 || ipd > 255 ||
          maskbits > 32)
            return false;

        unsigned int ip = (ipa << 24) | (ipb << 16) | (ipc << 8) | ipd;
        unsigned int mask = ((maskbits == 0)? 0: ~((1 << (32 - maskbits)) - 1));
        ip_mask_list.push_back(ip_mask(ip, mask));
      }
    else if(sscanf(ipstr.c_str(), "%u.%u.%u.%u%c", &ipa, &ipb, &ipc, &ipd,
      &c) == 4)
      {
        if(ipa > 255 || ipb > 255 || ipc > 255 || ipd > 255)
            return false;

        unsigned int ip = (ipa << 24) | (ipb << 16) | (ipc << 8) | ipd;
        ip_mask_list.push_back(ip_mask(ip, 0xffffffff));
      }
    else
      {
        return false;
      }

    return true;
  }

bool IPACL::check(unsigned int ip) const
  {
    if(ip_mask_list.empty())
        return true;
    
    list<ip_mask>::const_iterator p;
    for(p = ip_mask_list.begin(); p != ip_mask_list.end(); ++p)
      {
        if(!((ip ^ p->ip) & p->mask))
            return true;
      }

    return false;
  }

//*****************************************************************************
// Some utility functions
//*****************************************************************************

int packet_type(const sl_fsdh_s *fsdh, int size)
  {
    int b2000 = 0;
    const sl_blkt_head_s *p = (const sl_blkt_head_s *)((const char *)(fsdh) +
      ntohs(fsdh->begin_blockette));

    do
      {
        if((const char *)(p) - (const char *)(fsdh) > size)
            return -1;

        int blkt_type = ntohs(p->blkt_type);
        int next_blkt = ntohs(p->next_blkt);

        if(blkt_type >=200 && blkt_type <= 299) return SLDET;
        if(blkt_type >=300 && blkt_type <= 399) return SLCAL;
        if(blkt_type >=500 && blkt_type <= 599) return SLTIM;
        if(blkt_type == 2000) b2000 = 1;
        p = (const sl_blkt_head_s *)((const char *)(fsdh) + next_blkt);
      }
    while((sl_fsdh_s *)p != fsdh);

    if((int16_t)ntohs(fsdh->samprate_fact) == 0)
      {
        if((int16_t)ntohs(fsdh->num_samples) != 0) return SLMSG;
        if(b2000) return SLBLK;
      }

    return SLDATA;
  }

double packet_sample_rate(const sl_fsdh_s *fsdh)
  {
    double sf = (int16_t)ntohs(fsdh->samprate_fact);
    double sm = (int16_t)ntohs(fsdh->samprate_mult);
    double retval = 0;

    if(sf > 0 && sm > 0) retval = (double)sf * (double)sm;
    else if(sf < 0 && sm < 0) retval = 1.0 / ((double)sf * (double)sm);
    else if(sf > 0 && sm < 0) retval = -(double)sf / (double)sm;
    else if(sf < 0 && sm > 0) retval = -(double)sm / (double)sf;

    return retval;
  }
    
INT_TIME packet_begin_time(const sl_fsdh_s *fsdh)
  {
    EXT_TIME et;
    et.year = ntohs(fsdh->start_time.year);
    et.doy = ntohs(fsdh->start_time.day);
    et.hour = fsdh->start_time.hour;
    et.minute = fsdh->start_time.min;
    et.second = fsdh->start_time.sec;
    et.usec = ntohs(fsdh->start_time.fract) * 100;
    dy_to_mdy(et.doy, et.year, &et.month, &et.day);
    return ext_to_int(et);
  }

INT_TIME packet_end_time(const sl_fsdh_s *fsdh)
  {
    INT_TIME start_time = packet_begin_time(fsdh);

    int samprate_fact = (int16_t)ntohs(fsdh->samprate_fact);
    int samprate_mult = (int16_t)ntohs(fsdh->samprate_mult);
    int num_samples = ntohs(fsdh->num_samples);
    
    if(samprate_fact != 0 && samprate_mult != 0)
        return add_dtime(start_time,
          1000000 * num_samples / packet_sample_rate(fsdh));

    return start_time;
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

StreamDescriptor make_stream_descriptor(const sl_fsdh_s *fsdh, int size)
  {
    int type;
    if((type = packet_type(fsdh, size)) < 0)
      {
        logs(LOG_ERR) << "could not determine packet type" << endl;
        return StreamDescriptor();
      }

    string location_id;
    for(int n = LOCLEN; n > 0; --n)
        if(fsdh->location[n - 1] != ' ')
          {
            location_id = string(fsdh->location, n);
            break;
          }
    
    string channel_id;
    for(int n = CHLEN; n > 0; --n)
        if(fsdh->channel[n - 1] != ' ')
          {
            channel_id = string(fsdh->channel, n);
            break;
          }
    
    return StreamDescriptor(location_id, channel_id, type);
  }

bool convert_time(INT_TIME &it, int year, int month,
  int day, int hour, int minute, int second, int usec)
  {
    if(year   < 1900 || year   >   2099 ||
       month  <    1 || month  >     12 ||
       day    <    1 || day    >     31 ||
       hour   <    0 || hour   >     23 ||
       minute <    0 || minute >     59 ||
       second <    0 || second >     61 ||
       usec   <    0 || usec   > 999999) return false;

    EXT_TIME et;
    et.year = year;
    et.month = month;
    et.day = day;
    et.hour = hour;
    et.minute = minute;
    et.second = second;
    et.usec = usec;
    et.doy = mdy_to_doy(et.month, et.day, et.year);

    it = ext_to_int(et);
    return true;
  }

void xml_error(void *ctx, const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    
    char errbuf[ERR_SIZE];
    vsnprintf(errbuf, ERR_SIZE, fmt, ap);

    va_end(ap);
    
    const char* p = errbuf;
    while(*p)
      {
        int len;
        if((len = strcspn(p, "\n")) > 0)
            logs(LOG_ERR) << string(p, len);

        if(p[len] == '\n')
          {
            logs(LOG_ERR) << endl;
            ++p;
          }

        p += len;
      }
  }

xmlNodePtr xml_new_child(xmlNodePtr parent, const char *name)
  {
    xmlNodePtr retval;
    if((retval = xmlNewChild(parent, NULL, (const xmlChar *) name, NULL)) == NULL)
        throw bad_alloc();

    return retval;
  }

void xml_new_prop(xmlNodePtr node, const char *name, const char *value)
  {
    if(xmlNewProp(node, (const xmlChar *) name, (const xmlChar *) value) == NULL)
        throw bad_alloc();
  }

//*****************************************************************************
// StreamSelector
//*****************************************************************************

class StreamSelector
  {
  private:
    bool neg;
    int type;
    char loc[LOCLEN];
    char chn[CHLEN];

    bool string_match_helper(const char *tmpl, const char *str, int len) const;

  public:
    bool init(const char *selstr);
    bool match(const StreamDescriptor &str_desc) const;

    string pattern() const
      {
        const char *typestr = packet_type2string(type);
        return string(neg? "!": "") + string(loc, LOCLEN) +
          string(chn, CHLEN) + "." + (typestr == NULL? "?": typestr);
      }
    
    bool negative() const
      {
        return neg;
      }
  };
    
bool StreamSelector::init(const char *selstr)
  {
    const char* p = selstr;
    const char* pt = NULL;
    int len = strcspn(selstr, ".");

    if(p[len] == '.') pt = &p[len + 1];

    neg = false;
    
    if(p[0] == '!')
      {
        neg = true;
        ++p;
        --len;
      }

    if(len != (int) strspn(p, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?"))
        return false;

    if(len == LOCLEN + CHLEN)
      {
        strncpy(loc, p, LOCLEN);
        strncpy(chn, p + LOCLEN, CHLEN);
        if((type = packet_type2int(pt)) < 0) return false;
      }
    else if(len == CHLEN)
      {
        strncpy(loc, "??", LOCLEN);
        strncpy(chn, p, CHLEN);
        if((type = packet_type2int(pt)) < 0) return false;
      }
    else if(pt == NULL)
      {
        strncpy(loc, "??", LOCLEN);
        strncpy(chn, "???", CHLEN);
        if((type = packet_type2int(p)) < 0) return false;
      }
    else
      {
        return false;
      }

    return true;
  }

bool StreamSelector::string_match_helper(const char *tmpl, const char *str, int len) const
  {
    int i;
    const char *pt, *ps;
    for(i = 0, pt = tmpl, ps = str; i < len && *pt; ++i, ++pt, ((*ps)? ++ps: 0))
        if(*pt != '?' && ((*ps)? *ps: ' ') != *pt) return false;

    return true;
  }

bool StreamSelector::match(const StreamDescriptor &str_desc) const
  {
    return ((type == SLNUM || type == str_desc.type) &&
      string_match_helper(loc, str_desc.location.c_str(), LOCLEN) &&
      string_match_helper(chn, str_desc.seedname.c_str(), CHLEN));
  }

//*****************************************************************************
// StreamFilter
//*****************************************************************************

class StreamFilter
  {
  private:
    list<StreamSelector> sels;
  
  public:
    bool add_selector(const string &selstr);
    bool have_selectors() const;
    void clear_selectors();
    bool match(const StreamDescriptor &str_desc) const;
    void getinfo(xmlNodePtr parent, int info_level) const;
  };

bool StreamFilter::add_selector(const string &selstr)
  {
    StreamSelector sel;

    if((int) sels.size() >= MAXSEL || !sel.init(selstr.c_str())) return false;
    sels.push_back(sel);
    return true;
  }

bool StreamFilter::have_selectors() const
  {
    return !sels.empty();
  }

void StreamFilter::clear_selectors()
  {
    sels.clear();
  }

bool StreamFilter::match(const StreamDescriptor &str_desc) const
  {
    bool default_rule = true, result = false;
    list<StreamSelector>::const_iterator p;
    
    if(!sels.size()) return true;
    
    for(p = sels.begin(); p != sels.end(); ++p)
      {
        if(p->negative())
          {
            if(p->match(str_desc)) return false;
          }
        else
          {
            default_rule = false;
            result |= p->match(str_desc);
          }
      }
        
    return (default_rule || result);
  }

void StreamFilter::getinfo(xmlNodePtr parent, int info_level) const
  {
    xmlNodePtr child;

    list<StreamSelector>::const_iterator p;
    for(p = sels.begin(); p != sels.end(); ++p)
      {
        child = xml_new_child(parent, "selector");
        xml_new_prop(child, "pattern", p->pattern().c_str());
      }
  }

//*****************************************************************************
// TimeWindow
//*****************************************************************************

class TimeWindow
  {
  private:
    INT_TIME begin_time;
    INT_TIME end_time;
    bool begin_time_initialized;
    bool end_time_initialized;

  public:
    TimeWindow(): begin_time_initialized(false), end_time_initialized(false) {}

    bool valid();
    int time_cmp(INT_TIME it);
    void getinfo(xmlNodePtr parent, int info_level) const;
    
    void set_begin_time(INT_TIME it)
      {
        begin_time = it;
        begin_time_initialized = true;
      }
    
    void set_end_time(INT_TIME it)
      {
        end_time = it;
        end_time_initialized = true;
      }
    
    void reset()
      {
        begin_time_initialized = false;
        end_time_initialized = false;
      }
  };
    
bool TimeWindow::valid()
  {
    if(!begin_time_initialized) return true;

    return (!end_time_initialized || tdiff(end_time, begin_time) > 0);
  }

int TimeWindow::time_cmp(INT_TIME it)
  {
    if(!begin_time_initialized) return 0;

    if(tdiff(it, begin_time) < 0) return -1;

    if(end_time_initialized && tdiff(it, end_time) > 0) return 1;

    return 0;
  }

void TimeWindow::getinfo(xmlNodePtr parent, int info_level) const
  {
    if(!begin_time_initialized) return;

    xmlNodePtr child;
    child = xml_new_child(parent, "window");
    xml_new_prop(child, "begin_time", time_to_str(begin_time, MONTHS_FMT));
    xml_new_prop(child, "end_time", end_time_initialized ? time_to_str(end_time, MONTHS_FMT): "unset");
  }

//*****************************************************************************
// ConnectionMonitorImpl
//*****************************************************************************

class ConnectionMonitorPartner
  {
  public:
    virtual list<ConnectionMonitor *>::iterator attach(ConnectionMonitor *cw) =0;
    virtual void detach(list<ConnectionMonitor *>::iterator ptr) =0;
    virtual int time_to_seq(INT_TIME it, int seqstart, const StreamFilter &filter) const =0;
    virtual int time_cmp(INT_TIME it, const StreamFilter &filter) const =0;
    virtual ~ConnectionMonitorPartner() {}
  };

class ConnectionMonitorImpl: public ConnectionMonitor
  {
  private:
    const string address;
    const int port;
    INT_TIME ctime;
    TimeWindow win;
    StreamFilter filter;
    int begin_seq;
    int current_seq;
    int sequence_gaps;
    int txcount;
    bool begin_seq_valid;
    bool realtime;
    bool eod;
    bool check_eod;
    set<StreamDescriptor> stream_set;
    ConnectionMonitorPartner &partner;
    list<ConnectionMonitor *>::iterator cw_link;

  public:
    ConnectionMonitorImpl(ConnectionMonitorPartner &partner_init,
      const string &address_init, int port_init):
      address(address_init), port(port_init), begin_seq(-1), current_seq(-1),
      sequence_gaps(0), txcount(0), begin_seq_valid(false), realtime(true),
      eod(false), check_eod(false), partner(partner_init)
      {
        cw_link = partner.attach(this);
        ctime = get_real_time();
      }

    ~ConnectionMonitorImpl()
      {
        partner.detach(cw_link);
      }

    bool set_begin_time(int year, int month, int day,
      int hour, int minute, int sec, int usec, int seqstart);
    bool set_end_time(int year, int month, int day,
      int hour, int minute, int sec, int usec);
    void check_seq(int seq);
    void count_packet();
    bool match_packet(const void *head, int size);
    void getinfo(xmlNodePtr parent, int info_level) const;

    bool add_selector(const string &selstr)
      {
        return filter.add_selector(selstr);
      }
    
    bool have_selectors() const
      {
        return filter.have_selectors();
      }
    
    void clear_selectors()
      {
        filter.clear_selectors();
      }

    bool time_valid()
      {
        return win.valid();
      }
    
    void set_begin_seq(int seq, bool valid)
      {
        begin_seq = seq;
        begin_seq_valid = valid;
        current_seq = (valid? seq: -1);
      }
    
    int get_begin_seq()
      {
        return begin_seq;
      }
    
    void set_realtime(bool value)
      {
        realtime = value;
      }

    void set_eod(bool value)
      {
        eod = value;
      }
    
    bool end_of_data()
      {
        return eod;
      }

    void reset()
      {
        win.reset();
        stream_set.clear();
        realtime = true;
        eod = false;
        check_eod = false;
        begin_seq = -1;
        current_seq = -1;
      }
  };

bool ConnectionMonitorImpl::set_begin_time(int year, int month, int day,
  int hour, int minute, int sec, int usec, int seqstart)
  {
    INT_TIME it;
    if(convert_time(it, year, month, day, hour, minute, sec, usec))
      {
        win.set_begin_time(it);
        
        if(partner.time_cmp(it, filter) > 0)
          {
            begin_seq = -1;
            current_seq = -1;
//          eod = true;
          }
        else
          {
            begin_seq = partner.time_to_seq(it, seqstart, filter);
            begin_seq_valid = (seqstart == -1 || begin_seq == seqstart);
            current_seq = ((seqstart == -1)? begin_seq: seqstart);
          }

        return true;
      }

    return false;
  }

bool ConnectionMonitorImpl::set_end_time(int year, int month, int day,
  int hour, int minute, int sec, int usec)
  {
    INT_TIME it;
    if(convert_time(it, year, month, day, hour, minute, sec, usec))
      {
        win.set_end_time(it);
        check_eod = true;
        eod |= (partner.time_cmp(it, filter) < 0);
        return true;
      }

    return false;
  }

void ConnectionMonitorImpl::check_seq(int seq)
  {
    if(current_seq != -1 && seq != current_seq)
        ++sequence_gaps;

    current_seq = (seq + 1) & SEQ_MASK;
  }

void ConnectionMonitorImpl::count_packet()
  {
    ++txcount;
  }

bool ConnectionMonitorImpl::match_packet(const void *head, int size)
  {
    if(*(char *)head == 0) // empty packet
        return false;

    const sl_fsdh_s* fsdh = static_cast<const sl_fsdh_s *>(head);
    StreamDescriptor str_desc = make_stream_descriptor(fsdh, size);

    if(str_desc.type < 0)
        return false;
    
    if(!filter.match(str_desc) || win.time_cmp(packet_end_time(fsdh)) < 0)
        return false;

    if(check_eod && !eod)
      {
        if(win.time_cmp(packet_begin_time(fsdh)) <= 0)
          {
            stream_set.insert(str_desc);
          }
        else
          {
            set<StreamDescriptor>::iterator p;
            if((p = stream_set.find(str_desc)) != stream_set.end())
                stream_set.erase(p);

            if(stream_set.empty()) eod = true;
            return false;
          }
      }

    return true;
  }

void ConnectionMonitorImpl::getinfo(xmlNodePtr parent, int info_level) const
  {
    char buf[20];
    xmlNodePtr child;

    child = xml_new_child(parent, "connection");
    xml_new_prop(child, "host", address.c_str());

    snprintf(buf, 7, "%d", port);
    xml_new_prop(child, "port", buf);

    xml_new_prop(child, "ctime", time_to_str(ctime, MONTHS_FMT));

    if(begin_seq == -1) strcpy(buf, "unset");
    else snprintf(buf, 8, "%06X", begin_seq);

    xml_new_prop(child, "begin_seq", buf);

    if(current_seq == -1) strcpy(buf, "unset");
    else snprintf(buf, 8, "%06X", current_seq);

    xml_new_prop(child, "current_seq", buf);

    snprintf(buf, 19, "%d", sequence_gaps);
    xml_new_prop(child, "sequence_gaps", buf);

    snprintf(buf, 19, "%d", txcount);
    xml_new_prop(child, "txcount", buf);

    xml_new_prop(child, "begin_seq_valid", (begin_seq_valid ? "yes": "no"));
    xml_new_prop(child, "realtime", (realtime ? "yes": "no"));
    xml_new_prop(child, "end_of_data", (eod ? "yes": "no"));

    win.getinfo(child, info_level);
    filter.getinfo(child, info_level);
  }

//*****************************************************************************
// StreamMonitor
//*****************************************************************************

struct DataGap
  {
    INT_TIME begin_time;
    INT_TIME end_time;
    DataGap(INT_TIME begin_time_init, INT_TIME end_time_init):
      begin_time(begin_time_init), end_time(end_time_init) {}
  };

struct DataSegment
  {
    INT_TIME end_time;
    int end_recno;
    int end_seq;
    list<DataGap> gaps;
  };

class StreamMonitor
  {
  private:
    INT_TIME begin_time;
    int begin_recno;
    bool gap_check;
    int gap_treshold;
    int begin_seq;
    list<DataSegment> segments;
    list<DataSegment>::iterator current_segment;
    int segment_count;

  public:
    const StreamDescriptor name;
    
    StreamMonitor(const StreamDescriptor &name_init, bool gap_check_init,
      int gap_treshold_init, INT_TIME begin_time_init, INT_TIME end_time,
      int begin_recno_init, int end_recno, int begin_seq_init, int end_seq):
      begin_time(begin_time_init), begin_recno(begin_recno_init),
      gap_check(gap_check_init), gap_treshold(gap_treshold_init),
      begin_seq(begin_seq_init), segment_count(1), name(name_init)
      {
        current_segment = segments.insert(segments.end(), DataSegment());
        current_segment->end_time = end_time;
        current_segment->end_recno = end_recno;
        current_segment->end_seq = end_seq;
      }
    
    StreamMonitor(const StreamDescriptor &name_init, bool gap_check_init,
      int gap_treshold_init):
      gap_check(gap_check_init), gap_treshold(gap_treshold_init),
      begin_seq(-1), segment_count(1), name(name_init)
      {
        memset(&begin_time, 0, sizeof(INT_TIME));

        current_segment = segments.insert(segments.end(), DataSegment());
        memset(&current_segment->end_time, 0, sizeof(INT_TIME));
        current_segment->end_seq = -1;
      }
    
    void init_segment(INT_TIME end_time, int end_recno, int end_seq);
    void init_gap(INT_TIME begin_time, INT_TIME end_time);
    void add_packet(INT_TIME start_time, int samples, double rate,
      bool rate_defined, int recno, int seq);
    int time_to_seq(INT_TIME it) const;
    int time_cmp(INT_TIME it) const;
    void new_segment();
    void delete_oldest_segment(int station_segment_count);
    bool empty() const;
    void getinfo(xmlNodePtr parent, int info_level) const;
    void getstate(xmlNodePtr parent) const;
  };

void StreamMonitor::init_segment(INT_TIME end_time, int end_recno, int end_seq)
  {
    list<DataSegment>::iterator p;
    p = segments.insert(segments.end(), DataSegment());
    p->end_time = current_segment->end_time;
    p->end_recno = current_segment->end_recno;
    p->end_seq = current_segment->end_seq;
    current_segment->end_time = end_time;
    current_segment->end_recno = end_recno;
    current_segment->end_seq = end_seq;
    current_segment = p;
    ++segment_count;
  }
  
void StreamMonitor::init_gap(INT_TIME begin_time, INT_TIME end_time)
  {
    current_segment->gaps.push_back(DataGap(begin_time, end_time));
  }

void StreamMonitor::add_packet(INT_TIME start_time, int samples, double rate,
  bool rate_defined, int recno, int seq)
  {
    if(begin_seq < 0)
      {
        begin_time = start_time;
        begin_recno = recno;
        begin_seq = seq;
      }
    else if(gap_check && rate_defined && name.type == SLDATA)
      {
        double td = tdiff(current_segment->end_time, start_time);

        if(td < -gap_treshold || td > gap_treshold)
            current_segment->gaps.push_back(DataGap(current_segment->end_time,
              start_time));
      }
    
    if(rate_defined)
        current_segment->end_time = add_dtime(start_time,
          1000000 * samples / rate);
    else
        current_segment->end_time = start_time;

    current_segment->end_recno = (recno + 1) % 1000000;
    current_segment->end_seq = (seq + 1) & SEQ_MASK;
  }

int StreamMonitor::time_to_seq(INT_TIME it) const
  {
    int retval = begin_seq;
    list<DataSegment>::const_iterator p;
    for(p = segments.begin(); p != segments.end(); ++p)
      {
        if(tdiff(p->end_time, it) > 0) break;
        retval = p->end_seq;
      }

    return retval;
  }

int StreamMonitor::time_cmp(INT_TIME it) const
  {
    if(begin_seq < 0) return 0;
    
    if(tdiff(it, begin_time) < 0) return -1;

    if(tdiff(it, current_segment->end_time) > 0) return 1;

    return 0;
  }

void StreamMonitor::new_segment()
  {
    list<DataSegment>::iterator p;
    p = segments.insert(segments.end(), DataSegment());
    p->end_time = current_segment->end_time;
    p->end_recno = current_segment->end_recno;
    p->end_seq = current_segment->end_seq;
    current_segment = p;
    ++segment_count;
  }

void StreamMonitor::delete_oldest_segment(int station_segment_count)
  {
    if(station_segment_count > segment_count) return;
    
    internal_check(!segments.empty());

    begin_time = segments.front().end_time;
    begin_recno = segments.front().end_recno;
    begin_seq = segments.front().end_seq;
    segments.pop_front();
    --segment_count;
  }

bool StreamMonitor::empty() const
  {
    return (begin_seq == current_segment->end_seq);
  }

void StreamMonitor::getinfo(xmlNodePtr parent, int info_level) const
  {
    char buf[20];
    xmlNodePtr child;

    child = xml_new_child(parent, "stream");
    xml_new_prop(child, "location", name.location.c_str());
    xml_new_prop(child, "seedname", name.seedname.c_str());
    xml_new_prop(child, "type", packet_type2string(name.type));
    xml_new_prop(child, "begin_time", time_to_str(begin_time, MONTHS_FMT));
    xml_new_prop(child, "end_time", time_to_str(current_segment->end_time, MONTHS_FMT));

    snprintf(buf, 19, "%06d", begin_recno);
    xml_new_prop(child, "begin_recno", buf);

    snprintf(buf, 19, "%06d", current_segment->end_recno);
    xml_new_prop(child, "end_recno", buf);

    xml_new_prop(child, "gap_check", (gap_check ? "enabled": "disabled"));

    snprintf(buf, 19, "%d", gap_treshold);
    xml_new_prop(child, "gap_treshold", buf);

    if(info_level >= GapInfo)
      {
        list<DataSegment>::const_iterator p;
        for(p = segments.begin(); p != segments.end(); ++p)
          {
            list<DataGap>::const_iterator p1;
            for(p1 = p->gaps.begin(); p1 != p->gaps.end(); ++p1)
              {
                xmlNodePtr child1;
                child1 = xml_new_child(child, "gap");
                xml_new_prop(child1, "begin_time", time_to_str(p1->begin_time, MONTHS_FMT));
                xml_new_prop(child1, "end_time", time_to_str(p1->end_time, MONTHS_FMT));
              }
          }
      }
  }

void StreamMonitor::getstate(xmlNodePtr parent) const
  {
    char buf[20];
    xmlNodePtr child = xml_new_child(parent, "stream");
    xml_new_prop(child, "location", name.location.c_str());
    xml_new_prop(child, "seedname", name.seedname.c_str());
    xml_new_prop(child, "type", packet_type2string(name.type));
    xml_new_prop(child, "begin_time", time_to_str(begin_time, MONTHS_FMT));
    xml_new_prop(child, "end_time", time_to_str(current_segment->end_time, MONTHS_FMT));

    snprintf(buf, 19, "%06d", begin_recno);
    xml_new_prop(child, "begin_recno", buf);

    snprintf(buf, 19, "%06d", current_segment->end_recno);
    xml_new_prop(child, "end_recno", buf);

    snprintf(buf, 8, "%06X", begin_seq);
    xml_new_prop(child, "begin_seq", buf);

    snprintf(buf, 8, "%06X", current_segment->end_seq);
    xml_new_prop(child, "end_seq", buf);

    xml_new_prop(child, "gap_check", (gap_check ? "enabled": "disabled"));

    snprintf(buf, 19, "%d", gap_treshold);
    xml_new_prop(child, "gap_treshold", buf);

    list<DataSegment>::const_iterator p;
    for(p = segments.begin(); p != segments.end(); ++p)
      {
        xmlNodePtr child1 = xml_new_child(child, "segment");
        xml_new_prop(child1, "end_time", time_to_str(p->end_time, MONTHS_FMT));
        
        snprintf(buf, 19, "%06d", p->end_recno);
        xml_new_prop(child1, "end_recno", buf);

        snprintf(buf, 8, "%06X", p->end_seq);
        xml_new_prop(child1, "end_seq", buf);

        list<DataGap>::const_iterator p1;
        for(p1 = p->gaps.begin(); p1 != p->gaps.end(); ++p1)
          {
            xmlNodePtr child2 = xml_new_child(child1, "gap");
            xml_new_prop(child2, "begin_time", time_to_str(p1->begin_time, MONTHS_FMT));
            xml_new_prop(child2, "end_time", time_to_str(p1->end_time, MONTHS_FMT));
          }
      }
  }

//*****************************************************************************
// StreamTypeAttribute
//*****************************************************************************

class StreamTypeAttribute: public CfgAttribute
  {
  private:
    int &valref;

  public:
    StreamTypeAttribute(const string &name, int &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        int n;
        
        if((n = packet_type2int(value.c_str())) < 0)
            return false;
        
        valref = n;
        return true;
      }
  };

//*****************************************************************************
// SeqAttribute
//*****************************************************************************

class SeqAttribute: public CfgAttribute
  {
  private:
    int &valref;

  public:
    SeqAttribute(const string &name, int &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        unsigned int n;
        char c;
        
        if(sscanf(value.c_str(), "%X%c", &n, &c) != 1 || (n & ~SEQ_MASK))
            return false;

        valref = n;
        return true;
      }
  };

//*****************************************************************************
// TimeAttribute
//*****************************************************************************

class TimeAttribute: public CfgAttribute
  {
  private:
    INT_TIME &valref;

  public:
    TimeAttribute(const string &name, INT_TIME &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        int year, month, day, hour, min, sec, tms;
        char c;
        
        if(sscanf(value.c_str(), "%d/%d/%d %d:%d:%d.%d%c", &year, &month, &day,
          &hour, &min, &sec, &tms, &c) != 7)
            return false;

        return convert_time(valref, year, month, day, hour, min, sec, tms * 100);
      }
  };

//*****************************************************************************
// GapElement
//*****************************************************************************

class GapElement: public CfgElement
  {
  private:
    INT_TIME begin_time;
    INT_TIME end_time;
    rc_ptr<StreamMonitor> strm;
    bool &error;

  public:
    GapElement(rc_ptr<StreamMonitor> strm_init, bool &error_init):
      CfgElement("gap"), strm(strm_init), error(error_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> GapElement::start_attributes(ostream &cfglog,
  const string &)
  {
    memset(&begin_time, 0, sizeof(INT_TIME));
    memset(&end_time, 0, sizeof(INT_TIME));
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(TimeAttribute("begin_time", begin_time));
    atts->add_item(TimeAttribute("end_time", end_time));
    return atts;
  }

void GapElement::end_attributes(ostream &cfglog)
  {
    if(begin_time.year == 0)
      {
        cfglog << "begin_time is invalid or unspecified" << endl;
        error = true;
      }
  
    if(end_time.year == 0)
      {
        cfglog << "end_time is invalid or unspecified" << endl;
        error = true;
      }

    if(error) return;

    strm->init_gap(begin_time, end_time);
  }

//*****************************************************************************
// SegmentElement
//*****************************************************************************

class SegmentElement: public CfgElement
  {
  private:
    INT_TIME end_time;
    int end_recno;
    int end_seq;
    rc_ptr<StreamMonitor> strm;
    bool &error;
    bool first;

  public:
    SegmentElement(rc_ptr<StreamMonitor> strm_init, bool &error_init):
      CfgElement("segment"), strm(strm_init), error(error_init), first(true) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> SegmentElement::start_attributes(ostream &cfglog,
  const string &)
  {
    if(!first && !error)
        strm->init_segment(end_time, end_recno, end_seq);
    
    memset(&end_time, 0, sizeof(INT_TIME));
    end_recno = -1;
    end_seq = -1;
    first = false;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(TimeAttribute("end_time", end_time));
    atts->add_item(IntAttribute("end_recno", end_recno, 0, 999999));
    atts->add_item(SeqAttribute("end_seq", end_seq));
    return atts;
  }

void SegmentElement::end_attributes(ostream &cfglog)
  {
    if(end_time.year == 0)
      {
        cfglog << "end_time is invalid or unspecified" << endl;
        error = true;
      }
  
    if(end_recno == -1)
      {
        cfglog << "end_recno is invalid or unspecified" << endl;
        error = true;
      }
  
    if(end_seq == -1)
      {
        cfglog << "end_seq is invalid or unspecified" << endl;
        error = true;
      }
  }

rc_ptr<CfgElementMap> SegmentElement::start_children(ostream &cfglog,
  const string &)
  {
    if(error) return NULL;
    
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(GapElement(strm, error));
    return elms;
  }

//*****************************************************************************
// StreamElement
//*****************************************************************************

class StreamElement: public CfgElement
  {
  private:
    string location;
    string seedname;
    int type;
    INT_TIME begin_time;
    INT_TIME end_time;
    bool gap_check;
    int gap_treshold;
    int begin_recno;
    int end_recno;
    int begin_seq;
    int end_seq;
    rc_ptr<StreamMonitor> strm;
    map<StreamDescriptor, rc_ptr<StreamMonitor> > &stream_map;
    bool &error;

  public:
    StreamElement(map<StreamDescriptor, rc_ptr<StreamMonitor> > &stream_map_init,
      bool &error_init):
      CfgElement("stream"), stream_map(stream_map_init), error(error_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> StreamElement::start_attributes(ostream &cfglog,
  const string &)
  {
    location = "";
    seedname = "";
    type = -1;
    memset(&begin_time, 0, sizeof(INT_TIME));
    memset(&end_time, 0, sizeof(INT_TIME));
    gap_check = false;
    gap_treshold = 0;
    begin_recno = -1;
    end_recno = -1;
    begin_seq = -1;
    end_seq = -1;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("location", location));
    atts->add_item(StringAttribute("seedname", seedname));
    atts->add_item(StreamTypeAttribute("type", type));
    atts->add_item(TimeAttribute("begin_time", begin_time));
    atts->add_item(TimeAttribute("end_time", end_time));
    atts->add_item(IntAttribute("begin_recno", begin_recno));
    atts->add_item(IntAttribute("end_recno", end_recno));
    atts->add_item(BoolAttribute("gap_check", gap_check, "enabled", "disabled"));
    atts->add_item(IntAttribute("gap_treshold", gap_treshold, 100,
      IntAttribute::lower_bound));
    atts->add_item(IntAttribute("begin_recno", begin_recno, 0, 999999));
    atts->add_item(IntAttribute("end_recno", end_recno, 0, 999999));
    atts->add_item(SeqAttribute("begin_seq", begin_seq));
    atts->add_item(SeqAttribute("end_seq", end_seq));
    return atts;
  }

void StreamElement::end_attributes(ostream &cfglog)
  {
    if(location.length() > 2)
      {
        cfglog << "location is invalid or unspecified" << endl;
        error = true;
      }
        
    if(seedname.empty() || seedname.length() > 3)
      {
        cfglog << "seedname is invalid or unspecified" << endl;
        error = true;
      }
    
    if(type == -1)
      {
        cfglog << "type is invalid or unspecified" << endl;
        error = true;
      }

    if(begin_time.year == 0)
      {
        cfglog << "begin_time is invalid or unspecified" << endl;
        error = true;
      }
  
    if(end_time.year == 0)
      {
        cfglog << "end_time is invalid or unspecified" << endl;
        error = true;
      }
  
    if(gap_treshold == 0)
      {
        cfglog << "gap_treshold is invalid or unspecified" << endl;
        error = true;
      }

    if(begin_recno == -1)
      {
        cfglog << "begin_recno is invalid or unspecified" << endl;
        error = true;
      }

    if(end_recno == -1)
      {
        cfglog << "end_recno is invalid or unspecified" << endl;
        error = true;
      }

    if(begin_seq == -1)
      {
        cfglog << "begin_seq is invalid or unspecified" << endl;
        error = true;
      }

    if(end_seq == -1)
      {
        cfglog << "end_seq is invalid or unspecified" << endl;
        error = true;
      }

    if(error) return;
    
    StreamDescriptor str_desc(location, seedname, type);

    if(stream_map.find(str_desc) != stream_map.end())
      {
        cfglog << "multiple definitions of stream " << location <<
          " " << seedname << " " << packet_type2string(type) << endl;
        error = true;
        return;
      }

    strm = new StreamMonitor(str_desc, gap_check, gap_treshold,
      begin_time, end_time, begin_recno, end_recno, begin_seq, end_seq);

    stream_map.insert(make_pair(str_desc, strm));
  }

rc_ptr<CfgElementMap> StreamElement::start_children(ostream &cfglog,
  const string &)
  {
    if(error) return NULL;
    
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(SegmentElement(strm, error));
    return elms;
  }

//*****************************************************************************
// StationMonitorImpl
//*****************************************************************************

class StationMonitorPartner
  {
  public:
    virtual list<StationMonitor *>::iterator attach(StationMonitor *cw) =0;
    virtual void detach(list<StationMonitor *>::iterator ptr) =0;
    virtual ~StationMonitorPartner() {}
  };

class StationMonitorImpl: public StationMonitor, private ConnectionMonitorPartner
  {
  private:
    const string name;
    const string network;
    const string description;
    const IPACL ip_access;
    int begin_seq;
    int end_seq;
    map<StreamDescriptor, rc_ptr<StreamMonitor> > stream_map;
    int segment_count;
    bool stream_check;
    int gap_treshold;
    regex_t gap_check_rx;
    bool gap_check_rx_initialized;
    list<ConnectionMonitor *> cws;
    StationMonitorPartner &partner;
    list<StationMonitor *>::iterator sw_link;

    list<ConnectionMonitor *>::iterator attach(ConnectionMonitor *cw);
    void detach(list<ConnectionMonitor *>::iterator ptr);
    
    int time_to_seq(INT_TIME it, int seqstart, const StreamFilter &filter) const;
    int time_cmp(INT_TIME it, const StreamFilter &filter) const;

  public:
    StationMonitorImpl(StationMonitorPartner &partner_init,
      const string &name_init, const string &network_init,
      const string &description_init, const IPACL &ip_access_init):
      name(name_init), network(network_init), description(description_init),
      ip_access(ip_access_init), begin_seq(0), end_seq(0), segment_count(1),
      stream_check(false), gap_check_rx_initialized(false),
      partner(partner_init)
      {
        sw_link = partner.attach(this);
      }

    ~StationMonitorImpl();

    void configure_stream_check(bool enabled, const string &regex,
      int treshold);
    void add_packet(int seq, const void *head, int size);
    void new_segment();
    void delete_oldest_segment();

    rc_ptr<ConnectionMonitor> add_connection(const string &address, int port);

    void getmsg(ostream &mout, unsigned int ipaddr) const;
    void getinfo(xmlNodePtr parent, int info_level, unsigned int ipaddr) const;
    void save_state(const string &filename) const;
    void restore_state(const string &filename);

    bool ipaccess(unsigned int ipaddr) const
      {
        return ip_access.check(ipaddr);
      }

    void set_begin_seq(int seq)
      {
        begin_seq = (seq & SEQ_MASK);
      }

    void set_end_seq(int seq)
      {
        end_seq = (seq & SEQ_MASK);
      }

    void reset()
      {
        begin_seq = 0;
        end_seq = 0;
        segment_count = 1;
        // stream_check = false;
        stream_map.clear();
      }
  };

StationMonitorImpl::~StationMonitorImpl()
  {
    if(gap_check_rx_initialized) regfree(&gap_check_rx);
    
    if(!cws.empty())
      {
        // It is better to use plain printf() here, because stream objects
        // can be already destroyed when this destructor is called. However,
        // this message should never appear unless there is a serious bug
        // in the program.
        printf("deleting StationMonitor object, while some ConnectionMonitor "
          "objects still exist\n");
      }

    partner.detach(sw_link);
  }

list<ConnectionMonitor *>::iterator StationMonitorImpl::attach(ConnectionMonitor *cw)
  {
    return cws.insert(cws.end(), cw);
  }

void StationMonitorImpl::detach(list<ConnectionMonitor *>::iterator ptr)
  {
    cws.erase(ptr);
  }

int StationMonitorImpl::time_to_seq(INT_TIME it, int seqstart,
  const StreamFilter &filter) const
  {
    int retval = end_seq;
    int distance = SEQ_MASK + 1;
    map<StreamDescriptor, rc_ptr<StreamMonitor> >::const_iterator p;
    for(p = stream_map.begin(); p != stream_map.end(); ++p)
      {
        if(!filter.match(p->first)) continue;

        if(distance == SEQ_MASK + 1)
          {
            distance = (end_seq - begin_seq) & SEQ_MASK;
            retval = begin_seq;
          }
        
        int seq = p->second->time_to_seq(it);
        if(((seq - begin_seq) & SEQ_MASK) < distance)
          {
            distance = (seq - begin_seq) & SEQ_MASK;
            retval = seq;
          }
      }

    if(seqstart >=0 &&
      ((end_seq - seqstart) & SEQ_MASK) < ((end_seq - retval) & SEQ_MASK))
        return seqstart;
    
    return retval;
  }

int StationMonitorImpl::time_cmp(INT_TIME it, const StreamFilter &filter) const
  {
    int retval = 0;
    map<StreamDescriptor, rc_ptr<StreamMonitor> >::const_iterator p;
    for(p = stream_map.begin(); p != stream_map.end(); ++p)
      {
        if(!filter.match(p->first)) continue;

        int d = p->second->time_cmp(it);

        if(retval == 0) retval = d;

        if(d == 0 || d != retval) return 0;
      }

    return retval;
  }
 
void StationMonitorImpl::configure_stream_check(bool enabled,
  const string &regex, int treshold)
  {
    if(gap_check_rx_initialized)
      {
        regfree(&gap_check_rx);
        gap_check_rx_initialized = false;
      }

    if((stream_check = enabled) == false || regex.length() == 0)
      {
        gap_treshold = 0;
        return;
      }

    if((gap_treshold = treshold) == 0)
        return;
    
    int err = regcomp(&gap_check_rx, regex.c_str(),
      REG_EXTENDED | REG_ICASE | REG_NOSUB);

    if(err != 0)
      {
        char errbuf[ERR_SIZE];
        regerror(err, &gap_check_rx, errbuf, ERR_SIZE);
        throw MonitorRegexError(errbuf);
      }

    gap_check_rx_initialized = true;
  }

void StationMonitorImpl::add_packet(int seq, const void *head, int size)
  {
    if(!stream_check) return;
    
    if(*(char *)head == 0) // empty packet
        return;

    const sl_fsdh_s* fsdh = static_cast<const sl_fsdh_s *>(head);
    StreamDescriptor str_desc = make_stream_descriptor(fsdh, size);

    if(str_desc.type < 0)
        return;
    
    map<StreamDescriptor, rc_ptr<StreamMonitor> >::iterator p;
    if((p = stream_map.find(str_desc)) == stream_map.end())
      {
        bool gap_check = (str_desc.type == SLDATA && gap_check_rx_initialized &&
          regexec(&gap_check_rx, (str_desc.location + str_desc.seedname).c_str(),
          0, NULL, 0) == 0);
    
        p = stream_map.insert(make_pair(str_desc,
          new StreamMonitor(str_desc, gap_check, gap_treshold))).first;
      }

    bool rate_defined = ((int16_t)ntohs(fsdh->samprate_fact) != 0 &&
      (int16_t)ntohs(fsdh->samprate_mult) != 0);
    
    int recno;
    sscanf(fsdh->sequence_number, "%6d", &recno);
    
    p->second->add_packet(packet_begin_time(fsdh), ntohs(fsdh->num_samples),
      packet_sample_rate(fsdh), rate_defined, recno, seq);

    // if(seq != end_seq)
    //     logs(LOG_ERR) << "sequence gap: end_seq = " << end_seq << ", "
    //       "seq = " << seq << endl;
  }

void StationMonitorImpl::new_segment()
  {
    if(!stream_check) return;

    map<StreamDescriptor, rc_ptr<StreamMonitor> >::iterator p;
    for(p = stream_map.begin(); p != stream_map.end(); ++p)
        p->second->new_segment();

    ++segment_count;
  }

void StationMonitorImpl::delete_oldest_segment()
  {
    if(!stream_check) return;

    map<StreamDescriptor, rc_ptr<StreamMonitor> >::iterator p = stream_map.begin();
    while(p != stream_map.end())
      {
        p->second->delete_oldest_segment(segment_count);

        if(p->second->empty()) stream_map.erase(p++);
        else ++p;
      }

    --segment_count;
  }

rc_ptr<ConnectionMonitor> StationMonitorImpl::add_connection(const string &address,
  int port)
  {
    return new ConnectionMonitorImpl(*this, address, port);
  }

void StationMonitorImpl::getmsg(ostream &mout, unsigned int ipaddr) const
  {
    if(!ipaccess(ipaddr)) return;

    mout << setw(2) << right << network << " "
         << setw(5) << left << name << " "
         << description << endl;
  }
 
void StationMonitorImpl::getinfo(xmlNodePtr parent, int info_level,
  unsigned int ipaddr) const
  {
    if(!ipaccess(ipaddr)) return;

    char buf[10];
    xmlNodePtr child;

    child = xml_new_child(parent, "station");
    xml_new_prop(child, "name", name.c_str());
    xml_new_prop(child, "network", network.c_str());
    xml_new_prop(child, "description", description.c_str());

    snprintf(buf, 8, "%06X", begin_seq);
    xml_new_prop(child, "begin_seq", buf);

    snprintf(buf, 8, "%06X", end_seq);
    xml_new_prop(child, "end_seq", buf);

    xml_new_prop(child, "stream_check", (stream_check ? "enabled": "disabled"));

    if(((info_level >= StreamInfo && info_level <= GapInfo) ||
      info_level == AllInfo) && stream_check)
      {
        map<StreamDescriptor, rc_ptr<StreamMonitor> >::const_iterator p;
        for(p = stream_map.begin(); p != stream_map.end(); ++p)
            p->second->getinfo(child, info_level);
      }
    
    if(info_level >= ConnectionInfo)
      {
        list<ConnectionMonitor *>::const_iterator p;
        for(p = cws.begin(); p != cws.end(); ++p)
            (*p)->getinfo(child, info_level);
      }
  }

void StationMonitorImpl::save_state(const string &filename) const
  {
    xmlSetGenericErrorFunc(NULL, xml_error);
    
    char buf[20];
    xmlDocPtr doc;
    
    if((doc = xmlNewDoc((const xmlChar *) "1.0")) == NULL)
        throw bad_alloc();
        
    if((doc->children = xmlNewDocNode(doc, NULL, (const xmlChar *) "buffer", NULL)) == NULL)
        throw bad_alloc();

    snprintf(buf, 8, "%06X", begin_seq);
    xml_new_prop(doc->children, "begin_seq", buf);

    snprintf(buf, 8, "%06X", end_seq);
    xml_new_prop(doc->children, "end_seq", buf);

    snprintf(buf, 19, "%d", segment_count);
    xml_new_prop(doc->children, "segment_count", buf);

    xml_new_prop(doc->children, "stream_check", (stream_check ? "enabled": "disabled"));
    
    map<StreamDescriptor, rc_ptr<StreamMonitor> >::const_iterator p;
    for(p = stream_map.begin(); p != stream_map.end(); ++p)
        p->second->getstate(doc->children);
    
    xmlSaveFormatFile(filename.c_str(), doc, 1);
    xmlFreeDoc(doc);
    xmlSetGenericErrorFunc(NULL, NULL);
  }

void StationMonitorImpl::restore_state(const string &filename)
  {
    bool error = false;

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(SeqAttribute("begin_seq", begin_seq));
    atts->add_item(SeqAttribute("end_seq", end_seq));
    atts->add_item(IntAttribute("segment_count", segment_count));
    atts->add_item(BoolAttribute("stream_check", stream_check, "enabled", "disabled"));

    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(StreamElement(stream_map, error));
    read_config_xml(filename, "buffer", atts, elms);

    if(error) throw MonitorConfigError(filename);
  }

//*****************************************************************************
// MasterMonitorImpl
//*****************************************************************************

struct Capability
  {
    string name;
    bool restricted;
    Capability(const string &name_init, bool restricted_init):
      name(name_init), restricted(restricted_init) {}
  };

class MasterMonitorImpl: public MasterMonitor, private StationMonitorPartner
  {
  private:
    const int reclen;
    const string info_streamname;
    const string error_streamname;
    const string software;
    const string organization;
    const IPACL ip_trusted;
    INT_TIME start_time;
    list<Capability> caps;
    list<StationMonitor *> sws;

    list<StationMonitor *>::iterator attach(StationMonitor *sw);
    void detach(list<StationMonitor *>::iterator ptr);
    
  public:
    MasterMonitorImpl(int reclen_init, const string &info_streamname_init, 
      const string &error_streamname_init, const string &software_init,
      const string &organization_init, const IPACL &ip_trusted_init):
      reclen(reclen_init), info_streamname(info_streamname_init),
      error_streamname(error_streamname_init), software(software_init),
      organization(organization_init), ip_trusted(ip_trusted_init)
      {
        start_time = get_real_time();
      }

    ~MasterMonitorImpl();

    void add_capability(const string &cap, bool restricted);
    rc_ptr<StationMonitor> add_station(const string &name,
      const string &network, const string &description,
      const IPACL &ip_access);

    void cat_out(list<rc_ptr<MessageBuffer> > &buflist,
      unsigned int ipaddr) const;
    void error_out(list<rc_ptr<InfoBuffer> > &buflist) const;
    void info_out(list<rc_ptr<InfoBuffer> > &buflist, int info_level,
      unsigned int ipaddr) const;
      
    bool iptrusted(unsigned int ipaddr) const
      {
        return ip_trusted.check(ipaddr);
      }
  };

MasterMonitorImpl::~MasterMonitorImpl()
  {
    if(!sws.empty())
      {
        // It is better to use plain printf() here, because stream objects
        // can be already destroyed when this destructor is called. However,
        // this message should never appear unless there is a serious bug
        // in the program.
        printf("deleting MasterMonitor object, while some StationMonitor "
          "objects still exist\n");
      }
  }

list<StationMonitor *>::iterator MasterMonitorImpl::attach(StationMonitor *sw)
  {
    return sws.insert(sws.end(), sw);
  }

void MasterMonitorImpl::detach(list<StationMonitor *>::iterator ptr)
  {
    sws.erase(ptr);
  }

void MasterMonitorImpl::add_capability(const string &name, bool restricted)
  {
    caps.push_back(Capability(name, restricted));
  }

rc_ptr<StationMonitor> MasterMonitorImpl::add_station(const string &name,
  const string &network, const string &description, const IPACL &ip_access)
  {
    return new StationMonitorImpl(*this, name, network, description, ip_access);
  }

void MasterMonitorImpl::cat_out(list<rc_ptr<MessageBuffer> > &buflist,
  unsigned int ipaddr) const
  {
    ostream mout(NULL);
    redirect_ostream(mout, MessageOutput(buflist, (1 << reclen)));
    
    list<StationMonitor *>::const_iterator p;
    for(p = sws.begin(); p != sws.end(); ++p)
        (*p)->getmsg(mout, ipaddr);

    delete mout.rdbuf();
  }

void MasterMonitorImpl::error_out(list<rc_ptr<InfoBuffer> > &buflist) const
  {
    xmlSetGenericErrorFunc(NULL, xml_error);
    
    xmlDocPtr doc;
    if((doc = xmlNewDoc((const xmlChar *) "1.0")) == NULL)
        throw bad_alloc();
        
    if((doc->children = xmlNewDocNode(doc, NULL, (const xmlChar *) "seedlink", NULL)) == NULL)
        throw bad_alloc();

    xml_new_prop(doc->children, "software", software.c_str());
    xml_new_prop(doc->children, "organization", organization.c_str());
    xml_new_prop(doc->children, "started", time_to_str(start_time, MONTHS_FMT));

    InfoGenerator xg(reclen, error_streamname, buflist);

    xg.write_doc(doc);
    xmlFreeDoc(doc);
    xmlSetGenericErrorFunc(NULL, NULL);
  }

void MasterMonitorImpl::info_out(list<rc_ptr<InfoBuffer> > &buflist,
  int info_level, unsigned int ipaddr) const
  {
    internal_check(info_level >= 0 && info_level < N_InfoLevel);
    
    xmlSetGenericErrorFunc(NULL, xml_error);
    
    xmlDocPtr doc;
    if((doc = xmlNewDoc((const xmlChar *) "1.0")) == NULL)
        throw bad_alloc();
        
    if((doc->children = xmlNewDocNode(doc, NULL, (const xmlChar *) "seedlink", NULL)) == NULL)
        throw bad_alloc();

    xml_new_prop(doc->children, "software", software.c_str());
    xml_new_prop(doc->children, "organization", organization.c_str());
    xml_new_prop(doc->children, "started", time_to_str(start_time, MONTHS_FMT));

    if(info_level == CapabilityInfo || info_level == AllInfo)
      {
        list<Capability>::const_iterator p;
        for(p = caps.begin(); p != caps.end(); ++p)
          {
            if(p->restricted && !iptrusted(ipaddr)) continue;
            
            xmlNodePtr child;
            child = xml_new_child(doc->children, "capability");
            xml_new_prop(child, "name", p->name.c_str());
          }
      }
          
    if(info_level >= StationInfo)
      {
        list<StationMonitor *>::const_iterator p;
        for(p = sws.begin(); p != sws.end(); ++p)
            (*p)->getinfo(doc->children, info_level, ipaddr);
      }
        
    InfoGenerator xg(reclen, info_streamname, buflist);

    xg.write_doc(doc);
    xmlFreeDoc(doc);
    xmlSetGenericErrorFunc(NULL, NULL);
  }

//*****************************************************************************
// Entry Point
//*****************************************************************************

rc_ptr<MasterMonitor> make_master_monitor(int reclen,
  const string &info_streamname, const string &error_streamname,
  const string &software, const string &organization, const IPACL &ip_trusted)
  {
    return new MasterMonitorImpl(reclen, info_streamname, error_streamname,
      software, organization, ip_trusted);
  }

} // namespace SeedlinkMonitor_private

