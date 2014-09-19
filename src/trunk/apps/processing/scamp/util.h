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




#ifndef __SEISCOMP_APPLICATIONS_AMPTOOL_UTIL_H__
#define __SEISCOMP_APPLICATIONS_AMPTOOL_UTIL_H__


#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/datamodel/waveformstreamid.h>


namespace Seiscomp {

namespace DataModel {

class Origin;
class Arrival;
class Amplitude;
class WaveformStreamID;

}

namespace Private {

bool equivalent(const DataModel::WaveformStreamID&, const DataModel::WaveformStreamID&);

double
arrivalWeight(const DataModel::Arrival *arr, double defaultWeight=1.);

double
arrivalDistance(const DataModel::Arrival *arr);

DataModel::EvaluationStatus
status(const DataModel::Origin *origin);

char
shortPhaseName(const std::string &phase);

std::string
toStreamID(const DataModel::WaveformStreamID &wfid);

DataModel::WaveformStreamID
setStreamComponent(const DataModel::WaveformStreamID &wfid, char comp);

std::ostream &
operator<<(std::ostream &os, const DataModel::Amplitude &ampl);


}
}


#endif

