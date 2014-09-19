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


#ifndef __SEISCOMP_CONFIGURATION_GUI_PANEL_MODULES_H__
#define __SEISCOMP_CONFIGURATION_GUI_PANEL_MODULES_H__

#include "../gui.h"


class ModulesPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		ModulesPanel(QWidget *parent = 0);

	public:
		void setModel(ConfigurationTreeItemModel *model);
		void aboutToClose();

	private slots:
		void moduleSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*);
		void moduleSelected(QTreeWidgetItem*,int);
		void moduleChanged(const QModelIndex &index);
		void search(const QString &text);
		void search();
		void closeSearch();


	private:
		bool                _modified;
		QAbstractItemView  *_moduleView;
		QTreeWidget        *_moduleTree;
		QWidget            *_searchWidget;
};


#endif
