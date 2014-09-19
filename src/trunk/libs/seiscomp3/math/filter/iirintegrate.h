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


#ifndef __SEISCOMP_IIRINTEGRATE_H__
#define __SEISCOMP_IIRINTEGRATE_H__

#include<vector>
#include <seiscomp3/math/filter.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template <typename T>
class IIRIntegrate : public InPlaceFilter<T> {
	public:
		IIRIntegrate(double a = 0, double fsamp = 0);
		IIRIntegrate(const IIRIntegrate<T> &other);


	public:
		void reset();


	// InPlaceFilter interface
	public:
		void setSamplingFrequency(double fsamp);
		int setParameters(int n, const double *params);

		void apply(int n, T *inout);

		InPlaceFilter<T>* clone() const;

	private:
		void init(double a);

	private:
		double _ia0, _ia1, _ia2;

		double _a0, _a1, _a2;
        double _b0, _b1, _b2;

        T _v1, _v2;
};


}
}
}

#endif
