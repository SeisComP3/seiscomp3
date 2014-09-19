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


#ifndef __SEISCOMP_GUI_FMSYMBOL_H__
#define __SEISCOMP_GUI_FMSYMBOL_H__


#include <seiscomp3/gui/core/tensorrenderer.h>
#include <seiscomp3/gui/map/mapsymbol.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API TensorSymbol : public Map::Symbol
{

	public:
		TensorSymbol(const Math::Tensor2Sd &t,
		             Map::Decorator* decorator = NULL);
		~TensorSymbol();

		void setShadingEnabled(bool);
		void setDrawConnectorEnabled(bool);
		void setBorderColor(QColor);
		void setTColor(QColor);
		void setPColor(QColor);

		void setPosition(QPointF geoPosition);
		void setOffset(QPoint offset);


	public:
		virtual void calculateMapPosition(const Map::Canvas *canvas);
		virtual bool hasValidMapPosition() const;
		virtual bool isInside(int x, int y) const;


	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter);


	private:
		void resize(int w, int h);


	private:
		TensorRenderer     _renderer;
		QImage             _buffer;
		Math::Tensor2Sd    _tensor;
		Math::Matrix3f     _rotation;

		QSize              _lastSize;
		QSize              _size;
		QPointF            _geoPosition;
		QPoint             _offset;
		QPoint             _mapPosition;

		bool               _drawLocationConnector;

};



}
}

#endif
