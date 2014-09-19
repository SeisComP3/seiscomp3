/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct sccsdrotg_1_ {
    char sccsid[80];
};

#define sccsdrotg_1 (*(struct sccsdrotg_1_ *) &sccsdrotg_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsdrotg_ = { {'@', '(', '#', ')', 'd', 'r', 'o', 't', 'g', '.', 'f', 
	    '\t', '4', '4', '.', '1', '\t', '9', '/', '2', '0', '/', '9', '1',
	     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Table of constant values */

static doublereal c_b4 = 1.;

/* NAME */
/* 	drotg -- Construct a Givens plane rotation. */
/* FILE */
/* 	drotg.f */
/* SYNOPSIS */
/* 	LINPACK routine applies a Givens plane rotation. */
/* DESCRIPTION */
/* 	Subroutine.  Construct a Givens plane rotation, a scalar, da and */
/* 	db, at a time.  Called in SVD routine, dsvdc, prior to application */
/* 	of normal plane rotation (subr. drot). */
/* DIAGNOSTICS */

/* NOTES */

/* SEE ALSO */
/* 	LINPACK documentation by John Dongarra. */
/* AUTHOR */
/* 	John Dongarra, March 1978. */
/* Subroutine */ int drotg_(da, db, c__, s)
doublereal *da, *db, *c__, *s;
{
    /* System generated locals */
    doublereal d__1, d__2;

    /* Builtin functions */
    double sqrt(), d_sign();

    /* Local variables */
    static doublereal r__, scale, z__, roe;

/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
/*     ---- On entry and return ---- */
/*     ---- Internal variables ---- */
    roe = *db;
    if (abs(*da) > abs(*db)) {
	roe = *da;
    }
    scale = abs(*da) + abs(*db);
    if (scale != 0.) {
	goto L1000;
    }
    *c__ = 1.;
    *s = 0.;
    r__ = 0.;
    goto L1010;
L1000:
/* Computing 2nd power */
    d__1 = *da / scale;
/* Computing 2nd power */
    d__2 = *db / scale;
    r__ = scale * sqrt(d__1 * d__1 + d__2 * d__2);
    r__ = d_sign(&c_b4, &roe) * r__;
    *c__ = *da / r__;
    *s = *db / r__;
L1010:
    z__ = 1.;
    if (abs(*da) > abs(*db)) {
	z__ = *s;
    }
    if (abs(*db) >= abs(*da) && *c__ != 0.) {
	z__ = 1. / *c__;
    }
    *da = r__;
    *db = z__;
    return 0;
} /* drotg_ */

