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


#include <seiscomp3/qc/qcprocessor_timing.h>
#include "qcplugin_timing.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {

using namespace std;
using namespace Seiscomp::Processing;


#define REGISTERED_NAME "QcTiming"

IMPLEMENT_SC_CLASS_DERIVED(QcPluginTiming, QcPlugin, "QcPluginTiming");
ADD_SC_PLUGIN("Qc Parameter Timing", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 1, 0)
REGISTER_QCPLUGIN(QcPluginTiming, REGISTERED_NAME);


QcPluginTiming::QcPluginTiming(): QcPlugin() {
    _qcProcessor = new QcProcessorTiming();
    _qcProcessor->subscribe(this);

    _name = REGISTERED_NAME;
    _parameterNames.push_back("timing quality");
}

string QcPluginTiming::registeredName() const {
    return _name;
}

vector<string> QcPluginTiming::parameterNames() const {
    return _parameterNames;
}


}
}
}

