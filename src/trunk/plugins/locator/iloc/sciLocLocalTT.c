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
 * Public functions:
 *     iLoc_GenerateLocalTTtables
 */
/*
 * Local functions
 */
static int ReadLocalVelocityModel(char *fname, ILOC_VMODEL *LocalVelocityModelp,
        ILOC_TTINFO *LocalTTinfo);
static void FreeLocalVelocityModel(ILOC_VMODEL *LocalVelocityModelp);
static int GenerateLocalTT(double depth, double delta,
        ILOC_VMODEL *LocalVelocityModelp, ILOC_PHASELIST *phcd,
        double *ttc, double *dtdd, double *dtdh,
        double *tref, double *tid, double *did);
static double DirectPhase(int n, double *v, double *vsq, double *thk, int iq,
        double dq, double delta, double depth, double *tdir);
static int RefractedPhase(int n, double *v, double *vsq, double *thk, int iq,
        double dq, double delta, double *tref, double *tid, double *did);
static void CriticalDistanceTTIntercept(int n, double *v, double *vsq,
        double *thk, int iq, double *tid, double *did);

/*
 * file scope globals
 */
#define ILOC_NDEP 33
#define ILOC_NDIS 20


/*
 *  Title:
 *     GenerateLocalTTtables
 *  Desc:
 *     Generate travel-time tables from local velocity model
 *  Input Arguments:
 *     auxdir - pathname for the auxiliary data files directory
 *     TTInfo - TTInfo structure
 *  Return:
 *     tt_tables - pointer to ILOC_TT_TABLE structure or NULL on error
 *  Calls:
 *     ReadLocalVelocityModel, GenerateLocalTT, iLoc_GetPhaseIndex,
 *     FreeLocalVelocityModel, iLoc_Free
 */
ILOC_TT_TABLE *iLoc_GenerateLocalTTtables(char *auxdir, ILOC_TTINFO *LocalTTInfo,
        int verbose)
{
    ILOC_PHASELIST *phcd = (ILOC_PHASELIST *)NULL;
    double *tref  = (double *)NULL;
    double *tid  = (double *)NULL;
    double *did  = (double *)NULL;
    double *ttc  = (double *)NULL;
    double *dtdd = (double *)NULL;
    double *dtdh = (double *)NULL;
    double *h = (double *)NULL;
    ILOC_TT_TABLE *tt_tables = (ILOC_TT_TABLE *)NULL;
    ILOC_VMODEL LocalVelocityModel;
    char filename[ILOC_FILENAMELEN];
    double hmax, hd, delta, depth;
    int npha, n, i, j, k, ind, ndists, ndepths, icon, imoh;
    double dists[ILOC_NDIS] = {
          0.0,  0.025, 0.05, 0.1, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5,
          1.75, 2.0,   2.5,  3.0, 3.5,  4.0, 4.5,  5.0, 5.5,  6.0
    };
    double depths[ILOC_NDEP] = {
          0.0,   1.0,   2.5,   5.0,   7.5,  10.0,  12.5,  15.0,  17.5,  20.0,
         22.5,  25.0,  27.5,  30.0,  32.5,  35.0,  40.0,  45.0,  50.0,  75.0,
        100.0, 150.0, 200.0, 250.0, 300.0, 350.0, 400.0, 450.0, 500.0, 550.0,
        600.0, 650.0, 700.0
    };
    ndists = ILOC_NDIS;
/*
 *  read local velocity model
 */
    sprintf(filename, "%s/localmodels/%s.localmodel.dat",
            auxdir, LocalTTInfo->TTmodel);
    if (ReadLocalVelocityModel(filename, &LocalVelocityModel, LocalTTInfo))
        return (ILOC_TT_TABLE *)NULL;
    n = LocalVelocityModel.n;
    icon = LocalVelocityModel.iconr;
    imoh = LocalVelocityModel.imoho;
    hmax = LocalVelocityModel.h[n-1] - 1.;
    if (verbose > 2) {
/*
 *      print local velocity model
 */
        fprintf(stderr, "    LAYER   DEPTH   VP    VS\n");
        for (i = 0; i < n; i++) {
            if (i == icon)
                fprintf(stderr, "%9d %7.3f %5.3f %5.3f CONRAD\n",
                        i, LocalVelocityModel.h[i],
                        LocalVelocityModel.vp[i], LocalVelocityModel.vs[i]);
            else if (i == imoh)
                fprintf(stderr, "%9d %7.3f %5.3f %5.3f MOHO\n",
                        i, LocalVelocityModel.h[i],
                        LocalVelocityModel.vp[i], LocalVelocityModel.vs[i]);
            else
                fprintf(stderr, "%9d %7.3f %5.3f %5.3f\n",
                        i, LocalVelocityModel.h[i],
                        LocalVelocityModel.vp[i], LocalVelocityModel.vs[i]);
        }
    }
/*
 *  generate depth samples
 */
    k = ILOC_NDEP + 5;
    if ((h = (double *)calloc(k, sizeof(double))) == NULL) {
        FreeLocalVelocityModel(&LocalVelocityModel);
        fprintf(stderr, "GenerateLocalTTtables: cannot allocate memory\n");
        return (ILOC_TT_TABLE *) NULL;
    }
    k = 0;
    if (icon) {
        hd = LocalVelocityModel.h[icon] - 0.01;
        for (i = 0; depths[i] < hd; i++)
            h[k++] = depths[i];
        h[k++] = hd;
        if (LocalVelocityModel.h[icon] <= depths[i]) {
            h[k++] = LocalVelocityModel.h[icon];
            h[k++] = LocalVelocityModel.h[icon] + 0.01;
            i++;
        }
        if (imoh) {
            hd = LocalVelocityModel.h[imoh] - 0.01;
            for (; depths[i] < hd; i++)
                h[k++] = depths[i];
            h[k++] = hd;
            if (LocalVelocityModel.h[imoh] <= depths[i]) {
                h[k++] = LocalVelocityModel.h[imoh];
                h[k++] = LocalVelocityModel.h[imoh] + 0.01;
                i++;
            }
            for (; depths[i] < hmax; i++)
                h[k++] = depths[i];
        }
        else {
            for (; depths[i] < hmax; i++)
                h[k++] = depths[i];
        }
    }
    else if (imoh) {
        hd = LocalVelocityModel.h[imoh] - 0.01;
        for (i = 0; depths[i] < hd; i++)
            h[k++] = depths[i];
        h[k++] = hd;
        if (LocalVelocityModel.h[imoh] <= depths[i]) {
            h[k++] = LocalVelocityModel.h[imoh];
            h[k++] = LocalVelocityModel.h[imoh] + 0.01;
        }
        for (; depths[i] < hmax; i++)
            h[k++] = depths[i];
    }
    else {
        for (i = 0; depths[i] < hmax; i++)
            h[k++] = depths[i];
    }
/*
 *  memory allocations
 */
    ndepths = k;
    tref = (double *)calloc(n, sizeof(double));
    tid = (double *)calloc(n, sizeof(double));
    did = (double *)calloc(n, sizeof(double));
    ttc = (double *)calloc(LocalTTInfo->numPhaseTT, sizeof(double));
    dtdd = (double *)calloc(LocalTTInfo->numPhaseTT, sizeof(double));
    dtdh = (double *)calloc(LocalTTInfo->numPhaseTT, sizeof(double));
    phcd = (ILOC_PHASELIST *)calloc(LocalTTInfo->numPhaseTT, sizeof(ILOC_PHASELIST));
    tt_tables = (ILOC_TT_TABLE *)calloc(LocalTTInfo->numPhaseTT, sizeof(ILOC_TT_TABLE));
    if (tt_tables == NULL) {
        iLoc_Free(tref); iLoc_Free(ttc); iLoc_Free(dtdd); iLoc_Free(dtdh);
        iLoc_Free(phcd); iLoc_Free(tid); iLoc_Free(did); iLoc_Free(h);
        FreeLocalVelocityModel(&LocalVelocityModel);
        fprintf(stderr, "GenerateLocalTTtables: cannot allocate memory\n");
        return (ILOC_TT_TABLE *) NULL;
    }
    for (ind = 0; ind < LocalTTInfo->numPhaseTT; ind++) {
        ttc[i] = dtdd[i] = dtdh[i] = -999.;
        strcpy(tt_tables[ind].Phase, LocalTTInfo->PhaseTT[ind].Phase);
        tt_tables[ind].ndel = ndists;
        tt_tables[ind].ndep = ndepths;
        tt_tables[ind].isbounce = 0;
        tt_tables[ind].deltas = (double *)calloc(ndists, sizeof(double));
        tt_tables[ind].depths = (double *)calloc(ndepths, sizeof(double));
        tt_tables[ind].bpdel = (double **)NULL;
        tt_tables[ind].tt = iLoc_AllocateFloatMatrix(ndists, ndepths);
        tt_tables[ind].dtdd = iLoc_AllocateFloatMatrix(ndists, ndepths);
        tt_tables[ind].dtdh = iLoc_AllocateFloatMatrix(ndists, ndepths);
        if (tt_tables[ind].dtdh == NULL) {
            iLoc_FreeTTtables(LocalTTInfo->numPhaseTT, tt_tables);
            iLoc_Free(tref); iLoc_Free(ttc); iLoc_Free(dtdd); iLoc_Free(dtdh);
            iLoc_Free(phcd); iLoc_Free(tid); iLoc_Free(did); iLoc_Free(h);
            FreeLocalVelocityModel(&LocalVelocityModel);
            fprintf(stderr, "GenerateLocalTTtables: cannot allocate memory\n");
            return (ILOC_TT_TABLE *) NULL;
        }
/*
 *      initializations
 */
        for (j = 0; j < ndepths; j++)
            tt_tables[ind].depths[j] = h[j];
        for (i = 0; i < ndists; i++) {
            tt_tables[ind].deltas[i] = dists[i];
            for (j = 0; j < ndepths; j++) {
                tt_tables[ind].tt[i][j] = ILOC_NULLVAL;
                tt_tables[ind].dtdd[i][j] = -999.;
                tt_tables[ind].dtdh[i][j] = -999.;
            }
        }
    }
    iLoc_Free(h);
/*
 *  generate local TT tables: direct and refracted P and S phases
 */
    for (i = 0; i < ndists; i++) {
        delta = tt_tables[0].deltas[i];
        for (j = 0; j < ndepths; j++) {
            depth = tt_tables[0].depths[j];
            npha = GenerateLocalTT(depth, delta, &LocalVelocityModel,
                                   phcd, ttc, dtdd, dtdh, tref, tid, did);
            if (npha) {
                for (k = 0; k < npha; k++) {
                    if ((ind = iLoc_GetPhaseIndex(phcd[k].Phase, LocalTTInfo)) < 0)
                        continue;
                    tt_tables[ind].tt[i][j] = ttc[k];
                    tt_tables[ind].dtdd[i][j] = dtdd[k];
                    tt_tables[ind].dtdh[i][j] = dtdh[k];
                    if (ttc[k] < 0.) continue;
/*
 *                  first arriving P
 */
                    if (LocalTTInfo->PhaseTT[ind].Phase[0] == 'P' &&
                        ttc[k] < tt_tables[0].tt[i][j]) {
                        tt_tables[0].tt[i][j] = ttc[k];
                        tt_tables[0].dtdd[i][j] = dtdd[k];
                        tt_tables[0].dtdh[i][j] = dtdh[k];
                    }
/*
 *                  first arriving S
 */
                    if (LocalTTInfo->PhaseTT[ind].Phase[0] == 'S' &&
                        ttc[k] < tt_tables[1].tt[i][j]) {
                        tt_tables[1].tt[i][j] = ttc[k];
                        tt_tables[1].dtdd[i][j] = dtdd[k];
                        tt_tables[1].dtdh[i][j] = dtdh[k];
                    }
                }
            }
        }
    }
/*
 *  set ILOC_NULLVAL to the default -999 in tt
 */
    for (ind = 0; ind < LocalTTInfo->numPhaseTT; ind++) {
        for (i = 0; i < ndists; i++) {
            for (j = 0; j < ndepths; j++) {
                if (tt_tables[ind].tt[i][j] == ILOC_NULLVAL)
                    tt_tables[ind].tt[i][j] = -999.;
            }
        }
    }
/*
 *  free memory
 */
    iLoc_Free(tref); iLoc_Free(ttc); iLoc_Free(dtdd); iLoc_Free(dtdh);
    iLoc_Free(phcd); iLoc_Free(tid); iLoc_Free(did);
    FreeLocalVelocityModel(&LocalVelocityModel);
    return tt_tables;
}

/*
 *  Title:
 *     GenerateLocalTT
 *  Desc:
 *     Calculate travel-times from local velocity model for (depth, delta)
 *  Input Arguments:
 *     depth - source depth in km
 *     delta - receiver distance in degrees
 *     LocalVelocityModelp - local velocity model
 *     tref - auxiliary vector for head wave travel times in each layer
 *  Output Arguments:
 *     phcd - phasenames with valid TT prediction
 *     ttc  - TT predictions
 *     dtdd - horizontal slowness predictions
 *     dtdh - vertical slowness predictions
 *  Return:
 *     npha - number of phases with valid TT prediction
 *  Calls:
 *     DirectPhase, RefractedPhase
 */
static int GenerateLocalTT(double depth, double delta,
        ILOC_VMODEL *LocalVelocityModelp, ILOC_PHASELIST *phcd,
        double *ttc, double *dtdd, double *dtdh,
        double *tref, double *tid, double *did)
{
    char phase[ILOC_PHALEN], wtype[ILOC_PHALEN], lag[ILOC_PHALEN];
    double *v, *vsq, *thk, tdir, u, dq, dkm;
    int iq, i, ks, n, npha, icon, imoh, noref;
    n = LocalVelocityModelp->n;
    icon = LocalVelocityModelp->iconr;
    imoh = LocalVelocityModelp->imoho;
    thk = LocalVelocityModelp->thk;
    dkm = delta * ILOC_DEG2KM;
/*
 *  cannot generate TT for depths below the bottom of the velocity model
 */
    if (depth > LocalVelocityModelp->h[n-1])
        return 0;
/*
 *  phase loop for S and P type waves
 */
    npha = 0;
    for (ks = 0; ks < 2; ks++) {
        if (ks) {
            v = LocalVelocityModelp->s;
            vsq = LocalVelocityModelp->s2;
            strcpy(wtype, "S");
        }
        else {
            v = LocalVelocityModelp->p;
            vsq = LocalVelocityModelp->p2;
            strcpy(wtype, "P");
        }
/*
 *      get event layer (iq), get depth of event from top of event layer (dq)
 *      make sure that source depth doesn't fall on layer boundary
 */
        for (i = 0; i < n; i++) {
            if (fabs(depth - LocalVelocityModelp->z[i]) < 0.0001)
                depth -= 0.001;
        }
        if (depth < 0.)
            depth = 0.001;
        iq = n;
        for (i = 0; i < n - 1; i++) {
            if (depth >= LocalVelocityModelp->z[i] &&
                depth <  LocalVelocityModelp->z[i+1]) {
                iq = i;
                dq = depth - LocalVelocityModelp->z[i];
            }
        }
/*
 *      direct wave Pg/Pb/Pn, Sg/Sb/Sn
 *
 *      takeoff angle = u
 *      horizontal slowness = sin(u) / v[iq]
 *      vertical slowness   = cos(u) / v[iq]
 */
        u = DirectPhase(n, v, vsq, thk, iq, dq, dkm, depth, &tdir);
        if (iq < imoh) {
            if (iq < icon) strcpy(lag, "g");
            else           strcpy(lag, "b");
        }
        else {
            strcpy(lag, "n");
        }
        if (depth > 400.) strcpy(lag, "");
	    strcpy(phase, wtype);
	    strcat(phase, lag);
        strcpy(phcd[npha].Phase, phase);
	    ttc[npha] = tdir;
        dtdh[npha] = -cos(u) / v[iq];
	    dtdd[npha] = sin(u) * ILOC_DEG2KM / v[iq];
	    npha++;
	    if (ILOC_STREQ(phase, "Sg")) {
/*
 *          duplicate Sg as Lg
 */
            strcpy(phcd[npha].Phase, "Lg");
	        ttc[npha] = tdir;
            dtdh[npha] = -cos(u) / v[iq];
	        dtdd[npha] = sin(u) * ILOC_DEG2KM / v[iq];
	        npha++;
	    }
/*
 *      refracted waves
 *
 *      takeoff angle =  u  = asin(v[iq] / v[imoh])
 *      horizontal slowness = sin(u) / v[iq] = 1 / v[imoh]
 *      vertical slowness   = cos(u) / v[iq] = ILOC_SQRT(1 - sin2(u)) / v[iq]
 *                          = ILOC_SQRT(vsq[imoh] - vsq[iq]) / (v[imoh] * v[iq])
 */
        if (iq > imoh)
           continue;
        noref = RefractedPhase(n, v, vsq, thk, iq, dq, dkm, tref, tid, did);
        if (noref)
            continue;
        if (iq < icon) {
/*
 *          Pb/Sb
 */
            if (tref[icon] < 999999.) {
                strcpy(lag, "b");
	            strcpy(phase, wtype);
	            strcat(phase, lag);
                strcpy(phcd[npha].Phase, phase);
	            ttc[npha] = tref[icon];
                dtdh[npha] = -ILOC_SQRT(vsq[icon] - vsq[iq]) / (v[icon] * v[iq]);
	            dtdd[npha] = ILOC_DEG2KM / v[icon];
	            npha++;
	        }
	    }
/*
 *      Pn/Sn
 */
        if (tref[imoh] < 999999.) {
            strcpy(lag, "n");
	        strcpy(phase, wtype);
	        strcat(phase, lag);
            strcpy(phcd[npha].Phase, phase);
	        ttc[npha] = tref[imoh];
            dtdh[npha] = -ILOC_SQRT(vsq[imoh] - vsq[iq]) / (v[imoh] * v[iq]);
	        dtdd[npha] = ILOC_DEG2KM / v[imoh];
	        npha++;
	    }
	}
	return npha;
}

/*
 *  Title:
 *     DirectPhase
 *  Synopsis:
 *	   Computes DirectPhase wave travel time and takeoff angle.
 *     To find the takeoff angle of the ray the regula falsi method
 *     is applied. From u and x, tdir is found by summing the travel
 *     time in each layer.  finally, a slight correction to tdir is made,
 *     based on the misfit between the final del and delta.
 *  Input Arguments:
 *     n     - number of layers
 *     v     - velocities
 *     vsq   - velocity squares
 *     thk   - layer thicknesses
 *     iq    - event layer
 *     dq    - depth of event in event layer
 *     delta - epicentral distance in km
 *     depth - depth of event
 *  Output Arguments:
 *     tdir - DirectPhase wave travel time
 *  Return:
 *     u - takeoff angle
 *  Called by:
 *     GenerateLocalTT
 */
static double DirectPhase(int n, double *v, double *vsq, double *thk, int iq,
        double dq, double delta, double depth, double *tdir)
{
    int i, j, imax;
    double u, ua, ub, usq, uasq, ubsq, xa, xb, x, del, dela, delb;
    double t, dmax, vmax;
/*
 *  source is in the surface layer
 */
    if (iq == 0) {
        x = ILOC_SQRT(depth * depth + delta * delta);
        *tdir = x / v[iq];
        u = 0.;
        if (x > 0.)
            u = ILOC_PI - asin(delta / x);
        return u;
    }
/*
 *  Find the fastest layer above and including event layer
 */
    imax = iq;
    dmax = dq;
    vmax = v[iq];
    for (i = 0; i < iq; i++) {
        if (v[i] > vmax) {
            imax = i;
            dmax = thk[i];
            vmax = v[i];
        }
    }
/*
 *  Initial bounds on the sine of takeoff angle
 *               v[iq]          v[iq]           delta
 *      sin(i) = ----- sin(j) = ----- * -------------------------
 *               vmax           vmax    sqrt(depth**2 + delta**2)
 *
 *      x = dq * tan(i)
 */
    ua = v[iq] * delta / (vmax * ILOC_SQRT(depth * depth + delta * delta));
    x = ILOC_SQRT(dmax * dmax + delta * delta);
    ub = 0.999999;
    if (x > 0.)
        ub = v[iq] * delta / (vmax * x);
    uasq = ua * ua;
    ubsq = ub * ub;
    xa = dq * ua / ILOC_SQRT(1. - uasq);
    if (imax == iq) {
        xb = delta;
        if (delta < 0.001) xb += 0.05;
    }
    else
        xb = dq * ub / ILOC_SQRT(1. - ubsq);
    dela = xa;
    delb = xb;
    for (i = 0; i < iq; i++) {
        dela += thk[i] * ua / ILOC_SQRT(vsq[iq] / vsq[i] - uasq);
        delb += thk[i] * ub / ILOC_SQRT(vsq[iq] / vsq[i] - ubsq);
    }
/*
 *  Root finding by regula falsi method
 *     find u so that del = delta
 */
    for (j = 0; j < 25; j++) {
        if ((delb - dela) < 0.01) {
            x = 0.5 * (xa + xb);
            u = x / ILOC_SQRT(dq *dq + x * x);
            usq = u * u;
            break;
        }
        x = xa + (delta - dela) * (xb - xa) / (delb - dela);
        u = x / ILOC_SQRT(dq *dq + x * x);
        usq = u * u;
        del = x;
        for (i = 0; i < iq; i++)
            del += thk[i] * u / ILOC_SQRT(vsq[iq] / vsq[i] - usq);
        if (fabs(del - delta) < 0.01)
            break;
        if ((del - delta) < 0.) {
            xa = x;
            dela = del;
        }
        else {
            xb = x;
            delb = del;
        }
    }
/*
 *  travel time and takeoff angle of DirectPhase ray
 */
    t = ILOC_SQRT(dq * dq + x * x) / v[iq];
    for (i = 0; i < iq; i++)
        t += thk[i] * v[iq] / (vsq[i] * ILOC_SQRT(vsq[iq] / vsq[i] - usq));
    t -= u * (del - delta) / v[iq];
    *tdir = t;
    u = ILOC_PI - asin(u);
    return u;
}

/*
 *  Title:
 *     RefractedPhase
 *  Synopsis:
 *	   Computes head wave travel times from the Conrad and Moho.
 *     There may not be a RefractedPhaseed ray, either because all layers
 *     below the event layer are low velocity layers or because for
 *     all layers below the event layer which are not low velocity
 *     layers the critical distance exceeds delta.
 *  Input Arguments:
 *     n     - number of layers
 *     v     - velocities
 *     vsq   - velocity squares
 *     thk   - layer thicknesses
 *     iq    - event layer
 *     dq    - depth of event in event layer
 *     delta - epicentral distance in km
 *  Output Arguments:
 *     tref - head wave travel times (999999. if not exists)
 *  Return:
 *     0/1 on success/error
 *  Called by:
 *     GenerateLocalTT
 *  Calls:
 *     CriticalDistanceTTIntercept
 */
static int RefractedPhase(int n, double *v, double *vsq, double *thk, int iq,
        double dq, double delta, double *tref, double *tid, double *did)
{
    int noref = 0, m;
    double didq, tinq, tmin, sqt;

    CriticalDistanceTTIntercept(n, v, vsq, thk, iq, tid, did);
    tmin = 999999.;
    for (m = iq + 1; m < n; m++) {
        tref[m] = 999999.;
        if (tid[m] < 999999.) {
            sqt = ILOC_SQRT(vsq[m] - vsq[iq]);
            tinq = tid[m] - dq * sqt / (v[m] * v[iq]);
            didq = did[m] - dq * v[iq] / sqt;
            tref[m] = tinq + delta / v[m];
            if (didq > delta)
                tref[m] = 999999.;
            if (tref[m] < tmin)
                tmin = tref[m];
        }
    }
    if (tmin > 999998.)
        noref = 1;
    return noref;
}

/*
 *  Title:
 *     CriticalDistanceTTIntercept
 *  Synopsis:
 *     Compute intercept times and critical distances for refracted rays
 *     Determines the travel time intercept and critical distance for
 *     a seismic ray in a layered earth model originating at the top of
 *     layer iq, refracted in layer m, and terminating at the surface.
 *  Input Arguments:
 *     n   - number of layers
 *     v   - velocities
 *     vsq - velocity squares
 *     thk - layer thicknesses
 *     iq  - event layer
 *  Output Arguments:
 *     tid - travel time intercepts (999999. if not exists)
 *     did - critical distances
 *  Called by:
 *     RefractedPhase
 */
static void CriticalDistanceTTIntercept(int n, double *v, double *vsq,
        double *thk, int iq, double *tid, double *did)
{
    int i, m;
    double tid1, tid2, did1, did2, sqt, tim, dim;
    for (m = iq + 1; m < n; m++) {
        tid[m] = 0.;
        did[m] = 0.;
        tid1 = tid2 = did1 = did2 = 0.;
        for (i = 0; i < m; i++) {
            if (vsq[m] <= vsq[i]) {
                tid[m] = 999999.;
                did[m] = 999999.;
            }
            else {
                sqt = ILOC_SQRT(vsq[m] - vsq[i]);
                tim = thk[i] * sqt / (v[i] * v[m]);
                dim = thk[i] * v[i] / sqt;
                if (i < iq) {
/*
 *                  sum for layers above event layer
 */
                    tid1 += tim;
                    did1 += dim;
                }
                else {
/*
 *                  sum for layers below and including the event layer
 */
                    tid2 += tim;
                    did2 += dim;
                }
            }
        }
        if (tid[m] < 999999.) {
            tid[m] = tid1 + 2. * tid2;
            did[m] = did1 + 2. * did2;
        }
    }
}


/*
 *  Title:
 *     ReadLocalVelocityModel
 *  Desc:
 *     Read local velocity model
 *  Input Arguments:
 *     fname - pathname for local velocity model
 *  Output Arguments:
 *     LocalVelocityModelp - pointer to ILOC_VMODEL structure
 *     TTInfo - TTInfo structure
 *  Return:
 *     0/1 on success/error
 *  Calls:
 *     iLoc_Free
 */
static int ReadLocalVelocityModel(char *fname, ILOC_VMODEL *LocalVelocityModelp,
        ILOC_TTINFO *LocalTTinfo)
{
    FILE *fp;
    char *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    char layname[ILOC_VALLEN];
    int n = 0, i = 0, j, k = 0;
    double x, y, z, b;
/*
 *  open local velocity model file
 */
    if ((fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "ReadLocalVelocityModel: cannot open %s\n", fname);
        return ILOC_CANNOT_OPEN_FILE;
    }
/*
 *  read local velocity model
 */
    LocalVelocityModelp->iconr = LocalVelocityModelp->imoho = 0;
    i = 0;
    k = 0;
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#' || line[0] == '\n')         /* skip comments */
            continue;
        if (k == 0) {
/*
 *          number of layers
 */
            sscanf(line, "%d", &n);
            LocalVelocityModelp->n = n;
/*
 *          memory allocations
 */
            LocalVelocityModelp->z = (double *)calloc(n, sizeof(double));
            LocalVelocityModelp->thk = (double *)calloc(n, sizeof(double));
            LocalVelocityModelp->p = (double *)calloc(n, sizeof(double));
            LocalVelocityModelp->s = (double *)calloc(n, sizeof(double));
            LocalVelocityModelp->p2 = (double *)calloc(n, sizeof(double));
            LocalVelocityModelp->s2 = (double *)calloc(n, sizeof(double));
            LocalVelocityModelp->h = (double *)calloc(n, sizeof(double));
            LocalVelocityModelp->vp = (double *)calloc(n, sizeof(double));
            if ((LocalVelocityModelp->vs = (double *)calloc(n, sizeof(double))) == NULL) {
                fprintf(stderr, "ReadLocalVelocityModel: cannot allocate memory!\n");
                iLoc_Free(LocalVelocityModelp);
                iLoc_Free(line);
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
            k++;
        }
        else {
/*
 *          velocity model:
 *              h [km], Vp [km/s], Vs [km/s], discontinuity (x|CONRAD|MOHO)
 *          Earth flattening yields z, p, s, p2, s2 and thk
 */
            sscanf(line, "%lf%lf%lf%s", &z, &x, &y, layname);
            b = ILOC_EARTHRADIUS / (ILOC_EARTHRADIUS - z);
            LocalVelocityModelp->h[i] = z;
            LocalVelocityModelp->z[i] = ILOC_EARTHRADIUS * log(b);
            LocalVelocityModelp->vp[i] = x;
            LocalVelocityModelp->p[i] = b * x;
            LocalVelocityModelp->p2[i] = b * x * b * x;
            LocalVelocityModelp->vs[i] = y;
            LocalVelocityModelp->s[i] = b * y;
            LocalVelocityModelp->s2[i] = b * y * b * y;
            if (ILOC_STREQ(layname, "CONRAD")) {
                LocalTTinfo->Conrad = z;
                LocalVelocityModelp->iconr = i;
            }
            if (ILOC_STREQ(layname, "MOHO")) {
                LocalTTinfo->Moho = z;
                LocalVelocityModelp->imoho = i;
            }
            i++;
        }
    }
    fclose(fp);
    for (i = 0; i < n - 1; i++) {
        LocalVelocityModelp->thk[i] = LocalVelocityModelp->z[i+1] -
                                      LocalVelocityModelp->z[i];
    }
    LocalVelocityModelp->thk[i] = 10.;
    iLoc_Free(line);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     FreeLocalVelocityModel
 *  Desc:
 *     frees memory allocated to ILOC_VMODEL structure
 *  Input Arguments:
 *     LocalVelocityModelp - pointer to ILOC_VMODEL structure
 *  Calls:
 *     iLoc_Free
 */
static void FreeLocalVelocityModel(ILOC_VMODEL *LocalVelocityModelp)
{
    iLoc_Free(LocalVelocityModelp->z);
    iLoc_Free(LocalVelocityModelp->thk);
    iLoc_Free(LocalVelocityModelp->p);
    iLoc_Free(LocalVelocityModelp->s);
    iLoc_Free(LocalVelocityModelp->p2);
    iLoc_Free(LocalVelocityModelp->s2);
    iLoc_Free(LocalVelocityModelp->h);
    iLoc_Free(LocalVelocityModelp->vp);
    iLoc_Free(LocalVelocityModelp->vs);
}

