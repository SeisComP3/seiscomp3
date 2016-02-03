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
#include <seiscomp3/gui/map/legend.h>
#include <seiscomp3/gui/map/imagetree.h>
#include <seiscomp3/gui/map/mapsymbolcollection.h>
#include <seiscomp3/gui/map/layers/citieslayer.h>
#include <seiscomp3/gui/map/layers/gridlayer.h>
#include <seiscomp3/math/coord.h>
#include <seiscomp3/geo/geofeature.h>
#endif

#include <QHash>
#include <QObject>
#include <QPolygon>

class QMouseEvent;

namespace Seiscomp {
namespace Gui {
namespace Map {

class Layer;
class Projection;

DEFINE_SMARTPOINTER(TextureCache);


struct SC_GUI_API LayerProperties {
	std::string name;
	const LayerProperties *parent;
	bool visible;
	QPen pen;
	QBrush brush;
	QFont font;
	bool drawName;
	bool debug;
	int  rank;
	int  roughness;
	bool filled;

	LayerProperties(const std::string &name) :
	                name(name), parent(NULL), visible(true),
	                drawName(false), debug(false), rank(-1), roughness(3),
	                filled(false) {}
	LayerProperties(const std::string &name, const LayerProperties* parent) :
	                name(name), parent(parent), visible(parent->visible),
	                pen(parent->pen), brush(parent->brush), font(parent->font),
	                drawName(parent->drawName), debug(parent->debug), rank(-1),
	                roughness(parent->roughness), filled(parent->filled) {}

	bool isChild(const LayerProperties* child) const;
};


class CanvasDelegate;

class SC_GUI_API Canvas : public QObject {
	Q_OBJECT

	public:
		Canvas(const MapsDesc &);
		Canvas(ImageTree* mapTree);
		~Canvas();

		void setFont(QFont f);
		QFont font() const { return _font; }

		void setBackgroundColor(QColor c);
		bool setProjectionByName(const char *name);
		const std::string &projectionName() const { return _projectionName; }

		void setPreviewMode(bool);
		void setBilinearFilter(bool);

		void setSize(int w, int h);
		int width() const { return _buffer.width(); }
		int height() const { return _buffer.height(); }

		void setGrayScale(bool);
		bool isGrayScale() const;

		void setDrawGrid(bool);
		bool isDrawGridEnabled() const;

		void setDrawLayers(bool);
		bool isDrawLayersEnabled() const;

		void setDrawCities(bool);
		bool isDrawCitiesEnabled() const;

		void setDrawLegends(bool);
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

		SymbolCollection *symbolCollection() const;
		void setSymbolCollection(SymbolCollection *collection);

		void setSelectedCity(const Math::Geo::CityD*);

		//! Draws a geometric line (great circle) given in geographical coordinates (lon, lat)
		//! Returns the distance in degree of the line
		double drawGeoLine(QPainter& p, const QPointF& start, const QPointF& end) const;

		//! Draws a geofeature with layer properties.
		bool drawGeoFeature(QPainter &painter, const Geo::GeoFeature *f,
		                    const LayerProperties *props, const QPen &debugPen,
		                    size_t &linesPlotted, size_t &polyPlotted,
		                    bool filled) const;

		//! Draws a polyline in geographical coordinates (lon, lat).
		//! Returns number of line segments drawn.
		//! Does not check for clipping
		size_t drawGeoPolyline(QPainter& p, size_t n, const Math::Geo::CoordF *line,
		                       bool isClosedPolygon, uint minPixelDist = 3,
		                       bool interpolate = false) const;
		//! Does not check for clipping
		size_t drawGeoPolygon(QPainter& p, size_t n, const Math::Geo::CoordF *line,
		                      uint minPixelDist = 3) const;
		//! Draws a GeoFeature as either polyline or filled polygon and
		//! checks for clipping
		size_t drawGeoFeature(QPainter& p, const Geo::GeoFeature *feature,
		                      uint minPixelDist = 3, bool interpolate = false,
		                      bool filled = false) const;

		void draw(QPainter& p);

		void centerMap(const QPoint &centerPnt);
		void translate(const QPoint &delta);
		void translate(const QPointF &delta);

		//! Returns a reference to the layerproperties vector
		const std::vector<LayerProperties*> &layerProperties() {
			return _layerProperties;
		}

		void drawImageLayer(QPainter&);
		void drawVectorLayer(QPainter&);

		void drawGeoFeatures(QPainter& p);
		void drawLayers(QPainter& p);
		void drawDrawables(QPainter& p);
		void drawImage(const QRectF &geoReference, const QImage &image);

		void updateBuffer();

		void setBuffer(QImage buffer) { _buffer = buffer; }
		QImage &buffer() { return _buffer; }


		CanvasDelegate* delegate() const { return _delegate; }
		void setDelegate(CanvasDelegate *delegate);

		//! Returns the number of layers
		int layerCount() const { return _layers.count(); }

		//! Returns the i-th layer if the index is valid
		Layer* layer(int i) const {
			if ( i < 0 || i >= _layers.count() ) return NULL;

			return _layers[i];
		}

		//! Canvas does not take ownership of layer.
		void addLayer(Layer*);
		void removeLayer(Layer*);

		void lower(Layer*);
		void raise(Layer*);

		bool filterContextMenuEvent(QContextMenuEvent*, QWidget*);
		bool filterMouseMoveEvent(QMouseEvent*);
		bool filterMouseDoubleClickEvent(QMouseEvent*);
		bool filterMousePressEvent(QMouseEvent*);

		QMenu* menu(QWidget*) const;

		void setMargin(int);

		void setVisible(Legend*);

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

		void bringToFront(Seiscomp::Gui::Map::Legend*);
		void onObjectDestroyed(QObject *object);
		void setLegendEnabled(Seiscomp::Gui::Map::Legend*, bool);

	signals:
		//! This signal is emitted if draw() caused asynchronous data requests
		//! and when those requests are finished.
		//! This signal was introduced with API 1.1.
		void renderingCompleted();

		void bufferUpdated();
		void projectionChanged(Seiscomp::Gui::Map::Projection*);
		void updateRequested();
		void customLayer(QPainter*);

	private:
		void init();

		void zoomGridIn();
		void zoomGridOut();

		void updateDrawablePositions() const;

		void drawCity(QPainter &painter, const Math::Geo::CityD &,
		              QVector< QList<QRect> > &grid,
		              QFont &font, QFontMetrics &fontMetrics,
		              int rowHeight,
		              bool &lastUnderline, bool &lastBold);

		void drawDrawables(QPainter& painter, Symbol::Priority priority);

		void drawLegends(QPainter&);

		void updateLayout();

		/**
		 * Initializes the layer property vector with properties read
		 * from the symbol collection.
		 */
		void initLayerProperites();

		int polyToCache(size_t n, const Math::Geo::CoordF *points,
		                uint minPixelDist) const;


	private slots:
		void updatedTiles();
		void updateLayer(const Layer::UpdateHints&);

	private:
		typedef QVector<Legend*> Legends;
		struct LegendArea : public Legends {
			LegendArea() : currentIndex(-1), lastIndex(-1) {}

			bool hasIndex(int index) {
				return ( index >= 0 && index < count() );
			}

			bool mousePressEvent(QMouseEvent *e );

			int findNext(bool forward = true) const {
				int count = this->count(),
				    index = currentIndex,
				    tmp = forward ? 1 : -1;

				index = currentIndex;
				for ( int i = 0; i < count - 1; ++i ) {
					index += tmp;
					if ( index < 0 || index >= count ) {
						if ( forward )
							index = 0;
						else
							index = count -1;
					}

					if ( this->at(index)->isEnabled() ) return index;
				}

				return -1;
			}

			QRect      header;
			QRect      decorationRects[2];
			int        currentIndex;
			int        lastIndex;
		};

		typedef QHash<Qt::Alignment, LegendArea> LegendAreas;

		typedef QList<Layer*> Layers;
		typedef boost::shared_ptr<SymbolCollection> SymbolCollectionPtr;

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
		bool                          _drawLayers;
		bool                          _filterMap;
		bool                          _dirtyImage;
		bool                          _dirtyLayers;
		bool                          _previewMode;

		SymbolCollectionPtr           _mapSymbolCollection;
		std::vector<LayerProperties*> _layerProperties;

		Layers                        _layers;
		CitiesLayer                   _citiesLayer;
		GridLayer                     _gridLayer;

		LegendAreas                   _legendAreas;
		int                           _margin;
		bool                          _isDrawLegendsEnabled;
		CanvasDelegate               *_delegate;

		mutable QPolygon              _polyCache;
};

class CanvasDelegate : public QObject {
	public:
		struct Margins {
			int left;
			int top;
			int right;
			int bottom;
		};

	public:
		CanvasDelegate(Canvas *canvas, QObject *parent = NULL)
		    : QObject(parent), _canvas(canvas), _spacing(6) {
			int margin = 9;
			setContentsMargins(margin, margin, margin, margin);
		}

		Canvas* canvas() { return _canvas; }
		void setCanas(Canvas* canvas) { _canvas = canvas; }

		virtual void doLayout() = 0;
		virtual void drawLegends(QPainter &painter) {}

		virtual bool filterMouseDoubleClickEvent(QMouseEvent *mouseEvent) {
			return false;
		}

		virtual bool filterMousePressEvent(QMouseEvent *mouseEvent) {
			return false;
		}

		Margins margins() const { return _margins; }
		void setContentsMargins(int left, int top, int right, int bottom) {
			_margins.left = left;
			_margins.top = top;
			_margins.right = right;
			_margins.bottom = bottom;
		}

		int spacing() const { return _spacing; }
		void setSpacing(int spacing) { _spacing = spacing; }

	protected:
		Canvas        *_canvas;
		Margins        _margins;
		int            _spacing;
};

}
}
}

#endif
