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



#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/origin.h>

bool dumpOrigin(const Seiscomp::DataModel::Origin *origin);

bool equivalent(const Seiscomp::DataModel::WaveformStreamID&, const Seiscomp::DataModel::WaveformStreamID&);

double arrivalWeight(const Seiscomp::DataModel::ArrivalPtr *arr, double defaultWeight=1.);
char getShortPhaseName(const std::string &phase);

bool validArrival(const Seiscomp::DataModel::Arrival *arr, double minWeight = 0.5);

Seiscomp::DataModel::EvaluationStatus
status(const Seiscomp::DataModel::Origin *origin);

