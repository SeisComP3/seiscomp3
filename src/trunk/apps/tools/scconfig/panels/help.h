/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_CONFIGURATION_GUI_PANEL_HELP_H__
#define __SEISCOMP_CONFIGURATION_GUI_PANEL_HELP_H__

#include "../gui.h"

#include <QWidget>


class HelpPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		HelpPanel(QWidget *parent = 0);


	private slots:
		void refresh();
		void openIndex(const QModelIndex & index);


	private:
		QListView          *_folderView;
		QStandardItemModel *_model;
};


#endif
