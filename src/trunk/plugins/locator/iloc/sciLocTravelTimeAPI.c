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
 *    iLoc_TravelTimePredictions
 */

/*
 *  Title:
 *     iLoc_TravelTimePredictions
 *  Synopsis:
 *     Provides travel time predictions for a phase-source-receiver triplet
 *     iLocConfig specifies what TT tables (ak135, iasp91, local or RSTT)
 *        will be used
 *     If phase is a NULL string, all phases will be calculated
 *     User is responsible for freeing predictedTT
 *  Input arguments:
 *     iLocConfig    - pointer to ILOC_CONF structure
 *     ec            - array of ILOC_EC_COEF structures
 *     topo          - ETOPO bathymetry/elevation matrix
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - array of ILOC_TT_TABLE structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - array of ILOC_TT_TABLE structures
 *     lat           - source latitude
 *     lon           - source longitude
 *     depth         - source depth [km]
 *     staLat        - station latitude
 *     staLon        - station longitude
 *     staElev       - station elevation [m]
 *     phase         - phase
 *  Output arguments:
 *     Delta         - epicentral distance [deg]
 *     Esaz          - event-to-station azimuth [deg]
 *     numPhase      - number of phases in ILOC_TT array
 *  Return:
 *     Array of ILOC_TT structure
 *  Called by:
 *     SeisComp3 iLoc TT app
 *  Calls:
 *     iLoc_DistAzimuth, iLoc_GetTravelTimePrediction, iLoc_Free
 */
ILOC_TT *iLoc_TravelTimePredictions(ILOC_CONF *iLocConfig, ILOC_EC_COEF *ec,
        short int **topo, ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables,
        ILOC_TTINFO *LocalTTInfo, ILOC_TT_TABLE *LocalTTtables,
        char *phase, double lat, double lon, double depth,
        double staLat, double staLon, double staElev,
        double *Delta, double *Esaz, int *numPhase)
{
    ILOC_TT *predictedTT = (ILOC_TT *)NULL;
    ILOC_HYPO Hypocenter;
    ILOC_STA StaLoc;
    ILOC_ASSOC Assoc;
    int i, n = 0, np = 0, isall = 0, iszderiv = 1, isfirst = -1, is2nderiv = 0;
    double delta, esaz, seaz;
/*
 *  source is too deep
 */
    if (depth > TTInfo->MaxHypocenterDepth) {
        fprintf(stderr, "source is too deep %f > %f!\n",
                depth, TTInfo->MaxHypocenterDepth);
        return (ILOC_TT *)NULL;
    }
/*
 *  get source-receiver distance
 */
    delta = iLoc_DistAzimuth(staLat, staLon, lat, lon, &seaz, &esaz);
    *Delta = delta;
    *Esaz = esaz;
/*
 *  create Hypocenter, Assoc and Staloc structures to use iLoc routines
 */
    Hypocenter.numReading = Hypocenter.numSta = Hypocenter.numPhase = 1;
    Hypocenter.Lat = lat;
    Hypocenter.Lon = lon;
    Hypocenter.Depth = depth;
    StaLoc.StaLat= staLat;
    StaLoc.StaLon = staLon;
    StaLoc.StaElevation= staElev;
    Assoc.Delta = delta;
    Assoc.Esaz = esaz;
    Assoc.Seaz = seaz;
/*
 *  calculate all phases or just a single phase?
 */
    if (phase == NULL || strlen(phase) == 0) {
/*
 *      calculate all phases
 */
        np = TTInfo->numPhaseTT;
        isall = 1;
    }
    else {
        np = 1;
        strcpy(Assoc.Phase, phase);
        isall = 0;
    }
    if ((predictedTT = (ILOC_TT *)calloc(np, sizeof(ILOC_TT))) == NULL) {
        fprintf(stderr, "Cannot allocate memory for predictedTT!\n");
        return (ILOC_TT *)NULL;
    }
    *numPhase = n = 0;
    for (i = 0; i < np; i++) {
        if (isall) {
            if (ILOC_STREQ(TTInfo->PhaseTT[i].Phase, "firstP") ||
                ILOC_STREQ(TTInfo->PhaseTT[i].Phase, "firstS"))
            continue;
            strcpy(Assoc.Phase, TTInfo->PhaseTT[i].Phase);
        }
        Assoc.ttime = Assoc.dtdd = Assoc.dtdh = ILOC_NULLVAL;
        if (iLoc_GetTravelTimePrediction(iLocConfig, &Hypocenter, &Assoc,
                &StaLoc, ec, TTInfo, TTtables, LocalTTInfo, LocalTTtables,
                topo, iszderiv, isfirst, is2nderiv)) {
            if (isall) {
               continue;
            }
            else {
                fprintf(stderr, "Cannot get travel-time for %s!\n", phase);
                iLoc_Free(predictedTT);
                return (ILOC_TT *)NULL;
            }
        }
        strcpy(predictedTT[n].Phase, Assoc.Phase);
        strcpy(predictedTT[n].Vmodel, Assoc.Vmodel);
        predictedTT[n].ttime = Assoc.ttime;
        predictedTT[n].dtdd = Assoc.dtdd;
        predictedTT[n].dtdh = Assoc.dtdh;
        n++;
    }
    *numPhase = n;
    return predictedTT;
}
