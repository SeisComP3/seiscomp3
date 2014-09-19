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
 * Copyright (c) 1996-2003 The Regents of the University of California.
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

/*	$Id: qutils.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__qutils_h
#define	__qutils_h

#include "stdio.h"
#include "sdr.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int my_wordorder;	/* Unknown endian.			*/
extern int hdr_wordorder;	/* desired hdr wordorder. (W)		*/
extern int data_wordorder;	/* desired data wordorder. (W)		*/
extern int default_data_hdr_ind;/* dflt data_hdr_ind (R/W).		*/
extern int qlib2_errno;		/* qlib2 extented error code.		*/
extern int qlib2_op_mode;	/* qlib2 operation mode.		*/

extern void seed_to_comp
   (char	*seed,		/* SEED channel name (input).		*/
    char	**stream,	/* SEED stream name (output).		*/
    char	**component);	/* SEED component name (output).	*/

extern void comp_to_seed 
   (char	*stream,	/* SEED stream name (input).		*/
    char	*component,	/* SEED component name (input).		*/
    char	**seed);	/* SEED channel name (output).		*/

extern int allnull
   (char	*p,		/* ptr to block of memory to check.	*/
    int		n);		/* number of bytes to check.		*/

extern int roundoff
   (double	d);		/* double precision value to round.	*/

extern int xread 
   (int		fd,		/* file descriptor of input file.	*/
    char	*buf,		/* ptr to input buffer.			*/
    int		n);		/* desired number of bytes to read.	*/

extern int xwrite 
   (int		fd,		/* file descriptor of output file.	*/
    char	*buf,		/* ptr to ouput buffer.			*/
    int		n);		/* desired number of bytes to write.	*/

extern void cstr_to_fstr
   (char	*str,		/* input null-terminated string.	*/
    int		flen);		/* output fortran blank-padded string.	*/

extern int date_fmt_num 
   (char	*str);		/* string containing date fmt string.	*/

extern int print_syntax
   (char	*cmd,		/* program name.			*/
    char	*syntax[],	/* syntax array.			*/
    FILE	*fp);		/* FILE ptr for output.			*/

extern char *uppercase
   (char	*string);	/* string to convert to upper case.	*/

extern char *lowercase
   (char	*string);	/* string to convert to lower case.	*/

extern char *charncpy
   (char	*out,		/* ptr to output string.		*/
    char	*in,		/* ptr to input string.			*/
    int		n);		/* number of characters to copy.	*/

extern char *charvncpy
   (char	*out,		/* ptr to output string.		*/
    char	*in,		/* ptr to input string.			*/
    int		n,		/* number of characters to copy.	*/
    int		i);		/* index of SEED var string to copy.	*/

extern char *trim
   (char *str);			/* string to trim trailing blanks.	*/

extern char *tail
   (char	*path);		/* pathname.				*/

extern char *capnstr
   (char	*dst,		/* destination char array to fill.	*/
    char	*src,		/* source string to copy.		*/
    int		n);		/* char length for destination array.	*/

extern char *capnint
   (char	*dst,		/* destination char array to fill.	*/
    int		ival,		/* integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern char *capnlong
   (char	*dst,		/* destination char array to fill.	*/
    long int	ival,		/* long integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern char *capntstr
   (char	*dst,		/* destination char array to fill.	*/
    char	*src,		/* source string to copy.		*/
    int		n);		/* char length for destination array.	*/

extern char *capntint
   (char	*dst,		/* destination char array to fill.	*/
    int		ival,		/* integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern char *capntlong
   (char	*dst,		/* destination char array to fill.	*/
    long int	ival,		/* long integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern int get_my_wordorder(void);	

extern int set_hdr_wordorder
   (int		wordorder);	/* desired wordorder for headers.	*/

extern int set_data_wordorder
   (int		wordorder);	/* desired wordorder for data.		*/

char set_record_type
   (char	record_type);	/* desired default record_type.		*/

int init_qlib2
   (int		mode);		/* desired qlib2 operation mode.	*/

extern int wordorder_from_time
   (unsigned char *p);		/* ptr to fixed data time field.	*/

extern int is_data_hdr_ind 
    (char c);			/* data_hdr_ind char.			*/

extern int is_vol_hdr_ind 
    (char c);			/* vol_hdr_ind char.			*/

extern void swab2
   (short int	*in);		/* ptr to short integer to byteswap.	*/

extern void swab3
   (unsigned char *in);		/* ptr to byte array to byteswap.	*/

extern void swab4
   (int		*in);		/* ptr to integer to byteswap.		*/

void swab8
   (double	*in);		/* ptr to double to byteswap.		*/

void swabt 
   (SDR_TIME	*st);		/* ptr to SDR_TIME to byteswap.		*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to qutils routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
int f_get_my_wordorder_ (void);
#else
int f_get_my_wordorder (void);
#endif

#ifdef	fortran_suffix
int f_set_hdr_wordorder_
#else
int f_set_hdr_wordorder
#endif
   (int		*wordorder);	/* desired wordorder for headers.	*/

#ifdef	fortran_suffix
int f_set_data_wordorder_
#else
int f_set_data_wordorder
#endif
   (int		*wordorder);	/* desired wordorder for data.		*/

#ifdef	fortran_suffix
void f_set_record_type_
#else
void f_set_record_type
#endif
   (char 	*i_record_type,	/* desired default record_type.		*/
    char	*o_record_type,	/* output default record_type.		*/
    int		ilen,		/* (fortran supplied) length of string.	*/
    int		olen);		/* (fortran supplied) length of string.	*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

