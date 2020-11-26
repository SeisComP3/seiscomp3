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


#ifndef __SEISCOMP_GUI_CORE_XMLVIEW_H__
#define __SEISCOMP_GUI_CORE_XMLVIEW_H__

#include <QtGui>

#include <seiscomp3/gui/core/ui_xmlview.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API XMLView : public QWidget {
	Q_OBJECT
	public:
		XMLView(QWidget * parent = 0, Qt::WindowFlags f = 0,
		        bool deleteOnClose = true);
		~XMLView();

		void setContent(const QString& content);

	private:
		Ui::XMLViewDialog _ui;
};


}
}

#endif
