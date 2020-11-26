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


#ifndef __ABOUTWIDGET__
#define __ABOUTWIDGET__

#include <QWidget>
#include <QPixmap>

#include <seiscomp3/gui/core/ui_aboutwidget.h>


namespace Seiscomp {
namespace Gui {


class AboutWidget : public QWidget {
	Q_OBJECT

	public:
		AboutWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
		~AboutWidget();

	private:
		Ui::AboutWidget _ui;
};


}
}

#endif
