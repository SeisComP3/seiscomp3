
/*
 * NAME
 *	ratint -- Rational function interpolating function routine.

 * FILE
 *	ratint.c

 * SYNOPSIS
 *	A rational function is constructed so as to go through a chosen
 *	set of tabulated function values.

 * DESCRIPTION
 *	Function.  Given arrays xa[1..n] and ya[1..n], and given a value, x,
 *	this routine returns a value of y and an accuracy estimate, dy.  The
 *	value returned is that of the diagonal rational function, evaluated
 *	at x, which passes through the n points (xa[i], ya[i]), i = 1..n.

 * DIAGNOSTICS
 *	An error will be return if an interpolating function has a pole at
 *	the requested value of x.

 * FILES
 *	None.

 * NOTES
 *	Note that this routine only need be called once to process the
 *	entire tabulated function in x and y arrays.

 * SEE ALSO
 *	Press, W.H. et al., 1988, "Numerical Recipes", 94-110.  Use is
 *	analogous to that of routine, polint, page 90.

 * AUTHOR
 *
 */

#include <math.h>

#ifdef SCCSID
static char	SccsId[] = "@(#)ratint.c	44.1	9/20/91";
#endif

#define	TINY		1.0e-25;
#define	FREERETURN	{ free_dvector(d,1,n); free_dvector(c,1,n); return; }

void ratint (xa, ya, n, x, y, dy)

double	xa[], ya[], x, *y, *dy;
int	n;

{
	int	i, m, ns = 1;
	double	*c, *d, dd, h, hh, t, w, *dvector();
	void	nrerror(), free_dvector();

	c  = dvector(1,n);
	d  = dvector(1,n);
	hh = fabs(x - xa[1]);

	for (i = 1; i <= n; i++)
	{
		h = fabs(x - xa[i]);
		if (h == 0.0)
		{
			*y  = ya[i];
			*dy = 0.0;
			FREERETURN
		}
		else if (h < hh)
		{
			ns = i;
			hh = h;
		}
		c[i] = ya[i];
		d[i] = ya[i] + TINY;	/* The TINY part is needed to prevent */
	}				/* a rare zero-over-zero condition    */

	*y = ya[ns--];

	for (m = 1; m < n; m++)
	{
		for (i = 1; i <= n-m; i++)
		{
			w  = c[i+1] - d[i];
			h  = xa[i+m] - x;
			t  = (xa[i] - x) * d[i]/h;
			dd = t - c[i+1];
			if (dd == 0.0)
				nrerror ("Error in routine, RATINT");
			dd = w/dd;
			d[i] = c[i+1] * dd;
			c[i] = t * dd;
		}
		*y += (*dy = (2*ns < (n-m) ? c[ns+1] : d[ns--]) );
	}
	FREERETURN
}

