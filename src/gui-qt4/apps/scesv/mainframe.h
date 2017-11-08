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
#include <seiscomp3/gui/core/mainwindow.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/databasequery.h>
#endif
#include "ui_mainframe.h"
#include <seiscomp3/gui/datamodel/magnitudeview.h>


namespace Seiscomp {

namespace Gui {

class EventSummaryView;
class EventListView;

}

namespace Applications {
namespace EventSummaryView {


class MainFrame : public Gui::MainWindow {
	Q_OBJECT

	public:
		MainFrame();
		~MainFrame();

		Gui::EventSummaryView* eventSummaryView() const;

		void loadEvents(float days);


	protected slots:
		void toggleDock();
		void showLocator();
		void toggledFullScreen(bool);
		void toggleEventList();
		void showInStatusbar(QString text, int time);
		void setLastNonFakeEvent();
		void clearStatusbar();
		void showESVTab();

	private:
		Ui::MainFrame _ui;
		QLabel *_wt;
		Gui::EventSummaryView *_eventSummary;
		Gui::EventListView    *_listPage;
};


}
}
}

#endif
