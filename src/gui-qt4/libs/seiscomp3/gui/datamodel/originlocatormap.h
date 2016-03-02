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



#ifndef __ORIGINLOCATORMAP_H__
#define __ORIGINLOCATORMAP_H__

#include <QtGui>
#include <seiscomp3/gui/map/mapwidget.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/origin.h>
#endif
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {

namespace DataModel {
DEFINE_SMARTPOINTER(Station);
}

namespace Gui {


class OriginSymbol;

class SC_GUI_API OriginLocatorMap : public MapWidget {
	Q_OBJECT

	public:
		OriginLocatorMap(const MapsDesc &maps,
		                 QWidget *parent = 0, Qt::WFlags f = 0);

		OriginLocatorMap(Map::ImageTree* mapTree,
		                 QWidget *parent = 0, Qt::WFlags f = 0);

		//! Sets the maximum distance for stations to be displayed
		//! if they are not part of the origin
		void setStationsMaxDist(double);

		//! Make the station state changeable interactively
		//! The default is TRUE
		void setStationsInteractive(bool);
		void setOrigin(DataModel::Origin* o);

		bool waveformPropagation() const;
		bool drawStations() const;

		void setArrivalState(int id, bool state);
		void setStationState(const std::string& code, bool state);

		void setOriginCreationEnabled(bool enable);

	public slots:
		void setDrawStations(bool);
		void setDrawStationLines(bool);
		void setWaveformPropagation(bool);


	signals:
		void hoverArrival(int id);
		void clickedArrival(int id);
		void arrivalChanged(int id, bool state);
		void clickedStation(const std::string &net, const std::string &code);
		void stationChanged(const std::string &stationCode, bool state);
		void artificialOriginRequested(const QPointF &epicenter,
		                               const QPoint &dialogPos);


	protected:
		void drawCustomLayer(QPainter*);
		void mouseMoveEvent(QMouseEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);
		void contextMenuEvent(QContextMenuEvent*);

	private:
		void addArrival();
		void setStationState(int i, bool state);

	private:
		struct StationEntry {
			StationEntry()
			 : validLocation(false), isActive(false), isArrival(false),
			   arrivalId(-1) {}

			StationEntry(QPointF loc, const std::string &nc,
			             const std::string &sc, bool valid)
			 : location(loc), validLocation(valid),
			   isActive(false), isArrival(false),
			   net(nc), code(sc), arrivalId(-1) {}

			QPointF location;
			bool validLocation;
			bool isActive;
			bool isArrival;
			std::string net;
			std::string code;
			QColor color;
			int arrivalId;
		};

		DataModel::OriginPtr _origin;
		OriginSymbol *_originSymbol;
		bool _drawStations;
		bool _drawStationsLines;
		bool _interactive;
		bool _waveformPropagation;
		bool _enabledCreateOrigin;
		QVector<StationEntry> _stations;
		QVector<int> _arrivals;
		std::map<std::string, int> _stationCodes;
		int _lastSymbolSize;
		int _hoverId;
		double _stationsMaxDist;
};


}
}


#endif
