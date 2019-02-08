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
 *    iLoc_Locator
 *    iLoc_Readings
 */

/*
 * Local functions
 *    ResidualsForFixedHypocenter
 *    LocateEvent
 *    GetNdef
 *    GetResiduals
 *    BuildGd
 *    ProjectGd
 *    WeightGd
 *    ConvergenceTestValue
 *    ConvergenceTest
 *    WxG
 *    Uncertainties
 *    Ftest
 */
static void ResidualsForFixedHypocenter(ILOC_CONF *iLocConfig,
        ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs, ILOC_STA *StaLocs,
        ILOC_READING *rdindx, ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo);
static int LocateEvent(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, double **distmatrix,
        ILOC_VARIOGRAM *variogram, ILOC_PHADEF *PhaDef,
        ILOC_STAORDER *staorder, int is2nderiv, int has_depdpres, int isfixed);
static int GetNdef(int numPhase, ILOC_ASSOC *Assocs, double *toffset);
static int GetResiduals(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int iszderiv,
        int is2nderiv, int *has_depdpres, int *ischanged, int iter,
        int ispchange, int *nunp, ILOC_PHASELIST *phundef, double **dcov,
        double **w);
static double BuildGd(int ndef, ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs,
        int fixdepthfornow, double **g, double *d, int verbose);
static int ProjectGd(int ndef, int m, double **g, double *d, double **w,
        double *dnorm, double *wrms, int verbose);
static int WxG(int j, int ndef, double **w, double **g);
static void WeightGd(int ndef, int m, int numPhase, ILOC_ASSOC *Assocs,
        double **g, double *d, double *dnorm, double *wrms);
static double ConvergenceTestValue(double gtdnorm, double gnorm, double dnorm);
static int ConvergenceTest(ILOC_CONF *iLocConfig, int iter, int m, int *nds,
        double *sol,  double *oldsol, double wrms, double *modelnorm,
        double *convgtest, double *oldcvgtst, double *step, int *isdiv);
static void Uncertainties(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs);
static double Ftest(int m, int n);

/*
 *  Title:
 *     iLoc_Locator
 *  Synopsis:
 *     Prepares for iterative linearised least-squares inversion
 *        sets starting hypocentre
 *           if hypocenter is fixed then just calculate residuals and return
 *        calculates station separations and nearest-neighbour station order
 *        performs phase identification w.r.t. starting hypocentre
 *        performs NA grid search to get initial guess for linearised inversion
 *        reidentifies phases according to best NA hypocentre
 *        tests for depth resolution; fixes depth if necessary
 *        locates event
 *        if convergent solution is obtained
 *           discards free-depth solution if depth error is too large
 *              fixes depth and start over again
 *           performs depth-phase stack if there is depth-phase depth resolution
 *           calculates residuals for all reported phases
 *           calculates location quality metrics
 *        else
 *           calculates residuals for all reported phases
 *        reports results in Hypocenter and Assocs structures
 *  Input arguments:
 *     iLocConfig    - pointer to ILOC_CONF structure
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     fe            - pointer to ILOC_FE structure
 *     DefaultDepth  - pointer to ILOC_DEFAULTDEPTH structure
 *     Variogram     - pointer to ILOC_VARIOGRAM structure
 *     ec            - array of ILOC_EC_COEF structures
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - array of ILOC_TT_TABLE structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - array of ILOC_TT_TABLE structures
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *     StaLocs       - array of ILOC_STA structures
 *  Output arguments:
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *  Return:
 *     Success/error
 *  Called by:
 *     SeisComp3 iLoc app
 *  Calls:
 *     iLoc_InitializeEvent, iLoc_Readings, ResidualsForFixedHypocenter,
 *     iLoc_Free, iLoc_GetDistanceMatrix, iLoc_HierarchicalCluster,
 *     iLoc_FreeFloatMatrix, iLoc_GetDefaultDepth, iLoc_GetDeltaAzimuth,
 *     iLoc_EpochToHuman, iLoc_IdentifyPhases, iLoc_SetNASearchSpace,
 *     iLoc_NASearch, iLoc_ReIdentifyPhases, iLoc_TravelTimeResiduals,
 *     iLoc_GetNumDef, iLoc_DepthPhaseCheck, iLoc_DepthResolution, LocateEvent,
 *     iLoc_DepthPhaseStack, iLoc_DistAzimuth, iLoc_LocationQuality,
 *     iLoc_GregionNumber, iLoc_Gregion, iLoc_PrintPhases, iLoc_PrintSolution
 */
int iLoc_Locator(ILOC_CONF *iLocConfig, ILOC_PHASEIDINFO *PhaseIdInfo,
        ILOC_FE *fe, ILOC_DEFAULTDEPTH *DefaultDepth, ILOC_VARIOGRAM *variogram,
        ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables,
        ILOC_TTINFO *LocalTTInfo, ILOC_TT_TABLE *LocalTTtables,
        ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs, ILOC_STA *StaLocs)
{
    ILOC_HYPO grds;
    ILOC_NASPACE nasp;              /* Neighbourhood algorithm search space */
    double **distmatrix = (double **)NULL;               /* distance matrix */
    ILOC_STAORDER *staorder = (ILOC_STAORDER *)NULL;    /* NN station order */
    ILOC_READING *rdindx = (ILOC_READING *)NULL;                /* Readings */
    ILOC_PHADEF *PhaDef = (ILOC_PHADEF *)NULL;
    char timestr[25], gregname[255];
    int iszderiv = 0, is2nderiv = 1, isgridsearch;
    int isfixed, firstpass = 1, grn = 0, retval = ILOC_UNKNOWN_ERROR;
    int hasDepthResolution = 0, has_depdpres = 0, isdefdep = 0;
    double mediandepth, medianot, medianlat, medianlon, epidist, x, y;
    isgridsearch = iLocConfig->DoGridSearch;
/*
 *  print input structures
 */
    if (iLocConfig->Verbose > 1)
        iLoc_PrintIOstructures(iLocConfig, Hypocenter, Assocs, StaLocs, 1);
/*
 *  Initializations
 *     Initializes output parameters in Hypocenter and Assoc structures.
 *     Fixes the hypocenter if all hypocenter parameters are fixed,
 *         or the number of associations is less than the number of unknowns.
 *     For anthropogenic events fixes the depth at the surface,
 *         unless the user fixed it explicitly to some depth
 *     Calculates Delta, Esaz and Seaz for each phase.
 *     Sorts phase structures by increasing Delta, StaInd, ArrivalTime
 *     Groups Assocs into readings
 */
    iLoc_InitializeEvent(iLocConfig, Hypocenter, Assocs, StaLocs);
/*
 *  save initial hypocenter
 */
    medianot = Hypocenter->Time;
    medianlat = Hypocenter->Lat;
    medianlon = Hypocenter->Lon;
    mediandepth = Hypocenter->Depth;
/*
 *  depth derivatives are only needed when depth is a free parameter
 */
    if (!Hypocenter->FixDepth)
        iszderiv = 1;
/*
 *  Allocate memory for rdindx and populate structure
 */
    if ((rdindx = (ILOC_READING *)calloc(Hypocenter->numReading,
                                         sizeof(ILOC_READING))) == NULL) {
        fprintf(stderr, "Locator: cannot allocate memory for rdindx!\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
    iLoc_Readings(Hypocenter->numPhase, Hypocenter->numReading, Assocs, rdindx);
/*
 *
 *  If hypocenter is fixed then just calculate residuals
 *
 */
    if (Hypocenter->FixHypo) {
        if (iLocConfig->Verbose)
            fprintf(stderr, "Calculate residuals for fixed hypocentre\n");
        ResidualsForFixedHypocenter(iLocConfig, Hypocenter, Assocs, StaLocs,
                rdindx, PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo,
                LocalTTtables, DefaultDepth->Topo);
        iLoc_Free(rdindx);
/*
 *      print output structures
 */
        if (iLocConfig->Verbose)
            iLoc_PrintIOstructures(iLocConfig, Hypocenter, Assocs, StaLocs, 0);
        return ILOC_SUCCESS;
    }
/*
 *
 *  Correlated errors
 *     Allocate memory for nearest-neighbor station order and distmatrix
 *
 */
    if (iLocConfig->DoCorrelatedErrors) {
        PhaDef = (ILOC_PHADEF *)calloc(TTInfo->numPhaseTT, sizeof(ILOC_PHADEF));
        staorder = (ILOC_STAORDER *)calloc(Hypocenter->numSta, sizeof(ILOC_STAORDER));
        if (staorder == NULL) {
            fprintf(stderr, "staorder: cannot allocate memory\n");
            iLoc_Free(PhaDef);
            iLoc_Free(rdindx);
            return ILOC_MEMORY_ALLOCATION_ERROR;
        }
/*
 *      calculate station separations
 */
        distmatrix = iLoc_GetDistanceMatrix(Hypocenter->numSta, StaLocs);
        if (distmatrix == NULL) {
            fprintf(stderr, "cannot get distmatrix!\n");
            iLoc_Free(staorder);
            iLoc_Free(rdindx);
            return ILOC_MEMORY_ALLOCATION_ERROR;
        }
/*
 *      nearest-neighbour station order
 */
        if (iLoc_HierarchicalCluster(Hypocenter->numSta, distmatrix, staorder)) {
            fprintf(stderr, "iLoc_HierarchicalCluster failed!\n");
            iLoc_Free(staorder);
            iLoc_FreeFloatMatrix(distmatrix);
            iLoc_Free(rdindx);
            return ILOC_MEMORY_ALLOCATION_ERROR;
        }
    }
/*
 *
 *  Try first for a free depth solution; fix the depth if it fails
 *
 */
    for (isfixed = 0; isfixed < 2; isfixed++) {
        if (isfixed == 0) {
/*
 *          do not attempt a free depth solution if depth is already fixed
 */
            if (Hypocenter->FixDepth)
                continue;
        }
        else {
            if (!Hypocenter->FixDepth) {
/*
 *              either no depth resolution or the locator did not converge
 *                  fix depth to region-dependent default depth
 */
                Hypocenter->Depth = iLoc_GetDefaultDepth(Hypocenter, DefaultDepth,
                                        fe, &isdefdep, iLocConfig->Verbose);
/*
 *              if there is a large depth difference, fall back to the
 *                  initial hypocentre and recalcaluate the default depth
 */
                if (fabs((Hypocenter->Depth - mediandepth)) > 20.) {
                    fprintf(stderr, "Large depth difference, ");
                    fprintf(stderr, "fall back to median hypocentre\n");
                    Hypocenter->Time = medianot;
                    Hypocenter->Lat = medianlat;
                    Hypocenter->Lon = medianlon;
                    Hypocenter->Depth = iLoc_GetDefaultDepth(Hypocenter,
                                             DefaultDepth, fe, &isdefdep,
                                             iLocConfig->Verbose);
                }
                iszderiv = 1;
                Hypocenter->FixDepth = 1;
                Hypocenter->numUnknowns--;
/*
 *              adjust origin time according to depth change if OT is not fixed
 */
                if (!Hypocenter->FixOT)
                    Hypocenter->Time += (Hypocenter->Depth - mediandepth) / 10.;
/*
 *              if there is still a large depth difference, do NA again
 */
                if (fabs((Hypocenter->Depth - mediandepth)) > 20.)
                    firstpass = 1;
/*
 *              delta, esaz and seaz for each phase
 */
                iLoc_GetDeltaAzimuth(Hypocenter, Assocs, StaLocs);
            }
        }
        iLoc_EpochToHuman(timestr, Hypocenter->Time);
        if (iLocConfig->Verbose) {
            fprintf(stderr, "Initial hypocentre:\n");
            fprintf(stderr, "  OT = %s Lat = %7.3f Lon = %8.3f Depth = %.1f\n",
                timestr, Hypocenter->Lat, Hypocenter->Lon, Hypocenter->Depth);
        }
/*
 *      phase identification
 */
        iLoc_IdentifyPhases(iLocConfig, Hypocenter, Assocs, StaLocs, rdindx,
                       PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo,
                       LocalTTtables, DefaultDepth->Topo, &is2nderiv);
        if (!Hypocenter->FixOT) {
/*
 *          Check if we have any time defining phases left
 */
            if (Hypocenter->numTimedef < Hypocenter->numUnknowns) {
                fprintf(stderr, "%d time defining phase left, fixing origin time!\n",
                        Hypocenter->numTimedef);
                Hypocenter->FixOT = 1;
                Hypocenter->numUnknowns--;
            }
        }
/*
 *
 *      Neighbourhood algorithm search to get initial hypocentre guess
 *          - may be executed only once
 *          - search in 4D (lat, lon, OT, depth)
 *          - reidentify phases w.r.t. each trial hypocentre
 *          - ignore correlated error structure for the sake of speed
 */
        if (isgridsearch && firstpass) {
            memmove(&grds, Hypocenter, sizeof(ILOC_HYPO));
/*
 *          set up search space for NA
 */
            if (iLoc_SetNASearchSpace(iLocConfig, &grds, &nasp,
                                 TTInfo->MaxHypocenterDepth)) {
                fprintf(stderr, "    WARNING: iLoc_SetNASearchSpace failed!\n");
            }
            else {
/*
 *              Neighbourhood algorithm
 */
                retval = iLoc_NASearch(iLocConfig, &grds, Assocs, StaLocs,
                             PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo,
                             LocalTTtables, DefaultDepth->Topo, distmatrix,
                             variogram, staorder, PhaDef, &nasp, is2nderiv);
                if (retval) {
                    fprintf(stderr, "    WARNING: iLoc_NASearch failed with error %d!\n",
                            retval);
                    memmove(&grds, Hypocenter, sizeof(ILOC_HYPO));
                }
                else {
/*
 *                  store the best hypo from NA in the Hypocenter structure
 */
                    Hypocenter->Time = grds.Time;
                    Hypocenter->Lat = grds.Lat;
                    Hypocenter->Lon = grds.Lon;
                    Hypocenter->Depth = grds.Depth;
                    iLoc_EpochToHuman(timestr, Hypocenter->Time);
                    epidist = ILOC_DEG2KM *
                              iLoc_DistAzimuth(medianlat, medianlon,
                                   Hypocenter->Lat, Hypocenter->Lon, &x, &y);
                    if (iLocConfig->Verbose) {
                        fprintf(stderr, "Best fitting hypocentre from NA search:\n");
                        fprintf(stderr, "  OT = %s Lat = %7.3f Lon = %8.3f ",
                            timestr, Hypocenter->Lat, Hypocenter->Lon);
                        fprintf(stderr, "Depth = %.1f\n", Hypocenter->Depth);
                        fprintf(stderr, "Distance from initial guess = %.1f km\n",
                                epidist);
                        fprintf(stderr, "Reidentify phases after NA\n");
                    }
/*
 *                  reidentify phases w.r.t. best hypocentre
 */
                    iLoc_GetDeltaAzimuth(Hypocenter, Assocs, StaLocs);
                    iLoc_ReIdentifyPhases(iLocConfig, Hypocenter, Assocs, StaLocs,
                                rdindx, PhaseIdInfo, ec, TTInfo, TTtables,
                                LocalTTInfo, LocalTTtables, DefaultDepth->Topo,
                                is2nderiv, 1);
                    if (iLocConfig->Verbose > 1) {
                        fprintf(stderr, "numTimedef=%d numAzimdef=%d numSlowdef=%d\n",
                                Hypocenter->numTimedef, Hypocenter->numAzimdef,
                                Hypocenter->numSlowdef);
                        iLoc_PrintPhases(Hypocenter->numPhase, Assocs);
                    }
                }
            }
        }
/*
 *      disable further grid searches
 */
        firstpass = 0;
/*
 *      set ttime, residual, dtdh, and dtdd for each phase
 */
        if (iLoc_TravelTimeResiduals(iLocConfig, Hypocenter, Assocs, StaLocs,
                        ec, TTInfo, TTtables, LocalTTInfo, LocalTTtables,
                        DefaultDepth->Topo, PhaseIdInfo, 0, iszderiv, is2nderiv))
            continue;
/*
 *      number of initial defining observations
 */
        iLoc_GetNumDef(Hypocenter, Assocs);
        if (Hypocenter->numDef < Hypocenter->numUnknowns) {
            fprintf(stderr, "Insufficient number (%d) of phases left\n",
                    Hypocenter->numDef);
            continue;
        }
        if (isfixed == 0) {
/*
 *          pointless to try free-depth solution with just a few phases
 */
            if (Hypocenter->numDef <= Hypocenter->numUnknowns + 1) {
                fprintf(stderr, "Not enough phases for free-depth solution!\n");
                if (fabs(Hypocenter->Depth - mediandepth) > 20.)
                    firstpass = 1;
                continue;
            }
/*
 *          depth-phase depth resolution
 *              ndepassoc >= MinDepthPhases
 *              also flag first arriving defining P for a reading
 */
            has_depdpres = iLoc_DepthPhaseCheck(iLocConfig, Hypocenter, Assocs,
                                                rdindx);
/*
 *          recount number of defining observations as iLoc_DepthPhaseCheck
 *              may make depth phases non-defining
 */
            iLoc_GetNumDef(Hypocenter, Assocs);
            if (Hypocenter->numDef < Hypocenter->numUnknowns) {
                fprintf(stderr, "Insufficient number (%d) of phases left\n",
                        Hypocenter->numDef);
                continue;
            }
/*
 *          depth resolution
 *              (has_depdpres || nlocal >= MinLocalStations ||
 *               nsdef >= MinSPpairs || ncoredef >= MinCorePhases)
 */
            hasDepthResolution = iLoc_DepthResolution(iLocConfig, Hypocenter,
                                                      Assocs, rdindx);
/*
 *          pointless to try free-depth solution without depth resolution
 */
            if (!hasDepthResolution) {
                fprintf(stderr, "No depth resolution for free-depth solution!\n");
                if (fabs(Hypocenter->Depth - mediandepth) > 20.)
                    firstpass = 1;
                continue;
            }
        }
        if (!Hypocenter->FixOT) {
/*
 *          Check again if we have any time defining phases left
 */
            if (Hypocenter->numTimedef < Hypocenter->numUnknowns) {
                fprintf(stderr, "%d time defining phase left, fixing origin time!\n",
                        Hypocenter->numTimedef);
                Hypocenter->Time = medianot;
                Hypocenter->FixOT = 1;
                Hypocenter->numUnknowns--;
                firstpass = 1;
                isfixed = 0;
                continue;
            }
        }
/*
 *
 *      locate event
 *
 */
        retval = LocateEvent(iLocConfig, Hypocenter, Assocs, StaLocs, rdindx,
                             PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo,
                             LocalTTtables, DefaultDepth->Topo, distmatrix,
                             variogram, PhaDef, staorder, is2nderiv,
                             has_depdpres, isfixed);
        if (retval) {
/*
 *          divergent solution
 */
            if (isgridsearch) {
/*
 *              if grid search was enabled:
 *                  disable it, reinitialize and give it one more try
 */
                isgridsearch = 0;
                firstpass = 1;
                isfixed = 0;
            }
            else if (isfixed == 0)
/*
 *              reinitialize solution for the fixed depth option
 */
                firstpass = 1;
            else {
/*
 *              give up
 */
                fprintf(stderr, "Locator failed!\n");
            }
            continue;
        }
/*
 *
 *      converged?
 *
 */
        if (Hypocenter->Converged) {
/*
 *          Discard free-depth solution if depth error is too large
 */
            if (isfixed == 0 &&
                ((Hypocenter->Depth > 0. &&
                  Hypocenter->Depth <= TTInfo->Moho &&
                  Hypocenter->Errors[3] > iLocConfig->MaxShallowDepthError) ||
                 (Hypocenter->Depth >  TTInfo->Moho &&
                  Hypocenter->Errors[3] > iLocConfig->MaxDeepDepthError))) {
                fprintf(stderr, "Discard free-depth solution due to large errors!\n");
                fprintf(stderr, "     depth = %5.1f depth error = %.1f\n",
                        Hypocenter->Depth, Hypocenter->Errors[3]);
                firstpass = 1;
                continue;
            }
/*
 *          We're done.
 */
            else
                break;
        }
    }
/*
 *  End of isfixed loop
 */
/*
 *  Free memory
 */
    if (iLocConfig->DoCorrelatedErrors) {
        iLoc_FreeFloatMatrix(distmatrix);
        iLoc_Free(staorder);
        iLoc_Free(PhaDef);
    }
/*
 *
 *  Convergence: wrap up
 *
 */
    if (Hypocenter->Converged) {
/*
 *      Calculate depth-phase depth if possible
 */
        Hypocenter->DepthDp = Hypocenter->DepthDpError = ILOC_NULLVAL;
        has_depdpres = iLoc_DepthPhaseCheck(iLocConfig, Hypocenter, Assocs, rdindx);
        if (has_depdpres)
            iLoc_DepthPhaseStack(iLocConfig, Hypocenter, Assocs, TTInfo, TTtables,
                                 DefaultDepth->Topo);
/*
 *      Free memory
 */
        iLoc_Free(rdindx);
/*
 *      Calculate residuals for all phases
 */
        iLoc_TravelTimeResiduals(iLocConfig, Hypocenter, Assocs, StaLocs, ec,
                                 TTInfo, TTtables, LocalTTInfo, LocalTTtables,
                                 DefaultDepth->Topo, PhaseIdInfo, 1, 0, 0);
/*
 *      Calculate location quality metrics
 */
        iLoc_LocationQuality(Hypocenter, Assocs);
        if (iLocConfig->Verbose) {
/*
 *          Print final solution, phases and residuals to log file.
 */
            grn = iLoc_GregionNumber(Hypocenter->Lat, Hypocenter->Lon, fe);
            iLoc_Gregion(grn, gregname);
            iLoc_PrintPhases(Hypocenter->numPhase, Assocs);
            iLoc_PrintSolution(Hypocenter, grn);
            fprintf(stderr, "nsta=%d ndefsta=%d nreading=%d nass=%d ",
                    Hypocenter->numSta, Hypocenter->numDefsta,
                    Hypocenter->numReading, Hypocenter->numPhase);
            fprintf(stderr, "ndef=%d (T=%d A=%d S=%d) nrank=%d\n",
                    Hypocenter->numDef, Hypocenter->numTimedef,
                    Hypocenter->numAzimdef, Hypocenter->numSlowdef,
                    Hypocenter->numRank);
            fprintf(stderr, "sgap=%5.1f ", Hypocenter->Sgap);
            if (Hypocenter->semiMajax != ILOC_NULLVAL)
                fprintf(stderr, "smajax=%.1f sminax=%.1f strike=%.1f",
                        Hypocenter->semiMajax, Hypocenter->semiMinax,
                        Hypocenter->Strike);
            if (Hypocenter->Errors[0] != ILOC_NULLVAL)
                fprintf(stderr, " stime=%.3f", Hypocenter->Errors[0]);
            if (Hypocenter->Errors[3] != ILOC_NULLVAL)
                fprintf(stderr, " sdepth=%.1f", Hypocenter->Errors[3]);
            if (Hypocenter->SdevObs != ILOC_NULLVAL)
                fprintf(stderr, " sdobs=%.3f", Hypocenter->SdevObs);
            fprintf(stderr, "\n");
            if (Hypocenter->DepthDp != ILOC_NULLVAL)
                fprintf(stderr, "depdp=%.2f +/- %.2f ndp=%d\n",
                        Hypocenter->DepthDp, Hypocenter->DepthDpError,
                        Hypocenter->numDepthDp);
            if (Hypocenter->FixOT || Hypocenter->FixLat ||
                Hypocenter->FixLon || Hypocenter->FixDepth) {
                fprintf(stderr, "Fixed: ");
                if (Hypocenter->FixOT) fprintf(stderr, "OT ");
                if (Hypocenter->FixLat) fprintf(stderr, "Lat ");
                if (Hypocenter->FixLon) fprintf(stderr, "Lon ");
                if (Hypocenter->FixDepth) fprintf(stderr, "Depth");
                fprintf(stderr, "\n");
            }
            if (Hypocenter->FixedDepthType == 8)
                fprintf(stderr, "depth fixed by user\n");
            else if (Hypocenter->FixedDepthType == 1)
                fprintf(stderr, "airquake/deepquake, depth fixed to surface/MaxHypocenterDepth\n");
            else if (Hypocenter->FixedDepthType == 4)
                fprintf(stderr, "anthropogenic event, depth fixed to surface\n");
            else if (Hypocenter->FixedDepthType == 5)
            fprintf(stderr, "depth fixed to default depth grid depth\n");
            else if (Hypocenter->FixedDepthType == 6) {
                if (!isdefdep) {
                    fprintf(stderr, "no default depth grid point exists, ");
                    fprintf(stderr, "depth fixed to median reported depth\n");
                }
                else {
                    fprintf(stderr, "depth fixed to median reported depth\n");
                }
            }
            else if (Hypocenter->FixedDepthType == 7) {
                fprintf(stderr, "no default depth grid point exists, ");
                fprintf(stderr, "depth fixed to GRN-dependent depth\n");
            }
            else
                fprintf(stderr, "free-depth solution\n");
            fprintf(stderr, "GT5cand=%d (dU=%5.3f sgap=%5.1f numStaWithin10km=%d)\n",
                    Hypocenter->GT5candidate, Hypocenter->localDU,
                    Hypocenter->localSgap, Hypocenter->numStaWithin10km);
        }
/*
 *      print output structures
 */
        if (iLocConfig->Verbose > 1)
            iLoc_PrintIOstructures(iLocConfig, Hypocenter, Assocs, StaLocs, 0);
        return ILOC_SUCCESS;
    }
    else
        return retval;
}

/*
 *  Title:
 *     iLoc_Readings
 *  Synopsis:
 *     records starting index and number of phases in a reading
 *  Input Arguments:
 *     numPhase   - number of associated phases
 *     numReading - number of Readings
 *     Assocs     - array of ILOC_ASSOC structures
 *  Output Arguments:
 *     rdindx     - array of ILOC_READING structures
 *  Called by:
 *     iLoc_Locator, LocateEvent, iLoc_NASearch
 */
void iLoc_Readings(int numPhase, int numReading, ILOC_ASSOC *Assocs,
        ILOC_READING *rdindx)
{
    int i, j = 0, rdid;
    for (i = 0; i < numReading; i++) {
        rdindx[i].start = j;
        rdindx[i].npha = 0;
        rdid = Assocs[j].rdid;
        for (; j < numPhase; j++) {
            if (Assocs[j].rdid != rdid)
                break;
            rdindx[i].npha++;
        }
    }
}

/*
 *  Title:
 *     ResidualsForFixedHypocenter
 *  Synopsis:
 *     Calculate residuals w.r.t. a fixed hypocentre.
 *  Input Arguments:
 *     iLocConfig    - configuration parameter ILOC_CONF structure
 *     Hypocenter    - ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *     StaLocs       - array of ILOC_STA structures
 *     rdindx        - array of ILOC_READING structures
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     ec            - pointer to ILOC_EC_COEF structures
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - pointer to ILOC_TT_TABLE structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - pointer to ILOC_TT_TABLE structures
 *     topo          - ETOPO bathymetry/elevation matrix
 *  Output Arguments:
 *     Assocs        - array of ILOC_ASSOC structures
 *  Called by:
 *     iLoc_Locator
 *  Calls:
 *     iLoc_IdentifyPhases, iLoc_TravelTimeResiduals, iLoc_PrintPhases
 */
static void ResidualsForFixedHypocenter(ILOC_CONF *iLocConfig,
        ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs, ILOC_STA *StaLocs,
        ILOC_READING *rdindx, ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo)
{
    int is2nderiv = 0;
    Hypocenter->numUnknowns = 0;
/*
 *  Identify phases (also gets residuals)
 */
    iLoc_IdentifyPhases(iLocConfig, Hypocenter, Assocs, StaLocs, rdindx,
                PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo, LocalTTtables,
                topo, &is2nderiv);
/*
 *  Print phases and residuals to log file.
 */
    if (iLocConfig->Verbose)
        iLoc_PrintPhases(Hypocenter->numPhase, Assocs);
}

/*
 *  Title:
 *     LocateEvent
 *  Synopsis:
 *     Iterative linearised least-squares inversion of travel-times to
 *         obtain a solution for the hypocentre.
 *     Bondár, I., and K. McLaughlin, 2009,
 *        Seismic location bias and uncertainty in the presence of correlated
 *        and non-Gaussian travel-time errors,
 *        Bull. Seism. Soc. Am., 99, 172-193.
 *     Bondár, I., and D. Storchak, 2011,
 *        Improved location procedures at the International Seismological
 *        Centre,
 *        Geophys. J. Int., doi: 10.1111/j.1365-246X.2011.05107.x.
 *
 *     If DoCorrelatedErrors is true, it projects Gm = d into the
 *         eigensystem defined by the full data covariance matrix,
 *         i.e. in the system where the full data covariance matrix
 *         becomes diagonal
 *     otherwise assumes independent errors and weights Gm = d with
 *         the a priori estimates of measurement error variances.
 *     WGm = Wd is solved with singular value decomposition.
 *     Damping is applied if condition number is large.
 *     Convergence test is based on the Paige-Saunder convergence test value
 *         and the history of model and data norms.
 *     Formal uncertainties obtained from the model covariance matrix
 *         are scaled to 90% confidence level.
 *     Free-depth solutions:
 *         Depth is fixed in the first MinIterations-1 iterations
 *         Depth remains fixed if number of airquakes/deepquakes exceeds 2
 *         Phases are reidentified if depth crosses Moho/Conrad discontinuities
 *     Correlated errors:
 *         The data covariance and projection matrices are calculated once
 *         The data covariance matrix is recalculated if defining phases were
 *             renamed during an iteration
 *         The projection matrix is recalculated if defining phases were
 *            renamed or defining phases were made non-defining during an
 *            iteration
 *  Input arguments:
 *     iLocConfig    - pointer to ILOC_CONF structure
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *     StaLocs       - array of ILOC_STA structures
 *     rdindx        - array of ILOC_READING structures
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     ec            - array of ILOC_EC_COEF structures
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - array of ILOC_TT_TABLE structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - array of ILOC_TT_TABLE structures
 *     topo          - ETOPO bathymetry/elevation matrix
 *     distmatrix    - station separation matrix
 *     variogram     - pointer to ILOC_VARIOGRAM structure
 *     PhaDef        - array of ILOC_PHADEF structures
 *     staorder      - array of ILOC_STAORDER structures (nearest-neighbour order)
 *     is2nderiv     - calculate second derivatives [0/1]]
 *     has_depdpres  - do we have depth resolution by depth phases?
 *     isfixed       - fixed depth?
 *  Output arguments:
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_Locator
 *  Calls:
 *     GetNdef, iLoc_SortAssocsNN, iLoc_Readings, iLoc_DepthPhaseCheck,
 *     iLoc_GetDeltaAzimuth, iLoc_ReIdentifyPhases, GetResiduals,
 *     iLoc_GetDataCovarianceMatrix, iLoc_ProjectionMatrix,
 *     iLoc_AllocateFloatMatrix, iLoc_FreeFloatMatrix, iLoc_Free,
 *     iLoc_GetNumDef, BuildGd, ProjectGd, WeightGd, iLoc_SVDdecompose,
 *     iLoc_SVDthreshold, iLoc_SVDrank, iLoc_SVDnorm, iLoc_SVDsolve,
 *     ConvergenceTestValue, ConvergenceTest, iLoc_PointAtDeltaAzimuth,
 *     iLoc_PrintSolution, iLoc_PrintDefiningPhases, iLoc_SortAssocs,
 *     iLoc_SVDModelCovarianceMatrix, Uncertainties
 */
static int LocateEvent(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, double **distmatrix,
        ILOC_VARIOGRAM *variogram, ILOC_PHADEF *PhaDef,
        ILOC_STAORDER *staorder, int is2nderiv, int has_depdpres, int isfixed)
{
    int i, j, k, m, iter, nds[3];
    int retval = ILOC_UNKNOWN_ERROR, isconv = 0, isdiv = 0;
    int iszderiv = 0, fixdepthfornow = 0, nairquakes = 0, ndeepquakes = 0;
    int nrank = 0, dpok = 0, ndef = 0, nd = 0, nr = 0, nunp = 0;
    int ischanged = 0, ispchange = 0;
    ILOC_PHASELIST *phundef = (ILOC_PHASELIST *)NULL;
    double **g = (double **)NULL;
    double *d = (double *)NULL;
    double *sv = (double *)NULL;
    double svundamped[4];
    double **v = (double **)NULL;
    double **dcov = (double **)NULL;
    double **w = (double **)NULL;
    double mcov[4][4], sol[4], oldsol[4], modelnorm[3], convgtest[3];
    double toffset = 0., torg = 0., delta = 0., azim = 0.;
    double svth = 0., damp = 0., dmax = 0., step = 0., prevDepth = 0.;
    double urms = 0., wrms = 0., scale = 0.;
    double gtd = 0., gtdnorm = 0., dnorm = 0., gnorm = 0., mnorm = 0.;
    double cnvgtst = 0., oldcvgtst = 0., cond = 0., x = 0., y = 0.;
    dpok = has_depdpres;
    prevDepth = Hypocenter->Depth;
/*
 *  get number of defining observations and earliest arrival time
 */
    nd = GetNdef(Hypocenter->numPhase, Assocs, &toffset);
    nrank = nd;
    if (nd <= Hypocenter->numUnknowns) {
        if (iLocConfig->Verbose)
            fprintf(stderr, "LocateEvent: insufficient number of phases (%d)!\n",
                    nd);
        return ILOC_INSUFFICIENT_NUMBER_OF_PHASES;
    }
/*
 *  reduced origin time
 */
    torg = Hypocenter->Time - toffset;
/*
 *  initializations
 */
    for (i = 0; i < 4; i++) {
        sol[i] = 0.;
        oldsol[i] = 0.;
    }
    i = 0;
    if (!Hypocenter->FixOT)    sol[i++] = torg;
    if (!Hypocenter->FixLon)   sol[i++] = Hypocenter->Lon;
    if (!Hypocenter->FixLat)   sol[i++] = Hypocenter->Lat;
    if (!Hypocenter->FixDepth) sol[i]   = Hypocenter->Depth;
    nairquakes = ndeepquakes = 0;
    ischanged = ispchange = isconv = isdiv = 0;
    step = oldcvgtst = 1.;
    for (i = 0; i < 4; i++) {
        Hypocenter->Errors[i] = Hypocenter->ModelCov[i][i] = ILOC_NULLVAL;
        for (j = i + 1; j < 4; j++)
            Hypocenter->ModelCov[i][j] = Hypocenter->ModelCov[j][i] = ILOC_NULLVAL;
    }
/*
 *  convergence history
 */
    for (j = 0; j < 3; j++) {
        modelnorm[j] = 0.;
        convgtest[j] = 0.;
        nds[j] = 0;
    }
    nds[0] = nd;
/*
 *  allocate memory
 */
    phundef = (ILOC_PHASELIST *)calloc(TTInfo->numPhaseTT, sizeof(ILOC_PHASELIST));
    if (phundef == NULL) {
        fprintf(stderr, "LocateEvent: cannot allocate memory\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  reorder phaserecs by staorder, rdid, time so that covariance matrices
 *  for various phases will become block-diagonal
 */
    if (iLocConfig->DoCorrelatedErrors) {
        iLoc_SortAssocsNN(Hypocenter->numPhase, Hypocenter->numSta, Assocs,
                          StaLocs, staorder);
        iLoc_Readings(Hypocenter->numPhase, Hypocenter->numReading, Assocs, rdindx);
        dpok = iLoc_DepthPhaseCheck(iLocConfig, Hypocenter, Assocs, rdindx);
    }
/*
 *
 * iteration loop
 *
 */
    for (iter = 0; iter < iLocConfig->MaxIterations; iter++) {
/*
 *      number of model parameters
 */
        m = Hypocenter->numUnknowns;
        iszderiv = 1;
/*
 *      check if necessary to fix depth if free depth solution
 */
        if (isfixed == 0) {
            fixdepthfornow = 0;
/*
 *          fix depth for the first MinIterations - 1 iterations
 */
            if (iter < iLocConfig->MinIterations - 1) {
                fixdepthfornow = 1;
                m--;
            }
/*
 *          do not allow airquakes
 */
            else if (Hypocenter->Depth < 0.) {
                if (iLocConfig->Verbose > 2)
                    fprintf(stderr, "    airquake, fixing depth to 0\n");
                nairquakes++;
                Hypocenter->Depth = 0.;
                fixdepthfornow = 1;
                m--;
            }
/*
 *          do not allow deepquakes
 */
            else if (Hypocenter->Depth > TTInfo->MaxHypocenterDepth) {
                if (iLocConfig->Verbose > 2)
                    fprintf(stderr, "    deepquake, fixing depth to max depth\n");
                ndeepquakes++;
                Hypocenter->Depth = TTInfo->MaxHypocenterDepth;
                fixdepthfornow = 1;
                m--;
            }
        }
/*
 *      various fix depth instructions
 */
        else
            fixdepthfornow = 1;
/*
 *      enough of airquakes!
 */
        if (nairquakes > 2 || ndeepquakes > 2) {
            fixdepthfornow = 1;
            m = Hypocenter->numUnknowns - 1;
            Hypocenter->FixedDepthType = 1;
        }
/*
 *      no need for z derivatives when depth is fixed
 */
        if (fixdepthfornow)
            iszderiv = 0;
/*
 *      set delta, esaz and seaz for each phase
 */
        iLoc_GetDeltaAzimuth(Hypocenter, Assocs, StaLocs);
/*
 *      Reidentify phases iff depth crosses Moho or Conrad
 */
        if (iLocConfig->UseLocalTT) {
            if ((Hypocenter->Depth > LocalTTInfo->Moho &&
                        prevDepth <= LocalTTInfo->Moho) ||
                (Hypocenter->Depth < LocalTTInfo->Moho &&
                        prevDepth >= LocalTTInfo->Moho) ||
                (Hypocenter->Depth > LocalTTInfo->Conrad &&
                        prevDepth <= LocalTTInfo->Conrad) ||
                (Hypocenter->Depth < LocalTTInfo->Conrad &&
                        prevDepth >= LocalTTInfo->Conrad)) {
                ispchange = iLoc_ReIdentifyPhases(iLocConfig, Hypocenter, Assocs,
                                    StaLocs, rdindx, PhaseIdInfo, ec, TTInfo,
                                    TTtables, LocalTTInfo, LocalTTtables,
                                    topo, is2nderiv, 0);
            }
        }
        else {
            if ((Hypocenter->Depth > TTInfo->Moho &&
                        prevDepth <= TTInfo->Moho) ||
                (Hypocenter->Depth < TTInfo->Moho &&
                        prevDepth >= TTInfo->Moho) ||
                (Hypocenter->Depth > TTInfo->Conrad &&
                        prevDepth <= TTInfo->Conrad) ||
                (Hypocenter->Depth < TTInfo->Conrad &&
                        prevDepth >= TTInfo->Conrad)) {
                ispchange = iLoc_ReIdentifyPhases(iLocConfig, Hypocenter, Assocs,
                                    StaLocs, rdindx, PhaseIdInfo, ec, TTInfo,
                                    TTtables, LocalTTInfo, LocalTTtables,
                                    topo, is2nderiv, 0);
            }
        }
/*
 *      get residuals w.r.t. current solution
 */
        retval = GetResiduals(iLocConfig, Hypocenter, Assocs, StaLocs, rdindx,
                        PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo,
                        LocalTTtables, topo, iszderiv, is2nderiv, &dpok,
                        &ischanged, iter, ispchange, &nunp, phundef, dcov, w);
        if (retval)
            break;
        ndef = Hypocenter->numDef;
        if (!Hypocenter->FixOT) {
/*
 *          Check if we have any time defining phases left
 */
            if (Hypocenter->numTimedef < Hypocenter->numUnknowns) {
                if (iLocConfig->Verbose)
                    fprintf(stderr, "%d time defining phase left, fixing origin time!\n",
                            Hypocenter->numTimedef);
                Hypocenter->FixOT = 1;
                Hypocenter->numUnknowns--;
                m--;
            }
        }
        if (ndef <= Hypocenter->numUnknowns) {
            if (iLocConfig->Verbose)
                fprintf(stderr, "Insufficient number (%d) of phases left; ",
                        ndef);
            retval = ILOC_INSUFFICIENT_NUMBER_OF_PHASES;
            break;
        }
        for (j = 2; j > 0; j--) nds[j] = nds[j-1];
        nds[0] = ndef;
/*
 *      initial memory allocations
 */
        if (iter == 0) {
            nd = ndef;
            nrank = ndef;
            g = iLoc_AllocateFloatMatrix(nd, 4);
            v = iLoc_AllocateFloatMatrix(4, 4);
            d = (double *)calloc(nd, sizeof(double));
            if ((sv = (double *)calloc(4, sizeof(double))) == NULL) {
                fprintf(stderr, "LocateEvent: cannot allocate memory!\n");
                retval = ILOC_MEMORY_ALLOCATION_ERROR;
                break;
            }
/*
 *          account for correlated error structure
 */
            if (iLocConfig->DoCorrelatedErrors) {
/*
 *              construct data covariance matrix
 */
                if ((dcov = iLoc_GetDataCovarianceMatrix(Hypocenter->numSta,
                                         Hypocenter->numPhase, nd, Assocs,
                                         StaLocs, distmatrix, variogram,
                                         iLocConfig->Verbose)) == NULL) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
/*
 *              projection matrix
 */
                if ((w = iLoc_AllocateFloatMatrix(nd, nd)) == NULL) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
                if (iLoc_ProjectionMatrix(TTInfo->numPhaseTT, PhaDef,
                                     Hypocenter->numPhase, Assocs, nd, 95.,
                                     dcov, w, &nrank, nunp, phundef, 1,
                                     iLocConfig->Verbose)) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
            }
        }
/*
 *      rest of the iterations:
 *          check if set of time-defining phases or their names were changed
 */
        else if (ispchange || ndef > nd) {
/*
 *          change in defining phase names or increase in defining phases:
 *              reallocate memory for G and d
 */
            if (nd != ndef) {
                iLoc_FreeFloatMatrix(g);
                iLoc_Free(d);
                isconv = 0;
                nd = ndef;
                g = iLoc_AllocateFloatMatrix(nd, 4);
                if ((d = (double *)calloc(nd, sizeof(double))) == NULL) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
            }
            if (iLocConfig->DoCorrelatedErrors) {
/*
 *              recalculate the data covariance and projection matrices
 */
                if (iLocConfig->Verbose > 2) {
                    fprintf(stderr, "    Changes in defining phasenames, ");
                    fprintf(stderr, "recalculating projection matrix\n");
                }
                iLoc_FreeFloatMatrix(dcov);
                iLoc_FreeFloatMatrix(w);
                if ((dcov = iLoc_GetDataCovarianceMatrix(Hypocenter->numSta,
                                         Hypocenter->numPhase, nd, Assocs,
                                         StaLocs, distmatrix, variogram,
                                         iLocConfig->Verbose)) == NULL) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
/*
 *              projection matrix
 */
                if ((w = iLoc_AllocateFloatMatrix(nd, nd)) == NULL) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
                if (iLoc_ProjectionMatrix(TTInfo->numPhaseTT, PhaDef,
                                    Hypocenter->numPhase, Assocs, nd, 95.,
                                    dcov, w, &nrank, nunp, phundef, 1,
                                    iLocConfig->Verbose)) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
            }
            else
                nrank = ndef;
        }
        else if (ischanged) {
/*
 *          change in number of defining phases:
 *              recalculate the projection matrix for phases that were changed
 */
            isconv = 0;
            nd = ndef;
            if (iLocConfig->DoCorrelatedErrors) {
                if (iLocConfig->Verbose > 2) {
                    fprintf(stderr, "    Changes in defining phasenames, ");
                    fprintf(stderr, "recalculating projection matrix\n");
                }
/*
 *              readjust row index pointers to covariance matrix
 */
                for (k = 0, i = 0; i < Hypocenter->numPhase; i++) {
                    if (Assocs[i].Timedef) {
                        Assocs[i].CovIndTime = k;
                        k++;
                   }
                }
                for (i = 0; i < Hypocenter->numPhase; i++) {
                    if (Assocs[i].Azimdef) {
                        Assocs[i].CovIndAzim = k;
                        k++;
                    }
                }
                for (i = 0; i < Hypocenter->numPhase; i++) {
                    if (Assocs[i].Slowdef) {
                        Assocs[i].CovIndSlow = k;
                        k++;
                    }
                }
/*
 *              Check if we have any time defining phases left
 */
                iLoc_GetNumDef(Hypocenter, Assocs);
                if (!Hypocenter->FixOT) {
/*
 *                  Check if we have any time defining phases left
 */
                    if (Hypocenter->numTimedef < Hypocenter->numUnknowns) {
                        if (iLocConfig->Verbose)
                            fprintf(stderr, "%d time defining phase left, fixing origin time!\n",
                                    Hypocenter->numTimedef);
                        Hypocenter->FixOT = 1;
                        Hypocenter->numUnknowns--;
                        m--;
                    }
                }
                if (iLoc_ProjectionMatrix(TTInfo->numPhaseTT, PhaDef,
                                    Hypocenter->numPhase, Assocs, nd, 95.,
                                    dcov, w, &nrank, nunp, phundef, ispchange,
                                    iLocConfig->Verbose)) {
                    retval = ILOC_MEMORY_ALLOCATION_ERROR;
                    break;
                }
            }
            else
                nrank = ndef;
        }
        Hypocenter->numRank = nrank;
        if (nrank < Hypocenter->numUnknowns) {
            fprintf(stderr, "Insufficient number of independent phases (%d, %d)!\n",
                    nrank, Hypocenter->numUnknowns);
            retval = ILOC_INSUFFICIENT_NUMBER_OF_INDEPENDENT_PHASES;
            break;
        }
/*
 *      build G matrix and d vector
 */
        urms = BuildGd(nd, Hypocenter, Assocs, fixdepthfornow, g, d,
                       iLocConfig->Verbose);
        if (iLocConfig->DoCorrelatedErrors) {
/*
 *          project Gm = d into eigensystem
 */
            if (ProjectGd(nd, m, g, d, w, &dnorm, &wrms, iLocConfig->Verbose)) {
                retval = ILOC_MEMORY_ALLOCATION_ERROR;
                break;
            }
        }
        else {
/*
 *          independent observations: weight Gm = d by measurement errors
 */
            WeightGd(nd, m, Hypocenter->numPhase, Assocs, g, d, &dnorm, &wrms);
        }
/*
 *      finish if convergent or divergent solution
 *          for the last iteration we only need urms and wrms
 */
        if (isconv || isdiv)
            break;
/*
 *      transpose(G) * d matrix norm
 */
        gtdnorm = 0.;
        for (i = 0; i < nd; i++) {
            gtd = 0.0;
            for (j = 0; j < m; j++)
                gtd += g[i][j] * d[i];
            gtdnorm += gtd * gtd;
        }
/*
 *      SVD of G (G is overwritten by U matrix!)
 */
        if (iLoc_SVDdecompose(nd, m, g, sv, v)) {
            retval = ILOC_MEMORY_ALLOCATION_ERROR;
            break;
        }
        for (j = 0; j < m; j++) svundamped[j] = sv[j];
/*
 *      condition number, G matrix norm, rank and convergence test value
 */
        svth = iLoc_SVDthreshold(nd, m, sv);
        nr = iLoc_SVDrank(nd, m, sv, svth);
        gnorm = iLoc_SVDnorm(m, sv, svth, &cond);
        if (nr < m) {
            fprintf(stderr, "Singular G matrix (%d < %d)!\n", nr, m);
            retval = ILOC_SINGULAR_MATRIX;
            break;
        }
        if (cond > 30000.) {
            fprintf(stderr, "Abnormally ill-conditioned problem (cond=%.0f)!\n",
                    cond);
            retval = ILOC_ABNORMALLY_ILL_CONDITIONED_PROBLEM;
            break;
        }
        cnvgtst = ConvergenceTestValue(gtdnorm, gnorm, dnorm);
/*
 *      If damping is enabled, apply damping if condition number is large.
 *      Apply of 1% largest singular value for moderately ill-conditioned,
 *               5% for more severely ill-conditioned and
 *              10% for highly ill-conditioned problems.
 */
        if (iLocConfig->AllowDamping && cond > 30.) {
            damp = 0.01;
            if (cond > 300.)  damp = 0.05;
            if (cond > 3000.) damp = 0.1;
            for (j = 1; j < nr; j++)
                sv[j] += sv[0] * damp;
            if (iLocConfig->Verbose > 2) {
                fprintf(stderr, "    Large condition number (%.3f): ", cond);
                fprintf(stderr, "%.0f%% damping is applied.\n", 100. * damp);
            }
        }
/*
 *      solve Gm = d
 */
        if (iLoc_SVDsolve(nd, m, g, sv, v, d, sol, svth)) {
            retval = ILOC_MEMORY_ALLOCATION_ERROR;
            break;
        }
/*
 *      model norm
 */
        for (mnorm = 0., j = 0; j < m; j++)
            mnorm += sol[j] * sol[j];
        mnorm = ILOC_SQRT(mnorm);
/*
 *      scale down hypocenter perturbations if they are very large
 */
        dmax = 1000.;
        if (mnorm > dmax) {
            scale = dmax / mnorm;
            for (j = 0; j < m; j++)
                sol[j] *= scale;
            mnorm = dmax;
            if (iLocConfig->Verbose > 2) {
                fprintf(stderr, "    Large perturbation: ");
                fprintf(stderr, "%.g scaling is applied.\n", scale);
            }
        }
/*
 *      convergence test
 */
        for (j = 2; j > 0; j--) {
            modelnorm[j] = modelnorm[j-1];
            convgtest[j] = convgtest[j-1];
        }
        modelnorm[0] = mnorm;
        convgtest[0] = cnvgtst;
        if (iter > iLocConfig->MinIterations - 1)
            isconv = ConvergenceTest(iLocConfig, iter, m, nds, sol, oldsol,
                        wrms, modelnorm, convgtest, &oldcvgtst, &step, &isdiv);
/*
 *      update hypocentre coordinates
 */
        prevDepth = Hypocenter->Depth;
        if (iLocConfig->Verbose > 1) {
            fprintf(stderr, "iteration = %d: ", iter);
            if     (isconv) fprintf(stderr, "  converged!\n");
            else if (isdiv) fprintf(stderr, "  diverged!\n");
            else            fprintf(stderr, "\n");
            fprintf(stderr, "  ||Gt*d|| = %.5f ||G|| = %.5f ", gtdnorm, gnorm);
            fprintf(stderr, "||d|| = %.5f ||m|| = %.5f\n", dnorm, mnorm);
            fprintf(stderr, "  convgtst = %.5f condition number = %.3f\n",
                    cnvgtst, cond);
            fprintf(stderr, "  eigenvalues: ");
            for (i = 0; i < m; i++) fprintf(stderr, "%g ", sv[i]);
            fprintf(stderr, "\n  unweighted RMS residual = %8.4f\n", urms);
            fprintf(stderr, "    weighted RMS residual = %8.4f\n", wrms);
            fprintf(stderr, "  ndef = %d rank = %d m = %d ischanged = %d\n",
                    nd, nrank, m, ischanged);
        }
        i = 0;
        if (iLocConfig->Verbose > 1) fprintf(stderr, "    ");
        if (!Hypocenter->FixOT) {
            if (iLocConfig->Verbose > 1)
                fprintf(stderr, "dOT = %g ", sol[i]);
            torg += sol[i++];
            Hypocenter->Time = torg + toffset;
        }
        if (!Hypocenter->FixLon || !Hypocenter->FixLat) {
            if (!Hypocenter->FixLon) x = sol[i++];
            if (!Hypocenter->FixLat) y = sol[i++];
            azim = ILOC_RAD2DEG * atan2(x, y);
            delta = ILOC_SQRT(x * x + y * y);
            delta = ILOC_RAD2DEG * (delta / (ILOC_EARTHRADIUS - Hypocenter->Depth));
            iLoc_PointAtDeltaAzimuth(Hypocenter->Lat, Hypocenter->Lon, delta,
                                     azim, &Hypocenter->Lat, &Hypocenter->Lon);
            if (iLocConfig->Verbose > 1)
                fprintf(stderr, "dx = %g dy = %g ", x, y);
        }
        if (!fixdepthfornow) {
            if (iLocConfig->Verbose > 1)
                fprintf(stderr, "dz = %g ", -sol[i]);
            Hypocenter->Depth -= sol[i];
        }
        if (iLocConfig->Verbose > 1) {
            fprintf(stderr, "\n");
            iLoc_PrintSolution(Hypocenter, 0);
        }
        if (iLocConfig->Verbose > 2)
            iLoc_PrintDefiningPhases(Hypocenter->numPhase, Assocs);
    }
/*
 *
 *  end of iteration loop
 *
 */
/*
 *  Sort phase structures by increasing Delta, StaInd, rdid, ArrivalTime
 */
    iLoc_SortAssocs(Hypocenter->numPhase, Assocs);
    iLoc_Readings(Hypocenter->numPhase, Hypocenter->numReading, Assocs, rdindx);
/*
 *
 *  max number of iterations reached
 *
 */
    if (iter >= iLocConfig->MaxIterations) {
        fprintf(stderr, "Maximum number of iterations is reached!\n");
        retval = ILOC_SLOW_CONVERGENCE;
        isdiv = 1;
    }
/*
 *
 *  convergent solution
 *
 */
    else if (isconv) {
        fprintf(stderr, "Convergent solution after %d iterations\n", iter);
        retval = ILOC_SUCCESS;
        Hypocenter->uRMS = urms;
        Hypocenter->wRMS = wrms;
        Hypocenter->numRank = nrank;
        Hypocenter->numDef = nd;
        if (!Hypocenter->FixedDepthType && Hypocenter->Depth < ILOC_DEPSILON) {
            fixdepthfornow = 1;
            Hypocenter->FixedDepthType = 1;
            Hypocenter->Depth = 0.;
            m--;
        }
        if (!Hypocenter->FixedDepthType &&
            Hypocenter->Depth > TTInfo->MaxHypocenterDepth - ILOC_DEPSILON) {
            fixdepthfornow = 1;
            Hypocenter->FixedDepthType = 1;
            Hypocenter->Depth = TTInfo->MaxHypocenterDepth;
            m--;
        }
        Hypocenter->numUnknowns = m;
        Hypocenter->FixDepth = fixdepthfornow;
/*
 *      model covariance matrix
 */
        iLoc_SVDModelCovarianceMatrix(m, svth, svundamped, v, mcov);
        if (!Hypocenter->FixOT) {
            Hypocenter->ModelCov[0][0] = mcov[0][0];                /* stt */
            if (!Hypocenter->FixLon && !Hypocenter->FixLat) {
                Hypocenter->ModelCov[0][1] = mcov[0][1];            /* stx */
                Hypocenter->ModelCov[1][0] = mcov[1][0];            /* sxt */
                Hypocenter->ModelCov[0][2] = mcov[0][2];            /* sty */
                Hypocenter->ModelCov[2][0] = mcov[2][0];            /* syt */
                Hypocenter->ModelCov[1][1] = mcov[1][1];            /* sxx */
                Hypocenter->ModelCov[1][2] = mcov[1][2];            /* sxy */
                Hypocenter->ModelCov[2][1] = mcov[2][1];            /* syx */
                Hypocenter->ModelCov[2][2] = mcov[2][2];            /* syy */
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[0][3] = mcov[0][3];        /* stz */
                    Hypocenter->ModelCov[3][0] = mcov[3][0];        /* szt */
                    Hypocenter->ModelCov[1][3] = mcov[1][3];        /* sxz */
                    Hypocenter->ModelCov[3][1] = mcov[3][1];        /* szx */
                    Hypocenter->ModelCov[2][3] = mcov[2][3];        /* syz */
                    Hypocenter->ModelCov[3][2] = mcov[3][2];        /* szy */
                    Hypocenter->ModelCov[3][3] = mcov[3][3];        /* szz */
                }
            }
            else if (!Hypocenter->FixLon && Hypocenter->FixLat) {
                Hypocenter->ModelCov[0][1] = mcov[0][1];            /* stx */
                Hypocenter->ModelCov[1][0] = mcov[1][0];            /* sxt */
                Hypocenter->ModelCov[1][1] = mcov[1][1];            /* sxx */
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[0][3] = mcov[0][2];        /* stz */
                    Hypocenter->ModelCov[3][0] = mcov[2][0];        /* szt */
                    Hypocenter->ModelCov[1][3] = mcov[1][2];        /* sxz */
                    Hypocenter->ModelCov[3][1] = mcov[2][1];        /* szx */
                    Hypocenter->ModelCov[3][3] = mcov[2][2];        /* szz */
                }
            }
            else if (Hypocenter->FixLon && !Hypocenter->FixLat) {
                Hypocenter->ModelCov[0][2] = mcov[0][1];            /* sty */
                Hypocenter->ModelCov[2][0] = mcov[1][0];            /* syt */
                Hypocenter->ModelCov[2][2] = mcov[1][1];            /* syy */
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[0][3] = mcov[0][2];        /* stz */
                    Hypocenter->ModelCov[3][0] = mcov[2][0];        /* szt */
                    Hypocenter->ModelCov[2][3] = mcov[1][2];        /* syz */
                    Hypocenter->ModelCov[3][2] = mcov[2][1];        /* szy */
                    Hypocenter->ModelCov[3][3] = mcov[2][2];        /* szz */
                }
            }
            else {
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[3][3] = mcov[1][1];        /* szz */
                }
            }
        }
        else {
            if (!Hypocenter->FixLon && !Hypocenter->FixLat) {
                Hypocenter->ModelCov[1][1] = mcov[0][0];            /* sxx */
                Hypocenter->ModelCov[1][2] = mcov[0][1];            /* sxy */
                Hypocenter->ModelCov[2][1] = mcov[1][0];            /* syx */
                Hypocenter->ModelCov[2][2] = mcov[1][1];            /* syy */
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[1][3] = mcov[0][2];        /* sxz */
                    Hypocenter->ModelCov[3][1] = mcov[2][0];        /* szx */
                    Hypocenter->ModelCov[2][3] = mcov[1][2];        /* syz */
                    Hypocenter->ModelCov[3][2] = mcov[2][1];        /* szy */
                    Hypocenter->ModelCov[3][3] = mcov[2][2];        /* szz */
                }
            }
            else if (!Hypocenter->FixLon && Hypocenter->FixLat) {
                Hypocenter->ModelCov[1][1] = mcov[0][0];            /* sxx */
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[1][3] = mcov[0][1];        /* sxz */
                    Hypocenter->ModelCov[3][1] = mcov[1][0];        /* szx */
                    Hypocenter->ModelCov[3][3] = mcov[1][1];        /* szz */
                }
            }
            else if (Hypocenter->FixLon && !Hypocenter->FixLat) {
                Hypocenter->ModelCov[2][2] = mcov[0][0];            /* syy */
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[2][3] = mcov[0][1];        /* syz */
                    Hypocenter->ModelCov[3][2] = mcov[1][0];        /* szy */
                    Hypocenter->ModelCov[3][3] = mcov[1][1];        /* szz */
                }
            }
            else {
                if (!Hypocenter->FixDepth) {
                    Hypocenter->ModelCov[3][3] = mcov[0][0];        /* szz */
                }
            }
        }
/*
 *      location uncertainties
 */
        Uncertainties(Hypocenter, Assocs);
    }
/*
 *
 *  divergent solution
 *
 */
    else if (isdiv) {
        fprintf(stderr, "Divergent solution\n");
        retval = ILOC_DIVERGING_SOLUTION;
    }
/*
 *  abnormal exit
 */
    else {
        fprintf(stderr, "Abnormal exit from iteration loop!\n");
        isdiv = 1;
    }
    Hypocenter->Converged = isconv;
/*
 *  free memory allocated to various arrays
 */
    if (iLocConfig->DoCorrelatedErrors) {
        iLoc_FreeFloatMatrix(w);
        iLoc_FreeFloatMatrix(dcov);
    }
    iLoc_Free(phundef);
    iLoc_Free(sv);
    iLoc_Free(d);
    iLoc_FreeFloatMatrix(v);
    iLoc_FreeFloatMatrix(g);
    return retval;
}

/*
 *  Title:
 *     GetNdef
 *  Synopsis:
 *     Finds number of defining phases and the earliest arrival time.
 *  Input Arguments:
 *     numPhase - number of associated phases
 *     Assocs   - array of ILOC_ASSOC structures
 *  Output Arguments:
 *     toffset  - earliest arrival time
 *  Return:
 *     nd - number of defining phases
 *  Called by:
 *     LocateEvent
 */
static int GetNdef(int numPhase, ILOC_ASSOC *Assocs, double *toffset)
{
    int i, nd = 0;
    double toff = 1e+32;
    for (i = 0; i < numPhase; i++) {
        if (!Assocs[i].Timedef) continue;
        Assocs[i].prevTimedef = Assocs[i].Timedef;
        strcpy(Assocs[i].prevPhase, Assocs[i].Phase);
        if (Assocs[i].ArrivalTime < toff)
            toff = Assocs[i].ArrivalTime;
        nd++;
    }
    for (i = 0; i < numPhase; i++) {
        if (!Assocs[i].Azimdef) continue;
        Assocs[i].prevAzimdef = Assocs[i].Azimdef;
        strcpy(Assocs[i].prevPhase, Assocs[i].Phase);
        nd++;
    }
    for (i = 0; i < numPhase; i++) {
        if (!Assocs[i].Slowdef) continue;
        Assocs[i].prevSlowdef = Assocs[i].Slowdef;
        strcpy(Assocs[i].prevPhase, Assocs[i].Phase);
        nd++;
    }
    if (toff < 1e+32) *toffset = toff;
    return nd;
}

/*
 *  Title:
 *     GetResiduals
 *  Synopsis:
 *     Sets residuals for defining phases.
 *     Flags first arriving P and remove orphan depth phases
 *     Makes a phase non-defining if its residual is larger than
 *         SigmaThreshold times the prior measurement error and
 *         deletes corresponding row and column in the data covariance and
 *         projection matrices.
 *  Input Arguments:
 *     iLocConfig    - pointer to ILOC_CONF structure
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *     StaLocs       - array of ILOC_STA structures
 *     rdindx        - array of ILOC_READING structures
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     ec            - array of  ILOC_EC_COEF structures
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - array of  ILOC_TT_TABLE structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - array of  ILOC_TT_TABLE structures
 *     topo          - ETOPO bathymetry/elevation matrix
 *     iszderiv      - calculate dtdh [0/1]?
 *     is2nderiv     - calculate second derivatives [0/1]]
 *     iter          - iteration number
 *     ispchange     - change in phase names?
 *     dcov          - data covariance matrix from previous iteration
 *     w             - projection matrix from previous iteration
 *  Output Arguments:
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *     has_depdpres  - do we have depth-phase depth resolution?
 *     ischanged     - change in the set of defining phases? [0/1]
 *     nunp          - number of distinct phases made non-defining
 *     phundef       - list of distinct phases made non-defining
 *     dcov          - data covariance matrix
 *     w             - projection matrix
 *  Return:
 *     Success/error
 *  Called by:
 *     LocateEvent
 *  Calls:
 *     iLoc_DepthPhaseCheck, iLoc_TravelTimeResiduals
 */
static int GetResiduals(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int iszderiv,
        int is2nderiv, int *has_depdpres, int *ischanged, int iter,
        int ispchange, int *nunp, ILOC_PHASELIST *phundef, double **dcov,
        double **w)
{
    int i, j, k = 0, m = 0, kp = 0, nd = 0, nund = 0, isdiff = 0, isfound = 0;
    int retval = ILOC_UNKNOWN_ERROR;
    double thres = 0.;
    int prevndef = Hypocenter->numDef;
/*
 *  flag first arriving P and remove orphan depth phases
 */
    k = iLoc_DepthPhaseCheck(iLocConfig, Hypocenter, Assocs, rdindx);
    *has_depdpres = k;
/*
 *  set ttime, residual, dtdh, and dtdd for defining phases
 */
    retval = iLoc_TravelTimeResiduals(iLocConfig, Hypocenter, Assocs, StaLocs,
                        ec, TTInfo, TTtables, LocalTTInfo, LocalTTtables,
                        topo, PhaseIdInfo, 0, iszderiv, is2nderiv);
    if (retval)
        return retval;
/*
 *  see if set of time defining phases has changed
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].Timedef) {
            thres = iLocConfig->SigmaThreshold * Assocs[i].Deltim;
/*
 *          make phase non-defining if its residual is larger than
 *          SigmaThreshold times the prior measurement error
 */
            if (fabs(Assocs[i].TimeRes) > thres) {
                Assocs[i].Timedef = 0;
                Hypocenter->numTimedef--;
            }
            else nd++;
        }
        if (Assocs[i].Timedef < Assocs[i].prevTimedef) {
            isdiff = 1;
            nund++;
/*
 *          observation no longer used; delete corresponding row and column
 *          in data covariance and projection matrices
 */
            if (iter && !ispchange && iLocConfig->DoCorrelatedErrors) {
/*
 *              add phase to list of phases whose block has changed
 */
                isfound = 0;
                for (j = 0; j < kp; j++) {
                    if (ILOC_STREQ(phundef[j].Phase, Assocs[i].Phase))
                        isfound = 1;
                }
                if (!isfound) {
                    strcpy(phundef[kp].Phase, Assocs[i].Phase);
                    kp++;
                }
/*
 *             index of phase in dcov and w
 */
                k = Assocs[i].CovIndTime;
                for (j = k; j < prevndef - 1; j++) {
                    for (m = 0; m < prevndef; m++) {
                        dcov[j][m] = dcov[j+1][m];
                        w[j][m] = w[j+1][m];
                    }
                }
                for (j = k; j < prevndef - 1; j++) {
                    for (m = 0; m < prevndef; m++) {
                        dcov[m][j] = dcov[m][j+1];
                        w[m][j] = w[m][j+1];
                    }
                }
            }
        }
        Assocs[i].prevTimedef = Assocs[i].Timedef;
    }
/*
 *  see if set of azimuth defining phases has changed
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].Azimdef) {
            thres = iLocConfig->SigmaThreshold * Assocs[i].Delaz;
/*
 *          make phase non-defining if its residual is larger than
 *          SigmaThreshold times the prior measurement error
 */
            if (fabs(Assocs[i].AzimRes) > thres) {
                Assocs[i].Azimdef = 0;
                Hypocenter->numAzimdef--;
            }
            else nd++;
        }
        if (Assocs[i].Azimdef < Assocs[i].prevAzimdef) {
            isdiff = 1;
            nund++;
/*
 *          observation no longer used; delete corresponding row and column
 *          in data covariance and projection matrices
 */
            if (iter && !ispchange && iLocConfig->DoCorrelatedErrors) {
/*
 *              add phase to list of phases whose block has changed
 */
                isfound = 0;
                for (j = 0; j < kp; j++) {
                    if (ILOC_STREQ(phundef[j].Phase, Assocs[i].Phase))
                        isfound = 1;
                }
                if (!isfound) {
                    strcpy(phundef[kp].Phase, Assocs[i].Phase);
                    kp++;
                }
/*
 *             index of phase in dcov and w
 */
                k = Assocs[i].CovIndAzim;
                for (j = k; j < prevndef - 1; j++) {
                    for (m = 0; m < prevndef; m++) {
                        dcov[j][m] = dcov[j+1][m];
                        w[j][m] = w[j+1][m];
                    }
                }
                for (j = k; j < prevndef - 1; j++) {
                    for (m = 0; m < prevndef; m++) {
                        dcov[m][j] = dcov[m][j+1];
                        w[m][j] = w[m][j+1];
                    }
                }
            }
        }
        Assocs[i].prevAzimdef = Assocs[i].Azimdef;
    }
/*
 *  see if set of slowness defining phases has changed
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].Slowdef) {
            thres = iLocConfig->SigmaThreshold * Assocs[i].Delslo;
/*
 *          make phase non-defining if its residual is larger than
 *          SigmaThreshold times the prior measurement error
 */
            if (fabs(Assocs[i].SlowRes) > thres) {
                Assocs[i].Slowdef = 0;
                Hypocenter->numSlowdef--;
            }
            else nd++;
        }
        if (Assocs[i].Slowdef < Assocs[i].prevSlowdef) {
            isdiff = 1;
            nund++;
/*
 *          delete corresponding row and column in dcov and w
 */
            if (iter && !ispchange && iLocConfig->DoCorrelatedErrors) {
/*
 *             add phase to list of phases whose block has changed
 */
                isfound = 0;
                for (j = 0; j < kp; j++) {
                    if (ILOC_STREQ(phundef[j].Phase, Assocs[i].Phase))
                        isfound = 1;
                }
                if (!isfound) {
                    strcpy(phundef[kp].Phase, Assocs[i].Phase);
                    kp++;
                }
/*
 *             index of phase in dcov and w
 */
                k = Assocs[i].CovIndSlow;
                for (j = k; j < prevndef - 1; j++) {
                    for (m = 0; m < prevndef; m++) {
                        dcov[j][m] = dcov[j+1][m];
                        w[j][m] = w[j+1][m];
                    }
                }
                for (j = k; j < prevndef - 1; j++) {
                    for (m = 0; m < prevndef; m++) {
                        dcov[m][j] = dcov[m][j+1];
                        w[m][j] = w[m][j+1];
                    }
                }
            }
        }
        Assocs[i].prevSlowdef = Assocs[i].Slowdef;
    }
    Hypocenter->numDef = nd;
    *nunp = kp;
    *ischanged = isdiff;
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     BuildGd
 *  Synopsis:
 *     Builds G matrix and d vector for equation Gm = d.
 *     G is the matrix of partial derivates of travel-times,
 *     d is the vector of residuals.
 *  Input arguments
 *     ndef - number of defining observations
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - pointer to ILOC_ASSOC structures
 *     fixdepthfornow - fix depth for this iteration?
 *  Output arguments
 *     g    - G matrix G(N x 4)
 *     d    - residual vector d(N)
 *  Return:
 *     urms - unweighted rms residual
 *  Called by:
 *     LocateEvent
 */
static double BuildGd(int ndef, ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs,
        int fixdepthfornow, double **g, double *d, int verbose)
{
    int i, j, k, im = 0;
    double urms = 0., depthcorr = 0., esaz = 0., acorr = 0., azcorr = 0.;
	depthcorr = ILOC_DEG2RAD * (ILOC_EARTHRADIUS - Hypocenter->Depth);
	acorr = ILOC_DEG2RAD * ILOC_EARTHRADIUS;
/*
 *  G matrix of partial derivates of travel-times
 */
    if (verbose > 4)
        fprintf(stderr, "G matrix and d vector:\n");
    for (k = 0, i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].Timedef) {
            for (j = 0; j < 4; j++) g[k][j] = 0.;
            esaz = ILOC_DEG2RAD * Assocs[i].Esaz;
            im = 0;
            if (!Hypocenter->FixOT)                                    /* T */
                g[k][im++] = 1.;
            if (!Hypocenter->FixLon)                                   /* x */
                g[k][im++] = -(Assocs[i].dtdd / depthcorr) * sin(esaz);
            if (!Hypocenter->FixLat)                                   /* y */
                g[k][im++] = -(Assocs[i].dtdd / depthcorr) * cos(esaz);
            if (!fixdepthfornow)                                      /* Up */
                g[k][im++] = -Assocs[i].dtdh;
/*
 *          d vector and unweighted rms residual
 */
            d[k] = Assocs[i].TimeRes;
            urms += d[k] * d[k];
            if (verbose > 4) {
                fprintf(stderr, "  %6d %6d %-9s (T) ",
                        k, Assocs[i].StaInd, Assocs[i].Phase);
                for (j = 0; j < im; j++) fprintf(stderr, "%10.3f ", g[k][j]);
                fprintf(stderr, " , %10.3f\n", d[k]);
            }
            k++;
        }
    }
/*
 *  G matrix of partial derivates of azimuths
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].Azimdef) {
            for (j = 0; j < 4; j++) g[k][j] = 0.;
            esaz = ILOC_DEG2RAD * Assocs[i].Esaz;
	        azcorr = sin(ILOC_DEG2RAD * Assocs[i].Delta) * acorr;
	        if (fabs(azcorr) < 0.0001) {
	            if (azcorr < 0.) azcorr = -0.0001;
	            else             azcorr = 0.0001;
	        }
            im = 0;
            if (!Hypocenter->FixOT)                                    /* T */
                g[k][im++] = 0.;
            if (!Hypocenter->FixLon)                                   /* x */
                g[k][im++] = -cos(esaz) / azcorr;
            if (!Hypocenter->FixLat)                                   /* y */
                g[k][im++] = sin(esaz) / azcorr;
            if (!fixdepthfornow)                                      /* Up */
                g[k][im++] = 0.;
/*
 *          d vector and unweighted rms residual
 */
            d[k] = ILOC_DEG2RAD * Assocs[i].AzimRes;
            urms += d[k] * d[k];
            if (verbose > 4) {
                fprintf(stderr, "  %6d %6d %-9s (A) ",
                        k, Assocs[i].StaInd, Assocs[i].Phase);
                for (j = 0; j < im; j++) fprintf(stderr, "%10.3f ", g[k][j]);
                fprintf(stderr, " , %10.3f\n", d[k]);
            }
            k++;
        }
    }
/*
 *  G matrix of partial derivates of slownesses
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].Slowdef) {
            for (j = 0; j < 4; j++) g[k][j] = 0.;
            esaz = ILOC_DEG2RAD * Assocs[i].Esaz;
            im = 0;
            if (!Hypocenter->FixOT)                                    /* T */
                g[k][im++] = 0.;
            if (!Hypocenter->FixLon)                                   /* x */
                g[k][im++] = -(Assocs[i].d2tdd / depthcorr) * sin(esaz);
            if (!Hypocenter->FixLat)                                   /* y */
                g[k][im++] = -(Assocs[i].d2tdd / depthcorr) * cos(esaz);
            if (!fixdepthfornow)                                      /* Up */
                g[k][im++] = -Assocs[i].d2tdh;
/*
 *          d vector and unweighted rms residual
 */
            d[k] = Assocs[i].SlowRes / ILOC_DEG2KM;
            urms += d[k] * d[k];
            if (verbose > 4) {
                fprintf(stderr, "  %6d %6d %-9s (S) ",
                        k, Assocs[i].StaInd, Assocs[i].Phase);
                for (j = 0; j < im; j++) fprintf(stderr, "%10.3f ", g[k][j]);
                fprintf(stderr, " , %10.3f\n", d[k]);
            }
            k++;
        }
    }
    urms = ILOC_SQRT(urms / (double)ndef);
    if (verbose > 2) fprintf(stderr, "urms = %12.6f\n", urms);
    return urms;
}

/*
 *  Title:
 *     ProjectGd
 *  Synopsis:
 *     Projects G matrix and d vector into eigensystem.
 *  Input arguments:
 *     ndef  - number of defining observations
 *     m     - number of model parameters
 *     g     - G matrix G(N x M)
 *     d     - residual vector d(N)
 *     w     - W projection matrix W(N x N)
 *  Output arguments:
 *     g     - projected G matrix G(N x M)
 *     d     - projected residual vector d(N)
 *     dnorm - data norm (sum of squares of weighted residuals)
 *     wrms  - weighted rms residual
 *  Returns:
 *     0/1 on success/error
 *  Called by:
 *     LocateEvent
 *  Calls:
 *     WxG
 */
static int ProjectGd(int ndef, int m, double **g, double *d, double **w,
        double *dnorm, double *wrms, int verbose)
{
    int i, j, k;
    double *temp = (double *)NULL;
    double wssq = 0.;
/*
 *  allocate memory for temporary storage
 */
    if ((temp = (double *)calloc(ndef, sizeof(double))) == NULL) {
        fprintf(stderr, "ProjectGd: cannot allocate memory\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  WG(NxM) = W(NxN) * G(NxM)
 */
    for (j = 0; j < m; j++) {
        if (WxG(j, ndef, w, g)) {
            iLoc_Free(temp);
            return ILOC_MEMORY_ALLOCATION_ERROR;
        }
    }
/*
 *  Wd(N) = W(NxN) * d(N) and sum of squares of weighted residuals
 */
    for (i = 0; i < ndef; i++) {
        temp[i] = 0.;
        for (k = 0; k < ndef; k++)
            temp[i] += w[i][k] * d[k];
        if (fabs(temp[i]) < ILOC_ZEROTOL) temp[i] = 0.;
    }
    wssq = 0.;
    for (i = 0; i < ndef; i++) {
        d[i] = temp[i];
        wssq += d[i] * d[i];
    }
    iLoc_Free(temp);
    *dnorm = wssq;
    *wrms = ILOC_SQRT(wssq / (double)ndef);
    if (verbose > 4) {
        fprintf(stderr, "WG(%d x %d) matrix and Wd vector:\n", ndef, m);
        for (i = 0; i < ndef; i++) {
            fprintf(stderr, " %6d ", i);
            for (j = 0; j < m; j++) fprintf(stderr, "%10.3f ", g[i][j]);
            fprintf(stderr, " , %10.3f\n", d[i]);
        }
        fprintf(stderr, "wrms = %10.3f\n", *wrms);
    }
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     WxG
 *  Synopsis:
 *     W * G matrix multiplication
 *  Input arguments:
 *     j     - model dimension index
 *     ndef  - number of defining observations
 *     w     - W projection matrix W(N x N)
 *     g     - G matrix G(N x M)
 *  Output arguments:
 *     g     - projected G matrix G(N x M)
 *  Returns:
 *     0/1 on success/error
 *  Called by:
 *     ProjectGd
 */
static int WxG(int j, int ndef, double **w, double **g)
{
    int i, k;
    double *temp = (double *)NULL;
    if ((temp = (double *)calloc(ndef, sizeof(double))) == NULL) {
        fprintf(stderr, "WxG: cannot allocate memory\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
    for (k = 0; k < ndef; k++)
        temp[k] = g[k][j];
    for (i = 0; i < ndef; i++) {
        g[i][j] = 0.;
        for (k = 0; k < ndef; k++)
            g[i][j] += w[i][k] * temp[k];
        if (fabs(g[i][j]) < ILOC_ZEROTOL) g[i][j] = 0.;
    }
    iLoc_Free(temp);
    return ILOC_SUCCESS;
}


/*
 *  Title:
 *     WeightGd
 *  Synopsis:
 *     Independence assumption: weight Gm = d by measurement errors.
 *  Input arguments
 *     ndef     - number of defining observations
 *     m        - number of model parameters
 *     numPhase - number of associated phases
 *     Assocs   - pointer to ILOC_ASSOC structures
 *     g        - G matrix G(N x M)
 *     d        - residual vector d(N)
 *  Output arguments
 *     g     - weighted G matrix G(N x M)
 *     d     - weighted residual vector d(N)
 *     dnorm - data norm (sum of squares of weighted residuals)
 *     wrms  - weighted rms residual
 *  Called by:
 *     LocateEvent
 */
static void WeightGd(int ndef, int m, int numPhase, ILOC_ASSOC *Assocs,
        double **g, double *d, double *dnorm, double *wrms)
{
    int i, j, k;
    double wssq = 0., weight = 0.;
    for (k = 0, i = 0; i < numPhase; i++) {
        if (!Assocs[i].Timedef) continue;
        if (Assocs[i].Deltim < ILOC_DEPSILON)
            weight = 1.;
        else
            weight = 1. / Assocs[i].Deltim;
        for (j = 0; j < m; j++)
            g[k][j] *= weight;
        d[k] *= weight;
        wssq += d[k] * d[k];
        k++;
    }
    for (k = 0, i = 0; i < numPhase; i++) {
        if (!Assocs[i].Azimdef) continue;
        if (Assocs[i].Delaz < ILOC_DEPSILON)
            weight = 1.;
        else
            weight = 1. / Assocs[i].Delaz;
        for (j = 0; j < m; j++)
            g[k][j] *= weight;
        d[k] *= weight;
        wssq += d[k] * d[k];
        k++;
    }
    for (k = 0, i = 0; i < numPhase; i++) {
        if (!Assocs[i].Slowdef) continue;
        if (Assocs[i].Delslo < ILOC_DEPSILON)
            weight = 1.;
        else
            weight = 1. / Assocs[i].Delslo;
        for (j = 0; j < m; j++)
            g[k][j] *= weight;
        d[k] *= weight;
        wssq += d[k] * d[k];
        k++;
    }
    *dnorm = wssq;
    *wrms = ILOC_SQRT(wssq / (double)ndef);
}

/*
 *  Title:
 *     ConvergenceTestValue
 *  Synopsis:
 *     Convergence test value of Paige and Saunders (1982)
 *
 *     Paige, C. and Saunders, M., 1982,
 *         LSQR: An Algorithm for Sparse Linear Equations and
 *         Sparse Least Squares,
 *         ACM Trans. Math. Soft. 8, 43-71.
 *
 *               ||transpose(G) * d||
 *     cvgtst = ----------------------
 *                  ||G|| * ||d||
 *
 *  Input arguments:
 *     gtdnorm  - ||transpose(G) * d||
 *     gnorm    - G matrix norm (Sum(sv^2))
 *     dnorm    - d vector norm (Sum(d^2))
 *  Return:
 *     cnvgtst  - Paige-Saunders convergence test number
 *  Called by:
 *     LocateEvent
 */
static double ConvergenceTestValue(double gtdnorm, double gnorm, double dnorm)
{
    double cnvgtst = 0., gd = 0.;
    gd = gnorm * dnorm;
    if (gtdnorm > ILOC_DEPSILON && gd < ILOC_DEPSILON)
        cnvgtst = 999.;
    else
        cnvgtst = gtdnorm / gd;
    return cnvgtst;
}

/*
 *  Title:
 *     ConvergenceTest
 *  Synopsis:
 *     Convergence/divergence is decided based on
 *         the Paige-Saunder convergence test value and
 *         the history of model and data norms.
 *  Input Arguments:
 *     iLocConfig  - pointer to ILOC_CONF structure
 *     iter        - iteration number
 *     m           - number of model parameters
 *     nds[]       - current and past number of defining observations
 *     sol[]       - current solution vector
 *     oldsol[]    - previous solution vector
 *     wrms        - weighted RMS residual
 *     modelnorm[] - current and past model norms
 *     convgtest[] - current and past convergence test values
 *     oldcvgtst   - previous convergence test value
 *     step        - step length
 *  Output Arguments:
 *     oldsol[]    - previous solution vector
 *     oldcvgtst   - previous convergence test value
 *     step        - step length
 *     isdiv       - divergent solution [0/1]
 *  Returns:
 *     isconv - convergent solution [0/1]
 *  Called by:
 *     LocateEvent
 */
static int ConvergenceTest(ILOC_CONF *iLocConfig, int iter, int m, int *nds,
        double *sol,  double *oldsol, double wrms, double *modelnorm,
        double *convgtest, double *oldcvgtst, double *step, int *isdiv)
{
    double dm01 = 0., dm12 = 0., dc01 = 0., dc12 = 0.;
    double sc = *step, oldcvg = 0.;
    int i, convergent = 0, divergent = 0;
    oldcvg = *oldcvgtst;
    if (modelnorm[0] > 0. && convgtest[0] > 0.) {
/*
 *      indicators of increasing/decreasing model norms and convergence tests
 */
        if (modelnorm[1] <= 0. || modelnorm[2] <= 0.)
            dm01 = dm12 = 1.05;
        else {
            dm01 = modelnorm[0] / modelnorm[1];
            dm12 = modelnorm[1] / modelnorm[2];
        }
        if (convgtest[1] <= 0. || convgtest[2] <= 0.)
            dc01 = convgtest[0];
        else {
            dc01 = convgtest[0] / convgtest[1];
            dc12 = convgtest[1] / convgtest[2];
            dc01 = fabs(dc12 - dc01);
        }
        dc12 = fabs(convgtest[0] - convgtest[2]);
/*
 *      divergent solution if increasing model norm
 */
        if (dm12 > 1.1 && dm01 > dm12 &&
            iter > iLocConfig->MinIterations + 2 && modelnorm[0] > 500)
            divergent = 1;
/*
 *      convergent solution if vanishing
 *      convergence test value or model norm or weighted RMS residual
 */
        else if (nds[0] == nds[1] &&
                (convgtest[0] < ILOC_CVGTOL || modelnorm[0] < 0.1 || wrms < 0.01))
            convergent = 1;
/*
 *      convergent solution if vanishing convergence test value
 */
        else if ((convgtest[0] < 1.01 * oldcvg && convgtest[0] < ILOC_CVGTOL) ||
                 (iter > 3 * iLocConfig->MaxIterations / 4 &&
                     (convgtest[0] < ILOC_SQRT(ILOC_CVGTOL) ||
                      dc01 < ILOC_CVGTOL ||
                      dc12 < ILOC_SQRT(ILOC_CVGTOL))
                ))
            convergent = 1;
    }
    else
        convergent = 1;
    if (iter == iLocConfig->MaxIterations - 1)
        convergent = 0;
/*
 *  Apply step-length weighting if convergence test value is increasing.
 *  Steps are applied in half-lengths of previous solution vector
 */
    if (iter > iLocConfig->MinIterations + 2 && sc > 0.05 &&
        (convgtest[0] > *oldcvgtst || convgtest[0] - convgtest[2] == 0.)) {
        sc *= 0.5;
        if (sc != 0.5) {
            for (i = 0; i < m; i++) {
                if (fabs(oldsol[i]) < ILOC_ZEROTOL) oldsol[i] = sol[i];
                sol[i] = sc * oldsol[i];
            }
        }
        else {
            for (i = 0; i < m; i++) {
                sol[i] = sc * sol[i];
                oldsol[i] = sol[i];
            }
        }
    }
    else {
        sc = 1;
        *oldcvgtst = convgtest[0];
    }
    *step = sc;
    *isdiv = divergent;
    return convergent;
}

/*
 *  Title:
 *     Uncertainties
 *  Synopsis:
 *     Calculates numDefsta, SdevdObs.
 *        numDefsta  = the number of defining stations
 *        SdevdObs    = sqrt(sum of squares of UNweighted residuals / (N - M))
 *     Calculates 1D and 2D formal uncertainties scaled to 90% confidence level
 *        Jordan T.H. and K.A. Sverdrup,
 *        Teleseismic location techniques and their application
 *        to earthquake clusters in the South-Central Pacific,
 *        Bull. Seism. Soc. Am., 71, 1105-1130, 1981.
 *  Input Arguments:
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - pointer to ILOC_ASSOC structures
 *  Called by:
 *     LocateEvent
 *  Calls:
 *     Ftest
 */
static void Uncertainties(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs)
{
    double fs = 0., fscale = 0.;
    double ndf = 0., b = 0., d = 0., eigen1 = 0., eigen2 = 0., xn;
    double ssq = 0., sigmahat = 0., sxx = 0., sxy = 0., syy = 0., strike = 0.;
    int PrevStaInd = -1;
    int totndf = 0, k = 0, i;
/*
 *  Count defining stations
 *  Phases ordered by increasing Delta, StaInd, rdid, ArrivalTime
 */
    Hypocenter->numDefsta = 0;
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if ((Assocs[i].Timedef || Assocs[i].Azimdef || Assocs[i].Slowdef) &&
           Assocs[i].StaInd != PrevStaInd) {
            Hypocenter->numDefsta++;
        }
        PrevStaInd = Assocs[i].StaInd;
    }
/*
 *  sdobs = sqrt(sum of squares of UNweighted residuals / (N - M))
 */
    totndf = ILOC_MAX(Hypocenter->numDef - Hypocenter->numUnknowns, 1);
    xn = ILOC_SQRT((double)Hypocenter->numDef / (double)(totndf));
    Hypocenter->SdevObs = Hypocenter->uRMS * xn;
/*
 *  sigmahat and total number of degrees of freedom
 *      Jordan T.H. and K.A. Sverdrup,
 *      Teleseismic location techniques and their application
 *      to earthquake clusters in the South-Central Pacific,
 *      Bull. Seism. Soc. Am., 71, 1105-1130, 1981.
 *
 *      K is taken as infinity -> coverage error ellipse
 */
    k = 99999;
    xn = (double)Hypocenter->numDef;
    ssq = Hypocenter->wRMS * Hypocenter->wRMS * xn + (double)k;
    totndf = Hypocenter->numRank - Hypocenter->numUnknowns + k;
    ndf = (double)totndf;
    if (fabs(ndf) < ILOC_DEPSILON)
        ndf = 0.001;
    if (fabs(totndf - ssq) < 0.00001)
        ndf = ssq;
    sigmahat = ILOC_SQRT(ssq / ndf);
/*
 *  Calculate 1D standard errors, scaled to 90% confidence level
 */
    fs = Ftest(1, totndf);
    fscale = ILOC_SQRT(fs) * sigmahat;
    for (i = 0; i < 4; i++)
        if (Hypocenter->ModelCov[i][i] != ILOC_NULLVAL)
            Hypocenter->Errors[i] = fscale * ILOC_SQRT(Hypocenter->ModelCov[i][i]);
/*
 *  Convert lat, lon errors to degrees
 */
    if (!Hypocenter->FixLon) Hypocenter->Errors[1] /= ILOC_DEG2KM;
    if (!Hypocenter->FixLat) Hypocenter->Errors[2] /= ILOC_DEG2KM;
/*
 *  Calculate 2D coverage error ellipse scaled to 90% confidence level
 */
    if (!Hypocenter->FixLon && !Hypocenter->FixLat) {
        sxx = Hypocenter->ModelCov[1][1];
        sxy = Hypocenter->ModelCov[1][2];
        syy = Hypocenter->ModelCov[2][2];
/*
 *      eigenvalues of the 2x2 covariance matrix
 */
        b = sxx + syy;
        d = ILOC_SQRT(b * b - 4. * (sxx * syy - sxy * sxy));
        eigen1 = fabs((b + d) / 2.);
        eigen2 = fabs((b - d) / 2.);
        strike = 0.5 * atan2(2. * sxy , (syy - sxx));
        if (strike < 0.)    strike += ILOC_TWOPI;
        if (strike > ILOC_TWOPI) strike -= ILOC_TWOPI;
        if (strike > ILOC_PI)    strike -= ILOC_PI;
        Hypocenter->Strike = ILOC_RAD2DEG * strike;
/*
 *      scale to 90% confidence level
 */
        fs = Ftest(2, totndf);
        fscale = ILOC_SQRT(2. * fs) * sigmahat;
        Hypocenter->semiMajax = fscale * ILOC_SQRT(eigen1);
        Hypocenter->semiMinax = fscale * ILOC_SQRT(eigen2);
    }
}

/*
 *  Title:
 *     Ftest
 *  Synopsis:
 *     Critical value of F distribution with M, N degrees of freedom
 *     at 90% confidence level
 *     The critical values for the F distribution are listed in
 *         D. Zwillinger and S. Kokoska, 2000,
 *         CRC Standard Probability and Statistics Tables and Formulae,
 *         Chapman and Hall/CRC.
 *  Input Arguments:
 *     m - number of model parameters [1..4]]
 *     n - number of degrees of freedom (observations - model params)
 *  Return:
 *     Critical value (Prob[F >= F(M,N)] = 0.1)
 *  Called by:
 *     Uncertainties
 */
#define	ILOC_NUMDF 24
static double Ftest(int m, int n)
{
    int i, j = 0;
    double x = (double)n;
    double x1 = 0., x2 = 0., y1 = 0., y2 = 0., critVal = 0.;
/*
 *  number of degree of freedoms
 */
    int ns[ILOC_NUMDF] = {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
                      11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                      25, 50, 100, 99999 };
/*
 *  F[M,N] critical values table at 90% confidence level for M=[1..4]
 */
    double cv[4][ILOC_NUMDF] = {
        { 39.86, 8.53, 5.54, 4.54, 4.06, 3.78, 3.59, 3.46, 3.36, 3.29,
           3.23, 3.18, 3.14, 3.10, 3.07, 3.05, 3.03, 3.01, 2.99, 2.97,
           2.92, 2.81, 2.76, 2.71 },
        { 49.50, 9.00, 5.46, 4.32, 3.78, 3.46, 3.26, 3.11, 3.01, 2.92,
           2.86, 2.81, 2.76, 2.73, 2.70, 2.67, 2.64, 2.62, 2.61, 2.59,
           2.53, 2.41, 2.36, 2.30 },
        { 53.59, 9.16, 5.39, 4.19, 3.62, 3.29, 3.07, 2.92, 2.81, 2.73,
           2.66, 2.61, 2.56, 2.52, 2.49, 2.46, 2.44, 2.42, 2.40, 2.38,
           2.32, 2.20, 2.14, 2.08 },
        { 55.83, 9.24, 5.34, 4.11, 3.52, 3.18, 2.96, 2.81, 2.69, 2.61,
           2.54, 2.48, 2.43, 2.39, 2.36, 2.33, 2.31, 2.29, 2.27, 2.25,
           2.18, 2.06, 2.00, 1.94 }
    };
    if (m < 1 || m > 4 || n < 1)
        return critVal;
/*
 *  find indexes
 */
    for (i = ILOC_NUMDF - 1; i >= 0; i--){
        if (n >= ns[i]) {
            j = i;
            break;
        }
    }
/*
 *  exact match
 */
    if (n == ns[j] || j == ILOC_NUMDF - 1)
        critVal = cv[m-1][j];
/*
 *  interpolate
 */
    else {
        x1 = (double)ns[j];
        x2 = (double)ns[j+1];
        x = (double)n;
        y1 = cv[m-1][j];
        y2 = cv[m-1][j+1];
        critVal = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
   }
   return critVal;
}

