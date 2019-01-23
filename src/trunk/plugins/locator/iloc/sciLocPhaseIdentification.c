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
 *    iLoc_IdentifyPhases
 *    iLoc_ReIdentifyPhases
 *    iLoc_GetNumDef
 *    iLoc_SortAssocs
 *    iLoc_SortAssocsNN
 */

/*
 * Local functions:
 *    PhaseIdentification
 *    isFirstP
 *    isFirstS
 *    GetPriorMeasurementError
 */
static void PhaseIdentification(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int isfirst);
static int isFirstP(char *phase, char *mappedphase, ILOC_PHASEIDINFO *PhaseIdInfo);
static int isFirstS(char *phase, char *mappedphase, ILOC_PHASEIDINFO *PhaseIdInfo);
static void GetPriorMeasurementError(ILOC_ASSOC *Assocs,
        ILOC_PHASEIDINFO *PhaseIdInfo, double SigmaThreshold);

/*
 *  Title:
 *     iLoc_IdentifyPhases
 *  Synopsis:
 *     Identifies phases with respect to the initial hypocentre.
 *     Maps reported phase ids to IASPEI standard phase names.
 *     Uses phase dependent information in PhaseIdInfo structure.
 *         PhaseMap - list of possible reported phases with their
 *                    corresponding IASPEI phase id.
 *     Sets unrecognized reported phase names to null.
 *     Assumes that unidentified reported first-arriving phase is P.
 *     Identifies phases within a reading and marks first-arriving P and S.
 *     Sets time-defining flags and a priori measurement errors.
 *  Input Arguments:
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
 *  Output Arguments:
 *     Assocs        - array of ILOC_ASSOC structures
 *     is2nderiv     - calculate second derivatives flag
 *  Called by:
 *     iLoc_Locator, ResidualsForFixedHypocenter
 *  Calls:
 *     PhaseIdentification, GetPriorMeasurementError
 */
void iLoc_IdentifyPhases(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int *is2nderiv)
{
    int i, j, np, isfirst = -1;
/*
 *  map phases to IASPEI phase names
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
/*
 *      initializations
 */
        strcpy(Assocs[i].prevPhase, "");
        Assocs[i].prevTimedef = Assocs[i].Timedef;
        Assocs[i].prevAzimdef = Assocs[i].Azimdef;
        Assocs[i].prevSlowdef = Assocs[i].Slowdef;
        Assocs[i].firstP = Assocs[i].firstS = Assocs[i].duplicate = 0;
        strcpy(Assocs[i].Vmodel, "null");
/*
 *      copy phasehint to phase if phase name is fixed by analysts
 */
        if (Assocs[i].phaseFixed) {
/*
 *          copy phasehint to phase if phase name is fixed by analysts
 */
            strcpy(Assocs[i].Phase, Assocs[i].PhaseHint);
        }
        else {
/*
 *          initialize phase name
 */
            strcpy(Assocs[i].Phase, "");
/*
 *          assume that unknown initial phases are P
 */
            if (ILOC_STREQ(Assocs[i].PhaseHint, "") && Assocs[i].initialPhase)
                strcpy(Assocs[i].Phase, "P");
/*
 *          map reported phase names to IASPEI standard
 */
            for (j = 0; j < PhaseIdInfo->numPhaseMap; j++) {
                if (ILOC_STREQ(Assocs[i].PhaseHint, PhaseIdInfo->PhaseMap[j].ReportedPhase)) {
                    strcpy(Assocs[i].Phase, PhaseIdInfo->PhaseMap[j].Phase);
                    break;
                }
            }
        }
    }
/*
 *  identify first arriving P and S in a reading
 */
    for (i = 0; i < Hypocenter->numReading; i++) {
        np = rdindx[i].start + rdindx[i].npha;
        for (j = rdindx[i].start; j < np; j++) {
            if (Assocs[j].Phase[0] == 'P' ||
                (islower(Assocs[j].Phase[0]) &&
                 (Assocs[j].Phase[1] == 'P' || Assocs[j].Phase[1] == 'w'))) {
                Assocs[j].firstP = 1;
                break;
            }
        }
        for (j = rdindx[i].start; j < np; j++) {
            if (Assocs[j].Phase[0] == 'S' || ILOC_STREQ(Assocs[j].Phase, "Lg") ||
                (islower(Assocs[j].Phase[0]) && Assocs[j].Phase[1] == 'S')) {
                Assocs[j].firstS = 1;
                break;
            }
        }
    }
/*
 *  identify phases within a reading
 */
    for (i = 0; i < Hypocenter->numReading; i++) {
        PhaseIdentification(iLocConfig, Hypocenter, Assocs, StaLocs, &rdindx[i],
                            PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo,
                            LocalTTtables, topo, isfirst);
    }
/*
 *  clear current GreatCircle object and the pool of CrustalProfile objects
 */
    if (iLocConfig->UseRSTT)
         slbm_shell_clear();
/*
 *  set defining flags and get prior measurement errors
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (ILOC_STREQ(Assocs[i].Phase, "")) {
/*
 *          unidentified phases are made non-defining
 */
            Assocs[i].Timedef = 0;
            Assocs[i].Slowdef = 0;
            Assocs[i].Azimdef = 0;
        }
        else {
/*
 *          set defining flags and get prior measurement errors
 */
            GetPriorMeasurementError(&Assocs[i], PhaseIdInfo,
                                     iLocConfig->SigmaThreshold);
            if (fabs(Assocs[i].d2tdd) < ILOC_DEPSILON)
                Assocs[i].Slowdef = 0;
        }
        strcpy(Assocs[i].prevPhase, Assocs[i].Phase);
    }
    iLoc_GetNumDef(Hypocenter, Assocs);
    if (iLocConfig->Verbose > 1) {
        fprintf(stderr, "numTimedef=%d numAzimdef=%d numSlowdef=%d\n",
                Hypocenter->numTimedef, Hypocenter->numAzimdef,
                Hypocenter->numSlowdef);
        iLoc_PrintPhases(Hypocenter->numPhase, Assocs);
    }
    if (Hypocenter->numTimedef > iLocConfig->MinNdefPhases) {
        *is2nderiv = 0;
        if (Hypocenter->numSlowdef) {
            if (iLocConfig->Verbose) {
                fprintf(stderr, "There is enough time defining phases ");
                fprintf(stderr, "to ignore slowness observations\n");
            }
            Hypocenter->numDef -= Hypocenter->numSlowdef;
        }
        for (i = 0; i < Hypocenter->numPhase; i++)
            Assocs[i].Slowdef = 0;
        Hypocenter->numSlowdef = 0;
    }
    if (iLocConfig->Verbose)
        fprintf(stderr, "Total number of defining observations=%d\n",
                Hypocenter->numDef);
}

/*
 *  Title:
 *     iLoc_ReIdentifyPhases
 *  Synopsis:
 *     Reidentifies phases after NA search is completed and/or
 *         if depth crosses Moho/Conrad discontinuity between iterations.
 *     At this point phase names are already mapped to IASPEI standards.
 *     Identifies phases within a reading and marks first-arriving P and S.
 *     Sets time-defining flags and a priori measurement errors for phases
 *         whose names have been changed.
 *  Input Arguments:
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
 *     is2nderiv     - calculate second derivatives flag
 *     isfirst       - use first arriving composite tables?
 *                        1: ignore phaseid and use them
 *                        0: don't use but allow for fix at crossover distances
 *                       -1: don't use (used in phase id routines)
 *  Output Arguments:
 *     Assocs        - array of ILOC_ASSOC structures
 *  Return:
 *     1 on phasename changes, 0 otherwise
 *  Called by:
 *     iLoc_Locator, LocateEvent
 *  Calls:
 *     PhaseIdentification, GetPriorMeasurementError, iLoc_GetNumDef
 */
int iLoc_ReIdentifyPhases(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int is2nderiv)
{
    int i, j, np, isphasechange = 0, isfirst = -1;
    for (i = 0; i < Hypocenter->numPhase; i++)
        Assocs[i].firstP = Assocs[i].firstS = Assocs[i].duplicate = 0;
/*
 *  identify first arriving P and S in a reading
 */
    for (i = 0; i < Hypocenter->numReading; i++) {
        np = rdindx[i].start + rdindx[i].npha;
        for (j = rdindx[i].start; j < np; j++) {
            if (Assocs[j].Phase[0] == 'P' ||
                (islower(Assocs[j].Phase[0]) &&
                 (Assocs[j].Phase[1] == 'P' || Assocs[j].Phase[1] == 'w'))) {
                Assocs[j].firstP = 1;
                break;
            }
        }
        for (j = rdindx[i].start; j < np; j++) {
            if (Assocs[j].Phase[0] == 'S' || ILOC_STREQ(Assocs[j].Phase, "Lg") ||
                (islower(Assocs[j].Phase[0]) && Assocs[j].Phase[1] == 'S')) {
                Assocs[j].firstS = 1;
                break;
            }
        }
    }
/*
 *  identify phases within a reading
 */
    for (i = 0; i < Hypocenter->numReading; i++) {
        PhaseIdentification(iLocConfig, Hypocenter, Assocs, StaLocs, &rdindx[i],
                            PhaseIdInfo, ec, TTInfo, TTtables, LocalTTInfo,
                            LocalTTtables, topo, isfirst);
    }
/*
 *  clear current GreatCircle object and the pool of CrustalProfile objects
 */
    if (iLocConfig->UseRSTT)
        slbm_shell_clear();
/*
 *  Check if any phase ids have been changed
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (ILOC_STREQ(Assocs[i].Phase, "")) {
/*
 *          unidentified phases are made non-defining
 */
            Assocs[i].Timedef = 0;
            Assocs[i].Slowdef = 0;
            Assocs[i].Azimdef = 0;
        }
        else {
/*
 *          set defining flags and get prior measurement errors
 */
            GetPriorMeasurementError(&Assocs[i], PhaseIdInfo,
                                     iLocConfig->SigmaThreshold);
            if (!is2nderiv || fabs(Assocs[i].d2tdd) < ILOC_DEPSILON)
                Assocs[i].Slowdef = 0;
        }
        if (strcmp(Assocs[i].Phase, Assocs[i].prevPhase)) {
            isphasechange = 1;
            strcpy(Assocs[i].prevPhase, Assocs[i].Phase);
        }
    }
    iLoc_GetNumDef(Hypocenter, Assocs);
    return isphasechange;
}

/*
 *  Title:
 *     PhaseIdentification
 *  Synopsis:
 *     Identifies phases in a reading according to their time residuals.
 *     At this point phase names are already mapped to IASPEI standards.
 *     Uses phase dependent information in PhaseIdInfo structure.
 *        PhaseWithoutResidual - list of phases that don't get residuals, i.e.
 *                         never used in the location (e.g. amplitude phases)
 *        AllowablePhases - list of phases to which reported phases can be
 *                          renamed
 *        AllowableFirstP - list of phases to which reported first-arriving
 *                          P phases can be renamed
 *        OptionalFirstP  - additional list of phases to which reported
 *                          first-arriving P phases can be renamed
 *        AllowableFirstS - list of phases to which reported first-arriving
 *                          S phases can be renamed
 *        OptionalFirstS  - additional list of phases to which reported
 *                          first-arriving S phases can be renamed
 *     Skips phases that do not get residuals.
 *        The list of no-residual phases contains the list of phases for which
 *        residuals will not be calculated, such as amplitude phases or IASPEI
 *        phases for which no travel-time tables exist. These phases are not
 *        used in the location.
 *     Considers only P and S type and I, H, O phases.
 *        phase type is determined by the first leg of the phase id;
 *        for depth phases phase type is determined by the second letter.
 *     Does not reidentify phases fixed by analyst (phase_fixed flag).
 *     For exact duplicates (|dT| < 0.01) keeps the first non-null phaseid.
 *     Checks if the reported phase is in the list of allowable phases.
 *        The list of allowable phases were introduced to prevent the locator
 *        to rename phases to unlikely 'exotic' phases, just because a
 *        travel-time prediction fits better the observed travel-time. For
 *        instance, we do not want to reidentify a reported Sn as SgSg, SPn or
 *        horribile dictu, sSn. Recall that phases may suffer from picking
 *        errors or the observed travel-times may reflect true 3D structures
 *        not modeled by the velocity model. Introducing the list of
 *        allowable phases helps to maintain the sanity of the bulletin and
 *        mitigates the risk of misidentifying phases. However, if a reported
 *        phase is not in the list of allowable phases, it is temporarily
 *        added to the list accepting the fact that station operators may
 *        confidently pick later phases. In other words, exotic phase names
 *        can appear in the final bulletin only if they were reported as such.
 *     Loops through the (possibly amended) list of allowable phases and
 *        calculates the time residual.
 *        Does not allow renaming a P-type phase to S-type, and vice versa.
 *        Does not allow allow S(*) phases to be renamed to depth phases (s*).
 *        Does not allow for repeating phaseids in a reading.
 *        Further restrictions apply to first-arriving P and S phases.
 *            First-arriving P and S phases can be identified as those in the
 *            list of allowable first-arriving P and S phases. Occasionally a
 *            station operator may not report the true first-arriving phase
 *            due to high noise conditions. To account for this situation the
 *            list of optional first-arriving P and S phases is also checked.
 *        Keeps track of the phaseid with the smallest residual.
 *     Sets the phase id to the phase in the allowable phase list with the
 *        smallest residual.
 *     If no eligible phase is found, leaves the phase unidentified.
 *  Input Arguments:
 *     iLocConfig    - pointer to  ILOC_CONF structure
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
 *     isfirst       - use first arriving composite tables?
 *                        1: ignore phaseid and use them
 *                        0: don't use but allow for fix at crossover distances
 *                       -1: don't use (used in phase id routines)
 *  Output Arguments:
 *     Assocs        - array of ILOC_ASSOC structures
 *  Called by:
 *     iLoc_IdentifyPhases, iLoc_ReIdentifyPhases
 *  Calls:
 *     iLoc_GetTravelTimePrediction, isFirstP, isFirstS
 */
static void PhaseIdentification(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int isfirst)
{
    double resid, bigres = 60., min_resid, dtdd = ILOC_NULLVAL;
    double ttime = ILOC_NULLVAL, pPttime = ILOC_NULLVAL;
    char candidate_phase[ILOC_PHALEN], mappedphase[ILOC_PHALEN], phase[ILOC_PHALEN];
    char Vmodel[ILOC_VALLEN];
    int isS = 0, isP = 0, isI = 0, isH = 0, iss = 0, isp = 0;
    int ptype = 0, stype = 0, ttype = 0;
    int j, k, m, n, ii, sind, isseen = 0, npha = 0;
    strcpy(Vmodel, "null");
/*
 *  loop over phases in this reading
 */
    n = rdindx->start + rdindx->npha;
    for (m = 0, k = rdindx->start; k < n; m++, k++) {
        Assocs[k].TimeRes = Assocs[k].AzimRes = Assocs[k].SlowRes = ILOC_NULLVAL;
        Assocs[k].d2tdd = Assocs[k].d2tdh = 0.;
/*
 *      skip phases that don't get residuals (amplitudes etc)
 */
        for (j = 0; j < PhaseIdInfo->numPhaseWithoutResidual; j++) {
            if (ILOC_STREQ(Assocs[k].Phase,
                           PhaseIdInfo->PhaseWithoutResidual[j].Phase))
                break;
        }
        if (j != PhaseIdInfo->numPhaseWithoutResidual) {
            Assocs[k].Timedef = 0;
            Assocs[k].Slowdef = 0;
            Assocs[k].Azimdef = 0;
            continue;
        }
        min_resid = bigres + 10.;
        resid = ILOC_NULLVAL;
/*
 *      phase type is determined by the first leg of the phase id
 */
        isS = isP = isI = iss = isp = ptype = stype = 0;
        if (Assocs[k].Phase[0] == 'P') isP = 1;
        if (Assocs[k].Phase[0] == 'S' || ILOC_STREQ(Assocs[k].Phase, "Lg")) isS = 1;
/*
 *      for depth phases phase type is determined by the second letter
 */
        if (islower(Assocs[k].Phase[0])) {
            if (Assocs[k].Phase[1] == 'P' || Assocs[k].Phase[1] == 'w') isp = 1;
            if (Assocs[k].Phase[1] == 'S') iss = 1;
        }
/*
 *      Infrasound phases
 */
        if (ILOC_STREQ(Assocs[k].Phase, "I")) {
            isI = 1;
            Assocs[k].phaseFixed = 1;
        }
/*
 *      Hydroacoustic phases
 */
        if (Assocs[k].Phase[0] == 'H' || Assocs[k].Phase[0] == 'O') {
            isH = 1;
            Assocs[k].phaseFixed = 1;
        }
/*
 *      consider only P or S-type or infrasound/hydro phases
 */
        if (!(isP || isp || isS || iss || isI || isH)) continue;
        if (isP || isp) ptype = 1;
        if (isS || iss) stype = 1;
/*
 *      station index
 */
        sind = Assocs[k].StaInd;
/*
 *      do not reidentify fixed phase names
 */
        if (Assocs[k].phaseFixed || iLocConfig->DoNotRenamePhases) {
/*
 *          get travel time for phase
 */
            if (iLoc_GetTravelTimePrediction(iLocConfig, Hypocenter, &Assocs[k],
                            &StaLocs[sind], ec, TTInfo, TTtables, LocalTTInfo,
                            LocalTTtables, topo, 0, isfirst, 0)) {
                Assocs[k].ttime = ILOC_NULLVAL;
                Assocs[k].TimeRes = ILOC_NULLVAL;
                Assocs[k].SlowRes = ILOC_NULLVAL;
            }
            else {
                Assocs[k].TimeRes = Assocs[k].ArrivalTime -
                                    Hypocenter->Time - Assocs[k].ttime;
                if (Assocs[k].BackAzimuth != ILOC_NULLVAL)
                    Assocs[k].AzimRes = Assocs[k].BackAzimuth - Assocs[k].Seaz;
                if (Assocs[k].Slowness != ILOC_NULLVAL)
                    Assocs[k].SlowRes = Assocs[k].Slowness - Assocs[k].dtdd;
            }
        }
/*
 *      phase is not fixed by analysts
 */
        else {
/*
 *          deal with duplicates: keep the first non-null phaseid
 *              phases ordered by time within a reading
 */
            if (m) {
                if (fabs(Assocs[k].ArrivalTime - Assocs[k-1].ArrivalTime) < 0.05) {
/*
 *                  if previous phaseid is null, rename it
 */
                    if (ILOC_STREQ(Assocs[k-1].Phase, "")) {
                        strcpy(Assocs[k-1].Phase, Assocs[k].Phase);
                        Assocs[k-1].ttime = Assocs[k].ttime;
                        Assocs[k-1].TimeRes = Assocs[k].TimeRes;
                        Assocs[k-1].dtdd = Assocs[k].dtdd;
                        Assocs[k].duplicate = 1;
                        continue;
                    }
/*
 *                  otherwise use previous phase
 */
                    else {
                        for (j = 0; j < PhaseIdInfo->numPhaseWithoutResidual; j++) {
                            if (ILOC_STREQ(Assocs[k-1].Phase,
                                      PhaseIdInfo->PhaseWithoutResidual[j].Phase))
                                break;
                        }
/*
 *                      if it is a no-residual phase, leave it alone
 */
                        if (j != PhaseIdInfo->numPhaseWithoutResidual)
                            continue;
                        strcpy(Assocs[k].Phase, Assocs[k-1].Phase);
                        Assocs[k].ttime = Assocs[k-1].ttime;
                        Assocs[k].TimeRes = Assocs[k-1].TimeRes;
                        Assocs[k].dtdd = Assocs[k-1].dtdd;
                        Assocs[k].duplicate = 1;
                        continue;
                    }
                }
            }
            strcpy(mappedphase, Assocs[k].Phase);
/*
 *
 *          see if mapped phase is in the allowable phase list
 *
 */
            npha = PhaseIdInfo->numAllowablePhases;
            for (j = 0; j < PhaseIdInfo->numAllowablePhases; j++) {
                if (ILOC_STREQ(mappedphase, PhaseIdInfo->AllowablePhases[j].Phase))
                    break;
            }
/*
 *          not in the list; temporarily add it to the list
 */
            if (j == PhaseIdInfo->numAllowablePhases) {
                strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, mappedphase);
                npha++;
/*
 *              deal with PP et al (PP is in the list of allowable phases)
 */
                if (ILOC_STREQ(mappedphase, "PnPn")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PbPb");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PgPg");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "PbPb")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PnPn");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PgPg");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "PgPg")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PnPn");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PbPb");
                    npha++;
                }
/*
 *              deal with SS et al (SS is in the list of allowable phases)
 */
                if (ILOC_STREQ(mappedphase, "SnSn")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SbSb");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SgSg");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "SbSb")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SnSn");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SgSg");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "SgSg")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SnSn");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SbSb");
                    npha++;
                }
/*
 *              deal with PS et al
 */
                if (ILOC_STREQ(mappedphase, "PS")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PnS");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PgS");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "PnS")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PS");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PgS");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "PgS")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PS");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PnS");
                    npha++;
                }
/*
 *              deal with SP et al
 */
                if (ILOC_STREQ(mappedphase, "SP")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SPn");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SPg");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "SPn")) {
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SP");
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SPg");
                    npha++;
                }
                else if (ILOC_STREQ(mappedphase, "SPg")) {
                    npha++;
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SP");
                    strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SPn");
                    npha++;
                }
            }
            if (ILOC_STREQ(mappedphase, "PP")) {
                strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PnPn");
                npha++;
                strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PbPb");
                npha++;
                strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "PgPg");
                npha++;
            }
            if (ILOC_STREQ(mappedphase, "SS")) {
                strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SnSn");
                npha++;
                strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SbSb");
                npha++;
                strcpy(PhaseIdInfo->AllowablePhases[npha].Phase, "SgSg");
                npha++;
            }
/*
 *
 *          loop through allowable phases and calculate residual
 *
 */
            for (j = 0; j < npha; j++) {
                strcpy(phase, PhaseIdInfo->AllowablePhases[j].Phase);
/*
 *              only do matching phase types
 */
                if (islower(phase[0])) {
                    if (phase[1] == 'P' || phase[1] == 'w') ttype = 'P';
                    if (phase[1] == 'S') ttype = 'S';
                }
                else
                    ttype = toupper(phase[0]);
                if ((ttype == 'P' && stype) ||
                    ((ttype == 'S' || ttype == 'L') && ptype))
                    continue;
/*
 *              do not allow repeating phase names in a reading
 */
                isseen = 0;
                for (ii = rdindx->start; ii < k; ii++)
                    if (ILOC_STREQ(Assocs[ii].Phase, phase)) isseen = 1;
                if (isseen) continue;
/*
 *              first-arriving P in a reading
 */
                if (Assocs[k].firstP) {
                    if (!isFirstP(phase, mappedphase, PhaseIdInfo))
                        continue;
                }
/*
 *              first-arriving S in a reading
 */
                if (Assocs[k].firstS) {
                    if (!isFirstS(phase, mappedphase, PhaseIdInfo))
                        continue;
                }
/*
 *              do not allow S(*) phases to be renamed to depth phases (s*)
 */
                if (isS && islower(phase[0]) && phase[1] == 'S') continue;
/*
 *              get travel time for candidate phase
 */
                strcpy(Assocs[k].Phase, phase);
                if (iLoc_GetTravelTimePrediction(iLocConfig, Hypocenter, &Assocs[k],
                            &StaLocs[sind], ec, TTInfo, TTtables, LocalTTInfo,
                            LocalTTtables, topo, 0, isfirst, 0))
                    continue;
/*
 *              keep record of pP ttime
 */
                if (ILOC_STREQ(phase, "pP")) pPttime = Assocs[k].ttime;
/*
 *              do not allow pwP if there was no water column correction
 */
                if (ILOC_STREQ(phase, "pwP") &&
                    fabs(pPttime - Assocs[k].ttime) < ILOC_DEPSILON)
                    continue;
/*
 *              time residual
 */
                resid = Assocs[k].ArrivalTime - Hypocenter->Time - Assocs[k].ttime;
/*
 *              find phase with smallest residual
 */
                if (fabs(resid) < fabs(min_resid)) {
                    strcpy(candidate_phase, Assocs[k].Phase);
                    strcpy(Vmodel, Assocs[k].Vmodel);
                    min_resid = resid;
                    ttime = Assocs[k].ttime;
                    dtdd = Assocs[k].dtdd;
                }
            }
/*
 *          if no eligible phase found, set phase code to "".
 */
            if (fabs(min_resid) > bigres) {
                strcpy(Assocs[k].Phase, "");
                strcpy(Assocs[k].Vmodel, "null");
            }
/*
 *          otherwise set to best fitting phase
 */
            else {
                strcpy(Assocs[k].Phase, candidate_phase);
                strcpy(Assocs[k].Vmodel, Vmodel);
                Assocs[k].TimeRes = min_resid;
                Assocs[k].ttime = ttime;
                Assocs[k].dtdd = dtdd;
                if (Assocs[k].BackAzimuth != ILOC_NULLVAL)
                    Assocs[k].AzimRes = Assocs[k].BackAzimuth - Assocs[k].Seaz;
                if (Assocs[k].Slowness != ILOC_NULLVAL)
                    Assocs[k].SlowRes = Assocs[k].Slowness - Assocs[k].dtdd;
            }
        }
    }
}

/*
 *  Title:
 *     isFirstP
 *  Synopsis:
 *     Finds if a phase is in the list of allowable first-arriving P phases.
 *     Uses phase dependent information from <vmodel>_model.txt file.
 *        AllowableFirstP - list of phases to which reported first-arriving
 *                         P phases can be renamed
 *        OptionalFirstP - additional list of phases to which reported
 *                         first-arriving P phases can be renamed
 *  Input Arguments:
 *     phase       - phase
 *     mappedphase - reported phase
 *     PhaseIdInfo - pointer to ILOC_PHASEIDINFO structure
 *  Returns 1 if found, 0 otherwise
 *  Called by:
 *     PhaseIdentification
 */
static int isFirstP(char *phase, char *mappedphase, ILOC_PHASEIDINFO *PhaseIdInfo)
{
    int j;
/*
 *  see if phase is in the list of allowable first-arriving P phases
 */
    for (j = 0; j < PhaseIdInfo->numFirstPphase; j++) {
        if (ILOC_STREQ(phase, PhaseIdInfo->firstPphase[j].Phase))
            return 1;
    }
/*
 *  not in the list of allowable first-arriving P phases;
 *  see if it is in the optional list
 */
    if (j == PhaseIdInfo->numFirstPphase && ILOC_STREQ(mappedphase, phase)) {
        for (j = 0; j < PhaseIdInfo->numFirstPoptional; j++) {
            if (ILOC_STREQ(phase, PhaseIdInfo->firstPoptional[j].Phase))
                return 1;
        }
    }
    return 0;
}

/*
 *  Title:
 *     isFirstS
 *  Synopsis:
 *     Finds if a phase is in the list of allowable first-arriving S phases.
 *     Uses phase dependent information from <vmodel>_model.txt file.
 *        AllowableFirstS - list of phases to which reported first-arriving
 *                         S phases can be renamed
 *        OptionalFirstS - additional list of phases to which reported
 *                         first-arriving S phases can be renamed
 *  Input Arguments:
 *     phase       - phase
 *     mappedphase - reported phase
 *     PhaseIdInfo - pointer to ILOC_PHASEIDINFO structure
 *  Returns 1 if found, 0 otherwise
 *  Called by:
 *     PhaseIdentification
 */
static int isFirstS(char *phase, char *mappedphase, ILOC_PHASEIDINFO *PhaseIdInfo)
{
    int j;
/*
 *  see if phase is in the list of allowable first-arriving S phases
 */
    for (j = 0; j < PhaseIdInfo->numFirstSphase; j++) {
        if (ILOC_STREQ(phase, PhaseIdInfo->firstSphase[j].Phase))
            return 1;
    }
/*
 *  not in the list of allowable first-arriving S phases;
 *  see if it is in the optional list
 */
    if (j == PhaseIdInfo->numFirstSphase && ILOC_STREQ(mappedphase, phase)) {
        for (j = 0; j < PhaseIdInfo->numFirstSoptional; j++) {
            if (ILOC_STREQ(phase, PhaseIdInfo->firstSoptional[j].Phase))
                return 1;
        }
    }
    return 0;
}

/*
 *  Title:
 *     iLoc_GetNumDef
 *  Synopsis:
 *     counts the number of time/azimuth/slowness defining phases
 *  Input Arguments:
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - array of ILOC_ASSOC structures
 *  Output Arguments:
 *     Hypocenter - pointer to ILOC_HYPO structure
 *  Called by:
 *     iLoc_Locator, iLoc_IdentifyPhases, iLoc_ReIdentifyPhases
 */
void iLoc_GetNumDef(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs)
{
    int i;
    Hypocenter->numTimedef = Hypocenter->numAzimdef = 0;
    Hypocenter->numSlowdef = Hypocenter->numDef = 0;
/*
 *  count the number of time defining phases
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].Timedef) Hypocenter->numTimedef++;
        if (Assocs[i].Azimdef) Hypocenter->numAzimdef++;
        if (Assocs[i].Slowdef) Hypocenter->numSlowdef++;
    }
    Hypocenter->numDef = Hypocenter->numTimedef +
                         Hypocenter->numAzimdef +
                         Hypocenter->numSlowdef;
}

/*
 *  Title:
 *     GetPriorMeasurementError
 *  Synopsis:
 *     Sets timedef flag and a priori estimate of measurement error for a phase.
 *     Uses phase dependent information from <vmodel>_model.txt file.
 *        PhaseWeight - list of a priori measurement errors within specified
 *                delta ranges for IASPEI phases
 *     Sets the defining flags to true if a valid entry is found in the
 *        PhaseWeight table.
 *     Makes the phase non-defining if its residual is larger than a threshold,
 *        or it was explicitly set to non-defining by an analyst.
 *     Only phases with timedef = 1 are used in the location.
 *     For infrasound phases set the timedef flag to zero and use the azimuth
 *        instead.
 *  Input Arguments:
 *     Assocs      - pointer to ILOC_ASSOC structure
 *     PhaseIdInfo - pointer to ILOC_PHASEIDINFO structure
 *  Output Arguments:
 *     Assocs      - pointer to ILOC_ASSOC structure
 *  Called by:
 *     iLoc_IdentifyPhases, iLoc_ReIdentifyPhases
 */
static void GetPriorMeasurementError(ILOC_ASSOC *Assocs,
        ILOC_PHASEIDINFO *PhaseIdInfo, double SigmaThreshold)
{
    int j;
    double threshold;
/*
 *  set timedef and azimdef flags and get measurement errors
 */
    Assocs->Deltim = Assocs->Delaz = Assocs->Delslo = 0;
    for (j = 0; j < PhaseIdInfo->numPhaseWeight; j++) {
        if (ILOC_STREQ(Assocs->Phase, PhaseIdInfo->PhaseWeight[j].Phase)) {
            if (Assocs->Delta >= PhaseIdInfo->PhaseWeight[j].delta1 &&
                Assocs->Delta <  PhaseIdInfo->PhaseWeight[j].delta2) {
                if (Assocs->ArrivalTime != ILOC_NULLVAL) {
/*
 *                  a priori time measurement error
 */
                    Assocs->Deltim = PhaseIdInfo->PhaseWeight[j].deltim;
                    if (Assocs->Timedef) {
/*
 *                      make time non-defining if its residual is large
 */
                        threshold = SigmaThreshold * Assocs->Deltim;
                        if (fabs(Assocs->TimeRes) > threshold)
                            Assocs->Timedef = 0;
                    }
                }
                if (Assocs->BackAzimuth != ILOC_NULLVAL) {
/*
 *                  a priori azimuth measurement error
 */
                    Assocs->Delaz = PhaseIdInfo->PhaseWeight[j].delaz;
                    if (Assocs->Azimdef) {
/*
 *                      make azimuth non-defining if its residual is large
 */
                        threshold = SigmaThreshold * Assocs->Delaz;
                        if (fabs(Assocs->AzimRes) > threshold)
                            Assocs->Azimdef = 0;
                    }
                }
                if (Assocs->Slowness != ILOC_NULLVAL) {
/*
 *                  a priori slowness measurement error
 */
                    Assocs->Delslo = PhaseIdInfo->PhaseWeight[j].delslo;
                    if (Assocs->Slowdef) {
/*
 *                      make slowness non-defining if its residual is large
 */
                        threshold = SigmaThreshold * Assocs->Delslo;
                        if (fabs(Assocs->SlowRes) > threshold)
                            Assocs->Slowdef = 0;
                    }
                }
                break;
            }
        }
    }
}

/*
 *  Title:
 *     iLoc_SortAssocs
 *  Synopsis:
 *     Sorts phase structures by increasing Delta, StaInd, rdid, ArrivalTime
 *  Input Arguments:
 *     numPhase - number of associated phases
 *     Assocs   - array of ILOC_ASSOC structures
 *  Called by:
 *     iLoc_Locator
 */
void iLoc_SortAssocs(int numPhase, ILOC_ASSOC *Assocs)
{
    int i, j;
    ILOC_ASSOC temp;
    for (i = 1; i < numPhase; i++) {
        for (j = i - 1; j > -1; j--) {
            if ((Assocs[j].ArrivalTime > Assocs[j+1].ArrivalTime &&
                 Assocs[j+1].ArrivalTime != ILOC_NULLVAL) ||
                 Assocs[j].ArrivalTime == ILOC_NULLVAL) {
                ILOC_SWAP(Assocs[j], Assocs[j+1]);
            }
        }
    }
    for (i = 1; i < numPhase; i++) {
        for (j = i - 1; j > -1; j--) {
            if (Assocs[j].rdid > Assocs[j+1].rdid) {
                ILOC_SWAP(Assocs[j], Assocs[j+1]);
            }
        }
    }
    for (i = 1; i < numPhase; i++) {
        for (j = i - 1; j > -1; j--) {
            if (Assocs[j].StaInd > Assocs[j+1].StaInd) {
                ILOC_SWAP(Assocs[j], Assocs[j+1]);
            }
        }
    }
    for (i = 1; i < numPhase; i++) {
        for (j = i - 1; j > -1; j--) {
            if (Assocs[j].Delta > Assocs[j+1].Delta) {
                ILOC_SWAP(Assocs[j], Assocs[j+1]);
            }
        }
    }
}

/*
 *  Title:
 *     iLoc_SortAssocsNN
 *  Synopsis:
 *     Sort phase structures by increasing staorder, rdid, time.
 *     Ensures that phase records are ordered by the nearest-neighbour
 *     ordering, thus block-diagonalizing the data covariance matrix.
 *  Input Arguments:
 *     numPhase - number of associated phases
 *     numSta   - number of distinct stations
 *     Assocs   - array of ILOC_ASSOC structures
 *     StaLocs  - array of ILOC_STA structures
 *     staorder - array of staorder structures
 *  Called by:
 *     LocateEvent, iLoc_NASearch
 */
void iLoc_SortAssocsNN(int numPhase, int numSta, ILOC_ASSOC *Assocs,
        ILOC_STA *StaLocs, ILOC_STAORDER *staorder)
{
    int i, j, k, kp, m;
    ILOC_ASSOC temp;
/*
 *  sort by arrival time
 */
    for (i = 1; i < numPhase; i++) {
        for (j = i - 1; j > -1; j--) {
            if ((Assocs[j].ArrivalTime > Assocs[j+1].ArrivalTime &&
                 Assocs[j+1].ArrivalTime != ILOC_NULLVAL) ||
                 Assocs[j].ArrivalTime == ILOC_NULLVAL) {
                ILOC_SWAP(Assocs[j], Assocs[j+1]);
            }
        }
    }
/*
 *  sort by rdid
 */
    for (i = 1; i < numPhase; i++) {
        for (j = i - 1; j > -1; j--) {
            if (Assocs[j].rdid > Assocs[j+1].rdid) {
                ILOC_SWAP(Assocs[j], Assocs[j+1]);
            }
        }
    }
/*
 *  sort by nearest-neighbour station order
 */
    for (i = 1; i < numPhase; i++) {
        for (j = i - 1; j > -1; j--) {
            m = Assocs[j].StaInd;
            kp = staorder[m].index;
            m = Assocs[j+1].StaInd;
            k = staorder[m].index;
            if (kp > k) {
                ILOC_SWAP(Assocs[j], Assocs[j+1]);
            }
        }
    }
}


/*  EOF  */
