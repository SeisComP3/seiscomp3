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

#include<math.h>
#include<vector>

template<typename TYPE>
int taper(std::vector<TYPE> f,	/* I    number of samples in trace  */
           double frac1,	/* I    taper widths as fraction    */
	   double frac2)	/* frac1+frac2 must be <= 1	    */
{
	int nsamp = f.size();
	TYPE  *fp = &f[0];

        if(frac1==frac2) {
                int n = (int)(nsamp*frac1);
                for(int i=0, inx=M_PI/n; i<n; i++) {
                        double ff = 0.5 - 0.5*cos(i*inx);
                        f[i] = f[nsamp-i-1] = ff;
                }
        }
        else {
                int n1 = (int)(nsamp*frac1), n2 = (int)(nsamp*frac2);
                for(int i=0, inx=M_PI/n1; i<n1; i++)
                        f[i] = 0.5-0.5*cos(i*inx);
                for(int i=0, inx=M_PI/n2; i<n2; i++)
                        f[nsamp-i-1] = 0.5-0.5*cos(i*inx);
        }

	return 0;
}

/*
template<typename TYPE>
int apply_taper(std::vector<TYPE> f, std::vector<double> t)
{
	// if (f.size() != t.size())
	// 	// throw
	int  nsamp = f.size();
	TYPE   *fp = &f[0];
	double *tp = &t[0];
	
	while(n--)
		fp[n] *= tp[n];
}

*/

