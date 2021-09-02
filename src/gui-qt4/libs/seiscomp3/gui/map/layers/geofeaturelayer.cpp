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

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/utils.h>
#include <seiscomp3/gui/map/layers/geofeaturelayer.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/gui/map/standardlegend.h>
#include <seiscomp3/geo/featureset.h>
#include <seiscomp3/seismology/regions.h>

#include <QMenu>

#include <iostream>

using namespace std;

namespace Seiscomp {
namespace Gui {
namespace Map {

namespace {


#define CFG_LAYER_PREFIX "map.layers"

QPen readPen(const Config::Config &cfg, const string &query, const QPen &base) {
	QPen p(base);

	try {
		const string &q = query + ".color";
		p.setColor(readColor(q, cfg.getString(q), base.color()));
	}
	catch ( ... ) {}

	try {
		const string &q = query + ".style";
		p.setStyle(readPenStyle(q, cfg.getString(q), base.style()));
	}
	catch ( ... ) {}

	try {
		p.setWidth(cfg.getDouble(query + ".width"));
	}
	catch ( ... ) {}

	return p;
}

QBrush readBrush(const Config::Config &cfg, const string &query, const QBrush &base) {
	QBrush b(base);

	try {
		const string &q = query + ".color";
		b.setColor(readColor(q, cfg.getString(q), base.color()));
	}
	catch ( ... ) {}

	try {
		const string &q = query + ".style";
		b.setStyle(readBrushStyle(q, cfg.getString(q), base.style()));
	}
	catch ( ... ) {}

	return b;
}

QFont readFont(const Config::Config &cfg, const string& query, const QFont &base) {
	QFont f(base);

	try {
		f.setFamily(cfg.getString(query + ".family").c_str());
	}
	catch ( ... ) {}

	try {
		f.setPointSize(cfg.getInt(query + ".size"));
	}
	catch ( ... ) {}

	try {
		f.setBold(cfg.getBool(query + ".bold"));
	}
	catch ( ... ) {}

	try {
		f.setItalic(cfg.getBool(query + ".italic"));
	}
	catch ( ... ) {}

	try {
		f.setUnderline(cfg.getBool(query + ".underline"));
	}
	catch ( ... ) {}

	try {
		f.setOverline(cfg.getBool(query + ".overline"));
	}
	catch ( ... ) {}

	return f;
}

Qt::Orientation getOrientation(const std::string &name) {
	if ( name == "horizontal" )
		return Qt::Horizontal;
	else if ( name == "vertical" )
		return Qt::Vertical;
	else
		return Qt::Vertical;
}

Qt::Alignment getAlignment(const std::string &name) {
	if ( name == "topleft" )
		return Qt::Alignment(Qt::AlignTop | Qt::AlignLeft);
	else if ( name == "topright" )
		return Qt::Alignment(Qt::AlignTop | Qt::AlignRight);
	else if ( name == "bottomleft" )
		return Qt::Alignment(Qt::AlignBottom | Qt::AlignLeft);
	else if ( name == "bottomright" )
		return Qt::Alignment(Qt::AlignBottom | Qt::AlignRight);
	else
		return Qt::Alignment(Qt::AlignTop | Qt::AlignLeft);
}


QPainter::CompositionMode getCompositionMode(const std::string &name) {
	if ( name == "src-over" )
		return QPainter::CompositionMode_SourceOver;
	else if ( name == "dst-over" )
		return QPainter::CompositionMode_DestinationOver;
	else if ( name == "clear" )
		return QPainter::CompositionMode_Clear;
	else if ( name == "src" )
		return QPainter::CompositionMode_Source;
	else if ( name == "dst" )
		return QPainter::CompositionMode_Destination;
	else if ( name == "src-in" )
		return QPainter::CompositionMode_SourceIn;
	else if ( name == "dst-in" )
		return QPainter::CompositionMode_DestinationIn;
	else if ( name == "src-out" )
		return QPainter::CompositionMode_SourceOut;
	else if ( name == "dst-out" )
		return QPainter::CompositionMode_DestinationOut;
	else if ( name == "src-atop" )
		return QPainter::CompositionMode_SourceAtop;
	else if ( name == "dst-atop" )
		return QPainter::CompositionMode_DestinationAtop;
	else if ( name == "xor" )
		return QPainter::CompositionMode_Xor;
	else if ( name == "plus" )
		return QPainter::CompositionMode_Plus;
	else if ( name == "multiply" )
		return QPainter::CompositionMode_Multiply;
	else if ( name == "screen" )
		return QPainter::CompositionMode_Screen;
	else if ( name == "overlay" )
		return QPainter::CompositionMode_Overlay;
	else if ( name == "darken" )
		return QPainter::CompositionMode_Darken;
	else if ( name == "lighten" )
		return QPainter::CompositionMode_Lighten;
	else if ( name == "color-dodge" )
		return QPainter::CompositionMode_ColorDodge;
	else if ( name == "color-burn" )
		return QPainter::CompositionMode_ColorBurn;
	else if ( name == "hard-light" )
		return QPainter::CompositionMode_HardLight;
	else if ( name == "soft-light" )
		return QPainter::CompositionMode_SoftLight;
	else if ( name == "difference" )
		return QPainter::CompositionMode_Difference;
	else if ( name == "exclusion" )
		return QPainter::CompositionMode_Exclusion;
	else if ( name == "src-or-dst" )
		return QPainter::RasterOp_SourceOrDestination;
	else if ( name == "src-and-dst" )
		return QPainter::RasterOp_SourceAndDestination;
	else if ( name == "src-xor-dst" )
		return QPainter::RasterOp_SourceXorDestination;
	else if ( name == "not-src-and-not-dst" )
		return QPainter::RasterOp_NotSourceAndNotDestination;
	else if ( name == "not-src-or-not-dst" )
		return QPainter::RasterOp_NotSourceOrNotDestination;
	else if ( name == "not-src-xor-dst" )
		return QPainter::RasterOp_NotSourceXorDestination;
	else if ( name == "not-src" )
		return QPainter::RasterOp_NotSource;
	else if ( name == "not-src-and-dst" )
		return QPainter::RasterOp_NotSourceAndDestination;
	else if ( name == "src-and-not-dst" )
		return QPainter::RasterOp_SourceAndNotDestination;
	else
		return QPainter::CompositionMode_SourceOver;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::LayerProperties::isChild(const LayerProperties* child) const {
	while ( child ) {
		if ( child == this ) return true;
		child = child->parent;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::LayerProperties::SymbolShape
GeoFeatureLayer::LayerProperties::getSymbolShape(const std::string &type) {
	if ( type == "square" )
		return Square;
	else if ( type == "circle" )
		return Circle;

	throw Core::ValueError();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::LayerProperties::read(const string &dataDir) {
	const static string cfgVisible = "visible";
	const static string cfgPen = "pen";
	const static string cfgBrush = "brush";
	const static string cfgFont = "font";
	const static string cfgDrawName = "drawName";
	const static string cfgDebug = "debug";
	const static string cfgRank = "rank";
	const static string cfgRoughness = "roughness";
	const static string cfgSymbolSize = "symbol.size";
	const static string cfgSymbolShape = "symbol.shape";
	const static string cfgSymbolIcon = "symbol.icon";
	const static string cfgSymbolIconX = "symbol.icon.hotspot.x";
	const static string cfgSymbolIconY = "symbol.icon.hotspot.y";
	const static string cfgTitle = "title";
	const static string cfgLabel = "label";
	const static string cfgIndex = "index";
	const static string cfgLegendArea = "legendArea";
	const static string cfgLegendOrientation = "orientation";
	const static string cfgCompositionMode = "composition";

	// Read additional configuration file (e.g. map.cfg in BNA folder)
	if ( !dataDir.empty() ) {
		Config::Config cfg;
		if ( cfg.readConfig(dataDir + "/map.cfg", -1, true) ) {
			try { visible = cfg.getBool(cfgVisible); } catch( ... ) {}
			pen = readPen(cfg, cfgPen, pen);
			brush = readBrush(cfg, cfgBrush, brush);
			font = readFont(cfg, cfgFont, font);
			try { drawName = cfg.getBool(cfgDrawName); } catch( ... ) {}
			try { debug = cfg.getBool(cfgDebug); } catch( ... ) {}
			try { rank = cfg.getInt(cfgRank); } catch( ... ) {}
			try { roughness = cfg.getInt(cfgRoughness); } catch( ... ) {}
			try { symbolSize = cfg.getInt(cfgSymbolSize); } catch( ... ) {}
			try { symbolShape = getSymbolShape(cfg.getString(cfgSymbolShape)); } catch( ... ) {}
			try {
				string fn = cfg.getString(cfgSymbolIcon);
				if ( !fn.empty() ) {
					if ( fn[0] == '/' )
						symbolIcon = QImage(fn.c_str());
					else
						symbolIcon = QImage((dataDir + '/' + fn).c_str());
				}
			}
			catch( ... ) {}
			try { symbolIconHotspot.setX(cfg.getInt(cfgSymbolIconX)); } catch( ... ) {}
			try { symbolIconHotspot.setY(cfg.getInt(cfgSymbolIconY)); } catch( ... ) {}
			try { title = cfg.getString(cfgTitle); } catch( ... ) {}
			try { label = cfg.getString(cfgLabel); } catch( ... ) {}
			try { index = cfg.getInt(cfgIndex); } catch( ... ) {}
			try { orientation = getOrientation(cfg.getString(cfgLegendOrientation)); } catch( ... ) {}
			try { legendArea = getAlignment(cfg.getString(cfgLegendArea)); } catch( ... ) {}
			try { compositionMode = getCompositionMode(cfg.getString(cfgCompositionMode)); } catch( ... ) {}
		}
	}

	// Query properties from config
	string query = CFG_LAYER_PREFIX ".";
	if ( !name.empty() ) query += name + ".";

	if ( SCApp ) {
		try { visible = SCApp->configGetBool(query + cfgVisible); } catch( ... ) {}
		pen = SCApp->configGetPen(query + cfgPen, pen);
		brush = SCApp->configGetBrush(query + cfgBrush, brush);
		font = SCApp->configGetFont(query + cfgFont, font);
		try { drawName = SCApp->configGetBool(query + cfgDrawName); } catch( ... ) {}
		try { debug = SCApp->configGetBool(query + cfgDebug); } catch( ... ) {}
		try { rank = SCApp->configGetInt(query + cfgRank); } catch( ... ) {}
		try { roughness = SCApp->configGetInt(query + cfgRoughness); } catch( ... ) {}
		try { symbolSize = SCApp->configGetInt(query + cfgSymbolSize); } catch( ... ) {}
		try { symbolShape = getSymbolShape(SCApp->configGetString(query + cfgSymbolShape)); } catch( ... ) {}
		try {
			string fn = SCApp->configGetString(query + cfgSymbolIcon);
			if ( !fn.empty() ) {
				if ( fn[0] == '/' )
					symbolIcon = QImage(fn.c_str());
				else
					symbolIcon = QImage((dataDir + '/' + fn).c_str());
			}
		}
		catch( ... ) {}
		try { symbolIconHotspot.setX(SCApp->configGetInt(query + cfgSymbolIconX)); } catch( ... ) {}
		try { symbolIconHotspot.setY(SCApp->configGetInt(query + cfgSymbolIconY)); } catch( ... ) {}
		try { title = SCApp->configGetString(query + cfgTitle); } catch( ... ) {}
		try { label = SCApp->configGetString(query + cfgLabel); } catch( ... ) {}
		try { index = SCApp->configGetInt(query + cfgIndex); } catch( ... ) {}
		try { orientation = getOrientation(SCApp->configGetString(query + cfgLegendOrientation)); } catch( ... ) {}
		try { legendArea = getAlignment(SCApp->configGetString(query + cfgLegendArea)); } catch( ... ) {}
		try { compositionMode = getCompositionMode(SCApp->configGetString(cfgCompositionMode)); } catch( ... ) {}
	}

	filled = brush.style() != Qt::NoBrush;

	if ( !symbolIcon.isNull() ) {
		if ( symbolSize > 0 ) {
			QSize oldSize = symbolIcon.size();
			symbolIcon = symbolIcon.scaled(symbolSize, symbolSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			symbolIconHotspot.setX(symbolIconHotspot.x() * symbolIcon.size().width() / oldSize.width());
			symbolIconHotspot.setY(symbolIconHotspot.y() * symbolIcon.size().height() / oldSize.height());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode::CategoryNode(const Geo::Category *c)
: category(c), properties(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode::~CategoryNode() {
	if ( properties ) delete properties;
	for ( size_t i = 0; i < childs.size(); ++i )
		delete childs[i];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode *
GeoFeatureLayer::CategoryNode::nodeForCategory(const Geo::Category *cat) {
	if ( category == cat )
		return this;

	for ( size_t i = 0; i < childs.size(); ++i ) {
		GeoFeatureLayer::CategoryNode *node = childs[i]->nodeForCategory(cat);
		if ( node != NULL )
			return node;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode *
GeoFeatureLayer::CategoryNode::nodeForProperties(const LayerProperties *props) {
	if ( properties == props )
		return this;

	for ( size_t i = 0; i < childs.size(); ++i ) {
		GeoFeatureLayer::CategoryNode *node = childs[i]->nodeForProperties(props);
		if ( node != NULL )
			return node;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::GeoFeatureLayer(QObject *parent)
: Layer(parent)
, _initialized(false)
, _root(NULL) {
	setName("features");
	Geo::GeoFeatureSetSingleton::getInstance().registerObserver(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::~GeoFeatureLayer() {
	if ( _root != NULL )
		delete _root;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Geo::GeoFeature *GeoFeatureLayer::findFeature(const Geo::GeoCoordinate &coord) const {
	if ( !isVisible() ) return NULL;

	if ( canvas() == NULL )
		// No canvas, no rank clipping possible
		return NULL;

	if ( _root == NULL )
		return NULL;

	return findFeature(_root, coord);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Geo::GeoFeature *GeoFeatureLayer::findFeature(qreal lat, qreal lon) const {
	return findFeature(Geo::GeoCoordinate(lat, lon));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::renderFeatures(Canvas *canvas, QPainter &painter) {
	if ( !_initialized ) {
		_initialized = true;
		initLayerProperites();
	}

	if ( !_root ) return;

	// Debug pen and label point
	QPen debugPen;
	debugPen.setColor(Qt::black);
	debugPen.setWidth(1);
	debugPen.setStyle(Qt::SolidLine);

	painter.setRenderHint(QPainter::Antialiasing,
	                      !canvas->previewMode() && SCScheme.map.vectorLayerAntiAlias);

	drawFeatures(_root, canvas, painter, debugPen);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::setVisible(bool flag) {
	if ( flag == isVisible() ) return;
	Layer::setVisible(flag);
	emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::bufferUpdated(Canvas *canvas, QPainter &painter) {
	painter.save();
	renderFeatures(canvas, painter);
	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::drawFeatures(CategoryNode *node, Canvas *canvas,
                                   QPainter &painter, const QPen &debugPen) {
	LayerProperties* layProp = node->properties;
	if ( !layProp->visible ) return;

	Projection *proj = canvas->projection();

	if ( proj->isClipped(node->bbox) ) {
		//std::cerr << "Clipped node '" << node->properties->name << "'" << std::endl;
		return;
	}

	painter.setFont(layProp->font);
	painter.setPen(layProp->pen);
	if ( layProp->filled )
		painter.setBrush(layProp->brush);
	else
		painter.setBrush(Qt::NoBrush);

#if BOOST_VERSION < 106000
	node->quadtree.query(proj->boundingBox(), boost::bind(&GeoFeatureLayer::drawFeature, this, canvas, &painter, &debugPen, layProp, _1), true);
#else
	node->quadtree.query(proj->boundingBox(), boost::bind(&GeoFeatureLayer::drawFeature, this, canvas, &painter, &debugPen, layProp, boost::placeholders::_1), true);
#endif

	/*
	for ( size_t i = 0; i < node->features.size(); ++i ) {
		Geo::GeoFeature *f = node->features[i];
		drawFeature(canvas, painter, debugPen, layProp, f);
	}
	*/

	for ( size_t i = 0; i < node->childs.size(); ++i )
		drawFeatures(node->childs[i], canvas, painter, debugPen);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::drawFeature(Canvas *canvas, QPainter *painter,
                                  const QPen *debugPen, LayerProperties *props,
                                  const Geo::GeoFeature *f) {
	int rank = props->rank < 0?f->rank():props->rank;
	if ( rank > canvas->zoomLevel() ) return true;

	Projection *proj = canvas->projection();

	if ( props->compositionMode != painter->compositionMode() )
		painter->setCompositionMode(props->compositionMode);

	// There must be at least 2 verticies to draw something
	if ( f->vertices().size() < 2 ) {
		if ( f->vertices().size() == 1 ) {
			QPoint p;
			if ( !proj->project(p, QPointF(f->vertices()[0].lon, f->vertices()[0].lat)) )
				return true;

			int width = props->symbolSize;
			if ( width < 0 ) width = 8;

			if ( !props->symbolIcon.isNull() ) {
				painter->drawImage(p - props->symbolIconHotspot, props->symbolIcon);
				p.setY(p.y() - props->symbolIconHotspot.y() + props->symbolIcon.height());
			}
			else {
				switch ( props->symbolShape ) {
					case LayerProperties::Square:
						painter->drawRect(p.x()-width/2, p.y()-width/2, width, width);
						break;
					case LayerProperties::Circle:
						painter->drawEllipse(p.x()-width/2, p.y()-width/2, width, width);
						break;
					default:
						break;
				}

				p.setY(p.y() + width/2);
			}

			if ( props->drawName ) {
				QString name = f->name().c_str();
				QRect textRect = painter->fontMetrics().boundingRect(name);
				textRect.moveTop(p.y() + textRect.height()/4);
				textRect.moveLeft(p.x() - textRect.width()/2);
				painter->drawText(textRect, Qt::AlignCenter, name);
			}
		}
	}
	else {
		canvas->drawFeature(*painter, f, props->filled, props->roughness);

		const Geo::GeoBoundingBox &bbox = f->bbox();

		// Draw the name if requested and if there is enough space
		if ( props->drawName ) {
			QPoint p1, p2;
			qreal lonMin = bbox.west;
			qreal lonMax = bbox.east;

			if ( fabs(lonMax-lonMin) > 180 )
				qSwap(lonMin, lonMax);

			if ( proj->project(p1, QPointF(lonMin, bbox.north))
			  && proj->project(p2, QPointF(lonMax, bbox.south)) ) {
				QRect bboxRect = QRect(p1, p2);
				QString name = f->name().c_str();
				QRect textRect = painter->fontMetrics().boundingRect(name);
				if ( textRect.width()*100 < bboxRect.width()*80 &&
				     textRect.height()*100 < bboxRect.height()*80 )
					painter->drawText(bboxRect, Qt::AlignCenter, name);
			}
		}

		// Debug: Print the segment name and draw the bounding box
		if ( props->debug ) {
			QPoint debugPoint;
			painter->setPen(*debugPen);
			// project the center of the bounding box
			float bboxWidth = bbox.width();
			float bboxHeight = bbox.height();
			Geo::GeoCoordinate center = bbox.center();

			if ( proj->project(debugPoint, QPointF(center.lon, center.lat)) ) {
				QFont font;
				float maxBBoxEdge = bboxWidth > bboxHeight ? bboxWidth : bboxHeight;
				int pixelSize = (int)(proj->pixelPerDegree() * maxBBoxEdge / 10.0);
				font.setPixelSize(pixelSize < 1 ? 1 : pixelSize > 30 ? 30 : pixelSize);
				QFontMetrics metrics(font);
				QRect labelRect(metrics.boundingRect(f->name().c_str()));
				labelRect.moveTo(debugPoint.x() - labelRect.width()/2,
				                 debugPoint.y() - labelRect.height()/2);

				painter->setFont(font);
				painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignTop,
				                  f->name().c_str());
			}

			proj->moveTo(QPointF(bbox.west, bbox.south));
			proj->lineTo(*painter, QPointF(bbox.east, bbox.south));
			proj->lineTo(*painter, QPointF(bbox.east, bbox.north));
			proj->lineTo(*painter, QPointF(bbox.west, bbox.north));
			proj->lineTo(*painter, QPointF(bbox.west, bbox.south));

			painter->setPen(props->pen);
			painter->setFont(props->font);
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMenu *GeoFeatureLayer::menu(QMenu *parentMenu) const {
	QMenu *menu = buildMenu(_root, parentMenu);

	QAction *reloadAction = new QAction(tr("Reload features"), menu);
	connect(reloadAction, SIGNAL(triggered()), this, SLOT(reloadFeatures()));

	if ( menu->isEmpty() )
		menu->addAction(reloadAction);
	else
		menu->insertAction(menu->actions().first(), reloadAction);

	return menu;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::geoFeatureSetUpdated() {
	_initialized = false;

	if ( _root != NULL ) {
		delete _root;
		_root = NULL;
	}

	emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::showFeatures() {
	QAction *action = static_cast<QAction*>(sender());
	void *nodePtr = action->data().value<void*>();
	CategoryNode *node = reinterpret_cast<CategoryNode*>(nodePtr);

	bool wantUpdate = false;
	for ( size_t i = 0; i < node->childs.size(); ++i ) {
		if ( toggleVisibility(node->childs[i], true) )
			wantUpdate = true;
	}

	if ( wantUpdate )
		emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::hideFeatures() {
	QAction *action = static_cast<QAction*>(sender());
	void *nodePtr = action->data().value<void*>();
	CategoryNode *node = reinterpret_cast<CategoryNode*>(nodePtr);

	bool wantUpdate = false;
	for ( size_t i = 0; i < node->childs.size(); ++i ) {
		if ( toggleVisibility(node->childs[i], false) )
			wantUpdate = true;
	}

	if ( wantUpdate )
		emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::reloadFeatures() {
	Geo::GeoFeatureSet &featureSet = Geo::GeoFeatureSetSingleton::getInstance();
	featureSet.load();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode *
GeoFeatureLayer::createOrGetNodeForCategory(const Geo::Category *cat) {
	if ( _root == NULL ) {
		_root = new CategoryNode(NULL);
		_root->properties = new LayerProperties("");
		_root->properties->read();
	}

	if ( (cat == NULL) )
		return _root;

	CategoryNode *node = _root->nodeForCategory(cat);
	if ( node != NULL )
		return node;

	// Create parent chain
	CategoryNode *parentNode = createOrGetNodeForCategory(cat->parent);
	node = new CategoryNode(cat);

	node->properties = new LayerProperties(cat->name.empty() ? "local" : cat->name.c_str(), parentNode->properties);
	node->properties->read(cat->dataDir);

	parentNode->childs.push_back(node);
	return node;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::buildLegends(CategoryNode *node) {
	if ( node == NULL ) return;

	LayerProperties *prop = node->properties;
	if ( !prop->title.empty() ) {
		StandardLegend *legend = new StandardLegend(this);
		legend->setTitle(prop->title.c_str());
		legend->setArea(prop->legendArea);
		legend->setOrientation(prop->orientation);

		QVector<LayerProperties*> items;
		// Find all child labels
		collectLegendItems(node, items);

		qSort(items.begin(), items.end(), compareByIndex);

		for ( int i = 0; i < items.count(); ++i ) {
			if ( items[i]->filled )
				legend->addItem(new StandardLegendItem(items[i]->pen,
				                                       items[i]->brush,
				                                       items[i]->label.c_str()));
			else
				legend->addItem(new StandardLegendItem(items[i]->pen,
				                                       items[i]->label.c_str()));
		}

		addLegend(legend);
	}

	for ( size_t i = 0; i < node->childs.size(); ++i )
		buildLegends(node->childs[i]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMenu *GeoFeatureLayer::buildMenu(CategoryNode *node, QMenu *parentMenu) const {
	QMenu *menu = new QMenu(parentMenu);
	if ( node == NULL )
		return menu;

	size_t visibleCount = 0;
	for ( size_t i = 0; i < node->childs.size(); ++i ) {
		LayerProperties *childProps = node->childs[i]->properties;
		std::string title = childProps->name;
		if ( node->childs[i]->category && !node->childs[i]->category->localName.empty() )
			title = node->childs[i]->category->localName;

		if ( !childProps->visible || node->childs[i]->childs.empty() ) {
			QAction *action = menu->addAction(title.c_str());
			action->setCheckable(true);
			action->setChecked(childProps->visible);
			if ( childProps->visible ) ++visibleCount;
			action->setData(QVariant::fromValue<void*>(childProps));
			connect(action, SIGNAL(toggled(bool)), this, SLOT(toggleFeatureVisibility(bool)));
		}
		else {
			QMenu *subMenu = buildMenu(node->childs[i], menu);
			subMenu->setTitle(title.c_str());
			menu->addMenu(subMenu);
		}
	}

	// Add "Select all" and "Select none" options if more than 1 property
	// is available
	QAction *firstPropertyAction = menu->actions().first();

	if ( (node != _root) && !node->childs.empty() ) {
		// Toggle layer
		QAction *toggleAction = new QAction(tr("Hide layer"), menu);
		toggleAction->setData(QVariant::fromValue<void*>(node->properties));
		connect(toggleAction, SIGNAL(triggered()), this, SLOT(disableFeatureVisibility()));
		menu->insertAction(firstPropertyAction, toggleAction);

		// Separator
		menu->insertSeparator(firstPropertyAction);
	}

	if ( node->childs.size() >= 2 ) {
		// Select all
		QAction *allAction = new QAction(tr("Show all sublayers"), menu);
		allAction->setEnabled(visibleCount < node->childs.size());
		allAction->setData(QVariant::fromValue<void*>(node));
		connect(allAction, SIGNAL(triggered()), this, SLOT(showFeatures()));
		menu->insertAction(firstPropertyAction, allAction);

		// Select none
		QAction *noneAction = new QAction(tr("Hide all sublayers"), menu);
		noneAction->setEnabled(visibleCount > 0);
		noneAction->setData(QVariant::fromValue<void*>(node));
		connect(noneAction, SIGNAL(triggered()), this, SLOT(hideFeatures()));
		menu->insertAction(firstPropertyAction, noneAction);

		// Separator
		menu->insertSeparator(firstPropertyAction);
	}

	return menu;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::collectLegendItems(GeoFeatureLayer::CategoryNode *node,
                                         QVector<LayerProperties*> &items) {
	if ( !node->properties->label.empty() )
		items.push_back(node->properties);

	for ( size_t i = 0; i < node->childs.size(); ++i )
		collectLegendItems(node->childs[i], items);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::orderTree(CategoryNode *node) {
	//sort(node->features.begin(), node->features.end(), compareByRank);
	sort(node->childs.begin(), node->childs.end(), compareNodeByIndex);

	for ( size_t i = 0; i < node->childs.size(); ++i )
		orderTree(node->childs[i]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::updateBbox(CategoryNode *node) {
	for ( size_t i = 0; i < node->childs.size(); ++i )
		updateBbox(node->childs[i]);

	// Do calculation
	/*
	bool noFeatures = node->features.empty();
	for ( size_t i = 0; i < node->features.size(); ++i ) {
		const Geo::GeoBoundingBox &bbox = node->features[i]->bbox();
		if ( !i )
			node->bbox = bbox;
		else
			node->bbox += bbox;
	}
	*/
	bool noFeatures = false;
	node->bbox = node->quadtree.bbox();

	for ( size_t i = 0; i < node->childs.size(); ++i ) {
		const Geo::GeoBoundingBox &bbox = node->childs[i]->bbox;
		if ( !i && noFeatures)
			node->bbox = bbox;
		else
			node->bbox += bbox;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::initLayerProperites() {
	// Create a layer properties from BNA geo features
	const Geo::GeoFeatureSet &featureSet = Geo::GeoFeatureSetSingleton::getInstance();

	vector<Geo::GeoFeature*>::const_iterator itf = featureSet.features().begin();
	for ( ; itf != featureSet.features().end(); ++itf ) {
		CategoryNode *node = createOrGetNodeForCategory((*itf)->category());
		node->quadtree.addItem(*itf);
		//node->features.push_back(*itf);
	}

	const Geo::PolyRegions &fepRegions = Regions::polyRegions();
	if ( fepRegions.regionCount() > 0 ) {
		// Add fep properties
		CategoryNode *fepNode = new CategoryNode(NULL);
		createOrGetNodeForCategory(NULL)->childs.push_back(fepNode);
		fepNode->properties = new LayerProperties("fep", _root->properties);
		fepNode->properties->read(fepRegions.dataDir());

		for ( size_t i = 0; i < fepRegions.regionCount(); ++i )
			//fepNode->features.push_back(fepRegions.region(i));
			fepNode->quadtree.addItem(fepRegions.region(i));
	}

	if ( _root != NULL ) {
		// Build legends
		buildLegends(_root);
		orderTree(_root);
		updateBbox(_root);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Geo::GeoFeature *GeoFeatureLayer::findFeature(CategoryNode *node, const Geo::GeoCoordinate &coord) const {
	if ( !node->properties->visible )
		return NULL;

	if ( !node->bbox.contains(coord) )
		return NULL;

	for ( size_t i = node->childs.size(); i > 0; --i ) {
		CategoryNode *child;
		child = node->childs[i-1];
		const Geo::GeoFeature *f = findFeature(child, coord);
		if ( f != NULL )
			return f;
	}

	/*
	for ( size_t i = node->features.size(); i > 0; --i ) {
		Geo::GeoFeature *f = node->features[i-1];
		int rank = node->properties->rank < 0?f->rank():node->properties->rank;
		if ( rank <= canvas()->zoomLevel() ) {
			if ( f->bbox().contains(coord) && f->contains(coord) )
				return f;
		}
	}
	*/
	return node->quadtree.findLast(coord);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::toggleVisibility(CategoryNode *node, bool visible) {
	bool updateRequired = false;
	if ( node->properties->visible != visible ) {
		node->properties->visible = visible;
		updateRequired = true;
	}

	return updateRequired;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::toggleFeatureVisibility(bool checked) {
	QAction *action = static_cast<QAction*>(sender());
	void *propertyPtr = action->data().value<void*>();
	LayerProperties *prop = reinterpret_cast<LayerProperties*>(propertyPtr);

	if ( prop == NULL )
		return;

	CategoryNode *node = _root->nodeForProperties(prop);
	if ( node && toggleVisibility(node, checked) )
		emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::disableFeatureVisibility() {
	toggleFeatureVisibility(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::compareByIndex(const LayerProperties *p1,
                                     const LayerProperties *p2) {
	return p1->index < p2->index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::compareNodeByIndex(const GeoFeatureLayer::CategoryNode *n1,
                                         const GeoFeatureLayer::CategoryNode *n2) {
	return n1->properties->index < n2->properties->index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
