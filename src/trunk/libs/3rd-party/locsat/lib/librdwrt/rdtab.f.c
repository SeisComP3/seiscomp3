/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsrdtab_1_ {
    char sccsid[80];
};

#define sccsrdtab_1 (*(struct sccsrdtab_1_ *) &sccsrdtab_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsrdtab_ = { {'@', '(', '#', ')', 'r', 'd', 't', 'a', 'b', '.', 'f', 
	    '\t', '3', '6', '.', '2', '\t', '3', '/', '1', '7', '/', '8', '9',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static integer c__1 = 1;

/* NAME */
/* 	rdtab -- Establish filename structures for reading tables */
/* FILE */
/* 	rdtab.f */
/* SYNOPSIS */
/* 	Create a file structures for reading the travel-time and amplitude */
/* 	tables. */
/* DESCRIPTION */
/*       Subroutine.  Read travel-time and amplitude tables applicable for */
/*       the given input phase-types for a given input file.  Read */
/* 	travel-time tables from files into memory.  The filenames to be */
/* 	read have the form root//'.'//wavid(k), where root is the non-blank */
/* 	part of froot, and where wavid(k) is a wave identifier for k = 1, */
/* 	nwav; e.g., 'Pg', 'Pn', 'Sn', 'Lg'.  The data read is put into the */
/* 	appropriate arrays as indicated by the filename suffix. */
/* 	Example: If froot = 'tab' and wavid = 'Pg', 'Lg', then this */
/* 		 routine reads files 'tab.Pg' and 'tab.Lg'. */
/*       ---- On entry ---- */
/* 	froot : Root-name of files to be read */
/* 	nwav  : Number of wave types to be used */
/* 	maxtbd: i'th dimension of tbd(), tbtt() arrays */
/* 	maxtbz: j'th dimension of tbz(), tbtt() arrays */
/* 	wavid(k), k = 1, nwav: Character identifier of k'th wave */
/*       ---- On entry ---- */
/* 	ntbd(k),     k = 1, nwav: Number of distance samples in tables */
/* 	ntbz(k),     k = 1, nwav: Number of depth samples in tables */
/* 	tbd(i,k),    i = 1, ntdb(k), k = 1, nwav: Angular distance (deg) */
/* 	tbz(j,k),    j = 1, ntbz(k), k = 1, nwav: Depth (km) */
/* 	tbtt(i,j,k), i = 1, ntbd(k), j = 1, ntbz(k), k = 1, nwav: */
/* 		     Travel-time (sec, sec/deg) */
/* 	ierr: Error flag; 0: No error */
/* 			  1: One or more files won't open */
/* 			  2: One or more files have unexpected EOF */
/* 	---- Subroutines called ---- */
/* 	From librdwrt */
/* 		rdtab1 - Read the actual travel-time and amplitude tables */
/* DIAGNOSTICS */
/* 	Will specify if absolutely no input files exist. */
/* AUTHOR */
/* 	Steve Bratt */
/* Subroutine */ int rdtab_(froot, wavid, nwav, maxtbd, maxtbz, ntbd, ntbz, 
	tbd, tbz, tbtt, ierr, froot_len, wavid_len)
char *froot, *wavid;
integer *nwav, *maxtbd, *maxtbz, *ntbd, *ntbz;
real *tbd, *tbz, *tbtt;
integer *ierr;
ftnlen froot_len;
ftnlen wavid_len;
{
    /* System generated locals */
    integer tbd_dim1, tbd_offset, tbtt_dim1, tbtt_dim2, tbtt_offset, tbz_dim1,
	     tbz_offset, i__1;

    /* Builtin functions */
    /* Subroutine */ int s_copy();
    integer s_wsfe(), do_fio(), e_wsfe();

    /* Local variables */
    static integer icnt, k, luerr;
    extern /* Subroutine */ int rdtab1_();
    static integer js, kr;
    static char filnam[100];
    extern integer lnblnk_();
    static integer nfiles;

    /* Fortran I/O blocks */
    static cilist io___8 = { 0, 0, 0, "(a)", 0 };


/*     Parameter declaration */
/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     On entry */
/*     On return */
/*     Internal variables */
/*     Set up */
    /* Parameter adjustments */
    --ntbz;
    --ntbd;
    wavid -= wavid_len;
    tbd_dim1 = *maxtbd;
    tbd_offset = 1 + tbd_dim1 * 1;
    tbd -= tbd_offset;
    tbtt_dim1 = *maxtbd;
    tbtt_dim2 = *maxtbz;
    tbtt_offset = 1 + tbtt_dim1 * (1 + tbtt_dim2 * 1);
    tbtt -= tbtt_offset;
    tbz_dim1 = *maxtbz;
    tbz_offset = 1 + tbz_dim1 * 1;
    tbz -= tbz_offset;

    /* Function Body */
    kr = lnblnk_(froot, froot_len);
    s_copy(filnam, froot, (ftnlen)100, kr);
/* Computing MIN */
    i__1 = kr + 1;
    js = min(i__1,98);
    *(unsigned char *)&filnam[js - 1] = '.';
    ++js;
    nfiles = 0;
/*     Read tables */
    icnt = 0;
    i__1 = *nwav;
    for (k = 1; k <= i__1; ++k) {
	++icnt;
	s_copy(filnam + (js - 1), wavid + k * wavid_len, 100 - (js - 1), 
		wavid_len);
	rdtab1_(filnam, maxtbd, maxtbz, &luerr, &ntbd[k], &ntbz[k], &tbd[k * 
		tbd_dim1 + 1], &tbz[k * tbz_dim1 + 1], &tbtt[(k * tbtt_dim2 + 
		1) * tbtt_dim1 + 1], ierr, (ftnlen)100);
	if (*ierr == 0) {
	    ++nfiles;
	}
	if (*ierr == 2) {
	    return 0;
	}
/* L1000: */
    }
/*     Error: No files read */
    if (nfiles == 0 && icnt > 0) {
	io___8.ciunit = luerr;
	s_wsfe(&io___8);
	do_fio(&c__1, "? rdtab: No table files can be read", (ftnlen)35);
	e_wsfe();
	*ierr = 1;
    } else {
	*ierr = 0;
    }
    return 0;
} /* rdtab_ */

