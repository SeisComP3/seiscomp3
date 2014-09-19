/***************************************************************************** 
 * arclink.cc
 *
 * ArcLink server
 *
 * (c) 2004 Andres Heinloo, GFZ Potsdam
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
#include <cstdarg>
#include <csignal>
#include <cerrno>

#include <unistd.h>
#include <syslog.h>
#include <termios.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libxml/xmlIO.h>
#include <libxml/tree.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#ifdef TCPWRAP
extern "C" {
#include "tcpd.h"
}
#endif

#include "confbase.h"
#include "conf_ini.h"
#include "conf_xml.h"
#include "confattr.h"
#include "cppstreams.h"
#include "utils.h"
#include "diag.h"
// BIANCHI|ENCRYPTION: Added the encryption class to arclink
#include "encrypt.h"

#define MYVERSION "1.2 (2013.070)"

using namespace std;
using namespace CfgParser;
using namespace CPPStreams;
using namespace Utilities;
// BIANCHI|ENCRYPTION: Added the encryption class to arclink
using namespace SSLWrapper;

namespace {

typedef int64_t reqsize_t;

const int         BLOCKSIZE        = 512;
const int         MAXCMDLEN        = 1024;
const int         ERR_SIZE         = 100;
const int         REQUEST_FD       = 62;
const int         RESPONSE_FD      = 63;
const int         CLEANUP_PERIOD   = 60;
const int         QUEUE_STATUS_PER = 3600;
const char *const SHELL            = "/bin/bash";
const char *const ident_str        = "ArcLink v" MYVERSION;
string            config_file      = "/home/sysop/config/arclink.ini";

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

string daemon_name;
int verbosity = 0;
bool daemon_mode = false, daemon_init = false;
volatile sig_atomic_t terminate_proc = 0;

void int_handler(int sig)
  {
    terminate_proc = 1;
  }

//*****************************************************************************
// Close-on-Exec versions of some UNIX calls
//*****************************************************************************

// We don't want to give open file descriptors to request handlers

inline int xopen(const char *pathname, int flags)
  {
    int fd;
    if((fd = open(pathname, flags)) >= 0)
        fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
  }

inline int xcreat(const char *pathname, mode_t mode)
  {
    int fd;
    if((fd = creat(pathname, mode)) >= 0)
        fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
  }

inline int xsocket(int domain, int type, int protocol)
  {
    int fd;
    if((fd = socket(domain, type, protocol)) >= 0)
        fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
  }

#if defined(__GNU_LIBRARY__) && __GNU_LIBRARY__ < 2
inline int xaccept(int s, struct sockaddr *addr, int *addrlen)
#else
inline int xaccept(int s, struct sockaddr *addr, socklen_t *addrlen)
#endif
  {
    int fd;
    if((fd = accept(s, addr, addrlen)) >= 0)
        fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
  }

//*****************************************************************************
// XML Helpers
//*****************************************************************************

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
// Exceptions
//*****************************************************************************

class ArclinkError: public GenericException
  {
  public:
    ArclinkError(const string &message):
      GenericException("Arclink", message) {}
  };

class ArclinkLibraryError: public ArclinkError
  {
  public:
    ArclinkLibraryError(const string &message):
      ArclinkError(message + " (" + strerror(errno) + ")") {}
  };

class ArclinkCannotOpenFile: public ArclinkLibraryError
  {
  public:
    ArclinkCannotOpenFile(const string &name):
      ArclinkLibraryError("cannot open file '" + name + "'") {}
  };

class ArclinkCannotOpenDir: public ArclinkLibraryError
  {
  public:
    ArclinkCannotOpenDir(const string &name):
      ArclinkLibraryError("cannot open directory '" + name + "'") {}
  };

class ArclinkRequestError: public ArclinkError
  {
  public:
    ArclinkRequestError(const string &message):
      ArclinkError(message) {}
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

    rc_ptr<MessageBuffer> get_buffer();
    void queue_buffer(rc_ptr<MessageBuffer> buf, int size);
  };

rc_ptr<MessageBuffer> MessageStore::get_buffer()
  {
    return new MessageBuffer(bufsize);
  }

void MessageStore::queue_buffer(rc_ptr<MessageBuffer> buf, int size)
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
    rc_ptr<MessageBuffer> buf;
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
// XMLOutput
//*****************************************************************************

class XMLOutput
  {
  private:
    rc_ptr<MessageStore> msgs;
    rc_ptr<MessageBuffer> buf;
    int pos;

    void iowrite(const char *buf, int len);
    void ioclose();

    static int iowrite_proxy(void *ctx, const char *buf, int len)
      {
        static_cast<XMLOutput *>(ctx)->iowrite(buf, len);
        return len;
      }

    static int ioclose_proxy(void *ctx)
      {
        static_cast<XMLOutput *>(ctx)->ioclose();
        return 0;
      }

  public:
    XMLOutput(list<rc_ptr<MessageBuffer> > &buffer_list, int bufsize):
      msgs(new MessageStore(buffer_list, bufsize)), buf(NULL), pos(0) {}
    ~XMLOutput();
     void write_doc(xmlDocPtr doc);
  };

void XMLOutput::iowrite(const char *p, int len)
  {
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
  }

void XMLOutput::ioclose()
  {
    if(buf != NULL)
      {
        msgs->queue_buffer(buf, pos);
        buf = NULL;
      }
  }

XMLOutput::~XMLOutput()
  {
    ioclose();
  }
    
void XMLOutput::write_doc(xmlDocPtr doc)
  {
    xmlOutputBufferPtr obuf;
    if((obuf = xmlOutputBufferCreateIO(iowrite_proxy, ioclose_proxy, this, NULL)) == NULL)
        throw bad_alloc();

//    xmlSaveFileTo(obuf, doc, NULL);
//  pretty-print for easier debugging
    xmlSaveFormatFileTo(obuf, doc, NULL, 1);
  }

//*****************************************************************************
// StatusInterface
//*****************************************************************************

enum RequestStatus
  {
    STATUS_UNKNOWN = -1,
    STATUS_UNSET,
    STATUS_PROC,
    STATUS_CANCEL,
    STATUS_OK,
    STATUS_WARN,
    STATUS_ERROR,
    STATUS_DENIED,
    STATUS_RETRY,
    STATUS_NODATA
  };

const char *request_status2string(RequestStatus status)
  {
    switch(status)
      {
      case STATUS_UNKNOWN: return "UNKNOWN";
      case STATUS_UNSET:   return "UNSET";
      case STATUS_PROC:    return "PROCESSING";
      case STATUS_CANCEL:  return "CANCELLED";
      case STATUS_OK:      return "OK";
      case STATUS_WARN:    return "WARNING";
      case STATUS_ERROR:   return "ERROR";
      case STATUS_DENIED:  return "DENIED";
      case STATUS_RETRY:   return "RETRY";
      case STATUS_NODATA:  return "NODATA";
      }

    return "UNKNOWN";
  }

RequestStatus request_string2status(const string& status)
  {
    if(!strcasecmp(status.c_str(), "UNSET"))
       return STATUS_UNSET;

    if(!strcasecmp(status.c_str(), "PROCESSING"))
       return STATUS_PROC;

    if(!strcasecmp(status.c_str(), "CANCELLED"))
       return STATUS_CANCEL;

    if(!strcasecmp(status.c_str(), "OK"))
       return STATUS_OK;

    if(!strcasecmp(status.c_str(), "WARNING"))
       return STATUS_WARN;

    if(!strcasecmp(status.c_str(), "ERROR"))
       return STATUS_ERROR;

    if(!strcasecmp(status.c_str(), "DENIED"))
       return STATUS_DENIED;

    if(!strcasecmp(status.c_str(), "RETRY"))
       return STATUS_RETRY;

    if(!strcasecmp(status.c_str(), "NODATA"))
        return STATUS_NODATA;

    return STATUS_UNKNOWN;
  }

class StatusInterface
  {
  protected:
    RequestStatus _status;
    string _message;
    reqsize_t _size;

  public:
    StatusInterface(): _status(STATUS_UNSET), _size(0) {}

    void set_status(RequestStatus s)
      {
        _status = s;
      }
      
    enum RequestStatus status() const
      {
        return _status;
      }
    
    void set_message(const string &msg)
      {
        _message = msg;
      }

    string message() const
      {
        return _message;
      }
    
    void set_size(reqsize_t s)
      {
        _size = s;
      }
    
    reqsize_t size() const
      {
        return _size;
      }
    
    bool status_unset() const
      {
        return (_status == STATUS_UNSET);
      }
  };

//*****************************************************************************
// RequestLine
//*****************************************************************************

class RequestLine: public StatusInterface
  {
  private:
    const string ref;

  public:
    RequestLine(const string &ref_init): ref(ref_init) {}

    void cancel();
    void getxml(xmlNodePtr parent) const;
  };

void RequestLine::cancel()
  {
    if(_status == STATUS_PROC || _status == STATUS_OK || \
      _status == STATUS_WARN || _status == STATUS_UNSET)
      {
        set_status(STATUS_CANCEL);
        set_size(0);
      }
  }

void RequestLine::getxml(xmlNodePtr parent) const
  {
    xmlNodePtr child;
    child = xml_new_child(parent, "line");
    xml_new_prop(child, "content", ref.c_str());
    xml_new_prop(child, "status", request_status2string(_status));
    xml_new_prop(child, "size", to_string(_size).c_str());
    xml_new_prop(child, "message", _message.c_str());
  }
  
//*****************************************************************************
// RequestVolume
//*****************************************************************************

class RequestVolume: public StatusInterface
  {
  private:
    bool _restricted;
    bool _encryption;
    string _dcid;
    list<rc_ptr<RequestLine> > req_lines;

  public:
    const string id;

    RequestVolume(const string &id_init, const string &dcid_init, const bool &encryption_init,
        bool restricted_init):
       _restricted(restricted_init), _encryption(encryption_init), _dcid(dcid_init), id(id_init) {}
    
    void add_line(rc_ptr<RequestLine> ln)
      {
        req_lines.push_back(ln);
      }

    bool downloadable()
      {
        return (_status == STATUS_OK || _status == STATUS_WARN);
      }

    bool doEncrypt() const
      {
        return (_restricted && _encryption);
      }

    void cancel();
    void getxml(xmlNodePtr parent, bool raw) const;
  };

void RequestVolume::cancel()
  {
    if(_status == STATUS_PROC || _status == STATUS_OK || \
      _status == STATUS_WARN || _status == STATUS_UNSET)
      {
        set_status(STATUS_CANCEL);
        set_size(0);
            
        list<rc_ptr<RequestLine> >::iterator p;
        for(p = req_lines.begin(); p != req_lines.end(); ++p)
            (*p)->cancel();
      }
  }

void RequestVolume::getxml(xmlNodePtr parent, bool raw) const
  {
    xmlNodePtr child;
    child = xml_new_child(parent, "volume");
    xml_new_prop(child, "id", id.c_str());
    xml_new_prop(child, "dcid", _dcid.c_str());
    xml_new_prop(child, "status", request_status2string(_status));

    if (raw){
        xml_new_prop(child, "size", to_string(_size).c_str());
        xml_new_prop(child, "restricted", (_restricted? "true": "false"));
    } else {
        reqsize_t volsize = (doEncrypt() && (status() == STATUS_OK || status() == STATUS_WARN))?(SSLWrapper::Encrypt::expectedSize (_size)):_size;
        xml_new_prop(child, "size", to_string(volsize).c_str());
        xml_new_prop(child, "encrypted", (doEncrypt())?"true":"false");
    }
    
    xml_new_prop(child, "message", _message.c_str());

    list<rc_ptr<RequestLine> >::const_iterator p;
    for(p = req_lines.begin(); p != req_lines.end(); ++p)
        (*p)->getxml(child);
  }

//*****************************************************************************
// Request
//*****************************************************************************

const string RID_ALL = "";

enum RequestType
  {
    REQ_UNKNOWN = -1,
    REQ_WAVEFORM,
    REQ_INVENTORY,
    REQ_ROUTING,
    REQ_RESPONSE,
    REQ_QC,
    REQ_GREENSFUNC,
    NREQTYPES
  };

const char *request_type2string(RequestType type)
  {
    switch(type)
      {
      case REQ_UNKNOWN:   return "UNKNOWN";
      case REQ_WAVEFORM:  return "WAVEFORM";
      case REQ_INVENTORY: return "INVENTORY";
      case REQ_ROUTING:   return "ROUTING";
      case REQ_RESPONSE:  return "RESPONSE";
      case REQ_QC:        return "QC";
      case REQ_GREENSFUNC:return "GREENSFUNC";
      case NREQTYPES:     return "UNKNOWN";
      }

    return "UNKNOWN";
  }

RequestType request_string2type(const string& type)
  {
    if(!strcasecmp(type.c_str(), "WAVEFORM"))
        return REQ_WAVEFORM;
    
    if(!strcasecmp(type.c_str(), "INVENTORY"))
        return REQ_INVENTORY;

    if(!strcasecmp(type.c_str(), "ROUTING"))
        return REQ_ROUTING;

    if(!strcasecmp(type.c_str(), "RESPONSE"))
        return REQ_RESPONSE;

    if(!strcasecmp(type.c_str(), "QC"))
        return REQ_QC;

    if(!strcasecmp(type.c_str(), "GREENSFUNC"))
        return REQ_GREENSFUNC;

    return REQ_UNKNOWN;
  }

class Request
  {
  private:
    bool _restricted;
    const bool _encryption;
    const string _dcid;

    const int max_lines;
    list<string> content;
    vector<rc_ptr<RequestLine> > req_lines;
    map<string, rc_ptr<RequestVolume> > req_volumes;
    rc_ptr<RequestVolume> restored_volume;
    string req_message;
    bool req_ready;
    bool req_error;
    bool req_zombie;

  public:
    const string id;
    const RequestType type;
    const string user;
    const string institution;
    const string label;
    const string client_ip;
    const string user_ip;
    const string req_args;
    const string location;

    Request(const string &id_init, RequestType type_init,
      const string &user_init, const string &institution_init,
      const string &label_init, const string &client_ip_init,
      const string &user_ip_init, const string &req_args_init,
      const string &location_init, int max_lines_init, const string &dcid_init, const bool &encryption_init,
      bool ready_init = false, bool restricted_init = false, bool error_init = false);

    ~Request();

    reqsize_t size() const;
    void response_accepted(const vector<string> &cmdvec);
    void response_rejected(const vector<string> &cmdvec, const string &msg);
    void response_STATUS(const vector<string> &cmdvec);
    void response(const vector<string> &s);
    void push_line(const string &str);
    void push_end();
    int open_first(map<string, rc_ptr<RequestVolume> >::iterator &vol_iter,
      size_t pos);
    int open_next(map<string, rc_ptr<RequestVolume> >::iterator &vol_iter);
    int open_vol(const string &volume, size_t pos, reqsize_t &volsize);
    void getmsg(ostream &mout) const;
    void getxml(xmlNodePtr parent, bool show_user, bool raw) const;
    void restore_volume(const string &id, RequestStatus status, reqsize_t size,
      const string &message);
    void restore_line(const string &content, RequestStatus status, reqsize_t size,
      const string &message);
    void restore_end();

    list<string>::const_iterator begin() const
      {
        return content.begin();
      }

    list<string>::const_iterator end() const
      {
        return content.end();
      }

    string dcid() const {
        return _dcid;
    }

    bool doEncrypt() const
      {
        return (_restricted && _encryption);
      }

    bool ready() const
      {
        return req_ready;
      }
    
    bool error() const
      {
        return req_error;
      }
    
    bool zombie() const
      {
        return req_zombie;
      }
    
    void kill()
      {
        req_zombie = true;
      }

    void reset()
      {
        vector<rc_ptr<RequestLine> >::iterator p1;
        for(p1 = req_lines.begin(); p1 != req_lines.end(); ++p1)
          {
            (*p1)->set_status(STATUS_UNSET);
            (*p1)->set_size(0);
            (*p1)->set_message("");
          }

        map<string, rc_ptr<RequestVolume> >::iterator p2;
        for(p2 = req_volumes.begin(); p2 != req_volumes.end(); ++p2)
            unlink((location + "." + p2->first).c_str());

        req_volumes.clear();
      }
  };

Request::Request(const string &id_init, RequestType type_init,
  const string &user_init, const string &institution_init,
  const string &label_init, const string &client_ip_init,
  const string &user_ip_init, const string &req_args_init,
  const string &location_init, int max_lines_init,
  const string &dcid_init, const bool &encryption_init, bool ready_init,
  bool restricted_init, bool error_init):
  _restricted(restricted_init), _encryption(encryption_init), _dcid(dcid_init), max_lines(max_lines_init), req_ready(ready_init), req_error(error_init),
  req_zombie(false), id(id_init), type(type_init), user(user_init),
  institution(institution_init), label(label_init), client_ip(client_ip_init),
  user_ip(user_ip_init), req_args(req_args_init), location(location_init)
  {
    content.push_back("USER " + user);

    if(institution.length() > 0)
        content.push_back("INSTITUTION " + institution);

    if(label.length() > 0)
        content.push_back("LABEL " + label);
    
    if(client_ip.length() > 0)
        content.push_back("CLIENT_IP " + client_ip);

    if(user_ip.length() > 0)
        content.push_back("USER_IP " + user_ip);

    content.push_back("REQUEST " + string(request_type2string(type)) + " " + 
      id + " " + req_args);
  }

Request::~Request()
  {
    if(req_zombie)
      {
        map<string, rc_ptr<RequestVolume> >::iterator p;
        for(p = req_volumes.begin(); p != req_volumes.end(); ++p)
            unlink((location + "." + p->first).c_str());
      }
  }

reqsize_t Request::size() const
  {
    reqsize_t n = 0;
    map<string, rc_ptr<RequestVolume> >::const_iterator p;
    for(p = req_volumes.begin(); p != req_volumes.end(); ++p)
        n += p->second->size();

    return n;
  }

void Request::response_STATUS(const vector<string> &cmdvec)
  {
    if(cmdvec.size() < 4)
        throw ArclinkRequestError("STATUS requires at least 3 arguments");

    rc_ptr<StatusInterface> sti;
    if(!strcasecmp(cmdvec[1].c_str(), "LINE"))
      {
        char c;
        unsigned int ln;

        if(sscanf(cmdvec[2].c_str(), "%u%c", &ln, &c) != 1 ||
          ln >= req_lines.size())
            throw ArclinkRequestError("invalid line number");

        if(!strcasecmp(cmdvec[3].c_str(), "PROCESSING"))
          {
            if(cmdvec.size() != 5)
                throw ArclinkRequestError("incorrect number of arguments");

            rc_ptr<RequestVolume> vol;
            map<string, rc_ptr<RequestVolume> >::iterator p;
            if((p = req_volumes.find(cmdvec[4])) != req_volumes.end())
              {
                vol = p->second;
              }
            else
              {
                vol = new RequestVolume(cmdvec[4], _dcid, _encryption, _restricted);
                vol->set_status(STATUS_PROC);
                req_volumes.insert(make_pair(cmdvec[4], vol));
              }
            
            req_lines[ln]->set_status(STATUS_PROC);
            vol->add_line(req_lines[ln]);
            return;
          }

        if(!strcasecmp(cmdvec[3].c_str(), "CANCEL"))
          {
            if(cmdvec.size() != 4)
                throw ArclinkRequestError("incorrect number of arguments");
            
            req_lines[ln]->cancel();
            return;
          }

        sti = rc_ptr_cast<StatusInterface>(req_lines[ln]);
      }
    else if(!strcasecmp(cmdvec[1].c_str(), "VOLUME"))
      {
        map<string, rc_ptr<RequestVolume> >::iterator p;
        if((p = req_volumes.find(cmdvec[2])) == req_volumes.end())
            throw ArclinkRequestError("volume not found");

        if(!strcasecmp(cmdvec[3].c_str(), "CANCEL"))
          {
            if(cmdvec.size() != 4)
                throw ArclinkRequestError("incorrect number of arguments");
            
            p->second->cancel();
            return;
          }

        sti = rc_ptr_cast<StatusInterface>(p->second);
      }

    if(!strcasecmp(cmdvec[3].c_str(), "OK"))
      {
          sti->set_status(STATUS_OK);
      }
    else if(!strcasecmp(cmdvec[3].c_str(), "WARN"))
      {
        sti->set_status(STATUS_WARN);
      }
    else if(!strcasecmp(cmdvec[3].c_str(), "ERROR"))
      {
        sti->set_status(STATUS_ERROR);
      }
    else if(!strcasecmp(cmdvec[3].c_str(), "DENIED"))
      {
        sti->set_status(STATUS_DENIED);
      }
    else if(!strcasecmp(cmdvec[3].c_str(), "RETRY"))
      {
        sti->set_status(STATUS_RETRY);
      }
    else if(!strcasecmp(cmdvec[3].c_str(), "NODATA"))
      {
        sti->set_status(STATUS_NODATA);
      }
    else if(!strcasecmp(cmdvec[3].c_str(), "MESSAGE"))
      {
        if(cmdvec.size() < 5)
            throw ArclinkRequestError("incorrect number of arguments");

        vector<string>::const_iterator p = cmdvec.begin() + 4;
        string msg = *p;

        while((++p) != cmdvec.end())
            msg += (" " + *p);

        sti->set_message(msg);
      }
    else if(!strcasecmp(cmdvec[3].c_str(), "SIZE"))
      {
        if(cmdvec.size() != 5)
            throw ArclinkRequestError("incorrect number of arguments");

        char c;
        unsigned int s;
        if(sscanf(cmdvec[4].c_str(), "%u%c", &s, &c) != 1)
            throw ArclinkRequestError("invalid size");

        sti->set_size(s);
      }
    else
      {
        throw ArclinkRequestError("invalid status message");
      }
  }
 
void Request::response(const vector<string> &cmdvec)
  {
    if(!strcasecmp(cmdvec[0].c_str(), "STATUS"))
      {
        response_STATUS(cmdvec);
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "CANCEL"))
      {
        if(cmdvec.size() != 1)
            throw ArclinkRequestError("syntax error");

        map<string, rc_ptr<RequestVolume> >::iterator p1;
        for(p1 = req_volumes.begin(); p1 != req_volumes.end(); ++p1)
            p1->second->cancel();
            
        // cancel also remaining lines that are not in any volume
        vector<rc_ptr<RequestLine> >::iterator p2;
        for(p2 = req_lines.begin(); p2 != req_lines.end(); ++p2)
            (*p2)->cancel();

        req_ready = true;
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "MESSAGE"))
      {
        if(cmdvec.size() < 2)
            throw ArclinkRequestError("syntax error");

        req_message = cmdvec[1];
        vector<string>::const_iterator p;
        for(p = cmdvec.begin() + 2; p != cmdvec.end(); ++p)
            req_message += (" " + *p);
      
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "ERROR"))
      {
        if(cmdvec.size() != 1)
            throw ArclinkRequestError("syntax error");

        map<string, rc_ptr<RequestVolume> >::iterator p1;
        for(p1 = req_volumes.begin(); p1 != req_volumes.end(); ++p1)
            if(p1->second->status() == STATUS_PROC)
                p1->second->cancel();
            
        // cancel also remaining lines that are not in any volume
        vector<rc_ptr<RequestLine> >::iterator p2;
        for(p2 = req_lines.begin(); p2 != req_lines.end(); ++p2)
            (*p2)->cancel();

        req_ready = true;
        req_error = true;
        return;
      }

    // BIANCHI|ENCRYPTED: Parse the restricted response from the 
    // request handler and set the request as containing restricted data.
    if(!strcasecmp(cmdvec[0].c_str(), "RESTRICTED"))
      {
        if(cmdvec.size() != 1)
            throw ArclinkRequestError("syntax error");
        _restricted = true;
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "END"))
      {
        if(cmdvec.size() != 1)
            throw ArclinkRequestError("syntax error");

        req_ready = true;
        return;
      }

    throw ArclinkRequestError("syntax error");
  }

void Request::push_line(const string &str)
  {
    if((int)content.size() >= max_lines)
        return;
    
    content.push_back(str);
    req_lines.push_back(new RequestLine(content.back()));
  }

void Request::push_end()
  {
    if((int)content.size() >= max_lines)
        throw ArclinkRequestError("maximum request size exceeded (user=" + user + ",req=" + id + ",lines>" + to_string(max_lines) + ")");

    content.push_back("END");
  }

int Request::open_first(map<string, rc_ptr<RequestVolume> >::iterator &vol_iter,
  size_t pos)
  {
    if(!req_ready)
        return -1;
    
    vol_iter = req_volumes.begin();

    while(vol_iter != req_volumes.end() && !vol_iter->second->downloadable())
        ++vol_iter;
    
    if(vol_iter == req_volumes.end())
        throw ArclinkRequestError("no downloadable volumes found");
    
    int fd;
    if((fd = xopen((location + "." + vol_iter->first).c_str(), O_RDONLY)) < 0)
        logs(LOG_ERR) << strerror(errno) << endl;

    // seek...

    ++vol_iter;
    return fd;
  }

int Request::open_next(map<string, rc_ptr<RequestVolume> >::iterator &vol_iter)
  {
    internal_check(req_ready);
    
    while(vol_iter != req_volumes.end() && !vol_iter->second->downloadable())
        ++vol_iter;
    
    if(vol_iter == req_volumes.end())
        return -1;
    
    int fd;
    if((fd = xopen((location + "." + vol_iter->first).c_str(), O_RDONLY)) < 0)
        logs(LOG_ERR) << strerror(errno) << endl;

    ++vol_iter;
    return fd;
  }

int Request::open_vol(const string &volume, size_t pos, reqsize_t &volsize)
  {
    map<string, rc_ptr<RequestVolume> >::iterator p;
    if((p = req_volumes.find(volume)) == req_volumes.end())
        throw ArclinkRequestError("volume not found");

    if(!p->second->downloadable())
        throw ArclinkRequestError("volume is not downloadable");
    
    int fd;
    if((fd = xopen((location + "." + p->first).c_str(), O_RDONLY)) < 0)
        logs(LOG_ERR) << strerror(errno) << endl;

    // seek...

    volsize = p->second->size();
    return fd;
  }

void Request::getmsg(ostream &mout) const
  {
    list<string>::const_iterator p;
    for(p = content.begin(); p != content.end(); ++p)
      {
        mout << *p << flush;
      }
  }

void Request::getxml(xmlNodePtr parent, bool show_user, bool raw) const
  {
    xmlNodePtr child;
    child = xml_new_child(parent, "request");
    xml_new_prop(child, "id", id.c_str());
    xml_new_prop(child, "type", request_type2string(type));

    if(show_user)
      {
        xml_new_prop(child, "user", user.c_str());
        xml_new_prop(child, "institution", institution.c_str());
        xml_new_prop(child, "client_ip", client_ip.c_str());
        xml_new_prop(child, "user_ip", user_ip.c_str());
      }
        
    xml_new_prop(child, "label", label.c_str());
    xml_new_prop(child, "args", req_args.c_str());

    if (raw) {
        xml_new_prop(child, "restricted", (_restricted? "true": "false"));
        xml_new_prop(child, "size", to_string(size()).c_str());
    } else {
        xml_new_prop(child, "encrypted", (doEncrypt())?"true":"false");
        reqsize_t reqsize = (doEncrypt())?(SSLWrapper::Encrypt::expectedSize (size())):size();
        xml_new_prop(child, "size", to_string(reqsize).c_str());
    }

    xml_new_prop(child, "ready", (req_ready? "true": "false"));
    xml_new_prop(child, "error", (req_error? "true": "false"));
    xml_new_prop(child, "message", req_message.c_str());

    map<string, rc_ptr<RequestVolume> >::const_iterator p;
    for(p = req_volumes.begin(); p != req_volumes.end(); ++p)
        p->second->getxml(child, raw);

    bool found_unassigned = false;
    vector<rc_ptr<RequestLine> >::const_iterator p1;
    for(p1 = req_lines.begin(); p1 != req_lines.end(); ++p1)
      {
        if((*p1)->status_unset())
          {
            if(!found_unassigned)
              {
                child = xml_new_child(child, "volume");
                xml_new_prop(child, "id", "UNSET");
                xml_new_prop(child, "status", "UNSET");
                xml_new_prop(child, "size", "0");
                xml_new_prop(child, "message", "");
                found_unassigned = true;
              }

            (*p1)->getxml(child);
          }
      }
  }

void Request::restore_volume(const string &id, RequestStatus status, reqsize_t size,
  const string &message)
  {
    restored_volume = new RequestVolume(id, _dcid, _encryption, _restricted);
    restored_volume->set_status(status);
    restored_volume->set_size(size);
    restored_volume->set_message(message);

    if(status != STATUS_UNSET && status != STATUS_PROC)
        req_volumes[id] = restored_volume;
  }

void Request::restore_line(const string &ref, RequestStatus status,
  reqsize_t size, const string &message)
  {
    rc_ptr<RequestLine> line = new RequestLine(ref);
    line->set_status(status);
    line->set_size(size);
    line->set_message(message);
    internal_check(restored_volume != NULL);
    restored_volume->add_line(line);
    
    if(restored_volume->status() == STATUS_UNSET ||
      restored_volume->status() == STATUS_PROC)
      {
        content.push_back(ref);
        req_lines.push_back(line);
      }
  }

void Request::restore_end()
  {
    content.push_back("END");
  }

//*****************************************************************************
// Reqhandler
//*****************************************************************************

class Reqhandler: private CFIFO_Partner
  {
  private:
    CFIFO cfifo;
    const string cmdline;
    bool used;
    pid_t pid;
    int request_fd;
    int response_fd;
    bool output_active;
    bool sigterm_sent;
    bool sigkill_sent;
    bool shutdown_requested;
    Timer start_retry_timer;
    Timer shutdown_timer;
    int send_timeout;
    rc_ptr<Request> req;
    list<string>::const_iterator req_iter;

    void check_proc();
    void kill_proc();
    void term_proc();

    void response_accepted(const vector<string> &cmdvec);
    void response_rejected(const vector<string> &cmdvec,
      const string &msg);
    void cfifo_callback(const string &cmd);
    void check_response();
    
  public:
    Reqhandler(const string &cmdline_init, int send_timeout_init,
      int start_retry, int shutdown_wait):
      cfifo(*this, MAXCMDLEN), cmdline(cmdline_init), used(false),
      pid(-1), request_fd(-1), response_fd(-1), output_active(false),
      sigterm_sent(false), sigkill_sent(false), shutdown_requested(false),
      start_retry_timer(start_retry, 0), shutdown_timer(shutdown_wait, 0),
      send_timeout(send_timeout_init), req(NULL) {}

    ~Reqhandler();
        
    void start();
    void push_request();
    bool check();
    void shutdown();
    
    void attach_request(rc_ptr<Request> r)
      {
        req = r;
        req_iter = req->begin();
        used = true;
      }
   
    rc_ptr<Request> detach_request()
      {
        rc_ptr<Request> ret = req;
        req = NULL;
        return ret;
      }
    
    bool ready()
      {
        return (req != NULL && req->ready());
      }

    bool cancelled()
      {
        return (req != NULL && req->zombie());
      }
    
    bool pending()
      {
        return (output_active && req != NULL && req_iter != req->end());
      }
    
    pair<int, int> filedes()
      {
        return make_pair(response_fd, request_fd);
      }
  };

Reqhandler::~Reqhandler()
  {
    if(request_fd >= 0)
        close(request_fd);

    if(response_fd >= 0)
        close(response_fd);
  }

void Reqhandler::check_proc()
  {
    internal_check(pid != 0);
    
    int status, completed;

    if(pid < 0) return;

    if((completed = waitpid(pid, &status, WNOHANG)) < 0)
      {
        logs(LOG_ERR) << "waitpid: " << strerror(errno) << endl;
        pid = -1;
        if(request_fd >= 0)
          {
            close(request_fd);
            request_fd = -1;
            output_active = false;
          }

        return;
      }

    if(!completed || WIFSTOPPED(status)) return;

    if(request_fd >= 0)
      {
        close(request_fd);
        request_fd = -1;
        output_active = false;
      }

    if(WIFSIGNALED(status))
      {
        logs(LOG_WARNING) << "[" << pid << "] terminated on signal " <<
          WTERMSIG(status) << endl;
      }
    else if(WIFEXITED(status) && WEXITSTATUS(status) != 0)
      {
        logs(LOG_WARNING) << "["<< pid << "] terminated with error status " <<
          WEXITSTATUS(status) << endl;
      }
    else
      {
        logs(LOG_NOTICE) << "["<< pid << "] terminated" << endl;
      }

    pid = -1;
    start_retry_timer.reset();
  }
    
void Reqhandler::kill_proc()
  {
    internal_check(pid > 0);
    
    kill(pid, SIGKILL);
    sigkill_sent = true;
  }

void Reqhandler::term_proc()
  {
    internal_check(pid > 0);
    
    kill(pid, SIGTERM);
    sigterm_sent = true;
    output_active = false;
    shutdown_timer.reset();
  }

void Reqhandler::response_accepted(const vector<string> &cmdvec)
  {
    vector<string>::const_iterator p = cmdvec.begin();
    string full_response;

    for(const char* q = p->c_str(); *q; ++q)
        full_response += toupper(*q);

    while((++p) != cmdvec.end())
        full_response += (string(" ") + *p);

    logs(LOG_INFO) << "["<< pid << "] " << full_response << endl;
  }

void Reqhandler::response_rejected(const vector<string> &cmdvec,
  const string &msg)
  {
    vector<string>::const_iterator p = cmdvec.begin();
    string full_response = *p;

    while((++p) != cmdvec.end())
        full_response += (string(" ") + *p);

    logs(LOG_WARNING) << "["<< pid << "] invalid response: " << full_response << endl;
    logs(LOG_WARNING) << "["<< pid << "] " << msg << endl;
  }

void Reqhandler::cfifo_callback(const string &cmd)
  {
    if(req == NULL)
      {
        if(!shutdown_requested)
            logs(LOG_WARNING) << "["<< pid << "] response '" << cmd << "' ignored, "
              "because the request handler is not associated with any request" << endl;

        return;
      }
    
    vector<string> cmdvec;
    const char *p = cmd.c_str();
    int arglen = 0;

    while(p += arglen, p += strspn(p, " "), arglen = strcspn(p, " "))
        cmdvec.push_back(string(p, arglen));

    if(cmdvec.size() == 0)
        return;

    try
      {
        req->response(cmdvec);
        response_accepted(cmdvec);
      }
    catch(ArclinkRequestError &e)
      {
        response_rejected(cmdvec, e.what());
        term_proc();                          // terminate or not ???
      }
  }

void Reqhandler::check_response()
  {
    int r = 0;
    
    try
      {
        if((r = cfifo.check(response_fd)) == 0 && !sigterm_sent && !sigkill_sent)
            logs(LOG_WARNING) << "["<< pid << "] unexpected eof cmd" << endl;
      }
    catch(CFIFO_ReadError &e)
      {
        logs(LOG_WARNING) << "["<< pid << "] read error (" <<
          e.what() << ")" << endl;
      }
    catch(CFIFO_Overflow &e)
      {
        logs(LOG_WARNING) << "["<< pid << "] response too long" << endl;
      }

    if(r == 0)
      {
        close(response_fd);
        response_fd = -1;
      }
  }

void Reqhandler::start()
  {
    internal_check(request_fd < 0);
    internal_check(response_fd < 0);
    
    int request_pipe[2];
    int response_pipe[2];

    N(pipe(request_pipe));
    N(pipe(response_pipe));
    
    N(pid = fork());

    if(pid) 
      {
        close(request_pipe[0]);
        close(response_pipe[1]);
        
        request_fd = request_pipe[1];
        response_fd = response_pipe[0];
        
        N(fcntl(request_fd, F_SETFD, FD_CLOEXEC));
        N(fcntl(response_fd, F_SETFD, FD_CLOEXEC));

        N(fcntl(request_fd, F_SETFL, O_NONBLOCK));
        N(fcntl(response_fd, F_SETFL, O_NONBLOCK));
        
        used = false;
        sigterm_sent = false;
        sigkill_sent = false;
        shutdown_requested = false;
        output_active = true;

        if(req != NULL)
          {
            req->reset();
            req_iter = req->begin();
          }

        return;
      }

    close(request_pipe[1]);
    close(response_pipe[0]);
    
    if(request_pipe[0] != REQUEST_FD)
      {
        N(dup2(request_pipe[0], REQUEST_FD));
        close(request_pipe[0]);
      }

    if(response_pipe[1] != RESPONSE_FD)
      {
        N(dup2(response_pipe[1], RESPONSE_FD));
        close(response_pipe[1]);
      }

    logs(LOG_INFO) << "["<< getpid() << "] starting shell" << endl;
    
    execl(SHELL, SHELL, "-c", cmdline.c_str(), NULL);

    logs(LOG_ERR) << string() + "cannot execute shell '" + SHELL + "' "
      "(" + strerror(errno) + ")" << endl;
    exit(0);
  }

void Reqhandler::push_request()
  {
    if(!output_active || req == NULL || req_iter == req->end())
        return;
    
    internal_check(request_fd >= 0);
    
    string req_str = *req_iter;
    
    int r;
    if((r = writen_tmo(request_fd, (req_str + "\r\n").c_str(),
      req_str.length() + 2, send_timeout)) < 0)
      {
        logs(LOG_WARNING) << "[" << pid << "] write error (" <<
          strerror(errno) << ")" << endl;

        term_proc();
      }
    else if(r == 0)
      {
        logs(LOG_WARNING) << "[" << pid << "] write timeout" << endl;
        term_proc();
      }
    else
      {
        ++req_iter;
      }
  }

bool Reqhandler::check()
  {
    internal_check(pid != 0);

    if(response_fd >= 0)
        check_response();
    
    check_proc();

    if(pid < 0)
      {
        if(shutdown_requested)
          {
            if(sigkill_sent && response_fd >= 0)
              {
                close(response_fd);
                response_fd = -1;
              }

            if(response_fd < 0)
                return true;
          }

        if(!shutdown_requested && (used || start_retry_timer.expired()))
          {
            if(response_fd >= 0)
              {
                close(response_fd);
                response_fd = -1;
              }

            start();
          }

        return false;
      }

    if(!sigterm_sent)
      {
        if(response_fd < 0)
          {
            term_proc();
          }

        return false;
      }
      
    if(!sigkill_sent && shutdown_timer.expired())
      {
        logs(LOG_WARNING) << "["<< pid << "] shutdown time expired" << endl;
        kill_proc();
      }

    return false;
  }

void Reqhandler::shutdown()
  {
    if(!sigterm_sent && pid > 0)
        term_proc();
    
    if(req != NULL && !req->ready())
        req->kill();

    shutdown_requested = true;
  }

//*****************************************************************************
// Connection
//*****************************************************************************

class ConnectionPartner
  {
  public:
    virtual rc_ptr<Request> new_request(RequestType reqtype,
      const string &user, const string &pass, const string &institution,
      const string &label, const string &client_ip, const string &user_ip,
      const string &req_args) = 0;
    virtual void queue_request(rc_ptr<Request> req) = 0;
    virtual bool purge_request(string rid, const string &user) = 0;
    virtual rc_ptr<Request> find_request(string rid,
      const string &user) = 0;
    virtual bool get_status(list<rc_ptr<MessageBuffer> > &buflist,
      string rid, const string &user) = 0;
    virtual ~ConnectionPartner() {};
  };

class Connection: private CFIFO_Partner
  {
  private:
    string serverEmail;
    string organization;
    ConnectionPartner &partner;
    CFIFO cfifo;
    Stream logs;
    const string ident;
    const int clientfd;
    const unsigned int ipaddr;
    const string host;
    const int port;
    const string adminpass;
    string username;
    string password;
    string institution;
    string label;
    string user_ip;
    string response_str;
    string errmsg;
    bool errflg;
    bool have_response;
    bool disconnect_requested;
    bool authentication;
    rc_ptr<Request> req;
    list<rc_ptr<MessageBuffer> > msg_bufs;
    bool downloading;
    bool download_vol;
    int download_fd;
    size_t download_pos;
    map<string, rc_ptr<RequestVolume> >::iterator vol_iter;
    // BIANCHI|ENCRYPTION: Add a encryption class to the class. This object has the 
    // ability to encrypt the supplied data. The class should be initialized before use and
    // should be reseted before re-use.
    Encrypt _encryptor;
    string _sslPasswordFile;

    void request_accepted(const vector<string> &cmdvec);
    void request_rejected(const vector<string> &cmdvec,
      const string &msg);
    void response(const string &str);
    void request_ok(const vector<string> &cmdvec);
    void request_error(const vector<string> &cmdvec,
      const string &errmsg);
            
    void command_STATUS(const vector<string> &cmdvec);
    void command_PURGE(const vector<string> &cmdvec);
    void command_REQUEST(const vector<string> &cmdvec);
    void command_DOWNLOAD(const vector<string> &cmdvec, bool blocking);
    void request(const vector<string> &cmdvec);
    void cfifo_callback(const string &cmd);
    enum DeliveryResult { Ok, Retry, Fail };
    DeliveryResult do_deliver();

    bool auth()
      {
        if(username == "admin" && password != adminpass)
          {
            authentication = false;
            return false;
          }

        authentication = true;
        return true;
      }

  public:
    Connection(ConnectionPartner &partner_init, const string &ident_init,
      unsigned int ipaddr_init, const string &host_init, int port_init,
      const string &adminpass_init, int fd, const Stream &logs_init,
      string password_file, string organization_init, string serverEmail_init):
        serverEmail(serverEmail_init),organization(organization_init),
        partner(partner_init), cfifo(*this, MAXCMDLEN), logs(logs_init),
        ident(ident_init), clientfd(fd), ipaddr(ipaddr_init), host(host_init),
        port(port_init), adminpass(adminpass_init), errmsg("success\r\n"),
        errflg(false), have_response(false), disconnect_requested(false),
        authentication(false), downloading(false), download_fd(-1),
        download_pos(0), _sslPasswordFile(password_file) {}
    
    ~Connection() { }

    bool deliver();
    bool input();
    void disconnect();

    bool pending()
      {
        return (have_response || !msg_bufs.empty() || (downloading && req->ready()));
      }

    int filedes()
      {
        return clientfd;
      }

    unsigned int ip()
      {
        return ipaddr;
      }
  };

void Connection::request_accepted(const vector<string> &cmdvec)
  {
    vector<string>::const_iterator p = cmdvec.begin();
    string full_request;

    for(const char* q = p->c_str(); *q; ++q)
        full_request += toupper(*q);

    while((++p) != cmdvec.end())
        full_request += (string(" ") + *p);

    logs(LOG_INFO) << full_request << endl;
    errmsg = "success\r\n";
  }

void Connection::request_rejected(const vector<string> &cmdvec,
  const string &msg)
  {
    vector<string>::const_iterator p = cmdvec.begin();
    string full_request = *p;

    while((++p) != cmdvec.end())
        full_request += (string(" ") + *p);

    logs(LOG_WARNING) << "invalid request: " << full_request << endl;
    logs(LOG_WARNING) << msg << endl;
    errmsg = msg + "\r\n";
  }

void Connection::response(const string &str)
  {
    response_str = str;
    have_response = true;
  }

void Connection::request_ok(const vector<string> &cmdvec)
  {
    request_accepted(cmdvec);
    response("OK\r\n");
  }

void Connection::request_error(const vector<string> &cmdvec,
  const string &msg)
  {
    request_rejected(cmdvec, msg);
    response("ERROR\r\n");
  }

void Connection::cfifo_callback(const string &cmd)
  {
    vector<string> cmdvec;
    const char *p = cmd.c_str();
    int arglen = 0;

    while(p += arglen, p += strspn(p, " "), arglen = strcspn(p, " "))
        cmdvec.push_back(string(p, arglen));

    if(cmdvec.size() > 0)
        request(cmdvec);
  }

void Connection::command_STATUS(const vector<string> &cmdvec)
  {
    if(cmdvec.size() != 2)
      {
        request_error(cmdvec, "STATUS requires 1 argument");
        return;
      }

    if(!strcasecmp(cmdvec[1].c_str(), "ALL"))
      {
        if(partner.get_status(msg_bufs, RID_ALL, username))
          {
            request_accepted(cmdvec);
            return;
          }

        request_error(cmdvec, "unknown error");
        return;
      }

    char c;
    unsigned int rid;
    if(sscanf(cmdvec[1].c_str(), "%u%c", &rid, &c) != 1)
      {
        request_error(cmdvec, "invalid request ID");
        return;
      }

    if(partner.get_status(msg_bufs, to_string(rid), username))
      {
        request_accepted(cmdvec);
        return;
      }

    request_error(cmdvec, string() + "request " + to_string(rid) + 
      " not found or access denied");
  }

void Connection::command_PURGE(const vector<string> &cmdvec)
  {
    if(cmdvec.size() != 2)
      {
        request_error(cmdvec, "PURGE requires 1 argument");
        return;
      }

    char c;
    unsigned int rid;
    if(sscanf(cmdvec[1].c_str(), "%u%c", &rid, &c) != 1)
      {
        request_error(cmdvec, "invalid request ID");
        return;
      }

    if(!partner.purge_request(to_string(rid), username))
      {
        request_error(cmdvec, string() + "request " + to_string(rid) + 
          " not found or access denied");
        return;
      }

    request_ok(cmdvec);
  }
    
void Connection::command_REQUEST(const vector<string> &cmdvec)
  {
    if(cmdvec.size() < 2)
      {
        request_error(cmdvec, "REQUEST requires at least 2 arguments");
        return;
      }

    string req_args;
    for(unsigned int i = 2; i < cmdvec.size(); ++i)
      {
        string::size_type sep;
        if((sep = cmdvec[i].find('=')) == string::npos)
          {
            request_error(cmdvec, "invalid request parameter");
            return;
          }

        if(i > 2)
            req_args += " ";

        req_args += cmdvec[i];
      }

    RequestType type;
    if((type = request_string2type(cmdvec[1])) == REQ_UNKNOWN)
      {
        request_error(cmdvec, "unsupported request type");
        return;
      }
    
    if((req = partner.new_request(type, username, password, institution,
      label, host, user_ip, req_args)) == NULL)
      {
        request_error(cmdvec, "maximum number of requests exceeded");
        return;
      }
    
    errflg = false;
    request_ok(cmdvec);
  }

void Connection::command_DOWNLOAD(const vector<string> &cmdvec, bool blocking)
  {
    int n;
    char c;
    unsigned int dlpos = 0;

    if(cmdvec.size() == 3)
      {
        if(sscanf(cmdvec[2].c_str(), "%u%c", &dlpos, &c) != 1)
          {
            request_error(cmdvec, "invalid start position");
            return;
          }
      }
    else if(cmdvec.size() != 2)
      {
        request_error(cmdvec, cmdvec[0] + " requires 1 or 2 arguments");
        return;
      }

    unsigned int rid;
    n = sscanf(cmdvec[1].c_str(), "%u%c", &rid, &c);

    if(n != 1 && (n != 2 || c != '.'))
      {
        request_error(cmdvec, "invalid request ID");
        return;
      }

    if((req = partner.find_request(to_string(rid), username)) == NULL)
      {
        request_error(cmdvec, string() + "request " + to_string(rid) + 
          " not found or access denied");
        return;
      }

    
    char volume[100];
    n = sscanf(cmdvec[1].c_str(), "%*u.%99s%c", volume, &c);

    try
      {
        
        if(n == -1) /* Normal requests - no volume */
          {
            if(!req->ready())
              {
                if(blocking)
                  {
                    download_fd = -1;
                    download_pos = dlpos;
                    download_vol = false;
                    downloading = true;
                    request_accepted(cmdvec);
                    return;
                  }
                 
                request_error(cmdvec, "request not ready for download");
                req = NULL;
                return;
              }
            
            if((download_fd = req->open_first(vol_iter, dlpos)) < 0)
              {
                request_error(cmdvec, "cannot open product");
                req = NULL;
                return;
              }

            // BIANCHI|ENCRYPT: Initializy the encryptor if needed
            if ((req->doEncrypt ()) && (! _encryptor.isReady ())) {
                _encryptor.initContext (_sslPasswordFile, adminpass, username, req->dcid(), organization, serverEmail);
                if (dlpos!=0)
                    logs(LOG_ERR) << "Encryption: Download pos != 0, partial down of encrypted data is not garantee to work, depend on client implementation." << endl;
            }

            download_pos = dlpos;
            download_vol = false;
            downloading = true;
            request_accepted(cmdvec);
            
            // BIANCHI|ENCRYPT: Check if the volume should be encrypted before reporting the correct size
            if (req->doEncrypt ()){
                response(to_string(Encrypt::expectedSize (req->size())) + "\r\n");
            } else {
                response(to_string(req->size()) + "\r\n");
            }
          }
        else if(n == 1) /* This handle when volume download is taken place */
          {
            reqsize_t size = -1;
            if((download_fd = req->open_vol(volume, dlpos, size)) < 0)
              {
                request_error(cmdvec, "cannot open product");
                req = NULL;
                return;
              }

            // BIANCHI|ENCRYPT: Initializy the encryptor if needed
            if ((req->doEncrypt ()) && (! _encryptor.isReady ())) {
                _encryptor.initContext (_sslPasswordFile, adminpass, username, req->dcid(), organization, serverEmail);
                if (dlpos!=0)
                    logs(LOG_ERR) << "Encryption: Download pos != 0, partial down of encrypted data is not garantee to work, depend on client implementation." << endl;
            }

            download_pos = dlpos;
            download_vol = true;
            downloading = true;
            request_accepted(cmdvec);
            
            // BIANCHI|ENCRYPT: Check if the volume should be encrypted before reporting the correct size
            if (req->doEncrypt ()){
                response(to_string(Encrypt::expectedSize (size)) + "\r\n");
            } else {
                response(to_string(size) + "\r\n");
            }
          }
        else
          {
            request_error(cmdvec, "invalid volume id");
            req = NULL;
            return;
          }
      }
    catch(ArclinkRequestError &e)
      {
        req = NULL;
        request_error(cmdvec, e.what());
      }
    catch(EncryptError &e) {
        req = NULL;
        request_error (cmdvec, e.what());
    }
  }

void Connection::request(const vector<string> &cmdvec)
  {
    msg_bufs.clear();

    if(downloading)
      {
        req = NULL;

        if(download_fd != -1)
            close(download_fd);

        download_fd = -1;
        download_pos = 0;
        downloading = false;
      }
    
    if(!strcasecmp(cmdvec[0].c_str(), "END"))
      {
        if(req == NULL)
          {
            request_error(cmdvec, "END out of context");
          }
        else if(cmdvec.size() != 1)
          {
            request_error(cmdvec, "END requires 0 arguments");
            req = NULL;
          }
        else
          {
            request_accepted(cmdvec);

            try
              {
                req->push_end();

                if(errflg)
                  {
                    logs(LOG_WARNING) << "request contains errors" << endl;
                    errmsg = "request contains errors\r\n";
                    response("ERROR\r\n");
                  }
                else
                  {
                    partner.queue_request(req);
                    response(to_string(req->id) + "\r\n");
                  }
              }
            catch(ArclinkRequestError &e)
              {
                logs(LOG_WARNING) << "request error: " + string(e.what()) << endl;
                errmsg = string(e.what()) + "\r\n";
                response("ERROR\r\n");
              }

            req = NULL;
          }

        errflg = false;
        return;
      }
    
    if(req != NULL)
      {
        try
          {
            vector<string>::const_iterator p = cmdvec.begin();
            string cmdstr = *p;

            while((++p) != cmdvec.end())
                cmdstr += (" " + *p);

            req->push_line(cmdstr);
          }
        catch(ArclinkRequestError &e)
          {
            request_rejected(cmdvec, e.what());
            errflg |= true;
          }

        return;
      }
    
    if(!strcasecmp(cmdvec[0].c_str(), "HELLO"))
      {
        if(cmdvec.size() == 1)
          {
            request_accepted(cmdvec);
            response(ident);
            return;
          }
        
        request_error(cmdvec, "HELLO requires 0 arguments");
        return;
      }
      
    if(!strcasecmp(cmdvec[0].c_str(), "BYE"))
      {
        if(cmdvec.size() == 1)
          {
            request_accepted(cmdvec);
            disconnect_requested = true;
            return;
          }

        request_error(cmdvec, "BYE requires 0 arguments");
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "USER"))
      {
        if(cmdvec.size() == 2)
          {
            username = cmdvec[1];
            password = "";
            if(auth())
              {
                request_ok(cmdvec);
                return;
              }

            request_error(cmdvec, "authentication failure");
            return;
          }
        else if(cmdvec.size() == 3)
          {
            username = cmdvec[1];
            password = cmdvec[2];
            if(auth())
              {
                request_ok(cmdvec);
                return;
              }

            request_error(cmdvec, "authentication failure");
            return;
          }

        request_error(cmdvec, "USER requires 1 or 2 arguments");
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "INSTITUTION"))
      {
        if(cmdvec.size() > 1)
          {
            institution = cmdvec[1];
            vector<string>::const_iterator p;
            for(p = cmdvec.begin() + 2; p != cmdvec.end(); ++p)
                institution += (" " + *p);
      
            request_ok(cmdvec);
            return;
          }

        request_error(cmdvec, "INSTITUTION requires at least 1 argument");
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "USER_IP"))
      {
        if(cmdvec.size() == 2)
          {
            user_ip = cmdvec[1];
            request_ok(cmdvec);
            return;
          }

        request_error(cmdvec, "USER_IP requires 1 argument");
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "LABEL"))
      {
        if(cmdvec.size() == 2)
          {
            label = cmdvec[1];
            request_ok(cmdvec);
            return;
          }

        request_error(cmdvec, "LABEL requires 1 argument");
        return;
      }

    if(!strcasecmp(cmdvec[0].c_str(), "SHOWERR"))
      {
        if(cmdvec.size() != 1)
          {
            request_error(cmdvec, "SHOWERR requires 0 arguments");
            return;
          }

        response(errmsg);
        request_accepted(cmdvec);
        return;
      }

    if(!authentication)
      {
        request_error(cmdvec, "authentication required");
        return;
      }
    
    if(!strcasecmp(cmdvec[0].c_str(), "REQUEST"))
        command_REQUEST(cmdvec);
    else if(!strcasecmp(cmdvec[0].c_str(), "STATUS"))
        command_STATUS(cmdvec);
    else if(!strcasecmp(cmdvec[0].c_str(), "DOWNLOAD"))
        command_DOWNLOAD(cmdvec, false);
    else if(!strcasecmp(cmdvec[0].c_str(), "BDOWNLOAD"))
        command_DOWNLOAD(cmdvec, true);
    else if(!strcasecmp(cmdvec[0].c_str(), "PURGE"))
        command_PURGE(cmdvec);
    else
        request_error(cmdvec, "unsupported command");
  }

Connection::DeliveryResult Connection::do_deliver()
  {
    if(have_response)
      {
        if(writen(clientfd, response_str.c_str(), response_str.length()) <= 0)
            return Fail;
        
        have_response = false;
      }
    else if(!msg_bufs.empty())
      {
        rc_ptr<MessageBuffer> buf = msg_bufs.front();
        msg_bufs.pop_front();
    
        if(writen(clientfd, buf->data(), buf->size()) <= 0)
            return Fail;

        if(msg_bufs.empty() && writen(clientfd, "END\r\n", 5) <= 0)
            return Fail;
      }
    else if(downloading)
      {
        internal_check(download_pos >= 0);
        internal_check(req != NULL);

        if(download_fd < 0)
          {
            if(!req->ready())
                return Ok;

            try
              {
                if((download_fd = req->open_first(vol_iter, download_pos)) < 0)
                  {
                    logs(LOG_WARNING) << "cannot open product" << endl;
                    errmsg = "cannot open product\r\n";
                    response("ERROR\r\n");
                    downloading = false;
                    req = NULL;
                    return Ok;
                  }

                if ((req->doEncrypt ()) && (! _encryptor.isReady ()))
                    _encryptor.initContext (_sslPasswordFile, adminpass, username, req->dcid(), organization, serverEmail);
            }
            catch(ArclinkRequestError &e)
              {
                logs(LOG_WARNING) << e.what() << endl;
                errmsg = string(e.what()) + "\r\n";
                response("ERROR\r\n");
                downloading = false;
                req = NULL;
                return Ok;
              }
            catch (EncryptError &e) /* For the encryptor error */ {
                logs(LOG_WARNING) << e.what() << endl;
                errmsg = string(e.what()) + "\r\n";
                response("ERROR\r\n");
                downloading = false;
                req = NULL;
                return Ok;
            }

            // BIANCHI|ENCRYPT: Check if the volume should be encrypted before reporting the correct size
            if (req->doEncrypt ()) {
                response(to_string(Encrypt::expectedSize (req->size())) + "\r\n");
            } else {
                response(to_string(req->size()) + "\r\n");
            }
            
            return Retry;
          }

        unsigned char buf[BLOCKSIZE];
        int bytes_read = read(download_fd, buf, BLOCKSIZE);

        if(bytes_read < 0)
          {
            logs(LOG_ERR) << "read error: " << strerror(errno) << endl;
            if(writen(clientfd, "END\r\n", 5) <= 0)
                return Fail;
          }
        else if(bytes_read == 0)
          {
            close(download_fd);
            download_fd = -1;
            download_pos = 0;
            
            if(download_vol == false)
                if ((download_fd = req->open_next(vol_iter)) >= 0)
                    return Retry;
            
            // BIANCHI|ENCRYPT: Flush the rest of the encrypt class for correctness
            if (req->doEncrypt ()) {
                int outPutSize = 0;
                const unsigned char *eBuf = NULL;
                
                // Check if encryptor is ready & has started
                if ((! _encryptor.isReady ()) || (! _encryptor.hasStarted ()))
                    return Fail;
                
                try {
                    // Encrypt
                    eBuf =  _encryptor.finish (&outPutSize);
                } catch(exception &e) {
                    logs(LOG_ERR) << e.what() << endl;
                    
                    if(writen(clientfd, "END\r\n", 5) <= 0)
                        return Fail;
                }
                
                // Deliver
                if(writen(clientfd, eBuf, outPutSize) != outPutSize)
                    return Fail;
                
                // Clean the Encryptor
                _encryptor.reset ();
            }
            
            downloading = false;
            req = NULL;
           
            if(writen(clientfd, "END\r\n", 5) <= 0)
                return Fail;
          }
        else
          {
              // BIANCHI|ENCRYPT: Check if we should send encrypted data, Encrypt & Send
              if (req->doEncrypt ()){
                  int outPutSize = 0;
                  const unsigned char *eBuf = NULL;

                  // Check if encryptor is ready
                  if (! _encryptor.isReady ())
                      return Fail;

                  try {
                      // Encrypt
                      eBuf = _encryptor.update (&outPutSize, buf, bytes_read);
                  } catch(exception &e) {
                      logs(LOG_ERR) << e.what() << endl;
                      return Fail;
                  }
                  
                  // Deliver
                  if(writen(clientfd, eBuf, outPutSize) != outPutSize)
                      return Fail;
              } else 
                  if(writen(clientfd, buf, bytes_read) != bytes_read)
                      return Fail;
              
              download_pos += bytes_read;
          }
      }
            
    return Ok;
  }

bool Connection::deliver()
  {
    errno = 0;
    int r;
    while((r = do_deliver()) == Retry);
    if(r == Fail)
      {
        if(errno != 0)
            logs(LOG_NOTICE) << "socket error: " << strerror(errno) << endl;

        if(downloading)
          {
            if(download_fd != -1)
                close(download_fd);

            download_fd = -1;
            download_pos = 0;
            downloading = false;
          }
    
        return true;
      }

    return false;
  }

bool Connection::input()
  {
    int r = 0;
    
    try
      {
        if((r = cfifo.check(clientfd)) == 0)
            return true;
      }
    catch(CFIFO_ReadError &e)
      {
        logs(LOG_NOTICE) << "socket error: " << strerror(errno) << endl;
        return true;
      }
    catch(CFIFO_Overflow &e)
      {
        logs(LOG_WARNING) << "command buffer overflow" << endl;
        return true;
      }

    if(disconnect_requested)
        return true;

    return false;
  }

void Connection::disconnect()
  {
    logs(LOG_NOTICE) << "closing connection" << endl;
    
    shutdown(clientfd, SHUT_RDWR);
    close(clientfd);  // maybe not the best to do it here
  }

//*****************************************************************************
// RequestInfo
//*****************************************************************************

class RequestInfo
  {
  public:
    const string id;
    const string user;
    time_t timestamp;
    bool ready;

    RequestInfo(const string& id_init, const string& user_init,
      time_t timestamp_init, bool ready_init):
      id(id_init), user(user_init), timestamp(timestamp_init),
      ready(ready_init) {}

    void getxml(xmlNodePtr parent) const;
  };

void RequestInfo::getxml(xmlNodePtr parent) const
  {
    xmlNodePtr child;
    child = xml_new_child(parent, "request");
    xml_new_prop(child, "id", id.c_str());
    xml_new_prop(child, "user", user.c_str());
    xml_new_prop(child, "timestamp", to_string(timestamp).c_str());
    xml_new_prop(child, "ready", (ready? "true": "false"));
  }

//*****************************************************************************
// RequestPrioritizer
//*****************************************************************************

class RequestPrioritizer
  {
  private:
    int max_req_per_user;
    map<string, list<rc_ptr<Request> > > user_requests[NREQTYPES];
    map<string, list<rc_ptr<Request> > >::iterator last_user[NREQTYPES];

  public:
    void put(rc_ptr<Request> req);
    rc_ptr<Request> get(RequestType type);
    void remove(rc_ptr<Request> req);
    int count(RequestType type);
    void dump();
    
    RequestPrioritizer(): max_req_per_user(0)
      {
        for(int i = 0; i < NREQTYPES; ++i)
          {
            last_user[i] = user_requests[i].end();
          }
      }

    void set_max_req_per_user(int n)
      {
        max_req_per_user = n;
      }
  };

void RequestPrioritizer::put(rc_ptr<Request> req)
  {
    internal_check(req->type >= 0 && req->type < NREQTYPES);

    map<string, list<rc_ptr<Request> > >::iterator p;
    
    int nreq = 0;
    for(int i = 0; i < NREQTYPES; ++i)
      {
        if((p = user_requests[i].find(req->user)) != user_requests[i].end())
            nreq += p->second.size();
      }

    if(max_req_per_user != 0 && nreq >= max_req_per_user)
        throw ArclinkRequestError("maximum number of queued requests exceeded (" + req->user + ")");

    if((p = user_requests[req->type].find(req->user)) != user_requests[req->type].end())
      {
        p->second.push_back(req);
      }
    else
      {
        list<rc_ptr<Request> > reqlist;
        reqlist.push_back(req);
        user_requests[req->type].insert(make_pair(req->user, reqlist));
      }
  }

rc_ptr<Request> RequestPrioritizer::get(RequestType type)
  {
    internal_check(type >= 0 && type < NREQTYPES);

    if(user_requests[type].empty())
        return NULL;

    if(last_user[type] == user_requests[type].end())
        last_user[type] = user_requests[type].begin();

    map<string, list<rc_ptr<Request> > >::iterator p = last_user[type]++;

    internal_check(!p->second.empty());

    rc_ptr<Request> req = p->second.front();
    p->second.pop_front();

    if(p->second.empty())
        user_requests[type].erase(p);

    return req;
  }
    
void RequestPrioritizer::remove(rc_ptr<Request> req)
  {
    for(int i = 0; i < NREQTYPES; ++i)
      {
        map<string, list<rc_ptr<Request> > >::iterator p;
        if((p = user_requests[i].find(req->user)) != user_requests[i].end())
          {
            list<rc_ptr<Request> >::iterator q;
            for(q = p->second.begin(); q != p->second.end(); ++q)
              {
                if((*q) == req)
                  {
                    p->second.erase(q);
                    if(p->second.empty())
                      {
                        if(last_user[req->type] == p)
                            ++last_user[req->type];

                        user_requests[i].erase(p);
                      }

                    logs(LOG_INFO) << "request " << req->id << " removed from queue" << endl;
                    return;
                  }
              }
          }
      }

    logs(LOG_INFO) << "request " << req->id << " not found in queue" << endl;
  }

int RequestPrioritizer::count(RequestType type)
  {
    internal_check(type >= 0 && type < NREQTYPES);

    int nreq = 0;
    map<string, list<rc_ptr<Request> > >::iterator p;
    for(p = user_requests[type].begin(); p != user_requests[type].end(); ++p)
        nreq += p->second.size();

    return nreq;
  }

void RequestPrioritizer::dump()
  {
    for(int i = 0; i < NREQTYPES; ++i)
      {
        map<string, list<rc_ptr<Request> > >::iterator p;
        for(p = user_requests[i].begin(); p != user_requests[i].end(); ++p)
            logs(LOG_INFO) << "queued (" << p->first << "," << request_type2string(RequestType(i)) << "): " << p->second.size() << endl;
      }
  }

//*****************************************************************************
// RequestTypeAttribute
//*****************************************************************************

class RequestTypeAttribute: public CfgAttribute
  {
  private:
    RequestType &valref;

  public:
    RequestTypeAttribute(const string &name, RequestType &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        RequestType x;
        if((x = request_string2type(value)) == REQ_UNKNOWN)
          {
            cfglog << "invalid request type " << value << endl;
            return false;
          }

        valref = x;
        return true;
      }
  };

//*****************************************************************************
// RequestStatusAttribute
//*****************************************************************************

class RequestStatusAttribute: public CfgAttribute
  {
  private:
    RequestStatus &valref;

  public:
    RequestStatusAttribute(const string &name, RequestStatus &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        RequestStatus x;
        if((x = request_string2status(value)) == STATUS_UNKNOWN)
          {
            cfglog << "invalid request status " << value << endl;
            return false;
          }

        valref = x;
        return true;
      }
  };

//*****************************************************************************
// TimeTAttribute
//*****************************************************************************

class TimeTAttribute: public CfgAttribute
  {
  private:
    time_t &valref;

  public:
    TimeTAttribute(const string &name, time_t &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        time_t x;
        char *tail;

        x = strtoul(value.c_str(), &tail, 10);
        if(*tail)
          {
            cfglog << "invalid timestamp" << value << endl;
            return false;
          }

        valref = x;
        return true;
      }
  };

//*****************************************************************************
// RequestInfoElement
//*****************************************************************************

class RequestInfoElement: public CfgElement
  {
  private:
    string id;
    string user;
    time_t timestamp;
    bool ready;
    map<string, rc_ptr<RequestInfo> > &request_infos;

  public:
    RequestInfoElement(map<string, rc_ptr<RequestInfo> >& request_infos_init):
      CfgElement("request"), timestamp(0), ready(false),
      request_infos(request_infos_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> RequestInfoElement::start_attributes(ostream &cfglog,
  const string &)
  {
    id = "";
    user = "";
    timestamp = 0;
    ready = false;

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("id", id));
    atts->add_item(StringAttribute("user", user));
    atts->add_item(TimeTAttribute("timestamp", timestamp));
    atts->add_item(BoolAttribute("ready", ready, "true", "false"));
    return atts;
  }

void RequestInfoElement::end_attributes(ostream &cfglog)
  {
    request_infos[id] = new RequestInfo(id, user, timestamp, ready);
  }

//*****************************************************************************
// RequestLineElement
//*****************************************************************************

class RequestLineElement: public CfgElement
  {
  private:
    string content;
    RequestStatus status;
    reqsize_t size;
    string message;
    rc_ptr<Request> request;

  public:
    RequestLineElement(const rc_ptr<Request>& request_init):
      CfgElement("line"), status(STATUS_UNSET), size(0),
      request(request_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> RequestLineElement::start_attributes(ostream &cfglog,
  const string &)
  {
    content = "";
    status = STATUS_UNSET;
    size = 0;
    message = "";

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("content", content));
    atts->add_item(RequestStatusAttribute("status", status));
    atts->add_item(Int64Attribute("size", size, 0, Int64Attribute::lower_bound));
    atts->add_item(StringAttribute("message", message));
    return atts;
  }

void RequestLineElement::end_attributes(ostream &cfglog)
  {
    request->restore_line(content, status, size, message);
  }

//*****************************************************************************
// RequestVolumeElement
//*****************************************************************************

class RequestVolumeElement: public CfgElement
  {
  private:
    string id;
    string _dcid;
    RequestStatus status;
    reqsize_t size;
    string message;
    rc_ptr<Request> request;

  public:
    RequestVolumeElement(const rc_ptr<Request>& request_init):
      CfgElement("volume"), status(STATUS_UNSET), size(0),
      request(request_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> RequestVolumeElement::start_attributes(ostream &cfglog,
  const string &)
  {
    id = "";
    _dcid = "";
    status = STATUS_UNSET;
    size = 0;
    message = "";

    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("id", id));
    atts->add_item(StringAttribute("dcid", _dcid));
    atts->add_item(RequestStatusAttribute("status", status));
    atts->add_item(Int64Attribute("size", size, 0, Int64Attribute::lower_bound));
    atts->add_item(StringAttribute("message", message));
    return atts;
  }

void RequestVolumeElement::end_attributes(ostream &cfglog)
  {
    request->restore_volume(id, status, size, message);
  }

rc_ptr<CfgElementMap> RequestVolumeElement::start_children(ostream &cfglog,
  const string &)
  {
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(RequestLineElement(request));
    return elms;
  }

//*****************************************************************************
// RequestElement
//*****************************************************************************

class RequestElement: public CfgElement
  {
  private:
    string id;
    RequestType type;
    string user;
    string institution;
    string label;
    string client_ip;
    string user_ip;
    string args;
    reqsize_t size;
    bool ready;
    bool error;
    string message;
    string location;
    map<string, rc_ptr<Request> > &requests;
    // BIANCHI|ENCRYPTION: Restricted flag
    bool _restricted;
    string _dcid;
    bool _encryption;

  public:
    RequestElement(map<string, rc_ptr<Request> >& requests_init,
      const string& location_init, const string& dcid_init, const bool &encryption_init):
      CfgElement("request"), type(REQ_UNKNOWN), size(0), ready(false),
      error(false), location(location_init), requests(requests_init),
      _dcid(dcid_init), _encryption(encryption_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &);

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &);

    void end_attributes(ostream &cfglog);

    void end_children(ostream &cfglog);
  };

rc_ptr<CfgAttributeMap> RequestElement::start_attributes(ostream &cfglog,
  const string &)
  {
    id = "";
    type = REQ_UNKNOWN;
    user = "";
    institution = "";
    label = "";
    client_ip = "";
    user_ip = "";
    args = "";
    size = 0;
    ready = false;
    error = false;
    message = "";
    // BIANCHI|ENCRYPTION: Restricted flag (Mirror on Request)
    _restricted = false;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(StringAttribute("id", id));
    atts->add_item(RequestTypeAttribute("type", type));
    atts->add_item(StringAttribute("user", user));
    atts->add_item(StringAttribute("institution", institution));
    atts->add_item(StringAttribute("label", label));
    atts->add_item(StringAttribute("client_ip", client_ip));
    atts->add_item(StringAttribute("user_ip", user_ip));
    atts->add_item(StringAttribute("args", args));
    atts->add_item(BoolAttribute("ready", ready, "true", "false"));
    // BIANCHI|ENCRYPTION: Restore the serialized restricted
    atts->add_item(BoolAttribute("restricted", _restricted, "true", "false"));
    atts->add_item(BoolAttribute("error", error, "true", "false"));
    atts->add_item(Int64Attribute("size", size, 0, Int64Attribute::lower_bound));
    atts->add_item(StringAttribute("message", message));
    return atts;
  }

void RequestElement::end_attributes(ostream &cfglog)
  {
    // BIANCHI|ENCRYPT: Added the restricted element
    requests[id] = new Request(id, type, user, institution, label,
      client_ip, user_ip, args, location, 0, _dcid, _encryption, ready, _restricted, error);
  }

rc_ptr<CfgElementMap> RequestElement::start_children(ostream &cfglog,
  const string &)
  {
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(RequestVolumeElement(requests[id]));
    return elms;
  }

void RequestElement::end_children(ostream &cfglog)
  {
    requests[id]->restore_end();
  }

//*****************************************************************************
// Arclink
//*****************************************************************************

class Arclink: private ConnectionPartner
  {
  private:
    string _serverEmail;
    string _dcid;
    string organization;
    string request_dir;
    string handler_cmd;
    string adminpass;
    // BIANCHI|ENCRYPTION: Location to the password file
    string _password_file;
    bool _encryption;
    int max_conn;
    int max_conn_per_ip;
    int max_req_per_user;
    int max_requests;
    int max_lines;
    int max_handlers_soft;
    int max_handlers_hard;
    int max_handlers[NREQTYPES];
    int num_handlers[NREQTYPES];
    int handler_start_retry;
    int handler_shutdown_wait;
    int handler_timeout;
    int tcp_port;
    int swapout_time;
    int purge_time;
    int listenfd;
    int request_count;
    time_t last_check;
    time_t last_cleanup;
    time_t last_queue_status;
    bool shutdown_requested;
    map<unsigned int, int> nconn_per_ip;

    map<string, rc_ptr<RequestInfo> > request_infos;
    map<string, rc_ptr<Request> > requests;
    RequestPrioritizer queue;
    list<rc_ptr<Reqhandler> > free_handlers;
    list<rc_ptr<Reqhandler> > busy_handlers;
    list<rc_ptr<Connection> > connections;

    // Callbacks from Connection;
    rc_ptr<Request> new_request(RequestType reqtype, const string &user,
      const string &pass, const string &institution, const string &label,
      const string &client_ip, const string &user_ip, const string &req_args);
    void queue_request(rc_ptr<Request> req);
    bool purge_request(string rid, const string &user);
    rc_ptr<Request> find_request(string rid, const string &user);
    bool get_status(list<rc_ptr<MessageBuffer> > &buflist,
      string rid, const string &user);
    
    void client_connect();
    void client_disconnect(rc_ptr<Connection> conn);
    void save_request(rc_ptr<Request> req);
    map<string, rc_ptr<Request> >::iterator get_request(string rid);
    rc_ptr<Request> get_next_req_from_queue();

    void recover_state();

  public:
    Arclink():
      _password_file(""), _encryption(false), max_conn(500), max_conn_per_ip(0),
      max_req_per_user(0), max_requests(500), max_lines(1000), max_handlers_soft(2),
      max_handlers_hard(4), handler_start_retry(60), handler_shutdown_wait(10),
      handler_timeout(600), tcp_port(0), swapout_time(0), purge_time(0), listenfd(-1),
      request_count(0), last_check(0), last_cleanup(0), last_queue_status(0),
      shutdown_requested(false)
      {
        for(int i = 0; i < NREQTYPES; ++i)
          {
            max_handlers[i] = 2;
            num_handlers[i] = 0;
          }
      }

    void config(rc_ptr<CfgAttributeMap> atts, rc_ptr<CfgElementMap> elms);
    void setup();
    void check();
    void shutdown();
    void restore_state(const string &filename);
    void save_state(const string &filename);
  };

rc_ptr<Request> Arclink::new_request(RequestType reqtype, const string &user,
  const string &pass, const string &institution, const string &label,
  const string &client_ip, const string &user_ip, const string &req_args)
  {
    internal_check(reqtype >= 0 && reqtype < NREQTYPES);

    int requests_queued = 0;
    for(int i = 0; i < NREQTYPES; ++i)
        requests_queued += queue.count(RequestType(i));
    
    if(requests_queued >= max_requests &&
      queue.count(reqtype) >= max_handlers[reqtype])
        return NULL;

    rc_ptr<Request> req = new Request(to_string(request_count), reqtype, user,
      institution, label, client_ip, user_ip, req_args,
      request_dir + "/" + to_string(request_count), max_lines, _dcid, _encryption);
    
    ++request_count;
    return req;
  }

void Arclink::queue_request(rc_ptr<Request> req)
  {
    internal_check(req->type >= 0 && req->type < NREQTYPES);

    queue.put(req);
    requests[req->id] = req;
    request_infos[req->id] = new RequestInfo(req->id, req->user, time(NULL), false);
  }

void Arclink::save_request(rc_ptr<Request> req)
  {
    xmlSetGenericErrorFunc(NULL, xml_error);

    xmlDocPtr doc;
    if((doc = xmlNewDoc((const xmlChar *) "1.0")) == NULL)
        throw bad_alloc();

    if((doc->children = xmlNewDocNode(doc, NULL, (const xmlChar *) "arclink", NULL)) == NULL)
        throw bad_alloc();

    req->getxml(doc->children, true, true);
    
    xmlSaveFormatFile((request_dir + "/" + req->id + ".desc").c_str(), doc, 1);
    xmlFreeDoc(doc);
    xmlSetGenericErrorFunc(NULL, NULL);
  }

map<string, rc_ptr<Request> >::iterator Arclink::get_request(string rid)
  {
    map<string, rc_ptr<Request> >::iterator p;
    if((p = requests.find(rid)) != requests.end())
        return p;

    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(RequestElement(requests, request_dir + "/" + rid, _dcid, _encryption));

    try
      {
        read_config_xml(request_dir + "/" + to_string(rid) + ".desc",
          "arclink", new CfgAttributeMap, elms);
      }
    catch(CfgError &e)
      {
        logs(LOG_ERR) << e.what() << endl;
        return requests.end();
      }

    if((p = requests.find(rid)) == requests.end())
      {
        logs(LOG_ERR) << "unable to find request " << rid << endl;
        return requests.end();
      }
        
    return p;
  }

bool Arclink::purge_request(string rid, const string &user)
  {
    map<string, rc_ptr<RequestInfo> >::iterator i;
    if((i = request_infos.find(rid)) == request_infos.end())
        return false;

    if(user != "admin" && user != i->second->user)
        return false;

    map<string, rc_ptr<Request> >::iterator p;
    if((p = get_request(rid)) != requests.end())
      {
        if(!p->second->ready())
            queue.remove(p->second);

        p->second->kill();
        requests.erase(p);
      }

    unlink((request_dir + "/" + i->second->id + ".desc").c_str());
    request_infos.erase(i);
    return true;
  }

rc_ptr<Request> Arclink::find_request(string rid, const string &user)
  {
    map<string, rc_ptr<RequestInfo> >::iterator i;
    if((i = request_infos.find(rid)) == request_infos.end())
        return NULL;

    if(user != "admin" && user != i->second->user)
        return NULL;

    i->second->timestamp = time(NULL);

    map<string, rc_ptr<Request> >::iterator p;
    if((p = get_request(rid)) == requests.end())
        return NULL;

    return p->second;
  }

bool Arclink::get_status(list<rc_ptr<MessageBuffer> > &buflist,
  string rid, const string &user)
  {
    rc_ptr<Request> req;
    if(rid != RID_ALL && (req = find_request(rid, user)) == NULL)
        return false;
    
    xmlSetGenericErrorFunc(NULL, xml_error);

    xmlDocPtr doc;
    if((doc = xmlNewDoc((const xmlChar *) "1.0")) == NULL)
        throw bad_alloc();

    if((doc->children = xmlNewDocNode(doc, NULL, (const xmlChar *) "arclink", NULL)) == NULL)
        throw bad_alloc();

    
    if(rid == RID_ALL)
      {
        map<string, rc_ptr<RequestInfo> >::iterator i;
        for(i = request_infos.begin(); i != request_infos.end(); ++i)
          {
            if(user != "admin" && user != i->second->user)
                continue;

            map<string, rc_ptr<Request> >::iterator p;
            if((p = get_request(i->second->id)) == requests.end())
                continue;
            
            p->second->getxml(doc->children, user == "admin", false);
          }
      }
    else
      {
        req->getxml(doc->children, false, false);
      }
    
    XMLOutput xout(buflist, BLOCKSIZE);

    xout.write_doc(doc);
    xmlFreeDoc(doc);
    xmlSetGenericErrorFunc(NULL, NULL);

    return true;
  }

void Arclink::config(rc_ptr<CfgAttributeMap> atts, rc_ptr<CfgElementMap> elms)
  {
    atts->add_item(StringAttribute("organization", organization));
    atts->add_item(StringAttribute("dcid", _dcid));
    atts->add_item(StringAttribute("contact_email", _serverEmail));
    atts->add_item(StringAttribute("request_dir", request_dir));
    atts->add_item(IntAttribute("connections", max_conn, 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("connections_per_ip", max_conn_per_ip, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("request_queue", max_requests, 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("request_queue_per_user", max_req_per_user, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("request_size", max_lines, 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_soft", max_handlers_soft, 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_hard", max_handlers_hard, 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_waveform", max_handlers[REQ_WAVEFORM], 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_inventory", max_handlers[REQ_INVENTORY], 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_routing", max_handlers[REQ_ROUTING], 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_response", max_handlers[REQ_RESPONSE], 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_qc", max_handlers[REQ_QC], 1, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handlers_greensfunc", max_handlers[REQ_GREENSFUNC], 1, IntAttribute::lower_bound));
    atts->add_item(StringAttribute("handler_cmd", handler_cmd));
    atts->add_item(IntAttribute("handler_start_retry", handler_start_retry, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handler_shutdown_wait", handler_shutdown_wait, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("handler_timeout", handler_timeout, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("port", tcp_port, 1, 65535));
    atts->add_item(IntAttribute("purge_time", purge_time, 0, IntAttribute::lower_bound));
    atts->add_item(IntAttribute("swapout_time", swapout_time, 0, IntAttribute::lower_bound));
    atts->add_item(StringAttribute("admin_password", adminpass));
    // BIANCHI|ENCRYPTION: Read-In configuration variables for encryption
    atts->add_item(BoolAttribute("encryption", _encryption, "true", "false"));
    atts->add_item(StringAttribute("password_file", _password_file));
  }

void Arclink::setup()
  {
    if(_encryption){
        int err = 0;
        logs(LOG_NOTICE) << "arclink encryption is enabled" << endl;

        if (_dcid.empty ()){
            logs(LOG_ERR) << "'dcid' is not specified but needed for encryption." << endl;
            err = 1;
        }

        if (adminpass.empty ()){
            logs(LOG_ERR) << "'adminpass' is not specified but needed for encryption." << endl;
            err = 1;
        }

        if (_password_file.empty ()){
            logs(LOG_ERR) << "'password_file' is not specified but needed for encryption." << endl;
            err = 1;
        }

        if (err)
            exit(1);
    }
    
    if(handler_cmd.length() == 0)
      {
        logs(LOG_ERR) << "'handler_cmd' is not specified" << endl;
        exit(1);
      }
    
    if(request_dir.length() == 0)
      {
        logs(LOG_ERR) << "'request_dir' is not defined" << endl;
        exit(1);
      }

    mkdir(request_dir.c_str(), 0755);

    int fd;
    const string test_file = request_dir + "/test";
    if((fd = creat(test_file.c_str(), 0644)) < 0)
      {
        logs(LOG_ERR) << "cannot create files under 'request_dir' (" <<
          request_dir << ")" << endl;
        exit(1);
      }  
    
    close(fd);
    unlink(test_file.c_str());
    
    N(listenfd = xsocket(PF_INET, SOCK_STREAM, 0));

    int optval = 1;
    N(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)));

    struct sockaddr_in inet_addr;
    inet_addr.sin_family = AF_INET;
    inet_addr.sin_port = htons(tcp_port);
    inet_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd, (struct sockaddr *) &inet_addr, sizeof(inet_addr)) < 0)
        throw ArclinkLibraryError("bind error");

    N(listen(listenfd, 5));
  }

void Arclink::client_connect()
  {
    int clientfd;
    struct sockaddr_in inet_addr;

#if defined(__GNU_LIBRARY__) && __GNU_LIBRARY__ < 2
    int len;
#else
    socklen_t len;
#endif

    len = sizeof(inet_addr);
    if((clientfd = xaccept(listenfd, (struct sockaddr *) &inet_addr, &len)) < 0)
      {
        logs(LOG_NOTICE) << "socket error: " << strerror(errno) << endl;
        return;
      }
    
    unsigned int ipaddr = ntohl(inet_addr.sin_addr.s_addr);
    int port = ntohs(inet_addr.sin_port);

#ifdef TCPWRAP
    struct request_info req;
    request_init(&req, RQ_DAEMON, daemon_name.c_str(), RQ_FILE, clientfd, 0);
    fromhost(&req);
    
    const char* host = eval_client(&req);
    
    if(!hosts_access(&req))
      {
        logs(LOG_WARNING) << host << ":" << port << " : not allowed" << endl;
        shutdown(clientfd, SHUT_RDWR);
        close(clientfd);
        return;
      }
#else
    int optval = 1;
    N(setsockopt(clientfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)));
    const char* host = inet_ntoa(inet_addr.sin_addr);
#endif
    
    if(max_conn != 0 && (int)connections.size() >= max_conn)
      {
        logs(LOG_NOTICE) << host << ":" << port << " : maximum number of "
          "connections (" << max_conn << ") exceeded" << endl;
        ::shutdown(clientfd, SHUT_RDWR);
        close(clientfd);
        return;
      }
        
    map<unsigned int, int>::iterator i;
    if((i = nconn_per_ip.find(ipaddr)) == nconn_per_ip.end())
      {
          nconn_per_ip.insert(make_pair(ipaddr, 1));
      }
    else if(max_conn_per_ip != 0 && i->second >= max_conn_per_ip)
      {
        logs(LOG_NOTICE) << host << ":" << port << " : maximum number of "
          "connections per IP (" << max_conn_per_ip << ") exceeded" << endl;
        ::shutdown(clientfd, SHUT_RDWR);
        close(clientfd);
        return;
      }
    else
      {
        ++(i->second);
      }
    
    logs(LOG_NOTICE) << string(host) << ":" << port <<
      " : opening connection" << endl;
    
    rc_ptr<Connection> conn = new Connection(*this, string(ident_str) +
      "\r\n" + organization + "\r\n", ipaddr, host, port, adminpass, clientfd,
      CPPStreams::logs.stream(string(host) + ":" + to_string(port) + " : "),
      _password_file, organization, _serverEmail);

    connections.push_back(conn);
  }

void Arclink::client_disconnect(rc_ptr<Connection> conn)
  {
    conn->disconnect();

    map<unsigned int, int>::iterator i = nconn_per_ip.find(conn->ip());
    internal_check(i != nconn_per_ip.end());

    if(--(i->second) == 0)
          nconn_per_ip.erase(i);
  }

rc_ptr<Request> Arclink::get_next_req_from_queue()
  {
    static unsigned int q = 0;

    for(int i = 0; i < NREQTYPES; ++i)
      {
        internal_check(q < NREQTYPES);

        rc_ptr<Request> req;
        if(num_handlers[q] < max_handlers[q])
          {
            rc_ptr<Request> req = queue.get(RequestType(q));

            if(req != NULL)
                return req;
          }

        q = (q + 1) % NREQTYPES;
      }
    
    return NULL;
  }

void Arclink::check()
  {
    fd_set read_set;
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_ZERO(&read_set);
    int fd_max = -1;

    time_t curtime = time(NULL);
    queue.set_max_req_per_user(max_req_per_user);
    
    if(!shutdown_requested)
      {
        // If there are any requests waiting, find handlers for those.
        
        rc_ptr<Request> req;
        while((int)busy_handlers.size() < max_handlers_hard &&
          (req = get_next_req_from_queue()) != NULL)
          {
            rc_ptr<Reqhandler> rqh;
            if(free_handlers.size() > 0)
              {
                rqh = free_handlers.front();
                free_handlers.pop_front();
              }
            else
              {
                rqh = new Reqhandler(handler_cmd, handler_timeout,
                  handler_start_retry, handler_shutdown_wait);
                rqh->start();
              }

            busy_handlers.push_back(rqh);
            rqh->attach_request(req);

            internal_check(req->type >= 0 && req->type < NREQTYPES);
            internal_check(num_handlers[req->type] < max_handlers[req->type]);
            ++num_handlers[req->type];
          }
      }
        
    // Shut down unneeded handlers, one at a time

    if(free_handlers.size() > 0 &&
      (int)free_handlers.size() > max_handlers_soft)
      {
        rc_ptr<Reqhandler> rqh = free_handlers.front();
        free_handlers.pop_front();
        busy_handlers.push_back(rqh);
        rqh->shutdown();
      }

    FD_SET(listenfd, &read_set);
    fd_max = listenfd;
    
    // If any requests are ready, detach those from their handlers and
    // move handlers to the free queue or shut them down.
    
    list<rc_ptr<Reqhandler> >::iterator h = busy_handlers.begin();
    while(h != busy_handlers.end())
      {
        if((*h)->ready())
          {
            rc_ptr<Request> req = (*h)->detach_request();
            free_handlers.push_back(*h);
            busy_handlers.erase(h++);

            internal_check(req->type >= 0 && req->type < NREQTYPES);
            internal_check(num_handlers[req->type] > 0);
            --num_handlers[req->type];

            if(!req->zombie())
              {
                map<string, rc_ptr<RequestInfo> >::iterator i;
                if((i = request_infos.find(req->id)) != request_infos.end())
                    i->second->ready = true;
                else
                    logs(LOG_ERR) << "unable to find request info for " <<
                      req->id << endl;

                save_request(req);
              }

            continue;
          }
        else if((*h)->cancelled())
          {
            rc_ptr<Request> req = (*h)->detach_request();

            internal_check(req->type >= 0 && req->type < NREQTYPES);
            internal_check(num_handlers[req->type] > 0);
            --num_handlers[req->type];

            (*h)->shutdown();

            logs(LOG_INFO) << "processing of request " << req->id << " cancelled" << endl;
          }

        // FD of any request handler is added to read set. If we want to
        // send something to the request handler ((*h)->pending()), add fd
        // also to write set.
        
        pair<int, int> fd = (*h)->filedes();
        
        if(fd.first != -1)
          {
            FD_SET(fd.first, &read_set);

            if(fd.first > fd_max)
                fd_max = fd.first;
          }
        
        if(fd.second != -1 && (*h)->pending())
          {
            FD_SET(fd.second, &write_set);
 
            if(fd.second > fd_max)
                fd_max = fd.second;
          }

        ++h;
      }

    // FD of any connection is added to read set. If we want to send
    // something to client ((*c)->pending()), add fd also to write set.
    
    list<rc_ptr<Connection> >::iterator c;
    for(c = connections.begin(); c != connections.end(); ++c)
      {
        int fd = (*c)->filedes();
        
        FD_SET(fd, &read_set); 

        if((*c)->pending() || last_check != curtime)
            FD_SET(fd, &write_set);
            
        if(fd > fd_max)
            fd_max = fd;
      }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if(select(fd_max + 1, &read_set, &write_set, NULL, &tv) < 0)
      {
        if(errno == EINTR) return;
        throw ArclinkLibraryError("select error");
      }

    // Check if somebody wants to connect us.
    
    if(FD_ISSET(listenfd, &read_set))
        client_connect();
    
    // Feed next line of request to busy handlers that are ready for it.
    // If any handlers have died, these will be restarted and we start
    // feeding the request from the beginning.
    // Remove any handlers that have finished shutdown.
    
    h = busy_handlers.begin();
    while(h != busy_handlers.end())
      {
        pair<int, int> fd = (*h)->filedes();

        if(fd.second != -1 && FD_ISSET(fd.second, &write_set))
            (*h)->push_request();
            
        if((fd.first != -1 && FD_ISSET(fd.first, &read_set)) ||
          last_check != curtime)
          {
            if((*h)->check())
              {
                busy_handlers.erase(h++);
                continue;
              }
          }

        ++h;
      }

    // Restart any free handlers that might have died.
    // Remove any free handlers that have finished shutdown. That should
    // never happen, because we never shut down free handlers--hence
    // the warning.
    
    h = free_handlers.begin();
    while(h != free_handlers.end())
      {
        if(last_check != curtime)
          { 
            if((*h)->check())
              {
                logs(LOG_WARNING) << "free handler has shut down" << endl;
                free_handlers.erase(h++);
                continue;
              }
          }

        ++h;
      }
        
    // Check connections.
    
    c = connections.begin();
    while(c != connections.end())
      {
        int fd = (*c)->filedes();

        if(fd != -1)
          {
            if((FD_ISSET(fd, &write_set) && (*c)->deliver()) ||
              (FD_ISSET(fd, &read_set) && (*c)->input()))
              {
                client_disconnect(*c);
                connections.erase(c++);
                continue;
              }
          }

        ++c;
      }

    last_check = curtime;

    if(last_cleanup + CLEANUP_PERIOD < curtime)
      {
        map<string, rc_ptr<RequestInfo> >::iterator i = request_infos.begin();
        while(i != request_infos.end())
          {
            if(!i->second->ready)
              {
                ++i;
                continue;
              }

            if(purge_time != 0 && i->second->timestamp + purge_time < curtime)
              {
                logs(LOG_INFO) << "purge request " << i->second->id << endl;
                map<string, rc_ptr<Request> >::iterator p;
                if((p = get_request(i->second->id)) != requests.end())
                  {
                    p->second->kill();
                    requests.erase(p);
                  }

                unlink((request_dir + "/" + i->second->id + ".desc").c_str());
                request_infos.erase(i++);
                continue;
              }

            if(swapout_time != 0 && i->second->timestamp + swapout_time < curtime)
              {
                map<string, rc_ptr<Request> >::iterator p;
                if((p = requests.find(i->second->id)) != requests.end())
                  {
                    logs(LOG_DEBUG) << "swapout request " << i->second->id << endl;
                    requests.erase(p);
                  }
              }

            ++i;
          }

        last_cleanup = curtime;
      }

    if(last_queue_status + QUEUE_STATUS_PER < curtime)
      {
        queue.dump();
        
        int requests_queued = 0;
        for(int i = 0; i < NREQTYPES; ++i)
          {
            requests_queued += queue.count(RequestType(i));
          }

        logs(LOG_INFO) << "queued total: " << requests_queued << endl;

        last_queue_status = curtime;
      }
  }

void Arclink::shutdown()
  {
    logs(LOG_INFO) << "shutting down" << endl;

    shutdown_requested = true;

    list<rc_ptr<Reqhandler> >::iterator p = free_handlers.begin();
    while(p != free_handlers.end())
      {
        busy_handlers.push_back(*p);
        free_handlers.erase(p++);
      }
    
    for(p = busy_handlers.begin(); p != busy_handlers.end(); ++p)
        (*p)->shutdown();
    
    while(!busy_handlers.empty())
        check();
  }

void Arclink::recover_state()
  {
    time_t timestamp = time(NULL);

    DIR *dir;
    if((dir = opendir(request_dir.c_str())) == NULL)
        throw ArclinkCannotOpenDir(request_dir);

    struct dirent *de;
    while((de = readdir(dir)) != NULL)
      {
        size_t len = strlen(de->d_name);
        if(len < 5 || strcmp(&de->d_name[len - 5], ".desc"))
            continue;

        string rid(de->d_name, len - 5);
        map<string, rc_ptr<Request> >::iterator p;
        if((p = get_request(rid)) != requests.end())
          {
            char c;
            unsigned int rid = 0;

            if(sscanf(p->second->id.c_str(), "%u%c", &rid, &c) != 1)
              {
                logs(LOG_ERR) << "invalid request ID " << p->second->id << endl;
                continue;
              }

            if((int)rid + 1 > request_count)
                request_count = rid + 1;

            request_infos[p->second->id] = new RequestInfo(p->second->id,
              p->second->user, timestamp, p->second->ready());
          }
      }
            
    closedir(dir);
  }

void Arclink::restore_state(const string &filename)
  {
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    atts->add_item(IntAttribute("request_count", request_count, 0,
      IntAttribute::lower_bound));

    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    elms->add_item(RequestInfoElement(request_infos));
    
    try
      {
        read_config_xml(filename, "arclink", atts, elms);
        unlink(filename.c_str());
      }
    catch(CfgError &e)
      {
        logs(LOG_WARNING) << e.what() << endl;
        recover_state();
      }

    map<string, rc_ptr<RequestInfo> >::iterator i = request_infos.begin();
    while(i != request_infos.end())
      {
        if(!i->second->ready)
          {
            map<string, rc_ptr<Request> >::iterator p;
            if((p = get_request(i->second->id)) == requests.end())
              {
                request_infos.erase(i++);
                continue;
              }

            internal_check(p->second->type >= 0 && p->second->type < NREQTYPES);
            queue.put(p->second);
          }

        ++i;
      }
  }

void Arclink::save_state(const string &filename)
  {
    map<string, rc_ptr<Request> >::iterator p;
    for(p = requests.begin(); p != requests.end(); ++p)
        if(!p->second->ready())
            save_request(p->second);
    
    xmlSetGenericErrorFunc(NULL, xml_error);

    xmlDocPtr doc;
    if((doc = xmlNewDoc((const xmlChar *) "1.0")) == NULL)
        throw bad_alloc();

    if((doc->children = xmlNewDocNode(doc, NULL, (const xmlChar *) "arclink", NULL)) == NULL)
        throw bad_alloc();

    xml_new_prop(doc->children, "request_count", to_string(request_count).c_str());

    map<string, rc_ptr<RequestInfo> >::iterator i;
    for(i = request_infos.begin(); i != request_infos.end(); ++i)
        i->second->getxml(doc->children);
    
    xmlSaveFormatFile(filename.c_str(), doc, 1);
    xmlFreeDoc(doc);
    xmlSetGenericErrorFunc(NULL, NULL);
  }

Arclink arclink;

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
        throw ArclinkCannotOpenFile(lockfile);

    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
  
    if(fcntl(fd, F_SETLK, &lock) < 0)
      {
        if(errno == EACCES || errno == EAGAIN) return 1;
        else throw ArclinkLibraryError("cannot lock file '" + lockfile + "'");
      }
  
    N(ftruncate(fd, 0));
    sprintf(buf, "%d\n", getpid());
    
    errno = 0;
    if(write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
        throw ArclinkLibraryError("cannot write pid to '" + lockfile + "'");

    N((val = fcntl(fd,F_GETFD,0)));

    val |= FD_CLOEXEC;
    N(fcntl(fd, F_SETFD, val));

    return 0;
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

    daemon_name = get_progname(argv[0]);
    
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

    if(optind != argc)
      {
        fprintf(stderr, help_message, get_progname(argv[0]).c_str());
        exit(1);
      }

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
        openlog(daemon_name.c_str(), 0, SYSLOG_FACILITY);
        daemon_init = true;
      }

    redirect_ostream(cout, LogFunc(), LOG_INFO);
    redirect_ostream(cerr, LogFunc(), LOG_ERR);
    redirect_ostream(clog, LogFunc(), LOG_ERR);

    logs(LOG_NOTICE) << ident_str << " started" << endl;
    
    rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
    rc_ptr<CfgElementMap> elms = new CfgElementMap;
    arclink.config(atts, elms);
    
    string lockfile, statefile;
    atts->add_item(StringAttribute("lockfile", lockfile));
    atts->add_item(StringAttribute("statefile", statefile));

    logs(LOG_INFO) << "loading configuration from file '" << config_file << "'" << endl;
    read_config_ini(config_file, daemon_name, atts, elms);
    
    if(lockfile.length() != 0 && run_check(lockfile))
      {
        logs(LOG_ERR) << "already running" << endl;
        exit(1);
      }
    
    try
      {
        arclink.setup();

        if(statefile.length() > 0)
            arclink.restore_state(statefile);

        while(!terminate_proc)
            arclink.check();

        if(statefile.length() > 0)
            arclink.save_state(statefile);
      }
    catch(exception &e)
      {
        arclink.shutdown();
        throw;
      }

    arclink.shutdown();
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
 
