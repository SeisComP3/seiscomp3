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


#ifndef __SEISCOMP_GUI_TTDECORATOR_H__
#define __SEISCOMP_GUI_TTDECORATOR_H__

#include <list>

#include <seiscomp3/math/geo.h>
#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/datamodel/timequantity.h>
#include <seiscomp3/gui/map/mapwidget.h>
#include <seiscomp3/gui/qt4.h>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API TTDecorator : public Map::Decorator {
	// ------------------------------------------------------------------
	// Nested Types
	// ------------------------------------------------------------------
	private:
		typedef std::vector<double> TravelTimes;
		enum Direction { NORTH_SOUTH, EAST_WEST };


	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		TTDecorator(Map::Decorator *decorator = NULL);

		void setPreferredMagnitudeValue(double val);

		double longitude() const;
		void setLongitude(double longitude);

		double latitude() const;
		void setLatitude(double latitude);

		double depth() const;
		void setDepth(double depth);

		const DataModel::TimeQuantity& originTime() const;
		void setOriginTime(const DataModel::TimeQuantity& time);


	// ------------------------------------------------------------------
	// Protected Interface
	// ------------------------------------------------------------------
	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter);


	// ------------------------------------------------------------------
	// Private Interface
	// ------------------------------------------------------------------
	private:
		void computeTTT(TravelTimes& travelTimes, const std::string& phase,
		                double phaseDistance);

		double computeTTTPolygon(const std::vector<double>& travelTimes,
		                         std::vector<QPointF>& polygon);

		void drawPolygon(const Map::Canvas *canvas, QPainter& painter,
		                 const std::vector<QPointF>& polygon);

		void annotatePropagation(const Map::Canvas *canvas, QPainter& painter,
		                         double distance, Direction direction);

	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		std::vector<QPointF> _polygonP;
		std::vector<QPointF> _polygonS;
		TravelTimeTable      _ttTable;
		TravelTimes          _travelTimesP;
		TravelTimes          _travelTimesS;
		int                  _deltaDepth;
		double               _maxPDistance;
		double               _maxSDistance;
		double               _pDistance;
		int                  _deltaDist;
		int                  _rotDelta;
		double               _preferredMagnitudeVal;
		double               _longitude;
		double               _latitude;
		double               _depth;
		DataModel::TimeQuantity _originTime;

};


} // namespace Gui
} // namespace Seiscomp

#endif
