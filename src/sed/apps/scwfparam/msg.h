/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/

#ifndef __SEISCOMP_APPLICATIONS_WFPARAM_MSG_H__
#define __SEISCOMP_APPLICATIONS_WFPARAM_MSG_H__


#include "util.h"
#include <seiscomp3/communication/connection.h>


bool sendMessages(Seiscomp::Communication::Connection *con,
                  Seiscomp::DataModel::Event *evt,
                  Seiscomp::DataModel::Origin *org,
                  Seiscomp::DataModel::Magnitude *mag,
                  const Seiscomp::StationMap &results);


#endif
