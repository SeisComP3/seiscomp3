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



#ifndef __SEISCOMP_GUI_MAPSYMBOL_H__
#define __SEISCOMP_GUI_MAPSYMBOL_H__

#include <QPolygon>
#include <QColor>

#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#endif
#include <seiscomp3/gui/map/decorator.h>
#include <seiscomp3/gui/qt4.h>

class QPainter;

namespace Seiscomp {
namespace Gui {
namespace Map {


class Canvas;


class SC_GUI_API Symbol {
	DECLARE_RTTI;

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	public:
		enum Priority { NONE = 0x0, LOW, MEDIUM, HIGH };

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		virtual ~Symbol() {}

	protected:
		Symbol(Decorator* decorator = NULL);

	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		void draw(const Map::Canvas *canvas, QPainter &painter);
		virtual void update();
		virtual void calculateMapPosition(const Map::Canvas *canvas);
		virtual bool hasValidMapPosition() const;
		virtual bool isClipped(const Map::Canvas *canvas) const;
		virtual bool isInside(int x, int y) const;

		const std::string& id() const;
		void setID(const std::string& id);

		// This is for the drawing order
		Priority priority() const;
		void setPriority(Priority priority);

		bool isClipped() const;
		void setVisible(bool visible);
		bool isVisible() const;

		const QSize& size() const;
		void setSize(const QSize& size);

		Decorator* decorator() const;

	// ----------------------------------------------------------------------
	// Protected interface
	// ----------------------------------------------------------------------
	protected:
		void setClipped(bool clipped);
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter) = 0;

	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		std::string                   _id;
		Priority                      _priority;
		bool                          _clipped;
		bool                          _visible;
		QSize                         _size;
		std::auto_ptr<Decorator>      _decorator;

};


} // namespace Map
} // namespace Gui
} // namespace Seiscomp

#endif
