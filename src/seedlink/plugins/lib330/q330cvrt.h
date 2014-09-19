/*   Lib330 Q330 packet host <-> network definitions
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
*/
#ifndef q330cvrt_h
/* Flag this file as included */
#define q330cvrt_h
#define VER_Q330CVRT 1

#ifndef q330types_h
#include "q330types.h"
#endif

extern void storeqdphdr (pbyte *p, byte cmd, word lth, word seq, word ack) ;
extern void loadqdphdr (pbyte *p, tqdp *hdr) ;
extern void storerqsrv (pbyte *p, t64 *sn) ;
extern void loadsrvch (pbyte *p, tsrvch *chal) ;
extern void storesrvrsp (pbyte *p, tsrvresp *resp) ;
extern word loadcerr (pbyte *p) ;
extern void storedsrv (pbyte *p, t64 *sn) ;
extern void storepollsn (pbyte *p, tpoll *poll) ;
extern void loadmysn (pbyte *p, tmysn *mysn) ;
extern void storeslog (pbyte *p, tlog *slog) ;
extern void loadfgl (pbyte *p, tfgl *fgl) ;
extern void loadlog (pbyte *p, tlog *log) ;
extern void loadglob (pbyte *p, tglobal *glob) ;
extern void loadfix (pbyte *p, tfixed *fix) ;
extern void loadsensctrl (pbyte *p, tsensctrl *sensctrl) ;
extern void storerqstat (pbyte *p, longword bitmap) ;
extern longword loadstatmap (pbyte *p) ;
extern void loadglobalstat (pbyte *p, tstat_global *globstat) ;
extern void loadgpsstat (pbyte *p, tstat_gps *gpsstat) ;
extern void loadpwrstat (pbyte *p, tstat_pwr *pwrstat) ;
extern void loadboomstat (pbyte *p, tstat_boom *boomstat) ;
extern void loadpllstat (pbyte *p, tstat_pll *pllstat) ;
extern void loadgpssats (pbyte *p, tstat_sats *gpssats) ;
extern void loadarpstat (pbyte *p, tstat_arp *arpstat) ;
extern void loadlogstat (pbyte *p, tstat_log *logstat) ;
extern void loadserstat (pbyte *p, tstat_serial *serstat) ;
extern void loadethstat (pbyte *p, tstat_ether *ethstat) ;
extern void loadbalestat (pbyte *p, tstat_baler *balestat) ;
extern void loaddynstat (pbyte *p, tdyn_ips *dynstat) ;
extern void loadauxstat (pbyte *p, tstat_auxad *auxstat) ;
extern void loadssstat (pbyte *p, tstat_sersens *ssstat) ;
extern void storeumsg (pbyte *p, tuser_message *umsg) ;
extern void loadumsg (pbyte *p, tuser_message *umsg) ;
extern void loadroutes (pbyte *p, integer datalth, troutelist *routelist) ;
extern void loadgpsids (pbyte *p, tgpsid *gpsids) ;
extern void storeoldweb (pbyte *p, told_webadv *oldweb) ;
extern void storenewweb (pbyte *p, tnew_webadv *newweb) ;
extern void loaddevs (pbyte *p, integer datalth, tdevs *devs) ;
extern void storepinghdr (pbyte *p, tpinghdr *hdr) ;
extern void loadpinghdr (pbyte *p, tpinghdr *hdr) ;
extern void storepingstatreq (pbyte *p, longword bitmap) ;
extern void loadpinginfo (pbyte *p, tpinglimits *pinginfo) ;
extern void loadpingstathdr (pbyte *p, tpingstathdr *pingstathdr) ;
extern void storememhdr (pbyte *p, tmem *memhdr) ;
extern void loadmemhdr (pbyte *p, tmem *memhdr) ;
extern void loadseghdr (pbyte *p, tseghdr *seghdr) ;
extern void storedack (pbyte *p, tdp_ack *dack) ;
#ifndef OMIT_SDUMP
extern void loadgps2 (pbyte *p, tgps2 *gps2) ;
extern void loadman (pbyte *p, tman *man) ;
extern void loaddcp (pbyte *p, tdcp *dcp) ;
#endif

#endif
