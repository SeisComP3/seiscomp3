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


#ifndef __SEISCOMP_DATAMODEL_SCDM_IMS10_EXCHANGE_H__
#define __SEISCOMP_DATAMODEL_SCDM_IMS10_EXCHANGE_H__


#include <seiscomp3/io/exporter.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/stationmagnitudecontribution.h>


namespace Seiscomp {
namespace DataModel {


class ExporterIMS10 : public IO::Exporter {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ExporterIMS10();

	// ------------------------------------------------------------------
	//  Exporter interface
	// ------------------------------------------------------------------
	protected:
		bool put(std::streambuf* buf, Core::BaseObject *);
	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::vector<Seiscomp::DataModel::ArrivalPtr> _arrivalList;
		std::vector<Seiscomp::DataModel::PickPtr> _pickList;
		std::vector<Seiscomp::DataModel::AmplitudePtr> _amplitudeList;
		std::vector<Seiscomp::DataModel::StationMagnitudePtr> _stationMagnitudeList;
};


}
}


#endif
