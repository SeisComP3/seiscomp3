/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT TabValues

#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/utils/tabvalues.h>

#include <cfloat>
#include <fstream>


using namespace std;


#define INVALID_VALUE        -1
#define	MAX_DIST_SAMPLES      7
#define	MAX_DEPTH_SAMPLES     4
#define	MIN_NUM_DIST_SAMPLES  3


namespace {


template <typename T>
inline bool isValidValue(T v) {
	return v > INVALID_VALUE;
}


// Table interpolation
template <typename T>
int bracket(const T *array, int n, T value) {
	bool ascend = array[n-1] > array[0];
	int lo = -1;
	int hi = n;

	while ( hi - lo  > 1 ) {
		int mid = (lo + hi) / 2;
		if ( (value > array[mid]) == ascend )
			lo = mid;
		else
			hi = mid;
	}

	return lo;
}


template <typename T>
int ratint(const T *xa, const T *ya, int n, T x, T *y, T *dy) {
	int	i, m, ns;
	double dd, h, hh, t, w;
	vector<double> c, d;

	c.resize(size_t(n));
	d.resize(size_t(n));

	hh = fabs(x - xa[0]);

	for ( ns = 0, i = 0; i < n; ++i ) {
		h = fabs(x - xa[i]);
		if ( h == 0.0 ) {
			*y  = ya[i];
			*dy = 0.0;
			return 0;
		}
		else if ( h < hh ) {
			ns = i;
			hh = h;
		}

		c[i] = ya[i];
		d[i] = ya[i] + FLT_EPSILON; // Needed to prevent a rare
		                            // zero-over-zero condition
	}

	*y = ya[ns--];

	for ( m = 0; m < n-1; ++m ) {
		for ( i = 0; i < n-1-m; ++i ) {
			w  = c[i+1] - d[i];
			h  = xa[i+m+1] - x;
			t  = (xa[i] - x) * d[i]/h;
			dd = t - c[i+1];

			if ( dd == 0.0 ) {
				// Interpolating function has a pole at the
				// requested value of x.  Return error
				return -1;
			}

			dd = w / dd;
			d[i] = c[i+1] * dd;
			c[i] = t * dd;
		}

		*y += (*dy = (2*(ns+1) < (n-m-1) ? c[ns+1] : d[ns--]));
	}

	return 0;
}


template <typename T>
void spline(T *x, T *y, int n, T yp1, T ypn, T *y2) {
	int	i, k;
	T p, qn, sig, un;
	vector<T> u;

	u.resize(size_t(n));

	if ( yp1 > 0.99e30 )
		y2[0] = u[0] = 0.0;
	else {
		y2[0] = -0.5;
		u[0]  = (3.0/(x[1]-x[0])) * ((y[1]-y[0])/(x[1]-x[0])-yp1);
	}

	// Decomposition loop for tridiagonal algorithm

	for ( i = 1; i < n-1; ++i ) {
		sig   = (x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p     = sig*y2[i-1] + 2.0;
		y2[i] = (sig-1.0)/p;
		u[i]  = (y[i+1]-y[i])/(x[i+1]-x[i]) -
		        (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]  = (6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1]) / p;
	}

	if ( ypn > 0.99e30 )
		qn = un = 0.0;
	else {
		qn = 0.5;
		un = (3.0/(x[n-1] - x[n-2]))*(ypn - (y[n-1] - y[n-2]) /
		     (x[n-1] - x[n-2]));
	}

	y2[n-1] = (un - qn*u[n-2])/(qn*y2[n-2] + 1.0);

	// Back substituition loop of tridiagonal algorithm

	for ( k = n-2; k >= 0; --k )
		y2[k] = y2[k]*y2[k+1] + u[k];
}


template <typename T>
void splint(T *xa, T *ya, T *y2a, int n, T x, T *y) {
	int	klo, khi, k;
	T h, b, a;

	klo = 0;
	khi = n-1;

	while ( khi-klo > 1 ) {
		k = (khi + klo) >> 1;
		if ( xa[k] > x )
			khi = k;
		else
			klo = k;
	} // klo and khi now bracket the input value of x

	h = xa[khi] - xa[klo];
	if ( h == 0 )
		SEISCOMP_ERROR("Bad xa input to routine splint");

	a  = (xa[khi] - x)/h;
	b  = (x - xa[klo])/h;
	*y = a*ya[klo] + b*ya[khi] + ((a*a*a - a)*y2a[klo]
	     + (b*b*b - b)*y2a[khi])*(h*h)/6.0;
}


template <typename T>
void splint_deriv(T *xa, T *ya, T *y2a, int n, T x, T *y, T *dy, T *d2y) {
	int	klo, khi, k;
	T h, b, a;

	klo = 0;
	khi = n-1;

	while ( khi-klo > 1 ) {
		k = (khi + klo) >> 1;
		if ( xa[k] > x )
			khi = k;
		else
			klo = k;
	} // klo and khi now bracket the input value of x

	h = xa[khi] - xa[klo];
	if ( h == 0 )
		SEISCOMP_ERROR("Bad xa input to routine splint");

	a    = (xa[khi] - x)/h;
	b    = (x - xa[klo])/h;
	*y   = a*ya[klo] + b*ya[khi] + ((a*a*a - a)*y2a[klo]
	       + (b*b*b - b)*y2a[khi])*(h*h) / 6.0;
	*dy  = ((ya[khi]-ya[klo])/h) - (((3.0*a*a-1.0)*h*y2a[klo]) / 6.0)
	       + (((3.0*b*b-1.0)*h*y2a[khi]) / 6.0);
	*d2y = a*y2a[klo] + b*y2a[khi];
}


template <typename T>
void splie2(T *xs, T *xf, T **ysf, T nslow, int nfast, T **y2sf) {
	int	i;

	for ( i = 0; i < nslow; ++i )
		spline<T>(xf, ysf[i], nfast, T(1.0e30), T(1.0e30), y2sf[i]);
}


template <typename T>
void splin2(T *xs, T *xf, T **ysf, T **y2sf,
            int nslow, int nfast, T rxs, T rxf,
            T *rysf, T *drysf, T *d2rysf) {
	int	j;
	vector<T> ytmp, y2tmp;

	if ( nfast > nslow ) {
		ytmp.resize(size_t(nfast));
		y2tmp.resize(size_t(nfast));
	}
	else {
		ytmp.resize(size_t(nslow));
		y2tmp.resize(size_t(nslow));
	}

	for (j = 0; j < nslow; j++)
		splint(xf, ysf[j], y2sf[j], nfast, rxf, &y2tmp[j]);

	spline<T>(xs, &y2tmp[0], nslow, 1.0e30, 1.0e30, &ytmp[0]);
	splint_deriv<T>(xs, &y2tmp[0], &ytmp[0], nslow, rxs, rysf, drysf, d2rysf);
}


template <typename T>
T **matrix(int nrl, int nrh, int ncl, int nch) {
	int	i;
	T **m;

	// Allocate pointers to rows
	m = new T*[nrh-nrl+1];
	m -= nrl;

	// Allocate rows and set pointers to them
	for ( i = nrl; i <= nrh; ++i ) {
		m[i] = new T[nch-ncl+1];
		m[i] -= ncl;
	}

	return m;
}

template <typename T>
void free_matrix(T **m, int nrl, int nrh, int ncl, int nch) {
	int i;

	for ( i = nrh; i >= nrl; i--)
		delete [] (m[i] + ncl);
	delete [] (m + nrl);
}


}


namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TabValues::TabValues() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TabValues::read(const std::string &filename) {
	// Reset table
	x.clear();
	y.clear();
	values.clear();
	lowerUndefinedX = upperUndefinedX = Core::None;

	ifstream ifs;
	ifs.open(filename.c_str());
	if ( !ifs.is_open() )
		return false;

	string line;

	while ( getline(ifs, line) ) {
		size_t hashPos = line.find('#');
		if ( hashPos != string::npos )
			line.erase(line.begin() + int(hashPos), line.end());
		Core::trim(line);
		header = line;
		break;
	}

	size_t numberOfYSamples = 0, numberOfXSamples = 0;
	ifs >> numberOfYSamples;
	getline(ifs, line);

	if ( numberOfYSamples == 0 || numberOfYSamples > 10000 ) {
		SEISCOMP_ERROR("%s: invalid number of depth samples: %d",
		               filename.c_str(), int(numberOfYSamples));
		return false;
	}

	y.resize(size_t(numberOfYSamples));
	for ( size_t i = 0; i < y.size(); ++i )
		ifs >> y[i];

	ifs >> numberOfXSamples;
	getline(ifs, line);

	if ( numberOfXSamples == 0 || numberOfXSamples > 10000 ) {
		SEISCOMP_ERROR("%s: invalid number of distance samples: %d",
		               filename.c_str(), int(numberOfXSamples));
		return false;
	}

	x.resize(size_t(numberOfXSamples));
	for ( size_t i = 0; i < x.size(); ++i )
		ifs >> x[i];

	values.resize(size_t(numberOfYSamples));

	for ( size_t i = 0; i < size_t(numberOfYSamples); ++i ) {
		if ( !ifs.good() ) break;

		while ( ifs.get() != '#' );
		while ( ifs.get() != '\n' );

		values[i].resize(size_t(numberOfXSamples));

		for ( size_t j = 0; j < size_t(numberOfXSamples); ++j )
			ifs >> values[i][j];
	}

	bool inUndefinedRange = false;
	for ( size_t i = 1; i < numberOfXSamples; ++i ) {
	    if ( isValidValue(values[0][i-1])
	     && !isValidValue(values[0][i]) ) {
			lowerUndefinedX = x[i-1];
			inUndefinedRange = true;
		}
		else if ( inUndefinedRange && isValidValue(values[0][i]) ) {
			upperUndefinedX = x[i];
			break;
		}
	}

	return ifs.good();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TabValues::interpolate(double &interpolatedValue,
                            bool extrapolation, bool inHole, bool yDerivs,
                            double xPos, double yPos,
                            double *x1stDeriv, double *y1stDeriv,
                            double *x2ndDeriv, double *y2ndDeriv,
                            int *interp_err) const {
	int	i, j, k, kk, m, n;
	int	ileft, jleft, nx_req, nz_req;
	int	ilow, ihigh, itop, ibottom;
	int	num_extrap = 0, num_samp = 0;
	int	idist = 0, idepth = 0;
	double mini_x[MAX_DIST_SAMPLES], mini_z[MAX_DEPTH_SAMPLES];
	double **mini_table, **mini_table_trans;
	double **deriv_2nd, **deriv_2nd_trans;
	double dy, xshift, zshift;

	int nx = int(this->x.size());
	int nz = int(this->y.size());

	const double *x = &this->x[0];
	const double *z = &this->y[0];
	const Util::TabValues::ValueList *xz = &values[0];

	// Requested # of samples in x-direction
	nx_req = min(MAX_DIST_SAMPLES, nx);
	// Requested # of samples in z-direction
	nz_req = min(MAX_DEPTH_SAMPLES, nz);

	interpolatedValue = INVALID_VALUE;
	*x1stDeriv = INVALID_VALUE;
	*y1stDeriv = INVALID_VALUE;
	*x2ndDeriv = INVALID_VALUE;
	*y2ndDeriv = INVALID_VALUE;

	if ( nz == 1 ) { // Special case: Only 1 depth sample available
		itop    = 0;
		ibottom	= 0;
	}
	else {
		// Bracket depth sample
		jleft = bracket(z, nz, yPos);

		if ( jleft < 0 ) { // depth < min. table depth
			if ( yPos == z[0] ) // Check if exactly equal
				jleft = 0;
			else
				--idepth;
			itop    = 0;
			ibottom	= nz_req-1;
		}
		else if ( jleft >= nz-1 ) { // depth > max. table depth
			++idepth;
			itop    = nz-nz_req;
			ibottom	= nz-1;
		}
		else { // requested depth within valid range
			ibottom = min(jleft + (nz_req/2), nz-1);
			itop    = max(ibottom-nz_req+1, 0);
			nz_req  = ibottom - itop + 1;
		}
	}

	// Bracket distance sample
	ileft = bracket(x, nx, xPos);

	if ( ileft < 0 ) { // dist < minimum table dist
		if ( xPos == x[0] ) /* Check if exactly equal */
			ileft = 0;
		else
			--idist;
		ilow  = 0;
		ihigh = nx_req-1;
	}
	else if ( ileft >= nx-1 ) { /* dist > maximum table dist */
		idist++;
		ilow  = nx-nx_req;
		ihigh = nx-1;
	}
	// Distance is within a valid table region, but may not have a
	// valid value.  Interogate table in order to obtain as many
	// valid values as possible for either direct interpolation or
	// eventual extrapolation.  This is determined by the ilow and
	// ihigh settings.
	else {
		// Make sure that high and low end requested does not run us
		// off one side of the distance curve or the other. We need
		// to do this even before we check the actual values contained
		// in the 2-D (x-z) array.

		ihigh = min(ileft + (nx_req/2), nx-1);
		if ( ihigh == nx-1 )
			ilow = nx-nx_req;
		ilow = max(ihigh-nx_req+1, 0);
		if ( ilow == 0 )
			ihigh = nx_req-1;
	}

	if ( (idist != 0 || idepth != 0 || inHole) && !extrapolation )
		goto done;

	// If requested distance sample is within table bounds, then we
	// need to find as many valid samples as possible.  If none exists
	// shift ilow and ihigh closest to a valid curve.  On the other
	// hand, if the requested distance sample is located clearly
	// outside the valid table region, create an artificial mini-table
	// surrounding the requested sample distance value.

	if ( inHole ) {
		if ( xz[itop][ihigh] == INVALID_VALUE ) {
			for ( i = 0; i < (nx_req-1)/2; ++i ) {
				--ihigh;
				if ( xz[itop][ihigh] != INVALID_VALUE )
					break;
			}

			ilow = ihigh - nx_req + 1;
		}
		else {
			ihigh = 109;
			ilow  = ihigh - nx_req + 1;
		}
	}
	else if ( idist == 0 ) {
		if ( xz[itop][0] != INVALID_VALUE && xz[itop][ihigh] == INVALID_VALUE ) {
			idist = 1;
			for ( i = 0; i < (nx_req-1)/2; ++i ) {
				--ihigh;
				if ( xz[itop][ihigh] != INVALID_VALUE ) {
					idist = 0;
					break;
				}
			}

			ilow = ihigh - nx_req + 1;
		}
		else if ( xz[itop][ilow] == INVALID_VALUE ) {
			idist = -1;
			for ( i = 0; i < (nx_req-1)/2; ++i ) {
				++ilow;
				if ( xz[itop][ilow] != INVALID_VALUE ) {
					idist = 0;
					break;
				}
			}

			ihigh = ilow + nx_req - 1;
		}
	}

	// Up to now we have only inspected the 1st depth component on the
	// distance vector.  Now we will build a complete mini-table which
	// will be used for actual inter/exptrapolation using rational
	// function and bi-cubic spline interpolation routines.

	mini_table = matrix<double>(0, nz_req-1, 0, nx_req-1);
	deriv_2nd  = matrix<double>(0, nz_req-1, 0, nx_req-1);

	for ( i = 0; i < nx_req; ++i )
		mini_x[i] = x[ilow+i];
	for ( i = 0; i < nz_req; ++i )
		mini_z[i] = z[itop+i];

	// First, construct mini-table assuming no depth extrapolation is
	// needed.  All distance extrapolation will be handled in this master
	// "for loop".

	for ( k = 0, kk = itop; k < nz_req; ++k, ++kk) {
		// First fill mini_table assuming all xz[][] values are valid

		for ( i = 0; i < nx_req; i++ )
			mini_table[k][i] = xz[kk][ilow+i];

		// Are we off the high end of the curve OR in a hole

		if ( inHole || idist > 0 ) {
			if ( idist > 0 && xPos > x[ihigh] ) {
				xshift = xPos - x[ihigh-((nx_req-1)/2)];
				for ( j = 0; j < nx_req; ++j ) {
					if ( k < 1 )
						mini_x[j] = mini_x[j] + xshift;
					mini_table[k][j] = INVALID_VALUE;
				}
				i = ilow;
			}
			else {
				for ( i = ihigh; i >= 0; --i ) {
					if ( xz[kk][i] != INVALID_VALUE )
						break;
				}
				i = i - nx_req + 1;
			}

			for ( j = 0; j < nx_req; ++j ) {
				if ( mini_table[k][j] == INVALID_VALUE ) {
					if ( (ratint(&x[i], &xz[kk][i], nx_req, mini_x[j],
					             &mini_table[k][j], &dy)) ) {
						free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
						free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
						return 1;
					}
				}
			}
		}

		// Are we off the low end of the curve
		else if ( idist < 0 ) {
			if ( xPos < x[ilow] ) {
				xshift = xPos - x[ilow+((nx_req-1)/2)];

				for ( j = 0; j < nx_req; ++j ) {
					if ( k < 1 )
						mini_x[j] = mini_x[j] + xshift;
					mini_table[k][j] = INVALID_VALUE;
				}

				i = ilow;
			}
			else {
				for ( i = ilow; i < nx; ++i ) {
					if ( xz[kk][i] != INVALID_VALUE )
						break;
				}
			}

			for ( j = 0; j < nx_req; ++j ) {
				if ( mini_table[k][j] == INVALID_VALUE ) {
					if ( (ratint(&x[i], &xz[kk][i], nx_req, mini_x[j],
					             &mini_table[k][j], &dy)) ) {
						free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
						free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
						return 1;
					}
				}
			}
		}

		// Make sure there are no single BAD_SAMPLE entries.  If so, extrapolate
		// as necessary.

		else {
			for ( j = 0; j < nx_req; ++j ) {
				if ( mini_table[k][j] == INVALID_VALUE ) {
					if ( j > 0 ) {
						// Go back and get as many valid samples for this
						// depth as is possible for a good sample space.
						num_extrap = nx_req - j;
						i = ilow - num_extrap;
						num_samp = nx_req;

						while ( i < 0 || xz[kk][i] == INVALID_VALUE ) {
							++i;
							--num_samp;

							if ( num_samp < MIN_NUM_DIST_SAMPLES ) {
								free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
								free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
								return -2; // Insufficient samples
							}
						}

						for ( n = 0; n < num_extrap; ++n ) {
							m = j + n;
							if ( mini_table[k][m] == INVALID_VALUE ) {
								if ( (ratint(&x[i], &xz[kk][i], num_samp,
								             mini_x[m], &mini_table[k][m],
								             &dy)) ) {
									free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
									free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
									return -2;
								}
							}
						}
					}
					else {
						// Advance in distance and get a many valid samples for
						// this depth as is possible for a good sample space.

						for ( n = 0, i = ilow; i < ihigh; ++i, ++n ) {
							if ( xz[kk][i] != INVALID_VALUE ) {
								ilow = i;
								num_extrap = n;
								for ( num_samp = 0, n = 0; n < nx_req; ++n ) {
									if (xz[kk][ilow+n] != INVALID_VALUE)
										++num_samp;
								}

								if ( num_samp < MIN_NUM_DIST_SAMPLES ) {
									free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
									free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
									return -2; // Insufficient samples
								}

								break;
							}
						}

						if ( i == ihigh ) {
							free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
							free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
							return -2; // Must have at least 1 sample
						}

						for ( n = 0; n < num_extrap; ++n ) {
							if ( mini_table[k][n] == INVALID_VALUE ) {
								if ( (ratint(&x[ilow], &xz[kk][ilow], num_samp,
								             mini_x[n], &mini_table[k][n],
								             &dy)) ) {
									free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
									free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
									return -2;
								}
							}
						}
					}

					break;
				}
			}
		}
	}

	// If only one depth component exists wrap it up here.

	if ( nz == 1 ) {
		// Perform a one-dimensional cubic spline interpolation on a
		// single row of mini_table[][] to get the desired value for
		// special case of only one depth available in the table.

		spline(mini_x, mini_table[0], nx_req, 1.0e30, 1.0e30,
		        deriv_2nd[0]);
		splint_deriv(mini_x, mini_table[0], deriv_2nd[0], nx_req,
		             xPos, &interpolatedValue, x1stDeriv, x2ndDeriv);

		free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
		free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);

		goto done;
	}

	// Now that the distance component of the mini-table is secure,
	// perform any necessary extrapolation for the depth component by
	// re-constructing the mini-table.  Also, build transposed mini-
	// table, mini_table_trans[][], to obtain distance derivatives
	// from spline routines below.

	mini_table_trans = matrix<double>(0, nx_req-1, 0, nz_req-1);
	deriv_2nd_trans  = matrix<double>(0, nx_req-1, 0, nz_req-1);

	for ( j = 0; j < nx_req; ++j ) {
		// Fill mini_table_trans[][] assuming all values from array,
		// mini_table[][], are valid

		for ( i = 0; i < nz_req; ++i )
			mini_table_trans[j][i] = mini_table[i][j];

		// Are we below the lowest depth component in the curve

		if ( idepth > 0 ) {
			zshift = yPos - z[ibottom-((nz_req-1)/2)];
			if ( j < 1 ) {
				for ( i = 0; i < nz_req; ++i )
					mini_z[i] = mini_z[i] + zshift;
			}

			for ( i = 0; i < nz_req; ++i ) {
				if ( (ratint(&z[itop], &mini_table_trans[j][0], nz_req,
				             mini_z[i], &mini_table[i][j], &dy)) ) {
					free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
					free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
					free_matrix(mini_table_trans, 0, nx_req-1, 0, nz_req-1);
					free_matrix(deriv_2nd_trans, 0, nx_req-1, 0, nz_req-1);
					return 1;
				}
			}

			for ( i = 0; i < nz_req; ++i )
				mini_table_trans[j][i] = mini_table[i][j];
		}
		else if ( idepth < 0 ) {
			zshift = yPos - z[itop+((nz_req-1)/2)];
			if ( j < 1 ) {
				for ( i = 0; i < nz_req; ++i )
					mini_z[i] = mini_z[i] + zshift;
			}

			for ( i = 0; i < nz_req; ++i ) {
				if ( (ratint(&z[itop], &mini_table_trans[j][0], nz_req,
				             mini_z[i], &mini_table[i][j], &dy)) ) {
					free_matrix (mini_table, 0, nz_req-1, 0, nx_req-1);
					free_matrix (deriv_2nd, 0, nz_req-1, 0, nx_req-1);
					free_matrix (mini_table_trans, 0, nx_req-1, 0, nz_req-1);
					free_matrix (deriv_2nd_trans, 0, nx_req-1, 0, nz_req-1);
					return 1;
				}
			}

			for ( i = 0; i < nz_req; ++i )
				mini_table_trans[j][i] = mini_table[i][j];
		}
	}

	// Now we have both mini-tables and can perform 2-D bi-cubic
	// spline interpolations on our mini-tables to obtain our value
	// of interest and 1st and 2nd derivatives in both the distance
	// and depth directions.  Note that the bi-cubic splines routines
	// need to be called twice in order to obtain the derivatives in
	// first the distance direction and then the depth direction.
	// If do_we_need_z_derivs is set FALSE, then depth derivatives
	// do not need to be computed.

	splie2<double>(mini_x, mini_z, mini_table_trans, nx_req, nz_req,
	               deriv_2nd_trans);
	splin2<double>(mini_x, mini_z, mini_table_trans, deriv_2nd_trans,
	               nx_req, nz_req, xPos, yPos, &interpolatedValue, x1stDeriv, x2ndDeriv);

	if ( yDerivs ) {
		splie2<double>(mini_z, mini_x, mini_table, nz_req, nx_req, deriv_2nd);
		splin2<double>(mini_z, mini_x, mini_table, deriv_2nd, nz_req, nx_req,
		               yPos, xPos, &interpolatedValue, y1stDeriv, y2ndDeriv);
	}

	free_matrix(mini_table, 0, nz_req-1, 0, nx_req-1);
	free_matrix(deriv_2nd, 0, nz_req-1, 0, nx_req-1);
	free_matrix(mini_table_trans, 0, nx_req-1, 0, nz_req-1);
	free_matrix(deriv_2nd_trans, 0, nx_req-1, 0, nz_req-1);

done:
	// All done here! Set interpolation error flags, as necessary.
	if ( interp_err ) {
		if ( inHole )
			*interp_err = 11;
		else if ( idist < 0 && idepth == 0 )
			*interp_err = 12;
		else if ( idist > 0 && idepth == 0 )
			*interp_err = 13;
		else if ( idist == 0 && idepth < 0 )
			*interp_err = 14;
		else if ( idist == 0 && idepth > 0 )
			*interp_err = 15;
		else if ( idist < 0 && idepth < 0 )
			*interp_err = 16;
		else if ( idist > 0 && idepth < 0 )
			*interp_err = 17;
		else if ( idist < 0 && idepth > 0 )
			*interp_err = 18;
		else if ( idist > 0 && idepth > 0 )
			*interp_err = 19;
		else
			*interp_err = 0;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TabValues::interpolate(double &interpolatedValue,
                            bool extrapolation, bool returnYDerivs,
                            double xPos, double yPos,
                            double *x1stDeriv, double *y1stDeriv,
                            double *x2ndDeriv, double *y2ndDeriv,
                            int *error) const {
	bool inHole = false;

	if ( lowerUndefinedX && upperUndefinedX
	  && xPos > *lowerUndefinedX && xPos < *upperUndefinedX ) {
		inHole = true;
	}

	return interpolate(interpolatedValue, extrapolation, inHole, returnYDerivs,
	                   xPos, yPos, x1stDeriv, y1stDeriv,
	                   x2ndDeriv, y2ndDeriv, error);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
