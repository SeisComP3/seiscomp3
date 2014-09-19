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


template <class TYPE> 
int
minmax(int n, const TYPE *f, int i1, int i2, TYPE *fmin, TYPE *fmax)
{
	int i;
/*
	if(i2<i1) {
		i=i1; i1=i2; i2=i;
	}
*/
	if(i1<0) i1=0;
	if(i2>n) i2=n;

	*fmin = *fmax = f[i1];
	f += i1+1;
	for (i=i1+1; i<i2; i++, f++) {
		if(*f > *fmax) *fmax = *f; 
		if(*f < *fmin) *fmin = *f; 
        }

	return 0;
}

template <class TYPE> 
int
find_max(int n, const TYPE *f, int i1, int i2, int *imax, TYPE *fmax)
{
	int i;

	if(i1<0) i1=0;
	if(i2>n) i2=n;

	*fmax = f[i1];
	*imax =   i1;
	f += i1+1;
	for (i=i1+1; i<i2; i++, f++) {
		if(*f > *fmax) {
			*imax =  i;
			*fmax = *f;
		}
        }

	return 0;
}

template <typename TYPE>
int
find_absmax(int n, const TYPE *f, int i1, int i2, TYPE offset=0)
{
	int i;

	if(i1<0) i1=0;
	if(i2>n) i2=n;

	double fmax = fabs(f[i1]-offset);
	int    imax = i1;

	for (i=i1+1; i<i2; i++) {
		double ff = fabs(f[i]-offset);
		if(ff > fmax) {
			imax = i;
			fmax = ff;
		}
	}

	return imax;
}

template <class TYPE>
void
find_minmax(int &lmin, int &lmax, int n, const TYPE *f, int i1, int i2, TYPE offset=0)
{
	int i;

	if(i1<0) i1=0;
	if(i2>n) i2=n;

	TYPE fmax = (f[i1]-offset);
	TYPE fmin = fmax;

	lmin = i1;
	lmax = i1;

	for (i=i1+1; i<i2; i++) {
		TYPE ff = f[i]-offset;
		if (ff > fmax) {
			lmax = i;
			fmax = ff;
		}
		else if (ff < fmin) {
			lmin = i;
			fmin = ff;
		}
	}
}

template <class TYPE>
int
minmax(std::vector<TYPE> const &f, int i1, int i2, TYPE *fmin, TYPE *fmax)
{
	const TYPE *f0 = &f[0];
	int n = f.size();
	return minmax(n, f0, i1, i2, fmin, fmax);
}

template <class TYPE> 
int
find_max(std::vector<TYPE> const &f, int i1, int i2, int *imax, TYPE *fmax)
{
	const TYPE *f0 = &f[0];
	int n = f.size();
	return find_max(n, f0, i1, i2, imax, fmax);
}
