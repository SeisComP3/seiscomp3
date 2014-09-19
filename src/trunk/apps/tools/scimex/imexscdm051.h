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

#ifndef __IMEXIMPORTERSCDM051_H___
#define __IMEXIMPORTERSCDM051_H___

#include <seiscomp3/datamodel/exchange/scdm051.h>


class ImexImporterScDm051 : public Seiscomp::DataModel::ImporterSCDM051 {
	public:
		ImexImporterScDm051();

};


class ImexExporterScDm051 : public Seiscomp::DataModel::ExporterSCDM051 {
	public:
		ImexExporterScDm051();
};

#endif
