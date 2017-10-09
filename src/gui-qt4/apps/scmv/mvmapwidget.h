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


#ifndef __MVMAPWIDGET_H___
#define __MVMAPWIDGET_H___

#include <seiscomp3/gui/map/mapwidget.h>

#include "legend.h"


class MvMapWidget : public Seiscomp::Gui::MapWidget {
	Q_OBJECT

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MvMapWidget(const Seiscomp::Gui::MapsDesc &,
		            QWidget* parent = 0, Qt::WFlags f = 0);

		~MvMapWidget() {}

	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(QPainter& painter);

		ApplicationStatus::Mode mode() const;
		void setMode(ApplicationStatus::Mode mode);

	// ----------------------------------------------------------------------
	// Public slots
	// ----------------------------------------------------------------------
	public slots:
		void showMapLegend(bool val);

	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		Legend         _mapLegend;
		EQSymbolLegend _eqSymbolLegend;
};


#endif
