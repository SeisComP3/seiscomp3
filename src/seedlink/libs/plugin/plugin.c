/***************************************************************************** 
 * plugin.c
 *
 * SeedLink plugin interface module
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "plugin.h"

static int send_log_helper(const char *station, const struct ptime *pt,
  const char *fmt, va_list argptr);
static int send_packet(const struct PluginPacketHeader *head,
  const void *dataptr, int data_bytes);
static ssize_t writen(int fd, const void *vptr, size_t n);

int send_raw3(const char *station, const char *channel, const struct ptime *pt,
  int usec_correction, int timing_quality, const int32_t *dataptr,
  int number_of_samples)
  {
    int r, sample_count, samples_sent = 0;
    struct PluginPacketHeader head;

    memset(&head, 0, sizeof(struct PluginPacketHeader));
    strncpy(head.station, station, PLUGIN_SIDLEN);
    strncpy(head.channel, channel, PLUGIN_CIDLEN);
        
    if(number_of_samples == 0 && pt != NULL)
      {
        head.packtype = PluginRawDataTimePacket;
        head.pt = *pt;
        head.usec_correction = usec_correction;
        head.timing_quality = timing_quality;

        return send_packet(&head, NULL, 0);
      }
    
    if(dataptr == NULL)
      {
        head.packtype = PluginRawDataGapPacket;
        head.data_size = number_of_samples;
        
        memset(&head.pt, 0, sizeof(struct ptime));
        head.usec_correction = usec_correction;
        head.timing_quality = timing_quality;

        return send_packet(&head, NULL, 0);
      }

    if(number_of_samples < 0)
        return -1;
    
    while(samples_sent < number_of_samples)
      {
        sample_count = number_of_samples - samples_sent;
        
        if(sample_count > (PLUGIN_MAX_DATA_BYTES >> 2))
            sample_count = (PLUGIN_MAX_DATA_BYTES >> 2);
        
        head.data_size = sample_count;
        
        if(samples_sent == 0 && pt != NULL)
          {
            head.packtype = PluginRawDataTimePacket;
            head.pt = *pt;
            head.usec_correction = usec_correction;
            head.timing_quality = timing_quality;
          }
        else
          {
            head.packtype = PluginRawDataPacket;
            memset(&head.pt, 0, sizeof(struct ptime));
            head.usec_correction = 0;
            head.timing_quality = 0;
          }

        if((r = send_packet(&head, dataptr + samples_sent,
          sample_count << 2)) <= 0)
            return r;
        
        samples_sent += sample_count;
      }

    return (number_of_samples << 2);
  }

int send_flush3(const char *station, const char *channel)
  {
    struct PluginPacketHeader head;

    memset(&head, 0, sizeof(struct PluginPacketHeader));
    strncpy(head.station, station, PLUGIN_SIDLEN);
    strncpy(head.channel, channel, PLUGIN_CIDLEN);
        
    head.packtype = PluginRawDataFlushPacket;
    head.data_size = 0;
    head.usec_correction = 0;
    head.timing_quality = 0;
    memset(&head.pt, 0, sizeof(struct ptime));

    return send_packet(&head, NULL, 0);
  }
    
int send_raw_depoch(const char *station, const char *channel, double depoch,
  int usec_correction, int timing_quality, const int32_t *dataptr,
  int number_of_samples)
  {
    time_t t;
    struct ptime pt;
    struct tm *t_parts;

    t = (time_t)depoch;
    t_parts = gmtime(&t);
    pt.year = t_parts->tm_year + 1900;
    pt.yday = t_parts->tm_yday + 1;
    pt.hour = t_parts->tm_hour;
    pt.minute = t_parts->tm_min;
    pt.second = t_parts->tm_sec;
    pt.usec   = (int)((depoch - (double)t) * 1000000.0);

    return send_raw3(station, channel, &pt, usec_correction, timing_quality,
      dataptr, number_of_samples);
  }

int send_mseed(const char *station, const void *dataptr, int packet_size)
  {
    struct PluginPacketHeader head;

    if(packet_size != PLUGIN_MSEED_SIZE) return 0;

    memset(&head, 0, sizeof(struct PluginPacketHeader));
    strncpy(head.station, station, PLUGIN_SIDLEN);
    head.packtype = PluginMSEEDPacket;
    head.data_size = packet_size;

    return send_packet(&head, dataptr, packet_size);
  }

int send_log3(const char *station, const struct ptime *pt, const char *fmt, ...)
  {
    int retval;
    va_list argptr;
    
    if(pt == NULL) return 0;
    
    va_start(argptr, fmt);
    retval = send_log_helper(station, pt, fmt, argptr);
    va_end(argptr);

    return retval;
  }
    
#ifdef PLUGIN_COMPATIBILITY
struct ptime it_to_pt(INT_TIME it)
  {
    EXT_TIME et = int_to_ext(it);
    struct ptime pt;

    pt.year = et.year;
    pt.yday = et.doy;
    pt.hour = et.hour;
    pt.minute = et.minute;
    pt.second = et.second;
    pt.usec = et.usec;
    return pt;
  }

int send_raw2(const char *station, const char *channel, const INT_TIME *it,
  int usec_correction, int timing_quality, const int32_t *dataptr,
  int number_of_samples)
  {
    struct ptime pt;
    
    if(it == NULL)
      {
        if(number_of_samples == 0)
            return send_flush3(station, channel);
        else
            return send_raw3(station, channel, NULL, usec_correction,
              timing_quality, dataptr, number_of_samples);
      }
        
    pt = it_to_pt(*it);
    return send_raw3(station, channel, &pt, usec_correction, timing_quality,
      dataptr, number_of_samples);
  }

int send_raw(const char *station, const char *channel, const INT_TIME *it,
  int usec_correction, const int32_t *dataptr, int number_of_samples)
  {
    return send_raw2(station, channel, it, usec_correction, -1, dataptr,
      number_of_samples);
  }

int send_log(const char *station, const INT_TIME *it, const char *fmt, ...)
  {
    int retval;
    struct ptime pt;
    va_list argptr;
    
    if(it == NULL) return 0;
    
    va_start(argptr, fmt);
    pt = it_to_pt(*it);
    retval = send_log_helper(station, &pt, fmt, argptr);
    va_end(argptr);

    return retval;
  }
#endif

int send_log_helper(const char *station, const struct ptime *pt,
  const char *fmt, va_list argptr)
  {
    struct PluginPacketHeader head;
    char buf[PLUGIN_MAX_MSG_SIZE];
    int msgsize;

    memset(&head, 0, sizeof(struct PluginPacketHeader));
    strncpy(head.station, station, PLUGIN_SIDLEN);
    head.pt = *pt;
    head.usec_correction = 0;
    head.timing_quality = 0;
    msgsize = vsnprintf(buf, PLUGIN_MAX_MSG_SIZE, fmt, argptr);

    if(msgsize < 0 || msgsize > PLUGIN_MAX_MSG_SIZE - 3)
        msgsize = PLUGIN_MAX_MSG_SIZE - 3;

    while(buf[msgsize - 1] == '\r' || buf[msgsize - 1] == '\n')
        --msgsize;
    
    strcpy(&buf[msgsize], "\r\n");

    head.packtype = PluginLogPacket;
    head.data_size = msgsize + 2;
    
    return send_packet(&head, buf, msgsize + 2);
  }

int send_packet(const struct PluginPacketHeader *head, const void *dataptr,
  int data_bytes)
  {
    int r;
    
    if((r = writen(PLUGIN_FD, head, sizeof(struct PluginPacketHeader))) <= 0)
        return r;
    
    if(dataptr != NULL && (r = writen(PLUGIN_FD, dataptr, data_bytes)) <= 0)
        return r;

    return data_bytes;
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

