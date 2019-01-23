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
/*
 * iLocInterface.h
 *    Istvan Bondar
 *    ibondar2014@gmail.com
 */
#ifndef ILOCINTERFACE_H
#define ILOCINTERFACE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
/*
 * Lapack (MacOS)
 */
#ifdef MACOSX
#include <Accelerate/Accelerate.h>
#endif
/*
 * RSTT libraries include file
 */
#ifndef SLBM_C_SHELL_H
#include "slbm_C_shell.h"
#define SLBM_C_SHELL_H
#endif

/*
 * String lengths
 */
#define ILOC_FILENAMELEN 1024                        /* max filename length */
#define ILOC_PHALEN 9                              /* max phase code length */
#define ILOC_VALLEN 255                         /* max variable name length */
#define ILOC_PARLEN 64                         /* max parameter name length */
/*
 * tolerance values
 */
#define ILOC_NULLVAL 9999999                                  /* null value */
#define ILOC_DEPSILON 1.e-8          /* for testing floating point equality */
#define ILOC_CVGTOL 1.e-8                          /* convergence tolerance */
#define ILOC_ZEROTOL 1.e-10                               /* zero tolerance */
/*
 *
 * Array sizes for SplineCoeffs interpolation routines
 *
 */
#define ILOC_DELTASAMPLES 6  /* max number of TT samples in delta direction */
#define ILOC_DEPTHSAMPLES 4  /* max number of TT samples in depth direction */
#define ILOC_MINSAMPLES 2        /* min number of samples for interpolation */
/*
 *
 * Array sizes for neighbourhood algorithm routines
 *
 */
#define ILOC_NA_MAXND       4             /* max number of model parameters */
#define ILOC_NA_MAXBIT     30  /* max direction numbers for Sobol sequences */
#define ILOC_NA_MAXDEG     10             /* max degree for SAS polynomials */
/*
 * degree <-> rad conversions
 */
#define ILOC_PI    M_PI                                      /* value of pi */
#define ILOC_PI2   M_PI_2                                  /* value of pi/2 */
#define ILOC_TWOPI 2 * ILOC_PI                              /* value of 2pi */
#define ILOC_EARTHRADIUS 6371.                            /* Earth's radius */
#define ILOC_RAD2DEG (180./ILOC_PI)            /* radian - degree transform */
#define ILOC_DEG2RAD (ILOC_PI/180.)            /* degree - radian transform */
#define ILOC_DEG2KM (ILOC_DEG2RAD * ILOC_EARTHRADIUS)      /* degrees to km */
#define ILOC_FLATTENING  0.00335281066474 /* WGS84 ellipsoid: f = 1/298.257 */
#define ILOC_MAX_RSTT_DIST 15                   /* max delta for RSTT Pn/Sn */

#define TRUE 1                                             /* logical true  */
#define FALSE 0                                            /* logical false */

#ifndef ILOC_FUNCS
#define ILOC_FUNCS
#define ILOC_MAX(A,B)   ((A)>(B) ? (A):(B))   /* returns the max of A and B */
#define ILOC_MIN(A,B)   ((A)<(B) ? (A):(B))   /* returns the min of A and B */
#define ILOC_SQRT(A)    ((A)>0 ? sqrt((A)):0.)          /* safe square root */
#define ILOC_STREQ(A,B) (strcmp ((A),(B)) == 0)        /* same two strings? */
#define ILOC_SWAP(A,B)  { temp=(A);(A)=(B);(B)=temp; }       /* swap values */
#define ILOC_SWAPI(A,B) { itemp=(A);(A)=(B);(B)=itemp; } /* swap int values */
#define ILOC_SIGN(A,B)  ((B) < 0 ? ((A) < 0 ? (A) : -(A)) : ((A) < 0 ? -(A) : (A)))
#endif

/*
 *
 * error and warning message codes (return values)
 *
 */
#ifndef ILOC_ERRORS
#define ILOC_ERRORS
#define ILOC_FAILURE -1
#define ILOC_SUCCESS 0
#define ILOC_CANNOT_OPEN_FILE 1
#define ILOC_MEMORY_ALLOCATION_ERROR 2
#define ILOC_STRING_TOO_LONG 3
#define ILOC_INCOMPLETE_INPUT_DATA 4
#define ILOC_INSUFFICIENT_NUMBER_OF_PHASES 5
#define ILOC_INSUFFICIENT_NUMBER_OF_INDEPENDENT_PHASES 6
#define ILOC_PHASE_LOSS 7
#define ILOC_SLOW_CONVERGENCE 8
#define ILOC_SINGULAR_MATRIX 9
#define ILOC_ABNORMALLY_ILL_CONDITIONED_PROBLEM 10
#define ILOC_DIVERGING_SOLUTION 11
#define ILOC_OUT_OF_RANGE 12
#define ILOC_INVALID_DEPTH 13
#define ILOC_INVALID_DELTA 14
#define ILOC_INVALID_PHASE 15
#define ILOC_RSTT_ERROR 16
#define ILOC_NO_TRAVELTIME 17
#define ILOC_UNKNOWN_ERROR 666
#endif

/*
 *
 * Input structures
 *    sciLoc configuration structure (CONF)
 *    Hypocenter (HYPOCENTER)
 *    Associated arrivals (ASSOCS)
 *    Station info (STA)
 * Structures read from auxiliary data files
 *    TT-independent phase info for sciLoc phase identification
 *    Global velocity model parameters and list of phases with travel time table
 *    Global travel-time tables
 *    Ellipticity correction coefficients
 *    Local velocity model parameters and list of phases with travel time table
 *    Local travel-time tables
 *    RSTT global 3D upper mantle model
 *    Generic variogram model
 *    Flinn-Engdahl geographic region grid
 *    Default depth info
 * Output structures
 *    Hypocenter
 *    Associated arrivals
 *
 */
/*
 *
 * sciLoc configuration structure
 *
 */
typedef struct iLocConfig {
/*
 *
 *  Config parameters can be set for an event by the user when invoking iLoc
 *
 */
/*
 *  Control parameters
 */
    int Verbose;                                  /* verbosity level [0..3] */
    int DoGridSearch;           /* perform grid search for initial location */
    int DoNotRenamePhases;                      /* do not reidentify phases */
/*
 *  directory of auxiliary data files
 */
    char auxdir[ILOC_FILENAMELEN];   /* pathname for iLoc auxdata directory */
/*
 *  Travel time predictions
 */
    char TTmodel[ILOC_VALLEN];                  /* name of global TT tables */
    char RSTTmodel[ILOC_FILENAMELEN];            /* pathname for RSTT model */
    int UseRSTTPnSn;                          /* use RSTT Pn/Sn predictions */
    int UseRSTTPgLg;                          /* use RSTT Pg/Lg predictions */
    int UseRSTT;                                    /* use RSTT predictions */
    char LocalVmodel[ILOC_VALLEN];         /* name for local velocity model */
    double MaxLocalTTDelta;             /* use local TT up to this distance */
    int UseLocalTT;                             /* use local TT predictions */
/*
 *  Linearized inversion
 */
    int MinIterations;                          /* min number of iterations */
    int MaxIterations;                          /* max number of iterations */
    int MinNdefPhases;               /* min number of defining observations */
    double SigmaThreshold;               /* to exclude phases from solution */
    int DoCorrelatedErrors;                /* account for correlated errors */
    int AllowDamping;                   /* allow damping in LSQR iterations */
/*
 *  depth resolution
 */
    double MaxLocalDistDeg;                     /* max local distance [deg] */
    int MinLocalStations;  /* min number of stations within MaxLocalDistDeg */
    double MaxSPDistDeg;                          /* max S-P distance [deg] */
    int MinSPpairs;                        /* min number of S-P phase pairs */
    int MinCorePhases;         /* min number of core reflections ([PS]c[PS] */
    int MinDepthPhases; /* min number of depth phases for depth-phase depth */
    double MaxShallowDepthError; /* max depth error for crustal free events */
    double MaxDeepDepthError;       /* max depth error for deep free events */
/*
 *  NA search parameters
 */
    double NAsearchRadius;   /* search radius around preferred origin [deg] */
    double NAsearchDepth;      /* search radius around preferred depth [km] */
    double NAsearchOT;             /* search radius around preferred OT [s] */
    double NAlpNorm;                  /* p-value for norm to compute misfit */
    int NAiterMax;                              /* max number of iterations */
    int NAinitialSample;                       /* number of initial samples */
    int NAnextSample;                       /* number of subsequent samples */
    int NAcells;                         /* number of cells to be resampled */
/*
 *  ETOPO parameters
 */
    char EtopoFile[ILOC_VALLEN];                          /* ETOPO filename */
    int EtopoNlon;                  /* number of longitude samples in ETOPO */
    int EtopoNlat;                   /* number of latitude samples in ETOPO */
    double EtopoRes;                                   /* cellsize in ETOPO */
} ILOC_CONF;

/*
 *
 * Hypocenter
 *   On input Time, Lat, Lon, Depth are either taken from Origin or set by user
 *     Any combinations of the hypocenter parameters can be fixed by the user
 *     NumPhase is the number of AssociatedPhase records passed to iLoc
 *     NumSta is the number of StationInfo records passed to iLoc
 *   On output the structure contains the solution
 *   Input:
 *     isManMade        - anthropogenic event [0/1]
 *     numSta           - number of associated stations
 *     numPhase         - number of associated arrivals (N)
 *     Time             - initial origin epoch time [decimal seconds]
 *     Lat              - initial latitude [deg]
 *     Lon              - initial longitude [deg]
 *     Depth            - initial depth [km]
 *     FixOT            - fix origin time to initial origin time [0/1]
 *     FixLat           - fix latitude to initial latitude [0/1]
 *     FixLon           - fix longitude to initial longitude [0/1]
 *     FixDepth         - fix depth to initial depth [0/1]
 *     FixHypo          - fixed hypocenter [0/1]
 *   Output:
 *     Converged        - convergent solution flag
 *     numUnknowns      - number of model parameters solved for (M)
 *     Time             - origin epoch time [decimal seconds]
 *     Lat              - latitude [deg]
 *     Lon              - longitude [deg]
 *     Depth            - depth [km]
 *     FixedDepthType   - fixed depth type [0..8]
 *       0 - free depth solution
 *       1 - airquake/deepquake, depth fixed to surface/MaxHypocenterDepth
 *       2 - depth fixed to depth reported by an agency (not used here)
 *       3 - depth fixed to depth-phase depth
 *       4 - anthropogenic event, depth fixed to surface
 *       5 - depth fixed to default depth grid depth
 *       6 - no default depth grid point exists, fixed to median reported depth
 *       7 - no default depth grid point exists, fixed to GRN-dependent depth
 *       8 - depth fixed by user provided value
 *     DepthDp          - hypocenter depth from depth phase stack [km]
 *     DepthDpError     - hypocenter depth error from depth phases [km]
 *     numDepthDp       - number of depth phases used in depth phase stack
 *     uRMS             - unweighted RMS residual [s]
 *     wRMS             - weighted RMS residual [s]
 *     ModelCov[4][4]   - model covariance matrix
 *     Errors[4]        - 1D (OT, lat, lon, depth) uncertainties
 *     semiMajax        - 90% coverage error ellipse semi-major axis [km]
 *     semiMinax        - 90% coverage error ellipse semi-minor axis [km]
 *     Strike           - 90% coverage error ellipse strike
 *     SdevObs          - uRMS * sqrt(N / (N - M))
 *     numDef           - number of defining observations
 *     numTimedef       - number of time defining phases
 *     numAzimdef       - number of azimuth defining phases
 *     numSlowdef       - number of slowness defining phases
 *     numRank          - number of independent observations
 *     numReading       - number of readings (picks from one station)
 *     numdDefsta       - number of defining stations
 *     minDist          - distance to closest station [deg]
 *     maxDist          - distance to furthest station [deg]
 *     Gap              - azimuthal gap (entire network)
 *     Sgap             - secondary azimuthal gap (entire network)
 *     FixHypo          - fixed hypocenter [0/1]
 *     GT5candidate     - 1 if GT5 candidate, 0 otherwise
 *     localSgap        - local secondary azimuthal gap [deg]
 *     localDU          - local network quality metric
 *     numStaWithin10km - number of defining stations within 10 km
 *     localNumDefsta   - number of local defining stations
 *     localNumDef      - number of local defining observations
 *
 */
typedef struct Hypocenter {
    int isManMade;                                  /* anthropogenic event? */
    int numSta;                            /* number of associated stations */
    int numPhase;                  /* total number of associated phases (N) */
    double Time;  /* initial origin epoch time on input, final OT on output */
    double Lat;      /* initial latitude on input, final latitude on output */
    double Lon;    /* initial longitude on input, final longitude on output */
    double Depth;          /* initial depth on input, final depth on output */
    int FixOT;                               /* fix to initial origin time? */
    int FixLat;                                 /* fix to initial latitude? */
    int FixLon;                                /* fix to initial longitude? */
    int FixDepth;                                  /* fix to initial depth? */
    int Converged;                              /* convergent solution flag */
    int numUnknowns;           /* number of model parameters solved for (M) */
    int FixedDepthType;                         /* fixed depth type [0..10] */
    double DepthDp;              /* hypocenter depth from depth phases [km] */
    double DepthDpError;   /* hypocenter depth error from depth phases [km] */
    int numDepthDp;     /* number of depth phases used in depth phase stack */
    double uRMS;                             /* unweighted RMS residual [s] */
    double wRMS;                               /* weighted RMS residual [s] */
    double ModelCov[4][4];                       /* model covariance matrix */
    double Errors[4];    /* 1D uncertainties scaled to 90% confidence level */
    double semiMajax;             /* 90% error ellipse semi-major axis [km] */
    double semiMinax;             /* 90% error ellipse semi-minor axis [km] */
    double Strike;                              /* 90% error ellipse strike */
    double SdevObs;                             /* uRMS * sqrt(N / (N - M)) */
    int numDef;                          /* number of defining observations */
    int numTimedef;                       /* number of time defining phases */
    int numAzimdef;                    /* number of azimuth defining phases */
    int numSlowdef;                   /* number of slowness defining phases */
    int numRank;                      /* number of independent observations */
    int numReading;          /* number of readings (picks from one station) */
    int numDefsta;                           /* number of defining stations */
    double minDist;         /* distance to closest station (entire network) */
    double maxDist;        /* distance to furthest station (entire network) */
    double Gap;                           /* azimuthal gap (entire network) */
    double Sgap;                /* secondary azimuthal gap (entire network) */
    int FixHypo;                                        /* fixed hypocenter */
    int GT5candidate;                    /* 1 if GT5 candidate, 0 otherwise */
    double localSgap;                /* local secondary azimuthal gap [deg] */
    double localDU;                         /* local network quality metric */
    int numStaWithin10km;       /* number of defining stations within 10 km */
    int localNumDefsta;                /* number of local defining stations */
    int localNumDef;               /* number of local defining observations */
} ILOC_HYPO;

/*
 *
 * Associated arrivals
 *   An array of Hypocenter->numPhase elements
 *   On input StaInd, PhaseHint, ArrivalTime, BackAzimuth, HorizontalSlowness,
 *     Timedef, Azimdef, Slowdef and phaseFixed values are expected
 *     ArrivalTime, BackAzimuth and HorizontalSlowness could be ILOC_NULLVAL
 *   On output the structure contains the arrival info with respect to the
 *     solution
 *   Input:
 *     arid              - Arrival _oid
 *     StaInd            - index of station in ILOC_STA structure (integer)
 *     PhaseHint         - phase name hint
 *     phaseFixed        - 1 to stop reidentifying a phase
 *     ArrivalTime       - observed arrival epoch time [s]
 *     Timedef           - time used in the location [0/1]
 *     BackAzimuth       - observed station-to-event azimuth [deg]
 *     Azimdef           - back azimuth used in the location [0/1]
 *     Slowness          - observed horizontal slowness [s/deg]
 *     Slowdef           - slowness used in the location [0/1]
 *   Output:
 *     Phase             - phase name
 *     Delta             - delta [deg]
 *     Esaz              - event-to-station azimuth [deg]
 *     Seaz              - tation-to-event azimuth [deg]
 *     Deltim            - a priori time measurement error [s]
 *     TimeRes           - time residual [s]
 *     Delaz             - a priori azimuth measurement error [deg]
 *     AzimRes           - back azimuth residual [deg]
 *     Delslo            - a priori slowness measurement error [s/deg]
 *     SlowRes           - slowness residual [s/deg]
 *     Vmodel            - velocity model
 *   Internal use:
 *     rdid              - reading id
 *     initialPhase      - initial phase in a reading [0/1]
 *     prevPhase         - phase from previous iteration
 *     prevTimeDef       - TimeDef from previous iteration
 *     prevAzimDef       - AzimDef from previous iteration
 *     prevSlowDef       - SlowDef from previous iteration
 *     CovIndTime        - position in covariance matrix time section
 *     CovIndAzim        - position in covariance matrix azimuth section
 *     CovIndSlow        - position in covariance matrix slowness section
 *     ttime             - travel time prediction with corrections [s]
 *     dtdd              - horizontal slowness prediction [s/deg]
 *     dtdh              - vertical slowness prediction [s/km]
 *     d2tdd             - second time derivatives
 *     d2tdh             - second time derivatives
 *     bpdel             - depth phase bounce point distance [deg]
 *     firstP            - 1 if first arriving defining P, 0 otherwise
 *     firstS            - 1 if first arriving S, 0 otherwise
 *     hasDepthPhase     - 1 if firstP and reading has depth phase(s)
 *     pPindex           - index pointer to pP in this reading
 *     pwPindex          - index pointer to pwP in this reading
 *     pSindex           - index pointer to pS in this reading
 *     sPindex           - index pointer to sP in this reading
 *     sSindex           - index pointer to sS in this reading
 *     duplicate         - 1 if duplicate, 0 otherwise
 *
 */
typedef struct AssociatedPhase {
    int arid;                                                  /* arrival id */
    int StaInd;                    /* index of station in ILOC_STA structure */
    char PhaseHint[ILOC_PHALEN];                               /* phase hint */
    char Phase[ILOC_PHALEN];                                        /* phase */
    double Delta;                                             /* delta [deg] */
    double Esaz;                           /* event-to-station azimuth [deg] */
    double Seaz;                           /* station-to-event azimuth [deg] */
    double ArrivalTime;                            /* arrival epoch time [s] */
    double Deltim;           /* a priori time measurement error estimate [s] */
    double TimeRes;                                     /* time residual [s] */
    int Timedef;                   /* 1 if used in the location, 0 otherwise */
    double BackAzimuth;           /* measured station-to-event azimuth [deg] */
    double Delaz;       /* a priori azimuth measurement error estimate [deg] */
    double AzimRes;                                /* azimuth residual [deg] */
    int Azimdef;                   /* 1 if used in the location, 0 otherwise */
    double Slowness;                 /* measured horizontal slowness [s/deg] */
    double Delslo;   /* a priori slowness measurement error estimate [s/deg] */
    double SlowRes;                             /* slowness residual [s/deg] */
    int Slowdef;                   /* 1 if used in the location, 0 otherwise */
    char Vmodel[ILOC_VALLEN];                         /* velocity model name */
    int rdid;                                                  /* reading id */
    int initialPhase;                    /* initial phase in a reading [0/1] */
    char prevPhase[ILOC_PHALEN];            /* phase from previous iteration */
    int phaseFixed;                       /* 1 to stop reidentifying a phase */
    int prevTimedef;                      /* TimeDef from previous iteration */
    int prevAzimdef;                      /* AzimDef from previous iteration */
    int prevSlowdef;                      /* SlowDef from previous iteration */
    int CovIndTime;            /* position in covariance matrix time section */
    int CovIndAzim;         /* position in covariance matrix azimuth section */
    int CovIndSlow;        /* position in covariance matrix slowness section */
    double ttime;             /* travel time prediction with corrections [s] */
    double dtdd;                   /* horizontal slowness prediction [s/deg] */
    double dtdh;                      /* vertical slowness prediction [s/km] */
    double d2tdd;                                 /* second time derivatives */
    double d2tdh;                                 /* second time derivatives */
    double bpdel;                 /* depth phase bounce point distance [deg] */
    int firstP;               /* 1 if first arriving defining P, 0 otherwise */
    int firstS;                        /* 1 if first arriving S, 0 otherwise */
    int hasDepthPhase;         /* 1 if firstP and reading has depth phase(s) */
    int pPindex;                      /* index pointer to pP in this reading */
    int pwPindex;                    /* index pointer to pwP in this reading */
    int pSindex;                      /* index pointer to pS in this reading */
    int sPindex;                      /* index pointer to sP in this reading */
    int sSindex;                      /* index pointer to sS in this reading */
    int duplicate;                            /* 1 if duplicate, 0 otherwise */
} ILOC_ASSOC;

/*
 *
 * Station info (input)
 *   An array of Hypocenter->numSta elements
 *   Input:
 *     StaLat       - station latitude [deg]
 *     StaLon       - station longitude [deg]
 *     StaElevation - station elevation [m]
 *
 */
typedef struct StationInfo {
    double StaLat;                                 /* station latitude [deg] */
    double StaLon;                                /* station longitude [deg] */
    double StaElevation;                            /* station elevation [m] */
} ILOC_STA;



/*
 *
 * The structures below are used by iLoc internally
 *
 */

/*
 * Phase specific structures for phase identification
 *
 */
/*
 *
 * Reading structure (phase indices belonging to a reading)
 *   An array of Hypocenter->numReading elements
 *   A reading is a set of picks for an event from the same station
 *     start - start phase index in the reading
 *     npha  - number of phases in the reading
 *
 */
typedef struct rdidx {
    int start;                          /* start phase index in the reading */
    int npha;                            /* number of phases in the reading */
} ILOC_READING;
/*
 * Map reported phase codes to IASPEI phase codes
 */
typedef struct IASPEIPhaseMap {
    char ReportedPhase[ILOC_PHALEN];                      /* reported phase */
    char Phase[ILOC_PHALEN];                                /* IASPEI phase */
} ILOC_PHASEMAP;
/*
 * A priori measurement error estimates
 *   Phase and distance dependent arrival time, slowness and backazimuth
 *   measurement errors.
 */
typedef struct Phaseweights {
    char Phase[ILOC_PHALEN];                                  /* phase name */
    double delta1;                                    /* min distance [deg] */
    double delta2;                                    /* max distance [deg] */
    double deltim;          /* a priori time measurement error estimate [s] */
    double delaz;      /* a priori azimuth measurement error estimate [deg] */
    double delslo;  /* a priori slowness measurement error estimate [s/deg] */
} ILOC_PHASEWEIGHT;
/*
 * Structure for various lists of phases
 */
typedef struct PhaseLists {
    char Phase[ILOC_PHALEN];
} ILOC_PHASELIST;
/*
 * Nearest-neighbour station order
 */
typedef struct StaNNOrder {
    int index;
    int x;
} ILOC_STAORDER;

/*
 *
 * TT-independent phase info for phase identification (input)
 *   See also the Manual for the description of the iLoc phase identification
 *   procedures.
 *
 */
typedef struct PhaseIdentification {
    int numPhaseMap;                        /* number of phases in PhaseMap */
    int numPhaseWeight;                  /* number of phases in PhaseWeight */
    int numPhaseWithoutResidual;            /* number of no-residual phases */
    int numAllowablePhases;                   /* number of allowable phases */
    int numFirstPphase;                /* number of first-arriving P phases */
    int numFirstSphase;                /* number of first-arriving S phases */
    int numFirstPoptional;             /* number of optional first P phases */
    int numFirstSoptional;             /* number of optional first S phases */
    ILOC_PHASEMAP *PhaseMap;        /* mapping phases to IASPEI phase codes */
    ILOC_PHASEWEIGHT *PhaseWeight;           /* a priori measurement errors */
    ILOC_PHASELIST *PhaseWithoutResidual;    /* phases with no travel times */
    ILOC_PHASELIST *AllowablePhases; /* allowable phases for identification */
    ILOC_PHASELIST *firstPphase;                 /* first arriving P phases */
    ILOC_PHASELIST *firstSphase;                 /* first arriving S phases */
    ILOC_PHASELIST *firstPoptional;   /* acceptable first arriving P phases */
    ILOC_PHASELIST *firstSoptional;   /* acceptable first arriving P phases */
} ILOC_PHASEIDINFO;

/*
 *
 * Travel-time table info
 *
 */
/*
 *
 * Velocity model parameters and list of phases with travel times (input)
 *
 */
typedef struct TravelTimeInfo {
    char TTmodel[ILOC_VALLEN];                  /* name of global TT tables */
    int numPhaseTT;                             /* number of phases with TT */
    int numECPhases;        /* number of phases with ellipticity correction */
    double Moho;                                           /* depth of Moho */
    double Conrad;                                       /* depth of Conrad */
    double MaxHypocenterDepth;                      /* max hypocenter depth */
    double PSurfVel;               /* Pg velocity for elevation corrections */
    double SSurfVel;               /* Sg velocity for elevation corrections */
    ILOC_PHASELIST *PhaseTT;      /* list of phases with travel-time tables */
} ILOC_TTINFO;
/*
 *
 * Travel time tables (input)
 *   Either read from files (global TT)
 *   or generated from a local velocity model (local TT)
 *
 */
typedef struct TTtables {
    char Phase[ILOC_PHALEN];                                       /* phase */
    int isbounce;                         /* surface reflection or multiple */
	int	ndel;                                 /* number of distance samples */
	int	ndep;                                    /* number of depth samples */
    double *depths;                                   /* depth samples [km] */
    double *deltas;                               /* distance samples [deg] */
    double **tt;                                   /* travel-time table [s] */
    double **bpdel;        /* depth phase bounce point distance table [deg] */
    double **dtdd;                     /* horizontal slowness table [s/deg] */
    double **dtdh;                        /* vertical slowness table [s/km] */
} ILOC_TT_TABLE;
/*
 *
 * Ellipticity correction coefficients structure (ak135 format) (input)
 *     Note: the tau corrections are stored at 5 degree intervals in distance
 *           and at the depths 0, 100, 200, 300, 500, 700 km.
 *
 */
typedef struct ec_coef {
    char Phase[ILOC_PHALEN];                                       /* phase */
    int numDistanceSamples;                   /* number of distance samples */
    int numDepthSamples;                         /* number of depth samples */
    double mindist;                               /* minimum distance [deg] */
    double maxdist;                               /* maximum distance [deg] */
    double depth[6];                                  /* depth samples [km] */
    double *delta;                                /* distance samples [deg] */
    double **t0;                                         /* t0 coefficients */
    double **t1;                                         /* t1 coefficients */
    double **t2;                                         /* t2 coefficients */
} ILOC_EC_COEF;

/*
 *
 * Flinn-Engdahl 1995 geographic region numbers (input)
 *
 */
typedef struct FlinnEngdahl {
	int	nlat;                                 /* number of latitude samples */
	int	*nl;                /* number of longitude samples at each latitude */
    int **lon;                         /* longitude ranges at each latitude */
    int **grn;                  /* grn in longitude ranges at each latitude */
} ILOC_FE;
/*
 *
 * Default depth info (input)
 *   default depth grid array
 *   default depth by geographic region number
 *   ETOPO file for bounce corrections
 *
 */
typedef struct DepthInfo {
    int numGrid;              /* number of grid point in default depth grid */
    int numGRN;                          /* number of geographic FE regions */
    double GridRes;                   /* grid spacing in default depth grid */
    double **DepthGrid;                               /* default depth grid */
    double *GrnDepth;                              /* default depths by grn */
    short int **Topo;                         /* ETOPO bathymetry/elevation */
} ILOC_DEFAULTDEPTH;

/*
 *
 * Variogram (input)
 *   Generic variogram model to calculate the a priori data covariance matrix
 *
 */
typedef struct variogram {
    int n;                                             /* number of samples */
    double sill;                        /* sill (background variance) [s^2] */
    double maxsep;          /* max station separation to be considered [km] */
    double *x;                                  /* station separations [km] */
    double *y;                                    /* variogram values [s^2] */
    double *d2y;     /* second derivatives for natural spline interpolation */
} ILOC_VARIOGRAM;

/*
 *
 * Local velocity model
 *
 */
typedef struct LocalVModel {
    int n;                                              /* number of layers */
    double *h;                                 /* depth of top of the layer */
    double *vp;                                               /* P velocity */
    double *vs;                                               /* S velocity */
    int iconr;                             /* index of Conrad discontinuity */
    int imoho;                               /* index of Moho discontinuity */
    double *z;                 /* Earth flattened depth of top of the layer */
    double *thk;                         /* Earth flattened layer thickness */
    double *p;                                /* Earth flattened P velocity */
    double *p2;                        /* Earth flattened P velocity square */
    double *s;                                /* Earth flattened S velocity */
    double *s2;                        /* Earth flattened S velocity square */
} ILOC_VMODEL;

/*
 *
 * structure for defining phases
 *
 */
typedef struct definingPhaselst {
    char Phase[ILOC_PHALEN];                                   /* phase name */
    int nTime;                                          /* number of samples */
    int nAzim;                                          /* number of samples */
    int nSlow;                                          /* number of samples */
    int *indTime;/* permutation vector to block-diagonalize data covariances */
    int *indAzim;/* permutation vector to block-diagonalize data covariances */
    int *indSlow;/* permutation vector to block-diagonalize data covariances */
} ILOC_PHADEF;

/*
 *
 * NA search space
 *
 */
typedef struct NASearchspace {
    int nd;                                   /* number of model parameters */
    int otfix, epifix, depfix;                     /* fixed parameter flags */
    double ot, lat, lon, depth;                   /* center point of search */
    double lpnorm;         /* p = [1,2] for L1, L2 norm or anything between */
    double range[ILOC_NA_MAXND][2];                  /* search space limits */
    double ranget[ILOC_NA_MAXND][2];      /* normalized search space limits */
    double scale[ILOC_NA_MAXND+1];                               /* scaling */
} ILOC_NASPACE;
/*
 *
 * Sobol-Antonov-Saleev coefficients for quasi-random sequences
 *
 */
typedef struct sobol_coeff {
    int n;                           /* max number of independent sequences */
    int *mdeg;                                                    /* degree */
    unsigned long *pol;                                       /* polynomial */
    unsigned long **iv;                              /* initializing values */
} ILOC_SOBOL;
/*
 *
 * node stucture from single-linkage clustering
 *
 */
typedef struct node {
    int left;
    int right;
    double linkdist;
} ILOC_NODE;



/*
 *
 * Public function declarations
 *   sciLoc app calls
 *      iLoc_ReadAuxDataFiles
 *      iLoc_Locator
 *      iLoc_FreeAuxData
 */

/*
 * sciLocReadAuxDataFiles.c
 */
int iLoc_ReadAuxDataFiles(ILOC_CONF *iLocConfig, ILOC_PHASEIDINFO *PhaseIdInfo,
        ILOC_FE *fe, ILOC_DEFAULTDEPTH *DefaultDepth, ILOC_VARIOGRAM *Variogram,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables[], ILOC_EC_COEF *ec[],
        ILOC_TTINFO *LocalTTInfo, ILOC_TT_TABLE *LocalTTtables[]);
double **iLoc_AllocateFloatMatrix(int nrow, int ncol);
short int **iLoc_AllocateShortMatrix(int nrow, int ncol);
unsigned long **iLoc_AllocateLongMatrix(int nrow, int ncol);
/*
 * sciLocFreeMemory.c
 */
int iLoc_FreeAuxData(ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_FE *fe,
        ILOC_DEFAULTDEPTH *DefaultDepth, ILOC_VARIOGRAM *Variogram,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_EC_COEF *ec,
        ILOC_TTINFO *LocalTTInfo, ILOC_TT_TABLE *LocalTTtables, int UseRSTT);
void iLoc_FreePhaseIdInfo(ILOC_PHASEIDINFO *PhaseIdInfo);
void iLoc_FreeDefaultDepth(ILOC_DEFAULTDEPTH *DefaultDepth);
void iLoc_FreeFlinnEngdahl(ILOC_FE *fep);
void iLoc_FreeVariogram(ILOC_VARIOGRAM *Variogram);
void iLoc_FreeEllipticityCorrections(int numECPhases, ILOC_EC_COEF *ec);
void iLoc_FreeTTtables(int numPhaseTT, ILOC_TT_TABLE *tt_tables);
void iLoc_Free(void *ptr);
void iLoc_FreeFloatMatrix(double **matrix);
void iLoc_FreeShortMatrix(short int **matrix);
void iLoc_FreeLongMatrix(unsigned long **matrix);
/*
 * sciLocLocator.c
 */
int iLoc_Locator(ILOC_CONF *iLocConfig, ILOC_PHASEIDINFO *PhaseIdInfo,
        ILOC_FE *fe, ILOC_DEFAULTDEPTH *DefaultDepth, ILOC_VARIOGRAM *variogram,
        ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables,
        ILOC_TTINFO *LocalTTInfo, ILOC_TT_TABLE *LocalTTtables,
        ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs, ILOC_STA *StaLocs);
void iLoc_Readings(int numPhase, int numReading, ILOC_ASSOC *Assocs,
        ILOC_READING *rdindx);

/*
 * sciLocCluster.c
 */
int iLoc_HierarchicalCluster(int nsta, double **distmatrix,
        ILOC_STAORDER *staorder);
/*
 * sciLocDataCovariance.c
 */
double **iLoc_GetDistanceMatrix(int numSta, ILOC_STA *StaLocs);
double **iLoc_GetDataCovarianceMatrix(int nsta, int numPhase, int nd,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, double **distmatrix,
        ILOC_VARIOGRAM *variogram, int verbose);
/*
 * sciLocDepthPhases.c
 */
int iLoc_DepthResolution(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_READING *rdindx);
int iLoc_DepthPhaseCheck(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_READING *rdindx);
int iLoc_DepthPhaseStack(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables,
        short int **topo);
/*
 * sciLocDistAzimuth.c
 */
void iLoc_GetDeltaAzimuth(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs,
        ILOC_STA *StaLocs);
double iLoc_DistAzimuth(double slat, double slon, double elat, double elon,
        double *azi, double *baz);
void iLoc_PointAtDeltaAzimuth(double lat1, double lon1, double delta,
        double azim, double *lat2, double *lon2);
/*
 * sciLocGregion.c
 */
int iLoc_GregionNumber(double lat, double lon, ILOC_FE *fep);
int iLoc_GregToSreg(int grn);
int iLoc_Gregion(int number, char *gregname);
int iLoc_Sregion(int number, char *sregname);
double iLoc_GetDefaultDepth(ILOC_HYPO *Hypocenter, ILOC_DEFAULTDEPTH *DefaultDepth,
        ILOC_FE *fep, int *isdefdep, int verbose);
/*
 * sciLocInitializations.c
 */
int iLoc_InitializeEvent(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs);
/*
 * sciLocInterpolate.c
 */
void iLoc_SplineCoeffs(int n, double *x, double *y, double *d2y, double *tmp);
double iLoc_SplineInterpolation(double xp, int n, double *x, double *y,
        double *d2y, int isderiv, double *dydx, double *d2ydx);
void iLoc_FloatBracket(double xp, int n, double *x, int *jlo, int *jhi);
void iLoc_IntegerBracket(int xp, int n, int *x, int *jlo, int *jhi);
double iLoc_BilinearInterpolation(double xp1, double xp2, int nx1, int nx2,
        double *x1, double *x2, double **y);
/*
 * sciLocLocalTT.c
 */
ILOC_TT_TABLE *iLoc_GenerateLocalTTtables(char *auxdir, ILOC_TTINFO *LocalTTInfo,
        int verbose);
/*
 * sciLocLocationQuality.c
 */
int iLoc_LocationQuality(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs);
double iLoc_GetdUGapSgap(int nsta, double *esaz, double *gap, double *sgap);
/*
 * sciLocNA.c
 */
int iLoc_SetNASearchSpace(ILOC_CONF *iLocConfig, ILOC_HYPO *grds,
        ILOC_NASPACE *nasp, double maxDepth);
int iLoc_NASearch(ILOC_CONF *iLocConfig, ILOC_HYPO *grds, ILOC_ASSOC *Assocs,
        ILOC_STA *StaLocs, ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, double **distmatrix,
        ILOC_VARIOGRAM *variogram, ILOC_STAORDER *staorder, ILOC_PHADEF *PhaDef,
        ILOC_NASPACE *nasp, int is2nderiv);
/*
 * sciLocPhaseIdentification.c
 */
void iLoc_IdentifyPhases(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int *is2nderiv);
int iLoc_ReIdentifyPhases(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_READING *rdindx,
        ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_EC_COEF *ec, ILOC_TTINFO *TTInfo,
        ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int is2nderiv);
void iLoc_GetNumDef(ILOC_HYPO *Hypocenter, ILOC_ASSOC *Assocs);
void iLoc_SortAssocs(int numPhase, ILOC_ASSOC *Assocs);
void iLoc_SortAssocsNN(int numPhase, int numSta, ILOC_ASSOC *Assocs,
        ILOC_STA *StaLocs, ILOC_STAORDER *staorder);
/*
 * sciLocPrintEvent.c
 */
void iLoc_PrintSolution(ILOC_HYPO *Hypocenter, int grn);
void iLoc_PrintHypocenter(ILOC_HYPO *Hypocenter);
void iLoc_PrintPhases(int numPhase, ILOC_ASSOC *Assocs);
void iLoc_PrintDefiningPhases(int numPhase, ILOC_ASSOC *Assocs);
void iLoc_EpochToHuman(char *htime, double etime);
void iLoc_PrintIOstructures(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, int isinput);
/*
 * sciLocSVD.c
 */
int iLoc_SVDdecompose(int n, int m, double **u, double sv[], double **v);
int iLoc_SVDsolve(int n, int m, double **u, double sv[], double **v, double *b,
        double *x, double thres);
void iLoc_SVDModelCovarianceMatrix(int m, double thres, double sv[], double **v,
        double mcov[][4]);
double iLoc_SVDthreshold(int n, int m, double sv[]);
int iLoc_SVDrank(int n, int m, double sv[], double thres);
double iLoc_SVDnorm(int m, double sv[], double thres, double *cond);
int iLoc_ProjectionMatrix(int numPhaDef, ILOC_PHADEF *PhaDef, int numPhase,
        ILOC_ASSOC *Assocs, int nd, double pctvar, double **cov, double **w,
        int *nrank, int nunp, ILOC_PHASELIST *phundef, int ispchange);
/*
 * sciLocTravelTimes.c
 */
int iLoc_GetPhaseIndex(char *phase, ILOC_TTINFO *TTInfo);
int iLoc_TravelTimeResiduals(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assocs, ILOC_STA *StaLocs, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo,
        ILOC_PHASEIDINFO *PhaseIdInfo, int isall, int iszderiv, int is2nderiv);
int iLoc_GetTravelTimePrediction(ILOC_CONF *iLocConfig, ILOC_HYPO *Hypocenter,
        ILOC_ASSOC *Assoc, ILOC_STA *StaLoc, ILOC_EC_COEF *ec,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_TTINFO *LocalTTInfo,
        ILOC_TT_TABLE *LocalTTtables, short int **topo, int iszderiv,
        int isfirst, int is2nderiv);
double iLoc_GetEtopoCorrection(ILOC_CONF *iLocConfig, int ips, double rayp,
        double bplat, double bplon, short int **topo,
        double Psurfvel, double Ssurfvel, double *tcorw);

#endif
