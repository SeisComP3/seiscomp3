/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"
#define ftnlen int

/* Common Block Declarations */

struct sccsrdtab1_1_ {
    char sccsid[80];
};

#define sccsrdtab1_1 (*(struct sccsrdtab1_1_ *) &sccsrdtab1_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsrdtab1_ = { {'@', '(', '#', ')', 'r', 'd', 't', 'a', 'b', '1', '.', 
	    'f', '\t', '3', '6', '.', '2', '\t', '3', '/', '1', '7', '/', '8',
	     '9', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static integer c__1 = 1;
static integer c__3 = 3;
static integer c__4 = 4;

/* NAME */
/* 	rdtab1 \(em Read travel-time tables */
/* FILE */
/* 	rdtab1.f */
/* SYNOPSIS */
/* 	Read travel-time and amplitude tables for individual phase-types */
/* DESCRIPTION */
/* 	Subroutine.  Read travel-time and amplitude tables applicable for */
/* 	the given input phase-types */
/* 	On entry \(em */
/* 	filnam: Name of file to be read */
/* 	maxtbd: Maximum number of distance samples in tables */
/* 	maxtbz: Maximum number of depth samples in tables */
/* 	luerr : Logical unit number for error output */
/* 	On return \(em */
/* 	ntbd: Number of distance samples in tables */
/* 	ntbz: Number of depth samples in tables */
/* 	   For i = 1, ntbd: tbd(i): Angular distance of (i,j)'th sample */
/* 	   For j = 1, ntbz: tbz(j): Depth of (i,j)'th sample */
/* 	   For i = 1, ntbd, j = 1, ntbz: tbtt(i,j): */
/* 	       Travel-time (i,j)'th sample (period for R wave) */
/* 	ierr: Error flag; 0: No error */
/* 			  1: File won't open */
/* 			  2: Unexpected End-Of-File */
/* 	Note: If file, filnam will not open, then arrays, ntbd() and ntbz() */
/* 	are returned as zero. */
/* DIAGNOSTICS */
/* 	Will specify when desired input file cannot be opened. */
/* AUTHOR */
/* 	Steve Bratt */
/* Subroutine */ int rdtab1_(filnam, maxtbd, maxtbz, luerr, ntbd, ntbz, tbd, 
	tbz, tbtt, ierr, filnam_len)
char *filnam;
integer *maxtbd, *maxtbz, *luerr, *ntbd, *ntbz;
real *tbd, *tbz, *tbtt;
integer *ierr;
ftnlen filnam_len;
{
    /* System generated locals */
    integer tbtt_dim1, tbtt_offset, i__1, i__2, i__3, i__4;
    olist o__1;
    cllist cl__1;

    /* Builtin functions */
    integer f_open(), s_wsfe(), do_fio(), e_wsfe(), s_rsfe(), e_rsfe(), 
	    s_rsle(), do_lio(), e_rsle(), f_clos();

    /* Local variables */
    static integer i__, j, k, ntbdx, ntbzx;
    extern integer lnblnk_();
    static char string[80];
    static real dum;
    static integer ios;

    /* Fortran I/O blocks */
    static cilist io___3 = { 0, 0, 0, "(3a)", 0 };
    static cilist io___4 = { 0, 11, 1, "(a)", 0 };
    static cilist io___6 = { 0, 11, 1, 0, 0 };
    static cilist io___8 = { 0, 0, 0, "(2a)", 0 };
    static cilist io___9 = { 0, 0, 0, "(2(a,i4))", 0 };
    static cilist io___10 = { 0, 11, 1, 0, 0 };
    static cilist io___13 = { 0, 11, 1, 0, 0 };
    static cilist io___15 = { 0, 0, 0, "(2a)", 0 };
    static cilist io___16 = { 0, 0, 0, "(2(a,i4))", 0 };
    static cilist io___17 = { 0, 11, 1, 0, 0 };
    static cilist io___19 = { 0, 11, 1, "(a)", 0 };
    static cilist io___20 = { 0, 11, 1, 0, 0 };
    static cilist io___21 = { 0, 0, 0, "(2a)", 0 };


/*     On entry */
/* K.S. 1-Dec-97, changed 'implicit' to 'none' */
/*     On return */
/*     Internal variables */
/*     Open file */
    /* Parameter adjustments */
    --tbd;
    tbtt_dim1 = *maxtbd;
    tbtt_offset = 1 + tbtt_dim1 * 1;
    tbtt -= tbtt_offset;
    --tbz;

    /* Function Body */
    k = lnblnk_(filnam, filnam_len);
    o__1.oerr = 1;
    o__1.ounit = 11;
    o__1.ofnmlen = filnam_len;
    o__1.ofnm = filnam;
    o__1.orl = 0;
    o__1.osta = "old";
    o__1.oacc = 0;
    o__1.ofm = 0;
    o__1.oblnk = 0;
    ios = f_open(&o__1);
    if (ios != 0) {
	io___3.ciunit = *luerr;
	s_wsfe(&io___3);
	do_fio(&c__1, "? File ", (ftnlen)7);
	do_fio(&c__1, filnam, k);
	do_fio(&c__1, " will not open", (ftnlen)14);
	e_wsfe();
	*ntbd = 0;
	*ntbz = 0;
	*ierr = 1;
	return 0;
    }
/*     Read title of the table. */
    i__1 = s_rsfe(&io___4);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = do_fio(&c__1, string, (ftnlen)80);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = e_rsfe();
    if (i__1 != 0) {
	goto L9000;
    }
/*     Read depth sampling */
    i__1 = s_rsle(&io___6);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = do_lio(&c__3, &c__1, (char *)&ntbzx, (ftnlen)sizeof(integer));
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = e_rsle();
    if (i__1 != 0) {
	goto L9000;
    }
    *ntbz = min(*maxtbz,ntbzx);
    if (ntbzx > *maxtbz) {
	io___8.ciunit = *luerr;
	s_wsfe(&io___8);
	do_fio(&c__1, "? Too many depth samples in file ", (ftnlen)33);
	do_fio(&c__1, filnam, k);
	e_wsfe();
	io___9.ciunit = *luerr;
	s_wsfe(&io___9);
	do_fio(&c__1, "  Number in file:", (ftnlen)17);
	do_fio(&c__1, (char *)&ntbzx, (ftnlen)sizeof(integer));
	do_fio(&c__1, "  Number kept:", (ftnlen)14);
	do_fio(&c__1, (char *)&(*maxtbz), (ftnlen)sizeof(integer));
	e_wsfe();
    }
    i__1 = s_rsle(&io___10);
    if (i__1 != 0) {
	goto L9000;
    }
    i__2 = *ntbz;
    for (i__ = 1; i__ <= i__2; ++i__) {
	i__1 = do_lio(&c__4, &c__1, (char *)&tbz[i__], (ftnlen)sizeof(real));
	if (i__1 != 0) {
	    goto L9000;
	}
    }
    i__3 = ntbzx;
    for (i__ = *ntbz + 1; i__ <= i__3; ++i__) {
	i__1 = do_lio(&c__4, &c__1, (char *)&dum, (ftnlen)sizeof(real));
	if (i__1 != 0) {
	    goto L9000;
	}
    }
    i__1 = e_rsle();
    if (i__1 != 0) {
	goto L9000;
    }
/*     Read distance sampling */
    i__1 = s_rsle(&io___13);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = do_lio(&c__3, &c__1, (char *)&ntbdx, (ftnlen)sizeof(integer));
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = e_rsle();
    if (i__1 != 0) {
	goto L9000;
    }
    *ntbd = min(*maxtbd,ntbdx);
    if (ntbdx > *maxtbd) {
	io___15.ciunit = *luerr;
	s_wsfe(&io___15);
	do_fio(&c__1, "? Too many distance samples in file ", (ftnlen)36);
	do_fio(&c__1, filnam, k);
	e_wsfe();
	io___16.ciunit = *luerr;
	s_wsfe(&io___16);
	do_fio(&c__1, "  Number in file:", (ftnlen)17);
	do_fio(&c__1, (char *)&ntbdx, (ftnlen)sizeof(integer));
	do_fio(&c__1, "  Number kept:", (ftnlen)14);
	do_fio(&c__1, (char *)&(*maxtbd), (ftnlen)sizeof(integer));
	e_wsfe();
    }
    i__1 = s_rsle(&io___17);
    if (i__1 != 0) {
	goto L9000;
    }
    i__2 = *ntbd;
    for (i__ = 1; i__ <= i__2; ++i__) {
	i__1 = do_lio(&c__4, &c__1, (char *)&tbd[i__], (ftnlen)sizeof(real));
	if (i__1 != 0) {
	    goto L9000;
	}
    }
    i__3 = ntbdx;
    for (i__ = *ntbd + 1; i__ <= i__3; ++i__) {
	i__1 = do_lio(&c__4, &c__1, (char *)&dum, (ftnlen)sizeof(real));
	if (i__1 != 0) {
	    goto L9000;
	}
    }
    i__1 = e_rsle();
    if (i__1 != 0) {
	goto L9000;
    }
/*     Read tables */
    i__1 = *ntbz;
    for (j = 1; j <= i__1; ++j) {
	i__2 = s_rsfe(&io___19);
	if (i__2 != 0) {
	    goto L9000;
	}
	i__2 = do_fio(&c__1, string, (ftnlen)80);
	if (i__2 != 0) {
	    goto L9000;
	}
	i__2 = e_rsfe();
	if (i__2 != 0) {
	    goto L9000;
	}
	i__2 = s_rsle(&io___20);
	if (i__2 != 0) {
	    goto L9000;
	}
	i__3 = *ntbd;
	for (i__ = 1; i__ <= i__3; ++i__) {
	    i__2 = do_lio(&c__4, &c__1, (char *)&tbtt[i__ + j * tbtt_dim1], (
		    ftnlen)sizeof(real));
	    if (i__2 != 0) {
		goto L9000;
	    }
	}
	i__4 = ntbdx;
	for (i__ = *ntbd + 1; i__ <= i__4; ++i__) {
	    i__2 = do_lio(&c__4, &c__1, (char *)&dum, (ftnlen)sizeof(real));
	    if (i__2 != 0) {
		goto L9000;
	    }
	}
	i__2 = e_rsle();
	if (i__2 != 0) {
	    goto L9000;
	}
/* L1000: */
    }
    cl__1.cerr = 0;
    cl__1.cunit = 11;
    cl__1.csta = 0;
    f_clos(&cl__1);
    *ierr = 0;
    return 0;
/*     Error */
L9000:
    io___21.ciunit = *luerr;
    s_wsfe(&io___21);
    do_fio(&c__1, "? Unexpected End-Of-File for file ", (ftnlen)34);
    do_fio(&c__1, filnam, k);
    e_wsfe();
    cl__1.cerr = 0;
    cl__1.cunit = 11;
    cl__1.csta = 0;
    f_clos(&cl__1);
    *ierr = 2;
    return 0;
} /* rdtab1_ */

