/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <seiscomp3/utils/units.h>


namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
UnitConverter::ConversionMap UnitConverter::_conversionMap;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define REGISTER_CONVERSION(input, output, outputQML, outputSEED, scale)\
	_conversionMap[input] = UnitConversion(input, output, outputQML, outputSEED, scale)

const UnitConversion *UnitConverter::get(const std::string &fromUnit) {
	if ( _conversionMap.empty() ) {
		#define MTS_SI_UNIT "M*S"
		#define MTS_QML_UNIT "m*s"
		#define MTS_SEED_UNIT "M*S"

		REGISTER_CONVERSION("m*s", MTS_SI_UNIT, MTS_QML_UNIT, MTS_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("M*S", MTS_SI_UNIT, MTS_QML_UNIT, MTS_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("dm*s", MTS_SI_UNIT, MTS_QML_UNIT, MTS_SEED_UNIT, 1E-1);
		REGISTER_CONVERSION("cm*s", MTS_SI_UNIT, MTS_QML_UNIT, MTS_SEED_UNIT, 1E-2);
		REGISTER_CONVERSION("mm*s", MTS_SI_UNIT, MTS_QML_UNIT, MTS_SEED_UNIT, 1E-3);
		REGISTER_CONVERSION("um*s", MTS_SI_UNIT, MTS_QML_UNIT, MTS_SEED_UNIT, 1E-6);
		REGISTER_CONVERSION("nm*s", MTS_SI_UNIT, MTS_QML_UNIT, MTS_SEED_UNIT, 1E-9);

		#define M_SI_UNIT "M"
		#define M_QML_UNIT "m"
		#define M_SEED_UNIT "M"
		REGISTER_CONVERSION("m",  M_SI_UNIT, M_QML_UNIT, M_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("M",  M_SI_UNIT, M_QML_UNIT, M_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("dm", M_SI_UNIT, M_QML_UNIT, M_SEED_UNIT, 1E-1);
		REGISTER_CONVERSION("cm", M_SI_UNIT, M_QML_UNIT, M_SEED_UNIT, 1E-2);
		REGISTER_CONVERSION("mm", M_SI_UNIT, M_QML_UNIT, M_SEED_UNIT, 1E-3);
		REGISTER_CONVERSION("um", M_SI_UNIT, M_QML_UNIT, M_SEED_UNIT, 1E-6);
		REGISTER_CONVERSION("nm", M_SI_UNIT, M_QML_UNIT, M_SEED_UNIT, 1E-9);

		#define M_S_SI_UNIT "M/S"
		#define M_S_QML_UNIT "m/s"
		#define M_S_SEED_UNIT "M/S"
		REGISTER_CONVERSION("m/s",  M_S_SI_UNIT, M_S_QML_UNIT, M_S_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("M/S",  M_S_SI_UNIT, M_S_QML_UNIT, M_S_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("dm/s", M_S_SI_UNIT, M_S_QML_UNIT, M_S_SEED_UNIT, 1E-1);
		REGISTER_CONVERSION("cm/s", M_S_SI_UNIT, M_S_QML_UNIT, M_S_SEED_UNIT, 1E-2);
		REGISTER_CONVERSION("mm/s", M_S_SI_UNIT, M_S_QML_UNIT, M_S_SEED_UNIT, 1E-3);
		REGISTER_CONVERSION("um/s", M_S_SI_UNIT, M_S_QML_UNIT, M_S_SEED_UNIT, 1E-6);
		REGISTER_CONVERSION("nm/s", M_S_SI_UNIT, M_S_QML_UNIT, M_S_SEED_UNIT, 1E-9);

	#define M_S2_SI_UNIT "M/S**2"
	#define M_S2_QML_UNIT "m/(s*s)"
	#define M_S2_SEED_UNIT "M/S**2"
		REGISTER_CONVERSION("m/s**2",    M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("M/S**2",    M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("m/(s*s)",   M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("M/(S**S)",  M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("m/s/s",     M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("M/S/S",     M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1.0);
		REGISTER_CONVERSION("dm/s**2",   M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-1);
		REGISTER_CONVERSION("dm/(s**s)", M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-1);
		REGISTER_CONVERSION("dm/s/s",    M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-1);
		REGISTER_CONVERSION("cm/s**2",   M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-2);
		REGISTER_CONVERSION("cm/(s**s)", M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-2);
		REGISTER_CONVERSION("cm/s/s",    M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-2);
		REGISTER_CONVERSION("mm/s**2",   M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-3);
		REGISTER_CONVERSION("mm/(s**s)", M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-3);
		REGISTER_CONVERSION("mm/s/s",    M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-3);
		REGISTER_CONVERSION("um/s**2",   M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-6);
		REGISTER_CONVERSION("um/(s**s)", M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-6);
		REGISTER_CONVERSION("um/s/s",    M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-6);
		REGISTER_CONVERSION("nm/s**2",   M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-9);
		REGISTER_CONVERSION("nm/(s**s)", M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-9);
		REGISTER_CONVERSION("nm/s/s",    M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 1E-9);
		REGISTER_CONVERSION("g",         M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 9.81);
		REGISTER_CONVERSION("%g",        M_S2_SI_UNIT, M_S2_QML_UNIT, M_S2_SEED_UNIT, 9.81E-2);
	}

	ConversionMap::const_iterator it = _conversionMap.find(fromUnit);
	if ( it == _conversionMap.end() )
		return NULL;

	return &it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
