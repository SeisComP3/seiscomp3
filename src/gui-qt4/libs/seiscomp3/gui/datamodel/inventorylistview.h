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



#ifndef __SEISCOMP_GUI_INVENTORYLISTVIEW_H__
#define __SEISCOMP_GUI_INVENTORYLISTVIEW_H__

#include <QTreeWidget>
#include <QStack>
#include <QMap>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {

namespace DataModel {

class Network;
class Station;
class Stream;
class Component;

}


namespace Gui {

class SC_GUI_API InventoryListView : public QTreeWidget,
                                               public DataModel::Visitor {
	Q_OBJECT


	public:
		InventoryListView(QWidget *parent, Qt::WFlags f = 0);

		bool selectStream(const QString& streamID, bool);
		void selectStreams(const QString&, bool);

		QStringList selectedStreams() const;
		void expandNetworks();

	protected:
		bool visit(DataModel::PublicObject*);
		void visit(DataModel::Object*);
		void finished();


	signals:
		void streamChanged(QString, bool);
	
	public slots:
		void clearSelection();
		void setNonSelectedHidden(bool);

	private slots:
		void onItemChanged(QTreeWidgetItem *item, int column);
		void onItemPressed(QTreeWidgetItem *item, int column);


	private:
		void updateChildSelection(QTreeWidgetItem *item);
		void updateParentSelection(QTreeWidgetItem *item);

		void setNonSelectedHidden(QTreeWidgetItem*, bool);

		template <typename T>
		QTreeWidgetItem* add(T*);

		template <typename T>
		QTreeWidgetItem* add(QTreeWidgetItem* parent, T*);

		template <typename T>
		QTreeWidgetItem* create(T* object);

		QTreeWidgetItem* createDefaultItem();
		QTreeWidgetItem* insert(DataModel::Object*);

		void setRow(QTreeWidgetItem*,
		            const QString& first,
		            const QString& second = "",
		            const QString& third = "");

		void notifyAboutStateChange(const QString& streamID, bool state, bool hightPriority = false);

	private:
		QStack<QTreeWidgetItem*> _itemStack;

		typedef QMap<QChar, QTreeWidgetItem*> ComponentMap;
		typedef QMap<QString, ComponentMap> StreamMap;
		typedef QMap<QString, StreamMap> StationMap;
		typedef QMap<QString, StationMap> NetworkMap;
		typedef QMap<QString, QTreeWidgetItem*> StreamItems;

		StreamItems _streamItems;

		QTreeWidgetItem* _highestChangedItem;
};


}
}


#endif


