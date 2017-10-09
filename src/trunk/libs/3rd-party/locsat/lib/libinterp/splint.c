/*
 * NAME
 *	splint -- Return a cubic spline interpolated value.

 * FILE
 *	splint.c

 * SYNOPSIS
 *	This routine return a cubic spline interpolated value from an array.

 * DESCRIPTION
 *	Function.  Given the arrays xa[1..n] and ya[1..n] which tabulate
 *	a function (with the xa[i]'s in order), and given the array 
 *	y2a[1..n], which is the output from subroutine spline(), and given
 *	a value of x, this routine returns a cubic-spline interpolated
 *	value of y.

 * DIAGNOSTICS
 *

 * FILES
 *

 * NOTES
 *	The correct position in the table is obtained by means of bi-section.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */

#ifdef SCCSID
static char	SccsId[] = "@(#)splint.c	40.1	10/12/90";
#endif


#include <stdio.h>


void splint (float xa[], float ya[], float y2a[], int n, float x, float *y) {
	int klo, khi, k;
	float h, b, a;

	klo = 1;
	khi = n;

	while ( khi-klo > 1 ) {
		k = (khi + klo) >> 1;
		if (xa[k] > x)	khi = k;
		else		klo = k;
	} /* klo and khi now bracket the input value of x */

	h = xa[khi] - xa[klo];

	if ( h == 0 )
		printf ("Bad xa input to routine splint");

	a  = (xa[khi] - x)/h;
	b  = (x - xa[klo])/h;
	*y = a*ya[klo] + b*ya[khi] + ((a*a*a - a)*y2a[klo] + (b*b*b - b)*y2a[khi])*(h*h)/6.0;
}
