/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT EventSummaryView

#include "mainframe.h"
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/messages.h>
#include <seiscomp3/gui/datamodel/eventsummaryview.h>
#include <seiscomp3/gui/datamodel/eventlistview.h>
#include <seiscomp3/gui/datamodel/eventlayer.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace Applications {
namespace EventSummaryView {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainFrame::MainFrame() {
	_ui.setupUi(this);

	_ui.menuOptions->insertAction(_ui.actionAutoSelect, _actionShowSettings);
	_ui.menuView->addAction(_actionToggleFullScreen);

	// action buttons
	addAction(_ui.actionAutoSelect);
	addAction(_ui.actionShowStations);
	addAction(_ui.actionToggleEventList);
	addAction(_ui.actionToggleWaveformPropagation);

	_ui.actionAutoSelect->setChecked(TRUE);

	// create SummaryView tab
	_eventSummary = new Gui::EventSummaryView(SCApp->mapsDesc(), SCApp->query());

	// create EventListView tab
	_listPage = new Gui::EventListView(SCApp->query(), false, false, this);

	if ( SCApp->nonInteractive() ) {
		delete _ui.tabWidget;
		_ui.tabWidget = NULL;
		setCentralWidget(_eventSummary);
		_eventSummary->setContentsMargins(4,4,4,4);
		_listPage->setVisible(false);
	}
	else {
		QLayout* layoutSummary = new QVBoxLayout(_ui.tabSummary);
		layoutSummary->setMargin(4);
		layoutSummary->addWidget(_eventSummary);
	
		QLayout* layoutEventList = new QVBoxLayout(_ui.tabEventList);
		layoutEventList->setMargin(4);
		layoutEventList->addWidget(_listPage);

		SCScheme.applyTabPosition(_ui.tabWidget);
	}

	_wt = new QLabel();
	_wt->setFrameStyle(QFrame::NoFrame);

	// connect action buttons
	connect(_ui.actionShowBeachballs, SIGNAL(toggled(bool)), _eventSummary, SLOT(drawBeachballs(bool)));
	connect(_ui.actionShowFullTensor, SIGNAL(toggled(bool)), _eventSummary, SLOT(drawFullTensor(bool)));
	connect(_ui.actionShowStations, SIGNAL(toggled(bool)), _eventSummary, SLOT(drawStations(bool)));
	connect(_ui.actionAutoSelect, SIGNAL(triggered(bool)), _eventSummary, SLOT(setAutoSelect(bool)));
	connect(_ui.actionToggleEventList, SIGNAL(triggered(bool)), this, SLOT(toggleEventList()));
	connect(_ui.actionToggleWaveformPropagation, SIGNAL(triggered(bool)), _eventSummary, SLOT(setWaveformPropagation(bool)));

	_eventSummary->drawStations(_ui.actionShowStations->isChecked());
	_eventSummary->setWaveformPropagation(_ui.actionToggleWaveformPropagation->isChecked());

	// connect add/update msg to SummaryView
	connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSummary, SLOT(addObject(const QString&, Seiscomp::DataModel::Object*)));
	connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSummary, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));
	connect(SCApp, SIGNAL(removeObject(const QString&,Seiscomp::DataModel::Object*)),
	        _eventSummary, SLOT(removeObject(const QString&,Seiscomp::DataModel::Object*)));

	// connect msg/notifier to listPage
/*	connect(SCApp, SIGNAL(messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)),
	        listPage, SLOT(messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)));*/
	connect(SCApp, SIGNAL(notifierAvailable(Seiscomp::DataModel::Notifier*)),
	        _listPage, SLOT(notifierAvailable(Seiscomp::DataModel::Notifier*)));

	// connect selected item and switch to SummaryView
	connect(_listPage, SIGNAL(eventSelected(Seiscomp::DataModel::Event*)),
	        _eventSummary, SLOT(showEvent(Seiscomp::DataModel::Event*)));
	connect(_listPage, SIGNAL(eventSelected(Seiscomp::DataModel::Event*)),
	        this, SLOT(showESVTab()));

	// connect locator button
	connect(_eventSummary, SIGNAL(toolButtonPressed()), this, SLOT(showLocator()));

	// connect display status message
	connect(_eventSummary, SIGNAL(showInStatusbar(QString, int)), this, SLOT(showInStatusbar(QString, int)));

	//! if fake event rmoved: reread Database
	//connect(_eventSummary, SIGNAL(reReadDatabase()), listPage, SLOT(readFromDatabase()));
	connect(_eventSummary, SIGNAL(newNofifier(Seiscomp::DataModel::Notifier*)),
	        _listPage, SLOT(notifierAvailable(Seiscomp::DataModel::Notifier*)));

	connect(_eventSummary, SIGNAL(requestNonFakeEvent()),
	        this, SLOT(setLastNonFakeEvent()));

	// Connect events layer with map
	Gui::EventLayer *eventMapLayer = new Gui::EventLayer(_eventSummary->map());
	connect(_listPage, SIGNAL(reset()), eventMapLayer, SLOT(clear()));
	connect(_listPage, SIGNAL(eventAddedToList(Seiscomp::DataModel::Event*)),
	        eventMapLayer, SLOT(addEvent(Seiscomp::DataModel::Event*)));
	connect(_listPage, SIGNAL(eventUpdatedInList(Seiscomp::DataModel::Event*)),
	        eventMapLayer, SLOT(updateEvent(Seiscomp::DataModel::Event*)));
	connect(_listPage, SIGNAL(eventRemovedFromList(Seiscomp::DataModel::Event*)),
	        eventMapLayer, SLOT(removeEvent(Seiscomp::DataModel::Event*)));

	_eventSummary->map()->canvas().addLayer(eventMapLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainFrame::~MainFrame() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Gui::EventSummaryView* MainFrame::eventSummaryView() const {
	return _eventSummary;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::loadEvents(float days) {
	Core::TimeWindow tw;

	tw.setEndTime(Core::Time::GMT());
	tw.setStartTime(tw.endTime() - Core::TimeSpan(days*86400));

	_listPage->setInterval(tw);
	_listPage->readFromDatabase();

	if ( _eventSummary->ignoreOtherEvents() )
		setLastNonFakeEvent();
	else
		_listPage->selectEvent(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::showLocator() {
	if ( !_eventSummary->currentOrigin() ) return;

	SCApp->sendCommand(Gui::CM_SHOW_ORIGIN, _eventSummary->currentOrigin()->publicID());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::toggleDock() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::toggleEventList() {
	if ( _ui.tabWidget == NULL ) return;

	if ( _ui.tabWidget->currentIndex() == 0 )
		_ui.tabWidget->setCurrentIndex(1);
	else
		_ui.tabWidget->setCurrentIndex(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::toggledFullScreen(bool isFullScreen) {
	if ( menuBar() )
		menuBar()->setVisible(!isFullScreen);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::showInStatusbar(QString text, int time) {
	if ( statusBar() ){
		if (time > 0)
			statusBar()->showMessage(text, time);
		else {
			if (text != ""){
				_wt->setText(text);
				statusBar()->insertPermanentWidget(0, _wt);
			}
			else 
				statusBar()->removeWidget(_wt);
		}	
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::setLastNonFakeEvent() {
	_listPage->selectFirstEnabledEvent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::clearStatusbar() {
	if ( statusBar() )
		statusBar()->showMessage("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::showESVTab() {
	if ( _ui.tabWidget )
		_ui.tabWidget->setCurrentIndex(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
