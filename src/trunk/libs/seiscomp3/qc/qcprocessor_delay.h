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


#ifndef __SEISCOMP_PROCESSING_QCPROCESSORDELAY_H__
#define __SEISCOMP_PROCESSING_QCPROCESSORDELAY_H__


#include <seiscomp3/qc/qcprocessor.h>


namespace Seiscomp {
namespace Processing {


//!  we can also calculate this from data-latency ...
//!  delay = record-arrival time  minus   record-data (mean|end) time


DEFINE_SMARTPOINTER(QcProcessorDelay);

class SC_SYSTEM_CLIENT_API QcProcessorDelay : public QcProcessor {
	DECLARE_SC_CLASS(QcProcessorDelay);

	public:
		QcProcessorDelay();

		double getDelay();
		bool setState(const Record* record, const DoubleArray& data);
};


}
}

#endif
