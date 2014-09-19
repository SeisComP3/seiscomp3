/*
 * NAME
 *      splie2 -- Construct natural splines and second derivatives.

 * FILE
 *      splie2.c

 * SYNOPSIS
 *      Perform 1-D natural cubic splines on rows and return the second
 *	derivatives.
 
 * DESCRIPTION
 *      Function.  Given a tabulated function ya[1..m][1..n], and 
 *	tabulated independent variables x1a[1..m] and x2a[1..n], this 
 *	routine constructs one-dimensional natural cubic splines of the 
 *	rows of ya and returns the second derivatives in the array 
 *	y2a[1..m][1..n].  This routine only needs to be called once,
 *	and then, any number of bi-cubic spline interpolations can be
 *	performed by successive calls to splin2.

 *	---- Functions called ----
 *		spline:	Return 2nd derivatives of an interpolating function

 * DIAGNOSTICS
 *	Values returned larger than 1.0e30 signal a natual spline.

 * FILES
 *

 * NOTES
 *

 * SEE ALSO
 *      Press, W.H. et al., 1988, "Numerical Recipes", 94-110.

 * AUTHOR
 *
 */

#ifdef SCCSID
static char	SccsId[] = "@(#)splie2.c	40.1	10/12/90";
#endif

void splie2 (x1a, x2a, ya, m, n, y2a)

float	x1a[], x2a[], **ya, **y2a;
int	m, n;

{
	int	j;
	void spline();

	for (j = 1; j <= m; j++)
		spline (x2a, ya[j], n, 1.0e30, 1.0e30, y2a[j]);
}

