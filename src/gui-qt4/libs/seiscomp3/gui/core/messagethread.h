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



#ifndef __SEISCOMP_GUI_MESSAGETHREAD_H__
#define __SEISCOMP_GUI_MESSAGETHREAD_H__

#include <seiscomp3/gui/qt4.h>

#include <QtGui>


namespace Seiscomp {

namespace Communication {
class Connection;
}

namespace Gui {


class SC_GUI_API MessageThread : public QThread {
	Q_OBJECT

	public:
		MessageThread(Seiscomp::Communication::Connection* c);
		~MessageThread();

		void run();

		Seiscomp::Communication::Connection* connection() const;
		void setReconnectOnErrorEnabled(bool e);


	signals:
		void messagesAvailable();
		void connectionLost();
		void connectionEstablished();
		void connectionError(int code);


	private:
		bool _reconnectOnError;
		Seiscomp::Communication::Connection *_connection;
};


}
}

#endif
