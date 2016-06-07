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

#define SEISCOMP_COMPONENT Gui::MapWidget

#include <seiscomp3/geo/geofeatureset.h>
#include <seiscomp3/gui/map/mapwidget.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/gui/map/texturecache.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/geo.h>

#include <cmath>

#ifdef WIN32
#undef min
#undef max
#endif

namespace Seiscomp {
namespace Gui {


namespace {


const static char *cmStrProjection = "Projection";
const static char *cmStrFilter = "Filter";
const static char *cmStrNearest = "Nearest";
const static char *cmStrBilinear = "Bilinear";
const static char *cmStrReload = "Reload";
const static char *cmStrGrid = "Show Grid";
const static char *cmStrCities = "Show Cities";
const static char *cmStrLayers = "Show Layers";
const static char *cmStrHideAll = "Hide All";


QString lat2String(double lat) {
	return QString("%1%2").arg(abs((int)lat)).arg(lat < 0?" S":lat > 0?" N":"");
}


QString lon2String(double lon) {
	lon = fmod(lon, 360.0);
	if ( lon < 0 ) lon += 360.0;
	if ( lon > 180.0 ) lon -= 360.0;

	return QString("%1%2").arg(abs((int)lon)).arg(lon < 0?" W":lon > 0?" E":"");
}


}


MapWidget::MapWidget(QWidget *parent, Qt::WFlags f)
 : QWidget(parent, f), _canvas(NULL) {
	init();
}


MapWidget::MapWidget(const MapsDesc &meta, QWidget *parent, Qt::WFlags f)
 : QWidget(parent, f), _canvas(meta) {
	init();
}


MapWidget::MapWidget(Map::ImageTree *mapTree, QWidget *parent, Qt::WFlags f)
 : QWidget(parent, f), _canvas(mapTree) {
	init();
}


MapWidget::~MapWidget() {}


void MapWidget::init() {
	_canvas.setBackgroundColor(palette().color(QPalette::Window));
	connect(&_canvas, SIGNAL(bufferUpdated()), this, SLOT(bufferUpdated()));
	connect(&_canvas, SIGNAL(projectionChanged(Seiscomp::Gui::Map::Projection*)),
	        this, SLOT(projectionChanged(Seiscomp::Gui::Map::Projection*)));
	connect(&_canvas, SIGNAL(customLayer(QPainter*)),
	        this, SLOT(drawCustomLayer(QPainter*)));
	connect(&_canvas, SIGNAL(updateRequested()), this, SLOT(update()));

	_isDragging = false;
	_isMeasuring = false;
	_filterMap = SCScheme.map.bilinearFilter;
	_canvas.setBilinearFilter(_filterMap);

	setFocusPolicy(Qt::StrongFocus);
	//setAttribute(Qt::WA_PaintOnScreen);

	try { _zoomSensitivity = SCApp->configGetDouble("map.zoom.sensitivity"); }
	catch ( ... ) { _zoomSensitivity = 0.5; }

	_zoomControls = new QWidget(this);
	QToolButton *zoomIn = new QToolButton;
	QToolButton *zoomOut = new QToolButton;
	QVBoxLayout *zoomLayout = new QVBoxLayout;
	_zoomControls->setLayout(zoomLayout);
	zoomLayout->addWidget(zoomIn);
	zoomLayout->addWidget(zoomOut);

	zoomIn->setIcon(QIcon(":/map/icons/zoomin.png"));
	zoomOut->setIcon(QIcon(":/map/icons/zoomout.png"));

	_zoomControls->move(0,0);
	_zoomControls->hide();

	connect(zoomIn, SIGNAL(pressed()), this, SLOT(zoomIn()));
	connect(zoomOut, SIGNAL(pressed()), this, SLOT(zoomOut()));
}


void MapWidget::projectionChanged(Seiscomp::Gui::Map::Projection *p) {
	if ( p == NULL )
		setAttribute(Qt::WA_OpaquePaintEvent, false);
	else
		setAttribute(Qt::WA_OpaquePaintEvent, p->isRectangular());
}


void MapWidget::zoomIn() {
	_canvas.setZoomLevel(_canvas.zoomLevel() * pow(2.0, _zoomSensitivity));
	update();
}


void MapWidget::zoomOut() {
	_canvas.setZoomLevel(_canvas.zoomLevel() / pow(2.0, _zoomSensitivity));
	update();
}


void MapWidget::setDrawGrid(bool e) {
	_canvas.setDrawGrid(e);
	update();
}


void MapWidget::setDrawLayers(bool e) {
	_canvas.setDrawLayers(e);
	update();
}


void MapWidget::setDrawCities(bool e) {
	_canvas.setDrawCities(e);
	update();
}


void MapWidget::draw(QPainter &painter) {
	_canvas.setPreviewMode(_isDragging || _isMeasuring);
	_canvas.setGrayScale(!isEnabled());
	_canvas.draw(painter);

	if ( _isMeasuring ) {
		painter.save();
		painter.setPen(QPen(Qt::red, 2));

		double dist = _canvas.drawGeoLine(painter, _measureStart, _measureEnd);

		QFont f = painter.font();
		painter.setFont(f);
		painter.setPen(QPen(Qt::red));
		painter.drawText(rect(), Qt::AlignLeft | Qt::AlignBottom | Qt::TextSingleLine,
		                 QString("Distance: %1 km / %2 deg")
		                 .arg(Math::Geo::deg2km(dist), 0, 'f', 1)
		                 .arg(dist, 0, 'f', 1));

		painter.restore();
	}
}


void MapWidget::paintEvent(QPaintEvent* e) {
	QPainter painter(this);
	draw(painter);
}


void MapWidget::resizeEvent(QResizeEvent* event) {
	_canvas.setSize(event->size().width(), event->size().height());
	_zoomControls->resize(_zoomControls->sizeHint());
}


void MapWidget::updateContextMenu(QMenu *menu) {
	QAction *action;

	_contextProjectionMenu = NULL;
	_contextFilterMenu = NULL;
	_contextLayerMenu = NULL;

	// Projection and Filters
	Map::ProjectionFactory::ServiceNames* services;
	services = Map::ProjectionFactory::Services();
	if ( services ) {
		if ( services->size() > 1 )
			_contextProjectionMenu = menu->addMenu(cmStrProjection);
		_contextFilterMenu = menu->addMenu(cmStrFilter);

		for ( Map::ProjectionFactory::ServiceNames::iterator it =
				services->begin(); it != services->end(); ++it ) {
			if ( _contextProjectionMenu ) {
				action = _contextProjectionMenu->addAction(it->c_str());
				if ( *it == _canvas.projectionName() )
					action->setEnabled(false);
			}
		}
		delete services;
	}

	action = _contextFilterMenu->addAction(cmStrNearest);
	action->setEnabled(_filterMap);
	action = _contextFilterMenu->addAction(cmStrBilinear);
	action->setEnabled(!_filterMap);

	// Layers (if any)

	QMenu* subMenu = _canvas.menu(this);
	if ( subMenu )
		menu->addMenu(subMenu);

	// Refresh
	action = menu->addAction(cmStrReload);

	// Grid
	action = menu->addAction(cmStrGrid);
	action->setCheckable(true);
	action->setChecked(_canvas.isDrawGridEnabled());

	// Cities
	action = menu->addAction(cmStrCities);
	action->setCheckable(true);
	action->setChecked(_canvas.isDrawCitiesEnabled());

	// Layers (if any)

	if ( !_canvas.layerProperties().empty() ) {
		// No sub categories available or all layers disabled: 1 enabled main menu entry
		if ( _canvas.layerProperties().size() == 1 || !_canvas.isDrawLayersEnabled() ) {
			action = menu->addAction(cmStrLayers);
			action->setCheckable(true);
			action->setChecked(_canvas.isDrawLayersEnabled());
		}
		// Subcategories available: create layer sub menu
		else {
			_contextLayerMenu = menu->addMenu(cmStrLayers);
			action = _contextLayerMenu->addAction(cmStrHideAll);
			action->setCheckable(true);
			_contextLayerMenu->addSeparator();

			std::vector<Map::LayerProperties*>::const_iterator it =
					_canvas.layerProperties().begin();
			const Map::LayerProperties *root = *it++;
			for ( ; it != _canvas.layerProperties().end(); ++it ) {
				if ( (*it)->parent != root ) continue;
				action = _contextLayerMenu->addAction((*it)->name.c_str());
				action->setCheckable(true);
				action->setChecked((*it)->visible);
			}
		}
	}
}


void MapWidget::executeContextMenuAction(QAction *action) {
	if ( action == NULL ) {
		_contextProjectionMenu = NULL;
		_contextFilterMenu = NULL;
		_contextLayerMenu = NULL;
		return;
	}

	if ( _contextProjectionMenu &&
	     action->parent() == _contextProjectionMenu )
		_canvas.setProjectionByName(action->text().toStdString().c_str());
	else if ( _contextFilterMenu &&
	          action->parent() == _contextFilterMenu ) {
		_filterMap = action->text() == cmStrBilinear;
		_canvas.setBilinearFilter(_filterMap);
	}
	else if ( action->text() == cmStrReload )
		_canvas.reload();
	else if ( action->text() == cmStrCities )
		_canvas.setDrawCities(action->isChecked());
	else if ( action->text() == cmStrLayers )
		_canvas.setDrawLayers(action->isChecked());
	else if ( action->text() == cmStrGrid )
		_canvas.setDrawGrid(action->isChecked());
	else if ( action->text() == cmStrLayers )
		_canvas.setDrawLayers(action->isChecked());
	else if ( _contextLayerMenu &&
			  action->parent() == _contextLayerMenu ) {
		if ( action->text() == cmStrHideAll )
			_canvas.setDrawLayers(false);
		else {
			// Find the LayerProperties which belongs to the action
			std::vector<Map::LayerProperties*>::const_iterator it =
				_canvas.layerProperties().begin();
			const Map::LayerProperties *root = *it++;
			Map::LayerProperties *prop = NULL;
			for ( ; it != _canvas.layerProperties().end(); ++it ) {
				if ( (*it)->parent != root ||
					 action->text() != (*it)->name.c_str() ) continue;
				prop = *it;
				break;
			}
			// If found set the visibility property of this
			// LayerProperties object and all children
			if ( prop ) {
				prop->visible = action->isChecked();
				it = _canvas.layerProperties().begin();
				for ( ; it != _canvas.layerProperties().end(); ++it ) {
					if ( !prop->isChild(*it) ) continue;
					(*it)->visible = action->isChecked();
				}
				_canvas.updateBuffer();
			}
		}
	}

	_contextProjectionMenu = NULL;
	_contextFilterMenu = NULL;
	_contextLayerMenu = NULL;

	update();
}

void MapWidget::contextMenuEvent(QContextMenuEvent *event) {
	if ( _canvas.filterContextMenuEvent(event, this) ) return;

	QMenu menu(this);

	updateContextMenu(&menu);

	// Evaluation action performed
	executeContextMenuAction(menu.exec(event->globalPos()));
}


void MapWidget::mousePressEvent(QMouseEvent* event) {
	if ( _canvas.filterMousePressEvent(event) ) return;

	if ( event->button() == Qt::LeftButton ) {
		_lastDraggingPosition = event->pos();
		_firstDrag = true;

		if ( event->modifiers() == Qt::NoModifier )
			_isDragging = true;
		else if ( event->modifiers() == Qt::ControlModifier ) {
			_isMeasuring = true;
			_canvas.projection()->unproject(_measureStart, _lastDraggingPosition);
		}
	}
}


void MapWidget::mouseReleaseEvent(QMouseEvent* event) {
	if ( event->button() == Qt::LeftButton ) {
		_isDragging = false;
		update();

		if ( _isMeasuring ) {
			_isMeasuring = false;
			update();
		}
	}
}


void MapWidget::mouseMoveEvent(QMouseEvent* event) {
	if ( _isDragging ) {
		if ( _firstDrag ) _firstDrag = false;

		QPoint delta = event->pos() - _lastDraggingPosition;
		_lastDraggingPosition = event->pos();

		_canvas.translate(delta);

		update();
	}
	else if ( _isMeasuring ) {
		_canvas.projection()->unproject(_measureEnd, event->pos());
		update();
	}
	else if ( !_canvas.filterMouseMoveEvent(event) )
		_zoomControls->setVisible(_zoomControls->geometry().contains(event->pos()));
}


void MapWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	if ( event->button() == Qt::LeftButton &&
	     !_canvas.filterMouseDoubleClickEvent(event) )
		_canvas.centerMap(event->pos());
}


void MapWidget::keyPressEvent(QKeyEvent* e) {
	e->accept();

	int key = e->key();

	switch ( key ) {
		case Qt::Key_Plus:
		case Qt::Key_I:
			if ( e->modifiers() == Qt::NoModifier )
				zoomIn();
			break;
		case Qt::Key_Minus:
		case Qt::Key_O:
			if ( e->modifiers() == Qt::NoModifier )
				zoomOut();
			break;
		case Qt::Key_Left:
			_canvas.translate(QPointF(-1.0,0.0));
			update();
			break;
		case Qt::Key_Right:
			_canvas.translate(QPointF(1.0,0.0));
			update();
			break;
		case Qt::Key_Up:
			_canvas.translate(QPointF(0.0,1.0));
			update();
			break;
		case Qt::Key_Down:
			_canvas.translate(QPointF(0.0,-1.0));
			update();
			break;
		case Qt::Key_G:
			_canvas.setDrawGrid(!_canvas.isDrawGridEnabled());
			break;
		case Qt::Key_C:
			_canvas.setDrawCities(!_canvas.isDrawCitiesEnabled());
			break;
		default:
			emit keyPressed(e);
			break;
	};
}


void MapWidget::wheelEvent(QWheelEvent *e) {
	double zoomDelta = (double)e->delta() * (1.0/120.0);
	if ( _canvas.setZoomLevel(_canvas.zoomLevel() * pow(2.0, zoomDelta*_zoomSensitivity)) )
		update();
}


void MapWidget::leaveEvent(QEvent *e) {
	QWidget::leaveEvent(e);
	_zoomControls->hide();
}


int MapWidget::heightForWidth(int w) const {
	return w;
}


void MapWidget::bufferUpdated() {}


}
}
