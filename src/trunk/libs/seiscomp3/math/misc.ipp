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

// namespace is Seiscomp::Filter

#include<math.h>

#include<vector>
#include<algorithm>
using namespace std;

template<typename TYPE>
static vector<TYPE> mytransform(vector<TYPE> const &f, double (*func)(double))
{
	vector<TYPE> y(f.size());
	transform( f.begin() , f.end() , y.begin() , func) ;
	return y;
}

template<typename TYPE>
vector<TYPE>   sin(vector<TYPE> const &f) { return mytransform(f, ::sin); }
template<typename TYPE>
vector<TYPE>   cos(vector<TYPE> const &f) { return mytransform(f, ::cos); }
template<typename TYPE>
vector<TYPE>   tan(vector<TYPE> const &f) { return mytransform(f, ::tan); }
template<typename TYPE>
vector<TYPE>  sqrt(vector<TYPE> const &f) { return mytransform(f, ::sqrt); }
template<typename TYPE>
vector<TYPE>   log(vector<TYPE> const &f) { return mytransform(f, ::log); }
template<typename TYPE>
vector<TYPE> log10(vector<TYPE> const &f) { return mytransform(f, ::log10); }
template<typename TYPE>
vector<TYPE>   exp(vector<TYPE> const &f) { return mytransform(f, ::exp); }

template<typename TYPE>
static vector<TYPE> arange(TYPE xmax)
{
	int size = int(xmax);
	size += int(ceil(xmax-size));
	vector<TYPE> y(size);
	TYPE *yy = &y[0];
	for(int i=0; i<size; i++)
		yy[i] = TYPE(i);
	return y;
}

#include<iostream>

int main()
{
	for (int count=0; count<5000; count++)
	{
//		vector<double> x(10000, 0.5), y;
//		y = sin(x);
//		cerr << x[4] << " " << y[4] << endl; break;
		vector<double> y;
		y = arange(10.1);
		for (int i=0; i<y.size(); i++)
		    cerr << i << " " << y[i]/3 << endl;
		break;
	}
}
