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
 *    iLoc_DepthResolution
 *    iLoc_DepthPhaseCheck
 *    iLoc_DepthPhaseStack
 */

/*
 * Local functions
 *    PhaseTTh
 *    Stacker
 *    StationTrace
 */
static int PhaseTTh(double delta, double esaz, ILOC_CONF *iLocConfig,
        ILOC_HYPO *Hypocenter, ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtable,
        ILOC_TT_TABLE *Pfirst, short int **topo, int ips, int ispwP, double *pt);
static void Stacker(int n, int nsamp, double moveout, double deltim,
        double *pt, double *tz, double *depths, double *pp, int *trace,
        int *stack);
static void StationTrace(int n, int nsamp, double moveout, double deltim,
        double *pp, double *h, int *trace);
/*
 *  Title:
 *     iLoc_DepthResolution
 *  Synopsis:
 *     Decides if there is sufficient depth resolution for a free depth solution
 *     We have depth resolution if
 *        has depth resolution from depth phases (
 *        OR
 *        nlocal >= MinLocalStations
 *        OR
 *        nsdef >= MinSPpairs
 *        OR
 *        (ncoredef >= MinCorePhases AND nagent >= MindDepthPhaseAgencies))
 *     For any of these cases a defining first-arriving P in the reading
 *        is required.
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - array of ILOC_ASSOC structures
 *     rdindx     - array of ILOC_READING structures
 *  Return:
 *     1 if we have depth resolution, 0 otherwise
 *  Called by:
 *     iLoc_Locator
 */
int iLoc_DepthResolution(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_READING *rdindx)
{
    int i, k, m, np, nsdef = 0, ncoredef = 0, nlocal = 0;
    int hasDepthResolution = 0;
/*
 *  loop over readings
 */
    for (i = 0; i < Hypocenter->numReading; i++) {
        m = rdindx[i].start;
        np = rdindx[i].start + rdindx[i].npha;
/*
 *      local stations
 */
        if (!Assocs[m].duplicate && Assocs[m].Timedef &&
            Assocs[m].firstP && Assocs[m].Delta <= iLocConfig->MaxLocalDistDeg)
            nlocal++;
/*
 *      multiple phases in the reading
 */
        for (k = m + 1; k < np; k++) {
/*
 *          both the first-arriving and the later phase must be defining
 */
            if (!Assocs[m].Timedef || !Assocs[k].Timedef)
                continue;
/*
 *          number of defining core reflections
 */
            if (!Assocs[k].duplicate &&
                (ILOC_STREQ(Assocs[k].Phase, "PcP") ||
                 ILOC_STREQ(Assocs[k].Phase, "ScS")))
                ncoredef++;
/*
 *          number of defining S-P pairs within MaxSPDistDeg
 */
            if (Assocs[m].firstP && Assocs[k].firstS &&
                Assocs[m].Delta <= iLocConfig->MaxSPDistDeg &&
                (Assocs[m].duplicate * Assocs[k].duplicate) == 0)
                nsdef++;
        }
    }
/*
 *  we have depth resolution if
 *      (has_depdpres || nlocal >= MinLocalStations || nsdef >= MinSPpairs ||
 *       ncoredef >= MinCorePhases
 */
    if (nlocal >= iLocConfig->MinLocalStations ||
        nsdef >= iLocConfig->MinSPpairs ||
        ncoredef >= iLocConfig->MinCorePhases ||
        Hypocenter->numDepthDp >= iLocConfig->MinDepthPhases)
        hasDepthResolution = 1;
    if (iLocConfig->Verbose) {
        fprintf(stderr, "Depth resolution: %d\n", hasDepthResolution);
        fprintf(stderr, "  %d defining depth phases\n",
                Hypocenter->numDepthDp);
        fprintf(stderr, "  %d stations within %.2f degrees\n",
                nlocal, iLocConfig->MaxLocalDistDeg);
        fprintf(stderr, "  %d defining S-P pairs within %.2f degrees\n",
                nsdef, iLocConfig->MaxSPDistDeg);
        fprintf(stderr, "  %d defining PcP/ScS phases\n", ncoredef);
    }
    strcat(Hypocenter->iLocInfo, "  Depth resolution from:\n");
    sprintf(Hypocenter->iLocInfo, "%s    %d defining depth phases\n",
            Hypocenter->iLocInfo, Hypocenter->numDepthDp);
    sprintf(Hypocenter->iLocInfo, "%s    %d stations within %.2f degrees\n",
            Hypocenter->iLocInfo, nlocal, iLocConfig->MaxLocalDistDeg);
    sprintf(Hypocenter->iLocInfo, "%s    %d defining S-P pairs within %.2f degrees\n",
            Hypocenter->iLocInfo, nsdef, iLocConfig->MaxSPDistDeg);
    sprintf(Hypocenter->iLocInfo, "%s    %d defining PcP/ScS phases\n",
            Hypocenter->iLocInfo, ncoredef);
    return hasDepthResolution;
}


/*
 *  Title:
 *     iLoc_DepthPhaseCheck
 *  Synopsis:
 *     Decides whether depth phases provide depth resolution.
 *     flags first-arriving defining P for a reading
 *     makes note of P - depth phase pairs
 *     makes orphan depth phases non-defining
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - array of ILOC_ASSOC structures
 *     rdindx     - array of ILOC_READING structures
 *  Return:
 *     1 if we have depth-phase depth resolution, 0 otherwise
 *  Called by:
 *     iLoc_Locator, LocateEvent, GetResiduals
 */
int iLoc_DepthPhaseCheck(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_READING *rdindx)
{
    int i, k, m, np, ndepassoc = 0, has_depdpres = 1;
    for (i = 0; i < Hypocenter->numPhase; i++) {
        Assocs[i].firstP = Assocs[i].hasDepthPhase = 0;
        Assocs[i].pPindex = Assocs[i].pwPindex = 0;
        Assocs[i].pSindex = Assocs[i].sPindex = Assocs[i].sSindex = 0;
    }
/*
 *  loop over readings
 */
    for (i = 0; i < Hypocenter->numReading; i++) {
        m = rdindx[i].start;
        np = rdindx[i].start + rdindx[i].npha;
        if (Assocs[m].Timedef && Assocs[m].Phase[0] == 'P')
            Assocs[m].firstP = 1;
/*
 *      only one phase in the reading
 */
        if (rdindx[i].npha == 1) {
/*
 *          do not allow defining depth phases without a defining first P
 */
            if (!Assocs[m].firstP && islower(Assocs[m].Phase[0]))
                Assocs[m].Timedef = 0;
        }
/*
 *      multiple phases in the reading
 */
        for (k = m + 1; k < np; k++) {
            if (!Assocs[k].Timedef || isupper(Assocs[k].Phase[0]))
                continue;
/*
 *          do not allow defining depth phases without a defining first P
 */
            if (!Assocs[m].firstP && islower(Assocs[k].Phase[0])) {
                Assocs[k].Timedef = 0;
                continue;
            }
/*
 *          pP*
 */
            if (strncmp(Assocs[k].Phase, "pP", 2) == 0) {
                Assocs[m].hasDepthPhase++;
                Assocs[m].pPindex = k;
                if (!Assocs[k].duplicate) ndepassoc++;
            }
/*
 *          pwP*
 */
            else if (strncmp(Assocs[k].Phase, "pw", 2) == 0) {
                Assocs[m].hasDepthPhase++;
                Assocs[m].pwPindex = k;
                if (!Assocs[k].duplicate) ndepassoc++;
            }
/*
 *          pS*
 */
            else if (strncmp(Assocs[k].Phase, "pS", 2) == 0) {
                Assocs[m].hasDepthPhase++;
                Assocs[m].pSindex = k;
                if (!Assocs[k].duplicate) ndepassoc++;
            }
/*
 *          sP*
 */
            else if (strncmp(Assocs[k].Phase, "sP", 2) == 0) {
                Assocs[m].hasDepthPhase++;
                Assocs[m].sPindex = k;
                if (!Assocs[k].duplicate) ndepassoc++;
            }
/*
 *          sS*
 */
            else if (strncmp(Assocs[k].Phase, "sS", 2) == 0) {
                Assocs[m].hasDepthPhase++;
                Assocs[m].sSindex = k;
                if (!Assocs[k].duplicate) ndepassoc++;
            }
            else
                continue;
        }
    }
    Hypocenter->numDepthDp = ndepassoc;
/*
 *  check for depth resolution by depth phases
 */
    if (ndepassoc < iLocConfig->MinDepthPhases)
        has_depdpres = 0;
    return has_depdpres;
}

/*
 *  Title:
 *     iLoc_DepthPhaseStack
 *  Synopsis:
 *     calculates depth-phase depth using the Murphy and Barker (BSSA, 2006)
 *        depth phase stacking method
 *     Murphy J.R. and B.W. Barker, 2006,
 *        Improved focal-depth determination through automated identification
 *        of the seismic depth phases pP and sP,
 *        Bull. Seism. Soc. Am., 96, 1213-1229.
 *
 *     builds station traces with 1 km steps in depth
 *     a station trace maps moveout times to depth for a given delta
 *     a boxcar function is placed in the station trace at the depth
 *         correHypocenteronding to the observed moveout
 *     the station traces then stacked across the network
 *     the depth-phase depth is identified as the median of the stack
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - array of ILOC_ASSOC structures
 *     TTInfo     - pointer to ILOC_TTINFO structure
 *     TTtables   - array of ILOC_TT_TABLE structures
 *     topo       - ETOPO bathymetry/elevation matrix
 *  Returns:
 *     number of depth phases used in depth phase stack
 *  Called by:
 *     iLoc_Locator
 *  Calls:
 *     iLoc_GetPhaseIndex, PhaseTTh, Stacker, iLoc_Free
 */
int iLoc_DepthPhaseStack(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables,
        short int **topo)
{
    int i, j, k, m, n, ndep = 0, ndel = 0, ndp = 0, ns = 0;
    int prev_rdid = 0, med = 0, d = 0, dlo = 0, dhi = 0;
    int iP = 0, ipP = 0, ipwP = 0, ipS = 0, isP = 0, isS = 0;
    int nsamp = (int)TTInfo->MaxHypocenterDepth + 1;
    double delta = 0., depth = 0., esaz = 0.;
    double moveout = 0., smad = 0.;
    double *depths;
    double *tz = (double *)NULL;
    double *pt = (double *)NULL;
    double *pp = (double *)NULL;
    int *stack = (int *)NULL;
    int *trace = (int *)NULL;
    ILOC_TT_TABLE *Pfirst = &TTtables[0];
    depths = TTtables[0].depths;
    Hypocenter->DepthDp = ILOC_NULLVAL;
    Hypocenter->DepthDpError = ILOC_NULLVAL;
/*
 *  number of depth and delta samples in firstP TT table
 */
    ndep = TTtables[0].ndep;
    ndel = TTtables[0].ndel;
    tz = (double *)calloc(ndep, sizeof(double));
    pt = (double *)calloc(ndep, sizeof(double));
    trace = (int *)calloc(nsamp, sizeof(int));
    stack = (int *)calloc(nsamp, sizeof(int));
    if ((pp = (double *)calloc(ndep, sizeof(double))) == NULL) {
        iLoc_Free(stack);
        iLoc_Free(trace);
        iLoc_Free(pt);
        iLoc_Free(tz);
        fprintf(stderr, "iLoc_DepthPhaseStack: cannot allocate memory\n");
        return 0;
    }
/*
 *  Loop through readings with first arriving P with depth phases
 */
    prev_rdid = -1;
    for (i = 0; i < Hypocenter->numPhase; i++) {
/*
 *      skip if same reading
 */
        if (Assocs[i].rdid == prev_rdid) continue;
/*
 *      skip if not first P or has no depth phases or non-defining phase
 */
        if (!Assocs[i].Timedef) continue;
        if (!Assocs[i].firstP) continue;
        if (!Assocs[i].hasDepthPhase) continue;
        delta = Assocs[i].Delta;
        esaz = Assocs[i].Esaz;
/*
 *      check for out of range delta
 */
        if (delta < TTtables[0].deltas[0] ||
            delta > TTtables[0].deltas[ndel - 1])
            continue;
/*
 *      get phase indexes
 */
        ipP = ipwP = ipS = isP = isS = 0;
        if ((iP = iLoc_GetPhaseIndex(Assocs[i].Phase, TTInfo)) < 0)
            continue;
        if (Assocs[i].pPindex) {
            k = Assocs[i].pPindex;
            if (!Assocs[k].Timedef || Assocs[k].duplicate) continue;
            ipP = ILOC_MAX(0, iLoc_GetPhaseIndex(Assocs[k].Phase, TTInfo));
            if (ipP) {
                n = TTtables[ipP].ndel;
                if (delta < TTtables[ipP].deltas[0] ||
                    delta > TTtables[ipP].deltas[n - 1])
                    ipP = 0;
            }
        }
        if (Assocs[i].pwPindex) {
            k = Assocs[i].pwPindex;
            if (!Assocs[k].Timedef || Assocs[k].duplicate) continue;
            ipwP = ILOC_MAX(0, iLoc_GetPhaseIndex(Assocs[k].Phase, TTInfo));
            if (ipwP) {
                n = TTtables[ipwP].ndel;
                if (delta < TTtables[ipwP].deltas[0] ||
                    delta > TTtables[ipwP].deltas[n - 1])
                    ipwP = 0;
            }
        }
        if (Assocs[i].pSindex) {
            k = Assocs[i].pSindex;
            if (!Assocs[k].Timedef || Assocs[k].duplicate) continue;
            ipS = ILOC_MAX(0, iLoc_GetPhaseIndex(Assocs[k].Phase, TTInfo));
            if (ipS) {
                n = TTtables[ipS].ndel;
                if (delta < TTtables[ipS].deltas[0] ||
                    delta > TTtables[ipS].deltas[n - 1])
                    ipS = 0;
            }
        }
        if (Assocs[i].sPindex) {
            k = Assocs[i].sPindex;
            if (!Assocs[k].Timedef || Assocs[k].duplicate) continue;
            isP = ILOC_MAX(0, iLoc_GetPhaseIndex(Assocs[k].Phase, TTInfo));
            if (isP) {
                n = TTtables[isP].ndel;
                if (delta < TTtables[isP].deltas[0] ||
                    delta > TTtables[isP].deltas[n - 1])
                    isP = 0;
            }
        }
        if (Assocs[i].sSindex) {
            k = Assocs[i].sSindex;
            if (!Assocs[k].Timedef || Assocs[k].duplicate) continue;
            isS = ILOC_MAX(0, iLoc_GetPhaseIndex(Assocs[k].Phase, TTInfo));
            if (isS) {
                n = TTtables[isS].ndel;
                if (delta < TTtables[isS].deltas[0] ||
                    delta > TTtables[isS].deltas[n - 1])
                    isS = 0;
            }
        }
/*
 *      no valid depth phases
 */
        if (!(ipP || ipwP || ipS || isP || isS))
            continue;
/*
 *      build first_P TT(h) for this delta
 */
        for (j = 0; j < ndep; j++) {
            tz[j] = -999.;
            pt[j] = -999.;
        }
        j = PhaseTTh(delta, esaz, iLocConfig, Hypocenter, TTInfo, &TTtables[iP],
                     Pfirst, topo, 0, 0, tz);
/*
 *      pP* - first P
 */
        if (ipP) {
            k = Assocs[i].pPindex;
            n = PhaseTTh(delta, esaz, iLocConfig, Hypocenter, TTInfo,
                         &TTtables[ipP], Pfirst, topo, 1, 0, pt);
            m = ILOC_MIN(n, j);
            moveout = Assocs[k].ttime - Assocs[i].ttime;
            Stacker(m, nsamp, moveout, Assocs[k].Deltim, pt, tz, depths, pp,
                    trace, stack);
            ndp++;
        }
/*
 *      pwP - first P
 */
        if (ipwP) {
            k = Assocs[i].pwPindex;
            n = PhaseTTh(delta, esaz, iLocConfig, Hypocenter, TTInfo,
                         &TTtables[ipwP], Pfirst, topo, 1, 1, pt);
            m = ILOC_MIN(n, j);
            moveout = Assocs[k].ttime - Assocs[i].ttime;
            Stacker(m, nsamp, moveout, Assocs[k].Deltim, pt, tz, depths, pp,
                    trace, stack);
            ndp++;
        }
/*
 *      pS* - first P
 */
        if (ipS) {
            k = Assocs[i].pSindex;
            n = PhaseTTh(delta, esaz, iLocConfig, Hypocenter, TTInfo,
                         &TTtables[ipS], Pfirst, topo, 2, 0, pt);
            m = ILOC_MIN(n, j);
            moveout = Assocs[k].ttime - Assocs[i].ttime;
            Stacker(m, nsamp, moveout, Assocs[k].Deltim, pt, tz, depths, pp,
                    trace, stack);
            ndp++;
        }
/*
 *      sP* - first P
 */
        if (isP) {
            k = Assocs[i].sPindex;
            n = PhaseTTh(delta, esaz, iLocConfig, Hypocenter, TTInfo,
                         &TTtables[isP], Pfirst, topo, 2, 0, pt);
            m = ILOC_MIN(n, j);
            moveout = Assocs[k].ttime - Assocs[i].ttime;
            Stacker(m, nsamp, moveout, Assocs[k].Deltim, pt, tz, depths, pp,
                    trace, stack);
            ndp++;
        }
/*
 *      sS* - first P
 */
        if (isS) {
            k = Assocs[i].sSindex;
            n = PhaseTTh(delta, esaz, iLocConfig, Hypocenter, TTInfo,
                         &TTtables[isS], Pfirst, topo, 3, 0, pt);
            m = ILOC_MIN(n, j);
            moveout = Assocs[k].ttime - Assocs[i].ttime;
            Stacker(m, nsamp, moveout, Assocs[k].Deltim, pt, tz, depths, pp,
                    trace, stack);
            ndp++;
        }
    }
/*
 *  find the maximum of the stack
 *      d is the index (i.e. the depth) of the maximum, m is the maximum value
 */
    if (ndp > 2) {
        d = 0;
        for (m = 0, j = 0; j < nsamp; j++) {
            if (stack[j] > m) {
                d = j;
                m = stack[j];
            }
        }
/*
 *      check number of constructively adding depth phases
 */
        ndp = m;
        if (ndp > 2) {
/*
 *          stack width at the maximum of the stack
 */
            ns = 0;
            for (j = d; j > -1; j--) {
                ns += stack[j];
                if (stack[j] < 1) break;
            }
            dlo = ILOC_MAX(j + 1, 0);
            for (j = d; j < nsamp; j++) {
                ns += stack[j];
                if (stack[j] < 1) break;
            }
            dhi = ILOC_MIN(j, nsamp);
/*
 *          get median from the cumulative of the stack
 */
            trace[0] = stack[dlo];
            k = 1;
            for (j = dlo + 1; j < dhi; j++) {
                trace[k] = trace[k - 1] + stack[j];
                if ((double)trace[k - 1] / (double)ns < 0.5 &&
                    (double)trace[k] / (double)ns >= 0.5)
                    m = k;
                k++;
            }
/*
 *          depth-phase depth
 */
            med = (double)(m + dlo) +
                  ((double)ns / 2. - (double)trace[m - 1]) /
                   (double)stack[m + dlo];
            depth = med;
/*
 *          get smad from the the stack
 */
            for (j = 0; j < nsamp; j++) trace[j] = 0;
            n = 0;
            for (j = dlo; j < dhi; j++) {
                k = abs(j - med);
                trace[k] += stack[j];
                if (k > n) n = k;
            }
            stack[0] = trace[0];
            k = 1;
            for (j = 1; j < n; j++) {
                stack[k] = stack[k - 1] + trace[j];
                if ((double)stack[k - 1] / (double)ns < 0.5 &&
                    (double)stack[k] / (double)ns >= 0.5)
                    m = k;
                k++;
            }
/*
 *          smad
 */
            smad = 1.4826 * (double)(m) +
                   ((double)ns / 2. - (double)stack[m - 1]) /
                   (double)trace[m];
/*
 *          set depth and depth error
 */
            Hypocenter->DepthDp = depth;
            Hypocenter->DepthDpError = smad;
            Hypocenter->numDepthDp = ndp;
        }
    }
    iLoc_Free(pp);
    iLoc_Free(stack);
    iLoc_Free(trace);
    iLoc_Free(pt);
    iLoc_Free(tz);
    return ndp;
}

/*
 *  Title:
 *     PhaseTTh
 *  Synopsis:
 *     Calculates TT(h) vector for a given delta and phase.
 *  Input Arguments:
 *     delta      - distance [deg]
 *     esaz       - event-to-station azimuth
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     TTInfo     - pointer to ILOC_TTINFO structure
 *     TTtable    - pointer to ILOC_TT_TABLE structure
 *     Pfirst     - pointer to ILOC_TT_TABLE structure of first-arriving P
 *     topo       - ETOPO bathymetry/elevation matrix
 *     ips        - depth phase index for bounce corrections
 *     ispwP      - is the phase pwP?
 *  Output Arguments:
 *     pt - TT(h) vector
 *  Returns:
 *     number of depth samples
 *  Called by:
 *     iLoc_DepthPhaseStack
 *  Calls:
 *     iLoc_FloatBracket, iLoc_SplineCoeffs, iLoc_SplineInterpolation,
 *     iLoc_PointAtDeltaAzimuth, iLoc_GetEtopoCorrection
 */
static int PhaseTTh(double delta, double esaz, ILOC_CONF *iLocConfig,
        ILOC_HYPO *Hypocenter, ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtable,
        ILOC_TT_TABLE *Pfirst, short int **topo, int ips, int ispwP, double *pt)
{
    int i, j, k, m, exactdelta = 0;
    int ilo = 0, ihi = 0, jlo = 0, jhi = 0, idel = 0, jdel = 0;
    double dydx = 0., d2ydx = 0.;
    double d2y[ILOC_DELTASAMPLES], tmp[ILOC_DELTASAMPLES];
    double x[ILOC_DELTASAMPLES], t[ILOC_DELTASAMPLES];
    double b[ILOC_DELTASAMPLES], p[ILOC_DELTASAMPLES];
    double tcor = 0., tcorw = 0., rayp = 0., tt = 0.;
    double bpaz = 0., bpdel = 0., bplat = 0., bplon = 0.;
    double Psurfvel = TTInfo->PSurfVel;
    double Ssurfvel = TTInfo->SSurfVel;
    int ndep = TTtable->ndep;
    int ndel = TTtable->ndel;
    int jndel = Pfirst->ndel;
/*
 *  delta range
 */
    iLoc_FloatBracket(delta, ndel, TTtable->deltas, &ilo, &ihi);
    if (fabs(delta - TTtable->deltas[ilo]) < ILOC_DEPSILON) {
        idel = ilo;
        exactdelta = 1;
    }
    else if (fabs(delta - TTtable->deltas[ihi]) < ILOC_DEPSILON) {
        idel = ihi;
        exactdelta = 1;
    }
    else {
        idel = ilo;
        ilo = idel - ILOC_DELTASAMPLES / 2 + 1;
        ihi = idel + ILOC_DELTASAMPLES / 2 + 1;
        if (ilo < 0) {
            ilo = 0;
            ihi = ilo + ILOC_DELTASAMPLES;
        }
        if (ihi > ndel) {
            ihi = ndel;
            ilo = ihi - ILOC_DELTASAMPLES;
        }
    }
    iLoc_FloatBracket(delta, jndel, Pfirst->deltas, &jlo, &jhi);
    if (fabs(delta - Pfirst->deltas[jlo]) < ILOC_DEPSILON) {
        jdel = jlo;
    }
    else if (fabs(delta - Pfirst->deltas[jhi]) < ILOC_DEPSILON) {
        jdel = jhi;
    }
    else {
        jdel = jlo;
        jlo = jdel - ILOC_DELTASAMPLES / 2 + 1;
        jhi = jdel + ILOC_DELTASAMPLES / 2 + 1;
        if (jlo < 0) {
            jlo = 0;
            jhi = jlo + ILOC_DELTASAMPLES;
        }
        if (jhi > jndel) {
            jhi = jndel;
            jlo = jhi - ILOC_DELTASAMPLES;
        }
    }
/*
 *  build phase TT(h) for this delta
 */
    for (j = 0; j < ndep; j++) {
/*
 *      no need for Spline interpolation if exact delta
 */
        if (exactdelta) {
            if (!ips) {
                if (TTtable->tt[idel][j] < 0.) {
                    pt[j] = Pfirst->tt[jdel][j];
                }
                else {
                    pt[j] = TTtable->tt[idel][j];
                }
            }
            else {
                pt[j] = TTtable->tt[idel][j];
            }
/*
 *          bounce point correction
 */
            if (ips) {
                bpdel = TTtable->bpdel[idel][j];
                rayp = TTtable->dtdd[idel][j];
                bpaz = esaz;
                if (rayp < 0.)   bpaz += 180.;
                if (bpaz > 360.) bpaz -= 360.;
                iLoc_PointAtDeltaAzimuth(Hypocenter->Lat, Hypocenter->Lon,
                                         bpdel, bpaz, &bplat, &bplon);
                tcor = iLoc_GetEtopoCorrection(iLocConfig, ips, rayp, bplat,
                                   bplon, topo, Psurfvel, Ssurfvel, &tcorw);
                pt[j] += tcor;
                if (ispwP) pt[j] += tcorw;
            }
        }
/*
 *      Spline interpolation in delta
 */
        else {
            for (m = 0, k = jlo, i = ilo; i < ihi; k++, i++) {
                if (!ips) {
                    if (TTtable->tt[i][j] < 0.) {
                        tt = Pfirst->tt[k][j];
                    }
                    else {
                        tt = TTtable->tt[i][j];
                    }
                }
                else {
                    tt = TTtable->tt[i][j];
                }
                if (tt < 0.) continue;
                x[m] = TTtable->deltas[i];
                t[m] = tt;
                if (ips) {
                    b[m] = TTtable->bpdel[i][j];
                    p[m] = TTtable->dtdd[i][j];
                }
                m++;
            }
            if (m < ILOC_MINSAMPLES)
                pt[j] = -999.;
            else {
                iLoc_SplineCoeffs(m, x, t, d2y, tmp);
                pt[j] = iLoc_SplineInterpolation(delta, m, x, t, d2y, 0,
                                                 &dydx, &d2ydx);
/*
 *              bounce point correction
 */
                if (ips) {
                    iLoc_SplineCoeffs(m, x, b, d2y, tmp);
                    bpdel = iLoc_SplineInterpolation(delta, m, x, b, d2y, 0,
                                                     &dydx, &d2ydx);
                    iLoc_SplineCoeffs(m, x, p, d2y, tmp);
                    rayp = iLoc_SplineInterpolation(delta, m, x, p, d2y, 0,
                                                    &dydx, &d2ydx);
                    bpaz = esaz;
                    if (rayp < 0.)   bpaz += 180.;
                    if (bpaz > 360.) bpaz -= 360.;
                    iLoc_PointAtDeltaAzimuth(Hypocenter->Lat, Hypocenter->Lon,
                                             bpdel, bpaz, &bplat, &bplon);
                    tcor = iLoc_GetEtopoCorrection(iLocConfig, ips, rayp, bplat,
                                    bplon, topo, Psurfvel, Ssurfvel, &tcorw);
                    pt[j] += tcor;
                    if (ispwP) pt[j] += tcorw;
                }
            }
        }
    }
    return ndep;
}

/*
 *  Title:
 *     Stacker
 *  Synopsis:
 *     Calculates station traces h(TT) and stacks them.
 *  Input Arguments:
 *     n       - number of samples
 *     nsamp   - max number of samples
 *     moveout - observed depth phase - first P time (moveout)
 *     deltim  - tolerance (width of the boxcar function)
 *     pt      - predicted depth phase times w.r.t. depth
 *     tz      - predicted first P times w.r.t. depth
 *     depths  - depth vector
 *  Output Arguments:
 *     pp     - predicted moveout times w.r.t. depth
 *     trace  - station trace h(TT) with a boxcar around moveout
 *     stack  - network stack
 *  Called by:
 *     iLoc_DepthPhaseStack
 *  Calls:
 *     StationTrace
 */
static void Stacker(int n, int nsamp, double moveout, double deltim,
        double *pt, double *tz, double *depths, double *pp, int *trace,
        int *stack)
{
    int j, k = 0;
/*
 *  depth phase - first P travel times (moveout curve vs depth)
 */
    pp[k++] = 0.;
    for (j = 0; j < n; j++) {
        if (tz[j+1] < 0. || pt[j] < 0.)
            continue;
        else
            pp[k++] = pt[j] - tz[j+1];
    }
    if (k < 1) return;
/*
 *  station trace: depth vs moveout curve with a boxcar around observed moveout
 */
    StationTrace(k, nsamp, moveout, deltim, pp, depths, trace);
/*
 *  stack station traces
 */
    for (j = 0; j < nsamp; j++)
        stack[j] += trace[j];
}

/*
 *  Title:
 *     StationTrace
 *  Synopsis:
 *     Calculates station trace (h(TT)) vector for a given
 *         moveout time with deltim tolerance.
 *  Input Arguments:
 *     n       - number of samples
 *     nsamp   - max number of samples
 *     moveout - observed moveout
 *     deltim  - tolerance
 *     pp      - predicted moveout w.r.t. depth
 *     h       - depth vector
 *  Output Arguments:
 *     trace - station trace Z(tt) with a boxcar around moveout
 *  Called by:
 *    iLoc_DepthPhaseStack
 *  Calls:
 *     iLoc_FloatBracket, iLoc_SplineCoeffs, iLoc_SplineInterpolation
 */
static void StationTrace(int n, int nsamp, double moveout, double deltim,
        double *pp, double *h, int *trace)
{
    int i, j, ilo, ihi, idel, hlo, hhi;
    double dydx = 0., d2ydx = 0., tp = 0., hp = 0.;
    double d2y[ILOC_DELTASAMPLES], tmp[ILOC_DELTASAMPLES];
    double z[ILOC_DELTASAMPLES], t[ILOC_DELTASAMPLES];
    for (j = 0; j < nsamp; j++) trace[j] = 0;
/*
 *  h(TT) lower limit
 */
    tp = moveout - deltim;
    iLoc_FloatBracket(tp, n, pp, &ilo, &ihi);
    if (fabs(tp - pp[ilo]) < ILOC_DEPSILON)
        hlo = (int)floor(h[ilo]);
    else if (fabs(tp - pp[ihi]) < ILOC_DEPSILON)
        hlo = (int)floor(h[ihi]);
    else {
        idel = ilo;
        ilo = idel - ILOC_DELTASAMPLES / 2 + 1;
        ihi = idel + ILOC_DELTASAMPLES / 2 + 1;
        if (ilo < 0) {
            ilo = 0;
            ihi = ilo + ILOC_DELTASAMPLES;
        }
        if (ihi > n) {
            ihi = n;
            ilo = ihi - ILOC_DELTASAMPLES;
        }
        for (j = 0, i = ilo; i < ihi; j++, i++) {
            z[j] = h[i];
            t[j] = pp[i];
        }
        iLoc_SplineCoeffs(j, t, z, d2y, tmp);
        hp = iLoc_SplineInterpolation(tp, j, t, z, d2y, 0, &dydx, &d2ydx);
        hlo = (int)floor(hp);
    }
    if (hlo < 0) hlo = 0;
/*
 *  h(TT) upper limit
 */
    tp = moveout + deltim;
    iLoc_FloatBracket(tp, n, pp, &ilo, &ihi);
    if (fabs(tp - pp[ilo]) < ILOC_DEPSILON)
        hhi = (int)ceil(h[ilo]);
    else if (fabs(tp - pp[ihi]) < ILOC_DEPSILON)
        hhi = (int)ceil(h[ihi]);
    else {
        idel = ilo;
        ilo = idel - ILOC_DELTASAMPLES / 2 + 1;
        ihi = idel + ILOC_DELTASAMPLES / 2 + 1;
        if (ilo < 0) {
            ilo = 0;
            ihi = ilo + ILOC_DELTASAMPLES;
        }
        if (ihi > n) {
            ihi = n;
            ilo = ihi - ILOC_DELTASAMPLES;
        }
        for (j = 0, i = ilo; i < ihi; j++, i++) {
            z[j] = h[i];
            t[j] = pp[i];
        }
        iLoc_SplineCoeffs(j, t, z, d2y, tmp);
        hp = iLoc_SplineInterpolation(tp, j, t, z, d2y, 0, &dydx, &d2ydx);
        hhi = (int)ceil(hp);
    }
    if (hhi > nsamp) hhi = nsamp;
/*
 *  put boxcar at depth consistent with moveout observation
 */
    for (i = hlo; i < hhi; i++)
        trace[i] = 1;
}
