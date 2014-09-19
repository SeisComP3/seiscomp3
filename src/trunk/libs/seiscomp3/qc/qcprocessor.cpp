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


#include <seiscomp3/qc/qcprocessor.h>

namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_ABSTRACT_CLASS(QcProcessorObserver, "QcProcessorObserver");
QcProcessorObserver::QcProcessorObserver() {}


IMPLEMENT_SC_CLASS(QcParameter, "QcParameter");
IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(QcProcessor, WaveformProcessor, "QcProcessor");

// QcParameter::QcParameter() 
//     : recordSampleCount(0), recordSamplingFrequency(0) {}

// Core::Time QcParameter::recordEndTime() throw (Core::ValueException) {
//     if (recordStartTime != Core::Time() && recordSampleCount && recordSamplingFrequency > 0.0) {
//         return(recordStartTime + Core::Time(recordSampleCount / recordSamplingFrequency));
//     }

//     throw (Core::ValueException);
// }

QcProcessor::QcProcessor(const Core::TimeSpan &deadTime,
						const Core::TimeSpan &gapThreshold) 
	: WaveformProcessor(deadTime, gapThreshold),
	_setFlag(false), _validFlag(false) {}

QcProcessor::~QcProcessor() {}




bool QcProcessor::subscribe(QcProcessorObserver *obs) {
	std::deque<QcProcessorObserver *>::iterator it = std::find(_observers.begin(),_observers.end(),obs);
	
	if (it != _observers.end())
		return false;
	
	_observers.push_back(obs);
	return true;
}




bool QcProcessor::unsubscribe(QcProcessorObserver *obs) {
	std::deque<QcProcessorObserver *>::iterator it = std::find(_observers.begin(),_observers.end(),obs);
	
	if (it == _observers.end())
		return false;

	_observers.erase(it);
	return true;
}




QcParameter* QcProcessor::getState() const {
	return _qcp.get();
}




void QcProcessor::process(const Record *record, const DoubleArray &data) {
	if (!record) return;

	_qcp = new QcParameter;
	_setFlag = false;    

	if ((record->samplingFrequency() > 0.0) && (data.size() > 0)) {
		_qcp->recordStartTime = record->startTime();
		_qcp->recordEndTime = record->endTime();
		_qcp->recordSamplingFrequency = record->samplingFrequency();

		_setFlag = true;
		_validFlag = setState(record,data);
	} 
	
	for (std::deque<QcProcessorObserver *>::iterator it = _observers.begin(); it != _observers.end(); ++it)
		(*it)->update();
}




bool QcProcessor::isSet() const {
	return _setFlag;
}




bool QcProcessor::isValid() const {
	return _validFlag;
}



}
}
