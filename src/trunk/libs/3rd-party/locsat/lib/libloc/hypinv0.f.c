/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static int c__9 = 9;
static int c__1 = 1;
static int c_n1 = -1;
static int c__4 = 4;
static int c__3 = 3;
static int c__2 = 2;

/* NAME */
/* 	hypinv0 -- Compute a hypocentral location. */
/* FILE */
/* 	hypinv0.f */
/* SYNOPSIS */
/* 	Computes event locations, confidence bounds, residuals and */
/* 	importances using arrival time, azimuths and slowness */
/* 	measurements from stations at regional and teleseismic */
/* 	distances. */
/* DESCRIPTION */
/* 	Subroutine.  Hypocenter inversion (event location), done as an */
/* 	iterative non-linear least squares inversion of travel-time, */
/* 	azimuth and slowness data.  Modified by Steve Bratt (March 1990). */
/* 	Modified by Walter Nagy (July 1990). */
/*       modified by johannes schweitzer (ellipticity correction) (march 1991) */
/* 	---- Indexing ---- */
/* 	i = 1, nsta;	j = 1, nwav;	k = 1, ntbd(j);	m = 1, ntbz(j); */
/* 	n = 1, ndata; */
/* 	---- On entry ---- */
/* 	ndata:		Number of data */
/* 	nsta:		Number of stations in network */
/* 	nwav:		Number of phases in list */
/* 	maxtbd:		Maximum dimension of k'th position in tbd(), tbtt() */
/* 	maxtbz:		Maximum dimension of m'th position in tbz(), tbtt() */
/* 	dstaid(n):	Name of station for n'th datum */
/* 	dwavid(n):	Name of phase for n'th datum */
/* 	dtype(n):	Data type for n'th datum (time, azim, slow) */
/* 	atype(n):	Arrival usage */
/* 			  = d: Defining, used in location */
/* 			  = n: Could be defining, but not used in location */
/* 			  = a: Not to be used in location */
/* 	dobs(n):	Value of n'th datum (sec, deg, sec/deg) */
/* 	dsd(n): 	Standard deviation in value of n'th datum */
/* 	stalat(i):	Station latitude  for i'th station (deg) */
/* 	stalon(i):	Station longitude for i'th station (deg) */
/* 	ntbd(j):	Number of distance samples in travel-time tables */
/* 	ntbz(j):	Number of depth samples in travel-time tables */
/* 	tbd(k,j):	Angular distance (deg) */
/* 	tbz(m,j):	Depth (km) */
/* 	tbtt(k,m,j):	Travel-time (sec) */
/* 	ipsta(n):	Station index for n'th observation */
/* 	ipwav(n):	Wave index for n'th observation */
/* 	idtyp(n):	Type code for n'th observation */
/* 			  = 0, Data type unknown */
/* 			  = 1, Arrival time datum */
/* 			  = 2, Azimuth datum */
/* 			  = 3, Slowness datum */
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
/* 		[NOTE:	If any of these codes is .le. 0 (e.g. iderr = -17), */
/* 			the datum was used to compute event location] */
/* 	alat0:		Initial educated guess of event latitude (deg) */
/* 	alon0:		Initial educated guess of event longitude (deg) */
/* 	zfoc0:		Initial educated guess of event focal depth (km) */
/* 	sig0:		Prior estimate of data standard error */
/* 	ndf0:		Number of degrees of freedom in sig0 */
/* 	pconf:		Confidence probability for confidence regions */
/* 			(0.0 - 1.0) */
/* 			[WARNING: subr. fstatx_() accepts only 0.90 for now] */
/* 	radius:		Radius of the earth (km) */
/* 	azwt:		Weight applied to azimuth data and partials */
/* 			(default = 1.0) */
/* 	damp:		Percent damping relative to largest singular value, */
/* 			if < 0.0, only damp when condition number > million */
/* 	maxit:		Maximum number of iterations allowed in inversion */
/* 	prtflg:		= y, Verbose printout */
/* 			= n, None */
/* 	fxdflg:		Flag for constraining focal depth; */
/* 			  = n, Focal depth is a free parameter in inversion */
/* 			  = y, Focal depth is constrained to equal zfoc0 */
/* 	luout:		Output file logical unit number */
/* 	---- On return ---- */
/* 	alat:		Final estimate of event latitude (deg) */
/* 	alon:		Final estimate of event longitude (deg) */
/* 	zfoc:		Final estimate of event focal depth (km) */
/* 	torg:		Final estimate of event origin time (sec) */
/* 	sighat:		Final estimate of data standard error */
/* 	snssd:		Normalized sample standard deviation */
/* 	ndf:		Number of degrees of freedom in sighat */
/* 	epmaj:		Length of semi-major axis of confidence ellipse on */
/* 			epicenter (km) */
/* 	epmin:		Length of semi-minor axis of confidence ellipse on */
/* 			epicenter (km) */
/* 	epstr:		Strike of semi-major axis of confidence ellipse on */
/* 			epicenter (deg) */
/* 	zfint:		Length of confidence semi-interval on focal depth (km) */
/* 			= < 0.0, if fxdflg = 'y' or depth was fixed by program */
/* 			due to convergence problem */
/* 	toint:		Length of confidence semi-interval on origin time (sec) */
/* 	sxx:		(Parameter covariance diagonal element) */
/* 	syy:		(Parameter covariance diagonal element) */
/* 	szz:		(Parameter covariance diagonal element) */
/* 			= < 0.0, if fxdflg = 'y' or depth was fixed due to */
/* 			to a convergence problem */
/* 	stt:		(Parameter covariance diagonal element) */
/* 	sxy:		(Parameter covariance element) */
/* 	sxz:		(Parameter covariance element) */
/* 	syz:		(Parameter covariance element) */
/* 	stx:		(Parameter covariance element) */
/* 	sty:		(Parameter covariance element) */
/* 	stz:		(Parameter covariance element) */
/* 	stadel(i):	Distance from event to i'th station (deg) */
/* 	staazi(i):	Azimuth  from event to i'th station (deg) */
/* 	stabaz(i):	Back-azimuth from event to i'th station (deg) */
/* 	epimp(n):	Epicenter importance of n'th datum */
/* 	resid(n):	Residual (obs-calc) for n'th datum (sec, deg, */
/* 			sec/deg) */
/* 	rank:		Effective rank of the sensitivity matrix */
/* 	igap:		Maximum azimuthal gap (deg) for data used in solution */
/* 	niter:		Number of iterations performed in inversion */
/* 	ierr:		Error flag; */
/* 			  0 = No error */
/* 			  1 = Maximum number of iterations exhausted */
/* 			  2 = Iteration diverged */
/* 			  3 = Too few usable data to constrain any parameters */
/* 			  4 = Too few usable data to constrain origin time, */
/* 			      however, a valid location was obtained */
/* 			  5 = Insufficient data for a solution */
/* 			  6 = SVD routine cannot decompose matrix */
/* 	---- Functions called ---- */
/* 	Local */
/* 		ttcal0: 	Compute travel-times and their partials */
/* 		azcal: 		Compute azimuthal partial derivatives */
/* 		slocal0:	Compute horizontal slownesses and partials */
/* 		denuis:		Denuisance data before first iteration */
/* 		solve_via_svd: 	Perform least squares inversion via SVD */
/* 		ellips: 	Compute hypocentral error ellipsoid */
/* 		fstatx:  	Make an F-test */
/* 	From libgeog */
/* 		distaz2: 	Determine the distance between between two */
/* 				lat./lon. pairs */
/* 		latlon2: 	Compute a second lat./lon. from the first, */
/* 				distance, and azimuth */
/* DIAGNOSTICS */
/* 	Complains when input data are bad ... */
/* FILES */
/* 	None. */
/* NOTES */
/* 	It will probably be a good idea to remove the divergence */
/* 	test condition (variable, divrg) altogether.  The removal of */
/* 	divrg will also facilitate the ommission of variables, dxnrms(), */
/* 	dxn12 and dxn23. */
/* SEE ALSO */
/* 	Bratt and Bache (1988) "Locating events with a sparse network of */
/* 	regional arrays", BSSA, 78, 780-798.  Also, Jordan and Sverdrup */
/* 	(1981) "Teleseismic location techniques and their application to */
/* 	earthquake clusters in the south-central Pacific", BSSA, 71, */
/* 	1105-1130. */
/* AUTHOR */
/* 	Steve Bratt, December 1988. */
int hypinv0_(char *dstaid, char *dwavid, char *dtype, char *atype,
             float *dobs, float *dsd, int *ndata,
             float *stalat, float *stalon, int *nsta, int *maxtbd, int *maxtbz, int *ntbd, int *ntbz,
             float *tbd, float *tbz, float *tbtt,
             int *ipsta, int *ipwav, int *idtyp, int *iderr,
             float *alat0, float *alon0, float *zfoc0, float *sig0,
             int *ndf0,
             float *pconf, float *radius, float *azwt, float *damp,
             int *maxit,
             char *prtflg, char *fxdflg,
             int *luout,
             float *alat, float *alon, float *zfoc, float *torg, float *sighat, float *snssd,
             int *ndf,
             float *epmaj, float *epmin, float *epstr,
             float *zfint, float *toint,
             float *sxx, float *syy, float *szz, float *stt, float *sxy,
             float *sxz, float *syz, float *stx, float *sty, float *stz,
             float *stadel, float *staazi, float *stabaz, float *epimp,
             double *rank, float *resid,
             int *igap, int *niter, int *nd, int *ierr,
             int dstaid_len, int dwavid_len, int dtype_len, int atype_len,
             int prtflg_len, int fxdflg_len) {
    /* System generated locals */
    int tbd_dim1, tbd_offset, tbtt_dim1, tbtt_dim2, tbtt_offset, tbz_dim1,
	     tbz_offset, i__1, i__2;
    float r__1, r__2, r__3;
    doublereal d__1, d__2;

    /* Builtin functions */
    /* Subroutine */ int s_copy();
    double r_sign();
    int s_wsle(), do_lio(), e_wsle();
    double sqrt();
    int s_cmp(), s_wsfe(), do_fio(), e_wsfe();
    double atan2();
    int i_nint();

    /* Local variables */
    static doublereal andf, sgh12, sgh23, dxn12, dxn23, dist, xold[4];
    extern /* Subroutine */ int exit_();
    static doublereal step, xsol[4], slwt;
    static int ntoodeep;
    static doublereal cnvghats[3], snssdden, alat2, alon2;
    static int ierr0;
    static doublereal snssdnum;
    static int i__, k, m, n;
    static doublereal dmean, scale, delta;
    extern /* Subroutine */ int azcal_();
    static float dcalx;
    static doublereal cnvg12;
    static char phase[8];
    static float colat;
    static doublereal cnvg23;
    static float ecorr;
    static doublereal covar[16]	/* was [4][4] */;
    static logical divrg;
    static doublereal hyrak;
    static int inerr__, iterr;
    static doublereal dxmax, a1;
    static int nairquake;
    static doublereal a2, hyplu;
    static logical cnvrg;
    static doublereal hystr, epmaj0, wtrms;
    extern /* Subroutine */ int ttcal0_();
    static doublereal epmin0, hymaj0, hymid0, resid2[9999], resid3[9999];
    extern /* Subroutine */ int solve_via_svd__();
    static doublereal hymin0, zfint0;
    static int idtyp2[9999];
    static doublereal at[39996]	/* was [4][9999] */, fs;
    static int np;
    static doublereal condit[2];
    extern /* Subroutine */ int elpcor_();
    static doublereal sghats[3];
    static char fxdsav[1];
    extern /* Subroutine */ int denuis_();
    static int ntimes, nazims;
    extern /* Subroutine */ int ellips_();
    static doublereal dxnorm;
    static int ip0[9999];
    extern /* Subroutine */ int fstatx_();
    static doublereal dxnrms[3];
    static int nslows;
    extern /* Subroutine */ int slocal0_();
    static doublereal fac;
    extern /* Subroutine */ int latlon2_(), distaz2_();
    static int iga[9999];
    static doublereal azi;
    static int nds[3];
    static doublereal atx[4], ssq, cnvgold;
    static int ndftemp;
    static float correct;
    static logical ldenuis;
    static doublereal dsd2[9999], cnvgtst, sta1, sta2, sta3, sta4, sta5, 
	    unwtrms;

    /* Fortran I/O blocks */
    static cilist io___36 = { 0, 6, 0, 0, 0 };
    static cilist io___45 = { 0, 0, 0, 0, 0 };
    static cilist io___46 = { 0, 0, 0, 0, 0 };
    static cilist io___47 = { 0, 0, 0, "(2(a,i3),/,a,f8.3,2(a,f9.3),a,f10.3,\
/,                             2(a,f8.4),a,e12.5,/)", 0 };
    static cilist io___48 = { 0, 0, 0, "(2a,2(/,2a))", 0 };
    static cilist io___49 = { 0, 0, 0, "(a6,1x,a8,1x,a4,2(f10.2,f12.2),f10.2)"
	    , 0 };
    static cilist io___57 = { 0, 0, 0, "(/,2(a,f7.3),3(a,f9.3),/,2(a,g11.3))",
	     0 };
    static cilist io___63 = { 0, 0, 0, 0, 0 };


/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declarations ---- */
/*     History of standard errors and perturbation vectors used to */
/*     decide on convergence */
/*     Minimum number of iterations required */
/*     Maximum number of data allowed */
/*     Maximum number of parameters allowed */
/*     Convert radians to degrees */
/*     Tolerance setting for condition number */
/*     Tolerance setting for convergence checks */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/* , nd  ! */
/* === Temporary Grid Test === top */
/*     integer*4 ilat, ilon, iprj, j, lnblnk, ncol, nrow, nz, ounit */
/*     real*4    cm, bl, dx, dy, rot, xcen, xo, ycen, yo */
/*     real*4    dcalx_array(80,240,20), deriv_array(80,240,20,4) */
/*     real*4    resid_array(80,240,20), wtrms_array(80,240) */
/*     character dd(4)*1, filename*80, id*56, pgm*8 */
/* === Temporary Grid Test === bottom */
/*     Initializations */
    /* Parameter adjustments */
    --resid;
    --epimp;
    --iderr;
    --idtyp;
    --ipwav;
    --ipsta;
    --dsd;
    --dobs;
    atype -= atype_len;
    dtype -= dtype_len;
    dwavid -= dwavid_len;
    dstaid -= dstaid_len;
    --stabaz;
    --staazi;
    --stadel;
    --stalon;
    --stalat;
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
    --ntbd;
    --ntbz;

    /* Function Body */
    *alat = *alat0;
    *alon = *alon0;
    *zfoc = *zfoc0;
    *torg = (float)0.;
    *sighat = (float)-1.;
    *snssd = (float)-1.;
    *ndf = -1;
    *epmaj = (float)-1.;
    *epmin = (float)-1.;
    *epstr = (float)-1.;
    *zfint = (float)-1.;
    *toint = (float)-1.;
    *sxx = (float)-999.;
    *syy = (float)-999.;
    *szz = (float)-999.;
    *stt = (float)-999.;
    *sxy = (float)-999.;
    *sxz = (float)-999.;
    *syz = (float)-999.;
    *stx = (float)-999.;
    *sty = (float)-999.;
    *stz = (float)-999.;
    slwt = (float)1.;
    *azwt = (float)1.;
    cnvrg = FALSE_;
    ldenuis = FALSE_;
    *niter = 0;
    *ierr = 0;
    ierr0 = 0;
    nairquake = 0;
    ntoodeep = 0;
    i__1 = *nsta;
    for (i__ = 1; i__ <= i__1; ++i__) {
	stadel[i__] = (float)-1.;
	staazi[i__] = (float)-1.;
	stabaz[i__] = (float)-1.;
/* L1000: */
    }
/* === Temporary Grid Test === top */
/*     alat0 = 1.15 */
/*     alon0 = 122.75 */
/*     ncol  = 240 */
/*     nrow  = 80 */
/*     xo    = 0.0 */
/*     yo    = 0.0 */
/*     dx    = 0.2316 */
/*     dy    = 0.2316 */
/*     bl    = alat0 */
/*     cm    = alon0 */
/*     do 4000 ilat = 1, nrow */
/*        alat = alat0 + 0.00208333*(ilat-1) */
/*        print*, 'Current row of latitude: ', alat, ' deg.' */
/*        do 4000 ilon = 1, ncol */
/*           alon = alon0 + 0.00208333*(ilon-1) */
/* === Temporary Grid Test === bottom */
/*     Main iterative loop */
L1020:
    i__1 = *ndata;
    for (n = 1; n <= i__1; ++n) {
	resid[n] = (float)-999.;
	resid2[n - 1] = (float)-999.;
	at[(n << 2) - 4] = (float)0.;
	at[(n << 2) - 3] = (float)0.;
	at[(n << 2) - 2] = (float)0.;
	at[(n << 2) - 1] = (float)0.;
/* L1030: */
    }
/*     Set fix-depth flag and number of parameters.  Depth is always fixed */
/*     during the first 2 iterations.  If depth becomes negative ("airquake"), */
/*     then fix the depth at 0.0 during the next iteration.  If several */
/*     airquakes occur, then fix the depth at zero for all subsequent */
/*     iterations.  Also fix events > 650.0 km to value 650.0 during the next */
/*     iteration, i.e., to the approximate depth of the deepest credible */
/*     earthquake -- WCN. */

/*     !!!MODIFICATION!!! Mathias, 2008.267 */
/*      max depth: 650 --> 750 km */

    if (*niter < 3) {
	*(unsigned char *)fxdsav = 'y';
    } else if (nairquake > 4) {
	*(unsigned char *)fxdsav = 'y';
	*zfoc = (float)0.;
	xsol[3] = (float)0.;
    } else if (ntoodeep > 4) {
	*(unsigned char *)fxdsav = 'y';
	*zfoc = (float)750.;
	xsol[3] = (float)0.;
    } else if (*zfoc < (float)0.) {
	++nairquake;
	*zfoc = (float)0.;
	xsol[3] = (float)0.;
    } else if (*zfoc > (float)750.) {
	++ntoodeep;
	*zfoc = (float)750.;
	xsol[3] = (float)0.;
    } else {
	s_copy(fxdsav, fxdflg, (ftnlen)1, fxdflg_len);
    }
/*     How many model parameters? */
    if (*(unsigned char *)fxdsav != 'y') {
	np = 4;
    } else {
	np = 3;
    }
/*     Compute distance and azimuths to stations (forward problem for */
/*     azimuths) */
    a1 = *alat;
    a2 = *alon;
    i__1 = *nsta;
    for (i__ = 1; i__ <= i__1; ++i__) {
	sta1 = stalat[i__];
	sta2 = stalon[i__];
	distaz2_(&a1, &a2, &sta1, &sta2, &sta3, &sta4, &sta5);
	stadel[i__] = sta3;
	staazi[i__] = sta4;
	stabaz[i__] = sta5;
/* L1040: */
    }
/*     Compute travel-times, slownesses and azimuths based on current */
/*     location hypothesis and determine partial derivatives.  Ignore */
/*     points with completely invalid data (i.e., iderr = 1, 2, 3). */
    *nd = 0;
    ntimes = 0;
    nazims = 0;
    nslows = 0;
    i__1 = *ndata;
    for (n = 1; n <= i__1; ++n) {
	if (iderr[n] < 1 || iderr[n] > 3) {
	    i__ = ipsta[n];
	    k = ipwav[n];
/*           Arrival times */
	    if (idtyp[n] == 1) {
/*              call ttime_calc (k-1, atx, staazi(i), stadel(i), radius, */
/*    &                          zfoc, dcalx, iterr) */
		ttcal0_(&k, zfoc, radius, &stadel[i__], &staazi[i__], maxtbd, 
			maxtbz, &ntbd[k], &ntbz[k], &tbd[k * tbd_dim1 + 1], &
			tbz[k * tbz_dim1 + 1], &tbtt[(k * tbtt_dim2 + 1) * 
			tbtt_dim1 + 1], &dcalx, atx, &iterr);

/*      ellipticity corrections for travel times included with routine */
/*      elpcor.f including the ellipticity corrections of the IASPEI 1991 */
/*      Travel Time Tables (Kennet, 1991). The routine was received by NEIC. */
/*      johannes schweitzer mar 24, 1991 */
/*      bochum , geress */

		s_copy(phase, dwavid + n * dwavid_len, (ftnlen)8, dwavid_len);
		colat = (float)90. - *alat;
		elpcor_(phase, &stadel[i__], zfoc, &staazi[i__], &colat, &
			ecorr, (ftnlen)8);
		dcalx += ecorr;
/*              Use only those data that have been interpolated (iterr = 0) */
/*              or have been extrapolated to depths beyond these curves */
/*              (iterr = 15).  If the number of iterations is less than */
/*              minit, allow all extrapolated values. */
		if (*niter < 4 || iterr == 15) {
		    iderr[n] = 0;
		} else {
		    iderr[n] = iterr;
		}
/*           Azimuths */
	    } else if (idtyp[n] == 2) {
		azcal_(radius, &stadel[i__], &staazi[i__], &stabaz[i__], &
			dcalx, atx);
/*           Slownesses */
	    } else if (idtyp[n] == 3) {
/*              call slow_calc (k-1, atx, staazi(i), stadel(i), radius, */
/*    &                         zfoc, dcalx, iterr) */
		slocal0_(&k, zfoc, radius, &stadel[i__], &staazi[i__], maxtbd,
			 maxtbz, &ntbd[k], &ntbz[k], &tbd[k * tbd_dim1 + 1], &
			tbz[k * tbz_dim1 + 1], &tbtt[(k * tbtt_dim2 + 1) * 
			tbtt_dim1 + 1], &dcalx, atx, &iterr);
/*              Same rules as for travel-time calculations. */
		if (*niter < 4 || iterr == 15) {
		    iderr[n] = 0;
		} else {
		    iderr[n] = iterr;
		}
	    }
/*           Apply station correction adjustments, if necessary */
	    correct = (float)0.;
/*           if (niter.gt.2 .and. idtyp(n).eq.1) */
/*    &         call ssscor (alat, alon, correct, 1, i, k) */
/*           Compute residual = [observed - calculated] datum */
/*                              + station correction */
	    if (idtyp[n] != 1) {
		resid[n] = dobs[n] - dcalx;
	    } else {
		resid[n] = dobs[n] - dcalx - *torg + correct;
	    }
/*           If the azimuth residual is > +/- 180.0 deg., change it to the */
/*           corresponding difference that is < +/- 180.0 deg. */
	    if (idtyp[n] == 2 && (r__1 = resid[n], dabs(r__1)) > (float)180.) 
		    {
		r__3 = (float)360. - (r__2 = resid[n], dabs(r__2));
		resid[n] = -r_sign(&r__3, &resid[n]);
	    }
/*           Load valid data and partials for defining detections into */
/*           arrays.  Note that parameters are ordered: origin-time; */
/*           longitude; latitude; depth.  If depth is fixed, np = 3. */
	    if (iderr[n] < 1 && *(unsigned char *)&atype[n * atype_len] == 
		    'd') {
		++(*nd);
		i__2 = np;
		for (m = 1; m <= i__2; ++m) {
/* L1050: */
		    at[m + (*nd << 2) - 5] = atx[m - 1];
		}
/*              Array ip0 holds the original index of the n'th valid datum */
		ip0[*nd - 1] = n;
		resid2[*nd - 1] = resid[n];
		dsd2[*nd - 1] = dsd[n];
		idtyp2[*nd - 1] = idtyp[n];
/*              Count valid data for each data type */
		if (idtyp2[*nd - 1] == 1) {
		    ++ntimes;
		} else if (idtyp2[*nd - 1] == 2) {
		    ++nazims;
		} else if (idtyp2[*nd - 1] == 3) {
		    ++nslows;
		}
/*              dcalx_array(ilat,ilon,nd) = dcalx */
	    }
	}
/* L1060: */
    }
/*     Quick check on array declarations */
    if (np > 4 || *nd > 9999) {
	s_wsle(&io___36);
	do_lio(&c__9, &c__1, "- Enlarge the dimensions of maxparm and/or max\
data", (ftnlen)50);
	e_wsle();
	exit_(&c_n1);
    }
/*     Check for insufficient data */
    if (*(unsigned char *)fxdsav != 'y') {
	if (*nd < 4) {
	    *ierr = 5;
	    return 0;
	}
    } else {
	if (*nd < 3) {
	    *ierr = 5;
	    return 0;
	}
    }
/*     If initial iteration, then orthogonalize out origin-time term */
    if (! ldenuis) {
	denuis_(idtyp2, nd, &np, resid2, dsd2, at, &dmean, &inerr__);
	*torg = dmean;
	if (inerr__ != 0) {
	    *torg = (float)0.;
	}
	ldenuis = TRUE_;
	goto L1020;
    }
/*     Compute weighted and unweighted RMS residual (dimensionless quantities). */
/*     Also normalize matrix and residuals w.r.t. data standard deviations */
/*     and apply weights to azimuth and slowness data, as necessary. */
    wtrms = (float)0.;
    unwtrms = (float)0.;
    i__1 = *nd;
    for (n = 1; n <= i__1; ++n) {
	resid3[n - 1] = resid2[n - 1];
/* Computing 2nd power */
	d__1 = resid3[n - 1];
	unwtrms += d__1 * d__1;
	if (idtyp2[n - 1] == 1) {
	    resid2[n - 1] /= dsd2[n - 1];
	} else if (idtyp2[n - 1] == 2) {
	    resid2[n - 1] = *azwt * resid2[n - 1] / dsd2[n - 1];
	} else if (idtyp2[n - 1] == 3) {
	    resid2[n - 1] = slwt * resid2[n - 1] / dsd2[n - 1];
	}
/* Computing 2nd power */
	d__1 = resid2[n - 1];
	wtrms += d__1 * d__1;
/*        resid_array(ilat,ilon,n) = resid2(n) */
	i__2 = np;
	for (m = 1; m <= i__2; ++m) {
	    if (idtyp2[n - 1] == 1) {
		at[m + (n << 2) - 5] /= dsd2[n - 1];
	    } else if (idtyp2[n - 1] == 2) {
		at[m + (n << 2) - 5] = *azwt * at[m + (n << 2) - 5] / dsd2[n 
			- 1];
	    } else if (idtyp2[n - 1] == 3) {
		at[m + (n << 2) - 5] = slwt * at[m + (n << 2) - 5] / dsd2[n - 
			1];
	    }
/*           deriv_array(ilat,ilon,n,m) = at(m,n) */
/* L1070: */
	}
/* L1080: */
    }
    wtrms = sqrt(wtrms / *nd);
    unwtrms = sqrt(unwtrms / *nd);
/* === Temporary Grid Test === top */
/*     wtrms_array(ilat,ilon) = wtrms */
/* 4000 continue */
/*     pgm = 'gsuperc' */
/*     nz = 1 */
/*     iprj = 2 */
/*     rot = 0.0 */
/*     xcen = 0.0 */
/*     ycen = 0.0 */
/*     ounit = 29 */
/*     id 	= 'Overall Weighted RMS Residual For Event' */
/*     filename	= 'wtrms.asc' */
/*     open (ounit, file = filename) */
/*     write (ounit, '(a56,a8)') id, pgm */
/*     write (ounit, '(6x,i8,8x,i8,6x,i8)') ncol, nrow, nz */
/*     write (ounit, '(4(5x,e14.8))') xo, dx, yo, dy */
/*     write (ounit, '(12x,i4,6x,e14.8,6x,e14.8)') iprj, cm, bl */
/*     write (ounit, '(6x,e14.8,7x,e14.8,7x,e14.8)') rot, xcen, ycen */
/*     do 4002 ilat = 1, nrow */
/*        write (ounit, '(e15.8)') 0.0 */
/*        do 4001 ilon = 1, ncol, 5 */
/*           write (ounit, '(5e15.8)') (wtrms_array(ilat,ilon+i-1), */
/*    &                                 i = 1, 5) */
/* 4001    continue */
/* 4002 continue */
/*     close (ounit) */
/*     do 4010 n = 1, nd */
/*        if (atype(ip0(n))(1:1).ne.'d') goto 4010 */
/*        k = lnblnk(dstaid(ip0(n))) */
/*        j = lnblnk(dwavid(ip0(n))) */
/*        filename = dstaid(ip0(n))(1:k)//'_'//dwavid(ip0(n))(1:j)//'_'// */
/*    &              dtype(ip0(n))(1:1)//'_res.asc' */
/*        id = 'Weighted Residuals: '//filename(1:lnblnk(filename)) */
/*        open (ounit, file = filename) */
/*        write (ounit, '(a56,a8)') id, pgm */
/*        write (ounit, '(6x,i8,8x,i8,6x,i8)') ncol, nrow, nz */
/*        write (ounit, '(4(5x,e14.8))') xo, dx, yo, dy */
/*        write (ounit, '(12x,i4,6x,e14.8,6x,e14.8)') iprj, cm, bl */
/*        write (ounit, '(6x,e14.8,7x,e14.8,7x,e14.8)') rot, xcen, ycen */
/*        do 4004 ilat = 1, nrow */
/*           write (ounit, '(e15.8)') 0.0 */
/*           do 4003 ilon = 1, ncol, 5 */
/*              write (ounit, '(5e15.8)') (resid_array(ilat,ilon+i-1,n), */
/*    &                                    i = 1, 5) */
/* 4003       continue */
/* 4004    continue */
/*        close (ounit) */
/*        filename = dstaid(ip0(n))(1:k)//'_'//dwavid(ip0(n))(1:j)//'_'// */
/*    &              dtype(ip0(n))(1:1)//'_dcalx.asc' */
/*        id = 'Calculated: '//filename(1:lnblnk(filename)) */
/*        open (ounit, file = filename) */
/*        write (ounit, '(a56,a8)') id, pgm */
/*        write (ounit, '(6x,i8,8x,i8,6x,i8)') ncol, nrow, nz */
/*        write (ounit, '(4(5x,e14.8))') xo, dx, yo, dy */
/*        write (ounit, '(12x,i4,6x,e14.8,6x,e14.8)') iprj, cm, bl */
/*        write (ounit, '(6x,e14.8,7x,e14.8,7x,e14.8)') rot, xcen, ycen */
/*        do 4006 ilat = 1, nrow */
/*           write (ounit, '(e15.8)') 0.0 */
/*           do 4005 ilon = 1, ncol, 5 */
/*              write (ounit, '(5e15.8)') (dcalx_array(ilat,ilon+i-1,n), */
/*    &                                    i = 1, 5) */
/* 4005       continue */
/* 4006    continue */
/*        close (ounit) */
/*        dd(1) = 'T' */
/*        dd(2) = 'X' */
/*        dd(3) = 'Y' */
/*        dd(4) = 'Z' */
/*        do 4009 m = 1, 4 */
/*           filename = dstaid(ip0(n))(1:k)//'_'//dwavid(ip0(n))(1:j)// */
/*    &                 '_'//dtype(ip0(n))(1:1)//'_deriv_'//dd(m)//'.asc' */
/*           id = dd(m)//'-derivative: '//filename(1:lnblnk(filename)) */
/*           open (ounit, file = filename) */
/*           write (ounit, '(a56,a8)') id, pgm */
/*           write (ounit, '(6x,i8,8x,i8,6x,i8)') ncol, nrow, nz */
/*           write (ounit, '(4(5x,e14.8))') xo, dx, yo, dy */
/*           write (ounit, '(12x,i4,6x,e14.8,6x,e14.8)') iprj, cm, bl */
/*           write (ounit, '(6x,e14.8,7x,e14.8,7x,e14.8)') rot, xcen, */
/*    &             ycen */
/*           do 4008 ilat = 1, nrow */
/*              write (ounit, '(e15.8)') 0.0 */
/*              do 4007 ilon = 1, ncol, 5 */
/*                 write (ounit, '(5e15.8)') */
/*    &                   (deriv_array(ilat,ilon+i-1,n,m), i = 1, 5) */
/* 4007          continue */
/* 4008       continue */
/*           close (ounit) */
/* 4009    continue */
/* 4010 continue */
/*     stop 'Grid completed !' */
/* === Temporary Grid Test === bottom */
/*     If convergence has been reached, break out of main iterative loop */
    if (cnvrg) {
	goto L1200;
    }
/*     Determine least squares solution */
    solve_via_svd__(&c__1, nd, &np, &c__4, at, resid2, damp, &cnvgtst, condit,
	     xsol, covar, &epimp[1], rank, ierr);
    if (*ierr == 6) {
	return 0;
    }
/*     Print information at each iterative step */
    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	io___45.ciunit = *luout;
	s_wsle(&io___45);
	do_lio(&c__9, &c__1, " ", (ftnlen)1);
	e_wsle();
	io___46.ciunit = *luout;
	s_wsle(&io___46);
	do_lio(&c__9, &c__1, " ", (ftnlen)1);
	e_wsle();
	io___47.ciunit = *luout;
	s_wsfe(&io___47);
	do_fio(&c__1, "- Iteration #", (ftnlen)13);
	do_fio(&c__1, (char *)&(*niter), (ftnlen)sizeof(int));
	do_fio(&c__1, "   Number of Obs. (Data):", (ftnlen)25);
	do_fio(&c__1, (char *)&(*nd), (ftnlen)sizeof(int));
	do_fio(&c__1, "- Lat:", (ftnlen)6);
	do_fio(&c__1, (char *)&(*alat), (ftnlen)sizeof(float));
	do_fio(&c__1, "   Lon:", (ftnlen)7);
	do_fio(&c__1, (char *)&(*alon), (ftnlen)sizeof(float));
	do_fio(&c__1, "   Depth:", (ftnlen)9);
	do_fio(&c__1, (char *)&(*zfoc), (ftnlen)sizeof(float));
	do_fio(&c__1, "   To:", (ftnlen)6);
	do_fio(&c__1, (char *)&(*torg), (ftnlen)sizeof(float));
	do_fio(&c__1, "- Unwt. RMS Res.:", (ftnlen)17);
	do_fio(&c__1, (char *)&unwtrms, (ftnlen)sizeof(doublereal));
	do_fio(&c__1, "   Wt. RMS Res.:", (ftnlen)16);
	do_fio(&c__1, (char *)&wtrms, (ftnlen)sizeof(doublereal));
	do_fio(&c__1, "   CNVGTST:", (ftnlen)11);
	do_fio(&c__1, (char *)&cnvgtst, (ftnlen)sizeof(doublereal));
	e_wsfe();
	io___48.ciunit = *luout;
	s_wsfe(&io___48);
	do_fio(&c__1, "       Phase    Data      Travel Times      ", (ftnlen)
		44);
	do_fio(&c__1, "       Residuals      Distance", (ftnlen)30);
	do_fio(&c__1, "Sta    Type     Type  Observed  Calculated  ", (ftnlen)
		44);
	do_fio(&c__1, "    True  Normalized    (deg.)", (ftnlen)30);
	do_fio(&c__1, "------ -------- ----  --------  ----------  ", (ftnlen)
		44);
	do_fio(&c__1, "--------  ----------  --------", (ftnlen)30);
	e_wsfe();
	i__1 = *nd;
	for (n = 1; n <= i__1; ++n) {
	    io___49.ciunit = *luout;
	    s_wsfe(&io___49);
	    do_fio(&c__1, dstaid + ip0[n - 1] * dstaid_len, dstaid_len);
	    do_fio(&c__1, dwavid + ip0[n - 1] * dwavid_len, dwavid_len);
	    do_fio(&c__1, dtype + ip0[n - 1] * dtype_len, dtype_len);
	    do_fio(&c__1, (char *)&dobs[ip0[n - 1]], (ftnlen)sizeof(float));
	    d__1 = dobs[ip0[n - 1]] - resid3[n - 1];
	    do_fio(&c__1, (char *)&d__1, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, (char *)&resid3[n - 1], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, (char *)&resid2[n - 1], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, (char *)&stadel[ipsta[ip0[n - 1]]], (ftnlen)sizeof(
		    float));
	    e_wsfe();
/* L1090: */
	}
    }
/*     Compute number of degrees of freedom and data-variance estimate */
/*     ndf0 is the K of Jordan and Sverdrup (1981); Bratt and Bache (1988) */
/* 	   set ndf0 = 8; here we set ndf0 = 9999 */
/*     sig0 is "not" the s-sub-k of Jordan and Sverdrup (1981); */
/*          here, sig0 = 1.0 */
/*     ndf  is the total degrees of freedom assuming a chi-squared */
/*          distribution = ndf0 + [# of data + # of parameters] */
/*     ssq  is the numerator for the a posteriori estimate for the */
/*          squared variance scale factor */
/*     sighat, is therefore, the actual estimate of the variance scale */
/* 	   factor (eqn. 34 of J&S, 1981), and subsequently, */
/*     snssd is the normalized a priori estimate for the estimated */
/*          variance scale factor */
    *ndf = *ndf0 + *nd - np;
    ssq = *ndf0 * *sig0 * *sig0;
    i__1 = *nd;
    for (n = 1; n <= i__1; ++n) {
/* L1100: */
/* Computing 2nd power */
	d__1 = resid2[n - 1];
	ssq += d__1 * d__1;
    }
    andf = (doublereal) (*ndf);
    if (*ndf == 0) {
	andf = (float).001;
    }
    if ((d__1 = (real) (*ndf) - ssq, abs(d__1)) < (float)1e-5) {
	andf = ssq;
    }
    *sighat = sqrt(ssq / andf);
    snssdnum = ssq - *ndf0 * *sig0 * *sig0;
    snssdden = andf - *ndf0;
    if (abs(snssdden) > (float).001 && snssdnum / snssdden >= (float)0.) {
	*snssd = sqrt(snssdnum / snssdden);
    } else {
	*snssd = (float)999.;
    }
/*     Compute norm of hypocenter perturbations */
    ssq = (float)0.;
    i__1 = np;
    for (m = 1; m <= i__1; ++m) {
/* L1110: */
/* Computing 2nd power */
	d__1 = xsol[m - 1];
	ssq += d__1 * d__1;
    }
    dxnorm = sqrt(ssq);
/*     Scale down hypocenter perturbations if they are very large.  Scale */
/*     down even more for lat(t)er iterations. */
    dxmax = (float)1500.;
    if (*niter < *maxit / 5 + 1) {
	dxmax = (float)3e3;
    }
    if (dxnorm > dxmax) {
	scale = dxmax / dxnorm;
	i__1 = np;
	for (m = 1; m <= i__1; ++m) {
/* L1120: */
	    xsol[m - 1] *= scale;
	}
	dxnorm = dxmax;
    }
    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	io___57.ciunit = *luout;
	s_wsfe(&io___57);
	do_fio(&c__1, "> Sighat:", (ftnlen)9);
	do_fio(&c__1, (char *)&(*sighat), (ftnlen)sizeof(float));
	do_fio(&c__1, "   NSSD:", (ftnlen)8);
	do_fio(&c__1, (char *)&(*snssd), (ftnlen)sizeof(float));
	do_fio(&c__1, "   dLat:", (ftnlen)8);
	do_fio(&c__1, (char *)&xsol[2], (ftnlen)sizeof(doublereal));
	do_fio(&c__1, "   dLon:", (ftnlen)8);
	do_fio(&c__1, (char *)&xsol[1], (ftnlen)sizeof(doublereal));
	do_fio(&c__1, "   dZ:", (ftnlen)6);
	do_fio(&c__1, (char *)&xsol[3], (ftnlen)sizeof(doublereal));
	do_fio(&c__1, "> True Cond. Num.:", (ftnlen)18);
	do_fio(&c__1, (char *)&condit[0], (ftnlen)sizeof(doublereal));
	do_fio(&c__1, "   Effective Cond. Num.:", (ftnlen)24);
	do_fio(&c__1, (char *)&condit[1], (ftnlen)sizeof(doublereal));
	e_wsfe();
    }
/*     Store the convergence test information from the 2 previous iterations */
/* Computing MIN */
    i__1 = 3, i__2 = *niter + 1;
    for (i__ = min(i__1,i__2); i__ >= 2; --i__) {
	cnvghats[i__ - 1] = cnvghats[i__ - 2];
	sghats[i__ - 1] = sghats[i__ - 2];
	dxnrms[i__ - 1] = dxnrms[i__ - 2];
	nds[i__ - 1] = nds[i__ - 2];
/* L1130: */
    }
/*     Current convergence test information */
    cnvghats[0] = cnvgtst;
    sghats[0] = *snssd;
    dxnrms[0] = dxnorm;
    nds[0] = *nd;
/*     Stop iterations if number of data < number of parameters.  The */
/*     exception is when the depth is fixed (np = 3) and we have only */
/*     azimuth, or one azimuth and one slowness data.  In that case */
/*     continue on even though it will be impossible to get an origin time. */
    ndftemp = *ndf0;
    if (np == 3 && (nazims > 1 || (nazims > 0 && nslows > 0))) {
	ndftemp = *ndf0 - 1;
    }
/*     Convergence, divergence or just keep on iterating */
    if (*ndf < ndftemp) {
	if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	    io___63.ciunit = *luout;
	    s_wsle(&io___63);
	    do_lio(&c__9, &c__1, "   Too few data usable to continue:", (
		    ftnlen)35);
	    do_lio(&c__3, &c__1, (char *)&(*nd), (ftnlen)sizeof(int));
	    e_wsle();
	}
	divrg = TRUE_;
	cnvrg = FALSE_;
	ierr0 = 1;
	*ierr = 2;
    } else if (*niter < 4) {
	divrg = FALSE_;
	cnvrg = FALSE_;
    } else {
	if (dxnorm > (float)0.) {
	    cnvg12 = cnvghats[0] / cnvghats[1];
	    cnvg23 = cnvghats[1] / cnvghats[2];
	    sgh12 = sghats[0] / sghats[1];
	    sgh23 = sghats[1] / sghats[2];
	    dxn12 = dxnrms[0] / dxnrms[1];
	    dxn23 = dxnrms[1] / dxnrms[2];
	    divrg = ((sgh23 > (float)1.1 && sgh12 > sgh23) || (dxn23 > (float)1.1 && dxn12 > dxn23 && *niter > 6 && dxnorm > (float)1e3));
	    cnvrg = nds[0] == nds[1] && ! divrg && (sgh12 > (float).99 && 
		    sgh12 < (float)1.001) && (cnvgtst < 1e-8 || dxnorm < (
		    float).5);
	    if ( (cnvgtst < cnvgold * (float)1.01 && cnvgtst < 1e-8) ||
	         (*niter > *maxit * 3 / 4 &&
	          (cnvgtst < sqrt(1e-8) ||
	          (d__1 = cnvg23 - cnvg12, abs(d__1)) < 1e-8 ||
	          (d__2 = cnvghats[0] - cnvghats[2], abs(d__2)) < (float)1e-5)) ) {
		cnvrg = TRUE_;
	    }
	    if ((wtrms < (float).001 || dxnrms[0] < (float).001) && *niter > 
		    6) {
		cnvrg = TRUE_;
	    }
	} else {
	    divrg = FALSE_;
	    cnvrg = TRUE_;
	}
    }
/*     Apply step-length weighting, if unweighted RMS residual is increasing */
    if (*niter > 6 && (cnvgtst > cnvgold || cnvghats[0] - cnvghats[2] == (
	    float)0.) && step > (float).05) {
	step *= (float).5;
	if (step != (float).5) {
	    i__1 = np;
	    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1140: */
		xsol[i__ - 1] = step * xold[i__ - 1];
	    }
	} else {
	    i__1 = np;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		xsol[i__ - 1] = step * xsol[i__ - 1];
		xold[i__ - 1] = xsol[i__ - 1];
/* L1150: */
	    }
	}
    } else {
	step = (float)1.;
	cnvgold = cnvgtst;
    }
/*     Perturb hypocenter */
    if (xsol[1] != (float)0. || xsol[2] != (float)0.) {
	azi = atan2(xsol[1], xsol[2]) * 57.2957795;
/* Computing 2nd power */
	d__1 = xsol[1];
/* Computing 2nd power */
	d__2 = xsol[2];
	dist = sqrt(d__1 * d__1 + d__2 * d__2);
	delta = dist / (*radius - *zfoc) * 57.2957795;
	a1 = *alat;
	a2 = *alon;
	latlon2_(&a1, &a2, &delta, &azi, &alat2, &alon2);
	*alat = alat2;
	*alon = alon2;
    }
    *torg += xsol[0];
    if (*(unsigned char *)fxdsav != 'y') {
	*zfoc -= xsol[3];
    }
/*     End of main iterative loop */
    if (cnvrg) {
	*ierr = 0;
	if (condit[0] > 1e4) {
	    *ierr = 5;
	    return 0;
	}
	goto L1020;
    } else if (divrg) {
	*ierr = 2;
	if (ierr0 == 1) {
	    *ierr = 3;
	}
	goto L1210;
    } else if (*niter >= *maxit) {
	*ierr = 1;
    } else {
	++(*niter);
	goto L1020;
    }
/*     Compute confidence regions */
L1200:
    solve_via_svd__(&c__2, nd, &np, &c__4, at, resid2, damp, &cnvgtst, condit,
	     xsol, covar, &epimp[1], rank, ierr);
    if (*ierr == 6) {
	return 0;
    }
/*     Compute location confidence bounds */
    ellips_(&np, covar, &hymaj0, &hymid0, &hymin0, &hystr, &hyplu, &hyrak, &
	    epmaj0, &epmin0, epstr, &zfint0, stt, stx, sty, sxx, sxy, syy, 
	    stz, sxz, syz, szz);
/*     Not currently used, so commented out (WCN) */
/*     call fstatx (3, ndf, pconf, fs) */
/*     fac   = dsqrt(3.0*fs)*sighat */
/*     hymaj = hymaj0*fac */
/*     hymid = hymid0*fac */
/*     hymin = hymin0*fac */
    fstatx_(&c__2, ndf, pconf, &fs);
    fac = sqrt(fs * (float)2.) * *sighat;
    *epmaj = epmaj0 * fac;
    *epmin = epmin0 * fac;
    fstatx_(&c__1, ndf, pconf, &fs);
    fac = sqrt(fs) * *sighat;
    *zfint = zfint0 * fac;
/*     szz   = zfint0*zfint0 */
    if (*stt < (float)0. || ntimes < 1) {
	*toint = (float)-999.;
    } else {
	*toint = sqrt(*stt) * fac;
    }
/*     Remove weights and standard deviations from residuals */
L1210:
    i__1 = *nd;
    for (n = 1; n <= i__1; ++n) {
	if (idtyp2[n - 1] == 1) {
	    resid2[n - 1] *= dsd2[n - 1];
	} else if (idtyp2[n - 1] == 2) {
	    resid2[n - 1] = resid2[n - 1] * dsd2[n - 1] / *azwt;
	} else if (idtyp2[n - 1] == 3) {
	    resid2[n - 1] = resid2[n - 1] * dsd2[n - 1] / slwt;
	}
/* L1220: */
    }
/*     Place input values back into original arrays */
    for (n = *nd; n >= 1; --n) {
	resid[ip0[n - 1]] = resid2[n - 1];
	epimp[ip0[n - 1]] = epimp[n];
/* L1230: */
    }
/*     Compute azimuthal GAP (deg.) for this event */
    i__1 = *nd;
    for (n = 1; n <= i__1; ++n) {
	iga[n - 1] = 0;
	k = ipsta[ip0[n - 1]];
	iga[n - 1] = i_nint(&staazi[k]);
/* L1240: */
    }
/*     Quick and dirty shell sort routine */
    for (k = (*nd + 1) / 2; k >= 1; --k) {
	i__1 = *nd - k;
	for (n = 1; n <= i__1; ++n) {
	    if (iga[n - 1] > iga[n + k - 1]) {
		m = iga[n - 1];
		iga[n - 1] = iga[n + k - 1];
		iga[n + k - 1] = m;
	    }
/* L1250: */
	}
    }
    *igap = 0;
    i__1 = *nd;
    for (n = 2; n <= i__1; ++n) {
/* L1260: */
	if (iga[n - 1] - iga[n - 2] > *igap) {
	    *igap = iga[n - 1] - iga[n - 2];
	}
    }
    m = 360 - iga[*nd - 1] + iga[0];
    if (m > *igap) {
	*igap = m;
    }
/*     Don't return a depth value < 0.0 */
    if (*zfoc < (float)0.) {
	*zfoc = (float)0.;
    }
/*     Correct non-defining arrival times for the origin time and then */
/*     load the default values into arrays for the erroneous data */
    i__1 = *ndata;
    for (n = 1; n <= i__1; ++n) {
	if ((iderr[n] > 0 && iderr[n] < 4) || iderr[n] == 11) {
	    resid[n] = (float)-999.;
	    epimp[n] = (float)-1.;
	    s_copy(atype + n * atype_len, "n", atype_len, (ftnlen)1);
	} else if (idtyp[n] == 1 && (*(unsigned char *)&atype[n * atype_len] 
		!= 'd' || iderr[n] > 0)) {
	    epimp[n] = (float)-1.;
	    s_copy(atype + n * atype_len, "n", atype_len, (ftnlen)1);
	} else if (iderr[n] > 0) {
	    epimp[n] = (float)-1.;
	    s_copy(atype + n * atype_len, "n", atype_len, (ftnlen)1);
	}
	if (*(unsigned char *)&atype[n * atype_len] != 'd') {
	    epimp[n] = (float)-1.;
	}
/* L1270: */
    }
    if (*toint <= (float)-888.) {
	*ierr = 4;
    }
    return 0;
} /* hypinv0_ */

