/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccshypcut0_1_ {
    char sccsid[80];
};

#define sccshypcut0_1 (*(struct sccshypcut0_1_ *) &sccshypcut0_)

/* Initialized data */

struct {
    char e_1[80];
    } sccshypcut0_ = { {'@', '(', '#', ')', 'h', 'y', 'p', 'c', 'u', 't', '0',
	     '.', 'f', '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0',
	    '/', '9', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	    ' ', ' ', ' ', ' ', ' '} };


/* Table of constant values */

static integer c__1 = 1;

/* NAME */
/* 	hypcut0 -- Make an initial location guess or prediction. */
/* FILE */
/* 	hypcut0.f */
/* SYNOPSIS */
/* 	Make a first-cut educated guess at the epicentral location. */
/* DESCRIPTION */
/* 	Subroutine.  This subroutine takes a first-cut at the epicentral */
/* 	location using one of the following techniques, in the order */
/* 	tried, */
/* 	  1.	Attempts to compute an initial location using S-P times */
/* 	      	for the closest station and the best-determined defining */
/* 		azimuth for that station (based on the smallest S.D.). */
/* 	  2.	Attempts to compute an intial location from various combi- */
/* 	  	nations of S-P times and P-wave arrival times.  The */
/* 		searches are preformed in the following order of */
/* 		importance: */
/* 	  	  A.	Uses the 3 least S-P times and finds a common */
/* 			crossing point. */
/* 	  	  B.	Uses the smallest P-wave travel time between the */
/* 			common crossing points of 2 S-P times. */
/* 	  	  C.	Uses a single S-P time to obtain a crude origin */
/* 			time and then find the nearest crossing points */
/* 			from two P-wave arrival times. */
/* 	  3.	Attempts to compute an initial location from various combi- */
/* 		nations of azimuth and P-wave arrival time data.  When the */
/* 		difference between azimuths is < 10 deg., then the inter- */
/* 		section will be poorly determined, and is therefore, */
/* 		ignored.  The searches are preformed in the following order */
/* 		of importance: */
/* 		  A.	Computes the intersection of 2 great circles given */
/* 			2 stations with azimuth data from those points. */
/* 			The location formed using the best-defining */
/* 			azimuths from those stations closest to the */
/* 			location is then chosen. */
/* 		  B.	Minimizes two P-wave arrival times constrained by a */
/* 			single azimuth datum, preferably one with an */
/* 			arrival time. */
/* 	  4.	Attempts to obtain an initial epicentral location based on */
/* 		an approximate minimization procedure using three or more */
/* 		P-wave arrival times as data. */
/* 	  5.	Attempts to compute an initial location using a P slowness */
/* 		and the best-determined azimuth datum at the station with */
/* 		the P-slowness with the smallest standard error between some */
/* 		bounds. */
/* 	  6.	Looks for the closest station based of arrival times and */
/* 		uses an associated azimuth (if available). */
/* 	  7.	Looks for a station with a good slowness (based on the */
/* 		slowness datum with the smallest a priori data standard */
/* 		error) and places the initial location near that station. */
/* 	  8.	It looks for a station with an azimuth and places the */
/* 		initial location near that station at the associated */
/* 		azimuth. */
/* 	---- Indexing ---- */
/*       i = 1, nsta;	j = 1, nwav;	k = 1, ntbd(j);	m = 1, ntbz(j); */
/* 	n = 1, ndata; */
/* 	---- On entry ---- */
/* 	ndata:	Number of data loaded into arrays */
/* 	nsta:	Number of stations */
/* 	nwav:	Number of phases in list */
/* 	maxtbd:	Maximum dimension of k'th position in tbd(), tbtt() */
/* 	maxtbz:	Maximum dimension of m'th position in tbz(), tbtt() */
/* 	staid(i):	List of all acceptible station names */
/* 	stalat(i):	Station latitudes  (deg) */
/* 	stalon(i):	Station longitudes (deg) */
/* 	dobs(n):	Observed data (sec, deg, sec/deg) */
/* 	dsd(n): 	Standard deviations of observed data */
/* 	dwavid(n):	Phase ID's of data */
/* 	ntbd(j):	Number of distance samples in travel-time tables */
/* 	tbd(k,j):	Angular distance (deg) */
/* 	tbtt(k,m,j):	Travel-time (sec) */
/* 	ipsta(n):	Station index of n'th datum */
/* 	ipwav(n):	Wave index of n'th datum */
/* 	idtyp(n):	Data type of n'th datum */
/* 			  = 0, Data type unknown */
/* 			  = 1, Arrival time datum */
/* 			  = 2, Azimuth datum */
/* 			  = 3, Slowness datum */
/* 	iderr(n):	Error code for n'th datum */
/* 			  = 0, Datum OK */
/* 			 != 0, Datum not valid, and not used here */
/* 	atype(n):	Arrival usage */
/* 			  = d: Defining, used in location */
/* 			  = n: Could be defining, but not used in location */
/* 			  = a: Not to be used in location */
/* 	prtflg:		Verbose printout (y = Yes; n = None) */
/* 	---- On return ---- */
/* 	alat0:	First cut at event latitude (deg) */
/* 	alon0:	First cut at event longitude (deg) */
/* 	ierr:	Error flag; */
/* 		  0, No error */
/* 		  1, Too few data to get initial location */
/* 	---- Functions called ---- */
/* 	From libgeog */
/* 		azcros2:	Determine crossing points of 2 great circles */
/* 		crossings:	Determine crossing points of 2 small circles */
/* 		distaz2:	Determine the distance between between two */
/* 				lat./lon. pairs */
/* 		latlon2:	Compute a second lat./lon. from the first, a */
/* 				distance and an azimuth */
/* DIAGNOSTICS */
/* 	Returns if it cannot find an initial educated guess given the */
/* 	above search conditions. */
/* NOTES */
/* 	Currently under-going substanitive additions. */
/* SEE ALSO */

/* AUTHORS */
/* 	Steve Bratt, December 1988; */
/* 	Walter Nagy, July 1990. */
/* Subroutine */ int hypcut0_(staid, stalat, stalon, nsta, dobs, dsd, dwavid,
	ipsta, ipwav, nwav, maxtbd, maxtbz, ntbd, tbd, tbtt, idtyp, iderr,
	atype, ndata, luout, prtflg, alat0, alon0, ierr, staid_len,
	dwavid_len, atype_len, prtflg_len)
char *staid;
real *stalat, *stalon;
integer *nsta;
real *dobs, *dsd;
char *dwavid;
integer *ipsta, *ipwav, *nwav, *maxtbd, *maxtbz, *ntbd;
real *tbd, *tbtt;
integer *idtyp, *iderr;
char *atype;
integer *ndata, *luout;
char *prtflg;
real *alat0, *alon0;
integer *ierr;
ftnlen staid_len;
ftnlen dwavid_len;
ftnlen atype_len;
ftnlen prtflg_len;
{
    /* Initialized data */

    static doublereal distance[19] = { 0.,10.,20.,30.,40.,50.,60.,70.,80.,90.,
	    100.,110.,120.,130.,140.,150.,160.,170.,180. };
    static doublereal slowness[19] = { 19.17,13.7,10.9,8.85,8.3,7.6,6.88,6.15,
	    5.4,4.66,4.44,1.96,1.91,1.88,1.79,1.57,1.14,.59,.01 };
    static doublereal sminusp[12] = { 0.,114.2,226.76,300.,367.49,432.64,
	    494.45,552.32,605.82,654.47,695.75,734.6 };

    /* System generated locals */
    integer tbd_dim1, tbd_offset, tbtt_dim1, tbtt_dim2, tbtt_offset, i__1,
	    i__2;
    doublereal d__1;

    /* Builtin functions */
    integer s_cmp(), s_wsfe(), do_fio(), e_wsfe();
    /* Subroutine */ int s_copy(), s_stop();

    /* Local variables */
    static doublereal bestazcross;
    static logical goodsminusp[9999];
    static doublereal sminusptime, torg;
    static shortint indexdsd[9999];
    static doublereal orderdsd[9999];
    static logical goodazim[9999];
    static doublereal fmaxtime, bestazim[9999], delcross, smallest;
    static integer itimeyet;
    static doublereal crosslat[12];
    static logical goodslow[9999];
    static doublereal bestslow[9999], crosslon[12], dist1, dist2;
    static integer isminusp;
    static shortint indexdsd2[9999];
    static doublereal orderdsd2[9999];
    static integer i__, j, k, n;
    static shortint indexsminusp[9999];
    static doublereal tcalc, ordersminusp[9999], delta;
    static integer icerr, iazim;
    static char wavid[2];
    static shortint iwave[9999];
    static doublereal a1, a2;
    static integer i1, i2, ierrx, islow, j1, j2, n1, n2;
    static doublereal sheartime[9999];
    static logical goodcompr[9999];
    static doublereal comprtime[9999], alat0x;
    extern /* Subroutine */ int crossings_();
    static doublereal alon0x, dist1x, dist2x;
    extern integer lnblnk_();
    static doublereal azimsd[9999];
    static integer icompr, ic2;
    static doublereal azisav;
    static integer icross;
    static doublereal slowsd[9999];
    static integer i1s, i2s, i3s;
    extern /* Subroutine */ int prtcut_();
    static shortint indexcompr[9999];
    static doublereal ordercompr[9999];
    extern /* Subroutine */ int latlon2_(), distaz2_(), azcros2_();
    static doublereal baz, dis[9999], azi, tmp;
    static integer iusesta;
    static doublereal useazim, sta1, sta2, res1, res2, sta3, sta4, useslow;

    /* Fortran I/O blocks */
    static cilist io___4 = { 0, 0, 0, "(/a/a)", 0 };
    static cilist io___41 = { 0, 0, 0, "(a/2a,3(/a,f7.2,a))", 0 };
    static cilist io___67 = { 0, 0, 0, "(a/4a/a,3f7.2,a)", 0 };
    static cilist io___70 = { 0, 0, 0, "(a/4a/a,2f7.2,a)", 0 };
    static cilist io___73 = { 0, 0, 0, "(a/4a/a,3f7.2,a)", 0 };
    static cilist io___82 = { 0, 0, 0, "(a/3a,2(/a,2f7.2,a))", 0 };
    static cilist io___84 = { 0, 0, 0, "(a/2a,3(/a,f7.2,a))", 0 };
    static cilist io___85 = { 0, 0, 0, "(a/2a,3(/a,f7.2,a))", 0 };
    static cilist io___86 = { 0, 0, 0, "(a/2a,3(/a,f7.2,a))", 0 };
    static cilist io___87 = { 0, 0, 0, "(a/2a,3(/a,f7.2,a))", 0 };
    static cilist io___88 = { 0, 0, 0, "(a/a/)", 0 };


/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- Parameter declarations ---- */
/*     Maximum azimuth setting (deg.) for initialization purposes */
/*     Maximum allowable stations */
/*     Convert degrees to radians */
/*     Default distance (deg.) from the "guessed" event to the station */
/*     Default azimuth (deg.) from the "guessed" event to the station */
/*     Maximum permissable elements for distance(), slowness() arrays */
/*     Maximum cross-azimuth setting (deg.) for initialization purposes */
/*     Maximum S-P time setting (sec.) for initialization purposes */
/*     Minimum allowable slowness */
/*     Maximum allowable slowness */
/*     ---- On entry ---- */
/*     ---- On return ---- */
/*     ---- Internal variables ---- */
/*     Slowness and S-P time as a function of distance for a surface event */
/*     Taken from IASPEI Seismological Tables of B.L.N. Kennett (1991) */
    /* Parameter adjustments */
    --stalon;
    --stalat;
    staid -= staid_len;
    --ntbd;
    tbd_dim1 = *maxtbd;
    tbd_offset = 1 + tbd_dim1 * 1;
    tbd -= tbd_offset;
    tbtt_dim1 = *maxtbd;
    tbtt_dim2 = *maxtbz;
    tbtt_offset = 1 + tbtt_dim1 * (1 + tbtt_dim2 * 1);
    tbtt -= tbtt_offset;
    atype -= atype_len;
    --iderr;
    --idtyp;
    --ipwav;
    --ipsta;
    dwavid -= dwavid_len;
    --dsd;
    --dobs;

    /* Function Body */
    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	io___4.ciunit = *luout;
	s_wsfe(&io___4);
	do_fio(&c__1, " hypcut0: General initial location information :", (
		ftnlen)48);
	do_fio(&c__1, " ===============================================", (
		ftnlen)48);
	e_wsfe();
    }
/*     Local initializations */
    i__1 = *nsta;
    for (i__ = 1; i__ <= i__1; ++i__) {
	azimsd[i__ - 1] = 40.;
	slowsd[i__ - 1] = 19.16;
	bestslow[i__ - 1] = (float)-888888.;
	bestazim[i__ - 1] = (float)-888888.;
	goodcompr[i__ - 1] = FALSE_;
	goodsminusp[i__ - 1] = FALSE_;
	goodazim[i__ - 1] = FALSE_;
	goodslow[i__ - 1] = FALSE_;
/* L1000: */
    }
    iusesta = 0;
    *ierr = 0;
    itimeyet = 0;
    bestazcross = 80.;
    sminusptime = 690.;
    fmaxtime = (float)888888.;
/*     Load valid P-wave and S-wave arrival times, P-slownesses and azimuths */
/*     (use P-type phase or phase with the smallest azimuth standard */
/*     deviation) at each station. */
    i__1 = *ndata;
    for (n = 1; n <= i__1; ++n) {
	if (iderr[n] == 0 && *(unsigned char *)&atype[n * atype_len] == 'd') {
	    i__ = ipsta[n];
	    s_copy(wavid, dwavid + n * dwavid_len, (ftnlen)2, dwavid_len);
	    k = lnblnk_(wavid, (ftnlen)2);
/*           ---- Arrival times ---- */
	    if (idtyp[n] == 1) {
		if (itimeyet == 0) {
		    fmaxtime = dobs[n] + (float)888888.;
		    i__2 = *nsta;
		    for (j = 1; j <= i__2; ++j) {
			comprtime[j - 1] = fmaxtime;
			sheartime[j - 1] = fmaxtime;
/* L1010: */
		    }
		    itimeyet = 1;
		}
/*              Load valid S and P times into arrays for each i'th station */
		if ((s_cmp(wavid, "P ", k, (ftnlen)2) == 0 || s_cmp(wavid,
			"Pn", k, (ftnlen)2) == 0 || s_cmp(wavid, "Pg", k, (
			ftnlen)2) == 0 || s_cmp(wavid, "Pb", k, (ftnlen)2) ==
			0) && dobs[n] < comprtime[i__ - 1]) {
		    comprtime[i__ - 1] = dobs[n];
		    iwave[i__ - 1] = (shortint) ipwav[n];
		} else if ((s_cmp(wavid, "S ", k, (ftnlen)2) == 0 || s_cmp(
			wavid, "Sn", k, (ftnlen)2) == 0 || s_cmp(wavid, "Sb",
			k, (ftnlen)2) == 0 || s_cmp(wavid, "Lg", k, (ftnlen)2)
			 == 0 || s_cmp(wavid, "Sg", k, (ftnlen)2) == 0) &&
			dobs[n] < sheartime[i__ - 1]) {
		    sheartime[i__ - 1] = dobs[n];
		}
/*           ---- Azimuths ---- */
	    } else if (idtyp[n] == 2 && dsd[n] > (float)0.) {
		if (dsd[n] < azimsd[i__ - 1]) {
		    azimsd[i__ - 1] = dsd[n];
		    bestazim[i__ - 1] = dobs[n];
		}
/*           ---- Slownesses ---- */
	    } else if (idtyp[n] == 3) {
		if (*(unsigned char *)wavid == 'P' && dsd[n] > (float)0. &&
			dobs[n] > .02 && dobs[n] < 19.16) {
		    if (dsd[n] < slowsd[i__ - 1]) {
			slowsd[i__ - 1] = dsd[n];
			bestslow[i__ - 1] = dobs[n];
		    }
		}
	    }
	}
/* L1020: */
    }
/*     Define good (valid) P-wave and S-wave arrival times, S-P times, */
/*     azimuths and slownesses.  Load these valid data in the order() */
/*     array and save their station index in the index() array. */
    icompr = 0;
    isminusp = 0;
    iazim = 0;
    islow = 0;
    i__1 = *nsta;
    for (i__ = 1; i__ <= i__1; ++i__) {
	goodcompr[i__ - 1] = comprtime[i__ - 1] < fmaxtime;
	if (goodcompr[i__ - 1]) {
	    ++icompr;
	    indexcompr[icompr - 1] = (shortint) i__;
	    ordercompr[icompr - 1] = comprtime[i__ - 1];
	}
	goodsminusp[i__ - 1] = goodcompr[i__ - 1] && sheartime[i__ - 1] <
		fmaxtime && sheartime[i__ - 1] > comprtime[i__ - 1];
	if (goodsminusp[i__ - 1]) {
	    ++isminusp;
	    indexsminusp[isminusp - 1] = (shortint) i__;
	    ordersminusp[isminusp - 1] = sheartime[i__ - 1] - comprtime[i__ -
		    1];
	}
	goodazim[i__ - 1] = bestazim[i__ - 1] >= (float)-180. && bestazim[i__
		- 1] <= (float)360.;
	if (goodazim[i__ - 1]) {
	    ++iazim;
	    indexdsd[iazim - 1] = (shortint) i__;
	    orderdsd[iazim - 1] = azimsd[i__ - 1];
	}
	goodslow[i__ - 1] = bestslow[i__ - 1] > .02 && bestslow[i__ - 1] <
		19.16;
	if (goodslow[i__ - 1]) {
	    ++islow;
	    indexdsd2[islow - 1] = (shortint) i__;
	    orderdsd2[islow - 1] = slowsd[i__ - 1];
	}
/* L1030: */
    }
/*     Sort P-wave arrival times in descending order (i.e., earliest */
/*     arrival-times first) */
    if (icompr > 0) {
	for (i__ = (icompr + 1) / 2; i__ >= 1; --i__) {
	    i__1 = icompr - i__;
	    for (j = 1; j <= i__1; ++j) {
		if (ordercompr[j - 1] > ordercompr[j + i__ - 1]) {
		    tmp = ordercompr[j - 1];
		    n = indexcompr[j - 1];
		    ordercompr[j - 1] = ordercompr[j + i__ - 1];
		    indexcompr[j - 1] = indexcompr[j + i__ - 1];
		    ordercompr[j + i__ - 1] = tmp;
		    indexcompr[j + i__ - 1] = (shortint) n;
		}
/* L1040: */
	    }
	}
    }
/*     Sort S-P times in descending order (i.e., earliest times first) */
    if (isminusp > 0) {
	for (i__ = (isminusp + 1) / 2; i__ >= 1; --i__) {
	    i__1 = isminusp - i__;
	    for (j = 1; j <= i__1; ++j) {
		if (ordersminusp[j - 1] > ordersminusp[j + i__ - 1]) {
		    tmp = ordersminusp[j - 1];
		    n = indexsminusp[j - 1];
		    ordersminusp[j - 1] = ordersminusp[j + i__ - 1];
		    indexsminusp[j - 1] = indexsminusp[j + i__ - 1];
		    ordersminusp[j + i__ - 1] = tmp;
		    indexsminusp[j + i__ - 1] = (shortint) n;
		}
/* L1050: */
	    }
	}
    }
/*     Sort azimuths according to their data standard errors in increasing */
/*     order (i.e., smallest azimuthal standard errors first) */
    if (iazim > 0) {
	for (i__ = (iazim + 1) / 2; i__ >= 1; --i__) {
	    i__1 = iazim - i__;
	    for (j = 1; j <= i__1; ++j) {
		if (orderdsd[j - 1] > orderdsd[j + i__ - 1]) {
		    tmp = orderdsd[j - 1];
		    n = indexdsd[j - 1];
		    orderdsd[j - 1] = orderdsd[j + i__ - 1];
		    indexdsd[j - 1] = indexdsd[j + i__ - 1];
		    orderdsd[j + i__ - 1] = tmp;
		    indexdsd[j + i__ - 1] = (shortint) n;
		}
/* L1060: */
	    }
	}
    }
/*     Sort slownesses in increasing order (i.e., largest slowness is */
/*     nearest the event and often can be quite diagnostic) */
    if (islow > 0) {
	for (i__ = (islow + 1) / 2; i__ >= 1; --i__) {
	    i__1 = islow - i__;
	    for (j = 1; j <= i__1; ++j) {
		if (orderdsd2[j - 1] > orderdsd2[j + i__ - 1]) {
		    tmp = orderdsd2[j - 1];
		    n = indexdsd2[j - 1];
		    orderdsd2[j - 1] = orderdsd2[j + i__ - 1];
		    indexdsd2[j - 1] = indexdsd2[j + i__ - 1];
		    orderdsd2[j + i__ - 1] = tmp;
		    indexdsd2[j + i__ - 1] = (shortint) n;
		}
/* L1070: */
	    }
	}
    }
/*     Find closest station with smallest S-P time and an azimuth.  Compute */
/*     the location from the S-P time and azimuth. */
/*     First and preferred search procedure ! */
    if (isminusp < 1) {
	goto L1280;
    }
    i__1 = isminusp;
    for (i__ = 1; i__ <= i__1; ++i__) {
	n = indexsminusp[i__ - 1];
	if (goodazim[n - 1]) {
	    sminusptime = ordersminusp[i__ - 1];
/*           Interpolate slowness to get distance. */
	    for (j = 1; j <= 11; ++j) {
		if (sminusptime > sminusp[j - 1] && sminusptime <= sminusp[j])
			 {
		    dis[0] = (distance[j] - distance[j - 1]) * (sminusptime -
			    sminusp[j - 1]) / (sminusp[j] - sminusp[j - 1]) +
			    distance[j - 1];
		    useazim = bestazim[n - 1];
		    iusesta = n;
		    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
			io___41.ciunit = *luout;
			s_wsfe(&io___41);
			do_fio(&c__1, "    Method: S-P time w/ azimuth at 1 \
station", (ftnlen)44);
			do_fio(&c__1, "   Station: ", (ftnlen)12);
			do_fio(&c__1, staid + iusesta * staid_len, staid_len);
			do_fio(&c__1, "  S-P time: ", (ftnlen)12);
			do_fio(&c__1, (char *)&sminusptime, (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, " sec.", (ftnlen)5);
			do_fio(&c__1, "  Distance: ", (ftnlen)12);
			do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, " deg.", (ftnlen)5);
			do_fio(&c__1, "   Azimuth: ", (ftnlen)12);
			do_fio(&c__1, (char *)&useazim, (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, " deg.", (ftnlen)5);
			e_wsfe();
		    }
/*                 Done.  Let's go find a lat./lon. pair ! */
		    sta1 = stalat[iusesta];
		    sta2 = stalon[iusesta];
		    latlon2_(&sta1, &sta2, dis, &useazim, &a1, &a2);
		    *alat0 = a1;
		    *alon0 = a2;
		    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
			prtcut_(alat0, alon0, luout);
		    }
		    return 0;
		}
/* L1080: */
	    }
	}
/* L1090: */
    }
/*     Look here for multiple S-P times and compute distances */
    if (icompr < 2) {
	goto L1280;
    }
    i__1 = isminusp;
    for (i__ = 1; i__ <= i__1; ++i__) {
	sminusptime = ordersminusp[i__ - 1];
/*        Interpolate slowness(es) to get distance(s). */
	for (j = 1; j <= 11; ++j) {
	    if (sminusptime > sminusp[j - 1] && sminusptime <= sminusp[j]) {
		dis[i__ - 1] = (distance[j] - distance[j - 1]) * (sminusptime
			- sminusp[j - 1]) / (sminusp[j] - sminusp[j - 1]) +
			distance[j - 1];
	    }
/* L1100: */
	}
/* L1110: */
    }
/*     Compute the approximate origin time */
    j = indexsminusp[0];
    n = iwave[j - 1];
/*     call find_ttime (n-1, dis(1), tcalc, iterr) */
/*     if (iterr.gt.0) goto 1130 */
/*     torg = ordercompr(j) - tcalc */
    if (tbd[n * tbd_dim1 + 1] > dis[0]) {
	goto L1130;
    }
    i__1 = ntbd[n];
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (tbd[i__ + n * tbd_dim1] > dis[0]) {
	    tmp = (dis[0] - tbd[i__ - 1 + n * tbd_dim1]) / (tbd[i__ + n *
		    tbd_dim1] - tbd[i__ - 1 + n * tbd_dim1]);
	    tcalc = tbtt[i__ - 1 + (n * tbtt_dim2 + 1) * tbtt_dim1] + tmp * (
		    tbtt[i__ + (n * tbtt_dim2 + 1) * tbtt_dim1] - tbtt[i__ -
		    1 + (n * tbtt_dim2 + 1) * tbtt_dim1]);
	    torg = ordercompr[j - 1] - tcalc;
	    goto L1130;
	}
/* L1120: */
    }
/*     Determine the necessary crossing points */
L1130:
    icross = 0;
    icerr = 0;
    if (isminusp > 1) {
	i__1 = isminusp;
	for (i1 = 2; i1 <= i__1; ++i1) {
	    i__2 = i1 - 1;
	    for (i2 = 1; i2 <= i__2; ++i2) {
		n1 = indexsminusp[i1 - 1];
		n2 = indexsminusp[i2 - 1];
		++icross;
		ic2 = icross << 1;
		sta1 = stalat[n2];
		sta2 = stalon[n2];
		sta3 = stalat[n1];
		sta4 = stalon[n1];
		crossings_(&sta1, &sta2, &sta3, &sta4, &dis[i2 - 1], &dis[i1
			- 1], &crosslat[ic2 - 2], &crosslon[ic2 - 2], &
			crosslat[ic2 - 1], &crosslon[ic2 - 1], &icerr);
		if (icerr > 0) {
		    --icross;
		    goto L1150;
		}
		if (icross > 1) {
		    if (n2 != i1s && n2 != i2s) {
			i2s = n2;
		    } else {
			i2s = n1;
		    }
		} else {
		    i1s = n2;
		    i3s = n1;
		}
/*              Find the best crossing from 3 S-P times ? */
		if (icross > 1) {
			smallest = (float)888888.;
			for (i__ = 1; i__ <= 2; ++i__) {
				for (j = 3; j <= 4; ++j) {
					distaz2_(&crosslat[i__ - 1], &crosslon[i__ - 1],
					         &crosslat[j - 1], &crosslon[j - 1],
					         &delcross, &azi, &baz);
					if (delcross < smallest) {
						smallest = delcross;
						azisav = azi;
	/*                          K.S. 1-Dec-97, abort here because of illegal code */
						//s_stop("*** K.S. ill code in hypcut0.f ***", (ftnlen)34);
	/*                           i1       = i */
	/*                           i2       = j */
						*ierr = 1;
						return 0;
					}
	/* L1140: */
				}
			}
			d__1 = smallest / (float)2.;
			latlon2_(&crosslat[i1 - 1], &crosslon[i1 - 1], &d__1,
			         &azisav, &a1, &a2);
			*alat0 = a1;
			*alon0 = a2;
			if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
			io___67.ciunit = *luout;
			s_wsfe(&io___67);
			do_fio(&c__1, "    Method: Nearest crossing of 3 S-P\
 times", (ftnlen)43);
			do_fio(&c__1, "  Stations: ", (ftnlen)12);
			do_fio(&c__1, staid + i1s * staid_len, staid_len);
			do_fio(&c__1, staid + i2s * staid_len, staid_len);
			do_fio(&c__1, staid + i3s * staid_len, staid_len);
			do_fio(&c__1, " Distances: ", (ftnlen)12);
			do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, (char *)&dis[1], (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, (char *)&dis[2], (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, " deg.", (ftnlen)5);
			e_wsfe();
		    }
/*                 Done.  Exit routine. */
		    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
			prtcut_(alat0, alon0, luout);
		    }
		    return 0;
		}
L1150:
		;
	    }
/* L1160: */
	}
	if (icross == 0 || icompr < 3) {
	    goto L1230;
	}
/*        Use 2 S-P times and the shortest independent arrival time */
/*        to determine initial location */
	i__1 = icompr;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    j = indexcompr[i__ - 1];
	    if (j != indexsminusp[0] && j != indexsminusp[1]) {
		goto L1180;
	    }
/* L1170: */
	}
L1180:
	n = iwave[j - 1];
/*        Calculate theoretical travel times */
	sta1 = stalat[j];
	sta2 = stalon[j];
	distaz2_(&sta1, &sta2, crosslat, crosslon, dis, &azi, &baz);
/*        call find_ttime (n-1, dis(1), tcalc, iterr) */
/*        res1 = abs(comprtime(j) - torg - tcalc) */
	i__1 = ntbd[n];
	for (i__ = 1; i__ <= i__1; ++i__) {
	    if (tbd[i__ + n * tbd_dim1] > dis[0]) {
		tmp = (dis[0] - tbd[i__ - 1 + n * tbd_dim1]) / (tbd[i__ + n *
			tbd_dim1] - tbd[i__ - 1 + n * tbd_dim1]);
		tcalc = tbtt[i__ - 1 + (n * tbtt_dim2 + 1) * tbtt_dim1] + tmp
			* (tbtt[i__ + (n * tbtt_dim2 + 1) * tbtt_dim1] - tbtt[
			i__ - 1 + (n * tbtt_dim2 + 1) * tbtt_dim1]);
		res1 = (d__1 = comprtime[j - 1] - torg - tcalc, abs(d__1));
		goto L1200;
	    }
/* L1190: */
	}
L1200:
	distaz2_(&sta1, &sta2, &crosslat[1], &crosslon[1], &dis[1], &azi, &
		baz);
/*        call find_ttime (n-1, dis(2), tcalc, iterr) */
/*        res2 = abs(comprtime(j) - torg - tcalc) */
	i__1 = ntbd[n];
	for (i__ = 1; i__ <= i__1; ++i__) {
	    if (tbd[i__ + n * tbd_dim1] > dis[1]) {
		tmp = (dis[1] - tbd[i__ - 1 + n * tbd_dim1]) / (tbd[i__ + n *
			tbd_dim1] - tbd[i__ - 1 + n * tbd_dim1]);
		tcalc = tbtt[i__ - 1 + (n * tbtt_dim2 + 1) * tbtt_dim1] + tmp
			* (tbtt[i__ + (n * tbtt_dim2 + 1) * tbtt_dim1] - tbtt[
			i__ - 1 + (n * tbtt_dim2 + 1) * tbtt_dim1]);
		res2 = (d__1 = comprtime[j - 1] - torg - tcalc, abs(d__1));
		goto L1220;
	    }
/* L1210: */
	}
/*        Choose travel time with the smallest residual */
L1220:
	if (res1 < res2) {
	    *alat0 = crosslat[0];
	    *alon0 = crosslon[0];
	} else {
	    *alat0 = crosslat[1];
	    *alon0 = crosslon[1];
	}
	if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	    io___70.ciunit = *luout;
	    s_wsfe(&io___70);
	    do_fio(&c__1, "    Method: S-P crossing and nearest arrival time",
		     (ftnlen)49);
	    do_fio(&c__1, "  Stations: ", (ftnlen)12);
	    do_fio(&c__1, staid + i1s * staid_len, staid_len);
	    do_fio(&c__1, staid + i2s * staid_len, staid_len);
	    do_fio(&c__1, staid + j * staid_len, staid_len);
	    do_fio(&c__1, " Distances: ", (ftnlen)12);
	    do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, (char *)&dis[1], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    e_wsfe();
	}
/*        Done.  Exit routine. */
	if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	    prtcut_(alat0, alon0, luout);
	}
	return 0;
    } else {
/*        Determine initial location using 1 S-P time and 2 nearest */
/*        independent arrival times */
L1230:
	icross = 0;
	n1 = indexsminusp[0];
	i__1 = icompr;
	for (k = 1; k <= i__1; ++k) {
	    j = indexcompr[k - 1];
	    if (j == n1) {
		goto L1270;
	    }
	    ++icross;
	    n = iwave[j - 1];
/*           Calculate distance from station to the origin and then */
/*           find the crossing points */
	    tcalc = ordercompr[k - 1] - torg;
/*           call find_dist (n-1, tcalc, dist1, iterr) */
/*           if (iterr.gt.0) goto 1270 */
	    if (tbtt[(n * tbtt_dim2 + 1) * tbtt_dim1 + 1] > tcalc) {
		goto L1270;
	    }
	    i__2 = ntbd[n];
	    for (i__ = 1; i__ <= i__2; ++i__) {
		if (tbtt[i__ + (n * tbtt_dim2 + 1) * tbtt_dim1] > tcalc) {
		    tmp = (tcalc - tbtt[i__ - 1 + (n * tbtt_dim2 + 1) *
			    tbtt_dim1]) / (tbtt[i__ + (n * tbtt_dim2 + 1) *
			    tbtt_dim1] - tbtt[i__ - 1 + (n * tbtt_dim2 + 1) *
			    tbtt_dim1]);
		    dist1 = tbd[i__ - 1 + n * tbd_dim1] + tmp * (tbd[i__ + n *
			     tbd_dim1] - tbd[i__ - 1 + n * tbd_dim1]);
		    goto L1250;
		}
/* L1240: */
	    }
L1250:
	    icerr = 0;
	    ic2 = icross << 1;
	    sta1 = stalat[n1];
	    sta2 = stalon[n1];
	    sta3 = stalat[j];
	    sta4 = stalon[j];
	    crossings_(&sta1, &sta2, &sta3, &sta4, dis, &dist1, &crosslat[ic2
		    - 2], &crosslon[ic2 - 2], &crosslat[ic2 - 1], &crosslon[
		    ic2 - 1], &icerr);
	    if (icerr < 1) {
		dis[icross] = dist1;
		if (icross > 1) {
		    j2 = j;
		    smallest = (float)888888.;
		    for (i__ = 1; i__ <= 2; ++i__) {
			for (j = 3; j <= 4; ++j) {
			    distaz2_(&crosslat[i__ - 1], &crosslon[i__ - 1], &
				    crosslat[j - 1], &crosslon[j - 1], &
				    delcross, &azi, &baz);
			    if (delcross < smallest) {
				smallest = delcross;
				azisav = azi;
				i1 = i__;
				i2 = j;
			    }
/* L1260: */
			}
		    }
		    d__1 = smallest / (float)2.;
		    latlon2_(&crosslat[i1 - 1], &crosslon[i1 - 1], &d__1, &
			    azisav, &a1, &a2);
		    *alat0 = a1;
		    *alon0 = a2;
		    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
			io___73.ciunit = *luout;
			s_wsfe(&io___73);
			do_fio(&c__1, "    Method: Nearest crossing of 1 S-P\
 & 2 P- times", (ftnlen)50);
			do_fio(&c__1, "  Stations: ", (ftnlen)12);
			do_fio(&c__1, staid + n1 * staid_len, staid_len);
			do_fio(&c__1, staid + j1 * staid_len, staid_len);
			do_fio(&c__1, staid + j2 * staid_len, staid_len);
			do_fio(&c__1, " Distances: ", (ftnlen)12);
			do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, (char *)&dis[1], (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, (char *)&dis[2], (ftnlen)sizeof(
				doublereal));
			do_fio(&c__1, " deg.", (ftnlen)5);
			e_wsfe();
		    }
/*                 Done!  Exit routine! */
		    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
			prtcut_(alat0, alon0, luout);
		    }
		    return 0;
		} else {
		    j1 = j;
		}
	    } else {
		--icross;
	    }
L1270:
	    ;
	}
    }
/*     Find crossing point of 2 defining azimuths.  Take the location */
/*     that is closest (on average) to the 2 stations */
/*     Second and next important search procedure ! */
L1280:
    if (iazim > 1) {
	i__1 = iazim;
	for (i1 = 2; i1 <= i__1; ++i1) {
	    i__2 = i1 - 1;
	    for (i2 = 1; i2 <= i__2; ++i2) {
		n1 = indexdsd[i1 - 1];
		n2 = indexdsd[i2 - 1];
/*              If the difference between 2 azimuths is less than 10 deg., */
/*              then the confidence one might put into the computed */
/*              crossing point would be quite uncertain, so ignore! */
		tmp = (d__1 = bestazim[n1 - 1] - bestazim[n2 - 1], abs(d__1));
		if (tmp > (float)350.) {
		    tmp = (float)360. - tmp;
		}
		if (tmp < (float)10.) {
		    goto L1290;
		}
		sta1 = stalat[n1];
		sta2 = stalon[n1];
		sta3 = stalat[n2];
		sta4 = stalon[n2];
		azcros2_(&sta1, &sta2, &bestazim[n1 - 1], &sta3, &sta4, &
			bestazim[n2 - 1], &dist1x, &dist2x, &alat0x, &alon0x,
			&ierrx);
		if (ierrx == 0) {
		    delta = (dist1x + dist2x) * (float).5;
/*                 dsdwt = 1.0/dsd(n1)*dsd(n2) */
/*                 delwt = 1.0/delta */
/*                 totwt = dsdwt*delwt */
		    if (delta < bestazcross) {
			bestazcross = delta;
			i1s = n1;
			i2s = n2;
			*alat0 = alat0x;
			*alon0 = alon0x;
			dist1 = dist1x;
			dist2 = dist2x;
		    }
		}
L1290:
		;
	    }
/* L1300: */
	}
	if (bestazcross < 80.) {
	    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
		io___82.ciunit = *luout;
		s_wsfe(&io___82);
		do_fio(&c__1, "    Method: Crossing of azimuths from 2 stati\
ons", (ftnlen)48);
		do_fio(&c__1, "  Stations: ", (ftnlen)12);
		do_fio(&c__1, staid + i1s * staid_len, staid_len);
		do_fio(&c__1, staid + i2s * staid_len, staid_len);
		do_fio(&c__1, "  Azimuths: ", (ftnlen)12);
		do_fio(&c__1, (char *)&bestazim[i1s - 1], (ftnlen)sizeof(
			doublereal));
		do_fio(&c__1, (char *)&bestazim[i2s - 1], (ftnlen)sizeof(
			doublereal));
		do_fio(&c__1, " deg.", (ftnlen)5);
		do_fio(&c__1, " Distances: ", (ftnlen)12);
		do_fio(&c__1, (char *)&dist1, (ftnlen)sizeof(doublereal));
		do_fio(&c__1, (char *)&dist2, (ftnlen)sizeof(doublereal));
		do_fio(&c__1, " deg.", (ftnlen)5);
		e_wsfe();
	    }
/*           Done!  Exit routine! */
	    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
		prtcut_(alat0, alon0, luout);
	    }
	    return 0;
	}
    }
/*     Look here for 1 azimuth and the 2 closest arrvial times */
/*     if (iazim.eq.1 .and. icompr.gt.1) then */
/*        n1 = indexdsd(1) */
/*        Preferably with an arrival time from the azimuth datum */
/*        do 1310 i = 1, icompr */
/*           if (n1.eq.indexcompr(i)) then */

/*           end if */
/* 1310    continue */
/*     end if */
/*     Find station with slowness and azimuth, then compute the location */
/*     from these data */
    if (islow > 0 && iazim > 0) {
	iusesta = indexdsd2[0];
	useslow = bestslow[iusesta - 1];
	useazim = bestazim[iusesta - 1];
/*        do 1320  i = 1, nsta */
/*           if (goodslow(i) .and. goodazim(i) .and. */
/*    &          bestslow(i).gt.useslow) then */
/*              useslow = bestslow(i) */
/*              useazim  = bestazim(i) */
/*              iusesta  = i */
/*           end if */
/* 1320    continue */
	for (j = 1; j <= 18; ++j) {
	    if (useslow < slowness[j - 1] && useslow >= slowness[j]) {
		dis[0] = (distance[j] - distance[j - 1]) * (useslow -
			slowness[j - 1]) / (slowness[j] - slowness[j - 1]) +
			distance[j - 1];
		if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
		    io___84.ciunit = *luout;
		    s_wsfe(&io___84);
		    do_fio(&c__1, "    Method: P slowness with azimuth at 1 \
station", (ftnlen)48);
		    do_fio(&c__1, "   Station: ", (ftnlen)12);
		    do_fio(&c__1, staid + iusesta * staid_len, staid_len);
		    do_fio(&c__1, "  Slowness: ", (ftnlen)12);
		    do_fio(&c__1, (char *)&useslow, (ftnlen)sizeof(doublereal)
			    );
		    do_fio(&c__1, " sec./deg.", (ftnlen)10);
		    do_fio(&c__1, "  Distance: ", (ftnlen)12);
		    do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(doublereal))
			    ;
		    do_fio(&c__1, " deg.", (ftnlen)5);
		    do_fio(&c__1, "   Azimuth: ", (ftnlen)12);
		    do_fio(&c__1, (char *)&useazim, (ftnlen)sizeof(doublereal)
			    );
		    do_fio(&c__1, " deg.", (ftnlen)5);
		    e_wsfe();
		}
/*              Done.  Let's go find a lat./lon. pair ! */
		sta1 = stalat[iusesta];
		sta2 = stalon[iusesta];
		latlon2_(&sta1, &sta2, dis, &useazim, &a1, &a2);
		*alat0 = a1;
		*alon0 = a2;
		if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
		    prtcut_(alat0, alon0, luout);
		}
		return 0;
	    }
/* L1330: */
	}
    }
/*     # Look here for 3 closest arrvial times */
/*     Use point near station with earlist arrival time.  Use the azimuth */
/*     at that station, if there is one.  Probably will become unneccesary! */
    if (icompr > 0) {
	iusesta = indexcompr[0];
	dis[0] = 5.;
	if (goodazim[iusesta - 1]) {
	    useazim = bestazim[iusesta - 1];
	} else {
	    useazim = 0.;
	}
	if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	    io___85.ciunit = *luout;
	    s_wsfe(&io___85);
	    do_fio(&c__1, "        Method: Station with earliest arrival time"
		    , (ftnlen)50);
	    do_fio(&c__1, "       Station: ", (ftnlen)16);
	    do_fio(&c__1, staid + iusesta * staid_len, staid_len);
	    do_fio(&c__1, " Earliest time: ", (ftnlen)16);
	    do_fio(&c__1, (char *)&ordercompr[0], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " sec.", (ftnlen)5);
	    do_fio(&c__1, "      Distance: ", (ftnlen)16);
	    do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, "       Azimuth: ", (ftnlen)16);
	    do_fio(&c__1, (char *)&useazim, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    e_wsfe();
	}
/*     Use best determied azimuth */
    } else if (iazim > 0) {
	iusesta = indexdsd[0];
	dis[0] = 5.;
	if (goodazim[iusesta - 1]) {
	    useazim = bestazim[iusesta - 1];
	} else {
	    useazim = 0.;
	}
	if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	    io___86.ciunit = *luout;
	    s_wsfe(&io___86);
	    do_fio(&c__1, "    Method: Station with smallest azimuth S.D.", (
		    ftnlen)46);
	    do_fio(&c__1, "   Station: ", (ftnlen)12);
	    do_fio(&c__1, staid + iusesta * staid_len, staid_len);
	    do_fio(&c__1, "  Distance: ", (ftnlen)12);
	    do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, "   Azimuth: ", (ftnlen)12);
	    do_fio(&c__1, (char *)&useazim, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, " Best S.D.: ", (ftnlen)12);
	    do_fio(&c__1, (char *)&bestazim[iusesta - 1], (ftnlen)sizeof(
		    doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    e_wsfe();
	}
/*     Finally try station with the best slowness (as defined by the */
/*     smallest s.d.) and the default azimuth */
    } else if (islow > 0) {
	iusesta = indexdsd2[0];
	useslow = bestslow[iusesta - 1];
	for (j = 1; j <= 18; ++j) {
	    if (useslow < slowness[j - 1] && useslow >= slowness[j]) {
		dis[0] = (distance[j] - distance[j - 1]) * (useslow -
			slowness[j - 1]) / (slowness[j] - slowness[j - 1]) +
			distance[j - 1];
	    }
/* L1340: */
	}
	useazim = 0.;
	if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	    io___87.ciunit = *luout;
	    s_wsfe(&io___87);
	    do_fio(&c__1, "    Method: Station with largest slowness", (
		    ftnlen)41);
	    do_fio(&c__1, "   Station: ", (ftnlen)12);
	    do_fio(&c__1, staid + iusesta * staid_len, staid_len);
	    do_fio(&c__1, "  Slowness: ", (ftnlen)12);
	    do_fio(&c__1, (char *)&useslow, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " sec./deg.", (ftnlen)10);
	    do_fio(&c__1, "  Distance: ", (ftnlen)12);
	    do_fio(&c__1, (char *)&dis[0], (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    do_fio(&c__1, "   Azimuth: ", (ftnlen)12);
	    do_fio(&c__1, (char *)&useazim, (ftnlen)sizeof(doublereal));
	    do_fio(&c__1, " deg.", (ftnlen)5);
	    e_wsfe();
	}
/*     Bail out! */
    } else {
	if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	    io___88.ciunit = *luout;
	    s_wsfe(&io___88);
	    do_fio(&c__1, " Initial location procedure failed - Bailing out",
		    (ftnlen)48);
	    do_fio(&c__1, " ===============================================",
		    (ftnlen)48);
	    e_wsfe();
	}
	*ierr = 1;
	return 0;
    }
/*     Done ! */
    sta1 = stalat[iusesta];
    sta2 = stalon[iusesta];
    latlon2_(&sta1, &sta2, dis, &useazim, &a1, &a2);
    *alat0 = a1;
    *alon0 = a2;
    if (s_cmp(prtflg, "y", prtflg_len, (ftnlen)1) == 0) {
	prtcut_(alat0, alon0, luout);
    }
    return 0;
} /* hypcut0_ */

/* Subroutine */ int prtcut_(alat, alon, luout)
real *alat, *alon;
integer *luout;
{
    /* System generated locals */
    real r__1, r__2;

    /* Builtin functions */
    /* Subroutine */ int s_copy();
    integer s_wsfe(), do_fio(), e_wsfe();

    /* Local variables */
    static char ew[2], ns[2];

    /* Fortran I/O blocks */
    static cilist io___91 = { 0, 0, 0, "(2(a,f7.2,2a/),a/)", 0 };


/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
    s_copy(ew, " E", (ftnlen)2, (ftnlen)2);
    s_copy(ns, " N", (ftnlen)2, (ftnlen)2);
    if (*alon < (float)0.) {
	s_copy(ew, " W", (ftnlen)2, (ftnlen)2);
    }
    if (*alat < (float)0.) {
	s_copy(ns, " S", (ftnlen)2, (ftnlen)2);
    }
    io___91.ciunit = *luout;
    s_wsfe(&io___91);
    do_fio(&c__1, "  Latitude: ", (ftnlen)12);
    r__1 = dabs(*alat);
    do_fio(&c__1, (char *)&r__1, (ftnlen)sizeof(real));
    do_fio(&c__1, " deg.", (ftnlen)5);
    do_fio(&c__1, ns, (ftnlen)2);
    do_fio(&c__1, " Longitude: ", (ftnlen)12);
    r__2 = dabs(*alon);
    do_fio(&c__1, (char *)&r__2, (ftnlen)sizeof(real));
    do_fio(&c__1, " deg.", (ftnlen)5);
    do_fio(&c__1, ew, (ftnlen)2);
    do_fio(&c__1, " ===============================================", (ftnlen)
	    48);
    e_wsfe();
    return 0;
} /* prtcut_ */

