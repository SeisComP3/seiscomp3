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


#include <seiscomp3/qc/qcprocessor_rms.h>
#include "qcplugin_rms.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {

using namespace std;
using namespace Seiscomp::Processing;


#define REGISTERED_NAME "QcRms"

IMPLEMENT_SC_CLASS_DERIVED(QcPluginRms, QcPlugin, "QcPluginRms");
ADD_SC_PLUGIN("Qc Parameter Rms", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 1, 0)
REGISTER_QCPLUGIN(QcPluginRms, REGISTERED_NAME);


QcPluginRms::QcPluginRms(): QcPlugin() {
    _qcProcessor = new QcProcessorRms();
    _qcProcessor->subscribe(this);

    _name = REGISTERED_NAME;
    _parameterNames.push_back("rms");
}

string QcPluginRms::registeredName() const {
    return _name;
}

vector<string> QcPluginRms::parameterNames() const {
    return _parameterNames;
}


}
}
}

