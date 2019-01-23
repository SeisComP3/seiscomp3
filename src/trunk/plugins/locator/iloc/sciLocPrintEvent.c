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
 *    iLoc_PrintSolution
 *    iLoc_PrintHypocenter
 *    iLoc_PrintPhases
 *    iLoc_PrintDefiningPhases
 *    iLoc_EpochToHuman
 */

/*
 *  Title:
 *      iLoc_PrintSolution
 *  Synopsis:
 *      Prints current solution to the logfile
 *  Input Arguments:
 *      Hypocenter - pointer to ILOC_HYPO structure
 *      grn - geographic region number. gregname is printed if grn > 0.
 *  Calls:
 *      iLoc_Gregion, iLoc_EpochToHuman
 *  Called by:
 *      iLoc_Locator, LocateEvent, iLoc_NASearch
 */
void iLoc_PrintSolution(ILOC_HYPO *Hypocenter, int grn)
{
    char timestr[25], gregname[255];
    if (grn) {
        iLoc_Gregion(grn, gregname);
        fprintf(stderr, "%s\n", gregname);
    }
    iLoc_EpochToHuman(timestr, Hypocenter->Time);
    fprintf(stderr, "OT = %s ", timestr);
    fprintf(stderr, "Lat = %.3f ", Hypocenter->Lat);
    fprintf(stderr, "Lon = %.3f ", Hypocenter->Lon);
    fprintf(stderr, "Depth = %.1f\n", Hypocenter->Depth);
}

/*
 *  Title:
 *      iLoc_PrintHypocenter
 *  Synopsis:
 *      Prints hypocentres for an event to the logfile
 *  Input Arguments:
 *      Hypocenter - pointer to ILOC_HYPO structure
 *  Calls:
 *      iLoc_EpochToHuman
 *  Called by:
 *      iLoc_InitializeEvent
 */
void iLoc_PrintHypocenter(ILOC_HYPO *Hypocenter)
{
    char timestr[25];
    iLoc_EpochToHuman(timestr, Hypocenter->Time);
    fprintf(stderr, "OT = %s ", timestr);
    fprintf(stderr, "Lat = %.3f ", Hypocenter->Lat);
    fprintf(stderr, "Lon = %.3f ", Hypocenter->Lon);
    fprintf(stderr, "Depth = %.1f ", Hypocenter->Depth);
    fprintf(stderr, "NumSta = %d ", Hypocenter->numSta);
    fprintf(stderr, "NumPhase = %d\n", Hypocenter->numPhase);
}

/*
 *  Title:
 *      iLoc_PrintPhases
 *  Synopsis:
 *      Prints a table with all the phases for one event.
 *  Input Arguments:
 *      numPhase - number of associated phases
 *      Assocs   - array of ILOC_ASSOC structures
 *  Calls:
 *      iLoc_EpochToHuman
 *  Called by:
 *      iLoc_Locator, ResidualsForFixedHypocenter
 */
void iLoc_PrintPhases(int numPhase, ILOC_ASSOC *Assocs)
{
    int i;
    char timestr[25];
    fprintf(stderr, "RDID      STAIND  DELTA   ESAZ ");
    fprintf(stderr, "HINT     IASPEI   TIME                     TIMERES   ");
    fprintf(stderr, "AZIM  AZRES   SLOW SLORES TAS MODEL\n");
    for (i = 0; i < numPhase; i++) {
        fprintf(stderr, "%-9d ", Assocs[i].rdid);
        fprintf(stderr, "%-6d ", Assocs[i].StaInd);
        fprintf(stderr, "%6.2f ", Assocs[i].Delta);
        fprintf(stderr, "%6.2f ", Assocs[i].Esaz);
        fprintf(stderr, "%-8s ", Assocs[i].PhaseHint);
        fprintf(stderr, "%-8s ", Assocs[i].Phase);
        if (Assocs[i].ArrivalTime != ILOC_NULLVAL) {
            iLoc_EpochToHuman(timestr, Assocs[i].ArrivalTime);
            fprintf(stderr, "%s ", timestr);
            if (Assocs[i].TimeRes != ILOC_NULLVAL)
                fprintf(stderr, "%8.2f ", Assocs[i].TimeRes);
            else
                fprintf(stderr, "%8s ", "");
        }
        else fprintf(stderr, "%32s ", "");
        if (Assocs[i].BackAzimuth != ILOC_NULLVAL) {
            fprintf(stderr, "%6.1f ", Assocs[i].BackAzimuth);
            if (Assocs[i].AzimRes != ILOC_NULLVAL)
                fprintf(stderr, "%6.1f ", Assocs[i].AzimRes);
            else
                fprintf(stderr, "%6s ", "");
        }
        else fprintf(stderr, "%13s ", "");
        if (Assocs[i].Slowness != ILOC_NULLVAL) {
            fprintf(stderr, "%6.1f ", Assocs[i].Slowness);
            if (Assocs[i].SlowRes != ILOC_NULLVAL)
                fprintf(stderr, "%6.1f ", Assocs[i].SlowRes);
            else
                fprintf(stderr, "%6s ", "");
        }
        else fprintf(stderr, "%13s ", "");
        if (Assocs[i].Timedef) fprintf(stderr, "T");
        else                   fprintf(stderr, "_");
        if (Assocs[i].Azimdef) fprintf(stderr, "A");
        else                   fprintf(stderr, "_");
        if (Assocs[i].Slowdef) fprintf(stderr, "S ");
        else                   fprintf(stderr, "_ ");
        fprintf(stderr, "%s\n", Assocs[i].Vmodel);
    }
}

/*
 *  Title:
 *      iLoc_PrintDefiningPhases
 *  Synopsis:
 *      Prints a table with all the time-defining phases for one event
 *  Input Arguments:
 *      numPhase - number of associated phases
 *      Assocs   - array of ILOC_ASSOC structures
 *  Calls:
 *      iLoc_EpochToHuman
 *  Called by:
 *      LocateEvent, iLoc_NASearch
 */
void iLoc_PrintDefiningPhases(int numPhase, ILOC_ASSOC *Assocs)
{
    int i;
    char timestr[25];
    fprintf(stderr, "RDID      STAIND  DELTA   ESAZ ");
    fprintf(stderr, "PHASE    TIME                     TIMERES   ");
    fprintf(stderr, "AZIM  AZRES   SLOW SLORES TAS MODEL\n");
    for (i = 0; i < numPhase; i++) {
        if (!Assocs[i].Timedef && !Assocs[i].Azimdef && !Assocs[i].Slowdef)
            continue;
        fprintf(stderr, "%-9d ", Assocs[i].rdid);
        fprintf(stderr, "%-6d ", Assocs[i].StaInd);
        fprintf(stderr, "%6.2f ", Assocs[i].Delta);
        fprintf(stderr, "%6.2f ", Assocs[i].Esaz);
        fprintf(stderr, "%-8s ", Assocs[i].Phase);
        if (Assocs[i].Timedef) {
            iLoc_EpochToHuman(timestr, Assocs[i].ArrivalTime);
            fprintf(stderr, "%s ", timestr);
            if (Assocs[i].TimeRes != ILOC_NULLVAL)
                fprintf(stderr, "%8.2f ", Assocs[i].TimeRes);
            else
                fprintf(stderr, "%8s ", "");
        }
        else fprintf(stderr, "%32s ", "");
        if (Assocs[i].Azimdef) {
            fprintf(stderr, "%6.1f ", Assocs[i].BackAzimuth);
            if (Assocs[i].AzimRes != ILOC_NULLVAL)
                fprintf(stderr, "%6.1f ", Assocs[i].AzimRes);
            else
                fprintf(stderr, "%6s ", "");
        }
        else fprintf(stderr, "%13s ", "");
        if (Assocs[i].Slowdef) {
            fprintf(stderr, "%6.1f ", Assocs[i].Slowness);
            if (Assocs[i].SlowRes != ILOC_NULLVAL)
                fprintf(stderr, "%6.1f ", Assocs[i].SlowRes);
            else
                fprintf(stderr, "%6s ", "");
        }
        else fprintf(stderr, "%13s ", "");
        if (Assocs[i].Timedef) fprintf(stderr, "T");
        else                   fprintf(stderr, "_");
        if (Assocs[i].Azimdef) fprintf(stderr, "A");
        else                   fprintf(stderr, "_");
        if (Assocs[i].Slowdef) fprintf(stderr, "S ");
        else                   fprintf(stderr, "_ ");
        fprintf(stderr, "%s\n", Assocs[i].Vmodel);
    }
}

/*
 *  Title:
 *     iLoc_EpochToHuman
 *  Synopsis:
 *     Converts epoch time to human time.
 *  Input Arguments:
 *     etime - epoch time (double, including fractional seconds)
 *  Output Arguments:
 *     htime - human time string 'YYYY-MM-DD HH:MI:SS.SSS'
 *  Called by:
 *     iLoc_Locator, iLoc_PrintSolution, iLoc_PrintHypocenter, iLoc_PrintPhases,
 *     iLoc_PrintDefiningPhases, iLoc_NASearch
 */
void iLoc_EpochToHuman(char *htime, double etime)
{
    struct tm *ht = (struct tm *)NULL;
    time_t et;
    int yyyy = 0, mm = 0, dd = 0, hh = 0, mi = 0;
    double t = 0., ft = 0., sec = 0.;
    int it = 0;
    char s[25];
    if (etime != ILOC_NULLVAL) {
/*
 *      break down epoch time to day and msec used by ISC DB schema
 *      also take care of negative epoch times
 */
        sprintf(s, "%.3f", etime);
        t = atof(s);
        it = (int)t;
        sprintf(s, "%.3f\n", t - it);
        ft = atof(s);
        if (ft < 0.) {
            ft += 1.;
            t -= 1.;
        }
/*
 *      set time structure
 */
        ht = (struct tm *)calloc(1, sizeof(struct tm));
        et = (time_t)t;
        gmtime_r(&et, ht);
        yyyy = ht->tm_year + 1900;
        mm = ht->tm_mon + 1;
        dd = ht->tm_mday;
        hh = ht->tm_hour;
        mi = ht->tm_min;
        sec = (double)ht->tm_sec + ft;
/*
 *      human time string 'YYYY-MM-DD HH:MI:SS.SSS'
 */
        sprintf(htime, "%04d-%02d-%02d %02d:%02d:%06.3f",
                yyyy, mm, dd, hh, mi, sec);
        iLoc_Free(ht);
    }
    else {
        strcpy(htime, "                       ");
    }
}

/*
 *  Title:
 *     iLoc_PrintInputStructures
 *  Synopsis:
 *     Prints inputs structures.
 *  Input Arguments:
 *     iLocConfig - pointer to ILOC_CONF structure
 *     Hypocenter - pointer to ILOC_HYPO structure
 *     Assocs     - array of ILOC_ASSOC structures
 *     StaLocs    - array of ILOC_STA structures
 *  Called by:
 *     iLoc_Locator
 */
void iLoc_PrintIOstructures(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, int isinput)
{
    int i, j;
    if (isinput) {
/*
 *      print ILOC structures on input
 */
        fprintf(stderr, "Configuration parameters\n");
        fprintf(stderr, "  auxdir=%s\n", iLocConfig->auxdir);
        fprintf(stderr, "  Verbose=%d\n", iLocConfig->Verbose);
        fprintf(stderr, "  TTmodel=%s\n", iLocConfig->TTmodel);
        fprintf(stderr, "  UseRSTT=%d\n", iLocConfig->UseRSTT);
        fprintf(stderr, "    RSTTmodel=%s\n", iLocConfig->RSTTmodel);
        fprintf(stderr, "    UseRSTTPnSn=%d\n", iLocConfig->UseRSTTPnSn);
        fprintf(stderr, "    UseRSTTPgLg=%d\n", iLocConfig->UseRSTTPgLg);
        fprintf(stderr, "  UseLocalTT=%d\n", iLocConfig->UseRSTT);
        fprintf(stderr, "    LocalVmodel=%s\n", iLocConfig->LocalVmodel);
        fprintf(stderr, "    MaxLocalTTDelta=%.1f\n", iLocConfig->MaxLocalTTDelta);
        fprintf(stderr, "  DoGridSearch=%d\n", iLocConfig->DoGridSearch);
        fprintf(stderr, "    NAsearchRadius=%.1f\n", iLocConfig->NAsearchRadius);
        fprintf(stderr, "    NAsearchDepth=%.1f\n", iLocConfig->NAsearchDepth);
        fprintf(stderr, "    NAsearchOT=%.1f\n", iLocConfig->NAsearchOT);
        fprintf(stderr, "    NAlpNorm=%.1f\n", iLocConfig->NAlpNorm);
        fprintf(stderr, "    NAiterMax=%d\n", iLocConfig->NAiterMax);
        fprintf(stderr, "    NAinitialSample=%d\n", iLocConfig->NAinitialSample);
        fprintf(stderr, "    NAnextSample=%d\n", iLocConfig->NAnextSample);
        fprintf(stderr, "    NAcells=%d\n", iLocConfig->NAcells);
        fprintf(stderr, "  DoNotRenamePhases=%d\n", iLocConfig->DoNotRenamePhases);
        fprintf(stderr, "  DoCorrelatedErrors=%d\n", iLocConfig->DoCorrelatedErrors);
        fprintf(stderr, "  SigmaThreshold=%.1f\n", iLocConfig->SigmaThreshold);
        fprintf(stderr, "  MinIterations=%d\n", iLocConfig->MinIterations);
        fprintf(stderr, "  MaxIterations=%d\n", iLocConfig->MaxIterations);
        fprintf(stderr, "  MinNdefPhases=%d\n", iLocConfig->MinNdefPhases);
        fprintf(stderr, "  AllowDamping=%d\n", iLocConfig->AllowDamping);
        fprintf(stderr, "  MaxLocalDistDeg=%.1f\n", iLocConfig->MaxLocalDistDeg);
        fprintf(stderr, "  MinLocalStations=%d\n", iLocConfig->MinLocalStations);
        fprintf(stderr, "  MaxSPDistDeg=%.1f\n", iLocConfig->MaxSPDistDeg);
        fprintf(stderr, "  MinSPpairs=%d\n", iLocConfig->MinSPpairs);
        fprintf(stderr, "  MinCorePhases=%d\n", iLocConfig->MinCorePhases);
        fprintf(stderr, "  MinDepthPhases=%d\n", iLocConfig->MinDepthPhases);
        fprintf(stderr, "  MaxShallowDepthError=%.1f\n", iLocConfig->MaxShallowDepthError);
        fprintf(stderr, "  MaxDeepDepthError=%.1f\n", iLocConfig->MaxDeepDepthError);
        fprintf(stderr, "  EtopoFile=%s\n", iLocConfig->EtopoFile);
        fprintf(stderr, "    EtopoNlat=%d EtopoNlon=%d EtopoRes=%.1f\n",
                iLocConfig->EtopoNlat, iLocConfig->EtopoNlon, iLocConfig->EtopoRes);

        fprintf(stderr, "Hypocenter data\n");
        fprintf(stderr, "  isManMade=%d\n", Hypocenter->isManMade);
        fprintf(stderr, "  numSta=%d\n", Hypocenter->numSta);
        fprintf(stderr, "  numPhase=%d\n", Hypocenter->numPhase);
        fprintf(stderr, "  Time=%.3f FixOT=%d\n",
                Hypocenter->Time, Hypocenter->FixOT);
        fprintf(stderr, "  Lat=%.3f FixLat=%d\n",
                Hypocenter->Lat, Hypocenter->FixLat);
        fprintf(stderr, "  Lon=%.3f FixLon=%d\n",
                Hypocenter->Lon, Hypocenter->FixLon);
        fprintf(stderr, "  Depth=%.2f FixDepth=%d\n",
                Hypocenter->Depth, Hypocenter->FixDepth);
        fprintf(stderr, "  FixHypo=%d\n", Hypocenter->FixHypo);

        fprintf(stderr, "Associated arrival data\n");
        for (i = 0; i < Hypocenter->numPhase; i++) {
            fprintf(stderr, "  arid=%d\n", Assocs[i].arid);
            fprintf(stderr, "  StaInd=%d\n", Assocs[i].StaInd);
            fprintf(stderr, "  PhaseHint=%s phaseFixed=%d\n",
                    Assocs[i].PhaseHint, Assocs[i].phaseFixed);
            fprintf(stderr, "  ArrivalTime=%.3f Timedef=%d\n",
                    Assocs[i].ArrivalTime, Assocs[i].Timedef);
            fprintf(stderr, "  BackAzimuth=%.3f Azimdef=%d\n",
                    Assocs[i].BackAzimuth, Assocs[i].Azimdef);
            fprintf(stderr, "  Slowness=%.3f Slowdef=%d\n",
                    Assocs[i].Slowness, Assocs[i].Slowdef);
        }

        fprintf(stderr, "Station data\n");
        for (i = 0; i < Hypocenter->numSta; i++)
            fprintf(stderr, "  %d StaLat=%.3f StaLon=%.3f StaElevation=%.3f\n",
                i, StaLocs[i].StaLat, StaLocs[i].StaLon, StaLocs[i].StaElevation);
        fprintf(stderr, "\n");
    }
    else {
/*
 *      print ILOC structures on return
 */
        fprintf(stderr, "Configuration parameters\n");
        fprintf(stderr, "  auxdir=%s\n", iLocConfig->auxdir);
        fprintf(stderr, "  Verbose=%d\n", iLocConfig->Verbose);
        fprintf(stderr, "  TTmodel=%s\n", iLocConfig->TTmodel);
        fprintf(stderr, "  UseRSTT=%d\n", iLocConfig->UseRSTT);
        fprintf(stderr, "    RSTTmodel=%s\n", iLocConfig->RSTTmodel);
        fprintf(stderr, "    UseRSTTPnSn=%d\n", iLocConfig->UseRSTTPnSn);
        fprintf(stderr, "    UseRSTTPgLg=%d\n", iLocConfig->UseRSTTPgLg);
        fprintf(stderr, "  UseLocalTT=%d\n", iLocConfig->UseRSTT);
        fprintf(stderr, "    LocalVmodel=%s\n", iLocConfig->LocalVmodel);
        fprintf(stderr, "    MaxLocalTTDelta=%.1f\n", iLocConfig->MaxLocalTTDelta);
        fprintf(stderr, "  DoGridSearch=%d\n", iLocConfig->DoGridSearch);
        fprintf(stderr, "    NAsearchRadius=%.1f\n", iLocConfig->NAsearchRadius);
        fprintf(stderr, "    NAsearchDepth=%.1f\n", iLocConfig->NAsearchDepth);
        fprintf(stderr, "    NAsearchOT=%.1f\n", iLocConfig->NAsearchOT);
        fprintf(stderr, "    NAlpNorm=%.1f\n", iLocConfig->NAlpNorm);
        fprintf(stderr, "    NAiterMax=%d\n", iLocConfig->NAiterMax);
        fprintf(stderr, "    NAinitialSample=%d\n", iLocConfig->NAinitialSample);
        fprintf(stderr, "    NAnextSample=%d\n", iLocConfig->NAnextSample);
        fprintf(stderr, "    NAcells=%d\n", iLocConfig->NAcells);
        fprintf(stderr, "  DoNotRenamePhases=%d\n", iLocConfig->DoNotRenamePhases);
        fprintf(stderr, "  DoCorrelatedErrors=%d\n", iLocConfig->DoCorrelatedErrors);
        fprintf(stderr, "  SigmaThreshold=%.1f\n", iLocConfig->SigmaThreshold);
        fprintf(stderr, "  MinIterations=%d\n", iLocConfig->MinIterations);
        fprintf(stderr, "  MaxIterations=%d\n", iLocConfig->MaxIterations);
        fprintf(stderr, "  MinNdefPhases=%d\n", iLocConfig->MinNdefPhases);
        fprintf(stderr, "  AllowDamping=%d\n", iLocConfig->AllowDamping);
        fprintf(stderr, "  MaxLocalDistDeg=%.1f\n", iLocConfig->MaxLocalDistDeg);
        fprintf(stderr, "  MinLocalStations=%d\n", iLocConfig->MinLocalStations);
        fprintf(stderr, "  MaxSPDistDeg=%.1f\n", iLocConfig->MaxSPDistDeg);
        fprintf(stderr, "  MinSPpairs=%d\n", iLocConfig->MinSPpairs);
        fprintf(stderr, "  MinCorePhases=%d\n", iLocConfig->MinCorePhases);
        fprintf(stderr, "  MinDepthPhases=%d\n", iLocConfig->MinDepthPhases);
        fprintf(stderr, "  MaxShallowDepthError=%.1f\n", iLocConfig->MaxShallowDepthError);
        fprintf(stderr, "  MaxDeepDepthError=%.1f\n", iLocConfig->MaxDeepDepthError);
        fprintf(stderr, "  EtopoFile=%s\n", iLocConfig->EtopoFile);
        fprintf(stderr, "    EtopoNlat=%d EtopoNlon=%d EtopoRes=%.1f\n",
                iLocConfig->EtopoNlat, iLocConfig->EtopoNlon, iLocConfig->EtopoRes);

        fprintf(stderr, "Hypocenter data\n");
        fprintf(stderr, "  isManMade=%d\n", Hypocenter->isManMade);
        fprintf(stderr, "  FixHypo=%d\n", Hypocenter->FixHypo);
        fprintf(stderr, "  numSta=%d\n", Hypocenter->numSta);
        fprintf(stderr, "  numPhase=%d\n", Hypocenter->numPhase);
        fprintf(stderr, "  numUnknowns=%d\n", Hypocenter->numUnknowns);
        fprintf(stderr, "  Time=%.3f FixOT=%d\n",
                Hypocenter->Time, Hypocenter->FixOT);
        fprintf(stderr, "  Lat=%.3f FixLat=%d\n",
                Hypocenter->Lat, Hypocenter->FixLat);
        fprintf(stderr, "  Lon=%.3f FixLon=%d\n",
                Hypocenter->Lon, Hypocenter->FixLon);
        if (Hypocenter->FixHypo) {
            fprintf(stderr, "  Depth=%.2f FixDepth=%d\n",
                Hypocenter->Depth, Hypocenter->FixDepth);
        }
        else {
            fprintf(stderr, "  Depth=%.2f FixDepth=%d FixedDepthType=%d\n",
                Hypocenter->Depth, Hypocenter->FixDepth, Hypocenter->FixedDepthType);
            fprintf(stderr, "  Converged=%d\n", Hypocenter->Converged);
            if (Hypocenter->DepthDp != ILOC_NULLVAL)
                fprintf(stderr, "  DepthDp=%.2f DepthDpError=%.2f numDepthDp=%d\n",
                    Hypocenter->DepthDp, Hypocenter->DepthDpError, Hypocenter->numDepthDp);
            fprintf(stderr, "  numDefsta=%d numReading=%d numDef=%d numRank=%d\n",
                    Hypocenter->numDefsta, Hypocenter->numReading,
                    Hypocenter->numDef, Hypocenter->numRank);
            fprintf(stderr, "  numTimedef=%d numAzimdef=%d numSlowdef=%d\n",
                    Hypocenter->numTimedef, Hypocenter->numAzimdef,
                    Hypocenter->numSlowdef);
            fprintf(stderr, "  Gap=%.1f Sgap=%.1f minDist=%.1f maxDist=%.1f\n",
                    Hypocenter->Gap, Hypocenter->Sgap,
                    Hypocenter->minDist, Hypocenter->maxDist);
            fprintf(stderr, "  Model covariance matrix:\n");
            for (i = 0; i < 4; i++) {
                fprintf(stderr, "    ");
                for (j = 0; j < 4; j++)
                    fprintf(stderr, "%12.3f ", Hypocenter->ModelCov[i][j]);
                fprintf(stderr, "\n");
            }
            if (Hypocenter->semiMajax != ILOC_NULLVAL)
                fprintf(stderr, "  semiMajax=%.1f semiMinax=%.1f Strike=%.1f\n",
                        Hypocenter->semiMajax, Hypocenter->semiMinax,
                        Hypocenter->Strike);
            if (Hypocenter->Errors[0] != ILOC_NULLVAL)
                fprintf(stderr, "  stime=%.2f", Hypocenter->Errors[0]);
            if (Hypocenter->Errors[3] != ILOC_NULLVAL)
                fprintf(stderr, "  sdepth=%.2f", Hypocenter->Errors[3]);
            if (Hypocenter->Errors[1] != ILOC_NULLVAL)
                fprintf(stderr, "  slon=%.2f", Hypocenter->Errors[1]);
            if (Hypocenter->Errors[2] != ILOC_NULLVAL)
                fprintf(stderr, "  slat=%.2f", Hypocenter->Errors[2]);
            fprintf(stderr, "\n  uRMS=%.3f wRMS=%.3f SdevObs=%.3f\n",
                    Hypocenter->SdevObs, Hypocenter->uRMS, Hypocenter->wRMS);
            fprintf(stderr, "  GT5candidate=%d localSgap=%.1f localDU=%.3f ",
                    Hypocenter->GT5candidate, Hypocenter->localSgap,
                    Hypocenter->localDU);
            fprintf(stderr, "numStaWithin10km=%d localNumDefsta=%d localNumDef=%d\n",
                    Hypocenter->numStaWithin10km, Hypocenter->localNumDefsta,
                    Hypocenter->localNumDef);
        }

        fprintf(stderr, "Associated arrival data\n");
        for (i = 0; i < Hypocenter->numPhase; i++) {
            fprintf(stderr, "  arid=%d\n", Assocs[i].arid);
            fprintf(stderr, "  StaInd=%d\n", Assocs[i].StaInd);
            fprintf(stderr, "  PhaseHint=%s phaseFixed=%d Phase=%s\n",
                    Assocs[i].PhaseHint, Assocs[i].phaseFixed, Assocs[i].Phase);
            fprintf(stderr, "  Delta=%.2f Esaz=%.1f Seaz=%.1f Vmodel=%s\n",
                    Assocs[i].Delta, Assocs[i].Esaz, Assocs[i].Seaz, Assocs[i].Vmodel);
            fprintf(stderr, "  ArrivalTime=%.3f Timedef=%d TimeRes=%.3f Deltim=%.2f\n",
                    Assocs[i].ArrivalTime, Assocs[i].Timedef,
                    Assocs[i].TimeRes, Assocs[i].Deltim);
            fprintf(stderr, "  BackAzimuth=%.3f Azimdef=%d AzimRes=%.3f Delaz=%.2f\n",
                    Assocs[i].BackAzimuth, Assocs[i].Azimdef,
                    Assocs[i].AzimRes, Assocs[i].Delaz);
            fprintf(stderr, "  Slowness=%.3f Slowdef=%d SlowRes=%.3f Delslo=%.2f\n",
                    Assocs[i].Slowness, Assocs[i].Slowdef,
                    Assocs[i].SlowRes, Assocs[i].Delslo);
        }

        fprintf(stderr, "Station data\n");
        for (i = 0; i < Hypocenter->numSta; i++)
            fprintf(stderr, "  %d StaLat=%.3f StaLon=%.3f StaElevation=%.3f\n",
                i, StaLocs[i].StaLat, StaLocs[i].StaLon, StaLocs[i].StaElevation);
        fprintf(stderr, "\n");
    }
}
/*  EOF  */

