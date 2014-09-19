/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"
#define ftnlen int

/* Common Block Declarations */

struct sccsrdcortab1_1_ {
    char sccsid[80];
};

#define sccsrdcortab1_1 (*(struct sccsrdcortab1_1_ *) &sccsrdcortab1_)

static struct {
    real splat[18900]	/* was [3][15][2][210] */, splon[18900]	/* was [3][15]
	    [2][210] */, xlat1[1350]	/* was [3][15][2][15] */, xlat2[1350]	
	    /* was [3][15][2][15] */, xlon1[1350]	/* was [3][15][2][15] 
	    */, xlon2[1350]	/* was [3][15][2][15] */;
    shortint stacor[303750]	/* was [3][15][2][3375] */, numsrcs[2250]	
	    /* was [3][15][2][25] */, cumulsrcs[2250]	/* was [3][15][2][25] 
	    */, nodecumul[1350]	/* was [3][15][2][15] */, nlats[1350]	/* 
	    was [3][15][2][15] */, nlons[1350]	/* was [3][15][2][15] */, 
	    indexstacor[900]	/* was [3][150][2] */;
} corrs_;

#define corrs_1 corrs_

/* Initialized data */

struct {
    char e_1[80];
    } sccsrdcortab1_ = { {'@', '(', '#', ')', 'r', 'd', 'c', 'o', 'r', 't', 
	    'a', 'b', '1', '.', 'f', '\t', '4', '4', '.', '1', '\t', '9', '/',
	     '2', '0', '/', '9', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' '} };


/* Table of constant values */

static integer c__1 = 1;
static integer c__9 = 9;

/* NAME */
/* 	rdcortab1 -- Read station correction data from a single file. */
/* FILE */
/* 	rdcortab1.f */
/* SYNOPSIS */
/* 	Read station-correction tables for a given wave, region and station */
/* 	from a individual correction file. */
/* DESCRIPTION */
/* 	Subroutine.  Do acutal reading of station correction information */
/* 	here.  For each correction type, station, region and wave type, */
/* 	read source-specific station corrections defined at lat/lon nodes. */
/* 	Origin of source-specific station correction lies in the northwest */
/* 	corner of the model. */
/* 	---- Indexing ---- */
/* 	k = 1, nwav; */
/* 	---- On entry ---- */
/* 	filnam:		Name of source-specific station correction file to */
/* 			be read */
/* 	nwav:		Number of phases in list */
/* 	nsta:		Number of stations */
/* 	iarea:		Regional area index (1 = regional; 2 = local) */
/* 	ista:		Station index */
/* 	jtype:		Correction-type index */
/* 	indx(2):	Specific-correction index; regional (1) or local (2) */
/* 	wavid(k):	List of all acceptible phases for arrival time and */
/* 			slowness data */
/* 	---- On return ---- */
/* 	ierr:	Error flag;	0: No error */
/* 				1: File won't open */
/* 				2: Unexpected End-Of-File */
/* DIAGNOSTICS */
/* 	Complains if it cannot open a requested file. */
/* FILES */
/* 	Reads all source-specific station correction files here. */
/* NOTES */
/* 	If file, filnam will not open, then arrays, ntbd() and ntbz() */
/* 	are returned as zero. */
/* 	Remember to initialize cumulsrcs()! */
/* SEE ALSO */
/* 	Complementary function, rdcortab(). */
/* AUTHOR */
/* 	Walter Nagy, October 1990. */
/* Subroutine */ int rdcortab1_(filnam, wavid, nwav, indx, jtype, ista, iarea,
	 ierr, filnam_len, wavid_len)
char *filnam, *wavid;
integer *nwav, *indx, *jtype, *ista, *iarea, *ierr;
ftnlen filnam_len;
ftnlen wavid_len;
{
    /* System generated locals */
    integer i__1, i__2, i__3, i__4;
    olist o__1;
    cllist cl__1;
    alist al__1;

    /* Builtin functions */
    integer f_open(), f_rew(), s_rsfe(), do_fio(), e_rsfe(), s_cmp();
    logical l_gt();
    integer s_wsle(), do_lio(), e_wsle();
    /* Subroutine */ int s_stop();
    integer s_rsle(), e_rsle(), f_clos(), s_wsfe(), e_wsfe();

    /* Local variables */
    static integer jend;
    static shortint isrc[25];
    static real dist;
    static integer isln;
    static real xlat;
    static integer iphz[25], islt;
    static shortint nphz;
    static real xlon;
    static integer i__, j, k, n, isave, j1, j2;
    extern integer lnblnk_();
    static char phases[200];
    extern /* Subroutine */ int clitok_();
    static char string[100];
    static integer ind;
    static shortint nln;
    static integer ios;
    static real sln[15];
    static shortint nlt;
    static char phz[8*25];
    static real slt[15];
    static shortint nodetot;

    /* Fortran I/O blocks */
    static cilist io___5 = { 0, 21, 1, "(a/)", 0 };
    static cilist io___11 = { 0, 21, 1, "(a200)", 0 };
    static cilist io___18 = { 0, 21, 0, "(20(i2,6x))", 0 };
    static cilist io___21 = { 0, 6, 0, 0, 0 };
    static cilist io___22 = { 0, 21, 1, "(/a)", 0 };
    static cilist io___23 = { 0, 21, 0, "(2f9.3)", 0 };
    static cilist io___26 = { 0, 21, 0, "(2i4)", 0 };
    static cilist io___31 = { 0, 21, 0, "(20f8.3)", 0 };
    static cilist io___33 = { 0, 21, 0, "(20f8.3)", 0 };
    static cilist io___36 = { 0, 6, 0, 0, 0 };
    static cilist io___37 = { 0, 6, 0, 0, 0 };
    static cilist io___38 = { 0, 6, 0, 0, 0 };
    static cilist io___39 = { 0, 6, 0, 0, 0 };
    static cilist io___40 = { 0, 21, 0, 0, 0 };
    static cilist io___41 = { 0, 21, 1, "(20i6)", 0 };
    static cilist io___42 = { 0, 0, 0, "(2a)", 0 };


/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry ---- */
/*     Parameter declarations */
/*     Station correction input file unit number */
/*     Maximum permissable different station correction types */
/*     Maximum number of total stations permitted */
/*     Maximum number of stations WITH corrections permitted */
/*     Maximum permissable number of nodes in lat/lon directions */
/*     Maximum average number of tables per station/correction-type pair */
/*     Maximum permissable number of phases */
/*     ---- On return ---- */
/*      integer*4 indx(2) */
/*     ---- Internal variables ---- */
    /* Parameter adjustments */
    wavid -= wavid_len;
    --indx;

    /* Function Body */
    *ierr = 0;
    if (*ista == 1) {
	isave = 0;
    }
    if (indx[*iarea] != 0) {
	isave = indx[*iarea];
    }
/*     Open merged station correction files with stations from station list */
    k = lnblnk_(filnam, filnam_len);
    o__1.oerr = 1;
    o__1.ounit = 21;
    o__1.ofnmlen = filnam_len;
    o__1.ofnm = filnam;
    o__1.orl = 0;
    o__1.osta = "old";
    o__1.oacc = 0;
    o__1.ofm = 0;
    o__1.oblnk = 0;
    ios = f_open(&o__1);
    if (ios != 0) {
	*ierr = 1;
	indx[*iarea] = 0;
	return 0;
    }
    al__1.aerr = 0;
    al__1.aunit = 21;
    f_rew(&al__1);
    indx[*iarea] = isave + 1;
    ind = indx[*iarea];
/*     Read title of the table */
    i__1 = s_rsfe(&io___5);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = do_fio(&c__1, string, (ftnlen)100);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = e_rsfe();
    if (i__1 != 0) {
	goto L9000;
    }
    nphz = 0;
    i__1 = *nwav;
    for (i__ = 1; i__ <= i__1; ++i__) {
	iphz[i__ - 1] = 0;
	isrc[i__ - 1] = 0;
	corrs_1.numsrcs[*jtype + (ind + (*iarea + (i__ << 1)) * 15) * 3 - 139]
		 = 0;
	corrs_1.cumulsrcs[*jtype + (ind + (*iarea + (i__ << 1)) * 15) * 3 - 
		139] = 0;
/* L1000: */
    }
/*     Determine the applicable phase-types and the number of associated */
/*     source regions */
    i__1 = s_rsfe(&io___11);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = do_fio(&c__1, phases, (ftnlen)200);
    if (i__1 != 0) {
	goto L9000;
    }
    i__1 = e_rsfe();
    if (i__1 != 0) {
	goto L9000;
    }
    for (i__ = 1; i__ <= 25; ++i__) {
	clitok_(phases, &i__, phz + (i__ - 1 << 3), &n, (ftnlen)200, (ftnlen)
		8);
	if (s_cmp(phz + (i__ - 1 << 3), " ", (ftnlen)8, (ftnlen)1) != 0) {
	    j1 = lnblnk_(phz + (i__ - 1 << 3), (ftnlen)8);
	    i__1 = *nwav;
	    for (k = 1; k <= i__1; ++k) {
		j2 = lnblnk_(wavid + k * wavid_len, wavid_len);
		i__2 = j2;
		for (j = 1; j <= i__2; ++j) {
		    if (l_gt("$", wavid + (k * wavid_len + (j - 1)), (ftnlen)
			    1, (ftnlen)1)) {
			j2 = j - 1;
			goto L1014;
		    }
/* L1012: */
		}
L1014:
		if (j1 == j2 && s_cmp(phz + (i__ - 1 << 3), wavid + k * 
			wavid_len, j1, j2) == 0) {
		    nphz = (shortint) (nphz + 1);
		    iphz[nphz - 1] = k;
		    goto L1020;
		}
/* L1010: */
	    }
	}
L1020:
	;
    }
    if (nphz == 0) {
	*ierr = 1;
	indx[*iarea] = 0;
	return 0;
    }
    s_rsfe(&io___18);
    i__1 = nphz;
    for (i__ = 1; i__ <= i__1; ++i__) {
	do_fio(&c__1, (char *)&isrc[iphz[i__ - 1] - 1], (ftnlen)sizeof(
		shortint));
    }
    e_rsfe();
    i__1 = nphz;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1030: */
	corrs_1.numsrcs[*jtype + (ind + (*iarea + (iphz[i__ - 1] << 1)) * 15) 
		* 3 - 139] = isrc[iphz[i__ - 1] - 1];
    }
    corrs_1.cumulsrcs[*jtype + (ind + (*iarea + (iphz[0] << 1)) * 15) * 3 - 
	    139] = isrc[iphz[0] - 1];
    i__1 = nphz;
    for (i__ = 2; i__ <= i__1; ++i__) {
/* L1040: */
	corrs_1.cumulsrcs[*jtype + (ind + (*iarea + (iphz[i__ - 1] << 1)) * 
		15) * 3 - 139] = corrs_1.cumulsrcs[*jtype + (ind + (*iarea + (
		iphz[i__ - 2] << 1)) * 15) * 3 - 139] + isrc[iphz[i__ - 1] - 
		1];
    }
/*     Read station corrections */
    nodetot = 0;
    jend = corrs_1.cumulsrcs[*jtype + (ind + (*iarea + (iphz[nphz - 1] << 1)) 
	    * 15) * 3 - 139];
    if (jend > 15) {
	s_wsle(&io___21);
	do_lio(&c__9, &c__1, "Number of tables for this station > parameter,\
 maxtab", (ftnlen)53);
	e_wsle();
	s_stop("", (ftnlen)0);
    }
    i__1 = jend;
    for (j = 1; j <= i__1; ++j) {
	corrs_1.nodecumul[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139]
		 = nodetot;
	i__2 = s_rsfe(&io___22);
	if (i__2 != 0) {
	    goto L9000;
	}
	i__2 = do_fio(&c__1, string, (ftnlen)100);
	if (i__2 != 0) {
	    goto L9000;
	}
	i__2 = e_rsfe();
	if (i__2 != 0) {
	    goto L9000;
	}
	s_rsfe(&io___23);
	do_fio(&c__1, (char *)&xlat, (ftnlen)sizeof(real));
	do_fio(&c__1, (char *)&xlon, (ftnlen)sizeof(real));
	e_rsfe();
	s_rsfe(&io___26);
	do_fio(&c__1, (char *)&nlt, (ftnlen)sizeof(shortint));
	do_fio(&c__1, (char *)&nln, (ftnlen)sizeof(shortint));
	e_rsfe();
	corrs_1.xlat1[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139] = 
		xlat;
	corrs_1.xlon1[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139] = 
		xlon;
	corrs_1.nlats[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139] = 
		nlt;
	corrs_1.nlons[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139] = 
		nln;
	j1 = j - 1;
	islt = nlt - 1;
	isln = nln - 1;
	s_rsfe(&io___31);
	i__2 = islt;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    do_fio(&c__1, (char *)&slt[i__ - 1], (ftnlen)sizeof(real));
	}
	e_rsfe();
	s_rsfe(&io___33);
	i__2 = isln;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    do_fio(&c__1, (char *)&sln[i__ - 1], (ftnlen)sizeof(real));
	}
	e_rsfe();
	i__2 = islt;
	for (i__ = 1; i__ <= i__2; ++i__) {
/* L1050: */
	    corrs_1.splat[*jtype + (ind + (*iarea + (j1 * islt + i__ << 1)) * 
		    15) * 3 - 139] = slt[i__ - 1];
	}
	i__2 = isln;
	for (i__ = 1; i__ <= i__2; ++i__) {
/* L1060: */
	    corrs_1.splon[*jtype + (ind + (*iarea + (j1 * isln + i__ << 1)) * 
		    15) * 3 - 139] = sln[i__ - 1];
	}
	dist = slt[0];
	i__2 = islt;
	for (i__ = 2; i__ <= i__2; ++i__) {
/* L1070: */
	    dist += slt[i__ - 1];
	}
	if (dist > (float)180.) {
	    j2 = lnblnk_(filnam, filnam_len);
	    s_wsle(&io___36);
	    do_lio(&c__9, &c__1, "- SSSC model bounds for lat. exceed 180 de\
g. in", (ftnlen)47);
	    e_wsle();
	    s_wsle(&io___37);
	    do_lio(&c__9, &c__1, "  file: ", (ftnlen)8);
	    do_lio(&c__9, &c__1, filnam, j2);
	    do_lio(&c__9, &c__1, " --> Phase-type: ", (ftnlen)17);
	    do_lio(&c__9, &c__1, wavid + iphz[j - 1] * wavid_len, wavid_len);
	    e_wsle();
	    s_stop("", (ftnlen)0);
	}
	corrs_1.xlat2[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139] = 
		xlat - dist;
	dist = sln[0];
	i__2 = isln;
	for (i__ = 2; i__ <= i__2; ++i__) {
/* L1080: */
	    dist += sln[i__ - 1];
	}
	if (dist > (float)360.) {
	    j2 = lnblnk_(filnam, filnam_len);
	    s_wsle(&io___38);
	    do_lio(&c__9, &c__1, "- SSSC model bounds for lon. exceed 360 de\
g. in", (ftnlen)47);
	    e_wsle();
	    s_wsle(&io___39);
	    do_lio(&c__9, &c__1, "  file: ", (ftnlen)8);
	    do_lio(&c__9, &c__1, filnam, j2);
	    do_lio(&c__9, &c__1, " --> Phase-type: ", (ftnlen)17);
	    do_lio(&c__9, &c__1, wavid + iphz[j - 1] * wavid_len, wavid_len);
	    e_wsle();
	    s_stop("", (ftnlen)0);
	}
	xlon += dist;
	if (xlon > (float)180.) {
	    corrs_1.xlon2[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139]
		     = xlon - (float)360.;
	} else {
	    corrs_1.xlon2[*jtype + (ind + (*iarea + (j << 1)) * 15) * 3 - 139]
		     = xlon;
	}
	s_rsle(&io___40);
	e_rsle();
	i__2 = nlt;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = s_rsfe(&io___41);
	    if (i__3 != 0) {
		goto L9000;
	    }
	    i__4 = nln;
	    for (i__ = 1; i__ <= i__4; ++i__) {
		i__3 = do_fio(&c__1, (char *)&corrs_1.stacor[*jtype + (ind + (
			*iarea + (nodetot + i__ << 1)) * 15) * 3 - 139], (
			ftnlen)sizeof(shortint));
		if (i__3 != 0) {
		    goto L9000;
		}
	    }
	    i__3 = e_rsfe();
	    if (i__3 != 0) {
		goto L9000;
	    }
	    nodetot += nln;
/* L1090: */
	}
/* L1100: */
    }
    cl__1.cerr = 0;
    cl__1.cunit = 21;
    cl__1.csta = 0;
    f_clos(&cl__1);
    *ierr = 0;
    return 0;
L9000:
    s_wsfe(&io___42);
    do_fio(&c__1, " Unexpected End-Of-File for file: ", (ftnlen)34);
    do_fio(&c__1, filnam, k);
    e_wsfe();
    cl__1.cerr = 0;
    cl__1.cunit = 21;
    cl__1.csta = 0;
    f_clos(&cl__1);
    *ierr = 2;
    return 0;
} /* rdcortab1_ */

