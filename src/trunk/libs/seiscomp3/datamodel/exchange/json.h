/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   -------------------------------------------------------------------   *
 *   Author: Jan Becker <jabe@gempa.de>                                    *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_DATAMODEL_SCDM_JSON_EXCHANGE_H__
#define __SEISCOMP_DATAMODEL_SCDM_JSON_EXCHANGE_H__


#include <seiscomp3/io/importer.h>
#include <seiscomp3/io/exporter.h>


namespace Seiscomp {
namespace DataModel {


class ImporterJSON : public IO::Importer {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ImporterJSON();


	// ------------------------------------------------------------------
	//  Importer interface
	// ------------------------------------------------------------------
	protected:
		Core::BaseObject *get(std::streambuf* buf);
};


class ExporterJSON : public IO::Exporter {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ExporterJSON();


	// ------------------------------------------------------------------
	//  Exporter interface
	// ------------------------------------------------------------------
	protected:
		bool put(std::streambuf* buf, Core::BaseObject *);
};


}
}


#endif
