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
 *    iLoc_LocationQuality
 *    iLoc_GetdUGapSgap
 */

/*
 * Local functions
 *    CompareDouble
 */
static int CompareDouble(const void *x, const void *y);

/*
 *  Title:
 *     iLoc_LocationQuality
 *  Synopsis:
 *     Calculates network geometry based location quality metrics
 *     gap, sgap and dU for local distance ranges and the entire network.
 *         Local network:  0 - 150 km
 *         Entire network: 0 - 180 degrees
 *     Only defining stations are considered.
 *     dU is defined in:
 *        Bondár, I. and K. McLaughlin, 2009,
 *        A new ground truth data set for seismic studies,
 *        Seism. Res. Let., 80, 465-472.
 *     sgap is defined in:
 *        Bondár, I., S.C. Myers, E.R. Engdahl and E.A. Bergman, 2004,
 *        Epicenter accuracy based on seismic network criteria,
 *        Geophys. J. Int., 156, 483-496, doi: 10.1111/j.1365-246X.2004.02070.x.
 *  Input Arguments:
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs   - array of ILOC_ASSOC structures
 *  Output Arguments:
 *     Hypocenter - pointer to ILOC_HYPO structure
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_Locator
 *  Calls:
 *     iLoc_GetdUGapSgap
 */
int iLoc_LocationQuality(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs)
{
    double *esaz = (double *)NULL;
    double gap = 0., sgap = 0., du = 0.;
    double delta = 0., d10 = 0., mind = 0., maxd = 0.;
    int i, ndef = 0, nsta = 0, numStaWithin10km = 0, PrevStaInd = -1;
    if ((esaz = (double *)calloc(Hypocenter->numPhase + 2, sizeof(double))) == NULL) {
        fprintf(stderr, "iLoc_LocationQuality: cannot allocate memory\n");
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  local network (0-150 km)
 */
    delta = 150. * ILOC_RAD2DEG / ILOC_EARTHRADIUS;
    d10 = 10. * ILOC_RAD2DEG / ILOC_EARTHRADIUS;
    mind = 180.;
    maxd = 0.;
    for (ndef = nsta = 0, i = 0; i < Hypocenter->numPhase; i++) {
        if (!Assocs[i].Timedef && !Assocs[i].Azimdef && !Assocs[i].Slowdef)
             continue;
        if (Assocs[i].Delta > delta) continue;
        if (Assocs[i].Timedef) ndef++;
        if (Assocs[i].Azimdef) ndef++;
        if (Assocs[i].Slowdef) ndef++;
        if (Assocs[i].StaInd == PrevStaInd) continue;
        esaz[nsta++] = Assocs[i].Esaz;
        if (Assocs[i].Delta <= d10) numStaWithin10km++;
        PrevStaInd = Assocs[i].StaInd;
    }
    Hypocenter->localNumDefsta = nsta;
    Hypocenter->localNumDef = ndef;
    Hypocenter->numStaWithin10km = numStaWithin10km;
    du = iLoc_GetdUGapSgap(nsta, esaz, &gap, &sgap);
    Hypocenter->localDU = du;
    Hypocenter->localSgap = sgap;
    Hypocenter->GT5candidate = 1;
    if (du > 0.35 || numStaWithin10km < 1 || sgap > 160.)
         Hypocenter->GT5candidate = 0;
/*
 *  entire network
 */
    PrevStaInd = -1;
    mind = 180.;
    maxd = 0.;
    for (ndef = nsta = 0, i = 0; i < Hypocenter->numPhase; i++) {
        if (!Assocs[i].Timedef && !Assocs[i].Azimdef && !Assocs[i].Slowdef)
             continue;
        if (Assocs[i].StaInd == PrevStaInd) continue;
        esaz[nsta++] = Assocs[i].Esaz;
        if (Assocs[i].Delta > maxd) maxd = Assocs[i].Delta;
        if (Assocs[i].Delta < mind) mind = Assocs[i].Delta;
        PrevStaInd = Assocs[i].StaInd;
    }
    du = iLoc_GetdUGapSgap(nsta, esaz, &gap, &sgap);
    Hypocenter->Gap = gap;
    Hypocenter->Sgap = sgap;
    Hypocenter->minDist = mind;
    Hypocenter->maxDist = maxd;
    iLoc_Free(esaz);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     iLoc_GetdUGapSgap
 *  Synopsis:
 *     Calculates gap, sgap and dU.
 *
 *           4 * sum|esaz[i] - (360 *i / nsta + b)|
 *     dU = ---------------------------------------
 *                      360 * nsta
 *
 *     b = avg(esaz) - avg(360i/N)  where esaz is sorted
 *
 *  Input Arguments:
 *     nsta - number of defining stations
 *     esaz - array of event-to-station azimuths
 *  Output Arguments:
 *     gap  - largest azimuthal gap
 *     sgap - largest secondary azimuthal gap
 *  Return:
 *     dU   - network quality metric
 *  Called by:
 *     iLoc_LocationQuality
 */
double iLoc_GetdUGapSgap(int nsta, double *esaz, double *gap, double *sgap)
{
    int i;
    double du = 1., bb = 0., uesaz = 0., w = 0., s1 = 0., s2 = 0.;
    *gap = 360.;
    *sgap = 360.;
    if (nsta < 2) return du;
/*
 *  sort esaz
 */
    qsort(esaz, nsta, sizeof(double), CompareDouble);
/*
 *  du: mean absolute deviation from best fitting uniform network
 */
    for (i = 0; i < nsta; i++) {
        uesaz = 360. * (double)i / (double)nsta;
        s1 += esaz[i];
        s2 += uesaz;
    }
    bb = (s1 - s2) / (double)nsta;
    for (w = 0., i = 0; i < nsta; i++) {
        uesaz = 360. * (double)i / (double)nsta;
        w += fabs(esaz[i] - uesaz - bb);
    }
    du = 4. * w / (360. * (double)nsta);
/*
 *  gap
 */
    esaz[nsta] = esaz[0] + 360.;
    for (w = 0., i = 0; i < nsta; i++)
        w = ILOC_MAX(w, esaz[i+1] - esaz[i]);
    if (w > 360.) w = 360.;
    *gap = w;
/*
 *  sgap
 */
    esaz[nsta+1] = esaz[1] + 360.;
    for (w = 0., i = 0; i < nsta; i++)
        w = ILOC_MAX(w, esaz[i+2] - esaz[i]);
    if (w > 360.) w = 360.;
    *sgap = w;
    return du;
}

/*
 *  Title:
 *     CompareDouble
 *  Synopsis:
 *     compares two doubles
 *  Returns:
 *     -1 if x < y, 1 if x > y and 0 if x == y
 */
static int CompareDouble(const void *x, const void *y)
{
    if (*(double *)x < *(double *)y)
        return -1;
    if (*(double *)x > *(double *)y)
        return 1;
    return 0;
}


/*  EOF  */
