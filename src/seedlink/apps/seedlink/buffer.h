/*****************************************************************************
 * buffer.h
 *
 * Abstract BufferStore interface
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

class Buffer
  {
  protected:
    Buffer(int size_init): size(size_init) {}
    virtual ~Buffer() {}
  
  public:
    const int size;

    virtual void *data() const =0;
  };

class BufferStore
  {
  public:
    virtual Buffer *get_buffer() =0;
    virtual void queue_buffer(Buffer *buf) =0;
    virtual int size() const { return 0; }
    virtual void enlarge(int newsize) {}
    virtual ~BufferStore() {}
  };

#endif // BUFFER_H

