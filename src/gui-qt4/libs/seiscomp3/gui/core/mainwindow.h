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



#ifndef __SEISCOMP_GUI_MAINWINDOW_H__
#define __SEISCOMP_GUI_MAINWINDOW_H__

#include <QtGui>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/gui/core/connectionstatelabel.h>


namespace Seiscomp {

namespace Core {

DEFINE_SMARTPOINTER(Message);

}

namespace Communication {

DEFINE_SMARTPOINTER(Connection);

}

namespace IO {

DEFINE_SMARTPOINTER(DatabaseInterface);

}

namespace Logging {

class FileOutput;

}

namespace DataModel {

DEFINE_SMARTPOINTER(DatabaseQuery);
DEFINE_SMARTPOINTER(Network);
DEFINE_SMARTPOINTER(Station);
DEFINE_SMARTPOINTER(Notifier);
DEFINE_SMARTPOINTER(Object);

}


namespace Gui {


class SC_GUI_API MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow(QWidget * parent = 0, Qt::WFlags = 0);
		~MainWindow();


	public:
		bool restoreGeometry(const QByteArray & geometry);


	protected:
		void paintEvent(QPaintEvent *e);
		void showEvent(QShowEvent*);
		void dropEvent(QDropEvent *);
		void dragEnterEvent(QDragEnterEvent *);

		virtual void toggledFullScreen(bool);

	signals:
		void fullScreenToggled(bool);

	public slots:
		void showNormal();
		void showFullScreen();

		void setTitle(QString);
		QString title() const;


	private slots:
		void onChangedConnection();
		void toggleFullScreen();

		void connectionEstablished();
		void connectionLost();

		void inspectConfig();
		void inspectInventory();


	protected:
		QAction* _actionToggleFullScreen;
		QAction* _actionShowSettings;


	private:
		QMenuBar *_menuBar;
		QWidget *_menuWidget;
		ConnectionStateLabel *_connectionState;
		QString _title;
		bool _showFullscreen;
};


}
}


#endif

