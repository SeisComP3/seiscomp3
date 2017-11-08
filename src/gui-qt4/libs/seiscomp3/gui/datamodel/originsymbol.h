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


#ifndef __SEISCOMP_GUI_ORIGIN_H__
#define __SEISCOMP_GUI_ORIGIN_H__


#include <vector>
#include <string>

#include <QPolygon>
#include <QColor>
#include <QPoint>
#include <QPainter>

#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/map/mapsymbol.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/origin.h>
#endif


namespace Seiscomp {
namespace Gui {


class Canvas;


class SC_GUI_API OriginSymbol : public Map::Symbol {
	public:
		OriginSymbol(Map::Decorator* decorator = NULL);
		OriginSymbol(double latitude,
		             double longitude,
		             double depth = 0,
		             Map::Decorator* decorator = NULL);

	public:
		void setPreferredMagnitudeValue(double magnitudeValue);
		double preferredMagnitudeValue() const;

		/**
		 * @brief Sets the symbol depth and updates the color according
		 *        to scheme.colors.originSymbol.depth.gradient.
		 * @param depth The depth in km
		 */
		void setDepth(double depth);
		double depth() const;

		/**
		 * @brief Sets the symbol color and therefore overrides what
		 *        a prior call to setDepth has been set. This method was
		 *        added with API version 11.
		 * @param c The desired color
		 */
		void setColor(const QColor &c);

		/**
		 * @brief Returns the current color. This method was added with API
		 *        version 11.
		 * @return The current symbol color.
		 */
		const QColor &color() const;

		/**
		 * @brief Sets the symbols fill color if filling is enabled. This
		 *        overrides the default fill color set with setDepth.
		 *        This method was added with API version 11.
		 * @param c The fill color
		 */
		void setFillColor(const QColor &c);

		/**
		 * @brief Returns the current fill color.
		 *        This method was added with API version 11.
		 * @return The fill color
		 */
		const QColor &fillColor() const;

		void setFilled(bool val);
		bool isFilled() const;

		virtual bool isInside(int x, int y) const;

		/**
		 * @brief Returns the size of an origin symbol in pixel depending on
		 *        the magnitude.
		 * @param mag The input magnitude
		 * @return The size in pixels
		 */
		static int getSize(double mag);


	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter);

		void init();
		void updateSize();
		void depthColorCoding();


	protected:
		Seiscomp::DataModel::Origin *_origin;
		QColor                       _color;
		QColor                       _fillColor;
		bool                         _filled;
		double                       _magnitude;
		double                       _depth;
};


inline const QColor &OriginSymbol::color() const {
	return _color;
}

inline const QColor &OriginSymbol::fillColor() const {
	return _fillColor;
}


} // namespace Gui
} // namespace Seiscomp


#endif
