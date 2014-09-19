/*   Lib330 Detector Routines
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
    0 2006-10-13 rdr Created
*/
#ifndef OMIT_SEED
#ifndef libdetect_h
#include "libdetect.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

static void Ibingo (pcon_sto con_ptr)
begin

  con_ptr->epf = FALSE ;
  con_ptr->evon = TRUE ;
  con_ptr->icheck = FALSE ;
  con_ptr->itc = NOR_OUT ;
  con_ptr->nn = 0 ;
end

static void Wbuff (pcon_sto con_ptr)
begin
  integer i ;

  inc(con_ptr->lst_pt) ;
  if (con_ptr->lst_pt == B_M1)
    then
      con_ptr->lst_pt = 0 ;
  i = con_ptr->lst_pt ;
  con_ptr->buf_flg[i] = con_ptr->th_wt ;
  con_ptr->buf_sc[i] = con_ptr->sumdsc ;
  con_ptr->buf_amp[i] = con_ptr->s_amp ;
  con_ptr->buf_tim[i] = con_ptr->tim_of_max ;
  con_ptr->buf_rec[i] = con_ptr->rec_of_max ;
end

static boolean Event (pcon_sto con_ptr)
begin
  integer j, m ;
  integer tfst_flg ;
  longint ab_amp ;
  tdetload *pdl ;
  boolean result ;

  result = FALSE ;
  pdl = addr(con_ptr->parent->ucon) ;
  tfst_flg = 0 ;
  ab_amp = abs (con_ptr->s_amp) ;
  if (con_ptr->jj > 3)
    then
      con_ptr->jj = 0 ;

  j = con_ptr->jj ;
  con_ptr->abuf_rec[j] = con_ptr->last_rec ;
  con_ptr->abuf_sc[j] = con_ptr->last_sc ;
  con_ptr->abuf_amp[j] = con_ptr->last_amp ;
  con_ptr->abuf_tim[j] = con_ptr->last_tim ;
  inc(con_ptr->jj) ;

  con_ptr->last_sc = con_ptr->s_sum_sc ;
  con_ptr->last_amp = con_ptr->s_amp ;
  con_ptr->last_tim = con_ptr->tim_of_max ;
  con_ptr->last_rec = con_ptr->rec_of_max ;
  if (con_ptr->evon)
    then
      begin
        con_ptr->lst_pt2 = con_ptr->lst_pt ;
        if (con_ptr->fst_flg > con_ptr->lst_pt2)
          then
            con_ptr->lst_pt2 = con_ptr->lst_pt2 + E_B ;
        if ((con_ptr->lst_pt2 - con_ptr->fst_flg) == 7)
          then
            begin
              con_ptr->sumdsc = con_ptr->sumdsc + con_ptr->s_sum_sc ;
              Wbuff (con_ptr) ;
              con_ptr->icheck = TRUE ;
              return TRUE ;
            end
        if ((con_ptr->lst_pt2 - con_ptr->fst_flg) < 8)
          then
            begin
              con_ptr->sumdsc = con_ptr->sumdsc + con_ptr->s_sum_sc ;
              Wbuff (con_ptr) ;
              return result ;
            end

        if (con_ptr->icheck)
          then
            return result ;
        con_ptr->icheck = TRUE ;
        return TRUE ;
      end

  if (lnot con_ptr->epf)
    then
      begin
        if (ab_amp < con_ptr->th2)
          then
            return result ;
        if (ab_amp < con_ptr->th1)
          then
            con_ptr->th_wt = 1 ;
          else
            con_ptr->th_wt = 2 ;
        con_ptr->epf = TRUE ;
        con_ptr->fst_flg = 4 ;
        con_ptr->lst_flg = 4 ;
        con_ptr->lst_pt = 3 ;
        m = con_ptr->jj ;
        if (m > 3)
          then
            m = 0 ;

        con_ptr->sumdsc = -(con_ptr->abuf_sc[m]) ;
        for (j = 0 ; j <= 3 ; j++)
          begin
            if (m > 3)
              then
                m = 0 ;
            con_ptr->sumdsc = con_ptr->sumdsc + con_ptr->abuf_sc[m] ;
            con_ptr->buf_rec[j] = con_ptr->abuf_rec[m] ;
            con_ptr->buf_sc[j] = con_ptr->sumdsc ;
            con_ptr->buf_amp[j] = con_ptr->abuf_amp[m] ;
            con_ptr->buf_tim[j] = con_ptr->abuf_tim[m] ;
            inc(m) ;
          end
        con_ptr->sumdsc = con_ptr->sumdsc + con_ptr->s_sum_sc ;
        Wbuff (con_ptr) ;
        return result ;
      end

  con_ptr->sumdsc = con_ptr->sumdsc + con_ptr->s_sum_sc ;

  if (ab_amp >= con_ptr->th2)
    then
      begin
        con_ptr->th_wt = 1 ;

        if (ab_amp >= con_ptr->th1)
          then
            con_ptr->th_wt = 2 ;

        if ((con_ptr->sumdsc - con_ptr->buf_sc[con_ptr->lst_flg]) <= pdl->filhi)
          then
            begin
              con_ptr->th_wt = 0 ;
              Wbuff (con_ptr) ;
              return result ;
            end

        if ((con_ptr->sumdsc - con_ptr->buf_sc[con_ptr->lst_flg]) >= pdl->fillo)
          then
            begin
              Wbuff (con_ptr) ;
              con_ptr->fst_flg = con_ptr->lst_pt ;
              con_ptr->lst_flg = con_ptr->lst_pt ;
              return result ;
            end

        if (con_ptr->fst_flg == con_ptr->lst_flg)
          then
            begin
              Wbuff (con_ptr) ;
              con_ptr->lst_flg = con_ptr->lst_pt ;
              return result ;
            end

        while ((con_ptr->sumdsc - con_ptr->buf_sc[con_ptr->fst_flg]) > pdl->iwin)
          begin
            con_ptr->indx = con_ptr->fst_flg ;
            con_ptr->lst_flg2 = con_ptr->lst_flg ;
            if ((con_ptr->indx + 1) > con_ptr->lst_flg)
              then
                con_ptr->lst_flg2 = con_ptr->lst_flg + E_B ;
            for (j = (con_ptr->indx + 1) ; j <= con_ptr->lst_flg2 ; j++)
              begin
                if (con_ptr->fst_flg == B_M1)
                  then
                    con_ptr->fst_flg = -1 ;
                inc(con_ptr->fst_flg) ;
                if (con_ptr->buf_flg[con_ptr->fst_flg])
                  then
                    break ;
              end
          end
        Wbuff (con_ptr) ;
        con_ptr->lst_flg = con_ptr->lst_pt ;
        con_ptr->iset = FALSE ;
        con_ptr->sumflg = 0 ;
        con_ptr->lst_flg2 = con_ptr->lst_flg ;
        if (con_ptr->lst_flg < con_ptr->fst_flg)
          then
            con_ptr->lst_flg2 = con_ptr->lst_flg + E_B ;

        for (j = con_ptr->fst_flg ; j <= con_ptr->lst_flg2 ; j++)
          begin
            con_ptr->index2 = j ;
            if (con_ptr->index2 > B_M1)
              then
                con_ptr->index2 = con_ptr->index2 - E_B ;
            if (con_ptr->buf_flg[con_ptr->index2])
              then
                inc(con_ptr->sumflg) ;
            if (con_ptr->buf_flg[con_ptr->index2] == 2)
              then
                con_ptr->iset = TRUE ;
          end
        if (con_ptr->sumflg < 3)
          then
            return result ;
        if (con_ptr->iset)
          then
            begin
              Ibingo (con_ptr) ;
              return result ;
            end
        if (con_ptr->sumflg >= pdl->n_hits)
          then
            Ibingo (con_ptr) ;
        return result ;
      end

  con_ptr->th_wt = 0 ;
  Wbuff (con_ptr) ;

  while ((con_ptr->buf_sc[con_ptr->lst_pt] -
         con_ptr->buf_sc[con_ptr->fst_flg]) > pdl->iwin)
    begin
      con_ptr->lst_flg2 = con_ptr->lst_flg ;
      if (con_ptr->fst_flg > con_ptr->lst_flg)
        then
          con_ptr->lst_flg2 = con_ptr->lst_flg + E_B ;
      if (con_ptr->fst_flg >= con_ptr->lst_flg2)
        then
          begin
            con_ptr->epf = FALSE ;
            return result ;
          end

      for (j = con_ptr->fst_flg + 1 ; j <= con_ptr->lst_flg2 ; j++)
        begin
          tfst_flg = j ;
          if (tfst_flg > B_M1)
            then
              tfst_flg = tfst_flg - E_B ;
          if (con_ptr->buf_flg[tfst_flg] > 0)
            then
              break ;
        end
      con_ptr->fst_flg = tfst_flg ;
    end
  return result ;
end

static void Count_dn (pcon_sto con_ptr)
begin

  if (con_ptr->itc > 0)
    then
      begin
        inc(con_ptr->nn) ;
        if (con_ptr->nn >= con_ptr->parent->ucon.wait_blk)
          then
            begin
              con_ptr->nn = 0 ;
              dec(con_ptr->itc) ;
            end
      end
  if (con_ptr->itc <= EV_OFF)
    then
      con_ptr->evon = FALSE ;
end

static longint Xth (pcon_sto con_ptr, longint xthi)
begin
  longint th ;
  longint x_left ;

  th = 0 ;
  x_left = xthi shr 3 ;

  if (x_left > 0)
    then
      begin
        if (x_left and 16)
          then
            th = con_ptr->twosd shl 4 ;
        if (x_left and 8)
          then
            th = th + (con_ptr->twosd shl 3) ;
        if (x_left and 4)
           then
            th = th + (con_ptr->twosd shl 2) ;
        if (x_left and 2)
          then
            th = th + (con_ptr->twosd shl 1) ;
        if (x_left and 1)
          then
            th = th + con_ptr->twosd ;
      end

  if (xthi and 7)
    then
      begin
        if (xthi and 1)
          then
            th = th + (con_ptr->twosd shr 3) ;
        if (xthi and 2)
          then
            th = th + (con_ptr->twosd shr 2) ;
        if (xthi and 4)
          then
            th = th + (con_ptr->twosd shr 1) ;
      end
  return th ;
end

static void Onset (pcon_sto con_ptr, integer ibak, longint sper,
            integer tm_indx, longint tc, integer amp_indx,
            longint pt0, longint pt1, longint pt2, longint pt3, longint pt4)
begin
  integer i, indx2, j ;
  longint isnr[5] ;
  longint mx_amp, base, temp ;
  longint pt[5] ;
  longint t_samp, d_samp ;
  byte ipol ;
  pdet_packet ppar ;
  tonset_mh *pon ;

  pon = con_ptr->onsetdata ;
  ppar = con_ptr->parent ;
  if (pt2 > 0)
    then
      ipol = 1 ;
    else
      ipol = 0 ;

  pt[0] = pt0 ;
  pt[1] = pt1 ;
  pt[2] = pt2 ;
  pt[3] = pt3 ;
  pt[4] = pt4 ;

  for (j = 0 ; j <= 4 ; j++)
    begin
      base = con_ptr->twosd - (con_ptr->twosd shr 1) ;
      for (i = 0 ; i <= 9 ; i++)
        begin
          isnr[j] = i ;
          if (abs(pt[j]) < base)
            then
              break ;
          base = base + con_ptr->twosd ;
        end
    end

  indx2 = amp_indx ;
  mx_amp = 0 ;
  for (j = 0 ; j <= 7 ; j++)
    begin
      temp = abs(con_ptr->buf_amp[indx2]) ;
      if (temp > mx_amp)
        then
          mx_amp = temp ;
      if (indx2 == B_M1)
        then
          indx2 = 0 ;
        else
          inc(indx2) ;
    end

  con_ptr->p_val = sper / con_ptr->sample_rate ;

  t_samp = con_ptr->buf_tim[tm_indx] ;
  d_samp = con_ptr->cur_rec - con_ptr->buf_rec[tm_indx] ;
  if (d_samp < 0)
    then
      d_samp = d_samp + CUR_MAX ;
  t_samp = t_samp - (ppar->sam_ch * d_samp) ;

  con_ptr->new_onset = TRUE ;
  pon->signal_amplitude = mx_amp shr DET_SCALE_SHIFT ;
  pon->signal_period = con_ptr->p_val ;
  pon->background_estimate = con_ptr->twosd shr DET_SCALE_SHIFT ;
  for (i = 0 ; i <= 4 ; i++)
    pon->snr[i] = isnr[i] ;
  pon->snr[5] = 0 ;
  pon->reserved_byte = 0 ;
  pon->signal_onset_time.seed_fpt = con_ptr->etime + (t_samp / con_ptr->sample_rate) - (tc / 1000.0) ; /* tc converted to seconds */
  pon->event_detection_flags = ipol ;
  pon->lookback_value = ibak ;
  if (con_ptr->iset)
    then
      pon->pick_algorithm = 0 ;
    else
      pon->pick_algorithm = 1 ;

/* at this point, if sam_sec >= VSP_SPS && p_rval >= PER_TRIG, itc == ITC_UP */
  for (i = 0 ; i <= 15 ; i++)
    con_ptr->tsstak[i] = (con_ptr->tsstak[i]) + ((con_ptr->tsstak[i]) shr 4) ; /* 1.0625 factor */
  con_ptr->twosd = ((con_ptr->twosd) + ((con_ptr->twosd) shr 4)) ;

  con_ptr->th1 = Xth (con_ptr, ppar->ucon.xth1) ;
  con_ptr->th2 = Xth (con_ptr, ppar->ucon.xth2) ;
  con_ptr->th3 = Xth (con_ptr, ppar->ucon.xth3) ;
  con_ptr->thx = Xth (con_ptr, ppar->ucon.xthx) ;
end

static void Onsetq (pcon_sto con_ptr)
begin
#define RO 2
  integer flg_p1, flg_p2, flg_m1, flg_m2, flg_m3, flg_m4, lb,
          i1, i2, i3, i4, i5, i6 ;
  longint tc ;
  integer per_bnd ;
  longint per_sc ;
  integer kase ;
  longint per_sav ;
  longint trunc_rate ;

  kase = 0 ;

  flg_m4 = con_ptr->fst_flg - 4 ;
  if (flg_m4 < 0)
    then
      flg_m4 = con_ptr->fst_flg + B_M4 ;
  flg_m3 = con_ptr->fst_flg - 3 ;
  if (flg_m3 < 0)
    then
      flg_m3 = con_ptr->fst_flg + B_M3 ;
  flg_m2 = con_ptr->fst_flg - 2 ;
  if (flg_m2 < 0)
    then
      flg_m2 = con_ptr->fst_flg + B_M2 ;
  flg_m1 = con_ptr->fst_flg - 1 ;
  if (flg_m1 < 0)
    then
      flg_m1 = con_ptr->fst_flg + B_M1 ;
  flg_p1 = con_ptr->fst_flg + 1 ;
  if (flg_p1 >= E_B)
    then
      flg_p1 = con_ptr->fst_flg - B_M1 ;
  flg_p2 = con_ptr->fst_flg + 2 ;
  if (flg_p2 >= E_B)
    then
      flg_p2 = con_ptr->fst_flg - B_M2 ;
  per_bnd = con_ptr->fst_flg + 8 ;
  if (per_bnd > B_M1)
    then
      per_bnd = per_bnd - E_B ;
  per_sc = con_ptr->buf_sc[per_bnd] - con_ptr->buf_sc[con_ptr->fst_flg] + RO ;
  per_sc = per_sc shr 2 ;
  per_sav = per_sc ;
  trunc_rate = con_ptr->sample_rate ; /* truncate, not round */
  if (per_sc < trunc_rate)
    then
      per_sc = trunc_rate ;
  con_ptr->haf_per = per_sc shr 1 ;

  if ((con_ptr->buf_sc[flg_m1] - con_ptr->buf_sc[flg_m2]) <= per_sc)
    then
      if (abs(con_ptr->buf_amp[flg_m2]) >= con_ptr->th3)
        then
          if ((con_ptr->buf_sc[flg_m2] - con_ptr->buf_sc[flg_m3]) > con_ptr->haf_per)
            then
              kase = 1 ;
            else
              kase = 2 ;

  if ((kase == 0) land ((con_ptr->buf_sc[con_ptr->fst_flg] - con_ptr->buf_sc[flg_m1]) <= per_sc))
    then
      if (abs(con_ptr->buf_amp[flg_m1]) >= con_ptr->th3)
        then
          if ((con_ptr->buf_sc[flg_m1] - con_ptr->buf_sc[flg_m2]) > con_ptr->haf_per)
            then
              kase = 3 ;
            else
              kase = 4 ;

  if ((kase == 0) land ((con_ptr->buf_sc[con_ptr->fst_flg] - con_ptr->buf_sc[flg_m1]) <= con_ptr->haf_per))
    then
      kase = 5 ;
  switch (kase) begin
    case 1 :
      lb = 2 ;
      tc = con_ptr->parent->ucon.def_tc ;
      i1 = flg_m2 ;
      i2 = flg_m4 ;
      i3 = flg_m3 ;
      i4 = flg_m2 ;
      i5 = flg_m1 ;
      i6 = con_ptr->fst_flg ;
      break ;
    case 2 :
      lb = 2 ;
      tc = 0 ;
      i1 = flg_m3 ;
      i2 = flg_m4 ;
      i3 = flg_m3 ;
      i4 = flg_m2 ;
      i5 = flg_m1 ;
      i6 = con_ptr->fst_flg ;
      break ;
    case 3 :
      lb = 1 ;
      tc = con_ptr->parent->ucon.def_tc ;
      i1 = flg_m1 ;
      i2 = flg_m3 ;
      i3 = flg_m2 ;
      i4 = flg_m1 ;
      i5 = con_ptr->fst_flg ;
      i6 = flg_p1 ;
      break ;
    case 4 :
      lb = 1 ;
      tc = 0 ;
      i1 = flg_m2 ;
      i2 = flg_m3 ;
      i3 = flg_m2 ;
      i4 = flg_m1 ;
      i5 = con_ptr->fst_flg ;
      i6 = flg_p1 ;
      break ;
    case 5 :
      lb = 0 ;
      tc = 0 ;
      i1 = flg_m1 ;
      i2 = flg_m2 ;
      i3 = flg_m1 ;
      i4 = con_ptr->fst_flg ;
      i5 = flg_p1 ;
      i6 = flg_p2 ;
      break ;
    default :
      lb = 0 ;
      tc = con_ptr->parent->ucon.def_tc ;
      i1 = con_ptr->fst_flg ;
      i2 = flg_m2 ;
      i3 = flg_m1 ;
      i4 = con_ptr->fst_flg ;
      i5 = flg_p1 ;
      i6 = flg_p2 ;
      break ;
  end
  Onset (con_ptr, lb, per_sav, i1, tc, i4, con_ptr->buf_amp[i2], con_ptr->buf_amp[i3],
         con_ptr->buf_amp[i4], con_ptr->buf_amp[i5], con_ptr->buf_amp[i6]) ;
end

static void P_two (pcon_sto con_ptr, longint maxamp)
begin
  integer i ;
  longint sum ;
  tdetload *pdl ;

  pdl = addr(con_ptr->parent->ucon) ;
  if (lnot con_ptr->evon)
    then
      begin
        con_ptr->tsstak[con_ptr->kk] = maxamp ;
        inc(con_ptr->kk) ;
        if (con_ptr->kk == pdl->val_avg)
          then
            con_ptr->kk = 0 ;

        sum = 0 ;
        for (i = 0 ; i <= pdl->val_avg - 1 ; i++)
          sum = sum + con_ptr->tsstak[i] ;
        con_ptr->twosd = sum div pdl->val_avg ;

        if (con_ptr->twosd <= 0)
          then
            con_ptr->twosd = 1000000 ;

        con_ptr->th1 = Xth (con_ptr, pdl->xth1) ;
        con_ptr->th2 = Xth (con_ptr, pdl->xth2) ;
        con_ptr->th3 = Xth (con_ptr, pdl->xth3) ;
        con_ptr->thx = Xth (con_ptr, pdl->xthx) ;
      end
end

boolean E_detect (tdet_packet *detector)
begin
  pdataarray longdata ;
  psinglearray realdata ;
  boolean realflag ;
  enum slope_type cur_slope ;
  longint in_data, ab_amp ;
  longint del_amp ;
  pcon_sto con_ptr ;
  integer i ;
  trealsamps *prs ;
  boolean result ;

  con_ptr = detector->cont ;
  result = FALSE ;
  con_ptr->new_onset = FALSE ;
  longdata = detector->indatar ;
  realflag = detector->singleflag ;
  realdata = (pointer)longdata ;

  if (detector->insamps)
    then
      begin
        if (con_ptr->sampcnt == 0)
          then
            begin
              con_ptr->etime = con_ptr->startt ;
              detector->sam_no = 0 ;
            end
        prs = (pointer)detector->insamps ;
        for (i = 0 ; i <= detector->grpsize - 1 ; i++)
          begin /* xfer into holding buffers */
            if (realflag)
              then
                (*prs)[con_ptr->sampcnt] = (*realdata)[i] ;
              else
                (*detector->insamps)[con_ptr->sampcnt] = (*longdata)[i] ;
            inc(con_ptr->sampcnt) ;
          end
        if (con_ptr->sampcnt >= detector->datapts)
          then
            begin
              con_ptr->sampcnt = 0 ; /* reset counter and process */
              longdata = (pointer)detector->insamps ; /* is buffered */
              realdata = (pointer)detector->insamps ;
            end
          else
            begin
              detector->remaining = FALSE ;
              return (con_ptr->itc > 0) ;
            end
      end
  else if (lnot detector->remaining)
    then
      begin
        con_ptr->etime = con_ptr->startt ;
        detector->sam_no = 0 ;
      end
  if (lnot detector->remaining)
    then
      begin
        inc(con_ptr->cur_rec) ;
        if (con_ptr->cur_rec >= CUR_MAX)
          then
            con_ptr->cur_rec = 0 ;
        if (detector->sam_ch < 0)
          then
            detector->sam_ch = detector->datapts ;
      end
  while (detector->sam_no < detector->sam_ch)
    begin
      if (realflag)
        then
          in_data = lib_round(((*realdata)[detector->sam_no]) * DET_SCALE_FACTOR) ;
        else
          in_data = ((*longdata)[detector->sam_no]) shl DET_SCALE_SHIFT ;

      del_amp = in_data - con_ptr->last_y ;
      inc(con_ptr->sum_s_c) ;
      if (del_amp < 0)
        then
          cur_slope = ST_NEG ;
        else
          cur_slope = ST_POS ;

      if (con_ptr->prev_slope != cur_slope)
        then
          begin

            con_ptr->s_amp = con_ptr->max_y - con_ptr->last_y ;
            con_ptr->tim_of_max = con_ptr->last_x ;
            con_ptr->rec_of_max = con_ptr->rec_last_x ;
            con_ptr->prev_slope = cur_slope ;
            con_ptr->max_y = con_ptr->last_y ;
            con_ptr->last_y = in_data ;
            con_ptr->last_x = detector->sam_no ;
            con_ptr->rec_last_x = con_ptr->cur_rec ;
            con_ptr->s_sum_sc = con_ptr->sum_s_c ;
            con_ptr->sum_s_c = 0 ;

            ab_amp = abs(con_ptr->s_amp) ;
            if (ab_amp > con_ptr->thx)
              then
                begin
                  if (Event (con_ptr))
                    then
                      begin
                        Onsetq (con_ptr) ;
                        Count_dn (con_ptr) ;
                        result = TRUE ;
                        inc(detector->sam_no) ;
                        break ;
                      end
                end
              else
                begin

                  if (ab_amp > con_ptr->maxamp)
                    then
                      con_ptr->maxamp = ab_amp ;
                  inc(con_ptr->index) ;
                  if (con_ptr->index == 20)
                    then
                      begin
                        if (con_ptr->maxamp > 0)
                          then
                            P_two (con_ptr, con_ptr->maxamp) ;

                        con_ptr->maxamp = 0 ;
                        con_ptr->index = 0 ;
                      end
                  if (Event (con_ptr))
                    then
                      begin
                        Onsetq (con_ptr) ;
                        Count_dn (con_ptr) ;
                        result = TRUE ;
                        inc(detector->sam_no) ;
                        break ;
                      end
                end
          end
        else
          begin
            con_ptr->last_y = in_data ;
            con_ptr->last_x = detector->sam_no ;
            con_ptr->rec_last_x = con_ptr->cur_rec ;
          end
      if (con_ptr->itc > 0)
        then
          begin
            Count_dn (con_ptr) ;
            result = TRUE ;
          end
      inc(detector->sam_no) ;
    end
  detector->remaining = (detector->sam_no < detector->sam_ch) ;
  return result ;
end

static void Cont_setup (tdet_packet *detector)
begin
  integer i ;
  pcon_sto con_ptr ;

  con_ptr = detector->cont ;
  memset(con_ptr, 0, sizeof(con_sto)) ;
  con_ptr->prev_slope = ST_NEITHER ;
  con_ptr->s_amp = 600000 ;
  con_ptr->th1 = 500000 ;
  con_ptr->th2 = 500000 ;
  con_ptr->th3 = 500000 ;
  con_ptr->twosd = 300000 ;
  con_ptr->thx = con_ptr->twosd shl 1 ;
  for (i = 0 ; i <= 15 ; i++)
    con_ptr->tsstak[i] = 1000000 ;
  con_ptr->itc = NOR_OUT ;
  detector->remaining = FALSE ;
  con_ptr->sample_rate = detector->samrte ;
  detector->sam_ch = -1 ;
  con_ptr->first_detection = TRUE ;
  con_ptr->detector_enabled = (detector->det_options and DO_RUN) ;
  con_ptr->default_enabled = (detector->det_options and DO_RUN) ;
  con_ptr->onsetdata = addr (detector->onset) ;
  con_ptr->parent = detector ;
end

/* this is a little weird because it was originally a nested procedure
   within te_detect and had to be moved outside of te_detect */
static void gettime (boolean high, pthreshold_control_struc ptcs,
                     tonset_base *pob, tdet_packet *pdet)
begin
  longint adj ;

  ptcs->new_onset = TRUE ;
  memset(pob, 0, sizeof(tonset_mh)) ;
  if (high)
    then
      begin
        pob->signal_amplitude = ptcs->peakhi ;
        pob->background_estimate = ptcs->parent->ucon.filhi ;
      end
    else
      begin
        pob->signal_amplitude = ptcs->peaklo ;
        pob->background_estimate = ptcs->parent->ucon.fillo ;
      end
  adj = pdet->sam_no - (ptcs->parent->ucon.n_hits - 1) ;
  pob->signal_onset_time.seed_fpt = ptcs->etime + adj / ptcs->sample_rate ;
end

boolean Te_detect (tdet_packet *detector)
begin
  pdataarray indatar ;
  psinglearray realdata ;
  boolean realflag ;
  longint in_data ;
  pthreshold_control_struc tcs_ptr ;
  tdetload *pdl ;

  tcs_ptr = detector->cont ;
  indatar = detector->indatar ;
  realdata = (pointer)indatar ;
  realflag = detector->singleflag ;
  pdl = addr(tcs_ptr->parent->ucon) ;
  tcs_ptr->new_onset = FALSE ;
  if (lnot detector->remaining)
    then
      begin
        detector->sam_ch = detector->datapts ;
        tcs_ptr->etime = tcs_ptr->startt ;
        detector->sam_no = 0 ;
      end
  while (detector->sam_no < detector->sam_ch)
    begin
      if (realflag)
        then
          in_data = lib_round((*realdata)[detector->sam_no]) ;
        else
          in_data = (*indatar)[detector->sam_no] ;
      if (in_data > tcs_ptr->peakhi)
        then
          tcs_ptr->peakhi = in_data ;
      if (in_data < tcs_ptr->peaklo)
        then
          tcs_ptr->peaklo = in_data ;
      if (in_data > pdl->filhi)
        then
          begin
            if (lnot tcs_ptr->hevon)
              then
                begin
                  inc(tcs_ptr->overhi) ;
                  if (tcs_ptr->overhi >= pdl->n_hits)
                    then
                      begin
                        tcs_ptr->hevon = TRUE ;
                        gettime (TRUE, tcs_ptr, tcs_ptr->onsetdata, detector) ;
                        inc(detector->sam_no) ;
                        break ;
                      end
                end
            tcs_ptr->waitdly = 0 ;
          end
      else if ((tcs_ptr->hevon) land (in_data < pdl->filhi - pdl->iwin))
        then
          begin
            inc(tcs_ptr->waitdly) ;
            if (tcs_ptr->waitdly > pdl->wait_blk)
              then
                begin
                  tcs_ptr->peakhi = -MAXLINT ;
                  tcs_ptr->overhi = 0 ;
                  tcs_ptr->hevon = FALSE ;
                end
          end
      if (in_data < pdl->fillo)
        then
          begin
            if (lnot tcs_ptr->levon)
              then
                begin
                  inc(tcs_ptr->overlo) ;
                  if (tcs_ptr->overlo >= pdl->n_hits)
                    then
                      begin
                        tcs_ptr->levon = TRUE ;
                        gettime (FALSE, tcs_ptr, tcs_ptr->onsetdata, detector) ;
                        inc(detector->sam_no) ;
                        break ;
                      end
                end
            tcs_ptr->waitdly = 0 ;
          end
      else if ((tcs_ptr->levon) land (in_data > pdl->fillo + pdl->iwin))
        then
          begin
            inc(tcs_ptr->waitdly) ;
            if (tcs_ptr->waitdly > pdl->wait_blk)
              then
                begin
                  tcs_ptr->peaklo = MAXLINT ;
                  tcs_ptr->overlo = 0 ;
                  tcs_ptr->levon = FALSE ;
                end
          end
      inc(detector->sam_no) ;
    end
  detector->remaining = (detector->sam_no < detector->sam_ch) ;
  return (tcs_ptr->hevon lor tcs_ptr->levon) ;
end

static void Tcs_setup (tdet_packet *detector)
begin
  pthreshold_control_struc tcs_ptr ;

  tcs_ptr = detector->cont ;
  memset(tcs_ptr, 0, sizeof(threshold_control_struc)) ;
  detector->remaining = FALSE ;
  tcs_ptr->first_detection = FALSE ;
  tcs_ptr->detector_enabled = (detector->det_options and DO_RUN) ;
  tcs_ptr->default_enabled = (detector->det_options and DO_RUN) ;
  tcs_ptr->peaklo = MAXLINT ;
  tcs_ptr->peakhi = -MAXLINT ;
  tcs_ptr->onsetdata = (tonset_base *)addr(detector->onset) ;
  tcs_ptr->parent = detector ;
  tcs_ptr->sample_rate = detector->samrte ;
end

void initialize_detector (pq330 q330, pdet_packet pdp, piirfilter pi)
begin
  boolean mh ;
  integer i, j ;
  pcon_sto pmh ;
  pthreshold_control_struc pt ;
  plcq q ;
  pdetector pdef ;

  pdef = pdp->detector_def ;
  q = pdp->parent ;
  mh = (pdef->dtype == MURDOCK_HUTT) ;
  if (q->raw_data_source == READ_PREV_STREAM)
    then
      begin
        pdp->indatar = addr(q->processed_stream) ;
        pdp->singleflag = TRUE ;
      end
    else
      begin
        pdp->indatar = q->databuf ;
        pdp->singleflag = FALSE ;
      end
  if (mh)
    then
      begin
        if (q->rate > 0)
          then
            begin
              pdp->samrte = q->rate ;
              if (q->rate >= MINPOINTS)
                then
                  begin
                    pdp->datapts = q->rate ;
                    if (q->raw_data_source == READ_PREV_STREAM)
                      then
                        pdp->grpsize = 1 ; /* processed data */
                      else
                        pdp->grpsize = 0 ; /*nonbuffered*/
                  end
                else
                  begin
                    if (q->raw_data_source == READ_PREV_STREAM)
                      then
                        pdp->grpsize = 1  ; /* processed data */
                      else
                        pdp->grpsize = q->rate ;
                    i = MINPOINTS ;
                    j = i div pdp->grpsize ;
                    if ((pdp->grpsize * j) != i) /*doesn't go evenly*/
                      then
                        i = pdp->grpsize * (j + 1) ;
                    pdp->datapts = i ;
                  end
            end
          else
            begin
              pdp->samrte = 1.0 / abs(q->rate) ;
              pdp->datapts = MINPOINTS ;
              pdp->grpsize = 1 ;
            end
        if (pdp->grpsize)
          then
            begin /*buffered*/
              if (pi)
                then
                  pdp->insamps_size = pdp->datapts * sizeof(tfloat) ;
                else
                  pdp->insamps_size = pdp->datapts * sizeof(longint) ;
              getbuf (q330, addr(pdp->insamps), pdp->insamps_size) ;
            end
          else
            pdp->insamps = NIL ;
      end
    else
      begin
        if (q->rate > 0)
          then
            begin
              pdp->datapts = q->rate ;
              pdp->samrte = q->rate ;
            end
          else
            begin
              pdp->datapts = 1 ;
              pdp->samrte = 1.0 / abs(q->rate) ;
            end
        pdp->insamps = NIL ;
      end
  if (mh)
    then
      begin
        getbuf (q330, addr(pmh), sizeof(con_sto)) ;
        pdp->cont = pmh ;
        pdp->cont_size = sizeof(con_sto) ;
        Cont_setup (pdp);
        memcpy(addr(pmh->parent->ucon), addr(pdef->uconst), sizeof(tdetload)) ;
      end
    else
      begin
        getbuf (q330, addr(pt), sizeof(threshold_control_struc)) ;
        pdp->cont = pt ;
        pdp->cont_size = sizeof(threshold_control_struc) ;
        Tcs_setup (pdp) ;
        memcpy(addr(pt->parent->ucon), addr(pdef->uconst), sizeof(tdetload)) ;
      end
  if (pi)
    then
      begin
        pdp->indatar = addr(pi->out) ;
        pdp->singleflag = TRUE ;
      end
end

enum tliberr lib_detstat (pq330 q330, tdetstat *detstat)
begin
  paqstruc paqs ;
  plcq q ;
  pdet_packet pd ;
  con_common *pcc ;
  tonedetstat *pone ;
  string15 s ;

  paqs = q330->aqstruc ;
  detstat->count = 0 ;
  if (q330->libstate != LIBSTATE_RUN)
    then
      return LIBERR_NOSTAT ;
  q = paqs->lcqs ;
  while ((detstat->count < MAX_DETSTAT) land (q))
    begin
      pd = q->det ;
      while ((detstat->count < MAX_DETSTAT) land (pd != NIL))
        begin
          pcc = pd->cont ;
          pone = addr(detstat->entries[detstat->count]) ;
          sprintf(addr(pone->name), "%s:%s", seed2string(q->location, q->seedname, addr(s)),
                  addr(pd->detector_def->detname)) ;
          pone->ison = pcc->detector_on ;
          pone->declared = pcc->detection_declared ;
          pone->first = pcc->first_detection ;
          pone->enabled = pcc->detector_enabled ;
          inc(detstat->count) ;
          pd = pd->link ;
        end
      q = q->link ;
    end
  return LIBERR_NOERR ;
end

void lib_changeenable (pq330 q330, tdetchange *detchange)
begin
  paqstruc paqs ;
  plcq q ;
  pdet_packet pd ;
  con_common *pcc ;
  char s[DETECTOR_NAME_LENGTH + 11] ;

  paqs = q330->aqstruc ;
  if (q330->libstate != LIBSTATE_RUN)
    then
      return ;
  q = paqs->lcqs ;
  while (q)
    begin
      pd = q->det ;
      while (pd)
        begin
          pcc = pd->cont ;
          sprintf(s, "%s:%s", seed2string(q->location, q->seedname, addr(s)),
                  addr(pd->detector_def->detname)) ;
          if (strcmp(addr(detchange->name), addr(s)) == 0)
            then
              begin
                pcc->detector_enabled = detchange->run_detector ;
                return ;
              end
          pd = pd->link ;
        end
      q = q->link ;
    end
end

#endif
