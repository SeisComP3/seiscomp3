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
#include <seiscomp3/gui/map/layers/geofeaturelayer.h>
#include <seiscomp3/gui/map/canvas.h>
#include <seiscomp3/geo/geofeatureset.h>
#include <seiscomp3/seismology/regions.h>

#include <iostream>


namespace Seiscomp {
namespace Gui {
namespace Map {

namespace {


#define CFG_LAYER_PREFIX "map.layers"

void readLayerProperties(LayerProperties *props) {
	const static std::string cfgVisible = ".visible";
	const static std::string cfgPen = ".pen";
	const static std::string cfgBrush = ".brush";
	const static std::string cfgFont = ".font";
	const static std::string cfgDrawName = ".drawName";
	const static std::string cfgDebug = ".debug";
	const static std::string cfgRank = ".rank";
	const static std::string cfgRoughness = ".roughness";

	// Query properties from config
	std::string query = CFG_LAYER_PREFIX;
	if ( !props->name.empty() ) query += "." + props->name;

	if ( SCApp ) {
		try { props->visible = SCApp->configGetBool(query + cfgVisible); } catch( ... ) {}
		props->pen = SCApp->configGetPen(query + cfgPen, props->pen);
		props->brush = SCApp->configGetBrush(query + cfgBrush, props->brush);
		props->font = SCApp->configGetFont(query + cfgFont, props->font);
		try { props->drawName = SCApp->configGetBool(query + cfgDrawName); } catch( ... ) {}
		try { props->debug = SCApp->configGetBool(query + cfgDebug); } catch( ... ) {}
		try { props->rank = SCApp->configGetInt(query + cfgRank); } catch( ... ) {}
		try { props->roughness = SCApp->configGetInt(query + cfgRoughness); } catch( ... ) {}
	}

	props->filled = props->brush.style() != Qt::NoBrush;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::GeoFeatureLayer(QObject* parent)
: Layer(parent)
, _initialized(false) {
	setName("features");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::~GeoFeatureLayer() {
	for ( size_t i = 0; i < _layerProperties.size(); ++i )
		delete _layerProperties[i];
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
void GeoFeatureLayer::bufferUpdated(Canvas *canvas) {
	if ( !_initialized ) {
		_initialized = true;
		initLayerProperites();
	}

	const Geo::GeoFeatureSet &featureSet = Geo::GeoFeatureSetSingleton::getInstance();

	size_t linesPlotted = 0;
	size_t polygonsPlotted = 0;

	size_t categoryId = 0;
	LayerProperties* layProp = NULL;

	// Debug pen and label point
	QPen debugPen;
	debugPen.setColor(Qt::black);
	debugPen.setWidth(1);
	debugPen.setStyle(Qt::SolidLine);

	bool filled = false;
	int zoomLevel = canvas->zoomLevel();

	QPainter painter(&canvas->buffer());
	painter.setRenderHint(QPainter::Antialiasing,
	                      !canvas->previewMode() && SCScheme.map.vectorLayerAntiAlias);

	// Iterate over all features
	std::vector<Geo::GeoFeature*>::const_iterator itf = featureSet.features().begin();
	for ( ; itf != featureSet.features().end(); ++itf ) {
		// Update painter settings if necessary
		if ( layProp == NULL || categoryId != (*itf)->category()->id ) {
			categoryId = (*itf)->category()->id;
			layProp = _layerProperties.at(categoryId);
			filled = canvas->projection()->isRectangular()?layProp->filled:false;
			painter.setFont(layProp->font);
			painter.setPen(layProp->pen);
			if ( filled ) painter.setBrush(layProp->brush);
		}

		if ( !canvas->drawGeoFeature(painter, *itf, layProp, debugPen, linesPlotted,
		                             polygonsPlotted, filled) ) break;
	}

	// Last property is for "fep"
	const Geo::PolyRegions &fepRegions = Regions().polyRegions();
	layProp = fepRegions.regionCount() > 0 ? _layerProperties.back() : NULL;

	// Skip, if the layer was disabled
	if ( layProp && layProp->visible && layProp->rank <= zoomLevel ) {
		painter.setFont(layProp->font);
		painter.setPen(layProp->pen);
		filled = canvas->projection()->isRectangular()?layProp->filled:false;
		if ( filled ) painter.setBrush(layProp->brush);
		for ( size_t i = 0; i < fepRegions.regionCount(); ++i ) {
			Geo::GeoFeature *reg = fepRegions.region(i);
			if ( !canvas->drawGeoFeature(painter, reg, layProp, debugPen, linesPlotted,
			                             polygonsPlotted, filled) ) break;
		}
	}

	/*
	if ( polygonsPlotted > 0 ) {
		SEISCOMP_DEBUG("zoom: %f, pixelPerDegree: %f -- %i polygons with %i lines plotted",
		               _zoomLevel, _projection->pixelPerDegree(),
		               (uint)polygonsPlotted, (uint)linesPlotted);
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMenu *GeoFeatureLayer::menu(QMenu *parent) const {
	if ( _layerProperties.size() < 2 )
		return NULL;

	QMenu *menu = new QMenu(parent);

	size_t visibleCount = 0;
	std::vector<LayerProperties*>::const_iterator it = _layerProperties.begin();
	const LayerProperties *root = *it++;
	for ( ; it != _layerProperties.end(); ++it ) {
		if ( (*it)->parent != root ) continue;
		QAction *action = menu->addAction((*it)->name.c_str());
		action->setCheckable(true);
		action->setChecked((*it)->visible);
		if ( (*it)->visible ) ++visibleCount;
		action->setData(QVariant::fromValue<void*>(*it));
		connect(action, SIGNAL(toggled(bool)), this, SLOT(toggleFeatureVisibility(bool)));
	}

	// Add "Select all" and "Select none" options if more than 1 property
	// is available
	if ( _layerProperties.size() >= 2 ) {
		QAction *firstPropertyAction = menu->actions().first();

		// Select all
		QAction *allAction = new QAction(tr("Select all"), menu);
		allAction->setEnabled(visibleCount < _layerProperties.size());
		connect(allAction, SIGNAL(triggered()), this, SLOT(showFeatures()));
		menu->insertAction(firstPropertyAction, allAction);

		// Select none
		QAction *noneAction = new QAction(tr("Select none"), menu);
		noneAction->setEnabled(visibleCount > 0);
		connect(noneAction, SIGNAL(triggered()), this, SLOT(hideFeatures()));
		menu->insertAction(firstPropertyAction, noneAction);

		// Separator
		menu->insertSeparator(firstPropertyAction);
	}

	return menu;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::toggleFeatureVisibility(bool checked) {
	QAction *action = static_cast<QAction*>(sender());
	void *propertyPtr = action->data().value<void*>();
	LayerProperties *prop = reinterpret_cast<LayerProperties*>(propertyPtr);
	if ( prop == NULL )
		return;

	if ( prop->visible == checked ) return;

	prop->visible = checked;

	std::vector<Map::LayerProperties*>::const_iterator it;
	for ( it = _layerProperties.begin(); it != _layerProperties.end(); ++it ) {
		if ( !prop->isChild(*it) ) continue;
		(*it)->visible = checked;
	}

	emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::showFeatures() {
	setFeaturesVisibility(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::hideFeatures() {
	setFeaturesVisibility(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::initLayerProperites() {
	// Create a layer properties from BNA geo features
	const Geo::GeoFeatureSet &featureSet = Geo::GeoFeatureSetSingleton::getInstance();
	std::vector<Geo::Category*>::const_iterator itc = featureSet.categories().begin();
	for ( ; itc != featureSet.categories().end(); ++itc ) {
		// Initialize the base pen with the parent pen if available,
		// else use the default constructor
		Geo::Category *cat = *itc;
		LayerProperties *props = cat->parent == 0 ?
			new LayerProperties(cat->name.c_str()) :
			new LayerProperties(cat->name.c_str(), _layerProperties.at(cat->parent->id));
		_layerProperties.push_back(props);
		readLayerProperties(props);
	}

	const Geo::PolyRegions &fepRegions = Regions::polyRegions();
	if ( fepRegions.regionCount() > 0 ) {
		// Add empty root property if not exists yet
		if ( _layerProperties.empty() ) {
			_layerProperties.push_back(new LayerProperties(""));
			readLayerProperties(_layerProperties.front());
		}
		// Add fep properties
		_layerProperties.push_back(new LayerProperties("fep", _layerProperties.front()));
		readLayerProperties(_layerProperties.back());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::setFeaturesVisibility(bool visible) {
	bool updateRequired = false;
	std::vector<LayerProperties*>::const_iterator it = _layerProperties.begin();
	for ( ; it != _layerProperties.end(); ++it ) {
		if ( (*it)->visible != visible ) {
			(*it)->visible = visible;
			updateRequired = true;
		}
	}

	if ( updateRequired ) {
		emit updateRequested(RasterLayer);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
