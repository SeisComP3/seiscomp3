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



#ifndef __MATH_MT_CONVERSIONS_H__
#define __MATH_MT_CONVERSIONS_H__


#include <seiscomp3/math/vector3.h>
#include <seiscomp3/math/tensor.h>

#include <algorithm>


namespace Seiscomp {
namespace Math {


struct NODAL_PLANE {
	double str;
	double dip;
	double rake;
};


struct AXIS {
	double str;
	double dip;
	double val;
	int    expo;
};


/**
 * Calculates the strike and dip of an axis from a cartesian vector
 */
template <typename T>
void vec2angle(const Math::Vector3<T> &v, T &strike, T &dip);


/**
 * Calculates the axis of a cartesian vector from strike and dip.
 */
template <typename T>
void angle2vec(T strike, T dip, Math::Vector3<T> &v);


/**
 * Calculates the fault normal and slip vector from principle axis P and T
 */
template <typename T>
bool pa2nd(const Math::Vector3<T> &t, const Math::Vector3<T> &p, Math::Vector3<T> &n, Math::Vector3<T> &d);


/**
 * Calculates principle axis P and T from fault normal and slip vector
 */
template <typename T>
bool nd2pa(const Math::Vector3<T> &n, const Math::Vector3<T> &d, Math::Vector3<T> &t, Math::Vector3<T> &p);


/**
 * Calculates the nodal plane from the normal and slip vector of the fault plane.
 * The result is in radiants.
 */
template <typename T>
bool nd2np(const Math::Vector3<T> &n, const Math::Vector3<T> &d, NODAL_PLANE &np);


/**
 * Calculates the the normal and slip vector of the fault plane
 * from a nodal plane. The nodal plane attributes are expected to be
 * in radiants.
 */
template <typename T>
bool np2nd(const NODAL_PLANE &np, Math::Vector3<T> &n, Math::Vector3<T> &d);


/**
 * Calculates a tensor from fault normal and slip vector
 */
template <typename T>
bool nd2tensor(const Math::Vector3<T> &n, const Math::Vector3<T> &d, Math::Tensor2S<T> &t);


/**
 * Calculates a tensor from a nodal plane
 */
template <typename T>
bool np2tensor(const NODAL_PLANE &np, Math::Tensor2S<T> &t);



/**
 * Converts a nodal plane from radiants to degree
 */
void np2deg(NODAL_PLANE &np);


/**
 * Computes the double couples from normal and slip vector
 */
template <typename T>
bool nd2dc(const Math::Vector3<T> &n, const Math::Vector3<T> &d, NODAL_PLANE *NP1, NODAL_PLANE *NP2);


template <typename T>
void rtp2tensor(T mrr, T mtt, T mpp, T mrt, T mrp, T mtp, Math::Tensor2S<T> &tensor);


template <typename T>
double spectral2clvd(Math::Tensor2S<T> &tensor);


template <typename T>
void spectral2axis(const Math::Spectral2S<T> &spec,
                   AXIS &t, AXIS &n, AXIS &p, int expo);

template <typename T>
void axis2spectral(const AXIS &t, const AXIS &n, const AXIS &p, int expo,
                   Math::Spectral2S<T> &spec);

template <typename T>
void axis2tensor(const AXIS &t, const AXIS &n, const AXIS &p, int expo,
                 Math::Tensor2S<T> &tensor);

template <typename T1, typename T2>
void spectral2matrix(const Math::Spectral2S<T1> &t, Math::Matrix3<T2> &m);

template <typename T1, typename T2>
bool tensor2matrix(const Math::Tensor2S<T1> &t, Math::Matrix3<T2> &m);


#include "conversions.ipp"


}
}

#endif
