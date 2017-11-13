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



#include <seiscomp3/gui/datamodel/inventorylistview.h>
#include <seiscomp3/client/inventory.h>

#include <iostream>

namespace Seiscomp {
namespace Gui {


namespace {


class NetworkTreeItem : public QTreeWidgetItem {
	public:
		NetworkTreeItem()
		 : QTreeWidgetItem(), _initialState(false) {}
		NetworkTreeItem(QTreeWidgetItem *parent)
		 : QTreeWidgetItem(parent), _initialState(false) {}

		void setInitialState(bool s) {
			_initialState = true;
		}

		bool initialState() const {
			return _initialState;
		}

	private:
		bool _initialState;
};


}


InventoryListView::InventoryListView(QWidget *parent, Qt::WFlags f)
 : QTreeWidget(parent), DataModel::Visitor()
{
	setWindowFlags(f);

	DataModel::Inventory* inv = Client::Inventory::Instance()->inventory();

	setColumnCount(3);
	setHeaderLabels(QStringList() << "Name" << "Information" << "");

	if ( !inv ) return;

	QTreeWidgetItem *invItem = add(inv);
	if ( !invItem ) return;

	invItem->setFlags(Qt::ItemIsEnabled);
	_itemStack.push(invItem);

	QTreeWidgetItem *networks = createDefaultItem();
	networks->setText(0, "Networks");

	invItem->addChild(networks);
	networks->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);

	_itemStack.push(networks);

	for ( size_t i = 0; i < inv->networkCount(); ++i )
		inv->network(i)->accept(this);

	_itemStack.clear();

	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
	        this, SLOT(onItemChanged(QTreeWidgetItem*, int)));

	connect(this, SIGNAL(itemPressed(QTreeWidgetItem*, int)),
	        this, SLOT(onItemPressed(QTreeWidgetItem*, int)));

	networks->setCheckState(0, Qt::Unchecked);
}


template <typename T>
QTreeWidgetItem* InventoryListView::add(T* object) {
	QTreeWidgetItem* item = create<T>(object);
	if ( item )
		addTopLevelItem(item);

	return item;
}


template <typename T>
QTreeWidgetItem* InventoryListView::add(QTreeWidgetItem* parent, T* object) {
	QTreeWidgetItem* item = create<T>(object);
	if ( item ) {
		parent->addChild(item);
		if ( parent->flags() == 0 ) {
			item->setFlags(parent->flags());
			item->setFont(0, parent->font(0));
		}
		else if ( item->flags() != 0 )
			item->setFlags(parent->flags());

		if ( item->flags() & Qt::ItemIsSelectable )
			item->setCheckState(0, Qt::Checked);
	}

	return item;
}


template <typename T>
QTreeWidgetItem* InventoryListView::create(T* object) {
	QTreeWidgetItem* item = createDefaultItem();
	item->setText(0, "[unknown]");
	return item;
}


QTreeWidgetItem* InventoryListView::createDefaultItem() {
	QTreeWidgetItem* item = new NetworkTreeItem();
	return item;
}


template <>
QTreeWidgetItem* InventoryListView::create(DataModel::Inventory* i) {
	QTreeWidgetItem* item = createDefaultItem();
	item->setText(0, "Inventory");
	return item;
}


template <>
QTreeWidgetItem* InventoryListView::create(DataModel::Network* n) {
	QTreeWidgetItem* item = createDefaultItem();

	QString interval = n->start().toString("%F").c_str();
	QFont f = item->font(0);

	try {
		interval += QString(" - %1").arg(n->end().toString("%F").c_str());
		item->setFlags(0);
		f.setItalic(true);
	}
	catch ( ... ) {
		interval = "since " + interval;
		f.setBold(true);
	}

	item->setFont(0, f);
	setRow(item, n->code().c_str(), interval);

// 	QString streamID = QString("%1").arg(n->code().c_str());
// 	item->setData(0, Qt::UserRole, streamID);

	return item;
}


template <>
QTreeWidgetItem* InventoryListView::create(DataModel::Station* s) {
	QTreeWidgetItem* item = createDefaultItem();

	QString interval = s->start().toString("%F").c_str();
	QFont f = item->font(0);

	try {
		interval += QString(" - %1").arg(s->end().toString("%F").c_str());
		item->setFlags(0);
		f.setItalic(true);
	}
	catch ( ... ) {
		interval = "since " + interval;
		f.setBold(true);
	}

	item->setFont(0, f);
	setRow(item, s->code().c_str(), interval);

	QString streamID = QString("%1.%2.*.*")
	                      .arg(s->network()->code().c_str())
	                      .arg(s->code().c_str());
	item->setData(0, Qt::UserRole, streamID);

	return item;
}


template <>
QTreeWidgetItem* InventoryListView::create(DataModel::SensorLocation* loc) {
	QTreeWidgetItem* item = createDefaultItem();

	QString interval = loc->start().toString("%F").c_str();
	QFont f = item->font(0);

	try {
		interval += QString(" - %1").arg(loc->end().toString("%F").c_str());
		item->setFlags(0);
		f.setItalic(true);
	}
	catch ( ... ) {
		interval = "since " + interval;
		f.setBold(true);
	}

	item->setFont(0, f);
	setRow(item, loc->code().c_str(), interval);

	QString streamID = QString("%1.%2.%3.*")
	                      .arg(loc->station()->network()->code().c_str())
	                      .arg(loc->station()->code().c_str())
	                      .arg(loc->code().c_str());
	item->setData(0, Qt::UserRole, streamID);

	return item;
}


template <>
QTreeWidgetItem* InventoryListView::create(DataModel::Stream* s) {
	QTreeWidgetItem* item = createDefaultItem();
	QString code;

	code = s->code().c_str();

	QString interval = s->start().toString("%F").c_str();
	QFont f = item->font(0);

	try {
		interval += QString(" - %1").arg(s->end().toString("%F").c_str());
		item->setFlags(0);
		f.setItalic(true);
	}
	catch ( ... ) {
		interval = "since " + interval;
		f.setBold(true);
	}

	item->setFont(0, f);
	setRow(item, code, interval);

	QString streamID = QString("%1.%2.%3.%4")
	                      .arg(s->sensorLocation()->station()->network()->code().c_str())
	                      .arg(s->sensorLocation()->station()->code().c_str())
	                      .arg(s->sensorLocation()->code().c_str())
	                      .arg(s->code().c_str());
	item->setData(0, Qt::UserRole, streamID);

	_streamItems.insert(streamID, item);

	return item;
}


template <>
QTreeWidgetItem* InventoryListView::create(DataModel::Object* o) {
	DataModel::Network* n = DataModel::Network::Cast(o);
	if ( n )
		return create(n);

	DataModel::Station* s = DataModel::Station::Cast(o);
	if ( s )
		return create(s);

	DataModel::SensorLocation* sl = DataModel::SensorLocation::Cast(o);
	if ( sl )
		return create(sl);

	DataModel::Stream* st = DataModel::Stream::Cast(o);
	if ( st )
		return create(st);

	return NULL;
}


bool InventoryListView::visit(DataModel::PublicObject* po) {
	QTreeWidgetItem* item = insert(po);
	if ( item == NULL ) return false;

	_itemStack.push(item);
	return true;
}


void InventoryListView::visit(DataModel::Object* o) {
	insert(o);
}


QTreeWidgetItem* InventoryListView::insert(DataModel::Object* o) {
	if ( !_itemStack.empty() )
		return add(_itemStack.top(), o);
	else
		return add(o);
}


void InventoryListView::finished() {
	_itemStack.pop();
}


void InventoryListView::setRow(QTreeWidgetItem* item, const QString& first,
                               const QString& second, const QString& third) {
	item->setText(0, first);
	item->setText(1, second);
	item->setText(2, third);
}


void InventoryListView::onItemChanged(QTreeWidgetItem *item, int column) {
	if ( column == 0 ) {
		blockSignals(true);

		_highestChangedItem = NULL;

		updateParentSelection(item);

		/*if ( _highestChangedItem )
			std::cout << "Enable stream " << _highestChangedItem->data(0, Qt::UserRole).toString().toStdString() << std::endl;*/

		if ( !item->data(0, Qt::UserRole).isNull() )
			notifyAboutStateChange(item->data(0, Qt::UserRole).toString(), item->checkState(0) == Qt::Checked);

		updateChildSelection(item);

		blockSignals(false);
	}
}


void InventoryListView::onItemPressed(QTreeWidgetItem *item, int column) {
}


void InventoryListView::updateChildSelection(QTreeWidgetItem *item) {
	if ( item->checkState(0) != Qt::PartiallyChecked ) {
		for ( int i = 0; i < item->childCount(); ++i ) {
			QTreeWidgetItem* child = item->child(i);
			if ( !(child->flags() & Qt::ItemIsSelectable) ) continue;

			if ( child->checkState(0) != item->checkState(0) ) {
				child->setCheckState(0, item->checkState(0));

				if ( !child->data(0, Qt::UserRole).isNull() &&
				     item->data(0, Qt::UserRole).isNull() )
					notifyAboutStateChange(child->data(0, Qt::UserRole).toString(), child->checkState(0) == Qt::Checked);
			}

			updateChildSelection(child);
		}
	}
}


void InventoryListView::updateParentSelection(QTreeWidgetItem *it) {
	QTreeWidgetItem* parent = it->parent();
	if ( parent == NULL ) return;

	//std::cout << "ItemValue: " << parent->data(0, Qt::UserRole).toString().toStdString() << std::endl;

	if ( parent->flags() & Qt::ItemIsSelectable ) {
		bool allChecked = true;
		bool allUnchecked = true;
	
		for ( int i = 0; i < parent->childCount(); ++i ) {
			switch ( parent->child(i)->checkState(0) ) {
				case Qt::Unchecked:
					allChecked = false;
					break;
				case Qt::Checked:
					allUnchecked = false;
					break;
				case Qt::PartiallyChecked:
					allChecked = false;
					allUnchecked = false;
					break;
			};
		}

		Qt::CheckState state = allChecked?Qt::Checked:(allUnchecked?Qt::Unchecked:Qt::PartiallyChecked);

		if ( state != parent->checkState(0) ) {
			parent->setCheckState(0, state);

			if ( !parent->data(0, Qt::UserRole).isNull() ) {
				_highestChangedItem = parent;
				notifyAboutStateChange(parent->data(0, Qt::UserRole).toString(), state == Qt::Checked, true);
			}

			updateParentSelection(parent);
		}
	}
}


void InventoryListView::notifyAboutStateChange(const QString& streamID, bool state, bool hightPriority) {
	if ( hightPriority ) {
	}
	else {
		//std::cout << (state?"Enable":"Disable") << " stream " << streamID.toStdString() << std::endl;
	}

	
}


bool InventoryListView::selectStream(const QString& streamID, bool select) {
	StreamItems::iterator it = _streamItems.find(streamID);
	if ( it == _streamItems.end() )
		return false;

	static_cast<NetworkTreeItem*>(it.value())->setInitialState(select);
	it.value()->setCheckState(0, select?Qt::Checked:Qt::Unchecked);

	return true;
}

void InventoryListView::selectStreams(const QString& expr, bool select) {
	for (StreamItems::const_iterator it = _streamItems.begin(); it != _streamItems.end(); ++it ) {
		if ( Core::wildcmp(expr.toAscii(), it.key().toAscii() ) )
			it.value()->setCheckState(0, select?Qt::Checked:Qt::Unchecked);
	}
}

QStringList InventoryListView::selectedStreams() const {
	QStringList streams;
	for (StreamItems::const_iterator it = _streamItems.begin(); it != _streamItems.end(); ++it ) {
		if ( it.value()->checkState(0) == Qt::Checked ) {
			streams.push_back(it.key());
		}
	}
	return streams;
}

void InventoryListView::clearSelection() {
	for (StreamItems::const_iterator it = _streamItems.begin(); it != _streamItems.end(); ++it ) {
		if ( it.value()->checkState(0) )	
			it.value()->setCheckState(0, Qt::Unchecked);
	}
}

void InventoryListView::expandNetworks() {
	for ( int i = 0; i < topLevelItemCount(); ++i ) {
		QTreeWidgetItem* folder = topLevelItem(i);
		for ( int j = 0; j < folder->childCount(); ++j ) {
			QTreeWidgetItem* subFolder = folder->child(j);
			expandItem(subFolder);
		}
		expandItem(folder);
	}
}


void InventoryListView::setNonSelectedHidden(QTreeWidgetItem* item, bool hide) {
	for ( int i = 0; i < item->childCount(); ++i ) {
		QTreeWidgetItem* child = item->child(i);
		if ( hide ) {
			if ( !child->isHidden() && child->checkState(0) == Qt::Unchecked)
				child->setHidden(true);
			else
				setNonSelectedHidden(child, hide);
		} else {
			if ( child->isHidden() )
				child->setHidden(false);
			else
				setNonSelectedHidden(child, hide);
		}
	}
}

void InventoryListView::setNonSelectedHidden(bool hide) {
	blockSignals(true);
	for ( int i = 0; i < topLevelItemCount(); ++i )
		setNonSelectedHidden(topLevelItem(i), hide);
	blockSignals(false);
}


}
}
