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

#include <seiscomp3/plugins/qc/qcbuffer.h>
#include <seiscomp3/qc/qcprocessor_latency.h>
#include "qcplugin_latency.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {


using namespace std;
using namespace Seiscomp::Processing;


#define REGISTERED_NAME "QcLatency"

IMPLEMENT_SC_CLASS_DERIVED(QcPluginLatency, QcPlugin, "QcPluginLatency");
ADD_SC_PLUGIN("Qc Parameter Latency", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 1, 0)
REGISTER_QCPLUGIN(QcPluginLatency, REGISTERED_NAME);


QcPluginLatency::QcPluginLatency(): QcPlugin() {
    _qcProcessor = new QcProcessorLatency();
    _qcProcessor->subscribe(this);

    _lastArrivalTime = Core::Time::GMT();

    _name = REGISTERED_NAME;
    _parameterNames.push_back("latency");
}

string QcPluginLatency::registeredName() const {
    return _name;
}

vector<string> QcPluginLatency::parameterNames() const {
    return _parameterNames;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcPluginLatency::timeoutTask() {

	if (_qcBuffer->empty()) {
		SEISCOMP_WARNING("_qcBuffer->back() is empty");
		return;
	}

	QcParameter* qcp = new QcParameter();
	qcp->recordSamplingFrequency = -1;
	qcp->recordEndTime = Core::Time::GMT();
	
	// origin of previous buffer item was a real record
	if (_qcBuffer->back()->recordSamplingFrequency != -1) {
		_lastArrivalTime = _qcBuffer->back()->recordEndTime;
	}

	qcp->recordStartTime = _lastArrivalTime;
	qcp->parameter = (double)(qcp->recordEndTime - qcp->recordStartTime);
	_qcBuffer->push_back(qcp);

	Core::Time t;
	sendMessages(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}

