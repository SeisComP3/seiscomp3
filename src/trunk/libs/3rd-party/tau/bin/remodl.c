#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libtau/limits.h>

static int rough(float dr, int *nr, int *ncr, char *modnam, char *path);
static void findcp(int ncr);
static void crtpt(int k, int nph);
static void pgrid(int nph, int *n0, float xtol);
static float xmod(float pk, int nph, int ipart, int *nr, float *rb);
static void finrng(float xg, int nph, int n, float p0, float p1, float x0, float x1, float xtol, int *nr);
static void zgrid(int nph, int *m0);
static float findep(float u0, int k, int nph);
static void brkpts(int m, int nph);
static void phcod(int *lc, int nph, float z0, int kfl);
static void efe4(int n, int *na);
static void efe8(int n, double *da);
static void efec(int n, char **a);

void
tauint(double ptk, double ptj, double pti, double zj, double zi, double *tau, double *x);

/* extern */ int emdlv(float r, float *, float *);
/* extern */ int emdld(int*, float*, char*,char*);

/*
static void	rough();
static void	findcp();
static void	crtpt();
static void	pgrid();
static float	xmod();
static void	finrng();
static void	zgrid();
static float	findep();
static void	brkpts();
static void	phcod();
static void	efe4();
static void	efe8();
static void	efec();

*/

static struct
{
	char	*code[nbr1];
	int	loc[ndp1], ndex[ndp1], lbrk[nbr2];
	float	pp[nsl1], xx[nsl1], rr[nsl1], u[nmd0];
	double	pm[ndp1], zm[ndp1], zm0[ndp1];
} v[2];

static int	n, nk, nc = 0, lcb[2], lbb[2];
static float	r[nmd0], vp[nmd0], vs[nmd0], z[nmd0], ucrt[ncp0];
static float	xn, tn, pn, ric, roc;
static double	pb[nsl1];
static double	delx[] = {200.,200.}, dpmax[] = {.01,.01}, drmax[] = {75.,75};
static FILE *fp10, *fpout;


/*
 * Rough provides a rough interpolation of the earth model available
 * through routine emdlv.  The model radii, compressional velocity, and
 * shear velocity are provided in arrays r, vp, and vs respectively.
 * Between first order discontinuities available through routine emdld,
 * the radii are equally spaced as close to spacing dr as possible.  The
 * number of radii used is returned in variable nr and the index of the
 * core-mantle radius is returned in ncr.  Note that emdld returns the
 * radii of the discontinuities from the center of the earth out while
 * rough returns the model from the surface in.  Also note that in the
 * model returned by rough, each discontinuity will be represented by
 * two model ajacent model points with the same radius.
 *
 *  calls emdld and emdlv 
 */
static int
rough(float dr, int *nr, int *ncr, char *modnam, char *path)
{
	static float rd[30];
	static float tol = 1.e-6, vtol = 2.e-5;
	float dx, r0, r1;
	int i, j, l, m, np;

	/* Get the radii of model discontinuities.
	 * np = number of discontinuities, rd = radii(km).
	 */
	if ( emdld(&np, rd, modnam, path) ) {
		return 1;
	}
	/*
	 * Save the radii of the inner core-outer core and core-mantle
	 * boundaries respectively.
	 */
	ric = rd[0];
	roc = rd[1];
	/*
	 * Begin the interpolation.
	 */
	r1 = rd[np-1];
	/*
	 * Loop over each layer (between two discontinuities), from surface down
	 * rd[0] = inner core boundary.
	 * m == -1 is for radii almost 0. (r1 = .001)
	 * assume fluid core (from rd[1] to 0.001)
	 */
	for(i = np-2, m = -1; i >= -1; i--)
	{
		r0 = r1;
		r1 = .001;
		if(i >= 0) r1 = rd[i];
		l  = (r0 - r1)/dr - .5;
		dx = (r0 - r1)/(l+1);
		/*
		 * Set the outer most point of the layer.
		 */
		m++;
		r[m] = r0;
		emdlv(r0*(1.-tol), &vp[m], &vs[m]);
		/*
		 * Check for continuity across an apparant discontinuity.
		 */
		if(m > 0)
		{
			/* If vp is close to continuous, force it.
			 */
			if(fabs(vp[m-1]-vp[m]) <= vtol*vp[m])
			{
				vp[m-1] = .5*(vp[m-1]+vp[m]);
				vp[m] = vp[m-1];
			}
			/* If vs is close to continuous, force it.
			 */
			if(fabs(vs[m-1]-vs[m]) <= vtol*vs[m])
			{
				vs[m-1] = .5*(vs[m-1]+vs[m]);
				vs[m] = vs[m-1];
			}
		}
		/* Make P and S velocity the same if we are in a fluid.
		 */
		if(i <= 0) vs[m] = vp[m];
		/* Interpolate the model throughout the layer.
		 */
		for(j = 1; j <= l; j++)
		{
      			m++;
			r[m] = r0 - j*dx;
			emdlv(r[m], &vp[m], &vs[m]);
			if(i <= 0) vs[m] = vp[m];  /* fluid */
		}
		/* Set the inner most point of the layer.
		 */
		m++;
		r[m] = r1;
		emdlv(r1*(1.+tol), &vp[m], &vs[m]);
		if(i <= 0) vs[m] = vp[m]; /* fluid */
		/*
		 * Set the index to the core-mantle radius.
		 */
		if(i == 1) *ncr = m;
	}
	*nr = m+1;

	return 0;
}

/*	crtpt()
 * For each critical point (slowness corresponding to a first order
 * discontinuity) save slowness u[k,nph] in array ucrt and sort it
 * into descending order.  Note that k indexes an equivalent depth and
 * nph indexes either P or V slownesses.
 * 
 * calls no other routine 
 */
static void
crtpt(int k, int nph)
{
	static float tol = 1.e-6;
	int i;
	float utmp;

	/* Eliminate duplicates.
	 */
	for(i = 0; i < nc; i++)
	{
		if(fabs(ucrt[i]-v[nph].u[k]) <= tol)
		{
			fprintf(fp10,
				"duplicate critical value eliminated: %e\n",
				v[nph].u[k]);
			return;
		}
	}

	ucrt[nc++] = v[nph].u[k];
	/*
	 * preserve sorted ucrt, by increasing value.
	 */
	for(i = nc-1; i > 0; i--)
	{
		if(ucrt[i-1] >= ucrt[i]) return;
		utmp = ucrt[i-1];
		ucrt[i-1] = ucrt[i];
		ucrt[i] = utmp;
	}
}


static void
findcp(int ncr)
{
	/*
	 * find critical points
	 *
	 * calls crtpt
	 */
	static float tol = 1.e-6;
	int i, j, ifl, kfl;

	ifl = 0;
	kfl = 0;
	j = 0;
	crtpt(0, 0);
	crtpt(0, 1);
	for(i = 1; i < nk; i++)
	{
		if(fabs(z[j] - z[i]) <= tol)
		{
			crtpt(j, 0);
			crtpt(i, 0);
			if(j <= ncr) crtpt(j, 1);
			if(i <= ncr) crtpt(i, 1);
		}
		else
		{
			if(ifl == 0)
			{
 				if(v[0].u[i] > v[0].u[j])
				{
					ifl = 1;
					crtpt(j, 0);
				}
			}
			else
			{
				if(v[0].u[i] < v[0].u[j])
				{
					ifl = 0;
					crtpt(j, 0);
				}
			}
			if(i <= ncr)
			{
				if(kfl == 0)
				{
 					if(v[1].u[i] > v[1].u[j])
					{
						kfl = 1;
						crtpt(j, 1);
					}
				}
				else
				{
					if(v[1].u[i] < v[1].u[j])
					{
						kfl = 0;
						crtpt(j, 1);
					}
				}
			}
		}
		j = i;
	}
	ucrt[nc] = 0.;
}


#define mxtmp 200

static void
pgrid(int nph, int *n0, float xtol)
{
	int i, ic, j, k, l, m, n, ifl, nr, kk, lsav, mm, ll;
	float a0, x0, du, r0, r1, r2, sgn, u0, dx, rsav, rnew, ex;
	static float ps[mxtmp], xs[mxtmp], pb[3], xb[3];
	static float tol = 1.e-5;

	a0 = 1./xn;
	for ( i = 0; i < nc; i++ ) {
		if(fabs(v[nph].u[0] - ucrt[i]) <= tol) break;
	}

	ic = i;

	n = 0;
	j = ic + 1;
	for(i = ic; i < nc; i++) {
		fprintf(fp10, "i %d %12.6f %12.6f\n", i, ucrt[i], ucrt[j]);
		ifl = 1;
		ps[0] = ucrt[i];
		xs[0] = xmod(ucrt[i], nph, 1, &nr, &r0);
		x0 = xmod(ucrt[j], nph, -1, &nr, &r1);
		l = (int)(fabs(x0-xs[0])/delx[nph] + .8);

		if(l < 1) l = 1;

		du = (ps[0] - ucrt[j])/(l*l);
		fprintf(fp10, "overflow??? x0 x1 l du nr r1 %e %e %d %e %d %e\n",
		        xs[0],x0,l+1,du,nr,r1);
		l--;

		for ( k = 1; k <= l; k++ ) {
			ps[k] = ps[0] - k*k*du;
			xs[k] = xmod(ps[k], nph, 1, &nr, &r2);
		}

		l += 2;
		ps[l-1] = ucrt[j];
		xs[l-1] = x0;

		do {
			x0 = xs[1] - xs[0];
			for ( k = 2; k < l; k++ ) {
				if((xs[k] - xs[k-1])*x0 <= 0.) break;
			}

			if ( k < l ) {
				ifl++;
				r2 = r1;
				fprintf(fp10, "caustic\n");
				k--;
				kk = k-2;

				for(m = 0; m < 3; m++)
				{
					kk++;
					pb[m] = ps[kk];
					xb[m] = xs[kk];
				}

				sgn = (.5*(xb[0]+xb[2]) - xb[1] >= 0.) ? 1. : -1.;

				do
				{
					u0 = .5*(pb[0] + pb[1]);
					x0 = xmod(u0, nph, 1, &nr, &r1);
					if(sgn*(xb[1] - x0) >= 0.)
					{
						pb[2] = pb[1];
						xb[2] = xb[1];
						pb[1] = u0;
						xb[1] = x0;
					}
					else
					{
						pb[0] = u0;
						xb[0] = x0;
						u0 = .5*(pb[1] + pb[2]);
						x0 = xmod(u0, nph, 1, &nr, &r1);
						if(sgn*(xb[1] - x0) >= 0.)
						{
							pb[0] = pb[1];
							xb[0] = xb[1];
							pb[1] = u0;
							xb[1] = x0;
						}
						else
						{
							pb[2] = u0;
							xb[2] = x0;
						}
					}
				}
				while ( fabs(xb[2]-xb[0]) > xtol );

				ps[k] = pb[1];
				xs[k] = xb[1];
				lsav = l;
				l = k+1; /*** ?????? ***/
			}

			if ( i == ic ) {
				v[nph].pp[n] = ps[0];
				v[nph].xx[n] = xs[0];
				v[nph].rr[n] = r0;
				fprintf(fp10, "first %d %10.6f %10.6f %9.2f\n",
					n,v[nph].pp[n],v[nph].xx[n],v[nph].rr[n]);
				n++;
			}

			k = (int)(fabs(xs[l-1]-xs[0])/delx[nph] + .8);
			if(k < 1) k = 1;
			dx = (xs[l-1] - xs[0])/k;
			x0 = xs[0];
			rsav = r0;
			mm = 1;

			do
			{
				x0 = x0 + dx;
				if ( fabs(x0 - xs[l-1]) <= tol ) {
					v[nph].pp[n] = ps[l-1];
					v[nph].xx[n] = xs[l-1];
					v[nph].rr[n] = r1;
				}
				else {
					for ( kk = mm; kk < l-1; kk++ ) {
						if((x0-xs[kk])*(x0-xs[kk-1]) <= 0.)
							break;
					}
					mm = kk;
					finrng(x0, nph, n, ps[kk], ps[kk-1], xs[kk],
					       xs[kk-1], xtol, &nr);

					fprintf(fp10, "sol %d %10.6f %10.6f %9.2f ", n,
					        v[nph].pp[n],v[nph].xx[n],v[nph].rr[n]);
					fprintf(fp10, "%d %10.6f %9.2f %9.2f\n",
					        nr, v[nph].pp[n-1]-v[nph].pp[n],
					        a0*(v[nph].xx[n]-v[nph].xx[n-1]),
					        rsav-v[nph].rr[n]);
				}

				if ( fabs(v[nph].pp[n]-v[nph].pp[n-1]) > dpmax[nph] ) {
					ll = (int)(fabs(ps[l-1]-v[nph].pp[n-1])/dpmax[nph] + .99);

					if ( ll < 1 ) ll = 1;
					v[nph].pp[n] = v[nph].pp[n-1] + (ps[l-1] - v[nph].pp[n-1])/ll;
					v[nph].xx[n] = xmod(v[nph].pp[n], nph, 1, &nr, &v[nph].rr[n]);
					fprintf(fp10, "dpmax %d %10.6f %10.6f %9.2f ",n,
					v[nph].pp[n],v[nph].xx[n],v[nph].rr[n]);
					fprintf(fp10, "%d %10.6f %9.2f %9.2f\n",
					        nr, v[nph].pp[n-1] - v[nph].pp[n],
					        a0*(v[nph].xx[n] - v[nph].xx[n-1]),
					        rsav - v[nph].rr[n]);

					k = (int)(fabs(xs[l-1] - v[nph].xx[n])/delx[nph] + .8);

					if ( k < 1 ) k = 1;
					dx = (xs[l-1] - v[nph].xx[n])/k;
					x0 = v[nph].xx[n];
					mm = 1;
				}

				if ( fabs(v[nph].rr[n] - rsav) > drmax[nph] ) {
					rnew = rsav - drmax[nph];
					while ( rnew > r[nr] ) nr--;
					if ( nr < nk-1 ) {
						ex = log(v[nph].u[nr+1]/v[nph].u[nr])/log(r[nr+1]/r[nr]);
						du = fabs(v[nph].pp[n-1] -
						v[nph].u[nr]*pow(rnew/r[nr], ex));
					}
					else {
						du = fabs(v[nph].pp[n-1] - v[nph].u[nk-1]*rnew/r[nk-1]);
					}

					ll = (int)(fabs(ps[l-1]-v[nph].pp[n-1])/du+.99);
					if ( ll < 1 ) ll = 1;

					v[nph].pp[n] = v[nph].pp[n-1] + (ps[l-1]-v[nph].pp[n-1])/ll;
					v[nph].xx[n] = xmod(v[nph].pp[n], nph, 1, &nr, &v[nph].rr[n]);
					fprintf(fp10, "drmax %d %10.6f %10.6f %9.2f ",
					        n, v[nph].pp[n], v[nph].xx[n], v[nph].rr[n]);

					fprintf(fp10, "%d %10.6f %9.2f %9.2f\n",
					        nr, v[nph].pp[n-1]-v[nph].pp[n],
					        a0*(v[nph].xx[n]-v[nph].xx[n-1]),
					        rsav-v[nph].rr[n]);

					k = (int)(fabs(xs[l-1] - v[nph].xx[n])/delx[nph] + .8);
					if ( k < 1 ) k = 1;

					dx = (xs[l-1] - v[nph].xx[n])/k;
					x0 = v[nph].xx[n];
					mm = 1;
				}
				rsav = v[nph].rr[n];
				n++;
			}
			while ( fabs(x0 - xs[l-1]) > tol );

			fprintf(fp10, "end  %d %10.6f %10.6f %9.2f ",
			        n-1, v[nph].pp[n-1], v[nph].xx[n-1], v[nph].rr[n-1]);
			fprintf(fp10, "%d %10.6f %9.2f %9.2f\n",
			        nr, v[nph].pp[n-2]-v[nph].pp[n-1],
			        a0*(v[nph].xx[n-1]-v[nph].xx[n-2]),
			        v[nph].rr[n-2]-v[nph].rr[n-1]);
			ifl--;

			if ( ifl > 0 ) {
				for( m = l-1, k = 0; m < lsav; m++, k++ ) {
					ps[k] = ps[m];
					xs[k] = xs[m];
				}

				l = lsav - l + 1;
				r0 = r1;
				r1 = r2;
			}
		}
		while ( ifl > 0 );

		j++;
	}

	*n0 = n;
}

static float
xmod(float pk, int nph, int ipart, int *nr, float *rb)
{
	/* partial tau integrals.
	 */
	int i, j;
	float x, p, zb;

	if(pk <= 0.)
	{
		x = -1.5707963;
		*nr = nk-1;
		*rb = 0.;
		return( -2.*x );
	}

	x = 0.;
	j = 0;

	for(i = 1; i < nk; i++)
	{
		if(pk > v[nph].u[i])
		{
			*nr = j;
			p = log(r[i]/r[j])/log(v[nph].u[i]/v[nph].u[j]);
			*rb = r[j]*pow(pk/v[nph].u[j], p);
			zb = log(*rb*xn);
			if(v[nph].u[j] > pk)
			{
				x = x + (zb - z[j])*acos(pk/v[nph].u[j]) /
						log(v[nph].u[j]/pk);
			}
			return( -2.*x );
		}
		if(v[nph].u[j] != v[nph].u[i]) x = x + (z[i]-z[j])*
			(acos(pk/v[nph].u[j]) - acos(pk/v[nph].u[i]))/
				log(v[nph].u[j]/v[nph].u[i]);
		if(pk == v[nph].u[i] && ipart < 0)
		{
			*nr = i;
			*rb = r[i];
			return( -2.*x );
		}
		j = i;
	}
	*nr = nk-1;
	*rb = r[nk]*pk/v[nph].u[nk];
	zb = log(*rb*xn);
	if(v[nph].u[nk] > pk)
	{
		x = x + (zb - z[nk])*acos(pk/v[nph].u[nk]) /
					log(v[nph].u[nk]/pk);
	}
	return( -2.*x );
}

/*	iteration on range
 *
 *	calls xmod
 *
 *	Function find0 returns x0 for which f(x0)  =  0 where f(x) is a
 *	function supplied by the user (specified external in the calling
 *	routine).  X1 and x2 are the starting trial points.  Function
 *	evaluations are made until x0 is determined to a relative precision
 *	of eps by a process of inverse iterative interpolation using
 *	Aitken's method.
 *                                                    -rpb
 */
static void
finrng(float xg, int nph, int n, float p0, float p1, float x0, float x1, float xtol, int *nr)
{
	int j, m;
	float ps0, ps1, xi, yi, r0;
	static float x[20], y[20];

	m = 0;
	if((x1-xg)*(x0-xg) > 0.)
	{
		fprintf(stderr, "finrng: Root not bracketed.\n");
		exit(1);
	}
	/* If the limits are already close together, there is no point in
	 * iterating.
	 */
	if(fabs(x0-x1) <= xtol)
	{
		v[nph].pp[n] = .5*(p0 + p1);
		v[nph].xx[n] = xmod(v[nph].pp[n], nph, 1, nr, &v[nph].rr[n]);
		return;
	}
	/* Set up iteration with first two trial points, x1 and x2.
	 */
	y[0] = x0 - xg;
	if(fabs(y[0]) <= xtol)
	{
		v[nph].pp[n] = p0;
		v[nph].xx[n] = xmod(v[nph].pp[n], nph, 1, nr, &v[nph].rr[n]);
		return;
	}
	x[0] = p0;
	yi = x1 - xg;
	if(fabs(yi) <= xtol)
	{
		v[nph].pp[n] = p1;
		v[nph].xx[n] = xmod(v[nph].pp[n], nph, 1, nr, &v[nph].rr[n]);
		return;
	}
	if(y[0] <= yi)
	{
		ps0 = p0;
		ps1 = p1;
	}
	else
	{
		ps0 = p1;
		ps1 = p0;
	}
	xi = (p0*yi - p1*y[0])/(yi - y[0]);
	/*
	 * Iterate.
	 */
	for(m = 1; m < 20; m++)
	{
		if((xi-ps0)*(xi-ps1) > 0.) xi = .5*(ps0 + ps1);
		/*
		 * Save the current best guess of the zero.
		 */
		y[m] = yi;
		x[m] = xi;
		/*
		 * Start iteration at the current best guess of the zero.
		 */
		yi = xmod(xi, nph, 1, nr, &r0) - xg;
		/*
		 * Check for convergence.
		 */
		if(fabs(yi) <= xtol)
		{
			break;
		}
		else
		{
			if(yi <= 0.)
			{
				ps0 = xi;
			}
			else
			{
				ps1 = xi;
			}
			for(j = 0; j < m; j++)
			{
				xi = (x[j]*yi - xi*y[j])/(yi - y[j]);
			}
		}
	}
	if(m == 20)
	{
		fprintf(stderr, "finrng: Iteration did not converge:\n");
		fprintf(stderr, "n= %d nph= %d xg= %7.4f\n", n, nph, xg);
		exit(1);
	}
	/*
	 * Return the final best guess of the zero.
	 */
	v[nph].pp[n] = xi;
	v[nph].xx[n] = yi + xg;
	v[nph].rr[n] = r0;
}

static void
zgrid(int nph, int *m0)
{
	int i, j, l, n1, m;
	static float tol = 1.e-6;
	/*
	 * depth grid
	 */

	for(i = n; i > 0; i--)
	{
		if(fabs(v[nph].u[0] - pb[i]) <= tol) break;
	}
	n1 = i;
	fprintf(fp10, "zgrid %d %E %e\n", n1, v[nph].u[0], (float)pb[n1]);

	l = 1;
	i = n1-1;
	j = 0;
	v[nph].pm[0] = pb[n1];
	v[nph].zm[0] = z[0];
	v[nph].zm0[0] = 1.;
	v[nph].ndex[0] = n1;
	for(;;)
	{
		if(v[nph].u[j+1] <= pb[i])
		{
			v[nph].pm[l] = pb[i];
			v[nph].zm[l] = findep((float)pb[i], j, nph);
			v[nph].zm0[l] = exp(v[nph].zm[l]);
			v[nph].ndex[l] = i;
			l++;
			if(i <= 1) break;
			i--;
		}
		else if(v[nph].u[j+1] <= v[nph].u[j])
		{
			j++;
		}
		else
		{
			i += 2;
			while(v[nph].u[j+1] < pb[i]) j++;
			if(v[nph].u[j+1] == pb[i]) j++;
			v[nph].pm[l] = pb[i];
			v[nph].zm[l] = findep((float)pb[i], j, nph);
			v[nph].zm0[l] = exp(v[nph].zm[l]);
			v[nph].ndex[l] = i;
			l++;
			if(i <= 1) break;
			i--;
		}
	}
	m = l;
	v[nph].pm[m] = pb[0];
	v[nph].zm[m] = -1e+6;
	v[nph].zm0[m] = 0.;
	v[nph].ndex[m] = 1;
	*m0 = m-1;
}
/*
 *
 * calls emdlv 
 * 
 * Function findep returns the equivalent depth for which the model
 * slowness is u0.  If nph  =  1, P slownesses are searched.  If nph = 2,
 * S slownesses are searched.  K is taken as the index into equivalent
 * depth near where the desired slowness should be found.  Function
 * evaluations are made until u0 is fit to a relative precision of aep
 * by a process of inverse iterative interpolation using Aitken's method.
 */
#define amax1(a,b) ((a > b) ? a : b)

static float
findep(float u0, int k, int nph)
{
	int /* i, */ j, m, kph;
	float x1, x2, a0, xi, yi, vvp, vvs;
	static float aep = 1.e-6;
	static float x[20], y[20];

	a0 = 1./xn;
	if(fabs(z[k] - z[k+1]) <= -aep*z[k])
	{
		return(z[k]);
	}
	kph = nph;
	x1 = exp(z[k+1]);
	x2 = exp(z[k]);
	if(a0*x1 < roc) kph = 0;
	/*
	 * Set up iteration with first two trial points, x1 and x2.
	 */
	if(k+1 < nk-1 && fabs(z[k+1]-z[k+2]) <= -aep*z[k+1])
	{
		emdlv(a0*x1*(1.+aep), &vvp, &vvs);
	}
	else
	{
		emdlv(a0*x1, &vvp, &vvs);
	}	
	y[0] = (kph == 0) ? pn*x1/vvp-u0 : pn*x1/vvs-u0;
	if(fabs(y[0]) <= aep*u0)
	{
		return(z[k+1]);
	}
	x[0] = x1;
	if(k > 0 && fabs(z[k-1]-z[k]) <= -aep*z[k])
	{
		emdlv(a0*x2*(1.-aep), &vvp, &vvs);
	}
	else
	{
		emdlv(a0*x2, &vvp, &vvs);
	}
	yi = (kph == 0) ? pn*x2/vvp-u0 : pn*x2/vvs-u0;
	if(fabs(yi) <= aep*u0 || yi == y[0])
	{
		return(z[k]);
	}
	xi = (x1*yi - x2*y[0])/(yi - y[0]);
	if(fabs(xi-x1) <= aep*amax1(fabs(xi),aep) || fabs(xi-x2) <=
		aep*amax1(fabs(xi),aep))
	{
		return(log(amax1(xi,aep)));
	}
	/* Iterate.
	 */
	for(m = 1; m < 20; m++)
	{
		/* Save the current best guess of the zero.
		 */
		y[m] = yi;
		x[m] = xi;
		/*
		 * Start iteration at the current best guess of the zero.
		 */
		emdlv(a0*xi, &vvp, &vvs);
		yi = (kph == 0) ? pn*xi/vvp-u0 : pn*xi/vvs-u0;
		for(j = 0; j < m; j++)
		{
			if(yi == y[j])
			{
				return(log(amax1(xi, aep)));
			}
			xi = (x[j]*yi - xi*y[j])/(yi - y[j]);
		}
		/* Check for convergence.
		 */
		if(fabs(xi-x[m]) <= aep*amax1(fabs(xi),aep))
		{
			return(log(amax1(xi, aep)));
		}
	}
	/*
	 * Return the final best guess of the zero.
	 */
	m = 0;
	return(log(amax1(xi, aep)));
}

static void
brkpts(int m, int nph)
{
/* calls efec, efe4, efe8, and phcod 
 *
 * sets up discontinuity information
 */
	int lb, lc, i, isw;
	static double dtol = 1.e-6;

	lb = 0;
	v[nph].lbrk[0] = v[nph].ndex[0];
	lc = -1;
	i = 0;
	isw = 1;
   for(;;)
   {
	/*
	 * Search for discontinuities.
	 */
	while(fabs(v[nph].zm[i]-v[nph].zm[i+1]) > -v[nph].zm[i+1]*dtol)
	{
		if(i >= m)
		{
			/* We have hit the bottom of the model.
			 */
			phcod(&lc, nph, (float)v[nph].zm[m], -1);
			efe4(lb+1, v[nph].lbrk);
			efec(lc+1, v[nph].code);
			lbb[nph] = lb+1;
			lcb[nph] = lc+1;
			return;
		}
		i++;
		/*
		 * No discontinuity.
		 */
		if(isw == 1)
		{
			/* Have we hit a high slowness zone?
			 */
			if(v[nph].pm[i] > v[nph].pm[i-1])
			{
				/* Yes, flag it.
				 */
				isw = 2;
				/*
				 * If the high slowness zone is topped with
				 * a discontinuity, go back to the usual
				 * processing.
				 */
				if(v[nph].lbrk[lb] != v[nph].ndex[i-1])
				{
					/* Otherwise, mark the grazing ray to
					 * the top of the zone.
					 */
					lb++;
					v[nph].lbrk[lb] = v[nph].ndex[i-1];
					phcod(&lc,nph,(float)v[nph].zm[i-1],-1);
				}
			}
		}
		else if(isw == 2)
		{
			/* We are already in a high slowness zone.  See if we
			 * have hit bottom.
			 */
			if(v[nph].pm[i] < v[nph].pm[i-1])
			{
				/* Yes we have.  Reset the high slowness
				 * zone flag.
				 */
				isw = 1;
			}
		}
	}
	/* Discontinuity!  See what kind.
	 */
	if(v[nph].pm[i+1] <= v[nph].pm[i])
	{
		/* Velocity increase.  Flag the bottom of the step.
		 */
		lb++;
		v[nph].lbrk[lb] = v[nph].ndex[i];
		phcod(&lc, nph, (float)v[nph].zm[i], 1);
		if(v[nph].lbrk[lb] >= v[nph].lbrk[lb-1])
		{
			lb--;
			lc--;
			strcpy(v[nph].code[lc],v[nph].code[lc+1]);
		}
		/* Find the top of the discontinuity.
		 */
		do
		{
			if(i >= m)
			{
				/* We have hit the bottom of the model.
				 */
				phcod(&lc, nph, (float)v[nph].zm[m], -1);
				efe4(lb+1, v[nph].lbrk);
				efec(lc+1, v[nph].code);
				lbb[nph] = lb+1;
				lcb[nph] = lc+1;
				return;
			}
			i++;
		} while(fabs(v[nph].zm[i]-v[nph].zm[i+1]) <=
			-v[nph].zm[i+1]*dtol);
		/*
		 * Flag the top of the step.
		 */
		lb++;
		v[nph].lbrk[lb] = v[nph].ndex[i];
		if(v[nph].lbrk[lb] >= v[nph].lbrk[lb-1])
		{
			lb--;
			lc--;
		}
	}
	else
	{
		/* Velocity decrease. Flag the top of the step.
		*/
		lb++;
		v[nph].lbrk[lb] = v[nph].ndex[i];
		phcod(&lc, nph, (float)v[nph].zm[i], -1);
		/*
		* Find the bottom of the discontinuity.
		*/
		do
		{
			if(i >= m)
			{
				/* We have hit the bottom of the model.
				 */
				phcod(&lc, nph, (float)v[nph].zm[m], -1);
				efe4(lb+1, v[nph].lbrk);
				efec(lc+1, v[nph].code);
				lbb[nph] = lb+1;
				lcb[nph] = lc+1;
				return;
			}
      			i++;
		} while(fabs(v[nph].zm[i]-v[nph].zm[i+1]) +
				v[nph].zm[i+1]*dtol <= 0.);
	}
    }
}

static void
phcod(int *lc, int nph, float z0, int kfl)
{
	/* set up phase codes
	 */
	static char tag[2], suf[2], pre[3], dep[5], buf[11];
	static int ln = 8;
	int idep, i, j;
	float d0, r0;

	d0 = (1. - exp(z0))/xn;
	r0 = 1./xn - d0;
	idep = d0 + .5;
	if(*lc < 0)
	{
		*lc = 0;
		strcpy(pre, " ");
		strcpy(suf, " ");
		if(nph == 0) strcpy(tag, "P");
		else strcpy(tag, "S");
		strcpy(v[nph].code[*lc], "t");
		strcat(v[nph].code[*lc], tag);
		strcat(v[nph].code[*lc], "g");
	}
	else
	{
		*lc = *lc + 1;
		if(idep <= 70)
		{
			strcpy(v[nph].code[*lc], "t");
			strcat(v[nph].code[*lc], tag);
			strcat(v[nph].code[*lc], "b");
			if(v[nph].code[*lc-1][2] == 'b')
				v[nph].code[*lc-1][2] = 'g';
			if(v[nph].code[*lc-2][2] == 'b')
				v[nph].code[*lc-2][2] = 'g';
		}
		else
		{
			strcpy(v[nph].code[*lc], "t");
			strcat(v[nph].code[*lc], tag);
			if(pre[0] != ' ')
			{
				strcat(v[nph].code[*lc], pre);
				strcat(v[nph].code[*lc], suf);
				strcat(v[nph].code[*lc], tag);
			}
			if(v[nph].code[*lc-2][2] == 'g' ||
				v[nph].code[*lc-2][2] == 'b')
			{
				v[nph].code[*lc][2] = 'n';
				v[nph].code[*lc][3] = '\0';
				strcpy(v[nph].code[*lc-1], "r");
				strcat(v[nph].code[*lc-1], tag);
				strcat(v[nph].code[*lc-1], "m");
				strcat(v[nph].code[*lc-1], tag);
			}
			j = -1;
			for(i = 0; i < ln; i++)
			{
				if(v[nph].code[*lc][i] != ' ')
				{
					j++;
					v[nph].code[*lc][j] =
						v[nph].code[*lc][i];
				}
			}
			v[nph].code[*lc][ln] = '\0';
/*			for(j++; j < ln; j++) v[nph].code[*lc][j] = ' '; */
			if(!strncmp(v[nph].code[*lc]+1, "PKP", 3))
				strcpy(v[nph].code[*lc]+1, "PKPab");
			if(!strncmp(v[nph].code[*lc]+1, "PKIKP", 5))
				strcpy(v[nph].code[*lc]+1, "PKPdf");
			if(!strncmp(v[nph].code[*lc]+1, "SKS", 3))
				strcpy(v[nph].code[*lc]+1, "SKSab");
			if(!strncmp(v[nph].code[*lc]+1, "SKIKS", 5))
				strcpy(v[nph].code[*lc]+1, "SKSdf");
			if(fabs(r0-roc) <= 20.)
			{
				strcpy(pre, "K");
				if(kfl <= 0) return;
				*lc = *lc + 1;
				strcpy(v[nph].code[*lc], "r");
				strcat(v[nph].code[*lc], tag);
				strcat(v[nph].code[*lc], "c");
				strcat(v[nph].code[*lc], tag);
				return;
			}
			if(fabs(r0-ric) <= 20.)
			{
				strcpy(pre, "KI");
				strcpy(suf, "K");
				if(kfl <= 0) return;
				*lc = *lc + 1;
				strcpy(v[nph].code[*lc], "r");
				strcat(v[nph].code[*lc], tag);
				strcat(v[nph].code[*lc], "KiK");
				strcat(v[nph].code[*lc], tag);
				return;
			}
		}
	}
	if(kfl <= 0) return;
	*lc = *lc + 1;
	sprintf(dep, "%4d", idep);
	strcpy(buf, tag);
	strcat(buf, pre);
	strcat(buf, "d");
	strcat(buf, dep);
	strcat(buf, suf);
	strcat(buf, tag);
	strcpy(v[nph].code[*lc], "r");
	for(i = j = 0; buf[i] != '\0'; i++)
	{
		if(buf[i] != ' ')
		{
			j++;
			v[nph].code[*lc][j] = buf[i];
		}
	}
	v[nph].code[*lc][j] = '\0';

/*	for(j++; j < ln; j++) v[nph].code[*lc][j] = ' '; */
}

static void
efe4(int n, int *na)
{
/*
 * Integer array na[n] is transposed end-for-end.
 */
	int i, j, n2, nb;

	if(n <= 1) return;
	n2 = n/2;
	for(i = 0, j = n-1; i < n2; i++, j--)
	{
		nb = na[i];
		na[i] = na[j];
		na[j] = nb;
	}
}

static void
efe8(int n, double *da)
{
/*
 * Double precision array da[n] is transposed end-for-end.
 */
	int i, j, n2;
	double db;

	if(n <= 1) return;
	n2 = n/2;
	for(i = 0, j = n-1; i < n2; i++, j--)
	{
		db = da[i];
		da[i] = da[j];
		da[j] = db;
	}
}

static void
efec(int n, char **a)
{

/*
 * Character pointer array a[n] is transposed end-for-end.
 */
	char *b;
	int i, j, n2;

	if(n <= 1) return;
	n2 = n/2;
	for(i = 0, j = n-1; i < n2; i++, j--)
	{
		b = a[i];
		a[i] = a[j];
		a[j] = b;
	}
}

/*
 * program remodl.
 *
 */
int main(int argc, char **argv)
{
	char filespec[100];
	char modnam[21] = "iasp91";
	char datapath[1024];

	int i, j, k, l, j1, k1, /* k2, */ ncr, n1, n2, lz, mm, nrec, nph, ndasr;
	int mt[2], kb[2], lt[2];
	float a0, rn;
	struct
	{
		int lvz[jlvz];
		double taup[nsl1], xp[nsl1], taul[jlvz], xl[jlvz];
	} t[2];
	double ttau, tx, pmj, pmi, zmj, zmi, zlim, zmax, plim, zic, zoc;
	static double tol = 1.e-6, dtol = 1.e-6, xtol = 1., dmax = 800.;

	if ( (argc > 1) && (strcmp(argv[1], "-h") == 0) ) {
		printf("Usage: tau_remodl [model] [path]\n\n");
		printf("The default model is 'iasp91'. The file extensions '.tvel' is automatically\n"
		       "appended and the file is expected to be found in the current working directory.\n\n"
		       "An alternative input path can be passed with [path] as second argument.\n");
		return 0;
	}

	if ( argc > 1 )
		strncpy(modnam, argv[1], sizeof(modnam)-1);

	if ( argc > 2 )
		strncpy(datapath, argv[2], sizeof(datapath)-1);
	else
		datapath[0] = '\0';

	printf("libtau: model: %s, path: %s\n", modnam, datapath);

	for(i = 0; i < nbr1; i++)
	{
		v[0].code[i] = (char *)malloc(10);
		v[1].code[i] = (char *)malloc(10);
	}

	/* input the desired increment between model radii (50.) and get:
	 * nk = number of model radii, ncr = index of core-mantle radius, and
	 * modnam = model name.
	 */
	if ( rough(50., &nk, &ncr, modnam, datapath) ) {
		fprintf(stderr, "ERROR: input not read\n");
		return 1;
	}

	a0 = r[0];

	fp10 = fopen("remodl1.lis", "w"); 
	fprintf(fp10, "\n %d %d %10.2f\n", nk, ncr, a0);
	for(i = 0; i < nk; i++)
	{
		fprintf(fp10,"%d %10.2f %10.4f %10.4f\n",i,r[i],vp[i],vs[i]);
	}
	pn = vs[0];
	xn = 1/r[0];
	tn = pn*xn;
	for(i = 0; i < nk; i++)
	{
		rn = r[i]*xn;
		z[i] = log(rn);
		v[0].u[i] = rn*pn/vp[i];
		v[1].u[i] = (i <= ncr) ? rn*pn/vs[i] : v[0].u[i];
		
		fprintf(fp10, "%d %10.2f %10.2f %12.6f %12.6f\n",
			i, r[i], a0*z[i], v[0].u[i], v[1].u[i]);
	}
	fprintf(fp10, "delx = %10.2f %10.2f\n", (float)delx[0], (float)delx[1]);
	fprintf(fp10, "dpmax = %12.6f %12.6f\n", (float)dpmax[0],
				(float)dpmax[1]);
	fprintf(fp10, "drmax = %12.6f %12.6f\n", (float)drmax[0],
				(float)drmax[1]);
	delx[0] = xn*delx[0];
	delx[1] = xn*delx[1];
	xtol = xtol/a0;

	nc = 0;
	findcp(ncr);

	fprintf(fp10, "critical points\n");
	for(i = 0; i < nc; i++) fprintf(fp10, "%d %12.6f\n", i, ucrt[i]);

	pgrid(0, &n1, xtol);
	pgrid(1, &n2, xtol);

	for(i = n = j1 = k1 = 0; i < nc; i++)
	{
		if(v[0].pp[0] >= ucrt[i] - tol)
		{
			for(j = j1; j < n1-1; j++)
			{
				if(fabs(v[0].pp[j] - ucrt[i+1]) <= tol)
					break;
			}
		}
		else j = 0;

		for(k = k1; k < n2-1; k++)
		{
			if(fabs(v[1].pp[k] - ucrt[i+1]) <= tol) break;
		}
		if(j-j1 <= k-k1)
		{
			for(l = k1; l <= k; l++) {
				pb[n++] = v[1].pp[l];
			}
		}
		else
		{
			for(l = j1; l <= j; l++) pb[n++] = v[0].pp[l];
		}
		j1 = j + 1;
		k1 = k + 1;
	}
	fclose(fp10); 

	fp10 = fopen("remodl2.lis", "w"); 
	
	fprintf(fp10, "1 %12.6f\n", pb[0]);
	for(i = 1; i < n; i++)
	{
		fprintf(fp10, "%d %12.6f %12.2e\n", i, pb[i], pb[i-1]-pb[i]);
	}
	efe8(n, pb);
	n1 = n;
	n--;

	zgrid(0, &mt[0]);
	mm = mt[0];

	for(i = 0; i <= mm; i++)
	{
		fprintf(fp10, "%d %12.6f %12.6f %10.2f %12.4e %d\n", i,
			v[0].pm[i], v[0].zm[i], a0*(v[0].zm0[i]-v[0].zm0[i+1]),
			a0*(v[0].zm[i]-v[0].zm[i+1]), v[0].ndex[i]);
	}
	fprintf(fp10, "%d %12.6f %12.6f %d\n", mm+1, v[0].pm[mm+1],
			v[0].zm[mm+1], v[0].ndex[mm+1]);

	zgrid(1, &mt[1]);

	mm = mt[1];
	for(i = 0; i <= mm; i++)
	{
		fprintf(fp10, "%d %12.6f %12.6f %10.2f %12.4e %d\n", i,
			v[1].pm[i], v[1].zm[i], a0*(v[1].zm0[i]-v[1].zm0[i+1]),
			a0*(v[1].zm[i]-v[1].zm[i+1]), v[1].ndex[i]);
	}
	fprintf(fp10, "%d %12.6f %12.6f %d\n", mm+1, v[1].pm[mm+1],
			v[1].zm[mm+1], v[1].ndex[mm+1]);

	/* Set up break pointers.
	 */
	brkpts(mt[0], 0);

	for(i = j = 0; i < lbb[0]; i++)
	{
		fprintf(fp10, "%d ", v[0].lbrk[i]);
		if(++j == 20)
		{
			fprintf(fp10, "\n");
			j = 0;
		}
	}
	if(j != 0) fprintf(fp10, "\n");

	for(i = 0; i < lcb[0]; i++) fprintf(fp10, "%d %s\n", i, v[0].code[i]);

	kb[0] = v[0].lbrk[lbb[0]-1] + 1;
	brkpts(mt[1], 1);
	for(i = j = 0; i < lbb[1]; i++)
	{
		fprintf(fp10, "%d ", v[1].lbrk[i]);
		if(++j == 20)
		{
			fprintf(fp10, "\n");
			j = 0;
		}
	}
	if(j != 0) fprintf(fp10, "\n");
	for(i = 0; i < lcb[1]; i++) fprintf(fp10, "%d %s\n", i, v[1].code[i]);

	kb[1] = v[1].lbrk[lbb[1]-1] + 1;
	fprintf(fp10, "\nn1 kb %d %d %d\n", n1, kb[0], kb[1]);

	ndasr = 8*(1 + 2*kb[1]) + 4;

	snprintf(filespec, sizeof(filespec)-1, "remodl_%s.tbl", modnam);
	fpout = fopen(filespec, "wb");

	fprintf(fp10, "reclength for dasign: %d\n", ndasr);

	zmax = log((a0-dmax)*xn);
	zic = log(ric * xn);
	zoc = log(roc * xn);
	zlim = zmax;
	fprintf(fp10, "\nzmax zoc zic %E %E %E\n", zmax, zoc, zic);

	/*
	 * Loop over phases.
	 */

	nrec = 0;
	for(nph = 0; nph < 2; nph++)
	{
		j = 0;
		lz = 0;
		n1 = kb[nph];
		for(i = 0; i < n1; i++)
		{
			t[nph].taup[i] = 0.;
			t[nph].xp[i] = 0.;
		}
		t[nph].taup[n1-1] = tn*1.e-6;
		t[nph].xp[n1-1] = xn*1.e-6;
		n = n1 - 1;
		mm = mt[nph] + 1;
		v[nph].ndex[0] = -1;
		v[nph].loc[0] = -1;
		zmi = v[nph].zm[0];
		pmi = v[nph].pm[0];
		/*
		 * Loop over model slownesses.
		 */
		for(i = 1; i <= mm; i++)
		{
			zmj = zmi;
			zmi = v[nph].zm[i];
			pmj = pmi;
			pmi = v[nph].pm[i];
			if(fabs(zmj-zmi) > 0.)
			{
				/* Collect the tau and x integrals.
				 */
				for(k = 0; k < n; k++)
				{
					if(pmi < pb[k])
					{
						n = k;
						break;
					}
/* XXX XXX XXX				fprintf(stderr,"tauint k=%d pb[k]=%E %E %E %E %E\n", k, pb[k], pmj,pmi,zmj,zmi); */
					tauint(pb[k],pmj,pmi,zmj,zmi,&ttau,&tx);
					t[nph].taup[k] += ttau;
					t[nph].xp[k] += tx;
				}
				if(n > 1)
				{
					if(pb[n-1] == pb[n-2]) n--;
				}
				if(zmj >= zlim)
				{
				    j++;
				    if(zmj >= zmax) nrec++;
				    v[nph].zm[j] = zmi;
				    v[nph].pm[j] = pmi;
				    v[nph].ndex[j] = nrec;
				    v[nph].loc[j] = ftell(fpout);
				    if(zmj >= zmax)
				    {
					fprintf(fp10, "lev1 %d %d %d %e %e\n",
					    j, n, nph, (float)v[nph].pm[j],
					    (float)v[nph].zm[j]);
					fwrite(&zmi, 8, 1, fpout);
					fwrite(&n, 4, 1, fpout);
					fwrite(t[nph].taup, 8, n, fpout);
					fwrite(t[nph].xp, 8, n, fpout);
				    }
				}
			}
			else if(fabs(zmi-zoc) <= dtol || fabs(zmi-zic) <= dtol)
			{
				if(fabs(zmi-v[nph].zm[j]) > dtol)
				{
				    j++;
				    nrec = nrec+1;
				    v[nph].zm[j] = zmi;
				    v[nph].pm[j] = pmi;
				    v[nph].ndex[j] = nrec;
				    v[nph].loc[j] = ftell(fpout);
				    fprintf(fp10,"lev2 %d %d %d %e %e\n",
					j, n, nph, (float)v[nph].pm[j],
					(float)v[nph].zm[j]);
				    fwrite(&zmi, 8, 1, fpout);
				    fwrite(&n1, 4, 1, fpout);
				    fwrite(t[nph].taup, 8, n1,fpout);
				    fwrite(t[nph].xp, 8, n1, fpout);
				}
				else continue;
			}
			else if(zmi >= zmax)
			{
				if(fabs(zmi-v[nph].zm[j-1]) > dtol) j++;
				v[nph].zm[j] = zmi;
				v[nph].pm[j] = pmi;
				v[nph].ndex[j] = v[nph].ndex[j-1];
				v[nph].loc[j] = v[nph].loc[j-1];
			}
			if(pmi > pmj)
			{
				if(lz <= 0 || t[nph].lvz[lz] != n-1)
				{
				    t[nph].lvz[lz] = n-1;
				    t[nph].taul[lz] = t[nph].taup[n-1];
				    t[nph].xl[lz] = t[nph].xp[n-1];
				    fprintf(fp10,"lvz %d %d %d %e %e\n",
					lz, n, nph, (float)t[nph].taul[lz],
					(float)t[nph].xl[lz]);
				    lz++;
				}
			}
		}
		j++;
		nrec++;
		v[nph].zm[j] = zmi;
		v[nph].pm[j] = pmi;
		v[nph].ndex[j] = nrec;
		v[nph].loc[j] = ftell(fpout);
		fprintf(fp10,"lev3 %d %d %d %e %e\n", j, n, nph,
			(float)v[nph].pm[j], (float)v[nph].zm[j]);
		fwrite(&zmi, 8, 1, fpout);
		fwrite(&n1, 4, 1, fpout);
		fwrite(t[nph].taup, 8, n1,fpout);
		fwrite(t[nph].xp, 8, n1, fpout);

		mt[nph] = j+1;
		lt[nph] = lz;
		if(lz > 0)
		{
			efe4(lz, t[nph].lvz);
			efe8(lz, t[nph].taul);
			efe8(lz, t[nph].xl);
		}

		if(nph < 1)
		{
			for(i = 0; i < mm; i++)
			{
				if(v[0].zm[i] < zmax) break;
			}
			plim = v[0].pm[i];
			fprintf(fp10, "i zmax plim v[0].zm[i] = %d %E %e %e\n",
				i, zmax, plim, v[0].zm[i]);
			for(i = 0; i < mm; i++)
			{
      				if(v[1].pm[i] <= plim) break;
			}
			zlim = v[1].zm[i];
			fprintf(fp10, "i plim zlim = %d %e %e\n",i,plim,zlim);
		}
	}
	fclose(fpout);

	snprintf(filespec, sizeof(filespec)-1, "remodl_%s.hed", modnam);
	fpout = fopen(filespec, "wb");
	fwrite(&ndasr, 4, 1, fpout);
	fwrite(modnam, 1, 20, fpout);
	fwrite(&zmax, 8, 1, fpout);
	fwrite(&zoc, 8, 1, fpout);
	fwrite(&zic, 8, 1, fpout);
	fwrite(kb, 4, 2, fpout);
	fwrite(pb, 8, n1, fpout);
	fwrite(mt, 4, 2, fpout);
	fwrite(lt, 4, 2, fpout);
	fwrite(lbb, 4, 2, fpout);
	fwrite(lcb, 4, 2, fpout);
	fwrite(&xn, 4, 1, fpout);
	fwrite(&pn, 4, 1, fpout);
	fwrite(&tn, 4, 1, fpout);
	for(nph = 0; nph < 2; nph++)
	{
		fwrite(v[nph].lbrk, 4, lbb[nph], fpout);
		for(i = 0; i < lcb[nph]; i++)
			fwrite(v[nph].code[i], 8, 1, fpout);
		fwrite(v[nph].zm,   8, mt[nph], fpout);
		fwrite(v[nph].pm,   8, mt[nph], fpout);
		fwrite(v[nph].ndex, 4, mt[nph], fpout);
		fwrite(v[nph].loc,  4, mt[nph], fpout);
		fwrite(t[nph].lvz,  4, lt[nph], fpout);
		fwrite(t[nph].taul, 8, lt[nph], fpout);
		fwrite(t[nph].xl,   8, lt[nph], fpout);
	}
	fclose(fpout);
	fclose(fp10);
	return 0;
}
