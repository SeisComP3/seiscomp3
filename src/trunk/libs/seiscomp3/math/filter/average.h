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


#ifndef _SEISCOMP_MATH_FILTER_RCA_H_
#define _SEISCOMP_MATH_FILTER_RCA_H_

#include<vector>
#include<seiscomp3/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
class Average : public InPlaceFilter<TYPE> {
	public:
		Average(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);

	public:
		void setLength(double timeSpan);

		virtual void setSamplingFrequency(double fsamp);
		virtual int setParameters(int n, const double *params);

		// apply filter to data vector **in*place**
		virtual void apply(int n, TYPE *inout);
		virtual InPlaceFilter<TYPE>* clone() const;

		// Resets the filter values
		void reset();

	private:
		double _timeSpan;
		double _fsamp;
		double _oocount;
		int _sampleCount;
		int _index;
		double _lastSum;
		bool _firstSample;
		std::vector<TYPE> _buffer;
};


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif

