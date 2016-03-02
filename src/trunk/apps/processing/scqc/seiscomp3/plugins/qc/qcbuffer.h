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


#ifndef __SEISCOMP_QC_QCBUFFER_H__
#define __SEISCOMP_QC_QCBUFFER_H__


#include <seiscomp3/qc/qcprocessor.h>
#include <seiscomp3/plugins/qc/api.h>

using namespace Seiscomp::Processing;

namespace Seiscomp {
namespace Applications {
namespace Qc {

typedef std::list<QcParameterCPtr> BufferBase;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DEFINE_SMARTPOINTER(QcBuffer);

class SC_QCPLUGIN_API QcBuffer : public Core::BaseObject, public BufferBase {
	DECLARE_SC_CLASS(QcBuffer);

	public:
		QcBuffer();
		QcBuffer(double maxBufferSize);
		
		mutable Core::Time lastEvalTime;

 		void push_back(const QcParameter* qcp);

		const QcParameter* qcParameter(const Core::Time& time) const;
		const QcBuffer* qcParameter(const Core::Time& startTime, const Core::Time& endTime) const;
		const QcBuffer* qcParameter(const Core::TimeSpan& lastNSeconds) const;

		void info() const;
		void dump() const;
		bool recentlyUsed() const;
		void setRecentlyUsed(bool status);

		const Core::Time& startTime() const;
		const Core::Time& endTime() const;
		Core::TimeSpan length() const;

	protected:
	

	private:
		double _maxBufferSize;
		bool _recentlyUsed;


};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}
}
}


#endif
