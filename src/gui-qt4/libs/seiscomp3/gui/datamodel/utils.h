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



#ifndef __SEISCOMP_GUI_DATAMODEL_UTIL_H__
#define __SEISCOMP_GUI_DATAMODEL_UTIL_H__


#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/gui/core/utils.h>
#include <QCheckBox>


namespace Seiscomp {
namespace Gui {


class CheckBox : public QCheckBox {
	Q_OBJECT

	public:
		explicit CheckBox(QWidget *parent=0)
		: QCheckBox(parent) {}

		explicit CheckBox(const QString &text, QWidget *parent=0)
		: QCheckBox(text, parent) {}


	public slots:
		void check() {
			setChecked(true);
		}

		void unCheck() {
			setChecked(false);
		}
};


}
}


#endif
