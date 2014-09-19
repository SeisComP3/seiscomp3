/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccscart_1_ {
    char sccsid[80];
};

#define sccscart_1 (*(struct sccscart_1_ *) &sccscart_)

/* Initialized data */

struct {
    char e_1[80];
    } sccscart_ = { {'@', '(', '#', ')', 'c', 'a', 'r', 't', '.', 'f', '\t', 
	    '4', '3', '.', '1', '\t', '9', '/', '9', '/', '9', '1', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static doublereal c_b2 = .99664710813076274;
static doublereal c_b3 = 2.;

/* Subroutine */ int cart_(alat, alon, z__, radius, x)
real *alat, *alon, *z__, *radius, *x;
{
    /* Builtin functions */
    double pow_dd(), tan(), atan(), cos(), sin();

    /* Local variables */
    static real alat2, r13, r123, esq;


/* Convert a geographical location to geocentric cartesian coordinates, */

/*  changed by johannes schweitzer for geographical coordinates */
/*     mar 17, 1992 */
/*    (willmore (1979): manual of seismological practice) */

/* ########### ( assuming a spherical earth.) */

/* The cartesian axis are such that */
/*     - Axis 1 intersects equator at 90 deg longitude (east) */
/*     - Axis 2 intersects north pole */
/*     - Axis 3 intersects equator at  0 deg longitude */

/* Input */

/*   ALAT  =  latitude (degrees) */
/*   ALON  =  longitude (degrees) */
/*   Z     =  depth */
/*   RADIUS  =  radius of the earth */

/* Output */

/*   X(1:3)  =  vector of geocentric cartesian coordinates */
/*              axis 1 intersects equator at  0 deg longitude */
/*              axis 2 intersects equator at 90 deg longitude */
/*              axis 3 intersects north pole */



/*     transform geographical coodinates in geocentric (spherical) */
/*     coordinates */

    /* Parameter adjustments */
    --x;

    /* Function Body */
    esq = pow_dd(&c_b2, &c_b3);
    alat2 = atan(tan(*alat * (float).017453292523928399) * esq);
    r123 = *radius - *z__;
    r13 = r123 * cos(alat2);
    x[1] = r13 * sin(*alon * (float).017453292523928399);
    x[2] = r123 * sin(alat2);
    x[3] = r13 * cos(*alon * (float).017453292523928399);

    return 0;
} /* cart_ */

