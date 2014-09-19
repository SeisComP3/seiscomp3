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


#ifndef __SEISCOMP_CONFIGURATION_GUI_PANEL_BINDINGS_H__
#define __SEISCOMP_CONFIGURATION_GUI_PANEL_BINDINGS_H__

#include "../gui.h"


class QAction;
class QTreeView;

class BindingsViewDelegate : public QItemDelegate {
	Q_OBJECT

	public:
		BindingsViewDelegate(QObject *parent = 0);

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		                      const QModelIndex &index) const;

		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model,
		                  const QModelIndex &index) const;

		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

		void updateEditorGeometry(QWidget *editor,
		                          const QStyleOptionViewItem &option,
		                          const QModelIndex &index) const;

	private slots:
		void profileChanged(const QString &text);
};


class BindingView : public QWidget {
	Q_OBJECT

	public:
		BindingView(QWidget *parent = 0);

	public:
		void clear();

		void setModel(ConfigurationTreeItemModel *base, QAbstractItemModel *model);
		void setRootIndex(const QModelIndex & index);


	private slots:
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsRemoved(const QModelIndex &parent, int start, int end);


	private:
		QAbstractItemModel                     *_model;
		QPersistentModelIndex                   _rootIndex;
		QWidget                                *_header;
		QLabel                                 *_icon;
		QLabel                                 *_label;
		ConfigurationTreeItemModel             *_bindingModel;
		QAbstractItemView                      *_view;
};


class BindingsPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		BindingsPanel(QWidget *parent = 0);

	protected:
		void setModel(ConfigurationTreeItemModel *model);

	private:
		void deleteStation(const QModelIndex &idx);
		void deleteProfile(const QModelIndex &idx);
		void selectBindings(QItemSelectionModel *, void*, const QModelIndex &parent = QModelIndex());
		void syncProfileRemoval(QAbstractItemModel *, void*, const QModelIndex &parent = QModelIndex());
		void clearModuleBindings(const QModelIndex &idx);
		void removeModuleBindings(const QModelIndex &idx, const QString &mod,
		                          const QString *name);

		typedef QMap<QString, QSet<QString> > ModuleBindingMap;
		void collectModuleBindings(ModuleBindingMap &, const QModelIndex &idx);

	public:
		bool assignProfile(const QModelIndex &, const QString &module,
		                   const QString &profile);

	private slots:
		// BindingsView slots
		void moduleBindingsChanged(const QModelIndex &, const QModelIndex &);

		void bindingActivated(const QModelIndex &);
		void bindingDoubleClicked(const QModelIndex &);
		void changeFolder(const QModelIndex &);

		void folderViewContextMenu(const QPoint&);
		void modulesViewContextMenu(const QPoint&);
		void modulesFolderViewContextMenu(const QPoint&);

		// ModuleView slots
		void moduleTreeCurrentChanged(const QModelIndex &, const QModelIndex &);
		void profileActivated(const QModelIndex &);
		void profileDoubleClicked(const QModelIndex &);

		// FolderView tools
		void folderLevelUp();
		void deleteItem();
		void switchToStationsIconView();
		void switchToStationsListView();
		void switchToProfileIconView();
		void switchToProfileListView();

		// ModulesView tools
		void addProfile();
		void deleteProfile();


	private:
		QAbstractItemView      *_stationsTreeView;
		QListView              *_stationsFolderView;
		QAbstractItemView      *_modulesView;
		QListView              *_modulesFolderView;
		BindingView            *_bindingView;
		QStandardItemModel     *_bindingsModel;
		QStandardItemModel     *_profilesModel;
		QAction                *_folderLevelUp;
		QAction                *_deleteItem;
		QAction                *_addProfile;
		QAction                *_deleteProfile;

		QIcon                   _docIcon;
		QIcon                   _linkIcon;
		QIcon                   _docFolder;
};


#endif
