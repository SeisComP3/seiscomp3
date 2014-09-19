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

#include "modules.h"
#include "../fancyview.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QAction>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::System;


namespace {

QTreeWidgetItem *createPath(QTreeWidget *tree, const QString &path, bool expand) {
	QTreeWidgetItem *parent = NULL;
	QStringList dirs = path.split('/', QString::SkipEmptyParts);

	foreach ( const QString &dir, dirs ) {
		QTreeWidgetItem *node = NULL;
		if ( parent == NULL ) {
			for ( int i = 0; i < tree->topLevelItemCount(); ++i ) {
				if ( tree->topLevelItem(i)->text(0) == dir ) {
					node = tree->topLevelItem(i);
					break;
				}
			}

			if ( node == NULL )
				node = new QTreeWidgetItem(tree, QStringList() << dir, 0);
		}
		else {
			for ( int i = 0; i < parent->childCount(); ++i ) {
				if ( parent->child(i)->text(0) == dir ) {
					node = parent->child(i);
					break;
				}
			}

			if ( node == NULL )
				node = new QTreeWidgetItem(parent, QStringList() << dir, 0);
		}

		if ( expand )
			tree->expandItem(node);

		QFont boldFont = tree->font();
		boldFont.setBold(true);
		node->setData(0, Qt::FontRole, boldFont);
		node->setData(0, Qt::DecorationRole, tree->style()->standardIcon(QStyle::SP_DirLinkIcon));
		node->setFlags(node->flags() & ~Qt::ItemIsSelectable);
		node->setToolTip(0, "");
		parent = node;
	}

	return parent;
}

}

ModulesPanel::ModulesPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	_name = "Modules";
	_icon = QIcon(":/res/icons/config.png");
	setDescription("Configuration of module parameters");
	setHeadline("Configuration");

	QWidget *configurationPanel = this;
	configurationPanel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));
	//configurationPanel->setAutoFillBackground(true);
	QHBoxLayout *configurationLayout = new QHBoxLayout;
	configurationLayout->setMargin(0);
	configurationLayout->setSpacing(1);
	configurationPanel->setLayout(configurationLayout);

	QSplitter *configPanelSplitter = new QSplitter;
	configPanelSplitter->setHandleWidth(1);
	configPanelSplitter->setChildrenCollapsible(false);
	configurationLayout->addWidget(configPanelSplitter);

	_moduleTree = new QTreeWidget;
	_moduleTree->setFrameShape(QFrame::NoFrame);
	_moduleTree->setAutoFillBackground(true);
	configPanelSplitter->addWidget(_moduleTree);

	connect(_moduleTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	        this, SLOT(moduleSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	/*
	connect(_moduleTree, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
	        this, SLOT(moduleSelected(QTreeWidgetItem*,int)));
	*/

	/*
	QTreeView *treeView = new QTreeView;
	treeView->setFrameShape(QFrame::NoFrame);
	treeView->setAutoFillBackground(true);
	treeView->setAlternatingRowColors(true);

	treeView->setItemDelegate(new ConfigurationTreeItemItemDelegate(this));
	treeView->setSortingEnabled(true);
	treeView->sortByColumn(0, Qt::AscendingOrder);

	_moduleView = treeView;
	*/
	//FancyWidget *fancyView = new FancyWidget;
	FancyView *fancyView = new FancyView;
	_moduleView = fancyView;

	QWidget *settingsPanel = new QWidget;
	settingsPanel->setAutoFillBackground(true);
	QVBoxLayout *settingsLayout = new QVBoxLayout;
	settingsLayout->setMargin(0);
	settingsPanel->setLayout(settingsLayout);

	QSizePolicy sp = settingsPanel->sizePolicy();
	sp.setHorizontalStretch(1);
	settingsPanel->setSizePolicy(sp);

	QVBoxLayout *moduleViewLayout = new QVBoxLayout;

	_searchWidget = new QWidget;
	QHBoxLayout *searchLayout = new QHBoxLayout;
	_searchWidget->setLayout(searchLayout);
	QLabel *labelSearch = new QLabel;
	labelSearch->setText("Search parameter:");
	labelSearch->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred));
	QLineEdit *search = new QLineEdit;
	connect(search, SIGNAL(textEdited(const QString &)),
	        this, SLOT(search(const QString &)));
	connect(search, SIGNAL(returnPressed()), this, SLOT(search()));
	QPushButton *searchClose = new QPushButton;
	searchClose->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
	searchClose->setFixedSize(18,18);

	searchLayout->setMargin(8);
	searchLayout->addWidget(labelSearch);
	searchLayout->addWidget(search);
	searchLayout->addWidget(searchClose);

	connect(searchClose, SIGNAL(clicked()), this, SLOT(closeSearch()));

	QAction *activateSearch = new QAction(this);
	activateSearch->setShortcut(QKeySequence("Ctrl+f"));
	addAction(activateSearch);
	connect(activateSearch, SIGNAL(triggered()), _searchWidget, SLOT(show()));
	connect(activateSearch, SIGNAL(triggered()), search, SLOT(setFocus()));

	QAction *closeSearch = new QAction(_searchWidget);
	closeSearch->setShortcut(QKeySequence("Esc"));
#if QT_VERSION >= 0x040400
	closeSearch->setShortcutContext(Qt::WidgetWithChildrenShortcut);
#endif
	_searchWidget->addAction(closeSearch);
	connect(closeSearch, SIGNAL(triggered()), this, SLOT(closeSearch()));

	_searchWidget->hide();
	moduleViewLayout->addWidget(_searchWidget);
	moduleViewLayout->addWidget(_moduleView);

	settingsLayout->addLayout(moduleViewLayout);
	configPanelSplitter->addWidget(settingsPanel);

	_modified = false;
}


void ModulesPanel::setModel(ConfigurationTreeItemModel *model) {
	ConfiguratorPanel::setModel(model);

	((FancyView*)_moduleView)->setConfigStage(model->configStage());
	_moduleView->setModel(_model);

	while ( _moduleTree->topLevelItemCount() > 0 )
		delete _moduleTree->takeTopLevelItem(0);

	_moduleTree->setColumnCount(1);
	_moduleTree->header()->hide();

	QFont boldFont = _moduleTree->font();
	boldFont.setBold(true);
	QFont italicFont = _moduleTree->font();
	italicFont.setItalic(true);

	System::Model *base = model->model();

	QTreeWidgetItem *firstModule = NULL;
	QTreeWidgetItem *emptyItem = NULL;

	for ( size_t i = 0; i < base->modules.size(); ++i ) {
		if ( !base->modules[i]->hasConfiguration() ) continue;
		if ( !base->modules[i]->definition->category.empty() ) continue;

		if ( emptyItem == NULL ) {
			emptyItem = new QTreeWidgetItem(_moduleTree, QStringList() << "<empty>", 0);
			emptyItem->setToolTip(0, "");
			emptyItem->setData(0, Qt::FontRole, boldFont);
			emptyItem->setData(0, Qt::DecorationRole, style()->standardIcon(QStyle::SP_DirLinkIcon));
			emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
		}

		QTreeWidgetItem *mitem = new QTreeWidgetItem(emptyItem, QStringList() << base->modules[i]->definition->name.c_str(), 1);
		mitem->setData(0, Qt::DecorationRole, style()->standardIcon(QStyle::SP_FileLinkIcon));
		if ( base->modules[i]->definition->aliasedModule != NULL ) {
			mitem->setData(0, Qt::DecorationRole, QIcon(":/res/icons/document_link.png"));
			mitem->setData(0, Qt::FontRole, italicFont);
			mitem->setToolTip(0, tr("This is an alias for %1").arg(base->modules[i]->definition->aliasedModule->name.c_str()));
		}
		else {
			mitem->setData(0, Qt::DecorationRole, QIcon(":/res/icons/document.png"));
			mitem->setToolTip(0, "");
		}

		if ( firstModule == NULL ) firstModule = mitem;
	}

	if ( emptyItem )
		_moduleTree->expandItem(emptyItem);

	System::Model::Categories::iterator it;
	for ( it = base->categories.begin(); it != base->categories.end(); ++it ) {
		QTreeWidgetItem *citem = createPath(_moduleTree, it->c_str(), true);

		for ( size_t i = 0; i < base->modules.size(); ++i ) {
			if ( !base->modules[i]->hasConfiguration() ) continue;
			if ( base->modules[i]->definition->category != *it ) continue;
			QTreeWidgetItem *mitem = new QTreeWidgetItem(citem, QStringList() << base->modules[i]->definition->name.c_str(), 1);
			//mitem->setData(0, Qt::DecorationRole, style()->standardIcon(QStyle::SP_FileLinkIcon));
			if ( base->modules[i]->definition->aliasedModule != NULL ) {
				mitem->setData(0, Qt::DecorationRole, QIcon(":/res/icons/document_link.png"));
				mitem->setData(0, Qt::FontRole, italicFont);
				mitem->setToolTip(0, tr("This is an alias for %1").arg(base->modules[i]->definition->aliasedModule->name.c_str()));
			}
			else {
				mitem->setData(0, Qt::DecorationRole, QIcon(":/res/icons/document.png"));
				mitem->setToolTip(0, "");
			}

			if ( firstModule == NULL ) firstModule = mitem;
		}
	}

	if ( firstModule != NULL ) {
		_moduleTree->setCurrentItem(firstModule);
		moduleSelected(firstModule, 0);
	}

	_modified = false;
}


void ModulesPanel::moduleSelectionChanged(QTreeWidgetItem *curr,QTreeWidgetItem*) {
	moduleSelected(curr,0);
}


void ModulesPanel::moduleSelected(QTreeWidgetItem *item,int) {
	if ( item->type() != 1 ) return;

	int rows = _model->rowCount();
	for ( int i = 0; i < rows; ++i ) {
		QModelIndex idx = _model->index(i,0);
		if ( idx.data(ConfigurationTreeItemModel::Type).toInt() !=
		     ConfigurationTreeItemModel::TypeModule ) continue;

		if ( idx.data().toString() == item->text(0) ) {
			moduleChanged(idx);
			break;
		}
	}
}


void ModulesPanel::moduleChanged(const QModelIndex &index) {
	if ( index.data(ConfigurationTreeItemModel::Type).toInt() !=
	     ConfigurationTreeItemModel::TypeModule ) return;
	Module *mod = reinterpret_cast<Module*>(index.data(ConfigurationTreeItemModel::Link).value<void*>());

	//static_cast<FancyWidget*>(_moduleView)->setModel(mod);
	_moduleView->setRootIndex(index);

	setHeadline("Configuration / " + index.data(Qt::DisplayRole).toString());

	if ( mod )
		setDescription(mod->definition->description.c_str());
	else
		setDescription("...");

	/*
	if ( _listWidget->currentItem() == _configItem ) {
		_headline->setText(_configItem->data(Qt::UserRole+1).toString());
		_description->setText(firstLine(_configItem->data(Qt::UserRole+2).toString()));
		_description->setToolTip(multiline(_configItem->data(Qt::UserRole+2).toString(), 80));
	}
	*/
}


void ModulesPanel::search(const QString &text) {
	QModelIndexList hits;

	QModelIndex idx = _moduleView->rootIndex();
	QAbstractItemModel *model = _moduleView->model();

	if ( text.isEmpty() ) {
		_moduleView->scrollTo(idx);
		_moduleView->setCurrentIndex(idx);
		return;
	}

	int rows = model->rowCount(idx);

	for ( int i = 0; i < rows; ++i ) {
		hits = model->match(idx.child(i,0), Qt::DisplayRole, text, 1,
	                         Qt::MatchStartsWith |
	                         Qt::MatchRecursive |
	                         Qt::MatchWrap);

		if ( hits.isEmpty() ) continue;
		_moduleView->scrollTo(hits.first());
		_moduleView->setCurrentIndex(hits.first());
		break;
	}
}


void ModulesPanel::search() {
	search(static_cast<QLineEdit*>(sender())->text());
}


void ModulesPanel::closeSearch() {
	_moduleView->setCurrentIndex(_moduleView->rootIndex());
	_searchWidget->hide();
}
