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


#ifndef __SEISCOMP_QC_QCOUTAGE_H__
#define __SEISCOMP_QC_QCOUTAGE_H__

#include <map>

#include <seiscomp3/qc/qcprocessor.h>
#include <seiscomp3/plugins/qc/qcplugin.h>


namespace Seiscomp {
namespace Applications {
namespace Qc {


DEFINE_SMARTPOINTER(QcPluginOutage);

class QcPluginOutage : public QcPlugin {
    DECLARE_SC_CLASS(QcPluginOutage);

public:
    QcPluginOutage();
    bool init(QcApp* app, QcConfig *cfg, std::string streamID);
    std::string registeredName() const;
    std::vector<std::string> parameterNames() const;
    void update();

private:
    std::map<std::string, Core::Time> _recent;

    bool fillUp(const Processing::QcParameter *qcp);
};

}
}
}
#endif
