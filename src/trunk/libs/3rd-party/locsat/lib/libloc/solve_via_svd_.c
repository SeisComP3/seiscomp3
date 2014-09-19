
/*
 * NAME
 * 	solve_via_svd_ -- Perform Singular Value Decomposition.

 * FILE
 * 	solve_via_svd_.c

 * SYNOPSIS
 * 	Compute hypocentral solution vector by Singular Value Decomposition
 * 	of a given system matrix.

 * DESCRIPTION
 * 	Function.  Decompose an arbitrary NxM rectangular matrix, G,
 * 	via the method of Singular Value Decomposition (SVD) into its
 * 	component parts using a standard LINPACK routine.  This
 * 	mini-driver determines a solution of the form:

 * 	G =	U * sval * V-transpose, where U and V contain the left
 * 		and right singular vectors, respectively, while sval
 * 		holds the corresponding singular values.

 *	It is the rank of G that determines the maximum number of possible 
 *	singular values that are calculated.  So, if nd < np, then the 
 *	maximum rank of at[] is nd, and only nd singular values can be 
 *	calculated.  If the variable, info, is non-zero, then the number 
 *	of singular values will not be np.

 *	Given, k = MIN(nd,np)
 *	  Subr. dsvdc fills U with left singular vectors stored as,

 *			| vector 1   vector 2   ... vector k  |
 *			|   u1(1)      u2(1)    ...    uk(1)  |
 *			|   u1(2)      u2(2)    ...    uk(1)  |
 *	  u(np,np) =	|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|   u1(np)     u2(np)   ...    uk(np) |

 *	Given, k = MIN(nd,np)
 *	  Subr. dsvdc fills V with right singular vectors stored as,

 *			| vector 1   vector 2   ... vector k  |
 *			|   v1(1)      v2(1)    ...    vk(1)  |
 *			|   v1(2)      v2(2)    ...    vk(1)  |
 *	  v(nd,nd) =	|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|     .          .      ...      .    |
 *			|   v1(nd)     v2(nd)   ...    vk(nd) |

 *	  Then given,
 *	    G * m = d
 *	  Subr. dsvdc decomposes G as,
 *	    G  => ( V * LAMBDA * U-transpose )
 *	  So,
 *	    G * m = ( V * LAMBDA * U-transpose ) * m
 *	  And,
 *	    m = ( U * LAMBDA-inverse * V-transpose ) * d

 * 	  it's not,	Gm = (U * sval * V-transpos)m,
 * 	  but,		Gm = (V * sval * U-transpos)m
 * 	  so,		m  = (U * sval-inverse * V-Transpose) * d

 * 	---- Indexing ----
 * 	i = 0, *nd - 1;	j = 0, *np - 1;

 * 	---- On entry ----
 * 	nd:		Number of data (rows; observations)
 * 	np:		Number of parameters (columns)
 * 	maxp:		Leading dimension of at[] and covar[]
 * 	icov:		Compute covariance matrix: 1 = No; 2 = Yes
 * 	damp:		Percent damping applied to the diagonal elements of
 * 			at[] [g[]].  That is, damping as a percent of the
 * 			largest singular value.  If damp < 0.0, only damp when
 * 			condition number > 30000.0.
 * 	at[j][i]:	Transpose of the system matrix
 * 	d[i]:		Value of i'th datum -- data vector

 * 	---- On return ----
 * 	condit[0]:	True condition number of non-zero singular values
 * 			returned from dsvdc.  That is, largest sval/smallest
 * 			sval calculated before scaling limit enforced.
 * 	condit[1]:	Effective condition number of non-zero singular
 * 			values.  In this case, the actual largest
 * 			sval/smallest sval retained for use in obtaining
 * 			solution.
 * 	xsol[j]:	Solution vector
 * 	covar[j][j]:	Parameter covariance matrix [square-symmetric]
 * 	epimp[i]:	Epicenter importance of i'th datum
 * 	rank:		Effective rank of matrix
 *	ierr:		If subr. dsvdc cannot decompose matrix, ierr = 6.

 * 	---- Subroutines called ----
 * 	Local
 * 		dsvdc:	LINPACK Singular Value Decomposition routine

 * DIAGNOSTICS
 * 	Makes checks for invalid singular values and simply ignores them.
 * 	Because of the way subr. dsvdc returns the decomposition,

 * NOTES
 * 	Beware of problems associated with variable, info, from LINPACK
 * 	subroutine dsvdc.

 * SEE ALSO
 * 	LINPACK, John Dongarra for explanation of SVD application along
 * 	with corresponding subroutines; and NETLIB (netlib@ornl.gov) for
 * 	obtaining any LINPACK source code.

 * AUTHOR
 * 	Walter Nagy, June 1991.
 */


#include "aesir.h"

#ifdef SCCSID
static char	SccsId[] = "@(#)solve_via_svd_.c	44.1	9/20/91";
#endif

#define MAX_PARAM	4
#define MAX_DATA	9999
#define COND_NUM_LIMIT	30000.0
#define	MIN(a,b)	((a) <= (b) ? (a) : (b))

static int maxdata	= MAX_DATA;

int
solve_via_svd__ (icov, nd, np, maxp, at, d, damp, cnvgtst, condit, xsol,
		covar, epimp, rank, ierr)

int	*icov, *ierr, *maxp, *nd, *np;
float	*damp, *epimp;
double	*at, *cnvgtst, *condit, *covar, *d, *rank, *xsol;

{
	static int	i, icnt, info, j, job, k, neig, norder;
	static double	*e, *g, *gscale, *gtr, smax, sum, *sval;
	static double	*tmp, *u, *v, *work;
	extern int	dsvdc_();
	double		applied_damping, dscale, frob, gtrnorm, rnorm;
	double		sqrt();

	e	= UALLOC(double, *nd);
	g	= UALLOC(double, MAX_PARAM * (*nd));
	u	= UALLOC(double, MAX_PARAM * MAX_PARAM);
	v	= UALLOC(double, MAX_DATA * (*nd));
	work	= UALLOC(double, MAX_PARAM);
	tmp	= UALLOC(double, MAX_PARAM);
	gscale	= UALLOC(double, MAX_PARAM);
	gtr	= UALLOC(double, MAX_PARAM);
	sval	= UALLOC(double, MAX_PARAM + 1);


	/*
	 *  Variable, norder, limits the maximum possible number of singular
	 *  values.  Variable, job, controls the singular values sent back
	 *  from dsvdc.  Current job setting tells dsvdc to ignore bogus
	 *  values and pass both left and right singular vectors back.
	 */

	norder	= MIN(*np,*nd);
	job	= 21;

	/*
	 *  Unit-column normalize at[] matrix and stuff it into g[], since 
	 *  subr. dsvdc overwrites original matrix upon its return.
	 */

	for (j = 0; j < *np; ++j)
		gscale[j] = 0.0;

	for (i = 0; i < *nd; ++i)
		for (j = 0; j < *np; ++j)
		{
			dscale = at[j + i*(*maxp)];
	    		gscale[j] += dscale*dscale;
		}

	for (j = 0; j < *np; ++j)
		gscale[j] = 1.0/sqrt(gscale[j]);

	/*
	 *  Scale origin terms of similar dimension to spatial terms,
	 *  assuming a medium velocity of 8 km./sec.
	 */

	/* gscale[0] = 0.125*gscale[0]; */

	for (i = 0; i < *nd; ++i)
		for (j = 0; j < *np; ++j)
			g[j + i*(*maxp)] = at[j + i*(*maxp)] * gscale[j];

	/*
	 *  Compute norms and undertake convergence test.
	 */

	frob	= 0.0;
	rnorm	= 0.0;
	gtrnorm	= 0.0;
	for (i = 0; i < *nd; ++i)
	{
                rnorm = rnorm + d[i]*d[i];
		for (j = 0; j < *np; ++j)
		{
			dscale = g[j + i*(*maxp)];
	    		frob  += dscale*dscale;
		}
	}

	for (j = 0; j < *np; ++j)
	{
		gtr[j] = 0.0;
		for (i = 0; i < *nd; ++i)
			gtr[j] = gtr[j] + g[j + i*(*maxp)]*d[i];
		gtrnorm = gtrnorm + gtr[j]*gtr[j];
	}
	*cnvgtst = gtrnorm / (frob*rnorm);
	/* printf ("gtrnorm = %g  frob = %g\n", gtrnorm, frob); */
	/* printf ("rnorm = %g  cnvgtst = %g\n", rnorm, *cnvgtst); */

	/*
	 *  Decompose the matrix into its right and left singular vectors,
	 *  u[] and v[], respectively, as well as the diagonal matrix of
	 *  singular values, sval.  That is, perform an SVD.  LINPACK routine, 
	 *  dsvdc, of John Dongarra is chosen here.
	 */

	dsvdc_ (g, maxp, np, nd, sval, e, u, maxp, v, &maxdata, work,
		&job, &info);

	if (info >= norder)
	{
		*ierr = 6;
		goto done;
	}

	/*
	 *  Adjust variable, neig, to control singular value cutoff, 
	 *  effectively determining which singular values to keep and/or 
	 *  ignore.
	 *      Note:	The good singular values are stored at the end of 
	 *		sval(), not the beginning.  smax is always sval(info+1)
	 *		with the descending values immediately following.
	 */

	neig = norder - info;

	/*
	 *  Avoid small singular values (limit condition number to ctol)
	 *  That is, set a singular value cutoff (i.e., singular values 
	 *  < pre-set limit, in order to obtain an effective condition number).
	 */

	smax = sval[info];
	for (j = info + 1; j < norder; ++j)
		if (smax/sval[j] > COND_NUM_LIMIT)
		{
			neig = j - 1;
			goto rank_of_matrix;
		}

	/*
	 *  Construct the real (condit[0]) and effective (condit[1])
	 *  condition numbers from the given singular values
	 */

rank_of_matrix:
	condit[0] = sval[info] / sval[norder-1];
	condit[1] = sval[info] / sval[info+1 + (neig-1) - 1];

	/* Apply damping, if necessary */

	if (*damp < 0.0)
	{
		if (condit[0] > 30.0)
		{
			/*
			 *  Apply damping of 1% largest singular value for
			 *  moderately ill-conditioned system.  Make this
			 *  5% for more severely ill-conditioned system and
			 *  10% for highly ill-conditioned problems.
			 */

			icnt = info+1 + (neig-1);
			applied_damping = 0.01;
			if (condit[0] > 300.0) 
				applied_damping = 0.05;
			if (condit[0] > 3000.0) 
				applied_damping = 0.10;
			for (i = info; i < icnt; ++i)
				sval[i] += smax * applied_damping;
		}
	}

	else
	{
		icnt = info+1 + (neig-1);
		for (i = info; i < icnt; ++i)
			sval[i] += smax * (*damp)*0.01;
	}

	/* Find solution vector -- First, compute (1/sval) * V-trans * d, */

	icnt = info+1 + (neig-1);
	for (j = info; j < icnt; ++j)
	{
		sum = 0.0;
		for (i = 0; i < *nd; ++i)
			sum += v[i + j*maxdata] * d[i];
		tmp[j] = sum/sval[j];
	}

	/*
	 *  then multiply by U, which yields the desired solution vector,
	 *  (i.e.,  xsol = U * (1/sval) * V-trans*d) = U * tmp
	 */

	for (j = 0; j < *np; ++j)
	{
		sum = 0.0;
		icnt = info+1 + (neig-1);
		for (i = info; i < icnt; ++i)
			sum += u[j + i*(*maxp)] * tmp[i];
		xsol[j] = sum*gscale[j];
	}

	/* Construct the parameter (model) covariance matrix, if requested */

	if (*icov < 2)
		goto done;

	icnt = info+1 + (neig-1);
	for (i = info; i < icnt; ++i)
		sval[i] = 1.0/(sval[i]*sval[i]);

	for (i = 0; i < *np; ++i)
	{
		for (j = 0; j <= i; ++j)
		{
			sum = 0.0;
			icnt = info+1 + (neig-1);
			for (k = info; k < icnt; ++k)
				sum += u[i + k*(*maxp)] * u[j + k*(*maxp)] * sval[k];
			covar[i + j*(*maxp)] = sum * gscale[j]*gscale[i];
			covar[j + i*(*maxp)] = covar[i + j*(*maxp)];
		}
	}

	/*
	 *  and then, the data importances (i.e., the diagonal elements of
	 *  the data resolution matrix)
	 */

	*rank = 0.0;
	for (i = 0; i < *nd; ++i)
	{
		sum = 0.0;
		for (j = 0; j < *np; ++j)
		{
			icnt = i + j*maxdata;
			sum += v[icnt]*v[icnt];
		}
		*rank += sum;
		epimp[i] = sum;
	}

done:
	UFREE(e);
	UFREE(g);
	UFREE(u);
	UFREE(v);
	UFREE(sval);
	UFREE(gscale);
	UFREE(gtr);
	UFREE(work);
	UFREE(tmp);

	return 0;
}

