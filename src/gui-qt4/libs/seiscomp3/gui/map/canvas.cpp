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

#define SEISCOMP_COMPONENT Gui::Canvas

#include <seiscomp3/gui/map/canvas.h>

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/geo/featureset.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/gui/map/layer.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/gui/map/texturecache.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/geo.h>

#include <QMenu>
#include <QMouseEvent>
#include <QMutex>

#include <cmath>

#ifdef WIN32
#undef min
#undef max
#endif


#define CFG_LAYER_PREFIX "map.layers"
#define CFG_LAYER_INTERFACES_PREFIX "map.customLayers"


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


union RGBA {
	struct {
		unsigned char red;
		unsigned char green;
		unsigned char blue;
		unsigned char alpha;
	} components;
	unsigned int value;
};


QPoint alignmentToPos(Qt::Alignment area, int w, int h,
                      const QRect& rect, int margin) {
	QPoint pos = rect.topLeft();
	int tmpMargin = 2 * margin,
	    height = rect.height() - tmpMargin,
	    width = rect.width() - tmpMargin;

	if ( area & Qt::AlignHCenter ) {
		int x = std::max(width / 2 - w / 2, 0);
		pos += QPoint(x, 0);
	}
	else if ( area & Qt::AlignRight ) {
		int x = std::max(width - w, 0);
		pos += QPoint(x, 0);
	}

	if ( area & Qt::AlignVCenter ) {
		int y = std::max(height / 2 - h / 2, 0);
		pos += QPoint(0, y);
	} else if ( area & Qt::AlignBottom ) {
		int y = std::max(height - h, 0);
		pos += QPoint(0, y);
	}

	pos += QPoint(margin, margin);
	return pos;
}


QImage getDecorationSymbol(const QSize& size) {
	QImage image(size, QImage::Format_ARGB32);
	image.fill(Qt::transparent);

	QRect rect;
	rect.setSize(size);
	rect.adjust(8, 6, -8, -4);

	QPoint center = rect.center() + QPoint(1, 0);
	int left = rect.left(),
	    right= rect.right(),
	    top = rect.top(),
	    y = top;

	QPolygon polygon;
	polygon.append(QPoint(left, center.y()));
	polygon.append(QPoint(center.x(), y));

	y += rect.height() / 4;
	polygon.append(QPoint(center.x(), y));
	polygon.append(QPoint(right, y));

	y += 4;
	polygon.append(QPoint(right, y));
	polygon.append(QPoint(center.x(), y));
	polygon.append(QPoint(center.x(), y + 3));

	QPainter painter(&image);
	painter.setPen(Qt::white);
	painter.setBrush(Qt::white);
	painter.drawPolygon(polygon);

	return image;
}


} // ns anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define MAX_ZOOM (1 << 24)

bool Canvas::LegendArea::mousePressEvent(QMouseEvent *e) {
	if ( e->button() != Qt::LeftButton ) return false;
	return header.contains(e->pos());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::LegendArea::mouseReleaseEvent(QMouseEvent *e) {
	if ( e->button() != Qt::LeftButton ) return false;

	QPoint pos = e->pos();
	if ( header.contains(pos) ) {
		if ( currentIndex == -1 ) return true;

		int newIndex = currentIndex;

		if ( decorationRects[0].contains(pos) )
			newIndex = findNext(false);
		else if ( decorationRects[1].contains(pos) )
			newIndex = findNext(true);

		if ( newIndex != currentIndex ) {
			at(currentIndex).legend->setVisible(false);
			currentIndex = newIndex;
			if ( currentIndex != -1 )
				at(currentIndex).legend->setVisible(true);
		}

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Canvas::LegendArea::findNext(bool forward) const {
	int numberOfLegends = count(),
	    index = currentIndex,
	    tmp = forward ? 1 : -1;

	if ( currentIndex >= 0 ) {
		index = currentIndex;

		for ( int i = 0; i < numberOfLegends-1; ++i ) {
			index += tmp;
			if ( index < 0 || index >= numberOfLegends ) {
				if ( forward )
					index = 0;
				else
					index = numberOfLegends-1;
			}

			Legend *legend = at(index).legend;
			if ( legend->isEnabled() &&
				(legend->layer() == NULL || legend->layer()->isVisible()) )
				return index;
		}
	}
	else {
		for ( int i = 0; i < numberOfLegends; ++i ) {
			Legend *legend = at(i).legend;
			if ( legend->isEnabled() &&
				(legend->layer() == NULL || legend->layer()->isVisible()) )
				return i;
		}
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Canvas::Canvas(const MapsDesc &meta)
: _backgroundColor(Qt::lightGray)
, _dirtyRasterLayer(true)
, _dirtyVectorLayers(true)
, _hoverLayer(NULL)
, _margin(10)
, _isDrawLegendsEnabled(true) {
	_maptree = new ImageTree(meta);
	if ( !_maptree->valid() )
		_maptree = NULL;

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Canvas::Canvas(ImageTree *mapTree)
: _backgroundColor(Qt::lightGray)
, _dirtyRasterLayer(true)
, _dirtyVectorLayers(true)
, _hoverLayer(NULL)
, _margin(10)
, _isDrawLegendsEnabled(true) {
	_maptree = mapTree;

	if ( _maptree && !_maptree->valid() )
		_maptree = NULL;

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Canvas::~Canvas() {
	delete _projection;

	for ( CustomLayers::const_iterator it = _customLayers.begin();
	      it != _customLayers.end(); ++it ) {
		Layer *layer = it->get();
		for ( int i = 0; i < layer->legendCount(); ++i ) {
			layer->legend(i)->disconnect();
		}
	}

	// Remove this from Layers parent
	for ( Layers::const_iterator it = _layers.begin(); it != _layers.end(); ++it )
		(*it)->_canvas = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setBackgroundColor(QColor c) {
	_backgroundColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::setProjectionByName(const char *name) {
	Projection *newProjection = Map::ProjectionFactory::Create(name);
	if ( newProjection == NULL )
		return false;

	if ( _projection ) { delete _projection; _projection = NULL; }
	_projectionName = name;
	_projection = newProjection;
	_projection->setView(_center, _zoomLevel);

	projectionChanged(_projection);

	// Reset buffer
	setSize(width(), height());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setPreviewMode(bool previewMode) {
	if ( _previewMode == previewMode ) return;
	_previewMode = previewMode;
	updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setBilinearFilter(bool filter) {
	if ( _filterMap == filter ) return;
	_filterMap = filter;
	updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setFont(QFont f) {
	_font = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setSize(int w, int h) {
	_buffer = QImage(w, h, (_projection && !_projection->isRectangular())?QImage::Format_ARGB32:QImage::Format_RGB32);

	updateBuffer();

	LegendAreas::iterator it;
	for ( it = _legendAreas.begin(); it != _legendAreas.end(); ++it ) {
		LegendArea &area = it.value();
		int count = area.count();
		for ( int i = 0; i < count; ++i )
			area[i].dirty = true;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setLegendMargin(int margin) {
	_margin = margin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::init() {
	_font = SCScheme.fonts.base;
	_projection = NULL;

	_polygonRoughness = SCScheme.map.polygonRoughness;
	_filterMap = SCScheme.map.bilinearFilter;
	_previewMode = false;

	// If a default projection was found in the config search it among the
	// available projections
	const char *projection = "Rectangular";
	if ( !SCScheme.map.projection.empty() ) {
		Map::ProjectionFactory::ServiceNames* services = Map::ProjectionFactory::Services();
		if ( services && std::find(services->begin(), services->end(),
		                           SCScheme.map.projection) != services->end() )
			projection = SCScheme.map.projection.c_str();
		else
			SEISCOMP_WARNING("Projection %s not available, defaulting to %s",
			                 SCScheme.map.projection.c_str(), projection);
	}

	setProjectionByName(projection);

	if ( _maptree ) {
		connect(_maptree.get(), SIGNAL(tilesUpdated()), this, SLOT(updatedTiles()));
	}

	setupLayer(&_citiesLayer);
	_citiesLayer.setVisible(SCScheme.map.showCities);

	setupLayer(&_gridLayer);
	_gridLayer.setGridDistance(QPointF(15.0, 15.0));
	_gridLayer.setVisible(SCScheme.map.showGrid);

	setupLayer(&_geoFeatureLayer);
	_geoFeatureLayer.setVisible(SCScheme.map.showLayers);

	setupLayer(&_symbolLayer);
	_symbolLayer.setVisible(true);

	_layers.clear();
	_layers.append(&_gridLayer);
	_layers.append(&_citiesLayer);
	_layers.append(&_geoFeatureLayer);
	_layers.append(&_symbolLayer);

	_center = QPointF(0.0, 0.0);
	_zoomLevel = 1;

	_grayScale = false;
	_stackLegends = true;

	_projection->setView(_center, _zoomLevel);

	/*
	if ( _maptree )
		_maxZoom = 1 << (_maptree->depth()*2);
	else*/
		_maxZoom = MAX_ZOOM;

	setDrawLayers(SCScheme.map.showLayers);
	setDrawLegends(SCScheme.map.showLegends);

	// Read custom layers
	try {
		std::vector<std::string> customLayerInterfaces = SCApp->configGetStrings(CFG_LAYER_INTERFACES_PREFIX);
		for ( size_t i = 0; i < customLayerInterfaces.size(); ++i ) {
			LayerPtr customLayer = LayerFactory::Create(customLayerInterfaces[i].c_str());
			if ( !customLayer ) {
				SEISCOMP_WARNING("Could not create custom layer '%s'", customLayerInterfaces[i].c_str());
				continue;
			}

			customLayer->setName(customLayerInterfaces[i].c_str());
			_customLayers.append(customLayer);
			prependLayer(customLayer.get());
		}
	}
	catch ( ... ) {}

	// Read layer order
	try {
		std::vector<std::string> layerOrder;

		layerOrder = SCApp->configGetStrings(CFG_LAYER_PREFIX);
		if ( !layerOrder.empty() ) {
			QMap<std::string, Layer*> layerNameMap;

			// Create layer lookup
			foreach ( Layer *layer, _layers )
				layerNameMap[layer->name().toStdString()] = layer;

			Layers orderedLayers;
			for ( size_t i = 0; i < layerOrder.size(); ++i ) {
				Layer *layer = layerNameMap.value(layerOrder[i]);
				if ( layer == NULL )
					SEISCOMP_WARNING("Layer '%s' in layer list not found", layerOrder[i].c_str());
				else
					orderedLayers.append(layer);
			}

			// Append layers that are not already in ordered list
			foreach ( Layer *layer, _layers )
				if ( !orderedLayers.contains(layer) )
					orderedLayers.append(layer);

			// Finally copy the ordered layer list to the current layer list
			_layers = orderedLayers;
		}
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setGrayScale(bool e) {
	if ( _grayScale != e ) _dirtyRasterLayer = true;
	_grayScale = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::isGrayScale() const {
	return _grayScale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setDrawGrid(bool e) {
	_gridLayer.setVisible(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::isDrawGridEnabled() const {
	return _gridLayer.isVisible();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setDrawLayers(bool e) {
	_geoFeatureLayer.setVisible(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::isDrawLayersEnabled() const {
	return _geoFeatureLayer.isVisible();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setDrawCities(bool e) {
	_citiesLayer.setVisible(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::isDrawCitiesEnabled() const {
	return _citiesLayer.isVisible();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setDrawLegends(bool e) {
	if ( _isDrawLegendsEnabled == e ) return;

	_isDrawLegendsEnabled = e;

	foreach ( const LegendArea& area, _legendAreas ) {
		if ( !e ) {
			foreach (const LegendItem &item, area) {
				item.legend->setVisible(false);
			}
		}
		else {
			if ( area.currentIndex != -1 )
				area[area.currentIndex].legend->setVisible(true);
		}
	}

	emit legendVisibilityChanged(_isDrawLegendsEnabled);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::isDrawLegendsEnabled() const {
	return _isDrawLegendsEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::showLegends() {
	setDrawLegends(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::hideLegends() {
	setDrawLegends(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setLegendStacking(bool enable) {
	_stackLegends = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::updateBuffer() {
	_dirtyRasterLayer = true;
	_dirtyVectorLayers = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setMapCenter(QPointF c) {
	if ( _center == c ) return;
	_center = c;
	_projection->centerOn(_center);
	updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF& Canvas::mapCenter() const {
	return _center;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::setZoomLevel(float l) {
	// Assure: 1 <= l <= maxZoom
	l = l < 1.0f ? 1.0f : l > _maxZoom ? _maxZoom : l;

	if ( l == _zoomLevel )
		return false;
	else
		_zoomLevel = l;

	_projection->setZoom(_zoomLevel);
	updateBuffer();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float Canvas::zoomLevel() const {
	return _zoomLevel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float Canvas::pixelPerDegree() const {
	return _projection->pixelPerDegree();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setView(QPoint c, float zoom) {
	QPointF fc;
	if ( !_projection->unproject(fc, c) )
		return;

	setView(fc, zoom);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setView(QPointF c, float zoom) {
	if ( _center == c && _zoomLevel == zoom ) return;

	_center = c;
	_zoomLevel = zoom;

	_projection->setView(_center, _zoomLevel);
	updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::displayRect(const QRectF& rect) {
	_projection->displayRect(rect);
	_center = _projection->center();
	_zoomLevel = _projection->zoom();
	updateBuffer();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Canvas::drawLine(QPainter& p, const QPointF& start, const QPointF& end) const {
	QPoint pp0, pp1;

	if ( !_projection->project(pp0, start) && !_projection->project(pp1, end) )
		return -1.0;

	int steps = _projection->lineSteps(start, end);
	if ( steps <= 0 ) return -1.0;

	Math::Geo::PositionInterpolator ip(start.y(), start.x(),
	                                   end.y(), end.x(), steps);

	double dist = ip.overallDistance();
	if ( dist > 0.0 ) {
		++ip;

		_projection->moveTo(start);

		while ( !ip.end() ) {
			_projection->lineTo(p, QPointF(ip.longitude(),ip.latitude()));
			++ip;
		}
	}

	return dist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Canvas::drawPolygon(QPainter &painter, size_t n, const Geo::GeoCoordinate *poly,
                           bool isClosedPolygon, int roughness, ClipHint clipHint) const {
	QPainterPath pp;

	if ( !_projection->project(pp, n, poly, isClosedPolygon,
	                           roughness < 0 ? _polygonRoughness : roughness,
	                           clipHint) )
		return 0;

	if ( !isClosedPolygon ) {
		QBrush backup = painter.brush();
		painter.setBrush(Qt::NoBrush);
		painter.drawPath(pp);
		painter.setBrush(backup);
	}
	else
		painter.drawPath(pp);

	return pp.elementCount();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Canvas::drawPolyline(QPainter &painter, size_t n, const Geo::GeoCoordinate *line,
                            bool isClosedPolygon, bool interpolate,
                            int roughness) const {
	if ( n == 0 || !line ) return 0;

	uint linesPlotted = 0;
	float minDist = ((float)(roughness < 0 ? _polygonRoughness : roughness))/_projection->pixelPerDegree();

	if ( interpolate ) {
		QPointF p1(line[0].lon, line[0].lat);
		QPointF p2;
		if ( minDist == 0 ) {
			for ( size_t i = 1; i < n; ++i ) {
				p2.setX(line[i].lon); p2.setY(line[i].lat);
				drawLine(painter, p1, p2);
				p1 = p2;
			}
			linesPlotted =  n-1;
		}
		else {
			for ( size_t i = 1; i < n; ++i ) {
				p2.setX(line[i].lon); p2.setY(line[i].lat);
				if ( std::abs(p2.x() - p1.x()) > minDist ||
				     std::abs(p2.y() - p1.y()) > minDist ||
				     (!isClosedPolygon && i == n-1) ) {
					drawLine(painter, p1, p2);
					++linesPlotted;
					p1 = p2;
				}
			}
		}
		if ( isClosedPolygon ) {
			p2.setX(line[0].lon); p2.setY(line[0].lat);
			if ( p1 != p2 ) {
				drawLine(painter, p1, p2);
				++linesPlotted;
			}
		}
	}
	else {
		QPointF p(line[0].lon, line[0].lat);
		_projection->moveTo(p);
		if ( minDist == 0 ) {
			for ( size_t i = 1; i < n; ++i ) {
				p.setX(line[i].lon); p.setY(line[i].lat);
				_projection->lineTo(painter, p);
			}
			linesPlotted = n-1;
		}
		else {
			for ( size_t i = 1; i < n; ++i ) {
				if ( std::abs(line[i].lon - p.x()) > minDist ||
				     std::abs(line[i].lat - p.y()) > minDist ||
				     (!isClosedPolygon && i == n-1) ) {
					p.setX(line[i].lon); p.setY(line[i].lat);
					_projection->lineTo(painter, p);
					++linesPlotted;
				}
			}
		}
		if ( isClosedPolygon &&
		     ( p.x() != line[0].lon || p.y() != line[0].lat ) ) {
			p.setX(line[0].lon); p.setY(line[0].lat);
			_projection->lineTo(painter, p);
			++linesPlotted;
		}
	}

	return linesPlotted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Canvas::drawFeature(QPainter &painter, const Geo::GeoFeature *f,
                           bool filled, int roughness, ClipHint clipHint) const {
	if ( !f ) return 0;

	// Check whether the feature is visible
	const Geo::GeoBoundingBox &bbox = f->bbox();
	if ( _projection->isClipped(QPointF(bbox.east, bbox.south),
	                            QPointF(bbox.west, bbox.north)) )
		return 0;

	Geo::GeoBoundingBox::Relation relBB = _projection->boundingBox().relation(bbox);

	if ( relBB == Geo::GeoBoundingBox::Contains )
		clipHint = NoClip;

	int effectiveRoughness = roughness < 0 ? _polygonRoughness : roughness;
	size_t lines = 0, startIdx = 0, endIdx = 0;
	size_t nSubFeat = f->subFeatures().size();
	bool gotFirstPath = false;

	if ( f->closedPolygon() && filled ) {
		if ( nSubFeat > 0 ) {
			QPainterPath path;
			bool firstForward;

			for ( size_t i = 0; i <= nSubFeat; ++i, startIdx = endIdx ) {
				endIdx = (i == nSubFeat ? f->vertices().size() : f->subFeatures()[i]);

				if ( !gotFirstPath ) {
					if ( !_projection->project(path, endIdx - startIdx, &f->vertices()[startIdx], true, effectiveRoughness, clipHint) )
						continue;
					gotFirstPath = true;
					firstForward = Geo::GeoFeature::area(&f->vertices()[startIdx], endIdx-startIdx) > 0;
				}
				else {
					QPainterPath subPath;
					if ( !_projection->project(subPath, endIdx - startIdx, &f->vertices()[startIdx], true, effectiveRoughness, clipHint) )
						continue;

					bool forward = Geo::GeoFeature::area(&f->vertices()[startIdx], endIdx-startIdx) > 0;
					forward = firstForward == forward;

					if ( forward )
						path.addPath(subPath);
					else {
#if QT_VERSION >= 0x040500
						path -= subPath;
#elif QT_VERSION >= 0x040300
						path = path.subtracted(subPath);
#else
						path.addPath(subPath.toReversed());
#endif
					}

					lines += subPath.elementCount();
				}
			}

			painter.drawPath(path);
		}
		else
			lines += drawPolygon(painter, f->vertices().size(),
			                     &f->vertices()[0], f->closedPolygon(),
			                     roughness, clipHint);
	}
	else {
		for ( size_t i = 0; i <= nSubFeat; ++i) {
			endIdx = (i == nSubFeat ? f->vertices().size() : f->subFeatures()[i]);

			lines += drawPolygon(painter, endIdx - startIdx,
			                     &(f->vertices()[startIdx]),
			                     f->closedPolygon(), roughness, clipHint);
			startIdx = endIdx;
		}
	}

	return lines;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::isVisible(double lon, double lat) const {
	QPoint p;
	if ( !_projection->project(p, QPointF(lon, lat)) ) return false;
	return (p.x() >= 0) && (p.x() < _buffer.width()) &&
	       (p.y() >= 0) && (p.y() < _buffer.height());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setSelectedCity(const Math::Geo::CityD *c) {
	_citiesLayer.setSelectedCity(c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::drawLayers(QPainter &painter) {
	foreach ( Layer *layer, _layers ) {
		if ( _dirtyVectorLayers ) layer->setDirty();
		if ( !layer->isVisible() ) continue;

		if ( layer->isDirty() ) {
			layer->calculateMapPosition(this);
			layer->_dirty = false;
		}

		layer->draw(this, painter);
	}

	_dirtyVectorLayers = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::drawImage(const QRectF &geoReference, const QImage &image,
                       CompositionMode compositionMode, FilterMode filterMode) {
	bool smoothFilter = false;
	if ( filterMode == FilterMode_Bilinear )
		smoothFilter = true;
	else if ( filterMode == FilterMode_Auto )
		smoothFilter = _filterMap && !_previewMode;
	_projection->drawImage(_buffer, geoReference, image, smoothFilter, compositionMode);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::drawImageLayer(QPainter &painter) {
	static QMutex mapRenderMutex;

	if ( _dirtyRasterLayer ) {
		_projection->setBackgroundColor(_backgroundColor);
		mapRenderMutex.lock();
		_projection->draw(_buffer, _filterMap && !_previewMode,
		                  _maptree?_maptree->getCache():NULL);
		mapRenderMutex.unlock();
		_gridLayer.setGridDistance(_projection->gridDistance());

		if ( _maptree ) {
			if ( SCScheme.map.toBGR ) {
				int w = _buffer.width();
				int h = _buffer.height();

				RGBA *data = (RGBA*)_buffer.bits();
				for ( int i = 0; i < h; ++i ) {
					for ( int j = 0; j < w; ++j ) {
						std::swap(data->components.red, data->components.blue);
						++data;
					}
				}
			}

			if ( _grayScale ) {
				int w = _buffer.width();
				int h = _buffer.height();

				RGBA *data = (RGBA*)_buffer.bits();
				for ( int i = 0; i < h; ++i ) {
					for ( int j = 0; j < w; ++j ) {
						char gray = qGray(data->components.red, data->components.green, data->components.blue);
						data->components.red = data->components.green = data->components.blue = gray;
						++data;
					}
				}
			}
		}
		else
			_buffer.fill(_backgroundColor.rgba());

		if ( painter.device() == &_buffer ) {
			for ( Layers::const_iterator it = _layers.begin(); it != _layers.end(); ++it ) {
				if ( (*it)->isVisible() )
					(*it)->baseBufferUpdated(this, painter);
			}

			for ( Layers::const_iterator it = _layers.begin(); it != _layers.end(); ++it ) {
				if ( (*it)->isVisible() )
					(*it)->bufferUpdated(this, painter);
			}
		}
		else {
			QPainter p(&_buffer);

			for ( Layers::const_iterator it = _layers.begin(); it != _layers.end(); ++it ) {
				if ( (*it)->isVisible() )
					(*it)->baseBufferUpdated(this, p);
			}

			for ( Layers::const_iterator it = _layers.begin(); it != _layers.end(); ++it ) {
				if ( (*it)->isVisible() )
					(*it)->bufferUpdated(this, p);
			}
		}

		bufferUpdated();
		_dirtyRasterLayer = false;
	}

	if ( painter.device() != &_buffer )
		painter.drawImage(0,0,_buffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::drawVectorLayer(QPainter &painter) {
	if ( isDrawGridEnabled() || !_maptree ) {
		_gridLayer.setAntiAliasingEnabled(_projection->wantsGridAntialiasing() &&
		                                  _filterMap && !_previewMode);
	}

	painter.setRenderHint(QPainter::Antialiasing,
	                      !_previewMode && SCScheme.map.vectorLayerAntiAlias);
	drawLayers(painter);

	painter.setRenderHint(QPainter::Antialiasing,
	                      !_previewMode && SCScheme.map.vectorLayerAntiAlias);
	customLayer(&painter);

	if ( _isDrawLegendsEnabled ) drawLegends(painter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::drawLegends(QPainter& painter) {
	QFontMetrics fm(painter.font());

	QPainter::RenderHints hints = painter.renderHints();
	painter.setRenderHint(QPainter::Antialiasing, false);

	int innerMargin = 9;

	for ( LegendAreas::iterator it = _legendAreas.begin();
	      it != _legendAreas.end(); ++it ) {
		LegendArea &area = it.value();
		Legend *legend;

		if ( _stackLegends ) {
			if ( area.currentIndex == -1 ) {
				area.currentIndex = area.findNext();
				if ( area.currentIndex == -1 ) continue;
			}
			else {
				legend = area[area.currentIndex].legend;
				if ( !legend->isEnabled() || ((legend->layer() != NULL) && !legend->layer()->isVisible()) ) {
					if ( legend->isVisible() ) legend->setVisible(false);
					area.currentIndex = area.findNext();
					if ( area.currentIndex == -1 ) continue;
				}
			}

			legend = area[area.currentIndex].legend;

			if ( !legend->isVisible() ) legend->setVisible(true);

			if ( area[area.currentIndex].dirty ) {
				// TODO: Inject painter into contextResizeEvent
				legend->contextResizeEvent(size());
				area[area.currentIndex].dirty = false;
			}

			QRect decorationRect(0, 0, 52, 22);
			const QString &title = legend->title();
			QRect textRect(0, 0, fm.width(title) + 2 * innerMargin, fm.height());
			QSize contentSize = legend->size();
			int contentHeight = contentSize.height(),
			    contentWidth = contentSize.width(),
			    headerHeight = std::max(textRect.height(), decorationRect.height());

			if ( area.findNext() == -1 ) {
				decorationRect.setSize(QSize(0, 0));

				// No title and just one legend -> no header
				if ( title.isEmpty() ) headerHeight = 0;
			}

			int height = headerHeight + contentHeight,
			    width = textRect.width() + decorationRect.width();

			if ( contentWidth > width) {
				textRect.setWidth(textRect.width() + contentWidth - width);
				width = contentWidth;
			}
			else {
				contentWidth = width;
				contentSize.setWidth(contentWidth);
			}

			if ( headerHeight > textRect.height() )
				textRect.setHeight(headerHeight);

			QPoint pos = alignmentToPos(legend->alignment(), width, height,
			                            painter.viewport(), _margin);

			int x = pos.x(), y = pos.y();

			QRect headerRect(x, y, width, headerHeight);
			if ( legend->alignment() & Qt::AlignBottom )
				headerRect.translate(0, contentHeight);

			QLinearGradient gradient(headerRect.topLeft(), headerRect.bottomLeft());
			gradient.setColorAt(0, QColor(125, 125, 125 , 192));
			gradient.setColorAt(1, QColor(76, 76, 76, 192));

			QPen pen;
			pen.setBrush(gradient);

			painter.setPen(pen);
			painter.setBrush(gradient);
			painter.drawRect(headerRect);

			if ( legend->alignment() & Qt::AlignRight ) {
				textRect.moveTopLeft(headerRect.topLeft());
				decorationRect.moveTopLeft(textRect.topRight());
			}
			else {
				decorationRect.moveTopLeft(headerRect.topLeft());
				textRect.moveTopLeft(decorationRect.topRight());
			}

			QFont font = painter.font();

			painter.setFont(legend->titleFont());
			painter.setPen(Qt::white);
			painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, title);

			painter.setFont(font);

			if ( !decorationRect.isNull() ) {
				QSize size(26, 22);
				QImage image = getDecorationSymbol(size);
				painter.drawImage(decorationRect.topLeft(), image);

				QRect rect(decorationRect.topLeft(), size);
				it->decorationRects[0] = rect;
				rect.translate(26, 0);

				image = image.mirrored(true, false);
				painter.drawImage(rect.topLeft(), image);

				it->decorationRects[1] = rect;
			}
			else {
				it->decorationRects[0] = QRect();
				it->decorationRects[1] = QRect();
			}

			it->header = headerRect;

			QRect contentRect (headerRect.bottomLeft(), contentSize);
			if ( legend->alignment() & Qt::AlignBottom )
				contentRect.moveTopLeft(QPoint(x,y));

			painter.setPen(SCScheme.colors.legend.border);
			painter.setBrush(SCScheme.colors.legend.background);
			painter.drawRect(contentRect);

			painter.setPen(SCScheme.colors.legend.text);
			legend->draw(contentRect, painter);
		}
		else {
			Qt::Alignment align = it.key();
			int tx = 0, ty = 0;

			if ( align & Qt::AlignHCenter ) {
				if ( align & Qt::AlignTop )
					ty = 1;
				else if ( align & Qt::AlignBottom )
					ty = -1;
			}
			else if ( align & Qt::AlignRight )
				tx = -1;
			else
				tx = 1;

			int cx = 0, cy = 0;

			it->decorationRects[0] = QRect();
			it->decorationRects[1] = QRect();

			for ( LegendArea::iterator lit = area.begin(); lit != area.end(); ++lit ) {
				legend = lit->legend;

				if ( !legend->isEnabled() ) continue;
				if ( !legend->isVisible() ) legend->setVisible(true);

				if ( area[area.currentIndex].dirty ) {
					// TODO: Inject painter into contextResizeEvent
					legend->contextResizeEvent(size());
					area[area.currentIndex].dirty = false;
				}

				QRect decorationRect(0, 0, 52, 22);
				const QString &title = legend->title();
				QRect textRect(0, 0, fm.width(title) + 2 * innerMargin, fm.height());
				QSize contentSize = legend->size();
				int contentHeight = contentSize.height(),
				    contentWidth = contentSize.width(),
				    headerHeight = std::max(textRect.height(), decorationRect.height());

				// No title and just one legend -> no header
				if ( title.isEmpty() ) headerHeight = 0;

				int height = headerHeight + contentHeight,
				    width = textRect.width() + decorationRect.width();

				if ( contentWidth > width) {
					textRect.setWidth(textRect.width() + contentWidth - width);
					width = contentWidth;
				}
				else {
					contentWidth = width;
					contentSize.setWidth(contentWidth);
				}

				if ( headerHeight > textRect.height() )
					textRect.setHeight(headerHeight);

				QPoint pos = alignmentToPos(legend->alignment(), width, height,
				                            painter.viewport(), _margin);

				int x = cx + pos.x(), y = cy + pos.y();
				QRect headerRect(x, y, width, headerHeight);

				if ( headerHeight > 0 ) {
					if ( legend->alignment() & Qt::AlignBottom )
						headerRect.translate(0, contentHeight);

					QLinearGradient gradient(headerRect.topLeft(), headerRect.bottomLeft());
					gradient.setColorAt(0, QColor(125, 125, 125 , 192));
					gradient.setColorAt(1, QColor(76, 76, 76, 192));

					QPen pen;
					pen.setBrush(gradient);

					painter.setPen(pen);
					painter.setBrush(gradient);
					painter.drawRect(headerRect);

					QFont font = painter.font();

					painter.setFont(legend->titleFont());
					painter.setPen(Qt::white);
					painter.drawText(headerRect, Qt::AlignHCenter | Qt::AlignVCenter, title);

					painter.setFont(font);
				}

				it->header = headerRect;

				QRect contentRect(headerRect.bottomLeft(), contentSize);
				if ( legend->alignment() & Qt::AlignBottom )
					contentRect.moveTopLeft(QPoint(x,y));

				painter.setPen(SCScheme.colors.legend.border);
				painter.setBrush(SCScheme.colors.legend.background);
				painter.drawRect(contentRect);

				legend->draw(contentRect, painter);

				cx += tx * (contentRect.width() + _margin);
				cy += ty * (contentRect.height() + _margin);
			}
		}
	}

	painter.setRenderHints(hints);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::draw(QPainter &painter) {
	drawImageLayer(painter);
	drawVectorLayer(painter);

	if ( !_maptree || !_maptree->hasPendingRequests() )
		renderingCompleted();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::centerMap(const QPoint& centerPnt) {
	if ( !_projection->unproject(_center, centerPnt) ) return;
	_projection->centerOn(_center);
	updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::translate(const QPoint &delta) {
	if ( _center.y() > 90 || _center.y() < -90 )
		_center.setX(_center.x() + delta.x() / _projection->pixelPerDegree());
	else
		_center.setX(_center.x() - delta.x() / _projection->pixelPerDegree());
	_center.setY(_center.y() + delta.y() / _projection->pixelPerDegree());

	_projection->centerOn(_center);
	_center = _projection->center();
	updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::translate(const QPointF &delta) {
	_projection->centerOn(_center + delta);
	_center = _projection->center();
	updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::onLegendAdded(Legend *legend) {
	LegendAreas::iterator it = _legendAreas.find(legend->alignment());
	if ( it == _legendAreas.end() )
		it = _legendAreas.insert(legend->alignment(), LegendArea());

	LegendArea &area = *it;
	area.append(LegendItem(legend));
	if ( legend->layer() && legend->layer()->isVisible() &&
	     legend->isEnabled() && area.currentIndex == -1 )
		area.currentIndex = area.findNext();

	connect(legend, SIGNAL(enabled(Seiscomp::Gui::Map::Legend*, bool)),
	        this, SLOT(setLegendEnabled(Seiscomp::Gui::Map::Legend*, bool)));
	connect(legend, SIGNAL(bringToFrontRequested(Seiscomp::Gui::Map::Legend*)),
	        this, SLOT(bringToFront(Seiscomp::Gui::Map::Legend*)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::onLegendRemoved(Legend *legend) {
	LegendAreas::iterator it = _legendAreas.find(legend->alignment());
	if ( it != _legendAreas.end() ) {
		int index = it->find(legend);
		if ( index != -1 )
			it->remove(index);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setupLayer(Layer *layer) {
	// Take ownership of the layer if it does not have a parent yet
	if ( layer->parent() == NULL )
		layer->setParent(this);

	layer->_canvas = this;
	layer->setDirty();

	if ( SCApp ) {
		if ( !layer->name().isEmpty() ) {
			std::string cfgVisible = CFG_LAYER_PREFIX ".";
			cfgVisible += layer->name().toStdString();
			cfgVisible += ".visible";
			try {
				layer->setVisible(SCApp->configGetBool(cfgVisible));
			}
			catch ( ... ) {}
		}

		layer->init(SCApp->configuration());
	}

	connect(layer, SIGNAL(legendAdded(Legend*)), this, SLOT(onLegendAdded(Legend*)));
	connect(layer, SIGNAL(legendRemoved(Legend*)), this, SLOT(onLegendRemoved(Legend*)));
	connect(layer, SIGNAL(updateRequested(const Layer::UpdateHints&)),
	        this, SLOT(updateLayer(const Layer::UpdateHints&)));

	foreach ( Legend *legend, layer->legends() ) {
		if ( legend != NULL ) {
			onLegendAdded(legend);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::prependLayer(Layer* layer) {
	if ( layer->canvas() != NULL ) {
		qWarning("Layer is already part of another canvas");
		return false;
	}

	_layers.prepend(layer);
	setupLayer(layer);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::addLayer(Layer* layer) {
	if ( layer->canvas() != NULL ) {
		qWarning("Layer is already part of another canvas");
		return false;
	}

	_layers.append(layer);
	setupLayer(layer);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::insertLayerBefore(const Layer *referenceLayer, Layer *layer) {
	if ( layer->canvas() != NULL ) {
		qWarning("Layer is already part of another canvas");
		return false;
	}

	int index = _layers.indexOf(const_cast<Layer*>(referenceLayer));
	if ( index >= 0 )
		_layers.insert(index, layer);
	else
		_layers.append(layer);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::removeLayer(Layer* layer) {
	_layers.removeAll(layer);
	disconnect(layer, SIGNAL(updateRequested()));

	if ( layer == _hoverLayer )
		_hoverLayer = NULL;

	LegendAreas::iterator it = _legendAreas.begin();
	while ( it != _legendAreas.end() ) {
		Legends &legends = it.value();
		bool changed = false;
		Legends::iterator tmpIt = legends.begin();
		while ( tmpIt != legends.end() ) {
			if ( layer == (*tmpIt).legend->layer() ) {
				tmpIt = legends.erase(tmpIt);
				changed = true;
			}
			else
				++tmpIt;
		}

		if ( legends.isEmpty() )
			it = _legendAreas.erase(it);
		else {
			if ( changed ) it->currentIndex = it->findNext();
			++it;
		}
	}

	layer->_canvas = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::raise(Layer* layer) {
	int index = _layers.indexOf(layer);
	if ( index == -1 ) return;

	int next = index = -1;
	if ( next >= _layers.count() ) return;

	_layers.move(index, next);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::lower(Layer* layer) {
	int index = _layers.indexOf(layer);
	if ( index == -1 ) return;

	int prev = index = -1;
	if ( prev < 0 ) return;

	_layers.move(index, prev);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::filterKeyPressEvent(QKeyEvent *event) {
	if ( _hoverLayer )
		return _hoverLayer->filterKeyPressEvent(event);
	else
		return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::filterKeyReleaseEvent(QKeyEvent *event) {
	if ( _hoverLayer )
		return _hoverLayer->filterKeyReleaseEvent(event);
	else
		return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::filterMouseMoveEvent(QMouseEvent* e) {
	// TODO: Check legend hit that will eat the event

	QPointF geoPos;
	if ( !_projection->unproject(geoPos, e->pos()) )
		return false;

	Layers::iterator it = _layers.end();
	Layer *hoverLayer = NULL;

	while ( it != _layers.begin() ) {
		--it;
		if ( (*it)->isVisible() && (*it)->isInside(e, geoPos) ) {
			hoverLayer = *it;
			break;
		}
	}

	if ( _hoverLayer != hoverLayer ) {
		if ( _hoverLayer )
			_hoverLayer->handleLeaveEvent();
		if ( hoverLayer )
			hoverLayer->handleEnterEvent();
		_hoverLayer = hoverLayer;
	}

	if ( _hoverLayer ) {
		if ( _hoverLayer->filterMouseMoveEvent(e, geoPos) )
			return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::filterMouseDoubleClickEvent(QMouseEvent *e) {
	if ( _isDrawLegendsEnabled ) {
		for ( LegendAreas::iterator it = _legendAreas.begin();
		      it != _legendAreas.end(); ++it ) {
			if ( it->mousePressEvent(e) ) return true;
		}
	}

	if ( _hoverLayer ) {
		QPointF geoPos;
		if ( _projection->unproject(geoPos, e->pos()) )
			return _hoverLayer->filterMouseDoubleClickEvent(e, geoPos);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::filterMousePressEvent(QMouseEvent *e) {
	if ( _isDrawLegendsEnabled ) {
		for ( LegendAreas::iterator it = _legendAreas.begin();
		      it != _legendAreas.end(); ++it ) {
			if ( it->mousePressEvent(e) ) {
				updateRequested();
				return true;
			}
		}
	}

	if ( _hoverLayer ) {
		QPointF geoPos;
		if ( _projection->unproject(geoPos, e->pos()) )
			return _hoverLayer->filterMousePressEvent(e, geoPos);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::filterMouseReleaseEvent(QMouseEvent *e) {
	if ( _isDrawLegendsEnabled ) {
		for ( LegendAreas::iterator it = _legendAreas.begin();
		      it != _legendAreas.end(); ++it ) {
			if ( it->mouseReleaseEvent(e) ) {
				updateRequested();
				return true;
			}
		}
	}

	if ( _hoverLayer ) {
		QPointF geoPos;
		if ( _projection->unproject(geoPos, e->pos()) )
			return _hoverLayer->filterMouseReleaseEvent(e, geoPos);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::filterContextMenuEvent(QContextMenuEvent* e, QWidget* parent) {
	foreach ( Layer *layer, _layers ) {
		if ( layer->filterContextMenuEvent(e, parent) )
			return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMenu* Canvas::menu(QMenu *parent) const {
	QAction *action;

	QMenu *menu = new QMenu(tr("Layers"), parent);
	foreach ( Layer *layer, _layers ) {
		if ( layer->name().isEmpty() ) continue;

		if ( !layer->isVisible() ) {
			action = menu->addAction(layer->name());
			action->setCheckable(true);
			action->setChecked(false);
			connect(action, SIGNAL(toggled(bool)), layer, SLOT(setVisible(bool)));
		}
		else {
			QMenu *subMenu = layer->menu(menu);

			if ( subMenu ) {
				if ( subMenu->isEmpty() ) {
					delete subMenu;
					subMenu = NULL;
				}
				else {
					// Add "Hide layer" option as first option
					QAction *firstAction = subMenu->actions().first();

					subMenu->setTitle(layer->name());
					QAction *separator = subMenu->insertSeparator(firstAction);
					QAction *toggleAction = new QAction(tr("Hide layer"), subMenu);
					connect(toggleAction, SIGNAL(triggered()), layer, SLOT(hide()));

					subMenu->insertAction(separator, toggleAction);
					menu->addMenu(subMenu);
				}
			}

			if ( !subMenu ) {
				QAction *action = menu->addAction(layer->name());
				action->setCheckable(true);
				action->setChecked(true);
				connect(action, SIGNAL(toggled(bool)), layer, SLOT(setVisible(bool)));
			}
		}
	}

	if ( menu->isEmpty() ) {
		delete menu;
		menu = NULL;
	}
	else
		parent->addMenu(menu);

	action = parent->addAction(tr("Reload"));
	connect(action, SIGNAL(triggered()), this, SLOT(reload()));

	if ( isDrawLegendsEnabled() ) {
		action = parent->addAction(tr("Hide legend(s)"));
		connect(action, SIGNAL(triggered()), this, SLOT(hideLegends()));
	}
	else {
		action = parent->addAction(tr("Show legend(s)"));
		connect(action, SIGNAL(triggered()), this, SLOT(showLegends()));
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::updateLayer(const Layer::UpdateHints &hints) {
	if ( hints.testFlag(Layer::Position) ) {
		Layer *layer = static_cast<Layer*>(sender());
		layer->calculateMapPosition(this);
	}

	if ( hints.testFlag(Layer::RasterLayer) )
		_dirtyRasterLayer = true;

	updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::updatedTiles() {
	updateBuffer();
	updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::reload() {
	if ( !_maptree ) return;
	_maptree->refresh();
	updateBuffer();
	updateRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::bringToFront(Seiscomp::Gui::Map::Legend *legend) {
	LegendAreas::iterator it = _legendAreas.find(legend->alignment());
	if ( it == _legendAreas.end() ) return;

	int index = it->find(legend);
	if ( index == -1 ) return;
	if ( index == it->currentIndex ) return;

	const Legends& legends = *it;
	if ( it->currentIndex > 0 && legends.count() > it->currentIndex )
		legends[it->currentIndex].legend->setVisible(false);

	it->currentIndex = index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Canvas::setLegendEnabled(Seiscomp::Gui::Map::Legend* legend, bool enabled) {
	LegendAreas::iterator it = _legendAreas.find(legend->alignment());
	if ( it == _legendAreas.end() ) return;

	int index = it->find(legend);
	if ( index == -1 ) return;

	it->currentIndex = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Canvas::renderingComplete() const {
	return !_maptree || !_maptree->hasPendingRequests();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
