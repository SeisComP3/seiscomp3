/*   Lib330 Control Detector Routines
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
    1 2006-10-29 rdr Fix call to stackdetop in expand_control_detectors.
    2 2008-01-29 rdr Use new platform specific "uninteger" for pointer comparisons.
    3 2011-08-05 rdr Don't use NULL to zero byte variables to zero, use zero.
*/
#ifndef OMIT_SEED
#ifndef libctrldet_h
#include "libctrldet.h"
#endif

#ifndef libsampcfg_h
#include "libsampcfg.h"
#endif
#ifndef libdetect_h
#include "libdetect.h"
#endif
#ifndef libmsgs_h
#include "libmsgs.h"
#endif
#ifndef libsupport_h
#include "libsupport.h"
#endif

enum rettype {DETP, OP, TMP} ;
typedef struct {
  enum rettype t ;
  union {
    pboolean dx ;
    byte o ;
  } RETUNION ;
} ret ;
#define ret_dx RETUNION.dx
#define ret_o RETUNION.o

typedef struct { /* "C" doesn't allow nested procedures, create structure to simulate */
  ret result, nextoke ;
  pdetector_operation curpt ;
  pntrint tempcnt ;
  pcontrol_detector pcs ;
  pdop pop ;
  pq330 q330 ;
  paqstruc paqs ;
} texpand ;

static void stackdetop (texpand *pexp, pboolean tosdetpt, pboolean nosdetpt, byte detop)
begin

  getbuf (pexp->q330, addr(pexp->curpt), sizeof(tdetector_operation)) ;
  pexp->pcs->pdetop = extend_link (pexp->pcs->pdetop, pexp->curpt) ;
  pexp->curpt->link = NIL ;
  pexp->curpt->tospt = tosdetpt ;
  pexp->curpt->nospt = nosdetpt ;
  pexp->curpt->op = detop ;
  inc(pexp->tempcnt) ;
  pexp->curpt->temp_num = pexp->tempcnt ;
end

static void token (texpand *pexp)
begin
  pdet_packet pdp ;
  plcq pq ;
  con_common *pcc ;

  if (pexp->pop == NIL)
    then
      pexp->pop = pexp->pcs->token_list ;
    else
      pexp->pop = pexp->pop->link ;
  if (pexp->pop == NIL)
    then
      begin
        pexp->nextoke.t = OP ;
        pexp->nextoke.ret_o = DEO_DONE ;
        return ;
      end
  switch (pexp->pop->tok and DES_OP) begin
    case DES_DET :
      pdp = pexp->pop->point ;
      pexp->nextoke.t = DETP ;
      pcc = pdp->cont ;
      pexp->nextoke.ret_dx = addr(pcc->detection_declared) ;
      break ;
    case DES_CAL :
      pq = pexp->pop->point ;
      pexp->nextoke.t = DETP ;
      pexp->nextoke.ret_dx = addr(pq->cal_on) ;
      break ;
    case DES_COMM :
      pexp->nextoke.t = DETP ;
      pexp->nextoke.ret_dx = addr(pexp->paqs->commevents[pexp->pop->tok and (CE_MAX - 1)].ison) ;
      break ;
    case DES_OP :
      pexp->nextoke.t = OP ;
      pexp->nextoke.ret_o = pexp->pop->tok and not DES_OP ;
      break ;
  end
end

/* Again, due to lack of nesting support, provide forward declarations */
extern void factor (texpand *pexp, ret *fac) ;
extern void express (texpand *pexp, ret *expr) ;
extern void term (texpand *pexp, ret *ter) ;

static void unary (texpand *pexp, ret *uno, boolean forcex, byte *unop)
begin

  memcpy(uno, addr(pexp->nextoke), sizeof(ret)) ;
  if ((uno->t == OP) land (uno->ret_o == DEO_DONE))
    then
      return ;
  if ((pexp->nextoke.t == OP) land (pexp->nextoke.ret_o == DEO_NOT))
    then
      begin
        *unop = pexp->nextoke.ret_o ;
        token (pexp) ;
        if (forcex)
          then
            express (pexp, uno) ;
          else
            factor (pexp, uno) ;
        if ((uno->t != OP) lor (uno->ret_o != DEO_DONE))
          then
            switch (*unop) begin
              case DEO_NOT :
                stackdetop (pexp, uno->ret_dx, NIL, DEO_NOT) ;
                uno->ret_dx = (pboolean)pexp->tempcnt ;
                break ;
            end
      end
  else if (forcex)
    then
      express (pexp, uno) ;
    else
      token (pexp) ;
end

void factor (texpand *pexp, ret *fac)
begin
  byte unop = 0 ;

  memcpy(fac, addr(pexp->nextoke), sizeof(ret)) ;
  if ((fac->t == OP) land (fac->ret_o == DEO_DONE))
    then
      return ;
  if ((pexp->nextoke.t == OP) land (pexp->nextoke.ret_o == DEO_LPAR))
    then
      begin
        token (pexp) ;
        unary (pexp, fac, TRUE, addr(unop)) ;
        if ((fac->t == OP) land (fac->ret_o == DEO_DONE))
          then
            return ;
        if ((pexp->nextoke.t == OP) land (pexp->nextoke.ret_o == DEO_RPAR))
          then
            token (pexp) ;
          else
            begin
              fac->t = OP ;
              fac->ret_o = DEO_DONE ;
            end
      end
  else if (lnot ((pexp->nextoke.t == OP) land (pexp->nextoke.ret_o == DEO_DONE)))
    then
      unary (pexp, fac, FALSE, addr(unop)) ;
end

void term (texpand *pexp, ret *ter)
begin
  byte mulop = 0 ;
  ret nfac ;

  nfac.ret_o = 0 ;
  nfac.ret_dx = NULL;
  factor (pexp, ter) ;
  if ((ter->t == OP) land (ter->ret_o == DEO_DONE))
    then
      return ;
  while ((pexp->nextoke.t == OP) land
         ((pexp->nextoke.ret_o == DEO_AND) lor (pexp->nextoke.ret_o == DEO_EOR)))
    begin
      mulop = pexp->nextoke.ret_o ;
      token (pexp) ;
      factor (pexp, addr(nfac)) ;
      if ((nfac.t != OP) lor (nfac.ret_o != DEO_DONE))
        then
          switch (mulop) begin
            case DEO_AND :
              if ((nfac.t == DETP) land (ter->t == DETP))
                then
                  begin
                    stackdetop (pexp, ter->ret_dx, nfac.ret_dx, DEO_AND) ;
                    ter->ret_dx = (pboolean)pexp->tempcnt ;
                  end
                else
                  begin
                    ter->t = OP ;
                    ter->ret_o = DEO_DONE ;
                  end
              break ;
            case DEO_EOR :
              if ((nfac.t == DETP) land (ter->t == DETP))
                then
                  begin
                    stackdetop (pexp, ter->ret_dx, nfac.ret_dx, DEO_EOR) ;
                    ter->ret_dx = (pboolean)pexp->tempcnt ;
                  end
                else
                  begin
                    ter->t = OP ;
                    ter->ret_o = DEO_DONE ;
                  end
              break ;
          end /* case */
    end /* while */
end

void express (texpand *pexp, ret *expr)
begin
  byte addop = 0 ;
  ret nter ;

  nter.ret_o = 0 ;
  nter.ret_dx = NULL;
  term (pexp, expr) ;
  if (expr->t == DETP)
    then
      while ((pexp->nextoke.t == OP) land (pexp->nextoke.ret_o == DEO_OR))
        begin
          addop = pexp->nextoke.ret_o ;
          token (pexp) ;
          term (pexp, addr(nter)) ;
          if ((addop == DEO_OR) land (expr->t == DETP) land (nter.t == DETP))
            then
              begin
                stackdetop (pexp, expr->ret_dx, nter.ret_dx, DEO_OR) ;
                expr->ret_dx = (pboolean)pexp->tempcnt ;
              end
        end /* while */
end

void expand_control_detectors (paqstruc paqs)
begin
  texpand *pexp ;
  pq330 tempq330 ;

  tempq330 = paqs->owner ;
  getbuf (tempq330, addr(pexp), sizeof(texpand)) ;
  memset(pexp, 0, sizeof(texpand)) ;
  pexp->q330 = tempq330 ;
  pexp->pcs = paqs->ctrlchain ;
  pexp->paqs = paqs ;
  while (pexp->pcs)
    begin
      pexp->pcs->pdetop = NIL ;
      pexp->pop = NIL ; /* haven't started yet */
      pexp->tempcnt = 0 ;
      token (pexp) ;
      express (pexp, addr(pexp->result)) ;
      if (pexp->pcs->pdetop == NIL)
        then
          stackdetop (pexp, pexp->result.ret_dx, NIL, DEO_DONE) ;
      pexp->pcs = pexp->pcs->link ;
    end
end

void evaluate_detector_stack (pq330 q330, plcq q)
begin
#define EVALUATION_STACK_DEPTH 50
  char s[120] ;
  pboolean pdt1, pdt2 ;
  boolean b1, b2, b3 ;
  pdetector_operation pop ;
  boolean temps[EVALUATION_STACK_DEPTH] ;


  if (q->ctrl == NIL)
    then
      begin
        q->gen_on = FALSE ; /* can't be on */
        return ;
      end
  q->ctrl->ison = FALSE ; /* assume off */
  pop = q->ctrl->pdetop ;
  b3 = FALSE ;
  while (pop)
    begin
      pdt1 = pop->tospt ;
      pdt2 = pop->nospt ;
      if ((pntrint)pdt1 < EVALUATION_STACK_DEPTH)
        then
          b1 = temps[(pntrint)pdt1] ;
        else
          b1 = *pdt1 ;
      if ((pntrint)pdt2 < EVALUATION_STACK_DEPTH)
        then
          b2 = temps[(pntrint)pdt2] ;
        else
          b2 = *pdt2 ;
      switch (pop->op) begin
        case DEO_DONE :
          b3 = b1 ;
          break ;
        case DEO_AND :
          b3 = b1 land b2 ;
          break ;
        case DEO_OR :
          b3 = b1 lor b2 ;
          break ;
        case DEO_EOR :
          b3 = (b1 != b2) ;
          break ;
        case DEO_NOT :
          b3 = lnot b1 ;
          break ;
      end
      if (pop->link == NIL) /*last*/
        then
          q->ctrl->ison = b3 ;
        else
          temps[pop->temp_num] = b3 ;
      pop = pop->link ;
    end
  if (q->ctrl->ison != q->ctrl->wason)
    then
      begin
        if (q->ctrl->logmsg)
          then
            begin
              strcpy(s, addr(q->ctrl->cdname)) ;
              if (q->ctrl->ison)
                then
                  strcat(s, " On") ;
                else
                  strcat(s, " Off") ;
              libdatamsg(q330, LIBMSG_CTRLDET, addr(s)) ;
            end
      end
  q->ctrl->wason = q->ctrl->ison ; /* new current state */
  q->gen_on = q->gen_on or q->ctrl->ison ; /* merge into sticky status in lcq */
end

enum tliberr lib_ctrlstat (pq330 q330, tctrlstat *ctrlstat)
begin
  paqstruc paqs ;
  pcontrol_detector pc ;

  paqs = q330->aqstruc ;
  ctrlstat->count = 0 ;
  if (q330->libstate != LIBSTATE_RUN)
    then
      return LIBERR_NOSTAT ;
  pc = paqs->ctrlchain ;
  while ((ctrlstat->count < MAX_CTRLSTAT) land (pc))
    begin
      strcpy(addr(ctrlstat->entries[ctrlstat->count].name), addr(pc->cdname)) ;
      ctrlstat->entries[ctrlstat->count].ison = pc->ison ;
      inc(ctrlstat->count) ;
      pc = pc->link ;
    end
  return LIBERR_NOERR ;
end

#endif
