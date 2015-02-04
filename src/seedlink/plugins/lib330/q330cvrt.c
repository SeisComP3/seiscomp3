/*   Lib330 Q330 packet host <-> network routines
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
    0 2006-09-28 rdr Created
    1 2007-07-16 rdr Add routines for baler commands.
    2 2008-07-31 rdr Update loadssstat to handle met3.
    3 2008-08-05 rdr Add reading of optional baler config block for c2_back.
    4 2008-09-02 rdr Fix loadssstat for multiple sensors.
    5 2009-02-08 rdr Add loadep. Add loadepstat.
    6 2009-02-21 rdr Add loadepcfg and storeepcfg.
    7 2009-03-11 rdr Add new fields in loadepstat.
    8 2009-04-18 rdr Changes due to field changes in EP structures.
    9 2010-04-14 rdr loadfestat updated.
   10 2010-12-26 rdr loadboomstat now handles sensor currents.
   11 2011-01-12 rdr Sensor currents now overlay cal_timeouts.
*/
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef q330types_h
#include "q330types.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef q330cvrt_h
#include "q330cvrt.h"
#endif

void storeqdphdr (pbyte *p, byte cmd, word lth, word seq, word ack)
begin

  storelongint (p, 0) ; /* for now */
  storebyte (p, cmd) ;
  storebyte (p, QDP_VERSION) ;
  storeword (p, lth) ;
  storeword (p, seq) ;
  storeword (p, ack) ;
end

/* Note - this must be called before any other loadxxxx routines since you
  first need to know what the command received was, and the other routines
  expect a pointer past the header which will be the updated pointer after
  this routine is called. */
void loadqdphdr (pbyte *p, tqdp *hdr)
begin

  hdr->crc = loadlongint (p) ;
  hdr->command = loadbyte (p) ;
  hdr->version = loadbyte (p) ;
  hdr->datalength = loadword (p) ;
  hdr->sequence = loadword (p) ;
  hdr->acknowledge = loadword (p) ;
end

void storerqsrv (pbyte *p, t64 *sn)
begin

#ifdef ENDIAN_LITTLE
  storelongword (p, (*sn)[1]) ;
  storelongword (p, (*sn)[0]) ;
#else
  storelongword (p, (*sn)[0]) ;
  storelongword (p, (*sn)[1]) ;
#endif
end ;

void loadsrvch (pbyte *p, tsrvch *chal)
begin

#ifdef ENDIAN_LITTLE
  chal->challenge[1] = loadlongword (p) ;
  chal->challenge[0] = loadlongword (p) ;
#else
  chal->challenge[0] = loadlongword (p) ;
  chal->challenge[1] = loadlongword (p) ;
#endif
  chal->dpip = loadlongword (p) ;
  chal->dpport = loadword (p) ;
  chal->dpreg = loadword (p) ;
end

void storesrvrsp (pbyte *p, tsrvresp *resp)
begin

#ifdef ENDIAN_LITTLE
  storelongword (p, resp->serial[1]) ;
  storelongword (p, resp->serial[0]) ;
  storelongword (p, resp->challenge[1]) ;
  storelongword (p, resp->challenge[0]) ;
#else
  storelongword (p, resp->serial[0]) ;
  storelongword (p, resp->serial[1]) ;
  storelongword (p, resp->challenge[0]) ;
  storelongword (p, resp->challenge[1]) ;
#endif
  storelongword (p, resp->dpip) ;
  storeword (p, resp->dpport) ;
  storeword (p, resp->dpreg) ;
#ifdef ENDIAN_LITTLE
  storelongword (p, resp->counter_chal[1]) ;
  storelongword (p, resp->counter_chal[0]) ;
#else
  storelongword (p, resp->counter_chal[0]) ;
  storelongword (p, resp->counter_chal[1]) ;
#endif
  storelongword (p, resp->md5result[0]) ;
  storelongword (p, resp->md5result[1]) ;
  storelongword (p, resp->md5result[2]) ;
  storelongword (p, resp->md5result[3]) ;
end

word loadcerr (pbyte *p)
begin

  return loadword (p) ;
end

void storedsrv (pbyte *p, t64 *sn)
begin

#ifdef ENDIAN_LITTLE
  storelongword (p, (*sn)[1]) ;
  storelongword (p, (*sn)[0]) ;
#else
  storelongword (p, (*sn)[0]) ;
  storelongword (p, (*sn)[1]) ;
#endif
end

void storepollsn (pbyte *p, tpoll *poll)
begin

  storeword (p, poll->mask) ;
  storeword (p, poll->match) ;
end

void loadmysn (pbyte *p, tmysn *mysn)
begin

#ifdef ENDIAN_LITTLE
  mysn->sys_num[1] = loadlongword (p) ;
  mysn->sys_num[0] = loadlongword (p) ;
#else
  mysn->sys_num[0] = loadlongword (p) ;
  mysn->sys_num[1] = loadlongword (p) ;
#endif
  mysn->property_tag = loadlongword (p) ;
  mysn->user_tag = loadlongword (p) ;
end

void storeslog (pbyte *p, tlog *slog)
begin
  integer i ;

  storeword (p, slog->lport) ;
  storeword (p, slog->flags) ;
  storeword (p, slog->perc) ;
  storeword (p, slog->mtu) ;
  storeword (p, slog->grp_cnt) ;
  storeword (p, slog->rsnd_max) ;
  storeword (p, slog->grp_to) ;
  storeword (p, slog->rsnd_min) ;
  storeword (p, slog->window) ;
  storeword (p, slog->dataseq) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    storeword (p, slog->freqs[i]) ;
  storeword (p, slog->ack_cnt) ;
  storeword (p, slog->ack_to) ;
  storelongword (p, slog->olddata) ;
  storeword (p, slog->eth_throttle) ;
  storeword (p, slog->full_alert) ;
  storeword (p, slog->auto_filter) ;
  storeword (p, slog->man_filter) ;
  storelongword (p, slog->spare) ;
end

void loadfgl (pbyte *p, tfgl *fgl)
begin

  fgl->gl_off = loadword (p) ;
  fgl->sc_off = loadword (p) ;
  fgl->lp_off = loadword (p) ;
  fgl->spare = loadword (p) ;
end

void loadlog (pbyte *p, tlog *log)
begin
  integer i ;

  log->lport = loadword (p) ;
  log->flags = loadword (p) ;
  log->perc = loadword (p) ;
  log->mtu = loadword (p) ;
  log->grp_cnt = loadword (p) ;
  log->rsnd_max = loadword (p) ;
  log->grp_to = loadword (p) ;
  log->rsnd_min = loadword (p) ;
  log->window = loadword (p) ;
  log->dataseq = loadword (p) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    log->freqs[i] = loadword (p) ;
  log->ack_cnt = loadword (p) ;
  log->ack_to = loadword (p) ;
  log->olddata = loadlongword (p) ;
  log->eth_throttle = loadword (p) ;
  log->full_alert = loadword (p) ;
  log->auto_filter = loadword (p) ;
  log->man_filter = loadword (p) ;
  log->spare = loadlongword (p) ;
end

void loadglob (pbyte *p, tglobal *glob)
begin
  integer i, j ;

  glob->clock_to = loadword (p) ;
  glob->initial_vco = loadword (p) ;
  glob->gps_backup = loadword (p) ;
  glob->samp_rates = loadword (p) ;
  glob->gain_map = loadword (p) ;
  glob->filter_map = loadword (p) ;
  glob->input_map = loadword (p) ;
  glob->web_port = loadword (p) ;
  glob->server_to = loadword (p) ;
  glob->drift_tol = loadword (p) ;
  glob->jump_filt = loadword (p) ;
  glob->jump_thresh = loadword (p) ;
  glob->cal_offset = loadint16 (p) ;
  glob->sensor_map = loadword (p) ;
  glob->sampling_phase = loadword (p) ;
  glob->gps_cold = loadword (p) ;
  glob->user_tag = loadlongword (p) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    for (j = 0 ; j <= FREQUENCIES - 1 ; j++)
      glob->scaling[i][j] = loadint16 (p) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    glob->offsets[i] = loadint16 (p) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    glob->gains[i] = loadint16 (p) ;
  glob->msg_map = loadlongword (p) ;
end

void loadfix (pbyte *p, tfixed *fix)
begin
  word w ;
  integer i ;

  fix->last_reboot = loadlongword (p) ;
  fix->reboots = loadlongword (p) ;
  fix->backup_map = loadlongword (p) ;
  fix->default_map = loadlongword (p) ;
  fix->cal_type = loadword (p) ;
  fix->cal_ver = loadword (p) ;
  fix->aux_type = loadword (p) ;
  fix->aux_ver = loadword (p) ;
  fix->clk_type = loadword (p) ;
  fix->flags = loadword (p) ;
  fix->sys_ver = loadword (p) ;
  fix->sp_ver = loadword (p) ;
  fix->pld_ver = loadword (p) ;
  fix->mem_block = loadword (p) ;
  fix->property_tag = loadlongword (p) ;
#ifdef ENDIAN_LITTLE
  fix->sys_num[1] = loadlongword (p) ;
  fix->sys_num[0] = loadlongword (p) ;
#else
  fix->sys_num[0] = loadlongword (p) ;
  fix->sys_num[1] = loadlongword (p) ;
#endif
#ifdef ENDIAN_LITTLE
  fix->amb_num[1] = loadlongword (p) ;
  fix->amb_num[0] = loadlongword (p) ;
#else
  fix->amb_num[0] = loadlongword (p) ;
  fix->amb_num[1] = loadlongword (p) ;
#endif
#ifdef ENDIAN_LITTLE
  fix->seis1_num[1] = loadlongword (p) ;
  fix->seis1_num[0] = loadlongword (p) ;
#else
  fix->seis1_num[0] = loadlongword (p) ;
  fix->seis1_num[1] = loadlongword (p) ;
#endif
#ifdef ENDIAN_LITTLE
  fix->seis2_num[1] = loadlongword (p) ;
  fix->seis2_num[0] = loadlongword (p) ;
#else
  fix->seis2_num[0] = loadlongword (p) ;
  fix->seis2_num[1] = loadlongword (p) ;
#endif
  fix->qapchp1_num = loadlongword (p) ;
  fix->int_sz = loadlongword (p) ;
  fix->int_used = loadlongword (p) ;
  fix->ext_sz = loadlongword (p) ;
  fix->flash_sz = loadlongword (p) ;
  fix->ext_used = loadlongword (p) ;
  fix->qapchp2_num = loadlongword (p) ;
  for (w = LP_TEL1 ; w <= LP_TEL4 ; w++)
    fix->log_sz[w] = loadlongword (p) ;
  fix->freq7 = loadbyte (p) ;
  fix->freq6 = loadbyte (p) ;
  fix->freq5 = loadbyte (p) ;
  fix->freq4 = loadbyte (p) ;
  fix->freq3 = loadbyte (p) ;
  fix->freq2 = loadbyte (p) ;
  fix->freq1 = loadbyte (p) ;
  fix->freq0 = loadbyte (p) ;
  for (i = 0 ; i <= FREQUENCIES - 1 ; i++)
    fix->ch13_delay[i] = loadlongint (p) ;
  for (i = 0 ; i <= FREQUENCIES - 1 ; i++)
    fix->ch46_delay[i] = loadlongint (p) ;
end

void loadsensctrl (pbyte *p, tsensctrl *sensctrl)
begin
  integer i ;

  for (i = 0 ; i <= 7 ; i++)
    (*sensctrl)[i] = loadlongword (p) ;
end

void storerqstat (pbyte *p, longword bitmap)
begin

  storelongword (p, bitmap) ;
end

longword loadstatmap (pbyte *p)
begin

  return loadlongword (p) ;
end ;

void loadglobalstat (pbyte *p, tstat_global *globstat)
begin

  globstat->aqctrl = loadword (p) ;
  globstat->clock_qual = loadword (p) ;
  globstat->clock_loss = loadword (p) ;
  globstat->current_voltage = loadword (p) ;
  globstat->sec_offset = loadlongword (p) ;
  globstat->usec_offset = loadlongword (p) ;
  globstat->total_time = loadlongword (p) ;
  globstat->power_time = loadlongword (p) ;
  globstat->last_resync = loadlongword (p) ;
  globstat->resyncs = loadlongword (p) ;
  globstat->gps_stat = loadword (p) ;
  globstat->cal_stat = loadword (p) ;
  globstat->sensor_map = loadword (p) ;
  globstat->cur_vco = loadword (p) ;
  globstat->data_seq = loadword (p) ;
  globstat->pll_flag = loadword (p) ;
  globstat->stat_inp = loadword (p) ;
  globstat->misc_inp = loadword (p) ;
  globstat->cur_sequence = loadlongword (p) ;
end

void loadgpsstat (pbyte *p, tstat_gps *gpsstat)
begin

  gpsstat->gpstime = loadword (p) ;
  gpsstat->gpson = loadword (p) ;
  gpsstat->sat_used = loadword (p) ;
  gpsstat->sat_view = loadword (p) ;
  loadstring (p, 10, gpsstat->time) ;
  loadstring (p, 12, gpsstat->date) ;
  loadstring (p, 6, gpsstat->fix) ;
  loadstring (p, 12, gpsstat->height) ;
  loadstring (p, 14, gpsstat->lat) ;
  loadstring (p, 14, gpsstat->longt) ;
  gpsstat->last_good = loadlongword (p) ;
  gpsstat->check_err = loadlongword (p) ;
end

void loadpwrstat (pbyte *p, tstat_pwr *pwrstat)
begin

  pwrstat->phase = loadword (p) ;
  pwrstat->battemp = loadint16 (p) ;
  pwrstat->capacity = loadword (p) ;
  pwrstat->depth = loadword (p) ;
  pwrstat->batvolt = loadword (p) ;
  pwrstat->inpvolt = loadword (p) ;
  pwrstat->batcur = loadint16 (p) ;
  pwrstat->absorption = loadword (p) ;
  pwrstat->float_ = loadword (p) ;
  pwrstat->alerts = loadbyte (p) ;
  pwrstat->loads_off = loadbyte (p) ;
end

void loadboomstat (pbyte *p, tstat_boom *boomstat, boolean q335)
begin
  integer i ;

  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    boomstat->booms[i] = loadint16 (p) ;
  boomstat->amb_pos = loadword (p) ;
  boomstat->amb_neg = loadword (p) ;
  boomstat->supply = loadword (p) ;
  boomstat->sys_temp = loadint16 (p) ;
  boomstat->main_cur = loadword (p) ;
  boomstat->ant_cur = loadword (p) ;
  boomstat->seis1_temp = loadint16 (p) ;
  boomstat->seis2_temp = loadint16 (p) ;
  if (q335)
    then
      begin
        boomstat->cal_timeouts = 0 ;
        boomstat->sensa_cur = loadword (p) ;
        boomstat->sensb_cur = loadword (p) ;
      end
    else
      begin
        boomstat->cal_timeouts = loadlongword (p) ;
        boomstat->sensa_cur = 0 ;
        boomstat->sensb_cur = 0 ;
      end
end

void loadpllstat (pbyte *p, tstat_pll *pllstat)
begin

  pllstat->start_km = loadsingle (p) ;
  pllstat->time_error = loadsingle (p) ;
  pllstat->rms_vco = loadsingle (p) ;
  pllstat->best_vco = loadsingle (p) ;
  pllstat->spare = loadlongint (p) ;
  pllstat->ticks_track_lock = loadlongword (p) ;
  pllstat->km = loadint16 (p) ;
  pllstat->state = loadword (p) ;
end

void loadgpssats (pbyte *p, tstat_sats *gpssats)
begin
  pbyte pstart ;
  integer i ;
  tstat_sat1 *psat1 ;

  pstart = *p ;
  gpssats->sathdr.sat_count = loadword (p) ;
  gpssats->sathdr.blk_size = loadword (p) ;
  for (i = 0 ; i <= gpssats->sathdr.sat_count - 1 ; i++)
    begin
      psat1 = addr(gpssats->sats[i]) ;
      psat1->num = loadword (p) ;
      psat1->elevation = loadint16 (p) ;
      psat1->azimuth = loadint16 (p) ;
      psat1->snr = loadword (p) ;
    end
  incn(pstart, gpssats->sathdr.blk_size) ;
  *p = pstart ;
end

void loadarpstat (pbyte *p, tstat_arp *arpstat)
begin
  pbyte pstart ;
  integer i ;
  tarp1 *parp1 ;

  pstart = *p ;
  arpstat->arphdr.arp_count = loadword (p) ;
  arpstat->arphdr.blk_size = loadword (p) ;
  for (i = 0 ; i <= arpstat->arphdr.arp_count - 1 ; i++)
    begin
      parp1 = addr(arpstat->arps[i]) ;
      parp1->ip = loadlongword (p) ;
      loadmac (p, addr(parp1->mac)) ;
      parp1->timeout = loadword (p) ;
    end
  incn(pstart, arpstat->arphdr.blk_size) ;
  *p = pstart ;
end

void loadlogstat (pbyte *p, tstat_log *logstat)
begin

  logstat->sent = loadlongword (p) ;
  logstat->resends = loadlongword (p) ;
  logstat->fill = loadlongword (p) ;
  logstat->seq = loadlongword (p) ;
  logstat->pack_used = loadlongword (p) ;
  logstat->last_ack = loadlongword (p) ;
  logstat->phy_num = loadword (p) ;
  logstat->log_num = loadword (p) ;
  logstat->retran = loadword (p) ;
  logstat->flags = loadword (p) ;
end

void loadserstat (pbyte *p, tstat_serial *serstat)
begin

  serstat->check = loadlongword (p) ;
  serstat->ioerrs = loadlongword (p) ;
  serstat->phy_num = loadword (p) ;
  serstat->spare = loadword (p) ;
  serstat->unreach = loadlongword (p) ;
  serstat->quench = loadlongword (p) ;
  serstat->echo = loadlongword (p) ;
  serstat->redirect = loadlongword (p) ;
  serstat->over = loadlongword (p) ;
  serstat->frame = loadlongword (p) ;
end

void loadethstat (pbyte *p, tstat_ether *ethstat)
begin

  ethstat->check = loadlongword (p) ;
  ethstat->ioerrs = loadlongword (p) ;
  ethstat->phy_num = loadword (p) ;
  ethstat->spare = loadword (p) ;
  ethstat->unreach = loadlongword (p) ;
  ethstat->quench = loadlongword (p) ;
  ethstat->echo = loadlongword (p) ;
  ethstat->redirect = loadlongword (p) ;
  ethstat->runt = loadlongword (p) ;
  ethstat->crc_err = loadlongword (p) ;
  ethstat->bcast = loadlongword (p) ;
  ethstat->ucast = loadlongword (p) ;
  ethstat->good = loadlongword (p) ;
  ethstat->jabber = loadlongword (p) ;
  ethstat->outwin = loadlongword (p) ;
  ethstat->txok = loadlongword (p) ;
  ethstat->miss = loadlongword (p) ;
  ethstat->collide = loadlongword (p) ;
  ethstat->linkstat = loadword (p) ;
  ethstat->spare2 = loadword (p) ;
  ethstat->spare3 = loadlongword (p) ;
end

void loadbalestat (pbyte *p, tstat_baler *balestat)
begin
  word w ;
  tsbaler1 *pbale1 ;

  for (w = PP_SER1 ; w <= PP_ETH ; w++)
    begin
      pbale1 = addr((*balestat)[w]) ;
      pbale1->last_on = loadlongword (p) ;
      pbale1->powerups = loadlongword (p) ;
      pbale1->baler_status = loadword (p) ;
      pbale1->baler_time = loadword (p) ;
    end
end

void loaddynstat (pbyte *p, tdyn_ips *dynstat)
begin
  word w ;

  for (w = PP_SER1 ; w <= PP_ETH ; w++)
    (*dynstat)[w] = loadlongword (p) ;
end

void loadauxstat (pbyte *p, tstat_auxad *auxstat)
begin
  pbyte pstart ;
  integer i, cnt ;

  pstart = *p ;
  auxstat->hdr.size = loadword (p) ;
  auxstat->hdr.packver = loadword (p) ;
  auxstat->hdr.aux_type = loadword (p) ;
  auxstat->hdr.aux_ver = loadword (p) ;
  cnt = (auxstat->hdr.size - 8) div 4 ; /* number of entries */
  for (i = 0 ; i <= cnt - 1 ; i++)
    auxstat->conversions[i] = loadlongword (p) ;
  for (i = cnt ; i <= 7 ; i++)
    auxstat->conversions[i] = 0 ;
  incn(pstart, auxstat->hdr.size) ;
  *p = pstart ;
end

void loadssstat (pbyte *p, tstat_sersens *ssstat)
begin
  word w ;
  pbyte pstart, pnext ;
  word sz, st, pp ;
  tssstat *pss ;

  pstart = *p ;
  ssstat->hdr.totalsize = loadword (p) ;
  ssstat->hdr.count = loadword (p) ;
  for (w = 1 ; w <= ssstat->hdr.count ; w++)
    begin
      pnext = p ;
      sz = loadword (p) ;
      incn(pnext, sz) ; /* bytes for this entry */
      st = loadword (p) ;
      pp = loadword (p) ;
      if (pp <= PP_SER2)
        then
          begin
            pss = addr(ssstat->sensors[pp]) ;
            pss->size = sz ;
            pss->sensor_type = st ;
            pss->phyport = pp ;
            pss->sps = loadword (p) ;
            pss->units = loadword (p) ;
            pss->int_time = loadword (p) ;
            pss->fracdig = loadword (p) ;
            pss->validmeas = loadword (p) ;
            pss->pressure = loadlongint (p) ;
            if (pss->validmeas and 0xE)
              then
                begin /* if have int temp or later fields, then read the field */
                  pss->temperature = loadlongint (p) ;
                  if (pss->validmeas and 0xC)
                    then
                      begin
                        pss->humidity = loadlongint (p) ;
                        pss->exttemp = loadlongint (p) ;
                      end
                end
            p = pnext ;
          end
        else
          break ; /* invalid entry, skip entire block */
    end
  incn(pstart, ssstat->hdr.totalsize) ;
  *p = pstart ;
end ;

void storeumsg (pbyte *p, tuser_message *umsg)
begin

  storelongword (p, umsg->sender) ;
  storestring (p, 80, addr(umsg->msg)) ;
end

void loadumsg (pbyte *p, tuser_message *umsg)
begin

  umsg->sender = loadlongword (p) ;
  loadstring (p, 80, addr(umsg->msg)) ;
end

void loadroutes (pbyte *p, integer datalth, troutelist *routelist)
begin
#define ROUTE1_SIZE 12 /* size in bytes of one routing entry */
  integer i ;
  troute1 *prt1 ;

  routelist->count = datalth div ROUTE1_SIZE ;
  for (i = 0 ; i <= routelist->count - 1 ; i++)
    begin
      prt1 = addr(routelist->routes[i]) ;
      prt1->rt_ip = loadlongword (p) ;
      prt1->rt_pp = loadword (p) ;
      prt1->rt_lp = loadword (p) ;
      prt1->heard = loadlongword (p) ;
    end
end

void loadgpsids (pbyte *p, tgpsid *gpsids)
begin
  integer i ;

  for (i = 0 ; i <= 8 ; i++)
    loadstring (p, 32, (pchar) addr((*gpsids)[i])) ;
end

void storeoldweb (pbyte *p, told_webadv *oldweb)
begin

  storeblock (p, 24, addr(oldweb->ip_port)) ;
  storeblock (p, 8, addr(oldweb->name)) ;
end

void storenewweb (pbyte *p, tnew_webadv *newweb)
begin

  storeblock (p, 8, addr(newweb->name)) ;
  storestring (p, 256, (pchar) addr(newweb->dpaddress)) ;
end

void loaddevs (pbyte *p, integer datalth, tdevs *devs)
begin
#define DEV1_SIZE 20
  integer i ;
  tdev1 *pdev1 ;

  devs->count = datalth div DEV1_SIZE ;
  for (i = 0 ; i <= devs->count - 1 ; i++)
    begin
      pdev1 = addr(devs->alldev[i]) ;
      pdev1->dev_addr = loadword (p) ;
      pdev1->dev_id = loadword (p) ;
      pdev1->dev_ver = loadword (p) ;
      pdev1->dev_opt = loadword (p) ;
#ifdef ENDIAN_LITTLE
      pdev1->dev_num[1] = loadlongword (p) ;
      pdev1->dev_num[0] = loadlongword (p) ;
#else
      pdev1->dev_num[0] = loadlongword (p) ;
      pdev1->dev_num[1] = loadlongword (p) ;
#endif
      pdev1->dev_static = loadword (p) ;
      pdev1->heard = loadword (p) ;
    end
end

void storepinghdr (pbyte *p, tpinghdr *hdr)
begin

  storeword (p, hdr->ping_type) ;
  storeword (p, hdr->ping_opt) ;
end

void loadpinghdr (pbyte *p, tpinghdr *hdr)
begin

  hdr->ping_type = loadword (p) ;
  hdr->ping_opt = loadword (p) ;
end

/* call storepinghdr first */
void storepingstatreq (pbyte *p, longword bitmap)
begin

  storelongword (p, bitmap) ;
end

/* call loadpinghdr first */
void loadpingstathdr (pbyte *p, tpingstathdr *pingstathdr)
begin

  pingstathdr->drift_tol = loadword (p) ;
  pingstathdr->umsg_count = loadword (p) ;
  pingstathdr->last_reboot = loadlongword (p) ;
  pingstathdr->spare1 = loadlongint (p) ;
  pingstathdr->spare2 = loadlongint (p) ;
end

/* call loadpinghdr first */
void loadpinginfo (pbyte *p, tpinglimits *pinginfo)
begin
  word w ;

  pinginfo->version = loadword (p) ;
  pinginfo->flags = loadword (p) ;
  pinginfo->tag_id = loadlongword (p) ;
#ifdef ENDIAN_LITTLE
  pinginfo->serialnum[1] = loadlongword (p) ;
  pinginfo->serialnum[0] = loadlongword (p) ;
#else
  pinginfo->serialnum[0] = loadlongword (p) ;
  pinginfo->serialnum[1] = loadlongword (p) ;
#endif
  for (w = LP_TEL1 ; w <= LP_TEL4 ; w++)
    pinginfo->packetsizes[w] = loadlongword (p) ;
  for (w = PP_SER1 ; w <= PP_ETH ; w++)
    pinginfo->triggers[w] = loadlongword (p) ;
  for (w = PP_SER1 ; w <= PP_ETH ; w++)
    pinginfo->advflags[w] = loadlongword (p) ;
  for (w = PP_SER1 ; w <= PP_ETH ; w++)
    pinginfo->dataport[w] = loadlongword (p) ;
  pinginfo->calibration_errors = loadword (p) ;
  pinginfo->sys_ver = loadword (p) ;
end

void storememhdr (pbyte *p, tmem *memhdr)
begin

  storelongword (p, memhdr->start) ;
  storeword (p, memhdr->count) ;
  storeword (p, memhdr->memtype) ;
end

void loadmemhdr (pbyte *p, tmem *memhdr)
begin

  memhdr->start = loadlongword (p) ;
  memhdr->count = loadword (p) ;
  memhdr->memtype = loadword (p) ;
end

void loadseghdr (pbyte *p, tseghdr *seghdr)
begin

  seghdr->segnum = loadword (p) ;
  seghdr->segtotal = loadword (p) ;
end

void storedack (pbyte *p, tdp_ack *dack)
begin
  integer i ;

  storeword (p, dack->new_throttle) ;
  storeword (p, dack->spare2) ;
  for (i = 0 ; i <= 3 ; i++)
    storelongword (p, dack->acks[i]) ;
  storelongword (p, dack->spare3) ;
end

void storebrdy (pbyte *p, tbrdy *brdy)
begin

#ifdef ENDIAN_LITTLE
  storelongword (p, brdy->sernum[1]) ;
  storelongword (p, brdy->sernum[0]) ;
#else
  storelongword (p, brdy->sernum[0]) ;
  storelongword (p, brdy->sernum[1]) ;
#endif
  storeblock (p, 2, addr(brdy->net)) ;
  storeblock (p, 6, addr(brdy->stn)) ;
  storeword (p, brdy->model) ;
  storeword (p, brdy->version) ;
  storesingle (p, brdy->disk_size) ;
#ifdef ENDIAN_LITTLE
  storelongword (p, brdy->balersn[1]) ;
  storelongword (p, brdy->balersn[0]) ;
#else
  storelongword (p, brdy->balersn[0]) ;
  storelongword (p, brdy->balersn[1]) ;
#endif
end

void loadback (pbyte *p, tback *back)
begin

#ifdef ENDIAN_LITTLE
  back->sernum[1] = loadlongword (p) ;
  back->sernum[0] = loadlongword (p) ;
#else
  back->sernum[0] = loadlongword (p) ;
  back->sernum[1] = loadlongword (p) ;
#endif
  back->q330_ip = loadlongword (p) ;
  back->poc_ip = loadlongword (p) ;
  back->log2_ip = loadlongword (p) ;
  back->bport = loadword (p) ;
  back->lport = loadword (p) ;
  back->webbps = loadword (p) ;
  back->flags = loadword (p) ;
  back->access_to = loadword (p) ;
  back->spare2 = loadword (p) ;
#ifdef ENDIAN_LITTLE
  back->balersn[1] = loadlongword (p) ;
  back->balersn[0] = loadlongword (p) ;
#else
  back->balersn[0] = loadlongword (p) ;
  back->balersn[1] = loadlongword (p) ;
#endif
  if (back->flags and BA_CFG)
    then
      begin
        back->size = loadword (p) ;
        back->phyport = loadword (p) ;
        back->balertype = loadword (p) ;
        back->version = loadword (p) ;
        loadblock (p, back->size, addr(back->opaque)) ;
      end
end

void loadepstat (pbyte *p, tstat_ep *epstat)
begin
  integer i, j ;
  tstat_oneep *ps ;

  for (i = PP_SER1 ; i <= PP_SER2 ; i++)
    begin
      ps = addr((*epstat)[i]) ;
      ps->start_km = loadsingle (p) ;
      ps->time_error = loadsingle (p) ;
      ps->best_vco = loadsingle (p) ;
      ps->ticks_track_lock = loadlongword (p) ; /* ticks since last track or lock */
      ps->km = loadlongint (p) ;
      ps->state = loadword (p) ; /* hold/track/lock */
      ps->spare1 = loadword (p) ;
#ifdef ENDIAN_LITTLE
      ps->serial[1] = loadlongword (p) ;
      ps->serial[0] = loadlongword (p) ;
#else
      ps->serial[0] = loadlongword (p) ;
      ps->serial[1] = loadlongword (p) ;
#endif
      ps->procid = loadlongword (p) ;
      ps->secs_boot = loadlongword (p) ; /* seconds since boot */
      ps->secs_resync = loadlongword (p) ; /* seconds since last resync */
      ps->resyncs = loadlongword (p) ; /* Total number of resyncs */
      ps->q330_comm_errors = loadlongword (p) ; /* communications errors from 330 */
      ps->ep_comm_errors = loadlongword (p) ; /* communications errors from EP */
      ps->spare2 = loadword (p) ;
      ps->sdi_count = loadword (p) ; /* number of SDI-12 devices active */
      ps->version = loadword (p) ; /* version and revision */
      ps->flags = loadword (p) ;
      ps->analog_chans = loadword (p) ;
      ps->ep_model = (enum tep_model)(loadbyte (p)) ;
      ps->ep_rev = loadbyte (p) ;
      ps->gains = loadlongword (p) ;
      ps->inp_volts = loadword (p) ; /* in .1 volt increments */
      ps->humidity = loadword (p) ; /* in percent */
      ps->pressure = loadlongword (p) ; /* in ubar */
      ps->temperature = loadlongint (p) ; /* in 0.1C increments */
      for (j = 0 ; j <= 3 ; j++)
        ps->adcounts[j] = loadlongint (p) ;
      for (j = 0 ; j < MAX_SDI ; j++)
        begin
          ps->sdistats[j].address = (char) loadbyte (p) ;
          ps->sdistats[j].phase = (enum tsdi_phase) loadbyte (p) ;
          ps->sdistats[j].driver = (enum tsdi_driver) loadbyte (p) ;
          ps->sdistats[j].spare1 = loadbyte (p) ;
          loadblock (p, 6, addr(ps->sdistats[j].model)) ;
          loadblock (p, 13, addr(ps->sdistats[j].serial)) ;
          ps->sdistats[j].spare2 = (char) loadbyte (p) ;
          loadblock (p, 3, addr(ps->sdistats[j].version)) ;
          ps->sdistats[j].spare3 = (char) loadbyte (p) ;
        end
#ifdef ENDIAN_LITTLE
      ps->adc_serial[1] = loadlongword (p) ;
      ps->adc_serial[0] = loadlongword (p) ;
#else
      ps->adc_serial[0] = loadlongword (p) ;
      ps->adc_serial[1] = loadlongword (p) ;
#endif
      ps->adc_model = (enum tadc_model)(loadbyte (p)) ;
      ps->adc_rev = loadbyte (p) ;
      ps->adc_spare1 = loadword (p) ;
      ps->adc_spare2 = loadlongword (p) ;
      for (j = 0 ; j <= 2 ; j++)
        ps->spares[j] = loadlongword (p) ;
    end
end

void loadfestats (pbyte *p, tstat_fes *fes)
begin
  integer i, j ;
  tfestat *pfs ;

  fes->hdr.count = loadword (p) ;
  fes->hdr.lth = loadword (p) ;
  if (fes->hdr.count < 2)
    then
      memset (addr(fes->boards[1]), 0, sizeof(tfestat)) ; /* no data for second board */
  for (i = 0 ; i < fes->hdr.count ; i++)
    begin
      pfs = addr(fes->boards[i]) ;
      pfs->start_km = loadsingle (p) ;
      pfs->time_error = loadsingle (p) ;
      pfs->best_vco = loadsingle (p) ;
      pfs->ticks_track_lock = loadlongword (p) ;
      pfs->km = loadlongint (p) ;
      pfs->state = loadword (p) ;
      pfs->flags = loadword (p) ;
      pfs->secs_resync = loadlongword (p) ;
      pfs->resyncs = loadlongword (p) ;
      pfs->secs_boot = loadlongword (p) ;
      pfs->cp_comm_errors = loadlongword (p) ;
      pfs->inp_volts = loadword (p) ;
      pfs->sensor_bitmap = loadword (p) ;
      pfs->cal_status = loadword (p) ;
      pfs->sensor_temp = loadint16 (p) ;
#ifdef ENDIAN_LITTLE
      pfs->sensor_serial[1] = loadlongword (p) ;
      pfs->sensor_serial[0] = loadlongword (p) ;
#else
      pfs->sensor_serial[0] = loadlongword (p) ;
      pfs->sensor_serial[1] = loadlongword (p) ;
#endif
      for (j = 0 ; j <= 3 ; i++)
        pfs->booms[j] = loadbyte (p) ;
    end
end

void loadepd (pbyte *p, tepdelay *epdelay)
begin
  integer i ;

  epdelay->ress[PP_SER1] = loadlongword (p) ;
  epdelay->ress[PP_SER2] = loadlongword (p) ;
  epdelay->chancnt = loadword (p) ;
  epdelay->spare = loadword (p) ;
  for (i = 0 ; i < epdelay->chancnt ; i++)
    epdelay->chandlys[i] = loadlongword (p) ;
end

void loadepcfg (pbyte *p, tepcfg *epcfg)
begin
  integer i ;

  memset (epcfg, 0, sizeof(tepcfg)) ;
  epcfg->ress[PP_SER1] = loadlongword (p) ;
  epcfg->ress[PP_SER2] = loadlongword (p) ;
  epcfg->flags[PP_SER1] = loadlongword (p) ;
  epcfg->flags[PP_SER2] = loadlongword (p) ;
  epcfg->chancnt = loadword (p) ;
  epcfg->spare = loadword (p) ;
  for (i = 0 ; i < epcfg->chancnt ; i++)
    begin
      epcfg->chanmasks[i].chan = loadbyte (p) ;
      epcfg->chanmasks[i].mask = loadbyte (p) ;
    end
end

void storeepcfg (pbyte *p, tepcfg *epcfg)
begin
  integer i ;

  storelongword (p, epcfg->ress[PP_SER1]) ;
  storelongword (p, epcfg->ress[PP_SER2]) ;
  storelongword (p, epcfg->flags[PP_SER1]) ;
  storelongword (p, epcfg->flags[PP_SER2]) ;
  storeword (p, epcfg->chancnt) ;
  storeword (p, epcfg->spare) ;
  for (i = 0 ; i < epcfg->chancnt ; i++)
    begin
      storebyte (p, epcfg->chanmasks[i].chan) ;
      storebyte (p, epcfg->chanmasks[i].mask) ;
    end
end

void loadcomm (pbyte *p, tcomm *comm)
begin
  byte lp ;
  integer i ;

#ifdef ENDIAN_LITTLE
  comm->serial[1] = loadlongword (p) ;
  comm->serial[0] = loadlongword (p) ;
#else
  comm->serial[0] = loadlongword (p) ;
  comm->serial[1] = loadlongword (p) ;
#endif
  comm->version = loadword (p) ;
  comm->active_lth = loadword (p) ;
  comm->mtu = loadword (p) ;
  comm->base_port = loadword (p) ;
  comm->eth_ip = loadlongword (p) ;
  comm->eth_mask = loadlongword (p) ;
  comm->eth_gate = loadlongword (p) ;
  comm->pwr_cycling = loadlongword (p) ;
  for (lp = LP_TEL1 ; lp <= LP_TEL4 ; lp++)
    begin
      comm->timeouts[lp].idle_timeout = loadword (p) ;
      comm->timeouts[lp].busy_timeout = loadword (p) ;
    end
  for (lp = LP_TEL1 ; lp <= LP_TEL4 ; lp++)
    comm->triggers[lp] = loadword (p) ;
  comm->min_off = loadword (p) ;
  comm->listopts = loadword (p) ;
  comm->max_off = loadword (p) ;
  comm->baler_min_perc = loadword (p) ;
  for (i = 0 ; i < IP_LIST_SIZE ; i++)
    begin
      comm->iplist[i].low = loadlongword (p) ;
      comm->iplist[i].high = loadlongword (p) ;
    end
  comm->eth_flags = loadlongword (p) ;
  comm->min_on = loadword (p) ;
  comm->spare = loadword (p) ;
  loadblock (p, 364, addr(comm->other_exp)) ;
end

void loadbalecfg (pbyte *p, tbalecfg *bcfg)
begin

  bcfg->sub_command = loadword (p) ;
  bcfg->sub_response = loadword (p) ;
  bcfg->size = loadword (p) ;
  bcfg->phyport = loadword (p) ;
  bcfg->balertype = loadword (p) ;
  bcfg->version = loadword (p) ;
  loadstring (p, 236, addr(bcfg->opaque)) ;
end

#ifndef OMIT_SDUMP
void loadgps2 (pbyte *p, tgps2 *gps2)
begin

  gps2->mode = loadword (p) ;
  gps2->flags = loadword (p) ;
  gps2->off_time = loadword (p) ;
  gps2->resync = loadword (p) ;
  gps2->max_on = loadword (p) ;
  gps2->lock_usec = loadword (p) ;
  gps2->spare2 = loadlongword (p) ;
  gps2->interval = loadword (p) ;
  gps2->initial_pll = loadword (p) ;
  gps2->pfrac = loadsingle (p) ;
  gps2->vco_slope = loadsingle (p) ;
  gps2->vco_intercept = loadsingle (p) ;
  gps2->max_ikm_rms = loadsingle (p) ;
  gps2->ikm_weight = loadsingle (p) ;
  gps2->km_weight = loadsingle (p) ;
  gps2->best_weight = loadsingle (p) ;
  gps2->km_delta = loadsingle (p) ;
  gps2->spare4 = loadlongword (p) ;
  gps2->spare5 = loadlongword (p) ;
end

void loadman (pbyte *p, tman *man)
begin
  integer i ;

  loadlongword (p) ; /* skip over password */
  loadlongword (p) ;
  loadlongword (p) ;
  loadlongword (p) ;
  man->qap13_type = loadword (p) ;
  man->qap13_ver = loadword (p) ;
  man->qap46_type = loadword (p) ;
  man->qap46_ver = loadword (p) ;
  man->qap13_num = loadlongword (p) ;
  man->qap46_num = loadlongword (p) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    man->ref_counts[i] = loadlongword (p) ;
  man->born_on = loadlongword (p) ;
  man->packet_sz = loadlongword (p) ;
  man->clk_type = loadword (p) ;
  man->model = loadword (p) ;
  man->def_cal_offset = loadint16 (p) ;
  man->flags = loadint16 (p) ;
  man->property_tag = loadlongword (p) ;
  man->expiration_time = loadlongword (p) ;
end

void loaddcp (pbyte *p, tdcp *dcp)
begin
  integer i ;

  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    dcp->offsets[i] = loadlongint (p) ;
  for (i = 0 ; i <= CHANNELS - 1 ; i++)
    dcp->gains[i] = loadlongint (p) ;
end
#endif
