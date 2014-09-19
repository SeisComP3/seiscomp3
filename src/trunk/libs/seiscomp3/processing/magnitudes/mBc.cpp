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



#include <seiscomp3/processing/magnitudes/mBc.h>
#include <seiscomp3/seismology/magnitudes.h>
#include <math.h>

namespace Seiscomp {

namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_mBc, MagnitudeProcessor, "MagnitudeProcessor_mBc");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mBc, "mBc");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mBc::MagnitudeProcessor_mBc()
 : MagnitudeProcessor_mB("mBc") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mBc::estimateMw(
	double magnitude,
	double &Mw_estimate,
	double &Mw_stdError)
{
	// from Saul & Bormann (2009), preliminary + subject to revision
	const double a=1.22, b=-2.11;
//	if ( magnitude>=6 ) {
		Mw_estimate = a * magnitude + b;
//	}
//	else {
//		// This is to limit the difference between mB and Mw(mB)
//		// FIXME hack to be revised...
//		estimation = a * 6. + b + 0.7*(magnitude-6);
//	}

	Mw_stdError = 0.4; // This is a dummy!

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
