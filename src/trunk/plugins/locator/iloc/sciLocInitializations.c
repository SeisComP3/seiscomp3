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
 * Public Functions:
 *    iLoc_InitializeEvent
 */

/*
 * Local functions
 *    FirstSortAssocs
 */
static void FirstSortAssocs(int numPhase, ILOC_ASSOC *Assocs);

/*
 *  Title:
 *     iLoc_InitializeEvent
 *  Synopsis:
 *     Initializes output parameters in Hypocenter and Assoc structures.
 *     Fixes the hypocenter if all hypocenter parameters are fixed,
 *         or the number of associations is less than the number of unknowns.
 *     For anthropogenic events fixes the depth at the surface,
 *         unless the user fixed it explicitly to some depth
 *     Calculates Delta, Esaz and Seaz for each phase.
 *     Sorts phase structures by increasing Delta, StaInd, ArrivalTime
 *     Groups Assocs into readings
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - array of ILOC_ASSOC structures
 *     StaLocs    - array of ILOC_STA structures
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_Locator
 *  Calls:
 *     iLoc_DistAzimuth, FirstSortAssocs, iLoc_SortAssocs,
 *     iLoc_PrintHypocenter, iLoc_PrintPhases
 */
int iLoc_InitializeEvent(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs)
{
    int i, j, rdid, PrevStaInd = -1;
/*
 *  Initialize Hypocenter structure output parameters
 */
    Hypocenter->numReading = Hypocenter->numSta;
    for (i = 0; i < 4; i++) {
        Hypocenter->Errors[i] = ILOC_NULLVAL;
        for (j = 0; j < 4; j++) Hypocenter->ModelCov[i][j] = ILOC_NULLVAL;
    }
    Hypocenter->minDist = Hypocenter->maxDist = ILOC_NULLVAL;
    Hypocenter->semiMajax = Hypocenter->semiMinax = ILOC_NULLVAL;
    Hypocenter->Strike = Hypocenter->SdevObs = ILOC_NULLVAL;
    Hypocenter->numDef = Hypocenter->numTimedef = 0;
    Hypocenter->numSlowdef = Hypocenter->numAzimdef = 0;
    Hypocenter->numDefsta = Hypocenter->numDepthDp = 0;
    Hypocenter->DepthDp = Hypocenter->DepthDpError = ILOC_NULLVAL;
    Hypocenter->FixedDepthType = 0;
    Hypocenter->Converged = 0;
    Hypocenter->uRMS = Hypocenter->wRMS = 0.;
    Hypocenter->Gap = Hypocenter->Sgap = 360.;
    Hypocenter->GT5candidate = 0;
    Hypocenter->localSgap = 360.;
    Hypocenter->localDU = ILOC_NULLVAL;
    Hypocenter->numStaWithin10km = 0;
    Hypocenter->localNumDefsta = Hypocenter->localNumDef = 0;
    strcpy(Hypocenter->iLocInfo, "");
/*
 *  Check if there are sufficient number of observations
 */
    Hypocenter->numUnknowns = 4;
    if (Hypocenter->FixOT)    Hypocenter->numUnknowns--;
    if (Hypocenter->FixLat)   Hypocenter->numUnknowns--;
    if (Hypocenter->FixLon)   Hypocenter->numUnknowns--;
    if (Hypocenter->FixDepth) Hypocenter->numUnknowns--;
    if (!Hypocenter->FixHypo) {
        if (Hypocenter->numUnknowns == 0) {
            fprintf(stderr, "All hypocenter parameters are fixed, ");
            fprintf(stderr, "fixing the hypocentre\n");
            strcat(Hypocenter->iLocInfo, "  All hypocenter parameters are fixed, fixing the hypocentre\n");
            Hypocenter->FixHypo = 1;
        }
        else {
            if (Hypocenter->numPhase < Hypocenter->numUnknowns) {
                fprintf(stderr, "Insufficient number of phases (%d), ",
                        Hypocenter->numPhase);
                fprintf(stderr, "fixing the hypocentre\n");
                strcat(Hypocenter->iLocInfo, "  Insufficient number of phases, fixing the hypocentre\n");
                Hypocenter->FixHypo = 1;
            }
        }
    }
/*
 *  Depth fixed by user
 */
    if (Hypocenter->FixDepth) {
        Hypocenter->FixedDepthType = 8;
        if (iLocConfig->Verbose)
            fprintf(stderr, "Depth fixed to user provided depth\n");
        strcat(Hypocenter->iLocInfo, "  Depth fixed to user provided depth\n");
    }
/*
 *  Anthropogenic event:
 *    fix depth at surface, unless the user fixed it explicitly to some depth
 */
    if (Hypocenter->isManMade) {
        if (!Hypocenter->FixDepth) {
            Hypocenter->Depth = 0.;
            Hypocenter->FixDepth = 1;
            Hypocenter->FixedDepthType = 4;
            Hypocenter->numUnknowns--;
            if (iLocConfig->Verbose)
                fprintf(stderr, "Anthropogenic event, fix depth to zero\n");
            strcat(Hypocenter->iLocInfo, "  Anthropogenic event, fix depth to zero\n");
        }
    }
/*
 *
 *  A depth value larger than 700 km is a good indicator for a bogus event
 *
 */
    if (!Hypocenter->FixDepth && Hypocenter->Depth > 700.) {
        fprintf(stderr, "WARNING: Depth (%.0f) is larger than 700 km! ",
                Hypocenter->Depth);
        fprintf(stderr, "Possibly bogus event, setting depth to 35 km\n");
        Hypocenter->Depth = 35.;
    }
/*
 *
 *  Initialize Assocs structure output parameters and sort phases
 *
 */
    for (i = 0; i < Hypocenter->numPhase; i++) {
        Assocs[i].initialPhase = 0;
        Assocs[i].rdid = Assocs[i].duplicate = 0;
        strcpy(Assocs[i].prevPhase, "");
        Assocs[i].prevTimedef = Assocs[i].prevAzimdef = 0;
        Assocs[i].prevSlowdef = Assocs[i].CovIndSlow = 0;
        Assocs[i].CovIndTime = Assocs[i].CovIndAzim = 0;
        Assocs[i].ttime = Assocs[i].dtdd = Assocs[i].dtdh = ILOC_NULLVAL;
        Assocs[i].d2tdd = Assocs[i].d2tdh = Assocs[i].bpdel = ILOC_NULLVAL;
        Assocs[i].firstP = Assocs[i].firstS = Assocs[i].hasDepthPhase = 0;
        Assocs[i].pPindex = Assocs[i].pwPindex = Assocs[i].pSindex = 0;
        Assocs[i].sPindex = Assocs[i].sSindex = 0;
        Assocs[i].Delta = Assocs[i].Seaz = Assocs[i].Esaz = ILOC_NULLVAL;
/*
 *      calculate delta, esaz, seaz
 */
        j = Assocs[i].StaInd;
        Assocs[i].Delta = iLoc_DistAzimuth(StaLocs[j].StaLat, StaLocs[j].StaLon,
                                           Hypocenter->Lat, Hypocenter->Lon,
                                           &Assocs[i].Seaz, &Assocs[i].Esaz);
/*
 *      user provided picking error?
 */
        Assocs[i].userDeltim = 0;
        if (Assocs[i].Deltim != ILOC_NULLVAL) Assocs[i].userDeltim = 1;
    }
/*
 *  Sort phase structures by increasing Delta, StaInd, ArrivalTime
 */
    FirstSortAssocs(Hypocenter->numPhase, Assocs);
/*
 *  Split Assocs into readings
 */
    PrevStaInd = -1;
    rdid = 1;
    for (i = 0; i < Hypocenter->numPhase; i++) {
        if (Assocs[i].StaInd != PrevStaInd) {
            Assocs[i].rdid = rdid++;
            Assocs[i].initialPhase = 1;
        }
        else {
            Assocs[i].rdid = Assocs[i-1].rdid;
        }
        PrevStaInd = Assocs[i].StaInd;
    }
    Hypocenter->numReading = rdid - 1;
    if (iLocConfig->Verbose > 2) {
        iLoc_PrintHypocenter(Hypocenter);
        iLoc_PrintPhases(Hypocenter->numPhase, Assocs);
    }
/*
 *  Sort phase structures by increasing Delta, StaInd, rdid, ArrivalTime
 */
    iLoc_SortAssocs(Hypocenter->numPhase, Assocs);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     iLoc_FirstSortAssocs
 *  Synopsis:
 *     Sorts phase structures by increasing Delta, StaInd and ArrivalTime
 *  Input Arguments:
 *     numPhase - number of associated phases
 *     Assocs   - array of ILOC_ASSOC structures
 *  Called by:
 *     InitializeEvent
 */
static void FirstSortAssocs(int numPhase, ILOC_ASSOC *Assocs)
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

