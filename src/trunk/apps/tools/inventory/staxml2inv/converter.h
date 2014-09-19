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


#ifndef __SYNC_STAXML_CONVERTER_H__
#define __SYNC_STAXML_CONVERTER_H__


#include <seiscomp3/core/baseobject.h>


namespace Seiscomp {


class Converter {
	public:
		Converter() : _interrupted(false) {}
		virtual ~Converter() {}

	public:
		virtual void interrupt() { _interrupted = true; }

	protected:
		bool _interrupted;
};


}


#endif
