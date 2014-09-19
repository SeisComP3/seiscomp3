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


//#include"biquad.cpp"
//#include"resamp.cpp"

#include<seiscomp3/math/filter/filter.h>

namespace Seiscomp
{

namespace Math
{

namespace Filtering
{
	
template int rotate(std::vector<double> &f1, std::vector<double> &f2,
			double phi);
template int decompose(std::vector<double> &f1, std::vector<double> &f2,
			double p, double vs, double sigma);
//template int decompose(std::vector<double> &f1, std::vector<double> &f2,
//		       double p, double vs, double sigma);

#include<seiscomp3/math/filter/minmax.ipp>

template int minmax(std::vector<int> const &f, int i1, int i2, int *imax, int *fmax);
template int find_max(std::vector<int> const &f, int i1, int i2, int *imax, int *fmax);

//#include "seiscomp3/impl/hilbert.ipp"
//#include "src/filter/hilbert.cpp"
//#include "hilbert.cpp"
//template void hilbert_transform(std::vector<double> &f1, int direction);
//template void envelope(std::vector<double> &f1);

//BiquadCascade<int> intcascade;
//Biquad<int> intbiquad;
//BiquadCascade<double> doublecascade;
//Biquad<double> doublebiquad;

} // namespace Seiscomp::Math::Filter

} // namespace Seiscomp::Math

} // namespace Seiscomp
