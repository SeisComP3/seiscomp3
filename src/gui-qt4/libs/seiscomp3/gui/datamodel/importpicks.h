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



#ifndef __SEISCOMP_GUI_IMPORTPICKS_H__
#define __SEISCOMP_GUI_IMPORTPICKS_H__

#include <QtGui>
#include <seiscomp3/gui/datamodel/ui_importpicks.h>
#include <seiscomp3/gui/qt4.h>

namespace Seiscomp {

namespace Gui {


class SC_GUI_API ImportPicksDialog : public QDialog {
	Q_OBJECT

	public:
		enum Selection {
			LatestOrigin,
			LatestAutomaticOrigin,
			MaxPhaseOrigin,
			AllOrigins
		};


	public:
		ImportPicksDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);

		Selection currentSelection() const;
		bool importAllPicks() const;
		bool importAllPhases() const;
		bool preferTargetPhases() const;


	private:
		::Ui::ImportPicks _ui;
		static Selection _lastSelection;
};



}

}

#endif
