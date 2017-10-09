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


#include <seiscomp3/qc/qcprocessor_delay.h>


namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(QcProcessorDelay, QcProcessor, "QcProcessorDelay");

QcProcessorDelay::QcProcessorDelay() : QcProcessor() {}

double QcProcessorDelay::getDelay() {
	try {
		return boost::any_cast<double>(_qcp->parameter);
	}
	catch ( const boost::bad_any_cast & ) {
		throw Core::ValueException("no data");
	}
}

bool QcProcessorDelay::setState(const Record *record, const DoubleArray &data) {
	try {
		//! (mean) data delay time calculated at time of arrival
		//! calculate *mean* delay time, valid for all samples of this record
		//_qcp->parameter = (double)(Core::Time::GMT() - record->endTime()) + 0.5 * (double)(record->endTime() - record->startTime());

		//! calculate delay time based on time of last sample
		_qcp->parameter = (double)(Core::Time::GMT() - record->endTime());

		return true;
	}
	catch (Core::ValueException) {}

	return false;
}


}
}
