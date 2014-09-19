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



#ifndef __SEISCOMP_GUI_ORIGINTIME_H__
#define __SEISCOMP_GUI_ORIGINTIME_H__


#include <QtGui>
#include <seiscomp3/gui/datamodel/ui_origintime.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API OriginTimeDialog : public QDialog {
	public:
		OriginTimeDialog(double lon, double lat,
		                 Seiscomp::Core::Time time,
		                 QWidget * parent = 0, Qt::WFlags f = 0);

		Seiscomp::Core::Time time() const;

	private:
		Ui::OriginTimeDialog _ui;
};


}
}

#endif
