C***********************************************************************
C*  Fortran parameters used in qlib2.
C*									
C*	Douglas Neuhauser						
C*	Seismographic Station						
C*	University of California, Berkeley				
C*	doug@seismo.berkeley.edu					
C*									
C***********************************************************************

C*
C* Copyright (c) 1996 The Regents of the University of California.
C* All Rights Reserved.
C* 
C* Permission to use, copy, modify, and distribute this software and its
C* documentation for educational, research and non-profit purposes,
C* without fee, and without a written agreement is hereby granted,
C* provided that the above copyright notice, this paragraph and the
C* following three paragraphs appear in all copies.
C* 
C* Permission to incorporate this software into commercial products may
C* be obtained from the Office of Technology Licensing, 2150 Shattuck
C* Avenue, Suite 510, Berkeley, CA  94704.
C* 
C* IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
C* FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
C* INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
C* ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
C* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
C* 
C* THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
C* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
C* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
C* PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
C* CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
C* UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
C*

c $Id: qdefines.inc 2 2005-07-26 19:28:46Z andres $ 

	integer SEC_PER_MINUTE, SEC_PER_HOUR, SEC_PER_DAY
	integer TICKS_PER_SEC, TICKS_PER_MSEC
	integer USECS_PER_SEC, USECS_PER_MSEC, USECS_PER_TICK
	parameter ( SEC_PER_MINUTE = 60 )
	parameter ( SEC_PER_HOUR = 3600 )
	parameter ( SEC_PER_DAY = 86400 )
	parameter ( TICKS_PER_SEC = 10000 )
	parameter ( TICKS_PER_MSEC = (TICKS_PER_SEC/1000) )
	parameter ( USECS_PER_SEC = 1000000 )
	parameter ( USECS_PER_MSEC = (USECS_PER_SEC/1000) )
	parameter ( USECS_PER_TICK = (USECS_PER_SEC/TICKS_PER_SEC) )

	integer IHUGE
	double precision DIHUGE
	parameter ( IHUGE = (65536 * 32767) )
	parameter ( DIHUGE = 140737488355328.D0 )
c
	character DATA_HDR_IND, VOL_HDR_IND
	parameter ( DATA_HDR_IND = 'D' )
	parameter ( VOL_HDR_IND = 'V' )
c
	integer UNK_HDR_TYPE, QDA_HDR_TYPE, SDR_HDR_TYPE
	integer SDR_VOL_HDR_TYPE, DRM_HDR_TYPE
	parameter ( UNK_HDR_TYPE = 0 )
	parameter ( QDA_HDR_TYPE = 1 )
	parameter ( SDR_HDR_TYPE = 2 )
	parameter ( SDR_VOL_HDR_TYPE = 3 )
	parameter ( DRM_HDR_TYPE = 4 )
c
	integer JULIAN_FMT, JULIAN_FMT_1, MONTH_FMT, MONTH_FMT_1
	integer JULIANC_FMT, JULIANC_FMT_1, MONTHS_FMT, MONTHS_FMT_1
	parameter ( JULIAN_FMT = 0 )
	parameter ( JULIAN_FMT_1 = 1 )
	parameter ( MONTH_FMT = 2 )
	parameter ( MONTH_FMT_1 = 3 )
	parameter ( JULIANC_FMT = 4 )
	parameter ( JULIANC_FMT_1 = 5 )
	parameter ( MONTHS_FMT = 6 )
	parameter ( MONTHS_FMT_1 = 7  )
c
	integer TRUE, FALSE
	parameter ( TRUE = 1 )
	parameter ( FALSE = 0 )
c
c Datatype definitions
c

c	Define UNKNOWN datatype.
	integer UNKNOWN_DATATYPE
	parameter ( UNKNOWN_DATATYPE = 0 )

c	General datatype codes.
	integer INT_16, INT_24, INT_32
	integer IEEE_FP_SP, IEEE_FP_DP
	parameter ( INT_16 = 1 )
	parameter ( INT_24 = 2 )
	parameter ( INT_32 = 3 )
	parameter ( IEEE_FP_SP = 4 )
	parameter ( IEEE_FP_DP = 5 )

c	FDSN Network codes.
	integer STEIM1, STEIM2
	parameter ( STEIM1 = 10 )
	parameter ( STEIM2 = 11 )
	integer GEOSCOPE_M24, GEOSCOPE_M16_GR3, GEOSCOPE_M16_GR4
	parameter ( GEOSCOPE_M24 = 12 )
	parameter ( GEOSCOPE_M16_GR3 = 13 )
	parameter ( GEOSCOPE_M16_GR4 = 14 )
	integer USNN, CDSN
	parameter ( USNN = 15 )
	parameter ( CDSN = 16 )
	integer GRAEFENBERG_16, IPG_STRASBOURG_16
	parameter ( GRAEFENBERG_16 = 17 )
	parameter ( IPG_STRASBOURG_16 = 18 )

c	Older Network codes.
	integer SRO, HGLP, DWWSSN_GR, RSTN_16_GR
	parameter ( SRO = 30 )
	parameter ( HGLP = 31 )
	parameter ( DWWSSN_GR = 32 )
	parameter ( RSTN_16_GR = 33 )

c	Definitions for blockette 1000
	integer SEED_LITTLE_ENDIAN, SEED_BIG_ENDIAN
	parameter ( SEED_LITTLE_ENDIAN = 0 )
	parameter ( SEED_BIG_ENDIAN = 1 )

