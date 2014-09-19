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



#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/aboutwidget.h>
#include <seiscomp3/system/environment.h>
#include <license.h>
#include <seiscomp3/utils/files.h>


namespace Seiscomp {
namespace Gui {


AboutWidget::AboutWidget(QWidget* parent, Qt::WFlags f)
 : QWidget(parent, f) {
	_ui.setupUi(this);
	setWindowFlags(Qt::Tool| Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_DeleteOnClose);
	activateWindow();

	_ui.textLicense->setText(License::text());
	_ui.labelVersion->setText(SCApp->version());
}


AboutWidget::~AboutWidget() {}


}
}
