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



#ifndef __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_Mwp_H__
#define __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_Mwp_H__

#include <seiscomp3/processing/amplitudeprocessor.h>


namespace Seiscomp {

namespace Processing {


class SC_SYSTEM_CLIENT_API AmplitudeProcessor_Mwp : public AmplitudeProcessor {
	DECLARE_SC_CLASS(AmplitudeProcessor_Mwp);

	public:
		AmplitudeProcessor_Mwp();
		AmplitudeProcessor_Mwp(const Seiscomp::Core::Time& trigger);

	public:
		void setHint(ProcessingHint hint, double value);

		const DoubleArray *processedData(Component comp) const;

	protected:
		double timeWindowLength(double distance) const;
		bool computeAmplitude(const DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt,
		                      AmplitudeValue *amplitude,
		                      double *period, double *snr);

	private:
		void init();

	private:
		double _epizentralDistance;
		DoubleArray _processedData;
};



}

}


#endif
