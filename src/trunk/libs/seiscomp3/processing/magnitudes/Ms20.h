/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/



#ifndef __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MS20_H__
#define __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MS20_H__

#include <seiscomp3/processing/magnitudeprocessor.h>


namespace Seiscomp {

namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_ms20 : public MagnitudeProcessor {
	DECLARE_SC_CLASS(MagnitudeProcessor_ms20);

	public:
		MagnitudeProcessor_ms20();

		bool setup(const Settings &settings);

		Status computeMagnitude(double amplitude, const std::string &unit,
		                        double period, double snr,
		                        double delta, double depth,
		                        const DataModel::Origin *hypocenter,
		                        const DataModel::SensorLocation *receiver,
		                        const DataModel::Amplitude *,
		                        double &value);

	private:
		double                  lowPer;
		double                  upPer;
		double                  minDistanceDeg;
		double                  maxDistanceDeg;
		double                  maxDepthKm;
};


}

}


#endif
