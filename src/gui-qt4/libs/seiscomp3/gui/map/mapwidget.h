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



#ifndef __SEISCOMP_GUI_MAPWIDGET_H__
#define __SEISCOMP_GUI_MAPWIDGET_H__


#include <seiscomp3/gui/map/canvas.h>

#ifndef Q_MOC_RUN
#include <seiscomp3/math/coord.h>
#endif

#include <QColor>
#include <QPen>
#include <QtGui>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API MapWidget : public QWidget {
	Q_OBJECT

	public:
		MapWidget(QWidget *parent = 0, Qt::WFlags f = 0);
		MapWidget(const MapsDesc &meta, QWidget *parent = 0, Qt::WFlags f = 0);
		MapWidget(Map::ImageTree *mapTree, QWidget *parent = 0, Qt::WFlags f = 0);
		virtual ~MapWidget();

		Map::Canvas &canvas() { return _canvas; }
		const Map::Canvas &canvas() const { return _canvas; }

		int heightForWidth(int w) const;

		virtual void draw(QPainter&);


	public slots:
		void setDrawGrid(bool);
		void setDrawLayers(bool);
		void setDrawCities(bool);


	protected slots:
		virtual void bufferUpdated();
		virtual void drawCustomLayer(QPainter *p) {}


	private slots:
		void projectionChanged(Seiscomp::Gui::Map::Projection*);
		void zoomIn();
		void zoomOut();


	signals:
		void keyPressed(QKeyEvent*);


	protected:
		void paintEvent(QPaintEvent*);
		void resizeEvent(QResizeEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);
		void keyPressEvent(QKeyEvent*);
		void wheelEvent(QWheelEvent*);
		void contextMenuEvent(QContextMenuEvent*);
		void leaveEvent(QEvent*);

		void updateContextMenu(QMenu *menu);
		void executeContextMenuAction(QAction *action);


	private:
		void init();


	private:
		Map::Canvas _canvas;

		std::string _currentProjection;

		bool     _firstDrag;
		bool     _isDragging;
		bool     _isMeasuring;
		bool     _filterMap;

		QPointF  _measureStart;
		QPointF  _measureEnd;
		QPoint   _lastDraggingPosition;

		QMenu   *_contextProjectionMenu;
		QMenu   *_contextFilterMenu;
		QMenu   *_contextLayerMenu;

		double   _zoomSensitivity;

		QWidget *_zoomControls;
};


}
}

#endif
