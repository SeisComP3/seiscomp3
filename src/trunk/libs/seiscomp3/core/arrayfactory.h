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


#ifndef __SC_CORE_ARRAYFACTORY_H__
#define __SC_CORE_ARRAYFACTORY_H__

#include <seiscomp3/core/array.h>


namespace Seiscomp {

/**
 * Factory class for the different array classes.
 */
class SC_SYSTEM_CORE_API ArrayFactory {
 public:
	//! Creates an array object specified by the given data type
	static Array* Create(Array::DataType toCreate, Array::DataType caller, int size, const void *data);
	static Array* Create(Array::DataType toCreate, const Array *source);
};

}

#endif
