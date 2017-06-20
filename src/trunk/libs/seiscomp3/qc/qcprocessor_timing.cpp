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


// #ifdef HAVE_MSEED
#include <seiscomp3/io/records/mseedrecord.h>
// #endif

#include <seiscomp3/qc/qcprocessor_timing.h>


namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(QcProcessorTiming, QcProcessor, "QcProcessorTiming");


QcProcessorTiming::QcProcessorTiming() : QcProcessor() {}

bool QcProcessorTiming::setState(const Record *record, const DoubleArray &data) {
	const IO::MSeedRecord *mrec = IO::MSeedRecord::ConstCast(record);

	if ( mrec != NULL ) {
		if ((double)mrec->timingQuality() != -1) {
			_qcp->parameter = (double)mrec->timingQuality();
			return true;
		}
	}

	return false;
}

double QcProcessorTiming::getTiming() {
	try {
		return boost::any_cast<double>(_qcp->parameter);
	}
	catch (const boost::bad_any_cast &) {
		throw Core::ValueException("no data");
	}
}


}
}
