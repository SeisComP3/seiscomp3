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

#define _mat3_elem(row,column) d[row][column]

template <typename T>
inline Vector3<T> &Matrix3<T>::transform(Vector3<T> &dst, const Vector3<T> &v) const {
	for ( int r = 0; r < 3; ++r )
		dst[r] = _mat3_elem(r,0)*v[0] +
		         _mat3_elem(r,1)*v[1] +
		         _mat3_elem(r,2)*v[2];

	return dst;
}


template <typename T>
inline Vector3<T> &Matrix3<T>::invTransform(Vector3<T> &dst, const Vector3<T> &v) const {
	for ( int r = 0; r < 3; ++r )
		dst[r] = _mat3_elem(0,r)*v[0] +
		         _mat3_elem(1,r)*v[1] +
		         _mat3_elem(2,r)*v[2];

	return dst;
}


template <typename T>
inline Vector3<T> Matrix3<T>::operator*(const Vector3<T> &v) const {
	Vector3<T> r;
	transform(r, v);
	return r;
}
