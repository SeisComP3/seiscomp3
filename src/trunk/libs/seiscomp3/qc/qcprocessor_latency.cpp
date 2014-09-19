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


#include <seiscomp3/qc/qcprocessor_latency.h>

#define SEISCOMP_COMPONENT SCQC
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace Processing {

	
IMPLEMENT_SC_CLASS_DERIVED(QcProcessorLatency, QcProcessor, "QcProcessorLatency");


QcProcessorLatency::QcProcessorLatency() 
    : QcProcessor() {
	
	_lastRecordArrivalTime = Core::Time::GMT();

}




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcProcessorLatency::setState(const Record *record, const DoubleArray &data) {

	Core::Time now = Core::Time::GMT();

	_qcp->recordStartTime = now;
	_qcp->recordEndTime = now;
	_qcp->parameter = (double)(now - _lastRecordArrivalTime);
	_lastRecordArrivalTime = now;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double QcProcessorLatency::getLatency() throw(Core::ValueException) {

	try {
		return boost::any_cast<double>(_qcp->parameter);
	}
	catch (const boost::bad_any_cast &) {
		throw Core::ValueException("no data");
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
