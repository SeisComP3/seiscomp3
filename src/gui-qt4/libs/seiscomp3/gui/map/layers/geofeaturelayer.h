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

#ifndef __SEISCOMP_GUI_MAP_LAYERS_GEOFEATURELAYER_H__
#define __SEISCOMP_GUI_MAP_LAYERS_GEOFEATURELAYER_H__


#include <seiscomp3/gui/map/layer.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/geo/featureset.h>
#include <seiscomp3/geo/index/quadtree.h>
#endif


namespace Seiscomp {
namespace Gui {
namespace Map {


class Canvas;


class SC_GUI_API GeoFeatureLayer : public Layer,
                                   virtual public Geo::GeoFeatureSetObserver {
	Q_OBJECT


	public:
		GeoFeatureLayer(QObject *parent = NULL);
		virtual ~GeoFeatureLayer();


	public:
		/**
		 * @brief Returns a feature under the given coordinate.
		 * @param coord The coordinate.
		 * @return A pointer to the feature instance or NULL. Do not delete
		 *         the feature, its ownership is still at the global
		 *         GeoFeatureSet.
		 */
		const Geo::GeoFeature *findFeature(const Geo::GeoCoordinate &coord) const;

		/**
		 * @brief Convenience function for @findFeature(const Geo::GeoCoordinate &coord).
		 * @param lat The latitude of the coordinate
		 * @param lon The longitude of the coordinate
		 * @return See @findFeature(const Geo::GeoCoordinate &coord).
		 */
		const Geo::GeoFeature *findFeature(qreal lat, qreal lon) const;

		/**
		 * @brief Renders the geofeatures on a given canvas with a given
		 *        painter. This function is called from bufferUpdated.
		 * @param canvas The target canvas
		 * @param painter The painter to paint with
		 */
		void renderFeatures(Canvas *canvas, QPainter &painter);

		virtual void setVisible(bool flag);
		virtual void bufferUpdated(Canvas *canvas, QPainter &painter);

		virtual QMenu *menu(QMenu*) const;

		virtual void geoFeatureSetUpdated();


	private slots:
		void toggleFeatureVisibility(bool);
		void disableFeatureVisibility();
		void showFeatures();
		void hideFeatures();
		void reloadFeatures();


	private:
		void initLayerProperites();


	private:
		struct SC_GUI_API LayerProperties {
			enum SymbolShape {
				Square,
				Circle
			};

			const LayerProperties *parent;
			std::string            name;
			std::string            title;
			Qt::Orientation        orientation;
			Qt::Alignment          legendArea;
			std::string            label;
			int                    index;
			bool                   visible;
			QPen                   pen;
			QBrush                 brush;
			QFont                  font;
			bool                   drawName;
			bool                   debug;
			int                    rank;
			int                    roughness;
			bool                   filled;
			int                    symbolSize;
			SymbolShape            symbolShape;

			LayerProperties(const std::string &name)
			: parent(NULL), name(name), orientation(Qt::Vertical)
			, legendArea(Qt::AlignTop | Qt::AlignLeft)
			, index(0), visible(true), drawName(false)
			, debug(false), rank(-1), roughness(-1)
			, filled(false), symbolSize(8), symbolShape(Circle) {}

			LayerProperties(const std::string &name, const LayerProperties *parent)
			: parent(parent), name(name), orientation(Qt::Vertical)
			, legendArea(Qt::AlignTop | Qt::AlignLeft)
			, index(0), visible(parent->visible), pen(parent->pen)
			, brush(parent->brush), font(parent->font)
			, drawName(parent->drawName), debug(parent->debug)
			, rank(-1), roughness(parent->roughness)
			, filled(parent->filled), symbolSize(parent->symbolSize)
			, symbolShape(parent->symbolShape) {}

			bool isChild(const LayerProperties* child) const;
			void read(const std::string &dataDir = "");

			static SymbolShape getSymbolShape(const std::string &type);
		};


		struct CategoryNode {
			CategoryNode(const Geo::Category*);
			~CategoryNode();

			CategoryNode *nodeForCategory(const Geo::Category *cat);
			CategoryNode *nodeForProperties(const LayerProperties *props);

			const Geo::Category           *category;
			LayerProperties               *properties;
			std::vector<CategoryNode*>     childs;
			Geo::QuadTree                  quadtree;
			Geo::GeoBoundingBox            bbox;
		};

		static bool compareByIndex(const LayerProperties *p1,
		                           const LayerProperties *p2);

		static bool compareNodeByIndex(const GeoFeatureLayer::CategoryNode *n1,
		                               const GeoFeatureLayer::CategoryNode *n2);

		CategoryNode *createOrGetNodeForCategory(const Geo::Category *cat);
		void buildLegends(CategoryNode *node);
		QMenu *buildMenu(CategoryNode *node, QMenu *parentMenu) const;
		void collectLegendItems(CategoryNode *node, QVector<LayerProperties*> &items);
		void orderTree(CategoryNode *node);
		void updateBbox(CategoryNode *node);
		const Geo::GeoFeature *findFeature(CategoryNode *node, const Geo::GeoCoordinate &coord) const;
		bool toggleVisibility(CategoryNode *node, bool visible);
		void drawFeatures(CategoryNode *node, Canvas *canvas,
		                  QPainter &painter, const QPen &debugPen);
		bool drawFeature(Canvas *canvas, QPainter *painter,
		                 const QPen *debugPen, LayerProperties *props,
		                 const Geo::GeoFeature *f);

		bool                       _initialized;
		CategoryNode              *_root;
};


}
}
}


#endif
