/*===========================================================================*/
/* SEED reader     |               sac.h                   |     header file */
/*===========================================================================*/
/*
	Name:		sac.h
	Purpose:	structure for header of a SAC (Seismic Analysis Code) data file
	Usage:		#include "sac.h"
	Input:		not applicable
	Output:		not applicable
	Warnings:	not applicable
	Errors:		not applicable
	Called by:	output_data
	Calls to:	none
	Algorithm:	not applicable
	Notes:		Key to comment flags describing each field:
				Column 1:
					R = required by SAC
					  = (blank) optional
				Column 2:
					A = settable from a priori knowledge
					D = available in data
					F = available in or derivable from SEED fixed data header
					T = available in SEED header tables
					  = (blank) not directly available from SEED data, header
					    tables, or elsewhere
	Problems:	none known
	References:	O'Neill, D. (1987).  IRIS Interim Data Distribution Format
		(SAC ASCII), Version 1.0 (12 November 1987).  Incorporated
		Research Institutions for Seismology, 1616 North Fort Myer
		Drive, Suite 1440, Arlington, Virginia 22209.  11 pp.
		Tull, J. (1987).  SAC User's Manual, Version 10.2, October 7,
			1987.  Lawrence Livermore National Laboratory, L-205,
			Livermore, California 94550.  ??? pp.
	Language:	C, hopefully ANSI standard
	Author:	Dennis O'Neill
	Revisions:07/15/88  Dennis O'Neill  Initial preliminary release 0.9
		11/21/88  Dennis O'Neill  Production release 1.0
		09/19/89  Dennis O'Neill  corrected length of char strings
*/

typedef int32_t word;

struct sac
{
	float	delta;			/* RF time increment, sec    */
	float	depmin;			/*    minimum amplitude      */
	float	depmax;			/*    maximum amplitude      */
	float	scale;			/*    amplitude scale factor */
	float	odelta;			/*    observed time inc      */
	float	b;				/* RD initial value, ampl.   */
	float	e;				/* RD final value, amplitude */
	float	o;				/*    event start, sec > 0   */
	float	a;				/*    1st arrival time       */
	float	internal1;		/*    internal use           */
	float	t0;				/*    user-defined time pick */
	float	t1;				/*    user-defined time pick */
	float	t2;				/*    user-defined time pick */
	float	t3;				/*    user-defined time pick */
	float	t4;				/*    user-defined time pick */
	float	t5;				/*    user-defined time pick */
	float	t6;				/*    user-defined time pick */
	float	t7;				/*    user-defined time pick */
	float	t8;				/*    user-defined time pick */
	float	t9;				/*    user-defined time pick */
	float	f;				/*    event end, sec > 0     */
	float	resp0;			/*    instrument respnse parm*/
	float	resp1;			/*    instrument respnse parm*/
	float	resp2;			/*    instrument respnse parm*/
	float	resp3;			/*    instrument respnse parm*/
	float	resp4;			/*    instrument respnse parm*/
	float	resp5;			/*    instrument respnse parm*/
	float	resp6;			/*    instrument respnse parm*/
	float	resp7;			/*    instrument respnse parm*/
	float	resp8;			/*    instrument respnse parm*/
	float	resp9;			/*    instrument respnse parm*/
	float	stla;			/*  T station latititude     */
	float	stlo;			/*  T station worditude      */
	float	stel;			/*  T station elevation, m   */
	float	stdp;			/*  T station depth, m       */
	float	evla;			/*    event latitude         */
	float	evlo;			/*    event worditude        */
	float	evel;			/*    event elevation        */
	float	evdp;			/*    event depth            */
	float	mag;			/*    event magnitude	     */
	float	user0;			/*    available to user      */
	float	user1;			/*    available to user      */
	float	user2;			/*    available to user      */
	float	user3;			/*    available to user      */
	float	user4;			/*    available to user      */
	float	user5;			/*    available to user      */
	float	user6;			/*    available to user      */
	float	user7;			/*    available to user      */
	float	user8;			/*    available to user      */
	float	user9;			/*    available to user      */
	float	dist;			/*    stn-event distance, km */
	float	az;				/*    event-stn azimuth      */
	float	baz;			/*    stn-event azimuth      */
	float	gcarc;			/*    stn-event dist, degrees*/
	float	internal2;		/*    internal use           */
	float	internal3;		/*    internal use           */
	float	depmen;			/*    mean value, amplitude  */
	float	cmpaz;			/*  T component azimuth     */
	float	cmpinc;			/*  T component inclination */
	float	unused2;		/*    reserved for future use*/
	float	unused3;		/*    reserved for future use*/
	float	unused4;		/*    reserved for future use*/
	float	unused5;		/*    reserved for future use*/
	float	unused6;		/*    reserved for future use*/
	float	unused7;		/*    reserved for future use*/
	float	unused8;		/*    reserved for future use*/
	float	unused9;		/*    reserved for future use*/
	float	unused10;		/*    reserved for future use*/
	float	unused11;		/*    reserved for future use*/
	float	unused12;		/*    reserved for future use*/
	word	nzyear;			/*  F zero time of file, yr  */
	word	nzjday;			/*  F zero time of file, day */
	word	nzhour;			/*  F zero time of file, hr  */
	word	nzmin;			/*  F zero time of file, min */
	word	nzsec;			/*  F zero time of file, sec */
	word	nzmsec;			/*  F zero time of file, msec*/
	word	internal4;		/*    internal use           */
	word	internal5;		/*    internal use           */
	word	internal6;		/*    internal use           */
	word	npts;			/* RF number of samples      */
	word	internal7;		/*    internal use           */
	word	internal8;		/*    internal use           */
	word	unused13;		/*    reserved for future use*/
	word	unused14;		/*    reserved for future use*/
	word	unused15;		/*    reserved for future use*/
	word	iftype;			/* RA type of file          */
	word	idep;			/*    type of amplitude      */
	word	iztype;			/*    zero time equivalence  */
	word	unused16;		/*    reserved for future use*/
	word	iinst;			/*    recording instrument   */
	word	istreg;			/*    stn geographic region  */
	word	ievreg;			/*    event geographic region*/
	word	ievtyp;			/*    event type             */
	word	iqual;			/*    quality of data        */
	word	isynth;			/*    synthetic data flag    */
	word	unused17;		/*    reserved for future use*/
	word	unused18;		/*    reserved for future use*/
	word	unused19;		/*    reserved for future use*/
	word	unused20;		/*    reserved for future use*/
	word	unused21;		/*    reserved for future use*/
	word	unused22;		/*    reserved for future use*/
	word	unused23;		/*    reserved for future use*/
	word	unused24;		/*    reserved for future use*/
	word	unused25;		/*    reserved for future use*/
	word	unused26;		/*    reserved for future use*/
	word	leven;			/* RA data-evenly-spaced flag*/
	word	lpspol;			/*    station polarity flag  */
	word	lovrok;			/*    overwrite permission   */
	word	lcalda;			/*    calc distance, azimuth */
	word	unused27;		/*    reserved for future use*/
	char	kstnm[8];		/*  F station name           */
	char	kevnm[16];		/*    event name             */
	char	khole[8];		/*    man-made event name    */
	char	ko[8];			/*    event origin time id   */
	char	ka[8];			/*    1st arrival time ident */
	char	kt0[8];			/*    time pick 0 ident      */
	char	kt1[8];			/*    time pick 1 ident      */
	char	kt2[8];			/*    time pick 2 ident      */
	char	kt3[8];			/*    time pick 3 ident      */
	char	kt4[8];			/*    time pick 4 ident      */
	char	kt5[8];			/*    time pick 5 ident      */
	char	kt6[8];			/*    time pick 6 ident      */
	char	kt7[8];			/*    time pick 7 ident      */
	char	kt8[8];			/*    time pick 8 ident      */
	char	kt9[8];			/*    time pick 9 ident      */
	char	kf[8];			/*    end of event ident     */
	char	kuser0[8];		/*    available to user      */
	char	kuser1[8];		/*    available to user      */
	char	kuser2[8];		/*    available to user      */
	char	kcmpnm[8];		/*  F component name         */
	char	knetwk[8];		/*    network name           */
	char	kdatrd[8];		/*    date data read         */
	char	kinst[8];		/*    instrument name        */
};


/* definitions of constants for SAC enumerated data values */
/* undocumented == couldn't find a definition for it (denio, 07/15/88) */
#define SAC_IREAL   0								/* undocumented              */
#define SAC_ITIME   1								/* file: time series data    */
#define SAC_IRLIM   2								/* file: real&imag spectrum  */
#define SAC_IAMPH   3								/* file: ampl&phas spectrum  */
#define SAC_IXY     4								/* file: gen'l x vs y data   */
#define SAC_IUNKN   5								/* x data: unknown type      */
													/* zero time: unknown        */
													/* event type: unknown       */
#define SAC_IDISP   6								/* x data: displacement (nm) */
#define SAC_IVEL    7								/* x data: velocity (nm/sec) */
#define SAC_IACC    8								/* x data: accel (cm/sec/sec)*/
#define SAC_IB      9								/* zero time: start of file  */
#define SAC_IDAY   10								/* zero time: 0000 of GMT day*/
#define SAC_IO     11								/* zero time: event origin   */
#define SAC_IA     12								/* zero time: 1st arrival    */
#define SAC_IT0    13								/* zero time: user timepick 0*/
#define SAC_IT1    14								/* zero time: user timepick 1*/
#define SAC_IT2    15								/* zero time: user timepick 2*/
#define SAC_IT3    16								/* zero time: user timepick 3*/
#define SAC_IT4    17								/* zero time: user timepick 4*/
#define SAC_IT5    18								/* zero time: user timepick 5*/
#define SAC_IT6    19								/* zero time: user timepick 6*/
#define SAC_IT7    20								/* zero time: user timepick 7*/
#define SAC_IT8    21								/* zero time: user timepick 8*/
#define SAC_IT9    22								/* zero time: user timepick 9*/
#define SAC_IRADNV 23								/* undocumented              */
#define SAC_ITANNV 24								/* undocumented              */
#define SAC_IRADEV 25								/* undocumented              */
#define SAC_ITANEV 26								/* undocumented              */
#define SAC_INORTH 27								/* undocumented              */
#define SAC_IEAST  28								/* undocumented              */
#define SAC_IHORZA 29								/* undocumented              */
#define SAC_IDOWN  30								/* undocumented              */
#define SAC_IUP    31								/* undocumented              */
#define SAC_ILLLBB 32								/* undocumented              */
#define SAC_IWWSN1 33								/* undocumented              */
#define SAC_IWWSN2 34								/* undocumented              */
#define SAC_IHGLP  35								/* undocumented              */
#define SAC_ISRO   36								/* undocumented              */
#define SAC_INUCL  37								/* event type: nuclear shot  */
#define SAC_IPREN  38								/* event type: nuke pre-shot */
#define SAC_IPOSTN 39								/* event type: nuke post-shot*/
#define SAC_IQUAKE 40								/* event type: earthquake    */
#define SAC_IPREQ  41								/* event type: foreshock     */
#define SAC_IPOSTQ 42								/* event type: aftershock    */
#define SAC_ICHEM  43								/* event type: chemical expl */
#define SAC_IOTHER 44								/* event type: other source  */
												/* data quality: other problm*/
#define SAC_IGOOD  45								/* data quality: good        */
#define SAC_IGLCH  46								/* data quality: has glitches*/
#define SAC_IDROP  47								/* data quality: has dropouts*/
#define SAC_ILOWSN 48								/* data quality: low s/n     */
#define SAC_IRLDTA 49								/* data is real data         */
#define SAC_IVOLTS 50								/* file: velocity (volts)    */
#define SAC_IXYZ   51								/* file: gen'l xyz data      */
#define SAC_INIV52 52								/* undocumented              */
#define SAC_INIV53 53								/* undocumented              */
#define SAC_INIV54 54								/* undocumented              */
#define SAC_INIV55 55								/* undocumented              */
#define SAC_INIV56 56								/* undocumented              */
#define SAC_INIV57 57								/* undocumented              */
#define SAC_INIV58 58								/* undocumented              */
#define SAC_INIV59 59								/* undocumented              */
#define SAC_INIV60 60								/* undocumented              */

/* Format strings for writing headers for SAC ASCII files */
/* for floats */
#define FCS "%15.7g%15.7g%15.7g%15.7g%15.7g\n"
/* for integers */
#define ICS "%10d%10d%10d%10d%10d\n"
/* for strings */
#define CCS1 "%-8.8s%-8.8s%-8.8s\n"
/* for strings */
#define CCS2 "%-8.8s%-16.16s\n"
