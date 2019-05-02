/*   Lib330 Message Handlers
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
    0 2006-09-10 rdr Created
    1 2006-10-29 rdr In "showdot" add some castes just to make sure.
    2 2007-01-18 rdr Add LIBMSG_STATTO.
    3 2007-03-05 rdr Add LIBMSG_CONPURGE.
    4 2007-07-16 rdr Add Baler messages.
    5 2007-09-07 rdr Add LIBMSG_TOTAL.
    6 2008-02-18 rdr Don't use LOG lcq if network not yet set.
    7 2009-02-09 rdr Add EP Support.
    8 2009-09-28 rdr Add AUXMSG_DSS.
    9 2010-03-27 rdr Add messages for Q335.
   10 2010-08-21 rdr Change order of evaluation in msgadd and dump_msgqueue.
   11 2013-08-09 rdr Add conditional compilation to make string parameter to msgadd,
                     libmsgadd and libdataadd a const.
   12 2016-01-26 rdr Change LIBMSG_INVREG to indicate number of seconds.
*/
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

#ifndef OMIT_SEED
#ifndef liblogs_h
#include "liblogs.h"
#endif
#endif

char *lib_get_msg (word code, string95 *result)
begin
  string95 s ;

  sprintf(s, "Unknown Message Number %d ", code) ; /* default */
  switch (code div 100) begin
    case 0 : /* debugging */
      switch (code) begin
        case LIBMSG_GENDBG : strcpy(s, "") ; /* generic, all info in suffix */ break ;
        case LIBMSG_PKTIN : strcpy(s, "Recv") ; break ;
        case LIBMSG_PKTOUT : strcpy(s, "Sent") ; break ;
      end
      break ;
    case 1 : /* informational */
      switch (code) begin
        case LIBMSG_WINDOW : strcpy(s, "") ; /* all info in suffix */ break ;
        case LIBMSG_USER : strcpy(s, "Msg From ") ; break ;
        case LIBMSG_LOGCHG : strcpy(s, "Logical Port Configuration Change") ; break ;
        case LIBMSG_TOKCHG : strcpy(s, "DP Tokens have Changed") ; break ;
        case LIBMSG_MEMOP : strcpy(s, "Memory Operation already in progress") ; break ;
        case LIBMSG_DECNOTFOUND : strcpy(s, "Decimation filter not found for ") ; break ;
        case LIBMSG_FILTDLY : strcpy(s, "Filters & delay ") ; break ;
        case LIBMSG_DTOPEN : strcpy(s, "Sending DT_OPEN") ; break ;
        case LIBMSG_LINKRST : strcpy(s, "Link Reset, starting window sequence: ") ; break ;
        case LIBMSG_FILLJMP : strcpy(s, "Fill Jump from ") ; break ;
        case LIBMSG_SEQBEG : strcpy(s, "Sequence begins at ") ; break ;
        case LIBMSG_SEQOVER : strcpy(s, "Sequence resumes overlapping: ") ; break ;
        case LIBMSG_SEQRESUME : strcpy(s, "Sequence resumes: ") ; break ;
        case LIBMSG_CONTBOOT : strcpy(s, "Sequence continuity lost due to ") ; break ;
        case LIBMSG_CONTFND : strcpy(s, "Continuity found: ") ; break ;
        case LIBMSG_BUFSHUT : strcpy(s, "De-Registration due to reaching buffer empty percentage") ; break ;
        case LIBMSG_CONNSHUT : strcpy(s, "De-Registration due to reaching maximum connection time") ; break ;
        case LIBMSG_NOIP : strcpy(s, "No initial IP Address specified, waiting for POC") ; break ;
        case LIBMSG_CTRLDET : strcpy(s, "Control Detector ") ; break ;
        case LIBMSG_RESTCONT : strcpy(s, "Restoring continuity") ; break ;
        case LIBMSG_DISCARD : strcpy(s, "discarded 1: ") ; break ;
        case LIBMSG_CSAVE : strcpy(s, "continuity saved: ") ; break ;
        case LIBMSG_DETECT : strcpy(s, "") ; /* all info in suffix */ break ;
        case LIBMSG_NETSTN : strcpy(s, "Station: ") ; break ;
        case LIBMSG_ZONE : strcpy(s, "Changing Time Zone Adjustment from ") ; break ;
        case LIBMSG_AVG :
        case LIBMSG_TOTAL : strcpy(s, "") ; /* all info in suffix */ break ;
        case LIBMSG_EPDLYCHG : strcpy(s, "Environmental Processor Configuration Change") ; break ;
      end
      break ;
    case 2 : /* success code */
      switch (code) begin
        case LIBMSG_CREATED : strcpy(s, "Station Thread Created") ; break ;
        case LIBMSG_REGISTERED : strcpy(s, "Registered with Q330 ") ; break ;
        case LIBMSG_DEREGWAIT : strcpy(s, "De-Registering from Q330, Waiting for Acknowledgement") ; break ;
        case LIBMSG_DEALLOC : strcpy(s, "De-Allocating Data Structures") ; break ;
        case LIBMSG_COMBO : strcpy(s, "Combination Record Received") ; break ;
        case LIBMSG_GPSID : strcpy(s, "GPS Receiver ID Received") ; break ;
        case LIBMSG_DEREG : strcpy(s, "De-Registered with Q330") ; break ;
        case LIBMSG_READTOK : strcpy(s, "Starting to read DP Tokens") ; break ;
        case LIBMSG_TOKREAD : strcpy(s, "DP Tokens loaded, size=") ; break ;
        case LIBMSG_AQREM : strcpy(s, "Acquisition Structures Removed") ; break ;
        case LIBMSG_POCRECV : strcpy(s, "POC Received: ") ; break ;
        case LIBMSG_WRCONT : strcpy(s, "Writing Continuity File: ") ; break ;
        case LIBMSG_SOCKETOPEN : strcpy(s, "Socket Opened ") ; break ;
        case LIBMSG_DEREGTO : strcpy(s, "De-Registration Timeout") ; break ;
        case LIBMSG_BACK : strcpy(s, "Baler Acknowledged by Q330") ; break ;
        case LIBMSG_CONN : strcpy(s, "Connected to TCP Tunnel") ; break ;
        case LIBMSG_Q335 : strcpy(s, "Q335 Architecture Detected") ; break ;
      end
      break ;
    case 3 : /* converted Q330 blockettes */
      switch (code) begin
        case LIBMSG_GPSSTATUS : strcpy(s, "New GPS Status=") ; break ;
        case LIBMSG_DIGPHASE : strcpy(s, "New Digitizer Phase=") ; break ;
        case LIBMSG_SAVEBACKUP : strcpy(s, "Saving Configuration to Backup Memory") ; break ;
        case LIBMSG_SCHEDSTART : strcpy(s, "Start of Schedule") ; break ;
        case LIBMSG_SCHEDEND : strcpy(s, "End of Schedule") ; break ;
        case LIBMSG_LEAPDET : strcpy(s, "Leap Second Detected") ; break ;
        case LIBMSG_SMUPHASE : strcpy(s, "New SMU Phase=") ; break ;
        case LIBMSG_APWRON : strcpy(s, "Analog Power restored after Fault") ; break ;
        case LIBMSG_APWROFF : strcpy(s, "Analog Power shutdown due to Fault") ; break ;
        case LIBMSG_PHASERANGE : strcpy(s, "Phase Out of Range of ") ; break ;
        case LIBMSG_TIMEJMP : strcpy(s, "Time Jump of ") ; break ;
        case LIBMSG_INVBLKLTH : strcpy(s, "Invalid Blockette Size: ") ; break ;
        case LIBMSG_INVSPEC : strcpy(s, "Invalid Special Blockette ") ; break ;
        case LIBMSG_INVBLK : strcpy(s, "Invalid Blockette=") ; break ;
      end
      break ;
    case 4 : /* client faults */
      switch (code) begin
        case LIBMSG_BADIPADDR : strcpy(s, "Bad IP Address in Q330 Address Lookup") ; break ;
        case LIBMSG_DATADIS : strcpy(s, "Data Port not enabled in Q330") ; break ;
        case LIBMSG_PERM : strcpy(s, "No Permission") ; break ;
        case LIBMSG_PIU : strcpy(s, "Port in Use, Will retry registration in ") ; break ;
        case LIBMSG_SNR : strcpy(s, "Not Registered, Will retry registration in ") ; break ;
        case LIBMSG_INVREG : strcpy(s, "Invalid Registration Request, Will retry registration in ") ; break ;
        case LIBMSG_CALPROG : strcpy(s, "Calibration in Progress") ; break ;
        case LIBMSG_CMDABT : strcpy(s, "Command Aborted") ; break ;
        case LIBMSG_CONTIN : strcpy(s, "Continuity Error: ") ; break ;
        case LIBMSG_DATATO : strcpy(s, "Data Timeout, Will retry registratin in ") ; break ;
        case LIBMSG_CONCRC : strcpy(s, "Continuity CRC Error, Ignoring rest of file: ") ; break ;
        case LIBMSG_CONTNR : strcpy(s, "Continuity not restored") ; break ;
        case LIBMSG_STATTO : strcpy(s, "Status Timeout, Will retry registratin in ") ; break ;
        case LIBMSG_CONPURGE : strcpy(s, "Continuity was expired, Ignoring rest of file: ") ; break ;
        case LIBMSG_WRONGPORT : strcpy(s, "Wrong Data Port for Baler") ; break ;
      end
      break ;
    case 5 : /* server faults */
      switch (code) begin
        case LIBMSG_ROUTEFAULT : strcpy(s, "Possible Router Fault, ") ; break ;
        case LIBMSG_CANTSEND : strcpy(s, "Cannot send, error code: ") ; break ;
        case LIBMSG_SOCKETERR : strcpy(s, "Open Socket error: ") ; break ;
        case LIBMSG_BINDERR : strcpy(s, "Bind error: ") ; break ;
        case LIBMSG_RECVERR : strcpy(s, "Receive error: ") ; break ;
        case LIBMSG_PARERR : strcpy(s, "Parameter Error") ; break ;
        case LIBMSG_SNV : strcpy(s, "Structure Not Valid") ; break ;
        case LIBMSG_INVTOK : strcpy(s, "Invalid Tokens") ; break ;
        case LIBMSG_CMDCTRL : strcpy(s, "Command only valid on Control Port") ; break ;
        case LIBMSG_CMDSPEC : strcpy(s, "Command only valid on Special Functions Port") ; break ;
        case LIBMSG_CON : strcpy(s, "Command only valid on Console or IRDA") ; break ;
        case LIBMSG_RETRY : strcpy(s, "Retry of command type: ") ; break ;
        case LIBMSG_INVTVER : strcpy(s, "Invalid Token Version") ; break ;
        case LIBMSG_SEROPEN : strcpy(s, "Could not open Serial Interface ") ; break ;
        case LIBMSG_INVLTH : strcpy(s, "Invalid Packet Length: ") ; break ;
        case LIBMSG_SEQGAP : strcpy(s, "Sequence Gap ") ; break ;
        case LIBMSG_LBFAIL : strcpy(s, "Last block failed on ") ; break ;
        case LIBMSG_NILRING : strcpy(s, "NIL Ring on ") ; break ;
        case LIBMSG_UNCOMP : strcpy(s, "Uncompressable Data on ") ; break ;
        case LIBMSG_CONTERR : strcpy(s, "Continuity Error on ") ; break ;
        case LIBMSG_TIMEDISC : strcpy(s, "time label discontinuity: ") ; break ;
        case LIBMSG_RECOMP : strcpy(s, "Re-compression error on ") ; break ;
        case LIBMSG_SEGOVER : strcpy(s, "Segment buffer overflow on ") ; break ;
        case LIBMSG_TCPTUN : strcpy(s, "TCP Tunnelling error: ") ; break ;
        case LIBMSG_HFRATE : strcpy(s, "Sampling Rate mis-match ") ; break ;
      end
      break ;
    case 6 :
      strcpy(s, "") ; /* contained in suffix */
      break ;
    case 7 : /* aux server messages */
      switch (code) begin
        case AUXMSG_SOCKETOPEN : strcpy(s, "Socket Opened ") ; break ;
        case AUXMSG_SOCKETERR : strcpy(s, "Open Socket error: ") ; break ;
        case AUXMSG_BINDERR : strcpy(s, "Bind error: ") ; break ;
        case AUXMSG_LISTENERR : strcpy(s, "Listen error: ") ; break ;
        case AUXMSG_DISCON : strcpy(s, "Client Disconnected from ") ; break ;
        case AUXMSG_ACCERR : strcpy(s, "Accept error: ") ; break ;
        case AUXMSG_CONN : strcpy(s, "Client Connected ") ; break ;
        case AUXMSG_NOBLOCKS : strcpy(s, "No Blocks Available ") ; break ;
        case AUXMSG_SENT : strcpy(s, "Sent: ") ; break ;
        case AUXMSG_INVADDR : strcpy(s, "Invalid Web Server Override Address ") ; break ;
        case AUXMSG_WEBADV : strcpy(s, "Webserver advertising ") ; break ;
        case AUXMSG_RECVTO : strcpy(s, "Receive Timeout ") ; break ;
        case AUXMSG_WEBLINK : strcpy(s, "") ; /* contained in suffix */ break ;
        case AUXMSG_RECV : strcpy(s, "Recv: ") ; break ;
        case AUXMSG_DSS : strcpy(s, "") ; /* contained in suffix */ break ;
      end
      break ;
    case 8 :
      strcpy(s, "") ; /* contained in suffix */
      break ;
  end
  strcpy (result, s) ;
  return result ;
end

#ifndef OMIT_SEED
void dump_msgqueue (pq330 q330)
begin
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  if ((q330->libstate != LIBSTATE_TERM) land ((q330->network)[0]) land (q330->aqstruc) land
      (paqs->msgq_out != paqs->msgq_in) land (paqs->msg_lcq) land (paqs->msg_lcq->com->ring))
    then
      begin
        msglock (q330) ;
        while (paqs->msgq_out != paqs->msgq_in)
          begin
            log_message (q330, addr(paqs->msgq_out->msg)) ;
            paqs->msgq_out = paqs->msgq_out->link ;
          end
        msgunlock (q330) ;
      end
end
#endif

#ifdef CONSTMSG
void msgadd (pq330 q330, word msgcode, longword dt, const string95 *msgsuf, boolean client)
#else
void msgadd (pq330 q330, word msgcode, longword dt, string95 *msgsuf, boolean client)
#endif
begin
  string s, s1, s2 ;
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  msglock (q330) ;
  q330->msg_call.context = q330 ;
  q330->msg_call.timestamp = lib_round(now()) ;
  q330->msg_call.datatime = dt ;
  q330->msg_call.code = msgcode ;
  if (msgcode == 0)
    then
      s[0] = 0 ;
  strcpy(addr(q330->msg_call.suffix), msgsuf) ;
  if (dt)
    then
      sprintf(s1, "[%s] ", jul_string(dt, addr(s2))) ;
    else
      strcpy(s1, " ") ;
  sprintf(s, "{%d}%s%s%s", msgcode, s1, lib_get_msg (msgcode, addr(s2)), msgsuf) ;
  if (((msgcode div 100) != 7) lor (q330->cur_verbosity and VERB_AUXMSG))
    then
      begin
        inc(q330->msg_count) ;
        q330->msg_call.msgcount = q330->msg_count ;
#ifndef OMIT_SEED
        if (lnot q330->nested_log)
          then
            begin
              if ((client) lor (q330->libstate == LIBSTATE_TERM) lor ((q330->network)[0] == 0) lor
                  (paqs->msg_lcq == NIL) lor (paqs->msg_lcq->com->ring == NIL))
                then
                  begin
                    if (paqs->msgq_in->link != paqs->msgq_out)
                      then
                        begin
                          strcpy(addr(paqs->msgq_in->msg), addr(s)) ;
                          paqs->msgq_in = paqs->msgq_in->link ;
                        end
                  end
                else
                  begin
                    while (paqs->msgq_out != paqs->msgq_in)
                      begin
                        log_message (q330, addr(paqs->msgq_out->msg)) ;
                        paqs->msgq_out = paqs->msgq_out->link ;
                      end
                    log_message (q330, addr(s)) ;
                  end
            end
#endif
        if (q330->par_create.call_messages)
          then
            q330->par_create.call_messages (addr(q330->msg_call)) ;
      end
  msgunlock (q330) ;
end

#ifdef CONSTMSG
void libmsgadd (pq330 q330, word msgcode, const string95 *msgsuf)
#else
void libmsgadd (pq330 q330, word msgcode, string95 *msgsuf)
#endif
begin

  msgadd (q330, msgcode, 0, msgsuf, FALSE) ;
end

#ifdef CONSTMSG
void libdatamsg (pq330 q330, word msgcode, const string95 *msgsuf)
#else
void libdatamsg (pq330 q330, word msgcode, string95 *msgsuf)
#endif
begin
  longword dt ;
  paqstruc paqs ;

  paqs = q330->aqstruc ;
  dt = lib_round(paqs->data_timetag) ;
  msgadd (q330, msgcode, dt, msgsuf, FALSE) ;
end

char *lib_get_errstr (enum tliberr err, string63 *result)
begin
  string63 s ;

  strcpy(s, "Unknown Error Code") ;
  switch (err) begin
    case LIBERR_NOERR : strcpy(s, "No error") ; break ;
    case LIBERR_PERM : strcpy(s, "No Permission") ; break ;
    case LIBERR_TMSERV : strcpy(s, "Port in Use") ; break ;
    case LIBERR_NOTR : strcpy(s, "You are not registered") ; break ;
    case LIBERR_INVREG : strcpy(s, "Invalid Registration Request") ; break ;
    case LIBERR_PAR : strcpy(s, "Parameter Error") ; break ;
    case LIBERR_SNV : strcpy(s, "Structure not valid") ; break ;
    case LIBERR_CTRL : strcpy(s, "Control Port Only") ; break ;
    case LIBERR_SPEC : strcpy(s, "Special Port Only") ; break ;
    case LIBERR_MEM : strcpy(s, "Memory operation already in progress") ; break ;
    case LIBERR_CIP : strcpy(s, "Calibration in Progress") ; break ;
    case LIBERR_DNA : strcpy(s, "Data not available") ; break ;
    case LIBERR_DB9 : strcpy(s, "Console Port Only") ; break ;
    case LIBERR_MEMEW : strcpy(s, "Memory erase or Write Error") ; break ;
    case LIBERR_THREADERR : strcpy(s, "Could not create thread") ; break ;
    case LIBERR_BADDIR : strcpy(s, "Bad continuity directory") ; break ;
    case LIBERR_REGTO : strcpy(s, "Registration Timeout, ") ; break ;
    case LIBERR_STATTO : strcpy(s, "Status Timeout, ") ; break ;
    case LIBERR_DATATO : strcpy(s, "Data Timeout, ") ; break ;
    case LIBERR_NOSTAT : strcpy(s, "Your requested status is not yet available") ; break ;
    case LIBERR_INVSTAT : strcpy(s, "Your requested status in not a valid selection") ; break ;
    case LIBERR_CFGWAIT : strcpy(s, "Your requested configuration is not yet available") ; break ;
    case LIBERR_INVCFG : strcpy(s, "You can''t set that configuration") ; break ;
    case LIBERR_TOKENS_CHANGE : strcpy(s, "Tokens Changed") ; break ;
    case LIBERR_INVAL_TOKENS : strcpy(s, "Invalid Tokens") ; break ;
    case LIBERR_BUFSHUT : strcpy(s, "Shutdown due to reaching buffer percentage") ; break ;
    case LIBERR_CONNSHUT : strcpy(s, "Shutdown due to reaching buffer percentage") ; break ;
    case LIBERR_CLOSED : strcpy(s, "Closed by host") ; break ;
    case LIBERR_NETFAIL : strcpy(s, "Networking Failure") ; break ;
    case LIBERR_TUNBUSY : strcpy(s, "Tunnel Busy") ; break ;
    case LIBERR_INVCTX : strcpy(s, "Invalid Context") ; break ;
  end
  strcpy (result, s) ;
  return result ;
end

char *lib_get_statestr (enum tlibstate state, string63 *result)
begin
  string63 s ;

  strcpy(s, "Unknown State") ;
  switch (state) begin
    case LIBSTATE_IDLE : strcpy(s, "Not connected to Q330") ; break ;
    case LIBSTATE_TERM : strcpy(s, "Terminated") ; break ;
    case LIBSTATE_PING : strcpy(s, "Unregistered Ping") ; break ;
    case LIBSTATE_CONN : strcpy(s, "TCP Connect Wait") ; break ;
    case LIBSTATE_ANNC : strcpy(s, "Baler Announcement") ; break ;
    case LIBSTATE_REG : strcpy(s, "Requesting Registration") ; break ;
    case LIBSTATE_READCFG : strcpy(s, "Reading Configuration") ; break ;
    case LIBSTATE_READTOK : strcpy(s, "Reading Tokens") ; break ;
    case LIBSTATE_DECTOK : strcpy(s, "Decoding Tokens and allocating structures") ; break ;
    case LIBSTATE_RUNWAIT : strcpy(s, "Ready to Run") ; break ;
    case LIBSTATE_RUN : strcpy(s, "Running") ; break ;
    case LIBSTATE_DEALLOC : strcpy(s, "De-allocating structures") ; break ;
    case LIBSTATE_DEREG : strcpy(s, "De-registering") ; break ;
    case LIBSTATE_WAIT : strcpy(s, "Waiting for a new registration") ; break ;
  end
  strcpy (result, s) ;
  return result ;
end

char *showdot (longword num, string15 *result)
begin

  sprintf(result, "%d.%d.%d.%d", (integer)((num shr 24) and 255), (integer)((num shr 16) and 255),
          (integer)((num shr 8) and 255), (integer)(num and 255)) ;
  return result ;
end

char *command_name (byte cmd, string95 *result)
begin
  string95 s ;
  integer h ;

  strcpy (s, "Unknown") ;
  switch (cmd) begin
    case DT_DATA : strcpy(s, "Data Record") ; break ;
    case DT_FILL : strcpy(s, "Fill Record") ; break ;
    case DT_DACK : strcpy(s, "Data Acknowledge") ; break ;
    case DT_OPEN : strcpy(s, "Open Data Port") ; break ;
    case C1_CACK : strcpy(s, "Command Acknowledge") ; break ;
    case C1_RQSRV : strcpy(s, "Request Server Registration") ; break ;
    case C1_SRVCH : strcpy(s, "Server Challenge") ; break ;
    case C1_SRVRSP : strcpy(s, "Server Response") ; break ;
    case C1_CERR : strcpy(s, "Command Error") ; break ;
    case C1_DSRV : strcpy(s, "Delete Server") ; break ;
    case C1_POLLSN : strcpy(s, "Poll for Serial Number") ; break ;
    case C1_MYSN : strcpy(s, "My Serial Number") ; break ;
    case C1_SLOG : strcpy(s, "Set Logical Port") ; break ;
    case C1_RQLOG : strcpy(s, "Request Logical Port") ; break ;
    case C1_LOG : strcpy(s, "Logical Port") ; break ;
    case C1_RQSTAT : strcpy(s, "Request Status") ; break ;
    case C1_STAT : strcpy(s, "Status") ; break ;
    case C1_RQRT : strcpy(s, "Request Routing Table") ; break ;
    case C1_RT : strcpy(s, "Routing Table") ; break ;
    case C1_RQGID : strcpy(s, "Request GPS ID Strings") ; break ;
    case C1_GID : strcpy(s, "GPS ID Strings") ; break ;
    case C1_UMSG : strcpy(s, "Send User Message") ; break ;
    case C1_WEB : strcpy(s, "Webserver advertisement") ; break ;
    case C1_RQFGLS : strcpy(s, "Request Combination packet") ; break ;
    case C1_FGLS : strcpy(s, "Combination packet") ; break ;
    case C1_RQDEV : strcpy(s, "Request Devices") ; break ;
    case C1_DEV : strcpy(s, "Device list") ; break ;
    case C1_PING : strcpy(s, "Ping Q330") ; break ;
    case C1_RQMEM : strcpy(s, "Request Memory") ; break ;
    case C1_MEM : strcpy(s, "Memory Contents") ; break ;
    case C1_RQDCP : strcpy(s, "Request Calibration Packet") ; break ;
    case C1_DCP : strcpy(s, "Digitizer Calibration Packet") ; break ;
    case C1_RQMAN : strcpy(s, "Request Manufacturer''s Area") ; break ;
    case C1_MAN : strcpy(s, "Manufacturer''s Area") ; break ;
    case C1_COM : strcpy(s, "Communications packet") ; break ;
    case C2_RQGPS : strcpy(s, "Request GPS Parameters") ; break ;
    case C2_GPS : strcpy(s, "GPS Parameters") ; break ;
    case C2_BRDY : strcpy(s, "Baler Ready") ; break ;
    case C2_BOFF : strcpy(s, "Baler Off") ; break ;
    case C2_BACK : strcpy(s, "Baler Acknowledge") ; break ;
    default :
      begin
        h = cmd ;
        sprintf(s, "$%x", h) ;
      end
  end
  strcpy (result, s) ;
  return result ;
end

char *lib_gps_state (enum tgps_stat gs, string63 *result)
begin
  string63 s ;

  switch (gs) begin
    case GPS_OFF : strcpy(s, "Off") ; break ;
    case GPS_OFF_LOCK : strcpy(s, "Off due to case GPS Lock") ; break ;
    case GPS_OFF_PLL : strcpy(s, "Off due to PLL Lock") ; break ;
    case GPS_OFF_LIMIT : strcpy(s, "Off due to Time Limit") ; break ;
    case GPS_OFF_CMD : strcpy(s, "Off due to Command") ; break ;
    case GPS_ON : strcpy(s, "On") ; break ;
    case GPS_ON_AUTO : strcpy(s, "On automatically") ; break ;
    case GPS_ON_CMD : strcpy(s, "On by command") ; break ;
    case GPS_COLDSTART : strcpy(s, "Cold-start") ; break ;
    default : strcpy(s, "Florida") ;
  end
  strcpy (result, s) ;
  return result ;
end

char *lib_gps_fix (enum tgps_fix gf, string63 *result)
begin
  string63 s ;

  switch (gf) begin
    case GPF_LF : strcpy(s, "Off, never locked") ; break ;
    case GPF_OFF : strcpy(s, "Off, unknown lock") ; break ;
    case GPF_1DF : strcpy(s, "Off, last fix 1D") ; break ;
    case GPF_2DF : strcpy(s, "Off, last fix 2D") ; break ;
    case GPF_3DF : strcpy(s, "Off, last fix 3D") ; break ;
    case GPF_NL : strcpy(s, "On, never locked") ; break ;
    case GPF_ON : strcpy(s, "On, unknown lock") ; break ;
    case GPF_1D : strcpy(s, "1D Fix") ; break ;
    case GPF_2D : strcpy(s, "2D Fix") ; break ;
    case GPF_3D : strcpy(s, "3D Fix") ; break ;
    case GPF_NB : strcpy(s, "No GPS board") ; break ;
    default : strcpy(s, "Unknown Fix") ;
  end
  strcpy(result, s) ;
  return result ;
end

char *lib_pll_state (enum tpll_stat ps, string31 *result)
begin
  string31 s ;

  switch (ps) begin
    case PLS_LOCK : strcpy(s, "Locked") ; break ;
    case PLS_TRACK : strcpy(s, "Tracking") ; break ;
    case PLS_HOLD : strcpy(s, "Hold") ; break ;
    case PLS_OFF : strcpy(s, "Off") ; break ;
    default : strcpy(s, "Unknown") ;
  end
  strcpy(result, s) ;
  return result ;
end

char *lib_acc_types (enum tacctype acctype, string31 *result)
begin
  string31 s ;

  switch (acctype) begin
    case AC_GAPS : strcpy(s, "Data Gaps") ; break ;
    case AC_BOOTS : strcpy(s, "Re-Boots") ; break ;
    case AC_READ : strcpy(s, "Received Bps") ; break ;
    case AC_WRITE : strcpy(s, "Sent Bps") ; break ;
    case AC_COMATP : strcpy(s, "Comm. Attempts") ; break ;
    case AC_COMSUC : strcpy(s, "Comm. Successes") ; break ;
    case AC_PACKETS : strcpy(s, "Received Packets") ; break ;
    case AC_COMEFF : strcpy(s, "Comm. Efficiency") ; break ;
    case AC_POCS : strcpy(s, "POCs Received") ; break ;
    case AC_NEWIP : strcpy(s, "New IP Addresses") ; break ;
    case AC_DUTY : strcpy(s, "Duty Cycle") ; break ;
    case AC_THROUGH : strcpy(s, "Throughput") ; break ;
    case AC_MISSING : strcpy(s, "Missing Data") ; break ;
    case AC_FILL : strcpy(s, "Flood Packets") ; break ;
    case AC_CMDTO : strcpy(s, "Command Timeouts") ; break ;
    case AC_SEQERR : strcpy(s, "Sequence Errors") ; break ;
    case AC_CHECK : strcpy(s, "Checksum Errors") ; break ;
    case AC_IOERR : strcpy(s, "IO Errors") ; break ;
    default : strcpy(s, "Unknown") ;
  end
  strcpy(result, s) ;
  return result ;
end
