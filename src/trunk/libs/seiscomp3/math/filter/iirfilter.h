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


#ifndef _SEISCOMP_IIRFILT_H_
#define _SEISCOMP_IIRFILT_H_

#include<vector>
#include <seiscomp3/math/filter.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template <typename T>
class IIRFilter : public InPlaceFilter<T> {
	public:
		IIRFilter();
		IIRFilter(int na, int nb, const double *a, const double *b);
		~IIRFilter();


	public:
		void setCoefficients(int na, int nb, const double *a, const double *b);


	// InPlaceFilter interface
	public:
		void setSamplingFrequency(double fsamp);
		int setParameters(int n, const double *params);

		void apply(int n, T *inout);

		InPlaceFilter<T>* clone() const;


	private:
		int _na, _nb;
		std::vector<double> _a, _b;
		std::vector<double> _lastValues;
};


}
}
}

#endif
