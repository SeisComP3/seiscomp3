/*   Lib330 Statistics
     Copyright 2006 Certified Software Corporation

    This file is part of Lib330

    Lib330 is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Lib330 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lib330; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Edit History:
   Ed Date       By  Changes
   -- ---------- --- ---------------------------------------------------
    0 2006-09-29 rdr Created
    1 2006-11-23 rdr Communications efficiency status reworked.
    2 2006-11-29 rdr Make sure compiler uses floating point for com. eff. calculations
*/
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libmsg_h
#include "libmsgs.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

#ifndef OMIT_SEED
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libcompress_h
#include "libcompress.h"
#endif
#endif
static void process_dpstat (pq330 q330, plcq q, longint val)
begin
#define DPSTAT_QUALITY 100
  paqstruc paqs ;
  integer used ;
#ifndef OMIT_SEED
  tcom_packet *pcom ;
#endif
  string95 s ;
  string31 s1 ;
  string7 s2 ;

  paqs = q330->aqstruc ;
#ifndef OMIT_SEED
  pcom = q->com ;
  q->data_written = FALSE ;
#endif
  if ((q->last_timetag > 1) land (fabs(q330->dpstat_timestamp - q->last_timetag - q->gap_offset) > q->gap_secs))
    then
      begin
        if (q330->cur_verbosity and VERB_LOGEXTRA)
          then
            begin
              sprintf(s, "%s %s", seed2string(addr(q->location), addr(q->seedname), addr(s2)),
                      realtostr(q330->dpstat_timestamp - q->last_timetag - q->gap_offset, 6, addr(s1))) ;
              libdatamsg (q330, LIBMSG_TIMEDISC, addr(s)) ;
            end
#ifndef OMIT_SEED
        flush_lcq (paqs, q, pcom) ; /* gap in the data */
#endif
      end
  q->last_timetag = q330->dpstat_timestamp ;
  if (q->timetag == 0)
    then
      begin
        q->timetag = q330->dpstat_timestamp ;
        q->timequal = DPSTAT_QUALITY ;
#ifndef OMIT_SEED
        pcom->time_mark_sample = pcom->peek_total + pcom->next_compressed_sample ;
#endif
      end
  if (q->onesec_filter)
    then
      begin
        q330->onesec_call.total_size = sizeof(tonesec_call) - ((MAX_RATE - 1) * sizeof(longint)) ;
        q330->onesec_call.context = q330 ;
        memcpy(addr(q330->onesec_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
        strcpy(addr(q330->onesec_call.location), addr(q->slocation)) ;
        strcpy(addr(q330->onesec_call.channel), addr(q->sseedname)) ;
        q330->onesec_call.chan_number = q->lcq_num ;
        q330->onesec_call.cl_session = 0 ;
        q330->onesec_call.cl_offset = 0 ;
        q330->onesec_call.timestamp = q330->dpstat_timestamp ;
        q330->onesec_call.qual_perc = DPSTAT_QUALITY ;
        q330->onesec_call.filter_bits = q->onesec_filter ;
        q330->onesec_call.rate = q->rate ;
        q330->onesec_call.activity_flags = 0 ;
        if (paqs->data_qual >= q330->qclock.q_off)
         then
           q330->onesec_call.io_flags = SIF_LOCKED ;
         else
           q330->onesec_call.io_flags = 0 ;
        if (paqs->data_qual < q330->qclock.q_low)
          then
            q330->onesec_call.data_quality_flags = SQF_QUESTIONABLE_TIMETAG ;
          else
            q330->onesec_call.data_quality_flags = 0 ;
        q330->onesec_call.src_channel = q->raw_data_source ;
        q330->onesec_call.src_subchan = q->raw_data_field ;
        q330->onesec_call.samples[0] = val ;
        q330->par_create.call_secdata (addr(q330->onesec_call)) ;
      end
#ifndef OMIT_SEED
  pcom->peeks[pcom->next_in] = val ;
  pcom->next_in = (pcom->next_in + 1) and (PEEKELEMS - 1) ;
  inc(pcom->peek_total) ;
  if (pcom->peek_total < MAXSAMPPERWORD)
    then
      return ;
  used = compress_block (paqs, q, pcom) ;
  pcom->next_compressed_sample = pcom->next_compressed_sample + used ;
  if (pcom->frame >= (pcom->maxframes + 1))
    then
      finish_record (paqs, q, pcom) ;
#endif
end

void add_status (pq330 q330, enum tacctype acctype, longword count)
begin

  incn(q330->share.accmstats[acctype].accum, count) ;
  incn(q330->share.accmstats[acctype].accum_ds, count) ;
end

void lib_stats_timer (pq330 q330)
begin
  enum tacctype acctype ;
  integer minute, last_minute, comeff_valids ;
  longint total, sentdif, resdif, val, duty ;
  paqstruc paqs ;
  taccmstat *paccm ;

  lock (q330) ;
  if (q330->share.have_status and make_bitmap(SRB_LOG1 + q330->par_create.q330id_dataport))
    then
      begin
        sentdif = q330->share.stat_log.sent - q330->lastds_sent_count ;
        resdif = q330->share.stat_log.resends - q330->lastds_resent_count ;
        if ((sentdif + resdif) == 0)
          then
            q330->share.accmstats[AC_COMEFF].accum_ds = 0 ;
          else
            q330->share.accmstats[AC_COMEFF].accum_ds = (1000.0 * sentdif) / (double) (sentdif + resdif) + 0.5 ;
        q330->lastds_sent_count = q330->share.stat_log.sent ;
        q330->lastds_resent_count = q330->share.stat_log.resends ;
      end
    else
      q330->share.accmstats[AC_COMEFF].accum_ds = 0 ;
  duty = q330->share.accmstats[AC_DUTY].accum_ds ;
  for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
    begin
      paccm = addr(q330->share.accmstats[acctype]) ;
      if (paccm->ds_lcq)
        then
          begin
            switch (acctype) begin
              case AC_READ :
              case AC_WRITE :
                val = paccm->accum_ds div 10 ;
                break ;
              case AC_DUTY :
                val = (paccm->accum_ds * 1000) div 10 ;
                break ;
              case AC_THROUGH :
                val = (paccm->accum_ds * 100) div 10 ;
                break ;
              default :
                val = paccm->accum_ds ;
                break ;
            end
            unlock (q330) ;
            process_dpstat (q330, paccm->ds_lcq, val) ;
            lock (q330) ;
          end
      paccm->accum_ds = 0 ;
    end
  paqs = q330->aqstruc ;
  unlock (q330) ;
  if ((paqs->data_latency_lcq) land (q330->saved_data_timetag > 1))
    then
      process_dpstat (q330, paqs->data_latency_lcq, now () - q330->saved_data_timetag + 0.5) ;
  if ((paqs->status_latency_lcq) land (q330->last_status_received > 1))
    then
      process_dpstat (q330, paqs->status_latency_lcq, now () - q330->last_status_received + 0.5) ;
  inc(q330->minute_counter) ;
  if (q330->minute_counter >= 6)
    then
      q330->minute_counter = 0 ;
    else
      return ; /* not a new minute yet */
  lock (q330) ;
  if (q330->share.have_status and make_bitmap(SRB_LOG1 + q330->par_create.q330id_dataport))
    then
      begin
        sentdif = q330->share.stat_log.sent - q330->last_sent_count ;
        resdif = q330->share.stat_log.resends - q330->last_resent_count ;
        if ((sentdif + resdif) == 0)
          then
            begin
              if (q330->share.accmstats[AC_DUTY].accum >= 5)
                then
                  q330->share.accmstats[AC_COMEFF].accum = 0 ; /* was connected */
                else
                  q330->share.accmstats[AC_COMEFF].accum = INVALID_ENTRY ; /* not connected */
            end
          else
            q330->share.accmstats[AC_COMEFF].accum = (1000.0 * sentdif) / (double)(sentdif + resdif) + 0.5 ;
        q330->last_sent_count = q330->share.stat_log.sent ;
        q330->last_resent_count = q330->share.stat_log.resends ;
      end
    else
      q330->share.accmstats[AC_COMEFF].accum = INVALID_ENTRY ;
  last_minute = q330->share.stat_minutes ;
  q330->share.stat_minutes = (q330->share.stat_minutes + 1) mod 60 ;
  for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
    begin
      paccm = addr(q330->share.accmstats[acctype]) ;
      paccm->minutes[last_minute] = paccm->accum ;
      if (q330->share.stat_minutes == 0)
        then
          if (acctype != AC_COMEFF)
            then
              begin /* new hour, update current hour */
                total = 0 ;
                for (minute = 0 ; minute <= 59 ; minute++)
                  total = total + paccm->minutes[minute] ;
                paccm->hours[q330->share.stat_hours] = total ;
              end
            else
              begin /* same, but take into account invalid entries */
                comeff_valids = 0 ;
                total = 0 ;
                for (minute = 0 ; minute <= 59 ; minute++)
                  if (paccm->minutes[minute] != INVALID_ENTRY)
                    then
                      begin
                        inc(comeff_valids) ;
                        total = total + paccm->minutes[minute] ;
                      end
                if (comeff_valids > 0)
                  then
                    total = lib_round((double)total / (comeff_valids / 60.0)) ;
                  else
                    total = INVALID_ENTRY ;
                paccm->hours[q330->share.stat_hours] = total ;
              end
      paccm->accum = 0 ; /* start counting the new minute as zero */
    end
  if (q330->share.stat_minutes == 0)
    then
      begin /* move to next hour */
        q330->share.stat_hours = (q330->share.stat_hours + 1) mod 24 ;
        for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
          begin
            paccm = addr(q330->share.accmstats[acctype]) ;
            paccm->hours[q330->share.stat_hours] = 0 ; /* start counting the new hour as zero */
          end
      end
  inc(q330->share.total_minutes) ;
  unlock (q330) ;
  if (q330->par_create.call_state)
    then
      begin
        q330->state_call.context = q330 ;
        q330->state_call.state_type = ST_OPSTAT ;
        q330->state_call.subtype = 0 ;
        memcpy(addr(q330->state_call.station_name), addr(q330->station_ident), sizeof(string9)) ;
        q330->state_call.info = 0 ;
        q330->state_call.subtype = 0 ;
        q330->par_create.call_state (addr(q330->state_call)) ;
      end
end

/* Splits up latitude or longitude into degrees and minutes. Input
   format is dmm.mmmmmm up to dddmm.mmmmmmm */
void splittude (pchar src_degrees, pchar minutes)
begin
  pchar tmp, tmpsave ;

  tmp = strchr (src_degrees, '.') ;
  if (tmp)
    then
      begin
        decn(tmp, 2) ; /* backup to start of minutes */
        tmpsave = tmp ; /* save it */
        do
          *minutes++ = *tmp ; /* copy to minutes buffer */
        while (*tmp++) ; /* including terminator */
        *tmpsave = 0 ; /* truncate src_degrees */
      end
    else
      minutes[0] = 0 ; /* no decimal point, invalid */
end

typedef string7 tstate_strings[6] ;
const tstate_strings state_strings = {"NONE", "OFF", "3-D", "2-D", "1-D", "COLD"} ;

void update_gps_stats (pq330 q330)
begin
  enum tgps_fix previous, newfix ;
  integer i, lat_deg, long_deg, valid, sidx ;
  double lat_min, long_min, elev ;
  float r ;
  boolean good ;
  string31 s, right ;
  char lat_s, long_s ;
  tstat_gps *pgps ;
  topstat *pops ;

  pgps = addr(q330->share.stat_gps) ; /* "with" */
  pops = addr(q330->share.opstat) ; /* ditto */
  if (pgps->gpson)
    then
      begin
        if (pops->gps_stat <= GPS_OFF_CMD)
          then
            pops->gps_stat = GPS_ON ; /* current status is wrong, change to default on state */
      end
  else if ((pops->gps_stat >= GPS_ON) land (pops->gps_stat <= GPS_ON_CMD))
    then
      pops->gps_stat = GPS_OFF ; /* current status is wrong, change to default off state */
  good = TRUE ;
  i = 1 ;
  lat_min = 0 ;
  lat_deg = 0 ;
  long_min = 0 ;
  long_deg = 0 ;
  elev = 0 ;
  lat_s = 'N' ;
  long_s = 'W' ;
  strcpy(s, addr(pgps->lat)) ;
  if (strlen(s) >= 6)
    then
      begin
        lat_s = s[strlen(s) - 1] ; /* last character  is north/south */
        s[strlen(s) - 2] = 0 ; /* remove last character */
      end
    else
      good = FALSE ;
  if (good)
    then
      begin
        splittude (s, right) ;
        if (right[0])
          then
            begin
              valid = sscanf (s, "%d", addr(lat_deg)) ;
              valid = valid + sscanf (right, "%f", addr(r)) ;
              lat_min = r ;
              if (valid != 2)
                then
                  good = FALSE ;
            end
          else
            good = FALSE ;
      end
  strcpy(s, addr(pgps->longt)) ;
  if (strlen(s) >= 6)
    then
      begin
        long_s = s[strlen(s) - 1] ;
        s[strlen(s) - 2] = 0 ;
      end
    else
      good = FALSE ;
  if (good)
    then
      begin
        splittude (s, right) ; /* split at decimal point */
        if (right[0])
          then
            begin
              valid = sscanf (s, "%d", addr(long_deg)) ;
              valid = valid + sscanf (right, "%f", addr(r)) ;
              long_min = r ;
              if (valid != 2)
                then
                  good = FALSE ;
            end
          else
            good = FALSE ;
      end
  if (good)
    then
      begin
        lat_min = lat_deg + lat_min / 60.0 ;
        if (lat_s == 'S')
          then
            lat_min = -lat_min ;
        long_min = long_deg + long_min / 60.0 ;
        if (long_s == 'W')
          then
            long_min = -long_min ;
        pops->gps_lat = lat_min ;
        pops->gps_long = long_min ;
      end
    else
      begin
        pops->gps_lat = 0 ;
        pops->gps_long = 0 ;
      end
  strcpy(s, addr(pgps->height)) ;
  good = TRUE ;
  if (strlen(s) >= 3)
    then
      begin
        s[strlen(s) - 1] = 0 ; /* remove trailing 'M' */
        valid = sscanf (s, "%f", addr(r)) ;
        elev = r ;
        if (valid != 1)
          then
            good = FALSE ;
      end
    else
      good = FALSE ;
  if (good)
    then
      pops->gps_elev = elev ;
    else
      pops->gps_elev = 0 ;
  previous = pops->gps_fix ;
  sidx = -1 ;
  for (i = 0 ; i <= 5 ; i++)
    if (strcmp(addr(pgps->fix), addr(state_strings[i])) == 0)
      then
        begin
          sidx = i ;
          break ;
        end
  if ((sidx >= 2) land (sidx <= 4) land (pops->gps_stat == GPS_COLDSTART))
    then
      pops->gps_stat = GPS_ON ; /* out of coldstart state */
  newfix = previous ;
  switch (sidx) begin
    case -1 :
    case 1 :
      switch (previous) begin
        case GPF_1D :
        case GPF_1DF :
          newfix = GPF_1DF ;
          break ;
        case GPF_2D :
        case GPF_2DF :
          newfix = GPF_2DF ;
          break ;
        case GPF_3D :
        case GPF_3DF :
          newfix = GPF_3DF ;
          break ;
        default :
          if (pgps->last_good)
            then
              newfix = GPF_OFF ;
            else
              newfix = GPF_LF ;
      end
      break ;
    case 2 :
      newfix = GPF_3D ;
      break ;
    case 3 :
      newfix = GPF_2D ;
      break ;
    case 4 :
      newfix = GPF_1D ;
      break ;
    case 0 :
    case 5 :
      if (pgps->last_good)
        then
          newfix = GPF_ON ;
        else
          newfix = GPF_NL ;
      break ;
  end
  pops->gps_fix = newfix ;
  if (pgps->last_good)
    then
      begin
        pops->gps_age = now () - pgps->last_good + 0.5 ;
        if (pops->gps_age < 0)
          then
            pops->gps_age = 0 ;
      end
    else
      pops->gps_age = -1 ;
end

/* Adds supplemental information to basic OFF/ON status */
void process_gps_phase_change (pq330 q330, byte newphase, word coldreason)
begin
  enum tgps_stat gpstat ;

  gpstat = q330->share.opstat.gps_stat ;
  switch (newphase) begin
    case GPS_OFFLOCK : gpstat = GPS_OFF_LOCK ; break ;
    case GPS_OFFPLL : gpstat = GPS_OFF_PLL ; break ;
    case GPS_OFFMAX : gpstat = GPS_OFF_LIMIT ; break ;
    case GPS_OFFCMD : gpstat = GPS_OFF_CMD ; break ;
    case GPS_COLD : gpstat = GPS_COLDSTART ; break ;
    case GPS_ONAUTO : gpstat = GPS_ON_AUTO ; break ;
    case GPS_ONCMD : gpstat = GPS_ON_CMD ; break ;
  end
  q330->share.opstat.gps_stat = gpstat ;
end

void update_op_stats (pq330 q330)
begin
  paqstruc paqs ;
  enum tacctype acctype ;
  integer lastminute, minute, hour, valids, comeff_valids ;
  longint total ;
  taccmstat *paccm ;
  topstat *pops ;

  paqs = q330->aqstruc ;
  pops = addr(q330->share.opstat) ;
  if (q330->saved_data_timetag > 1)
    then
      pops->data_latency = now () - q330->saved_data_timetag + 0.5 ;
    else
      pops->data_latency = INVALID_LATENCY ;
  if (q330->last_status_received != 0)
    then
      pops->status_latency = now () - q330->last_status_received + 0.5 ;
    else
      pops->status_latency = INVALID_LATENCY ;
  memcpy(addr(pops->station_name), addr(q330->station_ident), sizeof(string9)) ;
  pops->station_port = q330->par_create.q330id_dataport ;
  pops->station_tag = q330->share.fixed.property_tag ;
  memcpy(addr(pops->station_serial), addr(q330->share.fixed.sys_num), sizeof(t64)) ;
  pops->station_reboot = q330->share.fixed.last_reboot ;
  pops->timezone_offset = q330->zone_adjust ;
  pops->calibration_errors = paqs->calerr_bitmap ;
  lastminute = q330->share.stat_minutes - 1 ;
  if (lastminute < 0)
    then
      lastminute = 59 ;
  for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
    begin
      paccm = addr(q330->share.accmstats[acctype]) ;
      if (q330->share.total_minutes >= 1)
        then
          switch (acctype) begin
            case AC_READ :
            case AC_WRITE :
              pops->accstats[acctype][AD_MINUTE] = paccm->minutes[lastminute] div 60 ;
              break ;
            case AC_DUTY :
              pops->accstats[acctype][AD_MINUTE] = (paccm->minutes[lastminute] * 1000) div 60 ;
              break ;
            case AC_THROUGH :
              pops->accstats[acctype][AD_MINUTE] = (paccm->minutes[lastminute] * 100) div 60 ;
              break ;
            default :
              pops->accstats[acctype][AD_MINUTE] = paccm->minutes[lastminute] ;
              break ;
          end
        else
          pops->accstats[acctype][AD_MINUTE] = INVALID_ENTRY ;
      valids = q330->share.total_minutes ;
      if (valids > 60)
        then
          valids = 60 ;
      total = 0 ;
      comeff_valids = 0 ;
      for (minute = 0 ; minute <= valids - 1 ; minute++)
        begin
          if (acctype == AC_COMEFF)
            then
              begin
                if (paccm->minutes[minute] != INVALID_ENTRY)
                  then
                    begin
                      inc(comeff_valids) ;
                      total = total + paccm->minutes[minute] ;
                    end
              end
            else
              total = total + paccm->minutes[minute] ;
        end
      if (valids == 0)
        then
          total = INVALID_ENTRY ;
      pops->minutes_of_stats = valids ;
      if (total == INVALID_ENTRY)
        then
          pops->accstats[acctype][AD_HOUR] = total ;
        else
          switch (acctype) begin
            case AC_READ :
            case AC_WRITE :
              pops->accstats[acctype][AD_HOUR] = total div (60 * valids) ;
              break ;
            case AC_DUTY :
              pops->accstats[acctype][AD_HOUR] = (total * 1000) div (60 * valids) ;
              break ;
            case AC_THROUGH :
              pops->accstats[acctype][AD_HOUR] = (total * 100) div (60 * valids) ;
              break ;
            case AC_COMEFF :
              if (comeff_valids == 0)
                then
                  begin
                    pops->accstats[acctype][AD_HOUR] = INVALID_ENTRY ;
                    total = INVALID_ENTRY ;
                  end
                else
                  begin
                    pops->accstats[acctype][AD_HOUR] = total div comeff_valids ;
                    total = lib_round((double)total / (comeff_valids / 60.0)) ;
                  end
              break ;
            default :
              pops->accstats[acctype][AD_HOUR] = total ;
              break ;
          end
      valids = q330->share.total_minutes div 60 ; /* hours */
      if (valids >= 24)
        then
          valids = 24 ;
        else
          total = 0 ;
      comeff_valids = 0 ;
      for (hour = 0 ; hour <= valids - 1 ; hour++)
        begin
          if (acctype == AC_COMEFF)
            then
              begin
                if ((hour == q330->share.stat_hours) land (total != INVALID_ENTRY))
                  then
                    inc(comeff_valids) ;
                else if ((hour != q330->share.stat_hours) land (paccm->hours[hour] != INVALID_ENTRY))
                  then
                    begin
                      inc(comeff_valids) ;
                      total = total + paccm->hours[hour] ;
                    end
              end
            else
              total = total + paccm->hours[hour] ;
        end
      if (valids == 0)
        then
          total = INVALID_ENTRY ; /* need at least one hour to extrapolate */
      pops->hours_of_stats = valids ;
      if (total == INVALID_ENTRY)
        then
          pops->accstats[acctype][AD_DAY] = total ;
        else
          switch (acctype) begin
            case AC_READ :
            case AC_WRITE :
              pops->accstats[acctype][AD_DAY] = total div (3600 * valids) ;
              break ;
            case AC_DUTY :
              pops->accstats[acctype][AD_DAY] = (total * 1000) div (3600 * valids) ;
              break ;
            case AC_THROUGH :
              pops->accstats[acctype][AD_DAY] = (total * 100) div (3600 * valids) ;
              break ;
            case AC_COMEFF :
              if (comeff_valids == 0)
                then
                  pops->accstats[acctype][AD_DAY] = INVALID_ENTRY ;
                else
                  pops->accstats[acctype][AD_DAY] = total div (60 * comeff_valids) ;
              break ;
            default :
              pops->accstats[acctype][AD_DAY] = total ;
              break ;
          end
    end
end
