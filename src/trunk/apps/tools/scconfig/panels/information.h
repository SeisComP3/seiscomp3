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


#ifndef __SEISCOMP_CONFIGURATION_GUI_PANEL_INFORMATION_H__
#define __SEISCOMP_CONFIGURATION_GUI_PANEL_INFORMATION_H__

#include "../gui.h"


class QTableWidget;

class InformationPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		InformationPanel(QWidget *parent = 0);

	private:
		void addRow(QTableWidget *, const QString &name, const QString &value);
};


#endif
