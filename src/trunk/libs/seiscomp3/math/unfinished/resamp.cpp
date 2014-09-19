/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


/***************************************************************************
 *   Copyright (C) 2006 by GFZ Potsdam                                     *
 *                                                                         *
 *   author: Joachim Saul                                                  *
 *   email:  saul@gfz-potsdam.de                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include<stdlib.h>
#include<math.h>

template<class TYPE>
static void
spline2 (int n, TYPE *y, double *y2, double yp1, double ypn)
{
        int i,k;
        double p,qn,un,*u;

        u = (double*) malloc(n*sizeof(double));

        if (yp1 > 0.99e30)
                y2[0]=u[0]=0.0;
        else {
                y2[0] = -0.5;
                u[0] =  3.0*((y[1]-y[0])-yp1);
        }
        for (i=1; i<n-1; i++) {
                p   = 0.5*y2[i-1] + 2.0;
                y2[i] = -0.5/p;
                u[i]  = y[i+1] - y[i] - y[i] + y[i-1];
                u[i]  = (3.0*u[i]-0.5*u[i-1])/p;
        }
        if (ypn > 0.99e30)
                qn=un=0.0;
        else {
                qn = 0.5;
                un = 3.0*(ypn-y[n-1]+y[n-2]);
        }
        y2[n-1] = (un-qn*u[n-2])/(qn*y2[n-2]+1.0);
        for (k=n-1; k>0; k--)
                y2[k-1] = y2[k-1]*y2[k] + u[k-1];

	free(u);
}

template<class TYPE>
static TYPE
splint2_TYPE (int n, TYPE *ya, double *y2a, double x)
{
        /* x in SAMPLES (and fractions thereof)!!! */
        int klo,khi;
	double a, b;
        TYPE value;

        if (x<0 || x>n) return 0;

        klo = (int)floor(x);
        khi = klo+1;

        a = khi - x;
        b = x - klo;
        value = (TYPE) (a*ya[klo] + b*ya[khi] +
            ((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*0.166666666666666666667);

        return value;
}

// Spline class to represent a 1-D function sampled at a regular interval.
// The x coordinate is the sample index.
template<class TYPE>
class Spline_1D
{
    public:
	Spline_1D(std::vector<TYPE> const &y,
		  double yp1=1.e30, double ypn=1.e30)
	{
		f = y;
	}

	TYPE interpolate(double x)
	{
		// x in SAMPLES (and fractions thereof)
		int klo,khi;
		double a, b;
		TYPE value;

		if (x<0 || x>f.size()) return 0;

		klo = (int)floor(x);
		khi = klo+1;

		a = khi - x;
		b = x - klo;
		value = (TYPE) (a*ya[klo] + b*ya[khi] +
			((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])
				*0.166666666666666666667);

		return value;
	}
    private:
	std::vector<TYPE> f, fa;
	//
};


template<class TYPE>
int
resamp(int n1, TYPE *f1,	/* input record		*/
       int n2, TYPE *f2,	/* output record	*/
       double q)		/* fs_before/fs_after	*/
{
        int  i;
        double *fa;

        if (fabs(q-1)<0.001) return 1;

        fa = (double*) malloc(n1*sizeof(double));

        spline2(n1, f1, fa, 1.0e30, 1.0e30);

        for (i=0; i<n2; i++)
                f2[i] = splint2(n1, f1, fa, i*q);

	free(fa);
        return 1;
}

