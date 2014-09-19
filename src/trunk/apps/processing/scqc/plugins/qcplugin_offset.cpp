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

#include <seiscomp3/qc/qcprocessor_mean.h>
#include "qcplugin_offset.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {


using namespace std;
using namespace Seiscomp::Processing;


#define REGISTERED_NAME "QcOffset"

IMPLEMENT_SC_CLASS_DERIVED(QcPluginOffset, QcPlugin, "QcPluginOffset");
ADD_SC_PLUGIN("Qc Parameter Offset", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 1, 0)
REGISTER_QCPLUGIN(QcPluginOffset, REGISTERED_NAME);


QcPluginOffset::QcPluginOffset(): QcPlugin() {
    _qcProcessor = new QcProcessorMean();
    _qcProcessor->subscribe(this);

    _name = REGISTERED_NAME;
    _parameterNames.push_back("offset");
}

string QcPluginOffset::registeredName() const {
    return _name;
}

vector<string> QcPluginOffset::parameterNames() const {
    return _parameterNames;
}

}
}
}

