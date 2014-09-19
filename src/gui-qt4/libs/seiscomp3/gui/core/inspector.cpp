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




#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/gui/core/inspector.h>

#include <QHeaderView>
#include <QLabel>

using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace Gui {


namespace {


class TreeItem : public QTreeWidgetItem {
	public:
		TreeItem(QTreeWidget * parent, BaseObject *obj) : QTreeWidgetItem(parent), _object(obj) {}
		TreeItem(QTreeWidgetItem * parent, BaseObject *obj) : QTreeWidgetItem(parent), _object(obj) {}

		BaseObject *object() const { return _object; }

	private:
		BaseObject *_object;
};


std::string propToString(const MetaProperty *prop, BaseObject *obj) {
	try {
		return prop->readString(obj);
	}
	catch ( ... ) {
		return "";
	}
}


}


Inspector::Inspector(QWidget * parent, Qt::WFlags f)
: QWidget(parent, f), _object(NULL), _currentSelection(NULL) {
	_ui.setupUi(this);

	_ui.treeWidget->setHeaderLabels(QStringList() << "Object" << "Type");

	setObject(NULL);

	_ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	_ui.tableWidget->setSelectionMode(QAbstractItemView::NoSelection);

	connect(_ui.treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
	        this, SLOT(selectionChanged()));

	connect(_ui.buttonBack, SIGNAL(clicked()), this, SLOT(back()));
}


Inspector::~Inspector() {}


void Inspector::setObject(BaseObject *obj) {
	_object = obj;
	_currentSelection = NULL;

	_ui.treeWidget->clear();
	_ui.tableWidget->clear();

	_ui.tableWidget->setColumnCount(3);
	_ui.tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Attribute"));
	_ui.tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Type"));
	_ui.tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Value"));

	if ( _object == NULL ) return;

	TreeItem *item = new TreeItem(_ui.treeWidget, _object);
	item->setText(0, _object->className());
	item->setText(1, _object->className());

	addObject(item);

	_ui.treeWidget->setCurrentItem(item);
	_ui.treeWidget->setFocus();
}


void Inspector::selectObject(Core::BaseObject *obj) {
	if ( obj == NULL ) return;

	for ( int i = 0; i < _ui.treeWidget->topLevelItemCount(); ++i ) {
		TreeItem *item = static_cast<TreeItem*>(_ui.treeWidget->topLevelItem(i));
		if ( item->object() == obj ) {
			_ui.treeWidget->setCurrentItem(item);
			return;
		}

		selectObject(item, obj);
	}
}


void Inspector::selectObject(QTreeWidgetItem *parent, Core::BaseObject *obj) {
	for ( int i = 0; i < parent->childCount(); ++i ) {
		TreeItem *item = static_cast<TreeItem*>(parent->child(i));
		if ( item->object() == obj ) {
			_ui.treeWidget->setCurrentItem(item);
			return;
		}

		selectObject(item, obj);
	}
}


void Inspector::addObject(QTreeWidgetItem *p) {
	TreeItem *parent = static_cast<TreeItem*>(p);
	BaseObject *obj = parent->object();

	const MetaObject *meta = obj->meta();
	if ( meta == NULL ) return;

	PublicObject *po = PublicObject::Cast(obj);
	if ( po )
		parent->setText(0, po->publicID().c_str());

	std::string indexName;

	for ( size_t i = 0; i < meta->propertyCount(); ++i ) {
		const MetaProperty *prop = meta->property(i);

		if ( prop->isIndex() ) {
			std::string strProp = propToString(prop, obj);
			if ( !strProp.empty() ) {
				if ( !indexName.empty() ) indexName += "_";
				indexName += strProp;
			}
		}

		// Attributes
		if ( !prop->isClass() ) continue;

		if ( !prop->isArray() ) {
			try {
				BaseObject *child = boost::any_cast<BaseObject*>(prop->read(obj));

				TreeItem *item = new TreeItem(parent, child);

				item->setText(0, prop->name().c_str());
				item->setText(1, prop->type().c_str());

				addObject(item);
			}
			catch ( ... ) {
				TreeItem *item = new TreeItem(parent, NULL);

				item->setText(0, prop->name().c_str());
				item->setText(1, prop->type().c_str());

				item->setForeground(0, Qt::gray);
				item->setForeground(1, Qt::gray);
			}
		}
		else {
			int cnt = prop->arrayElementCount(obj);

			if ( cnt > 0 ) {
				TreeItem *item = new TreeItem(parent, NULL);

				item->setText(0, (prop->name() + "s").c_str());
				item->setText(1, ("array<" + prop->type() + ">").c_str());

				QFont f = item->font(0); f.setItalic(true); item->setFont(0, f);
				f = item->font(1); f.setItalic(true); item->setFont(1, f);

				for ( int c = 0; c < cnt; ++c ) {
					BaseObject *child = prop->arrayObject(obj, c);
					TreeItem *sub_item = new TreeItem(item, child);

					sub_item->setText(0, prop->name().c_str());
					sub_item->setText(1, QString("%1 #%2").arg(prop->type().c_str()).arg(c));

					addObject(sub_item);
				}
			}
		}
	}

	if ( !po && !indexName.empty() )
		parent->setText(0, indexName.c_str());
}


void Inspector::selectionChanged() {
	while ( _ui.tableWidget->rowCount() )
		_ui.tableWidget->removeRow(0);

	TreeItem *item = static_cast<TreeItem*>(_ui.treeWidget->currentItem());
	if ( item == NULL ) return;

	_currentSelection = item->object();
	if ( _currentSelection == NULL ) return;

	const MetaObject *meta = _currentSelection->meta();
	if ( meta == NULL ) return;

	PublicObject *po = PublicObject::Cast(_currentSelection);
	if ( po )
		addProperty("publicID", "string", po->publicID(), true, false);

	for ( size_t i = 0; i < meta->propertyCount(); ++i ) {
		const MetaProperty *prop = meta->property(i);

		if ( prop->isClass() ) continue;

		addProperty(prop->name(), prop->type(), propToString(prop, _currentSelection),
		            prop->isIndex(), prop->isOptional(), prop->isReference());
	}
}


void Inspector::addProperty(const std::string &name, const std::string &type,
                            const std::string &value, bool isIndex,
                            bool isOptional, bool isReference) {
	QTableWidgetItem *item0 = new QTableWidgetItem(name.c_str());
	QTableWidgetItem *item1 = new QTableWidgetItem(type.c_str());
	QTableWidgetItem *item2 = NULL;
	QLabel           *link  = NULL;

	if ( isReference && !value.empty() ) {
		link = new QLabel;
		link->setText(QString("<a href=\"%1\">%2</a>").arg(value.c_str()).arg(value.c_str()));
		link->setMargin(4);
		if ( PublicObject::Find(value) == NULL ) {
			link->setEnabled(false);
			link->setToolTip(QString("The link is inactive because %1 is not available.").arg(value.c_str()));
		}
		else {
			link->setToolTip(value.c_str());
			connect(link, SIGNAL(linkActivated(QString)),
			        this, SLOT(linkClicked(QString)));
		}
	}
	else
		item2 = new QTableWidgetItem(value.c_str());

	if ( isIndex ) {
		QFont f = item0->font(); f.setBold(true); item0->setFont(f);
		f = item1->font(); f.setBold(true); item1->setFont(f);
		if ( item2 != NULL ) {
			f = item2->font(); f.setBold(true); item2->setFont(f);
		}
		if ( link != NULL ) {
			f = link->font(); f.setBold(true); link->setFont(f);
		}
	}
	else if ( isOptional && value.empty() ) {
		item0->setForeground(Qt::gray);
		item1->setForeground(Qt::gray);
		if ( item2 != NULL )
			item2->setForeground(Qt::gray);
	}

	item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);
	item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
	if ( item2 != NULL )
		item2->setFlags(item2->flags() & ~Qt::ItemIsEditable);

	int row = _ui.tableWidget->rowCount();
	_ui.tableWidget->insertRow(row);
	_ui.tableWidget->setItem(row, 0, item0);
	_ui.tableWidget->setItem(row, 1, item1);
	if ( item2 != NULL ) _ui.tableWidget->setItem(row, 2, item2);
	if ( link != NULL ) _ui.tableWidget->setCellWidget(row, 2, link);
}


void Inspector::linkClicked(QString id) {
	PublicObject *po = PublicObject::Find(id.toStdString());
	if ( !po ) return;

	/*
	Inspector *w = new Inspector(this, Qt::Tool);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setObject(po);
	w->show();
	*/

	if ( po == _object ) return;

	_history.push(State(_object, _currentSelection));
	BaseObject *obj = _currentSelection != NULL?_currentSelection:_object;
	PublicObject *posrc = PublicObject::Cast(obj);
	if ( posrc )
		_ui.buttonBack->setToolTip(QString("Object %1 of type %2 (Back)").arg(posrc->publicID().c_str()).arg(obj->className()));
	else
		_ui.buttonBack->setToolTip(QString("Object of type %1 (Back)").arg(obj->className()));
	_ui.buttonBack->setEnabled(true);

	setObject(po);
}


void Inspector::back() {
	if ( _history.empty() ) return;

	State state = _history.pop();
	setObject(state.first);
	selectObject(state.second);

	if ( _history.empty() ) _ui.buttonBack->setEnabled(false);
}


}
}
