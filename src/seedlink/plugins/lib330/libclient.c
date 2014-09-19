/*   Lib330 client interface
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
    0 2006-09-10 rdr Created
    1 2007-07-16 rdr Fix physical interface status comparisons.
    2 2007-09-04 rdr Fix size of tstat_log memcpy.
    3 2008-04-14 rdr lib_get_status did not return LIBERR_NOSTAT if status not available.
*/
#ifndef q330types_h
#include "q330types.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libslider_h
#include "libslider.h"
#endif
#ifndef libstats_h
#include "libstats.h"
#endif
#ifndef libcont_h
#include "libcont.h"
#endif
#ifndef libsample_h
#include "libsample.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif
#ifndef libmd5_h
#include "libmd5.h"
#endif
#ifndef libcvrt_h
#include "libcvrt.h"
#endif
#ifndef libcompress_h
#include "libcompress.h"
#endif
#ifndef libcmds_h
#include "libcmds.h"
#endif
#ifndef libverbose_h
#include "libverbose.h"
#endif
#ifndef libtokens_h
#include "libtokens.h"
#endif
#ifndef libnetserv_h
#include "libnetserv.h"
#endif
#ifndef q330io_h
#include "q330io.h"
#endif
#ifndef q330cvrt_h
#include "q330cvrt.h"
#endif
#ifndef libpoc_h
#include "libpoc.h"
#endif

#ifndef OMIT_SEED
#ifndef libdetect_h
#include "libdetect.h"
#endif
#ifndef libctrldet_h
#include "libctrldet.h"
#endif
#ifndef libfilters_h
#include "libfilters.h"
#endif
#ifndef libopaque_h
#include "libopaque.h"
#endif
#ifndef liblogs_h
#include "liblogs.h"
#endif
#ifndef libarchive_h
#include "libarchive.h"
#endif
#endif

void lib_create_context (tcontext *ct, tpar_create *cfg) /* If ct = NIL return, check resp_err */
begin

  lib_create_330 (ct, cfg) ;
end

enum tliberr lib_destroy_context (tcontext *ct) /* Return error if any */
begin

  return lib_destroy_330 (ct) ;
end

enum tliberr lib_register (tcontext ct, tpar_register *rpar)
begin
  pq330 q330 ;
  enum tliberr result ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return LIBERR_INVCTX ;
  result = lib_register_330 (q330, rpar) ;
  if (q330->share.target_state == LIBSTATE_WAIT)
    then
      libmsgadd (q330, LIBMSG_NOIP, "") ;
  return result ;
end

enum tlibstate lib_get_state (tcontext ct, enum tliberr *err, topstat *retopstat)
begin
  pq330 q330 ;

  q330 = ct ;
  if (q330 == NIL)
    then
      begin
        *err = LIBERR_INVCTX ;
        return LIBSTATE_IDLE ; /* not really */
      end
  *err = q330->share.liberr ;
  if (retopstat == NIL)
    then
      return q330->libstate ;
  lock (q330) ;
  update_op_stats (q330) ;
  memcpy (retopstat, addr(q330->share.opstat), sizeof(topstat)) ;
  if (q330->libstate == LIBSTATE_RUN)
    then
      memcpy (addr(retopstat->slidecopy), addr(q330->share.slidestat), sizeof(tslidestat)) ;
    else
      memset (addr(retopstat->slidecopy), 0, sizeof(tslidestat)) ;
  unlock (q330) ;
  return q330->libstate ; ;
end

void lib_change_state (tcontext ct, enum tlibstate newstate, enum tliberr reason)
begin
  pq330 q330 ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return ;
  lock (q330) ;
  q330->share.target_state = newstate ;
  q330->share.liberr = reason ;
  unlock (q330) ;
end

void lib_request_status (tcontext ct, longword bitmap, word interval)
begin
  pq330 q330 ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return ;
  lock (q330) ;
  q330->share.extra_status = bitmap ;
  q330->share.status_interval = interval ;
  q330->share.interval_counter = interval + 1 ; /* client is probably in a hurry if they are changing status */
  unlock (q330) ;
end

enum tliberr lib_get_status (tcontext ct, longword bitnum, pointer buf)
begin
  pq330 q330 ;
  enum tliberr result ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return LIBERR_INVCTX ;
  lock (q330) ;
  if (q330->share.have_status and make_bitmap(bitnum))
    then
      begin
        result = LIBERR_NOERR ; /* assume good */
        switch (bitnum) begin
          case SRB_GLB :
            memcpy(buf, addr(q330->share.stat_global), sizeof(tstat_global)) ; /* Global Status */
            break ;
          case SRB_GST :
            memcpy(buf, addr(q330->share.stat_gps), sizeof(tstat_gps)) ; /* GPS Status */
            break ;
          case SRB_PWR :
            memcpy(buf, addr(q330->share.stat_pwr), sizeof(tstat_pwr)) ; /* Power supply Status */
            break ;
          case SRB_BOOM :
            memcpy(buf, addr(q330->share.stat_boom), sizeof(tstat_boom)) ; /* Boom positions ... */
            break ;
          case SRB_THR :
            result = LIBERR_INVSTAT ; /* Thread Status not supported */
            break ;
          case SRB_PLL :
            memcpy(buf, addr(q330->share.stat_pll), sizeof(tstat_pll)) ; /* PLL Status */
            break ;
          case SRB_GSAT :
            memcpy(buf, addr(q330->share.stat_sats), sizeof(tstat_sats)) ; /* GPS Satellites */
            break ;
          case SRB_ARP :
            memcpy(buf, addr(q330->share.stat_arp), sizeof(tstat_arp)) ; /* ARP Status */
            break ;
          case SRB_LOG1 :
          case SRB_LOG2 :
          case SRB_LOG3 :
          case SRB_LOG4 :
            if (bitnum == ((longword)SRB_LOG1 + q330->par_create.q330id_dataport))
              then
                memcpy(buf, addr(q330->share.stat_log), sizeof(tstat_log)) ; /* MY logical port status */
              else
                result = LIBERR_INVSTAT ;
            break ;
          case SRB_SER1 :
            if (q330->q330phy == PP_SER1)
              then
                memcpy(buf, addr(q330->share.stat_serial), sizeof(tstat_serial)) ;
              else
                result = LIBERR_INVSTAT ; /* Serial Port 1 Status */
            break ;
          case SRB_SER2 :
            if (q330->q330phy == PP_SER2)
              then
                memcpy(buf, addr(q330->share.stat_serial), sizeof(tstat_serial)) ;
              else
                result = LIBERR_INVSTAT ; /* Serial Port 2 Status */
            break ;
          case SRB_ETH :
            if (q330->q330phy == PP_ETH)
              then
                memcpy(buf, addr(q330->share.stat_ether), sizeof(tstat_ether)) ;
              else
                result = LIBERR_INVSTAT ; /* Ethernet Status */
            break ;
          case SRB_BALER :
            memcpy(buf, addr(q330->share.stat_baler), sizeof(tstat_baler)) ; /* Baler Status */
            break ;
          case SRB_DYN :
            memcpy(buf, addr(q330->share.stat_dyn), sizeof(tdyn_ips)) ; /* Dynamic IP Address */
            break ;
          case SRB_AUX :
            memcpy(buf, addr(q330->share.stat_auxad), sizeof(tstat_auxad)) ; /* Aux Board Status */
            break ;
          case SRB_SS :
            memcpy(buf, addr(q330->share.stat_sersens), sizeof(tstat_sersens)) ; /* Serial Sensor Status */
            break ;
          default :
            result = LIBERR_INVSTAT ;
            break ;
        end
      end
    else
      result = LIBERR_NOSTAT ;
  unlock (q330) ;
  return result ;
end

enum tliberr lib_get_config (tcontext ct, longword bitnum, pointer buf)
begin
  pq330 q330 ;
  enum tliberr result ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return LIBERR_INVCTX ;
  lock (q330) ;
  if (q330->share.have_config and make_bitmap(bitnum))
    then
      begin
        result = LIBERR_NOERR ; /* assume good */
        switch (bitnum) begin
          case CRB_GLOB :
            memcpy(buf, addr(q330->share.global), sizeof(tglobal)) ;
            break ;
          case CRB_FIX :
            memcpy(buf, addr(q330->share.fixed), sizeof(tfixed)) ; /* what other kind of tfixed is there? */
            break ;
          case CRB_GPSIDS :
            memcpy(buf, addr(q330->share.gpsids), sizeof(tgpsid)) ;
            break ;
          case CRB_SENSCTRL :
            memcpy(buf, addr(q330->share.sensctrl), sizeof(tsensctrl)) ;
            break ;
          default :
            begin /* perishable configuration */
              switch (bitnum) begin
                case CRB_LOG :
                  memcpy(buf, addr(q330->share.log), sizeof(tlog)) ;
                  break ;
                case CRB_ROUTES :
                  memcpy(buf, addr(q330->share.routelist), sizeof(troutelist)) ;
                  break ;
                case CRB_DEVS :
                  memcpy(buf, addr(q330->share.devs), sizeof(tdevs)) ;
                  break ;
              end
              q330->share.have_config = q330->share.have_config and not make_bitmap(bitnum) ; /* perishable once read */
            end
        end
      end
    else
      begin
        result = LIBERR_CFGWAIT ;
        q330->share.want_config = q330->share.want_config or make_bitmap(bitnum) ; /* request it */
      end
  unlock (q330) ;
  return result ;
end

enum tliberr lib_set_config (tcontext ct, longword bitnum, pointer buf)
begin
  pq330 q330 ;
  enum tliberr result ;

  result = LIBERR_INVCFG ; /* Your can't set that configuration */
  q330 = ct ;
  if (q330 == NIL)
    then
      return LIBERR_INVCTX ;
  lock (q330) ;
  switch (bitnum) begin
    case CRB_LOG :
      begin
        memcpy(addr(q330->share.newlog), buf, sizeof(tlog)) ;
        result = LIBERR_NOERR ;
        q330->share.log_changed = TRUE ;
      end
  end
  unlock (q330) ;
  return result ;
end

void lib_abort_command (tcontext ct)
begin
  pq330 q330 ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return ;
  lock (q330) ;
  q330->share.abort_requested = TRUE ;
  unlock (q330) ;
end

void lib_ping_request (tcontext ct, tpingreq *ping_req)
begin
  pq330 q330 ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return ;
  lock (q330) ;
  memcpy(addr(q330->share.pingreq), ping_req, sizeof(tpingreq)) ;
  q330->share.client_ping = CLP_REQ ;
  unlock (q330) ;
end

enum tliberr lib_unregistered_ping (tcontext ct, tpar_register *rpar)
begin

  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
    else
      return lib_unregping_330 (ct, rpar) ;
end

word lib_change_verbosity (tcontext ct, word newverb)
begin

  pq330 q330 ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return newverb ;
  lock (q330) ;
  q330->cur_verbosity = newverb ;
  unlock (q330) ;
  return newverb ;
end

enum tliberr lib_get_slidestat (tcontext ct, tslidestat *slidecopy)
begin
  pq330 q330 ;
  enum tliberr result ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return LIBERR_INVCTX ;
  lock (q330) ;
  if (q330->libstate == LIBSTATE_RUN)
    then
      begin
        memcpy (slidecopy, addr(q330->share.slidestat), sizeof(tslidestat)) ;
        result = LIBERR_NOERR ;
      end
    else
      result = LIBERR_NOSTAT ;
  unlock (q330) ;
  return result ;
end

void lib_send_usermessage (tcontext ct, string79 *umsg)
begin
  pq330 q330 ;
  tuser_message *pum ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return ;
  lock (q330) ;
  pum = addr(q330->share.user_message) ;
  q330->share.user_message.sender = 0 ;
  strncpy (addr(pum->msg), umsg, 79) ;
  q330->share.usermessage_requested = TRUE ;
  unlock (q330) ;
end

void lib_poc_received (tcontext ct, tpocmsg *poc)
begin
  pq330 q330 ;
  string15 newaddr ;

  q330 = ct ;
  if (q330 == NIL)
    then
      return ;
  lock (q330) ;
  add_status (q330, AC_POCS, 1) ;
  if (q330->libstate == LIBSTATE_WAIT)
    then
      begin
        showdot(poc->new_ip_address, addr(newaddr)) ;
        if (strcmp(addr(q330->par_register.q330id_address), addr(newaddr)))
          then
            add_status (q330, AC_NEWIP, 1) ;
        strcpy(addr(q330->par_register.q330id_address), addr(newaddr)) ;
        q330->par_register.q330id_baseport = poc->new_base_port ;
        q330->q330cport = q330->par_register.q330id_baseport + (2 * (q330->par_create.q330id_dataport + 1)) ;
        q330->q330dport = q330->q330cport + 1 ;
        libmsgadd (q330, LIBMSG_POCRECV, poc->log_info) ;
        q330->dynip_age = 0 ;
        unlock (q330) ; /* lib_change_state locks again */
        lib_change_state (q330, LIBSTATE_RUNWAIT, LIBERR_NOERR) ;
        return ;
      end
  unlock (q330) ;
end

enum tliberr lib_get_commevents (tcontext ct, tcommevents *commevents)
begin

  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
    else
      return lib_commevents (ct, commevents) ;
end

#ifndef OMIT_SEED
void lib_set_commevent (tcontext ct, integer number, boolean seton)
begin

  if (ct)
    then
      lib_setcommevent (ct, number, seton) ;
end

enum tliberr lib_get_detstat (tcontext ct, tdetstat *detstat)
begin

  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
    else
      return lib_detstat (ct, detstat) ; /* ask someone who knows */
end

enum tliberr lib_get_ctrlstat (tcontext ct, tctrlstat *ctrlstat)
begin

  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
    else
      return lib_ctrlstat (ct, ctrlstat) ;
end

enum tliberr lib_get_lcqstat (tcontext ct, tlcqstat *lcqstat)
begin

  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
    else
      return lib_lcqstat (ct, lcqstat) ;
end

void lib_change_enable (tcontext ct, tdetchange *detchange)
begin

  if (ct)
    then
      lib_changeenable (ct, detchange) ;
end
#endif

enum tliberr lib_get_dpcfg (tcontext ct, tdpcfg *dpcfg)
begin

  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
    else
      return lib_getdpcfg (ct, dpcfg) ;
end

void lib_msg_add (tcontext ct, word msgcode, longword dt, string95 *msgsuf)
begin

  if (ct)
    then
      msgadd (ct, msgcode, dt, msgsuf) ;
end

void lib_webadvertise (tcontext ct, string15 *stnname, string *dpaddr)
begin
  pq330 q330 ;

  q330 = ct ;
  if (ct == NIL)
    then
      return ;
  lock (q330) ;
  while (strlen(stnname) < 8)
    strcat(stnname, " ") ;
  if (q330->share.fixed.flags and FF_NWEB)
    then
      begin
        memcpy(addr(q330->share.new_webadv.name), stnname, 8) ;
        memcpy(addr(q330->share.new_webadv.dpaddress), dpaddr, sizeof(string)) ;
      end
    else
      begin
        memcpy(addr(q330->share.old_webadv.name), stnname, 8) ;
        while (strlen(dpaddr) < 24)
          strcat (dpaddr, " ") ;
        memcpy(addr(q330->share.old_webadv.ip_port), dpaddr, 24) ;
      end
  q330->share.webadv_requested = TRUE ;
  unlock (q330) ;
end

enum tliberr lib_send_tunneled (tcontext ct, byte cmd, byte response, pointer buf, integer req_size)
begin
  pq330 q330 ;
  enum tliberr result ;

  q330 = ct ;
  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
  lock (q330) ;
  if (q330->share.tunnel_state == TS_READY)
    then
      q330->share.tunnel_state = TS_IDLE ; /* throw away last result */
  if (q330->share.tunnel_state != TS_IDLE)
    then
      result = LIBERR_TUNBUSY ;
    else
      begin
        q330->share.tunnel.reqcmd = cmd ; /* what to send */
        q330->share.tunnel.respcmd = response ; /* what we expect */
        q330->share.tunnel.paysize = req_size ;
        memcpy (addr(q330->share.tunnel.payload), buf, req_size) ;
        q330->share.tunnel_state = TS_REQ ;
        result = LIBERR_NOERR ;
      end
  unlock (q330) ;
  return result ;
end

enum tliberr lib_get_tunneled (tcontext ct, byte *response, pointer buf, integer *resp_size)
begin
  pq330 q330 ;
  enum tliberr result ;

  q330 = ct ;
  if (ct == NIL)
    then
      return LIBERR_INVCTX ;
  lock (q330) ;
  if (q330->share.tunnel_state != TS_READY)
    then
      result = LIBERR_TUNBUSY ;
    else
      begin
        *response = q330->share.tunnel.respcmd ;
        *resp_size = q330->share.tunnel.paysize ;
        memcpy (buf, addr(q330->share.tunnel.payload), *resp_size) ;
        result = LIBERR_NOERR ;
      end
  unlock (q330) ;
  return result ;
end

const tmodules modules =
   {{/*name*/"LibClient", /*ver*/VER_LIBCLIENT},     {/*name*/"LibStrucs", /*ver*/VER_LIBSTRUCS},
    {/*name*/"LibTypes", /*ver*/VER_LIBTYPES},       {/*name*/"LibMsgs", /*ver*/VER_LIBMSGS},
    {/*name*/"LibSupport", /*ver*/VER_LIBSUPPORT},   {/*name*/"LibStats", /*ver*/VER_LIBSTATS},
    {/*name*/"LibSlider", /*ver*/VER_LIBSLIDER},     {/*name*/"LibSeed", /*ver*/VER_LIBSEED},
    {/*name*/"LibSample", /*ver*/VER_LIBSAMPLE},     {/*name*/"LibSampglob", /*ver*/VER_LIBSAMPGLOB},
    {/*name*/"LibSampcfg", /*ver*/VER_LIBSAMPCFG},   {/*name*/"LibMD5", /*ver*/VER_LIBMD5},
    {/*name*/"LibCvrt", /*ver*/VER_LIBCVRT},         {/*name*/"LibCont", /*ver*/VER_LIBCONT},
    {/*name*/"LibCompress", /*ver*/VER_LIBCOMPRESS}, {/*name*/"LibCmds", /*ver*/VER_LIBCMDS},
    {/*name*/"LibVerbose", /*ver*/VER_LIBVERBOSE},   {/*name*/"LibTokens", /*ver*/VER_LIBTOKENS},
#ifndef OMIT_SEED
    {/*name*/"LibOpaque", /*ver*/VER_LIBOPAQUE},     {/*name*/"LibLogs", /*ver*/VER_LIBLOGS},
    {/*name*/"LibFilters", /*ver*/VER_LIBFILTERS},   {/*name*/"LibDetect", /*ver*/VER_LIBDETECT},
    {/*name*/"LibCtrlDet", /*ver*/VER_LIBCTRLDET},   {/*name*/"LibArchive", /*ver*/VER_LIBARCHIVE},
    {/*name*/"LibNetServ", /*ver*/VER_LIBNETSERV},
#endif
    {/*name*/"Q330Types", /*ver*/VER_Q330TYPES},     {/*name*/"Q330IO", /*ver*/VER_Q330IO},
    {/*name*/"Q330Cvrt", /*ver*/VER_Q330CVRT},       {/*name*/"LibPOC", /*ver*/VER_LIBPOC},
    {/*name*/"", /*ver*/0}} ;

pmodules lib_get_modules (void)
begin

  return addr(modules) ;
end

