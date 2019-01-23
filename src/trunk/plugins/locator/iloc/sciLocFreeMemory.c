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
 *     iLoc_FreeAuxData
 *     iLoc_FreePhaseIdInfo
 *     iLoc_FreeDefaultDepth
 *     iLoc_FreeFlinnEngdahl
 *     iLoc_FreeVariogram
 *     iLoc_FreeEllipticityCorrections
 *     iLoc_FreeTTtables
 *     iLoc_Free
 *     iLoc_FreeFloatMatrix
 *     iLoc_FreeShortMatrix
 */

/*
 *  Title:
 *     iLoc_FreeAuxData
 *  Synopsis:
 *     Frees memory allocated to auxiliary data structures
 *  Input Arguments:
 *     PhaseIdInfo   - pointer to ILOC_PHASEIDINFO structure
 *     fe            - pointer to ILOC_FE structure
 *     DefaultDepth  - pointer to ILOC_DEFAULTDEPTH structure
 *     Variogram     - pointer to ILOC_VARIOGRAM structure
 *     TTInfo        - pointer to ILOC_TTINFO structure
 *     TTtables      - array of ILOC_TT_TABLE structures
 *     ec            - array of ILOC_EC_COEF structures
 *     LocalTTInfo   - pointer to ILOC_TTINFO structure
 *     LocalTTtables - array of ILOC_TT_TABLE structures
 *  Called by:
 *     SeisComp3 iLoc app
 *  Calls:
 *     iLoc_FreePhaseIdInfo
 *     iLoc_FreeTTtables
 *     iLoc_FreeEllipticityCorrections
 *     iLoc_Free
 *     iLoc_FreeFlinnEngdahl
 *     iLoc_FreeDefaultDepth
 *     iLoc_FreeVariogram
 *     slbm_shell_delete
 */
int iLoc_FreeAuxData(ILOC_PHASEIDINFO *PhaseIdInfo, ILOC_FE *fe,
        ILOC_DEFAULTDEPTH *DefaultDepth, ILOC_VARIOGRAM *Variogram,
        ILOC_TTINFO *TTInfo, ILOC_TT_TABLE *TTtables, ILOC_EC_COEF *ec,
        ILOC_TTINFO *LocalTTInfo, ILOC_TT_TABLE *LocalTTtables, int UseRSTT)
{
    iLoc_FreePhaseIdInfo(PhaseIdInfo);
    iLoc_FreeTTtables(TTInfo->numPhaseTT, TTtables);
    iLoc_FreeEllipticityCorrections(TTInfo->numECPhases, ec);
    iLoc_Free(TTInfo->PhaseTT);
    if (LocalTTtables != NULL) {
        iLoc_FreeTTtables(LocalTTInfo->numPhaseTT, LocalTTtables);
        iLoc_Free(LocalTTInfo->PhaseTT);
    }
    iLoc_FreeFlinnEngdahl(fe);
    iLoc_FreeDefaultDepth(DefaultDepth);
    iLoc_FreeVariogram(Variogram);
    if (UseRSTT)
        slbm_shell_delete();
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     iLoc_FreePhaseIdInfo
 *  Synopsis:
 *     Frees memory allocated to arrays in ILOC_PHASEIDINFO structure
 *  Input Arguments:
 *     PhaseIdInfo - ILOC_PHASEIDINFO structure
 *  Called by:
 *     iLoc_FreeAuxData, iLoc_ReadAuxDataFiles, ReadIASPEIPhaseMapFile
 *  Calls:
 *     iLoc_Free
 */
void iLoc_FreePhaseIdInfo(ILOC_PHASEIDINFO *PhaseIdInfo)
{
    iLoc_Free(PhaseIdInfo->firstSoptional);
    iLoc_Free(PhaseIdInfo->firstPoptional);
    iLoc_Free(PhaseIdInfo->firstSphase);
    iLoc_Free(PhaseIdInfo->firstPphase);
    iLoc_Free(PhaseIdInfo->AllowablePhases);
    iLoc_Free(PhaseIdInfo->PhaseWithoutResidual);
    iLoc_Free(PhaseIdInfo->PhaseWeight);
    iLoc_Free(PhaseIdInfo->PhaseMap);
}

/*
 *  Title:
 *     iLoc_FreeDefaultDepth
 *  Synopsis:
 *     iLoc_Frees memory allocated to arrays in ILOC_DEFAULTDEPTH structure
 *  Input Arguments:
 *     DefaultDepth - ILOC_DEFAULTDEPTH structure
 *  Called by:
 *     iLoc_FreeAuxData, iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_Free, iLoc_FreeFloatMatrix, iLoc_FreeShortMatrix
 */
void iLoc_FreeDefaultDepth(ILOC_DEFAULTDEPTH *DefaultDepth)
{
    iLoc_Free(DefaultDepth->GrnDepth);
    iLoc_FreeFloatMatrix(DefaultDepth->DepthGrid);
    iLoc_FreeShortMatrix(DefaultDepth->Topo);
}

/*
 *  Title:
 *     iLoc_FreeFlinnEngdahl
 *  Synopsis:
 *     Frees memory allocated to ILOC_FE structure.
 *  Input Arguments:
 *     fep - pointer to ILOC_FE structure
 *  Called by:
 *     iLoc_ReadAuxDataFiles
 */
void iLoc_FreeFlinnEngdahl(ILOC_FE *fep)
{
    iLoc_Free(fep->nl);
    iLoc_Free(fep->lon[0]);
    iLoc_Free(fep->lon);
    iLoc_Free(fep->grn[0]);
    iLoc_Free(fep->grn);
}

/*
 *  Title:
 *     iLoc_FreeVariogram
 *  Synopsis:
 *     Frees memory allocated to ILOC_VARIOGRAM structure
 *  Input Arguments:
 *     Variogram - pointer to ILOC_VARIOGRAM structure
 *  Called by:
 *     iLoc_FreeAuxData, iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_Free
 */
void iLoc_FreeVariogram(ILOC_VARIOGRAM *Variogram)
{
    iLoc_Free(Variogram->d2y);
    iLoc_Free(Variogram->y);
    iLoc_Free(Variogram->x);
}

/*
 *  Title:
 *     iLoc_FreeEllipticityCorrections
 *  Synopsis:
 *     Frees memory allocated to ec_coef structure.
 *  Input Arguments:
 *      numECPhases - number of distinct phases
 *     ec           - array of ILOC_EC_COEF structures
 *  Calls:
 *     iLoc_FreeFloatMatrix, iLoc_Free
 *  Called by:
 *      iLoc_ReadAuxDataFiles
 */
void iLoc_FreeEllipticityCorrections(int numECPhases, ILOC_EC_COEF *ec)
{
    int i;
    for (i = 0; i < numECPhases; i++) {
        iLoc_FreeFloatMatrix(ec[i].t2);
        iLoc_FreeFloatMatrix(ec[i].t1);
        iLoc_FreeFloatMatrix(ec[i].t0);
        iLoc_Free(ec[i].delta);
    }
    iLoc_Free(ec);
}

/*
 *  Title:
 *     iLoc_FreeTTtables
 *  Synopsis:
 *     Frees memory allocated to ILOC_TT_TABLE structures
 *  Input Arguments:
 *     TTtables - array of TT table structures
 *  Called by:
 *     iLoc_FreeAuxData, iLoc_ReadAuxDataFiles
 *  Calls:
 *     iLoc_FreeFloatMatrix, iLoc_Free
 */
void iLoc_FreeTTtables(int numPhaseTT, ILOC_TT_TABLE *TTtables)
{
    int i, ndists = 0;
    for (i = 0; i < numPhaseTT; i++) {
        if ((ndists = TTtables[i].ndel) == 0) continue;
        iLoc_FreeFloatMatrix(TTtables[i].dtdh);
        iLoc_FreeFloatMatrix(TTtables[i].dtdd);
        iLoc_FreeFloatMatrix(TTtables[i].tt);
        if (TTtables[i].isbounce)
            iLoc_FreeFloatMatrix(TTtables[i].bpdel);
        iLoc_Free(TTtables[i].depths);
        iLoc_Free(TTtables[i].deltas);
    }
    iLoc_Free(TTtables);
}

/*
 *
 * iLoc_Free: a smart iLoc_Free
 *
 */
void iLoc_Free(void *ptr)
{
    if (ptr != NULL)
        free(ptr);
}

/*
 *  Title:
 *     iLoc_FreeFloatMatrix
 *  Synopsis:
 *     Frees memory allocated to a matrix.
 *  Input Arguments:
 *     matrix - matrix
 */
void iLoc_FreeFloatMatrix(double **matrix)
{
    if (matrix != NULL) {
        iLoc_Free(matrix[0]);
        iLoc_Free(matrix);
    }
}

/*
 *  Title:
 *     iLoc_FreeShortMatrix
 *  Synopsis:
 *     Frees memory allocated to a short integer matrix.
 *  Input Arguments:
 *     matrix - matrix
 */
void iLoc_FreeShortMatrix(short int **matrix)
{
    if (matrix != NULL) {
        iLoc_Free(matrix[0]);
        iLoc_Free(matrix);
    }
}

/*
 *  Title:
 *     iLoc_FreeLongMatrix
 *  Synopsis:
 *     Frees memory allocated to an unsigned long matrix.
 *  Input Arguments:
 *     matrix - matrix
 */
void iLoc_FreeLongMatrix(unsigned long **matrix)
{
    if (matrix != NULL) {
        iLoc_Free(matrix[0]);
        iLoc_Free(matrix);
    }
}

