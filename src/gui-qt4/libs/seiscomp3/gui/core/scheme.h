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



#ifndef __SEISCOMP_GUI_SCHEME_H__
#define __SEISCOMP_GUI_SCHEME_H__


#include <QColor>
#include <QPen>
#include <QBrush>
#include <QPoint>
#include <QFont>
#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/core/gradient.h>
#include <seiscomp3/gui/core/recordwidget.h>


class QTabWidget;


namespace Seiscomp {
namespace Gui {


class SC_GUI_API Scheme {
	public:
		Scheme();

		void applyTabPosition(QTabWidget *tab);

		struct Colors {
			struct Splash {
				Splash();
				QColor version;
				QColor message;
			};

			struct Picks {
				Picks();
				QColor manual;
				QColor automatic;
				QColor undefined;
				QColor disabled;
			};

			struct Arrivals {
				Arrivals();
				QColor manual;
				QColor automatic;
				QColor theoretical;
				QColor undefined;
				QColor disabled;
				Gradient residuals;
			};

			struct Magnitudes {
				Magnitudes();
				QColor set;
				QColor unset;
				QColor disabled;
				Gradient residuals;
			};

			struct RecordStates {
				RecordStates();
				QColor unrequested;
				QColor requested;
				QColor inProgress;
				QColor notAvailable;
			};

			struct BrushPen {
				QPen pen;
				QBrush brush;
			};

			struct RecordBorders {
				RecordBorders();
				BrushPen standard;
				BrushPen signatureValid;
				BrushPen signatureInvalid;
			};

			struct Records {
				Records();
				QColor alignment;
				QColor background;
				QColor alternateBackground;
				QColor foreground;
				QColor alternateForeground;
				QColor spectrogram;
				QPen offset;
				QPen gridPen;
				QPen subGridPen;
				QBrush gaps;
				QBrush overlaps;
				RecordStates states;
				RecordBorders borders;
			};

			struct Stations {
				Stations();
				QColor text;
				QColor associated;
				QColor selected;
				QColor triggering;
				QColor triggered0;
				QColor triggered1;
				QColor triggered2;
				QColor disabled;
				QColor idle;
			};

			struct QC {
				QC();
				QColor delay0;
				QColor delay1;
				QColor delay2;
				QColor delay3;
				QColor delay4;
				QColor delay5;
				QColor delay6;
				QColor delay7;
				QColor qcWarning;
				QColor qcError;
				QColor qcOk;
				QColor qcNotSet;
			};

			struct ConfigGradient {
				Gradient gradient;
				bool     discrete;
			};

			struct OriginSymbol {
				OriginSymbol();
				bool           classic;
				ConfigGradient depth;
			};

			struct OriginStatus {
				OriginStatus();
				QColor automatic;
				QColor manual;
			};

			struct GroundMotion {
				GroundMotion();
				QColor gmNotSet;
				QColor gm0;
				QColor gm1;
				QColor gm2;
				QColor gm3;
				QColor gm4;
				QColor gm5;
				QColor gm6;
				QColor gm7;
				QColor gm8;
				QColor gm9;
			};

			struct RecordView {
				RecordView();
				QColor selectedTraceZoom;
			};

			struct Map {
				Map();
				QColor lines;
				QColor outlines;
				QPen   grid;
				QColor stationAnnotations;
				QColor cityLabels;
				QColor cityOutlines;
				QColor cityCapital;
				QColor cityNormal;
			};

			struct Legend {
				Legend();
				QColor background;
				QColor border;
				QColor text;
				QColor headerText;
			};

			public:
				QColor        background;
				Splash        splash;
				Records       records;
				Picks         picks;
				Arrivals      arrivals;
				Magnitudes    magnitudes;
				Stations      stations;
				QC            qc;
				OriginSymbol  originSymbol;
				OriginStatus  originStatus;
				GroundMotion  gm;
				RecordView    recordView;
				Map           map;
				Legend        legend;
		};

		struct Fonts {
			Fonts();
			void setBase(const QFont& f);

			QFont base;
			QFont smaller;
			QFont normal;
			QFont large;
			QFont highlight;
			QFont heading1;
			QFont heading2;
			QFont heading3;

			QFont cityLabels;
			QFont splashVersion;
			QFont splashMessage;
		};

		struct Splash {
			Splash();

			struct Pos {
				QPoint pos;
				int    align;
			};

			Pos version;
			Pos message;
		};

		struct Map {
			Map();

			int stationSize;
			int originSymbolMinSize;
			bool vectorLayerAntiAlias;
			bool bilinearFilter;
			bool showGrid;
			bool showLayers;
			bool showCities;
			bool showLegends;
			int cityPopulationWeight;
			bool toBGR;
			int polygonRoughness;
			std::string projection;
		};

		struct Marker {
			Marker();

			int lineWidth;
		};

		struct RecordBorders {
			Gui::RecordWidget::RecordBorderDrawMode drawMode;
		};

		struct Records {
			Records();

			int lineWidth;
			bool antiAliasing;
			bool optimize;
			RecordBorders recordBorders;
		};

		struct Precision {
			Precision();

			int depth;
			int distance;
			int location;
			int magnitude;
			int pickTime;
			int traceValues;
			int rms;
			int uncertainties;
		};

		struct Unit {
			Unit();

			bool distanceInKM;
		};

		struct DateTime {
			DateTime();

			bool useLocalTime;
		};

	public:
		bool      showMenu;
		bool      showStatusBar;
		int       tabPosition;

		Splash    splash;
		Colors    colors;
		Marker    marker;
		Records   records;
		Map       map;
		Precision precision;
		Unit      unit;
		DateTime  dateTime;

		Fonts     fonts;

		QFont font(int relativeFontSize, bool bold = false, bool italic = false);

		void fetch();

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SC_GUI_API QColor blend(const QColor& c1, const QColor& c2, int percentOfC1);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SC_GUI_API QColor blend(const QColor& c1, const QColor& c2);


}
}


#endif

