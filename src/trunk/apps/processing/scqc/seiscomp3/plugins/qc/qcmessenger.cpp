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


#define SEISCOMP_COMPONENT SCQC
#include <seiscomp3/logging/log.h>

#include <seiscomp3/core/system.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/communication/connection.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/datamodel/outage.h>

#include <boost/visit_each.hpp>
#include <boost/bind.hpp>

#include "qcmessenger.h"

#ifndef WIN32
#include <unistd.h>
#endif


namespace Seiscomp {
namespace Applications {
namespace Qc {

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Communication;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::IO;

const char *const myGroup = "Qc";
const char *const myPackage = "QualityControl";


//! converts Object's Index to String
// poorly conceived, this ... looking for a more general solution.
std::string i2s (DataModel::Object* obj) {
	std::string index = "";
	if (!obj) return index;

	DataModel::WaveformQuality* wfq = DataModel::WaveformQuality::Cast(obj);
	if (wfq) {
		const WaveformQualityIndex& ix = wfq->index();
		index = ix.start.iso()+
				ix.waveformID.networkCode()+
				ix.waveformID.stationCode()+
				ix.waveformID.locationCode()+
				ix.waveformID.channelCode()+
				ix.type+
				ix.parameter;
	}

	DataModel::Outage* outage = DataModel::Outage::Cast(obj);
	if (outage) {
		const OutageIndex& ix = outage->index();
		index = ix.start.iso()+
				ix.waveformID.networkCode()+
				ix.waveformID.stationCode()+
				ix.waveformID.locationCode()+
				ix.waveformID.channelCode();
	}

	return index;
}




IMPLEMENT_SC_CLASS(QcMessenger, "QcMessenger");

QcMessenger::QcMessenger(){}

QcMessenger::QcMessenger(const QcApp* app)
: _notifierMsg(NULL), _dataMsg(NULL), _app(app) {

	_sendInterval = 1.0;
	_maxSize = 500;

	QcApp::TimerSignal::slot_type slot = boost::bind(&QcMessenger::scheduler, this);
	_app->addTimeout(slot);
}




//! attach object to message and schedule sending
bool QcMessenger::attachObject(DataModel::Object* obj, bool notifier, Operation operation) {
	//! send notifier msg
	if (notifier) {
		if (operation == OP_UNDEFINED) {
			if (_qcIndex.find(i2s(obj))) {
				cerr << _qcIndex.size() << "   found QcIndex: " << i2s(obj) << endl; //! DEBUG
				operation = OP_UPDATE;
			}
			else {
				operation = OP_ADD;
				_qcIndex.insert(i2s(obj));
			}
		}

		if ( !_notifierMsg ) _notifierMsg = new NotifierMessage;
		NotifierPtr notifier = new Notifier(myPackage, operation, obj);
		_notifierMsg->attach(notifier);

	}
	//! send data msg
	else {
		if ( !_dataMsg ) _dataMsg = new DataMessage;
		_dataMsg->attach(obj);
	}

	//! let scheduler decide, when to send the message
	scheduler();

	return true;
}




//! scheduler: send Qc messages every '_sendInterval' seconds or
//! if attachment count reaches limit of '_maxSize'
//!
void QcMessenger::scheduler(){

	bool msgSend = false;

	if (_notifierMsg) {
		try {
			if (((_timer.elapsed() > _sendInterval) && (_notifierMsg->size() > 0)) || (_notifierMsg->size() >=_maxSize)) {
// 				SEISCOMP_DEBUG("sending Qc NOTIFIER message with %d attachments", _notifierMsg->size());
				sendMessage((Message*)_notifierMsg.get());
				msgSend = true;
			}
		}
		catch(...){ //FIXME error handling
			if (_notifierMsg->size() > 2000) {
				_notifierMsg->clear();
				SEISCOMP_ERROR("Notifier message buffer overflow! Buffer cleared!");
			}
		}
	}

	if (_dataMsg) {
		try {
			if (((_timer.elapsed() > _sendInterval) && (_dataMsg->size() > 0)) || (_dataMsg->size() >=_maxSize)) {
// 				SEISCOMP_DEBUG("sending Qc DATA message with %d attachments", _dataMsg->size());
				sendMessage((Message*)_dataMsg.get());
				msgSend = true;
			}
		}
		catch(...){ //FIXME error handling
			if (_dataMsg->size() > 2000) {
				_dataMsg->clear();
				SEISCOMP_ERROR("Data message buffer overflow! Buffer cleared!");
			}
		}
	}

	if (msgSend)
		_timer.restart();

}




void QcMessenger::flushMessages(){

	Core::TimeSpan tmp = _sendInterval;
	_sendInterval = -1.0;
	scheduler();
	_sendInterval = tmp;

}





//! sending the message
bool QcMessenger::sendMessage(Message* msg) throw (ConnectionException) {
	Communication::Connection *con;

	try { con = _app->connection(); }
	catch (...) {
		throw ConnectionException("Qc msg connection() error");
		return false;
	};

	if ( msg && msg->size() > 0 ) {
		if ( !con->isConnected() )
			if ( con->reconnect() != Status::SEISCOMP_SUCCESS )
				throw ConnectionException("Could not send Qc message -> reconnection failed");

		if ( !con->send(msg) )
			throw ConnectionException("Could not send Qc message");

		msg->clear();
		return true;
	}

	return false;
}


}
}
}

