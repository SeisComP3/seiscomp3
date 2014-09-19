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



#include "origintime.h"
#include <iostream>
#include <seiscomp3/gui/core/application.h>


namespace Seiscomp {
namespace Gui {


OriginTimeDialog::OriginTimeDialog(double lon, double lat,
                                   Seiscomp::Core::Time time,
                                   QWidget * parent, Qt::WFlags f)
 : QDialog(parent, f) {

	_ui.setupUi(this);

	_ui.label->setFont(SCScheme.fonts.normal);
	_ui.label_2->setFont(SCScheme.fonts.normal);
	_ui.labelLatitude->setFont(SCScheme.fonts.highlight);
	_ui.labelLongitude->setFont(SCScheme.fonts.highlight);

	_ui.labelLatitude->setText(QString("%1 %2").arg(lat, 0, 'f', 2).arg(QChar(0x00b0)));
	_ui.labelLongitude->setText(QString("%1 %2").arg(lon, 0, 'f', 2).arg(QChar(0x00b0)));

	int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;
	time.get(&y, &M, &d, &h, &m, &s);
	_ui.timeEdit->setTime(QTime(h, m, s));
	_ui.dateEdit->setDate(QDate(y, M, d));
}


Seiscomp::Core::Time OriginTimeDialog::time() const {
	QDateTime dt(_ui.dateEdit->date(), _ui.timeEdit->time());
	Seiscomp::Core::Time t;

	t.set(_ui.dateEdit->date().year(),
	      _ui.dateEdit->date().month(),
	      _ui.dateEdit->date().day(),
	      _ui.timeEdit->time().hour(),
	      _ui.timeEdit->time().minute(),
	      _ui.timeEdit->time().second(),
	      0);

	std::cout << t.toString("%F %T") << std::endl;
	return t;
}


}
}
