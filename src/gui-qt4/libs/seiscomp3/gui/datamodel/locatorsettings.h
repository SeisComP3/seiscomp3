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



#ifndef __SEISCOMP_GUI_LOCATORSETTINGS_H__
#define __SEISCOMP_GUI_LOCATORSETTINGS_H__

#include <QtGui>
#include <seiscomp3/gui/datamodel/ui_locatorsettings.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API LocatorSettings : public QDialog {
	Q_OBJECT

	public:
		typedef QList< QPair<QString,QString> > ContentList;

	public:
		LocatorSettings(QWidget * parent = 0, Qt::WindowFlags f = 0);

	public:
		void addRow(const QString &name, const QString &value);
		void lastRowAdded();

		ContentList content() const;


	private:
		::Ui::LocatorSettings _ui;
};



}

}

#endif
