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


#include "bindings.h"
#include "../fancyview.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDragEnterEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QPushButton>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>


using namespace std;
using namespace Seiscomp::System;


enum StationRoles {
	Type = Qt::UserRole,
	Link = Qt::UserRole + 1,
	Net  = Qt::UserRole + 2,
	Sta  = Qt::UserRole + 3
};

enum Type {
	TypeUndefined = 0,
	TypeRoot,
	TypeNetwork,
	TypeStation,
	TypeModule,
	TypeProfile
};


template <typename T>
T *getLink(const QModelIndex &idx) {
	return static_cast<T*>(idx.data(Link).value<void*>());
}

class NameValidator : public QValidator {
	public:
		NameValidator(QWidget *parent)
		: QValidator(parent) {}

		State validate(QString &txt, int& pos) const {
			txt = txt.toUpper();
			return Acceptable;
		}
};

class NewNameDialog : public QDialog {
	public:
		NewNameDialog(const QModelIndex &root, bool forceUpper, QWidget *parent = 0)
			: QDialog(parent), _root(root) {
			QVBoxLayout *layout = new QVBoxLayout;
			setLayout(layout);

			QHBoxLayout *hlayout = new QHBoxLayout;
			QLabel *label = new QLabel("Name:");
			hlayout->addWidget(label);
			_name = new QLineEdit;
			if ( forceUpper )
				_name->setValidator(new NameValidator(this));
			hlayout->addWidget(_name);
			layout->addLayout(hlayout);

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			QPushButton *ok = new QPushButton("Ok");
			hlayout->addWidget(ok);
			QPushButton *cancel = new QPushButton("Cancel");
			hlayout->addWidget(cancel);

			layout->addLayout(hlayout);

			connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		}

		QString name() const {
			return _name->text();
		}

		void accept() {
			int rows = _root.model()->rowCount(_root);

			if ( _name->text().isEmpty() ) {
				QMessageBox::critical(NULL, "Empty name",
				                      "Empty names are not allowed. ");
				return;
			}

			for ( int i = 0; i < rows; ++i ) {
				if ( _root.child(i,0).data().toString() == _name->text() ) {
					QMessageBox::critical(NULL, "Duplicate name",
					                      "The name exists already and duplicate "
					                      "names are not allowed.");
					return;
				}
			}

			QDialog::accept();
		}

	private:
		QLineEdit   *_name;
		QModelIndex  _root;
};


class StationTreeView : public QTreeView {
	public:
		StationTreeView(BindingsPanel *panel) : _panel(panel) {
			setAcceptDrops(true);
		}

		Qt::DropActions supportedDropActions() const {
			return Qt::LinkAction;
		}

		void dragEnterEvent(QDragEnterEvent *event) {
			bool accepted = false;

			if ( event->mimeData() && event->mimeData()->hasText() ) {
				accepted = true;

				// Validate text content
				QStringList lines = event->mimeData()->text().split("\n", QString::SkipEmptyParts);
				foreach ( const QString &l, lines ) {
					// Do not accept unknown text
					if ( !l.startsWith("PROFILE ") ) {
						accepted = false;
						break;
					}
				}

				if ( accepted ) {
					_followSelection = selectionModel()->selectedRows().count() <= 1;
				}
			}

			event->setAccepted(accepted);
		}

		void dragMoveEvent(QDragMoveEvent *event) {
			if ( rootIndex().data(Type).toInt() == TypeStation ) {
				event->accept();
				return;
			}

			QModelIndex idx = indexAt(event->pos());

			if ( idx.isValid() ) {
				if ( _followSelection ) {
					selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
					event->setAccepted(true);
				}
				else
					event->setAccepted(selectionModel()->isSelected(idx));
			}
			else
				event->setAccepted(false);
		}

		void dropEvent(QDropEvent * event) {
			QStringList lines = event->mimeData()->text().split("\n", QString::SkipEmptyParts);
			foreach ( const QString &l, lines ) {
				if ( !l.startsWith("PROFILE ") ) continue;
				QString module,profile;
				QStringList toks = l.mid(8).split('/');
				if ( toks.size() != 2 ) continue;
				module = toks[0];
				profile = toks[1];

				foreach ( const QModelIndex &i, selectionModel()->selectedIndexes() )
					_panel->assignProfile(i, module, profile);
			}
		}

	private:
		BindingsPanel  *_panel;
		bool            _followSelection;
};


class StationsFolderView : public QListView {
	public:
		StationsFolderView(BindingsPanel *panel) : _panel(panel) {
			setAcceptDrops(true);
		}

		Qt::DropActions supportedDropActions() const {
			return Qt::LinkAction;
		}

		void dragEnterEvent(QDragEnterEvent *event) {
			/*
			event->setAccepted(event->mimeData() &&
			                   event->mimeData()->hasText() &&
			                   (selectionModel()->hasSelection() ||
			                    rootIndex().data(Type).toInt() == TypeStation));
			*/
			bool accepted = false;

			if ( event->mimeData() && event->mimeData()->hasText() ) {
				accepted = true;

				// Validate text content
				QStringList lines = event->mimeData()->text().split("\n", QString::SkipEmptyParts);
				foreach ( const QString &l, lines ) {
					// Do not accept unknown text
					if ( !l.startsWith("PROFILE ") ) {
						accepted = false;
						break;
					}
				}

				if ( accepted )
					_selectedItems = selectionModel()->selectedIndexes();
			}

			event->setAccepted(accepted);
		}

		void dragLeaveEvent(QDragLeaveEvent *event) {
			_selectedItems.clear();
		}

		void dragMoveEvent(QDragMoveEvent *event) {
			if ( rootIndex().data(Type).toInt() == TypeStation ) {
				event->accept();
				return;
			}

			QModelIndex idx = indexAt(event->pos());

			if ( idx.isValid() )
				selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
			else {
				selectionModel()->clear();
				foreach ( const QModelIndex &i, _selectedItems )
					selectionModel()->select(i, QItemSelectionModel::Select);
			}

			event->setAccepted(selectionModel()->hasSelection());
		}

		void dropEvent(QDropEvent * event) {
			QStringList lines = event->mimeData()->text().split("\n", QString::SkipEmptyParts);
			foreach ( const QString &l, lines ) {
				if ( !l.startsWith("PROFILE ") ) continue;
				QString module,profile;
				QStringList toks = l.mid(8).split('/');
				if ( toks.size() != 2 ) continue;
				module = toks[0];
				profile = toks[1];

				if ( rootIndex().data(Type).toInt() == TypeStation )
					_panel->assignProfile(rootIndex(), module, profile);
				else {
					foreach ( const QModelIndex &i, selectionModel()->selectedIndexes() )
						_panel->assignProfile(i, module, profile);
				}
			}

			_selectedItems.clear();
		}


	private:
		BindingsPanel  *_panel;
		QModelIndexList _selectedItems;
};


class ProfilesModel : public QStandardItemModel {
	public:
		ProfilesModel(QObject *parent = 0) : QStandardItemModel(parent) {
		}

	public:
		QMimeData *mimeData(const QModelIndexList &indexes) const {
			QString text;
			foreach ( const QModelIndex &i, indexes ) {
				if ( i.data(Type).toInt() != TypeProfile ) continue;
				ModuleBinding *b = getLink<ModuleBinding>(i);
				Module *mod = (Module*)b->parent;

				if ( !text.isEmpty() ) text += "\n";
				text += QString("PROFILE %1/%2")
				        .arg(mod->definition->name.c_str())
				        .arg(b->name.c_str());
			}

			if ( text.isEmpty() ) return NULL;

			QMimeData *data = new QMimeData;
			data->setText(text);
			return data;
		}

		Qt::DropActions supportedDragActions() {
			return Qt::LinkAction;
		}
};


BindingsViewDelegate::BindingsViewDelegate(QObject *parent) : QItemDelegate(parent) {}

QWidget *BindingsViewDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
	if ( index.column() != 1 ) return NULL;
	if ( index.data(Type).toInt() != TypeProfile ) return NULL;

	ModuleBinding *binding = getLink<ModuleBinding>(index.sibling(index.row(),0));
	if ( binding == NULL ) return NULL;

	Module *mod = static_cast<Module*>(binding->parent);
	if ( mod == NULL ) return NULL;

	QComboBox *editor = new QComboBox(parent);
	editor->addItem("");

	Module::Profiles::iterator it;
	for ( it = mod->profiles.begin(); it != mod->profiles.end(); ++it )
		editor->addItem((*it)->name.c_str());

	connect(editor, SIGNAL(currentIndexChanged(const QString &)),
	        this, SLOT(profileChanged(const QString &)));

	return editor;
}

void BindingsViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	QString value = index.model()->data(index, Qt::EditRole).toString();
	QComboBox *cBox = static_cast<QComboBox*>(editor);
	cBox->setCurrentIndex(cBox->findText(value));
}

void BindingsViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const {
	QComboBox *cBox = static_cast<QComboBox*>(editor);
	QString value = cBox->currentText();

	QModelIndex sibling = index.sibling(index.row(),0);
	ModuleBinding *binding = getLink<ModuleBinding>(sibling);
	Module *mod = static_cast<Module*>(binding->parent);

	StationID id(sibling.data(Net).toString().toStdString(),
	             sibling.data(Sta).toString().toStdString());

	// Skip, if we changed nothing
	if ( binding && binding->name == value.toStdString() )
		return;

	binding = mod->bind(id, value.toStdString());
	if ( binding == NULL ) {
		setEditorData(cBox, index);
		QMessageBox::critical(NULL, "Change profile",
		                            "Changing the profile failed.");
		return;
	}

	model->setData(sibling, qVariantFromValue((void*)binding), Link);
	model->setData(index, value, Qt::EditRole);
}

QSize BindingsViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index ) const {
	QSize hint = QItemDelegate::sizeHint(option, index);

	if ( index.column() != 1 ) return hint;

	if ( index.data(Type).toInt() != TypeProfile ) return hint;

	return QSize(hint.width(), hint.height()*150/100);
}

void BindingsViewDelegate::updateEditorGeometry(QWidget *editor,
                                                const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const {
	editor->setGeometry(option.rect);
	//editor->move(option.rect.topLeft());
}


void BindingsViewDelegate::profileChanged(const QString &text) {
	emit commitData(static_cast<QWidget*>(sender()));
}


BindingView::BindingView(QWidget *parent) : QWidget(parent), _model(NULL) {
	_bindingModel = new ConfigurationTreeItemModel(this);
	_view = new FancyView(this);
	_view->setAutoFillBackground(true);

	_header = new QWidget;
	QPalette pal = _header->palette();
	pal.setColor(QPalette::Window, Qt::white);
	pal.setColor(QPalette::WindowText, Qt::gray);
	_header->setPalette(pal);

	QFont f = _header->font();
	f.setPointSize(f.pointSize()*150/100);
	f.setBold(true);
	_header->setFont(f);

	_header->setAutoFillBackground(true);
	_header->hide();

	QHBoxLayout *hlayout = new QHBoxLayout;
	_header->setLayout(hlayout);

	_icon = new QLabel;
	hlayout->addWidget(_icon);
	hlayout->addStretch();

	_label = new QLabel;
	hlayout->addWidget(_label);

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	l->setSpacing(1);
	setLayout(l);
	l->addWidget(_header);
	l->addWidget(_view);
}


void BindingView::clear() {
	_bindingModel->clear();
	_view->setModel(NULL);
	_label->setText("");
	_icon->setPixmap(QPixmap());
	_rootIndex = QModelIndex();
	_header->hide();
}


void BindingView::setModel(ConfigurationTreeItemModel *base,
                           QAbstractItemModel *model) {
	if ( model == _model ) return;

	if ( _model )
		_model->disconnect(this);

	_bindingModel->disconnect();

	_model = model;

	if ( _model ) {
		connect(_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
		        this, SLOT(dataChanged(QModelIndex,QModelIndex)));
		connect(_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
		        this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
	}

	if ( base )
		connect(_bindingModel, SIGNAL(modificationChanged(bool)),
		        base, SLOT(setModified(bool)));
}


void BindingView::setRootIndex(const QModelIndex &index) {
	clear();

	//int type = index.data(Type).toInt();
	//if ( type != TypeModule ) return;

	_rootIndex = index;

	ModuleBinding *b = getLink<ModuleBinding>(index);
	Module *mod = (Module*)b->parent;
	_bindingModel->setModel(b);
	_view->setModel(_bindingModel);
	_view->setRootIndex(_bindingModel->index(0,0));

	QIcon icon = _rootIndex.data(Qt::DecorationRole).value<QIcon>();
	_icon->setPixmap(icon.pixmap(32,32));

	if ( b->name.empty() )
		_label->setText(QString("%1/%2.%3")
		                .arg(mod->definition->name.c_str())
		                .arg(index.data(Net).toString())
		                .arg(index.data(Sta).toString()));
	else
		_label->setText(QString("%1/%2")
		                .arg(mod->definition->name.c_str())
		                .arg(b->name.c_str()));

	_header->show();
}


void BindingView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
	if ( _rootIndex.parent() == topLeft.parent() &&
	     _rootIndex.row() == topLeft.row() &&
	     topLeft.data(Type).toInt() == TypeProfile )
		// Profile changed to my binding
		setRootIndex(topLeft.sibling(topLeft.row(), 0));
}


void BindingView::rowsRemoved(const QModelIndex &parent, int start, int end) {
	if ( !_rootIndex.isValid() ) return;

	QModelIndex p = _rootIndex.parent();
	QModelIndex i = _rootIndex;

	while ( p.isValid() ) {
		if ( parent == p && start <= i.row() && i.row() <= end ) {
			clear();
			return;
		}

		i = p;
		p = p.parent();
	}
}


BindingsPanel::BindingsPanel(QWidget *parent)
: ConfiguratorPanel(false, parent), _bindingsModel(NULL), _profilesModel(NULL) {
	_name = "Bindings";
	_icon = QIcon(":/res/icons/bindings.png");
	setHeadline("Bindings");
	setDescription("Configuration of module-station bindings and binding profiles.");

	_docIcon = QIcon(":/res/icons/document.png");
	_linkIcon = QIcon(":/res/icons/document_link.png");
	_docFolder = QIcon(":/res/icons/document_folder.png");

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	setLayout(l);

	QSplitter *splitter = new QSplitter;
	splitter->setHandleWidth(1);
	splitter->setChildrenCollapsible(false);
	l->addWidget(splitter);

	_stationsTreeView = new StationTreeView(this);
	_stationsTreeView->setFrameShape(QFrame::NoFrame);
	_stationsTreeView->setAutoFillBackground(true);
	static_cast<QTreeView*>(_stationsTreeView)->setRootIsDecorated(false);

	_stationsTreeView->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	_stationsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QWidget *folderView = new QWidget(this);
	folderView->setAutoFillBackground(true);
	QVBoxLayout *fl = new QVBoxLayout;
	fl->setMargin(0);
	fl->setSpacing(0);
	folderView->setLayout(fl);

	_stationsFolderView = new StationsFolderView(this);
	_stationsFolderView->setFrameShape(QFrame::NoFrame);
	_stationsFolderView->setResizeMode(QListView::Adjust);
	_stationsFolderView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_stationsFolderView->setContextMenuPolicy(Qt::CustomContextMenu);

	switchToStationsIconView();

	connect(_stationsFolderView, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(folderViewContextMenu(const QPoint&)));

	QToolBar *folderViewTools = new QToolBar;
	folderViewTools->setIconSize(QSize(16,16));
	folderViewTools->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	_folderLevelUp = folderViewTools->addAction("Up");
	_folderLevelUp->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
	_folderLevelUp->setEnabled(false);
	connect(_folderLevelUp, SIGNAL(triggered(bool)), this, SLOT(folderLevelUp()));

	_deleteItem = folderViewTools->addAction("Delete");
	_deleteItem->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	connect(_deleteItem, SIGNAL(triggered(bool)), this, SLOT(deleteItem()));

	QAction *a = folderViewTools->addAction("Icons");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToStationsIconView()));
	a = folderViewTools->addAction("List");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToStationsListView()));

	fl->addWidget(folderViewTools);
	fl->addWidget(_stationsFolderView);

	QWidget *container = new QWidget;
	QVBoxLayout *modulesFolderLayout = new QVBoxLayout;
	modulesFolderLayout->setMargin(0);
	modulesFolderLayout->setSpacing(0);
	container->setAutoFillBackground(true);
	container->setLayout(modulesFolderLayout);

	_bindingView = new BindingView;
	QSizePolicy sp = _bindingView->sizePolicy();
	sp.setHorizontalStretch(1);
	_bindingView->setSizePolicy(sp);

	folderViewTools = new QToolBar;
	folderViewTools->setIconSize(QSize(16,16));
	folderViewTools->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	_addProfile = folderViewTools->addAction(tr("Add profile"));
	_addProfile->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
	_addProfile->setEnabled(false);
	connect(_addProfile, SIGNAL(triggered(bool)), this, SLOT(addProfile()));

	_deleteProfile = folderViewTools->addAction(tr("Delete"));
	_deleteProfile->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	_deleteProfile->setEnabled(false);
	connect(_deleteProfile, SIGNAL(triggered(bool)), this, SLOT(deleteProfile()));

	a = folderViewTools->addAction("Icons");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToProfileIconView()));
	a = folderViewTools->addAction("List");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToProfileListView()));

	QSplitter *splitter2 = new QSplitter(Qt::Vertical);
	splitter2->setHandleWidth(1);
	splitter2->addWidget(_stationsTreeView);
	splitter2->addWidget(folderView);

	QTreeView *tree = new QTreeView;
	//tree->header()->hide();
	_modulesView = tree;
	_modulesView->setAutoFillBackground(true);
	_modulesView->setDragEnabled(true);
	_modulesView->setFrameShape(QFrame::NoFrame);
	_modulesView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_modulesView->setContextMenuPolicy(Qt::CustomContextMenu);
	_modulesView->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(_modulesView, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(modulesViewContextMenu(const QPoint&)));

	_modulesFolderView = new QListView;
	_modulesFolderView->setResizeMode(QListView::Adjust);
	_modulesFolderView->setFrameShape(QFrame::NoFrame);
	_modulesFolderView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_modulesFolderView->setContextMenuPolicy(Qt::CustomContextMenu);
	_modulesFolderView->setEnabled(false);

	connect(_modulesFolderView, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(modulesFolderViewContextMenu(const QPoint&)));

	switchToProfileIconView();

	modulesFolderLayout->addWidget(folderViewTools);
	modulesFolderLayout->addWidget(_modulesFolderView);

	QSplitter *splitter3 = new QSplitter(Qt::Vertical);
	splitter3->setHandleWidth(1);
	splitter3->addWidget(_modulesView);
	splitter3->addWidget(container);

	splitter->addWidget(splitter2);
	splitter->addWidget(_bindingView);
	splitter->addWidget(splitter3);

	connect(_stationsTreeView, SIGNAL(clicked(const QModelIndex &)),
	        this, SLOT(bindingActivated(const QModelIndex &)));
	connect(_stationsTreeView, SIGNAL(doubleClicked(const QModelIndex &)),
	        this, SLOT(bindingDoubleClicked(const QModelIndex &)));

	connect(_stationsFolderView, SIGNAL(activated(const QModelIndex &)),
	        this, SLOT(changeFolder(const QModelIndex &)));
	connect(_stationsFolderView, SIGNAL(doubleClicked(const QModelIndex &)),
	        this, SLOT(bindingDoubleClicked(const QModelIndex &)));

	connect(_modulesView, SIGNAL(activated(const QModelIndex &)),
	        this, SLOT(profileActivated(const QModelIndex &)));

	connect(_modulesView, SIGNAL(doubleClicked(const QModelIndex &)),
	        this, SLOT(profileDoubleClicked(const QModelIndex &)));
	connect(_modulesFolderView, SIGNAL(doubleClicked(const QModelIndex &)),
	        this, SLOT(profileDoubleClicked(const QModelIndex &)));
}


void BindingsPanel::setModel(ConfigurationTreeItemModel *model) {
	ConfiguratorPanel::setModel(model);

	_bindingView->setModel(NULL, NULL);
	_stationsTreeView->setModel(NULL);
	_stationsFolderView->setModel(NULL);
	_modulesView->setModel(NULL);
	_modulesFolderView->setModel(NULL);

	if ( _bindingsModel ) delete _bindingsModel;
	if ( _profilesModel ) delete _profilesModel;

	if ( model == NULL ) return;

	// -------------------------------
	// Create bindings model
	// -------------------------------
	_bindingsModel = new QStandardItemModel(this);
	_bindingsModel->setColumnCount(2);
	_bindingsModel->setHeaderData(0, Qt::Horizontal, "Name");
	_bindingsModel->setHeaderData(1, Qt::Horizontal, "Profile");

	connect(_bindingsModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
	        this, SLOT(moduleBindingsChanged(const QModelIndex &, const QModelIndex &)));

	QStandardItem *root = _bindingsModel->invisibleRootItem();

	Model *base = model->model();

	typedef QVector<ModuleBinding*> Bindings;
	typedef QMap<QString, Bindings> Stations;
	typedef QMap<QString, Stations> Networks;
	Networks networks;

	Model::Stations::iterator it;
	for ( it = base->stations.begin(); it != base->stations.end(); ++it ) {
		networks[it->first.networkCode.c_str()]
		        [it->first.stationCode.c_str()].clear();
	}

	for ( size_t i = 0; i < base->modules.size(); ++i ) {
		Module *mod = base->modules[i].get();

		Module::BindingMap::iterator it;
		for ( it = mod->bindings.begin(); it != mod->bindings.end(); ++it ) {
			// Build binding map
			networks[it->first.networkCode.c_str()]
			        [it->first.stationCode.c_str()].append(it->second.get());
		}
	}

	Networks::iterator nit;
	Stations::iterator sit;
	Bindings::iterator bit;

	QStandardItem *rootItem = new QStandardItem("Networks");
	rootItem->setColumnCount(2);
	rootItem->setData(TypeRoot, Type);
	root->appendRow(rootItem);

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();

	for ( nit = networks.begin(); nit != networks.end(); ++nit ) {
		QString networkCode = nit.key();
		QStandardItem *netItem = new QStandardItem(networkCode);
		netItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled/* | Qt::ItemIsDropEnabled*/);
		netItem->setData(TypeNetwork, Type);
		netItem->setData(_docFolder, Qt::DecorationRole);
		netItem->setColumnCount(2);
		rootItem->appendRow(netItem);

		for ( sit = nit.value().begin(); sit != nit.value().end(); ++sit ) {
			QString stationCode = sit.key();
			QStandardItem *staItem = new QStandardItem(stationCode);
			staItem->setData(TypeStation, Type);
			staItem->setData(_docFolder, Qt::DecorationRole);

			string rcFile = env->installDir();
			rcFile += "/var/lib/rc/station_";
			rcFile += networkCode.toStdString() + "_" + stationCode.toStdString();
			Seiscomp::Config::Config cfg;
			if ( cfg.readConfig(rcFile) ) {
				try {
					staItem->setData(cfg.getString("description").c_str(),
					                 Qt::ToolTipRole);
				}
				catch ( ... ) {}
			}

			staItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled/* | Qt::ItemIsDropEnabled*/);
			netItem->appendRow(staItem);

			for ( bit = sit.value().begin(); bit != sit.value().end(); ++bit ) {
				ModuleBinding *binding = *bit;

				Module *mod = static_cast<Module*>(binding->parent);
				QStandardItem *bindItem = new QStandardItem(mod->definition->name.c_str());
				bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled/* | Qt::ItemIsDropEnabled*/);
				bindItem->setData(networkCode, Net);
				bindItem->setData(stationCode, Sta);
				bindItem->setData(TypeModule, Type);
				bindItem->setData(qVariantFromValue((void*)binding), Link);

				if ( binding->name.empty() )
					bindItem->setData(_docIcon, Qt::DecorationRole);
				else
					bindItem->setData(_linkIcon, Qt::DecorationRole);

				QStandardItem *profileItem = new QStandardItem(binding->name.c_str());
				profileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable/* | Qt::ItemIsDropEnabled*/);
				profileItem->setData(TypeProfile, Type);
				//profileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

				staItem->appendRow(QList<QStandardItem*>() << bindItem << profileItem);
			}
		}
	}

	_bindingView->setModel(model, _bindingsModel);

	_stationsTreeView->setModel(_bindingsModel);
	_stationsTreeView->setRootIndex(root->index());
	_stationsTreeView->setItemDelegate(new BindingsViewDelegate(_stationsTreeView));
	static_cast<QTreeView*>(_stationsTreeView)->expand(rootItem->index());

	_stationsFolderView->setModel(_bindingsModel);
	_stationsFolderView->setRootIndex(rootItem->index());


	// -------------------------------
	// Create profiles model
	// -------------------------------
	_profilesModel = new ProfilesModel(this);
	_profilesModel->setColumnCount(1);
	_profilesModel->setHeaderData(0, Qt::Horizontal, "Name");

	root = _profilesModel->invisibleRootItem();
	rootItem = new QStandardItem("/");
	rootItem->setData(TypeRoot, Type);
	root->appendRow(rootItem);

	for ( size_t i = 0; i < base->modules.size(); ++i ) {
		Module *mod = base->modules[i].get();
		if ( !mod->supportsBindings() ) continue;
		QStandardItem *modItem = new QStandardItem(mod->definition->name.c_str());
		modItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		modItem->setData(TypeModule, Type);
		modItem->setData(qVariantFromValue((void*)mod), Link);
		modItem->setData(_docFolder, Qt::DecorationRole);
		rootItem->appendRow(modItem);

		for ( size_t j = 0; j < mod->profiles.size(); ++j ) {
			ModuleBinding *profile = mod->profiles[j].get();
			QStandardItem *profItem = new QStandardItem(profile->name.c_str());
			profItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
			profItem->setData(TypeProfile, Type);
			profItem->setData(qVariantFromValue((void*)profile), Link);
			profItem->setData(_docIcon, Qt::DecorationRole);
			modItem->appendRow(profItem);
		}
	}

	_modulesFolderView->setModel(NULL);
	_modulesView->setModel(_profilesModel);
	_modulesView->setRootIndex(rootItem->index());
	connect(_modulesView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
	        this, SLOT(moduleTreeCurrentChanged(const QModelIndex &, const QModelIndex &)));

	changeFolder(_stationsFolderView->rootIndex());
}


void BindingsPanel::moduleTreeCurrentChanged(const QModelIndex &curr, const QModelIndex &prev) {
	switch ( curr.data(Type).toInt() ) {
		case TypeModule:
			_addProfile->setEnabled(true);
			_deleteProfile->setEnabled(true);
			_modulesFolderView->setModel(_profilesModel);
			_modulesFolderView->setEnabled(true);
			if ( _modulesFolderView->rootIndex() != curr ) {
				_modulesFolderView->setRootIndex(curr);
				_modulesFolderView->selectionModel()->clear();
			}
			break;
		case TypeProfile:
			_addProfile->setEnabled(true);
			_deleteProfile->setEnabled(true);
			_modulesFolderView->setModel(_profilesModel);
			_modulesFolderView->setEnabled(true);
			if ( _modulesFolderView->rootIndex() != curr.parent() ) {
				_modulesFolderView->setRootIndex(curr.parent());
				_modulesFolderView->selectionModel()->clear();
			}
			break;
		default:
			_addProfile->setEnabled(false);
			_deleteProfile->setEnabled(false);
			_modulesFolderView->setEnabled(false);
			_modulesFolderView->setModel(NULL);
	}
}


void BindingsPanel::moduleBindingsChanged(const QModelIndex &topLeft,
                                          const QModelIndex &bottomRight) {
	// Only react on profile changes
	int type = topLeft.data(Type).toInt();

	if ( type == TypeProfile ) {
		// Update icon
		if ( topLeft.data().toString().isEmpty() )
			_bindingsModel->setData(topLeft.sibling(topLeft.row(),0), _docIcon, Qt::DecorationRole);
		else
			_bindingsModel->setData(topLeft.sibling(topLeft.row(),0), _linkIcon, Qt::DecorationRole);
		_model->setModified();
		return;
	}
}


void BindingsPanel::bindingActivated(const QModelIndex &idx) {
	changeFolder(idx);
}


void BindingsPanel::bindingDoubleClicked(const QModelIndex &idx) {
	int type = idx.data(Type).toInt();

	if ( type != TypeModule ) return;

	if ( !idx.sibling(idx.row(), 1).data().toString().isEmpty() ) {
		if ( QMessageBox::question(this, tr("Warning"),
		                           tr("The binding you are about to open is a profile and "
		                              "can potentially affect many stations. "
		                              "Do you want to continue?"),
		                           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
			return;
	}

	_bindingView->setModel(_model, _bindingsModel);
	_bindingView->setRootIndex(idx.sibling(idx.row(),0));
	_stationsTreeView->setCurrentIndex(idx.sibling(idx.row(),0));
	_stationsFolderView->setCurrentIndex(idx.sibling(idx.row(),0));

	if ( _stationsFolderView->rootIndex() != idx.parent() ) {
		_stationsFolderView->setRootIndex(idx.parent());
		_folderLevelUp->setEnabled(idx.parent().isValid());
	}
}


void BindingsPanel::profileActivated(const QModelIndex &idx) {
	/*
	int type = idx.data(Type).toInt();
	if ( type != TypeProfile ) return;

	_bindingView->setModel(_profilesModel);
	_bindingView->setRootIndex(idx);
	*/
}


void BindingsPanel::profileDoubleClicked(const QModelIndex &idx) {
	/*
	int type = idx.data(Type).toInt();
	if ( type != TypeProfile ) return;

	ModuleBinding *profile = static_cast<ModuleBinding*>(idx.data(Link).value<void*>());

	QItemSelectionModel *selectionModel = _stationsTreeView->selectionModel();
	if ( selectionModel ) {
		selectionModel->clear();
		selectBindings(selectionModel, profile);
	}
	*/

	int type = idx.data(Type).toInt();
	if ( type != TypeProfile ) return;

	_modulesView->setCurrentIndex(idx.sibling(idx.row(),0));
	_modulesFolderView->setCurrentIndex(idx.sibling(idx.row(),0));

	_bindingView->setModel(_model, _profilesModel);
	_bindingView->setRootIndex(idx);
}


void BindingsPanel::selectBindings(QItemSelectionModel *selectionModel,
                                   void *link, const QModelIndex &parent) {
	int rows = _bindingsModel->rowCount(parent);
	for ( int i = 0; i < rows; ++i ) {
		QModelIndex idx = _bindingsModel->index(i, 0, parent);
		if ( idx.data(Type).toInt() == TypeModule ) {
			if ( link == idx.data(Link).value<void*>() )
				selectionModel->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}

		selectBindings(selectionModel, link, idx);
	}
}


void BindingsPanel::changeFolder(const QModelIndex &idx_) {
	if ( QApplication::keyboardModifiers() != Qt::NoModifier ) return;

	QModelIndex idx = idx_.sibling(idx_.row(), 0);

	int type = idx.data(Type).toInt();

	if ( type >= TypeModule ) return;

	_stationsTreeView->setCurrentIndex(idx_);
	_stationsFolderView->setRootIndex(idx);
	_stationsFolderView->selectionModel()->clear();

	_folderLevelUp->setEnabled(idx.parent().isValid());
}


void BindingsPanel::collectModuleBindings(ModuleBindingMap &map,
                                          const QModelIndex &idx) {
	int type = idx.data(Type).toInt();
	if ( type == TypeNetwork || type == TypeStation ) {
		int rowCount = _stationsFolderView->model()->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i )
			collectModuleBindings(map, idx.child(i, 0));
		return;
	}

	if ( type == TypeModule ) {
		ModuleBinding *b = getLink<ModuleBinding>(idx);
		Module *mod = (Module*)b->parent;
		map[QString(mod->definition->name.c_str())].insert(QString(b->name.c_str()));
		return;
	}
}


void BindingsPanel::clearModuleBindings(const QModelIndex &idx) {
	int type = idx.data(Type).toInt();
	if ( type == TypeRoot || type == TypeNetwork ) {
		int rowCount = _stationsFolderView->model()->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i )
			clearModuleBindings(idx.child(i, 0));
		return;
	}

	if ( type == TypeStation ) {
		// Collect all childs (bindings)
		QList<QPersistentModelIndex> indexes;
		int rowCount = _stationsFolderView->model()->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i )
			indexes.append(idx.child(i, 0));

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) continue;

			StationID id(qPrintable(i.data(Net).toString()),
			             qPrintable(i.data(Sta).toString()));

			ModuleBinding *b = getLink<ModuleBinding>(i);
			Module *mod = (Module*)b->parent;

			if ( mod->removeStation(id) )
				_stationsFolderView->model()->removeRow(i.row(), idx);
		}

		_model->setModified();

		return;
	}
}


void BindingsPanel::removeModuleBindings(const QModelIndex &idx,
                                         const QString &modName,
                                         const QString *name) {
	int type = idx.data(Type).toInt();
	if ( type == TypeRoot || type == TypeNetwork ) {
		int rowCount = _stationsFolderView->model()->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i )
			removeModuleBindings(idx.child(i, 0), modName, name);
		return;
	}

	if ( type == TypeStation ) {
		// Collect all childs (bindings)
		QList<QPersistentModelIndex> indexes;
		int rowCount = _stationsFolderView->model()->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i )
			indexes.append(idx.child(i, 0));

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) continue;

			StationID id(qPrintable(i.data(Net).toString()),
			             qPrintable(i.data(Sta).toString()));

			ModuleBinding *b = getLink<ModuleBinding>(i);
			Module *mod = (Module*)b->parent;

			if ( modName != mod->definition->name.c_str() ) continue;
			if ( name != NULL && *name != b->name.c_str() ) continue;

			if ( mod->removeStation(id) )
				_stationsFolderView->model()->removeRow(i.row(), idx);
		}

		_model->setModified();

		return;
	}
}


void BindingsPanel::folderViewContextMenu(const QPoint &p) {
	QModelIndex idx = _stationsFolderView->rootIndex();
	QModelIndex hoveredIdx = _stationsFolderView->indexAt(p);
	QAction *a;

	if ( !idx.isValid() ) return;

	if ( hoveredIdx.isValid() ) {
		hoveredIdx = hoveredIdx.sibling(hoveredIdx.row(), 0);
		int type = hoveredIdx.data(Type).toInt();

		if ( type == TypeNetwork || type == TypeStation ) {
			QMenu menu;
			QAction *clearAction = menu.addAction("Clear all module bindings");
			QAction *deleteAction = menu.addAction("Delete");
			QMenu *removeBinding = NULL;

			QItemSelectionModel *sel = _stationsFolderView->selectionModel();

			ModuleBindingMap bindingMap;
			foreach ( const QModelIndex &idx, sel->selectedIndexes() )
				collectModuleBindings(bindingMap, idx);

			if ( !bindingMap.empty() ) {
				removeBinding = menu.addMenu("Remove module binding");
				ModuleBindingMap::iterator it;
				for ( it = bindingMap.begin(); it != bindingMap.end(); ++it ) {
					QMenu *sub = removeBinding->addMenu(it.key());
					sub->setObjectName(it.key());
					foreach ( const QString &name, it.value() ) {
						QAction *action;
						if ( name.isEmpty() )
							action = sub->addAction("station");
						else
							action = sub->addAction(QString("profile_%1").arg(name));

						action->setData(name);
					}

					if ( it.value().count() > 1 ) {
						sub->addSeparator();
						sub->addAction("Any");
					}
				}
			}

			a = menu.exec(_stationsFolderView->mapToGlobal(p));
			if ( a == clearAction ) {
				if ( sel->selectedIndexes().count() > 0 ) {
					if ( QMessageBox::question(NULL, "Clear bindings",
					       QString("Do you really want to remove all module\n"
					               "bindings of %1 selected %2?")
					       .arg(sel->selectedIndexes().count())
					       .arg(type == TypeNetwork?"network(s)":"station(s)"),
					       QMessageBox::Yes | QMessageBox::No
					     ) != QMessageBox::Yes )
						return;
				}

				foreach ( const QModelIndex &idx, sel->selectedIndexes() )
					clearModuleBindings(idx);
			}
			else if ( a == deleteAction )
				deleteItem();
			else if ( a != NULL && removeBinding != NULL &&
			          a->parentWidget() != NULL &&
			          a->parentWidget()->parentWidget() == removeBinding ) {
				QString bindingName = a->data().toString();
				QString moduleName = a->parentWidget()->objectName();

				QString *bindingNameFilter = NULL;
				if ( a->data().isValid() )
					bindingNameFilter = &bindingName;

				foreach ( const QModelIndex &idx, sel->selectedIndexes() )
					removeModuleBindings(idx, moduleName, bindingNameFilter);
			}
		}
		else if ( type == TypeModule ) {
			QItemSelectionModel *sel = _stationsFolderView->selectionModel();
			QMenu menu;
			QMenu *menuProfile = NULL;
			Module *mod = NULL;

			if ( sel->isSelected(hoveredIdx) &&
			     sel->selectedIndexes().count() == 1 ) {
				menuProfile = menu.addMenu("Change profile");
				ModuleBinding *b = getLink<ModuleBinding>(hoveredIdx);
				mod = (Module*)b->parent;
				a = menuProfile->addAction("None");
				a->setEnabled(!b->name.empty());
				a->setData(qVariantFromValue((void*)NULL));
				for ( size_t i = 0; i < mod->profiles.size(); ++i ) {
					a = menuProfile->addAction(mod->profiles[i]->name.c_str());
					a->setData(qVariantFromValue((void*)mod->profiles[i].get()));
					QFont f = a->font();
					f.setBold(true);
					a->setFont(f);
					if ( mod->profiles[i] == b )
						a->setEnabled(false);
				}
			}

			QAction *deleteAction = menu.addAction("Delete");

			a = menu.exec(_stationsFolderView->mapToGlobal(p));
			if ( a == NULL ) return;

			if ( a == deleteAction )
				deleteItem();
			else if ( a->parent() == menuProfile ) {
				StationID id(qPrintable(hoveredIdx.data(Net).toString()),
				             qPrintable(hoveredIdx.data(Sta).toString()));

				ModuleBinding *b = reinterpret_cast<ModuleBinding*>(a->data().value<void*>());
				if ( b ) {
					if ( !mod->bind(id, b) ) b = NULL;
				}
				else
					b = mod->bind(id, "");

				if ( !b ) {
					QMessageBox::critical(NULL, "Change profile",
					                            "Changing the profile failed.");
					return;
				}

				QAbstractItemModel *m = _stationsFolderView->model();
				m->setData(hoveredIdx, qVariantFromValue((void*)b), Link);
				m->setData(hoveredIdx.sibling(hoveredIdx.row(), 1), b->name.c_str(), Qt::EditRole);
			}
		}

		return;
	}

	int type = idx.data(Type).toInt();

	switch ( type ) {
		case TypeRoot:
		{
		QMenu menu;
		QAction *addNetwork = menu.addAction("Add network");
		a = menu.exec(_stationsFolderView->mapToGlobal(p));
		if ( a == NULL ) return;

		if ( a == addNetwork ) {
			NewNameDialog dlg(idx, true, this);
			dlg.setWindowTitle("New network name");
			if ( dlg.exec() != QDialog::Accepted )
				return;

			QStandardItem *netItem = new QStandardItem(dlg.name());
			netItem->setData(TypeNetwork, Type);
			netItem->setData(_docFolder, Qt::DecorationRole);
			netItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QStandardItem *rootItem = _bindingsModel->itemFromIndex(idx);
			rootItem->appendRow(netItem);
			_stationsFolderView->selectionModel()->setCurrentIndex(
				netItem->index(), QItemSelectionModel::ClearAndSelect
			);
		}
		}
		break;

		// Root is network, allow to modify stations
		case TypeNetwork:
		{
		QMenu menu;
		QAction *addStation = menu.addAction("Add station");
		a = menu.exec(_stationsFolderView->mapToGlobal(p));
		if ( a == NULL ) return;

		if ( a == addStation ) {
			NewNameDialog dlg(idx, true, this);
			dlg.setWindowTitle("New station name");
			if ( dlg.exec() != QDialog::Accepted )
				return;

			StationID id(qPrintable(idx.data().toString()),
			             qPrintable(dlg.name()));

			if ( !_model->model()->addStation(id) ) {
				QMessageBox::critical(this, "Add station",
				                      "Adding the station failed.");
				return;
			}

			QStandardItem *staItem = new QStandardItem(dlg.name());
			staItem->setData(TypeStation, Type);
			staItem->setData(_docFolder, Qt::DecorationRole);
			staItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QStandardItem *netItem = _bindingsModel->itemFromIndex(idx);
			netItem->appendRow(staItem);
			_stationsFolderView->selectionModel()->setCurrentIndex(
				staItem->index(), QItemSelectionModel::ClearAndSelect
			);
		}
		}
		break;

		// Root is station, allow to modify bindings
		case TypeStation:
		{
		QMenu menu;
		QMenu *bindingMenu = menu.addMenu("Add binding");
		Module *mod;

		StationID id(qPrintable(idx.parent().data().toString()),
		             qPrintable(idx.data().toString()));
		Model *model = _model->model();

		for ( size_t i = 0; i < model->modules.size(); ++i ) {
			mod = model->modules[i].get();
			if ( mod->supportsBindings() && mod->getBinding(id) == NULL ) {
				a = bindingMenu->addAction(mod->definition->name.c_str());
				a->setData(qVariantFromValue((void*)mod));
			}
		}

		if ( bindingMenu->isEmpty() )
			bindingMenu->setEnabled(false);

		a = menu.exec(_stationsFolderView->mapToGlobal(p));
		if ( a == NULL ) return;

		if ( a->parent() == bindingMenu ) {
			Module *mod = reinterpret_cast<Module*>(a->data().value<void*>());
			ModuleBinding *binding = mod->bind(id, "");
			if ( binding == NULL ) {
				QMessageBox::critical(NULL, "Add binding",
				                      "Creation of binding failed.");
				return;
			}

			QStandardItem *bindItem = new QStandardItem(mod->definition->name.c_str());
			bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			bindItem->setData(id.networkCode.c_str(), Net);
			bindItem->setData(id.stationCode.c_str(), Sta);
			bindItem->setData(TypeModule, Type);
			bindItem->setData(qVariantFromValue((void*)binding), Link);

			if ( binding->name.empty() )
				bindItem->setData(_docIcon, Qt::DecorationRole);
			else
				bindItem->setData(_linkIcon, Qt::DecorationRole);

			QStandardItem *profileItem = new QStandardItem(binding->name.c_str());
			profileItem->setData(TypeProfile, Type);

			QStandardItem *item = _bindingsModel->itemFromIndex(idx);
			item->appendRow(QList<QStandardItem*>() << bindItem << profileItem);
			_stationsFolderView->selectionModel()->setCurrentIndex(
				bindItem->index(), QItemSelectionModel::ClearAndSelect
			);

			_bindingView->setModel(_model, _bindingsModel);
			_bindingView->setRootIndex(bindItem->index());
		}
		}
		break;
	}
}


void BindingsPanel::modulesViewContextMenu(const QPoint &p) {
	QModelIndex idx = _modulesView->rootIndex();
	QModelIndex hoveredIdx = _modulesView->indexAt(p);

	if ( !idx.isValid() ) return;

	if ( hoveredIdx.data(Type).toInt() != TypeModule ) return;

	QMenu menu;
	QAction *addAction = menu.addAction(tr("Add %1 profile").arg(hoveredIdx.data().toString()));
	QAction *res = menu.exec(_modulesView->mapToGlobal(p));

	if ( res == addAction )
		addProfile();
}


void BindingsPanel::modulesFolderViewContextMenu(const QPoint &p) {
	QModelIndex idx = _modulesFolderView->rootIndex();
	QModelIndex hoveredIdx = _modulesFolderView->indexAt(p);

	if ( !idx.isValid() ) return;

	QMenu menu;

	QAction *addAction = menu.addAction(tr("Add profile"));
	QAction *delAction = NULL;

	if ( !_modulesFolderView->selectionModel()->selectedIndexes().empty() )
		delAction = menu.addAction(tr("Delete selected profiles"));

	QAction *res = menu.exec(_modulesFolderView->mapToGlobal(p));
	if ( res == addAction )
		addProfile();
	else if ( res == delAction )
		deleteProfile();
}


void BindingsPanel::folderLevelUp() {
	if ( _stationsFolderView->rootIndex().parent().isValid() )
		changeFolder(_stationsFolderView->rootIndex().parent());
}


void BindingsPanel::deleteItem() {
	QModelIndex idx = _stationsFolderView->rootIndex();
	int type = idx.data(Type).toInt();

	if ( type == TypeRoot ) {
		QItemSelectionModel *sel = _stationsFolderView->selectionModel();
		QModelIndexList selection = sel->selectedIndexes();
		if ( selection.count() > 0 ) {
			if ( QMessageBox::question(NULL, "Delete",
			       QString("Do you really want to delete %1 network(s)?")
			       .arg(selection.count()),
			       QMessageBox::Yes | QMessageBox::No
			     ) != QMessageBox::Yes )
				return;
		}

		QList<QPersistentModelIndex> indexes;
		foreach ( const QModelIndex &i, selection )
			if ( i.column() == 0 ) indexes.append(i);

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) continue;

			while ( i.model()->rowCount(i) > 0 )
				deleteStation(i.child(0, 0));

			_stationsFolderView->model()->removeRow(i.row(), idx);
		}
	}
	else if ( type == TypeNetwork ) {
		QItemSelectionModel *sel = _stationsFolderView->selectionModel();
		QModelIndexList selection = sel->selectedIndexes();
		if ( selection.count() > 0 ) {
			if ( QMessageBox::question(NULL, "Delete",
			       QString("Do you really want to delete %1 station(s)?")
			       .arg(selection.count()),
			       QMessageBox::Yes | QMessageBox::No
			     ) != QMessageBox::Yes )
				return;
		}

		QList<QPersistentModelIndex> indexes;
		foreach ( const QModelIndex &i, selection )
			if ( i.column() == 0 ) indexes.append(i);

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) continue;
			deleteStation(i);
		}
	}
	if ( type == TypeStation ) {
		QItemSelectionModel *sel = _stationsFolderView->selectionModel();
		QModelIndexList selection = sel->selectedIndexes();
		if ( selection.count() > 0 ) {
			if ( QMessageBox::question(NULL, "Delete",
			       QString("Do you really want to delete %1 binding(s)?")
			       .arg(selection.count()),
			       QMessageBox::Yes | QMessageBox::No
			     ) != QMessageBox::Yes )
				return;
		}

		QList<QPersistentModelIndex> indexes;
		foreach ( const QModelIndex &i, selection )
			if ( i.column() == 0 ) indexes.append(i);

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) continue;

			StationID id(qPrintable(i.data(Net).toString()),
			             qPrintable(i.data(Sta).toString()));

			ModuleBinding *b = getLink<ModuleBinding>(i);
			Module *mod = (Module*)b->parent;

			if ( mod->removeStation(id) ) {
				_stationsFolderView->model()->removeRow(i.row(), idx);
				_model->setModified();
			}
		}
	}
}


void BindingsPanel::deleteStation(const QModelIndex &idx) {
	StationID id(qPrintable(idx.parent().data().toString()),
	             qPrintable(idx.data().toString()));

	_model->model()->removeStation(id);
	_bindingsModel->removeRow(idx.row(), idx.parent());
	_model->setModified();
}


void BindingsPanel::deleteProfile(const QModelIndex &idx) {
	ModuleBinding *binding = getLink<ModuleBinding>(idx);
	Module *mod = (Module*)binding->parent;
	mod->removeProfile(binding);
	syncProfileRemoval(_bindingsModel, binding);
	_model->setModified();
}


void BindingsPanel::syncProfileRemoval(QAbstractItemModel *m, void *link, const QModelIndex &parent) {
	int rows = m->rowCount(parent);
	for ( int i = 0; i < rows; ++i ) {
		QModelIndex idx = m->index(i, 0, parent);
		syncProfileRemoval(m, link, idx);

		if ( idx.data(Type).toInt() == TypeModule ) {
			if ( link == idx.data(Link).value<void*>() ) {
				m->removeRow(idx.row(), parent);
				--i; --rows;
			}
		}
	}
}


bool BindingsPanel::assignProfile(const QModelIndex &idx, const QString &module,
                                  const QString &profile) {
	int type = idx.data(Type).toInt();
	switch ( type ) {
		case TypeRoot:
		{
			int rows = _bindingsModel->rowCount(idx);
			for ( int i = 0; i < rows; ++i )
				assignProfile(idx.child(i, 0), module, profile);
			return true;
		}
		case TypeNetwork:
		{
			int rows = _bindingsModel->rowCount(idx);
			for ( int i = 0; i < rows; ++i )
				assignProfile(idx.child(i, 0), module, profile);
			_model->setModified();
			return true;
		}
		case TypeStation:
		{
			// Get module
			Module *mod = _model->model()->module(qPrintable(module));
			if ( mod == NULL ) return false;
			ModuleBinding *prof = mod->getProfile(qPrintable(profile));
			if ( prof == NULL ) return false;

			StationID id(qPrintable(idx.parent().data().toString()),
			             qPrintable(idx.data().toString()));

			int rows = _bindingsModel->rowCount(idx);
			for ( int i = 0; i < rows; ++i ) {
				QModelIndex child = idx.child(i, 0);
				if ( mod->definition->name != qPrintable(child.data().toString()) )
					continue;

				if ( mod->bind(id, prof) ) {
					// Update an available binding
					_bindingsModel->setData(child, qVariantFromValue((void*)prof), Link);
					_bindingsModel->setData(child.sibling(child.row(), 1), prof->name.c_str(), Qt::EditRole);
				}
				else
					cerr << "ERROR: could not assign binding" << endl;

				_model->setModified();
				return true;
			}

			// Nothing updated so far: create a new binding
			if ( !mod->bind(id, prof) ) {
				cerr << "ERROR: binding failed" << endl;
				return false;
			}

			QStandardItem *bindItem = new QStandardItem(mod->definition->name.c_str());
			bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			bindItem->setData(id.networkCode.c_str(), Net);
			bindItem->setData(id.stationCode.c_str(), Sta);
			bindItem->setData(TypeModule, Type);
			bindItem->setData(qVariantFromValue((void*)prof), Link);

			if ( prof->name.empty() )
				bindItem->setData(_docIcon, Qt::DecorationRole);
			else
				bindItem->setData(_linkIcon, Qt::DecorationRole);

			QStandardItem *profileItem = new QStandardItem(prof->name.c_str());
			profileItem->setData(TypeProfile, Type);

			QStandardItem *item = _bindingsModel->itemFromIndex(idx);
			item->appendRow(QList<QStandardItem*>() << bindItem << profileItem);

			_model->setModified();
			return true;
		}

		default:
			break;
	}

	return false;
}


void BindingsPanel::switchToStationsIconView() {
	_stationsFolderView->setViewMode(QListView::IconMode);
	_stationsFolderView->setGridSize(QSize(64,64));
	_stationsFolderView->setAcceptDrops(true);
}


void BindingsPanel::switchToStationsListView() {
	_stationsFolderView->setViewMode(QListView::ListMode);
	_stationsFolderView->setGridSize(QSize());
	_stationsFolderView->setSpacing(0);
	_stationsFolderView->setAcceptDrops(true);
}


void BindingsPanel::switchToProfileIconView() {
	_modulesFolderView->setViewMode(QListView::IconMode);
	_modulesFolderView->setGridSize(QSize(80,64));
	_modulesFolderView->setDragEnabled(true);
}


void BindingsPanel::switchToProfileListView() {
	_modulesFolderView->setViewMode(QListView::ListMode);
	_modulesFolderView->setGridSize(QSize());
	_modulesFolderView->setSpacing(0);
	_modulesFolderView->setDragEnabled(true);
}


void BindingsPanel::addProfile() {
	if ( _modulesFolderView->rootIndex().data(Type) != TypeModule ) return;

	NewNameDialog dlg(_modulesFolderView->rootIndex(), false, this);
	dlg.setWindowTitle(QString("New %1 profile").arg(_modulesFolderView->rootIndex().data().toString()));
	if ( dlg.exec() != QDialog::Accepted ) return;

	Module *mod = getLink<Module>(_modulesFolderView->rootIndex());

	ModuleBinding *profile = mod->createProfile(dlg.name().toStdString());

	if ( profile == NULL ) {
		QMessageBox::critical(this, "Add profile",
		                      "Adding the profile failed.");
		return;
	}

	QStandardItem *profItem = new QStandardItem(profile->name.c_str());
	profItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
	profItem->setData(TypeProfile, Type);
	profItem->setData(qVariantFromValue((void*)profile), Link);
	profItem->setData(_docIcon, Qt::DecorationRole);

	QStandardItem *item = _profilesModel->itemFromIndex(_modulesFolderView->rootIndex());
	item->appendRow(profItem);
}


void BindingsPanel::deleteProfile() {
	if ( _modulesFolderView->rootIndex().data(Type) != TypeModule ) return;

	QItemSelectionModel *sel = _modulesFolderView->selectionModel();
	QModelIndexList selection = sel->selectedIndexes();
	if ( selection.isEmpty() ) return;

	if ( QMessageBox::question(NULL, "Delete",
	           QString("Do you really want to delete %1 profile(s)?\n"
	                   "Each active binding that is using one of the "
	                   "selected profiles will be removed as well.")
	           .arg(selection.count()),
	           QMessageBox::Yes | QMessageBox::No
	     ) != QMessageBox::Yes )
		return;

	QList<QPersistentModelIndex> indexes;
	foreach ( const QModelIndex &i, selection )
		if ( i.column() == 0 ) indexes.append(i);

	foreach ( const QPersistentModelIndex &i, indexes ) {
		if ( !i.isValid() ) continue;
		deleteProfile(i);
		_modulesFolderView->model()->removeRow(i.row(), i.parent());
	}

	_model->setModified();
}
