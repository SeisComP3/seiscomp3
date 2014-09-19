/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsrdcortab_1_ {
    char sccsid[80];
};

#define sccsrdcortab_1 (*(struct sccsrdcortab_1_ *) &sccsrdcortab_)

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
    } sccsrdcortab_ = { {'@', '(', '#', ')', 'r', 'd', 'c', 'o', 'r', 't', 
	    'a', 'b', '.', 'f', '\t', '4', '0', '.', '1', '\t', '1', '0', '/',
	     '1', '6', '/', '9', '0', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' '} };


/* Table of constant values */

static integer c__9 = 9;
static integer c__1 = 1;
static integer c__2 = 2;
static integer c__4 = 4;

/* NAME */
/* 	rdcortab -- Read station correction tables in succession. */
/* FILE */
/* 	rdcortab.f */
/* SYNOPSIS */
/* 	Each station, correction-type, region file encountered will be */
/* 	investigated for the existence of corresponding station corrections. */
/* DESCRIPTION */
/* 	Subroutine.  Read station correction tables from files into memory. */
/* 	The filenames to be read have the form froot/cortyp(m).STA.AREA, */
/* 	where froot is the root filename, cortyp(k) is the station */
/* 	correction type identifier, STA is the station name, and AREA is */
/* 	either the regional (*.reg) or local (*.local) station correction */
/* 	identifier. */
/* 	Example: If cortyp(m) = 'TT' and 'AMP', then this routine reads */
/* 	files, 'TT.STA.AREA' and 'AMP.STA.AREA' from the froot file prefix */
/* 	descriptor. */
/* 	---- Indexing ---- */
/* 	k = 1, nwav;	l = 1, nsta;	m = 1, ntype; */
/* 	---- On entry ---- */
/* 	froot:		Root-name of files */
/* 	ntype:		Number of station correction types */
/* 	nsta:		Number of stations */
/* 	nwav:		Number of phases in list */
/* 	staid(l):	Character identifier of l'th station */
/* 	wavid(k):	List of all acceptible phases for arrival time and */
/* 			slowness data */
/* 	cortyp(m):	Character identifier of m'th correction type */
/* 	---- On return ---- */
/* 	ierr:		Error flag; */
/* 			  0: No error */
/* 			  1: One or more files won't open */
/* 			  2: One or more files have unexpected EOF */
/* 			  3: File containing directory pointer for SSSC */
/* 			     is missing' */
/* 	---- Subroutines called ---- */
/* 	Local */
/* 		rdcortab1:	Read station correction files one at a time */
/* DIAGNOSTICS */
/* 	Will specify if absolutely no input files exist. */
/* FILES */

/* NOTES */

/* SEE ALSO */

/* AUTHOR */
/* 	Walter Nagy, October 1990. */
/* Subroutine */ int rdcortab_(froot, cortyp, ntype, staid, wavid, nsta, nwav,
	 ierr, froot_len, cortyp_len, staid_len, wavid_len)
char *froot, *cortyp;
integer *ntype;
char *staid, *wavid;
integer *nsta, *nwav, *ierr;
ftnlen froot_len;
ftnlen cortyp_len;
ftnlen staid_len;
ftnlen wavid_len;
{
    /* System generated locals */
    address a__1[2], a__2[4];
    integer i__1[2], i__2, i__3[4], i__4;
    olist o__1;
    cllist cl__1;

    /* Builtin functions */
    integer s_wsle(), do_lio(), e_wsle();
    /* Subroutine */ int s_stop(), s_copy();
    integer f_open(), s_rsfe(), do_fio(), e_rsfe();
    /* Subroutine */ int s_cat();
    integer f_clos(), s_cmp(), s_wsfe(), e_wsfe();

    /* Local variables */
    static integer icnt, ista, indx[2];
    static char corr_dir__[30];
    extern /* Subroutine */ int rdcortab1_();
    static integer itype, jtype;
    static char ct[8];
    static integer js, jt, kr;
    static char filnam[100];
    extern integer lnblnk_();
    static integer nfiles, ios;

    /* Fortran I/O blocks */
    static cilist io___1 = { 0, 6, 0, 0, 0 };
    static cilist io___6 = { 0, 6, 0, 0, 0 };
    static cilist io___7 = { 0, 6, 0, 0, 0 };
    static cilist io___9 = { 0, 21, 0, "(a)", 0 };
    static cilist io___16 = { 0, 6, 0, 0, 0 };
    static cilist io___19 = { 0, 6, 0, 0, 0 };
    static cilist io___20 = { 0, 0, 0, "(2a)", 0 };


/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     On entry */
/*     Parameter declarations */
/*     Station correction input file unit number */
/*     Maximum permissable different station correction types */
/*     Maximum number of total stations permitted */
/*     Maximum number of stations WITH corrections permitted */
/*     Maximum permissable number of nodes in lat/lon directions */
/*     Maximum average number of tables per station/correction-type pair */
/*     Maximum permissable number of phases */
/*     On return */
/*     Internal variables */
    /* Parameter adjustments */
    cortyp -= cortyp_len;
    staid -= staid_len;
    wavid -= wavid_len;

    /* Function Body */
    if (*ntype > 3) {
	s_wsle(&io___1);
	do_lio(&c__9, &c__1, "Number of station corr. types attempted > para\
meter, maxtyp", (ftnlen)59);
	e_wsle();
	s_stop("", (ftnlen)0);
    }
    kr = lnblnk_(froot, froot_len);
    s_copy(filnam, froot, (ftnlen)100, kr);
    js = kr + 1;
    s_copy(filnam + (js - 1), ".corr_dir", (ftnlen)9, (ftnlen)9);
/*     Open file pointer to station correction directory */
    o__1.oerr = 1;
    o__1.ounit = 21;
    o__1.ofnmlen = 100;
    o__1.ofnm = filnam;
    o__1.orl = 0;
    o__1.osta = "old";
    o__1.oacc = 0;
    o__1.ofm = 0;
    o__1.oblnk = 0;
    ios = f_open(&o__1);
    if (ios != 0) {
	*ierr = 3;
	s_wsle(&io___6);
	do_lio(&c__9, &c__1, "File containing directory pointer for SSSC is \
missing", (ftnlen)53);
	e_wsle();
	s_wsle(&io___7);
	do_lio(&c__9, &c__1, "File: ", (ftnlen)6);
	do_lio(&c__9, &c__1, filnam, js + 8);
	e_wsle();
	return 0;
    }
/*     Back-track to find current directory location */
    for (jt = kr; jt >= 1; --jt) {
	if (*(unsigned char *)&froot[jt - 1] == '/') {
	    js = jt + 1;
	    goto L1002;
	}
/* L1001: */
    }
/*     Now read location of SSSC directory for these travel-time tables */
L1002:
    s_rsfe(&io___9);
    do_fio(&c__1, corr_dir__, (ftnlen)30);
    e_rsfe();
    s_copy(filnam + (js - 1), "         ", 100 - (js - 1), (ftnlen)9);
    kr = lnblnk_(corr_dir__, (ftnlen)30);
/* Writing concatenation */
    i__1[0] = kr, a__1[0] = corr_dir__;
    i__1[1] = 1, a__1[1] = "/";
    s_cat(filnam + (js - 1), a__1, i__1, &c__2, 100 - (js - 1));
    js = js + kr + 1;
    cl__1.cerr = 0;
    cl__1.cunit = 21;
    cl__1.csta = 0;
    f_clos(&cl__1);
/*     Initialize file counter */
    nfiles = 0;
/*     Read station correction tables */
    icnt = 0;
    i__2 = *ntype;
    for (itype = 1; itype <= i__2; ++itype) {
	++icnt;
	kr = lnblnk_(cortyp + itype * cortyp_len, cortyp_len);
	s_copy(ct, cortyp + itype * cortyp_len, (ftnlen)8, cortyp_len);
	if (s_cmp(ct, "TT", kr, (ftnlen)2) == 0) {
	    jtype = 1;
	} else if (s_cmp(ct, "AZ", kr, (ftnlen)2) == 0) {
	    jtype = 2;
	} else if (s_cmp(ct, "AMP", kr, (ftnlen)3) == 0) {
	    jtype = 3;
	} else {
	    s_wsle(&io___16);
	    do_lio(&c__9, &c__1, "Illegal station correction type, ", (ftnlen)
		    33);
	    do_lio(&c__9, &c__1, ct, kr);
	    do_lio(&c__9, &c__1, " !", (ftnlen)2);
	    e_wsle();
	    s_stop("", (ftnlen)0);
	}
	jt = js + (kr << 1) + 2;
/* Writing concatenation */
	i__3[0] = kr, a__2[0] = ct;
	i__3[1] = 1, a__2[1] = "/";
	i__3[2] = kr, a__2[2] = ct;
	i__3[3] = 1, a__2[3] = ".";
	s_cat(filnam + (js - 1), a__2, i__3, &c__4, 100 - (js - 1));
	i__4 = *nsta;
	for (ista = 1; ista <= i__4; ++ista) {
	    kr = lnblnk_(staid + ista * staid_len, staid_len);
/* 	    Read regional station correction files here */
/* Writing concatenation */
	    i__1[0] = kr, a__1[0] = staid + ista * staid_len;
	    i__1[1] = 4, a__1[1] = ".reg";
	    s_cat(filnam + (jt - 1), a__1, i__1, &c__2, 100 - (jt - 1));
	    rdcortab1_(filnam, wavid + wavid_len, nwav, indx, &jtype, &ista, &
		    c__1, ierr, (ftnlen)100, wavid_len);
	    corrs_1.indexstacor[jtype + (ista + 150) * 3 - 454] = (shortint) 
		    indx[0];
	    if (*ierr == 0) {
		++nfiles;
	    } else if (*ierr == 1) {
		goto L1000;
	    } else {
		return 0;
	    }
/* 	    Read local station correction files here */
/* Writing concatenation */
	    i__1[0] = kr, a__1[0] = staid + ista * staid_len;
	    i__1[1] = 6, a__1[1] = ".local";
	    s_cat(filnam + (jt - 1), a__1, i__1, &c__2, 100 - (jt - 1));
	    rdcortab1_(filnam, wavid + wavid_len, nwav, indx, &jtype, &ista, &
		    c__2, ierr, (ftnlen)100, wavid_len);
	    corrs_1.indexstacor[jtype + (ista + 300) * 3 - 454] = (shortint) 
		    indx[1];
	    if (*ierr == 0) {
		++nfiles;
	    }
	    if (*ierr == 2) {
		return 0;
	    }
L1000:
	    ;
	}
/* L1010: */
    }
/*     Error: Too many stations with correction for parameter setting */
    if (indx[0] > 15) {
	s_wsle(&io___19);
	do_lio(&c__9, &c__1, "Number of stations with corrections > paramete\
r, maxcorrsta", (ftnlen)59);
	e_wsle();
	s_stop("", (ftnlen)0);
    }
/*     Error: No files read */
    if (nfiles == 0 && icnt > 0) {
	s_wsfe(&io___20);
	do_fio(&c__1, " rdcortab: No station correction files", (ftnlen)38);
	do_fio(&c__1, " can be read", (ftnlen)12);
	e_wsfe();
	*ierr = 1;
    } else {
	*ierr = 0;
    }
    return 0;
} /* rdcortab_ */

