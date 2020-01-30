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


#ifndef __SEISCOMP_GUI_CORE_UNCERTAINTIES_H__
#define __SEISCOMP_GUI_CORE_UNCERTAINTIES_H__

#include <QDialog>

#include <seiscomp3/gui/core/ui_uncertainties.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API EditUncertainties : public QDialog {
	Q_OBJECT

	public:
		EditUncertainties(QWidget * parent = 0, Qt::WindowFlags f = 0);
		~EditUncertainties();

		void setUncertainties(double lower, double upper);

		double lowerUncertainty() const;
		double upperUncertainty() const;


	signals:
		void uncertaintiesChanged(double, double);


	private slots:
		void lowerChanged(double);
		void upperChanged(double);

		void symmetryChanged(bool);


	private:
		Ui::Uncertainties _ui;
};


}
}

#endif
