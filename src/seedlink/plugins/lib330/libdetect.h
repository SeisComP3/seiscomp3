/*   Lib330 Detector Definitions
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
    0 2006-10-11 rdr Created
*/
#ifndef libdetect_h
/* Flag this file as included */
#define libdetect_h
#define VER_LIBDETECT 0

#ifndef OMIT_SEED
/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef libstrucs_h
#include "libstrucs.h"
#endif
#ifndef libsampglob_h
#include "libsampglob.h"
#endif

extern void initialize_detector (pq330 q330, pdet_packet pdp, piirfilter pi) ;
extern boolean Te_detect (tdet_packet *detector) ;
extern boolean E_detect (tdet_packet *detector) ;
extern enum tliberr lib_detstat (pq330 q330, tdetstat *detstat) ;
extern void lib_changeenable (pq330 q330, tdetchange *detchange) ;

#define MAXSAMP 38
#define NULL 0
#define EV_OFF 2
#define NOR_OUT 4
#define CUR_MAX 50
#define E_B 204
#define B_M1 (E_B - 1) /*buffer index for routine Event*/
#define B_M2 (E_B - 2)
#define B_M3 (E_B - 3)
#define B_M4 (E_B - 4) /* "  " " " */
#define DET_SCALE_SHIFT 4 /* shifting by this gives scale factor of 16 for detector */
#define DET_SCALE_FACTOR 16.0 /* 2.0 ** DET_SCALE_SHIFT */
#define MINPOINTS 20 /* minimum number of points to run detection on */
enum slope_type {ST_POS, ST_NEG, ST_NEITHER} ;

/*------------------The continuity structures-------------------------------*/
typedef struct {
  boolean detector_on ; /* if last record was detected on */
  boolean detection_declared ; /* detector_on filtered with first_detection */
  boolean first_detection ; /* if this is first detection after startup */
  boolean detector_enabled ; /* currently enabled */
  boolean new_onset ; /* TRUE if new onset */
  boolean default_enabled ; /* detector enabled by default */
  integer sampcnt ; /* Number of samples in "insamps" so far */
  longint total_detections ; /* used to keep track of detections over time */
  double startt ; /* Starting time of first data point */
  double etime ;
} con_common ;
/*-----------------Variables used in e_detect------------------------------*/
typedef struct {
  boolean detector_on ; /* if last record was detected on */
  boolean detection_declared ; /* detector_on filtered with first_detection */
  boolean first_detection ; /* if this is first detection after startup */
  boolean detector_enabled ; /* currently enabled */
  boolean new_onset ; /* TRUE if new onset */
  boolean default_enabled ; /* detector enabled by default */
  integer sampcnt ; /* Number of samples in "insamps" so far */
  longint total_detections ; /* used to keep track of detections over time */
  double startt ; /* Starting time of first data point */
  double etime ;
  integer cur_rec ;
/*-------------Variables Used in event subroutine-------------------------*/
  longint buf_flg[E_B] ; /* flags: 1 if >= th2; 2 if >= th1 */
  longint buf_sc[E_B] ;  /* summed delta sample counts */
  longint buf_amp[E_B] ; /* P-T amplitudes values */
  longint buf_tim[E_B] ; /* time coordinate of P-T values */
  longint buf_rec[E_B] ;  /*  record number array */
  longint abuf_sc[4] ; /* last 4 delta sample counts */
  longint abuf_amp[4] ;  /* last 4 P-T amplitudes */
  longint abuf_tim[4] ;  /* time cord. of last 4 P-T values */
  longint abuf_rec[4] ; /* companion to buf_rec */
  longint last_amp ;          /* last P-T value       */
  boolean epf ;   /* event possible flag */
  boolean evon ;    /* event detected flag */
  boolean icheck ;    /* flag, ensures period estimate */
  integer fst_flg ;    /* index to first flagged P-T value  */
  integer indx ;   /* saves fst_flg */
  integer lst_flg ;    /* index to last flagged P-T value */
  integer lst_flg2 ;   /* lst_flg corrected for overflow */
  integer lst_pt ;   /* index to last P-T value processed */
  longint lst_pt2 ;    /* lst_pt corrected for overflow */
  integer index2 ;   /* counter,checked for overflow */
  boolean iset ;    /* flag, = 1 when P-T >= th1 */
  integer jj ;   /* index for abrev buffers abuf */
  longint sumdsc ;   /* sum delta sample count */
  longint sumflg ;   /* number of P-T values > th2 */
  longint last_sc ;    /* loaded to abuf_sc[] */
  longint last_tim ;   /* loaded to abuf_tim[] */
  longint last_rec ;    /*  record of last_tim */
  longint th1 ;    /* largest detection threshold */
  longint th2 ;    /* smallest detection threshold */
/*---------------------Variables used in p_one----------------------------*/
  longint last_y ;   /* amplitude coordinate of previous sample */
  longint last_x ;   /* time coordinate of previous sample */
  longint rec_last_x ;  /*  Record of last time coordinate */
  longint sum_s_c ;    /* samples from last P or T to current sample */
  longint s_sum_sc ;   /* samples between last two P-T values */
  longint index ;    /* counter, calls ptwo */
  longint max_y ;    /* amplitude coordinate of P or T value */
  longint tim_of_max ; /* time coordinate of P or T value */
  longint rec_of_max ;  /*  record of tim_of_max */
  enum slope_type prev_slope ;  /* sign of last difference  */
  longint maxamp ;   /* abs max of 20 consec. P-T values < thx */
  longint thx ;    /* upper bound for noise est. */
  longint s_amp ;    /* signed amplitude of P-T value */
/*----------------------Variables used in p_two---------------------------*/
  longint tsstak[16] ; /* contains set of maxamp values */
  longint twosd ;    /* statistical dispersion of P-T values */
  integer kk ;   /* index for tsstak[] */
/*----------------------Variables used in onsetq--------------------------*/
  longint th3 ;    /* threshold for estimating onset */
/*----------------------Variables used in count_dn-------------------------*/
  longint itc ;    /* counter for interval of the event */
  longint nn ;   /* counter for itc */
/*----------------------Variables to set up the detector----------------------*/
  tfloat sample_rate ; /* sample rate in seconds */
  tfloat p_val ;      /* period expressed seconds */
  longint haf_per ;    /* samples per one-half period */
/*------------Miscellaneous Variables that were globals in C version---------*/
  longint th_wt ;  /*weight, = 1 if >= th2, = 2 if >= th1 -- event*/
  tonset_mh *onsetdata ;
  pdet_packet parent ;
} con_sto ;
typedef con_sto *pcon_sto ;

/*-------------------------Te_detect continuity structure---------------*/
typedef struct {
  boolean detector_on ; /* if last record was detected on */
  boolean detection_declared ; /* detector_on filtered with first_detection */
  boolean first_detection ; /* if this is first detection after startup */
  boolean detector_enabled ; /* currently enabled */
  boolean new_onset ; /* TRUE if new onset */
  boolean default_enabled ; /* detector enabled by default */
  integer sampcnt ; /* Number of samples in "insamps" so far */
  longint total_detections ; /* used to keep track of detections over time */
  double startt ; /* Starting time of first data point */
  double etime ;
  longint peakhi ; /*highest value of high limit*/
  longint peaklo ; /*lowest value of low limit*/
  longint waitdly ; /*sample countdown*/
  integer overhi ; /*number of points over the high limit*/
  integer overlo ; /*number of points under the low limit*/
  boolean hevon ; /*high limit event on*/
  boolean levon ; /*low limit event on*/
  tfloat sample_rate ; /* sample rate in seconds */
  tonset_base *onsetdata ;
  pdet_packet parent ;
} threshold_control_struc ;
typedef threshold_control_struc *pthreshold_control_struc ;
#endif

#endif
