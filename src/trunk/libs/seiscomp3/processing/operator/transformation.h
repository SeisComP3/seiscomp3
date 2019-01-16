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


#ifndef __SEISCOMP_PROCESSING_OPERATOR_TRANSFORMATION_H__
#define __SEISCOMP_PROCESSING_OPERATOR_TRANSFORMATION_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/math/matrix3.h>


namespace Seiscomp {
namespace Processing {
namespace Operator {


template <typename T, int N>
struct Transformation {
	Transformation(const Math::Matrix3<T> &m);

	// Process N traces in place of length n
	void operator()(const Record *, T *data[N], int n, const Core::Time &, double) const;

	// publishs a processed component
	bool publish(int c) const;
};


template <typename T>
struct Transformation<T,2> {
	Transformation(const Math::Matrix3<T> &m) : matrix(m) {}

	bool publish(int c) const { return true; }

	void operator()(const Record *, T *data[2], int n, const Core::Time &, double) const {
		for ( int i = 0; i < n; ++i ) {
			Math::Vector3<T> v = matrix*Math::Vector3<T>(*data[0], *data[1], 0);
			*data[0] = v.x;
			*data[1] = v.y;
			++data[0]; ++data[1];
		}
	}

	Math::Matrix3<T> matrix;
};


template <typename T>
struct Transformation<T,3> {
	Transformation(const Math::Matrix3<T> &m) : matrix(m) {}

	bool publish(int c) const { return true; }

	void operator()(const Record *, T *data[3], int n, const Core::Time &, double) const {
		for ( int i = 0; i < n; ++i ) {
			Math::Vector3<T> v = matrix*Math::Vector3<T>(*data[0], *data[1], *data[2]);
			*data[0] = v.x;
			*data[1] = v.y;
			*data[2] = v.z;
			++data[0]; ++data[1]; ++data[2];
		}
	}

	Math::Matrix3<T> matrix;
};


}
}
}


#endif
