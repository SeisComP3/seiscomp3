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


#define SEISCOMP_COMPONENT SCQC

#include <seiscomp3/logging/log.h>
#include <seiscomp3/qc/qcprocessor_gap.h>


namespace Seiscomp {
namespace Processing {

IMPLEMENT_SC_CLASS_DERIVED(QcProcessorGap, QcProcessor, "QcProcessorGap");


QcProcessorGap::QcProcessorGap() 
    : QcProcessor() {}

bool QcProcessorGap::setState(const Record *record, const DoubleArray &data) {
    if (_stream.lastRecord && record->samplingFrequency() > 0) {
        try {
            double diff = (double)(record->startTime() - _stream.lastRecord->endTime());

            if (diff >= (0.5 / record->samplingFrequency())) {
                _qcp->parameter = diff;
                return true;
            }
        } catch (Core::ValueException) {}
    }
    return false;
}

double QcProcessorGap::getGap() throw(Core::ValueException) {
    try {
        return boost::any_cast<double>(_qcp->parameter);
    } catch (const boost::bad_any_cast &) {
        throw Core::ValueException("no data");
    }
}

}
}

