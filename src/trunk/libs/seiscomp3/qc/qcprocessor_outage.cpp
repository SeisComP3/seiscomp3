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
#include <seiscomp3/qc/qcprocessor_outage.h>


namespace Seiscomp {
namespace Processing {

IMPLEMENT_SC_CLASS_DERIVED(QcProcessorOutage, QcProcessor, "QcProcessorOutage");


QcProcessorOutage::QcProcessorOutage() 
    : QcProcessor(), _threshold(1800) {}

void QcProcessorOutage::setThreshold(int threshold) {
    _threshold = threshold;
}

bool QcProcessorOutage::setState(const Record *record, const DoubleArray &data) {
    if (_stream.lastRecord) {
        try {
            Core::Time lastRecEnd = _stream.lastRecord->endTime();
            Core::Time curRecStart = record->startTime();                      
            double diff = 0.0;

            /* to handle out-of-order records */
            if (_recent < lastRecEnd) {
                diff = (double)(curRecStart - lastRecEnd);
                _recent = lastRecEnd;
            } else {
                SEISCOMP_DEBUG("QcProcessorOutage::setState() for %s.%s.%s.%s -> recent: %s lastRecEnd: %s curRecStart: %s",
                               record->networkCode().c_str(),record->stationCode().c_str(),record->locationCode().c_str(),
                               record->channelCode().c_str(),_recent.iso().c_str(),lastRecEnd.iso().c_str(),curRecStart.iso().c_str());
                if (_recent < curRecStart)
                    diff = (double)(curRecStart - _recent);
            }

            if (diff >= _threshold) {
              _qcp->parameter = diff;
              return true;
            }
        } catch (Core::ValueException) {}
    }

    return false;
}

double QcProcessorOutage::getOutage() throw (Core::ValueException) {
    try {
        return boost::any_cast<double>(_qcp->parameter);
    } catch (const boost::bad_any_cast &) {
        throw Core::ValueException("no data");
    }
}

}
}

