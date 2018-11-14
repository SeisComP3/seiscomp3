/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_GUI_MAP_LAYER_H__
#define __SEISCOMP_GUI_MAP_LAYER_H__


#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#endif

#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/map/legend.h>
#include <seiscomp3/gui/map/mapsymbol.h>

#include <QObject>


class QContextMenuEvent;
class QKeyEvent;
class QMouseEvent;
class QDialog;
class QMenu;
class QPainter;
class QWidget;


namespace Seiscomp {

namespace Config {

class Config;

}

namespace Gui {
namespace Map {


class Canvas;

DEFINE_SMARTPOINTER(Layer);

class SC_GUI_API Layer : public QObject, public Seiscomp::Core::BaseObject {
	Q_OBJECT

	public:
		enum UpdateHint {
			Redraw       = 0x00, // Just redraw
			Position     = 0x01, // Update the layers position
			RasterLayer  = 0x02  // Update the map base layer
		};

		Q_DECLARE_FLAGS(UpdateHints, UpdateHint)

		typedef QList<Legend*> Legends;

	public:
		Layer(QObject* parent = NULL);
		virtual ~Layer();

	public:
		virtual void setConfig(const std::string &/*config*/) {}
		virtual void init(const Config::Config&);
		virtual void draw(const Canvas*, QPainter&) {}

		virtual Layer &operator =(const Layer &other);
		virtual Layer *clone() const { return NULL; }

	signals:
		/// This signal is emitted when a legend was added to layer
		void legendAdded(Legend *legend);
		/// This signal is emitted when a legend was removed from layer
		void legendRemoved(Legend *legend);

	public slots:
		void setAntiAliasingEnabled(bool);
		virtual void setVisible(bool);
		virtual void show();
		virtual void hide();

	public:
		void setName(const QString&);
		const QString &name() const;

		void setDescription(const QString&);
		const QString &description() const;

		/**
		 * @brief Adds the legend to the layer. The ownership of the object
		 *        goes to the layer.
		 * @param legend The legend
		 * @return True, if the legend was added
		 */
		bool addLegend(Legend *legend);

		/**
		 * @brief Removes the legend from the layer and deletes the object.
		 * @param legend The legend
		 * @return True, if the legend was removed
		 */
		bool removeLegend(Legend *legend);

		int legendCount() const { return _legends.count(); }
		Legend *legend(int i) const;

		const QList<Legend*> &legends() const { return _legends; }

		bool isVisible() const;
		bool isAntiAliasingEnabled() const;

		void setDirty();
		bool isDirty() const;

		Canvas *canvas() const { return _canvas; }

	public:
		/**
		 * @brief Convenience function that returns the size in pixels of the
		 *        layer. By default it forwards the request to the canvas if
		 *        the layer is attached to canvas or an invalid size otherwise.
		 * @return The size of the layer.
		 */
		virtual QSize size() const;

		virtual void calculateMapPosition(const Canvas *canvas);
		virtual bool isInside(const QMouseEvent *event, const QPointF &geoPos);
		virtual void baseBufferUpdated(Canvas *canvas, QPainter &painter);
		virtual void bufferUpdated(Canvas *canvas, QPainter &painter);

		virtual void handleEnterEvent();
		virtual void handleLeaveEvent();

		virtual bool filterContextMenuEvent(QContextMenuEvent*, QWidget*);
		virtual bool filterKeyPressEvent(QKeyEvent *event);
		virtual bool filterKeyReleaseEvent(QKeyEvent *event);
		virtual bool filterMouseMoveEvent(QMouseEvent *event, const QPointF &geoPos);
		virtual bool filterMousePressEvent(QMouseEvent *event, const QPointF &geoPos);
		virtual bool filterMouseReleaseEvent(QMouseEvent *event, const QPointF &geoPos);
		virtual bool filterMouseDoubleClickEvent(QMouseEvent *event, const QPointF &geoPos);

		virtual QMenu *menu(QMenu*) const;

	signals:
		void updateRequested(const Layer::UpdateHints& = UpdateHints());

	private:
		Canvas  *_canvas;
		QString  _name;
		QString  _description;
		bool     _visible;
		bool     _antiAliasing;
		Legends  _legends;
		bool     _dirty;

	friend class Canvas;
};



inline bool Layer::isDirty() const {
	return _dirty;
}


Q_DECLARE_OPERATORS_FOR_FLAGS(Layer::UpdateHints)

DEFINE_INTERFACE_FACTORY(Layer);

#define REGISTER_LAYER_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Gui::Map::Layer, Class> __##Class##InterfaceFactory__(Service)


} // namespace Map
} // namespce Gui
} // namespace Seiscomp


#endif
