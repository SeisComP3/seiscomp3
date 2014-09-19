/*****************************************************************************
 * iosystem.cc
 *
 * Module "IOSystem"
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <algorithm>
#include <new>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef TCPWRAP
extern "C" {
#include "tcpd.h"
}
#endif

#include "bitstring.h"

#include "libslink.h"

#include "iosystem.h"
#include "monitor.h"
#include "confbase.h"
#include "cppstreams.h"
#include "utils.h"
#include "descriptor.h"
#include "buffer.h"
#include "diag.h"

#ifdef LONGWORD_64BIT
#define LONG long
#else
#define LONG long long
#endif

#ifndef SHUT_RD
#define SHUT_RD   0
#define SHUT_RW   1
#define SHUT_RDWR 2
#endif

namespace IOSystem_private {

using namespace std;
using namespace SeedlinkMonitor;
using namespace CPPStreams;
using namespace CfgParser;
using namespace Utilities;

// Conflicts with definition in libslink.h
// const char *const SIGNATURE         = "SL";
const int         IOSIZE            = 520;
const int         CMDLEN            = 100;

#ifdef FD_REALLOC
const int         FD_REALLOC_LIMIT    = 1024 * 1024;
bitstr_t bit_decl(fd_bitmap, (FD_REALLOC_LIMIT - FD_SETSIZE));
#endif

Fdset fds;

//*****************************************************************************
// Close-on-Exec versions of some UNIX calls
//*****************************************************************************

// We don't want to give open IOSystem file descriptors to seedlink plugins.
// If macro FD_REALLOC is defined, then file descriptors are moved above
// FD_SETSIZE.

inline int _fdrealloc(int fd)
  {
#ifdef FD_REALLOC
    if(fd < 0)
        return fd;

    int newfd = -1;
    bit_ffc(fd_bitmap, (FD_REALLOC_LIMIT - FD_SETSIZE), &newfd);

    if(newfd == -1)
        throw CannotAllocateFD();

    bit_set(fd_bitmap, newfd);
    
    newfd += FD_SETSIZE;
    if(dup2(fd, newfd) < 0)
      {
        logs(LOG_DEBUG) << "cannot dup fd " << fd << " to " << newfd << endl;
        bit_clear(fd_bitmap, (newfd - FD_SETSIZE));
        return fd;
      }

    close(fd);
    return newfd;
#else
    return fd;
#endif
  }

inline int xopen(const char *pathname, int flags)
  {
    int fd;
    if((fd = _fdrealloc(open(pathname, flags))) >= 0)
        fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
  }

inline int xcreat(const char *pathname, mode_t mode)
  {
    int fd;
    if((fd = _fdrealloc(creat(pathname, mode))) >= 0)
        fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
  }

inline int xclose(int fd)
  {
#ifdef FD_REALLOC
    if(fd >= FD_SETSIZE && fd < FD_REALLOC_LIMIT)
        bit_clear(fd_bitmap, (fd - FD_SETSIZE));
#endif

    return close(fd);
  }

inline int xsocket(int domain, int type, int protocol)
  {
    int fd;
    if((fd = socket(domain, type, protocol)) >= 0)
        fcntl(fd, F_SETFD, FD_CLOEXEC);

    if(fd >= FD_SETSIZE)
        throw FDSetsizeExceeded(fd);
    
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

    if(fd >= FD_SETSIZE)
        throw FDSetsizeExceeded(fd);
    
    return fd;
  }

//*****************************************************************************
// Sequence
//*****************************************************************************

class Sequence
  {
  friend ostream &operator<<(ostream &s, const Sequence &x);
  private:
    int value;

  public:
    enum { size = 6, mask = 0xffffff, uninitialized = -1 };
    
    Sequence(): value(uninitialized) {}
    
    Sequence(int value_init): value(value_init)
      {
        internal_check(value == uninitialized || !(value & ~mask));
      }

    void increment()
      {
        internal_check(value != uninitialized);
        value = (value + 1) & mask;
      }

    Sequence operator+(int n) const
      {
        internal_check(value != uninitialized && !(n & ~mask));
        return ((value + n) & mask);
      }

    Sequence operator-(int n) const
      {
        internal_check(value != uninitialized && !(n & ~mask));
        return ((value - n) & mask);
      }

    operator int() const
      {
        return value;
      }
  };

ostream &operator<<(ostream &s, const Sequence &x)
  {
    ios_base::fmtflags flags_saved = s.flags(ios_base::hex | ios_base::uppercase);
    char filler_saved = s.fill('0');
  
    internal_check(x != Sequence::uninitialized);
    s << setw(x.size) << x.value;
    s.flags(flags_saved);
    s.fill(filler_saved);
    return s;
  }

// ios_base::hex doesn't seem to work for input streams in libstdc++ 2.90.7
#ifdef USE_HEX_ISTREAM
istream &operator>>(istream &s, Sequence &x)
  {
    int testx;
    ios_base::fmtflags flags_saved = s.flags(ios_base::hex);
  
    if(s >> testx)
      {
        if(testx & ~Sequence::mask) s.clear(ios_base::badbit);
        else x = testx;
      }

    s.flags(flags_saved);
    return s;
  }

#else
istream &operator>>(istream &s, Sequence &x)
  {
    int testx;
    char *tail;
    string buf;

    if(s >> setw(Sequence::size) >> buf)
      {
        testx = strtoul(buf.c_str(), &tail, 16);
        if(*tail || (testx & ~Sequence::mask)) s.clear(ios_base::badbit);
        else x = testx;
      }

    return s;
  }
#endif

//*****************************************************************************
// BufferImpl
//*****************************************************************************

class BufferImpl: public Buffer
  {
  friend class BufferStoreImpl;
  private:
    BufferImpl *prevptr;
    BufferImpl *nextptr;
    void *dataptr;
    Sequence seq;

    ~BufferImpl()
      {
        free(dataptr);
      }
    
  public:
    BufferImpl(int size): Buffer(size), prevptr(NULL), nextptr(NULL), dataptr(NULL)
      {
        if((dataptr = malloc(size)) == NULL) throw bad_alloc();
      }

    BufferImpl *next() const
      {
        return nextptr;
      }

    void *data() const
      {
        return dataptr;
      }

    Sequence sequence() const
      {
        return seq;
      }
  };


//*****************************************************************************
// BufferStoreImpl
//*****************************************************************************

class BufferStoreImplPartner
  {
  public:
    virtual void new_buffer(BufferImpl *buf) =0;
    virtual void delete_oldest_buffer(BufferImpl *buf) =0;
    virtual ~BufferStoreImplPartner() {}
  };
  
class BufferStoreImpl: public BufferStore
  {
  private:
    BufferStoreImplPartner &partner;
    BufferImpl *buf_head, *buf_free, *buf_first, *buf_queue;
    Sequence seq;

    void next_buffer();
    
    void insert_buffer_after(BufferImpl *a, BufferImpl *b)
      {
        a->prevptr = b;

        if(b != NULL)
          {
            a->nextptr = b->nextptr;
            b->nextptr = a;
          }
        else
          {
            a->nextptr = NULL;
          }

        if(a->nextptr) a->nextptr->prevptr = a;
      }

    void insert_buffer_before(BufferImpl *a, BufferImpl *b)
      {
        a->nextptr = b;

        if(b != NULL)
          {
            a->prevptr = b->prevptr;
            b->prevptr = a;
          }
        else
          {
            a->prevptr = NULL;
          }

        if(a->prevptr) a->prevptr->nextptr = a;
      }

    void remove_buffer(BufferImpl *a)
      {
        if(a->prevptr) a->prevptr->nextptr = a->nextptr;
        if(a->nextptr) a->nextptr->prevptr = a->prevptr;
      }

  public:
    BufferStoreImpl(BufferStoreImplPartner &partner_init, int bufsize,
      int nbufs);
    ~BufferStoreImpl();
    Buffer *get_buffer();
    void queue_buffer(Buffer *buf1);
    void load_buffers(int fd);
    void create_blank_buffers(int n);
    
    BufferImpl *first() const
      {
        return buf_first;
      }

    int n_records() const
      {
        int nbufs = 0;
        for(const BufferImpl* p = buf_first; p; p = p->nextptr)
            ++nbufs;

        return nbufs;
      }

    void init_seq(Sequence seq_init)
      {
        seq = seq_init;
      }
    
    Sequence end_seq() const
      {
        return seq;
      }
  };

BufferStoreImpl::BufferStoreImpl(BufferStoreImplPartner &partner_init, int bufsize,
  int nbufs): partner(partner_init), buf_first(NULL), seq(0)
  {
    internal_check(nbufs >= 2);
    
    buf_head = new BufferImpl(bufsize);
    buf_head->prevptr = buf_head->nextptr = NULL;
    buf_queue = new BufferImpl(bufsize);
    insert_buffer_after(buf_queue, buf_head);
    buf_free = buf_head;

    for(int i = 2; i < nbufs; ++i)
      {
        BufferImpl* buf = new BufferImpl(bufsize);
        insert_buffer_after(buf, buf_free);
      }
  }
        
BufferStoreImpl::~BufferStoreImpl()
  {
    while(buf_head)
      {
        BufferImpl* buf = buf_head;
        buf_head = buf->nextptr;
        delete buf;
      }
  }

Buffer *BufferStoreImpl::get_buffer(void)
  {
    internal_check(buf_free != NULL);
    
    BufferImpl* buf = buf_free;
    buf_free = buf_free->nextptr;
    if(buf_first == buf) buf_first = buf->nextptr;
    if(buf->seq != Sequence::uninitialized) partner.delete_oldest_buffer(buf);
    buf->seq = Sequence::uninitialized;
    return(buf);
  }

void BufferStoreImpl::queue_buffer(Buffer *buf1)
  {
    BufferImpl *buf = dynamic_cast<BufferImpl *>(buf1);
    internal_check(buf != NULL);

    buf->seq = seq;
    seq.increment();
    if(buf_head == buf) buf_head = buf->nextptr;
    remove_buffer(buf);
    insert_buffer_after(buf, buf_queue);
    buf_queue = buf;
    if(buf_first == NULL) buf_first = buf;
    partner.new_buffer(buf);
  }

void BufferStoreImpl::next_buffer()
  {
    internal_check(buf_free != NULL);
    
    BufferImpl* buf = buf_free;
    buf_free = buf_free->nextptr;
    buf->seq = seq;
    seq.increment();
    if(buf_head == buf) buf_head = buf->nextptr;
    remove_buffer(buf);
    insert_buffer_after(buf, buf_queue);
    buf_queue = buf;
    if(buf_first == NULL) buf_first = buf;
  }

void BufferStoreImpl::load_buffers(int fd)
  {
    internal_check(buf_free != NULL);

    int r;
    while((r = read(fd, buf_free->dataptr, buf_free->size)) == buf_free->size)
        next_buffer();

    internal_check(r == 0);
  }

void BufferStoreImpl::create_blank_buffers(int n)
  {
    internal_check(buf_free != NULL);

    for(int i = 0; i < n; ++i)
      {
        BufferImpl* buf = buf_free;
        memset(buf->dataptr, 0, buf->size);
        next_buffer();
        partner.new_buffer(buf);
      }
  }

//*****************************************************************************
// FileBuffer
//*****************************************************************************

class FileBuffer
  {
  friend class FileStore;
  private:
    FileBuffer *nextptr;
    Sequence seq;
    int writefd, nbuf;

    ~FileBuffer() {}

  public:
    const string name;
    
    FileBuffer(const string &name_init):
      nextptr(NULL), writefd(-1), nbuf(0), name(name_init) {}
    void store(BufferImpl *buf);
    
    FileBuffer *next() const
      {
        return nextptr;
      }

    Sequence sequence() const
      {
        return seq;
      }

    int buffers_stored() const
      {
        return nbuf;
      }
  };
      
void FileBuffer::store(BufferImpl *buf)
  {
    internal_check(writefd >=0);
    if(write(writefd, buf->data(), buf->size) < buf->size)
        throw CannotWriteFile(name);

    if(!nbuf) seq = buf->sequence();
    ++nbuf;
  }

//*****************************************************************************
// FileStore
//*****************************************************************************

class FileStorePartner
  {
  public:
    virtual void delete_oldest_segment(FileBuffer *buf) =0;
    virtual ~FileStorePartner() {};
  };

class FileStore
  {
  private:
    FileStorePartner &partner;
    FileBuffer *buf_head, *buf_tail;
    const int bufsize;
    const string segments_dir;
    const int maxfiles;
    int nfiles;
    FileBuffer *get_file_helper(const string &filename);

  public:
    FileStore(FileStorePartner &partner_init, int bufsize_init,
      const string &segments_dir_init, int maxfiles_init);
    ~FileStore();
    FileBuffer *get_file(LONG seq_long);
    void release_file(FileBuffer *fb);
    LONG restore_state();

    FileBuffer *first()
      {
        return buf_head;
      }

    FileBuffer *last()
      {
        return buf_tail;
      }

    int n_files()
      {
        return nfiles;
      }

    int n_records()
      {
        int nbufs = 0;
        for(FileBuffer* p = buf_head; p; p = p->nextptr)
            nbufs += p->nbuf;
        
        return nbufs;
      }
  };

FileBuffer *FileStore::get_file_helper(const string &filename)
  {
    FileBuffer* p = new FileBuffer(segments_dir + "/" + filename);
    ++nfiles;

    if(buf_head == NULL)
      {
        buf_head = buf_tail = p;
      }
    else
      {
        buf_tail->nextptr = p;
        buf_tail = p;
      }

    while(nfiles > maxfiles)
      {
        internal_check(buf_head != NULL);
        p = buf_head;
        if((buf_head = p->nextptr) == NULL) buf_tail = NULL;
        
        partner.delete_oldest_segment(p);
        if(p->writefd != -1) xclose(p->writefd);

        if(unlink(p->name.c_str()) < 0)
            throw CannotDeleteFile(p->name);

        delete p;
        --nfiles;
      }

    return buf_tail;
  }

FileStore::FileStore(FileStorePartner &partner_init, int bufsize_init,
  const string &segments_dir_init, int maxfiles_init):
  partner(partner_init), buf_head(NULL), buf_tail(NULL), bufsize(bufsize_init), 
  segments_dir(segments_dir_init), maxfiles(maxfiles_init), nfiles(0) {}
  
FileStore::~FileStore()
  {
    while(buf_head)
      {
        FileBuffer* p = buf_head;
        buf_head = p->nextptr;
        if(p->writefd != -1) xclose(p->writefd);
        delete p;
      }
  }

FileBuffer *FileStore::get_file(LONG seq_long)
  {
    if(maxfiles == 0) return NULL;
    
    char tmpbuf[20];
#ifdef LONGWORD_64BIT
    sprintf(tmpbuf, "%016lX", seq_long);
#else
    sprintf(tmpbuf, "%08lX%08lX", long((seq_long >> 32) & 0xffffffff),
      long(seq_long & 0xffffffff));
#endif
    
    FileBuffer* p = get_file_helper(tmpbuf);
    
    if((p->writefd = xcreat(p->name.c_str(), 0644)) < 0)
        throw CannotCreateFile(p->name);
    
    return p;
  }
    
void FileStore::release_file(FileBuffer *fb)
  {
    xclose(fb->writefd);
    fb->writefd = -1;
  }

LONG FileStore::restore_state()
  {
    DIR *dir;
    struct dirent *de;
    LONG seq_long = 0;

    if((dir = opendir(segments_dir.c_str())) == NULL)
        throw CannotOpenDir(segments_dir);

    set<string> segment_files;
    while((de = readdir(dir)) != NULL)
        segment_files.insert(de->d_name);

    closedir(dir);
    
    FileBuffer* p = NULL;

    set<string>::iterator i;
    for(i = segment_files.begin(); i != segment_files.end(); ++i)
      {
#if LONGWORD_64BIT
        if(sscanf(i->c_str(), "%lX", &seq_long) != 1)
            continue;
#else
        unsigned long seq_high, seq_low;
        if(sscanf(i->c_str(), "%8lX%8lX", &seq_high, &seq_low) != 2)
            continue;

        seq_long = ((long long)(seq_high) << 32) | (long long)(seq_low);
        DEBUG_MSG("seq_long = " << seq_long << endl);
#endif

        struct stat st;
        if(stat((segments_dir + "/" + *i).c_str(), &st) < 0)
            throw CannotStatFile(segments_dir + "/" + *i);

        if(st.st_size % bufsize)
            throw BadFileFormat(segments_dir + "/" + *i);
            
        if((p = get_file_helper(*i)) != NULL)
          {
            p->nbuf = st.st_size / bufsize;
            p->seq = seq_long & Sequence::mask;
            seq_long += p->nbuf;
            DEBUG_MSG("p->nbuf = " << p->nbuf << endl);
          }
      }
        
    if(p != NULL && (p->writefd = xopen(p->name.c_str(), O_WRONLY | O_APPEND)) < 0)
        throw CannotOpenFile(p->name);

    return seq_long;
  }
        
//*****************************************************************************
// ConnectionStateBase
//*****************************************************************************

class ConnectionStateBase
  {
  protected:
    Stream logs;
    bool rlog;

    void request_ok(const vector<string> &cmdvec);
    void request_invalid(const vector<string> &cmdvec);
    
    ConnectionStateBase(const Stream &logs_init, bool rlog_init):
      logs(logs_init), rlog(rlog_init) {}
  };
    
void ConnectionStateBase::request_ok(const vector<string> &cmdvec)
  {
    if(!rlog) return;

    vector<string>::const_iterator i = cmdvec.begin();
    string full_request;

    for(const char* p = i->c_str(); *p; ++p)
        full_request += toupper(*p);
   
    while((++i) != cmdvec.end()) full_request += (string(" ") + *i);
    logs(LOG_INFO) << full_request << endl;
  }

void ConnectionStateBase::request_invalid(const vector<string> &cmdvec)
  {
    vector<string>::const_iterator i = cmdvec.begin();
    string full_request = *i;

    while((++i) != cmdvec.end()) full_request += (string(" ") + *i);
    logs(LOG_WARNING) << "invalid request: " << full_request << endl;
  }

//*****************************************************************************
// ConnectionState
//*****************************************************************************

// Variables shared by StationIO and Connction (one ConnectionState object
// is allocated per TCP connection, eg., it contains station-independent
// connection state)

class ConnectionState: private ConnectionStateBase
  {
  friend class StationIO;
  friend class Connection;
  friend class StationConnection;
  private:
    const int clientfd;
    const string host;
    const int port;
    const bool window_extraction;
    string response_str;
    bool multi;
    bool handshaking_done;
    bool have_response;
    int stations_active;
    int stations_ready;
    bool batchmode;

    ConnectionState(const string &host_init, int port_init, bool rlog,
      const Stream &logs, int clientfd_init, bool window_extraction_init):
      ConnectionStateBase(logs, rlog), clientfd(clientfd_init),
      host(host_init), port(port_init),
      window_extraction(window_extraction_init), multi(false),
      handshaking_done(false), have_response(false), stations_active(0),
      stations_ready(0), batchmode(false) {}

    void response(const string &str);
  };

void ConnectionState::response(const string &str)
  {
    if (!batchmode)
      {
        response_str = str;
        have_response = true;
        fds.set_write(clientfd);
      }
    else
      {
        if (str == "ERROR")
          logs(LOG_ERR) << "BATCH mode: Request failed" << endl;
      }
  }

//*****************************************************************************
// StationConnectionState
//*****************************************************************************

// Variables shared by StationIO and StationConnection (one
// StationConnectionState object is allocated per TCP connection/station
// combination, eg., it contains station-dependent connection state)

class StationConnectionState: private ConnectionStateBase
  {
  friend class StationIO;
  friend class StationConnection;
  public:
    const string station_key;
    
  private:
    ConnectionState &cx;
    const rc_ptr<ConnectionMonitor> monitor;
    BufferImpl *buffer_queue;
    FileBuffer *file_queue;
    Sequence seq;
    int bufpos;
    int readfd;
    bool realtime;
    bool discard;
    bool active;
    bool ready;

    void reset_queue();

    StationConnectionState(const string &station_key_init, bool rlog,
      const Stream &logs, ConnectionState &cx_init,
      rc_ptr<ConnectionMonitor> monitor_init):
      ConnectionStateBase(logs, rlog), station_key(station_key_init),
      cx(cx_init), monitor(monitor_init), buffer_queue(NULL), file_queue(NULL),
      seq(Sequence::uninitialized), bufpos(0), readfd(-1),
      realtime(true), discard(false), active(false), ready(false) {}
  };
    
void StationConnectionState::reset_queue()
  {
    if(readfd >= 0) xclose(readfd);
    readfd = -1;
    buffer_queue = NULL;
    file_queue = NULL;
    seq = Sequence::uninitialized;
    bufpos = 0;
//    realtime = true;
    discard = false;
//    active = false;
//    ready = false;
  }

ssize_t writen(int fd, const void *vptr, size_t n)
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

//*****************************************************************************
// StationConnection
//*****************************************************************************

class StationConnectionPartner
  {
  public:
    virtual list<StationConnectionState *>::iterator
      attach(StationConnectionState *sx) =0;
    virtual void detach(list<StationConnectionState *>::iterator &i) =0;
    virtual bool request(StationConnectionState &sx,
      const vector<string> &cmdvec) = 0;
    virtual ~StationConnectionPartner() {}
  };

class StationConnection
  {
  private:
    StationConnectionPartner &partner;
    StationConnectionState sx;
    list<StationConnectionState *>::iterator station_link;
    enum DeliveryResult { Ok, Retry, Fail };
  
    ssize_t write_zeros(int fd, int count);
    DeliveryResult do_deliver();

  public:
    StationConnection(StationConnectionPartner &partner_init,
      const string &station_key, bool rlog, ConnectionState &cx,
      rc_ptr<ConnectionMonitor> monitor, Stream &logs):
      partner(partner_init), sx(station_key, rlog, logs, cx, monitor) 
      {
        station_link = partner.attach(&sx);
      }

    ~StationConnection()
      {
        partner.detach(station_link);
      }

    bool deliver();
    
    void reset()
      {
        sx.reset_queue();
      }

    string station_key()
      {
        return sx.station_key;
      }
    
    bool ready()
      {
        return sx.ready;
      }
    
    bool request(const vector<string> &cmdvec)
      {
        return partner.request(sx, cmdvec);
      }
  };

ssize_t StationConnection::write_zeros(int fd, int count)
  {
    static char zero_buf[IOSIZE];

    internal_check(count <= IOSIZE);
    return writen(fd, zero_buf, count);
  }

StationConnection::DeliveryResult StationConnection::do_deliver()
  {
    char input_buf[IOSIZE];
    void *dataptr;

    int size = min(IOSIZE - Sequence::size - 2, (1 << MSEED_RECLEN) - sx.bufpos);
    
    if(sx.bufpos == 0) sx.discard = false;
    
    if(sx.discard)
      {
        if(write_zeros(sx.cx.clientfd, size) <= 0) return Fail;
        sx.bufpos = (sx.bufpos + size) % (1 << MSEED_RECLEN);
        return Ok;
      }
    
    if(sx.monitor->end_of_data())
      {
        sx.buffer_queue = NULL;
        sx.file_queue = NULL;
        if(sx.readfd >= 0) xclose(sx.readfd);
        sx.readfd = -1;
      }

    if(sx.buffer_queue != NULL && sx.file_queue != NULL &&
      sx.seq == sx.buffer_queue->sequence())
      {
        DEBUG_MSG(sx.station_key << " : " << "switching to memory buffer, "
          "seq = " << sx.seq << endl);
        sx.file_queue = NULL;
        if(sx.readfd >= 0) xclose(sx.readfd);
        sx.readfd = -1;
      }

    if(sx.file_queue != NULL)
      {
        if(sx.readfd < 0)
          {
            if((sx.readfd = xopen(sx.file_queue->name.c_str(), O_RDONLY)) < 0)
                throw CannotOpenFile(sx.file_queue->name);
            sx.seq = sx.file_queue->sequence();
            DEBUG_MSG(sx.station_key << " : " << "opening new file segment, "
              "seq = " << sx.seq << endl);
          }
          
        int r = read(sx.readfd, input_buf, size);
        
        if(r == 0)
          {
            xclose(sx.readfd);
            sx.readfd = -1;
            sx.file_queue = sx.file_queue->next();
            if(sx.file_queue == NULL)
              {
                logs(LOG_ERR) << sx.station_key << " : " << "end of file "
                  "segments, seq = " << sx.seq << endl;
                if(sx.buffer_queue == NULL)
                    logs(LOG_ERR) << sx.station_key << " : " << "no buffers "
                      "in queue" << endl;
                else
                    logs(LOG_ERR) << sx.station_key << " : " << "first buffer "
                      "in queue has seq = " << sx.buffer_queue->sequence() <<
                      endl;
              }
            return Retry;
          }
        else if(r < 0)
          {
            throw CannotReadFile(sx.file_queue->name);
          }
        else if(r != size)
          {
            throw BadFileFormat(sx.file_queue->name);
          }
          
        dataptr = input_buf;
      }
    else if(sx.buffer_queue != NULL)
      {
        dataptr = (char *) sx.buffer_queue->data() + sx.bufpos;
        sx.seq = sx.buffer_queue->sequence();
      }
    else
      {
        internal_check(sx.active);
        internal_check(sx.ready);
        sx.ready = false;
        --sx.cx.stations_ready;

        if(sx.monitor->end_of_data())
          {
            sx.logs(LOG_INFO) << "end of time window" << endl;
            sx.active = false;
            --sx.cx.stations_active;
          }
        else if(!sx.realtime)
          {
            sx.logs(LOG_INFO) << "end of data" << endl;
            sx.monitor->set_eod(true);
            sx.active = false;
            --sx.cx.stations_active;
          }
        
        if(sx.cx.stations_active == 0)
            writen(sx.cx.clientfd, "END", 3);

        return Ok;
      }
        
    if(sx.bufpos == 0)
      {
        sx.monitor->check_seq(sx.seq);
        if(sx.monitor->match_packet(dataptr, MAX_HEADER_LEN))
          {
            sx.monitor->count_packet();
            
            ostringstream sseqstr;
            sseqstr << SIGNATURE << sx.seq;
        
            string seqstr = sseqstr.str();
            if(writen(sx.cx.clientfd, seqstr.c_str(), seqstr.length()) <= 0)
                return Fail;
          }
        else
          {
            sx.seq.increment();
            if(sx.file_queue == NULL)
                sx.buffer_queue = sx.buffer_queue->next();
            else
                lseek(sx.readfd, (1 << MSEED_RECLEN) - size, SEEK_CUR);

            return Retry;
          }
      }

    if(writen(sx.cx.clientfd, dataptr, size) <= 0) return Fail;
    fds.clear_write2(sx.cx.clientfd);

    sx.bufpos = (sx.bufpos + size) % (1 << MSEED_RECLEN);
    
    if(sx.bufpos == 0)
      {
        sx.seq.increment();
        if(sx.file_queue == NULL)
          {
            internal_check(sx.buffer_queue != NULL);
            sx.buffer_queue = sx.buffer_queue->next();
          }
      }

    return Ok;
  }
        
bool StationConnection::deliver()
  {
    int r;
    
    while((r = do_deliver()) == Retry);
    return (r == Fail);
  }

//*****************************************************************************
// StationIO
//*****************************************************************************

// There is exactly one StationIO object in the IOSystem per station, so it 
// contains data that does not depend on a particular connection. However,
// StationIO has access to the conection-dependent data via ConnectionState
// and StationConnectionState objects (see above).

class StationIO: private BufferStoreImplPartner, private FileStorePartner,
  private StationConnectionPartner
  {
  private:
    const string ident;
    const bool rlog;
    const string station_dir;
    const int nbufs;
    const int blank_bufs;
    const int filesize;
    const int seq_gap_limit;
    const bool load_headers;
    const rc_ptr<StationMonitor> monitor;
    rc_ptr<BufferStoreImpl> bufs;
    rc_ptr<FileStore> fils;
    FileBuffer *curfb;
    LONG seq_long;
    list<StationConnectionState *> attached;
    
    // BufferStoreImpl callbacks
    void new_buffer(BufferImpl *buf);
    void delete_oldest_buffer(BufferImpl *buf);

    // FileStore callback
    void delete_oldest_segment(FileBuffer *buf);

    // StationConnection callbacks
    list<StationConnectionState *>::iterator
      attach(StationConnectionState *sx);
    void detach(list<StationConnectionState *>::iterator &i);
    bool request(StationConnectionState &sx,
      const vector<string> &cmdvec);
    
    bool command_SELECT(StationConnectionState &sx,
      const vector<string> &cmdvec);
    bool command_TIME(StationConnectionState &sx,
      const vector<string> &cmdvec);
    bool command_DATA(StationConnectionState &sx,
      const vector<string> &cmdvec);

    void do_load_headers();

    void find_oldest_packet(StationConnectionState &sx);
    bool find_packet_in_bufferstore(StationConnectionState &sx, Sequence n);
    bool find_packet_in_filestore(StationConnectionState &sx, Sequence n);
    bool find_packet(StationConnectionState &sx, Sequence n);
    
  public:
    const string station_key;
    
    StationIO(const string &station_key_init, const string &ident_init,
      bool rlog_init, const string &station_dir_init, int nbufs_init,
      int blank_bufs_init, int filesize_init, int nfiles,
      int seq_gap_limit_init, bool load_headers_init,
      rc_ptr<StationMonitor> monitor_init);
    rc_ptr<StationConnection> connection_instance(ConnectionState &cx);
    void save_state();
    void restore_state();
    
    bool ipaccess(unsigned int ipaddr)
      {
        return monitor->ipaccess(ipaddr);
      }
    
    rc_ptr<BufferStore> get_bufs()
      {
        return bufs;
      }
  };

// Callback from BufferStoreImpl (through BufferStoreImplPartner) to signal
// that a new buffer (eg., Mini-SEED record) has arrived.

void StationIO::new_buffer(BufferImpl *buf)
  {
    if(curfb == NULL)
      {
        curfb = fils->get_file(seq_long);
      }
    else if(curfb->buffers_stored() >= filesize)
      {
        fils->release_file(curfb);
        curfb = fils->get_file(seq_long);
        monitor->new_segment();
      }
      
    ++seq_long;
    internal_check(curfb != NULL);
    curfb->store(buf);

    monitor->add_packet(buf->sequence(), buf->data(), MAX_HEADER_LEN);
    monitor->set_end_seq(buf->sequence() + 1);
    
    list<StationConnectionState *>::iterator i;
    for(i = attached.begin(); i != attached.end(); ++i)
      {
        if(!(*i)->active) continue;
        
        if((*i)->buffer_queue == NULL)
          {
            if((*i)->monitor->match_packet(buf->data(), MAX_HEADER_LEN))
              {
                DEBUG_MSG(station_key << " : " << "first buffer in queue, "
                  "seq = " << buf->sequence() << endl);

                (*i)->buffer_queue = buf;
              }
            else
              {
                (*i)->monitor->check_seq(buf->sequence());
                continue;
              }
          }

        if((*i)->cx.handshaking_done)
          {
            if(!(*i)->ready)
              {
                (*i)->ready = true;
                ++(*i)->cx.stations_ready;
              }

            fds.set_write2((*i)->cx.clientfd);
          }
      }
  }

// Callback from BufferStoreImpl (through BufferStoreImplPartner) to signal
// that the oldest record in BufferStore ("memory buffer") will be deleted.

void StationIO::delete_oldest_buffer(BufferImpl *buf)
  {
    list<StationConnectionState *>::iterator i;
    for(i = attached.begin(); i != attached.end(); ++i)
      {
        if((*i)->buffer_queue == buf)
          {
            if((*i)->file_queue == NULL)
              {
                DEBUG_MSG(station_key << " : " << "deleting active buffer, "
                  "seq = " << buf->sequence() << endl);

                internal_check((*i)->readfd < 0);

                if(!find_packet_in_filestore(*(*i), buf->sequence()))
                  {
                    (*i)->discard = true;
                    (*i)->seq.increment();
                  }
              }

            (*i)->buffer_queue = buf->next();
          }
      }
  }

// Callback from FileStore (through FileStorePartner) to signal that the
// oldest file segment in FileStore ("disk buffer") will be deleted.

void StationIO::delete_oldest_segment(FileBuffer *fb)
  {
    monitor->delete_oldest_segment();
    monitor->set_begin_seq(fb->next()->sequence());

    list<StationConnectionState *>::iterator i;
    for(i = attached.begin(); i != attached.end(); ++i)
      {
        if((*i)->file_queue == fb)
          {
            DEBUG_MSG(station_key << " : " << "deleting active file segment, "
              "seq = " << (*i)->seq << endl);

            if((*i)->readfd >= 0) xclose((*i)->readfd);
            (*i)->readfd = -1;
            (*i)->file_queue = fb->next();
            (*i)->discard = true;
          }
      }
  }
    
list<StationConnectionState *>::iterator
StationIO::attach(StationConnectionState *st)
  {
    return attached.insert(attached.end(), st);
  }

void StationIO::detach(list<StationConnectionState *>::iterator &ptr)
  {
    attached.erase(ptr);
  }

bool StationIO::command_SELECT(StationConnectionState &sx,
  const vector<string> &cmdvec)
  {
    if(sx.active)
      {
        --sx.cx.stations_active;
        sx.active = false;
      }
    
    if(sx.ready)
      {
        --sx.cx.stations_ready;
        sx.ready = false;
      }

    sx.reset_queue();
    sx.monitor->reset();

    if(cmdvec.size() == 1)
      {
        sx.monitor->clear_selectors();
        sx.request_ok(cmdvec);
        sx.cx.response("OK\r\n");
        return true;
      }
    else if(cmdvec.size() == 2 && sx.monitor->add_selector(cmdvec[1]))
      {
        sx.request_ok(cmdvec);
        sx.cx.response("OK\r\n");
        return true;
      }

    return false;
  }
        
bool StationIO::command_TIME(StationConnectionState &sx,
  const vector<string> &cmdvec)
  {
    if(sx.active)
      {
        --sx.cx.stations_active;
        sx.active = false;
      }
    
    if(sx.ready)
      {
        --sx.cx.stations_ready;
        sx.ready = false;
      }

    sx.reset_queue();
    sx.monitor->reset();

    if((cmdvec.size() != 2 && cmdvec.size() != 3) || !sx.cx.window_extraction)
        return false;
    
    if(cmdvec.size() >= 2)
      {
        int year, month, day, hour, min, sec;
        char c;
        
        if(sscanf(cmdvec[1].c_str(), "%d,%d,%d,%d,%d,%d%c", &year, &month,
          &day, &hour, &min, &sec, &c) != 6)
            return false;

        if(!sx.monitor->set_begin_time(year, month, day, hour, min, sec, 0, -1))
            return false;
      }

    if(cmdvec.size() == 3)
      {
        int year, month, day, hour, min, sec;
        char c;
        
        if(sscanf(cmdvec[2].c_str(), "%d,%d,%d,%d,%d,%d%c", &year, &month,
          &day, &hour, &min, &sec, &c) != 6)
            return false;

        if(!sx.monitor->set_end_time(year, month, day, hour, min, sec, 0))
            return false;
      }

    if(!sx.monitor->time_valid())
        return false;

    sx.request_ok(cmdvec);
    sx.realtime = true; // wait until all data in the requested time window
                        // is available

    if(!sx.monitor->end_of_data() && sx.monitor->get_begin_seq() != -1)
        find_packet(sx, sx.monitor->get_begin_seq());

    sx.monitor->set_realtime(sx.realtime);

    internal_check(!sx.active);
    sx.active = true;
    ++sx.cx.stations_active;

    internal_check(!sx.ready);
    sx.ready = true;
    ++sx.cx.stations_ready;

    if(sx.cx.multi)
      {
        sx.cx.response("OK\r\n");
      }
    else
      {
        sx.cx.handshaking_done = true;
        fds.set_write2(sx.cx.clientfd);
      }

    return true;
  }

bool StationIO::command_DATA(StationConnectionState &sx,
  const vector<string> &cmdvec)
  {
    if(sx.active)
      {
        --sx.cx.stations_active;
        sx.active = false;
      }
    
    if(sx.ready)
      {
        --sx.cx.stations_ready;
        sx.ready = false;
      }

    sx.reset_queue();
    sx.monitor->reset();

    if(cmdvec.size() == 1)
      {
        sx.request_ok(cmdvec);
        
        if(sx.cx.multi) sx.cx.response("OK\r\n");
        else sx.cx.handshaking_done = true;
        
        sx.seq = bufs->end_seq();
        sx.monitor->set_begin_seq(sx.seq, true);
        sx.monitor->set_realtime(sx.realtime);

        internal_check(!sx.active);
        sx.active = true;
        ++sx.cx.stations_active;

        return true;
      }
    else if(cmdvec.size() == 2)
      {
        if(!strcasecmp(cmdvec[1].c_str(), "ALL"))
          {
            sx.request_ok(cmdvec);
            find_oldest_packet(sx);
            sx.monitor->set_begin_seq(sx.seq, true);

            internal_check(!sx.ready);
            sx.ready = true;
            ++sx.cx.stations_ready;
          }
        else
          {
            int n;
            char *tail;
    
            n = strtoul(cmdvec[1].c_str(), &tail, 16);
            if((n & ~Sequence::mask) || *tail)
                return false;
        
            sx.request_ok(cmdvec);
            bool valid = find_packet(sx, n);
            sx.monitor->set_begin_seq(n, valid);

            if(valid)
              {
                internal_check(!sx.ready);
                sx.ready = true;
                ++sx.cx.stations_ready;
              }
          }

        sx.monitor->set_realtime(sx.realtime);
        
        internal_check(!sx.active);
        sx.active = true;
        ++sx.cx.stations_active;

        if(sx.cx.multi)
          {
            sx.cx.response("OK\r\n");
          }
        else
          {
            sx.cx.handshaking_done = true;
            fds.set_write2(sx.cx.clientfd);
          }

        return true;
      }
    else if(cmdvec.size() == 3 && sx.cx.window_extraction)
      {
        int n;
        char *tail;
    
        n = strtoul(cmdvec[1].c_str(), &tail, 16);
        if((n & ~Sequence::mask) || *tail)
            return false;
      
        int year, month, day, hour, min, sec;
        char c;
        
        if(sscanf(cmdvec[2].c_str(), "%d,%d,%d,%d,%d,%d%c", &year, &month,
          &day, &hour, &min, &sec, &c) != 6)
            return false;

        if(!sx.monitor->set_begin_time(year, month, day, hour, min, sec, 0, n))
            return false;
        
        sx.request_ok(cmdvec);

        if(sx.monitor->get_begin_seq() != -1)
            find_packet(sx, sx.monitor->get_begin_seq());

        sx.monitor->set_realtime(sx.realtime);

        internal_check(!sx.active);
        sx.active = true;
        ++sx.cx.stations_active;

        internal_check(!sx.ready);
        sx.ready = true;
        ++sx.cx.stations_ready;
        
        if(sx.cx.multi)
          {
            sx.cx.response("OK\r\n");
          }
        else
          {
            sx.cx.handshaking_done = true;
            fds.set_write2(sx.cx.clientfd);
          }

        return true;
      }

    return false;
  }

bool StationIO::request(StationConnectionState &sx,
  const vector<string> &cmdvec)
  {
    if(!strcasecmp(cmdvec[0].c_str(), "HELLO"))
      {
        if(cmdvec.size() == 1)
          {
            sx.request_ok(cmdvec);
            sx.cx.response(ident);
            return false;
          }
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "SELECT"))
      {
        if(command_SELECT(sx, cmdvec)) return false;
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "TIME"))
      {
        if(command_TIME(sx, cmdvec))
            return false;

        sx.monitor->reset();
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "FETCH"))
      {
        sx.realtime = false;
        if(command_DATA(sx, cmdvec))
            return false;

        sx.monitor->reset();
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "DATA"))
      {
        sx.realtime = true;
        if(command_DATA(sx, cmdvec))
            return false;

        sx.monitor->reset();
      }

    sx.request_invalid(cmdvec);
    sx.cx.response("ERROR\r\n");
    return false;
  }
            
bool StationIO::find_packet_in_filestore(StationConnectionState &sx, Sequence seq)
  {
    for(FileBuffer* p = fils->first(); p != NULL; p = p->next())
      {
        if(p->sequence() != Sequence::uninitialized &&
          seq - p->sequence() < p->buffers_stored())
          {
            sx.buffer_queue = bufs->first();
            sx.file_queue = p;
            sx.seq = seq;
            if((sx.readfd = xopen(p->name.c_str(), O_RDONLY)) < 0)
                throw CannotOpenFile(p->name);
            lseek(sx.readfd, (seq - p->sequence()) * (1 << MSEED_RECLEN), SEEK_SET);
            return true;
          }
      }

    return false;
  }

bool StationIO::find_packet_in_bufferstore(StationConnectionState &sx, Sequence seq)
  {
    for(BufferImpl* p = bufs->first(); p != NULL; p = p->next())
      {
        if(p->sequence() == seq)
          {
            sx.buffer_queue = p;
            sx.file_queue = NULL;
            sx.seq = seq;
            return true;
          }
      }

    return false;
  }

void StationIO::find_oldest_packet(StationConnectionState &sx)
  {
    sx.buffer_queue = bufs->first();
    sx.file_queue = fils->first();
    sx.seq = Sequence::uninitialized;

    if(sx.file_queue != NULL)
      {
        sx.seq = sx.file_queue->sequence();
        find_packet_in_bufferstore(sx, sx.seq);
      }
    
    DEBUG_MSG(station_key << " : " << "sx.buffer_queue = " << sx.buffer_queue << endl);
    DEBUG_MSG(station_key << " : " << "sx.file_queue = " << sx.file_queue << endl);
    
    if(sx.seq == Sequence::uninitialized)
      {
        sx.logs(LOG_INFO) << "no packets in queue" << endl;
        sx.seq = 0;
      }
    else
      {
        sx.logs(LOG_INFO) << "oldest packet " << sx.seq << endl;
      }
  }

bool StationIO::find_packet(StationConnectionState &sx, Sequence seq)
  {
    if(find_packet_in_bufferstore(sx, seq))
        return true;

    if(find_packet_in_filestore(sx, seq))
        return true;
    
    if(seq == bufs->end_seq())
      {
        sx.seq = seq;
        return true;
      }
    
    sx.logs(LOG_WARNING) << "packet "  << seq << " not found" << endl;
    find_oldest_packet(sx);

    if(sx.seq - seq > seq_gap_limit)
      {
        sx.buffer_queue = NULL;
        sx.file_queue = NULL;
        sx.seq = bufs->end_seq();
        sx.logs(LOG_WARNING) << "sequence difference " << int(sx.seq - seq) << 
          " is too large" << endl;
        sx.logs(LOG_WARNING) << "resuming transmission from the next packet"
          " (" << sx.seq << ")" << endl;
        return false;
      }

    return true;
  }
 
StationIO::StationIO(const string &station_key_init, const string &ident_init,
  bool rlog_init, const string &station_dir_init, int nbufs_init,
  int blank_bufs_init, int filesize_init, int nfiles, int seq_gap_limit_init,
  bool load_headers_init, rc_ptr<StationMonitor> monitor_init):
  ident(ident_init), rlog(rlog_init), station_dir(station_dir_init),
  nbufs(nbufs_init), blank_bufs(blank_bufs_init), filesize(filesize_init),
  seq_gap_limit(seq_gap_limit_init), load_headers(load_headers_init),
  monitor(monitor_init), station_key(station_key_init)
  {
    bufs = new BufferStoreImpl(*this, (1 << MSEED_RECLEN), nbufs);
    fils = new FileStore(*this, (1 << MSEED_RECLEN), station_dir + "/segments", nfiles);
  }

rc_ptr<StationConnection> StationIO::connection_instance(ConnectionState &cx)
  {
    Stream station_logs = cx.logs.stream(station_key + " : ");
    return new StationConnection(*this, station_key, rlog, cx,
      monitor->add_connection(cx.host, cx.port), station_logs);
  }

void StationIO::save_state()
  {
    const string buffer_file = station_dir + "/buffer.xml";
    
    logs(LOG_INFO) << "saving disk buffer description to '" << buffer_file <<
      "'" << endl;

    monitor->save_state(buffer_file);
  }

void StationIO::do_load_headers()
  {
    char header_buf[MAX_HEADER_LEN];
    
    for(FileBuffer* p = fils->first(); p != NULL; p = p->next())
      {
        int fd;
        if((fd = xopen(p->name.c_str(), O_RDONLY)) < 0)
            throw CannotOpenFile(p->name);

        int seq = p->sequence();
        for(int i = 0; i < p->buffers_stored(); ++i)
          {
            lseek(fd, i * (1 << MSEED_RECLEN), SEEK_SET);
            int r = read(fd, header_buf, MAX_HEADER_LEN);

            if(r < 0)
                throw CannotReadFile(p->name);

            if(r != MAX_HEADER_LEN)
                throw BadFileFormat(p->name);

            monitor->add_packet(seq, header_buf, MAX_HEADER_LEN);
            monitor->set_end_seq(seq + 1);
            ++seq;
          }

        xclose(fd);

        if(p->next() != NULL) monitor->new_segment();
      }
  }

void StationIO::restore_state()
  {
    const string segments_dir = station_dir + "/segments";
    const string buffer_file = station_dir + "/buffer.xml";
    
    logs(LOG_INFO) << "trying to read disk buffer segments from '" <<
      segments_dir << "/'..." << endl;

    seq_long = fils->restore_state();
    curfb = fils->last();
    
    logs(LOG_INFO) << "..." << fils->n_records() << " records in " <<
      fils->n_files() << " files" << endl;

    bool recover = false;
    if(access(buffer_file.c_str(), F_OK) == 0)
      {
        logs(LOG_INFO) << "reading disk buffer description from '" <<
          buffer_file << "'" << endl;

        try
          {
            monitor->restore_state(buffer_file);
          }
        catch(MonitorConfigError &e)
          {
            logs(LOG_ERR) << e.message << endl;
            monitor->reset();
            recover = true;
          }
        catch(CfgError &e)
          {
            logs(LOG_ERR) << e.message << endl;
            monitor->reset();
            recover = true;
          }

        if(unlink(buffer_file.c_str()) < 0)
            throw CannotDeleteFile(buffer_file);
      }
    else if(seq_long != 0)
      {
        logs(LOG_INFO) << "cannot find '" << buffer_file << "'" << endl;
        recover = true;
      }

    if(recover && load_headers)
      {
        logs(LOG_INFO) << "scanning disk buffer of station " << 
          station_key << endl;
        do_load_headers();
      }
    
    logs(LOG_INFO) << "trying to load up to " << nbufs << " most recent "
      "records into memory..." << endl;
    
    Sequence seq = (seq_long - nbufs) & Sequence::mask;

    int fd = -1;
    FileBuffer* p;
    for(p = fils->first(); p != NULL; p = p->next())
      {
        if(p->sequence() != Sequence::uninitialized &&
          seq - p->sequence() < p->buffers_stored())
          {
            if((fd = xopen(p->name.c_str(), O_RDONLY)) < 0)
                throw CannotOpenFile(p->name);
            lseek(fd, (seq - p->sequence()) * (1 << MSEED_RECLEN), SEEK_SET);
            break;
          }
      }

    if(p == NULL)
      {
        if((p = fils->first()) == NULL) seq = 0;
        else seq = p->sequence();
      }

    bufs->init_seq(seq);

    for(; p != NULL; p = p->next())
      {
        if(fd < 0 && (fd = xopen(p->name.c_str(), O_RDONLY)) < 0)
            throw CannotOpenFile(p->name);

        bufs->load_buffers(fd);
        xclose(fd);
        fd = -1;
      }

    logs(LOG_INFO) << "..." << int(bufs->end_seq() - seq) <<
      " records loaded" << endl;
    
    if(!recover) return;

    logs(LOG_INFO) << "creating " << blank_bufs << " empty records" << endl;

    bufs->create_blank_buffers(blank_bufs);

    if(fils->first() != NULL)
        monitor->set_begin_seq(fils->first()->sequence());

    monitor->set_end_seq(bufs->end_seq());
  }

//*****************************************************************************
// Connection
//*****************************************************************************

class Connection
  {
  private:
    ConnectionState cx;
    const unsigned int ipaddr;
    const string default_network_id;
    const int info_level;
    const rc_ptr<MasterMonitor> monitor;
    map<StationDescriptor, rc_ptr<StationIO> > &stations;
    map<string, rc_ptr<StationConnection> > requested_stations;
    map<string, rc_ptr<StationConnection> >::iterator current_station;
    char cmdbuf[CMDLEN + 1];
    int cmdwp;
    bool invalid;
    list<rc_ptr<MessageBuffer> > msg_bufs;
    list<rc_ptr<InfoBuffer> > info_bufs;

    bool parse_request(const char *cmdstr);
    bool input();
    bool request(const vector<string> &cmdvec);
    bool deliver();

    bool command_STATION(const vector<string> &cmdvec);
    bool command_CAT(const vector<string> &cmdvec);
    bool command_INFO(const vector<string> &cmdvec);
    bool command_BATCH(const vector<string> &cmdvec);

  public:
    Connection(unsigned int ipaddr_init, const string &host, int port,
      rc_ptr<StationIO> default_station, const string &default_network_id_init,
      bool rlog, int info_level_init, bool window_extraction,
      rc_ptr<MasterMonitor> monitor_init,
      map<StationDescriptor, rc_ptr<StationIO> > &stations_init,
      const Stream &logs, int fd):
      cx(host, port, rlog, logs, fd, window_extraction), ipaddr(ipaddr_init),
      default_network_id(default_network_id_init), info_level(info_level_init),
      monitor(monitor_init), stations(stations_init), cmdwp(0), invalid(false)
      {
        current_station = requested_stations.insert(make_pair(default_station->station_key,
          default_station->connection_instance(cx))).first;
      }
      
    ~Connection()
      {
        map<string, rc_ptr<StationConnection> >::iterator i;
        for(i = requested_stations.begin(); i != requested_stations.end(); ++i)
            i->second->reset();
      }

    unsigned int ip() const
      {
        return ipaddr;
      }

    string host() const
      {
        return cx.host;
      }
    
    bool process();
    void disconnect();
  };

bool Connection::parse_request(const char *cmdstr)
  {
    vector<string> cmdvec;
    const char *p = cmdstr;
    int arglen = 0;

    while(p += arglen, p += strspn(p, " "), arglen = strcspn(p, " "))
        cmdvec.push_back(string(p, arglen));

    if(cmdvec.size() > 0) return request(cmdvec);
    return false;
  }

bool Connection::input()
  {
    int bytes_read;
    if((bytes_read = read(cx.clientfd, &cmdbuf[cmdwp], CMDLEN - cmdwp)) <= 0)
        return true;

    for(int i = 0; i < bytes_read; ++i, ++cmdwp)
        if(cmdbuf[cmdwp] == 0) break;

    cmdbuf[cmdwp] = 0;

    bool disconnect = false;
    int cmdrp = 0, cmdlen, seplen;
    while(!disconnect && (cmdlen = strcspn(cmdbuf + cmdrp, "\r\n"),
      seplen = strspn(cmdbuf + cmdrp + cmdlen, "\r\n")))
      {
        cmdbuf[cmdrp + cmdlen] = 0;
        disconnect = parse_request(cmdbuf + cmdrp);
        cmdrp += (cmdlen + seplen);
      }
    
    if(cmdlen >= CMDLEN)
      {
        logs(LOG_WARNING) << "command buffer overflow" << endl;
        cmdwp = cmdrp = 0;
        return disconnect;
      }
        
    memmove(cmdbuf, cmdbuf + cmdrp, cmdlen);
    cmdwp -= cmdrp;
    cmdrp = 0;

    return disconnect;
  }

bool Connection::command_STATION(const vector<string> &cmdvec)
  {
    StationDescriptor sd;
    
    if(cmdvec.size() == 2)
        sd = StationDescriptor(default_network_id, cmdvec[1]);
    else if(cmdvec.size() == 3)
        sd = StationDescriptor(cmdvec[2], cmdvec[1]);
    else
        return false;

    cx.request_ok(cmdvec);
    map<StationDescriptor, rc_ptr<StationIO> >::iterator i_all;
    if((i_all = stations.find(sd)) == stations.end())
      {
        cx.logs(LOG_WARNING) << "no such station" << endl;
        invalid = true;
        return false;
      }

    if(!i_all->second->ipaccess(ipaddr))
      {
        cx.logs(LOG_WARNING) << "access violation" << endl;
        invalid = true;
        return false;
      }

    invalid = false;
    cx.response("OK\r\n");
    
    if(!cx.multi)
      {
        DEBUG_MSG("deleting default station " << current_station->second->station_key() << " (ready = " << current_station->second->ready() << ")" << endl);
        if(current_station->second->ready()) --cx.stations_ready;
        requested_stations.erase(current_station);
      }
    
    map<string, rc_ptr<StationConnection> >::iterator i_requested;
    if((i_requested = requested_stations.find(i_all->second->station_key)) !=
      requested_stations.end())
      {
        cx.logs(LOG_NOTICE) << "station already requested" << endl;
      }
    else
      {
        i_requested = requested_stations.insert(make_pair(i_all->second->station_key,
          i_all->second->connection_instance(cx))).first;
      }

    current_station = i_requested;
    cx.multi = true;
    DEBUG_MSG("current station: " << current_station->second->station_key() << endl);

    return true;
  }

bool Connection::command_CAT(const vector<string> &cmdvec)
  {
    if(cmdvec.size() != 1) return false;
    
    monitor->cat_out(msg_bufs, ipaddr);
    cx.request_ok(cmdvec);
    fds.set_write2(cx.clientfd);
    return true;
  }

bool Connection::command_INFO(const vector<string> &cmdvec)
  {
    if(cmdvec.size() != 2 || info_level == -1) return false;

    int level;
    for(level = 0; level < N_InfoLevel; ++level)
        if(!strcasecmp(cmdvec[1].c_str(), InfoLevelNames[level])) break;

    if(level > info_level) monitor->error_out(info_bufs);
    else monitor->info_out(info_bufs, level, ipaddr);
    cx.request_ok(cmdvec);
    fds.set_write2(cx.clientfd);
    return true;
  }

bool Connection::command_BATCH(const vector<string> &cmdvec)
  {
    if(cmdvec.size() != 1) return false;

    cx.request_ok(cmdvec);
    cx.response("OK\r\n");
    cx.batchmode = true;

    return true;
  }

bool Connection::request(const vector<string> &cmdvec)
  {
    msg_bufs.clear();
    info_bufs.clear();
    
    if(!strcasecmp(cmdvec[0].c_str(), "BYE"))
      {
        if(cmdvec.size() == 1)
          {
            cx.request_ok(cmdvec);
            return true;
          }
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "END"))
      {
        if(cmdvec.size() == 1)
          {
            cx.request_ok(cmdvec);
            cx.handshaking_done = true;
            cx.batchmode = false;
            fds.set_write2(cx.clientfd);
            return false;
          }
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "STATION"))
      {
        if(command_STATION(cmdvec)) return false;
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "CAT"))
      {
        if(command_CAT(cmdvec)) return false;
      }
    else if(!strcasecmp(cmdvec[0].c_str(), "INFO"))
      {
        if(command_INFO(cmdvec)) return false;
      }
    else if (!strcasecmp(cmdvec[0].c_str(), "BATCH"))
      {
        if (command_BATCH(cmdvec)) return false;
      }
    else
      {
        if(!invalid)
            return current_station->second->request(cmdvec);
      }

    cx.request_invalid(cmdvec);
    cx.response("ERROR\r\n");
    return false;
  }

bool Connection::deliver()
  {
    if(cx.have_response)
      {
        if(writen(cx.clientfd, cx.response_str.c_str(),
          cx.response_str.length()) <= 0)
            return true;
        
//        map<StationDescriptor, rc_ptr<StationConnection> >::iterator i;
//        for(i = requested_stations.begin(); i != requested_stations.end(); ++i)
//            i->second->reset();

        cx.handshaking_done = false;
        cx.have_response = false;
        return false;
      }
    
    // Assume that (1 << MSEED_RECLEN) < IOSIZE (which is in fact the case),
    // then we don't have to split the packets.
    
    if(!msg_bufs.empty())
      {
        rc_ptr<MessageBuffer> buf = msg_bufs.front();
        msg_bufs.pop_front();
    
        if(writen(cx.clientfd, buf->data(), buf->size()) <= 0)
            return true;

        if(msg_bufs.empty() && writen(cx.clientfd, "END", 3) <= 0)
            return true;
        
        return false;
      }
            
    if(!info_bufs.empty())
      {
        rc_ptr<InfoBuffer> buf = info_bufs.front();
        info_bufs.pop_front();
    
        ostringstream sseqstr;
        sseqstr << SIGNATURE << "INFO " << (info_bufs.empty() ? " ": "*");
    
        string seqstr = sseqstr.str();
        if(writen(cx.clientfd, seqstr.c_str(), seqstr.length()) <= 0)
            return true;

        if(writen(cx.clientfd, buf->data(), (1 << MSEED_RECLEN)) <= 0)
            return true;

        return false;
      }
            
    if(!cx.handshaking_done || cx.stations_ready == 0)
      {
        fds.clear_write(cx.clientfd);
        return false;
      }
    
    int loop = 0;
    bool retval = false;
    
    do
      {
        DEBUG_MSG("loop = " << loop << endl);
        internal_check(++loop < 10000);
        if((++current_station) == requested_stations.end())
            current_station = requested_stations.begin();
        
        if(current_station->second->ready() &&
          (retval = current_station->second->deliver())) break;
      }
    while(!current_station->second->ready() && cx.stations_ready > 0);

    DEBUG_MSG("current station: " << current_station->first << endl);
    return retval;
  }

bool Connection::process()
  {
    errno = 0;
    if(fds.isactive_read(cx.clientfd) && input()) return true;
    if(fds.isactive_write(cx.clientfd) && deliver()) return true;
    return false;
  }
    
void Connection::disconnect()
  {
    if(errno != 0 && errno != EPIPE)
        cx.logs(LOG_NOTICE) << "socket error: " << strerror(errno) << endl;
    
    cx.logs(LOG_NOTICE) << "closing connection" << endl;
    
    fds.clear_read(cx.clientfd);
    fds.clear_write(cx.clientfd);
    shutdown(cx.clientfd, SHUT_RDWR);
    close(cx.clientfd);
  }
    
//*****************************************************************************
// ConnectionManagerImpl
//*****************************************************************************

class ConnectionManagerImpl: public ConnectionManager
  {
  private:
    const string daemon_name;
    const string software_ident;
    const string default_network_id;
    const bool rlog;
    const int max_conn;
    const int max_conn_per_ip;
    int trusted_info_level;
    int untrusted_info_level;
    bool trusted_window_extraction;
    bool untrusted_window_extraction;
    int listenfd;
    struct timeval timeout;
    struct timeval throttle;
    Timer th_timer;
    map<unsigned int, int> nconn_per_ip;

    // It is very important that "default_station" and "stations" are
    // declared after "monitor", because these objects send a callback
    // to "monitor" from their destructors and thus must be destroyed
    // before "monitor". Also, "connections" must be declared after
    // "default_station" and "stations".
    rc_ptr<MasterMonitor> monitor;
    rc_ptr<StationIO> default_station;
    map<StationDescriptor, rc_ptr<StationIO> > stations;
    list<rc_ptr<Connection> > connections;

    void client_connect();
    void client_disconnect(rc_ptr<Connection> conn);

  public:
    ConnectionManagerImpl(const string &daemon_name_init, 
      const string &software_ident_init, const string &default_network_id_init,
      rc_ptr<MasterMonitor> monitor_init, bool rlog_init, int max_conn_init,
      int max_conn_per_ip_init, int trusted_info_level_init,
      int untrusted_info_level_init, bool trusted_window_extraction_init,
      bool untrusted_window_extraction_init, int max_bps, int to_sec,
      int to_usec);
      
    rc_ptr<BufferStore> register_station(const string &station_key,
      const string &station_name, const string &network_id,
      const string &description, bool rlog, const string &station_dir,
      int nbufs, int blank_bufs, int filesize, int nfiles, int seq_gap_limit,
      bool stream_check, const string &gap_check_pattern, int gap_treshold,
      const IPACL &ip_access);

    void start(int port);
    void save_state();
    void restore_state();
  };
    
ConnectionManagerImpl::ConnectionManagerImpl(const string &daemon_name_init, 
  const string &software_ident_init, const string &default_network_id_init,
  rc_ptr<MasterMonitor> monitor_init, bool rlog_init, int max_conn_init,
  int max_conn_per_ip_init, int trusted_info_level_init,
  int untrusted_info_level_init, bool trusted_window_extraction_init,
  bool untrusted_window_extraction_init, int max_bps, int to_sec, int to_usec):
  daemon_name(daemon_name_init), software_ident(software_ident_init),
  default_network_id(default_network_id_init), rlog(rlog_init),
  max_conn(max_conn_init), max_conn_per_ip(max_conn_per_ip_init),
  trusted_info_level(trusted_info_level_init),
  untrusted_info_level(untrusted_info_level_init),
  trusted_window_extraction(trusted_window_extraction_init),
  untrusted_window_extraction(untrusted_window_extraction_init),
  listenfd(-1), monitor(monitor_init)
  {
    handler = new ConnectionHandler;
    
    monitor->add_capability("dialup", false);
    monitor->add_capability("multistation", false);

    if(untrusted_window_extraction) monitor->add_capability("window-extraction", false);
    else if(trusted_window_extraction) monitor->add_capability("window-extraction", true);
    
    for(int i = 0; i < N_InfoLevel; ++i)
      {
        if(i <= untrusted_info_level)
            monitor->add_capability(string("info:") + InfoLevelNames[i], false);
        else if(i <= trusted_info_level)
            monitor->add_capability(string("info:") + InfoLevelNames[i], true);
      }

    timeout.tv_sec = to_sec;
    timeout.tv_usec = to_usec;

    if(max_bps != 0)
      {
        div_t d_th_sec = div((1 << MSEED_RECLEN) + 8, max_bps);
        throttle.tv_sec = d_th_sec.quot;
        
        div_t d_th_usec = div(d_th_sec.rem * 1000000, max_bps);
        throttle.tv_usec = d_th_usec.quot;
      }
    else
      {
        throttle.tv_sec = 0;
        throttle.tv_usec = 0;
      }

    th_timer = Timer(throttle);
  }
      
void ConnectionManagerImpl::client_connect()
  {
    int clientfd = -1;
    struct sockaddr_in inet_addr;

#if defined(__GNU_LIBRARY__) && __GNU_LIBRARY__ < 2
    int len;
#else
    socklen_t len;
#endif

    len = sizeof(inet_addr);

    try
      {
        if((clientfd = xaccept(listenfd, (struct sockaddr *) &inet_addr, &len)) < 0)
          {
            logs(LOG_NOTICE) << "socket error: " << strerror(errno) << endl;
            return;
          }
      }
    catch(FDSetsizeExceeded &e)
      {
        logs(LOG_ERR) << "cannot accept connection: " << e.message << endl;
        close(clientfd);
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
        shutdown(clientfd, SHUT_RDWR);
        close(clientfd);
        return;
      }
        
    map<unsigned int, int>::iterator i;
    if((i = nconn_per_ip.find(ipaddr)) == nconn_per_ip.end())
      {
        nconn_per_ip.insert(make_pair(ipaddr, 1));
      }
    else
      {
        ++(i->second);

        if(max_conn_per_ip != 0 && i->second > max_conn_per_ip)
          {
            if(i->second == max_conn_per_ip + 1)
                logs(LOG_NOTICE) << host << ":" << port << " : maximum number of "
                  "connections per IP (" << max_conn_per_ip << ") exceeded" << endl;

            shutdown(clientfd, SHUT_RDWR);
            close(clientfd);
            return;
          }
      }
    
    logs(LOG_NOTICE) << string(host) << ":" << port <<
      " : opening connection" << endl;
    
    int info_level;
    bool window_extraction;

    if(monitor->iptrusted(ipaddr))
      {
        info_level = max(trusted_info_level, untrusted_info_level);
        window_extraction = trusted_window_extraction || untrusted_window_extraction;
      }
    else
      {
        info_level = untrusted_info_level;
        window_extraction = untrusted_window_extraction;
      }
        
    // check for default_station == NULL (no stations registered)...
    rc_ptr<Connection> conn = new Connection(ipaddr, host, port,
      default_station, default_network_id, rlog, info_level,
      window_extraction, monitor, stations,
      CPPStreams::logs.stream(string(host) + ":" + to_string(port) + " : "),
      clientfd);

    connections.push_back(conn);
    fds.set_read(clientfd);
  }
    
void ConnectionManagerImpl::client_disconnect(rc_ptr<Connection> conn)
  {
    conn->disconnect();

    map<unsigned int, int>::iterator i = nconn_per_ip.find(conn->ip());
    internal_check(i != nconn_per_ip.end());

    if(max_conn_per_ip != 0 && i->second > max_conn_per_ip)
      {
        logs(LOG_NOTICE) << "blocked " << (i->second - max_conn_per_ip) <<
          " connections from " << conn->host() << endl;

        i->second = max_conn_per_ip;
      }
    
    if(--(i->second) == 0)
          nconn_per_ip.erase(i);
  }

rc_ptr<BufferStore> ConnectionManagerImpl::register_station(const string &station_key,
  const string &station_name, const string &network_id,
  const string &description, bool rlog, const string &station_dir,
  int nbufs, int blank_bufs, int filesize, int nfiles, int seq_gap_limit,
  bool stream_check, const string &gap_check_pattern, int gap_treshold,
  const IPACL &ip_access)
  {
    StationDescriptor sd(network_id, station_name);
    rc_ptr<StationMonitor> statmon = monitor->add_station(station_name,
      network_id, description, ip_access);

    statmon->configure_stream_check(stream_check, gap_check_pattern,
      gap_treshold);

    rc_ptr<StationIO> stat = new StationIO(station_key,
      software_ident + "\r\n" + description + "\r\n", rlog, station_dir,
      nbufs, blank_bufs, filesize, nfiles, seq_gap_limit,
      stream_check, statmon);

    if(default_station == NULL) default_station = stat;

    if(!stations.insert(make_pair(sd, stat)).second)
        return NULL;
        
    return stat->get_bufs();
  }

void ConnectionManagerImpl::start(int port)
  {
    N(listenfd = xsocket(PF_INET, SOCK_STREAM, 0));

    int optval = 1;
    N(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)));

    struct sockaddr_in inet_addr;
    inet_addr.sin_family = AF_INET;
    inet_addr.sin_port = htons(port);
    inet_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd, (struct sockaddr *) &inet_addr, sizeof(inet_addr)) < 0)
        throw LibraryError("bind error");

    N(listen(listenfd, 5));
    fds.set_read(listenfd);

    struct timeval real_tv, tv, *ptv;

    real_tv = timeout;
    if(throttle.tv_sec != 0 || throttle.tv_usec != 0)
      {
        if(timeout.tv_sec > throttle.tv_sec) real_tv = throttle;
        else if(timeout.tv_sec == throttle.tv_sec &&
          timeout.tv_usec > throttle.tv_usec)
            real_tv = throttle;
      }
    
    if(real_tv.tv_sec != 0 || real_tv.tv_usec != 0) ptv = &tv;
    else ptv = NULL;
    
    th_timer.reset();
    
    while((*handler)(fds))
      {
        if(th_timer.expired())
          {
            fds.sync();
            th_timer.increment();
          }
        else if(throttle.tv_sec == 0 && throttle.tv_usec == 0)
          {
            fds.sync();
          }
        
        tv = real_tv;
        fds.select(ptv);
        
        if(fds.status() == 0 || (fds.status() < 0 && errno == EINTR)) continue;

        if(fds.status() < 0) throw LibraryError("select error");

        if(fds.isactive_read(listenfd)) client_connect();
    
        list<rc_ptr<Connection> >::iterator i;
        for(i = connections.begin(); i != connections.end(); ++i)
          {
            if((*i)->process())
              { 
                client_disconnect(*i);
                connections.erase(i);
                break;
              }
          }
      }

//    logs(LOG_NOTICE) << "shutting down" << endl;
    
    errno = 0;
    while(!connections.empty())
      {
        client_disconnect(connections.front());
        connections.pop_front();
      }

    close(listenfd);
  }

void ConnectionManagerImpl::save_state()
  {
    map<StationDescriptor, rc_ptr<StationIO> >::iterator i;
    for(i = stations.begin(); i != stations.end(); ++i)
        i->second->save_state();
  }

void ConnectionManagerImpl::restore_state()
  {
    map<StationDescriptor, rc_ptr<StationIO> >::iterator i;
    for(i = stations.begin(); i != stations.end(); ++i)
        i->second->restore_state();
  }

//*****************************************************************************
// Entry Point
//*****************************************************************************

rc_ptr<ConnectionManager> make_conn_manager(const string &daemon_name,
  const string &software_ident, const string &default_network_id,
  rc_ptr<MasterMonitor> monitor, bool rlog, int max_conn,
  int max_conn_per_ip, int trusted_info_level, int untrusted_info_level,
  bool trusted_window_extraction, bool untrusted_window_extraction,
  int max_bps, int to_sec, int to_usec)
  {
    return new ConnectionManagerImpl(daemon_name, software_ident,
      default_network_id, monitor, rlog, max_conn, max_conn_per_ip,
      trusted_info_level, untrusted_info_level, trusted_window_extraction,
      untrusted_window_extraction, max_bps, to_sec, to_usec);
  }

} // namespace IOSystem_private

