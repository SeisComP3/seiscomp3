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

class SaveBNADialog : public QDialog {
	Q_OBJECT

	public:
		SaveBNADialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

	public:
		QLineEdit *name;
		QCheckBox *closedPolygon;
		QCheckBox *fileAppend;
		QSpinBox  *rank;
		QLineEdit *filename;
};


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

		bool isGrayScale() const;

		virtual void draw(QPainter&);


	public slots:
		/**
		 * @brief Sets map rendering in grayscale mode even if the widget is
		 *        enabled.
		 * @param f The enable flag
		 */
		void setGrayScale(bool f);

		void setDrawGrid(bool);
		void setDrawLayers(bool);
		void setDrawCities(bool);
		void setDrawLegends(bool);


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
		void keyReleaseEvent(QKeyEvent*);
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
		bool     _isMeasureDragging;
		bool     _filterMap;
		bool     _forceGrayScale;

		QVector<QPointF> _measurePoints;
		QString          _measureText;
		SaveBNADialog   *_measureBNADialog;
		QPoint   _lastDraggingPosition;

		QMenu   *_contextProjectionMenu;
		QMenu   *_contextFilterMenu;

		double   _zoomSensitivity;

		QWidget *_zoomControls;
};


}
}

#endif
