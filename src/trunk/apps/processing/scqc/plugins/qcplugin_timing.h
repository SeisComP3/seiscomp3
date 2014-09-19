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


#ifndef __SEISCOMP_QC_QCTIMING_H__
#define __SEISCOMP_QC_QCTIMING_H__

#include <seiscomp3/plugins/qc/qcplugin.h>


namespace Seiscomp {
namespace Applications {
namespace Qc {


DEFINE_SMARTPOINTER(QcPluginTiming);

class QcPluginTiming : public QcPlugin {
    DECLARE_SC_CLASS(QcPluginTiming);

public:
    QcPluginTiming();
    std::string registeredName() const;
    std::vector<std::string> parameterNames() const;
};



}
}
}
#endif
