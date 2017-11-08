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
	// ----------------------------------------------------------------------
	// Public types
	// ----------------------------------------------------------------------
	public:
		enum Priority {
			NONE = 0x0,
			LOW,
			MEDIUM,
			HIGH
		};


	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	protected:
		//! C'tor
		Symbol(Decorator* decorator = NULL);
		Symbol(double lat, double lon, Decorator* decorator = NULL);
		Symbol(const QPointF &latlon, Decorator* decorator = NULL);

	public:
		virtual ~Symbol();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Draws the symbol. If a decorator is attached to the symbol
		 *        it is drawn first. To implement the actual painting of a
		 *        symbol, implement @customDraw.
		 * @param canvas The canvas the symbol is rendered on.
		 * @param painter The painter object.
		 */
		void draw(const Map::Canvas *canvas, QPainter &painter);

		/**
		 * @brief Sets an arbitrary type. The canvas does not use this attribute
		 *        for rendering. The type can be used in application code for
		 *        any purpose.
		 * @param type The new type id
		 */
		void setType(int type);

		int type() const;

		/**
		 * @brief Sets an arbitrary ID. The canvas does not use this attribute
		 *        for rendering. The ID can be used in application code for
		 *        any purpose.
		 * @param id The new ID
		 */
		void setID(const std::string &id);

		/**
		 * @brief Returns the (arbitrary) id of the symbol.
		 * @return A string reference to the id
		 */
		const std::string &id() const;

		//! Returns the current render priority.
		Priority priority() const;

		/**
		 * @brief Sets the symbols priority on the map. The higher the priority
		 *        the higher the layer it is rendered onto.
		 * @param priority The priority to be set.
		 */
		void setPriority(Priority priority);

		//! Returns the state of the clipping flag
		bool isClipped() const;

		//! Toggles the visibility flag
		void setVisible(bool visible);

		//! Returns the state of the visibility flag
		bool isVisible() const;

		//! Sets the current size
		void setSize(const QSize &size);

		//! Returns the current size
		const QSize &size() const;

		void setLocation(double latitude, double longitude);
		void setLocation(const QPointF &latlon);
		void setLatitude(double lat);
		void setLongitude(double lon);

		//! Returns the current map location
		const QPointF location() const;

		double longitude() const;
		double latitude() const;

		//! Return the current map position
		const QPoint &pos() const;

		//! Returns whether the map position is valid or not
		bool hasValidPosition() const;

		//! Returns the screen x position
		int x() const;

		//! Returns the screen y position
		int y() const;

		void setDecorator(Decorator *);

		//! Returns the attached decorator
		Decorator *decorator() const;


	// ----------------------------------------------------------------------
	// Public symbol interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Check whether a point in canvas coordinates is inside
		 *        the symbol. Because a symbol has no associated shape, it
		 *        can't provide a default implementation.
		 * @param x The x coordinate
		 * @param y The y coordinate
		 * @return true is (x,y) is inside the symbol, false otherwise
		 */
		virtual bool isInside(int x, int y) const = 0;

		/**
		 * @brief Calculates the map position of the symbols geo location.
		 *        The default implementation projects the geo location to the
		 *        map position and sets the clipping according to the result.
		 * The default implementation should be sufficient for almost any use
		 * case. If required, then derived classes can add their own
		 * implementation for this method.
		 * @param canvas
		 */
		virtual void calculateMapPosition(const Map::Canvas *canvas);

		/**
		 * @brief Draws a symbol. This method must be implemented by derived
		 *        classes.
		 * @param canvas The canvas to paint onto.
		 * @param painter The painter to paint with.
		 */
		virtual void customDraw(const Map::Canvas *canvas, QPainter &painter) = 0;


	// ----------------------------------------------------------------------
	// Protected symbol interface
	// ----------------------------------------------------------------------
	protected:
		/**
		 * @brief Sets the clipping flag
		 * @param clipped The flag
		 */
		void setClipped(bool clipped);


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	protected:
		int                           _type;
		std::string                   _id;
		Priority                      _priority;
		bool                          _clipped;
		bool                          _visible;
		QPointF                       _location;
		QPoint                        _position;
		QSize                         _size;
		std::auto_ptr<Decorator>      _decorator;

};



inline int Symbol::type() const {
	return _type;
}

inline const std::string &Symbol::id() const {
	return _id;
}

inline Symbol::Priority Symbol::priority() const {
	return _priority;
}

inline bool Symbol::isClipped() const {
	return _clipped;
}

inline bool Symbol::isVisible() const {
	return _visible;
}

inline const QSize &Symbol::size() const {
	return _size;
}

inline const QPointF Symbol::location() const {
	return _location;
}

inline double Symbol::latitude() const {
	return _location.y();
}

inline double Symbol::longitude() const {
	return _location.x();
}

inline const QPoint &Symbol::pos() const {
	return _position;
}

inline int Symbol::x() const {
	return _position.x();
}

inline int Symbol::y() const {
	return _position.y();
}

inline bool Symbol::hasValidPosition() const {
	return _position.x() >= 0 || _position.y() >= 0;
}

inline Decorator *Symbol::decorator() const {
	return _decorator.get();
}


} // namespace Map
} // namespace Gui
} // namespace Seiscomp


#endif
