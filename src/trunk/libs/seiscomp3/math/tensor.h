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


#ifndef __SEISCOMP_MATH_TENSOR_H__
#define __SEISCOMP_MATH_TENSOR_H__


#include <seiscomp3/math/matrix3.h>
#include <stdio.h>


namespace Seiscomp {
namespace Math {


template <typename T>
class Tensor2S;


/**
 * Non-Symmetric second rank tensor (gradient & rotation tensors)
 */
template <typename T>
class Tensor2N {
	public:
		Tensor2N() { assign(0.0); }

		//! Equality operator
		void assign(Tensor2N<T> &other);
		//! Initialize
		void assign(T k);

		//! Make unit tensor
		void unit();

		//! Add scaled tensor
		void sum(const Tensor2N<T> &other, T k);

		//! Multiply by a scalar
		void scale(T k);

		//! Determinant of gradient
		T det() const;

		//! Make inverse tensor
		void inverse(const Tensor2N<T>& src);

		void derive(T   *dX1,
		            T   *dX2,
		            T   *dX3,
		            T   *disp,
		            int * inodes,
		            int n);

		//! Spin tensor
		void spin    (Tensor2N<T> &U);

		//! FD = [det(F)^-1/3] * F
		void deviator(Tensor2N<T> &F);

		//! Product of non-symmetric
		//! tensor with symmetric tensor
		void product(const Tensor2N<T> &N,
		             const Tensor2S<T> &S);

		//! Product of two non-symmetric
		//! tensors
		void product(const Tensor2N<T> &A,
		             const Tensor2N<T> &B);

		//! Performs polar decomposition of the deformation gradient
		//! tensor into the right stretch tensor and the rotation tensor
		void polarDecomp(Tensor2N<T> &F);

		//! Hughes-Winget rotation increment
		void HughesWinget(Tensor2N<T> &U);


	public:
		T _11, _12, _13;
		T _21, _22, _23;
		T _31, _32, _33;
};


typedef Tensor2N<float>  Tensor2Nf;
typedef Tensor2N<double> Tensor2Nd;



/**
 * Symmetric second rank tensor (stress & strain tensors)
 */
template <typename T>
class Tensor2S {
	public:
		Tensor2S() { assign(0.0); }
		Tensor2S(T __11, T __12, T __13,
		                 T __22, T __23,
		                         T __33);

		//! Equality operator
		void assign(const Tensor2S<T> &A);
		//! Initialize
		void assign(T k = 0.0);

		//! Make spheric tensor
		void spheric(T k = 1.0);

		//! Add scaled tensor
		void sum(const Tensor2S<T> &A, T k = 1.0);

		//! Multiply by a scalar
		void scale(T k);

		//! Strain tensor
		void strain(const Tensor2N<T> &U);

		//! Strain deviator
		void deviator(const Tensor2N<T> &U);

		//! Green-Lagrange strain tensor
		void GLStrain(const Tensor2N<T> &F);

		//! Left Cauchy-Green deformation tensor
		void leftCG(const Tensor2N<T> &F);

		//! Right Cauchy-Green deformation tensor
		void rightCG(const Tensor2N<T> &F);

		//! Square of symmetric tensor C^2 = C^t*C
		void square(const Tensor2S<T> &C);

		//! Stress Push-Forward (PF)
		void pshFrwd(const Tensor2N<T> &F,
		             const Tensor2S<T> &S);

		//! Spatial to unrotated tensor transf.
		void unrotate(const Matrix3<T> &R);

		//! Spatial to unrotated tensor transf.
		void unrotate(const Matrix3<T> &R,
		              const Tensor2S<T> &S);

		//! Unrotated to spatial transf.
		void rotate(const Matrix3<T> &R);

		//! Unrotated to spatial tensor transf.
		void rotate(const Matrix3<T> &R,
		            const Tensor2S<T> &U);

		//! Trace of tensor
		T I1() const;

		//! Second invariant of tensor
		T I2() const;

		//! Mean stress
		T mean() const;

		//! Euclidean norm of tensor
		T norm() const;

		//! Euclidean norm of deviator
		T devnorm() const;

		//! Make deviator & return trace
		T dtrace();

		//! Make deviator & return mean value
		T dmean();

		//! Augment deviator with spheric tensor
		void augment(T p);

		//! Update deviatoric stress
		void update(Tensor2S<T> &d,
		            T G);

		//! Prints tensor to file
		void print(FILE * fl);

		//! apply Jacoby rotation
		void jacoby(T &pp,
		            T &qq,
		            T &pq,
		            T &rp,
		            T &rq);

		//! compute eigenvalues
		bool eigenval(T tol  = 1e-15,
		              int   itmax = 30);

	public:
		T _11, _12, _13;
		T      _22, _23;
		T           _33;
};


typedef Tensor2S<float>  Tensor2Sf;
typedef Tensor2S<double> Tensor2Sd;


/**
 * Spectral representation of symmetric tensor
 */
template <typename T>
class Spectral2S {
	public:
		Spectral2S() { assign(0.0); }

		//! initialize
		void assign(T k);

		//! spectral decomposition
		bool spect(const Tensor2S<T> &A,
		           T   atol  = 1e-12,
		           int itmax = 50);
		//! sort principal values in decending order & permute principal directions
		void sort();

		//! sort absolute principal values in decending order & permute principal directions
		void absSort();

		//! compose symmetric tensor from spectral representation
		void compose(Tensor2S<T> &A);

		//! compute norm of spectrally decomposed tensor
		T norm();

	public:
		T a1, a2, a3;          // principal values of tensor
		Vector3<T> n1, n2, n3; // principal directions of tensor
};


typedef Spectral2S<float>  Spectral2Sf;
typedef Spectral2S<double> Spectral2Sd;


}
}

#endif
