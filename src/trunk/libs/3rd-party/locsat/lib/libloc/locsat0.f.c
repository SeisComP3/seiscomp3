/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccslocsat0_1_ {
    char sccsid[80];
};

#define sccslocsat0_1 (*(struct sccslocsat0_1_ *) &sccslocsat0_)

/* Initialized data */

struct {
    char e_1[80];
    } sccslocsat0_ = { {'@', '(', '#', ')', 'l', 'o', 'c', 's', 'a', 't', '0',
	     '.', 'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', 
	    '/', '9', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' '} };


/* Table of constant values */

static integer c__1 = 1;
static integer c__9 = 9;
static real c_b80 = (float)6371.;
static real c_b183 = (float)0.;
static doublereal c_b322 = .5;

/* NAME */
/* 	locsat0 -- Locate events using Slowness, Azimuth and Time data. */
/* FILE */
/* 	locsat0.f */
/* SYNOPSIS */
/* 	Computes event locations, confidence bounds, residuals and */
/* 	importances using arrival time, azimuths and slowness */
/* 	measurements from stations at regional and teleseismic */
/* 	distances. */
/* DESCRIPTION */
/* 	Subroutine.  Information on travel-time tables, stations, */
/* 	detections and location parameters are passed to and from */
/* 	LocSAT via the argument list.  The phase and station names */
/* 	given for each datum (dstaid,dwavid) must match those in the */
/* 	lists of acceptable phases and stations (staid,wavid). */
/* 	---- Indexing ---- */
/* 	i = 1, nsta;	j = 1, nwav;	k = 1, ntbd(j);	m = 1, ntbz(j); */
/* 	n = 1, ndata; */
/* 	---- On entry ---- */
/* 	ndata:	Number of data */
/* 	nsta:	Number of stations in network */
/* 	nwav:	Number of phases in list */
/* 	maxtbd:	Maximum dimension of k'th position in tbd(), tbtt() */
/* 	maxtbz:	Maximum dimension of m'th position in tbz(), tbtt() */
/* 	dstaid(n):	Name of station for n'th datum */
/* 	dwavid(n):	Name of phase for n'th datum */
/* 	dtype(n):	Data type for n'th datum (time, azim, slow) */
/* 	atype(n):	Arrival usage */
/* 			  = d: Defining, used in location */
/* 			  = n: Could be defining, but not used in location */
/* 			  = a: Not to be used in location */
/* 	dobs(n):	Value of n'th datum (sec, deg, sec/deg) */
/* 	dsd(n): 	Standard deviation in value of n'th datum */
/* 	idarid(n):	Arrival ID for datum */
/* 	staid(i):	List of all acceptible station names */
/* 	stalat(i):	Station latitudes  (deg) */
/* 	stalon(i):	Station longitudes (deg) */
/* 	wavid(j):	List of all acceptible phases for arrival time and */
/* 		        slowness data (these are the suffixes for the */
/* 			travel-time tables */
/* 	ntbd(j):	Number of distance samples in travel-time tables */
/* 	ntbz(j):	Number of depth samples in travel-time tables */
/* 	tbd(k,j):	Distance to k'th lat/lon travel-time node (deg) */
/* 	tbz(m,j):	Depth to m'th travel-time node from Earths surface (km) */
/* 	tbtt(k,m,j):	Travel-time of k'th lat/lon and m'th depth nodes (sec) */
/* 	alat0:  	Initial guess of event latitude (deg) */
/* 	alon0:  	Initial guess of event longitude (deg) */
/* 			[If alat0 or alon0 non-possible values (e.g. */
/* 			abs (lat or lon) > 90 or 180), subroutine hypcut0 */
/* 			will estimate a starting location.] */
/* 	zfoc0:  	Initial guess of event focal depth (km) */
/* 			[WARNING: alat0, alon0, zfoc0 may be changed on */
/* 			output from this subroutine if the initial guess */
/* 			was changed during processing] */
/* 	sig0:   	Prior estimate of data standard error */
/* 	ndf0:   	Number of degrees of freedom in sig0 */
/* 	pconf:  	Confidence probability for confidence regions (0.0-1.0) */
/* 			[WARNING: subroutine fstat.f only accepts .9 for now] */
/* 	azwt:   	Weight applied to azimuth data and partials */
/* 			(default = 1.0) */
/* 	damp:   	Percent damping relative to largest singular value, */
/* 			if < 0.0, only damp when condition number > million */
/* 	maxit:  	Maximum number of iterations allowed in inversion */
/* 	prtflg: 	= y, Verbose printout */
/* 			= n, None. */
/* 	fxdflg: 	= n, Focal depth is a free parameter in inversion */
/* 			= y, Focal depth is constrained to equal zfoc0 */
/* 	outfile:	Output file name */
/* 	luout:  	Logical unit number for output file */
/* 	---- On return ---- */
/* 	alat:	Final estimate of event latitude (deg) */
/* 	alon:	Final estimate of event longitude (deg) */
/* 	zfoc:	Final estimate of event focal depth (km) */
/* 	torg:	Final estimate of event origin time (sec) */
/* 	sighat:	Final estimate of data standard error */
/* 	snssd:	Normalized sample standard deviation */
/* 	ndf:	Number of degrees of freedom in sighat */
/* 	epmaj:	Length of semi-major axis of confidence ellipse on */
/* 		epicenter (km) */
/* 	epmin:	Length of semi-minor axis of confidence ellipse on */
/* 		epicenter (km) */
/* 	epstr:	Strike of semi-major axis of confidence ellipse on */
/* 		epicenter (deg) */
/* 	zfint:	Length of confidence semi-interval on focal depth (km) */
/* 		= < 0.0 if fxdflg = y' or depth was fixed by program due */
/* 		to convergence problem */
/* 	toint:	Length of confidence semi-interval on origin time (sec) */
/* 	rank:	Effective rank of the sensitivity matrix */
/* 	niter:	Total number of iterations performed during inversion */
/* 	sxx:	(Parameter covariance element) */
/* 	syy:	(Parameter covariance element) */
/* 	szz:	(Parameter covariance element) */
/* 		= < 0.0 if fxdflg = y' or depth was fixed by program due */
/* 		to convergence problem */
/* 	stt:	(Parameter covariance element) */
/* 	sxy:	(Parameter covariance element) */
/* 	sxz:	(Parameter covariance element) */
/* 	syz:	(Parameter covariance element) */
/* 	stx:	(Parameter covariance element) */
/* 	sty:	(Parameter covariance element) */
/* 	stz:	(Parameter covariance element) */
/* 	stadel(i):	Distance from epicenter to i'th station (deg) */
/* 	staazi(i):	Azimuth from epicenter to i'th station (deg) */
/* 	stabaz(i):	Back-azimuth from epicenter to i'th station (deg) */
/* 	epimp(n):	Epicenter importance of n'th datum */
/* 	zfimp(n):	Depth importance of n'th datum */
/* 	resid(n):	Residual (obs-calc) for n'th datum (sec, deg, sec/deg) */
/* 	ipsta(n):	Station index for n'th observation */
/* 	iderr(n):	Error code for n'th observation */
/* 			=  0, No problem, normal interpolation */
/* 			=  1, No station information for datum */
/* 			=  2, No travel-time tables for datum */
/* 			=  3, Data type unknown */
/* 			=  4, S.D <= 0.0 for datum */
/* 			= 11, Distance-depth point (x0,z0) in hole of T-T curve */
/* 			= 12, x0 < x(1) */
/* 			= 13, x0 > x(max) */
/* 			= 14, z0 < z(1) */
/* 			= 15, z0 > z(max) */
/* 			= 16, x0 < x(1) and z0 < z(1) */
/* 			= 17, x0 > x(max) and z0 < z(1) */
/* 			= 18, x0 < x(1) and z0 > z(max) */
/* 			= 19, x0 > x(max) and z0 > z(max) */
/* 	[NOTE:	If any of these codes is .le. 0 (e.g. iderr = -17), */
/* 		then, the datum was used to compute event location] */
/* 	ierr:	Error flag; */
/* 		  = 0,	No error */
/* 		  = 1,	Maximum number of iterations exhausted */
/* 		  = 2,	Iteration diverged */
/* 		  = 3,	Too few usable data */
/* 		  = 4,	Too few usable data to constrain origin time, */
/* 			however, a valid location was obtained */
/*                 = 5,	Insufficient data for a solution */
/*                 = 6,	SVD routine cannot decompose matrix */
/* 	---- Subroutines called ---- */
/* 	Local */
/* 		check_data_:	Review and quality check of data */
/* 		hypcut0:	Compute first initial guess hypocenter */
/* 		hypinv0:	Compute location */
/* 		index_array	Sort in ascending order by index */
/* 	---- Functions called ---- */
/* 	From libtime */
/* 		etoh:  		Compute human time from epoch time */
/* DIAGNOSTICS */
/* 	Complains when input data are bad ... */
/* FILES */
/* 	Open an output file to a specified unit number or standard out. */
/* NOTES */
/* 	Remember to add time-offset variable and remove zfimp() from */
/* 	arguments in the calling subroutine. */
/* SEE ALSO */
/* 	Bratt and Bache (1988) Locating events with a sparse network of */
/* 	regional arrays, BSSA, 78, 780-798. */
/* AUTHORS */
/* 	Steve Bratt, December 1988. */
/* 	Walter Nagy, November 1990. */
/* Subroutine */ int locsat0_(dstaid, dwavid, dtype, atype, dobs, dsd, idarid,
	 ndata, staid, stalat, stalon, stelev, stacor, nsta, wavid, nwav, 
	maxtbd, maxtbz, ntbd, ntbz, tbd, tbz, tbtt, alat0, alon0, zfoc0, sig0,
	 ndf0, pconf, azwt, damp, maxit, prtflg, fxdflg, outfile, luout, alat,
	 alon, zfoc, torg, sighat, snssd, ndf, epmaj, epmin, epstr, zfint, 
	toint, sxx, syy, szz, stt, sxy, sxz, syz, stx, sty, stz, stadel, 
	staazi, stabaz, epimp, zfimp, resid, ipsta, iderr, niter, ierr, 
	dstaid_len, dwavid_len, dtype_len, atype_len, staid_len, wavid_len, 
	prtflg_len, fxdflg_len, outfile_len)
char *dstaid, *dwavid, *dtype, *atype;
real *dobs, *dsd;
integer *idarid, *ndata;
char *staid;
real *stalat, *stalon, *stelev, *stacor;
integer *nsta;
char *wavid;
integer *nwav, *maxtbd, *maxtbz, *ntbd, *ntbz;
real *tbd, *tbz, *tbtt, *alat0, *alon0, *zfoc0, *sig0;
integer *ndf0;
real *pconf, *azwt, *damp;
integer *maxit;
char *prtflg, *fxdflg, *outfile;
integer *luout;
real *alat, *alon, *zfoc, *torg, *sighat, *snssd;
integer *ndf;
real *epmaj, *epmin, *epstr, *zfint, *toint, *sxx, *syy, *szz, *stt, *sxy, *
	sxz, *syz, *stx, *sty, *stz, *stadel, *staazi, *stabaz, *epimp, *
	zfimp, *resid;
integer *ipsta, *iderr, *niter, *ierr;
ftnlen dstaid_len;
ftnlen dwavid_len;
ftnlen dtype_len;
ftnlen atype_len;
ftnlen staid_len;
ftnlen wavid_len;
ftnlen prtflg_len;
ftnlen fxdflg_len;
ftnlen outfile_len;
{
    /* System generated locals */
    integer tbd_dim1, tbd_offset, tbtt_dim1, tbtt_dim2, tbtt_offset, tbz_dim1,
	     tbz_offset, i__1, i__2;
    real r__1, r__2, r__3, r__4;
    doublereal d__1, d__2;
    olist o__1;
    cllist cl__1;

    /* Builtin functions */
    integer s_wsfe(), do_fio(), e_wsfe(), s_cmp();
    /* Subroutine */ int s_copy();
    integer i_len(), f_open(), s_wsle(), do_lio(), e_wsle();
    double sqrt(), cos(), pow_dd();
    integer f_clos();

    /* Local variables */
    static integer igap, iday;
    static doublereal rank;
    static integer imin, icnt;
    extern /* Subroutine */ int etoh_();
    static doublereal azim[9999];
    static integer indx[9999], idoy;
    static doublereal dist[9999];
    static logical lprt[19];
    static integer indx2[50], i__, j, m, n;
    static char mname[3];
    static integer isave;
    static logical opfil;
    static integer itdex[50], len_phase__;
    static doublereal torgd;
    static integer ipwav[9999], idtyp[9999], k2;
    static doublereal slovecres;
    static integer nd;
    extern /* Subroutine */ int check_data__();
    static char ew[2], ns[2];
    extern integer lnblnk_();
    static doublereal slodel;
    static integer itimes;
    static real sec;
    extern /* Subroutine */ int hypcut0_(), hypinv0_();
    static integer ihr;
    static doublereal obs[50];
    static integer imo, y1970, ios, iyr, len_sta__;
    static doublereal timeref, dsdnorm;
    extern /* Subroutine */ int index_array__();

    /* Fortran I/O blocks */
    static cilist io___1 = { 0, 6, 0, "(/a,i2)", 0 };
    static cilist io___11 = { 0, 6, 0, "(3a)", 0 };
    static cilist io___22 = { 0, 0, 0, "(/a/2a//a,2(i2,a),f5.2,3a,i3,a,i5,//\
2a/)", 0 };
    static cilist io___24 = { 0, 0, 0, "(a6,3(f10.4))", 0 };
    static cilist io___25 = { 0, 0, 0, "(/2a)", 0 };
    static cilist io___26 = { 0, 0, 0, "(i8,1x,a6,1x,a8,1x,a4,1x,a1,4x,f10.3\
,f8.3,i4)", 0 };
    static cilist io___27 = { 0, 0, 0, "(2a)", 0 };
    static cilist io___28 = { 0, 0, 0, 0, 0 };
    static cilist io___32 = { 0, 0, 0, "(/a,i3,a)", 0 };
    static cilist io___33 = { 0, 0, 0, "(/a,i3,a)", 0 };
    static cilist io___34 = { 0, 0, 0, "(/a,i3,a)", 0 };
    static cilist io___35 = { 0, 0, 0, "(/a,i3)", 0 };
    static cilist io___36 = { 0, 0, 0, "(2a/a)", 0 };
    static cilist io___37 = { 0, 0, 0, "(/2a//)", 0 };
    static cilist io___40 = { 0, 0, 0, "(a/a,f9.3,2a/a,f9.3,2a)", 0 };
    static cilist io___41 = { 0, 0, 0, "(a,2(/a,f9.3,3a,f9.3,a))", 0 };
    static cilist io___42 = { 0, 0, 0, "(9x,a,f9.3,a)", 0 };
    static cilist io___43 = { 0, 0, 0, "(9x,a,f9.3,a,f9.3,a)", 0 };
    static cilist io___44 = { 0, 0, 0, "(a,f9.3,a,f9.3,a)", 0 };
    static cilist io___45 = { 0, 0, 0, "(a,f9.3,a/a,f9.3,a)", 0 };
    static cilist io___46 = { 0, 0, 0, "(a,f9.3,a,f9.3,a/a,f9.3,a,f9.3,a)", 0 
	    };
    static cilist io___47 = { 0, 0, 0, "(/a,f4.2,a,2(/a,f8.1,a,f6.2,a))", 0 };
    static cilist io___48 = { 0, 0, 0, "(a,f8.1,a)", 0 };
    static cilist io___49 = { 0, 0, 0, "(a,f8.1,a)", 0 };
    static cilist io___50 = { 0, 0, 0, "(a,f8.1,a//a/,2(a,f6.2,a,i6,a,/),a,f\
6.2,a)", 0 };
    static cilist io___51 = { 0, 0, 0, "(2(/a,f5.2),/a,i4,a)", 0 };
    static cilist io___52 = { 0, 0, 0, "(8x,a,f12.8)", 0 };
    static cilist io___65 = { 0, 0, 0, "(4(/2a))", 0 };
    static cilist io___68 = { 0, 0, 0, "(6x,a,f8.3)", 0 };
    static cilist io___70 = { 0, 0, 0, "(i8,1x,a6,1x,a7,a4,2x,a1,f9.3,f11.3,\
f9.3,                          f8.2,f8.3,i4)", 0 };
    static cilist io___71 = { 0, 0, 0, "(2a/)", 0 };
    static cilist io___72 = { 0, 0, 0, 0, 0 };
    static cilist io___73 = { 0, 0, 0, 0, 0 };
    static cilist io___74 = { 0, 0, 0, 0, 0 };
    static cilist io___75 = { 0, 0, 0, 0, 0 };
    static cilist io___76 = { 0, 0, 0, 0, 0 };
    static cilist io___77 = { 0, 0, 0, 0, 0 };
    static cilist io___78 = { 0, 0, 0, 0, 0 };
    static cilist io___79 = { 0, 0, 0, 0, 0 };
    static cilist io___80 = { 0, 0, 0, 0, 0 };
    static cilist io___81 = { 0, 0, 0, 0, 0 };
    static cilist io___82 = { 0, 0, 0, 0, 0 };
    static cilist io___83 = { 0, 0, 0, 0, 0 };
    static cilist io___84 = { 0, 0, 0, 0, 0 };
    static cilist io___85 = { 0, 0, 0, 0, 0 };
    static cilist io___86 = { 0, 0, 0, "(/a/a)", 0 };


/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declarations ---- */
/*     Radius of the earth (km.) */
/*     Convert kilometers to degrees */
/*     Convert degrees to radians */
/*     Maximum number of data allowed for any single location */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     call ieee handler */
/*     call handle */
    /* Parameter adjustments */
    --iderr;
    --ipsta;
    --resid;
    --zfimp;
    --epimp;
    --idarid;
    --dsd;
    --dobs;
    atype -= atype_len;
    dtype -= dtype_len;
    dwavid -= dwavid_len;
    dstaid -= dstaid_len;
    --stabaz;
    --staazi;
    --stadel;
    --stacor;
    --stelev;
    --stalon;
    --stalat;
    staid -= staid_len;
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
    if (*ndata < 3) {
	s_wsfe(&io___1);
	do_fio(&c__1, " Insufficient data for a solution: # of Data = ", (
		ftnlen)47);
	do_fio(&c__1, (char *)&nd, (ftnlen)sizeof(integer));
	e_wsfe();
	*ierr = 5;
	return 0;
    }
    if (s_cmp(prtflg, "Y", prtflg_len, (ftnlen)1) == 0) {
	s_copy(prtflg, "y", prtflg_len, (ftnlen)1);
    }
/*     Check for valid data and load station, wave and data type pointers */
    len_sta__ = i_len(staid + staid_len, staid_len);
    len_phase__ = i_len(wavid + wavid_len, wavid_len);
    check_data__(dstaid + dstaid_len, dwavid + dwavid_len, dtype + dtype_len, 
	    &dsd[1], ndata, staid + staid_len, nsta, wavid + wavid_len, nwav, 
	    &len_sta__, &len_phase__, &ipsta[1], ipwav, idtyp, &iderr[1], 
	    dstaid_len, dwavid_len, dtype_len, staid_len, wavid_len);
/*     Check that each travel-time curve contains valid data.  For arrival */
/*     time or slowness data with empty curves, set iderr = 2 */
    i__1 = *ndata;
    for (n = 1; n <= i__1; ++n) {
/* L1000: */
	if ((idtyp[n - 1] == 1 || idtyp[n - 1] == 3) && (ntbd[ipwav[n - 1]] <=
		 0 || ntbz[ipwav[n - 1]] <= 0)) {
	    iderr[n] = 2;
	}
    }
/*     Open main output file, unless only screen output is desired */
    opfil = FALSE_;
    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0 && (*luout != 0 && *
	    luout != 6)) {
	o__1.oerr = 1;
	o__1.ounit = *luout;
	o__1.ofnmlen = outfile_len;
	o__1.ofnm = outfile;
	o__1.orl = 0;
	o__1.osta = 0;
	o__1.oacc = "append";
	o__1.ofm = 0;
	o__1.oblnk = 0;
	ios = f_open(&o__1);
	if (ios != 0) {
	    k2 = lnblnk_(outfile, outfile_len);
	    s_wsfe(&io___11);
	    do_fio(&c__1, " Output file ", (ftnlen)13);
	    do_fio(&c__1, outfile, k2);
	    do_fio(&c__1, " will not open", (ftnlen)14);
	    e_wsfe();
	    s_copy(prtflg, "n", prtflg_len, (ftnlen)1);
	    *luout = 6;
	} else {
	    opfil = TRUE_;
	}
    }
/*     Print stations and observations */
    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	y1970 = 1970;
	etoh_(&timeref, &y1970, &iyr, &imo, mname, &iday, &idoy, &ihr, &imin, 
		&sec, (ftnlen)3);
	io___22.ciunit = *luout;
	s_wsfe(&io___22);
	do_fio(&c__1, " LocSAT LOCATION RESULTS", (ftnlen)24);
	do_fio(&c__1, " =======================================", (ftnlen)40);
	do_fio(&c__1, "======================================= ", (ftnlen)40);
	do_fio(&c__1, "  First arrival detected at: ", (ftnlen)29);
	do_fio(&c__1, (char *)&ihr, (ftnlen)sizeof(integer));
	do_fio(&c__1, ":", (ftnlen)1);
	do_fio(&c__1, (char *)&imin, (ftnlen)sizeof(integer));
	do_fio(&c__1, ":", (ftnlen)1);
	do_fio(&c__1, (char *)&sec, (ftnlen)sizeof(real));
	do_fio(&c__1, " (GMT) on: ", (ftnlen)11);
	do_fio(&c__1, mname, (ftnlen)3);
	do_fio(&c__1, ".", (ftnlen)1);
	do_fio(&c__1, (char *)&iday, (ftnlen)sizeof(integer));
	do_fio(&c__1, ",", (ftnlen)1);
	do_fio(&c__1, (char *)&iyr, (ftnlen)sizeof(integer));
	do_fio(&c__1, " =======================================", (ftnlen)40);
	do_fio(&c__1, "======================================= ", (ftnlen)40);
	e_wsfe();
	i__1 = *nsta;
	for (i__ = 1; i__ <= i__1; ++i__) {
/* L1010: */
	    io___24.ciunit = *luout;
	    s_wsfe(&io___24);
	    do_fio(&c__1, staid + i__ * staid_len, staid_len);
	    do_fio(&c__1, (char *)&stalat[i__], (ftnlen)sizeof(real));
	    do_fio(&c__1, (char *)&stalon[i__], (ftnlen)sizeof(real));
	    do_fio(&c__1, (char *)&stelev[i__], (ftnlen)sizeof(real));
	    e_wsfe();
	}
	io___25.ciunit = *luout;
	s_wsfe(&io___25);
	do_fio(&c__1, " Ariv ID Statn  Phase    Type Atype", (ftnlen)35);
	do_fio(&c__1, "  Observed    S.D. Err", (ftnlen)22);
	e_wsfe();
	i__1 = *ndata;
	for (n = 1; n <= i__1; ++n) {
/* L1020: */
	    io___26.ciunit = *luout;
	    s_wsfe(&io___26);
	    do_fio(&c__1, (char *)&idarid[n], (ftnlen)sizeof(integer));
	    do_fio(&c__1, dstaid + n * dstaid_len, dstaid_len);
	    do_fio(&c__1, dwavid + n * dwavid_len, dwavid_len);
	    do_fio(&c__1, dtype + n * dtype_len, dtype_len);
	    do_fio(&c__1, atype + n * atype_len, atype_len);
	    do_fio(&c__1, (char *)&dobs[n], (ftnlen)sizeof(real));
	    do_fio(&c__1, (char *)&dsd[n], (ftnlen)sizeof(real));
	    do_fio(&c__1, (char *)&iderr[n], (ftnlen)sizeof(integer));
	    e_wsfe();
	}
	io___27.ciunit = *luout;
	s_wsfe(&io___27);
	do_fio(&c__1, " =======================================", (ftnlen)40);
	do_fio(&c__1, "======================================= ", (ftnlen)40);
	e_wsfe();
    }
/*     Compute initial first-cut guess location */
    if (dabs(*alat0) > (float)90. || dabs(*alon0) > (float)180.) {
	hypcut0_(staid + staid_len, &stalat[1], &stalon[1], nsta, &dobs[1], &
		dsd[1], dwavid + dwavid_len, &ipsta[1], ipwav, nwav, maxtbd, 
		maxtbz, &ntbd[1], &tbd[tbd_offset], &tbtt[tbtt_offset], idtyp,
		 &iderr[1], atype + atype_len, ndata, luout, prtflg, alat0, 
		alon0, ierr, staid_len, dwavid_len, atype_len, prtflg_len);
	if (*ierr > 0) {
	    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
		io___28.ciunit = *luout;
		s_wsle(&io___28);
		do_lio(&c__9, &c__1, "? LocSAT: Too few data to get an initi\
al location", (ftnlen)49);
		e_wsle();
	    }
	    goto L1040;
	}
    }
    *sighat = (float)-1.;
    *zfint = (float)-1.;
    *toint = (float)-1.;
    i__1 = *ndata;
    for (i__ = 1; i__ <= i__1; ++i__) {
	epimp[i__] = (float)-1.;
	zfimp[i__] = (float)-1.;
/* L1030: */
    }
    hypinv0_(dstaid + dstaid_len, dwavid + dwavid_len, dtype + dtype_len, 
	    atype + atype_len, &dobs[1], &dsd[1], ndata, &stalat[1], &stalon[
	    1], nsta, maxtbd, maxtbz, &ntbd[1], &ntbz[1], &tbd[tbd_offset], &
	    tbz[tbz_offset], &tbtt[tbtt_offset], &ipsta[1], ipwav, idtyp, &
	    iderr[1], alat0, alon0, zfoc0, sig0, ndf0, pconf, &c_b80, azwt, 
	    damp, maxit, prtflg, fxdflg, luout, alat, alon, zfoc, torg, 
	    sighat, snssd, ndf, epmaj, epmin, epstr, zfint, toint, sxx, syy, 
	    szz, stt, sxy, sxz, syz, stx, sty, stz, &stadel[1], &staazi[1], &
	    stabaz[1], &epimp[1], &rank, &resid[1], &igap, niter, &nd, ierr, 
	    dstaid_len, dwavid_len, dtype_len, atype_len, prtflg_len, 
	    fxdflg_len);
    torgd = timeref + *torg;
/*     Print location results, if requested */
L1040:
    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	if (*ierr == 0) {
	    io___32.ciunit = *luout;
	    s_wsfe(&io___32);
	    do_fio(&c__1, " Location ran for", (ftnlen)17);
	    do_fio(&c__1, (char *)&(*niter), (ftnlen)sizeof(integer));
	    do_fio(&c__1, " iterations ... Converged!", (ftnlen)26);
	    e_wsfe();
	} else if (*ierr == 1) {
	    io___33.ciunit = *luout;
	    s_wsfe(&io___33);
	    do_fio(&c__1, " Location ran for", (ftnlen)17);
	    do_fio(&c__1, (char *)&(*niter), (ftnlen)sizeof(integer));
	    do_fio(&c__1, " iterations ... Exhausted!", (ftnlen)26);
	    e_wsfe();
	} else if (*ierr == 2) {
	    io___34.ciunit = *luout;
	    s_wsfe(&io___34);
	    do_fio(&c__1, " Location ran for", (ftnlen)17);
	    do_fio(&c__1, (char *)&(*niter), (ftnlen)sizeof(integer));
	    do_fio(&c__1, " iterations ... Diverged!", (ftnlen)25);
	    e_wsfe();
	} else if (*ierr == 5) {
	    io___35.ciunit = *luout;
	    s_wsfe(&io___35);
	    do_fio(&c__1, " Insufficient data for a solution: # of Data = ", (
		    ftnlen)47);
	    do_fio(&c__1, (char *)&nd, (ftnlen)sizeof(integer));
	    e_wsfe();
	    goto L1120;
	} else if (*ierr == 6) {
	    io___36.ciunit = *luout;
	    s_wsfe(&io___36);
	    do_fio(&c__1, " SVD routine cannot invert given matrix --", (
		    ftnlen)42);
	    do_fio(&c__1, " No singular values found", (ftnlen)25);
	    do_fio(&c__1, " Returning !!", (ftnlen)13);
	    e_wsfe();
	    goto L1120;
	}
	io___37.ciunit = *luout;
	s_wsfe(&io___37);
	do_fio(&c__1, " =======================================", (ftnlen)40);
	do_fio(&c__1, "======================================= ", (ftnlen)40);
	e_wsfe();
	s_copy(ew, " E", (ftnlen)2, (ftnlen)2);
	s_copy(ns, " N", (ftnlen)2, (ftnlen)2);
	if (*alon < (float)0.) {
	    s_copy(ew, " W", (ftnlen)2, (ftnlen)2);
	}
	if (*alat < (float)0.) {
	    s_copy(ns, " S", (ftnlen)2, (ftnlen)2);
	}
	if (*sxx < (float)0. || *syy < (float)0.) {
	    io___40.ciunit = *luout;
	    s_wsfe(&io___40);
	    do_fio(&c__1, " Final location estimate:", (ftnlen)25);
	    do_fio(&c__1, "      Latitude:", (ftnlen)15);
	    r__1 = dabs(*alat);
	    do_fio(&c__1, (char *)&r__1, (ftnlen)sizeof(real));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, ns, (ftnlen)2);
	    do_fio(&c__1, "     Longitude:", (ftnlen)15);
	    r__2 = dabs(*alon);
	    do_fio(&c__1, (char *)&r__2, (ftnlen)sizeof(real));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, ew, (ftnlen)2);
	    e_wsfe();
	} else {
	    io___41.ciunit = *luout;
	    s_wsfe(&io___41);
	    do_fio(&c__1, " Final location estimate (+/- S.D.):", (ftnlen)36);
	    do_fio(&c__1, "      Latitude:", (ftnlen)15);
	    r__1 = dabs(*alat);
	    do_fio(&c__1, (char *)&r__1, (ftnlen)sizeof(real));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, ns, (ftnlen)2);
	    do_fio(&c__1, " +/- ", (ftnlen)5);
	    r__2 = sqrt(*syy);
	    do_fio(&c__1, (char *)&r__2, (ftnlen)sizeof(real));
	    do_fio(&c__1, " km.", (ftnlen)4);
	    do_fio(&c__1, "     Longitude:", (ftnlen)15);
	    r__3 = dabs(*alon);
	    do_fio(&c__1, (char *)&r__3, (ftnlen)sizeof(real));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, ew, (ftnlen)2);
	    do_fio(&c__1, " +/- ", (ftnlen)5);
	    r__4 = sqrt(*sxx);
	    do_fio(&c__1, (char *)&r__4, (ftnlen)sizeof(real));
	    do_fio(&c__1, " km.", (ftnlen)4);
	    e_wsfe();
	}
	if (s_cmp(fxdflg, "n", fxdflg_len, (ftnlen)1) == 0) {
	    if (*szz < (float)0.) {
		io___42.ciunit = *luout;
		s_wsfe(&io___42);
		do_fio(&c__1, "Depth:", (ftnlen)6);
		do_fio(&c__1, (char *)&(*zfoc), (ftnlen)sizeof(real));
		do_fio(&c__1, " km.", (ftnlen)4);
		e_wsfe();
	    } else {
		io___43.ciunit = *luout;
		s_wsfe(&io___43);
		do_fio(&c__1, "Depth:", (ftnlen)6);
		do_fio(&c__1, (char *)&(*zfoc), (ftnlen)sizeof(real));
		do_fio(&c__1, "  km.   +/- ", (ftnlen)12);
		r__1 = sqrt(*szz);
		do_fio(&c__1, (char *)&r__1, (ftnlen)sizeof(real));
		do_fio(&c__1, " km.", (ftnlen)4);
		e_wsfe();
	    }
	} else {
	    io___44.ciunit = *luout;
	    s_wsfe(&io___44);
	    do_fio(&c__1, "         Depth:", (ftnlen)15);
	    do_fio(&c__1, (char *)&(*zfoc), (ftnlen)sizeof(real));
	    do_fio(&c__1, "  km.   +/- ", (ftnlen)12);
	    do_fio(&c__1, (char *)&c_b183, (ftnlen)sizeof(real));
	    do_fio(&c__1, " km. (Fixed)", (ftnlen)12);
	    e_wsfe();
	}
	if (*stt < (float)0.) {
	    io___45.ciunit = *luout;
	    s_wsfe(&io___45);
	    do_fio(&c__1, " Relative O.T.:", (ftnlen)15);
	    do_fio(&c__1, (char *)&(*torg), (ftnlen)sizeof(real));
	    do_fio(&c__1, " sec.", (ftnlen)5);
	    do_fio(&c__1, " Absolute O.T.:", (ftnlen)15);
	    do_fio(&c__1, (char *)&torgd, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " sec.", (ftnlen)5);
	    e_wsfe();
	} else {
	    io___46.ciunit = *luout;
	    s_wsfe(&io___46);
	    do_fio(&c__1, " Relative O.T.:", (ftnlen)15);
	    do_fio(&c__1, (char *)&(*torg), (ftnlen)sizeof(real));
	    do_fio(&c__1, " sec.   +/- ", (ftnlen)12);
	    r__1 = sqrt(*stt);
	    do_fio(&c__1, (char *)&r__1, (ftnlen)sizeof(real));
	    do_fio(&c__1, " sec.", (ftnlen)5);
	    do_fio(&c__1, " Absolute O.T.:", (ftnlen)15);
	    do_fio(&c__1, (char *)&torgd, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " sec.   +/- ", (ftnlen)12);
	    r__2 = sqrt(*stt);
	    do_fio(&c__1, (char *)&r__2, (ftnlen)sizeof(real));
	    do_fio(&c__1, " sec.", (ftnlen)5);
	    e_wsfe();
	}
	y1970 = 1970;
	etoh_(&torgd, &y1970, &iyr, &imo, mname, &iday, &idoy, &ihr, &imin, &
		sec, (ftnlen)3);
/*        write (luout, '(14x,a,i5,3i3,a,i2,a,f5.2)') */
/*    &          ':', iyr, imo, iday, ihr, ':', imin, ':', sec */
	io___47.ciunit = *luout;
	s_wsfe(&io___47);
	do_fio(&c__1, " Confidence region at ", (ftnlen)22);
	do_fio(&c__1, (char *)&(*pconf), (ftnlen)sizeof(real));
	do_fio(&c__1, " level:", (ftnlen)7);
	do_fio(&c__1, "   Semi-major axis:", (ftnlen)19);
	do_fio(&c__1, (char *)&(*epmaj), (ftnlen)sizeof(real));
	do_fio(&c__1, "  km. =", (ftnlen)7);
	r__1 = *epmaj * (float).00899322;
	do_fio(&c__1, (char *)&r__1, (ftnlen)sizeof(real));
	do_fio(&c__1, " deg.", (ftnlen)5);
	do_fio(&c__1, "   Semi-minor axis:", (ftnlen)19);
	do_fio(&c__1, (char *)&(*epmin), (ftnlen)sizeof(real));
	do_fio(&c__1, "  km. =", (ftnlen)7);
	r__2 = *epmin * (float).00899322;
	do_fio(&c__1, (char *)&r__2, (ftnlen)sizeof(real));
	do_fio(&c__1, " deg.", (ftnlen)5);
	e_wsfe();
	io___48.ciunit = *luout;
	s_wsfe(&io___48);
	do_fio(&c__1, " Major-axis strike:", (ftnlen)19);
	do_fio(&c__1, (char *)&(*epstr), (ftnlen)sizeof(real));
	do_fio(&c__1, " deg. clockwise from North", (ftnlen)26);
	e_wsfe();
	if (s_cmp(fxdflg, "n", fxdflg_len, (ftnlen)1) == 0) {
	    io___49.ciunit = *luout;
	    s_wsfe(&io___49);
	    do_fio(&c__1, "       Depth error:", (ftnlen)19);
	    do_fio(&c__1, (char *)&(*zfint), (ftnlen)sizeof(real));
	    do_fio(&c__1, "  km.", (ftnlen)5);
	    e_wsfe();
	}
	io___50.ciunit = *luout;
	s_wsfe(&io___50);
	do_fio(&c__1, "  Orig. time error:", (ftnlen)19);
	do_fio(&c__1, (char *)&(*toint), (ftnlen)sizeof(real));
	do_fio(&c__1, " sec.", (ftnlen)5);
	do_fio(&c__1, " Standard errors (sigma):", (ftnlen)25);
	do_fio(&c__1, "              Prior:", (ftnlen)20);
	do_fio(&c__1, (char *)&(*sig0), (ftnlen)sizeof(real));
	do_fio(&c__1, " (", (ftnlen)2);
	do_fio(&c__1, (char *)&(*ndf0), (ftnlen)sizeof(integer));
	do_fio(&c__1, " deg. of freedom)", (ftnlen)17);
	do_fio(&c__1, "          Posterior:", (ftnlen)20);
	do_fio(&c__1, (char *)&(*sighat), (ftnlen)sizeof(real));
	do_fio(&c__1, " (", (ftnlen)2);
	do_fio(&c__1, (char *)&(*ndf), (ftnlen)sizeof(integer));
	do_fio(&c__1, " deg. of freedom)", (ftnlen)17);
	do_fio(&c__1, "          Posterior:", (ftnlen)20);
	do_fio(&c__1, (char *)&(*snssd), (ftnlen)sizeof(real));
	do_fio(&c__1, " (Normalized sample S.D.)", (ftnlen)25);
	e_wsfe();
	io___51.ciunit = *luout;
	s_wsfe(&io___51);
	do_fio(&c__1, "      Azimuthal weighting: ", (ftnlen)27);
	do_fio(&c__1, (char *)&(*azwt), (ftnlen)sizeof(real));
	do_fio(&c__1, " Effective rank of matrix: ", (ftnlen)27);
	do_fio(&c__1, (char *)&rank, (ftnlen)sizeof(doublereal));
	do_fio(&c__1, "    Maximum azimuthal GAP: ", (ftnlen)27);
	do_fio(&c__1, (char *)&igap, (ftnlen)sizeof(integer));
	do_fio(&c__1, " deg.", (ftnlen)5);
	e_wsfe();
	if (*damp >= (float)0.) {
	    io___52.ciunit = *luout;
	    s_wsfe(&io___52);
	    do_fio(&c__1, " Percent damping: ", (ftnlen)18);
	    do_fio(&c__1, (char *)&(*damp), (ftnlen)sizeof(real));
	    e_wsfe();
/*        else */
/*           write (luout, '(a)') '  - No damping required !' */
	}
	for (i__ = 1; i__ <= 19; ++i__) {
/* L1050: */
	    lprt[i__ - 1] = FALSE_;
	}
/*        Sort data by distance, then by time */
	i__1 = *ndata;
	for (n = 1; n <= i__1; ++n) {
	    i__ = ipsta[n];
	    dist[n - 1] = stadel[i__];
	    azim[n - 1] = staazi[i__];
	    stadel[i__] += (float)1e-4;
/* L1060: */
	}
	index_array__(ndata, dist, indx);
	isave = 0;
	icnt = 1;
	i__1 = *ndata;
	for (n = 1; n <= i__1; ++n) {
	    j = indx[n - 1];
	    i__ = ipsta[j];
	    if (i__ != isave) {
		if (icnt > 1 && itimes > 1) {
		    index_array__(&icnt, obs, indx2);
		    i__2 = icnt;
		    for (m = 1; m <= i__2; ++m) {
/* L1070: */
			indx[n - (icnt - m) - 2] = itdex[indx2[m - 1] - 1];
		    }
		}
		itimes = 0;
		icnt = 1;
		itdex[0] = j;
		obs[0] = dobs[j];
	    } else {
		++icnt;
		itdex[icnt - 1] = j;
		if (*(unsigned char *)&dtype[j * dtype_len] == 't') {
		    obs[icnt - 1] = dobs[j];
		    ++itimes;
		} else if (*(unsigned char *)&dtype[j * dtype_len] == 'a') {
		    obs[icnt - 1] = dobs[j - 1] + (float)1e-4;
		} else {
		    obs[icnt - 1] = dobs[j - 2] + (float)2e-4;
		}
	    }
	    isave = i__;
/* L1080: */
	}
	io___65.ciunit = *luout;
	s_wsfe(&io___65);
	do_fio(&c__1, " =======================================", (ftnlen)40);
	do_fio(&c__1, "======================================= ", (ftnlen)40);
	do_fio(&c__1, "                       Data           Residuals    ", (
		ftnlen)51);
	do_fio(&c__1, "Distance Azimuth   Data", (ftnlen)23);
	do_fio(&c__1, " Ariv ID Statn  Phase  Type at     True Normalized ", (
		ftnlen)51);
	do_fio(&c__1, "  (deg.)  (deg.)  Import Err", (ftnlen)28);
	do_fio(&c__1, " =======================================", (ftnlen)40);
	do_fio(&c__1, "======================================= ", (ftnlen)40);
	e_wsfe();
	i__1 = *ndata;
	for (n = 1; n <= i__1; ++n) {
	    j = indx[n - 1];
/*           Kludge for the Center */
	    if (*(unsigned char *)&dtype[j * dtype_len] == 's') {
		j = indx[n - 2];
	    } else if (*(unsigned char *)&dtype[j * dtype_len] == 'a') {
		j = indx[n];
	    }
/*           Calculate the residual of vector slowness */
	    if (*(unsigned char *)&dtype[j * dtype_len] == 's') {
		slodel = dobs[j] - resid[j];
/* Computing 2nd power */
		r__1 = dobs[j];
/* Computing 2nd power */
		d__2 = slodel;
		d__1 = r__1 * r__1 + d__2 * d__2 - dobs[j] * 2 * slodel * cos(
			resid[j - 1] * .017453293);
		slovecres = pow_dd(&d__1, &c_b322);
		io___68.ciunit = *luout;
		s_wsfe(&io___68);
		do_fio(&c__1, "Slowness Vector Residual:", (ftnlen)25);
		do_fio(&c__1, (char *)&slovecres, (ftnlen)sizeof(doublereal));
		e_wsfe();
	    }
/*           Calculate normalized residual */
	    if (resid[j] != (float)-999.) {
		dsdnorm = resid[j] / dsd[j];
	    } else {
		dsdnorm = (float)-999.;
	    }
	    io___70.ciunit = *luout;
	    s_wsfe(&io___70);
	    do_fio(&c__1, (char *)&idarid[j], (ftnlen)sizeof(integer));
	    do_fio(&c__1, dstaid + j * dstaid_len, dstaid_len);
	    do_fio(&c__1, dwavid + j * dwavid_len, dwavid_len);
	    do_fio(&c__1, dtype + j * dtype_len, dtype_len);
	    do_fio(&c__1, atype + j * atype_len, atype_len);
	    do_fio(&c__1, (char *)&resid[j], (ftnlen)sizeof(real));
	    do_fio(&c__1, (char *)&dsdnorm, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, (char *)&dist[j - 1], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, (char *)&azim[j - 1], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, (char *)&epimp[j], (ftnlen)sizeof(real));
	    do_fio(&c__1, (char *)&iderr[j], (ftnlen)sizeof(integer));
	    e_wsfe();
	    if (iderr[n] > 0) {
		lprt[iderr[n] - 1] = TRUE_;
	    }
/* L1090: */
	}
/*        ---- Open new location file ---- */
/*        open (32, file = 'new.loc', access = 'append', iostat = ios) */
/*        if (ios.ne.0) then */
/*           write (6, '(a)') ' Output file new.loc will not open' */
/*        else */
/*           Right out new location and phase data to file, "new.loc" */
/*           iyr = iyr - 1900 */
/*           write (32, '(3i2,1x,2i2,f6.2,f9.4,f10.4,f7.2,f7.2,i3,i4,)') */
/*    &             iyr, imo, iday, ihr, imin, sec, alat, alon, zfoc, */
/*    &             0.0, ndata, igap */
/*           do 1100  n = 1, ndata */
/*              j = indx(n) */
/*              write (32, '(i8,1x,a6,1x,a8,1x,a4,1x,a1,4x,f10.3, */
/*    &                      f8.3,i4)') idarid(j), dstaid(j), dwavid(j), */
/*    &                                 dtype(j), atype(j), dobs(j), */
/*    &                                 dsd(j), iderr(j) */
/* 1100       continue */
/*           write (32, *) ' ' */
/*           close (32) */
/*        end if */
	io___71.ciunit = *luout;
	s_wsfe(&io___71);
	do_fio(&c__1, " =======================================", (ftnlen)40);
	do_fio(&c__1, "======================================= ", (ftnlen)40);
	e_wsfe();
	io___72.ciunit = *luout;
	s_wsle(&io___72);
	do_lio(&c__9, &c__1, " =  0, No problem, normal interpolation", (
		ftnlen)39);
	e_wsle();
	if (lprt[0]) {
	    io___73.ciunit = *luout;
	    s_wsle(&io___73);
	    do_lio(&c__9, &c__1, " =  1, No station information", (ftnlen)29);
	    e_wsle();
	}
	if (lprt[1]) {
	    io___74.ciunit = *luout;
	    s_wsle(&io___74);
	    do_lio(&c__9, &c__1, " =  2, No travel-time tables", (ftnlen)28);
	    e_wsle();
	}
	if (lprt[2]) {
	    io___75.ciunit = *luout;
	    s_wsle(&io___75);
	    do_lio(&c__9, &c__1, " =  3, Data type unknown", (ftnlen)24);
	    e_wsle();
	}
	if (lprt[3]) {
	    io___76.ciunit = *luout;
	    s_wsle(&io___76);
	    do_lio(&c__9, &c__1, " =  4, S.D. <= 0.0", (ftnlen)18);
	    e_wsle();
	}
	if (lprt[10]) {
	    io___77.ciunit = *luout;
	    s_wsle(&io___77);
	    do_lio(&c__9, &c__1, " = 11, Distance-depth point ", (ftnlen)28);
	    do_lio(&c__9, &c__1, "(x0,z0) in hole of travel-time curve", (
		    ftnlen)36);
	    e_wsle();
	}
	if (lprt[11]) {
	    io___78.ciunit = *luout;
	    s_wsle(&io___78);
	    do_lio(&c__9, &c__1, " = 12, x0 < x(1)", (ftnlen)16);
	    e_wsle();
	}
	if (lprt[12]) {
	    io___79.ciunit = *luout;
	    s_wsle(&io___79);
	    do_lio(&c__9, &c__1, " = 13, x0 > x(max)", (ftnlen)18);
	    e_wsle();
	}
	if (lprt[13]) {
	    io___80.ciunit = *luout;
	    s_wsle(&io___80);
	    do_lio(&c__9, &c__1, " = 14, z0 < z(1)", (ftnlen)16);
	    e_wsle();
	}
	if (lprt[14]) {
	    io___81.ciunit = *luout;
	    s_wsle(&io___81);
	    do_lio(&c__9, &c__1, " = 15, z0 > z(max)", (ftnlen)18);
	    e_wsle();
	}
	if (lprt[15]) {
	    io___82.ciunit = *luout;
	    s_wsle(&io___82);
	    do_lio(&c__9, &c__1, " = 16, x0 < x(1) & z0 < z(1)", (ftnlen)28);
	    e_wsle();
	}
	if (lprt[16]) {
	    io___83.ciunit = *luout;
	    s_wsle(&io___83);
	    do_lio(&c__9, &c__1, " = 17, x0 > x(max) & z0 < z(1)", (ftnlen)30)
		    ;
	    e_wsle();
	}
	if (lprt[17]) {
	    io___84.ciunit = *luout;
	    s_wsle(&io___84);
	    do_lio(&c__9, &c__1, " = 18, x0 < x(1) & z0 > z(max)", (ftnlen)30)
		    ;
	    e_wsle();
	}
	if (lprt[18]) {
	    io___85.ciunit = *luout;
	    s_wsle(&io___85);
	    do_lio(&c__9, &c__1, " = 19, x0 > x(max) & z0 > z(max)", (ftnlen)
		    32);
	    e_wsle();
	}
	for (i__ = 11; i__ <= 19; ++i__) {
	    if (lprt[i__ - 1]) {
		io___86.ciunit = *luout;
		s_wsfe(&io___86);
		do_fio(&c__1, " NOTE: If any of these codes are negative , t\
hen, ", (ftnlen)50);
		do_fio(&c__1, "       use the datum to compute the event loc\
ation!", (ftnlen)51);
		e_wsfe();
		goto L1120;
	    }
/* L1110: */
	}
    }
L1120:
    if (opfil) {
	cl__1.cerr = 0;
	cl__1.cunit = *luout;
	cl__1.csta = 0;
	f_clos(&cl__1);
    }
    return 0;
} /* locsat0_ */

