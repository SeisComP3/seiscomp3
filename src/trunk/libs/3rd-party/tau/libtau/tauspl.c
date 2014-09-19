#include <stdio.h>
#include <math.h>

/*
 * Given ray parameter grid pt;i (pt sub i), i=i1,i1+1,...,i2, tauspl
 * determines the i2-i1+3 basis functions for interpolation I such
 * that:
 * 
 * tau(p) = a;1,i + Dp * a;2,i + Dp**2 * a;3,i + Dp**(3/2) * a;4,i
 * 
 * where Dp = pt;n - p, pt;i <= p < pt;i+1, and the a;j,i's are
 * interpolation coefficients.  Rather than returning the coefficients,
 * a;j,i, which necessarily depend on tau(pt;i), i=i1,i1+1,...,i2 and
 * x(pt;i) (= -d tau(p)/d p | pt;i), i=i1,i2, tauspl returns the
 * contribution of each basis function and its derivitive at each
 * sample.  Each basis function is non-zero at three grid points,
 * therefore, each grid point will have contributions (function values
 * and derivitives) from three basis functions.  Due to the basis
 * function normalization, one of the function values will always be
 * one and is not returned in array coef with the other values.
 * Rewritten on 23 December 1983 by R. Buland.
 */

void
tauspl(int i1, int i2,
       double *pt, double *c1, double *c2, double *c3, double *c4, double *c5)
{
	double del[5], sdel[5], deli[5], d3h[4], d1h[4], dih[4], d[4], 
		ali, alr, b3h, b1h, bih, th0p, th2p, th3p, th2m;
	int i, j, k, l, /* m, */ is, n2;

	n2 = i2 - i1 - 1;
	if(n2 <= -1) return;
	is = i1 + 1;
	/*
	 * To achieve the requisite stability, proceed by constructing basis
	 * functions G;i, i = 0,1,...,n+1.  G;i will be non-zero only on the
	 * interval [p;i-2,p;i+2] and will be continuous with continuous first
	 * and second derivitives.  G;i(p;i-2) and G;i(p;i+2) are constrained
	 * to be zero with zero first and second derivitives.  G;i(p;i) is
	 * normalized to unity.
	 * 
	 * Set up temporary variables appropriate for G;-1.  Note that to get
	 * started, the ray parameter grid is extrapolated to yield
	 * p;i, i = -2,-1,0,1,...,n.
	 */
	del[1] = pt[i2] - pt[i1] + 3.0*(pt[is] - pt[i1]);
	sdel[1] = sqrt(fabs(del[1]));
	deli[1] = 1.0/sdel[1];
	for(k = 2; k <= 4; k++)
	{
		del[k] = pt[i2] - pt[i1] + (4-k)*(pt[is] - pt[i1]);
		sdel[k] = sqrt(fabs(del[k]));
		deli[k] = 1.0/sdel[k];
		d3h[k-1] = del[k]*sdel[k] - del[k-1]*sdel[k-1];
		d1h[k-1] = sdel[k] - sdel[k-1];
		dih[k-1] = deli[k] - deli[k-1];
	}
	l = i1-1;
	/* Loop over G;i, i = 0,1,...,n-3.
	 */
	for(i = 0; i < n2; i++)
	{
		/* Update temporary variables for G;i-1.
		 */
		for(k = 1; k <= 4; k++)
		{
			del[k-1] = del[k];
			sdel[k-1] = sdel[k];
			deli[k-1] = deli[k];
		}
		for(k = 1; k <= 3; k++)
		{
			d3h[k-1] = d3h[k];
			d1h[k-1] = d1h[k];
			dih[k-1] = dih[k];
		}
		l++;
		del[4] = pt[i2] - pt[l+1];
		sdel[4] = sqrt(fabs(del[4]));
		deli[4] = 1.0/sdel[4];
		d3h[3] = del[4]*sdel[4] - del[3]*sdel[3];
		d1h[3] = sdel[4] - sdel[3];
		dih[3] = deli[4] - deli[3];
		/*
		 * Construct G;i-1.
		 */
		ali = 1./(.125*d3h[0] - (.75*d1h[0]+.375*dih[0]*del[2])*del[2]);
		alr = ali*(.125*del[1]*sdel[1] - (.75*sdel[1] + .375*del[2]*
				deli[1] - sdel[2])*del[2]);
		b3h = d3h[1] + alr*d3h[0];
		b1h = d1h[1] + alr*d1h[0];
		bih = dih[1] + alr*dih[0];
		th0p = d1h[0]*b3h - d3h[0]*b1h;
		th2p = d1h[2]*b3h - d3h[2]*b1h;
		th3p = d1h[3]*b3h - d3h[3]*b1h;
		th2m = dih[2]*b3h - d3h[2]*bih;
		/* 
		 * The d;i's completely define G;i-1.
		 */
		d[3] = ali*((dih[0]*b3h - d3h[0]*bih)*th2p - th2m*th0p) /
			((dih[3]*b3h - d3h[3]*bih)*th2p - th2m*th3p);
		d[2] = (th0p*ali - th3p*d[3])/th2p;
		d[1] = (d3h[0]*ali - d3h[2]*d[2] - d3h[3]*d[3])/b3h;
		d[0] = alr*d[1] - ali;
		/*
		 * Construct the contributions G;i-1[p;i-2] and G;i-1[p;i].
		 * G;i-1[p;i-1] need not be constructed as it is normalized
		 * to unity.
		 */
		c1[l] = (.125*del[4]*sdel[4] - (.75*sdel[4] +
			.375*deli[4]*del[3] - sdel[3])*del[3])*d[3];
		if(i >= 2) c2[l-2] = (.125*del[0]*sdel[0] - (.75*sdel[0]
				+ .375*deli[0]*del[1] - sdel[1])*del[1])*d[0];
		/*
		 * Construct the contributions -dG;i-1[p]/dp | p;i-2, p;i-1,
		 * and p;i.
		 */
		c3[l] = -.75*(sdel[4] + deli[4]*del[3] - 2*sdel[3])*d[3];
		if(i >= 1) c4[l-1] = -.75*((sdel[1] + deli[1]*del[2] -
			2.0*sdel[2])*d[1] - (d1h[0] + dih[0]*del[2])*d[0]);
		if(i >= 2) c5[l-2] = -.75*(sdel[0] + deli[0]*del[1]
			- 2.0*sdel[1])*d[0];
	}
	/* Loop over G;i, i = n-2,n-1,n,n+1.  These cases must be handle
	 * seperately because of the singularities in the second derivitive
	 * at p;n.
	 */
	for(j = 0; j < 4; j++)
	{
		/* Update temporary variables for G;i-1.
		 */
		for(k = 1; k < 5; k++)
		{
			del[k-1] = del[k];
			sdel[k-1] = sdel[k];
			deli[k-1] = deli[k];
		}
		for(k = 1; k < 4; k++)
		{
			d3h[k-1] = d3h[k];
			d1h[k-1] = d1h[k];
			dih[k-1] = dih[k];
		}
		l++;
		del[4] = 0.0;
		sdel[4] = 0.0;
		deli[4] = 0.0;
		/*
		 * Construction of the d;i's is different for each case.  
		 * In cases G;i, i = n-1,n,n+1, G;i is truncated at p;n to 
		 * avoid patching across the singularity in the second
		 * derivitive.
		 */
		if(j >= 3)
		{
			/* For G;n+1 constrain G;n+1[p;n] to be .25.
		 	 */
			d[0] = 2.0/(del[0]*sdel[0]);
		}
		else
		{
			/* For G;i, i = n-2,n-1,n, the condition dG;i[p]/dp|p;
			 * i = 0 has been substituted for the second derivitive
			 * continuity condition that can no longer be satisfied.
			 */
			alr = (sdel[1] + deli[1]*del[2] - 2*sdel[2])/
				(d1h[0] + dih[0]*del[2]);
			d[1] = 1./(.125*del[1]*sdel[1] - (.75*sdel[1] 
				+ .375*deli[1]*del[2] - sdel[2])*del[2]
				- (.125*d3h[0] -
				(.75*d1h[0] + .375*dih[0]*del[2])*del[2])*alr);
			d[0] = alr*d[1];
			if(j == 0)
			{
				/* No additional constraints are required
				 * for G;n-2.
				 */
				d[2] = -((d3h[1] - d1h[1]*del[3])*d[1]
					+ (d3h[0] - d1h[0]*del[3])*d[0])/
					(d3h[2] - d1h[2]*del[3]);
				d[3] = (d3h[2]*d[2] + d3h[1]*d[1] +
					d3h[0]*d[0])/(del[3]*sdel[3]);
			}
			else if(j == 1)
			{
				/* For G;n-1 constrain G;n-1[p;n] to be .25.
				 */
				d[2] = (2. + d3h[1]*d[1] + d3h[0]*d[0])/
						(del[2]*sdel[2]);
			}
		}
		/* Construct the contributions G;i-1[p;i-2] and G;i-1[p;i].
		 */
		if(j <= 1) c1[l] = (
			.125*del[2]*sdel[2] - (.75*sdel[2] + .375*deli[2]*del[3]
			 - sdel[3])*del[3])*d[2] - (.125*d3h[1] - (.75*d1h[1] +
			 .375*dih[1]*del[3])*del[3])*d[1] - (.125*d3h[0]
			- (.75*d1h[0] + .375*dih[0]*del[3])*del[3])*d[0];
		if(l-i1 > 1) c2[l-2] = (.125*del[0]*sdel[0] - (.75*sdel[0]
			+ .375*deli[0]*del[1] - sdel[1])*del[1])*d[0];
		/*
		 * Construct the contributions -dG;i-1[p]/dp | p;i-2, p;i-1,
		 * and p;i.
		 */
		if(j <= 1) c3[l] = -.75*((sdel[2] + deli[2]*del[3] -
			2.0*sdel[3])*d[2] - (d1h[1] + dih[1]*del[3])*d[1]
			-(d1h[0] + dih[0]*del[3])*d[0]);
		if(j <= 2 && l-i1 > 0) c4[l-1] = 0.0;
		if(l-i1 > 1) c5[l-2] = -.75*(sdel[0] + deli[0]*del[1]
						- 2.0*sdel[1])*d[0];
	}
}
