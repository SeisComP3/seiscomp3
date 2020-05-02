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



#define SEISCOMP_COMPONENT Gui::MainWindow

#include "mainwindow.h"

#include <seiscomp3/core/platform/platform.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/inspector.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/communication/connection.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/client/configdb.h>
#include <seiscomp3/client/inventory.h>

#ifdef MACOSX
#include <seiscomp3/gui/core/osx.h>
#endif

#include <QMenuBar>
#include <QStatusBar>

using namespace Seiscomp;
using namespace Seiscomp::Communication;
using namespace Seiscomp::DataModel;


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainWindow::MainWindow(QWidget * parent, Qt::WindowFlags flags)
 : QMainWindow(parent, flags)
{
	_connectionState = NULL;
	_title = SCApp->name().c_str();
	_showFullscreen = false;

	setWindowTitle(_title);

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif

	_actionToggleFullScreen = new QAction(this);
	_actionToggleFullScreen->setObjectName(QString::fromUtf8("toggleFS"));
#if QT_VERSION >= 0x050000
	_actionToggleFullScreen->setShortcut(QApplication::translate("MainWindow", "F11", 0));
	_actionToggleFullScreen->setText(QApplication::translate("MainWindow", "Toggle FullScreen", 0));
#else
	_actionToggleFullScreen->setShortcut(QApplication::translate("MainWindow", "F11", 0, QApplication::UnicodeUTF8));
	_actionToggleFullScreen->setText(QApplication::translate("MainWindow", "Toggle FullScreen", 0, QApplication::UnicodeUTF8));
#endif

	_actionShowSettings = new QAction(this);
	_actionShowSettings->setObjectName(QString::fromUtf8("showSettings"));
#if QT_VERSION >= 0x050000
	_actionShowSettings->setShortcut(QApplication::translate("MainWindow", "F2", 0));
	_actionShowSettings->setText(QApplication::translate("MainWindow", "Configure &Connection...", 0));
#else
	_actionShowSettings->setShortcut(QApplication::translate("MainWindow", "F2", 0, QApplication::UnicodeUTF8));
	_actionShowSettings->setText(QApplication::translate("MainWindow", "Configure &Connection...", 0, QApplication::UnicodeUTF8));
#endif

	_actionShowSettings->setEnabled(SCApp->isMessagingEnabled() || SCApp->isDatabaseEnabled());

	connect(_actionToggleFullScreen, SIGNAL(triggered(bool)), this, SLOT(toggleFullScreen()));
	connect(_actionShowSettings, SIGNAL(triggered(bool)), SCApp, SLOT(showSettings()));	

	addAction(_actionToggleFullScreen);
	addAction(_actionShowSettings);

	QAction *inspectConfig = new QAction(this);
	inspectConfig->setObjectName(QString::fromUtf8("inspectConfig"));
#if QT_VERSION >= 0x050000
	inspectConfig->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+C", 0));
	inspectConfig->setText(QApplication::translate("MainWindow", "Inspect &Configmodule...", 0));
#else
	inspectConfig->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+C", 0, QApplication::UnicodeUTF8));
	inspectConfig->setText(QApplication::translate("MainWindow", "Inspect &Configmodule...", 0, QApplication::UnicodeUTF8));
#endif

	QAction *inspectInventory = new QAction(this);
	inspectInventory->setObjectName(QString::fromUtf8("inspectInventory"));
#if QT_VERSION >= 0x050000
	inspectInventory->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+I", 0));
	inspectInventory->setText(QApplication::translate("MainWindow", "Inspect &Inventory...", 0));
#else
	inspectInventory->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+I", 0, QApplication::UnicodeUTF8));
	inspectInventory->setText(QApplication::translate("MainWindow", "Inspect &Inventory...", 0, QApplication::UnicodeUTF8));
#endif

	addAction(inspectConfig);
	addAction(inspectInventory);

	connect(inspectConfig, SIGNAL(triggered(bool)),
	        this, SLOT(inspectConfig()));

	connect(inspectInventory, SIGNAL(triggered(bool)),
	        this, SLOT(inspectInventory()));

	connect(SCApp, SIGNAL(connectionEstablished()),
	        this, SLOT(connectionEstablished()));

	connect(SCApp, SIGNAL(connectionLost()),
	        this, SLOT(connectionLost()));

	setAcceptDrops(true);

	setWindowIcon(QIcon(QPixmap(":/images/images/gfz-logo.png")));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainWindow::~MainWindow() {
	if ( SCApp ) {
		SCApp->settings().beginGroup(objectName());
		SCApp->settings().setValue("geometry", saveGeometry());
		SCApp->settings().setValue("state", saveState());
		SCApp->settings().endGroup();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
	if ( event->mimeData()->hasFormat("text/plain") ) {
		event->acceptProposedAction();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::dropEvent(QDropEvent *event) {
	if ( event->mimeData()->hasFormat("text/plain") ) {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
		PublicObject *po = PublicObject::Find(event->mimeData()->text().toStdString());
#else
		PublicObject *po = PublicObject::Find(event->mimeData()->data("text/plain").toStdString());
#endif
		if ( po ) {
			Inspector *w = new Inspector(this, Qt::Tool);
			w->setAttribute(Qt::WA_DeleteOnClose);
			w->setObject(po);
			w->show();
			event->acceptProposedAction();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MainWindow::restoreGeometry(const QByteArray & geometry) {
	bool res = QMainWindow::restoreGeometry(geometry);
#ifdef MACOSX
		Mac::addFullscreen(this);
#endif
	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::paintEvent(QPaintEvent *e) {
	QMainWindow::paintEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showEvent(QShowEvent *e) {
	QMainWindow::showEvent(e);

	if ( _connectionState ) return;

	if ( SCApp ) {
		SCApp->settings().beginGroup(objectName());
		restoreState(SCApp->settings().value("state").toByteArray());
		restoreGeometry(SCApp->settings().value("geometry").toByteArray());
		SCApp->settings().endGroup();
	}

	if ( _showFullscreen != isFullScreen() )
		toggleFullScreen();

	if ( !statusBar() ) return;

	_connectionState = new ConnectionStateLabel(statusBar());
	statusBar()->addPermanentWidget(_connectionState);

	onChangedConnection();

	connect(SCApp, SIGNAL(connectionEstablished()),
	        _connectionState, SLOT(start()));

	connect(SCApp, SIGNAL(connectionLost()),
	        _connectionState, SLOT(stop()));

	connect(SCApp, SIGNAL(changedConnection()),
	        this, SLOT(onChangedConnection()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showNormal() {
	_showFullscreen = false;
	QMainWindow::showNormal();
	toggledFullScreen(false);
	fullScreenToggled(false);

	if ( menuBar() && !SCScheme.showMenu )
		menuBar()->setVisible(false);

	if ( statusBar() && !SCScheme.showStatusBar )
		statusBar()->setVisible(false);

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showFullScreen() {
	_showFullscreen = true;
	QMainWindow::showFullScreen();
	toggledFullScreen(true);
	fullScreenToggled(true);

	if ( menuBar() && !SCScheme.showMenu )
		menuBar()->setVisible(false);

	if ( statusBar() && !SCScheme.showStatusBar )
		statusBar()->setVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::toggleFullScreen() {
	if ( isFullScreen() )
		showNormal();
	else
		showFullScreen();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setTitle(QString t) {
	_title = t;

	if ( SCApp->connection() ) {
		QString title = _title + "@" + SCApp->messagingHost().c_str();
		if ( SCApp->isReadOnlyMessaging() )
			title += " (read-only)";
		setWindowTitle(title);
	}
	else
		setWindowTitle(_title);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString MainWindow::title() const {
	return _title;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::connectionEstablished() {
	QString title = _title + "@" + SCApp->messagingHost().c_str();
	if ( SCApp->isReadOnlyMessaging() )
		title += " (read-only)";
	setWindowTitle(title);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::connectionLost() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::onChangedConnection() {
	if ( SCApp->connection() && SCApp->connection()->isConnected() )
		_connectionState->start();
	else
		_connectionState->stop();

	if ( SCApp->connection() ) {
		QString title = _title + "@" + SCApp->messagingHost().c_str();
		if ( SCApp->isReadOnlyMessaging() )
			title += " (read-only)";
		setWindowTitle(title);
	}
	else
		setWindowTitle(_title);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::toggledFullScreen(bool) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::inspectConfig() {
	if ( !SCApp->configModule() ) return;

	Inspector *w = new Inspector(this, Qt::Tool);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setObject(Client::ConfigDB::Instance()->config());
	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::inspectInventory() {
	if ( !Client::Inventory::Instance()->inventory() ) return;

	Inspector *w = new Inspector(this, Qt::Tool);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setObject(Client::Inventory::Instance()->inventory());
	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
