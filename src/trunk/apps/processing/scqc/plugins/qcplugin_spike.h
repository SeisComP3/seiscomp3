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


#ifndef __SEISCOMP_QC_QCSPIKE_H__
#define __SEISCOMP_QC_QCSPIKE_H__

#include <vector>

#include <seiscomp3/plugins/qc/qcplugin.h>


namespace Seiscomp {
namespace Applications {
namespace Qc {

typedef Processing::QcProcessorSpike::Spikes Spikes;


DEFINE_SMARTPOINTER(QcPluginSpike);

class QcPluginSpike : public QcPlugin {
    DECLARE_SC_CLASS(QcPluginSpike);

public:
    QcPluginSpike();
    std::string registeredName() const;
    std::vector<std::string> parameterNames() const;

private:
    void generateReport(const QcBuffer* reportBuffer) const;
    void generateAlert(const QcBuffer* staBuffer, const QcBuffer* ltaBuffer) const;
    std::vector<double> _mean(const QcBuffer* sta) const;
    std::vector<double> _stdDev(const QcBuffer* sta, double iMean, double aMean) const;
};


}
}
}
#endif
