
/*
 * NAME
 *	fstat -- F-distribution test.

 * FILE
 *	fstat_.c

 * SYNOPSIS
 *	Make an F-distribution test for M parameters and N degrees of freedom.

 * DESCRIPTION
 *	Function.  Make an Snedecor's F-distribution test for M parameters
 *	and N degrees of freedom equal to the the given confidence limit.

 *	---- On entry ----
 *	m:	Number of parameters;	Must be 1, 2, or 3
 *	n:	Degrees of freedom;	Must be greater than 1
 *	p:	Confidence level;	Must be 0.90

 *	---- On return ----
 *	x:	Argument making Snedecor's F-distribution test for M parametes
 *		and N degrees of freedom equal to P (i.e., F(X) = P).  x is 
 *		done with a crude table and interpolation and is accurate only 
 *		to about +/- 0.01.

 * DIAGNOSTICS
 *	Currently, the only allowable confidence limit is 90% (0.90).

 * NOTES
 *	Likely extension to other levels (limits).

 * SEE ALSO
 *	S. Ross (1984, "A First Course in Probability Theory", 391 pp.).

 * AUTHOR
 *	Walter Nagy, February 1991.
 */


#ifdef SCCSID
static	char	SccsId[] = "@(#)fstat_.c	44.1	9/20/91";
#endif

#include <math.h>

void fstatx_ (m, n, p, x)

int	*m, *n;
float	*p;
double	*x;

{

	int	i, j;
	double	an, an1, an2, dp, x1, x2, y ,y1, y2;
	double	fabs();

	static	int	ns[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14,
				 16, 18, 20, 25, 30, 40, 60, 120 };
	static	double	ps[] = { 0.90, 0.95, 0.99 };
	static	double	xs[][21] =
		{
		  39.86, 8.53, 5.54, 4.54, 4.06, 3.78, 3.59, 3.46, 3.36,
		   3.29, 3.18, 3.10, 3.05, 3.01, 2.97, 2.92, 2.88, 2.84,
		   2.79, 2.75, 2.71,
		  49.50, 9.00, 5.46, 4.32, 3.78, 3.46, 3.26, 3.11, 3.01,
		   2.92, 2.81, 2.73, 2.67, 2.62, 2.59, 2.53, 2.49, 2.44,
		   2.39, 2.35, 2.30,
		  53.59, 9.16, 5.39, 4.19, 3.62, 3.29, 3.07, 2.92, 2.81,
		   2.73, 2.61, 2.52, 2.46, 2.42, 2.38, 2.32, 2.28, 2.23,
		   2.18, 2.13, 2.08
		};
 
	*x = 0.0;
	if (*m < 1 || *m > 3)
		return;
	if (*n < 1)
	{
		*x = 1000.0; 
		return;
	}
	dp = fabs(*p-ps[0]);
	if (dp > 0.001)
		return;
 
	for (i = 19; i >= 0; i--)
		if (*n >= ns[i])
		{
			j = i;
			break;
		}
 
	if (*n != ns[j])
	{
		an1 = ns[j];
		if (j < 19)
			an2 = ns[j+1];
		else
			an2 = *n + 1000;
		an = *n;
		y1 = an1/(1.0 + an1);
		y2 = an2/(1.0 + an2);
		y  = an/(1.0 + an);
		x1 = *(xs[*m-1] + j);
		x2 = *(xs[*m-1] + j+1);
		*x  = x1 + (x2-x1)*((y-y1)/(y2-y1));
	}
	else
		*x = *(xs[*m-1] + j);
}

