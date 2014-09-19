/*****************************************************************************
 * cppstreams.h
 *
 * Customized iostreams
 *
 * (c) 2001 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef CPPSTREAMS_H
#define CPPSTREAMS_H

#include <iostream>
#include <streambuf>
#include <string>

#include <syslog.h>

#include "utils.h"

namespace CPPStreams_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// OutputFunction
//*****************************************************************************

class OutputFunction
  {
  public:
    virtual int msglen() =0;
    virtual int operator()(int priority, const string &msg) =0;
    virtual ~OutputFunction() {};
  };

//*****************************************************************************
// StreambufBase
//*****************************************************************************

class StreambufBasePartner
  {
  public:
    virtual string msgprefix() =0;
    virtual ~StreambufBasePartner() {};
  };

class StreambufBase: public basic_streambuf<char>
  {
  private:
    StreambufBasePartner &partner;
    rc_ptr<OutputFunction> outfn;
    int priority;
    bool buffer_deleted;

    // No regular copy constructor
    StreambufBase(const StreambufBase &ls);
     
    StreambufBase *setbuf(char *p, streamsize n)
      {
        sync();
    
        if(!buffer_deleted) delete pbase();
        buffer_deleted = true;
        setp(p, p + n);
        return this;
      }
    
    int sync()
      {
        if(pptr() != pbase()) overflow();
        return 0;
      }
    
    int_type overflow(int_type c = traits_type::eof())
      {
        string msg = partner.msgprefix();
    
        msg.append(pbase(), pptr() - pbase());

        if(c != traits_type::eof())
            msg += traits_type::to_char_type(c);

        int nwritten = (*outfn)(priority, msg);
        int nleft = msg.length() - nwritten;

        setp(pbase(), epptr());
        
        if(nwritten == 0)
            return traits_type::eof();
        
        if(nleft > 0)
          {
            memcpy(pbase(), msg.c_str() + nwritten, nleft);
            pbump(nleft);
          }

        return traits_type::not_eof(c);
      }

  public:
    StreambufBase(StreambufBasePartner &partner_init,
      rc_ptr<OutputFunction> outfn_init):
      partner(partner_init), outfn(outfn_init), priority(0), buffer_deleted(false)
      {
        char *p = new char[outfn->msglen() - 1];
        setp(p, p + outfn->msglen() - 1);
      }
    
    StreambufBase(const StreambufBase &lsb, StreambufBasePartner &partner_init):
      partner(partner_init), outfn(lsb.outfn), priority(0), buffer_deleted(false)
      {
        char *p = new char[lsb.epptr() - lsb.pbase()];
        setp(p, p + (lsb.epptr() - lsb.pbase()));
      }
    
    virtual ~StreambufBase()
      {
        if(!buffer_deleted) delete pbase();
      }
    
    void set_priority(int prio)
      {
        if(priority != prio) sync();
        priority = prio;
      }
  };

//*****************************************************************************
// Streambuf
//*****************************************************************************

class Streambuf: public StreambufBase, private StreambufBasePartner
  {
  private:
    string prefix;

    string msgprefix()
      {
        return prefix;
      }
  
  public:
    Streambuf(rc_ptr<OutputFunction> outfn, int priority, const string &prefix_init):
      StreambufBase(*this, outfn), prefix(prefix_init)
      {
        set_priority(priority);
      }

    Streambuf(const StreambufBase &sbuf, int priority,
      const string &prefix_init):
      StreambufBase(sbuf, *this), prefix(prefix_init)
      {
        set_priority(priority);
      }
    
    Streambuf(const Streambuf &ssbuf):
      StreambufBase(ssbuf, *this), prefix(ssbuf.prefix) {}
  };

//*****************************************************************************
// Stream
//*****************************************************************************

class Stream: private StreambufBasePartner
  {
  private:
    rc_ptr<OutputFunction> outfn;
    rc_ptr<ostream> str;
    string prefix;

    StreambufBase *get_sbuf()
      {
        StreambufBase *sbuf;
        
        if(str == NULL)
          {
            sbuf = new StreambufBase(*this, outfn);
            str = new ostream(sbuf);
          }
        else
          {
            sbuf = dynamic_cast<StreambufBase *>(str->rdbuf());
          }

        return sbuf;
      }
    
    string msgprefix()
      {
        return prefix;
      }
  
  public:
    Stream(rc_ptr<OutputFunction> outfn_init, const string &prefix_init):
      outfn(outfn_init), prefix(prefix_init) {}

    Stream(const Stream &ls): outfn(ls.outfn), prefix(ls.prefix) {}
    
    ~Stream()
      {
        if(str != NULL) delete str->rdbuf(NULL);
      }
    
    Stream &operator=(const Stream &ls)
      {
        if(this != &ls)
          {
            this->~Stream();
            new(this) Stream(ls);
          }

        return *this;
      }

    ostream &operator()(int prio)
      {
        StreambufBase *sbuf;
        
        if((sbuf = get_sbuf()) != NULL) sbuf->set_priority(prio);
        return *str;
      }

    // stream() returns a new Stream, optionally adding a string
    // to the existing message prefix

    Stream stream(const string &add_prefix = "")
      {
        Stream ls(*this);
        ls.prefix += add_prefix;
        return ls;
      }
    
    // // buf() can be used to construct ostreams with fixed priority
    // 
    // Stream log = make_stream(outfn);
    // ostream errlog(log.buf(LOG_ERR));
    //
    // // Next two statements are equivalent:
    //
    // log(LOG_ERR) << "error!" << endl;
    // errlog << "error!" << endl;
    //
    // // redirect_ostream(), defined below, can be also used:
    //
    // ostream errlog(NULL);
    // redirect_ostream(errlog, outfn, LOG_ERR);
    //
    // // Note that unlike fstream and stringstream destructors,
    // // istream/ostream destructors do not delete streambuf, so it has to
    // // be deleted explicitly:
    //
    // delete errlog.rdbuf();
    //
    // // Standard streams can be redirected as well:
    //
    // redirect_ostream(cout, outfn, LOG_INFO);
    // redirect_ostream(cerr, outfn, LOG_ERR);
    //
    // // Streambufs of standard streams are deleted automatically on program
    // // exit

    Streambuf *buf(int priority = LOG_ERR, const string &add_prefix = "")
      {
        StreambufBase *sbuf;
        
        if((sbuf = get_sbuf()) != NULL)
            return new Streambuf(*sbuf, priority, prefix + add_prefix);
        else
            return NULL;
      }

    StreambufBase *bufbase(StreambufBasePartner &partner)
      {
        StreambufBase *sbuf;
        
        if((sbuf = get_sbuf()) != NULL)
            return new StreambufBase(*sbuf, partner);
        else
            return NULL;
      }
  };

//*****************************************************************************
// OutputFunctionAdapter etc.
//*****************************************************************************

template<class T>
class OutputFunctionAdapter: public OutputFunction
  {
  private:
    T rep;
    
  public:
    OutputFunctionAdapter(const T &rep_init): rep(rep_init) {}
    
    int msglen()
      {
        return rep.msglen;
      }
    
    int operator()(int priority, const string &msg)
      {
        return rep(priority, msg);
      }
  };

template<class T>
inline Stream make_stream(const T &outfn, const string &prefix = "")
  {
    return Stream(new OutputFunctionAdapter<T>(outfn), prefix);
  }

template<class T>
inline void redirect_ostream(ostream &str, const T &outfn, int priority = 0,
  const string &prefix = "")
  {
    str.rdbuf(new Streambuf(new OutputFunctionAdapter<T>(outfn), priority,
      prefix));
  }

} // namespace CPPStreams_private

namespace CPPStreams {

using CPPStreams_private::OutputFunction;
using CPPStreams_private::OutputFunctionAdapter;
using CPPStreams_private::StreambufBasePartner;
using CPPStreams_private::StreambufBase;
using CPPStreams_private::Streambuf;
using CPPStreams_private::Stream;
using CPPStreams_private::make_stream;
using CPPStreams_private::redirect_ostream;

//*****************************************************************************
// Global Stream object
//*****************************************************************************

extern Stream logs;

} // namespace CPPStreams

#endif // CPPSTREAMS_H

