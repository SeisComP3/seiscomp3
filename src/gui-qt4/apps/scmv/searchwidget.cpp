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

#include "searchwidget.h"

#include <QHeaderView>
#include <QTableWidgetItem>
#include <QKeyEvent>


int SearchWidget::_ObjectCount = 0;




SearchWidget::SearchWidget(QWidget* widget, Qt::WFlags f)
 : QWidget(widget, f) {
	_ui.setupUi(this);
	setWindowFlags(Qt::Tool);
	setAttribute(Qt::WA_DeleteOnClose);

	_ObjectCount++;

	_ui.searchLineEdit->setFocus(Qt::TabFocusReason);
	_ui.resultTableWidget->sortItems(0 , Qt::AscendingOrder);
	_ui.resultTableWidget->verticalHeader()->hide();
	_ui.resultTableWidget->horizontalHeader()->hide();
	_ui.resultTableWidget->horizontalHeader()->setStretchLastSection(true);
	_ui.resultTableWidget->resizeColumnsToContents();

	connect(_ui.searchLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(findStation(const QString&)));

	connect(_ui.resultTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(getSelectedStation()));
	connect(_ui.resultTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(findStation()));
	connect(_ui.findPushButton, SIGNAL(clicked(bool)), this, SIGNAL(stationSelected()));
	connect(_ui.canclePushButton, SIGNAL(clicked(bool)), this, SLOT(close()));

	activateWindow();
}




SearchWidget::~SearchWidget() {
	_ObjectCount--;
}




void SearchWidget::addStationName(const std::string& name) {
	_stationNames.push_back(name);
	_matches.push_back(name);
	addTableWidgetItem(name);
}




const SearchWidget::Matches& SearchWidget::matches() {
	_previousMatches.assign(_matches.begin(), _matches.end());
	return _matches;
}



const SearchWidget::Matches& SearchWidget::previousMatches() const {
	return _previousMatches;
}




SearchWidget* SearchWidget::Create(QWidget* parent, Qt::WFlags f) {
	if ( _ObjectCount > 0 )
		return NULL;

	return new SearchWidget(parent, f);
}




void SearchWidget::keyPressEvent(QKeyEvent* event) {
	switch ( event->key() ) {
		case Qt::Key_Escape:
			close();
			break;

		case Qt::Key_Return:
			emit stationSelected();
			break;

		default:
			break;
	}
}




void SearchWidget::updateContent() {
	Matches::const_iterator it = _matches.begin();
	for ( ; it != _matches.end(); it++ )
		addTableWidgetItem(*it);
}




void SearchWidget::matchString(const std::string& str) {
	StationNames::const_iterator it = _stationNames.begin();
	for ( ; it != _stationNames.end(); it++ ) {
		QString tmpStr(it->c_str());
		bool match = str.empty() ||
		             tmpStr.contains(str.c_str(), Qt::CaseInsensitive);
		if ( match )
			addMatch(*it);
	}
}




void SearchWidget::addTableWidgetItem(const std::string& name) {
	_ui.resultTableWidget->insertRow(0);

	QTableWidgetItem* item = new QTableWidgetItem(name.c_str());
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

	_ui.resultTableWidget->setItem(0, 0, item);
}




void SearchWidget::removeContent() {
	while ( _ui.resultTableWidget->rowCount() > 0 )
		_ui.resultTableWidget->removeRow(0);
}




void SearchWidget::removeMatches() {
	_matches.clear();
}




void SearchWidget::addMatch(const std::string& station) {
	_matches.push_back(station);
}




void SearchWidget::findStation(const QString& str) {
	removeMatches();
	matchString(str.toStdString());

	removeContent();
	updateContent();
}



void SearchWidget::findStation() {
	getSelectedStation();

	emit stationSelected();
}




void SearchWidget::getSelectedStation() {
	removeMatches();

	QList<QTableWidgetItem*> selectedItems = _ui.resultTableWidget->selectedItems ();
	QList<QTableWidgetItem*>::iterator it = selectedItems.begin();
	for ( ; it != selectedItems.end(); it++ ) {
		QTableWidgetItem* item = *it;
		std::string station = item->text().toStdString();
		addMatch(station);
	}
}
