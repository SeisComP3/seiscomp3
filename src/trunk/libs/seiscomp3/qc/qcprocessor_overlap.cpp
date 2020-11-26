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


#include <seiscomp3/qc/qcprocessor_overlap.h>


namespace Seiscomp {
namespace Processing {

IMPLEMENT_SC_CLASS_DERIVED(QcProcessorOverlap, QcProcessor, "QcProcessorOverlap");


QcProcessorOverlap::QcProcessorOverlap() : QcProcessor() {}

bool QcProcessorOverlap::setState(const Record *record, const DoubleArray &data) {
	if (_stream.lastRecord && record->samplingFrequency() > 0) {
		try {
			double diff = (double)(record->startTime() - _stream.lastRecord->endTime());

			if (diff < (-0.5 / record->samplingFrequency())) {
				_qcp->parameter = -1.0*diff;
				return true;
			}
		}
		catch (Core::ValueException &) {}
	}

	return false;
}

double QcProcessorOverlap::getOverlap() {
	try {
		return boost::any_cast<double>(_qcp->parameter);
	}
	catch (const boost::bad_any_cast &) {
		throw Core::ValueException("no data");
	}
}

}
}

