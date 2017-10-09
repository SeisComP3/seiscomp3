/*
 * NAME
 *	spline -- Return the second derivatives of an interpolating function.

 * FILE
 *	spline.c

 * SYNOPSIS
 *	Construct an array of second derivatives based on an interpolating
 *	function at given tabulated points.

 * DESCRIPTION
 *	Function.  Given arrays x[1..n] and y[1..n] containing a tabulated
 *	function, i.e., y[i] = f(x[i]), with x[1] < x[2] < x[n], and given
 *	values yp1 and ypn for the first derivative of the interpolating
 *	function at points 1 and n, respectively, this routine returns an
 *	array y2[1..n] that contains the second derivatives of the
 *	interpolating function at the tabulated points x[i].  

 * DIAGNOSTICS
 *	If yp1 and/or ypn are equal to 1.0e30 or larger, the routine is 
 *	signalled to set the corresponding boundary condition for a natural 
 *	spline, with zero second derivative on that boundary.

 * FILES
 *

 * NOTES
 *	Note that this routine only need be called once to process the
 *	entire tabulated function in x and y arrays.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */

#ifdef SCCSID
static char	SccsId[] = "@(#)spline.c	40.1	10/12/90";
#endif

float *vector(int nl, int nh);
void free_vector(float *v, int nl);

void spline(float x[], float y[], int n, float yp1, float ypn, float y2[]) {
	int i, k;
	float p, qn, sig, un, *u;

	u = vector(1,n-1);

	if ( yp1 > 0.99e30 )
		y2[1] = u[1] = 0.0;
	else {
		y2[1] = -0.5;
		u[1]  = (3.0/(x[2]-x[1])) * ((y[2]-y[1])/(x[2]-x[1])-yp1);
	}

	/* Decomposition loop for tridiagonal algorithm */

	for ( i = 2; i <=n-1; ++i ) {
		sig   = (x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p     = sig*y2[i-1] + 2.0;
		y2[i] = (sig-1.0)/p;
		u[i]  = (y[i+1]-y[i])/(x[i+1]-x[i]) 
			- (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]  = (6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}

	if (ypn > 0.99e30)
		qn = un = 0.0;
	else {
		qn = 0.5;
		un = (3.0/(x[n] - x[n-1]))*(ypn - (y[n] - y[n-1]) / (x[n] - x[n-1]));
	}

	y2[n] = (un - qn*u[n-1])/(qn*y2[n-1] + 1.0);

	/* Back substituition loop of tridiagonal algorithm */
	for ( k = n; k >= 1; --k )
		y2[k] = y2[k]*y2[k+1] + u[k];

	free_vector(u, 1);
}

