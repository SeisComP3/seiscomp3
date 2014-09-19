/************************************************************************/
/*  Utility routines for Quanterra data processing.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2004 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef lint
static char sccsid[] = "$Id: qutils.c 2 2005-07-26 19:28:46Z andres $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#ifdef	SUNOS4
#include <malloc.h>
#endif

#include "qdefines.h"
#include "data_hdr.h"
#include "sdr.h"
#include "qutils.h"

#define	N_NULL_CHECK    32
extern int errno;

/************************************************************************/
/*  External variables to control reading and writing wordorder.	*/
/************************************************************************/
int my_wordorder = -1;			/* Byteorder of my computer.	*/
int hdr_wordorder = -1;			/* desired hdr wordorder (R/W).	*/
int data_wordorder = -1;		/* desired data wordorder (R/W).*/
int default_data_hdr_ind = DATA_HDR_IND_D;  /* dflt data_hdr_ind (R/W).	*/
int qlib2_errno = 0;			/* qlib2 extented error code.	*/
int qlib2_op_mode = 0;			/* op mode for qlib2 (R/W)	*/

/************************************************************************/
/*  SEED channel to station/stream mapping tables.			*/
/*									*/
/*  This information is STATION/TIME SPECIFIC, and should NOT be hard	*/
/*  coded.  It is currenly used only for 2 UCB stations, and is		*/
/*  consistent between those 2 stations.				*/
/************************************************************************/

typedef struct stream_map {
    char    *seed_stream;
    char    *stream;
    char    *component;
} STREAM_MAP;

/************************************************************************/
/*  Table of known and valid streama.					*/
/************************************************************************/

STREAM_MAP known_streams[] = {
/*  Broadband data streams:						*/
/*  VSP - 80 SPS from broadband seismometer.				*/
    "HHZ",  "VSP",  "Z",
    "HHN",  "VSP",  "N",
    "HHE",  "VSP",  "E",
/*  Old (incorrect) names for VSP 80 SPS broadband data.		*/
/*  Used for some station's VSP 100 SPS data.				*/
    "EHZ",  "VSP",  "Z",
    "EHN",  "VSP",  "N",
    "EHE",  "VSP",  "E",
/*  VBB - 20 SPS from broadband seismometer.				*/
    "BHZ",  "VBB",  "Z",
    "BHN",  "VBB",  "N",
    "BHE",  "VBB",  "E",
/*  LP - 1 SPS from broadband seismometer.				*/
    "LHZ",  "LP",   "Z",
    "LHN",  "LP",   "N",
    "LHE",  "LP",   "E",
/*  VLP - 1/10 SPS from broadband seismometer.				*/
    "VHZ",  "VLP",  "Z",
    "VHN",  "VLP",  "N",
    "VHE",  "VLP",  "E",
/*  ULP - 1/100 SPS from broadband seismometer.				*/
    "UHZ",  "ULP",  "Z",
    "UHN",  "ULP",  "N",
    "UHE",  "ULP",  "E",
/*  LG - 80 SPS (low gain) force balance accelerometer (fba) data.	*/
    "HLZ",  "LG",  "Z",
    "HLN",  "LG",  "N",
    "HLE",  "LG",  "E",
/*  Old (incorrect) LG - 80 SPS (low gain) FBA data.			*/
    "ELZ",  "LG",   "Z",
    "ELN",  "LG",   "N",
    "ELE",  "LG",   "E",
/*  BKS VBB channels from ULP instrument.				*/
    "BHA",  "UBB",   "Z",
    "BHB",  "UBB",   "N",
    "BHC",  "UBB",   "E",
/*  Experimental channels.						*/
/*::
    "LXZ",  "LX",   "Z",
    "LXN",  "LX",   "N",
    "LXE",  "LX",   "E",
    NULL,   UNKNOWN_STREAM,  UNKNOWN_COMP,
    UNKNOWN_STREAM,   NULL,   NULL,
::*/
/*  Table terminator.							*/
    NULL,   NULL,   NULL };

/************************************************************************/
/*  seed_to_comp:							*/
/*	Determine "common" stream & component name from SEED channel.	*/
/************************************************************************/
void seed_to_comp
   (char	*seed,		/* SEED channel name (input).		*/
    char	**stream,	/* SEED stream name (output).		*/
    char	**component)	/* SEED component name (output).	*/
{
    STREAM_MAP	*p = known_streams;
    while ( p->seed_stream != NULL && (strcmp(p->seed_stream, seed) != 0) )
	++p;
    *stream = p->stream;
    *component = p->component;
    /*	Create a geoscope channel name for unknown streams. */
    if (*stream==NULL) {
	if ((*stream = (char *)malloc(3)) != NULL) {
	    strncpy (*stream, seed, 2);
	    *((*stream)+2) = '\0';
	}
	if ((*component = (char *)malloc(2)) != NULL) {
	    strncpy (*component, seed+2, 1);
	    *((*component)+1) = '\0';
	}
    }
}

/************************************************************************/
/*  comp_to_seed:							*/
/*	Determine SEED name from "common" stream and component name.	*/
/************************************************************************/
void comp_to_seed 
   (char	*stream,	/* SEED stream name (input).		*/
    char	*component,	/* SEED component name (input).		*/
    char	**seed)		/* SEED channel name (output).		*/
{
    STREAM_MAP	*p = known_streams;
    while ( p->seed_stream != NULL && 
	   ((strcmp(p->stream, stream) != 0) ||
	    (strcmp(p->component, component) != 0)) )
	++p;
    *seed= p->seed_stream;
}

/************************************************************************/
/*  allnull:								*/
/*	Determine whether the specified block of characters is "null".	*/
/*	Due to a bug in some of the Quanterra data packing software, 	*/
/*	extraneous blocks of NULL characters may be present every 16-th	*/
/*	SEED data record.						*/
/*	Changed:	02/24/92 by doug@seismo.berkeley.edu		*/
/*	Experience shows that only the first 32-40 bytes may be null,	*/
/*	and the rest will be garbage.					*/
/*  return:								*/
/*	1 of all nulls, 0 otherwise.					*/
/************************************************************************/
int allnull
   (char	*p,		/* ptr to block of memory to check.	*/
    int		n)		/* number of bytes= to check.		*/
{
    int ncheck = MIN(n,N_NULL_CHECK);
    while (ncheck-- > 0)
	if (*p++) return (0);
    return (1);
}

/************************************************************************/
/*  roundoff:								*/
/*	Round a value to the closest integer.				*/
/*  return:								*/
/*	Closest integer value.						*/
/*	x.5 rounds to sign(x)*(|x|+1)					*/
/************************************************************************/
int roundoff
   (double	d)		/* double precision value to round.	*/
{
    int sign, result;
    double ad;

    sign = (d > 0) ? 1 : -1;
    ad = fabs(d);
    result = sign * (int)(ad+.5);
    return (result);
}

/************************************************************************/
/*  xread:								*/
/*	Read input buffer.  Continue reading until N bytes are read	*/
/*	or until error or EOF reached.					*/
/*  return:								*/
/*	number of bytes read.						*/
/************************************************************************/
int xread 
   (int		fd,		/* file descriptor of input file.	*/
    char	*buf,		/* ptr to input buffer.			*/
    int		n)		/* desired number of bytes to read.	*/
{
    int nr;
    int togo = n;
    while (togo > 0) {
	nr = read(fd,buf+(n-togo),togo);
	if ( nr <= 0) return (n-togo);
	togo -= nr;
    }
    return (n);
}

#define MAX_RETRIES 20
/************************************************************************/
/*  xwrite:								*/
/*	Write output buffer.  Continue writing until all N bytes are	*/
/*	written or until error.						*/
/*  return:								*/
/*	number of bytes written.					*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int xwrite 
   (int		fd,		/* file descriptor of output file.	*/
    char	*buf,		/* ptr to ouput buffer.			*/
    int		n)		/* desired number of bytes to write.	*/
{
    int nw;
    int left = n;
    int retries = 0;
    while (left > 0) {
	if ( (nw = write (fd, buf+(n-left), left)) <= 0 && errno != EINTR) {
	    fprintf (stderr, "Error: writing output, unit= %d errno = %d\n", fd, errno);
	    if (QLIB2_CLASSIC) exit (1);
	    return (left - n);	/* (# of bytes written) * -1 */
	}
	if (nw == -1) {
	    fprintf (stderr, "Interrupted write unit = %d, retry %d.\n", fd, retries);
	    ++retries;
	    if (retries > MAX_RETRIES) {
		fprintf (stderr, "Giving up, unit = %d ...\n", fd);
		return(n-left);
	    }
	    continue;
	}
	left -= nw;
    }
    return (n);
}

/************************************************************************/
/*  cstr_to_fstr:							*/
/*	Convert C null-terminated string to Fortran blank-padded string.*/
/*	Initial string MUST BE dimensioned long enough.			*/
/************************************************************************/
void cstr_to_fstr
   (char	*str,		/* input null-terminated string.	*/
    int		flen)		/* output Fortran blank-padded string.	*/
{
    int i, n;
    n = strlen(str);
    for (i=n; i<flen; i++) str[i] = ' ';
}

/************************************************************************/
/*  date_fmt_num:							*/
/*	Convert a date_fmt string to the corresponding numeric value.	*/
/*  return:								*/  
/*	date format numeric value.					*/
/************************************************************************/
int date_fmt_num 
   (char	*str)		/* string containing date fmt string.	*/
{
    if (str == NULL || strlen(str)==0) return (JULIAN_FMT);

    if (strcasecmp(str,"j")==0) return (JULIAN_FMT);	/* julian	*/
    if (strcasecmp(str,"j1")==0) return (JULIAN_FMT_1);	/* julian1	*/
    if (strcasecmp(str,"m")==0) return (MONTH_FMT);	/* month	*/
    if (strcasecmp(str,"m1")==0) return (MONTH_FMT_1);	/* month1	*/
    if (strcasecmp(str,"jc")==0) return (JULIANC_FMT);	/* julian comma	*/
    if (strcasecmp(str,"jc1")==0) return (JULIANC_FMT_1);/* julian1 comma*/
    if (strcasecmp(str,"ms")==0) return (MONTHS_FMT);	/* month slash	*/
    if (strcasecmp(str,"ms1")==0) return (MONTHS_FMT_1);/* month1 slash	*/

    if (strcasecmp(str,"jt")==0) return (JULIAN_FMT_1);	/* julian	*/
    if (strcasecmp(str,"mt")==0) return (MONTH_FMT_1);	/* month tag	*/

    if (strcasecmp(str,"mc")==0) return (MONTHS_FMT);	/* month comma	*/
    if (strcasecmp(str,"mc1")==0) return (MONTHS_FMT_1);/* month1 comma	*/
    return (0);
}

/************************************************************************/
/*  print_syntax:							*/
/*	Print the syntax description of program.			*/
/************************************************************************/
int print_syntax
   (char	*cmd,		/* program name.			*/
    char	*syntax[],	/* syntax array.			*/
    FILE	*fp)		/* FILE ptr for output.			*/
{
    int i;
    for (i=0; syntax[i] != NULL; i++) {
	fprintf (fp, syntax[i], cmd);
	fprintf (fp, "\n");
    }
    return (0);
}

/************************************************************************/
/* String utils for packing fields into SEED Data Records.		*/
/************************************************************************/

/************************************************************************/
/*  uppercase:								*/
/*	Convert a string to upper case in place.			*/
/*  return:								*/
/*	pointer to string.						*/
/************************************************************************/
char *uppercase
   (char	*string)	/* string to convert to upper case.	*/
{
    char *p = string;
    unsigned char c;
    while (c = *p) *(p++) = islower(c) ? toupper(c) : c;
    return (string);
}

/************************************************************************/
/*  lowercase:								*/
/*	Convert a string to lower case in place.			*/
/*  return:								*/
/*	pointer to string.						*/
/************************************************************************/
char *lowercase
   (char	*string)	/* string to convert to lower case.	*/
{
    char *p = string;
    unsigned char c;
    while (c = *p) *(p++) = isupper(c) ? tolower(c) : c;
    return (string);
}

/************************************************************************/
/*  charncpy:								*/
/*	strncpy through N characters, but ALWAYS add NULL terminator.	*/
/*	Output string is dimensioned one longer than max string length.	*/
/*  return:								*/
/*	pointer to output string.					*/
/************************************************************************/
char *charncpy
   (char	*out,		/* ptr to output string.		*/
    char	*in,		/* ptr to input string.			*/
    int		n)		/* number of characters to copy.	*/
{
    char    *p = out;

    while ( (n-- > 0) && (*p++ = *in++) ) ;
    *p = '\0';
    return (out);
}

/************************************************************************/
/*  charvncpy:								*/
/*	strncpy through N characters, but ALWAYS add NULL terminator.	*/
/*	Output string is dimensioned one longer than max string length.	*/
/*	Copy the i-th SEED variable-length string (terminated by ~).	*/
/*  return:								*/
/*	pointer to output string.					*/
/************************************************************************/
char *charvncpy
   (char	*out,		/* ptr to output string.		*/
    char	*in,		/* ptr to input string.			*/
    int		n,		/* number of characters to copy.	*/
    int		i)		/* index of SEED var string to copy.	*/
{
    char    *p = out;
    while (i > 0) {
	if (*in++ == '~') --i;
    }

    while ( (n-- > 0) && (*in != '~') ) *p++ = *in++;
    *p = '\0';
    return (out);
}

/************************************************************************/
/*  trim:								*/
/*	Trim trailing blanks from a string.  Return pointer to string.	*/
/*  return:								*/
/*	pointer to string.						*/
/************************************************************************/
char *trim
   (char *str)			/* string to trim trailing blanks.	*/
{
	char *p = str + strlen(str);
	while (--p >= str) 
		if (*p == ' ') *p = '\0'; else break;
	return (str);
}

/************************************************************************/
/*  tail:								*/
/*	Return the tail (ie filename) portion of a pathname.		*/
/*  return:								*/
/*	Pointer to the tail of the pathname.				*/
/************************************************************************/
char *tail
   (char	*path)		/* pathname.				*/
{
    char *t;
    return (((t = strrchr(path,'/')) != NULL) ? ++t : path);
}

/************************************************************************/
/*  capnstr:								*/
/*	Copy the contents of a string to a char array, and blank pad	*/
/*	to the specified  output length.  Do not null-terminate the	*/
/*	output char array and do not exceed the specified output length.*/
/*  return:								*/
/*	pointer to the destination char array.				*/
/************************************************************************/
char *capnstr
   (char	*dst,		/* destination char array to fill.	*/
    char	*src,		/* source string to copy.		*/
    int		n)		/* char length for destination array.	*/
{
    char *dp = dst;
    char *sp = src;
    int sl = strlen(src);
    int i;
    for (i=0; i<n; i++) *dp++ = (i<sl) ? *sp++ : ' ';
    return (dst);
}

/************************************************************************/
/*  capnint:								*/
/*	Copy the 0-padded ascii representation of an integer to a	*/
/*	char array, and blank pad the char array to the specified	*/
/*	output length.  Do not null-terminate the output char array	*/
/*	and to not exceed the specified output length.			*/
/*  return:								*/
/*	pointer to the destination char array.				*/
/************************************************************************/
char *capnint
   (char	*dst,		/* destination char array to fill.	*/
    int		ival,		/* integer to be encoded.		*/
    int		n)		/* char length for destination array.	*/
{
    char tmpstr[80];
    char tmpfmt[10];
    char *dp = dst;
    sprintf (tmpfmt, "%%0%dd",n);
    sprintf (tmpstr, tmpfmt, ival);
    strncpy (dst, tmpstr, n);
    return (dst);
}

/************************************************************************/
/*  capnlong:								*/
/*	Copy the 0-padded ascii representation of a long integer to a	*/
/*	char array, and blank pad the char array to the specified	*/
/*	output length.  Do not null-terminate the output char array	*/
/*	and to not exceed the specified output length.			*/
/*  return:								*/
/*	pointer to the destination char array.				*/
/************************************************************************/
char *capnlong
   (char	*dst,		/* destination char array to fill.	*/
    long int	ival,		/* long integer to be encoded.		*/
    int		n)		/* char length for destination array.	*/
{
    char tmpstr[80];
    char tmpfmt[10];
    char *dp = dst;
    sprintf (tmpfmt, "%%0%dld",n);
    sprintf (tmpstr, tmpfmt, ival);
    strncpy (dst, tmpstr, n);
    return (dst);
}

/************************************************************************/
/*  Tab terminated versions of the same above routines.			*/
/*  Space for tab is not include in the specified length.		*/
/************************************************************************/
char *capntstr
   (char	*dst,		/* destination char array to fill.	*/
    char	*src,		/* source string to copy.		*/
    int		n)		/* char length for destination array.	*/
{
    capnstr (dst, src, n);
    *(dst+n) = '\t';
    return (dst);
}

char *capntint
   (char	*dst,		/* destination char array to fill.	*/
    int		ival,		/* integer to be encoded.		*/
    int		n)		/* char length for destination array.	*/
{
    capnint (dst, ival, n);
    *(dst+n) = '\t';
    return (dst);
}

char *capntlong
   (char	*dst,		/* destination char array to fill.	*/
    long int	ival,		/* long integer to be encoded.		*/
    int		n)		/* char length for destination array.	*/
{
    capnlong (dst, ival, n);
    *(dst+n) = '\t';
    return (dst);
}

/************************************************************************/
/*  init_qlib2								*/
/*	Set the operation mode of qlib2.				*/
/*	0 = classic, 1 = enhanced (no exit).				*/
/*  Return:								*/
/*	QLIB2 operation mode on success.				*/
/*	negative QLIB2 error code on failure.				*/
/************************************************************************/
int init_qlib2
   (int		mode)		/* desired qlib2 operation mode.	*/
{
    int status;
    /* Check new value for validity. */
    if (mode == 0 || mode == 1) qlib2_op_mode = mode;
    /* Initialize leapsecond table, so that we can return any error.	*/
    if ((status = init_leap_second_table()) < 0) return (status);
    return (qlib2_op_mode);
}

/************************************************************************/
/*  get_my_wordorder:							*/
/*	Determine which endian (byte order) this machine is.		*/
/*	Set default hdr and data wordorder if not previously set.	*/
/*  Return:								*/
/*	machine wordorder on success.					*/
/*	MS_ERROR on error.						*/
/************************************************************************/
int get_my_wordorder()
{
    char *force_qlib2_version = qlib2_version;
    int ival = 0x01234567;		/* hex 01234567			*/
    unsigned char *pc;
    pc = (unsigned char *)&ival;
    my_wordorder = 
	(*pc == 0x01) ? SEED_BIG_ENDIAN :
	(*pc == 0x67) ? SEED_LITTLE_ENDIAN : MS_ERROR;
    if (my_wordorder < 0) {
	fprintf (stderr, "Error: Unable to determine computer wordorder.\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit (1);
	return (MS_ERROR);
    }
    if (hdr_wordorder < 0) set_hdr_wordorder (my_wordorder);
    if (data_wordorder < 0) set_data_wordorder (my_wordorder);
    return (my_wordorder);
}

/************************************************************************/
/*  set_hdr_wordorder:							*/
/*	Set the endian order for reading and writing MiniSEED headers.	*/
/************************************************************************/
int set_hdr_wordorder
   (int		wordorder)	/* desired wordorder for headers.	*/
{
    hdr_wordorder = wordorder;
    return (hdr_wordorder);
}

/************************************************************************/
/*  set_data_wordorder:							*/
/*	Set the endian order for reading and writing MiniSEED data.	*/
/************************************************************************/
int set_data_wordorder
   (int		wordorder)	/* desired wordorder for data.		*/
{
    data_wordorder = wordorder;
    return (data_wordorder);
}

/************************************************************************/
/*  set_record_type:							*/
/*	Set the default record_type.					*/
/*	Return new default record_type.					*/
/************************************************************************/
char set_record_type
   (char	record_type)	    /* desired default record_type.	*/
{
    /* Check new value for validity. */
    if (is_data_hdr_ind (record_type)) default_data_hdr_ind = record_type;
    return (default_data_hdr_ind);
}

/************************************************************************/
/*  wordorder_from_time:						*/
/*	Determine wordorder from the fixed data header date_time field.	*/
/*  return:								*/
/*	wordorder of the record on success.				*/
/*	MS_ERROR on error.						*/
/************************************************************************/
int wordorder_from_time
   (unsigned char *p)		/* ptr to fixed data time field.	*/
{
    int wordorder;
    unsigned char *cyear = p;
    /* This check ONLY works for dates in the range [1800, ..., 2054].	*/
    if (my_wordorder < 0) get_my_wordorder();
    if      ((cyear[0] == 0x07 && cyear[1] >= 0x08) ||
	     (cyear[0] == 0x08 && cyear[1] <  0x07)) wordorder = SEED_BIG_ENDIAN;
    else if ((cyear[1] == 0x07 && cyear[0] >= 0x08) ||
	     (cyear[1] == 0x08 && cyear[0] <  0x07)) wordorder = SEED_LITTLE_ENDIAN;
    else {
	fprintf (stderr, "Error: Unable to determine wordorder from time\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	return (MS_ERROR);
    }
    return (wordorder);
}

/************************************************************************/
/*  is_data_hdr_ind:							*/
/*	Determine if arg is one of the possible data header identifiers.*/
/*	Return 1 if true, 0 if false.					*/
/************************************************************************/
int is_data_hdr_ind 
    (char c)			/* data_hdr_ind char.			*/
{
    return (c == DATA_HDR_IND_D || c == DATA_HDR_IND_R || c == DATA_HDR_IND_Q);
}

/************************************************************************/
/*  is_vol_hdr_ind:							*/
/*	Determine if arg is one of the possible volume identifiers.	*/
/*	Return 1 if true, 0 if false.					*/
/************************************************************************/
int is_vol_hdr_ind 
    (char c)			/* vol_hdr_ind char.			*/
{
    return (c == VOL_HDR_IND);
}

/************************************************************************/
/*  Byteswapping routines.						*/
/************************************************************************/
void swab2
   (short int	*in)		/* ptr to short integer to byteswap.	*/
{
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+1);    
    *(p+1) = tmp;
}

void swab3
   (unsigned char *in)		/* ptr to byte array to byteswap.	*/
{
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+2);    
    *(p+2) = tmp;
}

void swab4
   (int		*in)		/* ptr to integer to byteswap.		*/
{
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+3);    
    *(p+3) = tmp;
    tmp = *(p+1);
    *(p+1) = *(p+2);
    *(p+2) = tmp;
}

void swab8
   (double	*in)		/* ptr to double to byteswap.		*/
{
    unsigned char *p = (unsigned char *)in;
    unsigned char tmp;
    tmp = *p;
    *p = *(p+7);    
    *(p+7) = tmp;
    tmp = *(p+1);
    *(p+1) = *(p+6);
    *(p+6) = tmp;
    tmp = *(p+2);
    *(p+2) = *(p+5);
    *(p+5) = tmp;
    tmp = *(p+3);
    *(p+3) = *(p+4);
    *(p+4) = tmp;
}

void swabt 
   (SDR_TIME	*st)		/* ptr to SDR_TIME to byteswap.		*/
{
    swab2 ((short int *)&st->year);
    swab2 ((short int *)&st->day);
    swab2 ((short int *)&st->ticks);
}

void swabf
   (float	*f)		/* ptr to float to byteswap.		*/
{
    swab4 ((int *)f);
}

/************************************************************************/
/* Fortran interludes to qutils routines.				*/
/************************************************************************/

/*  Determine which endian (byte order) this machine is.		*/
#ifdef	fortran_suffix
int f_get_my_wordorder_ (void)
#else
int f_get_my_wordorder (void)
#endif
{
    return (get_my_wordorder());
}

/*  Set the endian order for reading and writing MiniSEED headers.	*/
#ifdef	fortran_suffix
int f_set_hdr_wordorder_
#else
int f_set_hdr_wordorder
#endif
   (int		*wordorder)	/* desired wordorder for headers.	*/
{
    return (set_hdr_wordorder(*wordorder));
}

/*  Set the endian order for reading and writing MiniSEED data.		*/
#ifdef	fortran_suffix
int f_set_data_wordorder_
#else
int f_set_data_wordorder
#endif
   (int		*wordorder)	/* desired wordorder for data.		*/
{
    return (set_data_wordorder(*wordorder));
}

/*  Set the default record type.					*/
#ifdef	fortran_suffix
void f_set_record_type_
#else
void f_set_record_type
#endif
   (char 	*i_record_type,	/* desired default record_type.		*/
    char	*o_record_type,	/* output default record_type.		*/
    int		ilen,		/* (fortran supplied) length of string.	*/
    int		olen)		/* (fortran supplied) length of string.	*/
{
    int i;
    char dhi = i_record_type[0];
    dhi = (set_data_wordorder(dhi));
    o_record_type[0] = dhi;
    for (i=1; i<olen; i++) {
	o_record_type[i] = ' ';
    }
}
