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
 *     iLoc_ReadAuxDataFiles
 *     iLoc_AllocateFloatMatrix
 *     iLoc_AllocateShortMatrix
 */

/*
 * Private functions
 *     ReadPhaseTTConfigFile
 *     ReadIASPEIPhaseMapFile
 *     GetNumPhases
 *     GetPhaseCodes
 *     ReadDefaultDepthGregion
 *     ReadDefaultDepthGrid
 *     ReadEtopo1
 *     ReadVariogram
 *     ReadEllipticityCorrections
 *     ReadTTtables
 *     ReadRSTTModel
 */
static int ReadPhaseTTConfigFile(char *filename, ILOC_TTINFO *TTInfo);
static int ReadIASPEIPhaseMapFile(char *filename, ILOC_PHASEIDINFO *PhaseIdInfo);
static int GetNumPhases(FILE *fp);
static int GetPhaseCodes(ILOC_PHASELIST *pc, FILE *fp);
static short int **ReadEtopo1(char *filename, int EtopoNlat, int EtopoNlon);
static int ReadVariogram(char *filename, ILOC_VARIOGRAM *variogramp);
static ILOC_EC_COEF *ReadEllipticityCorrections(int *numECPhases, char *filename);
static ILOC_TT_TABLE *ReadTTtables(char *auxdir, ILOC_TTINFO *TTInfo);
static int ReadRSTTModel(char *filename);
static int ReadFlinnEngdahl(char *filename, ILOC_FE *fep);
static double *ReadDefaultDepthGregion(char *filename);
static double **ReadDefaultDepthGrid(char *filename, double *gres, int *ngrid);

/*
 *  Title:
 *     iLoc_ReadAuxDataFiles
 *  Synopsis:
 *     read data files from auxiliary data directory, iLocConfig.auxdir
 *
 *     iLocConfig->auxdir/iLocpars/IASPEIPhaseMap.txt
 *     iLocConfig->auxdir/iLocpars/PhaseConfig.iLocConfig->TTmodel.txt
 *     iLocConfig->auxdir/iLocConfig->TTmodel/iLocConfig->TTmodel.*.tab
 *     iLocConfig->auxdir/iLocConfig->TTmodel/ELCOR.dat
 *     iLocConfig->auxdir/FlinnEngdahl/FE.dat
 *     iLocConfig->auxdir/FlinnEngdahl/DefaultDepth0.5.grid
 *     iLocConfig->auxdir/FlinnEngdahl/GRNDefaultDepth.iLocConfig->TTmodel.dat
 *     iLocConfig->auxdir/topo/etopo5_bed_g_i2.bin
 *     iLocConfig->auxdir/variogram/variogram.dat
 *     iLocConfig->auxdir/iLocpars/PhaseConfig.iLocConfig->LocalVmodel.txt
 *     iLocConfig->auxdir/localmodels/iLocConfig->LocalVmodel.localmodel.dat
 *     iLocConfig->RSTTmodel
 *
 *  Input Arguments:
 *     iLocConfig    - configuration parameter structure
 *  Output Arguments:
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     fe            - pointer to ILOC_FE structure
 *     DefaultDepth  - pointer to ILOC_DEFAULTDEPTH structure
 *     Variogram     - pointer to ILOC_VARIOGRAM structure
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - pointer to array of ILOC_TT_TABLE structures
 *     ec            - pointer to array of ILOC_EC_COEF structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - pointer to array of ILOC_TT_TABLE structures
 *  Return:
 *     Success/error
 *  Called by:
 *     SeisComp3 iLoc app
 *  Calls:
 *     ReadPhaseTTConfigFile
 *     ReadIASPEIPhaseMapFile
 *     ReadFlinnEngdahl
 *     ReadDefaultDepthGregion
 *     ReadDefaultDepthGrid
 *     ReadEtopo1
 *     ReadVariogram
 *     ReadEllipticityCorrections
 *     ReadTTtables
 *     iLoc_GenerateLocalTTtables
 *     ReadRSTTModel
 */
int iLoc_ReadAuxDataFiles(ILOC_CONF *iLocConfig, ILOC_PHASEIDINFO *PhaseIdInfo,
        ILOC_FE *fe, ILOC_DEFAULTDEPTH *DefaultDepth, ILOC_VARIOGRAM *Variogram,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables[], ILOC_EC_COEF *ec[],
        ILOC_TTINFO *LocalTTInfo, ILOC_TT_TABLE *LocalTTtables[])
{
    char filename[ILOC_FILENAMELEN];
    double *GrnDepth = (double *)NULL;
    double **DepthGrid = (double **)NULL;
    short int **Topo = (short int **)NULL;
    double gres;
    int n = 0;
/*
 *
 *  Read global TT specific info
 *      auxdir/iLocpars/PhaseConfig.<iLocConfig.TTmodel>.txt
 *
 */
    sprintf(filename, "%s/iLocpars/PhaseConfig.%s.txt",
            iLocConfig->auxdir, iLocConfig->TTmodel);
    if (ReadPhaseTTConfigFile(filename, TTInfo))
        return ILOC_FAILURE;
/*
 *
 *  Read phase-specific info
 *      auxdir/iLocpars/IASPEIPhaseMap.txt
 *
 */
    sprintf(filename, "%s/iLocpars/IASPEIPhaseMap.txt", iLocConfig->auxdir);
    if (ReadIASPEIPhaseMapFile(filename, PhaseIdInfo)) {
        iLoc_Free(TTInfo->PhaseTT);
        return ILOC_FAILURE;
    }
/*
 *
 *  Read Flinn-Engdahl regions
 *      auxdir/FlinnEngdahl/FE.dat
 *
 */
    sprintf(filename, "%s/FlinnEngdahl/FE.dat", iLocConfig->auxdir);
    if (ReadFlinnEngdahl(filename, fe)) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        return ILOC_FAILURE;
    }
/*
 *
 *  Read default depth files
 *      auxdir/FlinnEngdahl/GRNDefaultDepth.<iLocConfig.TTmodel>.dat
 *      auxdir/FlinnEngdahl/DefaultDepth0.5.grid
 *
 */
    sprintf(filename, "%s/FlinnEngdahl/GRNDefaultDepth.%s.dat",
            iLocConfig->auxdir, iLocConfig->TTmodel);
    if ((GrnDepth = ReadDefaultDepthGregion(filename)) == NULL) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        iLoc_FreeFlinnEngdahl(fe);
        return ILOC_FAILURE;
    }
    sprintf(filename, "%s/FlinnEngdahl/DefaultDepth0.5.grid",
            iLocConfig->auxdir);
    if ((DepthGrid = ReadDefaultDepthGrid(filename, &gres, &n)) == NULL) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        iLoc_FreeFlinnEngdahl(fe); iLoc_Free(GrnDepth);
        return ILOC_FAILURE;
    }
/*
 *
 *  read ETOPO1 file for bounce point corrections
 *      auxdir/topo/etopo5_bed_g_i2.bin
 *
 */
    sprintf(filename, "%s/topo/%s", iLocConfig->auxdir, iLocConfig->EtopoFile);
    if ((Topo = ReadEtopo1(filename, iLocConfig->EtopoNlat,
                           iLocConfig->EtopoNlon)) == NULL) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        iLoc_FreeFlinnEngdahl(fe); iLoc_Free(GrnDepth);
        iLoc_FreeFloatMatrix(DepthGrid);
        return ILOC_FAILURE;
    }
/*
 *
 *  Set DefaultDepth structure
 *
 */
    DefaultDepth->numGrid = n;
    DefaultDepth->GridRes = gres;
    DefaultDepth->numGRN = 757;
    DefaultDepth->DepthGrid = DepthGrid;
    DefaultDepth->GrnDepth = GrnDepth;
    DefaultDepth->Topo = Topo;
/*
 *
 *  Read generic variogram model
 *      auxdir/variogram/variogram.model
 *
 */
    sprintf(filename, "%s/variogram/variogram.model", iLocConfig->auxdir);
    if (ReadVariogram(filename, Variogram)) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        iLoc_FreeFlinnEngdahl(fe);
        iLoc_FreeDefaultDepth(DefaultDepth);
        return ILOC_FAILURE;
    }
/*
 *
 *  Read global travel-time table specific files
 *
 */
    strcpy(TTInfo->TTmodel, iLocConfig->TTmodel);
/*
 *  read ellipticity correction file
 *      auxdir/<iLocConfig.TTmodel>/ELCOR.dat
 */
    sprintf(filename, "%s/%s/ELCOR.dat", iLocConfig->auxdir, TTInfo->TTmodel);
    TTInfo->numECPhases = 0;
    if ((*ec = ReadEllipticityCorrections(&n, filename)) == NULL) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        iLoc_FreeFlinnEngdahl(fe);
        iLoc_FreeDefaultDepth(DefaultDepth);
        iLoc_FreeVariogram(Variogram);
        return ILOC_FAILURE;
    }
    TTInfo->numECPhases = n;
/*
 *  read TT tables
 *      auxdir/<iLocConfig.TTmodel>/<iLocConfig.TTmodel>.*.tab
 */
    if ((*TTtables = ReadTTtables(iLocConfig->auxdir, TTInfo)) == NULL) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        iLoc_FreeFlinnEngdahl(fe);
        iLoc_FreeDefaultDepth(DefaultDepth);
        iLoc_FreeVariogram(Variogram);
        iLoc_FreeEllipticityCorrections(TTInfo->numECPhases, *ec);
        return ILOC_FAILURE;
    }
/*
 *
 *  Read local velocity model specific files
 *
 */
    if (iLocConfig->UseLocalTT) {
/*
 *
 *      Read local velocity specific info
 *          auxdir/iLocpars/PhaseConfig.<iLocConfig.LocalVmodel>.txt
 *
 */
        sprintf(filename, "%s/iLocpars/PhaseConfig.%s.txt",
                iLocConfig->auxdir, iLocConfig->LocalVmodel);
        if (ReadPhaseTTConfigFile(filename, LocalTTInfo)) {
            iLoc_Free(TTInfo->PhaseTT);
            iLoc_FreePhaseIdInfo(PhaseIdInfo);
            iLoc_FreeFlinnEngdahl(fe);
            iLoc_FreeDefaultDepth(DefaultDepth);
            iLoc_FreeVariogram(Variogram);
            iLoc_FreeEllipticityCorrections(TTInfo->numECPhases, *ec);
            iLoc_FreeTTtables(TTInfo->numPhaseTT, *TTtables);
            return ILOC_FAILURE;
        }
        LocalTTInfo->numECPhases = 0;
        strcpy(LocalTTInfo->TTmodel, iLocConfig->LocalVmodel);
/*
 *      read local velocity model and generate local TT tables
 *          auxdir/localmodels/<iLocConfig.LocalVmodel>.localmodel.dat
 */
        if ((*LocalTTtables = iLoc_GenerateLocalTTtables(iLocConfig->auxdir,
                              LocalTTInfo, iLocConfig->Verbose)) == NULL) {
            fprintf(stderr, "Cannot generate %s TT tables!\n", LocalTTInfo->TTmodel);
            iLoc_Free(TTInfo->PhaseTT);
            iLoc_FreePhaseIdInfo(PhaseIdInfo);
            iLoc_FreeFlinnEngdahl(fe);
            iLoc_FreeDefaultDepth(DefaultDepth);
            iLoc_FreeVariogram(Variogram);
            iLoc_FreeEllipticityCorrections(TTInfo->numECPhases, *ec);
            iLoc_FreeTTtables(TTInfo->numPhaseTT, *TTtables);
            iLoc_Free(LocalTTInfo->PhaseTT);
            return ILOC_FAILURE;
        }
    }
    if (iLocConfig->UseRSTT) {
/*
 *
 *      Read and initialize RSTT
 *
 */
        if (ReadRSTTModel(iLocConfig->RSTTmodel)) {
            iLoc_Free(TTInfo->PhaseTT);
            iLoc_FreePhaseIdInfo(PhaseIdInfo);
            iLoc_FreeFlinnEngdahl(fe);
            iLoc_FreeDefaultDepth(DefaultDepth);
            iLoc_FreeVariogram(Variogram);
            iLoc_FreeEllipticityCorrections(TTInfo->numECPhases, *ec);
            iLoc_FreeTTtables(TTInfo->numPhaseTT, *TTtables);
            iLoc_Free(LocalTTInfo->PhaseTT);
            iLoc_FreeTTtables(LocalTTInfo->numPhaseTT, *LocalTTtables);
            return ILOC_FAILURE;
        }
    }
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     ReadPhaseTTConfigFile
 *  Synopsis:
 *     Read velocity model parameters and list of phases with TT tables
 *  Input Arguments:
 *     filename - pathname for the model file
 *  Output Arguments:
 *     TTInfo - ILOC_TTINFO structure
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     GetNumPhases, GetPhaseCodes, iLoc_Free
 */
static int ReadPhaseTTConfigFile(char *filename, ILOC_TTINFO *TTInfo)
{
    FILE *fp;
    int j;
    char par[ILOC_PARLEN], value[ILOC_VALLEN], *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
/*
 *  Open model file or return an error.
 */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ReadPhaseTTConfigFile: cannot open %s\n", filename);
        return ILOC_CANNOT_OPEN_FILE;
    }
/*
 *  Read par = value pairs
 */
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';      /* replace CR with LF */
        }
        if (line[0] == '#' || line[0] == '\n') continue;   /* skip comments */
        if (sscanf(line, "%s = %s", par, value) == 2) {
/*
 *          par = value pairs
 */
            if (ILOC_STREQ(par, "Moho"))
                TTInfo->Moho = atof(value);
            else if (ILOC_STREQ(par, "Conrad"))
                TTInfo->Conrad = atof(value);
            else if (ILOC_STREQ(par, "MaxHypocenterDepth"))
                TTInfo->MaxHypocenterDepth = atof(value);
            else if (ILOC_STREQ(par, "PSurfVel"))
                TTInfo->PSurfVel = atof(value);
            else if (ILOC_STREQ(par, "SSurfVel"))
                TTInfo->SSurfVel = atof(value);
            else continue;
        }
        else if (!strncmp(line, "PhaseTT", 7))
            break;
        else continue;
    }
/*
 *  Count number of phases with travel-time tables and allocate memory
 */
    TTInfo->numPhaseTT = GetNumPhases(fp);
    TTInfo->PhaseTT = (ILOC_PHASELIST *)calloc(TTInfo->numPhaseTT, sizeof(ILOC_PHASELIST));
    if (TTInfo->PhaseTT == NULL) {
        fprintf(stderr, "ReadPhaseTTConfigFile: cannot allocate memory\n");
        iLoc_Free(line);
        fclose(fp);
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  Read list of phases with travel-time tables
 */
    rewind(fp);
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';      /* replace CR with LF */
        }
        if (line[0] == '#' || line[0] == '\n') continue;   /* skip comments */
        if (!strncmp(line, "PhaseTT", 7))
            break;
    }
    if (GetPhaseCodes(TTInfo->PhaseTT, fp)) {
        iLoc_Free(TTInfo->PhaseTT);
        iLoc_Free(line);
        fclose(fp);
        return ILOC_STRING_TOO_LONG;
    }
    fclose(fp);
    iLoc_Free(line);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     ReadIASPEIPhaseMapFile
 *  Synopsis:
 *     Read TT independent phase info
 *  Input Arguments:
 *     filename - pathname for the model file
 *  Output Arguments:
 *     PhaseIdInfo - ILOC_PHASEIDINFO structure
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     GetNumPhases, GetPhaseCodes, iLoc_FreePhaseIdInfo
 */
static int ReadIASPEIPhaseMapFile(char *filename, ILOC_PHASEIDINFO *PhaseIdInfo)
{
    FILE *fp;
    int i = 0, j;
    char phasehint[ILOC_PARLEN], phase[ILOC_PARLEN], *line = NULL;
    double delta1, delta2, deltim, delaz, delslo;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
/*
 *  Open model file or return an error.
 */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ReadIASPEIPhaseMapFile: Cannot open %s\n", filename);
        return ILOC_CANNOT_OPEN_FILE;
    }
/*
 *  Read through the file to count number of elements in the various lists
 */
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';   /* replace CR with LF */
        }
        if (line[0] == '#' || line[0] == '\n') continue;   /* skip comments */
        if (!strncmp(line, "PhaseMap", 8))
            PhaseIdInfo->numPhaseMap = GetNumPhases(fp);
        else if (!strncmp(line, "AllowablePhases", 15))
            PhaseIdInfo->numAllowablePhases = GetNumPhases(fp);
        else if (!strncmp(line, "AllowableFirstP", 15))
            PhaseIdInfo->numFirstPphase = GetNumPhases(fp);
        else if (!strncmp(line, "OptionalFirstP", 14))
            PhaseIdInfo->numFirstPoptional = GetNumPhases(fp);
        else if (!strncmp(line, "AllowableFirstS", 15))
            PhaseIdInfo->numFirstSphase = GetNumPhases(fp);
        else if (!strncmp(line, "OptionalFirstS", 14))
            PhaseIdInfo->numFirstSoptional = GetNumPhases(fp);
        else if (!strncmp(line, "PhaseWithoutResidual", 20))
            PhaseIdInfo->numPhaseWithoutResidual = GetNumPhases(fp);
        else if (!strncmp(line, "PhaseWeight", 11))
            PhaseIdInfo->numPhaseWeight = GetNumPhases(fp);
        else continue;
    }
/*
 *  Allocate memory for PhaseIdInfo ILOC_PHASELIST structures
 *    For AllowablePhases allocated somewhat larger memory to accommodate
 *    for rare phases identified by the analyst, that are temporarily added
 *    to the allowable phase list, the set of phases iloc can rename a phase
 */
    j = PhaseIdInfo->numAllowablePhases + 50;
    PhaseIdInfo->PhaseMap = (ILOC_PHASEMAP *)calloc(PhaseIdInfo->numPhaseMap,
                                        sizeof(ILOC_PHASEMAP));
    PhaseIdInfo->PhaseWeight = (ILOC_PHASEWEIGHT *)calloc(PhaseIdInfo->numPhaseWeight,
                                        sizeof(ILOC_PHASEWEIGHT));
    PhaseIdInfo->PhaseWithoutResidual = (ILOC_PHASELIST *)calloc(PhaseIdInfo->numPhaseWithoutResidual,
                                        sizeof(ILOC_PHASELIST));
    PhaseIdInfo->AllowablePhases = (ILOC_PHASELIST *)calloc(j, sizeof(ILOC_PHASELIST));
    PhaseIdInfo->firstPphase = (ILOC_PHASELIST *)calloc(PhaseIdInfo->numFirstPphase,
                                        sizeof(ILOC_PHASELIST));
    PhaseIdInfo->firstSphase = (ILOC_PHASELIST *)calloc(PhaseIdInfo->numFirstSphase,
                                        sizeof(ILOC_PHASELIST));
    PhaseIdInfo->firstPoptional = (ILOC_PHASELIST *)calloc(PhaseIdInfo->numFirstPoptional,
                                        sizeof(ILOC_PHASELIST));
    if ((PhaseIdInfo->firstSoptional = (ILOC_PHASELIST *)calloc(PhaseIdInfo->numFirstSoptional,
                                        sizeof(ILOC_PHASELIST))) == NULL) {
        fprintf(stderr, "ReadIASPEIPhaseMapFile: cannot allocate memory\n");
        iLoc_FreePhaseIdInfo(PhaseIdInfo);
        iLoc_Free(line);
        fclose(fp);
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
/*
 *  Read the various lists
 */
    rewind(fp);
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';   /* replace CR with LF */
        }
        if (line[0] == '#' || line[0] == '\n') continue;   /* skip comments */
/*
 *      IASPEI phase map
 */
        if (!strncmp(line, "PhaseMap", 8)) {
            i = 0;
            while ((nb = getline(&line, &nbytes, fp)) > 0) {
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
                if (line[0] == '#') continue;              /* skip comments */
                if (line[0] == '\n') break;       /* ends with a blank line */
                if (sscanf(line, "%s%s", phasehint, phase) > 1) {
                    if (strlen(phasehint) > ILOC_PHALEN ||
                        strlen(phase) > ILOC_PHALEN) {
                        fprintf(stderr, "PhaseMap: phase too long %s %s\n",
                                phasehint, phase);
                        iLoc_FreePhaseIdInfo(PhaseIdInfo);
                        iLoc_Free(line);
                        fclose(fp);
                        return ILOC_STRING_TOO_LONG;
                    }
                    strcpy(PhaseIdInfo->PhaseMap[i].ReportedPhase, phasehint);
                    strcpy(PhaseIdInfo->PhaseMap[i].Phase, phase);
                    i++;
                }
            }
        }
/*
 *      List of allowable phase codes
 */
        else if (!strncmp(line, "AllowablePhases", 15)) {
            if (GetPhaseCodes(PhaseIdInfo->AllowablePhases, fp)) {
                iLoc_FreePhaseIdInfo(PhaseIdInfo);
                iLoc_Free(line);
                fclose(fp);
                return ILOC_STRING_TOO_LONG;
            }
        }
/*
 *      List of allowable first-arriving P phase codes
 */
        else if (!strncmp(line, "AllowableFirstP", 15)) {
            if (GetPhaseCodes(PhaseIdInfo->firstPphase, fp)) {
                iLoc_FreePhaseIdInfo(PhaseIdInfo);
                iLoc_Free(line);
                fclose(fp);
                return ILOC_STRING_TOO_LONG;
            }
        }
/*
 *      List of optional first-arriving P phase codes
 */
        else if (!strncmp(line, "OptionalFirstP", 14)) {
            if (GetPhaseCodes(PhaseIdInfo->firstPoptional, fp)) {
                iLoc_FreePhaseIdInfo(PhaseIdInfo);
                iLoc_Free(line);
                fclose(fp);
                return ILOC_STRING_TOO_LONG;
            }
        }
/*
 *      List of allowable first-arriving S phase codes
 */
        else if (!strncmp(line, "AllowableFirstS", 15)) {
            if (GetPhaseCodes(PhaseIdInfo->firstSphase, fp)) {
                iLoc_FreePhaseIdInfo(PhaseIdInfo);
                iLoc_Free(line);
                fclose(fp);
                return ILOC_STRING_TOO_LONG;
            }
        }
/*
 *      List of optional first-arriving S phase codes
 */
        else if (!strncmp(line, "OptionalFirstS", 14)) {
            if (GetPhaseCodes(PhaseIdInfo->firstSoptional, fp)) {
                iLoc_FreePhaseIdInfo(PhaseIdInfo);
                fclose(fp);
                iLoc_Free(line);
                return ILOC_STRING_TOO_LONG;
            }
        }
/*
 *      List of phase codes with no residual
 */
        else if (!strncmp(line, "PhaseWithoutResidual", 20)) {
            if (GetPhaseCodes(PhaseIdInfo->PhaseWithoutResidual, fp)) {
                iLoc_FreePhaseIdInfo(PhaseIdInfo);
                iLoc_Free(line);
                fclose(fp);
                return ILOC_STRING_TOO_LONG;
            }
        }
/*
 *      distance-phase dependent a priori measurement errors
 *      time, azimuth and slowness
 */
        else if (!strncmp(line, "PhaseWeight", 11)) {
            i = 0;
            while ((nb = getline(&line, &nbytes, fp)) > 0) {
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
                if (line[0] == '#') continue;              /* skip comments */
                if (line[0] == '\n') break;       /* ends with a blank line */
                if (sscanf(line, "%s%lf%lf%lf%lf%lf",
                    phase, &delta1, &delta2, &deltim, &delaz, &delslo) < 6) {
                    fprintf(stderr, "PhaseWeight: incomplete entry %s\n", line);
                    iLoc_FreePhaseIdInfo(PhaseIdInfo);
                    iLoc_Free(line);
                    fclose(fp);
                    return ILOC_INCOMPLETE_INPUT_DATA;
                }
                if (strlen(phase) > ILOC_PHALEN) {
                    fprintf(stderr, "PhaseWeight: phase too long %s\n", phase);
                    iLoc_FreePhaseIdInfo(PhaseIdInfo);
                    iLoc_Free(line);
                    fclose(fp);
                    return ILOC_STRING_TOO_LONG;
                }
                strcpy(PhaseIdInfo->PhaseWeight[i].Phase, phase);
                PhaseIdInfo->PhaseWeight[i].delta1 = delta1;
                PhaseIdInfo->PhaseWeight[i].delta2 = delta2;
                PhaseIdInfo->PhaseWeight[i].deltim = deltim;
                PhaseIdInfo->PhaseWeight[i].delaz = delaz;
                PhaseIdInfo->PhaseWeight[i].delslo = delslo;
                i++;
            }
        }
        else continue;
    }
    fclose(fp);
    iLoc_Free(line);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     GetNumPhases
 *  Synopsis:
 *     Counts the numebr of phases to be read from a block of phases
 *  Input Arguments:
 *     fp - file pointer to read from
 *  Return:
 *     n - number of entries in a block
 *  Called by:
 *     iLoc_ReadAuxDataFiles, ReadIASPEIPhaseMapFile, ReadPhaseTTConfigFile
 */
static int GetNumPhases(FILE *fp)
{
    int n = 0, j;
    char *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#') continue;                      /* skip comments */
        if (line[0] == '\n') break;               /* ends with a blank line */
        n++;
    }
    iLoc_Free(line);
    return n;
}

/*
 *  Title:
 *     GetPhaseCodes
 *  Synopsis:
 *     Read a block of phases in a ILOC_PHASELIST structure
 *  Input Arguments:
 *     pc - pointer to a ILOC_PHASELIST structure
 *     fp - file pointer to read from
 *  Output Arguments:
 *     pc - pointer to a ILOC_PHASELIST structure
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_ReadAuxDataFiles, ReadIASPEIPhaseMapFile, ReadPhaseTTConfigFile
 */
static int GetPhaseCodes(ILOC_PHASELIST *pc, FILE *fp)
{
    int i = 0, j;
    char phase[ILOC_PARLEN], *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#') continue;                      /* skip comments */
        if (line[0] == '\n') break;         /* block ends with a blank line */
        if (sscanf(line, "%s", phase) > 0) {
            if (strlen(phase) > ILOC_PHALEN) {
                fprintf(stderr, "GetPhaseCodes: phase too long %s\n", phase);
                iLoc_Free(line);
                return ILOC_STRING_TOO_LONG;
            }
            strcpy(pc[i].Phase, phase);
            i++;
        }
    }
    iLoc_Free(line);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *      ReadEtopo1
 *  Synopsis:
 *      Reads ETOPO1 topography file and store it in global short int topo
 *      array
 *      ETOPO1:
 *         etopo1_bed_g_i2.bin
 *         Amante, C. and B. W. Eakins,
 *           ETOPO1 1 Arc-Minute Global Relief Model: Procedures, Data Sources
 *           and Analysis.
 *           NOAA Technical Memorandum NESDIS NGDC-24, 19 pp, March 2009.
 *         NCOLS         21601
 *         NROWS         10801
 *         XLLCENTER     -180.000000
 *         YLLCENTER     -90.000000
 *         CELLSIZE      0.01666666667
 *         NODATA_VALUE  -32768
 *         BYTEORDER     LSBFIRST
 *         NUMBERTYPE    2_BYTE_INTEGER
 *         ZUNITS        METERS
 *         MIN_VALUE     -10898
 *         MAX_VALUE     8271
 *         1'x1' resolution, 21601 lons, 10801 lats
 *      Resampled ETOPO1 versions:
 *         etopo2_bed_g_i2.bin
 *           grdfilter -I2m etopo1_bed.grd -Fg10 -D4 -Getopo2_bed.grd
 *             Gridline node registration used
 *             x_min: -180 x_max: 180 x_inc: 0.0333333 name: nx: 10801
 *             y_min: -90 y_max: 90 y_inc: 0.0333333 name: ny: 5401
 *             z_min: -10648.7 z_max: 7399.13 name: m
 *             scale_factor: 1 add_offset: 0
 *         etopo5_bed_g_i2.bin
 *           grdfilter -I5m etopo1_bed.grd -Fg15 -D4 -Getopo5_bed.grd
 *             Gridline node registration used
 *             x_min: -180 x_max: 180 x_inc: 0.0833333 nx: 4321
 *             y_min: -90 y_max: 90 y_inc: 0.0833333 ny: 2161
 *             z_min: -10515.5 z_max: 6917.75 name: m
 *             scale_factor: 1 add_offset: 0
 *  ETOPO parameters are specified in the iLocConfig struct:
 *     EtopoFile - pathname for ETOPO file
 *     EtopoNlon - number of longitude samples in ETOPO
 *     EtopoNlat - number of latitude samples in ETOPO
 *     EtopoRes  - cellsize in ETOPO
 *  Input Arguments:
 *     filename - filename pathname
 *  Return:
 *     topo - ETOPO bathymetry/elevation matrix or NULL on error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_AllocateShortMatrix, iLoc_FreeShortMatrix
 */
static short int **ReadEtopo1(char *filename, int EtopoNlat, int EtopoNlon)
{
    FILE *fp;
    short int **topo = (short int **)NULL;
    unsigned long n, m;
/*
 *  open etopo file
 */
    if ((fp = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "ReadEtopo1: Cannot open %s!\n", filename);
        return (short int **)NULL;
    }
/*
 *  allocate memory
 */
    if ((topo = iLoc_AllocateShortMatrix(EtopoNlat, EtopoNlon)) == NULL)
        return (short int **)NULL;
/*
 *  read etopo file
 */
    n = EtopoNlat * EtopoNlon;
    if ((m = fread(topo[0], sizeof(short int), n, fp)) != n) {
        fprintf(stderr, "ReadEtopo1: Corrupted %s!\n", filename);
        fclose(fp);
        iLoc_FreeShortMatrix(topo);
        return (short int **)NULL;
    }
    fclose(fp);
    return topo;
}

/*
 *  Title:
 *     ReadVariogram
 *  Synopsis:
 *     Reads generic variogram from file and stores it in ILOC_VARIOGRAM structure.
 *  Input Arguments:
 *     fname - pathname of variogram file
 *  Output Arguments:
 *     variogramp - pointer to ILOC_VARIOGRAM structure
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_SplineCoeffs, iLoc_Free
 */
static int ReadVariogram(char *filename, ILOC_VARIOGRAM *variogramp)
{
    FILE *fp;
    int i, j, n = 0, k = 0;
    double sill = 0., maxsep = 0., x, y;
    double *tmp = (double *)NULL;
    char *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
/*
 *  open variogram file and get number of phases
 */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ReadVariogram: cannot open %s\n", filename);
        return ILOC_CANNOT_OPEN_FILE;
    }
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#') continue;                      /* skip comments */
        if (k == 0) {                                  /* number of samples */
            sscanf(line, "%d", &n);
            variogramp->n = n;
/*
 *          memory allocations
 */
            tmp = (double *)calloc(n, sizeof(double));
            variogramp->x = (double *)calloc(n, sizeof(double));
            variogramp->y = (double *)calloc(n, sizeof(double));
            if ((variogramp->d2y = (double *)calloc(n, sizeof(double))) == NULL) {
                fprintf(stderr, "ReadVariogram: cannot allocate memory!\n");
                iLoc_Free(variogramp->y);
                iLoc_Free(variogramp->x);
                iLoc_Free(tmp);
                iLoc_Free(line);
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
            k++;
        }
        else if (k == 1) {                                 /* sill variance */
            sscanf(line, "%lf", &sill);
            variogramp->sill = sill;
            k++;
        }
        else if (k == 2) {                        /* max station separation */
            sscanf(line, "%lf", &maxsep);
            variogramp->maxsep = maxsep;
            k++;
        }
        else if (k == 3) { /* variogram: x = dist [km], y = gamma(x) [s**2] */
            i = 0;
            sscanf(line, "%lf%lf", &x, &y);
            variogramp->x[i] = x;
            variogramp->y[i] = y;
            i++;
            while ((nb = getline(&line, &nbytes, fp)) > 0) {
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
                if (line[0] == '#') continue;               /* skip comments */
                sscanf(line, "%lf%lf", &x, &y);
                variogramp->x[i] = x;
                variogramp->y[i] = y;
                i++;
            }
        }
        else continue;
    }
    fclose(fp);
    iLoc_Free(line);
/*
 *  second derivatives of the natural Spline interpolating function
 */
    iLoc_SplineCoeffs(n, variogramp->x, variogramp->y, variogramp->d2y, tmp);
    iLoc_Free(tmp);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     ReadEllipticityCorrections
 *  Synopsis:
 *     Reads ellipticity correction coefficients table; follows the format of
 *        Kennett, B.L.N. and O. Gudmundsson, 1996,
 *        Ellipticity corrections for seismic phases,
 *        Geophys. J. Int., 127, 40-48.
 *     The tau corrections are stored at 5 degree intervals in distance
 *         and at the depths of 0, 100, 200, 300, 500, 700km.
 *  Input Arguments:
 *     filename - filename
 *  Output Arguments:
 *     numECPhases - number of phases with ellipticity correction
 *  Return:
 *     ec - pointer to ILOC_EC_COEF structure or NULL on error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_AllocateFloatMatrix, iLoc_FreeEllipticityCorrections
 */
static ILOC_EC_COEF *ReadEllipticityCorrections(int *numECPhases, char *filename)
{
    FILE *fp;
    char *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    ILOC_EC_COEF *ec = (ILOC_EC_COEF *)NULL;
    char phase[ILOC_PHALEN], *s;
    int num_pha = 0, num_dist = 0, numDepthSamples = 6;
    int i, j, k, m;
    double mindist = 0., maxdist = 0., d = 0.;
/*
 *  open ellipticity correction file
 */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ReadEllipticityCorrections: cannot open %s\n",
                filename);
        return (ILOC_EC_COEF *)NULL;
    }
/*
 *  count number of phases in ellipticity correction file
 */
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#' || line[0] == '\n') continue;   /* skip comments */
        if (isalpha(line[0])) num_pha++;                    /* count phases */
    }
    *numECPhases = num_pha;
/*
 *  allocate memory for ILOC_EC_COEF structures
 */
    ec = (ILOC_EC_COEF *)calloc(num_pha, sizeof(ILOC_EC_COEF));
    if (ec == NULL) {
        fprintf(stderr, "ReadEllipticityCorrections: cannot allocate memory\n");
        fclose(fp);
        iLoc_Free(line);
        return (ILOC_EC_COEF *)NULL;
    }
/*
 *  populate ec structures
 */
    rewind(fp);
    i = 0;
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#' || line[0] == '\n') continue;   /* skip comments */
/*
 *      read phase block
 */
        if (isalpha(line[0])) {
            sscanf(line, "%s%d%lf%lf", phase, &num_dist, &mindist, &maxdist);
            strcpy(ec[i].Phase, phase);
            ec[i].numDepthSamples = numDepthSamples;
            ec[i].numDistanceSamples = num_dist;
            ec[i].mindist = mindist;
            ec[i].maxdist = maxdist;
            ec[i].depth[0] = 0.;
            ec[i].depth[1] = 100.;
            ec[i].depth[2] = 200.;
            ec[i].depth[3] = 300.;
            ec[i].depth[4] = 500.;
            ec[i].depth[5] = 700.;
/*
 *          allocate memory for distance and EC coefficients
 */
            ec[i].delta = (double *)calloc(num_dist, sizeof(double));
            ec[i].t0 = iLoc_AllocateFloatMatrix(num_dist, numDepthSamples);
            ec[i].t1 = iLoc_AllocateFloatMatrix(num_dist, numDepthSamples);
            if ((ec[i].t2 = iLoc_AllocateFloatMatrix(num_dist, numDepthSamples)) == NULL) {
                iLoc_FreeEllipticityCorrections(num_pha, ec);
                iLoc_Free(line);
                return (ILOC_EC_COEF *) NULL;
            }
            for (m = 0; m < num_dist; m++) {
                nb = getline(&line, &nbytes, fp);
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
                sscanf(line, "%lf", &d);
                ec[i].delta[m] = d;
/*
 *              t0
 */
                nb = getline(&line, &nbytes, fp);
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
                s = strtok(line, " ");
                ec[i].t0[m][0] = atof(s);
                for (k = 1; k < numDepthSamples; k++) {
                    s = strtok(NULL, " ");
                    ec[i].t0[m][k] = atof(s);
                }
/*
 *              t1
 */
                nb = getline(&line, &nbytes, fp);
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
                s = strtok(line, " ");
                ec[i].t1[m][0] = atof(s);
                for (k = 1; k < numDepthSamples; k++) {
                    s = strtok(NULL, " ");
                    ec[i].t1[m][k] = atof(s);
                }
/*
 *              t2
 */
                nb = getline(&line, &nbytes, fp);
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
                s = strtok(line, " ");
                ec[i].t2[m][0] = atof(s);
                for (k = 1; k < numDepthSamples; k++) {
                    s = strtok(NULL, " ");
                    ec[i].t2[m][k] = atof(s);
                }
            }
            i++;
        }
    }
    fclose(fp);
    iLoc_Free(line);
    return ec;
}

/*
 *  Title:
 *     ReadTTtables
 *  Synopsis:
 *     Read travel-time tables for from files in dirname.
 *  Input Arguments:
 *     auxdir - pathname for the auxiliary data files directory
 *     TTInfo - TTInfo structure
 *  Return:
 *     tt_tables - pointer to array of ILOC_TT_TABLE structures or NULL on error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_FreeTTtables, iLoc_AllocateFloatMatrix
 */
static ILOC_TT_TABLE *ReadTTtables(char *auxdir, ILOC_TTINFO *TTInfo)
{
    FILE *fp;
    ILOC_TT_TABLE *tt_tables = (ILOC_TT_TABLE *)NULL;
    char *line = NULL, *s;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    char filename[ILOC_FILENAMELEN];
    int ndists = 0, ndepths = 0, isbounce = 0, ind = 0;
    int i, j, k, kk = 0, m;
/*
 *  memory allocation
 */
    tt_tables = (ILOC_TT_TABLE *)calloc(TTInfo->numPhaseTT, sizeof(ILOC_TT_TABLE));
    if (tt_tables == NULL) {
        fprintf(stderr, "ReadTTtables: cannot allocate memory\n");
        return (ILOC_TT_TABLE *) NULL;
    }
/*
 *  read TT table files
 */
    for (ind = 0; ind < TTInfo->numPhaseTT; ind++) {
/*
 *      initialize tt_tables for this phase
 */
        isbounce = 0;
        if (TTInfo->PhaseTT[ind].Phase[0] == 'p' || TTInfo->PhaseTT[ind].Phase[0] == 's')
            isbounce = 1;
        strcpy(tt_tables[ind].Phase, TTInfo->PhaseTT[ind].Phase);
        tt_tables[ind].ndel = 0;
        tt_tables[ind].ndep = 0;
        tt_tables[ind].isbounce = isbounce;
        tt_tables[ind].deltas = (double *)NULL;
        tt_tables[ind].depths = (double *)NULL;
        tt_tables[ind].tt = (double **)NULL;
        tt_tables[ind].dtdd = (double **)NULL;
        tt_tables[ind].dtdh = (double **)NULL;
        tt_tables[ind].bpdel = (double **)NULL;
/*
 *      open TT table file for this phase
 */
        if (isbounce)
            sprintf(filename, "%s/%s/%s.little%s.tab", auxdir, TTInfo->TTmodel,
                    TTInfo->TTmodel, TTInfo->PhaseTT[ind].Phase);
        else
            sprintf(filename, "%s/%s/%s.%s.tab", auxdir, TTInfo->TTmodel,
                    TTInfo->TTmodel, TTInfo->PhaseTT[ind].Phase);
/*
 *      if no TT table exists for this phase, just skip it
 */
        if ((fp = fopen(filename, "r")) == NULL)
            continue;
/*
 *      read TT table for this phase
 */
        kk = 0;
        while ((nb = getline(&line, &nbytes, fp)) > 0) {
            if ((j = nb - 2) >= 0) {
                if (line[j] == '\r') line[j] = '\n';
            }
            if (line[0] == '#' || line[0] == '\n')         /* skip comments */
                continue;
            if (kk == 0) {
/*
 *              number of delta and depth samples
 */
                sscanf(line, "%d%d", &ndists, &ndepths);
                tt_tables[ind].ndel = ndists;
                tt_tables[ind].ndep = ndepths;
/*
 *              memory allocations
 */
                tt_tables[ind].deltas = (double *)calloc(ndists, sizeof(double));
                tt_tables[ind].depths = (double *)calloc(ndepths, sizeof(double));
                if (isbounce)
                    tt_tables[ind].bpdel = iLoc_AllocateFloatMatrix(ndists, ndepths);
                tt_tables[ind].tt = iLoc_AllocateFloatMatrix(ndists, ndepths);
                tt_tables[ind].dtdd = iLoc_AllocateFloatMatrix(ndists, ndepths);
                tt_tables[ind].dtdh = iLoc_AllocateFloatMatrix(ndists, ndepths);
                if (tt_tables[ind].dtdh == NULL) {
                    iLoc_FreeTTtables(TTInfo->numPhaseTT, tt_tables);
                    iLoc_Free(line);
                    fclose(fp);
                    return (ILOC_TT_TABLE *) NULL;
                }
                kk++;
            }
            else if (kk == 1) {
/*
 *              delta samples (broken into lines of 25 values)
 */
                m = ceil((double)ndists / 25.);
                for (i = 0, k = 0; k < m - 1; k++) {
                    s = strtok(line, " ");
                    tt_tables[ind].deltas[i++] = atof(s);
                    for (j = 1; j < 25; j++) {
                        s = strtok(NULL, " ");
                        tt_tables[ind].deltas[i++] = atof(s);
                    }
                    nb = getline(&line, &nbytes, fp);
                    if ((j = nb - 2) >= 0) {
                        if (line[j] == '\r') line[j] = '\n';
                    }
                }
                if (i < ndists) {
                    s = strtok(line, " ");
                    tt_tables[ind].deltas[i++] = atof(s);
                    for (j = i; j < ndists; j++) {
                        s = strtok(NULL, " ");
                        tt_tables[ind].deltas[i++] = atof(s);
                    }
                }
                kk++;
            }
            else if (kk == 2) {
/*
 *              depth samples (a single line)
 */
                s = strtok(line, " ");
                tt_tables[ind].depths[0] = atof(s);
                for (i = 1; i < ndepths; i++) {
                    s = strtok(NULL, " ");
                    tt_tables[ind].depths[i] = atof(s);
                }
                kk++;
            }
            else if (kk == 3) {
/*
 *              travel-times (ndists rows, ndepths columns)
 */
                for (i = 0; i < ndists; i++) {
                    s = strtok(line, " ");
                    tt_tables[ind].tt[i][0] = atof(s);
                    for (j = 1; j < ndepths; j++) {
                        s = strtok(NULL, " ");
                        tt_tables[ind].tt[i][j] = atof(s);
                    }
                    nb = getline(&line, &nbytes, fp);
                    if ((j = nb - 2) >= 0) {
                        if (line[j] == '\r') line[j] = '\n';
                    }
                }
                kk++;
            }
            else if (kk == 4) {
/*
 *              horizontal slowness (ndists rows, ndepths columns)
 */
                for (i = 0; i < ndists; i++) {
                    s = strtok(line, " ");
                    tt_tables[ind].dtdd[i][0] = atof(s);
                    for (j = 1; j < ndepths; j++) {
                        s = strtok(NULL, " ");
                        tt_tables[ind].dtdd[i][j] = atof(s);
                    }
                    nb = getline(&line, &nbytes, fp);
                    if ((j = nb - 2) >= 0) {
                        if (line[j] == '\r') line[j] = '\n';
                    }
                }
                kk++;
            }
            else if (kk == 5) {
/*
 *              vertical slowness (ndists rows, ndepths columns)
 */
                for (i = 0; i < ndists; i++) {
                    s = strtok(line, " ");
                    tt_tables[ind].dtdh[i][0] = atof(s);
                    for (j = 1; j < ndepths; j++) {
                        s = strtok(NULL, " ");
                        tt_tables[ind].dtdh[i][j] = atof(s);
                    }
                    nb = getline(&line, &nbytes, fp);
                    if ((j = nb - 2) >= 0) {
                        if (line[j] == '\r') line[j] = '\n';
                    }
                }
                kk++;
            }
            else if (kk == 6) {
/*
 *              depth phase bounce point distances (ndists rows, ndepths cols)
 */
                if (isbounce) {
                    for (i = 0; i < ndists; i++) {
                        s = strtok(line, " ");
                        tt_tables[ind].bpdel[i][0] = atof(s);
                        for (j = 1; j < ndepths; j++) {
                            s = strtok(NULL, " ");
                            tt_tables[ind].bpdel[i][j] = atof(s);
                        }
                        nb = getline(&line, &nbytes, fp);
                        if ((j = nb - 2) >= 0) {
                            if (line[j] == '\r') line[j] = '\n';
                        }
                    }
                }
                kk++;
            }
            else continue;
        }
        fclose(fp);
    }
    iLoc_Free(line);
    return tt_tables;
}

/*
 *  Title:
 *     ReadRSTTModel
 *  Synopsis:
 *     Read and initialize RSTT
 *  Input Arguments:
 *     filename - pathname for RSTT model
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     slbm_shell_create, slbm_shell_loadVelocityModelBinary, slbm_shell_delete,
 *     slbm_shell_setMaxDistance, slbm_shell_setInterpolatorType
 */
static int ReadRSTTModel(char *filename)
{
    char *buffer = "NATUTAL_NEIGHBOR";
    double d = ILOC_DEG2RAD * ILOC_MAX_RSTT_DIST; /* max RSTT distance in rad */
    slbm_shell_create();
    if (slbm_shell_loadVelocityModelBinary(filename)) {
        fprintf(stderr, "ReadRSTTModel: Cannnot read RSTT model %s\n", filename);
        slbm_shell_delete();
        return ILOC_CANNOT_OPEN_FILE;
    }
    slbm_shell_setMaxDistance(&d);
    slbm_shell_setInterpolatorType(buffer);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     ReadFlinnEngdahl
 *  Synopsis:
 *     Reads Flinn-Engdahl regionalization file and stores it in ILOC_FE structure.
 *
 *     Young, J.B., Presgrave, B.W., Aichele, H., Wiens, D.A. and Flinn, E.A.,
 *     1996, The Flinn-Engdahl Regionalisation Scheme: the 1995 revision,
 *     Physics of the Earth and Planetary Interiors, 96, 223-297.
 *
 *     For each latitude (from 90N to 90S) a set of longitude ranges
 *         is given (first part of the file). The second part of the file
 *         lists the geographic region numbers for each latitude for within
 *         the longitude ranges.
 *  Input Arguments:
 *     filename - pathname for the Flinn-Engdahl regionalization file
 *  Output Arguments:
 *     fep - pointer to ILOC_FE structure
 *  Return:
 *     Success/error
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_Free
 */
static int ReadFlinnEngdahl(char *filename, ILOC_FE *fep)
{
    FILE *fp;
    char *line = NULL, *s;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    int nlat = 0, nlon = 0, i, j, n = 0, k = 0;
/*
 *  open and read Flinn-Engdahl regionalization file
 */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ReadFlinnEngdahl: cannot open %s\n", filename);
        return ILOC_CANNOT_OPEN_FILE;
    }
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#' || line[0] == '\n')         /* skip comments */
            continue;
        if (k == 0) {
/*
 *          number of latitudes
 */
            sscanf(line, "%d", &nlat);
            fep->nlat = nlat;
/*
 *          memory allocations
 */
            fep->lon = (int **)calloc(nlat, sizeof(int *));
            fep->grn = (int **)calloc(nlat, sizeof(int *));
            if ((fep->nl = (int *)calloc(nlat, sizeof(int))) == NULL) {
                fprintf(stderr, "ReadFlinnEngdahl: cannot allocate memory\n");
                fclose(fp);
                iLoc_Free(fep->grn);
                iLoc_Free(fep->lon);
                iLoc_Free(line);
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
            k++;
        }
        else if (k == 1) {
/*
 *          number of longitudes at each latitude
 */
            s = strtok(line, " ");
            fep->nl[0] = atoi(s);
            nlon = fep->nl[0];
            for (i = 1; i < nlat; i++) {
                 s = strtok(NULL, " ");
                 fep->nl[i] = atoi(s);
                 nlon += fep->nl[i];
            }
/*
 *          memory allocations
 */
            if ((fep->lon[0] = (int *)calloc(nlat * nlon, sizeof(int))) == NULL) {
                fprintf(stderr, "ReadFlinnEngdahl: cannot allocate memory\n");
                fclose(fp);
                iLoc_Free(fep->nl);
                iLoc_Free(fep->grn);
                iLoc_Free(fep->lon);
                iLoc_Free(line);
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
            if ((fep->grn[0] = (int *)calloc((nlat - 1) * nlon, sizeof(int))) == NULL) {
                fprintf(stderr, "ReadFlinnEngdahl: cannot allocate memory\n");
                fclose(fp);
                iLoc_Free(fep->nl);
                iLoc_Free(fep->lon[0]);
                iLoc_Free(fep->lon);
                iLoc_Free(fep->grn);
                iLoc_Free(line);
                return ILOC_MEMORY_ALLOCATION_ERROR;
            }
            for (i = 1; i < nlat; i++) {
                fep->lon[i] = fep->lon[i - 1] + fep->nl[i - 1];
                fep->grn[i] = fep->grn[i - 1] + fep->nl[i - 1] - 1;
            }
            k++;
        }
        else if (k == 2) {
/*
 *          longitude ranges at each latitude
 */
            for (i = 0; i < nlat; i++) {
                s = strtok(line, " ");
                n = atoi(s);
                for (j = 0; j < n; j++) {
                    s = strtok(NULL, " ");
                    fep->lon[i][j] = atoi(s);
                }
                nb = getline(&line, &nbytes, fp);
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
            }
            k++;
        }
        else if (k == 3) {
/*
 *          geographic region numbers for corresponding longitude ranges
 *          per latitude
 */
            for (i = 0; i < nlat; i++) {
                s = strtok(line, " ");
                n = atoi(s);
                for (j = 0; j < n; j++) {
                    s = strtok(NULL, " ");
                    fep->grn[i][j] = atoi(s);
                }
                nb = getline(&line, &nbytes, fp);
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
            }
            k++;
        }
        else continue;
    }
    fclose(fp);
    iLoc_Free(line);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     ReadDefaultDepthGregion
 *  Synopsis:
 *     Reads geographic region number and model specific default depths from
 *     file.
 *     File contains default depths for each Flinn-Engdahl geographic
 *     region number.
 *     columns:
 *         grn:   Flinn-Engdahl geographic region number
 *         depth: default depth for a GRN
 *  Input Arguments:
 *     filename - pathname for the default depth file
 *  Return:
 *     GrnDepth - pointer to default depth vector
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_Free
 */
static double *ReadDefaultDepthGregion(char *filename)
{
    FILE *fp;
    char *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    double *GrnDepth = (double *)NULL;
    double depth = 0.;
    int ngreg = 757, i, j, m;
/*
 *  memory allocation
 */
    if ((GrnDepth = (double *)calloc(ngreg, sizeof(double))) == NULL) {
        fprintf(stderr, "ReadDefaultDepth: cannot allocate memory\n");
        return (double *)NULL;
    }
/*
 *  open GRN default depth file
 */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ReadDefaultDepth: cannot open %s\n", filename);
        iLoc_Free(GrnDepth);
        iLoc_Free(line);
        return (double *)NULL;
    }
/*
 *  read GRN default depth file
 */
    for (i = 0; i < ngreg; i++) {
        nb = getline(&line, &nbytes, fp);
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        sscanf(line, "%d%lf", &m, &depth);
        GrnDepth[i] = depth;
    }
    fclose(fp);
    iLoc_Free(line);
    return GrnDepth;
}

/*
 *  Title:
 *     ReadDefaultDepthGrid
 *  Synopsis:
 *     Reads default depth grid from file.
 *
 *     The default depth grid follows gridline registration, i.e. the nodes are
 *     centered on the grid line intersections and the data points represent
 *     the median value in a cell of dimensions (gres x gres) centered on the
 *     nodes.
 *
 *      lat(i-1) +--------+--------+--------+
 *               |        |        |        |
 *               |        |        |        |
 *               |    #########    |        |
 *               |    #   |   #    |        |
 *      lat(i)   +----#---o---#----+--------+
 *               |    #   |   #    |        |
 *               |    #########    |        |
 *               |        |        |        |
 *               |        |        |        |
 *      lat(i+1) +--------+--------+--------+
 *            lon(j-1)  lon(j)   lon(j+1) lon(j+2)
 *
 *     columns:
 *        lat, lon: center of grid cells
 *        depth: median depth in the cell
 *        min:   minimum depth in the cell
 *        25Q:   25th percentile depth in the cell
 *        75Q:   75th percentile depth in the cell
 *        max:   maximum depth in the cell
 *        N:     number of observations in the cell
 *        range: quartile range (75Q - 25Q)
 *     rows ordered by descending latitude and increasing longitude
 *  Input Arguments:
 *     filename - pathname for the default depth file
 *  Output Arguments:
 *     gres  - grid spacing
 *     ngrid - number of grid points
 *  Return:
 *     DepthGrid - pointer to default depth grid (lat, lon, depth)
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_AllocateFloatMatrix
 */
static double **ReadDefaultDepthGrid(char *filename, double *gres, int *ngrid)
{
    FILE *fp;
    char *line = NULL;
    ssize_t nb = (ssize_t)0;
    size_t nbytes = 0;
    double **DepthGrid = (double **)NULL;
    double lat = 0., lon = 0., depth = 0., res = 1.;
    double q25 = 0., q75 = 0., lo = 0., hi = 0., range = 0.;
    int n = 0, i,j,  m = 0, k = 0, kk = 0;
/*
 *  open and read default depth grid file
 */
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "ReadDefaultDepthGrid: cannot open %s\n", filename);
        return (double **)NULL;
    }
    while ((nb = getline(&line, &nbytes, fp)) > 0) {
        if ((j = nb - 2) >= 0) {
            if (line[j] == '\r') line[j] = '\n';
        }
        if (line[0] == '#' || line[0] == '\n')         /* skip comments */
            continue;
        if (kk == 0) {
/*
 *          grid spacing
 */
            sscanf(line, "%lf", &res);
            kk++;
        }
        else if (kk == 1) {
/*
 *          number of gridpoints
 */
            sscanf(line, "%d", &n);
/*
 *          memory allocation
 */
            if ((DepthGrid = iLoc_AllocateFloatMatrix(n, 3)) == NULL) {
                iLoc_Free(line);
                return (double **)NULL;
            }
            kk++;
        }
        else if (kk == 2) {
/*
 *          default depth grid
 */
            k = 0;
            for (i = 0; i < n; i++) {
                sscanf(line, "%lf%lf%lf%lf%lf%lf%lf%d%lf",
                       &lat, &lon, &depth, &lo, &q25, &q75, &hi, &m, &range);
/*
 *              ignore estimates with too large range
 */
                if (range < 100.) {
/*
 *                  populate default depth grid
 */
                    DepthGrid[k][0] = lat;
                    DepthGrid[k][1] = lon;
                    DepthGrid[k][2] = depth;
                    k++;
                }
                nb = getline(&line, &nbytes, fp);
                if ((j = nb - 2) >= 0) {
                    if (line[j] == '\r') line[j] = '\n';
                }
            }
            kk++;
        }
        else continue;
    }
    fclose(fp);
    iLoc_Free(line);
    *gres = res;
    *ngrid = k;
    return DepthGrid;
}

/*
 *  Title:
 *     iLoc_AllocateFloatMatrix
 *  Synopsis:
 *     Allocates memory to a double matrix.
 *  Input Arguments:
 *     nrow - number of rows
 *     ncol - number of columns
 *  Returns:
 *     pointer to matrix
 */
double **iLoc_AllocateFloatMatrix(int nrow, int ncol)
{
    double **matrix = (double **)NULL;
    int i;
    if ((matrix = (double **)calloc(nrow, sizeof(double *))) == NULL) {
        fprintf(stderr, "iLoc_AllocateFloatMatrix: cannot allocate memory\n");
        return (double **)NULL;
    }
    if ((matrix[0] = (double *)calloc(nrow * ncol, sizeof(double))) == NULL) {
        fprintf(stderr, "iLoc_AllocateFloatMatrix: cannot allocate memory\n");
        iLoc_Free(matrix);
        return (double **)NULL;
    }
    for (i = 1; i < nrow; i++)
        matrix[i] = matrix[i - 1] + ncol;
    return matrix;
}

/*
 *  Title:
 *     iLoc_AllocateShortMatrix
 *  Synopsis:
 *     Allocates memory to a short integer matrix.
 *  Input Arguments:
 *     nrow - number of rows
 *     ncol - number of columns
 *  Returns:
 *     pointer to matrix
 */
short int **iLoc_AllocateShortMatrix(int nrow, int ncol)
{
    short int **matrix = (short int **)NULL;
    int i;
    if ((matrix = (short int **)calloc(nrow, sizeof(short int *))) == NULL) {
        fprintf(stderr, "iLoc_AllocateShortMatrix: cannot allocate memory\n");
        return (short int **)NULL;
    }
    if ((matrix[0] = (short int *)calloc(nrow * ncol, sizeof(short int))) == NULL) {
        fprintf(stderr, "iLoc_AllocateShortMatrix: cannot allocate memory\n");
        iLoc_Free(matrix);
        return (short int **)NULL;
    }
    for (i = 1; i < nrow; i++)
        matrix[i] = matrix[i - 1] + ncol;
    return matrix;
}

/*
 *  Title:
 *     iLoc_AllocateLongMatrix
 *  Synopsis:
 *     Allocates memory to an unsigned long matrix.
 *  Input Arguments:
 *     nrow - number of rows
 *     ncol - number of columns
 *  Returns:
 *     pointer to matrix
 */
unsigned long **iLoc_AllocateLongMatrix(int nrow, int ncol)
{
    unsigned long **matrix = (unsigned long **)NULL;
    int i;
    if ((matrix = (unsigned long **)calloc(nrow, sizeof(unsigned long *))) == NULL) {
        fprintf(stderr, "iLoc_AllocateLongMatrix: cannot allocate memory\n");
        return (unsigned long **)NULL;
    }
    if ((matrix[0] = (unsigned long *)calloc(nrow * ncol, sizeof(unsigned long))) == NULL) {
        fprintf(stderr, "iLoc_AllocateLongMatrix: cannot allocate memory\n");
        iLoc_Free(matrix);
        return (unsigned long **)NULL;
    }
    for (i = 1; i < nrow; i++)
        matrix[i] = matrix[i - 1] + ncol;
    return matrix;
}

