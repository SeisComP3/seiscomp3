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
#include <seiscomp3/gui/map/layers/citieslayer.h>
#include <seiscomp3/gui/map/layers/gridlayer.h>
#include <seiscomp3/gui/map/layers/geofeaturelayer.h>
#include <seiscomp3/gui/map/layers/symbollayer.h>
#include <seiscomp3/math/coord.h>
#include <seiscomp3/geo/feature.h>
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

// For backward compatibility
typedef SymbolLayer SymbolCollection;


class SC_GUI_API Canvas : public QObject {
	Q_OBJECT

	public:
		Canvas(const MapsDesc &);
		Canvas(ImageTree *mapTree);
		~Canvas();


	public:
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

		bool displayRect(const QRectF& rect);

		void setMapCenter(QPointF c);
		const QPointF& mapCenter() const;

		bool setZoomLevel(float);
		float zoomLevel() const;

		float pixelPerDegree() const;

		void setView(QPoint c, float zoom);
		void setView(QPointF c, float zoom);

		/**
		 * @brief Sets the minimum pixel distance of two adjacent vertices
		 *        to be rendered as a line when rendering GeoFeatures and polylines.
		 * Or in other words, if the pixel distance of two adjacent vertices is
		 * less than the given distance, both vertices collapse into a single
		 * vertex. The greater the distance the less details the rendered shape
		 * will have.
		 * @param pixel The distance in pixels
		 */
		void setPolygonRoughness(uint pixel);
		uint polygonRoughness() const;

		/**
		 * @brief Sets the polygon clip hint.
		 *
		 * The hint might or might not be interpreted by projections. If the
		 * clipHint is DoClip then polygons are clipped against the viewport
		 * before rendering them. This will reduce the number of vertices
		 * significantly but also increases processing time. The default
		 * is DoClip.
		 * @param hint The clipping hint
		 */
		void setPolygonClipHint(ClipHint hint);
		ClipHint polygonClipHint() const;

		Map::Projection *projection() const { return _projection; }

		bool isVisible(double lon, double lat) const;

		void setSelectedCity(const Math::Geo::CityD*);

		//! Draws a geometric line (great circle) given in geographical coordinates (lon, lat)
		//! Returns the distance in degree of the line
		double drawLine(QPainter &painter, const QPointF &start, const QPointF &end) const;

		//! Draws a polyline in geographical coordinates (lon, lat).
		//! Returns number of line segments drawn.
		//! Does not check for clipping
		size_t drawPolyline(QPainter &painter, size_t n, const Geo::GeoCoordinate *line,
		                    bool isClosedPolygon, bool interpolate = false,
		                    int roughness = -1) const;

		//! Does not check for clipping
		size_t drawPolygon(QPainter &painter, size_t n, const Geo::GeoCoordinate *line,
		                   bool isClosedPolygon = true, int roughness = -1,
		                   ClipHint clipHint = DoClip) const;

		//! Draws a GeoFeature as either polyline or filled polygon and
		//! checks for clipping
		size_t drawFeature(QPainter &painter, const Geo::GeoFeature *feature,
		                   bool filled = false, int roughness = -1,
		                   ClipHint clipHint = DoClip) const;

		/**
		 * @brief Draws an image onto the current map buffer. If this function
		 *        is called *after* @drawImageLayer then it does not have an
		 *        effect because the map buffer has been rendered already onto
		 *        the painter device.
		 * @param geoReference The geo reference of the image.
		 * @param image The image to be rendered.
		 * @param compositionMode The composition mode which will be used to
		 *                        combine the current base layer (tiles) with
		 *                        the image.
		 * @param filterMode The filter mode used to render the image.
		 */
		void drawImage(const QRectF &geoReference, const QImage &image,
		               CompositionMode compositionMode = CompositionMode_Default,
		               FilterMode filterMode = FilterMode_Auto);

		void draw(QPainter &p);

		void centerMap(const QPoint &centerPnt);
		void translate(const QPoint &delta);
		void translate(const QPointF &delta);

		void drawImageLayer(QPainter &painter);
		void drawVectorLayer(QPainter &painter);

		void drawLayers(QPainter &painter);

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

		//! Canvas does take ownership of layer if the layer does not
		//! have a parent at the time of appending or prepending.
		bool prependLayer(Layer*);
		bool addLayer(Layer*);
		bool insertLayerBefore(const Layer*, Layer*);

		void removeLayer(Layer*);

		void lower(Layer*);
		void raise(Layer*);

		Layer *hoverLayer() const;

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


	public:
		GridLayer *gridLayer();
		const GridLayer *gridLayer() const;

		CitiesLayer *citiesLayer();
		const CitiesLayer *citiesLayer() const;

		GeoFeatureLayer *geoFeatureLayer();
		const GeoFeatureLayer *geoFeatureLayer() const;

		SymbolLayer *symbolCollection();
		const SymbolLayer *symbolCollection() const;


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

		void drawLegends(QPainter &painter);
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

		uint                          _polygonRoughness;
		double                        _maxZoom;
		QPointF                       _center;
		float                         _zoomLevel;
		bool                          _grayScale;
		bool                          _filterMap;
		bool                          _dirtyRasterLayer;
		bool                          _dirtyVectorLayers;
		bool                          _previewMode;
		bool                          _stackLegends;

		Layers                        _layers;
		Layer                        *_hoverLayer;
		CustomLayers                  _customLayers;
		CitiesLayer                   _citiesLayer;
		GridLayer                     _gridLayer;
		GeoFeatureLayer               _geoFeatureLayer;
		SymbolLayer                   _symbolLayer;

		LegendAreas                   _legendAreas;
		int                           _margin;
		bool                          _isDrawLegendsEnabled;
};


inline bool Canvas::previewMode() const {
	return _previewMode;
}

inline Layer *Canvas::hoverLayer() const {
	return _hoverLayer;
}

inline GridLayer *Canvas::gridLayer() {
	return &_gridLayer;
}

inline const GridLayer *Canvas::gridLayer() const {
	return &_gridLayer;
}

inline CitiesLayer *Canvas::citiesLayer() {
	return &_citiesLayer;
}

inline const CitiesLayer *Canvas::citiesLayer() const {
	return &_citiesLayer;
}

inline GeoFeatureLayer *Canvas::geoFeatureLayer() {
	return &_geoFeatureLayer;
}

inline const GeoFeatureLayer *Canvas::geoFeatureLayer() const {
	return &_geoFeatureLayer;
}

inline SymbolLayer *Canvas::symbolCollection() {
	return &_symbolLayer;
}

inline const SymbolLayer *Canvas::symbolCollection() const {
	return &_symbolLayer;
}

inline uint Canvas::polygonRoughness() const {
	return _polygonRoughness;
}

inline void Canvas::setPolygonRoughness(uint pixel) {
	_polygonRoughness = pixel;
}


}
}
}


#endif
