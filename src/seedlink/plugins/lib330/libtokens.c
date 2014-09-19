/*   Lib330 DP Token Processing
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
    0 2006-10-01 rdr Created
    1 2007-03-06 rdr Add call to purge_continuity.
    2 2007-03-12 rdr Don't purge continuity here, wait until receive data.
*/
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libcmds_h
#include "libcmds.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libslider_h
#include "libslider.h"
#endif
#ifndef q330cvrt_h
#include "q330cvrt.h"
#endif
#ifndef OMIT_SEED
#ifndef libopaque_h
#include "libopaque.h"
#endif
#ifndef libfilters_h
#include "libfilters.h"
#endif
#ifndef libctrldet_h
#include "libctrldet.h"
#endif
#ifndef libarchive_h
#include "libarchive.h"
#endif
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libcont_h
#include "libcont.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libtokens_h
#include "libtokens.h"
#endif

#ifndef OMIT_SEED
void read_iir (paqstruc paqs, pbyte *p)
begin
  piirdef cur_iir ;
  integer i ;
  pq330 q330 ;
  tsection_base *psb ;
  integer len ;

  q330 = paqs->owner ;
  getbuf (q330, addr(cur_iir), sizeof(tiirdef)) ;
  if (paqs->iirchain == NIL)
    then
      paqs->iirchain = cur_iir ;
    else
      paqs->iirchain = extend_link (paqs->iirchain, cur_iir) ;
  cur_iir->iir_num = loadbyte (p) ;
  len = loadbyte(p) ;
  loadblock (p, len, addr(cur_iir->fname)) ;
  cur_iir->fname[len] = 0 ; /* terminate string */
  cur_iir->sects = loadbyte (p) ;
  cur_iir->gain = loadsingle (p) ;
  cur_iir->rate = loadsingle (p) ;
  for (i = 1 ; i <= cur_iir->sects ; i++)
    begin
      psb = addr(cur_iir->filt[i]) ;
      psb->ratio = loadsingle (p) ;
      psb->poles = loadbyte (p) ;
      if (psb->poles and 128)
        then
          begin
            psb->poles = psb->poles and 127 ;
            psb->highpass = TRUE ;
          end
      calc_section (addr(cur_iir->filt[i])) ;
    end
end

/* upshift a C string */
static char *uppercase (pchar s)
begin
  size_t i ;

  for (i = 0 ; i < strlen(s) ; i++)
    s[i] = toupper (s[i]) ;
  return s ;
end

void read_fir (paqstruc paqs, pbyte *p)
begin
  pfilter cur_fir ;
  byte fnum ;
  string31 ucfname, ucs ;
  integer len ;

  fnum = loadbyte (p) ;
  len = loadbyte(p) ;
  loadblock (p, len, addr(ucs)) ;
  ucs[len] = 0 ;
  uppercase(addr(ucs)) ;
  cur_fir = paqs->firchain ;
  while (cur_fir)
    begin
      strcpy(ucfname, cur_fir->fname) ;
      uppercase(addr(ucfname)) ;
      if (strcmp(ucfname, ucs) == 0)
        then
          begin
            cur_fir->fir_num = fnum ;
            return ;
          end
        else
          cur_fir = cur_fir->link ;
    end
end

void read_detector (paqstruc paqs, pbyte *p, boolean mh)
begin
  pdetector cur_def ;
  tdet_mh dm ;
  pq330 q330 ;
  integer len ;

  q330 = paqs->owner ;
  getbuf (q330, addr(cur_def), sizeof(tdetector)) ;
  if (paqs->defchain == NIL)
    then
      paqs->defchain = cur_def ;
    else
      paqs->defchain = extend_link (paqs->defchain, cur_def) ;
  dm.num = loadbyte (p) ; /* detector number */
  dm.filtnum = loadbyte (p) ; /* filter number */
  dm.iw = loadbyte (p) ; /* iw parameter */
  dm.nht = loadbyte (p) ; /* nht parameter */
  dm.fhi = loadlongint (p) ; /* fhi/filhi parameter */
  dm.flo = loadlongint (p) ; /* flo/fillo parameter */
  dm.wa = loadword (p) ; /* wa parameter */
  dm.spare = loadword (p) ; /* needed for this compiler */
  if (mh)
    then
      begin
        dm.tc = loadword (p) ; /* tc number */
        dm.x1_2 = loadbyte (p) ; /* x1 parameter / 2 */
        dm.x2_2 = loadbyte (p) ; /* x2 parameter / 2 */
        dm.x3_2 = loadbyte (p) ; /* x3 parameter / 2 */
        dm.xx = loadbyte (p) ; /* xx parameter */
        dm.av = loadbyte (p) ; /* av parameter */
      end
  cur_def->detector_num = dm.num ;
  if (dm.filtnum != 0xFF)
    then
      cur_def->detfilt = find_iir(paqs, dm.filtnum) ;
  cur_def->uconst.iwin = dm.iw ;
  cur_def->uconst.filhi = dm.fhi ;
  cur_def->uconst.fillo = dm.flo ;
  cur_def->uconst.wait_blk = dm.wa ;
  cur_def->uconst.n_hits = dm.nht ;
  if (mh)
    then
      begin
        cur_def->uconst.xth1 = dm.x1_2 shl 1 ;
        cur_def->uconst.xth2 = dm.x2_2 shl 1 ;
        cur_def->uconst.xth3 = dm.x3_2 shl 1 ;
        cur_def->uconst.xthx = dm.xx ;
        cur_def->uconst.def_tc = dm.tc ;
        cur_def->uconst.val_avg = dm.av ;
        cur_def->dtype = MURDOCK_HUTT ;
      end
    else
      cur_def->dtype = THRESHOLD ;
  len = loadbyte(p) ;
  loadblock (p, len, addr(cur_def->detname)) ;
  cur_def->detname[len] = 0 ;
end
#endif

void set_loc_name (plcq q)
begin
  integer i, lth ;
  string3 s ;

  lth = 0 ;
  for (i = 0 ; i <= 1 ; i++)
    if (q->location[i] != ' ')
      then
        s[lth++] = q->location[i] ;
  s[lth] = 0 ;
  strcpy(addr(q->slocation), s) ;
  lth = 0 ;
  for (i = 0 ; i <= 2 ; i++)
    if (q->seedname[i] != ' ')
      then
        s[lth++] = q->seedname[i] ;
  s[lth] = 0 ;
  strcpy(addr(q->sseedname), s) ;
end

static void read_lcq (paqstruc paqs, pbyte *p)
begin
  plcq cur_lcq ;
  byte b ;
  longword mask ;
  pq330 q330 ;
  string7 s ;
#ifndef OMIT_SEED
  plcq pt ;
  pdownstream_packet pds ;
  pdet_packet pdp ;
  pdetector pd ;
#endif

  q330 = paqs->owner ;
  getbuf (q330, addr(cur_lcq), sizeof(tlcq)) ;
  if (paqs->lcqs == NIL)
    then
      paqs->lcqs = cur_lcq ;
    else
      paqs->lcqs = extend_link (paqs->lcqs, cur_lcq) ;
  loadblock (p, 2, addr(cur_lcq->location)) ;
  loadblock (p, 3, addr(cur_lcq->seedname)) ;
  set_loc_name (cur_lcq) ;
  cur_lcq->lcq_num = loadbyte (p) ;
  if (cur_lcq->lcq_num > paqs->highest_lcqnum)
    then
      paqs->highest_lcqnum = cur_lcq->lcq_num ;
  cur_lcq->raw_data_source = loadbyte (p) ;
  cur_lcq->raw_data_field = loadbyte (p) ;
  cur_lcq->lcq_opt = loadlongword (p) ;
  cur_lcq->rate = loadint16 (p) ;
  cur_lcq->caldly = 60 ;
#ifndef OMIT_SEED
  getbuf (q330, addr(cur_lcq->com), sizeof(tcom_packet)) ;
  cur_lcq->firfixing_gain = 1.000 ; /* default if not over-ridden */
  cur_lcq->com->maxframes = 255 ; /* 16K! */
  cur_lcq->com->frame = 1 ;
  cur_lcq->com->next_compressed_sample = 1 ;
#endif
  if (cur_lcq->lcq_opt and LO_PEB)
    then
#ifndef OMIT_SEED
      cur_lcq->pre_event_buffers =
#endif
                           loadword (p) ;
  if (cur_lcq->lcq_opt and LO_GAP)
    then
      cur_lcq->gap_threshold = loadsingle (p) ;
  if (cur_lcq->gap_threshold == 0.0)
    then
      cur_lcq->gap_threshold = 0.5 ;
  set_gaps (cur_lcq) ;
  if (cur_lcq->lcq_opt and LO_CALDLY)
    then
      cur_lcq->caldly = loadword (p) ;
  if (cur_lcq->lcq_opt and LO_FRAME)
    then
#ifndef OMIT_SEED
      cur_lcq->com->maxframes =
#endif
                        loadbyte (p) ;
  if (cur_lcq->lcq_opt and LO_FIRMULT)
    then
#ifndef OMIT_SEED
      cur_lcq->firfixing_gain =
#endif
                        loadsingle (p) ;
  if (cur_lcq->lcq_opt and LO_AVG)
    then
      begin
#ifndef OMIT_SEED
        cur_lcq->avg_length = loadlongword (p) ;
        b = loadbyte (p) ; /* iir filter number */
        if (b != 0xFF)
          then
            cur_lcq->avg_source = find_iir(paqs, b) ;
        getbuf (q330, addr(cur_lcq->avg), sizeof(tavg_packet)) ;
#else
        loadlongword (p) ;
        loadbyte (p) ;
#endif
      end
  if (cur_lcq->lcq_opt and LO_CDET)
    then
      begin
        b = loadbyte (p) ; /* control detector number */ ;
#ifndef OMIT_SEED
        if (b != 0xFF)
          then
            cur_lcq->ctrl = (pointer)(0xFFFFFF00 or b) ; /* have to resolve these later */
#endif
      end
  if (cur_lcq->lcq_opt and LO_DEC)
    then
      begin
        b = loadbyte (p) ; /* Source LCQ NUMBER */
#ifndef OMIT_SEED
        pt = paqs->lcqs ;
        while (pt)
          if (pt->lcq_num == b)
            then
              begin
                cur_lcq->prev_link = (pointer) pt ; /* where I get my data from */
                getbuf (q330, addr(pds), sizeof(tdownstream_packet)) ;
                pds->link = NIL ;
                pds->derived_q = cur_lcq ;
                pt->downstream_link = extend_link (pt->downstream_link, pds) ;
                break ;
              end
            else
              pt = pt->link ;
        b = loadbyte (p) ; /* FIR FILTER */
        if (b != 0xFF)
          then
            begin
              cur_lcq->source_fir = find_fir (paqs, b) ;
              if (cur_lcq->source_fir == NIL)
                then
                  begin
                    seed2string(cur_lcq->location, cur_lcq->seedname, addr(s)) ;
                    libmsgadd(q330, LIBMSG_DECNOTFOUND, addr(s)) ;
                  end
            end
#endif
      end
  mask = LO_DET1 ;
  while (mask <= LO_DET8)
    begin
      if (cur_lcq->lcq_opt and mask)
        then
          begin
            b = loadbyte (p) ; /* base detector number */
#ifndef OMIT_SEED
            pd = paqs->defchain ;
            while (pd)
              if (pd->detector_num == b)
                then
                  break ;
                else
                  pd = pd->link ;
            getbuf (q330, addr(pdp), sizeof(tdet_packet)) ;
            pdp->detector_def = pd ;
            pdp->det_num = loadbyte (p) ;
            pdp->det_options = loadbyte (p) ;
            pdp->parent = cur_lcq ;
            cur_lcq->det = extend_link (cur_lcq->det, pdp) ;
#endif
          end
        else
          break ;
      mask = mask shl 1 ;
    end
end

static void read_dplcq (paqstruc paqs, pbyte *p)
begin
  plcq cur_lcq ;
  tlocation loc ;
  tseed_name name ;
  boolean newone ;
  pq330 q330 ;

  q330 = paqs->owner ;
  loadblock (p, 2, addr(loc)) ;
  loadblock (p, 3, addr(name)) ;
  cur_lcq = paqs->dplcqs ;
  newone = FALSE ;
  while (cur_lcq)
    if ((memcmp(addr(cur_lcq->location), addr(loc), 2) == 0) land
        (memcmp(addr(cur_lcq->seedname), addr(name), 3) == 0))
      then
        break ;
      else
        cur_lcq = cur_lcq->link ;
  if (cur_lcq == NIL)
    then
      begin /* add new one */
        newone = TRUE ;
        cur_lcq = malloc (sizeof(tlcq)) ;
        memset (cur_lcq, 0, sizeof(tlcq)) ;
        if (paqs->dplcqs == NIL)
          then
            paqs->dplcqs = cur_lcq ;
          else
            paqs->dplcqs = extend_link (paqs->dplcqs, cur_lcq) ;
      end
  memcpy(addr(cur_lcq->location), addr(loc), 2) ;
  memcpy(addr(cur_lcq->seedname), addr(name), 3) ;
  set_loc_name (cur_lcq) ;
  cur_lcq->lcq_num = loadbyte (p) ;
  cur_lcq->lcq_num = 0xFF ; /* we don't actually use indexing for these lcqs */
  cur_lcq->validated = TRUE ;
  cur_lcq->raw_data_source = loadbyte (p) ;
  cur_lcq->raw_data_field = loadbyte (p) ;
  cur_lcq->lcq_opt = loadlongword (p) ;
#ifndef OMIT_SEED
  cur_lcq->firfixing_gain = 1.000 ; /* default if not over-ridden */
  if (newone)
    then
      begin
        cur_lcq->com = malloc (sizeof(tcom_packet)) ;
        memset (cur_lcq->com, 0, sizeof(tcom_packet)) ;
        cur_lcq->com->frame = 1 ;
        cur_lcq->com->next_compressed_sample = 1 ;
        cur_lcq->com->maxframes = 255 ;
      end
#endif
  cur_lcq->caldly = 60 ;
  cur_lcq->rate = loadint16 (p) ;
  if (cur_lcq->lcq_opt and LO_GAP)
    then
      cur_lcq->gap_threshold = loadsingle (p) ;
  if (cur_lcq->gap_threshold == 0.0)
    then
      cur_lcq->gap_threshold = 0.5 ;
  set_gaps (cur_lcq) ;
  if (cur_lcq->lcq_opt and LO_FRAME)
    then
#ifndef OMIT_SEED
      cur_lcq->com->maxframes =
#endif
                        loadbyte (p) ;
  init_dplcq (paqs, cur_lcq, newone) ;
#ifndef OMIT_SEED
  if (newone)
    then
      preload_archive (q330, FALSE, cur_lcq) ;
#endif
end

#ifndef OMIT_SEED
void read_control_detector (paqstruc paqs, pbyte *p)
begin
  pcontrol_detector cur_ctrl ;
  pdop po ;
  byte b ;
  plcq pchan ;
  pdet_packet pdp ;
  boolean found ;
  pq330 q330 ;
  integer len ;

  q330 = paqs->owner ;
  getbuf (q330, addr(cur_ctrl), sizeof(tcontrol_detector)) ;
  paqs->ctrlchain = extend_link (paqs->ctrlchain, cur_ctrl) ;
  cur_ctrl->ctrl_num = loadbyte (p) ;
  cur_ctrl->logmsg = (loadbyte (p) and 1) ;
  len = loadbyte (p) ;
  loadblock (p, len, addr(cur_ctrl->cdname)) ;
  cur_ctrl->cdname[len] = 0 ;
  repeat
    b = loadbyte (p) ;
    if (b == (DES_OP or DEO_DONE))
      then
        break ;
    getbuf (q330, addr(po), sizeof(tdop)) ;
    po->link = NIL ;
    po->tok = b ;
    po->point = NIL ;
    switch (b and DES_OP) begin
      case DES_DET :
        pchan = paqs->lcqs ;
        b = b and (not DES_OP) ;
        found = FALSE ;
        while ((lnot found) land (pchan))
          begin
            pdp = pchan->det ;
            while (pdp)
              if (pdp->det_num == b)
                then
                  break ;
                else
                  pdp = pdp->link ;
            if (pdp)
              then
                begin
                  found = TRUE ;
                  po->point = pdp ;
                end
            pchan = pchan->link ;
          end
        break ;
      case DES_CAL :
        pchan = paqs->lcqs ;
        b = b and (not DES_OP) ;
        while (pchan)
          if (pchan->lcq_num == b)
            then
              break ;
            else
              pchan = pchan->link ;
        po->point = pchan ;
        break ;
    end
    cur_ctrl->token_list = extend_link (cur_ctrl->token_list, po) ;
  until FALSE) ;
end

void resolve_control_detectors (paqstruc paqs)
begin
  plcq cur_lcq ;
  pcontrol_detector pc ;

  cur_lcq = paqs->lcqs ;
  while (cur_lcq)
    begin
      if (cur_lcq->ctrl)
        then
          begin
            pc = paqs->ctrlchain ;
            while (pc)
              if (pc->ctrl_num == ((integer)cur_lcq->ctrl and 0xFF))
                then
                  break ;
                else
                  pc = pc->link ;
            cur_lcq->ctrl = pc ;
          end
      cur_lcq = cur_lcq->link ;
    end
end
#endif

static void read_comm_events (paqstruc paqs, pbyte *p, pbyte stop)
begin
  string s ;
  byte b ;
  integer lth ;

  while ((integer)*p < (integer)stop)
    begin
      b = loadbyte(p) ; /* comm event number */
      lth = loadbyte(p) ;
      loadblock (p, lth, addr(s)) ;
      s[lth] = 0 ; /* null terminate */
      if (b <= 31)
        then
          strncpy(addr(paqs->commevents[b].name), addr(s), COMMLENGTH) ;
    end
end

static void read_dss (paqstruc paqs, pbyte *p)
begin
  tdss *pdss ;

  pdss = addr(paqs->dss_def) ;
  loadstring (p, 8, addr(pdss->high_pass)) ;
  loadstring (p, 8, addr(pdss->mid_pass)) ;
  loadstring (p, 8, addr(pdss->low_pass)) ;
  pdss->timeout = loadlongint (p) ;
  pdss->max_bps = loadlongint (p) ;
  pdss->verbosity = loadbyte (p) ;
  pdss->max_cpu_perc = loadbyte (p) ;
  pdss->port_number = loadword (p) ;
  pdss->max_mem = loadword (p) ;
  pdss->reserved = loadword (p) ;
end

static void read_clock (paqstruc paqs, pbyte *p)
begin
  tclock *pclk ;
  pq330 q330 ;

  q330 = paqs->owner ;
  pclk = addr(q330->qclock) ;
  pclk->zone = loadlongint (p) ;
  pclk->degrade_time = loadword (p) ;
  pclk->q_locked = loadbyte (p) ;
  pclk->q_track = loadbyte (p) ;
  pclk->q_hold = loadbyte (p) ;
  pclk->q_off = loadbyte (p) ;
  pclk->q_spare = loadbyte (p) ;
  pclk->q_high = loadbyte (p) ;
  pclk->q_low = loadbyte (p) ;
  pclk->q_never = loadbyte (p) ;
  pclk->clock_filt = loadword (p) ;
end

static void read_log_tim (paqstruc paqs, pbyte *p)
begin
  tlog_tim *plt ;

  plt = addr(paqs->log_tim) ;
  loadblock (p, 2, addr(plt->log_location)) ;
  loadblock (p, 3, addr(plt->log_seedname)) ;
  loadblock (p, 2, addr(plt->tim_location)) ;
  loadblock (p, 3, addr(plt->tim_seedname)) ;
end

static void read_cfgid (paqstruc paqs, pbyte *p)
begin
  tlog_cfg *plc ;

  plc = addr(paqs->log_cfg) ;
  loadblock (p, 2, addr(plc->cfg_location)) ;
  loadblock (p, 3, addr(plc->cfg_seedname)) ;
  plc->flags = loadbyte (p) ;
  plc->interval = loadword (p) ;
end

#ifndef OMIT_SEED
void add_msg_tim_lcqs (paqstruc paqs)
begin
  plcq cur_lcq ;
  pq330 q330 ;

  q330 = paqs->owner ;
  getbuf (q330, addr(cur_lcq), sizeof(tlcq)) ;
  getbuf (q330, addr(cur_lcq->com), sizeof(tcom_packet)) ;
  paqs->lcqs = extend_link (paqs->lcqs, cur_lcq) ;
  paqs->msg_lcq = cur_lcq ;
  memcpy(addr(cur_lcq->location), addr(paqs->log_tim.log_location), 2) ;
  memcpy(addr(cur_lcq->seedname), addr(paqs->log_tim.log_seedname), 3) ;
  set_loc_name (cur_lcq) ;
  cur_lcq->raw_data_source = MESSAGE_STREAM ;
  inc(paqs->highest_lcqnum) ;
  cur_lcq->lcq_num = paqs->highest_lcqnum ;
  if (q330->par_create.call_minidata)
    then
      cur_lcq->mini_filter = q330->par_create.opt_minifilter and (OMF_ALL or OMF_MSG) ;
  if ((paqs->arc_size > 0) land (q330->par_create.call_aminidata))
    then
      cur_lcq->arc.amini_filter = q330->par_create.opt_aminifilter and (OMF_ALL or OMF_MSG) ;
  getbuf (q330, addr(cur_lcq), sizeof(tlcq)) ;
  getbuf (q330, addr(cur_lcq->com), sizeof(tcom_packet)) ;
  paqs->lcqs = extend_link (paqs->lcqs, cur_lcq) ;
  paqs->tim_lcq = cur_lcq ;
  memcpy(addr(cur_lcq->location), addr(paqs->log_tim.tim_location), 2) ;
  memcpy(addr(cur_lcq->seedname), addr(paqs->log_tim.tim_seedname), 3) ;
  set_loc_name (cur_lcq) ;
  cur_lcq->raw_data_source = TIMING_STREAM ;
  inc(paqs->highest_lcqnum) ;
  cur_lcq->lcq_num = paqs->highest_lcqnum ;
  if (q330->par_create.call_minidata)
    then
      cur_lcq->mini_filter = q330->par_create.opt_minifilter and (OMF_ALL or OMF_TIM) ;
  if ((paqs->arc_size > 0) land (q330->par_create.call_aminidata))
    then
      cur_lcq->arc.amini_filter = q330->par_create.opt_aminifilter and (OMF_ALL or OMF_TIM) ;
  getbuf (q330, addr(cur_lcq), sizeof(tlcq)) ;
  getbuf (q330, addr(cur_lcq->com), sizeof(tcom_packet)) ;
  paqs->lcqs = extend_link (paqs->lcqs, cur_lcq) ;
  paqs->cfg_lcq = cur_lcq ;
  memcpy(addr(cur_lcq->location), addr(paqs->log_cfg.cfg_location), 2) ;
  memcpy(addr(cur_lcq->seedname), addr(paqs->log_cfg.cfg_seedname), 3) ;
  set_loc_name (cur_lcq) ;
  cur_lcq->raw_data_source = CFG_STREAM ;
  inc(paqs->highest_lcqnum) ;
  cur_lcq->lcq_num = paqs->highest_lcqnum ;
  if (q330->par_create.call_minidata)
    then
      cur_lcq->mini_filter = q330->par_create.opt_minifilter and (OMF_ALL or OMF_CFG) ;
  if ((paqs->arc_size > 0) land (q330->par_create.call_aminidata))
    then
      cur_lcq->arc.amini_filter = q330->par_create.opt_aminifilter and (OMF_ALL or OMF_CFG) ;
end
#endif

pointer tonext (pointer base, integer offset)
begin

  return (pointer)((integer)base + offset) ;
end

void decode_cfg (pq330 q330)
begin
  byte tok ;
  word next ;
  integer i, lth ;
  string s ;
  string s1 ;
  pbyte p, pend, pref ;
  paqstruc paqs ;
  plcq q ;
  enum tacctype acctype ;
  taccmstat *paccm ;

  paqs = q330->aqstruc ;
  for (i = 0 ; i <= CE_MAX - 1 ; i++)
    sprintf(addr(paqs->commevents[i].name), "COMM:%d", i + 1) ;
  q = paqs->dplcqs ;
  while (q)
    begin
      q->validated = FALSE ;
      q = q->link ;
    end
  p = (pointer)q330->cfgbuf ;
  pend = p ;
  incn(pend, q330->cfgsize) ;
  while ((integer)p < (integer)pend)
    begin
      tok = loadbyte (addr(p)) ;
      switch (tok) begin
        case TF_NOP :
          break ;
        case TF_VERSION :
          tok = loadbyte (addr(p)) ; /* token version number */
          if (tok != T_VER)
            then
              begin
                libmsgadd (q330, LIBMSG_INVTVER, "") ;
                p = pend ; /* force leaving the loop */
                break ;
              end
          break ;
        case TF_NET_STAT :
          loadblock (addr(p), 2, addr(q330->network)) ;
          loadblock (addr(p), 5, addr(q330->station)) ;
          break ;
        case TF_NETSERV :
          paqs->netport = loadword (addr(p)) ;
          break ;
        case TF_DSS :
          read_dss (paqs, addr(p)) ;
          break ;
        case TF_WEB :
          paqs->webport = loadword (addr(p)) ;
          break ;
        case TF_DATASERV :
          paqs->dservport = loadword (addr(p)) ;
          break ;
        case TF_CLOCK :
          read_clock (paqs, addr(p)) ;
          break ;
        case TF_MT :
          read_log_tim (paqs, addr(p)) ;
          break ;
        case TF_CFG :
          read_cfgid (paqs, addr(p)) ;
          break ;
        case T1_MHD :
        case T1_THRD :
          pref = p ;
          next = loadbyte (addr(p)) ; /* total size */
#ifndef OMIT_SEED
          read_detector (paqs, addr(p), (tok == T1_MHD)) ;
#endif
          p = tonext(pref, next) ;
          break ;
        case T1_IIR :
          pref = p ;
          next = loadbyte (addr(p)) ; /* total size */
#ifndef OMIT_SEED
          read_iir (paqs, addr(p)) ;
#endif
          p = tonext(pref, next) ;
          break ;
        case T1_FIR :
          pref = p ;
          next = loadbyte (addr(p)) ; /* total size */
#ifndef OMIT_SEED
          read_fir (paqs, addr(p)) ;
#endif
          p = tonext(pref, next) ;
          break ;
        case T1_LCQ :
          pref = p ;
          next = loadbyte (addr(p)) ; /* total size */
          read_lcq (paqs, addr(p)) ;
          p = tonext(pref, next) ;
          break ;
        case T1_DPLCQ :
          pref = p ;
          next = loadbyte (addr(p)) ; /* total size */
          read_dplcq (paqs, addr(p)) ;
          p = tonext(pref, next) ;
          break ;
        case T1_CTRL :
          pref = p ;
          next = loadbyte (addr(p)) ;
#ifndef OMIT_SEED
          read_control_detector (paqs, addr(p)) ;
#endif
          p = tonext(pref, next) ;
          break ;
        case T1_NONCOMP :
          pref = p ;
          paqs->non_comp = TRUE ;
          next = loadbyte (addr(p)) ;
          p = tonext(pref, next) ;
          break ;
        case T2_CNAMES :
          pref = p ;
          next = loadword (addr(p)) ;
          read_comm_events (paqs, addr(p), (pointer)((integer)pref + next)) ;
          p = tonext(pref, next) ;
          break ;
        case T2_OPAQUE :
          pref = p ;
          next = loadword (addr(p)) ; /* get length */
#ifndef OMIT_SEED
          paqs->opaque_size = next - 2 ; /* don't count length word */
          getbuf (q330, addr(paqs->opaque_buf), paqs->opaque_size) ;
          loadblock (addr(p), paqs->opaque_size, addr(paqs->opaque_size)) ;
#endif
          p = tonext(pref, next) ;
          break ;
        default :
          if (tok >= 0x80)
            then /* variable length */
              if (tok >= 0xC0)
                then
                  begin /* two byte length */
                    pref = p ;
                    next = loadword (addr(p)) ;
                    p = tonext(pref, next) ;
                  end
                else
                  begin /* one byte length */
                    pref = p ;
                    next = loadbyte (addr(p)) ;
                    p = tonext(pref, next) ;
                  end
          break ;
      end
    end
#ifndef OMIT_SEED
  add_msg_tim_lcqs (paqs) ;
  resolve_control_detectors (paqs) ; /* lcqs didn't have anything to point to yet */
#endif
  q = paqs->dplcqs ;
  while (q)
    begin
      if (lnot q->validated)
        then
          begin
            for (acctype = AC_FIRST ; acctype <= AC_LAST ; acctype++)
              begin
                paccm = addr(q330->share.accmstats[acctype]) ;
                if (paccm->ds_lcq == q)
                  then
                    paccm->ds_lcq = NIL ;
              end
            if (paqs->data_latency_lcq == q)
              then
                paqs->data_latency_lcq = NIL ;
            else if (paqs->status_latency_lcq == q)
              then
                paqs->status_latency_lcq = NIL ;
          end
      q = q->link ;
    end
  lth = 0 ;
  for (i = 0 ; i <= 4 ; i++)
    if (q330->station[i] != ' ')
      then
        s[lth++] = q330->station[i] ;
  s[lth] = 0 ; /* build string representation of station (3-5 characters) */
  s1[0] = q330->network[0] ;
  s1[1] = q330->network[1] ;
  s1[2] = 0 ; /* build string representation of network (always 2 characters) */
  sprintf (addr(q330->station_ident), "%s-%s", s1, s) ; /* network-station */
  init_lcq (paqs) ;
#ifndef OMIT_SEED
  expand_control_detectors (paqs) ;
#endif
  check_continuity (q330) ;
  if (paqs->data_timetag > 1.0) /* if non-zero continuity was restored, write cfg blks at start of session */
    then
      begin
        libdatamsg (q330, LIBMSG_RESTCONT, "") ;
        paqs->contingood = restore_continuity (q330) ;
        if (lnot paqs->contingood)
          then
            libdatamsg (q330, LIBMSG_CONTNR, "") ;
      end
  paqs->last_data_qual = NO_LAST_DATA_QUAL ;
  paqs->first_data = TRUE ;
  q330->data_timer = 0 ;
  q330->status_timer = 0 ;
  if (lnot paqs->non_comp)
    then
      verify_mapping (q330) ;
#ifndef OMIT_SEED
  if (paqs->arc_size > 0)
    then
      preload_archive (q330, TRUE, NIL) ;
  if ((paqs->data_timetag > 1) land (paqs->log_cfg.flags and LC_START))
    then
      start_cfgblks (paqs) ;
#endif
  new_state (q330, LIBSTATE_RUNWAIT) ;
end

void read_q330_cfg (pq330 q330, pbyte pb)
begin
  word w ;
  tseghdr seghdr ;
  pbyte p ;
  tmem *pmem ;
  string31 s ;

  p = pb ; /* p now has location in received buffer */
  pmem = addr(q330->mem_hdr) ;
  loadseghdr (addr(p), addr(seghdr)) ;
  w = pmem->count - 4 ; /* remove segment numbers */
  q330->cfgnow = (seghdr.segnum - 1) * MAXSEG ;
  memcpy (addr((*(q330->cfgbuf))[q330->cfgnow]), p, w) ;
  q330->cfgsize = q330->cfgnow + w ;
  if (seghdr.segnum == seghdr.segtotal)
    then
      begin
        sprintf(s, "%d bytes", q330->cfgsize) ;
        libmsgadd (q330, LIBMSG_TOKREAD, addr(s)) ;
        new_state (q330, LIBSTATE_DECTOK) ;
      end
    else
      begin
        q330->cfgoffset = q330->cfgoffset + w + OVERHEAD ;
        pmem = addr(q330->mem_req) ;
        pmem->start = q330->cfgoffset ;
        pmem->count = 0 ;
        pmem->memtype = MT_CFG1 + q330->par_create.q330id_dataport ;
        new_cmd (q330, C1_RQMEM, MAXSEG) ;
      end
end

void cfg_start (pq330 q330)
begin
  tmem *pmem ;

  pmem = addr(q330->mem_req) ;
  new_state (q330, LIBSTATE_READTOK) ;
  libmsgadd (q330, LIBMSG_READTOK, "") ;
  q330->data_timer = 0 ;
  pmem->start = 0 ;
  pmem->count = 0 ;
  pmem->memtype = MT_CFG1 + q330->par_create.q330id_dataport ;
  new_cmd (q330, C1_RQMEM, MAXSEG) ;
  q330->cfgsize = 0 ;
  q330->cfgnow = 0 ;
  q330->cfgoffset = 0 ;
end
