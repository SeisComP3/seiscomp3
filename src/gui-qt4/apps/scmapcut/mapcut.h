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


#ifndef __SEISCOMP_GUI_SCMAPCUT_H__
#define __SEISCOMP_GUI_SCMAPCUT_H__


#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/originsymbol.h>
#include <seiscomp3/gui/map/canvas.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/origin.h>
#endif
#include <string.h>



class MapCut : public Seiscomp::Gui::Application {
	Q_OBJECT

	public:
		MapCut(int& argc, char **argv, Seiscomp::Gui::Application::Type);

		void createCommandLineDescription();

		bool run();


	private slots:
		void drawArrivals(QPainter *p);
		void renderingCompleted();
		void renderCanvas();


	private:
		std::string _region;
		std::string _margin;
		std::string _output;
		std::string _dimension;

		double      _latitude;
		double      _longitude;
		double      _depth;
		double      _magnitude;

		std::string _epFile;
		std::string _eventID;

		QRectF      _reg;
		QSizeF      _margins;
		QSize       _dim;
		QImage      _image;

		bool        _htmlArea;

		Seiscomp::Gui::Map::Canvas    *_canvas;
		Seiscomp::DataModel::OriginPtr _org;
		Seiscomp::Gui::OriginSymbol   *_symbol;
};



#endif
