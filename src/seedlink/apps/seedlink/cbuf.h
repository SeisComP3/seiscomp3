/***************************************************************************** 
 * cbuf.h
 *
 * Circular buffer
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef CBUF_H
#define CBUF_H

#include <iterator>

#include "utils.h"

namespace SProc_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// CircularBuffer
//*****************************************************************************

template<class T>
class CircularBuffer;

template<class T>
class CBIterator: public iterator<input_iterator_tag, const T>
  {
  friend class CircularBuffer<T>;
  private:
    typedef typename iterator<input_iterator_tag, const T>::reference reference;
    typedef typename iterator<input_iterator_tag, const T>::pointer pointer;

    pointer dataptr;
    unsigned int mask;
    unsigned int rdp;

    CBIterator(pointer dataptr_init, int mask_init, int wrp):
      dataptr(dataptr_init), mask(mask_init), rdp(wrp) {}

    void invalid_iterator(const char *file, int line) const
      {
        clog << file << ":" << line << ": invalid iterator" << endl;
        exit(1);
      }

  public:
    CBIterator(): mask(0), rdp(0) {}
    
    reference operator*() const
      {
        if(dataptr == NULL) invalid_iterator(__FILE__, __LINE__);
        return *(dataptr + rdp);
      }

    pointer operator->() const
      {
        if(dataptr == NULL) invalid_iterator(__FILE__, __LINE__);
        return (dataptr + rdp);
      }

    CBIterator &operator++()
      {
        rdp = (rdp + 1) & mask;
        return *this;
      }

    CBIterator operator++(int)
      {
        CBIterator tmp = *this;
        rdp = (rdp + 1) & mask;
        return tmp;
      }

    bool operator==(const CBIterator &iter) const
      {
        if(dataptr != iter.dataptr) invalid_iterator(__FILE__, __LINE__);
        return (rdp == iter.rdp);
      }

    bool operator!=(const CBIterator &iter) const
      {
        if(dataptr != iter.dataptr) invalid_iterator(__FILE__, __LINE__);
        return (rdp != iter.rdp);
      }

    // Not required for input iterators

    CBIterator &operator+=(int n)
      {
        rdp = (rdp + n) & mask;
        return *this;
      }
  };

template<class T>
class CircularBuffer
  {
  private:
    T *dataptr;
    int mask;
    int wrp;

  public:
    typedef CBIterator<T> iterator;

    // Size is given in powers of two
    
    CircularBuffer(int size)
      {
        dataptr = new T[1 << size];
        mask = (1 << size) - 1;
        wrp = 0;
      }
    
    ~CircularBuffer()
      {
        delete[] dataptr;
      }
    
    int used(const CBIterator<T> &iter) const
      {
        return (wrp - iter.rdp) & mask;
      }
    
    int free(const CBIterator<T> &iter) const
      {
        return (iter.rdp - wrp - 1) & mask;
      }

    CBIterator<T> read_ptr() const
      {
        return CBIterator<T>(dataptr, mask, wrp);
      }
    
    void write(T val)
      {
        dataptr[wrp] = val;
        wrp = (wrp + 1) & mask;
      }
  };
    
} // namespace SProc_private

#endif // CBUF_H

