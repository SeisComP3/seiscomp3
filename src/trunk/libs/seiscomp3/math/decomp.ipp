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

#include<vector>
#include<math.h>

#define VPVS(sigma)  sqrt((2.-2.*(sigma))/(1.-2.*(sigma)))

namespace Seiscomp
{

namespace Math
{

namespace Filtering
{

template<typename TYPE>
int rotate(std::vector<TYPE> &f1, std::vector<TYPE> &f2, double phi)
{
        double  m11 = cos(phi*M_PI/180.),   m12 = sin(phi*M_PI/180.),
                m21 = -m12,                 m22 = m11,    x, y;

	if (f1.size() != f2.size()) 
		throw AlignmentError("rotate(): vector's not aligned");

	TYPE *p1 = &f1[0], *p2 = &f2[0];
	int n = f1.size();
	
        while (n--) {
                x = (*p1)*m11 + (*p2)*m12;
                y = (*p1)*m21 + (*p2)*m22;
                (*p1++) = (TYPE) x;
                (*p2++) = (TYPE) y;
        }

        return 0;
}

template<typename TYPE>
int decompose(std::vector<TYPE> &f1, std::vector<TYPE> &f2,
	      double p, double vs, double sigma)
{
        double	vp = vs*VPVS(sigma), qa, qb;
        double	m11, m12, m21, m22, x, y;

        if (p >= 1./vp)
                return -1;

	if (f1.size() != f2.size()) 
		throw AlignmentError("decompose(): vector's not aligned");

	TYPE *p1 = &f1[0], *p2 = &f2[0];
	int n = f1.size();

        qa  = sqrt (1./(vp*vp)-p*p);
        qb  = sqrt (1./(vs*vs)-p*p);
        m11 = -(2*vs*vs*p*p-1.)/(vp*qa);
        m12 =   2.*p*vs*vs/vp;
        m21 =  -2.*p*vs;
        m22 =  (1.-2.*vs*vs*p*p)/(vs*qb);

        while (n--) {
		x = (*p1)*m11 + (*p2)*m12;
                y = (*p1)*m21 + (*p2)*m22;
                (*p1++) = (TYPE) x;
                (*p2++) = (TYPE) y;
        }

        return 0;
}

}       // namespace Seiscomp::Math::Filter

}       // namespace Seiscomp::Math

}       // namespace Seiscomp
