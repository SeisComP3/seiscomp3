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


#ifndef __SEISCOMP_PROCESSING_PICKER_S_L2_H__
#define __SEISCOMP_PROCESSING_PICKER_S_L2_H__


#include <seiscomp3/processing/secondarypicker.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API SL2Picker : public SecondaryPicker {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		struct L2Config {
			double      threshold;
			double      minSNR;
			double      margin;
			double      timeCorr;
			std::string filter;
			std::string detecFilter;
		};

		struct State {
			State();
			bool        aicValid;
			double      aicStart;
			double      aicEnd;
			Core::Time  detection;
			Core::Time  pick;
			double      snr;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SL2Picker();

		//! D'tor
		~SL2Picker();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setup(const Settings &settings);
		void setSaveIntermediate(bool);

		const std::string &methodID() const;

		bool setL2Config(const L2Config &l2config);

		//! Returns the current configuration
		const L2Config &l2Config() const { return _l2Config; }

		const State &state() const { return _state; }
		const Result &result() const { return _result; }

		//! Returns detection data from noiseBegin if setSaveIntermediate
		//! has been enabled before processing started.
		const DoubleArray &processedData() const { return _detectionTrace; }

	protected:
		bool applyConfig();
		void fill(size_t n, double *samples);
		void process(const Record *rec, const DoubleArray &filteredData);

	private:
		bool        _initialized;
		L2Config    _l2Config;
		State       _state;
		Result      _result;
		Filter     *_compFilter;
		bool        _saveIntermediate;
		DoubleArray _detectionTrace;
};


}
}

#endif
