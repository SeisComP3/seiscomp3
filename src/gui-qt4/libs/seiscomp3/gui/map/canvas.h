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


#ifndef __SEISCOMP_GUI_MAP_CANVAS_H__
#define __SEISCOMP_GUI_MAP_CANVAS_H__


#ifndef Q_MOC_RUN
#include <seiscomp3/gui/core/maps.h>
#include <seiscomp3/gui/map/projection.h>
#include <seiscomp3/gui/map/legend.h>
#include <seiscomp3/gui/map/imagetree.h>
#include <seiscomp3/gui/map/mapsymbolcollection.h>
#include <seiscomp3/gui/map/layers/citieslayer.h>
#include <seiscomp3/gui/map/layers/gridlayer.h>
#include <seiscomp3/gui/map/layers/geofeaturelayer.h>
#include <seiscomp3/math/coord.h>
#include <seiscomp3/geo/geofeature.h>
#endif

#include <QHash>
#include <QObject>
#include <QPolygon>

class QMouseEvent;
class QMenu;


namespace Seiscomp {
namespace Gui {
namespace Map {


class Layer;

DEFINE_SMARTPOINTER(TextureCache);


struct SC_GUI_API LayerProperties {
	const LayerProperties *parent;
	std::string            name;
	bool                   visible;
	QPen                   pen;
	QBrush                 brush;
	QFont                  font;
	bool                   drawName;
	bool                   debug;
	int                    rank;
	int                    roughness;
	bool                   filled;

	LayerProperties(const std::string &name)
	: parent(NULL), name(name)
	, visible(true), drawName(false)
	, debug(false), rank(-1), roughness(3)
	, filled(false) {}

	LayerProperties(const std::string &name, const LayerProperties* parent)
	: parent(parent), name(name)
	, visible(parent->visible), pen(parent->pen)
	, brush(parent->brush), font(parent->font)
	, drawName(parent->drawName), debug(parent->debug)
	, rank(-1), roughness(parent->roughness)
	, filled(parent->filled) {}

	bool isChild(const LayerProperties* child) const;
};


class SC_GUI_API Canvas : public QObject {
	Q_OBJECT

	public:
		Canvas(const MapsDesc &);
		Canvas(ImageTree *mapTree);
		~Canvas();

		void setFont(QFont f);
		QFont font() const { return _font; }

		void setBackgroundColor(QColor c);
		bool setProjectionByName(const char *name);
		const std::string &projectionName() const { return _projectionName; }

		void setSize(int w, int h);
		QSize size() const { return _buffer.size(); }

		int width() const { return _buffer.width(); }
		int height() const { return _buffer.height(); }

		/**
		 * @brief Sets the margin of the legend area with respect to the
		 *        canvas border. The default value is 10 pixels. This methods
		 *        was added with API 11.
		 * @param margin The margin in pixels.
		 */
		void setLegendMargin(int margin);

		/**
		 * @brief Returns the margin of the legend area with respect to the
		 *        canvas border. This methods was added with API 11.
		 * @return The margin in pixels
		 */
		int legendMargin() const { return _margin; }

		void setGrayScale(bool);
		bool isGrayScale() const;

		/**
		 * @brief Sets bilinear filtering for map tile interpolation. This is
		 *        only used if not in preview mode.
		 * @param enable Boolean flag
		 */
		void setBilinearFilter(bool enable);

		/**
		 * @brief Enables the preview mode. The preview mode is meant to be
		 *        used for fast rendering the map, e.g. with antialiasing
		 *        switched of. This is mainly used while dragging the map.
		 * @param enable Boolean flag
		 */
		void setPreviewMode(bool enable);

		/**
		 * @brief Returns whether the canvas rendering is currently in preview
		 *        mode or not.
		 * @return A boolean flag
		 */
		bool previewMode() const;

		void setDrawGrid(bool);
		bool isDrawGridEnabled() const;

		void setDrawLayers(bool);
		bool isDrawLayersEnabled() const;

		void setDrawCities(bool);
		bool isDrawCitiesEnabled() const;

		bool isDrawLegendsEnabled() const;

		void setImageFilter(bool);
		bool isImageFilterEnabled() const;

		const QRectF& geoRect() const;
		bool displayRect(const QRectF& rect);

		void setMapCenter(QPointF c);
		const QPointF& mapCenter() const;

		bool setZoomLevel(float);
		float zoomLevel() const;

		float pixelPerDegree() const;

		void setView(QPoint c, float zoom);
		void setView(QPointF c, float zoom);

		Map::Projection *projection() const { return _projection; }

		bool isInside(double lon, double lat) const;
		bool isVisible(double lon, double lat) const;

		const SymbolCollection *symbolCollection() const;
		SymbolCollection *symbolCollection();

		void setSelectedCity(const Math::Geo::CityD*);

		//! Draws a geometric line (great circle) given in geographical coordinates (lon, lat)
		//! Returns the distance in degree of the line
		double drawGeoLine(QPainter& painter, const QPointF& start, const QPointF& end) const;

		//! Draws a geofeature with layer properties.
		bool drawGeoFeature(QPainter &painter, const Geo::GeoFeature *f,
		                    const LayerProperties *props, const QPen &debugPen,
		                    size_t &linesPlotted, size_t &polyPlotted,
		                    bool filled) const;

		//! Draws a polyline in geographical coordinates (lon, lat).
		//! Returns number of line segments drawn.
		//! Does not check for clipping
		size_t drawGeoPolyline(QPainter &painter, size_t n, const Math::Geo::CoordF *line,
		                       bool isClosedPolygon, uint minPixelDist = 3,
		                       bool interpolate = false) const;
		//! Does not check for clipping
		size_t drawGeoPolygon(QPainter &painter, size_t n, const Math::Geo::CoordF *line,
		                      uint minPixelDist = 3) const;
		//! Draws a GeoFeature as either polyline or filled polygon and
		//! checks for clipping
		size_t drawGeoFeature(QPainter &painter, const Geo::GeoFeature *feature,
		                      uint minPixelDist = 3, bool interpolate = false,
		                      bool filled = false) const;

		void draw(QPainter& p);

		void centerMap(const QPoint &centerPnt);
		void translate(const QPoint &delta);
		void translate(const QPointF &delta);

		void drawImageLayer(QPainter &painter);
		void drawVectorLayer(QPainter &painter);

		void drawLayers(QPainter &painter);
		void drawDrawables(QPainter &painter);
		void drawImage(const QRectF &geoReference, const QImage &image,
		               CompositionMode compositionMode = CompositionMode_Default);

		void updateBuffer();

		void setBuffer(QImage buffer) { _buffer = buffer; }
		QImage &buffer() { return _buffer; }

		//! Returns the number of layers
		int layerCount() const { return _layers.count(); }

		//! Returns the i-th layer if the index is valid
		Layer* layer(int i) const {
			if ( i < 0 || i >= _layers.count() ) return NULL;

			return _layers[i];
		}

		//! Canvas does not take ownership of layer.
		bool prependLayer(Layer*);
		bool addLayer(Layer*);
		bool insertLayerBefore(const Layer*, Layer*);

		void removeLayer(Layer*);

		void lower(Layer*);
		void raise(Layer*);

		bool filterContextMenuEvent(QContextMenuEvent*, QWidget*);
		bool filterKeyPressEvent(QKeyEvent *event);
		bool filterKeyReleaseEvent(QKeyEvent *event);
		bool filterMouseMoveEvent(QMouseEvent*);
		bool filterMousePressEvent(QMouseEvent*);
		bool filterMouseReleaseEvent(QMouseEvent*);
		bool filterMouseDoubleClickEvent(QMouseEvent*);

		QMenu* menu(QMenu*) const;

		//! Returns whether the rendering is complete or if there are
		//! still some updates in the pipeline that updated later. If this
		//! function return false, the signal renderingCompleted() is emitted
		//! once it is done.
		//! This function was introduced in API 1.1.
		bool renderingComplete() const;


	public slots:
		//! Reloads all tiles and empties the texture cache
		//! This slot was introduced in API 1.1.
		void reload();

		//! This slot was added in API 11
		void setDrawLegends(bool);

		//! This slot was added in API 11
		void showLegends();

		//! This slot was added in API 11
		void hideLegends();

		/**
		 * @brief Enables/disables legend stacking.
		 *
		 * If legend stacking is enabled then two toggle buttons will be
		 * rendered in the legends title bar to swap the visible legend. If
		 * stacking is disabled then all legends of a particular edge will
		 * be rendered next to each other. This slot was added in API 11.
		 */
		void setLegendStacking(bool);

		void bringToFront(Seiscomp::Gui::Map::Legend*);
		void setLegendEnabled(Seiscomp::Gui::Map::Legend*, bool);

		/**
		 * @brief This handler is called when a new legend is
		 * added to a layer.
		 * This slot was introduced with API XX
		 * @param legend The legend
		 */
		void onLegendAdded(Legend *legend);

		/**
		 * @brief This handler is called when a legend is removed
		 * from a layer.
		 * This slot was introduced with API XX
		 * @param legend
		 */
		void onLegendRemoved(Legend *legend);

	signals:
		//! This signal is emitted if draw() caused asynchronous data requests
		//! and when those requests are finished.
		//! This signal was introduced with API 1.1.
		void renderingCompleted();

		void bufferUpdated();
		void projectionChanged(Seiscomp::Gui::Map::Projection*);
		void legendVisibilityChanged(bool);
		void updateRequested();
		void customLayer(QPainter*);


	private:
		void init();

		void drawCity(QPainter &painter, const Math::Geo::CityD &,
		              QVector< QList<QRect> > &grid,
		              QFont &font, QFontMetrics &fontMetrics,
		              int rowHeight,
		              bool &lastUnderline, bool &lastBold);

		void drawDrawables(QPainter &painter, Symbol::Priority priority);

		void drawLegends(QPainter &painter);

		int polyToCache(size_t n, const Math::Geo::CoordF *points,
		                uint minPixelDist) const;

		void setupLayer(Layer *layer);


	private slots:
		void updatedTiles();
		void updateLayer(const Layer::UpdateHints&);


	private:
		typedef QVector<Legend*> Legends;
		struct LegendArea : public Legends {
			LegendArea() : currentIndex(-1) {}

			bool hasIndex(int index) {
				return ( index >= 0 && index < count() );
			}

			bool mousePressEvent(QMouseEvent *e);
			bool mouseReleaseEvent(QMouseEvent *e);
			int findNext(bool forward = true) const;

			QRect      header;
			QRect      decorationRects[2];
			int        currentIndex;
		};

		typedef QHash<Qt::Alignment, LegendArea> LegendAreas;

		typedef QList<Layer*> Layers;
		typedef QList<LayerPtr> CustomLayers;

	private:
		QFont                         _font;
		Projection*                   _projection;
		ImageTreePtr                  _maptree;

		std::string                   _projectionName;

		QImage                        _buffer;
		QColor                        _backgroundColor;

		QRectF                        _geoReference;

		double                        _maxZoom;
		QPointF                       _center;
		float                         _zoomLevel;
		bool                          _grayScale;
		bool                          _filterMap;
		bool                          _dirtyImage;
		bool                          _dirtyLayers;
		bool                          _previewMode;
		bool                          _stackLegends;

		DefaultSymbolCollection       _mapSymbolCollection;

		Layers                        _layers;
		Layer                        *_hoverLayer;
		CustomLayers                  _customLayers;
		CitiesLayer                   _citiesLayer;
		GridLayer                     _gridLayer;
		GeoFeatureLayer               _geoFeatureLayer;

		LegendAreas                   _legendAreas;
		int                           _margin;
		bool                          _isDrawLegendsEnabled;

		mutable QPolygon              _polyCache;
};


inline bool Canvas::previewMode() const {
	return _previewMode;
}


}
}
}


#endif
