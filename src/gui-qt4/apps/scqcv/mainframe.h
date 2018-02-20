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




#ifndef __MAINFRAME_H__
#define __MAINFRAME_H__

#include <QtGui>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/datamodel/configstation.h>
#endif
#include <seiscomp3/gui/core/mainwindow.h>
#include <seiscomp3/gui/core/messages.h>

#include <utility>

#ifndef Q_MOC_RUN
#include "qcview.h"
#endif

#include "ui_mainframe.h"


namespace Seiscomp {

namespace Applications {
namespace Qc {

class QcConfig;
class QcPlugin;

DEFINE_SMARTPOINTER(QcPlugin);
typedef std::vector<QcPluginPtr> QcPlugins;

}}

using namespace Seiscomp::Applications::Qc;

namespace Gui {


class MainFrame : public MainWindow {
	Q_OBJECT

	public:
		MainFrame();
		~MainFrame();

	protected slots:
		void toggleDock();
		void toggledFullScreen(bool);
		std::list<std::pair<std::string, bool> > streams();

	public slots:
		void readMessage(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*);
		bool sendStationState(QString, bool);

	private:
		void prepareNotifier(QString streamID, bool enable);
		DataModel::ConfigStation *configStation(const std::string& net, const std::string& sta) const;

	private:
		Ui::MainFrame  _ui;
		QLabel        *_wt;

		QcViewConfig  *_config;
		QcModel       *_qcModel;
		QcView        *_qcReportView;
		QcView        *_qcOverView;
};


}
}

#endif
