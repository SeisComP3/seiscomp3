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


#ifndef __SEISCOMP_GUI_DECORATOR_H__
#define __SEISCOMP_GUI_DECORATOR_H__

#include <memory>

#include <QPainter>

#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {
namespace Map {

class Canvas;


/** \class Decorator
 *
 * Abstract class to decorate a Symbol derived class. When a derived decorator is
 * passed to a symbol the map and baseObject member are automatically set by the
 * respective symbol. The same is true for all other (nested) decorator which are
 * associated whith the passed decorator. Therefore the user is obliged to assure
 * that the other decorator operate on the same type of datamodel object (pick, origin...)
 */

class SC_GUI_API Decorator {

		// ------------------------------------------------------------------
		// X'struction
		// ------------------------------------------------------------------
	public:
		Decorator(Decorator* decorator = NULL)
		 : _decorator(decorator),
		   _visible(true) {
		}

		virtual ~Decorator() {}


		// ------------------------------------------------------------------
		// Public Interface
		// ------------------------------------------------------------------
	public:
		void draw(const Canvas *canvas, QPainter& painter) {
			if ( _decorator.get() )
				_decorator->draw(canvas, painter);
			customDraw(canvas, painter);
		}

		void setVisible(bool visible) {
			if ( _decorator.get() )
				_decorator->setVisible(visible);
			_visible = visible;
		}

		bool isVisible() const {
			return _visible;
		}


		// ------------------------------------------------------------------
		// Protected Interface
		// ------------------------------------------------------------------
	protected:
		virtual void customDraw(const Canvas *canvas, QPainter &painter) = 0;


		// ------------------------------------------------------------------
		// Private Members
		// ------------------------------------------------------------------
	private:
		std::auto_ptr<Decorator> _decorator;
		bool                     _visible;

};


} // namespace Map
} // namespace Gui
} // namespace Seiscomp

#endif
