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

#ifndef __SEISCOMP_SEARCHWIDGET_H__
#define __SEISCOMP_SEARCHWIDGET_H__


#include <vector>

#include <QString>
#include <QWidget>

#include "ui_searchwidget.h"


class SearchWidget : public QWidget {
	Q_OBJECT

	// ----------------------------------------------------------------------
	// Nested Types
	// ----------------------------------------------------------------------
	public:
		typedef std::vector<std::string> Matches;

	private:
		typedef std::vector<std::string> StationNames;

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	private:
		SearchWidget(QWidget* parent = 0, Qt::WFlags f = 0);
		~SearchWidget();

	// ----------------------------------------------------------------------
	// Public Interface
	// ----------------------------------------------------------------------
	public:
		void addStationName(const std::string& name);
		const Matches& matches();
		const Matches& previousMatches() const;

		static SearchWidget* Create(QWidget* parent = 0, Qt::WFlags f = 0);

	// ----------------------------------------------------------------------
	// Protected Interface
	// ----------------------------------------------------------------------
	protected:
		virtual void keyPressEvent(QKeyEvent* evt);

	// ----------------------------------------------------------------------
	// Private Interface
	// ----------------------------------------------------------------------
	private:
		void updateContent();

		void matchString(const std::string& str);

		void addTableWidgetItem(const std::string& name);

		void removeContent();
		void removeMatches();

		void addMatch(const std::string& station);

	// ----------------------------------------------------------------------
	// Private Slots
	// ----------------------------------------------------------------------
	private slots:
		void findStation(const QString& str);
		void findStation();
		void getSelectedStation();

	signals:
		void stationSelected();

	// ----------------------------------------------------------------------
	// Private Datamember
	// ----------------------------------------------------------------------
	private:
		Ui::SearchWidget _ui;

		Matches      _matches;
		Matches      _previousMatches;
		StationNames _stationNames;

		static int _ObjectCount;
};


#endif
