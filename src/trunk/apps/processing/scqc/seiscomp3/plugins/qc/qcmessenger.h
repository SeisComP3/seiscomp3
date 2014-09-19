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


#ifndef __SEISCOMP_QCMESSENGER_H__
#define __SEISCOMP_QCMESSENGER_H__

#include <string>
#include <list>

#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/plugins/qc/api.h>
#include <seiscomp3/communication/connection.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/datamodel/waveformquality.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/plugins/qc/qcplugin.h>

#include <boost/thread.hpp>

namespace Seiscomp {
namespace Applications {
namespace Qc {

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Communication;
using namespace Seiscomp::DataModel;

SC_QCPLUGIN_API std::string i2s(DataModel::Object* obj);


class SC_QCPLUGIN_API ConfigException: public GeneralException {
	public:
		ConfigException(): GeneralException("config exception") {}
		ConfigException(const string& what): GeneralException(what) {}
};

class SC_QCPLUGIN_API ConnectionException: public GeneralException {
	public:
	ConnectionException(): GeneralException("connection exception") {}
	ConnectionException(const string& what): GeneralException(what) {}
};

class SC_QCPLUGIN_API DatabaseException: public GeneralException {
	public:
	DatabaseException(): GeneralException("database exception") {}
	DatabaseException(const string& what): GeneralException(what) {}
};

class SC_QCPLUGIN_API QcIndex {
	public:
		QcIndex(size_t maxSize=100000) : _maxSize(maxSize) {};

		bool find(const std::string& s) {
			return (_buffer.find(s) != _buffer.end());
		}

		void insert(const std::string& s) {
			_buffer.insert(s);
			if (_buffer.size() > _maxSize)
				_buffer.erase(_buffer.begin());
		}

		size_t size() {
			return _buffer.size();
		}
	
	private:
		std::set<std::string> _buffer;
		size_t _maxSize;
};


DEFINE_SMARTPOINTER(QcMessenger);

class SC_QCPLUGIN_API QcMessenger : public Core::BaseObject {
	DECLARE_SC_CLASS(QcMessenger);

	public:
		//! Default Constructor
		QcMessenger();
		
		//! Initializing Constructor
		QcMessenger(const QcApp* app);
		
		//! Attach object to message and schedule sending 
		//! (if notifier is true send as notifier message; as data message otherwise)
		bool attachObject(DataModel::Object* obj, bool notifier, Operation operation=OP_UNDEFINED);

		//! Scheduler for sending messages (called periodically by application)
		void scheduler();

		//! Send Qc Message
		bool sendMessage(Message* msg) throw (ConnectionException);

		void flushMessages();

	public:
		QcIndex _qcIndex;

	private:
		NotifierMessagePtr _notifierMsg;
		DataMessagePtr _dataMsg;
		const QcApp* _app;
		Core::TimeSpan _sendInterval;
		int _maxSize;
		Util::StopWatch _timer;

};


}
}
}

#endif
