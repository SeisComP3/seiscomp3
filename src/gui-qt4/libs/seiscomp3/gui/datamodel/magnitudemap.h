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



#ifndef __MAGNITUDEMAP_H__
#define __MAGNITUDEMAP_H__

#include <QtGui>
#include <seiscomp3/gui/map/mapwidget.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#endif
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {

namespace DataModel {
DEFINE_SMARTPOINTER(Origin);
DEFINE_SMARTPOINTER(Magnitude);
class StationMagnitude;
DEFINE_SMARTPOINTER(Station);
}

namespace Gui {


class SC_GUI_API MagnitudeMap : public MapWidget {
	Q_OBJECT

	public:
		MagnitudeMap(const MapsDesc &maps,
		             QWidget *parent = 0, Qt::WindowFlags f = 0);

		MagnitudeMap(Map::ImageTree* mapTree,
		             QWidget *parent = 0, Qt::WindowFlags f = 0);

		~MagnitudeMap();

		//! Sets the maximum distance for stations to be displayed
		//! if they are not part of the origin
		void setStationsMaxDist(double);

		//! Make the station state changeable interactively
		//! The default is TRUE
		void setStationsInteractive(bool);
		void setOrigin(DataModel::Origin* o);
		void setMagnitude(DataModel::Magnitude* nm);

		void setMagnitudeState(int id, bool state);
		void setStationState(const std::string& code, bool state);

		void addStationMagnitude(DataModel::StationMagnitude* staMag, int index);

		QString stationSymbolToolTip() const;

	public slots:
		void setDrawStations(bool);

	signals:
		void hoverMagnitude(int id);
		void clickedMagnitude(int id);
		void clickedStation(const std::string &net, const std::string &code);
		void magnitudeChanged(int id, bool state);
		void stationChanged(const std::string &code, bool state);


	protected:
		void paintEvent(QPaintEvent*);
		void drawCustomLayer(QPainter*);
		void drawLines(QPainter&);
		void mouseMoveEvent(QMouseEvent *);
		void mousePressEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);

	private:
		void addMagnitude(int stationId, int magId);
		void setStationState(int i, bool state);
		void setStationResidual(int i, double residual);
		void setMagnitudeResidual(int id, double residual);
		int findStation(const std::string& stationCode) const;

	private:
		struct StationEntry {
			StationEntry()
			 : validLocation(false), isActive(false), isMagnitude(false), magnitudeId(-1) {}

			StationEntry(QPointF loc, const std::string &nc,
			             const std::string &sc, bool valid)
			 : location(loc), validLocation(valid),
			   isActive(false), isMagnitude(false),
			   net(nc), code(sc), magnitudeId(-1) {}

			QPointF location;
			QColor color;
			bool validLocation;
			bool isActive;
			bool isMagnitude;
			std::string net;
			std::string code;
			double residual;
			int magnitudeId;
		};

		DataModel::OriginPtr _origin;
		DataModel::MagnitudePtr _magnitude;
		bool _interactive;
		bool _drawStations;
		QVector<StationEntry> _stations;
		QVector<int> _magnitudes;
		std::map<std::string, int> _stationCodes;
		int _lastSymbolSize;
		int _hoverId;
		double _stationsMaxDist;
};


}
}


#endif
