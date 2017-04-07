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
 *                                                                         *
 *   Author: Jan Becker, gempa GmbH                                        *
 ***************************************************************************/


#ifndef __SEISCOMP_MATH_WINDOWS_HAMMING_H__
#define __SEISCOMP_MATH_WINDOWS_HAMMING_H__


#include "../windowfunc.h"


namespace Seiscomp {
namespace Math {


template <typename TYPE>
class SC_SYSTEM_CORE_API HammingWindow : public WindowFunc<TYPE> {
	protected:
		virtual void process(int n, TYPE *inout, double width = 0.5) const;
};


}
}


#endif
