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
 *    iLoc_GetDistanceMatrix
 *    iLoc_GetDataCovarianceMatrix
 */

/*
 *  Title:
 *     iLoc_GetDistanceMatrix
 *  Synopsis:
 *     Calculates station separations in km
 *  Input Arguments:
 *     numSta  - number of distinct stations
 *     StaLocs - array of ILOC_STA structures
 *  Return:
 *     distmatrix
 *  Called by:
 *     iLoc_Locator
 *  Calls:
 *     iLoc_AllocateFloatMatrix, iLoc_DistAzimuth
 */
double **iLoc_GetDistanceMatrix(int numSta, ILOC_STA *StaLocs)
{
    double **distmatrix = (double **)NULL;
    int i, j;
    double d = 0., esaz = 0., seaz = 0.;
/*
 *  memory allocation
 */
    if ((distmatrix = iLoc_AllocateFloatMatrix(numSta, numSta)) == NULL) {
        fprintf(stderr, "iLoc_GetDistanceMatrix: cannot allocate memory\n");
        return (double **)NULL;
    }
/*
 *  populate distmatrix; station separations in km
 */
    for (i = 0; i < numSta; i++) {
        distmatrix[i][i] = 0.;
        for (j = i + 1; j < numSta; j++) {
            d = ILOC_DEG2KM *
                iLoc_DistAzimuth(StaLocs[j].StaLat, StaLocs[j].StaLon,
                                 StaLocs[i].StaLat, StaLocs[i].StaLon,
                                 &seaz, &esaz);
            distmatrix[i][j] = distmatrix[j][i] = d;
        }
    }
    return distmatrix;
}

/*
 *  Title:
 *     iLoc_GetDataCovarianceMatrix
 *  Synopsis:
 *     Constructs full data covariance matrix from variogram (model errors)
 *     and prior phase variances (measurement errors)
 *  Input Arguments:
 *     nsta       - number of distinct stations
 *     numPhase   - number of associated phases
 *     nd         - number of defining phases
 *     Assocs     - array of ILOC_ASSOC structures
 *     StaLocs    - array of ILOC_STA structures
 *     distmatrix - matrix of station separations
 *     variogram  - pointer to ILOC_VARIOGRAM stucture
 *  Return:
 *     data covariance matrix
 *  Called by:
 *     LocateEvent, iLoc_NASearch
 *  Calls:
 *     iLoc_AllocateFloatMatrix, iLoc_FreeFloatMatrix, GetStationIndex,
 *     iLoc_SplineInterpolation
 */
double **iLoc_GetDataCovarianceMatrix(int nsta, int numPhase, int nd,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, double **distmatrix,
        ILOC_VARIOGRAM *variogram, int verbose)
{
    int i, j, k, m, sind1 = 0, sind2 = 0;
    double stasep = 0., var = 0., dydx = 0., d2ydx = 0.;
    double **dcov = (double **)NULL;
/*
 *  allocate memory for dcov
 */
    if ((dcov = iLoc_AllocateFloatMatrix(nd, nd)) == NULL) {
        fprintf(stderr, "iLoc_GetDataCovarianceMatrix: cannot allocate memory\n");
        return (double **)NULL;
    }
/*
 *  construct data covariance matrix from variogram and prior measurement
 *  error variances
 */
    for (k = 0, i = 0; i < numPhase; i++) {
/*
 *      arrival time
 */
        if (Assocs[i].Timedef) {
            sind1 = Assocs[i].StaInd;
/*
 *          prior picking error variances add to the diagonal
 */
            dcov[k][k] = variogram->sill + Assocs[i].Deltim * Assocs[i].Deltim;
            Assocs[i].CovIndTime = k;
/*
 *          covariances
 */
            for (m = k + 1, j = i + 1; j < numPhase; j++) {
                if (!Assocs[j].Timedef) continue;
/*
 *              different phases have different ray paths so they do not correlate
 */
                if (strcmp(Assocs[i].Phase, Assocs[j].Phase)) {
                    m++;
                    continue;
                }
                sind2 = Assocs[j].StaInd;
/*
 *              station separation
 */
                var = 0.;
                stasep = distmatrix[sind1][sind2];
                if (stasep < variogram->maxsep) {
/*
 *                  interpolate variogram
 */
                    var = iLoc_SplineInterpolation(stasep, variogram->n,
                                variogram->x, variogram->y, variogram->d2y,
                                0, &dydx, &d2ydx);
/*
 *                  covariance: sill - variogram
 */
                    var = variogram->sill - var;
                }
                dcov[k][m] = var;
                dcov[m][k] = var;
                m++;
            }
            k++;
        }
    }
    for (i = 0; i < numPhase; i++) {
/*
 *      azimuth
 */
        if (Assocs[i].Azimdef) {
            sind1 = Assocs[i].StaInd;
/*
 *          prior picking error variances add to the diagonal
 */
            dcov[k][k] = variogram->sill + Assocs[i].Delaz * Assocs[i].Delaz;
            Assocs[i].CovIndAzim = k;
/*
 *          covariances
 */
            for (m = k + 1, j = i + 1; j < numPhase; j++) {
                if (!Assocs[j].Azimdef) continue;
/*
 *              different phases have different ray paths so they do not correlate
 */
                if (strcmp(Assocs[i].Phase, Assocs[j].Phase)) {
                    m++;
                    continue;
                }
                sind2 = Assocs[j].StaInd;
/*
 *              station separation
 */
                var = 0.;
                stasep = distmatrix[sind1][sind2];
                if (stasep < variogram->maxsep) {
/*
 *                  interpolate variogram
 */
                    var = iLoc_SplineInterpolation(stasep, variogram->n,
                                variogram->x, variogram->y, variogram->d2y,
                                0, &dydx, &d2ydx);
/*
 *                  covariance: sill - variogram
 */
                    var = variogram->sill - var;
                }
                dcov[k][m] = var;
                dcov[m][k] = var;
                m++;
            }
            k++;
        }
    }
    for (i = 0; i < numPhase; i++) {
/*
 *      slowness
 */
        if (Assocs[i].Slowdef) {
            sind1 = Assocs[i].StaInd;
/*
 *          prior picking error variances add to the diagonal
 */
            dcov[k][k] = variogram->sill + Assocs[i].Delslo * Assocs[i].Delslo;
            Assocs[i].CovIndSlow = k;
/*
 *          covariances
 */
            for (m = k + 1, j = i + 1; j < numPhase; j++) {
                if (!Assocs[j].Slowdef) continue;
/*
 *              different phases have different ray paths so they do not correlate
 */
                if (strcmp(Assocs[i].Phase, Assocs[j].Phase)) {
                    m++;
                    continue;
                }
                sind2 = Assocs[j].StaInd;
/*
 *              station separation
 */
                var = 0.;
                stasep = distmatrix[sind1][sind2];
                if (stasep < variogram->maxsep) {
/*
 *                  interpolate variogram
 */
                    var = iLoc_SplineInterpolation(stasep, variogram->n,
                                variogram->x, variogram->y, variogram->d2y,
                                0, &dydx, &d2ydx);
/*
 *                  covariance: sill - variogram
 */
                    var = variogram->sill - var;
                }
                dcov[k][m] = var;
                dcov[m][k] = var;
                m++;
            }
            k++;
        }
    }
    if (verbose > 2) {
        fprintf(stderr, "        Data covariance matrix C(%d x %d):\n", nd, nd);
        for (k = 0, i = 0; i < numPhase; i++) {
            if (Assocs[i].Timedef) {
                fprintf(stderr, "          %4d %4d %4d %-8s T ",
                        i, k, Assocs[i].StaInd, Assocs[i].Phase);
                for (j = 0; j < nd; j++) fprintf(stderr, "%6.3f ", dcov[k][j]);
                fprintf(stderr, "\n");
                k++;
            }
        }
        for (i = 0; i < numPhase; i++) {
            if (Assocs[i].Azimdef) {
                fprintf(stderr, "          %4d %4d %4d %-8s A ",
                        i, k, Assocs[i].StaInd, Assocs[i].Phase);
                for (j = 0; j < nd; j++) fprintf(stderr, "%6.3f ", dcov[k][j]);
                fprintf(stderr, "\n");
                k++;
            }
        }
        for (i = 0; i < numPhase; i++) {
            if (Assocs[i].Slowdef) {
                fprintf(stderr, "          %4d %4d %4d %-8s S ",
                        i, k, Assocs[i].StaInd, Assocs[i].Phase);
                for (j = 0; j < nd; j++) fprintf(stderr, "%6.3f ", dcov[k][j]);
                fprintf(stderr, "\n");
                k++;
            }
        }
    }
    return dcov;
}

