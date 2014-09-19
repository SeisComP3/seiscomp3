/***************************************************************************
* Copyright (C) 2009 BY GFZ Potsdam,                                       *
* EMail: seiscomp-devel@gfz-potsdam.de                                     * 
* The term "Non-Commercial Entity" is limited to the following:            *
* - Research institutes                                                    *
* - Public institutes dealing with natural hazards warnings                * 
*                                                                          *
* Provided that you qualify for a Non-Commercial Version License as        *
* specified above, and subject to the terms and conditions contained       *
* herein,GFZ Potsdam hereby grants the licensee, acting as an end user,    *
* an institute wide, non-transferable, non-sublicensable license, valid    *
* for an unlimited period of time, to install and use the Software, free   *
* of charge, for non-commercial purposes only. The licensee is allowed     *
* to develop own software modules based on the SeisComP3 libraries         *
* licensed under GPL and connect them with software modules under this     *
* license. The number of copies is for the licensee not limited within     *
* the user community of the licensee.                                      *
*                                                                          *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
* IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR    * 
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, * 
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        *
* SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.                 *
*                                                                          * 
* GFZ Potsdam reserve all rights not expressly granted to the licensee     *
* herein. Find more details of the license at geofon.gfz-potsdam.de        *
***************************************************************************/

#include<math.h>
#include "mwp_utils.h"

void
Mwp_demean(int n, double *f, int i0)
{
	int i;
	double sum = 0, mean;

	for (i=0; i<i0; i++)
		sum += f[i];
	mean = sum/i0;

	for (i=0; i<n; i++)
		f[i] -= mean;
}

void
Mwp_taper(int n, double *f, int i0)
{
	int i, nn=i0/2;
	double q = M_PI/nn;
	for (i=0; i<nn; i++)
		f[i] *= 0.5*(1-cos(i*q));
}

static void
Mwp_integr(int n, double *f, int i0)
{
	int i;
	double sum = 0;

//	Mwp_demean(n, f, i0);

	for (i=0; i<n; i++) {
		sum += f[i];
		f[i] = sum;
	}
}


static void
Mwp_scale(int n, double *f, double factor)
{
	int i;

	for (i=0; i<n; i++) {
		f[i] *= factor;
	}
}

double
Mwp_SNR(int n, double *f, int i0)
{
	int i;
	double smax = 0, nmax = 0;

	for (i=0; i<i0; i++) {
		double n = fabs(f[i]);
		if (n > nmax)
			nmax = n;
	}
	for (i=i0; i<n; i++) {
		double s = fabs(f[i]);
		if (s > smax)
			smax = s;
	}

	return smax/nmax;
}


double
Mwp_amplitude(int n, double *f, int i0, int *pos)
{
	int i;
	double smax = 0;
	*pos = i0;

	for (i=i0; i<n; i++) {
		double s = fabs(f[i]);
		if (s > smax) {
			*pos = i;
			smax = s;
		}
	}

	return smax;
}


void
Mwp_double_integration(int n, double *f, int i0, double fsamp)
{
	Mwp_integr(n, f, i0);
	Mwp_integr(n, f, i0);
	Mwp_scale (n, f, 1/(fsamp*fsamp));
}
