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


#ifndef __SEISCOMP_DATAMODEL_SCDM_TRUNK_EXCHANGE_H__
#define __SEISCOMP_DATAMODEL_SCDM_TRUNK_EXCHANGE_H__


#include <seiscomp3/io/importer.h>
#include <seiscomp3/io/exporter.h>


namespace Seiscomp {
namespace DataModel {


class ImporterTrunk : public IO::Importer {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ImporterTrunk();


	// ------------------------------------------------------------------
	//  Importer interface
	// ------------------------------------------------------------------
	protected:
		Core::BaseObject *get(std::streambuf* buf);
};


class ExporterTrunk : public IO::Exporter {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ExporterTrunk();


	// ------------------------------------------------------------------
	//  Exporter interface
	// ------------------------------------------------------------------
	protected:
		bool put(std::streambuf* buf, Core::BaseObject *);
		bool put(std::streambuf* buf, const IO::ExportObjectList &objects);
};


}
}


#endif
