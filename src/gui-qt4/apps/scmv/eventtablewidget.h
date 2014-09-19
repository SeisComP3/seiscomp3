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


#ifndef __EVENTTABLEWIDGET_H___
#define __EVENTTABLEWIDGET_H___

#include <memory>

#include <QTableWidget>


class EventTableWidget : public QTableWidget {
	Q_OBJECT

	// ----------------------------------------------------------------------
	// Nested Types
	// ----------------------------------------------------------------------
	public:
		enum ColumnIdentifier {
			EVENT_ID,
			ORIGIN_TIME,
			MAGNITUDE,
			MAGNITUDE_TYPE,
			EVENT_REGION,
			LATITUDE,
			LONGITUDE,
			DEPTH
		};

		class RowData : public std::map<ColumnIdentifier, QString> {
			public:
				bool isActive() const { return _isActive; }
				void setActive(bool val) { _isActive = val; }

				bool isSelected() const { return _isSelected; }
				void setSelected(bool val) { _isSelected = val; }

			private:
				bool _isActive;
				bool _isSelected;
		};

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		EventTableWidget(QWidget* parent = 0);
		virtual ~EventTableWidget();

	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		void removeEventTableEntries();
		void addRow(RowData& row);

	// ----------------------------------------------------------------------
	// Private Slots
	// ----------------------------------------------------------------------
	private slots:
		void handleCellDoubleClickEvent(int row);
		void handleCellPressedEvent(int row);

	// ----------------------------------------------------------------------
	// Signals
	// ----------------------------------------------------------------------
	signals:
		void eventSelected(const QString& eventId);
		void eventDeselected(const QString& eventId);
		void eventDoubleClicked(const QString& eventId);

	// ----------------------------------------------------------------------
	// Protected interface
	// ----------------------------------------------------------------------
	protected:
		virtual void keyPressEvent(QKeyEvent* keyEvent);
		virtual void keyReleaseEvent(QKeyEvent* keyEvent);

	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		void uiInit();
		QString getEventId(int row);

		int selectedRow() const;
		void setSelectedRow(int row);
		bool isRowSelected(int row);


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		bool _controlKeyPressed;
		int  _selectedRow;

};

#endif
