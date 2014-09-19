/*   Lib330 MD5 Routines
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
    0 2006-09-30 rdr Created
    1 2006-10-26 rdr Don't repeat pass 2 of restore_thread_continuity if already
                     have DP LCQ's.
    2 2006-11-28 rdr Save and restore backup timetag and quality for DA LCQ's.
                     Remove setting last_valid, no longer used.
    3 2006-11-29 rdr Initialize communications efficiency arrays to INVALID_ENTRY
                     to avoid dilution using new algorithm.
    4 2007-03-05 rdr Add purge_continuity and purge_thread_continuity.
*/
#ifndef libcont_h
#include "libcont.h"
#endif

#ifndef libseed_h
#include "libseed.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libstats_h
#include "libstats.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libslider_h
#include "libslider.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libtokens_h
#include "libtokens.h"
#endif
#ifndef OMIT_SEED
#ifndef libdetect_h
#include "libdetect.h"
#endif
#endif

#ifdef OMIT_SEED
#define CT_VER 48 /* RELEASE AT 50 */
#else
#define CT_VER 98 /* RELEASE AT 100 */
#endif
#define CTY_STATIC 0 /* Static storage for status, etc */
#define CTY_SYSTEM 1 /* system identification */
#ifndef OMIT_SEED
#define CTY_IIR 2 /* IIR filter */
#define CTY_FIR 3 /* FIR filter */
#define CTY_AVG 4 /* Averaging */
#endif
#define CTY_LCQ 5 /* A LCQ */
#ifndef OMIT_SEED
#define CTY_RING 6 /* Ring Buffer Entry */
#define CTY_MH 7 /* Murdock Hutt Detector */
#define CTY_THR 8 /* Threshold Detector */
#endif
#define CTY_DPLCQ 9 /* DP LCQ */
#define CTY_PURGED 86 /* Continuity file already used */

typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
} tctyhdr ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  byte version ;
  tseed_net net ;
  tseed_stn stn ; /* these make sure nothing tricky has happened */
  boolean auto_adjust ; /* restore timezone adjustment from continuity file */
  longint timezone_adjust ; /* current timezone offset */
  integer mem_required ; /* amount of memory required */
  longint time_written ; /* seconds since 2000 that this was written */
  integer stat_minutes ; /* minutes worth of info will go in this slot */
  integer stat_hours ; /* hours worth will go into this slot */
  longint total_minutes ; /* total minutes accumulated */
  double timetag_save ; /* for the purposes of calculating data latency */
  double last_status_save ; /* for the purposes of calculating status latency */
  longint tag_save ; /* tagid save */
  t64 sn_save ; /* serial number */
  longword reboot_save ; /* reboot time save */
  topstat opstat ; /* snapshot of operational status */
  taccmstats accmstats ; /* snapshot of opstat generation information */
} tstatic ;
typedef tstatic *pstatic ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  byte version ;
  t64 serial ; /* serial number of q330 */
  double lasttime ; /* data_timetag of last second of data */
  word last_dataqual ; /* 0-100% */
  longword last_dataseq ; /* data record sequence. sequence continuity important too */
  longint comm_event_bitmask ;
  longword reboot_counter ; /* last reboot counter */
} tsystem ;
#ifndef OMIT_SEED
typedef struct {
  tvector x ;
  tvector y ;
} tiirvalues ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  char fn[FILTER_NAME_LENGTH] ;
  tiirvalues flt[MAXSECTIONS + 1] ; /* element 0 not used */
  tfloat outbuf ; /* this may be an array */
} tctyiir ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  word fcnt ; /* current number of samples in fir buffer */
  longint foff ; /* offset from start of buffer to "f" pointer */
  char fn[FILTER_NAME_LENGTH] ;
  tfloat fbuffer ; /* this is an array */
} tctyfir ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  tavg_packet average ;
} tctyavg ;
#endif
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  longword lastdtsequence ; /* last data record sequence processed */
  dholdqtype multbuf ; /* holding buffer for out-of-order DC_MULT packet */
  integer prev_rate ; /* rate when continuity written */
  tfloat prev_delay ; /* delay when continuity written */
#ifndef OMIT_SEED
  boolean glast ; /* gen_last_on */
  boolean con ; /* cal_on */
  boolean cstat ; /* calstat */
  boolean overwrite_slipping ;
  byte qpad ;
  word cinc ; /* calinc */
  longint rec_written ; /* records written */
  longint arec_written ; /* archive records written */
  longword gnext ; /* gen_next */
  longint last_sample ; /* last compression sample */
  double nextrec_tag ; /* this is the expected starting time of the next record */
  double lastrec_tag ; /* this was the last timetag used */
  double backup_timetag ;
  word backup_timequal ;
#endif
} tctylcq ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  byte dp_src ; /* selects among the DP Statistics */
  longword lcq_options ;
  integer frame_limit ;
  single gap_thresh ;
#ifndef OMIT_SEED
  longint rec_written ; /* records written */
  longint arec_written ; /* archive records written */
  longint last_sample ; /* last compression sample */
  double nextrec_tag ; /* this is the expected starting time of the next record */
  double lastrec_tag ; /* this was the last timetag used */
#endif
} tctydplcq ;
#ifndef OMIT_SEED
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  word spare ; /* for longword alignment */
  completed_record comprec ;
} tctyring ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  char dn[DETECTOR_NAME_LENGTH] ;
  con_sto mh_cont ;
} tctymh ;
typedef struct {
  longint crc ; /* CRC of everything following */
  word id ; /* what kind of entry */
  word size ; /* size of this entry */
  tlocation loc ;
  tseed_name name ;
  byte lpad ;
  char dn[DETECTOR_NAME_LENGTH] ;
  threshold_control_struc thr_cont ;
} tctythr ;
#endif

static void process_status (pq330 q330, tstatic *statstor) /* static is reserved word in C, change name */
begin
#define MINS_PER_DAY 1440
  longint tdiff, useable, new_total ;
  enum tacctype acctype ;
  integer hour, dhour, uhours, thours, min, dmin, umins, tmins ;
  taccmstat *paccm ;

  min = lib_round(now()) ;
  tdiff = (min - statstor->time_written) div 60 ; /* get minutes difference */
  if (tdiff >= MINS_PER_DAY)
    then
      return ; /* a day old, can't use anything */
  new_total = statstor->total_minutes + tdiff ; /* new status time period */
  if (new_total > MINS_PER_DAY)
    then
      new_total = MINS_PER_DAY ;
  useable = new_total - tdiff ;
  if (useable <= 0)
    then
      return ; /* don't have enough data to fill the gap */
  /* clear out accumulators */
  for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
    begin
      paccm = addr(statstor->accmstats[acctype]) ;
      paccm->accum = 0 ;
      paccm->accum_ds = 0 ;
      paccm->ds_lcq = NIL ;
    end
  /* initialize communications efficiency to INVALID_ENTRY */
  for (hour = 0 ; hour <= 23 ; hour++)
    q330->share.accmstats[AC_COMEFF].hours[hour] = INVALID_ENTRY ;
  for (min = 0 ; min <= 59 ; min++)
    q330->share.accmstats[AC_COMEFF].minutes[min] = INVALID_ENTRY ;
  if (useable > MINS_PER_DAY)
    then
      useable = MINS_PER_DAY ;
  thours = new_total div 60 ; /* total hours */
  tmins = new_total - (thours * 60) ;
  q330->share.total_minutes = new_total ;
  q330->share.stat_minutes = tmins ;
  q330->share.stat_hours = thours ;
  if (q330->share.stat_hours > 23)
    then
      q330->share.stat_hours = 0 ; /* wrap the day */
  uhours = useable div 60 ; /* useable old hours */
  umins = useable - (uhours * 60) ;
  if (uhours > 0)
    then
      begin /* have at least an hour, copy the hours plus the entire minutes */
        /* transfer in hour entries */
        hour = statstor->stat_hours - 1 ; /* last useable data */
        dhour = thours - 1 ; /* new last useable data */
        while (uhours > 0)
          begin
            if (hour < 0)
              then
                hour = 23 ;
            if (dhour < 0)
              then
                dhour = 23 ;
            for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
              q330->share.accmstats[acctype].hours[dhour] = statstor->accmstats[acctype].hours[hour] ;
            dec(uhours) ;
            dec(hour) ;
            dec(dhour) ;
          end
        for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
          memcpy(addr(q330->share.accmstats[acctype].minutes), addr(statstor->accmstats[acctype].minutes), sizeof(taccminutes)) ;
      end
    else
      begin
        /* transfer in minutes entries */
        min = statstor->stat_minutes - 1 ;
        dmin = tmins - 1 ; /* new last useable */
        while (umins > 0)
          begin
            if (min < 0)
              then
                min = 59 ;
            if (dmin < 0)
              then
                dmin = 59 ;
            for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
              q330->share.accmstats[acctype].minutes[dmin] = statstor->accmstats[acctype].minutes[min] ;
            dec(umins) ;
            dec(min) ;
            dec(dmin) ;
          end
      end
  memcpy(addr(q330->station_ident), addr(statstor->opstat.station_name), sizeof(string9)) ;
end

void restore_thread_continuity (pq330 q330, boolean pass1, string *result)
begin
  tstatic statstor ;
  tfile_handle cf ;
  integer loops, next ;
  paqstruc paqs ;
  tctydplcq *pdlsrc ;
  plcq cur_lcq ;
  string fname ;

  if (result)
   then
     (*result)[0] = 0 ;
  paqs = q330->aqstruc ;
  if ((lnot pass1) land (paqs->dplcqs))
    then
      return ; /* we already did this */
  if (q330->par_create.opt_contfile[0] == 0)
    then
      return ; /*don't want a file */
  strcpy(fname, q330->par_create.opt_contfile) ;
  strcat(fname, "t") ;
  cf = lib_file_open (addr(fname), LFO_OPEN or LFO_READ) ;
  if (cf == INVALID_FILE_HANDLE)
    then
      return ;
  if (pass1)
    then
      begin
        lib_file_read (cf, addr(statstor), sizeof(tstatic)) ;
        lib_file_close (cf) ;
        if (statstor.id != CTY_STATIC)
          then
            begin
              if (result)
                then
                  strcpy(result, "Thread Continuity Format Mis-match") ;
              lib_file_delete (addr(fname)) ;
              return ;
            end
        if (statstor.version != CT_VER)
          then
            begin
              if (result)
                then
                  sprintf(result, "Thread Continuity Version Mis-match, Got=%d, Expected=%d", statstor.version, CT_VER) ;
              lib_file_delete (addr(fname)) ;
              return ;
            end
        if (statstor.crc != gcrccalc (addr(q330->crc_table), (pointer)((integer)addr(statstor) + 4), sizeof(tstatic) - 4))
          then
            begin
              if (result)
                then
                  strcpy(result, "Thread Continuity CRC Error, Ignoring rest of file") ;
              lib_file_delete (addr(fname)) ;
              return ;
            end
        memcpy(addr(q330->network), addr(statstor.net), sizeof(tseed_net)) ;
        memcpy(addr(q330->station), addr(statstor.stn), sizeof(tseed_stn)) ;
        q330->cur_memory_required = statstor.mem_required ;
        q330->saved_data_timetag = statstor.timetag_save ;
        q330->last_status_received = statstor.last_status_save ;
        q330->share.fixed.property_tag = statstor.tag_save ;
        memcpy(addr(q330->share.fixed.sys_num), addr(statstor.sn_save), sizeof(t64)) ;
        q330->share.fixed.last_reboot = statstor.reboot_save ;
        if (statstor.auto_adjust)
          then
            q330->zone_adjust = statstor.timezone_adjust ;
        process_status (q330, addr(statstor)) ;
      end
    else
      begin
        lib_file_seek (cf, sizeof(tstatic)) ; /* skip this */
        loops = 0 ;
        next = sizeof(tstatic) ;
        repeat
          if (lib_file_read (cf, q330->cbuf, sizeof(tctydplcq)))
            then
              break ;
          pdlsrc = (pointer)q330->cbuf ;
          next = next + pdlsrc->size ; /* next record */
          if (pdlsrc->crc != gcrccalc (addr(q330->crc_table), (pointer)((integer)pdlsrc + 4), pdlsrc->size - 4))
            then
              begin
                libmsgadd (q330, LIBMSG_CONCRC, "Thread") ;
                lib_file_close (cf) ;
                return ;
              end
          if (pdlsrc->id != CTY_DPLCQ)
            then
              begin
                libmsgadd (q330, LIBMSG_CONPURGE, "Thread") ;
                lib_file_close (cf) ;
                return ;
              end
          cur_lcq = malloc(sizeof(tlcq)) ;
          memset (cur_lcq, 0, sizeof(tlcq)) ;
          if (paqs->dplcqs == NIL)
            then
              paqs->dplcqs = cur_lcq ;
            else
              paqs->dplcqs = extend_link (paqs->dplcqs, cur_lcq) ;
          memcpy(addr(cur_lcq->location), addr(pdlsrc->loc), sizeof(tlocation)) ;
          memcpy(addr(cur_lcq->seedname), addr(pdlsrc->name), sizeof(tseed_name)) ;
          set_loc_name (cur_lcq) ;
          cur_lcq->validated = TRUE ; /* unless new tokens remove it */
          cur_lcq->raw_data_source = DC_DPSTAT ;
          cur_lcq->raw_data_field = pdlsrc->dp_src ;
          cur_lcq->lcq_num = 0xFF ; /* flag as not indexed */
          cur_lcq->lcq_opt = pdlsrc->lcq_options ;
          cur_lcq->rate = -10 ;
          cur_lcq->gap_threshold = pdlsrc->gap_thresh ;
          if (cur_lcq->gap_threshold == 0.0)
            then
              cur_lcq->gap_threshold = 0.5 ;
         set_gaps (cur_lcq) ;
#ifndef OMIT_SEED
          cur_lcq->firfixing_gain = 1.000 ; /* default if not over-ridden */
          cur_lcq->com = malloc(sizeof(tcom_packet)) ;
          memset (cur_lcq->com, 0, sizeof(tcom_packet)) ;
          cur_lcq->com->frame = 1 ;
          cur_lcq->com->next_compressed_sample = 1 ;
          cur_lcq->com->maxframes = pdlsrc->frame_limit ;
          cur_lcq->com->records_written = pdlsrc->rec_written ;
          cur_lcq->com->last_sample = pdlsrc->last_sample ;
          cur_lcq->backup_tag = pdlsrc->nextrec_tag ;
          cur_lcq->last_timetag = 0 ; /* Expecting a gap, don't report */
          cur_lcq->arc.records_written = pdlsrc->arec_written ;
#endif
          lib_file_seek (cf, next) ;
          inc(loops) ;
        until (loops > 9999)) ;
        lib_file_close (cf) ;
        init_dplcqs (q330->aqstruc) ;
      end
end

void save_thread_continuity (pq330 q330)
begin
  tctydplcq *pdldest ;
  pstatic pstat ;
  plcq q ;
  paqstruc paqs ;
  tfile_handle cf ;
  string fname ;

  paqs = q330->aqstruc ;
  if (q330->par_create.opt_contfile[0] == 0)
    then
      return ; /*don't want a file */
  strcpy(fname, q330->par_create.opt_contfile) ;
  strcat(fname, "t") ;
  cf = lib_file_open (addr(fname), LFO_CREATE or LFO_WRITE) ;
  if (cf == INVALID_FILE_HANDLE)
    then
      return ;
  pstat = (pointer)q330->cbuf ;
  pstat->id = CTY_STATIC ;
  pstat->size = sizeof(tstatic) ;
  pstat->version = CT_VER ;
  memcpy(addr(pstat->net), addr(q330->network), sizeof(tseed_net)) ;
  memcpy(addr(pstat->stn), addr(q330->station), sizeof(tseed_stn)) ;
  pstat->timezone_adjust = q330->zone_adjust ;
  pstat->auto_adjust = (q330->par_create.opt_zoneadjust != 0) ;
  lock (q330) ;
  pstat->time_written = lib_round(now()) ;
  pstat->stat_minutes = q330->share.stat_minutes ;
  pstat->stat_hours = q330->share.stat_hours ;
  pstat->total_minutes = q330->share.total_minutes ;
  pstat->timetag_save = q330->saved_data_timetag ;
  pstat->last_status_save = q330->last_status_received ;
  pstat->tag_save = q330->share.fixed.property_tag ;
  memcpy(addr(pstat->sn_save), addr(q330->share.fixed.sys_num), sizeof(t64)) ;
  pstat->reboot_save = q330->share.fixed.last_reboot ;
  memcpy(addr(pstat->opstat), addr(q330->share.opstat), sizeof(topstat)) ;
  memcpy(addr(pstat->accmstats), addr(q330->share.accmstats), sizeof(taccmstats)) ;
  unlock (q330) ;
  pstat->mem_required = q330->cur_memory_required ;
  pstat->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)pstat + 4), sizeof(tstatic) - 4) ;
  lib_file_write (cf, pstat, sizeof(tstatic)) ;
  q = paqs->dplcqs ;
  while (q)
    begin
      pdldest = (pointer)q330->cbuf ;
      if (q->validated)
        then
          begin /* wasn't removed by new tokens */
            pdldest->id = CTY_DPLCQ ;
            pdldest->size = sizeof(tctydplcq) ;
            memcpy(addr(pdldest->loc), addr(q->location), sizeof(tlocation)) ;
            memcpy(addr(pdldest->name), addr(q->seedname), sizeof(tseed_name)) ; ;
            pdldest->lpad = 0 ;
            pdldest->dp_src = q->raw_data_field ;
            pdldest->lcq_options = q->lcq_opt ;
            pdldest->gap_thresh = q->gap_threshold ;
#ifndef OMIT_SEED
            pdldest->frame_limit = q->com->maxframes ;
            pdldest->rec_written = q->com->records_written ;
            pdldest->arec_written = q->arc.records_written ;
            pdldest->last_sample = q->com->last_sample ;
            pdldest->nextrec_tag = q->backup_tag ;
            pdldest->lastrec_tag = q->last_timetag ;
#endif
            pdldest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)pdldest + 4), pdldest->size - 4) ;
            lib_file_write (cf, pdldest, pdldest->size) ;
          end
      q = q->link ;
    end
  lib_file_close (cf) ;
end

void check_continuity (pq330 q330)
begin
  tsystem system ;
  paqstruc paqs ;
  longword newreboots ;
  tfile_handle cf ;
  string fname, s ;

  paqs = q330->aqstruc ;
  if (q330->par_create.opt_contfile[0] == 0)
    then
      return ; /*don't want a file */
  strcpy(fname, q330->par_create.opt_contfile) ;
  strcat(fname, "q") ;
  cf = lib_file_open (addr(fname), LFO_OPEN or LFO_READ) ;
  if (cf == INVALID_FILE_HANDLE)
    then
      return ;
  lib_file_read (cf, addr(system), sizeof(tsystem)) ;
  lib_file_close (cf) ;
  if (system.version != CT_VER)
    then
      begin
        sprintf(s, "Version Mis-match, Got=%d, Expected=%d", system.version, CT_VER) ;
        libmsgadd (q330, LIBMSG_CONTIN, addr(s)) ;
        return ;
      end
  if (system.crc != gcrccalc (addr(q330->crc_table), (pointer)((integer)addr(system) + 4), sizeof(tsystem) - 4))
    then
      begin
        libmsgadd (q330, LIBMSG_CONCRC, "Q330") ;
        return ;
      end
  if (system.id != CTY_SYSTEM)
    then
      begin
        libmsgadd (q330, LIBMSG_CONPURGE, "Q330") ;
        return ;
      end
  if ((system.serial[0] != q330->par_create.q330id_serial[0]) lor (system.serial[1] != q330->par_create.q330id_serial[1]))
    then
      return ;
  lock (q330) ;
  newreboots = q330->share.fixed.reboots ;
  unlock (q330) ;
  if (system.reboot_counter != newreboots)
    then
      begin
        sprintf(s, "%d Q330 Reboot(s)", newreboots - system.reboot_counter) ;
        libmsgadd (q330, LIBMSG_CONTBOOT, addr(s)) ;
        add_status (q330, AC_BOOTS, newreboots - system.reboot_counter) ;
        return ;
      end
  paqs->data_qual = system.last_dataqual ;
  paqs->dt_data_sequence = system.last_dataseq ;
  paqs->data_timetag = system.lasttime ;
  sprintf(s, "%d %s Q=%d", paqs->dt_data_sequence, realtostr(system.lasttime, 6, addr(fname)), paqs->data_qual) ;
  libdatamsg (q330, LIBMSG_CONTFND, addr(s)) ;
end

boolean restore_continuity (pq330 q330)
begin
  integer next, loops, sz ;
  plcq q ;
  tsystem *psystem ;
  tctylcq *plsrc ;
#ifndef OMIT_SEED
  tctyiir *psrc ;
  tctyfir *pfsrc ;
  tctyavg *pasrc ;
  tctyring *prsrc ;
  piirfilter pdest ;
  tctymh *pmsrc ;
  tctythr *ptsrc ;
  pdet_packet pdp ;
  plong pl ;
  con_sto *pmc ;
  threshold_control_struc *pmt ;
  integer points ;
#endif
  integer i ;
  longint bm ;
  paqstruc paqs ;
  tfile_handle cf ;
  string fname ;
  pbyte p ;

  paqs = q330->aqstruc ;
  if (q330->par_create.opt_contfile[0] == 0)
    then
      return FALSE ; /*don't want a file */
  strcpy(fname, q330->par_create.opt_contfile) ;
  strcat(fname, "q") ;
  cf = lib_file_open (addr(fname), LFO_OPEN or LFO_READ) ;
  if (cf == INVALID_FILE_HANDLE)
    then
      return FALSE ;
  psystem = (pointer)q330->cbuf ;
  lib_file_read (cf, psystem, sizeof(tsystem)) ;
  bm = psystem->comm_event_bitmask ;
  for (i = 0 ; i <= CE_MAX - 1 ; i++)
    if (bm and ((longint)1 shl i))
      then
        paqs->commevents[i].ison = TRUE ;
      else
        paqs->commevents[i].ison = FALSE ;
  next = sizeof(tsystem) ;
  loops = 0 ;
  plsrc = (pointer)q330->cbuf ;
  repeat
    if (lib_file_read (cf, q330->cbuf, sizeof(tctyhdr)))
      then
        break ;
    sz = plsrc->size ;
    next = next + sz ; /* next record */
    p = (pointer)q330->cbuf ;
    incn(p, sizeof(tctyhdr)) ; /* skip part we already read */
    if (lib_file_read (cf, p, sz - sizeof(tctyhdr)))
      then
        break ;
    if (plsrc->crc != gcrccalc (addr(q330->crc_table), (pointer)((integer)plsrc + 4), sz - 4))
      then
        begin
          libmsgadd (q330, LIBMSG_CONCRC, "Q330") ;
          lib_file_close (cf) ;
          return FALSE ;
        end
    plsrc = (pointer)q330->cbuf ;
#ifndef OMIT_SEED
    psrc = (pointer)q330->cbuf ;
    pfsrc = (pointer)q330->cbuf ;
    pasrc = (pointer)q330->cbuf ;
    prsrc = (pointer)q330->cbuf ;
    pmsrc = (pointer)q330->cbuf ;
    ptsrc = (pointer)q330->cbuf ;
#endif
    switch (plsrc->id) begin
#ifndef OMIT_SEED
      case CTY_IIR :
        q = paqs->lcqs ;
        pdest = NIL ;
        while (q)
          if ((memcmp(addr(q->location), addr(psrc->loc), 2) == 0) land
              (memcmp(addr(q->seedname), addr(psrc->name), 3) == 0))
            then
              begin
                pdest = q->stream_iir ;
                break ;
              end
            else
              q = q->link ;
        while (pdest)
          if (strcmp(addr(pdest->def->fname), addr(psrc->fn)) == 0)
            then
              begin
                if (q->rate > 0)
                  then
                    points = q->rate ;
                  else
                    points = 1 ;
                for (i = 1 ; i <= pdest->sects ; i++)
                  begin
                    memcpy(addr(pdest->filt[i].x), addr(psrc->flt[i].x), sizeof(tvector)) ;
                    memcpy(addr(pdest->filt[i].y), addr(psrc->flt[i].y), sizeof(tvector)) ;
                  end
                memcpy (addr(pdest->out), addr(psrc->outbuf), sizeof(tfloat) * points) ;
                break ;
              end
            else
              pdest = pdest->link ;
        break ;
      case CTY_FIR :
        q = paqs->lcqs ;
        while (q)
          if ((memcmp(addr(q->location), addr(pfsrc->loc), 2) == 0) land
              (memcmp(addr(q->seedname), addr(pfsrc->name), 3) == 0) land
              (q->source_fir) land (strcmp(addr(q->source_fir->fname), addr(pfsrc->fn)) == 0))
            then
              begin
                q->fir->fcount = pfsrc->fcnt ;
                q->fir->f = (pointer)((integer)q->fir->fbuf + pfsrc->foff) ;
                memcpy (q->fir->fbuf, addr(pfsrc->fbuffer), q->fir->flen * sizeof(tfloat)) ;
                q->com->charging = FALSE ; /* not any more */
                break ;
              end
            else
              q = q->link ;
        break ;
      case CTY_AVG :
        q = paqs->lcqs ;
        while (q)
          if ((memcmp(addr(q->location), addr(pasrc->loc), 2) == 0) land
              (memcmp(addr(q->seedname), addr(pasrc->name), 3) == 0) land (q->avg))
            then
              begin
                memcpy (q->avg, addr(pasrc->average), sizeof(tavg_packet)) ;
                break ;
              end
            else
              q = q->link ;
        break ;
#endif
      case CTY_LCQ :
        q = paqs->lcqs ;
        while (q)
          if ((memcmp(addr(q->location), addr(plsrc->loc), 2) == 0) land
              (memcmp(addr(q->seedname), addr(plsrc->name), 3) == 0))
            then
              begin
                if (q->rate == 0)
                  then
                    begin /* use backup values */
                      q->rate = plsrc->prev_rate ;
                      q->delay = plsrc->prev_delay ;
                    end
                if (q->dholdq) /* if dholdq non-nil, this lcq has a holding q */
                  then
                    begin
                      memcpy (q->dholdq, addr(plsrc->multbuf), sizeof(dholdqtype)) ;
                      if (q->dholdq->ppkt) /* if non-nil, ppkt signals a valid packet */
                        then
                          q->dholdq->ppkt = addr(q->dholdq->pkt) ;
                    end
                q->dtsequence = plsrc->lastdtsequence ;
#ifndef OMIT_SEED
                q->gen_last_on = plsrc->glast ;
                q->cal_on = plsrc->con ;
                q->calstat = plsrc->cstat ;
                q->calinc = plsrc->cinc ;
                q->com->records_written = plsrc->rec_written ;
                q->arc.records_written = plsrc->arec_written ;
                q->gen_next = plsrc->gnext ;
                q->com->last_sample = plsrc->last_sample ;
                q->backup_tag = plsrc->nextrec_tag ;
                q->last_timetag = plsrc->lastrec_tag ;
                q->slipping = plsrc->overwrite_slipping ;
                q->backup_tag = plsrc->backup_timetag ;
                q->backup_qual = plsrc->backup_timequal ;
#endif
                break ;
              end
            else
              q = q->link ;
        break ;
#ifndef OMIT_SEED
      case CTY_RING :
        q = paqs->lcqs ;
        while (q)
          if ((memcmp(addr(q->location), addr(prsrc->loc), 2) == 0) land
              (memcmp(addr(q->seedname), addr(prsrc->name), 3) == 0))
            then
              begin
                q->com->ring->full = TRUE ;
                memcpy (addr(q->com->ring->rec), addr(prsrc->comprec), LIB_REC_SIZE) ;
                q->com->last_in_ring = q->com->ring ; /* last one we filled */
                q->com->ring = q->com->ring->link ;
                break ;
              end
            else
              q = q->link ;
        break ;
      case CTY_MH :
        q = paqs->lcqs ;
        pdp = NIL ;
        while (q)
          if ((memcmp(addr(q->location), addr(pmsrc->loc), 2) == 0) land
              (memcmp(addr(q->seedname), addr(pmsrc->name), 3) == 0))
            then
              begin
                pdp = q->det ;
                break ;
              end
            else
              q = q->link ;
        while (pdp)
          if (strcmp(addr(pdp->detector_def->detname), addr(pmsrc->dn)) == 0)
            then
              begin
                pmc = pdp->cont ;
                memcpy (pmc, addr(pmsrc->mh_cont), (integer)addr(pmc->onsetdata) - (integer)pmc) ;
                if ((pdp->insamps) land (pmsrc->size > sizeof(tctymh)))
                  then
                    begin
                      pl = (pointer)((integer)pmsrc + sizeof(tctymh)) ;
                      memcpy (pdp->insamps, pl, pdp->insamps_size) ;
                    end
                if ((pmc->default_enabled) != (pdp->det_options and DO_RUN))
                  then
                    begin
                      pmc->default_enabled = (pdp->det_options and DO_RUN) ;
                      pmc->detector_enabled = (pdp->det_options and DO_RUN) ;
                    end
                break ;
              end
            else
              pdp = pdp->link ;
        break ;
      case CTY_THR :
        q = paqs->lcqs ;
        pdp = NIL ;
        while (q)
          if ((memcmp(addr(q->location), addr(ptsrc->loc), 2) == 0) land
              (memcmp(addr(q->seedname), addr(ptsrc->name), 3) == 0))
            then
              begin
                pdp = q->det ;
                break ;
              end
            else
              q = q->link ;
        while (pdp)
          if (strcmp(addr(pdp->detector_def->detname), addr(ptsrc->dn)) == 0)
            then
              begin
                pmt = pdp->cont ;
                memcpy (pmt, addr(ptsrc->thr_cont), (integer)addr(pmt->onsetdata) - (integer)pmt) ;
                if ((pmt->default_enabled) != (pdp->det_options and DO_RUN))
                  then
                    begin
                      pmt->default_enabled = (pdp->det_options and DO_RUN) ;
                      pmt->detector_enabled = (pdp->det_options and DO_RUN) ;
                    end
                break ;
              end
            else
              pdp = pdp->link ;
        break ;
#endif
    end
    lib_file_seek (cf, next) ;
    inc(loops) ;
  until (loops > 9999)) ;
  lib_file_close (cf) ;
  return TRUE ;
end

void save_continuity (pq330 q330)
begin
  tsystem *psystem ;
  tctylcq *pldest ;
#ifndef OMIT_SEED
  tctyiir *pdest ;
  piirfilter psrc ;
  tctyfir *pfdest ;
  tctyavg *padest ;
  tctyring *prdest ;
  tctymh *pmdest ;
  tctythr *ptdest ;
  pcompressed_buffer_ring pr ;
  pdet_packet pdp ;
  con_sto *pmc ;
  threshold_control_struc *pmt ;
  plong pl ;
  integer points ;
#endif
  plcq q ;
  integer i ;
  longint bm ;
  paqstruc paqs ;
  tfile_handle cf ;
  string fname, s ;

  paqs = q330->aqstruc ;
  if (q330->par_create.opt_contfile[0] == 0)
    then
      return ; /*don't want a file */
  strcpy(fname, q330->par_create.opt_contfile) ;
  strcat(fname, "q") ;
  cf = lib_file_open (addr(fname), LFO_CREATE or LFO_WRITE) ;
  if (cf == INVALID_FILE_HANDLE)
    then
      return ;
  libmsgadd (q330, LIBMSG_WRCONT, "Q330") ;
  psystem = (pointer)q330->cbuf ;
  psystem->id = CTY_SYSTEM ;
  psystem->size = sizeof(tsystem) ;
  psystem->version = CT_VER ;
  memcpy(addr(psystem->serial), addr(q330->par_create.q330id_serial), sizeof(t64)) ;
  psystem->lasttime = paqs->data_timetag ;
  psystem->last_dataqual = paqs->data_qual ;
  psystem->last_dataseq = paqs->dt_data_sequence ;
  sprintf(s, "%d %s", paqs->dt_data_sequence, realtostr(psystem->lasttime, 6, addr(fname))) ;
  libmsgadd (q330, LIBMSG_CSAVE, addr(s)) ;
  bm = 0 ;
  for (i = 0 ; i <= CE_MAX - 1 ; i++)
    if (paqs->commevents[i].ison)
      then
        bm = bm or ((longint)1 shl i) ;
  psystem->comm_event_bitmask = bm ;
  lock (q330) ;
  psystem->reboot_counter = q330->share.fixed.reboots ;
  unlock (q330) ;
  psystem->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)psystem + 4), sizeof(tsystem) - 4) ;
  lib_file_write (cf, psystem, sizeof(tsystem)) ;
  q = paqs->lcqs ;
  while (q)
    begin
      pldest = (pointer)q330->cbuf ;
#ifndef OMIT_SEED
      pfdest = (pointer)q330->cbuf ;
      padest = (pointer)q330->cbuf ;
      prdest = (pointer)q330->cbuf ;
      pmdest = (pointer)q330->cbuf ;
      ptdest = (pointer)q330->cbuf ;
#endif
      pldest->id = CTY_LCQ ;
      pldest->size = sizeof(tctylcq) ;
      memcpy(addr(pldest->loc), addr(q->location), sizeof(tlocation)) ;
      memcpy(addr(pldest->name), addr(q->seedname), sizeof(tseed_name)) ;
      pldest->lpad = 0 ;
      pldest->lastdtsequence = q->dtsequence ;
      if (q->dholdq == NIL)
        then
          pldest->multbuf.ppkt = NIL ;
        else /* if dholdq non-nil, signifies holding queue exists */
          memcpy (addr(pldest->multbuf), q->dholdq, sizeof(dholdqtype)) ; /* if ppkt non-nil, ppkt signals a valid packet */
      pldest->prev_rate = q->rate ;
      pldest->prev_delay = q->delay ;
#ifndef OMIT_SEED
      pldest->glast = q->gen_last_on ;
      pldest->con = q->cal_on ;
      pldest->cstat = q->calstat ;
      pldest->cinc = q->calinc ;
      pldest->qpad = 0 ;
      pldest->rec_written = q->com->records_written ;
      pldest->arec_written = q->arc.records_written ;
      pldest->gnext = q->gen_next ;
      pldest->last_sample = q->com->last_sample ;
      pldest->nextrec_tag =q-> backup_tag ;
      pldest->lastrec_tag = q->last_timetag ;
      pldest->overwrite_slipping = q->slipping ;
      pldest->backup_timetag = q->backup_tag ;
      pldest->backup_timequal = q->backup_qual ;
#endif
      pldest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)pldest + 4), pldest->size - 4) ;
      lib_file_write (cf, pldest, pldest->size) ;
#ifndef OMIT_SEED
      if ((q->fir) land (q->source_fir))
        then
          begin
            pfdest->id = CTY_FIR ;
            pfdest->size = sizeof(tctyfir) - sizeof(tfloat) ;
            memcpy(addr(pfdest->loc), addr(q->location), sizeof(tlocation)) ;
            memcpy(addr(pfdest->name), addr(q->seedname), sizeof(tseed_name)) ;
            strcpy(addr(pfdest->fn), addr(q->source_fir->fname)) ;
            pfdest->lpad = 0 ;
            pfdest->fcnt = q->fir->fcount ;
            pfdest->foff = (integer)q->fir->f - (integer)q->fir->fbuf ;
            memcpy (addr(pfdest->fbuffer), q->fir->fbuf, q->fir->flen * sizeof (tfloat)) ;
            pfdest->size = pfdest->size + sizeof(tfloat) * q->fir->flen ;
            pfdest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)pfdest + 4), pfdest->size - 4) ;
            lib_file_write (cf, pfdest, pfdest->size) ;
          end
      if (q->avg)
        then
          begin
            padest->id = CTY_AVG ;
            padest->size = sizeof(tctyavg) ;
            memcpy(addr(padest->loc), addr(q->location), sizeof(tlocation)) ;
            memcpy(addr(padest->name), addr(q->seedname), sizeof(tseed_name)) ;
            padest->lpad = 0 ;
            memcpy(addr(padest->average), q->avg, sizeof(tavg_packet)) ;
            padest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)padest + 4), padest->size - 4) ;
            lib_file_write (cf, padest, padest->size) ;
          end
      psrc = q->stream_iir ;
      if (q->rate > 0)
        then
          points = q->rate ;
        else
          points = 1 ;
      while (psrc)
        begin
          pdest = (pointer)q330->cbuf ;
          pdest->id = CTY_IIR ;
          pdest->size = sizeof(tctyiir) - sizeof(tfloat) ; /* not counting any output buffer yet */
          memcpy(addr(pdest->loc), addr(q->location), sizeof(tlocation)) ;
          memcpy(addr(pdest->name), addr(q->seedname), sizeof(tseed_name)) ;
          strcpy(addr(pdest->fn), addr(psrc->def->fname)) ;
          pdest->lpad = 0 ;
          for (i = 1 ; i <= psrc->sects ; i++)
            begin
              memcpy(addr(pdest->flt[i].x), addr(psrc->filt[i].x), sizeof(tvector)) ;
              memcpy(addr(pdest->flt[i].y), addr(psrc->filt[i].y), sizeof(tvector)) ;
            end
          memcpy (addr(pdest->outbuf), addr(psrc->out), sizeof(tfloat) * points) ;
          pdest->size = pdest->size + sizeof(tfloat) * points ;
          pdest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)pdest + 4), pdest->size - 4) ;
          lib_file_write (cf, pdest, pdest->size) ;
          psrc = psrc->link ;
        end
      pdp = q->det ;
      while (pdp)
        begin
          if (pdp->detector_def->dtype == MURDOCK_HUTT)
            then
              begin
                pmdest->id = CTY_MH ;
                pmdest->size = sizeof(tctymh) ;
                memcpy(addr(pmdest->loc), addr(q->location), sizeof(tlocation)) ;
                memcpy(addr(pmdest->name), addr(q->seedname), sizeof(tseed_name)) ;
                strcpy(addr(pmdest->dn), addr(pdp->detector_def->detname)) ;
                pmdest->lpad = 0 ;
                pmc = pdp->cont ;
                memcpy (addr(pmdest->mh_cont), pmc, sizeof(con_sto)) ;
                if (pdp->insamps_size)
                  then
                    begin /* append contents of insamps array for low frequency stuff */
                      pmdest->size = pmdest->size + pdp->insamps_size ;
                      pl = (pointer)((integer)pmdest + sizeof(tctymh)) ;
                      memcpy (pl, pdp->insamps, pdp->insamps_size) ;
                    end
                pmdest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)pmdest + 4), pmdest->size - 4) ;
                lib_file_write (cf, pmdest, pmdest->size) ;
              end
            else
              begin
                ptdest->id = CTY_THR ;
                ptdest->size = sizeof(tctythr) ;
                memcpy(addr(ptdest->loc), addr(q->location), sizeof(tlocation)) ;
                memcpy(addr(ptdest->name), addr(q->seedname), sizeof(tseed_name)) ;
                strcpy(addr(ptdest->dn), addr(pdp->detector_def->detname)) ;
                ptdest->lpad = 0 ;
                pmt = pdp->cont ;
                memcpy (addr(ptdest->thr_cont), pmt, sizeof(threshold_control_struc)) ;
                ptdest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)ptdest + 4), ptdest->size - 4) ;
                lib_file_write (cf, ptdest, ptdest->size) ;
              end
          pdp = pdp->link ;
        end
      if (q->pre_event_buffers > 0)
        then
          begin
            pr = q->com->ring->link ;
            while (pr != q->com->ring)
              begin
                if (pr->full)
                  then
                    begin
                      prdest->id = CTY_RING ;
                      prdest->size = sizeof(tctyring) ;
                      memcpy(addr(prdest->loc), addr(q->location), sizeof(tlocation)) ;
                      memcpy(addr(prdest->name), addr(q->seedname), sizeof(tseed_name)) ;
                      prdest->lpad = 0 ;
                      prdest->spare = 0 ;
                      memcpy (addr(prdest->comprec), addr(pr->rec), LIB_REC_SIZE) ;
                      prdest->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)prdest + 4), prdest->size - 4) ;
                      lib_file_write (cf, prdest, prdest->size) ;
                    end
                pr = pr->link ;
              end
          end
#endif
      q = q->link ;
    end
  lib_file_close (cf) ;
end

void purge_continuity (pq330 q330)
begin
  tsystem *psystem ;
  tfile_handle cf ;
  string fname ;

  if (q330->par_create.opt_contfile[0] == 0)
    then
      return ; /*don't want a file */
  strcpy(fname, q330->par_create.opt_contfile) ;
  strcat(fname, "q") ;
  cf = lib_file_open (addr(fname), LFO_OPEN or LFO_READ or LFO_WRITE) ;
  if (cf == INVALID_FILE_HANDLE)
    then
      return ;
  psystem = (pointer)q330->cbuf ;
  if (lib_file_read (cf, psystem, sizeof(tsystem)))
    then
      begin
        lib_file_close (cf) ;
        return ;
      end
  psystem->id = CTY_PURGED ;
  psystem->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)psystem + 4), sizeof(tsystem) - 4) ;
  lib_file_seek (cf, 0) ;
  lib_file_write (cf, psystem, sizeof(tsystem)) ;
  lib_file_close (cf) ;
end

void purge_thread_continuity (pq330 q330)
begin
  tctydplcq *pdlsrc ;
  tfile_handle cf ;
  string fname ;

  if (q330->par_create.opt_contfile[0] == 0)
    then
      return ; /*don't want a file */
  strcpy(fname, q330->par_create.opt_contfile) ;
  strcat(fname, "t") ;
  cf = lib_file_open (addr(fname), LFO_OPEN or LFO_READ or LFO_WRITE) ;
  if (cf == INVALID_FILE_HANDLE)
    then
      return ;
  pdlsrc = (pointer)q330->cbuf ;
  lib_file_seek (cf, sizeof(tstatic)) ; /* skip this */
  if (lib_file_read (cf, pdlsrc, sizeof(tctydplcq)))
    then
      begin
        lib_file_close (cf) ;
        return ;
      end
  pdlsrc->id = CTY_PURGED ;
  pdlsrc->crc = gcrccalc (addr(q330->crc_table), (pointer)((integer)pdlsrc + 4), sizeof(tctydplcq) - 4) ;
  lib_file_seek (cf, sizeof(tstatic)) ;
  lib_file_write (cf, pdlsrc, sizeof(tctydplcq)) ;
  lib_file_close (cf) ;
end
