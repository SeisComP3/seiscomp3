/*   Lib330 Status Dump Routine
     Copyright 2006-2010 Certified Software Corporation

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
    1 2006-10-29 rdr Fix length of "s1" in report_channel_and_preamp_settings.
    2 2007-09-07 rdr Add print_generated_rectotals.
    3 2010-03-27 rdr Add Q335 support.
    4 2010-05-09 rdr Some cosemetic Q335 changes.
    5 2010-07-25 rdr Change in Q335 PGA encoding.
    6 2012-07-08 rdr Fix clock type 2 display.
*/
#ifndef libverbose_h
#include "libverbose.h"
#endif

#ifndef OMIT_SDUMP
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif

static integer get_q335_gain (pq330 q330, word chan)
begin
  word w ;

  if (chan >= 3)
    then
      w = (q330->share.global.input_map shr (2 + (chan shl 1))) and 3 ;
    else
      w = (q330->share.global.input_map shr (chan shl 1)) and 3 ; /* get PGA gain bits */
  w = 1 shl w ;
  return w ;
end

static void report_channel_and_preamp_settings (pq330 q330)
begin
  word w, gm ;
  float pg ;
  float vpct[CHANNELS] ;
  string95 chenb, prenb, s ;
  string31 s1 ;

  chenb[0] = 0 ;
  prenb[0] = 0 ;
  gm = q330->share.global.gain_map ;
  for (w = 0 ; w <= CHANNELS - 1 ; w++)
    begin
      vpct[w] = 0.000002384 ;
      if (lnot q330->q335)
        then
          begin
            if ((w <= 2) land (q330->man.flags and MANF_26QAP1))
              then
                vpct[w] = 0.25 * vpct[w] ; /* 26 bit output */
            if ((w >= 3) land (q330->man.flags and MANF_26QAP2))
              then
                vpct[w] = 0.25 * vpct[w] ; /* 26 bit output */
          end
      sprintf(s1, "%d", w + 1) ;
      switch ((gm shr (w shl 1)) and 3) begin
        case GAIN_POFF :
          strcat(chenb, s1) ;
          if (q330->q335)
            then
              vpct[w] = vpct[w] / get_q335_gain (q330, w) ;
          break ;
        case GAIN_PON :
          strcat(chenb, s1) ;
          strcat(prenb, s1) ;
          if (q330->q335)
            then
              pg = 8.0 * get_q335_gain (q330, w) ;
            else
              begin
                pg = 30.0 ;
                if ((w <= 2) land (q330->man.qap13_type >= 2))
                  then
                    pg = 20.0 ;
                if ((w >= 3) land (q330->man.qap46_type >= 2))
                  then
                    pg = 20.0 ;
              end
          vpct[w] = vpct[w] / pg ;
          break ;
        default :
          vpct[w] = 0.0 ;
      end
    end
  sprintf(s, "Channels Enabled: %s Preamps ON channels: %s", chenb, prenb) ;
  libmsgadd(q330, LIBMSG_CHANINFO, addr(s)) ;
  libmsgadd(q330, LIBMSG_CHANINFO, "Channel Sensitivities (uV per count):") ;
  s[0] = 0 ;
  for (w = 0 ; w <= CHANNELS - 1 ; w++)
    if (vpct[w] > 0.0)
      then
        begin
          sprintf(s1, "%d:%6.4f ", w + 1, vpct[w] * 1.0e6) ;
          strcat(s, s1) ;
        end
  libmsgadd(q330, LIBMSG_CHANINFO, addr(s)) ;
end

static char *getgain (pq330 q330, integer idx, string31 *result)
begin
  float actual, desired ;

  actual = q330->dcp.gains[idx] ;
  desired = q330->man.ref_counts[idx] ;
  if (desired < 1)
    then
      strcpy(result, "Disabled ") ;
    else
      sprintf(result, "%5.3f%% ", ((actual - desired) / desired) * 100.0) ;
  return result ;
end

static void report_digitizer_gain_and_offet (pq330 q330)
begin
  string95 s, s1, s2 ;
  integer i ;

  libmsgadd(q330, LIBMSG_CAL, "Digitizer Calibration Results:") ;
  s[0] = 0 ;
  for (i = 0 ; i <= 2 ; i++)
    begin
      sprintf(s1, " %d:%d, %s", i + 1, (integer)q330->dcp.offsets[i], getgain(q330, i, addr(s2))) ;
      strcat(s, s1) ;
    end
  libmsgadd(q330, LIBMSG_CAL, addr(s)) ;
  s[0] = 0 ;
  for (i = 3 ; i <= 5 ; i++)
    begin
      sprintf(s1, " %d:%d, %s", i + 1, (integer)q330->dcp.offsets[i], getgain(q330, i, addr(s2))) ;
      strcat(s, s1) ;
    end
  libmsgadd(q330, LIBMSG_CAL, addr(s)) ;
end

static void log_nonblank (pq330 q330, string95 *s)
begin

  if ((*s)[0])
    then
      libmsgadd (q330, LIBMSG_GPSIDS, s) ;
end

#endif
void log_all_info (pq330 q330)
begin
#ifndef OMIT_SDUMP
  string95 s ;
  word w ;
  integer i, j ;
  longint v, l ;
  string31 s1, s2, s3, s4 ;
  tfixed *pfix ;
  tstat_global *psglob ;
  tclock *pclk ;
  tglobal *pglob ;
  tstat_boom *pboom ;
  tstat_gps *psgps ;
  tstat_pll *pspll ;
  tgps2 *pgps ;
  tstat_log *pslog ;

  pfix = addr(q330->share.fixed) ;
  sprintf(s, "Q330 Serial Number: %s", showsn(pfix->sys_num, addr(s1))) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  if (lnot q330->q335)
    then
      begin
        sprintf(s, "AMB Serial Number: %s", showsn(pfix->amb_num, addr(s1))) ;
        libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
      end
  sprintf(s, "Seismo 1 Serial Number: %s", showsn(pfix->seis1_num, addr(s1))) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  sprintf(s, "Seismo 2 Serial Number: %s", showsn(pfix->seis2_num, addr(s1))) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  sprintf(s, "QAPCHP 1 Serial Number: %d", (integer)pfix->qapchp1_num) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  sprintf(s, "QAPCHP 2 Serial Number: %d", (integer)pfix->qapchp2_num) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  sprintf(s, "KMI Property Tag Number: %d", (integer)pfix->property_tag) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  sprintf(s, "System Software Version: %d.%d", pfix->sys_ver shr 8, (integer)(pfix->sys_ver and 255)) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  if (q330->q335)
    then
      sprintf(s, "Core Processor Version: %d.%d", pfix->sp_ver shr 8, (integer)(pfix->sp_ver and 255)) ;
    else
      sprintf(s, "Slave Processor Version: %d.%d", pfix->sp_ver shr 8, (integer)(pfix->sp_ver and 255)) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  switch (pfix->cal_type) begin
    case 33 :
      strcpy(s1, "QCAL330") ;
      break ;
    case 35 :
      strcpy(s1, "QCAL335") ;
      break ;
    default :
      strcpy(s1, "Unknown") ;
  end
  sprintf(s, "Calibrator Type: %s", s1) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  sprintf(s, "Calibrator Version: %d.%d", (integer)(pfix->cal_ver shr 8), (integer)(pfix->cal_ver and 255)) ;
  libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
  switch (pfix->aux_type) begin
    case AUXAD_ID :
      libmsgadd(q330, LIBMSG_FIXED, "Auxiliary Board Type: AUXAD") ;
      sprintf(s, "Auxiliary Board Version: %d.%d", (integer)(pfix->aux_ver shr 8), (integer)(pfix->aux_ver and 255)) ;
      libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
      break ;
    default :
      libmsgadd(q330, LIBMSG_FIXED, "Auxiliary Board Type: None") ;
  end
  switch (pfix->clk_type) begin
    case 1 :
      strcpy(s1, "Motorola M12") ;
      break ;
    case 2 :
      strcpy(s1, "Fastrax IT530") ;
    default :
      strcpy(s1, "None") ;
  end
  sprintf(s, "Clock Type: %s", s1) ;
  libmsgadd(q330, LIBMSG_FIXED,  addr(s)) ;
  if (lnot q330->q335)
    then
      begin
        sprintf(s, "PLD Version: %d.%d", (integer)(pfix->pld_ver shr 8), (integer)(pfix->pld_ver and 255)) ;
        libmsgadd(q330, LIBMSG_FIXED, addr(s)) ;
      end
  if (q330->share.gpsids[0][0])
    then
      begin
        libmsgadd(q330, LIBMSG_GPSIDS, "GPS Engine Identification") ;
        log_nonblank(q330, addr(q330->share.gpsids[0])) ;
        log_nonblank(q330, addr(q330->share.gpsids[1])) ;
        log_nonblank(q330, addr(q330->share.gpsids[2])) ;
        log_nonblank(q330, addr(q330->share.gpsids[3])) ;
        log_nonblank(q330, addr(q330->share.gpsids[4])) ;
        log_nonblank(q330, addr(q330->share.gpsids[5])) ;
        log_nonblank(q330, addr(q330->share.gpsids[6])) ;
        log_nonblank(q330, addr(q330->share.gpsids[7])) ;
        log_nonblank(q330, addr(q330->share.gpsids[8])) ;
      end

  pglob = addr(q330->share.global) ;
  pclk = addr(q330->qclock) ;
  psglob = addr(q330->share.stat_global) ;
  sprintf(s, "Total Hours: %4.2f", (float)(psglob->total_time / 3600)) ;
  libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
  sprintf(s, "Power On Hours: %4.2f", (float)(psglob->power_time / 3600)) ;
  libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
  l = q330->share.fixed.last_reboot ;
  sprintf(s, "Time of Last Boot: %s", jul_string(l, addr(s1))) ;
  libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
  sprintf(s, "Total Number of Boots: %d", (integer)q330->share.fixed.reboots) ;
  libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
  l = psglob->last_resync ;
  sprintf(s, "Time of Last Re-Sync: %s", jul_string(l, addr(s1))) ;
  libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
  sprintf(s, "Total Number of Re-Syncs: %d", (integer)psglob->resyncs) ;
  libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
  i = pglob->samp_rates ;
  if ((i shr 8) and 3)
    then
      begin
        s1[0] = 0 ;
        for (i = 7 ; i >= 0 ; i--)
          if (psglob->stat_inp and (1 shl i))
            then
              strcat(s1, "1") ;
            else
              strcat(s1, "0") ;
        sprintf(s, "Status Inputs: %s", s1) ;
        libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
      end
  w = psglob->misc_inp ;
  if (w and 1)
    then
      strcpy(s1, "On") ;
    else
      strcpy(s1, "Off") ;
  if (w and 2)
    then
      strcpy(s2, "On") ;
    else
      strcpy(s2, "Off") ;
  if (w and 4)
    then
      strcpy(s3, "On") ;
    else
      strcpy(s3, "Off") ;
  if (w and 8)
    then
      strcpy(s4, "On") ;
    else
      strcpy(s4, "Off") ;
  sprintf(s, "AC OK: %s, Input 1,2: %s,%s, Analog Fault: %s", s1, s2, s2, s4) ;
  libmsgadd(q330, LIBMSG_GLSTAT, addr(s)) ;
  sprintf(s, "Clock Quality: %d%%", (integer)translate_clock(pclk, psglob->clock_qual, psglob->clock_loss)) ;
  libmsgadd(q330, LIBMSG_CLOCK, addr(s)) ;
  sprintf(s, "Clock quality mapping: L=%d T=%d H=%d N=%d zone=%d", (integer)pclk->q_locked, (integer)pclk->q_track,
          (integer)pclk->q_hold, (integer)pclk->q_never, (integer)pclk->zone) ;
  libmsgadd (q330, LIBMSG_CLOCK, addr(s)) ;
  if (psglob->usec_offset < 500000)
    then
      v = psglob->usec_offset ;
    else
      v = (psglob->usec_offset - 1000000) ;
  sprintf(s, "Clock Phase: %d usec. max allowed=%d", (integer)v, (integer)pglob->drift_tol) ;
  libmsgadd(q330, LIBMSG_CLOCK, addr(s)) ;

  pboom = addr(q330->share.stat_boom) ;
  libmsgadd(q330, LIBMSG_BOOM, "Boom positions:") ;
  s[0] = 0 ;
  for (i = 0 ; i <= 5 ; i++)
    begin
      j = pboom->booms[i] ;
      sprintf(s1, "Ch%d: %d ", i + 1, j) ;
      strcat(s, s1) ;
    end
  libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
  libmsgadd(q330, LIBMSG_BOOM, "Analog Status") ;
  sprintf(s, "Analog Positive Supply: %4.2fV", pboom->amb_pos * 0.01) ;
  libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
  sprintf(s, "Input Voltage: %4.2fV", pboom->supply * 0.15) ;
  libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
  sprintf(s, "System Temperature: %dC", (integer)sex(pboom->sys_temp)) ;
  libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
  sprintf(s, "Main Current: %dma", pboom->main_cur) ;
  libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
  sprintf(s, "Antenna Current: %dma", pboom->ant_cur) ;
  libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
  i = sex(pboom->seis1_temp) ;
  if (i != TEMP_UNKNOWN)
    then
      begin
        sprintf(s, "Seismo 1 Temperature: %dC", i) ;
        libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
      end
  i = sex(pboom->seis2_temp) ;
  if (i != TEMP_UNKNOWN)
    then
      begin
        sprintf(s, "Seismo 2 Temperature: %dC", i) ;
        libmsgadd(q330, LIBMSG_BOOM, addr(s)) ;
      end

  psgps = addr(q330->share.stat_gps) ;
  libmsgadd(q330, LIBMSG_GPS, "GPS Status") ;
  sprintf(s, "Time: %s", addr(psgps->time)) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "Date: %s", addr(psgps->date)) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "Fix Type: %s", addr(psgps->fix)) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "Height: %s", addr(psgps->height)) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "Latitude: %s", addr(psgps->lat)) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "Longitude: %s", addr(psgps->longt)) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  if (psgps->gpson)
    then
      sprintf(s, "On Time: %dmin", (integer)psgps->gpstime) ;
    else
      sprintf(s, "Off Time: %dmin", (integer)psgps->gpstime) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "Sat. Used: %d", (integer)psgps->sat_used) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "In View: %d", (integer)psgps->sat_view) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  sprintf(s, "Checksum Errors: %d", (integer)psgps->check_err) ;
  libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
  l = psgps->last_good ;
  if (l)
    then
      begin
        sprintf(s, "Last GPS timemark: %s", jul_string(l, addr(s1))) ;
        libmsgadd(q330, LIBMSG_GPS, addr(s)) ;
      end

  pspll = addr(q330->share.stat_pll) ;
  libmsgadd(q330, LIBMSG_PLL, "PLL Status") ;
  switch (pspll->state) begin
    case PLL_HOLD :
      strcpy(s1, "Hold") ;
      break ;
    case PLL_TRACK :
      strcpy(s1, "Track") ;
      break ;
    case PLL_LOCK :
      strcpy(s1, "Lock") ;
      break ;
    default :
      strcpy(s1, "Unknown") ;
  end
  sprintf(s, "State: %s", s1) ;
  libmsgadd(q330, LIBMSG_PLL, addr(s)) ;
  sprintf(s, "Intitial VCO: %8.6f", (float)pspll->start_km) ;
  libmsgadd(q330, LIBMSG_PLL, addr(s)) ;
  sprintf(s, "Time Error: %8.6f", (float)pspll->time_error) ;
  libmsgadd(q330, LIBMSG_PLL, addr(s)) ;
  sprintf(s, "RMS VCO: %9.7f", (float)pspll->rms_vco) ;
  libmsgadd(q330, LIBMSG_PLL, addr(s)) ;
  sprintf(s, "Best VCO: %4.2f", (float)(pspll->best_vco + 2048.0)) ;
  libmsgadd(q330, LIBMSG_PLL, addr(s)) ;
  sprintf(s, "Seconds Since Track or Lock: %3.1f", (float)(pspll->ticks_track_lock / 1000.0)) ;
  libmsgadd(q330, LIBMSG_PLL, addr(s)) ;
  i = sex(pspll->km) ;
  sprintf(s, "Vco Control: %d", (integer)(i + 2048)) ;
  libmsgadd(q330, LIBMSG_PLL, addr(s)) ;

  pgps = addr(q330->gps2) ;
  switch (pgps->mode and 7) begin
    case AG_INT :
      strcpy(s1, "internal GPS") ;
      break ;
    case AG_EXT :
      strcpy(s1, "external GPS") ;
      break ;
    case AG_ESEA :
      strcpy(s1, "external seascan") ;
      break ;
    case AG_NET :
      strcpy(s1, "network timing") ;
      break ;
    case AG_EACC :
      strcpy(s1, "external access to internal GPS") ;
      break ;
    default :
      strcpy(s1, "Unknown") ;
  end
  sprintf(s, "timing mode: %s", s1) ;
  libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
  if ((pgps->mode and 7) == AG_INT)
    then
      begin
        switch (pgps->flags and 3) begin
          case AG_CONT :
            strcpy(s1, "Continuous Operation") ;
            break ;
          case AG_MAX :
            strcpy(s1, "Until maximum on time") ;
            break ;
          case AG_PLL :
            strcpy(s1, "Until PLL lock") ;
            break ;
          case AG_GPS :
            strcpy(s1, "Until GPS time acquisition") ;
        end
        sprintf(s, "internal GPS power management mode: %s", s1) ;
        libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
      end
  if (pgps->initial_pll and 1)
    then
      strcpy(s, "PLL enabled, ") ;
    else
      strcpy(s, "PLL DISABLED, ") ;
  if (pgps->initial_pll and 2)
    then
      strcat(s, "allow 2D, ") ;
    else
      strcat(s, "REQUIRE 3D, ") ;
  if (pgps->initial_pll and 4)
    then
      strcat(s, "WARNING: EXPERIMENTAL TEMPCO ENABLED") ;
    else
      strcat(s, "tempco normal") ;
  libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
  if (((pgps->mode and 7) == AG_INT) land (pgps->flags and 3))
    then
      begin
        sprintf(s, "power off-time: %dm max on-time: %dm resync at: %d",
                (integer)pgps->off_time, (integer)pgps->max_on, (integer)pgps->resync) ;
        libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
      end
  sprintf(s, "PLL update: %ds  PLL lock criterion: %dus", (integer)pgps->interval, (integer)pgps->lock_usec) ;
  libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
  sprintf(s, "Pfrac: %4.2f", (float)pgps->pfrac) ;
  libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
  sprintf(s, "VCO slope: %9.7f", (float)pgps->vco_slope) ;
  libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
  sprintf(s, "VCO intercept: %9.7f", pgps->vco_intercept) ;
  libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;
  sprintf(s, "Km delta: %9.7f", pgps->km_delta) ;
  libmsgadd(q330, LIBMSG_GPSCFG, addr(s)) ;

  pslog = addr(q330->share.stat_log) ;
  switch (pslog->log_num) begin
    case LP_TEL1 :
      strcpy(s, "Logical Port 1 Status") ;
      break ;
    case LP_TEL2 :
      strcpy(s, "Logical Port 2 Status") ;
      break ;
    case LP_TEL3 :
      strcpy(s, "Logical Port 3 Status") ;
      break ;
    case LP_TEL4 :
      strcpy(s, "Logical Port 4 Status") ;
      break ;
  end
  libmsgadd(q330, LIBMSG_LOG, addr(s)) ;
  sprintf(s, "Data Packets Sent: %d", (integer)pslog->sent) ;
  libmsgadd(q330, LIBMSG_LOG, addr(s)) ;
  sprintf(s, "Flood Packets Sent: %d", (integer)pslog->fill) ;
  libmsgadd(q330, LIBMSG_LOG, addr(s)) ;
  sprintf(s, "Packets Re-Sent: %d", (integer)pslog->resends) ;
  libmsgadd(q330, LIBMSG_LOG, addr(s)) ;
  sprintf(s, "Sequence Errors: %d", (integer)pslog->seq) ;
  libmsgadd(q330, LIBMSG_LOG, addr(s)) ;
  sprintf(s, "Packet Buffer Used: %d", (integer)pslog->pack_used) ;
  libmsgadd(q330, LIBMSG_LOG, addr(s)) ;
  if (q330->share.stat_log.flags and LPSF_BADMEM)
    then
      libmsgadd(q330, LIBMSG_LOG, "WARNING: PACKET MEMORY REDUCED BECAUSE OF Q330 MEMORY FAULT") ;
  switch (pslog->phy_num) begin
    case PP_SER1 :
      strcpy(s1, "Serial 1") ;
      break ;
    case PP_SER2 :
      strcpy(s1, "Serial 2") ;
      break ;
    case PP_SER3 :
      strcpy(s1, "Serial 3") ;
      break ;
    case PP_ETH :
      strcpy(s1, "Ethernet") ;
      break ;
    default :
      strcpy(s1, "None") ;
  end
  sprintf(s, "Physical Port: %s", s1) ;
  libmsgadd(q330, LIBMSG_LOG, addr(s)) ;
  report_channel_and_preamp_settings (q330) ;
  if (lnot q330->q335)
    then
      report_digitizer_gain_and_offet (q330) ;
#endif
end

#ifndef OMIT_SEED
longword print_generated_rectotals (pq330 q330)
begin
  paqstruc paqs ;
  plcq q ;
  string m ;
  string31 s ;
  string15 s1 ;
  longint futuremr ;
  longword totrec ;
  boolean secondphase ;

  paqs = q330->aqstruc ;
  q = paqs->lcqs ;
  strcpy(m, "written:") ;
  totrec = 0 ;
  secondphase = FALSE ;
  while (q)
    begin
      futuremr = 0 ; /* forecast pending message record */
      if ((q == paqs->msg_lcq) land (q->com->ring) land (q->com->frame >= 2))
        then
          futuremr = 1 ;
      totrec = totrec + q->records_generated_session + futuremr ;
      if ((q->records_generated_session + futuremr) > 0)
        then
          begin
            if (strlen(m) >= 68)
              then
                begin
                  libmsgadd(q330, LIBMSG_TOTAL, addr(m)) ;
                  strcpy(m, "written:") ;
                end
            sprintf(s, " %s-%d", seed2string(q->location, q->seedname, addr(s1)),
                    q->records_generated_session + futuremr) ;
            strcat (m, s) ;
          end
      q = q->link ;
      if (q == NIL)
        then
          if (lnot secondphase)
            then
              begin
                secondphase = TRUE ;
                q = paqs->dplcqs ;
              end
    end
  if (strlen(m) > 8)
    then
      libmsgadd(q330, LIBMSG_TOTAL, addr(m)) ;
  return totrec ;
end
#endif
