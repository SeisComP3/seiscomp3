/***************************************************************************
 *   Copyright (C) by GFZ Potsdam and gempa GmbH                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_APPLICATIONS_EVENTTOOL_CONSTRAINTS_H__
#define __SEISCOMP_APPLICATIONS_EVENTTOOL_CONSTRAINTS_H__

#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/focalmechanism.h>

#include <string>


namespace Seiscomp {

namespace Client {


struct Constraints {
	Constraints() : fixMw(false), fixType(false) {}

	std::string                    preferredMagnitudeType;
	std::string                    preferredOriginID;
	std::string                    preferredFocalMechanismID;
	bool                           fixMw;
	bool                           fixType;
	OPT(DataModel::EvaluationMode) preferredOriginEvaluationMode;
	OPT(DataModel::EvaluationMode) preferredFocalMechanismEvaluationMode;

	bool fixedOrigin() const;
	bool fixOrigin(const DataModel::Origin *org) const;
	bool fixOriginMode(const DataModel::Origin *org) const;

	bool fixedFocalMechanism() const;
	bool fixFocalMechanism(const DataModel::FocalMechanism *fm) const;
	bool fixFocalMechanismMode(const DataModel::FocalMechanism *fm) const;
};


}

}


#endif
