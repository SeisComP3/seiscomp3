/***************************************************************************** 
 * utils.h
 *
 * Module "Utilities"
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <exception>
#include <new>
#include <cstring>
#include <cerrno>

#include <sys/time.h>
#include <sys/types.h>

namespace Utilities_private {

using namespace std;

//*****************************************************************************
// Pointer with reference counting
//*****************************************************************************

template<class T>
class rc_ptr
  {
  template<class U, class V>
  friend rc_ptr<U> rc_ptr_cast(const rc_ptr<V> &p);

  private:
    int *ref_count;
    T *ptr;

  public:
    rc_ptr(): ref_count(NULL), ptr(NULL) {}
    
    rc_ptr(T *ptr_init): ref_count(new int(1)), ptr(ptr_init) {}
    
    // According to an example given in Stroustrup (1997), p. 347 (13.6.2,
    // Member Templates), next two helper functions should be redundant for
    // the template contructor, eg. rc_ptr<X> should have access to private
    // members of rc_ptr<Y>
    
    int *_ref_count() const
      {
        return ref_count;
      }

    T *_ptr() const
      {
        return ptr;
      }
    
    template<class U>
    rc_ptr(const rc_ptr<U> &p): ref_count(p._ref_count()), ptr(p._ptr())
      {
        if(ref_count) ++(*ref_count);
      }

    // We need to define a normal copy contructor as well, because a
    // template constructor is never used to generate a copy constructor;
    // without the explicitly declared copy contructor, a default copy
    // constructor will be generated (Stroustrup 1997, p. 348)
    
    rc_ptr(const rc_ptr<T> &p): ref_count(p.ref_count), ptr(p.ptr)
      {
        if(ref_count) ++(*ref_count);
      }
    
    rc_ptr &operator=(const rc_ptr &p)
      {
        if(this != &p)
          {
            this->~rc_ptr();
            new(this) rc_ptr(p);
          }
        
        return *this;
      }

    ~rc_ptr()
      {
        if(ref_count && --(*ref_count) == 0)
          {
            delete ref_count;
            delete ptr;
          }
      }
    
    T &operator*() const
      {
        return *ptr;
      }

    T *operator->() const
      {
        return ptr;
      }

    bool operator==(const rc_ptr<T> &p) const
      {
        return (ptr == p.ptr);
      }
    
    bool operator==(const T *p) const
      {
        return (ptr == p);
      }
    
    bool operator!=(const rc_ptr<T> &p) const
      {
        return (ptr != p.ptr);
      }
    
    bool operator!=(const T *p) const
      {
        return (ptr != p);
      }
  };

template<class U, class V>
inline rc_ptr<U> rc_ptr_cast(const rc_ptr<V> &p)
  {
    rc_ptr<U> ru;
    U *u;
    
    if((u = dynamic_cast<U *>(p.ptr)) != NULL)
      {
        ru.ptr = u;
        ru.ref_count = p.ref_count;
        ++(*p.ref_count);
      }
        
    return ru;
  }

//*****************************************************************************
// insert_object(), has_object(), get_object()
//*****************************************************************************

template<class T>
inline void insert_object(map<string, rc_ptr<T> > &m, rc_ptr<T> obj)
  {
    m.insert(make_pair(obj->name, obj));
  }

template<class T>
inline void insert_object(map<string, rc_ptr<T> > &m, T *obj)
  {
    m.insert(make_pair(obj->name, obj));
  }

template<class T>
inline bool has_object(const map<string, rc_ptr<T> > &m, const string &name)
  {
    return (m.find(name) != m.end());
  }

template<class T>
inline rc_ptr<T> get_object(const map<string, rc_ptr<T> > &m, const string &name)
  {
    typename map<string, rc_ptr<T> >::const_iterator p = m.find(name);
    
    if(p == m.end()) return NULL;
    else return p->second;
  }
    
//*****************************************************************************
// Replicator
//*****************************************************************************

template<class T>
class Replicator
  {
  public:
    virtual rc_ptr<T> instance() =0;
    virtual ~Replicator() {}
  };

template<class T, class U>
class ReplicatorImpl: public Replicator<T>
  {
  private:
    const rc_ptr<const U> rep;

  public:
    ReplicatorImpl(rc_ptr<const U> rep_init): rep(rep_init) {}

    rc_ptr<T> instance()
      {
        return new U(*rep);
      }
  };

template<class T, class U>
inline rc_ptr<Replicator<T> > make_replicator(rc_ptr<const U> rep)
  {
    return new ReplicatorImpl<T,U>(rep);
  }

template<class T, class U>
inline rc_ptr<Replicator<T> > make_replicator(const U *rep)
  {
    return new ReplicatorImpl<T,U>(rep);
  }

//*****************************************************************************
// to_string()
//*****************************************************************************

template<class T>
inline string to_string(T val)
  {
    ostringstream s;

    s << val;
    return s.str();
  }

//*****************************************************************************
// GenericException
//*****************************************************************************

class GenericException: public exception
  {
  public:
    const string subsys;
    const string message;
    GenericException(const string &subsys_init, const string &message_init):
      subsys(subsys_init), message(message_init) {}
    ~GenericException() throw () {}

    const char *what() const throw()
      {
        return message.c_str();
      }
  };

inline string exception_from_subsys(exception &e)
  {
    GenericException *ge;

    if((ge = dynamic_cast<GenericException *>(&e)) == NULL)
        return "standard library";

    return ge->subsys;
  }

//*****************************************************************************
// Read/Write
//*****************************************************************************

ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t writen_tmo(int fd, const void *vptr, size_t n, int tmo);

//*****************************************************************************
// Timer
//*****************************************************************************

class Timer
  {
  private:
    const int iv_sec, iv_usec;
    timeval lasttime;

  public:
    Timer(): iv_sec(0), iv_usec(0) {}
    
    Timer(int interval_sec, int interval_usec):
      iv_sec(interval_sec), iv_usec(interval_usec)
      {
        lasttime.tv_sec = 0;
        lasttime.tv_usec = 0;
      }

    Timer(timeval interval):
      iv_sec(interval.tv_sec), iv_usec(interval.tv_usec)
      {
        lasttime.tv_sec = 0;
        lasttime.tv_usec = 0;
      }
    
    Timer &operator=(const Timer &t)
      {
        if(this != &t)
          {
            this->~Timer();
            new(this) Timer(t);
          }

        return *this;
      }

    bool expired();
    void increment();
    void reset();
  };

//*****************************************************************************
// CFIFO
//*****************************************************************************

class CFIFO_ReadError: public GenericException
  {
  public:
    CFIFO_ReadError():
      GenericException("CFIFO", strerror(errno)) {}
  };

class CFIFO_Overflow: public GenericException
  {
  public:
    CFIFO_Overflow():
      GenericException("CFIFO", "overflow") {}
  };

class CFIFO_Partner
  {
  public:
    virtual void cfifo_callback(const string &cmd) = 0;
    virtual ~CFIFO_Partner() {};
  };

class CFIFO
  {
  private:
    CFIFO_Partner &partner;
    const int cmdmaxlen;
    char *cmdbuf;
    int cmdwp;

  public:
    CFIFO(CFIFO_Partner &partner_init, int cmdmaxlen_init):
      partner(partner_init), cmdmaxlen(cmdmaxlen_init), cmdwp(0)
      {
        cmdbuf = new char[cmdmaxlen + 1];
      }

    ~CFIFO()
      {
        delete cmdbuf;
      }
    
    int check(int fd);
  };

} // namespace Utilities_private

namespace Utilities {

using Utilities_private::rc_ptr;
using Utilities_private::rc_ptr_cast;
using Utilities_private::insert_object;
using Utilities_private::has_object;
using Utilities_private::get_object;
using Utilities_private::Replicator;
using Utilities_private::make_replicator;
using Utilities_private::to_string;
using Utilities_private::GenericException;
using Utilities_private::exception_from_subsys;
using Utilities_private::readn;
using Utilities_private::writen;
using Utilities_private::writen_tmo;
using Utilities_private::Timer;
using Utilities_private::CFIFO_ReadError;
using Utilities_private::CFIFO_Overflow;
using Utilities_private::CFIFO_Partner;
using Utilities_private::CFIFO;

} // namespace Utilities

#endif // UTILS_H

