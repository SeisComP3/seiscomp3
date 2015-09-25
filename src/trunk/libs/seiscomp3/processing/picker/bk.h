/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   2010 Modified by Stefan Heimers at the Swiss Seismological Service    *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/



#ifndef __SEISCOMP_PROCESSING_BKPICKER_H__
#define __SEISCOMP_PROCESSING_BKPICKER_H__


#include <seiscomp3/processing/picker.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API BKPicker : public Picker {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		BKPicker();
		BKPicker(const Core::Time& trigger);
		//! D'tor
		~BKPicker();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setup(const Settings &settings);
		const std::string &methodID() const;
		const std::string &filterID() const;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		// filter settings
		std::string filterType;
		int    filterPoles; // number of poles
		double f1; // bandpass lower cutoff freq. in Hz
		double f2; // bandpass upper cutoff freq. in Hz
		std::string usedFilter;

		// picker parameters
		double thrshl1; // threshold to trigger for pick (c.f. paper), default 10 
		double thrshl2; //  threshold for updating sigma  (c.f. paper), default 20 

		int    debugOutput;
		
		void bk_wrapper(int n, double *data, int &kmin, double &snr, double samplespersec=120);
		bool calculatePick(int n, const double *data,
		                   int signalStartIdx, int signalEndIdx,
		                   int &triggerIdx, int &lowerUncertainty,
		                   int &upperUncertainty, double &snr,
		                   OPT(Polarity) &polarity);
};


}
}

#endif
