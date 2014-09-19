/***************************************************************************
 *   Copyright (C) 2006 by GFZ Potsdam
 *
 *   $Author: jabe $
 *   $Email: geofon_devel@gfz-potsdam.de $
 *   $Date: 2007-01-17 16:43:50 +0100 (Wed, 17 Jan 2007) $
 *   $Revision: 683 $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_TYPES_H__
#define __SEISCOMP_STATIONXML_TYPES_H__


#include <seiscomp3/core/enumeration.h>


namespace Seiscomp {
namespace StationXML {



MAKEENUM(
	OutputType,
	EVALUES(
		OT_TRIGGERED,
		OT_CONTINUOUS,
		OT_HEALTH,
		OT_GEOPHYSICAL,
		OT_WEATHER,
		OT_FLAG,
		OT_SYNTHESIZED,
		OT_INPUT,
		OT_EXPERIMENTAL,
		OT_MAINTENANCE,
		OT_BEAM
	),
	ENAMES(
		"TRIGGERED",
		"CONTINUOUS",
		"HEALTH",
		"GEOPHYSICAL",
		"WEATHER",
		"FLAG",
		"SYNTHESIZED",
		"INPUT",
		"EXPERIMENTAL",
		"MAINTENANCE",
		"BEAM"
	)
);


MAKEENUM(
	StageType,
	EVALUES(
		ST_LAPLACE,
		ST_HERTZ,
		ST_COMPOSITE,
		ST_DIGITAL
	),
	ENAMES(
		"LAPLACE",
		"HERTZ",
		"COMPOSITE",
		"DIGITAL"
	)
);


MAKEENUM(
	SymmetryType,
	EVALUES(
		ST_NONE,
		ST_EVEN,
		ST_ODD
	),
	ENAMES(
		"NONE",
		"EVEN",
		"ODD"
	)
);


MAKEENUM(
	NominalType,
	EVALUES(
		NT_NOMINAL,
		NT_CALCULATED
	),
	ENAMES(
		"NOMINAL",
		"CALCULATED"
	)
);


MAKEENUM(
	PzTransferFunctionType,
	EVALUES(
		PZTFT_LAPLACE_RAD,
		PZTFT_LAPLACE_HZ,
		PZTFT_DIGITAL_Z_TRANSFORM
	),
	ENAMES(
		"LAPLACE (RAD/SEC)",
		"LAPLACE (HZ)",
		"DIGITAL (Z-TRANSFORM)"
	)
);


MAKEENUM(
	CfTransferFunctionType,
	EVALUES(
		CFTFT_ANALOG_RAD,
		CFTFT_ANALOG_HZ,
		CFTFT_DIGITAL
	),
	ENAMES(
		"ANALOG (RAD/SEC)",
		"ANALOG (HZ)",
		"DIGITAL"
	)
);


MAKEENUM(
	ApproximationType,
	EVALUES(
		AT_MACLAURIN
	),
	ENAMES(
		"MACLAURIN"
	)
);



}
}


#endif
