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


#ifndef __SEISCOMP_MATH_MATRIX3_H__
#define __SEISCOMP_MATH_MATRIX3_H__


#include <seiscomp3/math/vector3.h>
#include <string.h>


namespace Seiscomp {
namespace Math {


template <typename T>
struct Matrix3 {
	Matrix3() {}
	Matrix3(const Matrix3<T> &other) {
		memcpy(d, other.d, sizeof(d));
	}

	Matrix3<T> &identity();

	Matrix3<T> &setRow(int row, const Vector3<T> &v);
	Matrix3<T> &setColumn(int col, const Vector3<T> &v);

	//! Returns a copy of the r-th row
	Vector3<T> row(int r) const;

	//! Returns a copy of the c-th column
	Vector3<T> column(int c) const;

	Matrix3<T> &loadRotateX(T theta);
	Matrix3<T> &loadRotateY(T theta);
	Matrix3<T> &loadRotateZ(T theta);

	Matrix3<T> &mult(const Matrix3<T> &a, const Matrix3<T> &b);

	Vector3<T> &transform(Vector3<T> &dst, const Vector3<T> &v) const;
	Vector3<T> &invTransform(Vector3<T> &dst, const Vector3<T> &v) const;

	operator T *() { return (T*)this; }
	operator const T *() const { return (const T*)this; }

	Vector3<T> operator*(const Vector3<T> &v) const;

	// Coefficients
	union {
		T d[3][3];

		struct {
			T _11, _12, _13;
			T _21, _22, _23;
			T _31, _32, _33;
		} c;
	};
};


typedef Matrix3<float>  Matrix3f;
typedef Matrix3<double> Matrix3d;


#include <seiscomp3/math/matrix3.ipp>


} // namespace Math
} // namespace Seiscomp

#endif
