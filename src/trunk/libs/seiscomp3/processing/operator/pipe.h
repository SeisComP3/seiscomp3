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



#ifndef __SEISCOMP_PROCESSING_OPERATOR_PIPE_H__
#define __SEISCOMP_PROCESSING_OPERATOR_PIPE_H__


#include <seiscomp3/processing/waveformoperator.h>


namespace Seiscomp {
namespace Processing {


//! A simple wrapper for WaveformOperator::connect. It additionally
//! manages the two connected operators.
class PipeOperator : public WaveformOperator {
	public:
		PipeOperator(WaveformOperator *op1, WaveformOperator *op2);

		WaveformProcessor::Status feed(const Record *record);
		void reset();


	private:
		WaveformOperatorPtr _op1;
		WaveformOperatorPtr _op2;
};


}
}


#endif
