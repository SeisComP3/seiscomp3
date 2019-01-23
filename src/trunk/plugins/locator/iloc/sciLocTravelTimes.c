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
 *    iLoc_GetPhaseIndex
 *    iLoc_TravelTimeResiduals
 *    iLoc_GetTravelTimePrediction
 *    iLoc_GetEtopoCorrection
 */

/*
 * Local functions:
 *    GetTTResidual
 *    isRSTT
 *    GetTravelTimeTableValue
 *    TravelTimeCorrections
 *    GetElevationCorrection
 *    GetLastLag
 *    GetBounceCorrection
 *    GetEtopoElevation
 *    GetEllipticityCorrection
 *    ECPhaseIndex
 */
static double GetTTResidual(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, ILOC_STA *StaLoc, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo,
        ILOC_PHASEIDINFO *PhaseIdInfo, int isall, int iszderiv, int is2nderiv);
static int isRSTT(ILOC_CONF *iLocConfig, ILOC_ASSOC *Assoc);
static double GetTravelTimeTableValue(double depth, double delta,
        ILOC_TT_TABLE *TTtable, int iszderiv, double *dtdd, double *dtdh,
        double *bpdel, int is2nderiv, double *d2tdd, double *d2tdh);
static void TravelTimeCorrections(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, ILOC_STA *StaLoc, ILOC_EC_COEF *ec, short int **topo,
        double Psurfvel, double Ssurfvel);
static double GetElevationCorrection(ILOC_ASSOC *Assoc, ILOC_STA *StaLoc,
        double PSurfvel, double SSurfvel);
static int GetLastLag(char phase[]);
static double GetBounceCorrection(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, short int **topo, double Psurfvel, double Ssurfvel,
        double *tcorw);
static double GetEtopoElevation(ILOC_CONF *iLocConfig, double lat, double lon,
        short int **topo);
static double GetEllipticityCorrection(ILOC_EC_COEF *ec, char *phase,
        double ecolat, double delta, double depth, double esaz);
static int ECPhaseIndex(char *phase, double delta);


/*
 *  Title:
 *     iLoc_GetPhaseIndex
 *  Synopsis:
 *	   Returns index of ILOC_TT_TABLE struct array for a given phase
 *  Input Arguments:
 *     phase  - phase
 *     TTInfo - pointer to ILOC_TTINFO structure
 *  Return:
 *     phase index or -1 on error
 *  Called by:
 *     iLoc_GetTravelTimePrediction, GenerateLocalTTtables
 */
int iLoc_GetPhaseIndex(char *phase, ILOC_TTINFO *TTInfo)
{
    int i;
    for (i = 0; i < TTInfo->numPhaseTT; i++) {
        if (ILOC_STREQ(phase, TTInfo->PhaseTT[i].Phase))
            return i;
    }
    return -1;
}

/*
 *  Title:
 *     iLoc_TravelTimeResiduals
 *  Synopsis:
 *     Calculates time residuals.
 *     Uses phase dependent information in PhaseIdInfo structure.
 *        PhaseWithoutResidual - list of phases that don't get residuals, i.e.
 *                         never used in the location (e.g. amplitude phases)
 *     If isall is set to '1 it attempts to get time residuals for all
 *        associated phases, otherwise considers only defining phases.
 *  Input Arguments:
 *     iLocConfig    - pointer to ILOC_CONF structure
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assocs        - array of ILOC_ASSOC structures
 *     StaLocs       - array of ILOC_STA structures
 *     ec            - array of ILOC_EC_COEF structures
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - array of ILOC_TT_TABLE structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - array of ILOC_TT_TABLE structures
 *     topo          - ETOPO bathymetry/elevation matrix
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     isall         - 1 if want residuals for all phases
 *                     0 if want residuals for defining phases only
 *     iszderiv      - calculate dtdh [0/1]?
 *     is2nderiv     - calculate second derivatives [0/1]]
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_Locator, GetResiduals, ResidualsForFixedHypocenter
 *  Calls:
 *     GetTTResidual, iLoc_GetStaIndex
 */
int iLoc_TravelTimeResiduals(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo,
        ILOC_PHASEIDINFO *PhaseIdInfo, int isall, int iszderiv, int is2nderiv)
{
    int i, j;
/*
 *  can't calculate residuals unless have a depth for the source
 */
    if (Hypocenter->Depth == ILOC_NULLVAL) {
        fprintf(stderr, "iLoc_TravelTimeResiduals: depthless hypocentre\n");
        return ILOC_INVALID_DEPTH;
    }
/*
 *  won't get residuals if source has gone too deep
 */
    if (Hypocenter->Depth > TTInfo->MaxHypocenterDepth) {
        fprintf(stderr, "iLoc_TravelTimeResiduals: solution too deep %f > %f \n",
                Hypocenter->Depth, TTInfo->MaxHypocenterDepth);
        return ILOC_INVALID_DEPTH;
    }
/*
 *  calculate time residual for associated/defining phases
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        j = Assocs[i].StaInd;
        Assocs[i].TimeRes = GetTTResidual(iLocConfig, Hypocenter, &Assocs[i],
                                    &StaLocs[j], ec, TTInfo, TTtables,
                                    LocalTTInfo, LocalTTtables, topo,
                                    PhaseIdInfo, isall, iszderiv, is2nderiv);
    }
/*
 *  clear current GreatCircle object and the pool of CrustalProfile objects
 */
    if (iLocConfig->UseRSTT)
        slbm_shell_clear();
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     GetTTResidual
 *  Synopsis:
 *     Calculates the time residual for a single phase.
 *     Uses phase dependent information from <vmodel>_model.txt file.
 *        PhaseWithoutResidual - list of phases that don't get residuals, i.e.
 *                         never used in the location (e.g. amplitude phases)
 *  Input Arguments:
 *     iLocConfig    - pointer to ILOC_CONF structure
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assoc         - pointer to ILOC_ASSOC structure
 *     StaLoc        - pointer to ILOC_STA structure
 *     ec            - pointer to ILOC_EC_COEF structures
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - pointer to ILOC_TT_TABLE structure
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - pointer to ILOC_TT_TABLE structures
 *     topo          - ETOPO bathymetry/elevation matrix
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     isall         - 1 if want residuals for all phases
 *                     0 if want residuals for defining phases only
 *     iszderiv      - calculate dtdh [0/1]?
 *     is2nderiv     - calculate second derivatives [0/1]]
 *  Output Arguments:
 *     Assoc         - pointer to ILOC_ASSOC structure
 *  Return:
 *     resid - time residual for a phase
 *  Called by:
 *     iLoc_TravelTimeResiduals
 *  Calls:
 *     iLoc_GetTravelTimePrediction
 */
static double GetTTResidual(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, ILOC_STA *StaLoc, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo,
        ILOC_PHASEIDINFO *PhaseIdInfo, int isall, int iszderiv, int is2nderiv)
{
    double obtime = 0., resid = ILOC_NULLVAL;
    int j, isfirst = 0, iszd = iszderiv, is2nd = is2nderiv;
/*
 *  azimuth residual
 */
    if (Assoc->BackAzimuth != ILOC_NULLVAL) {
        Assoc->AzimRes = Assoc->BackAzimuth - Assoc->Seaz;
    }
/*
 *  timeless or codeless phases don't have residuals
 */
    if (!isall && (!Assoc->Timedef || !Assoc->Phase[0]))
        return resid;
    if (Assoc->ArrivalTime == ILOC_NULLVAL)
        return resid;
    if (isall) {
/*
 *      try to get residuals for all associated phases
 */
        if (Assoc->Phase[0]) {
/*
 *          check for phases that don't get residuals (amplitudes etc)
 */
            for (j = 0; j < PhaseIdInfo->numPhaseWithoutResidual; j++)
                if (ILOC_STREQ(Assoc->Phase, PhaseIdInfo->PhaseWithoutResidual[j].Phase))
                    break;
            if (j != PhaseIdInfo->numPhaseWithoutResidual)
                return resid;
        }
        if (!Assoc->Timedef)
            iszd = is2nd = 0;
    }
/*
 *  do not use first-arriving P/S TT tables for IHO or nondefining phases
 */
    if (!Assoc->Timedef || ILOC_STREQ(Assoc->Phase, "I") ||
        ILOC_STREQ(Assoc->Phase, "H") || ILOC_STREQ(Assoc->Phase, "O"))
        isfirst = -1;
/*
 *  observed travel time
 */
    obtime = Assoc->ArrivalTime - Hypocenter->Time;
/*
 *  predicted TT with corrections; partial derivatives if requested
 */
    if (iLoc_GetTravelTimePrediction(iLocConfig, Hypocenter, Assoc, StaLoc, ec,
                                TTInfo, TTtables, LocalTTInfo, LocalTTtables,
                                topo, iszd, isfirst, is2nd)) {
/*
 *      no valid TT prediction
 */
        if (isall && !Assoc->phaseFixed)
            strcpy(Assoc->Phase, "");
        Assoc->ttime = resid = ILOC_NULLVAL;
        Assoc->dtdd = Assoc->dtdh = Assoc->d2tdd = Assoc->d2tdh = 0.;
    }
    else {
/*
 *      time residual
 */
        resid = obtime - Assoc->ttime;
        if (Assoc->Slowness != ILOC_NULLVAL)
            Assoc->SlowRes = Assoc->Slowness - Assoc->dtdd;
    }
    if (iLocConfig->Verbose > 2) {
        fprintf(stderr, "    %6d %-8s ", Assoc->StaInd, Assoc->Phase);
        fprintf(stderr, "delta=%8.3f obsTT=%7.1f predTT=", Assoc->Delta, obtime);
        if (Assoc->ttime != ILOC_NULLVAL) fprintf(stderr, "%7.1f ", Assoc->ttime);
        else                         fprintf(stderr, "%7s ", "");
        if (resid != ILOC_NULLVAL) fprintf(stderr, "timres=%7.2f ", resid);
        else                  fprintf(stderr, "timres=%7s ", "");
        if (Assoc->BackAzimuth != ILOC_NULLVAL)
            fprintf(stderr, "azim=%7.2f seaz=%7.2f azimres=%7.2f ",
                    Assoc->BackAzimuth, Assoc->Seaz, Assoc->AzimRes);
        else
            fprintf(stderr, "azim=%7s seaz=%7s azimres=%7s ", "", "", "");
        if (Assoc->Slowness != ILOC_NULLVAL)
            fprintf(stderr, "slowness=%7.2f dtdd=%7.2f slowres=%7.2f d2tdd=%.2f\n",
                    Assoc->Slowness, Assoc->dtdd, Assoc->SlowRes, Assoc->d2tdd);
        else
            fprintf(stderr, "slowness=%7s dtdd=%7s slowres=%7s\n", "", "", "");
    }
    return resid;
}

/*
 *  Title:
 *     iLoc_GetTravelTimePrediction
 *  Synopsis:
 *     Returns the travel-time prediction with elevation, ellipticity and
 *         optional bounce-point corrections for a phase.
 *     Horizontal and vertical slownesses are calculated if requested.
 *     The first two indices (0 and 1) in the TT table structures are
 *        reserved for the composite first-arriving P and S TT tables.
 *        The isfirst flags controls the use of these tables.
 *        1: in this case the phaseids are ignored and it returns the TT for
 *           the first-arriving P or S (never actually used)
 *        0: phaseids are respected, but first_arriving P or S tables can
 *           be used to get a valid TT table value at local/regional crossover
 *           distances without reidentifying the phase during the subsequent
 *           iterations of the location algorithm;
 *       -1: do not use them at all (the behaviour in phase id routines).
 *  Input Arguments:
 *     iLocConfig    - pointer to ILOC_CONF structure
 *     Hypocenter    - pointer to ILOC_HYPO structure
 *     Assoc         - pointer to ILOC_ASSOC structure
 *     StaLoc        - pointer to ILOC_STA structure
 *     ec            - array of ILOC_EC_COEF structures
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - array of ILOC_TT_TABLE structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - array of ILOC_TT_TABLE structures
 *     topo          - ETOPO bathymetry/elevation matrix
 *     iszderiv      - calculate dtdh [0/1]?
 *     isfirst       - use first arriving composite tables?
 *                        1: ignore phaseid and use them
 *                        0: don't use but allow for fix at crossover distances
 *                       -1: don't use (used in phase id routines)
 *     is2nderiv     - calculate second derivatives [0/1]]
 *  Input Arguments:
 *     Assoc         - pointer to ILOC_ASSOC structure
 *  Return:
 *     Success/error
 *  Called by:
 *     PhaseIdentification, GetTTResidual, NAForwardProblem,
 *  Calls:
 *     iLoc_GetPhaseIndex, GetTravelTimeTableValue, TravelTimeCorrections
 */
int iLoc_GetTravelTimePrediction(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, ILOC_STA *StaLoc, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int iszderiv,
        int isfirst, int is2nderiv)
{
    int pind = 0, isdepthphase = 0, rstt_phase = 0;
    double ttim = -1, dtdd = 0., dtdh = 0., bpdel = 0., d2tdd = 0., d2tdh = 0.;
    double dtdlat = 0., dtdlon = 0.;
    double lat, lon, depth, slat, slon, elev;
    double Psurfvel = TTInfo->PSurfVel, Ssurfvel = TTInfo->SSurfVel;
    char phase[ILOC_PHALEN];
    strcpy(Assoc->Vmodel, "null");
/*
 *  invalid depth
 */
    if (Hypocenter->Depth < 0. || Hypocenter->Depth == ILOC_NULLVAL ||
        Hypocenter->Depth > TTInfo->MaxHypocenterDepth) {
        fprintf(stderr, "iLoc_GetTravelTimePrediction: invalid depth (%.2f)\n",
                Hypocenter->Depth);
        return ILOC_INVALID_DEPTH;
    }
/*
 *  invalid delta
 */
    if (Assoc->Delta < 0. || Assoc->Delta > 180. || Assoc->Delta == ILOC_NULLVAL) {
        fprintf(stderr, "iLoc_GetTravelTimePrediction: invalid delta (%.2f)\n",
                Assoc->Delta);
        return ILOC_INVALID_DELTA;
    }
/*
 *  get travel-time table index for phase
 */
    if (isfirst == 1) {
/*
 *      use composite first-arriving P or S travel-time tables
 */
        if      (toupper(Assoc->Phase[0]) == 'P') pind = 0;
        else if (toupper(Assoc->Phase[0]) == 'S') pind = 1;
        else                                      pind = -1;
    }
    else {
/*
 *      use travel-time tables specified for the phase
 */
        if (iLocConfig->UseLocalTT && Assoc->Delta <= iLocConfig->MaxLocalTTDelta)
            pind = iLoc_GetPhaseIndex(Assoc->Phase, LocalTTInfo);
        else
            pind = iLoc_GetPhaseIndex(Assoc->Phase, TTInfo);
    }
    if (pind < 0)
        return ILOC_INVALID_PHASE;
/*
 *  if depth phase, we need dtdd for bounce point correction
 */
    isdepthphase = 0;
    if (Assoc->Phase[0] == 'p' || Assoc->Phase[0] == 's')
        isdepthphase = 1;
    if (iLocConfig->UseLocalTT && Assoc->Delta <= iLocConfig->MaxLocalTTDelta) {
/*
 *      use local travel-time tables
 */
        Psurfvel = TTInfo->PSurfVel;
        Ssurfvel = TTInfo->SSurfVel;
        ttim = GetTravelTimeTableValue(Hypocenter->Depth, Assoc->Delta,
                    &LocalTTtables[pind], iszderiv, &dtdd, &dtdh, &bpdel,
                    is2nderiv, &d2tdd, &d2tdh);
/*
 *      couldn't get valid TT table value
 *         if we are allowed to use composite first-arriving travel-time
 *         tables, try them to deal with local/regional crossover ranges
 *         without renaming the phase
 */
        if (ttim < 0 && isfirst == 0) {
            if (ILOC_STREQ(Assoc->Phase, "Pg") ||
                ILOC_STREQ(Assoc->Phase, "Pb") ||
                ILOC_STREQ(Assoc->Phase, "Pn") ||
                ILOC_STREQ(Assoc->Phase, "P")) {
                ttim = GetTravelTimeTableValue(Hypocenter->Depth, Assoc->Delta,
                            &LocalTTtables[0], iszderiv, &dtdd, &dtdh, &bpdel,
                            is2nderiv, &d2tdd, &d2tdh);
            }
            if (ILOC_STREQ(Assoc->Phase, "Sg") ||
                ILOC_STREQ(Assoc->Phase, "Sb") ||
                ILOC_STREQ(Assoc->Phase, "Sn") ||
                ILOC_STREQ(Assoc->Phase, "S")  ||
                ILOC_STREQ(Assoc->Phase, "Lg")) {
                ttim = GetTravelTimeTableValue(Hypocenter->Depth, Assoc->Delta,
                            &LocalTTtables[1], iszderiv, &dtdd, &dtdh, &bpdel,
                            is2nderiv, &d2tdd, &d2tdh);
            }
        }
        if (ttim >= 0) strcpy(Assoc->Vmodel, LocalTTInfo->TTmodel);
    }
    else if (iLocConfig->UseRSTT) {
/*
 *      decide if the phase belongs to RSTT domain
 */
        if ((rstt_phase = isRSTT(iLocConfig, Assoc)) == 0) {
/*
 *          get travel-time prediction from TT table
 */
            ttim = GetTravelTimeTableValue(Hypocenter->Depth, Assoc->Delta,
                            &TTtables[pind], iszderiv, &dtdd, &dtdh, &bpdel,
                            is2nderiv, &d2tdd, &d2tdh);
/*
 *          couldn't get valid TT table value
 *             if we are allowed to use composite first-arriving travel-time
 *             tables, try them to deal with local/regional crossover ranges
 *             without renaming the phase
 */
            if (ttim < 0 && isfirst == 0) {
                if (Assoc->Delta < 23) {
                    if (ILOC_STREQ(Assoc->Phase, "Pg") ||
                        ILOC_STREQ(Assoc->Phase, "Pb") ||
                        ILOC_STREQ(Assoc->Phase, "Pn") ||
                        ILOC_STREQ(Assoc->Phase, "P")) {
                        ttim = GetTravelTimeTableValue(Hypocenter->Depth,
                                        Assoc->Delta, &TTtables[0],
                                        iszderiv, &dtdd, &dtdh, &bpdel,
                                        is2nderiv, &d2tdd, &d2tdh);
                    }
                    if (ILOC_STREQ(Assoc->Phase, "Sg") ||
                        ILOC_STREQ(Assoc->Phase, "Sb") ||
                        ILOC_STREQ(Assoc->Phase, "Sn") ||
                        ILOC_STREQ(Assoc->Phase, "S")  ||
                        ILOC_STREQ(Assoc->Phase, "Lg")) {
                        ttim = GetTravelTimeTableValue(Hypocenter->Depth,
                                        Assoc->Delta, &TTtables[1],
                                        iszderiv, &dtdd, &dtdh, &bpdel,
                                        is2nderiv, &d2tdd, &d2tdh);
                    }
                }
            }
            if (ttim >= 0) strcpy(Assoc->Vmodel, TTInfo->TTmodel);
        }
        else {
/*
 *          get travel-time prediction from RSTT
 */
            dtdd = dtdh = 0.;
            lat = ILOC_DEG2RAD * Hypocenter->Lat;
            lon = ILOC_DEG2RAD * Hypocenter->Lon;
            depth = Hypocenter->Depth;
            slat = ILOC_DEG2RAD * StaLoc->StaLat;
            slon = ILOC_DEG2RAD * StaLoc->StaLon;
            elev = -StaLoc->StaElevation / 1000.;
            if      (ILOC_STREQ(Assoc->Phase, "Pb")) strcpy(phase, "Pg");
            else if (ILOC_STREQ(Assoc->Phase, "Sb")) strcpy(phase, "Lg");
            else if (ILOC_STREQ(Assoc->Phase, "Sg")) strcpy(phase, "Lg");
            else                                strcpy(phase, Assoc->Phase);
            if (slbm_shell_createGreatCircle(phase, &lat, &lon, &depth,
                                             &slat, &slon, &elev))
                return ILOC_RSTT_ERROR;
            if (slbm_shell_getTravelTime(&ttim)) ttim = -999.;
            if (ttim >= 0.) {
/*
 *              derivatives
 */
                if (slbm_shell_get_dtt_dlat(&dtdlat)) dtdlat = ILOC_NULLVAL;
                if (slbm_shell_get_dtt_dlon(&dtdlon)) dtdlon = ILOC_NULLVAL;
                if (dtdlat < ILOC_NULLVAL && dtdlon < ILOC_NULLVAL)
                    dtdd = ILOC_SQRT(dtdlat * dtdlat + dtdlon * dtdlon) / ILOC_RAD2DEG;
                if (iszderiv) {
                    if (slbm_shell_get_dtt_ddepth(&dtdh)) dtdh = 0.;
                }
                strcpy(Assoc->Vmodel, "RSTT");
            }
        }
    }
    else {
/*
 *      get travel-time table value from global tables
 */
        ttim = GetTravelTimeTableValue(Hypocenter->Depth, Assoc->Delta,
                    &TTtables[pind], iszderiv, &dtdd, &dtdh, &bpdel,
                    is2nderiv, &d2tdd, &d2tdh);
/*
 *      couldn't get valid TT table value
 *         if we are allowed to use composite first-arriving travel-time
 *         tables, try them to deal with local/regional crossover ranges
 *         without renaming the phase
 */
        if (ttim < 0 && isfirst == 0) {
            if (Assoc->Delta < 23) {
                if (ILOC_STREQ(Assoc->Phase, "Pg") ||
                    ILOC_STREQ(Assoc->Phase, "Pb") ||
                    ILOC_STREQ(Assoc->Phase, "Pn") ||
                    ILOC_STREQ(Assoc->Phase, "P")) {
                    ttim = GetTravelTimeTableValue(Hypocenter->Depth,
                                    Assoc->Delta, &TTtables[0],
                                    iszderiv, &dtdd, &dtdh, &bpdel,
                                    is2nderiv, &d2tdd, &d2tdh);
                }
                if (ILOC_STREQ(Assoc->Phase, "Sg") ||
                    ILOC_STREQ(Assoc->Phase, "Sb") ||
                    ILOC_STREQ(Assoc->Phase, "Sn") ||
                    ILOC_STREQ(Assoc->Phase, "S")  ||
                    ILOC_STREQ(Assoc->Phase, "Lg")) {
                    ttim = GetTravelTimeTableValue(Hypocenter->Depth,
                                    Assoc->Delta, &TTtables[1],
                                    iszderiv, &dtdd, &dtdh, &bpdel,
                                    is2nderiv, &d2tdd, &d2tdh);
                }
            }
        }
        if (ttim >= 0) strcpy(Assoc->Vmodel, TTInfo->TTmodel);
    }
/*
 *  couldn't get valid travel time prediction
 */
    if (ttim < 0)
        return ILOC_NO_TRAVELTIME;
/*
 *  model predictions
 */
    Assoc->ttime = ttim;
    Assoc->dtdd = dtdd;
    if (iszderiv)     Assoc->dtdh = dtdh;
    if (isdepthphase) Assoc->bpdel = bpdel;
    if (is2nderiv) {
        Assoc->d2tdd = d2tdd;
        Assoc->d2tdh = d2tdh;
    }
/*
 *  elevation, ellipticity and bounce point corrections
 *  Note: RSTT includes all these
 */
    if (!rstt_phase)
        TravelTimeCorrections(iLocConfig, Hypocenter, Assoc, StaLoc, ec, topo,
                              Psurfvel, Ssurfvel);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     isRSTT
 *  Synopsis:
 *     Decides whether phase belongs to RSTT domain:
 *         delta < ILOC_MAX_RSTT_DIST && phase is
 *         Pn/Sn or first-arriving Pg/Pb/Sg/Sb if crustal phases to be used)
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Assoc      - pointer to ILOC_ASSOC structure
 *  Return:
 *     1 if phase qualifies for RSTT, 0 otherwise
 *  Called by:
 *     iLoc_GetTravelTimePrediction
 */
static int isRSTT(ILOC_CONF *iLocConfig, ILOC_ASSOC *Assoc)
{
    if (iLocConfig->UseRSTT == 0)
        return 0;
/*
 *  delta must be less than ILOC_MAX_RSTT_DIST
 */
    if (Assoc->Delta > ILOC_MAX_RSTT_DIST)
        return 0;
/*
 *  check if RSTT phase
 */
    if (!(ILOC_STREQ(Assoc->Phase, "Pn") || ILOC_STREQ(Assoc->Phase, "Sn") ||
          ILOC_STREQ(Assoc->Phase, "Pg") || ILOC_STREQ(Assoc->Phase, "Lg") ||
          ILOC_STREQ(Assoc->Phase, "Sg") ||
          ILOC_STREQ(Assoc->Phase, "Pb") || ILOC_STREQ(Assoc->Phase, "Sb"))) {
        return 0;
    }
    if ((ILOC_STREQ(Assoc->Phase, "Pn") || ILOC_STREQ(Assoc->Phase, "Sn")) &&
         !iLocConfig->UseRSTTPnSn)
        return 0;
    if ((ILOC_STREQ(Assoc->Phase, "Pg") || ILOC_STREQ(Assoc->Phase, "Lg") ||
         ILOC_STREQ(Assoc->Phase, "Sg") ||
         ILOC_STREQ(Assoc->Phase, "Pb") || ILOC_STREQ(Assoc->Phase, "Sb")) &&
         !iLocConfig->UseRSTTPgLg)
        return 0;
    return 1;
}

/*
 *  Title:
 *     GetTravelTimeTableValue
 *  Synopsis:
 *	   Returns TT table values for a given phase, depth and delta.
 *     Bicubic spline interpolation is used to get interpolated values.
 *     Horizontal and vertical slownesses are calculated if requested.
 *     Horizontal and vertical second time derivatives, needed for defining
 *         slownesses, are calculated if requested.
 *     Bounce point distance is calculated for depth phases.
 *  Input Arguments:
 *     depth     - depth
 *     delta     - delta
 *     TTtable   - pointer to ILOC_TT_TABLE structure
 *     iszderiv  - do we need dtdh [0/1]?
 *     is2nderiv - do we need d2tdd and d2tdh [0/1]?
 *  Output Arguments:
 *     dtdd  - interpolated dtdd (horizontal slowness, s/deg)
 *     dtdh  - interpolated dtdh (vertical slowness, s/km)
 *     bpdel - bounce point distance (deg) if depth phase
 *     d2tdd - interpolated second horizontal time derivative
 *     d2tdh - interpolated second vertical time derivative
 *  Return:
 *     TT table value for a phase at depth and delta or -1. on error
 *  Called by:
 *     iLoc_GetTravelTimePrediction
 *  Calls:
 *     iLoc_FloatBracket, iLoc_SplineCoeffs, iLoc_SplineInterpolation
 */
static double GetTravelTimeTableValue(double depth, double delta,
        ILOC_TT_TABLE *TTtable, int iszderiv, double *dtdd, double *dtdh,
        double *bpdel, int is2nderiv, double *d2tdd, double *d2tdh)
{
    int i, j, k, m, ilo, ihi, jlo, jhi, idel, jdep, ndep, ndel;
    int exactdelta = 0, exactdepth = 0, isbounce = 0;
    double ttim = -1., dydx = 0., d2ydx = 0., d2nd = 0.;
    double  x[ILOC_DELTASAMPLES],  z[ILOC_DEPTHSAMPLES], d2y[ILOC_DELTASAMPLES];
    double tx[ILOC_DELTASAMPLES], tz[ILOC_DEPTHSAMPLES], t2z[ILOC_DEPTHSAMPLES];
    double dx[ILOC_DELTASAMPLES], dz[ILOC_DEPTHSAMPLES];
    double hx[ILOC_DELTASAMPLES], hz[ILOC_DEPTHSAMPLES];
    double px[ILOC_DELTASAMPLES], pz[ILOC_DEPTHSAMPLES], tmp[ILOC_DELTASAMPLES];
    ndep = TTtable->ndep;
    ndel = TTtable->ndel;
    isbounce = TTtable->isbounce;
    *bpdel = 0.;
    *dtdd  = 0.;
    *dtdh  = 0.;
    *d2tdd  = 0.;
    *d2tdh  = 0.;
/*
 *  check if travel time table exists
 */
    if (ndel == 0)
        return ttim;
/*
 *  check for out of range depth or delta
 */
    if (depth < TTtable->depths[0] || depth > TTtable->depths[ndep - 1] ||
        delta < TTtable->deltas[0] || delta > TTtable->deltas[ndel - 1]) {
        return ttim;
    }
/*
 *  delta range
 */
    iLoc_FloatBracket(delta, ndel, TTtable->deltas, &ilo, &ihi);
    if (fabs(delta - TTtable->deltas[ilo]) < ILOC_DEPSILON) {
        idel = ilo;
        ihi = ilo + ILOC_DELTASAMPLES;
        exactdelta = 1;
    }
    else if (fabs(delta - TTtable->deltas[ihi]) < ILOC_DEPSILON) {
        idel = ihi;
        ihi++;
        ilo = ihi - ILOC_DELTASAMPLES;
        exactdelta = 1;
    }
    else if (ndel <= ILOC_DELTASAMPLES) {
        ilo = 0;
        ihi = ndel;
        idel = ilo;
    }
    else {
        idel = ilo;
        ilo = idel - ILOC_DELTASAMPLES / 2 + 1;
        ihi = idel + ILOC_DELTASAMPLES / 2 + 1;
        if (ilo < 0) {
            ilo = 0;
            ihi = ilo + ILOC_DELTASAMPLES;
        }
        if (ihi > ndel - 1) {
            ihi = ndel;
            ilo = ihi - ILOC_DELTASAMPLES;
        }
    }
/*
 *  depth range
 */
    iLoc_FloatBracket(depth, ndep, TTtable->depths, &jlo, &jhi);
    if (fabs(depth - TTtable->depths[jlo]) < ILOC_DEPSILON) {
        jdep = jlo;
        jhi = jlo + ILOC_DEPTHSAMPLES;
        exactdepth = 1;
    }
    else if (fabs(depth - TTtable->depths[jhi]) < ILOC_DEPSILON) {
        jdep = jhi;
        jhi++;
        jlo = jhi - ILOC_DEPTHSAMPLES;
        exactdepth = 1;
    }
    else if (ndep <= ILOC_DEPTHSAMPLES) {
        jlo = 0;
        jhi = ndep;
        jdep = jlo;
    }
    else {
        jdep = jlo;
        jlo = jdep - ILOC_DEPTHSAMPLES / 2 + 1;
        jhi = jdep + ILOC_DEPTHSAMPLES / 2 + 1;
        if (jlo < 0) {
            jlo = 0;
            jhi = jlo + ILOC_DEPTHSAMPLES;
        }
        if (jhi > ndep - 1) {
            jhi = ndep;
            jlo = jhi - ILOC_DEPTHSAMPLES;
        }
    }
    if (exactdelta && exactdepth) {
        if (is2nderiv == 0) {
            ttim  = TTtable->tt[idel][jdep];
            *dtdd  = TTtable->dtdd[idel][jdep];
            if (iszderiv)
                *dtdh  = TTtable->dtdh[idel][jdep];
            if (isbounce)
                *bpdel = TTtable->bpdel[idel][jdep];
            return ttim;
        }
    }
/*
 *  bicubic spline interpolation
 */
    for (k = 0, j = jlo; j < jhi; j++) {
/*
 *      no need for spline interpolation if exact delta
 */
        if (exactdelta && (is2nderiv == 0)) {
            if (TTtable->tt[idel][j] < 0)
                continue;
            z[k] = TTtable->depths[j];
            tz[k] = TTtable->tt[idel][j];
            if (isbounce)
                pz[k] = TTtable->bpdel[idel][j];
            dz[k] = TTtable->dtdd[idel][j];
            if (iszderiv)
                hz[k] = TTtable->dtdh[idel][j];
            k++;
        }
/*
 *      spline interpolation in delta
 */
        else {
            for (m = 0, i = ilo; i < ihi; i++) {
                if (TTtable->tt[i][j] < 0)
                    continue;
                x[m] = TTtable->deltas[i];
                tx[m] = TTtable->tt[i][j];
                if (isbounce)
                    px[m] = TTtable->bpdel[i][j];
                dx[m] = TTtable->dtdd[i][j];
                if (iszderiv)
                    hx[m] = TTtable->dtdh[i][j];
                m++;
            }
            if (m < ILOC_MINSAMPLES)
                continue;
            iLoc_SplineCoeffs(m, x, tx, d2y, tmp);
            z[k] = TTtable->depths[j];
            tz[k] = iLoc_SplineInterpolation(delta, m, x, tx, d2y, 0,
                                             &dydx, &d2ydx);
            if (is2nderiv) t2z[k] = d2ydx;
            if (isbounce) {
                iLoc_SplineCoeffs(m, x, px, d2y, tmp);
                pz[k] = iLoc_SplineInterpolation(delta, m, x, px, d2y, 0,
                                                 &dydx, &d2ydx);
            }
            iLoc_SplineCoeffs(m, x, dx, d2y, tmp);
            dz[k] = iLoc_SplineInterpolation(delta, m, x, dx, d2y, 0,
                                             &dydx, &d2ydx);
            if (iszderiv) {
                iLoc_SplineCoeffs(m, x, hx, d2y, tmp);
                hz[k] = iLoc_SplineInterpolation(delta, m, x, hx, d2y, 0,
                                                 &dydx, &d2ydx);
            }
            k++;
        }
    }
/*
 *  no valid data
 */
    if (k == 0) return ttim;
/*
 *  no need for Spline interpolation if exact depth
 */
    if (exactdepth) {
        if (is2nderiv == 0) {
            ttim = tz[0];
            if (isbounce)
                *bpdel = pz[0];
            *dtdd = dz[0];
            if (iszderiv)
                *dtdh = hz[0];
            return ttim;
        }
    }
/*
 *  insufficient data for Spline interpolation
 */
    if (k < ILOC_MINSAMPLES)
        return ttim;
/*
 *  Spline interpolation in depth
 */
    iLoc_SplineCoeffs(k, z, tz, d2y, tmp);
    ttim = iLoc_SplineInterpolation(depth, k, z, tz, d2y, 0, &dydx, &d2ydx);
    if (is2nderiv && d2ydx > -999.)
        *d2tdh = d2ydx;
    if (isbounce) {
        iLoc_SplineCoeffs(k, z, pz, d2y, tmp);
        *bpdel = iLoc_SplineInterpolation(depth, k, z, pz, d2y, 0, &dydx, &d2ydx);
    }
    iLoc_SplineCoeffs(k, z, dz, d2y, tmp);
    *dtdd = iLoc_SplineInterpolation(depth, k, z, dz, d2y, 0, &dydx, &d2ydx);
    if (is2nderiv) {
        iLoc_SplineCoeffs(k, z, t2z, d2y, tmp);
        d2nd = iLoc_SplineInterpolation(depth, k, z, t2z, d2y, 0, &dydx, &d2ydx);
        if (d2nd > -999.)
            *d2tdd = d2nd;
    }
    if (iszderiv) {
        iLoc_SplineCoeffs(k, z, hz, d2y, tmp);
        *dtdh = iLoc_SplineInterpolation(depth, k, z, hz, d2y, 0, &dydx, &d2ydx);
    }
    return ttim;
}

/*
 *  Title:
 *     TravelTimeCorrections
 *  Synopsis:
 *     Applies travel-time corrections to a predicted TT table value.
 *     Approximate geoid correction is calculated for Jeffreys-Bullen;
 *     otherwise the ak135 (Kennett and Gudmundsson, 1996) ellipticity
 *         correction is used.
 *     Bounce point correction is applied for depth phases, and for pwP,
 *        water depth correction is also calculated.
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assoc      - pointer to ILOC_ASSOC structure
 *     StaLoc     - pointer to ILOC_STA structure
 *     ec         - pointer to ILOC_EC_COEF structure
 *     topo       - ETOPO bathymetry/elevation matrix
 *     PSurfvel - P velocity in surface layer
 *     SSurfvel - S velocity in surface layer
 *  Input Arguments:
 *     Assoc      - pointer to ILOC_ASSOC structure
 *  Called by:
 *     iLoc_GetTravelTimePrediction
 *  Calls:
 *     GetEllipticityCorrection, GetElevationCorrection, GetBounceCorrection
 */
static void TravelTimeCorrections(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, ILOC_STA *StaLoc, ILOC_EC_COEF *ec, short int **topo,
        double Psurfvel, double Ssurfvel)
{
    double elevcorr = 0., ellipcorr = 0., bouncecorr = 0., watercorr = 0.;
    double f = (1. - ILOC_FLATTENING) * (1. - ILOC_FLATTENING);
    double ecolat = 0.;
/*
 *  keep approximate ellipticity corrections for JB (legacy)
 *  otherwise use ak135 ellipticity corrections (Kennett and Gudmundsson, 1996)
 */
    if (Hypocenter->Lat != ILOC_NULLVAL) {
/*
 *      ellipticity correction using Dziewonski and Gilbert (1976)
 *      formulation for ak135
 */
        ecolat = ILOC_PI2 - atan(f * tan(ILOC_DEG2RAD * Hypocenter->Lat));
        ellipcorr = GetEllipticityCorrection(ec, Assoc->Phase, ecolat,
                                Assoc->Delta, Hypocenter->Depth, Assoc->Esaz);
        Assoc->ttime += ellipcorr;

    }
/*
 *  elevation correction
 */
    elevcorr = GetElevationCorrection(Assoc, StaLoc, Psurfvel, Ssurfvel);
    Assoc->ttime += elevcorr;
/*
 *  depth phase bounce point correction
 */
    if (Assoc->Phase[0] == 'p' || Assoc->Phase[0] == 's') {
        bouncecorr = GetBounceCorrection(iLocConfig, Hypocenter, Assoc, topo,
                                         Psurfvel, Ssurfvel, &watercorr);
        Assoc->ttime += bouncecorr;
/*
 *      water depth correction for pwP
 */
        if (ILOC_STREQ(Assoc->Phase, "pwP"))
            Assoc->ttime += watercorr;
    }
}

/*
 *  Title:
 *     GetBounceCorrection
 *  Synopsis:
 *     Returns the correction for topography/bathymetry at the bounce point
 *        for a depth phase, as well as the water depth correction for pwP.
 *     Adopted from Bob Engdahl's libtau extensions.
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assoc      - pointer to ILOC_ASSOC structure
 *     topo       - ETOPO bathymetry/elevation matrix
 *     PSurfvel - P velocity in surface layer
 *     SSurfvel - S velocity in surface layer
 *  Output Arguments:
 *     tcorw - water travel time correction (water column)
 *  Return:
 *     tcorc - crust travel time correction (topography)
 *  Called by:
 *     TravelTimeCorrections
 *  Calls:
 *     iLoc_PointAtDeltaAzimuth, iLoc_GetEtopoCorrection
 */
static double GetBounceCorrection(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, short int **topo, double Psurfvel, double Ssurfvel,
        double *tcorw)
{
    int ips = 0;
    double tcor = 0., bp2 = 0., bpaz = 0., bplat = 0., bplon = 0.;
    *tcorw = 0.;
/*
 *  get geographic coordinates of bounce point
 */
    bp2 = Assoc->dtdd;
    bpaz = Assoc->Esaz;
    if (bp2 < 0.)    bpaz += 180.;
    if (bpaz > 360.) bpaz -= 360.;
    iLoc_PointAtDeltaAzimuth(Hypocenter->Lat, Hypocenter->Lon, Assoc->bpdel,
                             bpaz, &bplat, &bplon);
/*
 *  get topography/bathymetry correction for upgoing part of depth phase
 */
    if      (strncmp(Assoc->Phase, "pP", 2) == 0)  ips = 1;
    else if (strncmp(Assoc->Phase, "pwP", 3) == 0) ips = 1;
    else if (strncmp(Assoc->Phase, "pS", 2) == 0)  ips = 2;
    else if (strncmp(Assoc->Phase, "sP", 2) == 0)  ips = 2;
    else if (strncmp(Assoc->Phase, "sS", 2) == 0)  ips = 3;
    else                                           ips = 4;
    tcor = iLoc_GetEtopoCorrection(iLocConfig, ips, bp2, bplat, bplon, topo,
                              Psurfvel, Ssurfvel, tcorw);
    return tcor;
}

/*
 *  Title:
 *     iLoc_GetEtopoCorrection
 *  Synopsis:
 *     Calculates bounce point correction for depth phases.
 *     Calculates water depth correction for pwP if water column > 1.5 km.
 *     Uses Bob Engdahl's topography equations.
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     ips   - 1 if pP* wave, 2 if sP* or pS* wave, 3 if sS* wave
 *     rayp  - horizontal slowness [s/deg]
 *     bplat - latitude of surface reflection point
 *     bplon - longitude of surface reflection point
 *     topo  - ETOPO bathymetry/elevation matrix
 *     PSurfvel - P velocity in surface layer
 *     SSurfvel - S velocity in surface layer
 *  Output Arguments:
 *     tcorw - water travel time correction (water column)
 *  Return:
 *     tcorc - crust travel time correction (topography)
 *  Called by:
 *     GetBounceCorrection
 *  Calls:
 *     GetEtopoElevation
 */
double iLoc_GetEtopoCorrection(ILOC_CONF *iLocConfig, int ips, double rayp,
        double bplat, double bplon, short int **topo,
        double Psurfvel, double Ssurfvel, double *tcorw)
{
    double watervel = 1.5;                    /* P velocity in water [km/s] */
    double delr = 0., term = 0., term1 = 0., term2 = 0.;
    double bp2 = 0., tcorc = 0.;
    *tcorw = 0.;
/*
 *  get topography/bathymetry elevation
 */
    delr = GetEtopoElevation(iLocConfig, bplat, bplon, topo);
    if (fabs(delr) < ILOC_DEPSILON) return tcorc;
    bp2 = fabs(rayp) * ILOC_RAD2DEG / ILOC_EARTHRADIUS;
    if (ips == 1) {
/*
 *      pP*
 */
        term = Psurfvel * Psurfvel * bp2 * bp2;
        if (term > 1.) term = 1.;
        tcorc = 2. * delr * ILOC_SQRT(1. - term) / Psurfvel;
        if (delr < -1.5) {
/*
 *          water depth is larger than 1.5 km
 */
            term = watervel * watervel * bp2 * bp2;
            if (term > 1.) term = 1.;
            *tcorw = -2. * delr * ILOC_SQRT(1. - term) / watervel;
        }
    }
    else if (ips == 2) {
/*
 *      pS* or sP*
 */
        term1 = Psurfvel * Psurfvel * bp2 * bp2;
        if (term1 > 1.) term1 = 1.;
        term2 = Ssurfvel * Ssurfvel * bp2 * bp2;
        if (term2 > 1.) term2 = 1.;
        tcorc = delr * (ILOC_SQRT(1. - term1) / Psurfvel +
                        ILOC_SQRT(1. - term2) / Ssurfvel);
    }
    else if (ips == 3) {
/*
 *      sS*
 */
        term = Ssurfvel * Ssurfvel * bp2 * bp2;
        if (term > 1.) term = 1.;
        tcorc = 2. * delr * ILOC_SQRT(1. - term) / Ssurfvel;
    }
    else
        tcorc = 0.;
    return tcorc;
}

/*
 *  Title:
 *      GetEtopoElevation
 *  Synopsis:
 *      Returns ETOPO1 topography in kilometers for a lat, lon pair.
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     lat, lon   - latitude, longitude in degrees
 *     topo       - ETOPO bathymetry/elevation matrix
 *  Returns:
 *     elevation above sea level [km]
 *     topography above sea level is taken positive, below sea level negative.
 *  Called by:
 *     iLoc_GetEtopoCorrection
 */
static double GetEtopoElevation(ILOC_CONF *iLocConfig, double lat, double lon,
        short int **topo)
{
    int i, j, m, k1, k2;
    double a1, a2, lat2, lon2, lat1, lon1;
    double top, topo1, topo2, topo3, topo4;
/*
 *  bounding box
 */
    i = (int)((lon + 180.) / iLocConfig->EtopoRes);
    j = (int)((90. - lat) / iLocConfig->EtopoRes);
    lon1 = (double)(i) * iLocConfig->EtopoRes - 180.;
    lat1 = 90. - (double)(j) * iLocConfig->EtopoRes;
    lon2 = (double)(i + 1) * iLocConfig->EtopoRes - 180.;
    lat2 = 90. - (double)(j + 1) * iLocConfig->EtopoRes;
    k1 = i;
    k2 = i + 1;
    m = j;
    a1 = (lon2 - lon) / (lon2 - lon1);
    a2 = (lat2 - lat) / (lat2 - lat1);
/*
 *  take care of grid boundaries
 */
    if (i < 0 || i > iLocConfig->EtopoNlon - 2) {
        k1 = iLocConfig->EtopoNlon - 1;
        k2 = 0;
    }
    if (j < 0) {
        m = 0;
        a2 = 0.;
    }
    if (j > iLocConfig->EtopoNlat - 2) {
        m = iLocConfig->EtopoNlat - 2;
        a2 = 1.;
    }
/*
 *  interpolate
 */
    topo1 = (double)topo[m][k1];
    topo2 = (double)topo[m+1][k1];
    topo3 = (double)topo[m][k2];
    topo4 = (double)topo[m+1][k2];
    top = (1. - a1) * (1. - a2) * topo1 + a1 * (1. - a2) * topo3 +
          (1. - a1) * a2 * topo2 + a1 * a2 * topo4;
    return top / 1000.;
}

/*
 *  Title:
 *     GetElevationCorrection
 *  Synopsis:
 *     Calculates elevation correction for a station
 *  Input Arguments:
 *     Assoc    - pointer to ILOC_ASSOC structure
 *     StaLoc   - pointer to ILOC_STA structure
 *     PSurfvel - P velocity in surface layer
 *     SSurfvel - S velocity in surface layer
 *  Return:
 *     Travel time correction for station elevation
 *  Called by:
 *     TravelTimeCorrections
 *  Calls:
 *     GetLastLag
 */
static double GetElevationCorrection(ILOC_ASSOC *Assoc, ILOC_STA *StaLoc,
        double Psurfvel, double Ssurfvel)
{
    double elevcorr = 0., surfvel = 0.;
    int lastlag = 0;
/*
 *  unknown station elevation
 */
    if (StaLoc->StaElevation == ILOC_NULLVAL)
        return elevcorr;
/*
 *  find last lag of phase (P or S-type)
 */
    lastlag = GetLastLag(Assoc->Phase);
    if (lastlag == 1)                 /* last lag is P */
        surfvel = Psurfvel;
    else if (lastlag == 2)            /* last lag is S */
        surfvel = Ssurfvel;
    else                              /* invalid/unknown */
        return elevcorr;
/*
 *  elevation correction
 */
    elevcorr  = surfvel * (Assoc->dtdd / ILOC_DEG2KM);
    elevcorr *= elevcorr;
    if (elevcorr > 1.)
        elevcorr = 1./ elevcorr;
    elevcorr  = ILOC_SQRT(1. - elevcorr);
    elevcorr *= StaLoc->StaElevation / (1000. * surfvel);
    return elevcorr;
}


/*
 *  Title:
 *     GetLastLag
 *  Synopsis:
 *     Finds last lag of phase (P or S-type)
 *  Input Arguments:
 *     phase - phase name
 *  Called by:
 *    GetElevationCorrection
 *  Return:
 *     1 if P-type, 2 if S-type, 0 otherwise
 */
static int GetLastLag(char phase[]) {
    int lastlag = 0;
    int i, n;
    n = (int)strlen(phase);
    if (n == 0)
        return lastlag;
    for (i = n - 1; i > -1; i--) {
        if (isupper(phase[i])) {
            if (phase[i] == 'P') {
                lastlag = 1;
                break;
            }
            if (phase[i] == 'S') {
                lastlag = 2;
                break;
            }
        }
    }
    return lastlag;
}

/*
 *
 *  Title:
 *      GetEllipticityCorrection
 *  Synopsis:
 *      Calculates ellipticity correction for a phase.
 *      Dziewonski, A.M. and F. Gilbert, 1976,
 *         The effect of small, aspherical perturbations on travel times
 *         and a re-examination of the correction for ellipticity,
 *         Geophys., J. R. Astr. Soc., 44, 7-17.
 *      Kennett, B.L.N. and O. Gudmundsson, 1996,
 *         Ellipticity corrections for seismic phases,
 *         Geophys. J. Int., 127, 40-48.
 *      The routine uses the Dziewonski and Gilbert (1976) representation.
 *      The code is based on Brian Kennett's ellip.f
 *      The ellipticity corrections are found by linear interpolation
 *          in terms of values calculated for the ak135 model for a wide
 *          range of phases to match the output of the libtau software.
 *  Input Arguments:
 *      ec     - pointer to ILOC_EC_COEF structure
 *      phase  - phase
 *      ecolat - epicenter's co-latitude [rad]
 *      edist  - epicentral distance [deg]
 *      edepth - source depth [km]
 *      esaz   - event-to-station azimuth [deg]
 *  Return:
 *      tcor - ellipticity correction
 *  Called by:
 *     TravelTimeCorrections
 *  Calls:
 *      ECPhaseIndex, iLoc_BilinearInterpolation
 *  Notes:
 *      The available phases and the tabulated distance ranges are:
 *      (Kennett and Gudmundsson, 1996)
 *        #  Phase  ndist delta_min delta_max
 *
 *        0  Pup      3       0.0      10.0
 *        1  P       20       5.0     100.0     (Pg, Pb, Pn)
 *        2  Pdiff   11     100.0     150.0     (pPdiff, sPdiff)
 *        3  PKPab    7     145.0     175.0
 *        4  PKPbc    3     145.0     155.0     (PKPdiff)
 *        5  PKPdf   14     115.0     180.0
 *        6  PKiKP   32       0.0     155.0
 *        7  pP      17      20.0     100.0     (pwP, pPg, pPb, pPn, pPdiff)
 *        8  pPKPab   7     145.0     175.0
 *        9  pPKPbc   3     145.0     155.0     (pPKPdiff)
 *       10  pPKPdf  14     115.0     180.0
 *       11  pPKiKP  32       0.0     155.0
 *       12  sP      20       5.0     100.0     (sPg, sPb, sPn, sPdiff)
 *       13  sPKPab   7     145.0     175.0
 *       14  sPKPbc   3     145.0     155.0     (sPKPdiff)
 *       15  sPKPdf  14     115.0     180.0
 *       16  sPKiKP  32       0.0     155.0
 *       17  PcP     19       0.0      90.0
 *       18  ScP     13       0.0      60.0
 *       19  SKPab    3     130.0     140.0
 *       20  SKPbc    5     130.0     150.0
 *       21  SKPdf   15     110.0     180.0
 *       22  SKiKP   30       0.0     145.0
 *       23  PKKPab   5     235.0     255.0
 *       24  PKKPbc  11     235.0     285.0
 *       25  PKKPdf  31     210.0     360.0
 *       26  SKKPab   2     215.0     220.0
 *       27  SKKPbc  14     215.0     280.0
 *       28  SKKPdf  32     205.0     360.0
 *       29  PP      31      40.0     190.0     (PnPn)
 *       30  P'P'    26     235.0     360.0
 *       31  Sup      3       0.0      10.0
 *       32  S       19       5.0      95.0     (Sg, Sb, Sn)
 *       33  Sdiff   11     100.0     150.0     (pSdiff, sSdiff)
 *       34  SKSac   16      65.0     140.0
 *       35  SKSdf   16     105.0     180.0
 *       36  pS       9      60.0     100.0     (pSg, pSb, pSn, pSdiff)
 *       37  pSKSac  15      70.0     140.0
 *       38  pSKSdf  15     110.0     180.0
 *       39  sS      17      20.0     100.0     (sSg, sSb, sSn, sSdiff)
 *       40  sSKSac  16      65.0     140.0
 *       41  sSKSdf  15     110.0     180.0
 *       42  ScS     19       0.0      90.0
 *       43  PcS     13       0.0      60.0
 *       44  PKSab    3     130.0     140.0
 *       45  PKSbc    4     130.0     145.0
 *       46  PKSdf   15     110.0     180.0
 *       47  PKKSab   2     215.0     220.0
 *       48  PKKSbc  14     215.0     280.0
 *       49  PKKSdf  32     205.0     360.0
 *       50  SKKSac  43      65.0     275.0
 *       51  SKKSdf  33     200.0     360.0
 *       52  SS      31      40.0     190.0     (SnSn)
 *       53  S'S'    47     130.0     360.0
 *       54  SP      17      55.0     135.0     (SPn, SPb, SPg, SnP)
 *       55  PS      10      90.0     135.0     (PSn)
 *       56  PnS      6      65.0      90.0
 *
 *      The phase code is checked against the tabulated phases to find
 *          the entry point to the table corresponding to the class of arrivals.
 *      The presence of a value in the tables does not imply a physical
 *          arrival at all distance, depth combinations.  Where necessary,
 *          extrapolation has been used to ensure satisfactory results.
 *
 */
static double GetEllipticityCorrection(ILOC_EC_COEF *ec, char *phase,
        double ecolat, double delta, double depth, double esaz)
{
    int k = -1;
    double azim = 0., tau0 = 0., tau1 = 0., tau2 = 0.;
    double s3 = ILOC_SQRT(3.) / 2.;
    double tcor = 0.;
    double sc0 = 0., sc1 = 0., sc2 = 0.;
    azim = esaz * ILOC_DEG2RAD;
/*
 *  get corresponding index in ec
 */
    k = ECPhaseIndex(phase, delta);
/*
 *  no phase was found, return zero correction
 */
    if (k < 0) return tcor;
/*
 *  bilinear interpolation of tau coefficients of Dziewonski and Gilbert (1976)
 */
    tau0 = iLoc_BilinearInterpolation(delta, depth, ec[k].numDistanceSamples,
                    ec[k].numDepthSamples, ec[k].delta, ec[k].depth, ec[k].t0);
    tau1 = iLoc_BilinearInterpolation(delta, depth, ec[k].numDistanceSamples,
                    ec[k].numDepthSamples, ec[k].delta, ec[k].depth, ec[k].t1);
    tau2 = iLoc_BilinearInterpolation(delta, depth, ec[k].numDistanceSamples,
                    ec[k].numDepthSamples, ec[k].delta, ec[k].depth, ec[k].t2);
/*
 *  ellipticity correction: eqs. (22) and (26) of Dziewonski and Gilbert (1976)
 */
    sc0 = 0.25 * (1.0 + 3.0 * cos(2.0 * ecolat));
    sc1 = s3 * sin(2.0 * ecolat);
    sc2 = s3 * sin(ecolat) * sin(ecolat);
    tcor = sc0 * tau0 + sc1 * cos(azim) * tau1 + sc2 * cos(2. * azim) * tau2;
    return tcor;
}

/*
 *  Title:
 *      ECPhaseIndex
 *  Synopsis:
 *      Returns index to ILOC_EC_COEF struct based on phase and delta
 *  Input Arguments:
 *      phase - phase
 *      delta - delta
 *  Called by:
 *      GetEllipticityCorrection
 *
 *      Pup,    P,      Pdiff,  PKPab,  PKPbc,  PKPdf,  PKiKP,  pP,
 *      pPKPab, pPKPbc, pPKPdf, pPKiKP, sP,     sPKPab, sPKPbc, sPKPdf,
 *      sPKiKP, PcP,    ScP,    SKPab,  SKPbc,  SKPdf,  SKiKP,  PKKPab,
 *      PKKPbc, PKKPdf, SKKPab, SKKPbc, SKKPdf, PP,     P'P',   Sup,
 *      S,      Sdiff,  SKSac,  SKSdf,  pS,     pSKSac, pSKSdf, sS,
 *      sSKSac, sSKSdf, ScS,    PcS,    PKSab,  PKSbc,  PKSdf,  PKKSab,
 *      PKKSbc, PKKSdf, SKKSac, SKKSdf, SS,     S'S',   SP,     PS,
 *      PnS
 */
static int ECPhaseIndex(char *phase, double delta)
{
    if (ILOC_STREQ(phase, "Pup") ||
        ILOC_STREQ(phase, "p"))      return 0;
    if (ILOC_STREQ(phase, "P")    ||
        ILOC_STREQ(phase, "Pg")   ||
        ILOC_STREQ(phase, "Pb")   ||
        ILOC_STREQ(phase, "Pn")   ||
        ILOC_STREQ(phase, "PgPg") ||
        ILOC_STREQ(phase, "PbPb"))   return 1;
    if (ILOC_STREQ(phase, "Pdiff") ||
        ILOC_STREQ(phase, "Pdif"))   return 2;
    if (ILOC_STREQ(phase, "PKPab"))  return 3;
    if (ILOC_STREQ(phase, "PKPbc"))  return 4;
    if (ILOC_STREQ(phase, "PKPdf"))  return 5;
    if (ILOC_STREQ(phase, "PKiKP"))  return 6;
    if (ILOC_STREQ(phase, "pP")  ||
        ILOC_STREQ(phase, "pPg") ||
        ILOC_STREQ(phase, "pPb") ||
        ILOC_STREQ(phase, "pPn") ||
        ILOC_STREQ(phase, "pwP"))    return 7;
    if (ILOC_STREQ(phase, "pPKPab")) return 8;
    if (ILOC_STREQ(phase, "pPKPbc")) return 9;
    if (ILOC_STREQ(phase, "pPKPdf")) return 10;
    if (ILOC_STREQ(phase, "pPKiKP")) return 11;
    if (ILOC_STREQ(phase, "sP")  ||
        ILOC_STREQ(phase, "sPg") ||
        ILOC_STREQ(phase, "sPb") ||
        ILOC_STREQ(phase, "sPn"))    return 12;
    if (ILOC_STREQ(phase, "sPKPab")) return 13;
    if (ILOC_STREQ(phase, "sPKPbc")) return 14;
    if (ILOC_STREQ(phase, "sPKPdf")) return 15;
    if (ILOC_STREQ(phase, "sPKiKP")) return 16;
    if (ILOC_STREQ(phase, "PcP"))    return 17;
    if (ILOC_STREQ(phase, "ScP"))    return 18;
    if (ILOC_STREQ(phase, "SKPab"))  return 19;
    if (ILOC_STREQ(phase, "SKPbc"))  return 20;
    if (ILOC_STREQ(phase, "SKPdf"))  return 21;
    if (ILOC_STREQ(phase, "SKiKP"))  return 22;
    if (ILOC_STREQ(phase, "PKKPab")) return 23;
    if (ILOC_STREQ(phase, "PKKPbc")) return 24;
    if (ILOC_STREQ(phase, "PKKPdf")) return 25;
    if (ILOC_STREQ(phase, "SKKPab")) return 26;
    if (ILOC_STREQ(phase, "SKKPbc")) return 27;
    if (ILOC_STREQ(phase, "SKKPdf")) return 28;
    if (ILOC_STREQ(phase, "PP") ||
        ILOC_STREQ(phase, "PnPn"))   return 29;
    if (ILOC_STREQ(phase, "P'P'ab") ||
        ILOC_STREQ(phase, "P'P'bc") ||
        ILOC_STREQ(phase, "P'P'df")) return 30;
    if (ILOC_STREQ(phase, "Sup") ||
        ILOC_STREQ(phase, "s"))      return 31;
    if (ILOC_STREQ(phase, "S")    ||
        ILOC_STREQ(phase, "Sg")   ||
        ILOC_STREQ(phase, "Sb")   ||
        ILOC_STREQ(phase, "Sn")   ||
        ILOC_STREQ(phase, "SgSg") ||
        ILOC_STREQ(phase, "SbSb"))   return 32;
    if (ILOC_STREQ(phase, "Sdiff") ||
        ILOC_STREQ(phase, "Sdif"))   return 33;
    if (ILOC_STREQ(phase, "SKSac"))  return 34;
    if (ILOC_STREQ(phase, "SKSdf"))  return 35;
    if (ILOC_STREQ(phase, "pS")  ||
        ILOC_STREQ(phase, "pSg") ||
        ILOC_STREQ(phase, "pSb") ||
        ILOC_STREQ(phase, "pSn"))    return 36;
    if (ILOC_STREQ(phase, "pSKSac")) return 37;
    if (ILOC_STREQ(phase, "pSKSdf")) return 38;
    if (ILOC_STREQ(phase, "sS")  ||
        ILOC_STREQ(phase, "sSg") ||
        ILOC_STREQ(phase, "sSb") ||
        ILOC_STREQ(phase, "sSn"))    return 39;
    if (ILOC_STREQ(phase, "sSKSac")) return 40;
    if (ILOC_STREQ(phase, "sSKSdf")) return 41;
    if (ILOC_STREQ(phase, "ScS"))    return 42;
    if (ILOC_STREQ(phase, "PcS"))    return 43;
    if (ILOC_STREQ(phase, "PKSab"))  return 44;
    if (ILOC_STREQ(phase, "PKSbc"))  return 45;
    if (ILOC_STREQ(phase, "PKSdf"))  return 46;
    if (ILOC_STREQ(phase, "PKKSab")) return 47;
    if (ILOC_STREQ(phase, "PKKSbc")) return 48;
    if (ILOC_STREQ(phase, "PKKSdf")) return 49;
    if (ILOC_STREQ(phase, "SKKSac")) return 50;
    if (ILOC_STREQ(phase, "SKKSdf")) return 51;
    if (ILOC_STREQ(phase, "SS") ||
        ILOC_STREQ(phase, "SnSn"))   return 52;
    if (ILOC_STREQ(phase, "S'S'ac") ||
        ILOC_STREQ(phase, "S'S'df")) return 53;
    if (ILOC_STREQ(phase, "SP")  ||
        ILOC_STREQ(phase, "SPg") ||
        ILOC_STREQ(phase, "SPb") ||
        ILOC_STREQ(phase, "SPn") ||
        ILOC_STREQ(phase, "SnP"))    return 54;
    if (ILOC_STREQ(phase, "PS")  ||
        ILOC_STREQ(phase, "PSg") ||
        ILOC_STREQ(phase, "PSb") ||
        ILOC_STREQ(phase, "PSn"))    return 55;
    if (ILOC_STREQ(phase, "PnS"))    return 56;

    if (delta <= 100) {
        if (ILOC_STREQ(phase, "pPdiff") || ILOC_STREQ(phase, "pPdif")) return 7;
        if (ILOC_STREQ(phase, "sPdiff") || ILOC_STREQ(phase, "sPdif")) return 12;
        if (ILOC_STREQ(phase, "pSdiff") || ILOC_STREQ(phase, "pSdif")) return 36;
        if (ILOC_STREQ(phase, "sSdiff") || ILOC_STREQ(phase, "sSdif")) return 39;
    }
    if (delta > 100) {
        if (ILOC_STREQ(phase, "pPdiff") || ILOC_STREQ(phase, "sPdiff") ||
            ILOC_STREQ(phase, "pPdif")  || ILOC_STREQ(phase, "sPdif")) return 2;
        if (ILOC_STREQ(phase, "pSdiff") || ILOC_STREQ(phase, "sSdiff") ||
            ILOC_STREQ(phase, "pSdif")  || ILOC_STREQ(phase, "sSdif")) return 33;
    }
    if (delta <= 165) {
        if (ILOC_STREQ(phase, "PKPdiff")  || ILOC_STREQ(phase, "PKPdif"))  return 4;
        if (ILOC_STREQ(phase, "pPKPdiff") || ILOC_STREQ(phase, "pPKPdif")) return 9;
        if (ILOC_STREQ(phase, "sPKPdiff") || ILOC_STREQ(phase, "sPKPdif")) return 14;
    }
    return -1;
}

/*  EOF  */

