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
 *    iLoc_SVDdecompose
 *    iLoc_SVDsolve
 *    iLoc_SVDModelCovarianceMatrix
 *    iLoc_SVDthreshold
 *    iLoc_SVDrank
 *    iLoc_SVDnorm
 *    iLoc_ProjectionMatrix
 */

/*
 * Local functions:
 *    SVDreorder
 *    Pythagorean
 *    Wmatrix for parallelisation
 *    EigenDecompose
 *    GetPhaDef
 *    FreePhaDef
 *    dlamch
 */
static int SVDreorder(int n, int m, double **u, double w[], double **v);
static double Pythagorean(double a, double b);
static int Wmatrix(ILOC_PHADEF *PhaDef, double pct, double **cov, double **w,
        int nunp, ILOC_PHASELIST *phundef, int ispchange);
static int EigenDecompose(int nd, double *avec, double **u, double *sv);
static int GetPhaDef(int numPhase, ILOC_ASSOC *Assocs, int numPhaDef,
        ILOC_PHADEF *PhaDef);
static void FreePhaDef(int nphases, ILOC_PHADEF *PhaDef);
static double dlamch(char CMACH);

#ifndef MACOSX
extern void dsyevr_(char *jobz, char *range, char *uplo, int *n, double *a,
        int *lda, double *vl, double *vu, int *il, int *iu, double *abstol,
        int *m, double* w, double *z, int *ldz, int *isuppz, double *work,
        int *lwork, int *iwork, int *liwork, int *info);
extern double dlamch_(char *CMACHp);
#endif

/*
 * Singular value decomposition of an (NxM) matrix
 *    A = U * SV * transpose(V)
 *    The matrix U(N x M) replaces A on output.
 *    The diagonal matrix of singular values SV is output as a vector SV(M).
 *    The matrix V (not the transpose) is output as V(M x M).
 *    Adopted from Numerical Recipes
 *
 *    Input arguments:
 *       n  - number of data (rows)
 *       m  - number of model parameters (columns)
 *       u  - A matrix to be decomposed U(N x M)
 *       sv - singular values SV(M), descending order
 *       v  - orthonormal V matrix V(M x M). V * transpose(V) = I
 *    Output arguments:
 *       u  - U matrix U(N x M) (orthonormal if N >= M)
 *    Returns:
 *       Success/error
 *    Called by:
 *       LocateEvent
 *    Calls:
 *       SVDreorder, Pythagorean
 */
int iLoc_SVDdecompose(int n, int m, double **u, double sv[], double **v)
{
    int flag = 0;
    int i, its = 0, j, jj, k, l, nm;
    double anorm = 0., c = 0., f = 0., g = 0., h = 0., s = 0.;
    double scale = 0., x = 0., y = 0., z = 0.;
    double *rv1 = (double *)NULL;
    if ((rv1 = (double *)calloc(m, sizeof(double))) == NULL) {
        fprintf(stderr, "iLoc_SVDdecompose: cannot allocate memory\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  Householder reduction to bidiagonal form
 */
    g = scale = anorm = 0.0;
    for (i = 0; i < m; i++) {
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if (i < n) {
            for (k = i; k < n; k++) scale += fabs(u[k][i]);
            if (scale != 0.0) {
                for (k = i; k < n; k++) {
                    u[k][i] /= scale;
                    s += u[k][i] * u[k][i];
                }
                f = u[i][i];
                g = -ILOC_SIGN(ILOC_SQRT(s), f);
                h = f * g - s;
                u[i][i] = f - g;
                for (j = l; j < m; j++) {
                    s = 0.0;
                    for (k = i; k < n; k++) s += u[k][i] * u[k][j];
                    f = s / h;
                    for (k = i; k < n; k++) u[k][j] += f * u[k][i];
                }
                for (k = i; k < n; k++) u[k][i] *= scale;
            }
        }
        sv[i] = scale * g;
        g = s = scale = 0.0;
        if (i < n && i != (m - 1)) {
            for (k = l; k < m; k++) scale += fabs(u[i][k]);
            if (scale != 0.0) {
                for (k = l; k < m; k++) {
                    u[i][k] /= scale;
                    s += u[i][k] * u[i][k];
                }
                f = u[i][l];
                g = -ILOC_SIGN(ILOC_SQRT(s), f);
                h = f * g - s;
                u[i][l] = f - g;
                for (k = l; k < m; k++) rv1[k] = u[i][k] / h;
                for (j = l; j < n; j++) {
                    s = 0.0;
                    for (k = l; k < m; k++) s += u[j][k] * u[i][k];
                    for (k = l; k < m; k++) u[j][k] += s * rv1[k];
                }
                for (k = l; k < m; k++) u[i][k] *= scale;
            }
        }
        anorm = ILOC_MAX(anorm, (fabs(sv[i]) + fabs(rv1[i])));
    }
/*
 *  accumulation of right-hand transformations
 */
    for (i = m - 1; i >= 0; i--) {
        if (i < (m - 1)) {
            if (g != 0.0) {
                for (j = l; j < m; j++)
                    v[j][i] = (u[i][j] / u[i][l]) / g;
                for (j = l; j < m; j++) {
                    s = 0.0;
                    for (k = l; k < m; k++) s += u[i][k] * v[k][j];
                    for (k = l; k < m; k++) v[k][j] += s * v[k][i];
                }
            }
            for (j = l; j < m; j++) v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }
/*
 *  accumulation of left-hand transformations
 */
    for (i = ILOC_MIN(m, n) - 1; i >= 0; i--) {
        l = i + 1;
        g = sv[i];
        for (j = l; j < m; j++) u[i][j] = 0.0;
        if (g != 0.0) {
            g = 1.0 / g;
            for (j = l; j < m; j++) {
                s = 0.0;
                for (k = l; k < n; k++) s += u[k][i] * u[k][j];
                f =(s / u[i][i]) * g;
                for (k = i; k < n; k++) u[k][j] += f * u[k][i];
            }
            for (j = i; j < n; j++) u[j][i] *= g;
        }
        else
            for (j = i; j < n; j++) u[j][i] = 0.0;
        u[i][i] += 1.;
    }
/*
 *  diagonalization of the bidiagonal form
 */
    for (k = m - 1; k >= 0; k--) {
        for (its = 0; its < 30; its++) {
            flag = 1;
            for (l = k; l >= 0; l--) {
                nm = l - 1;
                if (l == 0 || fabs(rv1[l]) < ILOC_DEPSILON) {
                    flag = 0;
                    break;
                }
                if (fabs(sv[nm]) < ILOC_DEPSILON) break;
            }
            if (flag) {
/*
 *              cancellation of rv1[l] if l greater than 0
 */
                c = 0.0;
                s = 1.0;
                for (i = l; i < k + 1; i++) {
                    f = s * rv1[i];
                    rv1[i] = c * rv1[i];
                    if (fabs(f) < ILOC_DEPSILON) break;
                    g = sv[i];
                    h = Pythagorean(f, g);
                    sv[i] = h;
                    if (h > ILOC_ZEROTOL) {
                        h = 1.0 / h;
                        c = g * h;
                        s = -f * h;
                    }
                    for (j = 0; j < n; j++) {
                        y = u[j][nm];
                        z = u[j][i];
                        u[j][nm] = y * c + z * s;
                        u[j][i]  = z * c - y * s;
                    }
                }
            }
/*
 *          test for convergence
 */
            z = sv[k];
            if (l == k) {
                if (z < 0.0) {
                    sv[k] = -z;
                    for (j = 0; j < m; j++) v[j][k] = -v[j][k];
                }
                break;
            }
            if (its == 29) {
                iLoc_Free(rv1);
                fprintf(stderr, "iLoc_SVDdecompose: max iteration reached!\n");
                return ILOC_SLOW_CONVERGENCE;
            }
/*
 *          shift from bottom 2 by 2 minor
 */
            x = sv[l];
            nm = k - 1;
            y = sv[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = Pythagorean(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + ILOC_SIGN(g, f))) - h)) / x;
            c = s = 1.0;
/*
 *          next QR transformation
 */
            for (j = l; j <= nm; j++) {
                i = j + 1;
                g = rv1[i];
                y = sv[i];
                h = s * g;
                g = c * g;
                z = Pythagorean(f, h);
                rv1[j] = z;
                if (z > ILOC_ZEROTOL) {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y *= c;
                for (jj = 0; jj < m; jj++) {
                    x = v[jj][j];
                    z = v[jj][i];
                    v[jj][j] = x * c + z * s;
                    v[jj][i] = z * c - x * s;
                }
                z = Pythagorean(f, h);
                sv[j] = z;
/*
 *              rotation can be arbitrary if z is zero
 */
                if (z > ILOC_ZEROTOL) {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = c * g + s * y;
                x = c * y - s * g;
                for (jj = 0; jj < n; jj++) {
                    y = u[jj][j];
                    z = u[jj][i];
                    u[jj][j] = y * c + z * s;
                    u[jj][i] = z * c - y * s;
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            sv[k] = x;
        }
    }
    iLoc_Free(rv1);
    if (SVDreorder(n, m, u, sv, v))
        return ILOC_MEMORY_ALLOCATION_ERROR;
    return ILOC_SUCCESS;
}

/*
 * Order singular values
 *    Descending order of singular values and corresponding U and V matrices
 *    A = U * W * transpose(V)
 *    Adopted from Numerical Recipes
 *
 *    Input arguments:
 *       n  - number of data (rows)
 *       m  - number of model parameters (columns)
 *       u  - U matrix  U(N x M)
 *       w  - singular values SV(M), unordered
 *       v  - V matrix V(M x M)
 *    Output arguments:
 *       u  - U matrix U(N x M)
 *       w  - singular values SV(M), ordered
 *       v  - V matrix V(M x M)
 *    Returns:
 *       Success/error
 *    Called by:
 *       iLoc_SVDdecompose
 */
static int SVDreorder(int n, int m, double **u, double w[], double **v)
{
    int i, j, k, s, inc = 1;
    double sw = 0.;
    double *su = (double *)NULL;
    double *sv = (double *)NULL;
    su = (double *)calloc(n, sizeof(double));
    if ((sv = (double *)calloc(m, sizeof(double))) == NULL) {
        iLoc_Free(su);
        fprintf(stderr, "SVDreorder: cannot allocate memory\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
    do { inc *= 3; inc++; } while (inc <= m);
    do {
        inc /= 3;
        for (i = inc; i < m; i++) {
            sw = w[i];
            for (k = 0; k < n; k++) su[k] = u[k][i];
            for (k = 0; k < m; k++) sv[k] = v[k][i];
            j = i;
            while (w[j-inc] < sw) {
                w[j] = w[j-inc];
                for (k = 0; k < n; k++) u[k][j] = u[k][j-inc];
                for (k = 0; k < m; k++) v[k][j] = v[k][j-inc];
                j -= inc;
                if (j < inc) break;
            }
            w[j] = sw;
            for (k = 0; k < n; k++) u[k][j] = su[k];
            for (k = 0; k < m; k++) v[k][j] = sv[k];

        }
    } while (inc > 1);
    for (k = 0; k < m; k++) {
        s = 0;
        for (i = 0; i < n; i++) if (u[i][k] < 0.) s++;
        for (j = 0; j < m; j++) if (v[j][k] < 0.) s++;
        if (s > (m + n) / 2) {
            for (i = 0; i < n; i++) u[i][k] = -u[i][k];
            for (j = 0; j < m; j++) v[j][k] = -v[j][k];
        }
    }
    iLoc_Free(sv);
    iLoc_Free(su);
    return ILOC_SUCCESS;
}

static double Pythagorean(double a, double b)
{
    double absa, absb;
    absa = fabs(a);
    absb = fabs(b);
    if (absa > absb)
        return absa * ILOC_SQRT(1.0 + pow(absb / absa, 2));
    else if (absb < ILOC_DEPSILON)
        return 0.;
    else
        return absb * ILOC_SQRT(1.0 + pow(absa / absb, 2));
}

/*
 * Solve Ax = b with SVD
 *    Solve Ax = b for a vector x, where A = U * W * transpose(V)
 *    as returned by iLoc_SVDdecompose and SVDreorder. N >= M is assumed.
 *    No input quantities are destroyed, so the routine may be called
 *    sequentially with different b's.
 *
 *    Input arguments:
 *       n  - number of data (rows)
 *       m  - number of model parameters (columns)
 *       u  - U matrix U(N x M)
 *       sv - singular values SV(M)
 *       v  - V matrix V(M x M)
 *       b  - b vector b(N)
 *       thres - threshold to zero out singular values
 *               if thres is negative a default value based on
 *               estimated roundoff is used
 *    Output arguments:
 *       x - x vector x(M)
 *    Returns:
 *       Success/error
 *    Called by:
 *       LocateEvent
 *    Calls:
 *       iLoc_SVDthreshold
 */
int iLoc_SVDsolve(int n, int m, double **u, double sv[], double **v, double *b,
        double *x, double thres)
{
    int i, j, jj;
    double s = 0., tsh = 0.;
    double *tmp = (double *)NULL;
    if ((tmp = (double *)calloc(m, sizeof(double))) == NULL) {
        fprintf(stderr, "iLoc_SVDsolve: cannot allocate memory\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
    tsh = (thres >= 0.) ? thres : iLoc_SVDthreshold(n, m, sv);
    for (j = 0; j < m; j++) {
        s = 0.0;
        if (sv[j] > tsh) {
            for (i = 0; i < n; i++) s += u[i][j] * b[i];
            s /= sv[j];
        }
        tmp[j] = s;
    }
    for (j = 0; j < m; j++) {
        s = 0.0;
        for (jj = 0; jj < m; jj++) s += v[j][jj] * tmp[jj];
        x[j] = s;
    }
    iLoc_Free(tmp);
    return ILOC_SUCCESS;
}

/*
 * Calculates a posteriori model covariance matrix
 *    ModCov(M x M) = V * (1 / SV^2) * transpose(V) =
 *                    Ginv * DataCov * transpose(Ginv)
 *    Ginv = V * 1/SV * transpose(U)
 *
 *    Input arguments:
 *       m     - number of model parameters
 *       thres - threshold to zero out singular values
 *       sv    - singular values SV(M)
 *       v     - V matrix V(M x M)
 *    Output arguments:
 *       mcov  - model covariance matrix MCOV(M x M)
 *    Called by:
 *       LocateEvent
 */
void iLoc_SVDModelCovarianceMatrix(int m, double thres, double sv[], double **v,
        double mcov[][4])
{
    int i, j, k;
    double s = 0.;
    for (i = 0; i < m; i++) {
        for (j = 0; j < i + 1; j++) {
            mcov[i][j] = mcov[j][i] = 0.0;
            for (s = 0., k = 0; k < m; k++) {
                if (sv[k] > thres)
                    s += v[i][k] * v[j][k] / (sv[k] * sv[k]);
            }
            mcov[j][i] = mcov[i][j] = s;
        }
    }
}

/*
 * Get default threshold to zero out singular values
 *    Input arguments:
 *       n  - number of data (rows)
 *       m  - number of model parameters (columns)
 *       sv - singular values SV(M)
 *    Returns:
 *       threshold
 *    Called by:
 *       LocateEvent, iLoc_SVDsolve, iLoc_SVDrank
 */
double iLoc_SVDthreshold(int n, int m, double sv[])
{
    return 0.5 * ILOC_SQRT(n + m + 1.) * sv[0] * ILOC_DEPSILON;
}


/*
 * Rank of A(N x M) after zeroing out singular values less than a threshold
 *    if thres is negative a default value based on estimated roundoff is used
 *    Input arguments:
 *       n  - number of data (rows)
 *       m  - number of model parameters (columns)
 *       sv - singular values SV(M)
 *       thres - threshold to zero out singular values
 *               if thres is negative a default value based on
 *               estimated roundoff is used
 *    Returns:
 *       rank
 *    Called by:
 *       LocateEvent
 *    Calls:
 *       iLoc_SVDthreshold
 */
int iLoc_SVDrank(int n, int m, double sv[], double thres)
{
    int j, nr = 0;
    double tsh = 0.;
    tsh = (thres >= 0.) ? thres : iLoc_SVDthreshold(n, m, sv);
    for (j = 0; j < m; j++) if (sv[j] > tsh) nr++;
    return nr;
}

/*
 * Condition number and G matrix norm
 *    Input arguments:
 *       m     - number of model parameters )M)
 *       sv    - singular values SV(M)
 *       thres - threshold to zero out singular values
 *    Output arguments:
 *       cond   - condition number, largest / smallest singular value used
 *       isvmax - index of largest singular value
 *    Returns:
 *       G matrix norm = sum of squares of singular values
 *    Called by:
 *       LocateEvent
 */
double iLoc_SVDnorm(int m, double sv[], double thres, double *cond)
{
    double norm = 0., cnum = 0.;
    int i;
    for (i = 0; i < m; i++) {
        if (sv[i] <= thres) break;
        norm += sv[i] * sv[i];
    }
    if (i == m) i--;
    cnum = (sv[0] <= 0. || sv[i] <= 0.) ? 99999. : sv[0] / sv[i];
    *cond = cnum;
    return norm;
}

/*
 *  Projection matrix to project Gm = d into eigen system: WGm = Wd
 *
 *     Bondár, I., and K. McLaughlin, 2009,
 *        Seismic location bias and uncertainty in the presence of correlated
 *        and non-Gaussian travel-time errors,
 *        Bull. Seism. Soc. Am., 99, 172-193.
 *     Bondár, I., and D. Storchak, 2011,
 *        Improved location procedures at the International Seismological
 *        Centre,
 *        Geophys. J. Int., doi: 10.1111/j.1365-246X.2011.05107.x.
 *
 *    W(N x N) = 1 / sqrt(SV) * transpose(U) = Binv
 *           B = U * sqrt(SV)
 *           C = B * transpose(B) = U * SV * transpose(V)
 *        Cinv = transpose(W) * W = V * 1/SV * transpose(U)
 *
 *    Since different phases travel along different ray paths,
 *    they are uncorrelated. This makes the data covariance matrix
 *    block-diagonal (when ordered by phases). Furthermore, if the
 *    observations are ordered by the nearest-neighbour station order
 *    the phase blocks themselves exhibit a block-diagonal structure.
 *    To improve efficiency and speed, the data covariance matrix is inverted
 *    block by block instead of doing one monster inversion.
 *
 *    Input arguments:
 *       numPhaDef - number of phases in PhaDef
 *       PhaDef    - array of ILOC_PHADEF structures
 *       numPhase  - number of associated phases
 *       Assocs    - array of ILOC_ASSOC structures
 *       nd        - number of defining phases
 *       pctvar    - percentage of total variance to be explained
 *       cov       - data covariance matrix C(N x N)
 *       nunp      - number of distinct phases made non-defining
 *       phundef   - list of distinct phases made non-defining
 *       ispchange - was there a change in phase names?
 *    Output arguments:
 *       nrank     - rank of G matrix at pctvar level
 *       w         - projection matrix (N x N)
 *    Returns:
 *       Success/error
 *    Called by:
 *       LocateEvent, NAForwardProblem
 *    Calls:
 *       GetPhaDef, FreePhaDef, Wmatrix
 */
int iLoc_ProjectionMatrix(int numPhaDef, ILOC_PHADEF *PhaDef, int numPhase,
        ILOC_ASSOC *Assocs, int nd, double pctvar, double **cov, double **w,
        int *nrank, int nunp, ILOC_PHASELIST *phundef, int ispchange)
{
    int i, j, knull = 0, nphases = 0;
    double sum = 0., pct = 0.;
/*
 *  populate PhaDef structure
 */
    if ((nphases = GetPhaDef(numPhase, Assocs, numPhaDef, PhaDef)) == 0)
        return ILOC_MEMORY_ALLOCATION_ERROR;
    pct = pctvar / 100.;
/*
 *  calculate projection matrix
 */
    for (j = 0; j < nphases; j++) {
        if (Wmatrix(&PhaDef[j], pct, cov, w, nunp, phundef, ispchange)) {
            FreePhaDef(nphases, PhaDef);
            return ILOC_MEMORY_ALLOCATION_ERROR;
        }
    }
/*
 *  free memory in PhaDef
 */
    FreePhaDef(nphases, PhaDef);
/*
 *  calculate effective rank from projection matrix
 */
    knull = 0;
    for (i = 0; i < nd; i++) {
        sum = 0.;
        for (j = 0; j < nd; j++) {
            sum += w[i][j];
        }
        if (fabs(sum) < 1.e-5) knull++;
    }
    *nrank = nd - knull;
    return ILOC_SUCCESS;
}

/*
 * Calculate the projection matrix W for a phase block
 *        W = 1 / sqrt(SV) * transpose(U)
 *    Exploits the block-diagonal structure of a phase block
 *    by inverting the covariance matrix block by block.
 *    Uses Lapack routines to obtain the eigenvalue decomposition
 *    of the symmetric, positive semi-definite covariance matrix.
 *    Input arguments:
 *       PhaDef    - pointer to ILOC_PHADEF structure
 *       pct       - percentage of total variance to be explained
 *       cov       - data covariance matrix C(N x N)
 *       w         - projection matrix (N x N)
 *       nunp      - number of distinct phases made non-defining
 *       phundef   - list of distinct phases made non-defining
 *       ispchange - was there a change in phase names?
 *    Output arguments:
 *       nrank     - rank of G matrix at pctvar level
 *       w         - projection matrix (N x N)
 *    Returns:
 *       Success/error
 *    Called by:
 *       iLoc_ProjectionMatrix
 *    Calls:
 *       iLoc_AllocateFloatMatrix, iLoc_FreeFloatMatrix, iLoc_Free,
 *       EigenDecompose, iLoc_SVDthreshold
 */
static int Wmatrix(ILOC_PHADEF *PhaDef, double pct, double **cov, double **w,
        int nunp, ILOC_PHASELIST *phundef, int ispchange)
{
    int i, k, m, np = 0, mp = 0, ii, jj;
    int knull = 0, nr = 0, isfound = 0;
    double sum = 0., esum = 0., psum = 0., ths = 0., x = 0.;
    double **u = (double **)NULL;
    double **z = (double **)NULL;
    double *sv = (double *)NULL;
    double *avec = (double *)NULL;
/*
 *  deal only with those phases that were made non-defining
 *  if phase name changes have occured, build W from scratch
 */
    isfound = 0;
    for (i = 0; i < nunp; i++)
        if (ILOC_STREQ(phundef[i].Phase, PhaDef->Phase))
            isfound = 1;
    if (isfound == 0 && ispchange == 0)
        return ILOC_SUCCESS;
/*
 *  Time
 *      number of time observations for this phase
 *
 */
    np = PhaDef->nTime;
    if (np) {
/*
 *      only a single time observation for this phase
 */
        if (np == 1) {

            ii = PhaDef->indTime[0];
            w[ii][ii] = 0.;
            if (cov[ii][ii] > ILOC_ZEROTOL)
                w[ii][ii] = 1. / ILOC_SQRT(cov[ii][ii]);
        }
/*
 *      multiple time observations
 */
        else {
            if ((z = iLoc_AllocateFloatMatrix(np, np)) == NULL) {
                fprintf(stderr, "Wmatrix: cannot allocate memory\n");
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
/*
 *          build covariance block for this phase
 */
            for (k = 0; k < np; k++) {
                ii = PhaDef->indTime[k];
                z[k][k] = cov[ii][ii];
                for (m = 0; m < k; m++) {
                    jj = PhaDef->indTime[m];
                    z[k][m] = cov[ii][jj];
                    z[m][k] = cov[jj][ii];
                }
            }
/*
 *          find diagonal sub-blocks in the covariance block for this phase
 */
            for (k = 0; k < np; k++) {
                mp = np - 1;
                while (z[k][mp] < ILOC_ZEROTOL) mp--;
/*
 *              only a single observation in this sub-block
 */
                if (mp == k) {
                    ii = PhaDef->indTime[k];
                    w[ii][ii] = 0.;
                    if (z[k][k] > ILOC_ZEROTOL)
                        w[ii][ii] = 1. / ILOC_SQRT(z[k][k]);
                    continue;
                }
/*
 *              multiple observations in this sub-block
 */
                else {
                    for (i = k + 1; i < mp; i++) {
                        m = np - 1;
                        while (z[i][m] < ILOC_ZEROTOL) m--;
                        if (m > mp) mp = m;
                    }
                    mp = mp - k + 1;
                    u = iLoc_AllocateFloatMatrix(mp, mp);
                    avec = (double *)calloc(mp * mp, sizeof(double));
                    if ((sv = (double *)calloc(mp, sizeof(double))) == NULL) {
                        fprintf(stderr, "Wmatrix: cannot allocate memory\n");
                        iLoc_FreeFloatMatrix(u);
                        iLoc_FreeFloatMatrix(z);
                        iLoc_Free(avec);
                        return ILOC_MEMORY_ALLOCATION_ERROR;
                    }
/*
 *                  copy the covariance matrix of this sub-block into avec
 */
                    for (ii = 0, i = k; ii < mp; ii++, i++) {
                        for (m = 0, jj = k; m < mp; m++, jj++) {
                            avec[ii + m * mp] = z[i][jj];
                        }
                    }
/*
 *                  Eigenvalue decomposition
 */
                    if (EigenDecompose(mp, avec, u, sv)) {
                        iLoc_FreeFloatMatrix(u);
                        iLoc_FreeFloatMatrix(z);
                        iLoc_Free(avec);
                        iLoc_Free(sv);
                        return ILOC_MEMORY_ALLOCATION_ERROR;
                    }
                    ths = iLoc_SVDthreshold(mp, mp, sv);
/*
 *                  get effective rank that explains
 *                  pct percent of total variance
 */
                    for (esum = 0., m = 0; m < mp; m++) {
                        if (sv[m] <= ths) break;
                            esum += sv[m];
                    }
                    nr = m;
                    for (psum = 0., i = 0; i < nr; i++) {
                        psum += sv[i] / esum;
                        if (psum > pct) break;
                    }
                    m = ILOC_MIN(i, nr - 1);
                    ths = sv[m];
/*
 *                  projection matrix:
 *                      W(N x N) = (1 / sqrt(SV) * transpose(U)
 *
 *                  a zero rowsum in W indicates the projection of perfectly
 *                  correlated observations to the null space
 */
                    for (knull = 0, m = 0; m < mp; m++) {
                        ii = PhaDef->indTime[m+k];
                        sum = 0.;
                        for (i = 0; i < mp; i++) {
                            jj = PhaDef->indTime[i+k];
                            x = 0.;
                            if (sv[m] >= ths) {
                                x = u[i][m] / ILOC_SQRT(sv[m]);
                                if (fabs(x) < ILOC_ZEROTOL) x = 0.;
                                sum += x;
                            }
                            w[ii][jj] = x;
                        }
                        if (fabs(sum) < 1.e-5) knull++;
                    }
                    iLoc_FreeFloatMatrix(u);
                    iLoc_Free(avec); iLoc_Free(sv);
                }
                k += mp - 1;
            }
            iLoc_FreeFloatMatrix(z);
        }
    }
/*
 *  Azimuth
 *      number of azimuth observations for this phase
 *
 */
    np = PhaDef->nAzim;
    if (np) {
/*
 *      only a single azimuth observation for this phase
 */
        if (np == 1) {

            ii = PhaDef->indAzim[0];
            w[ii][ii] = 0.;
            if (cov[ii][ii] > ILOC_ZEROTOL)
                w[ii][ii] = 1. / ILOC_SQRT(cov[ii][ii]);
        }
/*
 *      multiple azimuth observations
 */
        else {
            if ((z = iLoc_AllocateFloatMatrix(np, np)) == NULL) {
                fprintf(stderr, "Wmatrix: cannot allocate memory\n");
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
/*
 *          build covariance block for this phase
 */
            for (k = 0; k < np; k++) {
                ii = PhaDef->indAzim[k];
                z[k][k] = cov[ii][ii];
                for (m = 0; m < k; m++) {
                    jj = PhaDef->indAzim[m];
                    z[k][m] = cov[ii][jj];
                    z[m][k] = cov[jj][ii];
                }
            }
/*
 *          find diagonal sub-blocks in the covariance block for this phase
 */
            for (k = 0; k < np; k++) {
                mp = np - 1;
                while (z[k][mp] < ILOC_ZEROTOL) mp--;
/*
 *              only a single observation in this sub-block
 */
                if (mp == k) {
                    ii = PhaDef->indAzim[k];
                    w[ii][ii] = 0.;
                    if (z[k][k] > ILOC_ZEROTOL)
                        w[ii][ii] = 1. / ILOC_SQRT(z[k][k]);
                    continue;
                }
/*
 *              multiple observations in this sub-block
 */
                else {
                    for (i = k + 1; i < mp; i++) {
                        m = np - 1;
                        while (z[i][m] < ILOC_ZEROTOL) m--;
                        if (m > mp) mp = m;
                    }
                    mp = mp - k + 1;
                    u = iLoc_AllocateFloatMatrix(mp, mp);
                    avec = (double *)calloc(mp * mp, sizeof(double));
                    if ((sv = (double *)calloc(mp, sizeof(double))) == NULL) {
                        fprintf(stderr, "Wmatrix: cannot allocate memory\n");
                        iLoc_FreeFloatMatrix(u);
                        iLoc_FreeFloatMatrix(z);
                        iLoc_Free(avec);
                        return ILOC_MEMORY_ALLOCATION_ERROR;
                    }
/*
 *                  copy the covariance matrix of this sub-block into avec
 */
                    for (ii = 0, i = k; ii < mp; ii++, i++) {
                        for (m = 0, jj = k; m < mp; m++, jj++) {
                            avec[ii + m * mp] = z[i][jj];
                        }
                    }
/*
 *                  Eigenvalue decomposition
 */
                    if (EigenDecompose(mp, avec, u, sv)) {
                        iLoc_FreeFloatMatrix(u);
                        iLoc_FreeFloatMatrix(z);
                        iLoc_Free(avec);
                        iLoc_Free(sv);
                        return ILOC_MEMORY_ALLOCATION_ERROR;
                    }
/*
 *                  get effective rank that explains
 *                  pct percent of total variance
 */
                    for (esum = 0., m = 0; m < mp; m++) {
                        if (sv[m] <= ths) break;
                            esum += sv[m];
                    }
                    nr = m;
                    for (psum = 0., i = 0; i < nr; i++) {
                        psum += sv[i] / esum;
                        if (psum > pct) break;
                    }
                    m = ILOC_MIN(i, nr - 1);
                    ths = sv[m];
/*
 *                  projection matrix:
 *                      W(N x N) = (1 / sqrt(SV) * transpose(U)
 *
 *                  a zero rowsum in W indicates the projection of perfectly
 *                  correlated observations to the null space
 */
                    for (knull = 0, m = 0; m < mp; m++) {
                        ii = PhaDef->indAzim[m+k];
                        sum = 0.;
                        for (i = 0; i < mp; i++) {
                            jj = PhaDef->indAzim[i+k];
                            x = 0.;
                            if (sv[m] >= ths) {
                                x = u[i][m] / ILOC_SQRT(sv[m]);
                                if (fabs(x) < ILOC_ZEROTOL) x = 0.;
                                sum += x;
                            }
                            w[ii][jj] = x;
                        }
                        if (fabs(sum) < 1.e-5) knull++;
                    }
                    iLoc_FreeFloatMatrix(u);
                    iLoc_Free(avec); iLoc_Free(sv);
                }
                k += mp - 1;
            }
            iLoc_FreeFloatMatrix(z);
        }
    }
/*
 *  Slowness
 *      number of slowness observations for this phase
 *
 */
    np = PhaDef->nSlow;
    if (np) {
/*
 *      only a single slowness observation for this phase
 */
        if (np == 1) {

            ii = PhaDef->indSlow[0];
            w[ii][ii] = 0.;
            if (cov[ii][ii] > ILOC_ZEROTOL)
                w[ii][ii] = 1. / ILOC_SQRT(cov[ii][ii]);
        }
/*
 *      multiple slowness observations
 */
        else {
            if ((z = iLoc_AllocateFloatMatrix(np, np)) == NULL) {
                fprintf(stderr, "Wmatrix: cannot allocate memory\n");
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
/*
 *          build covariance block for this phase
 */
            for (k = 0; k < np; k++) {
                ii = PhaDef->indSlow[k];
                z[k][k] = cov[ii][ii];
                for (m = 0; m < k; m++) {
                    jj = PhaDef->indSlow[m];
                    z[k][m] = cov[ii][jj];
                    z[m][k] = cov[jj][ii];
                }
            }
/*
 *          find diagonal sub-blocks in the covariance block for this phase
 */
            for (k = 0; k < np; k++) {
                mp = np - 1;
                while (z[k][mp] < ILOC_ZEROTOL) mp--;
/*
 *              only a single observation in this sub-block
 */
                if (mp == k) {
                    ii = PhaDef->indSlow[k];
                    w[ii][ii] = 0.;
                    if (z[k][k] > ILOC_ZEROTOL)
                        w[ii][ii] = 1. / ILOC_SQRT(z[k][k]);
                    continue;
                }
/*
 *              multiple observations in this sub-block
 */
                else {
                    for (i = k + 1; i < mp; i++) {
                        m = np - 1;
                        while (z[i][m] < ILOC_ZEROTOL) m--;
                        if (m > mp) mp = m;
                    }
                    mp = mp - k + 1;
                    u = iLoc_AllocateFloatMatrix(mp, mp);
                    avec = (double *)calloc(mp * mp, sizeof(double));
                    if ((sv = (double *)calloc(mp, sizeof(double))) == NULL) {
                        fprintf(stderr, "Wmatrix: cannot allocate memory\n");
                        iLoc_FreeFloatMatrix(u);
                        iLoc_FreeFloatMatrix(z);
                        iLoc_Free(avec);
                        return ILOC_MEMORY_ALLOCATION_ERROR;
                    }
/*
 *                  copy the covariance matrix of this sub-block into avec
 */
                    for (ii = 0, i = k; ii < mp; ii++, i++) {
                        for (m = 0, jj = k; m < mp; m++, jj++) {
                            avec[ii + m * mp] = z[i][jj];
                        }
                    }
/*
 *                  Eigenvalue decomposition
 */
                    if (EigenDecompose(mp, avec, u, sv)) {
                        iLoc_FreeFloatMatrix(u);
                        iLoc_FreeFloatMatrix(z);
                        iLoc_Free(avec);
                        iLoc_Free(sv);
                        return ILOC_MEMORY_ALLOCATION_ERROR;
                    }
                    ths = iLoc_SVDthreshold(mp, mp, sv);
/*
 *                  get effective rank that explains
 *                  pct percent of total variance
 */
                    for (esum = 0., m = 0; m < mp; m++) {
                        if (sv[m] <= ths) break;
                            esum += sv[m];
                    }
                    nr = m;
                    for (psum = 0., i = 0; i < nr; i++) {
                        psum += sv[i] / esum;
                        if (psum > pct) break;
                    }
                    m = ILOC_MIN(i, nr - 1);
                    ths = sv[m];
/*
 *                  projection matrix:
 *                      W(N x N) = (1 / sqrt(SV) * transpose(U)
 *
 *                  a zero rowsum in W indicates the projection of perfectly
 *                  correlated observations to the null space
 */
                    for (knull = 0, m = 0; m < mp; m++) {
                        ii = PhaDef->indSlow[m+k];
                        sum = 0.;
                        for (i = 0; i < mp; i++) {
                            jj = PhaDef->indSlow[i+k];
                            x = 0.;
                            if (sv[m] >= ths) {
                                x = u[i][m] / ILOC_SQRT(sv[m]);
                                if (fabs(x) < ILOC_ZEROTOL) x = 0.;
                                sum += x;
                            }
                            w[ii][jj] = x;
                        }
                        if (fabs(sum) < 1.e-5) knull++;
                    }
                    iLoc_FreeFloatMatrix(u);
                    iLoc_Free(avec); iLoc_Free(sv);
                }
                k += mp - 1;
            }
            iLoc_FreeFloatMatrix(z);
        }
    }
    return ILOC_SUCCESS;
}

/*
 * Calculate the eigenvalues and eigenvectors of an NxN symmetric matrix A.
 *        A = U * SV * transpose(U)
 *    The A matrix has to be in Fortran vector format (column order).
 *    Uses the Lapack dsyevr routine to obtain the eigenvalue decomposition
 *    of the symmetric, positive semi-definite covariance matrix.
 *    Input arguments:
 *       nd   - number of data
 *       avec - A matrix in Fortran vector format
 *    Output arguments:
 *       u    - eigenvector matrix (N x N)
 *       sv   - eigenvalue vector in descending order
 *    Returns:
 *       Success/error
 *    Called by:
 *       Wmatrix
 *    Calls:
 *       dlamch, dsyevr_
 */
static int EigenDecompose(int nd, double *avec, double **u, double *sv)
{
    double *work = (double *)NULL;
    double *uvec = (double *)NULL;
    int *isuppz = (int *)NULL;
    int *iwork = (int *)NULL;
    int n = nd, lda = nd, ldz = nd, m = nd, il = 0, iu = 0;
    int info, lwork = -1, liwork = -1, iwkopt, i, j;
    double abstol = dlamch('S');
    double vl = 0., vu = 0., wkopt;
/*
 *  allocate memory
 */
    uvec = (double *)calloc(n * n, sizeof(double));
    if ((isuppz = (int *)calloc(2 * n, sizeof(int))) == NULL) {
        fprintf(stderr, "EigenDecompose: cannot allocate memory\n");
        iLoc_Free(uvec);
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  query and allocate the optimal workspace
 */
    dsyevr_("Vectors", "All", "Upper", &n, avec, &lda, &vl, &vu, &il, &iu,
            &abstol, &m, sv, uvec, &ldz, isuppz, &wkopt, &lwork, &iwkopt,
            &liwork, &info);
    lwork = (int)wkopt;
    liwork = iwkopt;
    work = (double *)calloc(lwork, sizeof(double));
    if ((iwork = (int *)calloc(liwork, sizeof(int))) == NULL) {
        fprintf(stderr, "EigenDecompose: cannot allocate memory\n");
        iLoc_Free(isuppz); iLoc_Free(uvec); iLoc_Free(work);
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  eigenvalue decomposition
 */
    dsyevr_("Vectors", "All", "Upper", &n, avec, &lda, &vl, &vu, &il, &iu,
            &abstol, &m, sv, uvec, &ldz, isuppz, work, &lwork, iwork,
            &liwork, &info);
    if (info) {
        fprintf(stderr, "EigenDecompose: failed to compute eigenvalues\n");
        iLoc_Free(isuppz); iLoc_Free(uvec); iLoc_Free(work); iLoc_Free(iwork);
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  sort eigenvalues/eigenvectors in descending order
 */
    for (i = 0; i < nd; i++)
        for (j = 0; j < nd; j++)
            u[i][nd - j - 1] = uvec[i + j * nd];
    for (i = 0; i < nd; i++) avec[i] = sv[nd - i - 1];
    for (i = 0; i < nd; i++) sv[i] = avec[i];
/*
 *  free memory
 */
    iLoc_Free(isuppz); iLoc_Free(uvec); iLoc_Free(work); iLoc_Free(iwork);
    return ILOC_SUCCESS;
}

/*
 * Wrapper for Lapack machine precision function
 */
static double dlamch(char CMACH)
{
  return dlamch_(&CMACH);
}

/*
 *  Title:
 *     GetPhaDef
 *  Synopsis:
 *     populates ILOC_PHADEF structure for defining phases
 *     ILOC_PHADEF contains:
 *          phase name
 *          number of observations for this phase
 *          permutation vector that renders the data covariance matrix
 *              block-diagonal (phase by phase)
 *     returns number of distinct defining phases
 *  Input Arguments:
 *     numPhase  - number of associated phases
 *     Assocs    - array of ILOC_ASSOC structures
 *     numPhaDef - PhaDef dimension
 *     PhaDef    - array of ILOC_PHADEF structures
 *  Output Arguments:
 *     PhaDef    - array of ILOC_PHADEF structures
 *  Return:
 *     number of defining phases in PhaDef
 *  Called by:
 *     iLoc_ProjectionMatrix
 */
static int GetPhaDef(int numPhase, ILOC_ASSOC *Assocs, int numPhaDef,
        ILOC_PHADEF *PhaDef)
{
    int i, j, k, nphases = 0, isfound = 0;
    for (j = 0; j < numPhaDef; j++) {
        strcpy(PhaDef[j].Phase, "");
        PhaDef[j].nTime = 0;
        PhaDef[j].nAzim = 0;
        PhaDef[j].nSlow = 0;
        PhaDef[j].indTime = (int *)NULL;
        PhaDef[j].indAzim = (int *)NULL;
        PhaDef[j].indSlow = (int *)NULL;
    }
/*
 *  get number of defining observations for each phase
 */
    for (i = 0; i < numPhase; i++) {
        if (Assocs[i].Timedef) {
/*
 *          find phase in PhaDef
 */
            isfound = 0;
            for (j = 0; j < nphases; j++) {
                if (ILOC_STREQ(Assocs[i].Phase, PhaDef[j].Phase)) {
                    PhaDef[j].nTime++;
                    isfound = 1;
                    break;
                }
            }
/*
 *          new phase; add it to PhaDef
 */
            if (!isfound) {
                strcpy(PhaDef[j].Phase, Assocs[i].Phase);
                PhaDef[j].nTime = 1;
                nphases++;
            }
        }
    }
    for (i = 0; i < numPhase; i++) {
        if (Assocs[i].Azimdef) {
/*
 *          find phase in PhaDef
 */
            isfound = 0;
            for (j = 0; j < nphases; j++) {
                if (ILOC_STREQ(Assocs[i].Phase, PhaDef[j].Phase)) {
                    PhaDef[j].nAzim++;
                    isfound = 1;
                    break;
                }
            }
/*
 *          new phase; add it to PhaDef
 */
            if (!isfound) {
                strcpy(PhaDef[j].Phase, Assocs[i].Phase);
                PhaDef[j].nAzim = 1;
                nphases++;
            }
        }
    }
    for (i = 0; i < numPhase; i++) {
        if (Assocs[i].Slowdef) {
/*
 *          find phase in PhaDef
 */
            isfound = 0;
            for (j = 0; j < nphases; j++) {
                if (ILOC_STREQ(Assocs[i].Phase, PhaDef[j].Phase)) {
                    PhaDef[j].nSlow++;
                    isfound = 1;
                    break;
                }
            }
/*
 *          new phase; add it to PhaDef
 */
            if (!isfound) {
                strcpy(PhaDef[j].Phase, Assocs[i].Phase);
                PhaDef[j].nSlow = 1;
                nphases++;
            }
        }
    }
/*
 *  allocate memory
 */
    for (j = 0; j < nphases; j++) {
        if (PhaDef[j].nSlow)
            PhaDef[j].indSlow = (int *)calloc(PhaDef[j].nSlow, sizeof(int));
        if (PhaDef[j].nAzim) {
            if ((PhaDef[j].indAzim = (int *)calloc(PhaDef[j].nAzim, sizeof(int))) == NULL) {
                FreePhaDef(nphases, PhaDef);
                fprintf(stderr, "GetPhaDef: cannot allocate memory\n");
                return 0;
            }
        }
        if (PhaDef[j].nTime) {
            if ((PhaDef[j].indTime = (int *)calloc(PhaDef[j].nTime, sizeof(int))) == NULL) {
                FreePhaDef(nphases, PhaDef);
                fprintf(stderr, "GetPhaDef: cannot allocate memory\n");
                return 0;
            }
        }
    }
/*
 *  build permutation vectors
 */
    for (j = 0; j < nphases; j++) {
        for (k = 0, i = 0; i < numPhase; i++) {
            if (Assocs[i].Timedef) {
                if (ILOC_STREQ(Assocs[i].Phase, PhaDef[j].Phase)) {
                    PhaDef[j].indTime[k] = Assocs[i].CovIndTime;
                    k++;
                }
            }
        }
        for (k = 0, i = 0; i < numPhase; i++) {
            if (Assocs[i].Azimdef) {
                if (ILOC_STREQ(Assocs[i].Phase, PhaDef[j].Phase)) {
                    PhaDef[j].indAzim[k] = Assocs[i].CovIndAzim;
                    k++;
                }
            }
        }
        for (k = 0, i = 0; i < numPhase; i++) {
            if (Assocs[i].Slowdef) {
                if (ILOC_STREQ(Assocs[i].Phase, PhaDef[j].Phase)) {
                    PhaDef[j].indSlow[k] = Assocs[i].CovIndSlow;
                    k++;
                }
            }
        }
    }
    return nphases;
}

/*
 *  Title:
 *     FreePhaDef
 *  Synopsis:
 *     frees memory allocated in ILOC_PHADEF structure
 *  Input Arguments:
 *     nphases - number of distinct defining phases
 *     PhaDef  - array of ILOC_PHADEF structures
 *  Called by:
 *     iLoc_ProjectionMatrix
 */
static void FreePhaDef(int nphases, ILOC_PHADEF *PhaDef)
{
    int j;
    for (j = 0; j < nphases; j++) {
        strcpy(PhaDef[j].Phase, "");
        if (PhaDef[j].nTime) iLoc_Free(PhaDef[j].indTime);
        if (PhaDef[j].nAzim) iLoc_Free(PhaDef[j].indAzim);
        if (PhaDef[j].nSlow) iLoc_Free(PhaDef[j].indSlow);
        PhaDef[j].nTime = PhaDef[j].nAzim = PhaDef[j].nSlow = 0;
    }
}
