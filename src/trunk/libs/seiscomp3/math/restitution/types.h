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



#ifndef __SEISCOMP_MATH_RESTITUTION_TYPES_H__
#define __SEISCOMP_MATH_RESTITUTION_TYPES_H__


namespace Seiscomp {
namespace Math {
namespace Restitution {


typedef std::complex<double> Pole;
typedef std::complex<double> Zero;

typedef std::vector<Pole> Poles;
typedef std::vector<Zero> Zeros;


}
}
}


#endif
