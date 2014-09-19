/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISMOLOGY_MWP_H__
#define __SEISMOLOGY_MWP_H__

/*
 * i0 is the index at which the onset is expected.
 * i < i0 is noise, i >= i0 is P-wave signal
 */
void   Mwp_double_integration(int n, double *f, int i0, double fsamp);

double Mwp_SNR(int n, double *f, int i0);

double Mwp_amplitude(int n, double *f, int i0, int *pos);

void   Mwp_demean(int n, double *f, int i0);
void   Mwp_taper(int n, double *f, int i0);

#endif
