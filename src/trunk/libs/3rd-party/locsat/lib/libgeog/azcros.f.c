/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsazcros_1_ {
    char sccsid[80];
};

#define sccsazcros_1 (*(struct sccsazcros_1_ *) &sccsazcros_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsazcros_ = { {'@', '(', '#', ')', 'a', 'z', 'c', 'r', 'o', 's', '.', 
	    'f', '\t', '4', '3', '.', '1', '\t', '9', '/', '9', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static real c_b2 = (float)1.;

/* Given the locations of two reference points and two azimuths of */
/*  great circles passing through those points, the */
/*  program computes the location of the crossing of two great circles */
/*   and the distances from two reference points. */
/*   -- by Steve Bratt and Donna Williams, June 1986. */

/* Subroutine */ int azcros_(alat1, alon1, aza, alat2, alon2, azb, dista, 
	distb, alat, alon, ierr)
real *alat1, *alon1, *aza, *alat2, *alon2, *azb, *dista, *distb, *alat, *alon;
integer *ierr;
{
    /* System generated locals */
    real r__1;

    /* Builtin functions */
    double asin(), r_sign(), tan(), sin(), cos(), atan();

    /* Local variables */
    static real dist, e, f, g, h__, delta, c1, c2, c3, c4, c5, ra, rb, rc, az,
	     alatin, alonin;
    extern /* Subroutine */ int latlon_(), distaz_();
    static real baz, azi, degtrad;


/*  INPUT: */
/*  alat1,alon1,alat2,alon2:  locations of reference points */
/*  aza,azb: azimuths of great circles passing through points points */
/*           1 and 2, respectively */
/*  OUTPUT: */
/*  dista,distb:  distance from points 1 and 2 to point */
/*                great circles cross */
/*  alat,alon:  location of crossing point. */

/*  All distances, azimuths, and locations are input and output */
/*    in degrees. */

/*  ierr : = 0 all O.K., = 1 lines do not cross within reasonable */
/*         distance. */

/*  SUBROUTINES CALLED: */
/*    distaz */
/*    latlon, which calls cart */
/*                        rotate */
/*                        geog */


    degtrad = 2 * asin((float)1.) / (float)180.;

/* Find azimuth, back azimuth and radial distance between */
/*  stations. */

    distaz_(alat1, alon1, alat2, alon2, &delta, &azi, &baz);

/* Find sign (clockwise = +) and value of angle measured from line */
/*   between two stations to aza and azb. */

    ra = *aza - azi;
    if (dabs(ra) > (float)180.) {
	r__1 = (float)360. - dabs(ra);
	ra = -r_sign(&r__1, &ra);
    }
    rb = *azb - baz;
    if (dabs(rb) > (float)180.) {
	r__1 = (float)360. - dabs(rb);
	rb = -r_sign(&r__1, &rb);
    }

/* If the signs of ra and rb are the same, the great circles along */
/*  those azimuths will not cross within a "reasonable" distance. */

    if (r_sign(&c_b2, &ra) == r_sign(&c_b2, &rb)) {
	*ierr = 1;
	return 0;
    }

    ra = dabs(ra);
    rb = dabs(rb);

/* If the sum of ra and rb is greater than 180., there will be no */
/*  crossing within a reasonable distance. */

    if (ra + rb > (float)180.) {
	*ierr = 1;
	return 0;
    }

    ra *= degtrad;
    rb *= degtrad;
    rc = delta * degtrad;

    c1 = tan(rc * (float).5);
    c2 = (ra - rb) * (float).5;
    c3 = (ra + rb) * (float).5;

/*          equations for solving for the distances */

    f = c1 * sin(c2);
    g = sin(c3);
    h__ = cos(c2) * c1;
    e = cos(c3);

    c4 = atan(f / g);
    c5 = atan(h__ / e);

/*       Compute distances (lengths of the triangle) */

    *distb = (c4 + c5) / degtrad;
    *dista = (c5 - c4) / degtrad;

    if (*dista < (float)0. || *distb < (float)0.) {
	*ierr = 1;
	return 0;
    }

    if (*dista < *distb) {
	dist = *dista;
	az = *aza;
	alatin = *alat1;
	alonin = *alon1;
	goto L50;
    }
    dist = *distb;
    az = *azb;
    alatin = *alat2;
    alonin = *alon2;
L50:
    latlon_(&alatin, &alonin, &dist, &az, alat, alon);
    *ierr = 0;
    return 0;
} /* azcros_ */

