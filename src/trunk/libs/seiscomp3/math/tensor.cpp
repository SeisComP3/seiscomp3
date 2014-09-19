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



#include <stdlib.h>
#include <math.h>
#include <string>


#include <seiscomp3/core.h>
#include <seiscomp3/math/tensor.h>


namespace Seiscomp {
namespace Math {


#define ROTATE(a,i,j,k,l) { g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau); \
        a[k][l]=h+s*(g-h*tau); }

/*
Compute p-T axis strikes and dips from seismic moment tensor components.
Input moment tensor components mrr mtt mff mrt mrf mtf.

Angles are in degrees.
value, azimuth (en degres), plunge (in degrees) for T, N, P axis.

Genevieve Patau, 18 mars 1999
*/
template <typename T, int n>
bool jacobi(T a[n+1][n+1], T d[], T v[n+1][n+1], int *nrot) {
	int j,iq,ip,i;
	T tresh,theta,tau,t,sm,s,h,g,c;
	T b[n+1], z[n+1];
	
	for ( ip = 1; ip <= n; ++ip ) {
		for ( iq = 1; iq <=  n; ++iq ) v[ip][iq] = 0.0;
		v[ip][ip] = 1.0;
	}

	for ( ip = 1; ip <= n; ++ip ) {
		b[ip] = d[ip] = a[ip][ip];
		z[ip] = 0.0;
	}

	*nrot = 0;

	for ( i = 1; i <= 50; ++i ) {
		sm = 0.0;
		for ( ip = 1; ip <= n-1; ++ip ) {
			for ( iq = ip+1; iq <= n; ++iq )
				sm += (T)fabs((double)a[ip][iq]);
		}

		if ( sm == 0.0 )
			return true;

		if ( i < 4 )
			tresh = (T)0.2*sm/((T)(n*n));
		else
			tresh = 0.0;

		for ( ip = 1; ip <= n-1; ++ip ) {
			for ( iq = ip+1; iq <= n; ++iq ) {
				g = (T)(100.0*fabs(a[ip][iq]));
				if ( i > 4 && (T)(fabs(d[ip])+g) == (T)fabs(d[ip] )
				     && (T)(fabs(d[iq])+g) == (T)fabs(d[iq]) )
					a[ip][iq] = 0.0;
				else if ( fabs(a[ip][iq]) > tresh ) {
					h = d[iq]-d[ip];

					if ( (T)(fabs(h)+g) == (T)fabs(h) )
						t = (a[ip][iq])/h;
					else {
						theta = (T)0.5*h/(a[ip][iq]);
						t = (T)(1.0/(fabs(theta)+sqrt(1.0+theta*theta)));
						if ( theta < 0.0 ) t = -t;
					}

					c = (T)(1.0/sqrt(1+t*t));
					s = t*c;
					tau = s/((T)1.0+c);
					h = t*a[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					a[ip][iq] = 0.0;

					for ( j = 1; j <= ip-1; ++j )
						ROTATE(a,j,ip,j,iq)

					for ( j=ip+1; j <= iq-1; ++j )
						ROTATE(a,ip,j,j,iq)

					for ( j=iq+1; j<=n; ++j )
						ROTATE(a,ip,j,iq,j)

					for ( j=1; j<=n; ++j )
						ROTATE(v,j,ip,j,iq)

					++(*nrot);
				}
			}
		}
		for ( ip = 1; ip <= n; ++ip ) {
			b[ip] += z[ip];
			d[ip] = b[ip];
			z[ip] = 0.0;
		}
	}

	return false;
}
#undef ROTATE



/*---------------------------------------------------------------------------*
 *....    Non-Symmetric second rank tensor (gradient & rotation tensors) ....*
 *---------------------------------------------------------------------------*/
template <typename T>
void Tensor2N<T>::assign(Tensor2N<T> & A) {
	_11 = A._11; _12 = A._12; _13 = A._13;
	_21 = A._21; _22 = A._22; _23 = A._23;
	_31 = A._31; _32 = A._32; _33 = A._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::assign(T k) {
	_11 = k; _12 = k; _13 = k;
	_21 = k; _22 = k; _23 = k;
	_31 = k; _32 = k; _33 = k;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::unit() {
	_11 = 1.0; _12 = 0.0; _13 = 0.0;
	_21 = 0.0; _22 = 1.0; _23 = 0.0;
	_31 = 0.0; _32 = 0.0; _33 = 1.0;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::sum(const Tensor2N<T> & A, T k) {
	_11 += k*A._11; _12 += k*A._12; _13 += k*A._13;
	_21 += k*A._21; _22 += k*A._22; _23 += k*A._23;
	_31 += k*A._31; _32 += k*A._32; _33 += k*A._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::scale(T k)
{
	_11 *= k; _12 *= k; _13 *= k;
	_21 *= k; _22 *= k; _23 *= k;
	_31 *= k; _32 *= k; _33 *= k;
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2N<T>::det() const {
	return _11*(_22*_33 - _23*_32)
	+      _12*(_23*_31 - _21*_33)
	+      _13*(_21*_32 - _22*_31);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::inverse(const Tensor2N<T> & A) {
	// compute inverse of determinant
	double idet = 1.0 / A.det();
	// compute inverse tensor
	_11 = idet * (A._22 * A._33 - A._23 * A._32);
	_12 = idet * (A._13 * A._32 - A._12 * A._33);
	_13 = idet * (A._12 * A._23 - A._13 * A._22);
	_21 = idet * (A._23 * A._31 - A._21 * A._33);
	_22 = idet * (A._11 * A._33 - A._13 * A._31);
	_23 = idet * (A._13 * A._21 - A._11 * A._23);
	_31 = idet * (A._21 * A._32 - A._22 * A._31);
	_32 = idet * (A._12 * A._31 - A._11 * A._32);
	_33 = idet * (A._11 * A._22 - A._12 * A._21);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::derive(T * dX1, T * dX2, T * dX3,
                         T * disp, int * inodes, int n) {
	T du1, du2, du3, * dsp;
	T dx1, dx2, dx3;
	// dui/dxj
	_11 = 0.0; _12 = 0.0; _13 = 0.0;
	_21 = 0.0; _22 = 0.0; _23 = 0.0;
	_31 = 0.0; _32 = 0.0; _33 = 0.0;
	// compute gradients node-by-node
	for(int i(0); i < n; i++)
	{	// get nodal components
		dsp = disp + 3*inodes[i];
		du1 = dsp[0];
		du2 = dsp[1];
		du3 = dsp[2];
		// get shape functions derivatives
		dx1 = dX1[i];
		dx2 = dX2[i];
		dx3 = dX3[i];
		// compute gradients
		_11 += du1 * dx1;   _12 += du1 * dx2;   _13 += du1 * dx3;
		_21 += du2 * dx1;   _22 += du2 * dx2;   _23 += du2 * dx3;
		_31 += du3 * dx1;   _32 += du3 * dx2;   _33 += du3 * dx3;
	}
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::spin(Tensor2N<T> &U) {
	// INFINITISIMAL SPIN TENSOR:
	// w_ij = 1/2*(du_i/dx_j - du_j/dx_i)
	_11 = 0.0;
	_22 = 0.0;
	_33 = 0.0;
	_12 = 0.5*(U._12 - U._21);
	_21 = - _12;
	_13 = 0.5*(U._13 - U._31);
	_31 = - _13;
	_23 = 0.5*(U._23 - U._32);
	_32 = - _23;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::deviator(Tensor2N<T> & F) {
	// FD = [det(F)^-1/3] * F
	T J = (T)pow(F.det(), -1.0/3.0);
	_11 = J*F._11; _12 = J*F._12; _13 = J*F._13;
	_21 = J*F._21; _22 = J*F._22; _23 = J*F._23;
	_31 = J*F._31; _32 = J*F._32; _33 = J*F._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::product(const Tensor2N<T> &N, const Tensor2S<T> &S) {
	_11 = N._11*S._11 + N._12*S._12 + N._13*S._13;
	_12 = N._11*S._12 + N._12*S._22 + N._13*S._23;
	_13 = N._11*S._13 + N._12*S._23 + N._13*S._33;
	_21 = N._21*S._11 + N._22*S._12 + N._23*S._13;
	_22 = N._21*S._12 + N._22*S._22 + N._23*S._23;
	_23 = N._21*S._13 + N._22*S._23 + N._23*S._33;
	_31 = N._31*S._11 + N._32*S._12 + N._33*S._13;
	_32 = N._31*S._12 + N._32*S._22 + N._33*S._23;
	_33 = N._31*S._13 + N._32*S._23 + N._33*S._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::product(const Tensor2N<T> &A, const Tensor2N<T> &B) {
	_11 = A._11*B._11 + A._12*B._21 + A._13*B._31;
	_12 = A._11*B._12 + A._12*B._22 + A._13*B._32;
	_13 = A._11*B._13 + A._12*B._23 + A._13*B._33;
	_21 = A._21*B._11 + A._22*B._21 + A._23*B._31;
	_22 = A._21*B._12 + A._22*B._22 + A._23*B._32;
	_23 = A._21*B._13 + A._22*B._23 + A._23*B._33;
	_31 = A._31*B._11 + A._32*B._21 + A._33*B._31;
	_32 = A._31*B._12 + A._32*B._22 + A._33*B._32;
	_33 = A._31*B._13 + A._32*B._23 + A._33*B._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::polarDecomp(Tensor2N<T> & F) {
	// Performs polar decomposition of the deformation gradient
	// tensor into the right stretch tensor and the rotation tensor
	Tensor2S<T> c, cc, cs, u, ui;
	T  L1, L2, L3, i1, i2, i3, k1, k2, k3, k4;
	// compute right Cauchy-Green deformation tensor (RCGDT)
	c.rightCG(F);
	// compute square of RCGDT
	cs.square(c);
	// copy RCGDT
	cc.assign(c);
	// compute eigenvalues of RCGDT
	cc.eigenval();
	// compute principal stretches
	L1 = sqrt(cc._11);
	L2 = sqrt(cc._22);
	L3 = sqrt(cc._33);
	// compute invariants of the right stretch tensor
	i1 = L1 + L2 + L3;
	i2 = L1*L2 + L1*L3 + L2*L3;
	i3 = L1*L2*L3;
	// compute multipliers
	k1 = 1.0/(i1*i2 - i3);
	k2 = i1*i1 - i2;
	k3 = i1*i3;
	k4 = 1.0/i3;
	// compute right stretch tensor
	u._11 = k1*(k2*c._11 - cs._11 + k3);
	u._22 = k1*(k2*c._22 - cs._22 + k3);
	u._33 = k1*(k2*c._33 - cs._33 + k3);
	u._12 = k1*(k2*c._12 - cs._12);
	u._13 = k1*(k2*c._13 - cs._13);
	u._23 = k1*(k2*c._23 - cs._23);
	// compute inverse of right stretch tensor
	ui._11 = k4*(c._11 - i1*u._11 + i2);
	ui._22 = k4*(c._22 - i1*u._22 + i2);
	ui._33 = k4*(c._33 - i1*u._33 + i2);
	ui._12 = k4*(c._12 - i1*u._12);
	ui._13 = k4*(c._13 - i1*u._13);
	ui._23 = k4*(c._23 - i1*u._23);
	// compute rotation tensor R = F*U^-1
	product(F, ui);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2N<T>::HughesWinget(Tensor2N<T> &U) {
	Tensor2N<T> a, b, w;
	// compute spin tensor
	w.spin(U);
	// form combination
	b.unit();
	b.sum(w, -0.5);
	// compute inverse
	a.inverse(b);
	// compute product
	product(a, w);
	// add unit tensor
	_11 += 1.0;
	_22 += 1.0;
	_33 += 1.0;
}
/*---------------------------------------------------------------------------*
 *.......   Symmetric second rank tensor (stress & strain tensors)   ........*
 *---------------------------------------------------------------------------*/
template <typename T>
Tensor2S<T>::Tensor2S(T __11, T __12, T __13,
                              T __22, T __23,
                              T __33)
: _11(__11), _12(__12), _13(__13), _22(__22), _23(__23), _33(__33) {}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::assign(const Tensor2S<T> &A) {
	_11 = A._11;
	_12 = A._12; _22 = A._22;
	_13 = A._13; _23 = A._23; _33 = A._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::assign(T k) {
	_11 = k;
	_12 = k; _22 = k;
	_13 = k; _23 = k; _33 = k;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::spheric(T k) {
	_11 = k;
	_12 = 0.0; _22 = k;
	_13 = 0.0; _23 = 0.0; _33 = k;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::sum(Tensor2S<T> &A, T k) {
	_11 += k*A._11;
	_12 += k*A._12; _22 += k*A._22;
	_13 += k*A._13; _23 += k*A._23; _33 += k*A._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::scale(T k) {
	_11 *= k;
	_12 *= k; _22 *= k;
	_13 *= k; _23 *= k; _33 *= k;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::strain(Tensor2N<T> &U) {
	// INFINITISIMAL STRAIN TENSOR:
	// e_ij = 1/2*(du_i/dx_j + du_j/dx_i)
	_11 = U._11;
	_22 = U._22;
	_33 = U._33;
	_12 = 0.5*(U._12 + U._21);
	_13 = 0.5*(U._13 + U._31);
	_23 = 0.5*(U._23 + U._32);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::deviator(Tensor2N<T> &U) {
	// INFINITISIMAL STRAIN DEVIATOR:
	// e_ij = 1/2*(du_i/dx_j + du_j/dx_i) - 1/3*delta_ij*du_k/dx_k
	_11 = (2.0*U._11 -     U._22 -     U._33) / 3.0;
	_22 = (   -U._11 + 2.0*U._22 -     U._33) / 3.0;
	_33 = (   -U._11 -     U._22 + 2.0*U._33) / 3.0;
	_12 = 0.5*(U._12 + U._21);
	_13 = 0.5*(U._13 + U._31);
	_23 = 0.5*(U._23 + U._32);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::GLStrain(Tensor2N<T> &F) {
	// GREEN-LAGRANGE STRAIN TENSOR:
	// Eij = 1/2*(Fki*Fkj - delta_ij)
	_11 = 0.5*(F._11*F._11 + F._21*F._21 + F._31*F._31 - 1.0);
	_22 = 0.5*(F._12*F._12 + F._22*F._22 + F._32*F._32 - 1.0);
	_33 = 0.5*(F._13*F._13 + F._23*F._23 + F._33*F._33 - 1.0);
	_12 = 0.5*(F._11*F._12 + F._21*F._22 + F._31*F._32);
	_13 = 0.5*(F._11*F._13 + F._21*F._23 + F._31*F._33);
	_23 = 0.5*(F._12*F._13 + F._22*F._23 + F._32*F._33);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::leftCG(Tensor2N<T> &F) {
	// LEFT CAUCHY-GREEN DEFORMATION TENSOR
	// bij = Fik*Fjk
	_11 = F._11*F._11 + F._12*F._12 + F._13*F._13;
	_22 = F._21*F._21 + F._22*F._22 + F._23*F._23;
	_33 = F._31*F._31 + F._32*F._32 + F._33*F._33;
	_12 = F._11*F._21 + F._12*F._22 + F._13*F._23;
	_13 = F._11*F._31 + F._12*F._32 + F._13*F._33;
	_23 = F._21*F._31 + F._22*F._32 + F._23*F._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::rightCG(Tensor2N<T> &F) {
	// RIGHT CAUCHY-GREEN DEFORMATION TENSOR
	// Cij = Fki*Fkj
	_11 = F._11*F._11 + F._21*F._21 + F._31*F._31;
	_22 = F._12*F._12 + F._22*F._22 + F._32*F._32;
	_33 = F._13*F._13 + F._23*F._23 + F._33*F._33;
	_12 = F._11*F._12 + F._21*F._22 + F._31*F._32;
	_13 = F._11*F._13 + F._21*F._23 + F._31*F._33;
	_23 = F._12*F._13 + F._22*F._23 + F._32*F._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::square(Tensor2S<T> &C) {
	// SQUARE OF SYMMETRIC TENSOR C^2 = C^t*C
	_11 = C._11*C._11 + C._12*C._12 + C._13*C._13;
	_22 = C._12*C._12 + C._22*C._22 + C._23*C._23;
	_33 = C._13*C._13 + C._23*C._23 + C._33*C._33;
	_12 = C._11*C._12 + C._12*C._22 + C._13*C._23;
	_13 = C._11*C._13 + C._12*C._23 + C._13*C._33;
	_23 = C._12*C._13 + C._22*C._23 + C._23*C._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::pshFrwd(Tensor2N<T> & F, Tensor2S<T> &S) {
	// Stress Push-Forward transformation:
	// T = 1/det(F) * F * S * F^t
	T idet = 1.0 / F.det(), d1, d2, d3;
	 d1 = F._11*S._11 + F._12*S._12 + F._13*S._13;
	 d2 = F._11*S._12 + F._12*S._22 + F._13*S._23;
	 d3 = F._11*S._13 + F._12*S._23 + F._13*S._33;
	_11 = idet*(d1*F._11 + d2*F._12 + d3*F._13);
	_12 = idet*(d1*F._21 + d2*F._22 + d3*F._23);
	_13 = idet*(d1*F._31 + d2*F._32 + d3*F._33);
	 d1 = F._21*S._11 + F._22*S._12 + F._23*S._13;
	 d2 = F._21*S._12 + F._22*S._22 + F._23*S._23;
	 d3 = F._21*S._13 + F._22*S._23 + F._23*S._33;
	_22 = idet*(d1*F._21 + d2*F._22 + d3*F._23);
	_23 = idet*(d1*F._31 + d2*F._32 + d3*F._33);
	 d1 = F._31*S._11 + F._32*S._12 + F._33*S._13;
	 d2 = F._31*S._12 + F._32*S._22 + F._33*S._23;
	 d3 = F._31*S._13 + F._32*S._23 + F._33*S._33;
	_33 = idet*(d1*F._31 + d2*F._32 + d3*F._33);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::unrotate(const Matrix3<T> &R) {
	Tensor2S<T> tmp;
	tmp.unrotate(R, *this);
	*this = tmp;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::unrotate(const Matrix3<T> &R, const Tensor2S<T> &S) {
	// transform spatial tensor to unrotated configuration
	// U = R^t * S * R
	// Uij = Rmi * Smn * Rnj
	T d1, d2, d3;
	 d1 = R.c._11*S._11 + R.c._21*S._12 + R.c._31*S._13;
	 d2 = R.c._11*S._12 + R.c._21*S._22 + R.c._31*S._23;
	 d3 = R.c._11*S._13 + R.c._21*S._23 + R.c._31*S._33;
	_11 = d1*R.c._11 + d2*R.c._21 + d3*R.c._31;
	_12 = d1*R.c._12 + d2*R.c._22 + d3*R.c._32;
	_13 = d1*R.c._13 + d2*R.c._23 + d3*R.c._33;
	 d1 = R.c._12*S._11 + R.c._22*S._12 + R.c._32*S._13;
	 d2 = R.c._12*S._12 + R.c._22*S._22 + R.c._32*S._23;
	 d3 = R.c._12*S._13 + R.c._22*S._23 + R.c._32*S._33;
	_22 = d1*R.c._12 + d2*R.c._22 + d3*R.c._32;
	_23 = d1*R.c._13 + d2*R.c._23 + d3*R.c._33;
	 d1 = R.c._13*S._11 + R.c._23*S._12 + R.c._33*S._13;
	 d2 = R.c._13*S._12 + R.c._23*S._22 + R.c._33*S._23;
	 d3 = R.c._13*S._13 + R.c._23*S._23 + R.c._33*S._33;
	_33 = d1*R.c._13 + d2*R.c._23 + d3*R.c._33;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::rotate(const Matrix3<T> &R) {
	Tensor2S<T> tmp;
	tmp.rotate(R, *this);
	*this = tmp;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::rotate(const Matrix3<T> &R, const Tensor2S<T> &U) {
	// transform unrotated tensor back to spatial configuration
	// S = R * U * R^t
	// Sij = Rim * Umn * Rjn
	T d1, d2, d3;
	 d1 = R.c._11*U._11 + R.c._12*U._12 + R.c._13*U._13;
	 d2 = R.c._11*U._12 + R.c._12*U._22 + R.c._13*U._23;
	 d3 = R.c._11*U._13 + R.c._12*U._23 + R.c._13*U._33;
	_11 = d1*R.c._11 + d2*R.c._12 + d3*R.c._13;
	_12 = d1*R.c._21 + d2*R.c._22 + d3*R.c._23;
	_13 = d1*R.c._31 + d2*R.c._32 + d3*R.c._33;
	 d1 = R.c._21*U._11 + R.c._22*U._12 + R.c._23*U._13;
	 d2 = R.c._21*U._12 + R.c._22*U._22 + R.c._23*U._23;
	 d3 = R.c._21*U._13 + R.c._22*U._23 + R.c._23*U._33;
	_22 = d1*R.c._21 + d2*R.c._22 + d3*R.c._23;
	_23 = d1*R.c._31 + d2*R.c._32 + d3*R.c._33;
	 d1 = R.c._31*U._11 + R.c._32*U._12 + R.c._33*U._13;
	 d2 = R.c._31*U._12 + R.c._32*U._22 + R.c._33*U._23;
	 d3 = R.c._31*U._13 + R.c._32*U._23 + R.c._33*U._33;
	_33 = d1*R.c._31 + d2*R.c._32 + d3*R.c._33;
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2S<T>::I1() const {
	return _11 + _22 + _33;
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2S<T>::I2() const {
	return _11*_22 + _11*_33 + _22*_33 - _12*_12 - _13*_13 - _23*_23;
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2S<T>::mean() const {
	return (_11 + _22 + _33) / 3.0;
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2S<T>::norm() const {
	// Compute ||s|| = (2*J2)^0.5 = (s : s)^0.5
	return sqrt(_11*_11+_22*_22+_33*_33+2.0*(_12*_12+_13*_13+_23*_23));
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2S<T>::devnorm() const {
	// return norm of deviator
	T mean = (_11 + _22 + _33)/3.0;
	T m11  =  _11 - mean;
	T m22  =  _22 - mean;
	T m33  =  _33 - mean;
	return (T)sqrt(m11*m11+m22*m22+m33*m33+2.0*(_12*_12+_13*_13+_23*_23));
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2S<T>::dtrace() {
	// Make deviator & return trace
	T tr(_11 + _22 + _33);
	T mean(tr/3.0);
	_11 -= mean;
	_22 -= mean;
	_33 -= mean;
	return tr;
}
//---------------------------------------------------------------------------
template <typename T>
T Tensor2S<T>::dmean() {
	// Make deviator & return mean value
	T mean((_11 + _22 + _33) / 3.0);
	_11 -= mean;
	_22 -= mean;
	_33 -= mean;
	return mean;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::augment(T p) {
	// Augment deviator with spheric tensor
	_11 += p;
	_22 += p;
	_33 += p;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::update(Tensor2S<T> & d, T G) {
	T G2(2.0*G);
	// Update deviatoric stress
	_11 += G2*d._11;
	_22 += G2*d._22;
	_33 += G2*d._33;
	_12 += G2*d._12;
	_13 += G2*d._13;
	_23 += G2*d._23;
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::print(FILE * fl) {
	fprintf(fl, "%f %f %f \n", _11, _12, _13);
	fprintf(fl, "%f %f %f \n", _12, _22, _23);
	fprintf(fl, "%f %f %f \n", _13, _23, _33);
}
//---------------------------------------------------------------------------
template <typename T>
void Tensor2S<T>::jacoby(T &pp, T &qq, T &pq, T &rp, T &rq) {
	T theta, t, c, s, tau, w, z;
	// compute rotation angle
	theta = 0.5*(qq - pp)/pq;
	// compute update factors
	if(fabs(theta) > 1e32)
	{	t = 0.5/theta;
		printf("Warning! 'theta' > 1e32 is met in jacoby rotation ...\n\n");
	}
	else
	{	t = 1.0/(fabs(theta) + sqrt(theta*theta + 1.0));
		if(theta < 0.0) t = -t;
	}
	c = 1.0/sqrt(t*t + 1.0);
	s = t*c;
	tau = s/(1.0 + c);
	// update components
	pp -= t*pq;
	qq += t*pq;
	pq  = 0.0;
	w   = rp;
	z   = rq;
	rp -= s*(z + tau*w);
	rq += s*(w - tau*z);
}
//---------------------------------------------------------------------------
template <typename T>
bool Tensor2S<T>::eigenval(T tol, int itmax)
{
	int    iter(0), opt;
	T sum, max, f, eps, min(1e-16);
	// determine stop tolerance
	eps = tol*(fabs(_12) + fabs(_13) + fabs(_23));
	if(eps < min) eps = min;
	// Loop to zero out off-diagonal terms
	do
	{	// select the maximum off-diagonal term
		max = fabs(_12);
		opt = 1;
		sum = max;
		f   = fabs(_13);
		if(f > max)
		{	max = f;
			opt = 2;
		}
		sum += f;
		f = fabs(_23);
		if(f > max)
			opt = 3;
		sum += f;
		// check convergence
		if(sum < eps) break;
		// zero out _12 term
		if(opt == 1) jacoby(_11, _22, _12, _13, _23);
		// zero out _13 term
		if(opt == 2) jacoby(_11, _33, _13, _12, _23);
		// zero out _23 term
		if(opt == 3) jacoby(_22, _33, _23, _12, _13);
	} while(++iter < itmax);
	if(iter == itmax)
	{
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
template <typename T>
void Spectral2S<T>::assign(T k) {
	a1 = k;
	a2 = k;
	a3 = k;
	n1[0] = 1.0; n2[0] = 0.0; n3[0] = 0.0;
	n1[1] = 0.0; n2[1] = 1.0; n3[1] = 0.0;
	n1[2] = 0.0; n2[2] = 0.0; n3[2] = 1.0;
}
//---------------------------------------------------------------------------
template <typename T>
bool Spectral2S<T>::spect(const Tensor2S<T> &A,
                          T                  atol,
                          int                itmax) {
	/*
	//=====================================
	// algorithm for spectral decomposition
	//=====================================
	int    iter(0), opt;
	T a12, a13, a23, f, max, theta, t, c, s, tau, w, z;
	// macro for single Jacoby rotation
	#define ROT(pp, qq, pq, rp, rq, vp, vq)                                        \
	{	theta = 0.5*(qq - pp)/pq;                                                   \
		t     = 1.0/(fabs(theta) + sqrt(theta*theta + 1.0));                        \
		if(theta < 0.0) t = -t;                                                     \
		c = 1.0/sqrt(t*t + 1.0); s = t*c; tau = s/(1.0 + c);                        \
		pp -= t*pq;  qq += t*pq;   pq     = 0.0;                                    \
		w  =  rp;     z  = rq;     rp    -= s*(z + tau*w);  rq    += s*(w - tau*z); \
		w  =  vp[0];  z  = vq[0];  vp[0] -= s*(z + tau*w);  vq[0] += s*(w - tau*z); \
		w  =  vp[1];  z  = vq[1];  vp[1] -= s*(z + tau*w);  vq[1] += s*(w - tau*z); \
		w  =  vp[2];  z  = vq[2];  vp[2] -= s*(z + tau*w);  vq[2] += s*(w - tau*z); \
	}
	// copy tensor components
	a1  = A._11;
	a12 = A._12; a2  = A._22;
	a13 = A._13; a23 = A._23; a3 = A._33;
	// initialize principal directions
	n1[0] = 1.0; n2[0] = 0.0; n3[0] = 0.0;
	n1[1] = 0.0; n2[1] = 1.0; n3[1] = 0.0;
	n1[2] = 0.0; n2[2] = 0.0; n3[2] = 1.0;
	// zero out off-diagonal component
	do
	{	// select maximum off-diagonal component
		f = fabs(a12);               max = f;  opt = 1;
		f = fabs(a13); if(f > max) { max = f;  opt = 2; }
		f = fabs(a23); if(f > max) { max = f;  opt = 3; }
		// check convergence
		if(max < atol) break;
		// perform Jacoby rotation
		if     (opt == 1) ROT(a1, a2, a12, a13, a23, n1, n2) // a12 term
		else if(opt == 2) ROT(a1, a3, a13, a12, a23, n1, n3) // a13 term
		else              ROT(a2, a3, a23, a12, a13, n2, n3) // a23 term
	} while(++iter < itmax);

	if (iter >= itmax) return false;

	return true;
	*/


	T a[4][4], v[4][4];
	T d[4];
	int nrot;

	a[1][1] = A._11; a[1][2] = A._12; a[1][3] = A._13;
	a[2][1] = A._12; a[2][2] = A._22; a[2][3] = A._23;
	a[3][1] = A._13; a[3][2] = A._23; a[3][3] = A._33;

	if ( !jacobi<T,3>(a,d,v,&nrot) ) return false;

	a1 = d[1];
	a2 = d[2];
	a3 = d[3];

	n1.x = v[1][1]; n1.y = v[2][1]; n1.z = v[3][1];
	n2.x = v[1][2]; n2.y = v[2][2]; n2.z = v[3][2];
	n3.x = v[1][3]; n3.y = v[2][3]; n3.z = v[3][3];

	return true;
}
//---------------------------------------------------------------------------
template <typename T>
void Spectral2S<T>::sort() {
	// sort principal values in decending order & permute principal directions
	T atmp, ntmp[3];
	// macro for swapping two principal values & principal directions
	#define SWAP(ai, aj, ni, nj)                        \
	{	atmp    = ai;    ai    = aj;    aj    = atmp;    \
		ntmp[0] = ni[0]; ni[0] = nj[0]; nj[0] = ntmp[0]; \
		ntmp[1] = ni[1]; ni[1] = nj[1]; nj[1] = ntmp[1]; \
		ntmp[2] = ni[2]; ni[2] = nj[2]; nj[2] = ntmp[2]; \
	}
	if(a2 > a1)  SWAP(a1, a2, n1, n2)
	if(a3 > a1)  SWAP(a1, a3, n1, n3)
	if(a3 > a2)  SWAP(a2, a3, n2, n3)
}
//---------------------------------------------------------------------------
template <typename T>
void Spectral2S<T>::absSort() {
	// sort principal values in decending order & permute principal directions
	T atmp, ntmp[3];
	// macro for swapping two principal values & principal directions
	#define SWAP(ai, aj, ni, nj)                        \
	{	atmp    = ai;    ai    = aj;    aj    = atmp;    \
		ntmp[0] = ni[0]; ni[0] = nj[0]; nj[0] = ntmp[0]; \
		ntmp[1] = ni[1]; ni[1] = nj[1]; nj[1] = ntmp[1]; \
		ntmp[2] = ni[2]; ni[2] = nj[2]; nj[2] = ntmp[2]; \
	}
	if(fabs(a2) > fabs(a1))  SWAP(a1, a2, n1, n2)
	if(fabs(a3) > fabs(a1))  SWAP(a1, a3, n1, n3)
	if(fabs(a3) > fabs(a2))  SWAP(a2, a3, n2, n3)
}
//---------------------------------------------------------------------------
template <typename T>
void Spectral2S<T>::compose(Tensor2S<T> &A) {
	// compose symmetric tensor from spectral representation
	A._11 = a1*n1[0]*n1[0] + a2*n2[0]*n2[0] + a3*n3[0]*n3[0];
	A._12 = a1*n1[0]*n1[1] + a2*n2[0]*n2[1] + a3*n3[0]*n3[1];
	A._13 = a1*n1[0]*n1[2] + a2*n2[0]*n2[2] + a3*n3[0]*n3[2];
	A._22 = a1*n1[1]*n1[1] + a2*n2[1]*n2[1] + a3*n3[1]*n3[1];
	A._23 = a1*n1[1]*n1[2] + a2*n2[1]*n2[2] + a3*n3[1]*n3[2];
	A._33 = a1*n1[2]*n1[2] + a2*n2[2]*n2[2] + a3*n3[2]*n3[2];
}
//---------------------------------------------------------------------------
template <typename T>
T Spectral2S<T>::norm() {
	// compute norm of spectrally decomposed tensor
	return sqrt(a1*a1 + a2*a2 + a3*a3);
}
//---------------------------------------------------------------------------
template class SC_SYSTEM_CORE_API Tensor2N<float>;
template class SC_SYSTEM_CORE_API Tensor2N<double>;

template class SC_SYSTEM_CORE_API Tensor2S<float>;
template class SC_SYSTEM_CORE_API Tensor2S<double>;

template class SC_SYSTEM_CORE_API Spectral2S<float>;
template class SC_SYSTEM_CORE_API Spectral2S<double>;


}
}

