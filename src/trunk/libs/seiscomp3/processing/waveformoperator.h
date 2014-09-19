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



#ifndef __SEISCOMP_PROCESSING_WAVEFORMPIPE_H__
#define __SEISCOMP_PROCESSING_WAVEFORMPIPE_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/processing/waveformprocessor.h>

#include <boost/function.hpp>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(WaveformOperator);

//! WaveformPipe declares an interface to modify/manipulate/combine
//! records. It can be used to rotate 3 components or to combine
//! two horizontal components into a single component.
class SC_SYSTEM_CLIENT_API WaveformOperator : public Core::BaseObject {
	public:
		typedef boost::function<bool (const Record *)> StoreFunc;

		WaveformOperator();
		virtual ~WaveformOperator();


	public:
		//! Sets the storage function called when a new records is
		//! available.
		void setStoreFunc(const StoreFunc &func);


		//! Connects the output of op1 with input of op2. Calls setStoreFunc
		//! on op1.
		static void connect(WaveformOperator *op1, WaveformOperator *op2);


		//! Feeds a record. In case a status value larger that Terminated
		//! (which indicates an error) is returned, it is populated
		//! into the WaveformProcessor's status.
		virtual WaveformProcessor::Status feed(const Record *record) = 0;

		//! Resets the operator.
		virtual void reset() = 0;


	protected:
		bool store(const Record *rec) {
			if ( _func ) return _func(rec);
			return false;
		}


	private:
		StoreFunc     _func;
};


}
}


#endif
