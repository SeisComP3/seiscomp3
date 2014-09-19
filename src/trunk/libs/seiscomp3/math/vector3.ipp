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

template <typename T>
inline T Vector3<T>::length() const {
	return sqrt(x*x + y*y + z*z);
}


template <typename T>
inline T Vector3<T>::dot(const Vector3<T> &v) const {
	return x*v.x + y*v.y + z*v.z;
}


template <typename T>
inline Vector3<T> &Vector3<T>::operator=(const Vector3<T> &other) {
	x = other.x;
	y = other.y;
	z = other.z;

	return *this;
}
