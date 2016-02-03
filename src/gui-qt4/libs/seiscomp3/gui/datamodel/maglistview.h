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



#ifndef __SEISCOMP_GUI_MAGLISTVIEW_H__
#define __SEISCOMP_GUI_MAGLISTVIEW_H__

#include <QtGui>
#include <seiscomp3/gui/core/connectiondialog.h>
#include <seiscomp3/gui/datamodel/ui_maglistview.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/datamodel/magnitude.h>
#endif
#include <seiscomp3/gui/qt4.h>

namespace Seiscomp {

namespace DataModel {

DEFINE_SMARTPOINTER(Event);
DEFINE_SMARTPOINTER(Origin);
DEFINE_SMARTPOINTER(Pick);
DEFINE_SMARTPOINTER(Station);
class DatabaseQuery;
class Notifier;

}

namespace Client {

DEFINE_SMARTPOINTER(Connection);

}

namespace Gui {


class SC_GUI_API MagListView : public QWidget {
	Q_OBJECT

	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef QMap<QString, DataModel::StationPtr> StationMap;


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		MagListView(Seiscomp::DataModel::DatabaseQuery* reader,
		              bool withOrigins = true,
		              QWidget * parent = 0, Qt::WFlags f = 0);
		~MagListView();


	signals:
		void originAdded();
		void netMagAdded();

		void netMagSelected(Seiscomp::DataModel::Magnitude*,
		                    Seiscomp::DataModel::Origin* = NULL,
                            Seiscomp::DataModel::Event* = NULL);
		void originSelected(Seiscomp::DataModel::Origin*,
		                    Seiscomp::DataModel::Event* = NULL);
		void eventSelected(Seiscomp::DataModel::Event*);

		void originUpdated(Seiscomp::DataModel::Origin*);


	public slots:
		void setAutoSelect(bool);
		void messageAvailable(Seiscomp::Core::Message*);
		void notifierAvailable(Seiscomp::DataModel::Notifier*);
		void expandEventItem(QTreeWidgetItem* eventItem, int col);
		void expandOriginItem(QTreeWidgetItem* originItem, int col);

	protected slots:
		void itemSelected(QTreeWidgetItem*,int);

		void readFromDatabase();
		void clear();
		//! HACK
		void onShowAll();


	private:
		void initTree();

		QTreeWidgetItem* addEvent(Seiscomp::DataModel::Event*);
		QTreeWidgetItem* addOrigin(Seiscomp::DataModel::Origin*, bool bold, QTreeWidgetItem* parent = NULL);
		QTreeWidgetItem* addNetMag(Seiscomp::DataModel::Magnitude*, bool bold, QTreeWidgetItem* parent = NULL);

		QTreeWidgetItem* findEvent(const std::string&);
		QTreeWidgetItem* findOrigin(const std::string&);
		QTreeWidgetItem* findNetMag(const std::string&);

		void readPicks(Seiscomp::DataModel::Origin*);


	private:
		::Ui::OriginListView _ui;
		QTreeWidgetItem* _unassociatedEventItem;
		QVector<DataModel::PickPtr> _associatedPicks;
		//StationMap _associatedStations;
		Seiscomp::DataModel::DatabaseQuery* _reader;
		bool _autoSelect;
		bool _withOrigins;
		bool _blockSelection;
		bool _readLock;

};


}
}

#endif
