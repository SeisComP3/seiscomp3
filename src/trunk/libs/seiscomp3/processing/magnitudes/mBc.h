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



#ifndef __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_C_H__
#define __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_C_H__

#include <seiscomp3//processing/magnitudes/m_B.h>


namespace Seiscomp {

namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_mBc : public MagnitudeProcessor_mB {
	DECLARE_SC_CLASS(MagnitudeProcessor_mBc);

	public:
		MagnitudeProcessor_mBc();

		Status estimateMw(double magnitude, double &Mw_estimate,
		                  double &Mw_stdError);
};


}

}


#endif
