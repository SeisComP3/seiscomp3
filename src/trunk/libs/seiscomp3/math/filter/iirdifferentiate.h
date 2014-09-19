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


#ifndef __SEISCOMP_IIRDIFFERENTIATE_H__
#define __SEISCOMP_IIRDIFFERENTIATE_H__

#include<vector>
#include <seiscomp3/math/filter.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template <typename T>
class IIRDifferentiate : public InPlaceFilter<T> {
	public:
		IIRDifferentiate(double fsamp = 0);
		IIRDifferentiate(const IIRDifferentiate<T> &other);


	public:
		void reset();


	// InPlaceFilter interface
	public:
		void setSamplingFrequency(double fsamp);
		int setParameters(int n, const double *params);

		void apply(int n, T *inout);

		InPlaceFilter<T>* clone() const;

	private:
		T _v1;
		T _fsamp;
		bool _init;
};


}
}
}

#endif
