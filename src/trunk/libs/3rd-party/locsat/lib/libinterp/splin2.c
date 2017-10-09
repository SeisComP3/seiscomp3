/*
 * NAME
 *	splin2 -- Return values of a bi-cubic interpolated function.

 * FILE
 *	splin2.c

 * SYNOPSIS
 *	Return an interpolated function value by bi-cubic interpolation.

 * DESCRIPTION
 *	Function.  Given x1a, x2a, ya, m, n as described in subr. splie2 
 *	and y2a as produced by that routine; and given a desired
 *	interpolating point x1, x2; this routine returns an interpolated 
 *	function value y by bi-cubic spline interpolation.  The complemetary
 *	routine, splie2, needs to be called once prior to accessing this
 *	function to initializize natural splines and second derivatives.

 *	---- Functions called ----
 *		spline:	Return 2nd derivatives of an interpolating function
 *		splint:	Return a cubic spline interpolated value

 * DIAGNOSTICS
 *

 * FILES
 *

 * NOTES
 *	The above is accomplished by constructing row, then column splines,
 *	one-dimension at a time.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */

#ifdef SCCSID
static char	SccsId[] = "@(#)splin2.c	40.1	10/12/90";
#endif

void spline(float x[], float y[], int n, float yp1, float ypn, float y2[]);
void splint (float xa[], float ya[], float y2a[], int n, float x, float *y);
float *vector(int nl, int nh);
void free_vector(float *v, int nl);

void splin2(float x1a[], float x2a[], float **ya, float **y2a, int m, int n, float x1, float x2, float *y) {
	int j;
	float *ytmp, *yytmp;

	ytmp  = vector(1,n);
	yytmp = vector(1,n);
	for (j = 1; j <= m; j++)
		splint(x2a, ya[j], y2a[j], n, x2, &yytmp[j]);
	spline(x1a, yytmp, m, 1.0e30, 1.0e30, ytmp);
	splint(x1a, yytmp, ytmp, m, x1, y);
	free_vector(yytmp, 1);
	free_vector(ytmp, 1);
}
