/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/

#ifndef __IPGP_PLUGINS_SEISMOMETER_L4C1HZ_H__
#define __IPGP_PLUGINS_SEISMOMETER_L4C1HZ_H__

#include <seiscomp3/math/filter/seismometers.h>

namespace Seiscomp {
namespace Math {

namespace SeismometerResponse {
class SC_SYSTEM_CORE_API L4C_1Hz : public PolesAndZeros {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit L4C_1Hz(GroundMotion input);
};
} // namespace SeismometerResponse


namespace Filtering {
namespace IIR {

template<typename T>
class SC_SYSTEM_CORE_API L4C_1Hz_Filter : public Filter<T> {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		L4C_1Hz_Filter(GroundMotion input = Velocity);
		L4C_1Hz_Filter(const L4C_1Hz_Filter& other);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		int setParameters(int n, const double* params);
		Math::Filtering::InPlaceFilter<T>* clone() const;
		void setInput(GroundMotion input);
};

} // namespace IIR
} // namespace Filtering

} // namespace Math
} // namespace Seiscomp

#endif
