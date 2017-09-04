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

const static char *cmStrMeasure = "Measurements";
const static char *cmStrMeasureClipboard = "Copy to Clipboard";
const static char *cmStrMeasureSaveBNA = "Save as BNA File";
const static char *cmStrProjection = "Projection";
const static char *cmStrFilter = "Filter";
const static char *cmStrNearest = "Nearest";
const static char *cmStrBilinear = "Bilinear";
const static char *cmStrReload = "Reload";
const static char *cmStrGrid = "Show Grid";
const static char *cmStrCities = "Show Cities";
const static char *cmStrLayers = "Show Layers";
const static char *cmStrLayersGeofeatures = "Features";
const static char *cmStrHideAll = "Hide All";


bool isChildOf(QObject *child, QObject *parent) {
	QObject *p = child->parent();
	while ( p != NULL ) {
		if ( p == parent )
			return true;
		p = p->parent();
	}

	return false;
}


inline QString lat2String(double lat, int precision) {
	return QString("%1%2")
	        .arg(fabs(lat), 3+precision, 'f', precision)
	        .arg(lat < 0?" S":lat > 0?" N":"");
}

inline QString lon2String(double lon, int precision) {
	lon = fmod(lon, 360.0);
	if ( lon < 0 ) lon += 360.0;
	if ( lon > 180.0 ) lon -= 360.0;

	return QString("%1%2")
	        .arg(fabs(lon), 4+precision, 'f', precision)
	        .arg(lon < 0?" W":lon > 0?" E":"");
}


inline double hav(double x) {
	return (1.0 - cos(x)) / 2.0;
}

// TODO: fix poly area for sourth pole
double polyArea(const QVector<QPointF> &coords) {
	double lat1, lon1, lat2, lon2, e;
	double sum = 0.0;

	for ( int i = 0 ; i < coords.size() ; ++i ) {
		lat1 = coords[i].y() * M_PI / 180.0;
		lon1 = coords[i].x() * M_PI / 180.0;
		lat2 = coords[(i + 1) % coords.size()].y() * M_PI / 180.0;
		lon2 = coords[(i + 1) % coords.size()].x() * M_PI / 180.0;

		if ( lon1 == lon2 )
			continue;

		double h = hav(lat2 - lat1) + cos(lat1) * cos(lat2) * hav(lon2 - lon1);

		double a = 2 * asin(sqrt(h));
		double b = M_PI / 2.0 - lat2;
		double c = M_PI / 2.0 - lat1;
		double s = 0.5 * (a + b + c);
		double t = tan(s / 2.0 ) * tan((s - a) / 2.0) *
		           tan((s - b) / 2.0) * tan((s - c) / 2);

		e = fabs(4.0 * atan(sqrt(fabs(t))));
		if ( lon2 < lon1 )
			e = -e;
		if ( fabs(lon2 - lon1) > M_PI ) {
			e = -e;
		}
		sum += e;
	}

	return fabs(sum) * 6378.137 * 6378.137;
}

} // ns anonymous

SaveBNADialog::SaveBNADialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f) {
	this->setWindowTitle(cmStrMeasureSaveBNA);
	QGridLayout *layout = new QGridLayout();
	int row = 0;

	// polygon name
	layout->addWidget(new QLabel("Name"), row, 0);
	name = new QLineEdit("", this);
	name->setToolTip("Name of the polygon, may be plotted on map");
	layout->addWidget(name, row++, 1);

	// rank
	layout->addWidget(new QLabel("Rank"), row, 0);
	rank = new QSpinBox(this);
	rank->setMinimum(1);
	rank->setMaximum(1000);
	rank->setValue(1);
	rank->setToolTip("Zoom level the polygon will become visible");
	layout->addWidget(rank, row++, 1);

	// is closed polygon
	closedPolygon = new QCheckBox("Closed Polygon");
	closedPolygon->setToolTip("Defines if your line stroke should be saved as "
	                          "closed polygon or an open polyline");
	closedPolygon->setChecked(true);
	layout->addWidget(closedPolygon, row++, 1);

	// file name
	layout->addWidget(new QLabel("File Name"), row, 0);
	filename = new QLineEdit(QString("%1/bna/custom.bna")
	        .arg(Seiscomp::Environment::Instance()->configDir().c_str()));
	filename->setToolTip("Path and file name");
	layout->addWidget(filename, row++, 1);

	// is closed polygon
	fileAppend = new QCheckBox("Append to File");
	fileAppend->setToolTip("Defines if the new polygon should be appended to an "
	                       "existing file or if the file should be overridden");
	fileAppend->setChecked(true);
	layout->addWidget(fileAppend, row++, 1);

	// OK + Cancel buttons
	QDialogButtonBox * buttonBox =
	        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(buttonBox, row++, 0, 1, 0);
	this->setLayout(layout);
	this->setMinimumSize(450,0);
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
	_isMeasureDragging = false;
	_filterMap = SCScheme.map.bilinearFilter;
	_canvas.setBilinearFilter(_filterMap);

	setMouseTracking(true);
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

	_measureBNADialog = NULL;
	_forceGrayScale = false;
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


void MapWidget::setDrawLegends(bool e) {
	_canvas.setDrawLegends(e);
	update();
}


void MapWidget::setGrayScale(bool f) {
	if ( _forceGrayScale == f ) return;
	_forceGrayScale = f;
	update();
}


bool MapWidget::isGrayScale() const {
	return _forceGrayScale;
}


void MapWidget::draw(QPainter &painter) {
	_canvas.setPreviewMode(_isDragging || _isMeasureDragging);
	_canvas.setGrayScale(!isEnabled() || _forceGrayScale);
	_canvas.draw(painter);

	if ( _isMeasuring ) {
		painter.save();
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setPen(QPen(Qt::red, 2));

		// draw geo line
		QPoint p;
		double dist = 0.0;
		_canvas.projection()->project(p, _measurePoints[0]);
#if QT_VERSION >= 0x040400
		painter.drawEllipse(QPointF(p), 1.3f, 1.3f);
#else
		painter.drawEllipse(QRectF(p.x()-1.3f, p.y()-1.3f, 2.6f, 2.6f));
#endif
		for ( int i = 1; i < _measurePoints.size(); ++i ) {
			_canvas.projection()->project(p, _measurePoints[i]);
#if QT_VERSION >= 0x040400
			painter.drawEllipse(QPointF(p), 1.3f, 1.3f);
#else
			painter.drawEllipse(QRectF(p.x()-1.3f, p.y()-1.3f, 2.6f, 2.6f));
#endif
			dist += _canvas.drawGeoLine(painter, _measurePoints[i-1], _measurePoints[i]);
		}

		QString aziArea;
		if ( _measurePoints.size() > 2 ) {
			painter.save();
			QPen pen = QPen(Qt::red, 1, Qt::DashLine);
			QVector<qreal> dashes;
			dashes << 3 << 7;
			pen.setDashPattern(dashes);
			painter.setPen(pen);
			_canvas.drawGeoLine(painter, _measurePoints.last(), _measurePoints.first());
			painter.restore();
			aziArea = QString("Area    : %1 km²").arg(polyArea(_measurePoints));
		}
		else {
			double tmp, azi1, azi2;
			Math::Geo::delazi(_measurePoints.first().y(), _measurePoints.first().x(),
			                  _measurePoints.last().y(), _measurePoints.last().x(),
			                  &tmp, &azi1, &azi2);
			aziArea = QString("AZ / BAZ: %1 ° / %2 °")
			                  .arg(azi1, 0, 'f', 1)
			                  .arg(azi2, 0, 'f', 1);
		}

		int precision = 0;
		if ( _canvas.projection()->zoom() > 0 ) {
			precision = (int) log10(_canvas.projection()->zoom());
		}
		++precision;

		QString distStr = QString("Distance: %1 km / %2 °")
		                  .arg(Math::Geo::deg2km(dist), 0, 'f', precision)
		                  .arg(dist, 0, 'f', precision + 2);

		QFont f = painter.font();
		QFont mf = f;
		mf.setFamily("Monospace");
		mf.setStyleHint(QFont::TypeWriter);

		QFontMetrics mfm(mf);
		QFontMetrics fm(f);
		int h = mfm.height() * 4 + fm.height();
		int padding = fm.width(" ");
		QRect r = QRect(0, rect().height() - h, mfm.width(distStr) + 2*padding, h);
		painter.setPen(QPen(Qt::black));
		painter.fillRect(r, QBrush(QColor(255, 255, 255, 140)));

		r.setLeft(padding);
		painter.setFont(mf);
		_measureText = QString("Start   : %1 / %2\n"
		                       "End     : %3 / %4\n"
		                       "%5\n"
		                       "%6")
		               .arg(lat2String(_measurePoints.first().y(), precision))
		               .arg(lon2String(_measurePoints.first().x(), precision))
		               .arg(lat2String(_measurePoints.last().y(), precision))
		               .arg(lon2String(_measurePoints.last().x(), precision))
		               .arg(distStr)
		               .arg(aziArea);

		painter.drawText(r, Qt::AlignLeft, _measureText);

		r.setTop(rect().height() - fm.height());
		r.setRight(r.width()-padding);
		painter.setFont(f);
		painter.drawText(r, Qt::AlignRight, "(right click to copy/save)");
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

	// Copy Measurements
	if ( !_measureText.isEmpty() ) {
		QMenu *measurementsMenu = menu->addMenu(cmStrMeasure);
		measurementsMenu->addAction(cmStrMeasureClipboard);
		measurementsMenu->addAction(cmStrMeasureSaveBNA);
	}

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

	QMenu* subMenu = _canvas.menu(menu);
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

	int fixedLayers = 2; // cities + grid
	int fixedGeoFeatures = 1; // root feature
	int layerCount = (_canvas.layerProperties().size()-fixedGeoFeatures) + (_canvas.layerCount()-fixedLayers);
	if ( layerCount > 0 ) {
		layerCount = _canvas.layerCount()-fixedLayers;

		_contextLayerMenu = menu->addMenu(cmStrLayers);

		if ( (int)_canvas.layerProperties().size() > fixedGeoFeatures ) {
			if ( !_canvas.isDrawLayersEnabled() ) {
				action = _contextLayerMenu->addAction(cmStrLayersGeofeatures);
				action->setCheckable(true);
				action->setChecked(false);
			}
			else {
				QMenu *subMenu;

				if ( layerCount > 0 )
					subMenu = _contextLayerMenu->addMenu(cmStrLayersGeofeatures);
				else
					subMenu = _contextLayerMenu;

				action = subMenu->addAction(cmStrHideAll);
				action->setCheckable(true);
				subMenu->addSeparator();

				std::vector<Map::LayerProperties*>::const_iterator it = _canvas.layerProperties().begin();
				const Map::LayerProperties *root = *it++;
				for ( ; it != _canvas.layerProperties().end(); ++it ) {
					if ( (*it)->parent != root ) continue;
					action = subMenu->addAction((*it)->name.c_str());
					action->setCheckable(true);
					action->setChecked((*it)->visible);
					action->setData(QVariant::fromValue<void*>(*it));
				}
			}
		}

		if ( layerCount > 0 ) {
			for ( int i = 0; i < layerCount; ++i ) {
				Map::Layer *layer = _canvas.layer(i+fixedLayers);
				// What to do with empty names?
				if ( layer->name().isEmpty() ) continue;
				action = _contextLayerMenu->addAction(layer->name());
				action->setCheckable(true);
				action->setChecked(layer->isVisible());
				connect(action, SIGNAL(toggled(bool)), layer, SLOT(setVisible(bool)));
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

	if ( _contextProjectionMenu && action->parent() == _contextProjectionMenu )
		_canvas.setProjectionByName(action->text().toStdString().c_str());
	else if ( _contextFilterMenu && action->parent() == _contextFilterMenu ) {
		_filterMap = action->text() == cmStrBilinear;
		_canvas.setBilinearFilter(_filterMap);
	}
	else if ( action->text() == cmStrMeasureClipboard ) {
		QString text = _measureText;
		text.append("\n\nlat lon");
		for ( int i = 0; i < _measurePoints.size(); ++i ) {
			text.append(QString("\n%1 %2").arg(_measurePoints[i].y())
			                              .arg(_measurePoints[i].x()));
		}
		QApplication::clipboard()->setText(text);
	}
	else if ( action->text() == cmStrMeasureSaveBNA ) {
		if ( !_measureBNADialog ) {
			_measureBNADialog = new SaveBNADialog(this);
		}
		while ( _measureBNADialog->exec() ) {
			bool append = _measureBNADialog->fileAppend->isChecked();
			QFileInfo fileInfo(_measureBNADialog->filename->text());
			if ( fileInfo.isDir() ) {
				QMessageBox::warning(this, "Invalid file name",
				                     "The specified file is a directory");
				continue;
			}

			QDir dir = fileInfo.absoluteDir();
			if ( !dir.exists() && !dir.mkpath(".") ) {
				QMessageBox::warning(this, "Error creating path",
				                     QString("Could not create file path: %1")
				                            .arg(dir.absolutePath()));
				continue;
			}

			if ( !append && fileInfo.isFile() && QMessageBox::question(
			         this, "File exists", "The specified file already exists. "
			         "Do you want to override it?",
			         QMessageBox::Yes|QMessageBox::No) == QMessageBox::No ) {
				continue;
			}

			QFile file(fileInfo.absoluteFilePath());
			if ( !file.open(QIODevice::WriteOnly |
			                (append ? QIODevice::Append : QIODevice::Truncate)) ) {
				QMessageBox::warning(this, "Could not open file",
				                     QString("Could not open file for writing: %1")
				                            .arg(fileInfo.absoluteFilePath()));
				continue;
			}

			QTextStream stream(&file);
			QString header = QString("\"%1\",\"rank %2\",%3")
			    .arg(_measureBNADialog->name->text())
			    .arg(_measureBNADialog->rank->value())
			    .arg(_measureBNADialog->closedPolygon->isChecked()?_measurePoints.size():-_measurePoints.size());
			stream << header << endl;
			for ( int i = 0; i < _measurePoints.size(); ++i ) {
				stream << _measurePoints[i].x() << ","
				       << _measurePoints[i].y() << endl;;
			}
			file.close();
			break;
		}
	}
	else if ( action->text() == cmStrReload )
		_canvas.reload();
	else if ( action->text() == cmStrCities )
		_canvas.setDrawCities(action->isChecked());
	else if ( action->text() == cmStrGrid )
		_canvas.setDrawGrid(action->isChecked());
	else if ( action->text() == cmStrLayers )
		_canvas.setDrawLayers(action->isChecked());
	else if ( _contextLayerMenu && isChildOf(action, _contextLayerMenu) ) {
		if ( action->text() == cmStrHideAll )
			_canvas.setDrawLayers(false);
		else if ( action->text() == cmStrLayersGeofeatures )
			_canvas.setDrawLayers(action->isChecked());
		else {
			// Find the LayerProperties which belongs to the action
			std::vector<Map::LayerProperties*>::const_iterator it =
				_canvas.layerProperties().begin();
			const Map::LayerProperties *root = *it++;
			Map::LayerProperties *prop = NULL;
			for ( ; it != _canvas.layerProperties().end(); ++it ) {
				if ( (*it)->parent != root || action->data().value<void*>() != *it ) continue;
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
	_isMeasureDragging = false;

	if ( event->button() == Qt::LeftButton ) {
		_lastDraggingPosition = event->pos();
		_firstDrag = true;

		if ( event->modifiers() == Qt::ControlModifier ) {
			QPointF p;
			_canvas.projection()->unproject(p, _lastDraggingPosition);
			if ( !_isMeasuring ) {
				_isMeasuring = true;
				_measurePoints.push_back(p);
			}
			_measurePoints.push_back(p);
			update();
			return;
		}

		if ( !_isMeasuring ) {
			if ( _canvas.filterMousePressEvent(event) ) return;
		}

		if ( event->modifiers() == Qt::NoModifier ) {
			_isDragging = true;
			_isMeasuring = false;
			_isMeasureDragging = false;
			_measurePoints.clear();
			_measureText.clear();
		}
	}
	else if ( _canvas.filterMousePressEvent(event) )
		event->ignore();
}


void MapWidget::mouseReleaseEvent(QMouseEvent* event) {
	_isMeasureDragging = false;

	if ( _isDragging && (event->button() == Qt::LeftButton) ) {
		_isDragging = false;
		update();
	}

	_canvas.filterMouseReleaseEvent(event);
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
		_isMeasureDragging = true;
		_canvas.projection()->unproject(_measurePoints.last(), event->pos());
		update();
	}
	else if ( !_canvas.filterMouseMoveEvent(event) )
		_zoomControls->setVisible(_zoomControls->geometry().contains(event->pos()));
}


void MapWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	if ( event->button() == Qt::LeftButton &&
	     !_canvas.filterMouseDoubleClickEvent(event) ) {
		_canvas.centerMap(event->pos());
		update();
	}
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
			e->ignore();
			emit keyPressed(e);
			break;
	};
}


void MapWidget::keyReleaseEvent(QKeyEvent *e) {}


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
