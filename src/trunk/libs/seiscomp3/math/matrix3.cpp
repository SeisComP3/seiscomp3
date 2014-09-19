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



#include <seiscomp3/math/matrix3.h>


namespace Seiscomp {
namespace Math {


template <typename T>
Matrix3<T> &Matrix3<T>::identity() {
	_mat3_elem(0,0) = 1;
	_mat3_elem(1,1) = 1;
	_mat3_elem(2,2) = 1;

	_mat3_elem(0,1) = 0;
	_mat3_elem(0,2) = 0;

	_mat3_elem(1,0) = 0;
	_mat3_elem(1,2) = 0;

	_mat3_elem(2,0) = 0;
	_mat3_elem(2,1) = 0;

	return *this;
}


template <typename T>
Matrix3<T> &Matrix3<T>::setRow(int row, const Vector3<T> &v) {
	_mat3_elem(row,0) = v.x;
	_mat3_elem(row,1) = v.y;
	_mat3_elem(row,2) = v.z;

	return *this;
}


template <typename T>
Matrix3<T> &Matrix3<T>::setColumn(int col, const Vector3<T> &v) {
	_mat3_elem(0,col) = v.x;
	_mat3_elem(1,col) = v.y;
	_mat3_elem(2,col) = v.z;

	return *this;
}


template <typename T>
Vector3<T> Matrix3<T>::row(int r) const {
	return Vector3<T>(_mat3_elem(r,0), _mat3_elem(r,1), _mat3_elem(r,2));
}


template <typename T>
Vector3<T> Matrix3<T>::column(int c) const {
	return Vector3<T>(_mat3_elem(0,c), _mat3_elem(1,c), _mat3_elem(2,c));
}


template <typename T>
Matrix3<T> &Matrix3<T>::loadRotateX(T theta) {
	T sintheta = sin(theta);
	T costheta = cos(theta);

	_mat3_elem(0,0) = 1.0f;
	_mat3_elem(0,1) = 0.0f;
	_mat3_elem(0,2) = 0.0f;
	_mat3_elem(1,0) = 0.0f;
	_mat3_elem(2,0) = 0.0f;

	_mat3_elem(1,1) = costheta;
	_mat3_elem(1,2) = -sintheta;
	_mat3_elem(2,1) = sintheta;
	_mat3_elem(2,2) = costheta;

	return *this;
}


template <typename T>
Matrix3<T> &Matrix3<T>::loadRotateY(T theta) {
	T sintheta = sin(theta);
	T costheta = cos(theta);

	_mat3_elem(1,1) = 1.0f;
	_mat3_elem(0,1) = 0.0f;
	_mat3_elem(2,1) = 0.0f;
	_mat3_elem(1,0) = 0.0f;
	_mat3_elem(1,2) = 0.0f;

	_mat3_elem(0,0) = costheta;
	_mat3_elem(0,2) = sintheta;
	_mat3_elem(2,0) = -sintheta;
	_mat3_elem(2,2) = costheta;

	return *this;
}


template <typename T>
Matrix3<T> &Matrix3<T>::loadRotateZ(T theta) {
	T sintheta = sin(theta);
	T costheta = cos(theta);

	_mat3_elem(2,2) = 1.0f;
	_mat3_elem(0,2) = 0.0f;
	_mat3_elem(1,2) = 0.0f;
	_mat3_elem(2,0) = 0.0f;
	_mat3_elem(2,1) = 0.0f;

	_mat3_elem(0,0) = costheta;
	_mat3_elem(0,1) = -sintheta;
	_mat3_elem(1,0) = sintheta;
	_mat3_elem(1,1) = costheta;

	return *this;
}


template <typename T>
Matrix3<T> &Matrix3<T>::mult(const Matrix3<T> &m1, const Matrix3<T> &m2) {
	for ( int r = 0; r < 3; ++r )
		for ( int col = 0; col < 3; ++col )
			_mat3_elem(r,col) = m1._mat3_elem(r,0)*m2._mat3_elem(0,col) +
			             m1._mat3_elem(r,1)*m2._mat3_elem(1,col) +
			             m1._mat3_elem(r,2)*m2._mat3_elem(2,col);
	return *this;
}


template class SC_SYSTEM_CORE_API Matrix3<float>;
template class SC_SYSTEM_CORE_API Matrix3<double>;



}
}
