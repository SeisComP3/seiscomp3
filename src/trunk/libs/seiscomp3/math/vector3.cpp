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



#include <seiscomp3/math/vector3.h>


namespace Seiscomp {
namespace Math {


template <typename T>
Vector3<T> &Vector3<T>::cross(const Vector3<T> &a, const Vector3<T> &b) {
	x = a.y*b.z - a.z*b.y;
	y = a.z*b.x - a.x*b.z;
	z = a.x*b.y - a.y*b.x;
	return *this;
}


template <typename T>
Vector3<T> &Vector3<T>::normalize() {
	T len = 1.0 / length();
	x *= len;
	y *= len;
	z *= len;
	return *this;
}


template <typename T>
Vector3<T> Vector3<T>::operator*(T scale) const {
	Vector3<T> v(*this);
	v.x *= scale;
	v.y *= scale;
	v.z *= scale;
	return v;
}


template <typename T>
Vector3<T> &Vector3<T>::operator*=(T scale) {
	x *= scale;
	y *= scale;
	z *= scale;
	return *this;
}


template <typename T>
T Vector3<T>::operator*(const Vector3<T> &other) const {
	return dot(other);
}


template <typename T>
Vector3<T> &Vector3<T>::operator+=(const Vector3<T> &other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}


template <typename T>
Vector3<T> &Vector3<T>::operator-=(const Vector3<T> &other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

template <typename T>
Vector3<T> Vector3<T>::operator+(const Vector3<T> &other) const {
	return Vector3<T>(x + other.x, y + other.y, z + other.z);
}


template <typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T> &other) const {
	return Vector3<T>(x - other.x, y - other.y, z - other.z);
}


template <typename T>
Vector3<T> &Vector3<T>::fromAngles(T radAzimuth, T radDip) {
	x = cos(radDip)*sin(radAzimuth);
	y = cos(radDip)*cos(radAzimuth);
	z = sin(radDip);
	return *this;
}


template <typename T>
Vector3<T> &Vector3<T>::toAngles(T &radAzimuth, T &radDip) {
	radDip = acos(z);
	radAzimuth = atan2(y,x);
	return *this;
}


template class SC_SYSTEM_CORE_API Vector3<float>;
template class SC_SYSTEM_CORE_API Vector3<double>;


}
}
