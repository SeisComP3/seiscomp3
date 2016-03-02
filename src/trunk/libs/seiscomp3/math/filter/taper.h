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


#ifndef _SEISCOMP_FILTERING_TAPER_H_
#define _SEISCOMP_FILTERING_TAPER_H_

#include<vector>

#include<seiscomp3/math/filter.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {

template<typename TYPE>
class InitialTaper : public InPlaceFilter<TYPE> {
	public:

		InitialTaper(double taperLength=0, TYPE offset=0, double fsamp=0);
	//	InitialTaper(InitialTaper const &other);
		~InitialTaper() {}

		void setLength(double taperLength, TYPE offset=0) {
			_taperLength = taperLength;
			_offset = offset;
		}

		// apply filter to data vector **in*place**
		void apply(int n, TYPE *inout);

		virtual InPlaceFilter<TYPE>* clone() const;

		// resets the filter, i.e. erases the filter memory
		void reset() { _sampleCount = 0; }

		virtual void setSamplingFrequency(double fsamp) {
			_samplingFrequency = fsamp;
			_taperLengthI = int(_taperLength * _samplingFrequency);
		}

		virtual int setParameters(int n, const double *params);

	private:
		double _taperLength,  _samplingFrequency;
		int    _taperLengthI, _sampleCount;
		TYPE   _offset;
}; // class InitialTaper


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
