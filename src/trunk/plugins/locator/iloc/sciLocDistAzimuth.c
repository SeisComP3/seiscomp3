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
 *    iLoc_GetDeltaAzimuth
 *    iLoc_DistAzimuth
 *    iLoc_PointAtDeltaAzimuth
 */

/*
 *  Title:
 *     iLoc_GetDeltaAzimuth
 *  Synopsis:
 *     Calculates delta, esaz and seaz for each phase.
 *  Input Arguments:
 *     Hypocenter - ILOC_HYPO structure
 *     Assocs     - array of ILOC_ASSOC structures
 *     StaLocs    - array of ILOC_STA structures
 *  Called by:
 *     iLoc_Locator, LocateEvent, NAForwardProblem
 *  Calls:
 *     iLoc_DistAzimuth
 */
void iLoc_GetDeltaAzimuth(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs,
        ILOC_STA *StaLocs)
{
    int i, j;
    for (i = 0; i < Hypocenter->numPhase; i++) {
        j = Assocs[i].StaInd;
        Assocs[i].Delta = iLoc_DistAzimuth(StaLocs[j].StaLat, StaLocs[j].StaLon,
                                           Hypocenter->Lat, Hypocenter->Lon,
                                           &Assocs[i].Seaz, &Assocs[i].Esaz);
    }
}

/*
 *  Title:
 *     iLoc_DistAzimuth
 *  Synopsis:
 *     Computes the angular distance, azimuth and back azimuth between
 *     two geographic latitude and longitude positions
 *     (typically measured from a station to an event).
 *  Input Arguments:
 *     slat  - geographic latitude  of point 1 [deg]
 *     slon  - geographic longitude of point 1 [deg]
 *     elat  - geographic latitude  of point 2 [deg]
 *     elon  - geographic longitude of point 2 [deg]
 *  Output Arguments:
 *     azi   - geocentric azimuth of point 2 w.r.t. point 1 (seaz)
 *     baz   - geocentric back-azimuth (esaz)
 *  Return:
 *     delta - geocentric distance between points 1 and 2 [deg]
 *  Called by:
 *     iLoc_GetDeltaAzimuth, iLoc_GetDistanceMatrix
 */
double iLoc_DistAzimuth(double slat, double slon, double elat, double elon,
              double *azi, double *baz)
{
    double cslat = 0., sslat = 0., cdlon = 0., sdlon = 0., rdlon = 0.;
    double geoc_slat = 0., geoc_elat = 0.;
    double xazi = 0., xbaz = 0., yazi = 0., ybaz = 0., delta = 0., cdel = 0.;
    static double celat = 0., selat = 0.;
    double f = (1. - ILOC_FLATTENING) * (1. - ILOC_FLATTENING);
    if (fabs(slat - elat) < ILOC_DEPSILON && fabs(slon - elon) < ILOC_DEPSILON) {
        delta = 0.0;
        *azi = 0.0;
        *baz = 180.0;
        return delta;
    }
/*
 *  Convert elat from geographic latitude to geocentric latitude
 */
    geoc_elat = atan(f * tan(ILOC_DEG2RAD * elat));
    celat = cos(geoc_elat);
    selat = sin(geoc_elat);
/*
 *  Convert slat from geographic latitude to geocentric latitude
 */
    geoc_slat = atan(f * tan(ILOC_DEG2RAD * slat));
/*
 *  Distance
 */
    rdlon = ILOC_DEG2RAD * (elon - slon);
    cslat = cos(geoc_slat);
    sslat = sin(geoc_slat);
    cdlon = cos(rdlon);
    sdlon = sin(rdlon);
    cdel = sslat * selat + cslat * celat * cdlon;
    cdel = (cdel <  1.0) ? cdel :  1.0;
    cdel = (cdel > -1.0) ? cdel : -1.0;
    delta = ILOC_RAD2DEG * acos(cdel);
/*
 *  Azimuth, back azimuth
 */
    yazi = sdlon * celat;
    xazi = cslat * selat - sslat * celat * cdlon;
    ybaz = -sdlon * cslat;
    xbaz = celat * sslat - selat * cslat * cdlon;
    *azi = ILOC_RAD2DEG * atan2(yazi, xazi);
    *baz = ILOC_RAD2DEG * atan2(ybaz, xbaz);
    if (*azi < 0.0) *azi += 360.0;
    if (*baz < 0.0) *baz += 360.0;
    return delta;
}

/*
 *  Title:
 *     iLoc_PointAtDeltaAzimuth
 *  Synopsis:
 *     Calculates geographic lat, lon of a point which is
 *         delta distance away in azim direction from a point on the Earth.
 *  Input Arguments:
 *     lat1  - geographic latitude  of point 1 [deg]
 *     lon1  - geographic longitude of point 1 [deg]
 *     delta - geocentric distance between points 1 and 2 [deg]
 *     azim  - geocentric azimuth of point 2 w.r.t. point 1
 *  Output Arguments:
 *     lat2  - geographic latitude position of point 2 [deg]
 *     lon2  - geographic longitude position of point 2 [deg]
 *  Called by:
 *     LocateEvent, tolatlon, GetBounceCorrection
 */
void iLoc_PointAtDeltaAzimuth(double lat1, double lon1, double delta,
              double azim, double *lat2, double *lon2)
{
    double a = 0., b = 0., c = 0., sinlat = 0., coslat = 0., dlon = 0.;
    double geoc_co_lat = 0.;
    double r13 = 0., r13sq = 0.;
    double x1 = 0., x2 = 0., x3 = 0., dist = 0., azi = 0.;
    double f = (1. - ILOC_FLATTENING) * (1. - ILOC_FLATTENING);
/*
 *  Convert a geographical location to geocentric cartesian coordinates.
 */
    dist = ILOC_DEG2RAD * (90.0 - delta);
    azi = ILOC_DEG2RAD * (180.0 - azim);
    r13 = cos(dist);
/*
 *  x1: Axis 1 intersects equator at  0 deg longitude
 *  x2: Axis 2 intersects equator at 90 deg longitude
 *  x3: Axis 3 intersects north pole
 */
    x1 = r13 * sin(azi);
    x2 = sin(dist);
    x3 = r13 * cos(azi);
    geoc_co_lat = ILOC_PI2 - atan(f * tan(ILOC_DEG2RAD * lat1));
/*
 *  Rotate in cartesian coordinates. The cartesian coordinate system
 *  is most easily described in geocentric terms. The origin is at
 *  the Earth's center. Rotation by lat1 degrees southward, about
 *  the 1-axis.
 */
    sinlat = sin(geoc_co_lat);
    coslat = cos(geoc_co_lat);
    b = x2;
    c = x3;
    x2 = b * coslat - c * sinlat;
    x3 = b * sinlat + c * coslat;
/*
 *  Finally, convert geocentric cartesian coordinates back to
 *  a geographical location.
 */
    r13sq = x3 * x3 + x1 * x1;
    r13 = ILOC_SQRT(r13sq);
    dlon = ILOC_RAD2DEG * atan2(x1, x3);
    a = atan2(x2, r13);
    *lat2 = ILOC_RAD2DEG * atan(tan(a) / f);
    *lon2 = lon1 + dlon;
    if (fabs(*lat2) > 90.0) {
        a = 180.0 - fabs(*lat2);
        *lat2 = (*lat2 >= 0.) ? -a : a;
    }
    if (fabs(*lon2) > 180.0) {
        a = 360.0 - fabs(*lon2);
        *lon2 = (*lon2 >= 0.) ? -a : a;
    }
}

/*  EOF  */
