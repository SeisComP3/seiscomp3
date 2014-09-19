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



#include <seiscomp3/processing/operator/pipe.h>


namespace Seiscomp {
namespace Processing {


PipeOperator::PipeOperator(WaveformOperator *op1, WaveformOperator *op2)
: _op1(op1), _op2(op2) {
	WaveformOperator::connect(op1, op2);
}


WaveformProcessor::Status PipeOperator::feed(const Record *rec) {
	return _op1->feed(rec);
}


void PipeOperator::reset() {
	if ( _op1 ) _op1->reset();
	if ( _op2 ) _op2->reset();
}


}
}
