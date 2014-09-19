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
#include<seiscomp3/math/filter/taper.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {

template<typename TYPE>
InitialTaper<TYPE>::InitialTaper(double taperLength, TYPE offset, double fsamp)
	: _taperLength(taperLength), _samplingFrequency(0),
	  _taperLengthI(0), _sampleCount(0), _offset(offset)
{
	if ( fsamp )
		setSamplingFrequency(fsamp);
}

template<typename TYPE>
int InitialTaper<TYPE>::setParameters(int n, const double *params)
{
	if ( n < 1 || n > 2 ) return 1;

	_taperLength = (int)params[0];

	if ( n > 1 )
		_offset = (TYPE)params[1];
	else
		_offset = 0;

	return n;
}

template<typename TYPE>
InPlaceFilter<TYPE>* InitialTaper<TYPE>::clone() const
{
	return new InitialTaper<TYPE>(_taperLength, _offset, _samplingFrequency);
}

template<typename TYPE>
void InitialTaper<TYPE>::apply(int n, TYPE *inout)
{
	if (_sampleCount >= _taperLengthI) return;

	for (int i=0; i<n && _sampleCount<_taperLengthI; i++) {
		double frac = double(_sampleCount++)/_taperLengthI;
		inout[i] = (TYPE)((inout[i]-_offset)*0.5*(1-cos(M_PI*frac)) + _offset);
	}
}

INSTANTIATE_INPLACE_FILTER(InitialTaper, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(InitialTaper, "ITAPER");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
