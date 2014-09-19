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


#include<math.h>

// Disable hilbert transformation for now
#if 0

#include<fftw3.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


// low-level, real in-place hilbert transform
//
// The trace length need not be a power of 2
template<typename TYPE>
void hilbert_transform(std::vector<TYPE> &trace,
		       int direction=1)
{
	TYPE *f = &trace[0];
        int ndata=trace.size(),
	    nfft = next_power_of_2(ndata),
	    n2 = nfft/2;

        // temporary trace for real FFTW
	std::vector<double> temp(nfft);
	double *g = &temp[0];
        // copy real array and pad with zeros
        for (int i=0; i<ndata; i++)
                g[i] = f[i];
        for (int i=ndata; i<nfft; i++)
                g[i] = 0.;

        fftw_plan plan_fwd, plan_inv;
	plan_fwd = fftw_plan_r2r_1d (nfft, g, g, FFTW_R2HC, FFTW_ESTIMATE);
	plan_inv = fftw_plan_r2r_1d (nfft, g, g, FFTW_HC2R, FFTW_ESTIMATE);

        fftw_execute(plan_fwd); // forward FFT

        // perform actual Hilbert transform in frequency domain
        if (direction>0)
	{
                for (int i=1; i<n2; i++)
		{
                        double x  = g[nfft-i];
                        g[nfft-i] = -g[i];
                        g[i]      = x;
                }
	}
        else	{
		for (int i=1; i<n2; i++) {
                        double x  = g[nfft-i];
                        g[nfft-i] = g[i];
                        g[i]      = -x;
                }
	}

        fftw_execute(plan_inv); // inverse FFT

        // scale and copy back to real array
        double q = 1./nfft;
        for (int i=0; i<ndata; i++)
                f[i] = q * g[i];

	fftw_destroy_plan(plan_fwd);
	fftw_destroy_plan(plan_inv);
}

template<typename TYPE>
void envelope(std::vector<TYPE> &trace)
{
	std::vector<TYPE> copy(trace.begin(), trace.end());
	hilbert_transform(copy);
	int nsamp = trace.size();
	TYPE *f1 = &trace[0], *f2 = &copy[0];
	for(int i=0; i<nsamp; i++)
		f1[i] = (TYPE) ::sqrt( f1[i]*f1[i] + f2[i]*f2[i] );
}

} // namespace Seiscomp::Math::Filter

} // namespace Seiscomp::Math

} // namespace Seiscomp

#endif
