/***************************************************************************** 
 * utils.cc
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

#include <unistd.h>
#include <sys/types.h>

#include "utils.h"
#include "diag.h"

namespace Utilities_private {

using namespace std;

//*****************************************************************************
// Read/Write
//*****************************************************************************

ssize_t readn(int fd, void *vptr, size_t n)
  {
    ssize_t nread;
    size_t nleft = n;
    char* ptr = (char *)(vptr);
    
    while(nleft > 0)
      {
        if((nread = read(fd, ptr, nleft)) <= 0)
            return nread;

        nleft -= nread;
        ptr += nread;
      }

    return(n);
  }

ssize_t writen(int fd, const void *vptr, size_t n)
  {
    ssize_t nwritten;
    size_t nleft = n;
    const char *ptr = (const char *) vptr;   

    while(nleft > 0)
      {
        if((nwritten = write(fd, ptr, nleft)) <= 0)
          {
            if(nwritten < 0 && errno == EINTR) continue;
            return(nwritten);
          }

        nleft -= nwritten;
        ptr += nwritten;
      }
    
    return(n);
  }

ssize_t writen_tmo(int fd, const void *vptr, size_t n, int tmo)
  {
    ssize_t nwritten;
    size_t nleft = n;
    const char *ptr = (const char *) vptr;   
    int r;
    fd_set fds;
    struct timeval tv;

    while(nleft > 0)
      {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        
        tv.tv_sec = tmo;
        tv.tv_usec = 0;

        if((r = select(fd + 1, NULL, &fds, NULL, &tv)) < 0)
          {
            if(errno == EINTR) continue;
            return -1;
          }

        if(r == 0) return 0;
        
        if((nwritten = write(fd, ptr, nleft)) <= 0)
          {
            if(nwritten < 0 && errno == EINTR) continue;
            return(nwritten);
          }

        nleft -= nwritten;
        ptr += nwritten;
      }
    
    return(n);
  }

//*****************************************************************************
// Timer
//*****************************************************************************

bool Timer::expired()
  {
    if(iv_sec == 0 && iv_usec == 0) return false;

    timeval curtime;

    N(gettimeofday(&curtime, NULL));
    
    if(curtime.tv_sec - lasttime.tv_sec > iv_sec + 1) return true;
    else if(curtime.tv_sec - lasttime.tv_sec >= iv_sec)
      {
        if(curtime.tv_usec - lasttime.tv_usec - iv_usec +
          (curtime.tv_sec - lasttime.tv_sec - iv_sec) * 1000000 >= 0)
          return true;
      }
    
    return false;
  }

void Timer::increment()
  {
    lasttime.tv_sec += iv_sec;
    lasttime.tv_usec += iv_usec;

    while(lasttime.tv_usec >= 1000000)
      {
        lasttime.tv_usec -= 1000000;
        ++lasttime.tv_sec;
      }
  }

void Timer::reset()
  {
    if(iv_sec == 0 && iv_usec == 0) return;

    N(gettimeofday(&lasttime, NULL));
  }

//*****************************************************************************
// CFIFO
//*****************************************************************************

int CFIFO::check(int fd)
  {
    if(fd < 0) return -1;
    
    int bytes_read;
    if((bytes_read = read(fd, &cmdbuf[cmdwp], cmdmaxlen - cmdwp)) < 0)
      {
        if(errno == EAGAIN)
            return -1;
            
        throw CFIFO_ReadError();
      }

    if(bytes_read == 0)
        return 0;

    cmdwp += bytes_read;
    cmdbuf[cmdwp] = 0;

    int cmdrp = 0, cmdlen, seplen;
    while(cmdlen = strcspn(cmdbuf + cmdrp, "\r\n"),
      seplen = strspn(cmdbuf + cmdrp + cmdlen, "\r\n"))
      {
        cmdbuf[cmdrp + cmdlen] = 0;
        partner.cfifo_callback(cmdbuf + cmdrp);
        cmdrp += (cmdlen + seplen);
      }
    
    if(cmdlen >= cmdmaxlen)
      {
        cmdwp = cmdrp = 0;
        throw CFIFO_Overflow();
      }
        
    memmove(cmdbuf, cmdbuf + cmdrp, cmdlen);
    cmdwp -= cmdrp;
    cmdrp = 0;

    return bytes_read;
  }

} // namespace Utilities_private

