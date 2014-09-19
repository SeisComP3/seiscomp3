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


#ifndef __SEISCOMP_MATH_VECTOR3_H__
#define __SEISCOMP_MATH_VECTOR3_H__


#include <seiscomp3/math/math.h>


namespace Seiscomp {
namespace Math {


template <typename T>
struct Vector3 {
	Vector3() {}
	Vector3(const Vector3<T> &other) : x(other.x), y(other.y), z(other.z) {}
	Vector3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

	T length() const;
	T dot(const Vector3<T> &v) const;

	Vector3<T> &cross(const Vector3<T> &a, const Vector3<T> &b);
	Vector3<T> &normalize();

	Vector3<T> &operator=(const Vector3<T> &other);
	Vector3<T> operator*(T scale) const;
	Vector3<T> &operator*=(T scale);
	T operator*(const Vector3<T> &other) const;

	operator T *() { return (T*)this; }
	operator const T *() const { return (const T*)this; }

	Vector3<T> &operator+=(const Vector3<T> &other);
	Vector3<T> &operator-=(const Vector3<T> &other);

	Vector3<T> operator+(const Vector3<T> &other) const;
	Vector3<T> operator-(const Vector3<T> &other) const;

	Vector3<T> &fromAngles(T radAzimuth, T radDip);
	Vector3<T> &toAngles(T &radAzimuth, T &radDip);

	T  x,y,z;
};


typedef Vector3<float>  Vector3f;
typedef Vector3<double> Vector3d;


#include <seiscomp3/math/vector3.ipp>


} // namespace Math
} // namespace Seiscomp

#endif
