/*   Lib330 Sliding Window headers
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
    0 2006-09-29 rdr Created
*/
#ifndef libslider_h
/* Flag this file as included */
#define libslider_h
#define VER_LIBSLIDER 8

#ifndef libstrucs_h
#include "libstrucs.h"
#endif

  /* Data Channel High Order Bits and Masks */
#define READ_PREV_STREAM 0 /* Decimated */
#define MESSAGE_STREAM 1 /* Message LCQ */
#define TIMING_STREAM 2 /* Timing LCQ */
#define CFG_STREAM 3 /* Configuration LCQ */
#define DCM_ST 0xE0 /* Status mask */
#define DCM 0xF8 /* Mask for all others */
#define DC_ST38 0x00 /* Status - 3 x 8 bit parameters */
#define DC_ST816 0x20 /* Status - 8 and 16 */
#define DC_ST32 0x40 /* Status - 8, 16, and 32 */
#define DC_ST232 0x60 /* Status - 8, 16, and 2 X 32 */
#define DC_MN38 0x80 /* Main data sample - 3 x 8 */
#define DC_MN816 0x88 /* Main - 8 and 16 */
#define DC_MN32 0x90 /* Main - 8, 16, and 32 */
#define DC_MN232 0x98 /* Main - 8, 16, and 2 X 32 */
#define DC_AG38 0xA0 /* Analog sample - 3 x 8 */
#define DC_AG816 0xA8 /* Analog sample - 8 and 16 */
#define DC_AG32 0xB0 /* Analog sample - 8, 16, and 32 */
#define DC_AG232 0xB8 /* Analog sample - 8, 16, and 2 x 32 */
#define DC_CNP38 0xC0 /* CNP - 3 x 8 */
#define DC_CNP816 0xC8 /* CNP - 8 and 16 */
#define DC_CNP316 0xD0 /* CNP - 3 x 16 */
#define DC_CNP232 0xD8 /* CNP - 8, 16, and 2 x 32 */
#define DC_D32 0xE0 /* Digitizer - 8, 16, and 32 */
#define DC_COMP 0xE8 /* Digitizer - compression map and multiple samples */
#define DC_MULT 0xF0 /* Digitizer - continuation of above */
#define DC_SPEC 0xF8 /* Special purpose packets */
#define DC_DPSTAT 0x7F /* DP Statistics */
  /* DC_MULT flags */
#define DMLS 0x8000 /* Last segment */
#define DMSZ 0x3FF /* blockette size mask */
  /* Reporting Frequencies */
#define FR_1 1 /* 1hz */
#define FR_P1 255 /* 0.1Hz */
#define FR_P01 254 /* 0.01Hz */
  /* Data Port Records */
#define DT_DATA 0 /* Data Record */
#define DT_FILL 6 /* Fill Record */
#define DT_DACK 0xA /* Data Acknowledge */
#define DT_OPEN 0xB /* Open firewall and tell DP where to send data */
  /* GPS Status values */
#define GPS_OFFLOCK 0 /* Powered off due to GPS lock */
#define GPS_OFFPLL 1 /* Powered off due to PLL lock */
#define GPS_OFFMAX 2 /* Powered off due to maximum time */
#define GPS_OFFCMD 3 /* Powered off due to command */
#define GPS_COLD 4 /* Coldstart, see parameter 2 for reason */
#define GPS_ONAUTO 5 /* Powered on automatically */
#define GPS_ONCMD 6 /* Powered on by command */
  /* Group Status Bits */
#define GRP_ENON 1 /* Calibration enable on */
#define GRP_SGON 2 /* Calibration signal on */
  /* Status Reporting */
#define SRINT_M 3 /* Mask */
#define SRINT_OFF 0 /* do not report */
#define SRINT_1 1 /* Report at 1Hz */
#define SR_1S 0 /* Shift count for status 1 */
#define SR_2S 2 /* Shift count for status 2 */
  /* Charger Status */
#define CHRG_NOT 0 /* Not Charging */
#define CHRG_BULK 1 /* Bulk */
#define CHRG_ABS 2 /* Absorption */
  /* Status Messages */
#define ST38_GPSPH 0 /* GPS Status Change */
#define ST38_CNPERR 1 /* CNP Error */
#define ST38_SLVERR 2 /* Slave Processor Error */
#define ST38_DIGPH 3 /* Digitizer Phase Change */
#define ST38_BACKUP 4 /* Backup configuration */
#define ST38_WINSTAT 5 /* Recording Window Status */
#define ST38_LEAP 6 /* Leap Second detected */
#define ST816_PSPH 0 /* Power Supply Phase Change */
#define ST816_AFAULT 1 /* Analog Fault */
#define ST816_CALERR 2 /* Calibration Error map */
/*  ST816_TOKEN = 1 ; */ /* Tokens have changed */
#define ST32_PLLDRIFT 0 /* PLL Drift over last 10 minutes */
#define ST32_DRIFT 1 /* Drift out of range */
  /* Special Purpose Blockettes */
#define SP_CALSTART 0 /* calibration start */
#define SP_CALABORT 1 /* calibration abort */
#define SP_CNP 2 /* CNP Block data */
#define SP_CFGBLK 3 /* Configuration Blockette */
  /* Digitizer Phase Change Constants, first parameter */
#define DPC_STARTUP 0
#define DPC_WAIT 1
#define DPC_INT 2
#define DPC_EXT 3
  /* Digitizer Phase Change COnstants, second parameter */
#define DPR_NORM 0
#define DPR_DRIFT 1
#define DPR_GOTCLK 2
#define DPR_CMD 3
  /* GPS Coldstart Reasons, second parameter */
#define COLD_CMD 0 /* Commanded */
#define COLD_TO 1 /* Tracking Timeout */
#define COLD_INTPH 2 /* Phasing between GPS and internal clock */
#define COLD_JUMP 3 /* Too large a jump while running */
  /* Recording Window status, first parameter */
#define RWR_START 0
#define RWR_STOP 1
  /* Configuration Indexes */
#define CFG_GLOBAL 1 /* Global programming */
#define CFG_FIXED 2  /* Fixed */
#define CFG_AUXAD 3  /* AuxAD configuration */
#define CFG_SS1 4 /* Serial Sensor 1 configuration */
#define CFG_SS2 5 /* Serial Sensor 2 configuration */
#define CFGSZ_AUXAD 20 /* Size in bytes of AUXAD configuration */
/* Data record headers */
typedef struct {
  byte chan ;
  byte val8a ;
  byte val8b ;
  byte val8c ;
} tdp_38 ;
typedef struct {
  byte chan ;
  byte val8 ;
  word val16 ;
} tdp_816 ;
typedef struct {
  byte chan ;
  byte val8 ;
  word val16a ;
  word val16b ;
  word val16c ;
} tdp_316 ;
typedef struct {
  byte chan ;
  byte val8 ;
  word val16 ;
  longint val32 ;
} tdp_32 ;
typedef struct {
  byte chan ;
  byte val8 ;
  word val16 ;
  longint val32a ;
  longint val32b ;
} tdp_232 ;
typedef struct {
  byte chan ;
  byte freq ;
  word size ;
  longint previous ;
  word doff ;
  word map ; /* at least this long */
} tdp_comp ;
typedef tdp_comp *pdp_comp ;
typedef struct {
  byte chan ;
  byte seg_freq ;
  word size ;
} tdp_mult ;
typedef tdp_mult *pdp_mult ;
typedef struct {
  byte chan ;
  byte not_used ;
  word waveform ; /* type of waveform and flags */
  int16 amplitude ; /* in shifts */
  word duration ; /* duration in seconds */
  word calbit_map ; /* which channels are being calibrated */
  word monitor_map ; /* which channels are monitoring raw signal */
  word freqdiv ; /* frequency divider */
  word spare ;
  tseeda12 coupling ; /* coupling description */
} tdp_cals ;
typedef struct {
  byte chan ;
  byte not_used ;
  word calbit_map ; /* which channels are being calibrated */
  word monitor_map ; /* which channels are monitoring raw signal */
  word spare ;
} tdp_cabt ;
typedef struct {
  byte chan ;
  byte flags ;
  word size ;
  byte portnum ;
  byte device_id ;
  byte status ;
  byte msg_type ;
} tdp_cnp ;
typedef tdp_cnp *pdp_cnp ;
typedef struct {
  byte chan ;
  byte cfg ; /* CFG_xxx */
  word size ;
} tdp_cfg ;
  /* End of data record headers */

extern void allocate_packetbuffers (pq330 q330) ;
extern void process_data (pq330 q330) ;
extern void reset_link (pq330 q330) ;
extern void send_dopen (pq330 q330) ;
extern void dack_out (pq330 q330) ;

#endif
