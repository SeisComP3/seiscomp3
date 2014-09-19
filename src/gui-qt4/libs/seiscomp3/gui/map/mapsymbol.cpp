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


#include <QPoint>

#include <seiscomp3/gui/map/mapwidget.h>
#include <seiscomp3/gui/map/mapsymbol.h>


namespace Seiscomp {
namespace Gui {
namespace Map {


IMPLEMENT_ROOT_RTTI(Symbol, "Map::Symbol")
IMPLEMENT_RTTI_METHODS(Symbol)



Symbol::Symbol(Decorator* decorator)
 : _priority(NONE),
   _clipped(false),
   _visible(true),
   _size(0, 0),
   _decorator(decorator) {
}




void Symbol::draw(const Map::Canvas *canvas, QPainter &painter) {
	if ( _decorator.get() )
		_decorator->draw(canvas, painter);
	customDraw(canvas, painter);
}




void Symbol::update() {
}




void Symbol::calculateMapPosition(const Map::Canvas *canvas) {
}





bool Symbol::hasValidMapPosition() const {
	return true;
}



bool Symbol::isClipped(const Map::Canvas *canvas) const {
	return false;
}




const std::string& Symbol::id() const {
	return _id;
}




void Symbol::setID(const std::string& id) {
	_id = id;
}




bool Symbol::isInside(int x, int y) const {
	return true;
}





Symbol::Priority Symbol::priority() const {
	return _priority;
}




void Symbol::setPriority(Priority priority) {
	_priority = priority;
}




bool Symbol::isClipped() const {
	return _clipped;
}




void Symbol::setVisible(bool val) {
	_visible = val;
}




bool Symbol::isVisible() const {
	return _visible;
}




void Symbol::setClipped(bool clipped) {
	_clipped = clipped;
}




Decorator* Symbol::decorator() const {
	return _decorator.get();
}




const QSize& Symbol::size() const {
	return _size;
}




void Symbol::setSize(const QSize& size) {
	_size = size;
}


} // namespace Map
} // namespace Gui
} // namespace Seiscomp
