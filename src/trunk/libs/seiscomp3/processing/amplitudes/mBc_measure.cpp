#include <math.h>
#include "mBc_measure.h"

Measurement::Measurement(int nsamp)
	: nsamp(nsamp)
{
	offset = 0;
	processed = 0;
}

Measurement_mBc::Measurement_mBc(int nsamp, double q)
	: Measurement(nsamp), q(q)
{
	imax = 0;
	vcum = vmax = vpeak = 0;
}

void Measurement_mBc::feed(int n, const double *v)
{
	if (processed == 0 && n > 0)
		// deferred initialization until the first sample
		previous = (v[0] > 0);

	// Start loop at the 1st unprocessed sample
	for (int i=0; i<n; i++) {
		double vi = v[i]-offset;  // demeaned data sample
		// Any zero crossing ?
		if ( (vi > 0) != previous) {
			// Does the current peak exceed the threshold?
			if (vpeak > q*vmax) {
				// save the position of that peak
				subevent_indices.push_back(ipeak);
				subevent_values.push_back(vpeak);
				// update cumulative amplitude
				vcum += vpeak;
				icum  = ipeak;
			}
			// Is the current peak a new maximum?
			if (vpeak > vmax) {
				vmax = vpeak;
				imax = ipeak;
			}

			// Update the sign
			previous = (vi > 0);

			// Start a new peak measurement by resetting the peak amplitude
			vpeak = 0;
		}

		// We're within a peak -> update amplitude if necessary.
		vi = fabs(vi);
		if (vi > vpeak) {
			vpeak = vi;
			ipeak = processed+i;
		}
	}

	// keep track of processed samples
	processed += n;
}

int Measurement_mBc::subevent_count() const
{
	return subevent_indices.size();
}

int Measurement_mBc::subevent_index(int i) const
{
	return subevent_indices[i];
}

double Measurement_mBc::subevent_value(int i) const
{
	return subevent_values[i];
}
