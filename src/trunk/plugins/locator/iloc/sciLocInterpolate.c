/*
 * Copyright (c) 2018-2019, Istvan Bondar,
 * Written by Istvan Bondar, ibondar2014@gmail.com
 *
 * BSD Open Source License.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "sciLocInterface.h"

/*
 * Functions:
 *    iLoc_SplineCoeffs
 *    iLoc_SplineInterpolation
 *    iLoc_BilinearInterpolation
 *    iLoc_FloatBracket
 *    iLoc_IntegerBracket
 */

/*
 *  Title:
 *     iLoc_SplineCoeffs
 *  Synopsis:
 *	   Calculates interpolating coefficients for a natural spline
 *  Input Arguments:
 *     n   - number of points
 *     x   - x array
 *     y   - y array
 *  Output Arguments:
 *     d2y - second derivatives of the natural spline interpolating function
 *     tmp - temp array of n elements
 *  Called by:
 *     GetTravelTimeTableValue, ReadVariogram
 */
void iLoc_SplineCoeffs(int n, double *x, double *y, double *d2y, double *tmp)
{
    double temp = 0., d = 0.;
    int i;
    d2y[0] = tmp[0] = 0.;
    for (i = 1; i < n - 1; i++) {
        d = (x[i] - x[i-1]) / (x[i+1] - x[i-1]);
        temp = d * d2y[i-1] + 2.;
        d2y[i] = (d - 1.) / temp;
        tmp[i] = (y[i+1] - y[i])   / (x[i+1] - x[i]) -
                 (y[i]   - y[i-1]) / (x[i]   - x[i-1]);
        tmp[i] = (6. * tmp[i] / (x[i+1] - x[i-1]) - d * tmp[i-1]) / temp;
    }
    d2y[n-1] = 0;
    for (i = n - 2; i >= 0; i--) {
        d2y[i] = d2y[i] * d2y[i+1] + tmp[i];
    }
}

/*
 *  Title:
 *     iLoc_SplineInterpolation
 *  Synopsis:
 *	   Returns interpolated function value f(xp) by cubic spline interpolation
 *  Input Arguments:
 *     xp  - x point to be interpolated
 *     n   - number of points
 *     x   - x array
 *     y   - y array
 *     d2y - second derivatives of the natural spline interpolating function
 *     isderiv - calculate derivatives [0/1]
 *  Output Arguments:
 *     dydx  - first derivative
 *     d2ydx - second derivative
 *  Return:
 *     interpolated function value yp = f(xp)
 *  Called by:
 *     GetTravelTimeTableValue, iLoc_GetDataCovarianceMatrix
 *  Calls:
 *     iLoc_FloatBracket
 */
double iLoc_SplineInterpolation(double xp, int n, double *x, double *y,
        double *d2y, int isderiv, double *dydx, double *d2ydx)
{
    double h = 0., g = 0., a = 0., b = 0., c = 0., d = 0., yp = 0.;
    int klo = 0, khi = 0;
    *dydx = *d2ydx = -999.;
/*
 *  bracket xp
 */
    iLoc_FloatBracket(xp, n, x, &klo, &khi);
/*
 *  interpolate yp
 */
    h = x[khi] - x[klo];
    g = y[khi] - y[klo];
    a = (x[khi] - xp) / h;
    b = (xp - x[klo]) / h;
    c = (a * a * a - a) * h * h / 6.;
    d = (b * b * b - b) * h * h / 6.;
    yp = a * y[klo] + b * y[khi] + c * d2y[klo] + d * d2y[khi];
/*
 *  derivatives
 */
    if (isderiv) {
        *dydx = g / h - (3. * a * a - 1.) * h * d2y[klo] / 6. +
                        (3. * b * b - 1.) * h * d2y[khi] / 6.;
        *d2ydx = a * d2y[klo] + b * d2y[khi];
    }
    return yp;
}

/*
 *  Title:
 *     iLoc_FloatBracket
 *  Synopsis:
 *	   For a vector x, ordered in ascending order, return indices jlo and jhi
 *     such that x[jlo] <= xp < x[jhi]
 *  Input Arguments:
 *     xp  - x point to be bracketed
 *     n   - number of points in x
 *     x   - x array
 *  Output Arguments:
 *     jlo - lower index
 *     jhi - upper index
 *  Called by:
 *     iLoc_SplineInterpolation, iLoc_BilinearInterpolation,
 *     GetTravelTimeTableValue
 */
void iLoc_FloatBracket(double xp, int n, double *x, int *jlo, int *jhi)
{
    int klo = 0, khi = 0, k = 0;
    *jlo = klo = 0;
    *jhi = khi = n - 1;
    if (n < 2) return;
    while (khi - klo > 1) {
        k = (khi + klo) >> 1;
        if (x[k] > xp)
            khi = k;
        else
            klo = k;
    }
    if (klo < 0)     klo = 0;
    if (khi > n - 1) khi = n - 1;
    *jlo = klo;
    *jhi = khi;
}

/*
 *  Title:
 *     iLoc_IntegerBracket
 *  Synopsis:
 *	   For a vector x, ordered in ascending order, return indices jlo and jhi
 *     such that x[jlo] <= xp < x[jhi]
 *  Input Arguments:
 *     xp  - x point to be bracketed
 *     n   - number of points in x
 *     x   - x array
 *  Output Arguments:
 *     jlo - lower index
 *     jhi - upper index
 *  Called by:
 *     iLoc_GregionNumber, iLoc_GregToSreg
 */
void iLoc_IntegerBracket(int xp, int n, int *x, int *jlo, int *jhi)
{
    int klo = 0, khi = 0, k = 0;
    *jlo = klo = 0;
    *jhi = khi = n - 1;
    if (n < 2) return;
    while (khi - klo > 1) {
        k = (khi + klo) >> 1;
        if (x[k] > xp)
            khi = k;
        else
            klo = k;
    }
    if (klo < 0)     klo = 0;
    if (khi > n - 1) khi = n - 1;
    *jlo = klo;
    *jhi = khi;
}

/*
 *  Title:
 *     iLoc_BilinearInterpolation
 *  Synopsis:
 *	   Returns interpolated function value f(xp1,xp2) by bilinear interpolation
 *  Input Arguments:
 *     xp1  - x1 point to be interpolated
 *     xp2  - x2 point to be interpolated
 *     nx1  - number of points in x1
 *     nx2  - number of points in x2
 *     x1   - x1 vector
 *     x2   - x2 vector
 *     y    - y matrix over x1 and x2
 *  Return:
 *     interpolated function value yp = f(xp1, xp2)
 *  Called by:
 *     GetEllipticityCorrection
 *  Calls:
 *     iLoc_FloatBracket
 */
double iLoc_BilinearInterpolation(double xp1, double xp2, int nx1, int nx2,
                    double *x1, double *x2, double **y)
{
    int ilo = 0, ihi = 0, jlo = 0, jhi = 0;
    double f1 = 0., f2 = 0., yp = 0.;
/*
 *  bracket xp1 and xp2
 */
    iLoc_FloatBracket(xp1, nx1, x1, &ilo, &ihi);
    iLoc_FloatBracket(xp2, nx2, x2, &jlo, &jhi);
/*
 *  scalers
 */
    f1 = (xp1 - x1[ilo]) / (x1[ihi] - x1[ilo]);
    f2 = (xp2 - x2[jlo]) / (x2[jhi] - x2[jlo]);
/*
 *  interpolate
 */
    yp = (1. - f1) * (1. - f2) * y[ilo][jlo] +
         f1 * (1. - f2) * y[ihi][jlo] +
         f1 * f2  * y[ihi][jhi] +
         (1. - f1) * f2 * y[ilo][jhi];
    return yp;
}

