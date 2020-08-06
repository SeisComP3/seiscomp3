/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * You can redistribute and/or modify this program under the               *
 * terms of the SeisComP Public License.                                   *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * SeisComP Public License for more details.                               *
 ***************************************************************************/


#ifndef SEISCOMP_GUI_ADVANCEDORIGINSYMBOL_H__
#define SEISCOMP_GUI_ADVANCEDORIGINSYMBOL_H__


#include <QVector>

#include <seiscomp3/gui/datamodel/originsymbol.h>

#ifndef Q_MOC_RUN
#include <seiscomp3/geo/coordinate.h>
#endif

namespace Seiscomp {
namespace DataModel {
class Origin;
}

namespace Gui {


class SC_GUI_API AdvancedOriginSymbol : public OriginSymbol {
	public:
		AdvancedOriginSymbol(Map::Decorator* decorator = NULL);
		AdvancedOriginSymbol(double latitude,
		                     double longitude,
		                     double depth = 0,
		                     Map::Decorator* decorator = NULL);
		AdvancedOriginSymbol(DataModel::Origin *origin,
		                     Map::Decorator* decorator = NULL);

	public:
		/**
		 * @brief setOrigin sets hypocenter, magnitude and confidence ellipse
		 * based on supplied
		 * @param origin
		 */
		void setOrigin(DataModel::Origin *origin);

		/**
		 * @brief setConfidenceEllipse sets confidence ellipse
		 * @param major major semi axis in km
		 * @param minor minor semi axis in km
		 * @param azimuth angle in degree of the major axis relative to the#
		 * prime meridian
		 * @param points number of geo coordinates used to draw the ellipse
		 */
		void setConfidenceEllipse(double major, double minor,
		                          double azimuth = 0, int points = 36);

		void setConfidenceEllipsePen(const QPen &pen);
		const QPen& confidenceEllipsePen() const;

		void setConfidenceEllipseBrush(const QBrush &brush);
		const QBrush& confidenceEllipseBrush() const;

		void setConfidenceEllipseEnabled(bool enabled);
		bool confidenceEllipseEnabled() const;

	protected:
		virtual void calculateMapPosition(const Map::Canvas *canvas);
		virtual void customDraw(const Map::Canvas *canvas, QPainter &painter);

		void init();

	protected:
		QVector<Geo::GeoCoordinate> _confidenceEllipse;
		QPainterPath                _renderPath;
		QPen                        _confidenceEllipsePen;
		QBrush                      _confidenceEllipseBrush;
		bool                        _confidenceEllipseEnabled;
};


} // namespace Gui
} // namespace Seiscomp


#endif
