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

#ifndef __SEISCOMP_GUI_MAP_LEGEND_H__
#define __SEISCOMP_GUI_MAP_LEGEND_H__

#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#endif

#include <seiscomp3/gui/qt4.h>

#include <QFont>
#include <QRect>
#include <QSize>
#include <QString>
#include <QObject>
#include <QColor>

class QPainter;


namespace Seiscomp {
namespace Gui {
namespace Map {


class Canvas;
class Layer;


class SC_GUI_API Legend : public QObject {
	Q_OBJECT

	public:
		Legend(QObject *parent = NULL);
		Legend(const QString&, QObject *parent = NULL);
		virtual ~Legend() {}

		Legend &operator =(const Legend &other);

		virtual void bringToFront();

		virtual Legend* clone() const { return NULL; }

		virtual void draw(const QRect&, QPainter&) = 0;

		Qt::Alignment alignment() const { return _alignment; }
		void setArea(Qt::Alignment alignment) { _alignment = alignment; }

		void setEnabled(bool);
		bool isEnabled() const;

		const QFont& font() const { return _font; }
		void setFont(const QFont &font) { _font = font; }

		Layer *layer() const { return _layer; }
		void setLayer(Layer* layer) { _layer = layer; }

		int margin() const { return _margin; }
		void setMargin(int margin) { _margin = margin; }

		int spacing() const { return _spacing; }
		void setSpacing(int spacing) { _spacing = spacing; }

		const QString& title() const { return _title; }
		void setTitle(const QString &title) { _title = title; }

		const QFont& titleFont() const { return _titleFont; }
		void setTitleFont(const QFont &font) { _titleFont = font; }

		bool isVisible() const { return _visible; }

		const QSize &size() const { return _size; }

		virtual void contextResizeEvent(const QSize &size) {}


	signals:
		void bringToFrontRequested(Seiscomp::Gui::Map::Legend*);
		void enabled(Seiscomp::Gui::Map::Legend*, bool);
		void visibilityChanged(bool);


	private:
		void setVisible(bool visible) {
			_visible = visible;
			emit visibilityChanged(visible);
		}


	protected:
		int                           _margin;
		int                           _spacing;
		QFont                         _font;
		QFont                         _titleFont;
		QSize                         _size;
		Layer                        *_layer;
		QString                       _title;
		Qt::Alignment                 _alignment;
		bool                          _enabled;
		QPoint                        _pos;
		bool                          _visible;


	friend class Canvas;
};


class SC_GUI_API StandardLegend : public Legend {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		StandardLegend(QObject *parent);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void addItem(const QColor &c, const QString &l);

		void setMaximumColumns(int columns);
		void setOrientation(Qt::Orientation orientation);


	// ----------------------------------------------------------------------
	//  Legend interface
	// ----------------------------------------------------------------------
	public:
		virtual void contextResizeEvent(const QSize &size);
		virtual void draw(const QRect &r, QPainter &p);


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		void updateLayout(const QSize &size);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		struct Item {
			Item() {}
			Item(const QColor &c, const QString &l) : color(c), label(l) {}

			QColor  color;
			QString label;
		};

		Qt:: Orientation _orientation;
		QList<Item>      _items;
		int              _columns;
		int              _columnWidth;
		int              _maxColumns;
		bool             _layoutDirty;

};


} // namespace Map
} // namespce Gui
} // namespace Seiscomp


#endif
